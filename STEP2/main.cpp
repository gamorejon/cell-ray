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

const int   X_PIXELS = 640; // 1280;
const int   Y_PIXELS = 640; // 800;
const float PLANE_DISTANCE = 2.0f;
const float PLANE_ANGLE = 1.0f;
const float GRID_SIZE = 1.0f;
const int   DEFAULT_MAX_REFLECTIONS = 500;
const float DEFAULT_ZOOM = 0.065;
const float REFLECTIVE = 0.85;
const char* FILE_NAME = "myfile.bmp";

static RayTraceState* state;

static Raytracer raytracer;

void raytrace(Ray* ray, ColorGroup* color, Scene* scene, 
	RayTraceParameters* params, Intersection& intersection);

RayTraceState* getState();

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

	state = getState();

//	printf("Original camera: (%f, %f, %f), %p\n", state->camera.x, state->camera.y, state->camera.z, state);

	long stateTime = getMsTime() - start;

	PpuPThreadData data;

	/* Create a SPE context */
	if ((data.spe_ctx = spe_context_create (0, NULL)) == NULL) 
	{
	    perror ("Failed creating context");
    	exit (1);
	}

	/* Load SPE program into the SPE context*/
	if (spe_program_load (data.spe_ctx, &raytracer_spe))  
	{
		perror ("Failed loading program");
	    exit (1);
	}

	/* Initialize context run data */
	data.argp = state;

	/* Create pthread for each of the SPE contexts */
	if (pthread_create (&data.pthread, NULL, &ppu_pthread_function, &data)) 
	{
		perror ("Failed creating thread");
		exit (1);
	}

	/* Wait for the threads to complete */
	if (pthread_join (data.pthread, NULL)) 
	{
    	perror ("Failed joining thread\n");
	    exit (1);
	}

	/* Destroy SPE context */
	if (spe_context_destroy(data.spe_ctx) != 0) 
	{
		perror("Failed destroying context");
		exit (1);
	}

	long spuTime = getMsTime() - start - stateTime;

	RayTraceParameters* params = state->params;
	volatile ColorGroup* pixels = state->pixels;

	BMP myImage;

	myImage.SetSize(params->xPixels, params->yPixels);
	myImage.SetBitDepth(24);

	int xPixel_4 = params->xPixels / 4;

	VectorSInt rInt;
	VectorSInt gInt;
	VectorSInt bInt;

	for (int i = 0; i < params->yPixels; i++)
	{
		for (int j = 0; j < xPixel_4; j++)
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
		}
	}

    long processingTime = getMsTime() - start - stateTime - spuTime;

    myImage.WriteToFile(FILE_NAME);

    long fileWritingTime = getMsTime() - start - stateTime - spuTime - processingTime;

	long time = getMsTime() - start;

    printf("Finished.  Total time: %li (state: %li, spu: %li, processing: %li, file: %li)\n", time, stateTime, spuTime, processingTime, fileWritingTime);

/* cout "File writing time Ray casting elapsed time: " << rayCasting << " ms (setup: "
         << setupTime << " ms, tracing: " << tracing << " ms, file writing: "
        << fileWritingTime << " ms)" << endl;*/

	return(0);
}

RayTraceState* getState()
{
    float ratio = (float)X_PIXELS / (float)Y_PIXELS;

    float zoom = DEFAULT_ZOOM;

    RayTraceParameters* params = new RayTraceParameters(zoom * ratio, zoom,
		PLANE_DISTANCE, PLANE_ANGLE, X_PIXELS, Y_PIXELS,
		DEFAULT_MAX_REFLECTIONS);

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

	float p = 0.45f;

    Vertex* camera = new Vertex( p, -p, -p);
    Vertex* zBasis = new Vertex(-1,  1,  1);
    Vertex* yBasis = new Vertex(-.5,-1, .5);

	zBasis->normalize();
	yBasis->normalize();

    RayTraceState* state = new RayTraceState(camera, yBasis, zBasis, params, scene);

    return state;
}

