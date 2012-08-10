#ifndef STATE_H
#define STATE_H

#include "Vertex.h"
#include "RayTraceParameters.h"
#include "Scene.h"

struct RayTraceState
{
public:
    Vertex* camera;
    Vertex* xBasis;
    Vertex* yBasis;
    Vertex* zBasis;
    Vertex* screenCorner;
    RayTraceParameters* params;
    Scene* scene;
    bool drawn;

    RayTraceState(Vertex* _camera, Vertex* _yBasis, Vertex* _zBasis,
        RayTraceParameters* _params, Scene* _scene)
    {
        camera = _camera;
        yBasis = _yBasis;
        zBasis = _zBasis;
        params = _params;
        scene  = _scene;

        screenCorner = new Vertex();
        xBasis = new Vertex();

		drawn = false;

        updateState();
    }

    ~RayTraceState()
    {
		delete camera;
		delete xBasis;
		delete yBasis;
		delete zBasis;
		delete screenCorner;
		delete params;
		// delete scene;
    }

private:
    void updateState()
    {
		zBasis->cross(*yBasis, xBasis);

        cout << "xBasis: " << xBasis->x << "," << xBasis->y << "," << xBasis->z << endl;
        cout << "yBasis: " << yBasis->x << "," << yBasis->y << "," << yBasis->z << endl;
        cout << "zBasis: " << zBasis->x << "," << zBasis->y << "," << zBasis->z << endl;

        Vertex cornerV;

        cornerV.x = 0.5*(params->screenHeight*yBasis->x -
        	params->screenWidth*xBasis->x);
        cornerV.y = 0.5*(params->screenHeight*yBasis->y -
        	params->screenWidth*xBasis->y);
        cornerV.z = 0.5*(params->screenHeight*yBasis->z -
        	params->screenWidth*xBasis->z);

        float dist = sqrt(cornerV.x * cornerV.x + cornerV.y * cornerV.y +
        	cornerV.z * cornerV.z) / params->tanViewPlaneAngle;

        screenCorner->x = camera->x + dist * zBasis->x + cornerV.x;
        screenCorner->y = camera->y + dist * zBasis->y + cornerV.y;
        screenCorner->z = camera->z + dist * zBasis->z + cornerV.z;

        cout << "Screen corner (" << screenCorner->x << "," << screenCorner->y
        	 << "," << screenCorner->z << ")" << endl;
    }
};

#endif

