#ifndef VECTOR_UINT_H
#define VECTOR_UINT_H

#include <altivec.h>
#include <stdlib.h>

#include "AlignedStruct.h"

struct VectorUInt : public AlignedStruct
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
} __attribute__ ((aligned (16)));

#endif

