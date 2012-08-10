#include <math.h>
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

const int   X_PIXELS = 640;
const int   Y_PIXELS = 640;
const float PLANE_DISTANCE = 2.0f;
const float PLANE_ANGLE = 1.0f;
const float GRID_SIZE = 1.0f;
const int   DEFAULT_MAX_REFLECTIONS = 500;
const float DEFAULT_ZOOM = 0.065;
const float REFLECTIVE = 0.85;

static RayTraceParameters* params;

static Scene* scene;

static RayTraceState* state;

static Raytracer raytracer;

static ColorGroup* pixels;

void raytrace(Ray* ray, ColorGroup* color, Scene* scene, 
	RayTraceParameters* params, Intersection& intersection);

RayTraceState* getState();

void display(char* fileName);

inline unsigned int round(const int& toRound);

int main()
{
	state = getState();

	params = state->params;

	scene = state->scene;

	int size = sizeof(ColorGroup) * params->xPixels / 4 * params->yPixels;

	pixels = (ColorGroup*)aligned_alloc(size, 1024);

	display("myfile.bmp");

	aligned_free(pixels);
}

void display(char* fileName)
{
	Ray ray;

	long startTime = getMsTime();
	long setupTime = 0;
	long fileWritingTime = 0;

    Intersection intersection;

    if (!state->drawn)
    {
		vector float negZero = (vector float) vec_splat_u32(-1);
		negZero = (vector float) vec_sl( (vector unsigned int) negZero, (vector unsigned int) negZero);

		VertexGroup raystart = VertexGroup(*state->camera);

		float xStepFactor __attribute__ ((aligned (16))) = 4.0 * params->screenXStep;
		float yStepFactor __attribute__ ((aligned (16))) = params->screenYStep;

		VertexGroup start = VertexGroup(*state->camera);
		VertexGroup xStep = VertexGroup(*state->xBasis * xStepFactor);
		VertexGroup yStep = VertexGroup(*state->yBasis * yStepFactor);

		VertexGroup screenCorner = VertexGroup((*state->screenCorner - *state->camera));

		vector float offset = (vector float){0, .25, .5, .75};

		VertexGroup pixel = xStep.madd(offset, screenCorner);

		BMP myImage;

		myImage.SetSize(params->xPixels, params->yPixels);
		myImage.SetBitDepth(24);

		VertexGroup rowPixel = VertexGroup(Vertex());
		VertexGroup raydir = VertexGroup(Vertex());

		vector bool int allHit = vec_cmplt(negativeZero(), vectorOne());

		VectorSInt rInt;
		VectorSInt gInt;
		VectorSInt bInt;

		int xPixel_4 = params->xPixels / 4;

		setupTime = getMsTime() - startTime;

		for (int i = 0; i < params->yPixels; i++)
		{
			rowPixel = pixel;
		
			for (int j = 0; j < xPixel_4; j++)
			{
				raydir = rowPixel;

				raydir.normalize();

				int index = i * xPixel_4 + j;

				ray.start = &raystart;
				ray.direction = &raydir;
				intersection.hit = allHit;

                raytracer.raytrace(&ray, &pixels[index], scene, params, 
					intersection);
			
				// cout << "Raycasting (" << index << ") - (" << 4*j << "," << i << ")" << endl;
				
				rInt.vec = vec_cts(pixels[index].r, 8);
				gInt.vec = vec_cts(pixels[index].g, 8);
				bInt.vec = vec_cts(pixels[index].b, 8);

				for (int k = 0; k < 4; k++)
				{
       	        	myImage(4*j+k,i)->Red   = round(rInt.points[k]);
	           	    myImage(4*j+k,i)->Green = round(gInt.points[k]);
    	           	myImage(4*j+k,i)->Blue  = round(bInt.points[k]);
				}

				rowPixel += xStep;
			}

			pixel -= yStep;
		}

		long fileStart = getMsTime();

		myImage.WriteToFile(fileName);

		fileWritingTime = getMsTime() - fileStart;

	    state->drawn = true;
	}

	long rayCasting = getMsTime() - startTime;

	long tracing = rayCasting - setupTime - fileWritingTime;

	cout << "Ray casting elapsed time: " << rayCasting << " ms (setup: "
		 << setupTime << " ms, tracing: " << tracing << " ms, file writing: " 
		<< fileWritingTime << " ms)" << endl;
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

	Light** lights = new Light*[lightsSize];	

	float pri = 0.65;
	float sec = 0.05;
	float lInt = .35;

	lights[0] = new Light(new Vertex( b,  b,  b), 100, pri, sec, sec);
	lights[1] = new Light(new Vertex(-b,  b, -b), 100, sec, pri, sec);
	lights[2] = new Light(new Vertex(-b, -b,  b), 100, sec, sec, pri);
	lights[3] = new Light(new Vertex( b, -b, -b), 100, lInt, lInt, lInt);

	int objectsSize = 5;

	SceneObject** objects = new SceneObject*[objectsSize];

    float radius = sqrt(2) / 2;
    float reflective __attribute__ ((aligned (16))) = REFLECTIVE;

    float specularReflection __attribute__ ((aligned (16))) = 0.08;

	float c = 0.35;

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

