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
	printf("Begin: sizeof(ColorGroup: %d)\n", sizeof(ColorGroup));

	volatile RayTraceState* state = (volatile RayTraceState*)aligned_alloc(sizeof(RayTraceState), 7);
	volatile ColorGroup* pixels = (volatile ColorGroup*)
		aligned_alloc(sizeof(ColorGroup)*COLOR_GROUPS_PER_BLOCK, 7);
	Raytracer raytracer;

	// Reserve a tag ID
	unsigned int tag_id = mfc_tag_reserve();
	  
	spu_writech(MFC_WrTagMask, -1);

	printf("2\n");

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

	printf("3\n");

	Scene* scene = state->scene;

	Light* lights = scene->getLights();

	spu_mfcdma32(lights, (unsigned int)lights, 
		scene->getLightsSize() * sizeof(Light), tag_id, MFC_GET_CMD);

	SceneObject** objects = scene->getSceneObjects();

	printf("4\n");

	spu_mfcdma32(objects, (unsigned int)objects, 
		sizeof(SceneObject) * scene->getSceneObjectsSize(), 
		tag_id, MFC_GET_CMD);

	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

	printf("5\n");

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

		float xStepFactor __attribute__ ((aligned (16))) = 4.0 * params->screenXStep;
		float yStepFactor __attribute__ ((aligned (16))) = params->screenYStep;

		VertexGroup xStep = VertexGroup(const_cast<volatile Vertex&>(state->xBasis) * xStepFactor);
		VertexGroup yStep = VertexGroup(const_cast<volatile Vertex&>(state->yBasis) * yStepFactor);

		VertexGroup screenCorner = VertexGroup(const_cast<volatile Vertex&>(state->screenCorner) - const_cast<volatile Vertex&>(state->camera));

		vector float offset = (vector float){0, .25, .5, .75};

		VertexGroup pixel = xStep.madd(offset, screenCorner);

		VertexGroup rowPixel = VertexGroup(Vertex());

		vector unsigned int allHit = vec_cmplt(negativeZero(), vectorOne());

		volatile Cell_addr64* ptr __attribute__ ((aligned (16))) = new Cell_addr64();

		int xPixel_4 = params->xPixels / 4;

		int blockIndex = 0;

		printf("Pixels is: %p, %p\n", pixels, state->pixels);

		for (int i = 0; i < params->yPixels; i++)
		{
			rowPixel = pixel;
		
			for (int j = 0; j < xPixel_4; j++)
			{
				ray.start = state->camera;

				ray.direction = rowPixel;

				ray.direction.normalize();

				intersection.hit = allHit;

                raytracer.raytrace(&ray, &pixels[blockIndex], scene, params, 
					intersection);

				if (++blockIndex == COLOR_GROUPS_PER_BLOCK)
				{
					int index = i * xPixel_4 + j - blockIndex + 1;

					ptr->p = state->pixels + index;

					spu_dma_nowait((void *)(pixels), ptr, 
                        sizeof(ColorGroup)*blockIndex, tag_id, MFC_PUT_CMD);

					(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);

					blockIndex = 0;
				}

				rowPixel += xStep;
			}

			pixel -= yStep;
		}

	    state->drawn = true;
	}

//	cout << "Ray casting elapsed time: " << rayCasting << " ms (setup: "
//		 << setupTime << " ms, tracing: " << tracing << " ms, file writing: " 
//		<< fileWritingTime << " ms)" << endl;

	(void)spu_mfcstat(MFC_TAG_UPDATE_ALL);
}

