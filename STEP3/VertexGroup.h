#ifndef VERTEX_GROUP_H
#define VERTEX_GROUP_H

//#include <altivec.h>

#include "AltivecUtils.h"

#include "Vertex.h"

struct VertexGroup
{
	vector float x;
	vector float y;
	vector float z;
	// vector float a;

	VertexGroup()
	{
		Vertex zero;

		splat(zero);
	}

	VertexGroup(vector float _x, vector float _y, vector float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	VertexGroup(const Vertex& toSplat)
	{
		splat(toSplat);
	}

	VertexGroup(volatile Vertex& toSplat)
	{
		splat(toSplat);
	}

	VertexGroup(const VertexGroup& o)
	{
		x = o.x;
		y = o.y;
		z = o.z;
	}

    void splat(const volatile Vertex& toSplat)
    {
        x = vec_splat(toSplat.vec, 0);
        y = vec_splat(toSplat.vec, 1);
        z = vec_splat(toSplat.vec, 2);
    }

	VertexGroup madd(vector float mult, VertexGroup add)
	{
		vector float _x = vec_madd(mult, x, add.x);
		vector float _y = vec_madd(mult, y, add.y);
		vector float _z = vec_madd(mult, z, add.z);
		
		return VertexGroup(_x, _y, _z);
	}

	void normalize()
	{
        vector float negZero = negativeZero();

    	vector float norm = rsqrt(dot(*this));

		x = vec_madd(norm, x, negZero);
		y = vec_madd(norm, y, negZero);
		z = vec_madd(norm, z, negZero);
	}

	vector float dot(const VertexGroup& o)
	{
		vector float negZero = negativeZero();

		vector float dot = negZero;

		dot = vec_madd(x, o.x, dot);
		dot = vec_madd(y, o.y, dot);
		dot = vec_madd(z, o.z, dot);

		return dot;
	}

	vector float distance2(const VertexGroup& o)
	{
        vector float negZero = negativeZero();

        vector float xDist = vec_sub(o.x, x);
        vector float yDist = vec_sub(o.y, y);
        vector float zDist = vec_sub(o.z, z);

        vector float dist = negZero;

        dist = vec_madd(xDist, xDist, dist);
        dist = vec_madd(yDist, yDist, dist);
        dist = vec_madd(zDist, zDist, dist);

		return dist;
	}

	vector float distance(const VertexGroup& o)
	{
		vector float negZero = negativeZero();

		vector float dist = distance2(o);

		return vec_madd(dist, sqrt(dist), negZero);
	}

	VertexGroup& operator=(const VertexGroup& o)
	{
		x = o.x;
		y = o.y;
		z = o.z;

		return *this;
	}

	VertexGroup operator-(const VertexGroup& o)
	{
		return VertexGroup(vec_sub(x, o.x), 
						   vec_sub(y, o.y), 
						   vec_sub(z, o.z));
	}

	VertexGroup operator+(const VertexGroup& o)
	{
		return VertexGroup(vec_add(x, o.x),
						   vec_add(y, o.y),
						   vec_add(z, o.z));
	}

	void operator +=(const VertexGroup& o)
	{
		x = vec_add(x, o.x);
		y = vec_add(y, o.y);
		z = vec_add(z, o.z);
	}

	void operator -=(const VertexGroup& o)
	{
		x = vec_sub(x, o.x);
		y = vec_sub(y, o.y);
		z = vec_sub(z, o.z);
	}

	void copy(const VertexGroup& o)
	{
		x = o.x;
		y = o.y;
		z = o.z;
	}

} __attribute__ ((aligned (16)));

#endif

