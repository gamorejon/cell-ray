#ifndef COLOR_GROUP_H
#define COLOR_GROUP_H

#include <altivec.h>

struct ColorGroup
{
	vector float r;
	vector float g;
	vector float b;

	ColorGroup(vector float _r, vector float _g, vector float _b)
	{
		r = _r;
		g = _g;
		b = _b;
	}

	ColorGroup(const Color& toSplat)
	{
		splat(toSplat);
	}

	void splat(const Color& toSplat)
	{
        r = vec_splat(toSplat.vec, 0);
        g = vec_splat(toSplat.vec, 1);
        b = vec_splat(toSplat.vec, 2);
	}

	void zero()
	{
		vector float negZero = negativeZero();

		r = negZero;
		g = negZero;
		b = negZero;
	}

	void addColor(const ColorGroup& o)
	{
		r = vec_add(r, o.r);
		g = vec_add(g, o.g);
		b = vec_add(b, o.b);
	}

	void addColor(const ColorGroup& o, vector float f)
	{
		r = vec_madd(o.r, f, r);
		g = vec_madd(o.g, f, g);
		b = vec_madd(o.b, f, b);
	}

	void* operator new(size_t size)
	{
		return aligned_alloc(size, 16);
	}

	void operator delete(void* p)
	{
		aligned_free(p);
	}
} __attribute__ ((aligned (16)));

#endif

