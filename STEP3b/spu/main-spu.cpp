//#include <math.h>

#include <spu_intrinsics.h>
extern "C" {
#include <spu_mfcio.h>
}
#include <vmx2spu.h>

//#include "TimeUtils.h"
#include "RayTraceParameters.h"
#include "RayTraceState.h"
#include "Ray.h"
#include "Vertex.h"
#include "VertexGroup.h"
#include "Color.h"
#include "Scene.h"
#include "Intersection.h"
#include "Sphere.h"
#include "Raytracer.h"
#include "SpuUtils.h"

#define COLOR_GROUPS_PER_BLOCK 256

int main(unsigned long long spe_id, unsigned long long parm)
{
	volatile RayTraceState* state = (volatile RayTraceState*)aligned_alloc(sizeof(RayTraceState), 7);
	volatile ColorGroup* pixels = (volatile ColorGroup*)
		aligned_alloc(sizeof(ColorGroup)*COLOR_GROUPS_PER_BLOCK, 7);
	Raytracer raytracer;

	// Reserve a tag ID
	unsigned int tag_id = mfc_tag_reserve();
	  
	spu_writech(MFC_WrTagMask, -1);

	// Input parameter parm is a pointer to the particle parameter context.
	// Fetch the context, waiting for it to complete.
	spu_mfcdma32((void *)state, (unsigned int)parm, sizeof(RayTraceState), 
		tag_id, MFC_GET_CMD);
	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

	spu_mfcdma32((void*)state->params, (unsigned int)state->params, 
		sizeof(RayTraceParameters), tag_id, MFC_GET_CMD);
	spu_mfcdma32((void*)state->scene, (unsigned int)state->scene,
		sizeof(Scene), tag_id, MFC_GET_CMD);
	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

	Scene* scene = state->scene;

	Light* lights = scene->getLights();

	spu_mfcdma32(lights, (unsigned int)lights, 
		scene->getLightsSize() * sizeof(Light), tag_id, MFC_GET_CMD);

	SceneObject** objects = scene->getSceneObjects();

	spu_mfcdma32(objects, (unsigned int)objects, 
		sizeof(SceneObject) * scene->getSceneObjectsSize(), 
		tag_id, MFC_GET_CMD);

	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

	for (int i = 0; i < scene->getSceneObjectsSize(); i++)
	{
		SceneObject* object = scene->getSceneObject(i);

		spu_mfcdma32(object, (unsigned int)object, sizeof(Sphere),
			tag_id, MFC_GET_CMD); 
	}

	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

	RayTraceParameters* params = state->params;
  
    if (!state->drawn)
    {
		Ray ray;
		Intersection intersection;

		vector float negZero = (vector float) vec_splat_u32(-1);
		negZero = (vector float) vec_sl( (vector unsigned int) negZero, (vector unsigned int) negZero);

		ray.start = VertexGroup((const Vertex&)state->camera);

		float xStepFactorV __attribute__ ((aligned (16))) = 4.0 * params->screenXStep;
		float yStepFactorV __attribute__ ((aligned (16))) = params->screenYStep;

		vector float xStepFactor = splatScalar(xStepFactorV);
		vector float yStepFactor = splatScalar(yStepFactorV);

		VertexGroup xStep = VertexGroup(
			vec_madd(state->xBasis.vec, xStepFactor, negZero));
		VertexGroup yStep = VertexGroup(
			vec_madd(state->yBasis.vec, yStepFactor, negZero));

		VertexGroup screenCorner = VertexGroup(vec_sub(state->screenCorner.vec,
			state->camera.vec));

		vector float offset = (vector float){0, .25, .5, .75};

		vector unsigned int allHit = vec_cmplt(negativeZero(), vectorOne());

		volatile Cell_addr64* ptr __attribute__ ((aligned (16))) = new Cell_addr64();

		int xPixel_4 = params->xPixels / 4;

		int blockIndex = 0;
		int totalPixels = 0;

		// THE PROBLEM IS HERE: this is trying to do the whole picture, needs
		// to start from the screen corner and only do state->pixelsToProcess
		// pixels 

		int startRow __attribute__ ((aligned (16))) = state->offset / xPixel_4;
		float startRowF __attribute__ ((aligned (16))) = -(startRow);
		int startColumn __attribute__ ((aligned (16))) = 
			state->offset % xPixel_4;
		float startColumnF __attribute__ ((aligned (16))) = (startColumn);

		bool breakout = false;

        VertexGroup pixel = xStep.madd(offset, screenCorner);
		pixel = xStep.madd(splatScalar(startColumnF), pixel);
		pixel = yStep.madd(splatScalar(startRowF), pixel);
        VertexGroup rowPixel = VertexGroup(Vertex());

		startColumnF *= -1;

		int i, j;

		for (i = startRow; i < params->yPixels; i++)
		{
			rowPixel = pixel;
		
			for (j = startColumn; j < xPixel_4; j++)
			{
				ray.start = state->camera;

				ray.direction = rowPixel;

				ray.direction.normalize();

				intersection.hit = allHit;

                raytracer.raytrace(&ray, &pixels[blockIndex], scene, params, 
					intersection);

				if (++blockIndex == COLOR_GROUPS_PER_BLOCK)
				{
					int index = totalPixels - blockIndex + 1;

					ptr->p = state->pixels + index;

					spu_dma_nowait((void *)(pixels), ptr, 
                        sizeof(ColorGroup)*blockIndex, tag_id, MFC_PUT_CMD);

					(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

					blockIndex = 0;
				}

				if (++totalPixels == state->pixelsToProcess)
                {
					breakout = true;
                    break;
                }

				rowPixel.x = vec_add(rowPixel.x, xStep.x);
				rowPixel.y = vec_add(rowPixel.y, xStep.y);
				rowPixel.z = vec_add(rowPixel.z, xStep.z);
			}

			pixel.x = vec_sub(pixel.x, yStep.x);
			pixel.y = vec_sub(pixel.y, yStep.y);
			pixel.z = vec_sub(pixel.z, yStep.z);

			if (startColumn != 0)
			{
				vector float startColumnV = splatScalar(startColumnF);

				pixel.x = vec_madd(xStep.x, startColumnV, pixel.x);
				pixel.y = vec_madd(xStep.y, startColumnV, pixel.y);
				pixel.z = vec_madd(xStep.z, startColumnV, pixel.z);

				startColumn = 0;
			}

			if (breakout)
			{
				break;
			}
		}

	    state->drawn = true;

	    if (blockIndex != 0)
    	{
	        int index = i * xPixel_4 + j - blockIndex + 1;

    	    ptr->p = state->pixels + index;

        	spu_dma_nowait((void *)(pixels), ptr,
            	sizeof(ColorGroup)*blockIndex, tag_id, MFC_PUT_CMD);

	        (void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

    	    blockIndex = 0;
	    }
	}

	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

	aligned_free(state);
	aligned_free(pixels);
}

