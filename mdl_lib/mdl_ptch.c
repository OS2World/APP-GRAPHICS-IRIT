/******************************************************************************
* Mdl_ptch.c - Back patch models that have ref. indices instead of pointers.  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber	       		      		 Nov 1996     *
******************************************************************************/
/*#include "string.h"*/

#include "irit_sm.h"
#include "mdl_loc.h"

static MdlTrimSrfStruct *MdlPatchGetSrfIndex(MdlTrimSrfStruct *TrimSrfList,
					     IritIntPtrSizeType Index);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Patches a model to have proper (back) pointers between the different     M
* data structures.  The assumtions are that all reference and back pointers  M
* are set as indices into the proper lists:                                  M
* 1. Every reference to a surface, is given as its index in the Model's      M
*    TrimSrfList trimmed surfaces' list.				     M
* 2. Every reference to a trimming segment, is given as its index in the     M
*    Model's TrimSegList or trimming segments' list.			     M
* All lists are indexed starting from 1, 0 denotes an error.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:      To back patch its pointer.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlModelNew, MdlModelNew2, MdlReadModelFromFile                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlPatchTrimmingSegPointers                                              M
*****************************************************************************/
void MdlPatchTrimmingSegPointers(MdlModelStruct *Model)
{
    MdlTrimSrfStruct *TrimSrf,
	*TrimSrfList = Model -> TrimSrfList;
    MdlTrimSegStruct *TrimSeg,
	*TrimSegList = Model -> TrimSegList;

    for (TrimSeg = TrimSegList; TrimSeg != NULL; TrimSeg = TrimSeg -> Pnext) {
	TrimSeg -> SrfFirst = MdlPatchGetSrfIndex(TrimSrfList,
				    (IritIntPtrSizeType) TrimSeg -> SrfFirst);
	TrimSeg -> SrfSecond = MdlPatchGetSrfIndex(TrimSrfList,
				   (IritIntPtrSizeType) TrimSeg -> SrfSecond);

	if (TrimSeg -> SrfFirst == NULL && TrimSeg -> SrfSecond == NULL)
	    MDL_FATAL_ERROR(MDL_ERR_TSEG_NO_SRF);
    }

    for (TrimSrf = TrimSrfList; TrimSrf != NULL; TrimSrf = TrimSrf -> Pnext) {
	MdlLoopStruct
	    *Loops = TrimSrf -> LoopList;

	for ( ; Loops != NULL; Loops = Loops -> Pnext) {
	     MdlTrimSegRefStruct
		*SegRef = Loops -> SegRefList;

	     for ( ; SegRef != NULL; SegRef = SegRef -> Pnext) {
		 IritIntPtrSizeType
		     Index = IRIT_ABS((IritIntPtrSizeType) SegRef -> TrimSeg);

		 for (TrimSeg = TrimSegList;
		      TrimSeg != NULL && Index != 1;
		      TrimSeg = TrimSeg -> Pnext, Index--);

		 if (Index != 1 || TrimSeg == NULL) {
		     MDL_FATAL_ERROR(MDL_ERR_PTR_REF);
		     return;
		 }

		 SegRef -> TrimSeg = TrimSeg;
	     }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns a pointer to surface number Index in surface list TrimSrfList.   *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrfList:   List of surfaces to count.                                *
*   Index:         Of the surface we need.  Zero means no reference.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   MdlTrimSrfStruct *:  Reference to Index surface.                         *
*****************************************************************************/
static MdlTrimSrfStruct *MdlPatchGetSrfIndex(MdlTrimSrfStruct *TrimSrfList,
					     IritIntPtrSizeType Index)
{
    MdlTrimSrfStruct *TrimSrf;

    if (Index == 0)
	return NULL;

    for (TrimSrf = TrimSrfList;
	 TrimSrf != NULL && Index != 1;
	 TrimSrf = TrimSrf -> Pnext, Index--);

    if (Index != 1 || TrimSrf == NULL) {
	MDL_FATAL_ERROR(MDL_ERR_PTR_REF);
	return NULL;
    }

    return TrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the index of TrimSeg in TrimSegList, first index is 1.  Index is M
*  going to be negative if the Reversed flag is on.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSeg:      To search in TrimSegList for its index.                    M
*   TrimSegList:  List of trimming curve segments.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritIntPtrSizeType:     Index in list, or zero if not found.   This is a M
*			    special integer of a size of a pointer.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlGetSrfIndex                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlGetLoopSegIndex                                                       M
*****************************************************************************/
IritIntPtrSizeType MdlGetLoopSegIndex(const MdlTrimSegRefStruct *TrimSeg,
				      const MdlTrimSegStruct *TrimSegList)
{
    IritIntPtrSizeType
	Index = 1;

    for ( ; TrimSegList != NULL; TrimSegList = TrimSegList -> Pnext, Index++) {
	if (TrimSegList == TrimSeg -> TrimSeg)
	    return TrimSeg -> Reversed ? -Index : Index;
    }

    MDL_FATAL_ERROR(MDL_ERR_TSEG_NOT_FOUND);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the index of an Srf in TrimSrfList, first index is 1.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To search in TrimSrfList for its index.                    M
*   TrimSrfList:  List of surfaces.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritIntPtrSizeType:     Index in list, or zero if not found.   This is a M
*			    special integer of a size of a pointer.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlGetLoopSegIndex                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlGetSrfIndex                                                           M
*****************************************************************************/
IritIntPtrSizeType MdlGetSrfIndex(const MdlTrimSrfStruct *Srf,
				  const MdlTrimSrfStruct *TrimSrfList)
{
    IritIntPtrSizeType
	Index = 1;

    for ( ; TrimSrfList != NULL; TrimSrfList = TrimSrfList -> Pnext, Index++) {
	if (Srf == TrimSrfList)
	    return Index;
    }

    return 0;
}
