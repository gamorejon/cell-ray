#ifndef COLOR_GROUP_H
#define COLOR_GROUP_H

#include "AlignedStruct.h"

struct ColorGroup : public AlignedStruct
{
	vector float r;
	vector float g;
	vector float b;

	ColorGroup() 
	{
		Color zero = Color();

		splat(zero);
	}

	ColorGroup(vector float _r, vector float _g, vector float _b) 
	{
		r = _r;
		g = _g;
		b = _b;
	}

	ColorGroup(volatile const Color& toSplat) 
	{
		splat(toSplat);
	}

	void splat(volatile const Color& toSplat) volatile
	{
        r = vec_splat(toSplat.vec, 0);
        g = vec_splat(toSplat.vec, 1);
        b = vec_splat(toSplat.vec, 2);
	}

	void zero() volatile
	{
		vector float negZero = negativeZero();

		r = negZero;
		g = negZero;
		b = negZero;
	}

	void addColor(volatile const ColorGroup& o) volatile
	{
		r = vec_add(r, o.r);
		g = vec_add(g, o.g);
		b = vec_add(b, o.b);
	}

	void addColor(volatile const ColorGroup& o, vector float f) volatile
	{
		r = vec_madd(o.r, f, r);
		g = vec_madd(o.g, f, g);
		b = vec_madd(o.b, f, b);
	}
} __attribute__ ((aligned (32)));

#endif

