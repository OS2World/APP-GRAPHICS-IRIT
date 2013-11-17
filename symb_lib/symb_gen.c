/******************************************************************************
* Symb_gen.c - General routines used by all modules of SYMB_lib.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 2003.					      *
******************************************************************************/

#include <stdio.h>
#include "symb_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an array of Arc structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:      Size of Arc array to allocate.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbArcStruct *:  An array of Arc structures of size Size.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcArrayNew, allocation                                              M
*****************************************************************************/
SymbArcStruct *SymbArcArrayNew(int Size)
{
    int i;
    SymbArcStruct
	*NewArc = (SymbArcStruct *) IritMalloc(Size * sizeof(SymbArcStruct));

    for (i = 0; i < Size; i++) {
	NewArc[i].Pnext = NULL;
	NewArc[i].Attr = NULL;
	NewArc[i].Arc = TRUE;
    }

    return NewArc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of an Arc structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Arc:  TRUE for an arc, FALSE for degenerated-arc (a line...).            M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbArcStruct *:  A Arc structure.				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcNew, allocation                                                   M
*****************************************************************************/
SymbArcStruct *SymbArcNew(int Arc)
{
    SymbArcStruct
	*NewArc = (SymbArcStruct *) IritMalloc(sizeof(SymbArcStruct));

    NewArc -> Pnext = NULL;
    NewArc -> Attr = NULL;
    NewArc -> Arc = Arc;

    return NewArc;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a Arc structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Arc:       To be copied.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbArcStruct *:  A duplicate of Arc.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcCopy, copy                                                        M
*****************************************************************************/
SymbArcStruct *SymbArcCopy(SymbArcStruct *Arc)
{
    SymbArcStruct
	*NewArc = (SymbArcStruct *) IritMalloc(sizeof(SymbArcStruct));

    CAGD_GEN_COPY(NewArc, Arc, sizeof(SymbArcStruct));
    NewArc -> Pnext = NULL;
    NewArc -> Attr = IP_ATTR_COPY_ATTRS(Arc -> Attr);

    return NewArc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of arc structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   ArcList:       To be copied.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbArcStruct *:  A duplicated list of arcs.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcCopyList, copy                                                    M
*****************************************************************************/
SymbArcStruct *SymbArcCopyList(SymbArcStruct *ArcList)
{
    SymbArcStruct *ArcTemp, *NewArcList;

    if (ArcList == NULL)
	return NULL;
    ArcTemp = NewArcList = SymbArcCopy(ArcList);
    ArcList = ArcList -> Pnext;
    while (ArcList) {
	ArcTemp -> Pnext = SymbArcCopy(ArcList);
	ArcTemp = ArcTemp -> Pnext;
	ArcList = ArcList -> Pnext;
    }
    return NewArcList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of an arc structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Arc:       To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcFree, free                                                        M
*****************************************************************************/
void SymbArcFree(SymbArcStruct *Arc)
{
    if (Arc == NULL)
	return;

    IP_ATTR_FREE_ATTRS(Arc -> Attr);
    IritFree(Arc);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an arc structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ArcList:  To be deallocated.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcFreeList, free                                                    M
*****************************************************************************/
void SymbArcFreeList(SymbArcStruct *ArcList)
{
    SymbArcStruct *ArcTemp;

    while (ArcList) {
	ArcTemp = ArcList -> Pnext;
	SymbArcFree(ArcList);
	ArcList = ArcTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of Arc structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   ArcArray:     To be deallocated.                                         M
*   Size:         Of the deallocated array.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbArcArrayFree, free                                                   M
*****************************************************************************/
void SymbArcArrayFree(SymbArcStruct *ArcArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(ArcArray[i].Attr);

    IritFree(ArcArray);
}
