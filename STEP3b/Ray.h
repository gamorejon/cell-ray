#ifndef RAY_H
#define RAY_H

#include "VertexGroup.h"
#include "AlignedStruct.h"

struct Ray : public AlignedStruct
{
    VertexGroup start;
    VertexGroup direction;

    Ray()
    {
    }

    Ray(const Ray& ray)
    {
		start = VertexGroup(ray.start);
		direction = VertexGroup(ray.direction);
    }

    ~Ray()
    {
    }

/*    void copy(const Ray& ray)
    {
        start->copy(*ray.start);
		direction->copy(*ray.direction);
    } */
};

#endif

