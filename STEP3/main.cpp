#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <libspe2.h>
#include <pthread.h>
#include <iostream>
using namespace std;
#include "TimeUtils.h"
#include "RayTraceParameters.h"
#include "RayTraceState.h"
#include "Ray.h"
#include "Vertex.h"
#include "VertexGroup.h"
#include "Color.h"
#include "Scene.h"
#include "Intersection.h"
#include "Sphere.h"
#include "EasyBMP.h"
#include "VectorSInt.h"
#include "Raytracer.h"

#define MAX_SPE_THREADS 16

const int   X_PIXELS = 1280;
const int   Y_PIXELS = 800;
const float CAMERA_DISTANCE = 0.45f;
const float PLANE_DISTANCE = 2.0f;
const float PLANE_ANGLE = 1.0f;
const float GRID_SIZE = 1.0f;
const int   DEFAULT_MAX_REFLECTIONS = 500;
const float DEFAULT_ZOOM = 0.065;
const float REFLECTIVE = 0.85;
const char* FILE_NAME = "myfile.bmp";

static Raytracer raytracer;

void raytrace(Ray* ray, ColorGroup* color, Scene* scene, 
	RayTraceParameters* params, Intersection& intersection);

Scene* getScene();

RayTraceState* getState(ColorGroup* pixels, Scene* scene);

void display(char* fileName, RayTraceState* state);

inline unsigned int round(const int& toRound);

extern spe_program_handle_t raytracer_spe;

struct PpuPThreadData 
{
  spe_context_ptr_t spe_ctx;
  pthread_t pthread;
  void *argp;
};

void *ppu_pthread_function(void *arg) 
{
	PpuPThreadData *datap = (PpuPThreadData*)arg;
	unsigned int entry = SPE_DEFAULT_ENTRY;
	if (spe_context_run(datap->spe_ctx, &entry, 0, datap->argp, NULL, NULL) < 0)	{
    	perror ("Failed running context");
	    exit (1);
	}

	pthread_exit(NULL);
}

inline unsigned int round(const int& toRound)
{
    if (toRound < 0)
    {
        return 0;
    }
    else if (toRound > 255)
    {
        return 255;
    }
    else
    {
        return toRound;
    }
}

int main()
{
	long start = getMsTime();

	//state = getState();

//	printf("Original camera: (%f, %f, %f), %p\n", state->camera.x, state->camera.y, state->camera.z, state);

	PpuPThreadData datas[MAX_SPE_THREADS];

	int spe_threads = spe_cpu_info_get(SPE_COUNT_USABLE_SPES, -1);

	int count;
	
	if (spe_threads > MAX_SPE_THREADS) 
		spe_threads = MAX_SPE_THREADS;

	Scene* scene = getScene();

	int numberOfColorGroups = (int)round((X_PIXELS*Y_PIXELS)/4.0);
    int size = sizeof(ColorGroup)*numberOfColorGroups;

    ColorGroup* pixels = (ColorGroup*)aligned_alloc(size, 10);

    BMP myImage;

    myImage.SetSize(X_PIXELS, Y_PIXELS);
    myImage.SetBitDepth(24);

    int xPixel_4 = X_PIXELS / 4;

    VectorSInt rInt;
    VectorSInt gInt;
    VectorSInt bInt;

	long stateTime = getMsTime() - start;

	long spuCreationTime = getMsTime();

	for (int i = 0, offset = 0; i < spe_threads; i++, offset += count)
	{
		/* Create a SPE context */
		if ((datas[i].spe_ctx = spe_context_create (0, NULL)) == NULL) 
		{
		    perror ("Failed creating context");
	    	exit (1);
		}

		/* Load SPE program into the SPE context*/
		if (spe_program_load (datas[i].spe_ctx, &raytracer_spe))  
		{
			perror ("Failed loading program");
		    exit (1);
		}

		/* Initialize context run data */
		RayTraceState* state = getState(pixels + offset, scene);

		count = (numberOfColorGroups / spe_threads + 1023) & ~1023;
		state->offset = offset;
		state->pixelsToProcess = (i == spe_threads - 1) ? 
			numberOfColorGroups - offset : count;
		
		datas[i].argp = state;

		//printf("Pixels: %p, ncg: %d, count: %d, offset: %d\n", pixels + offset, numberOfColorGroups, count, offset);

		/* Create pthread for each of the SPE contexts */
		if (pthread_create (&datas[i].pthread, NULL, &ppu_pthread_function, &datas[i])) 
		{
			perror ("Failed creating thread");
			exit (1);
		}
	}

	spuCreationTime = getMsTime() - spuCreationTime;

	long processingTime = 0;
	long totalSpuTime = 0;

	for (int spe = 0; spe < spe_threads; spe++)
	{
		long spuTime = getMsTime();

		/* Wait for the threads to complete */
		if (pthread_join (datas[spe].pthread, NULL)) 
		{
	    	perror ("Failed joining thread\n");
	    	exit (1);
		}

        if (spe_context_destroy(datas[spe].spe_ctx) != 0)
        {
            perror("Failed destroying context");
            exit (1);
        }

		spuTime = getMsTime() - spuTime;
		totalSpuTime += spuTime;

		printf("Time waiting for spu %d: %li\n", spe, spuTime);

		long tidbitProcessingTime = getMsTime();

		RayTraceState* state = (RayTraceState*)datas[spe].argp;

		int offset = state->offset;
		int row = offset / xPixel_4;
		int column = offset % xPixel_4;

		int pixelsToProcess = state->pixelsToProcess;
		int pixelsProcessed = 0; 

		bool breakOut = false;

		printf("Now outputting pixels for row %d, column %d\n", row, column);

		int i = 0, j = 0;

        for (i = row; i < Y_PIXELS; i++)
        {
            for (j = column; j < xPixel_4; j++)
            {
                int index = i * xPixel_4 + j;

                rInt.vec = vec_cts(pixels[index].r, 8);
                gInt.vec = vec_cts(pixels[index].g, 8);
                bInt.vec = vec_cts(pixels[index].b, 8);

                for (int k = 0; k < 4; k++)
                {
                    myImage(4*j+k,i)->Red   = round(rInt.points[k]);
                    myImage(4*j+k,i)->Green = round(gInt.points[k]);
                    myImage(4*j+k,i)->Blue  = round(bInt.points[k]);
                }

				if (++pixelsProcessed == pixelsToProcess)
				{
					breakOut = true;
					break;
				}
            }

			column = 0;

			if (breakOut)
			{
				break;
			}
        }


		tidbitProcessingTime = getMsTime() - tidbitProcessingTime;
		processingTime += tidbitProcessingTime;

		printf("Processing %d pixels (offset: %d, up to row %d, column %d) for spu %d: %li\n", pixelsToProcess, offset, i, 4*j + 3, spe, tidbitProcessingTime);
	}

	long fileWritingTime = getMsTime();

    myImage.WriteToFile(FILE_NAME);

    fileWritingTime = getMsTime() - fileWritingTime;

	long time = getMsTime() - start;

    printf("Finished.  Total time: %li (state: %li, spu creation: %li, spu execution: %li, processing: %li, file: %li)\n", time, stateTime, spuCreationTime, totalSpuTime, processingTime, fileWritingTime);

	return(0);
}

