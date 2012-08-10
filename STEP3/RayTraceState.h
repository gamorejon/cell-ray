#ifndef STATE_H
#define STATE_H

#include "Vertex.h"
#include "RayTraceParameters.h"
#include "Scene.h"
#include "MemUtils.h"
#include "AlignedStruct.h"

struct RayTraceState : public AlignedStruct
{
public:
    Vertex camera;
    Vertex xBasis;
    Vertex yBasis;
    Vertex zBasis;
    Vertex screenCorner;
    RayTraceParameters* params;
    Scene* scene;
    bool drawn;
	volatile ColorGroup* pixels;
	int offset;
	int pixelsToProcess;

    RayTraceState(Vertex* _camera, Vertex* _yBasis, Vertex* _zBasis,
        Vertex* _screenCorner, RayTraceParameters* _params, Scene* _scene,
		volatile ColorGroup* _pixels)
    {
        camera = *_camera;
        yBasis = *_yBasis;
        zBasis = *_zBasis;
		screenCorner = *_screenCorner;
        params = _params;
        scene  = _scene;

		zBasis.cross(yBasis, xBasis);

//        screenCorner = new Vertex();
//        xBasis = new Vertex();

		drawn = false;

        pixels = _pixels;
    }

    ~RayTraceState()
    {
		/*delete camera;
		delete xBasis;
		delete yBasis;
		delete zBasis;
		delete screenCorner;
		delete params;*/

//		delete scene;
		aligned_free(pixels);
    }
} __attribute__ ((aligned(16)));

#endif

