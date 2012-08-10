#ifndef COLOR_H
#define COLOR_H

//#include <altivec.h>

#include "AlignedStruct.h"

struct Color : public AlignedStruct
{
    union
    {
		float rgba[4];

		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		vector float vec;
    };

    Color() : r(0.0f), g(0.0f), b(0.0f), a(0.0f)
    {
    }

    Color(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(0.0f)
    {
    }

	Color(vector float _vec)
	{
		vec = _vec;
	}

	~Color()
	{
	}

    void addColor(const Color& color)
    {
		vec = vec_add(vec, color.vec);
    }

    void addColor(const Color& color, const float& scale)
    {
		vector float scaleVec = splatScalar(scale);

		vec = vec_madd(color.vec, scaleVec, vec);
	}

} __attribute__ ((aligned (16)));

#endif

