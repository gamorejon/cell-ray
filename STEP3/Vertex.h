#ifndef VERTEX_H
#define VERTEX_H

#include <math.h>
//#include <altivec.h>

#include "AltivecUtils.h"
#include "AlignedStruct.h"

struct Vertex : public AlignedStruct
{
    union
    {
    	float points[4];

        struct
		{
			float x;
			float y;
			float z;
			float a;
		};

		vector float vec;
    };

    Vertex()
    {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		a = 0.0f;
    }

    Vertex(float _x, float _y, float _z)
    {
		x = _x;
		y = _y;
		z = _z;
		a = 0.0f;
    }

    Vertex(float _x, float _y, float _z, bool _normalize) : x(_x), y(_y), z(_z), a(0.0f)
    {
		if (_normalize)
		{
			normalize();
		}
    }

	Vertex(vector float _vec)
	{
		vec = _vec;
	}

    Vertex(const Vertex& vertex)
    {
		copy(vertex);
    }

	Vertex(volatile Vertex& vertex)
	{
		x = vertex.x;
		y = vertex.y;
		z = vertex.z;
		a = vertex.a;
	}

	~Vertex()
	{
	}

    void normalize()
    {
        float normal = 1/sqrt(x*x + y*y + z*z);
        x *= normal;
		y *= normal;
		z *= normal;
		a = 0.0f;
    }

    float dot(Vertex* other)
    {
		return x * other->x + y * other->y + z * other->z;
    }

    float distance(Vertex* o)
    {
		float xDist = o->x - x;
		float yDist = o->y - y;
		float zDist = o->z - z;

		return sqrt(xDist*xDist + yDist*yDist + zDist*zDist);
    }

    void copy(const Vertex& toCopy)
    {
		x = toCopy.x;
		y = toCopy.y;
		z = toCopy.z;
		a = toCopy.a;
    }

	void cross(const Vertex& o, Vertex& result)
	{
        result.x = y * o.z - z * o.y;
		result.y = z * o.x - x * o.z;
		result.z = x * o.y - y * o.x;
	}

	Vertex operator +(const Vertex& o)
	{
		return Vertex(vec_add(vec, o.vec));
	}

	Vertex operator +(const Vertex& o) volatile
	{
		return Vertex(vec_add(vec, o.vec));
	}

	Vertex operator -(const Vertex& o)
	{
		return Vertex(vec_sub(vec, o.vec));
	}

	Vertex operator-(volatile Vertex& o) volatile
	{
		return Vertex(vec_sub(vec, o.vec));
	}

	Vertex operator *(const float& f)
	{
		vector float fVec = splatScalar(f);

        vector float negZero = negativeZero();

		return Vertex(vec_madd(vec, fVec, negZero));
	}

	Vertex operator *(const float& f) volatile
	{
        vector float fVec = splatScalar(f);

        vector float negZero = negativeZero();

        return Vertex(vec_madd(vec, fVec, negZero));

	}
} __attribute__ ((aligned (16)));

#endif
