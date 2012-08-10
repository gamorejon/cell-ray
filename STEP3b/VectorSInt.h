#ifndef VECTOR_SINT_H
#define VECTOR_SINT_H

//#include <altivec.h>
#include <stdlib.h>

#include "AlignedStruct.h"

struct VectorSInt : public AlignedStruct
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

		vector signed int vec;
	};

	VectorSInt()
	{
	}

	VectorSInt(vector signed int _vec)
	{
		vec = _vec;
	}
} __attribute__ ((aligned (16)));

#endif

