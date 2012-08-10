#ifndef ALIGNED_STRUCT_H
#define ALIGNED_STRUCT_H

#include <libmisc.h>

typedef struct
{
    void* operator new(size_t size)
    {
        return malloc_align(size, 4);
    }

    void operator delete(void* ptr)
    {
        free_align(ptr);
    }
} __attribute__ ((aligned (16))) AlignedStruct;

#endif
