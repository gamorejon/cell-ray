#ifndef VECTOR_SINT_H
#define VECTOR_SINT_H

#include <altivec.h>
#include <stdlib.h>

#include "MemUtils.h"

struct VectorSInt
{
	union
	{
		int points[4];
		
		struct
		{
			  int a;
			  int b;
			  int c;
			  int d;
		};

		vector int vec;
	};

	VectorSInt()
	{
	}

	VectorSInt(vector int _vec)
	{
		vec = _vec;
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

