#ifndef MEM_UTILS_H
#define MEM_UTILS_H

// #include <stdlib.h>
#include <libmisc.h>

volatile void* aligned_alloc(size_t size, int _align)
{
	return (volatile void*)malloc_align(size, _align);
	
/*	int align = round(pow((float)2, _align));

	printf("Align: %d, %d\n", _align, align);

    unsigned char* ptr1 = new unsigned char[size+align];
    unsigned char* ptr2 = (unsigned char*)((unsigned)ptr1 + align & ~(align - 1));

    unsigned int offset = ptr2 - ptr1;

	unsigned short* ptrint = (unsigned short*)ptr2;

    ptrint[-1] = (unsigned short)offset;
    return ptr2;*/
}

void aligned_free(volatile void* ptr)
{
	free_align((void*)ptr);

/*    unsigned short* _ptrs = (unsigned short*)ptr;
	unsigned char* _ptr = (unsigned char*)ptr;

    _ptr -= _ptrs[-1];

    delete[] _ptr;*/
}

#endif

