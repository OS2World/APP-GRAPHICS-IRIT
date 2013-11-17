/******************************************************************************
* FstAlloc.h - header file for fast allocation functions in the RNDR library. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef IRNDR_FAST_ALLOC_H
#define IRNDR_FAST_ALLOC_H

typedef struct FastAllocStruct* FastAllocType;

FastAllocType FastAllocInit(unsigned TypeSz, unsigned BlkSz,
                            unsigned AllgnBits, unsigned Verbose);

void *FastAllocNew(FastAllocType alloc);

void FastAllocDestroy(FastAllocType alloc);

#endif /* IRNDR_FAST_ALLOC_H */
