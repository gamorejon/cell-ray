#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "Ray.h"
#include "VertexGroup.h"
#include "SceneObject.h"
#include "VectorSInt.h"

struct Intersection
{
    VertexGroup normal;
	Ray reflection;
    vec_bint4 hit;
    VectorSInt minIndex;

    Intersection()
    {
//		normal = new VertexGroup(Vertex());
//		reflection = new Ray();
    }

	~Intersection()
	{
		// delete normal;
	}
};

#endif

