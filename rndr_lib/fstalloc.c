/*****************************************************************************
* A smart bulk memory manager.                                               *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "fstalloc.h"
#include "misc_lib.h"

typedef char *BytePtr;

/* Field order in structs is deliberate - targets cache performance. */

typedef struct BlkStruct {
    BytePtr Bytes;                       /* Pointer to allocated raw memory. */
    struct BlkStruct *Next;             /* Pointer to next block descriptor. */
} BlkStruct;

typedef struct FastAllocStruct {
    BytePtr NextAlloc;                        /* Pointer to next allocation. */
    BytePtr End;     /* Pointer beyond the last allocation in current block. */
    unsigned Offset;                 /* Allocation size (aligned type size). */
    unsigned Count;        /* Number of allocations made so far (statistic). */
    BlkStruct *CurrBlk;              /* Pointer to current block descriptor. */
    BlkStruct *FirstBlk;               /* Pointer to first block descriptor. */
    unsigned TypeSize;                /* Copies of initialization variables. */
    unsigned BlkSize;
    unsigned AllgnBits;
    unsigned Verbose;
} FastAllocStruct;

#define FLOOR2(n, p) ((n) & ~((1 << (p)) - 1))
#define CEIL2(n, p)  (FLOOR2(n - 1, p) + (1 << (p)))

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initializes FastAllocType internal data.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Alloc:    IN, the FastAlloc instance.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InitPointers(FastAllocType Alloc)
{
    IritIntPtrSizeType
	Allgn = Alloc -> AllgnBits,
	Start = (IritIntPtrSizeType) Alloc -> CurrBlk -> Bytes;

    Alloc -> NextAlloc = (BytePtr) CEIL2(Start, Allgn);
    Alloc -> End = (BytePtr) Start + Alloc -> BlkSize;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes an instance of an FastAllocType.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TypeSz:    IN, size of allocated type (in bytes).                        M
*   BlkSz:     IN, size of blocks allocated (in bytes, >= TypeSz).           M
*   AllgnBits: IN, alignment of each allocation is 2^AllgnBits.              M
*   Verbose:   IN, iff TRUE, FastAllocDestroy() prints statistics.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   FastAllocType: The FastAlloc instance.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   FastAllocInit                                                            M
*****************************************************************************/
FastAllocType FastAllocInit(unsigned TypeSz,
                            unsigned BlkSz,
                            unsigned AllgnBits,
                            unsigned Verbose)
{
    FastAllocType Alloc;

    if (TypeSz > BlkSz) {
        return NULL;
    }

    Alloc = IritMalloc(sizeof(FastAllocStruct));
    Alloc -> TypeSize = TypeSz;
    Alloc -> BlkSize = BlkSz;
    Alloc -> AllgnBits = AllgnBits;
    Alloc -> Verbose = Verbose;
    Alloc -> Offset = CEIL2(TypeSz, AllgnBits);

    Alloc -> FirstBlk = IritMalloc(sizeof(BlkStruct));
    Alloc -> FirstBlk -> Next = NULL;
    Alloc -> FirstBlk -> Bytes = IritMalloc(Alloc -> BlkSize);
    Alloc -> CurrBlk = Alloc -> FirstBlk;
    Alloc -> Count = 0;
    InitPointers(Alloc);

    return Alloc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates new area of memory of size TypeSz (see FastAllocInit()).       M
*                                                                            *
* PARAMETERS:                                                                M
*   Alloc:     IN, FastAllocType instance.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Void pointer to the new memory area.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   FastAllocNew                                                             M
*****************************************************************************/
VoidPtr FastAllocNew(FastAllocType Alloc)
{
    BytePtr
        NextAlloc = Alloc -> NextAlloc + Alloc -> Offset,
        NewPtr = Alloc -> NextAlloc;

    if (NextAlloc >= Alloc -> End) {
        BlkStruct
	    *newBlk = IritMalloc(sizeof(BlkStruct));

        newBlk -> Next = NULL;
        newBlk -> Bytes = IritMalloc(Alloc -> BlkSize);
        Alloc -> CurrBlk -> Next = newBlk;
        Alloc -> CurrBlk = newBlk;
        InitPointers(Alloc);
    }
    else {
        Alloc -> NextAlloc = NextAlloc;
    }

    Alloc -> Count++;

    return NewPtr;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Frees all memory allocated via the FastAlloc.                            M
*   Prints statistics iff Verbose == TRUE (see FastAllocInit()).             M
*                                                                            *
* PARAMETERS:                                                                M
*   Alloc:     IN, FastAllocType instance.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*****************************************************************************/
void FastAllocDestroy(FastAllocType Alloc)
{
    BlkStruct *blk, *next;
    unsigned
        blkCount = 0;

    for (blk = Alloc -> FirstBlk; blk; blk = next) {
        IritFree(blk -> Bytes);
        next = blk -> Next;
        IritFree(blk);
        blkCount++;
    }
    if (Alloc -> Verbose) {
        IRIT_INFO_MSG("\nFastAlloc:\n\t");
        IRIT_INFO_MSG_PRINTF("type size = %d, block size = %d, alignment = %d\n\t",
		Alloc -> TypeSize, Alloc -> BlkSize, 1 << Alloc -> AllgnBits);
        IRIT_INFO_MSG_PRINTF("aligned size = %d, allocations = %d, ",
		Alloc -> Offset, Alloc -> Count);
        IRIT_INFO_MSG_PRINTF("%d blocks allocated = %0.1f KB",
		blkCount,
		(float)(blkCount * Alloc -> BlkSize) / 1024);
    }
    IritFree(Alloc);
}
