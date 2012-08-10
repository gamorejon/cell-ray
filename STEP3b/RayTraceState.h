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

    RayTraceState(Vertex* _camera, Vertex* _xBasis, Vertex* _yBasis, 
		Vertex* _zBasis, Vertex* _screenCorner, RayTraceParameters* _params, 
		Scene* _scene, volatile ColorGroup* _pixels)
    {
        camera = *_camera;
		xBasis = *_xBasis;
        yBasis = *_yBasis;
        zBasis = *_zBasis;
		screenCorner = *_screenCorner;
        params = _params;
        scene  = _scene;

//        screenCorner = new Vertex();
//        xBasis = new Vertex();

		drawn = false;

        pixels = _pixels;

		delete _camera;
		delete _xBasis;
		delete _yBasis;
		delete _zBasis;
		delete _screenCorner;
    }

    ~RayTraceState()
    {
		delete params;
    }
} __attribute__ ((aligned(16)));

#endif

