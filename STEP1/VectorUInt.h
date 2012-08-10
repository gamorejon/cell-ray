#ifndef VECTOR_UINT_H
#define VECTOR_UINT_H

#include <altivec.h>
#include <stdlib.h>

#include "MemUtils.h"

struct VectorUInt
{
	union
	{
		unsigned int points[4];
		
		struct
		{
			  unsigned int a;
			  unsigned int b;
			  unsigned int c;
			  unsigned int d;
		};

		vector unsigned int vec;
	};

	VectorUInt()
	{
	}

	VectorUInt(vector unsigned int _vec)
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

