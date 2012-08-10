#ifndef RAY_TRACE_PARAMETERS_H
#define RAY_TRACE_PARAMETERS_H

#include "AlignedStruct.h"

struct RayTraceParameters : public AlignedStruct
{
    float screenWidth;
    float screenHeight;
    float tanViewPlaneAngle;
    int   xPixels;
    int   yPixels;
    float screenXStep;
    float screenYStep;
    int   maxReflectionDepth;

    RayTraceParameters(float _screenWidth, float _screenHeight,
        float _viewPlaneDistance, float _tanViewPlaneAngle, int _xPixels,
        int _yPixels, int _maxReflectionDepth)
    {
        screenWidth = _screenWidth;
        screenHeight = _screenHeight;
        tanViewPlaneAngle = _tanViewPlaneAngle;
        xPixels = _xPixels;
        yPixels = _yPixels;

		// TODO: fake
		_viewPlaneDistance = _viewPlaneDistance;

        screenXStep = screenWidth / xPixels;
        screenYStep = screenHeight / yPixels;
		maxReflectionDepth = _maxReflectionDepth;
    }
};

#endif

