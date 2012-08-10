#ifndef RAY_H
#define RAY_H

#include "VertexGroup.h"

struct Ray
{
    VertexGroup* start;
    VertexGroup* direction;
	bool deleteMe;

    Ray()
    {
		start = new VertexGroup(Vertex());
		direction = new VertexGroup(Vertex());
		deleteMe = true;
    }

    Ray(const Ray& ray)
    {
		start = new VertexGroup(*ray.start);
		direction = new VertexGroup(*ray.direction);
		deleteMe = true;
    }

    ~Ray()
    {
		if (deleteMe)
		{
			//delete start;
			//delete direction;
		}
    }

/*    void copy(const Ray& ray)
    {
        start->copy(*ray.start);
		direction->copy(*ray.direction);
    } */
};

#endif

