#ifndef SPU_UTILS_H
#define SPU_UTILS_H

#include <stdlib.h>

#include <spu_intrinsics.h>

#define MAX_DMA_SIZE 16384
#define ALIGN_CHECKING
#define ALIGNMENT_QUADALIGN 128

typedef union
{
	unsigned long long ull;
	unsigned int ui[2];
	volatile void *p;
} Cell_addr64;

inline void CHECK_POINTER_ALIGN(void * src, void * dst, const size_t sz_arg) 
{
    #ifdef ALIGN_CHECKING
    if(  ( ((unsigned int)src) & (ALIGNMENT_QUADALIGN-1) ) || 
         ( ((unsigned int)dst) & (ALIGNMENT_QUADALIGN-1) ) ||
		 ( (sz_arg & (ALIGNMENT_QUADALIGN-1))) )
    { 
		int i = 5;
		i++;
        printf("unaligned: CHECK_POINTER_ALIGN(src=%p dst=%p, sz: %ld)\n", src, dst, sz_arg); 
    }
    #endif
}

/*-----------------------------------------------------------------------------
 / spu_dma_nowait() : read or write data using DMA, splits it into 16kB chunks,
 /                    start addresses must have proper 128-byte alignment
 / [function copied from Cell FFTW 3.2alpha
 / http://www-128.ibm.com/developerworks/forums/thread.jspa?threadID=166079
 ----------------------------------------------------------------------------*/
inline void spu_dma_nowait(void *spu_addr_arg,
                           volatile Cell_addr64 *ppu_addr_arg,
                           const size_t sz_arg, int tag, unsigned int cmd)
{
    char *spu_addr              = (char *)spu_addr_arg;
    #if XPU_64BIT
    unsigned long long ppu_addr = ppu_addr_arg->ull;
    #else
    unsigned long long ppu_addr = ppu_addr_arg->ui[0];
    #endif
    size_t sz                   = sz_arg;
    size_t chunk                = MAX_DMA_SIZE;
    CHECK_POINTER_ALIGN(spu_addr_arg, (void*)ppu_addr, sz_arg);
    
    while (sz > 0) 
    {
        if (sz < MAX_DMA_SIZE) 
        {
            chunk = sz;
        }
        
        spu_mfcdma64(spu_addr, mfc_ea2h(ppu_addr), mfc_ea2l(ppu_addr),
                  chunk, tag, cmd);
        sz -= chunk; ppu_addr += chunk; spu_addr += chunk;
    }
}

#endif