Scene* getScene()
{
    float a = 0.5;
    float b = 2*a;

    int lightsSize = 4;

    Light* lights = (Light*)aligned_alloc(sizeof(Light)*lightsSize, 7);

    float pri  = 0.65;
    float sec  = 0.05;
    float lInt = 0.35;

    lights[0] = Light(new Vertex( b,  b,  b), 100, pri, sec, sec);
    lights[1] = Light(new Vertex(-b,  b, -b), 100, sec, pri, sec);
    lights[2] = Light(new Vertex(-b, -b,  b), 100, sec, sec, pri);
    lights[3] = Light(new Vertex( b, -b, -b), 100, lInt, lInt, lInt);

    float radius = sqrt(2) / 2;
    float reflective __attribute__ ((aligned (16))) = REFLECTIVE;

    float specularReflection __attribute__ ((aligned (16))) = 0.08;

    float c = 0.35;

    int objectsSize = 5;

    SceneObject** objects = (SceneObject**)aligned_alloc(
        objectsSize * sizeof(SceneObject*), 7);

    objects[0] = new Sphere(radius, new Vertex(-a, -a, -a),
        reflective, new Color(c, c, c), specularReflection);
    // left sphere
    objects[1] = new Sphere(radius, new Vertex( a,  a, -a),
    	reflective, new Color(c, c, c), specularReflection);
    // right sphere
    objects[2] = new Sphere(radius, new Vertex( a, -a,  a),
        reflective, new Color(c, c, c), specularReflection);
    // back sphere
    objects[3] = new Sphere(radius, new Vertex(-a,  a,  a),
    	reflective, new Color(c, c, c), specularReflection);
    // center sphere
    objects[4] = new Sphere(sqrt(0.75) - radius, new Vertex(0, 0, 0),
        reflective, new Color(c, c, c), specularReflection);

    Color* ambientColor = new Color(0.05, 0.05, 0.05);

    Scene* scene = new Scene(lights, lightsSize, objects, objectsSize,
        ambientColor);

	return scene;
}

RayTraceState* getState(ColorGroup* pixels, Scene* scene)
{
    float ratio = (float)X_PIXELS / (float)Y_PIXELS;

    float zoom = DEFAULT_ZOOM;

    RayTraceParameters* params = new RayTraceParameters(zoom * ratio, 
		zoom, PLANE_DISTANCE, PLANE_ANGLE, X_PIXELS, Y_PIXELS, 
		DEFAULT_MAX_REFLECTIONS);

	float p = CAMERA_DISTANCE;

    Vertex* camera = new Vertex( p, -p, -p);
    Vertex* zBasis = new Vertex(-1,  1,  1);
    Vertex* yBasis = new Vertex(-.5,-1, .5);
    Vertex* xBasis = new Vertex();

    zBasis->normalize();
    yBasis->normalize();
    zBasis->cross(*yBasis, *xBasis);

    float oneHalf __attribute__ ((aligned (16))) = 0.5;

    Vertex cornerV;
    Vertex* screenCorner = new Vertex();

    float screenWidth = zoom*ratio;
    float screenHeight = zoom;

    cornerV.x = oneHalf*(screenHeight*yBasis->x - screenWidth*xBasis->x);
    cornerV.y = oneHalf*(screenHeight*yBasis->y - screenWidth*xBasis->y);
    cornerV.z = oneHalf*(screenHeight*yBasis->z - screenWidth*xBasis->z);
	
	float dist __attribute__ ((aligned (16))) = sqrt(cornerV.x * cornerV.x + 
		cornerV.y * cornerV.y + cornerV.z * cornerV.z) / PLANE_ANGLE;

    screenCorner->x = camera->x + dist * zBasis->x + cornerV.x;
    screenCorner->y = camera->y + dist * zBasis->y + cornerV.y;
    screenCorner->z = camera->z + dist * zBasis->z + cornerV.z;

    RayTraceState* state = new RayTraceState(camera, yBasis, zBasis, 
		screenCorner, params, scene, pixels);

    return state;
}

