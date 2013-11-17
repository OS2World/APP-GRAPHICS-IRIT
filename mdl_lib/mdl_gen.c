/******************************************************************************
* Mdl_gen.c - generic routine to interface to different free from types.      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Alexander Bogdanov and Gershon Elber		June 1997     *
******************************************************************************/

#include "irit_sm.h"
#include "mdl_loc.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"

#ifdef DEBUG
#undef MdlTrimSegFree
#undef MdlTrimSegFreeList
#undef MdlTrimSegRefFree
#undef MdlTrimSegRefFreeList
#undef MdlLoopFree
#undef MdlLoopFreeList
#undef MdlTrimSrfFree
#undef MdlTrimSrfFreeList
#undef MdlModelFree
#undef MdlModelFreeList
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a Model Trimming Segments structure.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   MTSeg:   A Trimming Segment to free.	        		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegFree, allocation                      	                     M
*****************************************************************************/
void MdlTrimSegFree(MdlTrimSegStruct *MTSeg)
{
    CagdCrvFree(MTSeg -> UVCrvFirst);
    CagdCrvFree(MTSeg -> UVCrvSecond);
    CagdCrvFree(MTSeg -> EucCrv);
    IP_ATTR_FREE_ATTRS(MTSeg -> Attr);
    IritFree(MTSeg); 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a Model Trimming Segments List  structure.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MTSegList:   A Trimming Segment List to free.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegFreeList, allocation                    	                     M
*****************************************************************************/
void MdlTrimSegFreeList(MdlTrimSegStruct *MTSegList)
{
    MdlTrimSegStruct *SegTemp;

    while (MTSegList) {
	SegTemp = MTSegList -> Pnext;
	MdlTrimSegFree(MTSegList);
	MTSegList = SegTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a Model Trimming Segment Reference structure.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   MTSegRef: A Segments Reference to free.	 		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegRefFree, allocation                  	                     M
*****************************************************************************/
void MdlTrimSegRefFree(MdlTrimSegRefStruct *MTSegRef)
{
    IP_ATTR_FREE_ATTRS(MTSegRef -> Attr);
    IritFree(MTSegRef);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates a Model Trimming Segment Reference List  structure.          M
*                                                                            *
* PARAMETERS:                                                                M
*   MTSegRefList: A list of loops to free.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegRefFreeList, allocation                  	                     M
*****************************************************************************/
void MdlTrimSegRefFreeList(MdlTrimSegRefStruct *MTSegRefList)
{
    MdlTrimSegRefStruct *SegRefTemp;

    while (MTSegRefList) {
	SegRefTemp = MTSegRefList -> Pnext;
	MdlTrimSegRefFree(MTSegRefList);
	MTSegRefList = SegRefTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates a Model Loop  structure.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlLoop:   A loop to free.	 		       		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlLoopFree, allocation                     	                     M
*****************************************************************************/
void MdlLoopFree(MdlLoopStruct *MdlLoop)
{
    MdlTrimSegRefFreeList(MdlLoop -> SegRefList);
    IP_ATTR_FREE_ATTRS(MdlLoop -> Attr);
    IritFree(MdlLoop);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates a Model Loop List structure.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlLoopList: A list of loops to free.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlLoopFreeList, allocation                     	                     M
*****************************************************************************/
void MdlLoopFreeList(MdlLoopStruct *MdlLoopList)
{
    MdlLoopStruct *LoopTemp;

    while (MdlLoopList) {
	LoopTemp = MdlLoopList -> Pnext;
	MdlLoopFree(MdlLoopList);
	MdlLoopList = LoopTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates a Model Trimming Surface structure.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:   A surface to free.                       	      	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSrfFree, allocation                                               M
*****************************************************************************/
void MdlTrimSrfFree(MdlTrimSrfStruct *TrimSrf)
{
    CagdSrfFree(TrimSrf -> Srf);
    MdlLoopFreeList(TrimSrf -> LoopList);
    IP_ATTR_FREE_ATTRS(TrimSrf -> Attr);
    IritFree(TrimSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates a Model Trimming Surface List structure.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSrfList: A list of trimming curve to free.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSrfFreeList, allocation                     	                     M
*****************************************************************************/
void MdlTrimSrfFreeList(MdlTrimSrfStruct *MdlTrimSrfList)
{
    MdlTrimSrfStruct *TrimSrfTemp;

    while (MdlTrimSrfList) {
	TrimSrfTemp = MdlTrimSrfList -> Pnext;
	MdlTrimSrfFree(MdlTrimSrfList);
	MdlTrimSrfList = TrimSrfTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a Model structure.      	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model: A Model to free.      		              	      	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelFree, allocation  	                                             M
*****************************************************************************/
void MdlModelFree(MdlModelStruct *Model)
{
    MdlTrimSrfFreeList(Model -> TrimSrfList);
    MdlTrimSegFreeList(Model -> TrimSegList);
    IP_ATTR_FREE_ATTRS(Model -> Attr);
    IritFree(Model);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates a list of Model structures.	               	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Model: A list of trimmed surface to free.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelFreeList, allocation               	                             M
*****************************************************************************/
void MdlModelFreeList(MdlModelStruct *Model)
{
    MdlModelStruct *ModelTemp;
    
    while (Model) {
	ModelTemp = Model -> Pnext;
	MdlModelFree(Model);
	Model = ModelTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Duplicates a trimming segments structure. 				     M
*   The reference pointers to the (upto) two surfaces are replaced with the  M
* indices of the surfaces in TrimSrfList.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSeg:   A trimming segment to duplicate.			     M
*   TrimSrfList:  The original trimmed surfaces.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegStruct *:   A trimming segment structure.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegCopy, allocation                                               M
*****************************************************************************/
MdlTrimSegStruct *MdlTrimSegCopy(const MdlTrimSegStruct *MdlTrimSeg,
				 const MdlTrimSrfStruct *TrimSrfList)
{
    MdlTrimSegStruct
	*NewMdlTrimSeg = (MdlTrimSegStruct *)
	    IritMalloc(sizeof(MdlTrimSegStruct));

    if (MdlTrimSeg -> SrfFirst)
        NewMdlTrimSeg -> SrfFirst = (MdlTrimSrfStruct *) 
	                 MdlGetSrfIndex(MdlTrimSeg -> SrfFirst, TrimSrfList);
    else
        NewMdlTrimSeg -> SrfFirst = NULL;

    if (MdlTrimSeg -> SrfSecond)
        NewMdlTrimSeg -> SrfSecond = (MdlTrimSrfStruct *)
	                 MdlGetSrfIndex(MdlTrimSeg -> SrfSecond, TrimSrfList);
    else
        NewMdlTrimSeg -> SrfSecond = NULL;

    if (MdlTrimSeg -> UVCrvFirst)
        NewMdlTrimSeg -> UVCrvFirst = CagdCrvCopy(MdlTrimSeg -> UVCrvFirst);
    else
        NewMdlTrimSeg -> UVCrvFirst = NULL;

    if (MdlTrimSeg -> UVCrvSecond)
        NewMdlTrimSeg -> UVCrvSecond = CagdCrvCopy(MdlTrimSeg -> UVCrvSecond);
    else
        NewMdlTrimSeg -> UVCrvSecond = NULL;

    if (MdlTrimSeg -> EucCrv)
	NewMdlTrimSeg -> EucCrv = CagdCrvCopy(MdlTrimSeg -> EucCrv);
    else
	NewMdlTrimSeg -> EucCrv = NULL;

    assert(NewMdlTrimSeg -> UVCrvFirst == NULL ||
	   NewMdlTrimSeg -> UVCrvFirst -> Pnext == NULL ||
	   NewMdlTrimSeg -> UVCrvSecond == NULL ||
	   NewMdlTrimSeg -> UVCrvSecond -> Pnext == NULL ||
	   NewMdlTrimSeg -> EucCrv == NULL ||
	   NewMdlTrimSeg -> EucCrv -> Pnext == NULL);

    NewMdlTrimSeg -> Pnext = NULL;
    NewMdlTrimSeg -> Attr = NULL;
    NewMdlTrimSeg -> Tags = 0;

    return NewMdlTrimSeg;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates and copies a list of trimming segment structures.		     M
*   The reference pointers to the (upto) two surfaces are replaced with the  M
* indices of the surfaces in TrimSrfList.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSegList:   To be copied.                                          M
*   TrimSrfList:      The original trimmed surfaces.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegStruct *:  A duplicated list of trimming segments.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegCopyList, copy                                                 M
*****************************************************************************/
MdlTrimSegStruct *MdlTrimSegCopyList(const MdlTrimSegStruct *MdlTrimSegList,
				     const MdlTrimSrfStruct *TrimSrfList)
{
    MdlTrimSegStruct *MdlTrimSegTemp, *NewMdlTrimSegList;

    if (MdlTrimSegList == NULL)
	return NULL;

    MdlTrimSegTemp = NewMdlTrimSegList = MdlTrimSegCopy(MdlTrimSegList,
							TrimSrfList);
    MdlTrimSegList = MdlTrimSegList -> Pnext;
    
    while (MdlTrimSegList) {
	MdlTrimSegTemp -> Pnext = MdlTrimSegCopy(MdlTrimSegList, TrimSrfList);
        MdlTrimSegTemp = MdlTrimSegTemp -> Pnext;
	MdlTrimSegList = MdlTrimSegList -> Pnext;
    }
    return NewMdlTrimSegList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Duplicates a trimming segment reference structure.                       M
*   The reference pointer to the trimming segment is replaced with the       M
* index of trimming segment in TrimSegList.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MTSegRefList:   A trimming curve segment reference to duplicate.	     M
*   TrimSegList:    The original trimmed segments.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegRefStruct *:   A trimming segment reference structure.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegRefCopy, allocation                                            M
*****************************************************************************/
MdlTrimSegRefStruct *MdlTrimSegRefCopy(const MdlTrimSegRefStruct *MTSegRefList,
				       const MdlTrimSegStruct *TrimSegList)
{
    MdlTrimSegRefStruct
	*NewMdlTrimSegRef =
            (MdlTrimSegRefStruct *) IritMalloc(sizeof(MdlTrimSegRefStruct));

    NewMdlTrimSegRef -> TrimSeg = 
        (MdlTrimSegStruct *) MdlGetLoopSegIndex(MTSegRefList, TrimSegList);

    NewMdlTrimSegRef -> Pnext = NULL;
    NewMdlTrimSegRef -> Reversed = MTSegRefList -> Reversed;
    NewMdlTrimSegRef -> Attr = NULL;
    NewMdlTrimSegRef -> Tags = 0;

    return NewMdlTrimSegRef;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates and copies a list of trimming segment reference structures.    M
*   The reference pointer to the trimming segment is replaced with the       M
* index of trimming segment in TrimSegList.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MTSegRefList:       To be copied.                                        M
*   TrimSegList:        The original trimmed segments.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegRefStruct *:  A duplicated list of trimming segments.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegRefCopyList, copy                                              M
*****************************************************************************/
MdlTrimSegRefStruct *MdlTrimSegRefCopyList(const MdlTrimSegRefStruct *MTSegRefList,
					   const MdlTrimSegStruct *TrimSegList)
{
    MdlTrimSegRefStruct *MTSegRefTemp, *NewMTSegRefList;

    if (MTSegRefList == NULL)
	return NULL;
    MTSegRefTemp = NewMTSegRefList = MdlTrimSegRefCopy(MTSegRefList,
						       TrimSegList);
    MTSegRefList = MTSegRefList -> Pnext;
    while (MTSegRefList) {
	MTSegRefTemp -> Pnext = MdlTrimSegRefCopy(MTSegRefList, TrimSegList);
	MTSegRefTemp = MTSegRefTemp -> Pnext;
	MTSegRefList = MTSegRefList -> Pnext;
    }
    return NewMTSegRefList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Duplicates a trimming surface structure.	                             M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlLoop:      A trimming surface to duplicate.			     M
*   TrimSegList:  The original trimmed segments.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlLoopStruct *:   A trimming surface structure.		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlLoopCopy, allocation                                                  M
*****************************************************************************/
MdlLoopStruct *MdlLoopCopy(const MdlLoopStruct *MdlLoop, 
			   const MdlTrimSegStruct *TrimSegList)
{
    MdlLoopStruct
	*NewMdlLoop = (MdlLoopStruct *) IritMalloc(sizeof(MdlLoopStruct));

    NewMdlLoop -> SegRefList =
	MdlTrimSegRefCopyList(MdlLoop -> SegRefList, TrimSegList);
    NewMdlLoop -> Pnext = NULL;
    NewMdlLoop -> Attr = NULL;
  
    return NewMdlLoop;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates and copies a list of Loops structures.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlLoopList: To be copied.                                 	             M
*   TrimSegList:  The original trimmed segments.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlLoopStruct *: A duplicated list of Loops.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlLoopCopyList, copy                                                    M
*****************************************************************************/
MdlLoopStruct *MdlLoopCopyList(const MdlLoopStruct *MdlLoopList, 
			       const MdlTrimSegStruct *TrimSegList)
{
    MdlLoopStruct *MdlLoopTemp, *NewMdlLoopList;

    if (MdlLoopList == NULL)
	return NULL;
    MdlLoopTemp = NewMdlLoopList = MdlLoopCopy(MdlLoopList, TrimSegList);
    MdlLoopList = MdlLoopList -> Pnext;
    while (MdlLoopList) {
	MdlLoopTemp -> Pnext = MdlLoopCopy(MdlLoopList, TrimSegList);
	MdlLoopTemp = MdlLoopTemp -> Pnext;
	MdlLoopList = MdlLoopList -> Pnext;
    }
    return NewMdlLoopList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Duplicates a trimming surface structure.	                             M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSrf:    A trimming surface to duplicate.			     M
*   TrimSegList:   The original trimmed segments.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSrfStruct *: A trimming surface structure.		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSrfCopy, allocation                                               M
*****************************************************************************/
MdlTrimSrfStruct *MdlTrimSrfCopy(const MdlTrimSrfStruct *MdlTrimSrf, 
				 const MdlTrimSegStruct *TrimSegList)
{
    MdlTrimSrfStruct
	*NewMdlTrimSrf = (MdlTrimSrfStruct *)
	    IritMalloc(sizeof(MdlTrimSrfStruct));

    NewMdlTrimSrf -> LoopList =
	MdlLoopCopyList(MdlTrimSrf -> LoopList, TrimSegList);
    NewMdlTrimSrf -> Srf = CagdSrfCopy(MdlTrimSrf -> Srf);
    NewMdlTrimSrf -> Pnext = NULL;
    NewMdlTrimSrf -> Attr = NULL; 

    return NewMdlTrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates and copies a list of trimming surface structures.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSrfList:  To be copied.                                	     M
*   TrimSegList:     The original trimmed segments.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSrfStruct *: A duplicated list of trimming surfaces.	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSrfCopyList, copy                                                 M
*****************************************************************************/
MdlTrimSrfStruct *MdlTrimSrfCopyList(const MdlTrimSrfStruct *MdlTrimSrfList, 
				     const MdlTrimSegStruct *TrimSegList)
{
    MdlTrimSrfStruct *MdlTrimSrfTemp, *NewMdlTrimSrfList;

    if (MdlTrimSrfList == NULL)
	return NULL;

    MdlTrimSrfTemp = NewMdlTrimSrfList = MdlTrimSrfCopy(MdlTrimSrfList,
							TrimSegList);
    MdlTrimSrfList = MdlTrimSrfList -> Pnext;
    while (MdlTrimSrfList) {
	MdlTrimSrfTemp -> Pnext = MdlTrimSrfCopy(MdlTrimSrfList, TrimSegList);
	MdlTrimSrfTemp = MdlTrimSrfTemp -> Pnext;
	MdlTrimSrfList = MdlTrimSrfList -> Pnext;
    }
    return NewMdlTrimSrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Duplicates a Model structure.   		                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:   A Model to duplicate.				    	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:   A Model structure.		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelCopy, allocation                                                 M
*****************************************************************************/
MdlModelStruct *MdlModelCopy(const MdlModelStruct *Model)
{
    MdlModelStruct
	*NewModel = (MdlModelStruct *) IritMalloc(sizeof(MdlModelStruct));

    NewModel -> Pnext = NULL;
    NewModel -> TrimSrfList = MdlTrimSrfCopyList(Model -> TrimSrfList,
						 Model -> TrimSegList);
    NewModel -> TrimSegList = MdlTrimSegCopyList(Model -> TrimSegList,
						 Model -> TrimSrfList);

    NewModel -> Attr = NULL;

    MdlPatchTrimmingSegPointers(NewModel);

    return NewModel;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates and copies a list of Model structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   ModelList:   To be copied.      	                          	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *:   A duplicated list of trimming surfaces.       	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelCopyList, copy                                                   M
*****************************************************************************/
MdlModelStruct *MdlModelCopyList(const MdlModelStruct *ModelList)
{
    MdlModelStruct *ModelTemp, *NewModelList;

    if (ModelList == NULL)
	return NULL;

    ModelTemp = NewModelList = MdlModelCopy(ModelList);
    ModelList = ModelList -> Pnext;
    
    while (ModelList) {
	ModelTemp -> Pnext = MdlModelCopy(ModelList);
	ModelTemp = ModelTemp -> Pnext;
	ModelList = ModelList -> Pnext;
    }

    return NewModelList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates a model trimming segment structure. Allows periodic and float  M
* end conditions - converts them to open end.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVCrv1:     A UV curve for SrfFirst. Must be an E2 curve. Used in place. M
*   UVCrv2:     A UV curve for SrfSecond. Must be an E2 curve. Used in place.M
*               Can be NULL.						     M
*   EucCrv1:    Optional Euclidean curve for SrfFirst. Must be an E3 curve.  M
*               Used in place.  Can be NULL.				     M
*   SrfFirst:   First surface of the segment.  Can be NULL.                  M
*   SrfSecond:  Second surface of the segment.  Can be NULL.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegStruct *:   A model trimming segment structure.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegNew, allocation                                                M
*****************************************************************************/
MdlTrimSegStruct *MdlTrimSegNew(CagdCrvStruct *UVCrv1,
				CagdCrvStruct *UVCrv2,
                                CagdCrvStruct *EucCrv1,
                                MdlTrimSrfStruct *SrfFirst,
                                MdlTrimSrfStruct *SrfSecond)
{
    MdlTrimSegStruct
	*MdlTrimSeg = (MdlTrimSegStruct *)
	    IritMalloc(sizeof(MdlTrimSegStruct));

    if (UVCrv1 != NULL && CAGD_IS_BSPLINE_CRV(UVCrv1)) {
	if (!BspCrvHasOpenEC(UVCrv1)) { /* Must be open end trimming curve. */
	    CagdCrvStruct *TCrv;
      
	    if (CAGD_IS_PERIODIC_CRV(UVCrv1)) {
		TCrv = CagdCnvrtPeriodic2FloatCrv(UVCrv1);
		CagdCrvFree(UVCrv1);
		UVCrv1 = TCrv;
	    }

	    TCrv = CagdCnvrtFloat2OpenCrv(UVCrv1);
	    CagdCrvFree(UVCrv1);
	    UVCrv1 = TCrv;
	}
    }

    if ((MdlTrimSeg -> UVCrvFirst = UVCrv1) != NULL)
	MdlTrimSeg -> UVCrvFirst -> Pnext = NULL;

    if (UVCrv2 != NULL && CAGD_IS_BSPLINE_CRV(UVCrv2)) {
	if (!BspCrvHasOpenEC(UVCrv2)) { /* Must be open end trimming curve. */
	    CagdCrvStruct *TCrv;

	    if (CAGD_IS_PERIODIC_CRV(UVCrv2)) {
		TCrv = CagdCnvrtPeriodic2FloatCrv(UVCrv2);
		CagdCrvFree(UVCrv2);
		UVCrv1 = TCrv;
	    }
	    TCrv = CagdCnvrtFloat2OpenCrv(UVCrv2);
	    CagdCrvFree(UVCrv2);
	    UVCrv2 = TCrv;
	}
    }
    
    if ((MdlTrimSeg -> UVCrvSecond = UVCrv2) != NULL)
	MdlTrimSeg -> UVCrvSecond -> Pnext = NULL;

    if (EucCrv1 != NULL && CAGD_IS_BSPLINE_CRV(EucCrv1)) {
	if (!BspCrvHasOpenEC(EucCrv1)) {/* Must be open end trimming curve. */
	    CagdCrvStruct *TCrv;

	    if (CAGD_IS_PERIODIC_CRV(EucCrv1)) {
		TCrv = CagdCnvrtPeriodic2FloatCrv(EucCrv1);
		CagdCrvFree(EucCrv1);
		EucCrv1 = TCrv;
	    }
	    TCrv = CagdCnvrtFloat2OpenCrv(EucCrv1);
	    CagdCrvFree(EucCrv1);
	    EucCrv1 = TCrv;
	}
    }

    if ((MdlTrimSeg -> EucCrv = EucCrv1) != NULL)
	MdlTrimSeg -> EucCrv -> Pnext = NULL;

    MdlTrimSeg -> Pnext = NULL;
    MdlTrimSeg -> SrfFirst = SrfFirst;
    MdlTrimSeg -> SrfSecond = SrfSecond;
    MdlTrimSeg -> Attr = NULL;
    MdlTrimSeg -> Tags = 0;

    return MdlTrimSeg;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates a model trimming segment reference structure.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSeg:    List of model trimming segments forming the trimming      M
*                  curve.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSegRefStruct *:   A trimmig curve.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSegRefNew, allocation                                             M
*****************************************************************************/
MdlTrimSegRefStruct *MdlTrimSegRefNew(MdlTrimSegStruct *MdlTrimSeg)
{
    MdlTrimSegRefStruct
	*SegRef = (MdlTrimSegRefStruct *)
	    IritMalloc(sizeof(MdlTrimSegRefStruct));

    SegRef -> TrimSeg = MdlTrimSeg;
    SegRef -> Pnext = NULL;
    SegRef -> Reversed = FALSE;
    SegRef -> Attr = NULL;
    SegRef -> Tags = 0;

    return SegRef;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates a model loop structure.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   MdlTrimSegRefList:    List of model loops forming the trimming loop.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlLoopStruct *:  A model loop.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlLoopNew, allocation                                                   M
*****************************************************************************/
MdlLoopStruct *MdlLoopNew(MdlTrimSegRefStruct *MdlTrimSegRefList)
{
    MdlLoopStruct
	*Loop = (MdlLoopStruct *) IritMalloc(sizeof(MdlLoopStruct));

    Loop -> SegRefList = MdlTrimSegRefList;
    Loop -> Pnext = NULL;
    Loop -> Attr = NULL;

    return Loop;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructor for a model trimmed surface.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             Surface to make into a trimmed surface. In place.       M
*   LoopList:        An optional list of loops.	Used in place.		     M
*   HasTopLvlTrim:   Do we have a top level outer most trimming curve?	     M
*   UpdateBackTSrfPtrs:  TRUE to also update back pointers from trimming     M
*                    curves to the trimmed surface.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSrfStruct *: The trimmed surface.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSrfNew, allocation,                                               M
*****************************************************************************/
MdlTrimSrfStruct *MdlTrimSrfNew(CagdSrfStruct *Srf,
				MdlLoopStruct *LoopList, 
				CagdBType HasTopLvlTrim,
				CagdBType UpdateBackTSrfPtrs)
{
    CagdRType UMin, UMax, VMin, VMax;
    MdlTrimSegStruct *MdlTrimSeg;
    MdlTrimSegRefStruct  *MdlTrimSegRef;
    MdlLoopStruct *Loop;
    MdlTrimSrfStruct
	*MdlTrimSrf = 
            (MdlTrimSrfStruct *) IritMalloc(sizeof(MdlTrimSrfStruct));

    if (CAGD_IS_BSPLINE_SRF(Srf)) {
        CagdSrfStruct *TSrf;

        if (CAGD_IS_PERIODIC_SRF(Srf)) {       /* No periodic trimmed srfs. */
	    TSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
	    CagdSrfFree(Srf);
	    Srf = TSrf;
	}
	if (BspSrfHasOpenEC(Srf)) {	          /* No float trimmed srfs. */
	    TSrf = CagdCnvrtFloat2OpenSrf(Srf);
	    CagdSrfFree(Srf);
	    Srf = TSrf;
	}
    }

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    MdlTrimSrf -> Pnext = NULL;
    MdlTrimSrf -> Attr = NULL;
    MdlTrimSrf -> Srf = Srf;
    MdlTrimSrf -> LoopList = NULL;
    
    if (!HasTopLvlTrim) {
	CagdRType **UVPoints;
	CagdCrvStruct *UVCrv, *RevCrv, *EucCrv;

	UVCrv = BzrCrvNew(2, CAGD_PT_E2_TYPE);
	UVPoints = UVCrv -> Points;
	UVPoints[1][0] = UMin;
	UVPoints[2][0] = VMin;
	UVPoints[1][1] = UMin;
	UVPoints[2][1] = VMax;
	EucCrv = CagdBndryCrvFromSrf(Srf, CAGD_U_MIN_BNDRY);
        MdlTrimSeg = MdlTrimSegNew(UVCrv, NULL, EucCrv, MdlTrimSrf, NULL);
	MdlTrimSegRef = MdlTrimSegRefNew(MdlTrimSeg);
	Loop = MdlLoopNew(MdlTrimSegRef);

	UVCrv = BzrCrvNew(2, CAGD_PT_E2_TYPE);
	UVPoints = UVCrv -> Points;
	UVPoints[1][0] = UMin;
	UVPoints[2][0] = VMax;
	UVPoints[1][1] = UMax;
	UVPoints[2][1] = VMax;
	EucCrv = CagdBndryCrvFromSrf(Srf, CAGD_V_MAX_BNDRY);
        MdlTrimSeg -> Pnext = MdlTrimSegNew(UVCrv, NULL, EucCrv, 
					    MdlTrimSrf, NULL);
	MdlTrimSegRef -> Pnext = MdlTrimSegRefNew(MdlTrimSeg -> Pnext);


	UVCrv = BzrCrvNew(2, CAGD_PT_E2_TYPE);
	UVPoints = UVCrv -> Points;
	UVPoints[1][0] = UMax;
	UVPoints[2][0] = VMax;
	UVPoints[1][1] = UMax;
	UVPoints[2][1] = VMin;
	EucCrv = CagdBndryCrvFromSrf(Srf, CAGD_U_MAX_BNDRY);
	RevCrv = CagdCrvReverse(EucCrv);
        CagdCrvFree(EucCrv);
        EucCrv = RevCrv;
        MdlTrimSeg -> Pnext -> Pnext = 
            MdlTrimSegNew(UVCrv, NULL, EucCrv, MdlTrimSrf, NULL);
	MdlTrimSegRef -> Pnext -> Pnext = 
            MdlTrimSegRefNew(MdlTrimSeg -> Pnext -> Pnext);

	UVCrv = BzrCrvNew(2, CAGD_PT_E2_TYPE);
	UVPoints = UVCrv -> Points;
	UVPoints[1][0] = UMax;
	UVPoints[2][0] = VMin;
	UVPoints[1][1] = UMin;
	UVPoints[2][1] = VMin;
	EucCrv = CagdBndryCrvFromSrf(Srf, CAGD_V_MIN_BNDRY);
	RevCrv = CagdCrvReverse(EucCrv); 
        CagdCrvFree(EucCrv);
        EucCrv = RevCrv;
        MdlTrimSeg -> Pnext -> Pnext -> Pnext = 
            MdlTrimSegNew(UVCrv, NULL, EucCrv, MdlTrimSrf, NULL);

	MdlTrimSegRef -> Pnext -> Pnext -> Pnext =
            MdlTrimSegRefNew(MdlTrimSeg -> Pnext -> Pnext -> Pnext);
	MdlTrimSrf -> LoopList = Loop;
    }

    /* Chain the input loop list into the trimmed surface. */
    if (MdlTrimSrf -> LoopList == NULL)
	MdlTrimSrf -> LoopList = LoopList;
    else
	MdlTrimSrf -> LoopList -> Pnext = LoopList;

    /* Update back pointers from trimming segments back to the surface. */
    if (UpdateBackTSrfPtrs) {
        for (Loop = LoopList; Loop != NULL; Loop = Loop -> Pnext) {
	    MdlTrimSegRefStruct
	        *SegRef = Loop -> SegRefList;

	    for ( ; SegRef != NULL; SegRef = SegRef -> Pnext) {
	        assert(SegRef -> TrimSeg -> UVCrvFirst != NULL);
		SegRef -> TrimSeg -> SrfFirst = MdlTrimSrf;
	    }
	}
    }

    return MdlTrimSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructor for a model trimmed surface.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             The original surface to be trimmed.  Used in place.     M
*   LoopList:        A list of trimming loops.	Used in place.		     M
*   HasTopLvlTrim:   If FALSE, add outer loops boundary.	 	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlTrimSrfStruct *:   The trimmed surface.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlTrimSrfNew2, allocation                                               M
*****************************************************************************/
MdlTrimSrfStruct *MdlTrimSrfNew2(CagdSrfStruct *Srf,
	 		         CagdCrvStruct *LoopList, 
			         CagdBType HasTopLvlTrim)
{
    MdlLoopStruct *Loop,
	*LoopList2 = NULL;
    CagdCrvStruct *LoopListNext;

    while (LoopList) {
	LoopListNext = LoopList -> Pnext; 
        Loop = MdlLoopNew(MdlTrimSegRefNew(MdlTrimSegNew(LoopList, NULL, NULL,
							 NULL, NULL)));
        LoopList = LoopListNext;

        IRIT_LIST_PUSH(Loop, LoopList2);
    }

    return MdlTrimSrfNew(Srf, LoopList2, HasTopLvlTrim, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructor for a Model.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             The original surface to be made into a model.	     M
*                    Used in place.			     		     M
*   LoopList:        A list of trimming loops.  Used in place.               M
*   HasTopLvlTrim:   If FALSE, add outer loops boundary.	 	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *: The Model.	                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelNew, Allocation                                                  M
*****************************************************************************/
MdlModelStruct *MdlModelNew(CagdSrfStruct *Srf,
		            CagdCrvStruct *LoopList,
		            CagdBType HasTopLvlTrim)
{
    MdlModelStruct
	*MdlModel = (MdlModelStruct *) IritMalloc(sizeof(MdlModelStruct));

    MdlModel -> TrimSrfList = MdlTrimSrfNew2(Srf, LoopList, HasTopLvlTrim);
    if (LoopList) {
        MdlModel -> TrimSegList = MdlModel -> TrimSrfList ->
                                            LoopList -> SegRefList -> TrimSeg;
    }
    else {
        if (HasTopLvlTrim)
	    MdlModel -> TrimSegList = NULL;
        else
            MdlModel -> TrimSegList = MdlModel -> TrimSrfList -> LoopList
                                                     -> SegRefList -> TrimSeg;
    }

    MdlModel -> Pnext = NULL;
    MdlModel -> Attr = NULL;

    return MdlModel;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructor for a Model.  No attempt is mode to verify the consistency   M
* of the given data and proper pointers between the different data strcutres.M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfs:  Trimming surfaces. Used in place.                             M
*   TrimSegs:  A list of trimming segments. Used in place.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlModelStruct *: The construct Model.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MdlModelNew, MdlPatchTrimmingSegPointers                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelNew2, Allocation                                                 M
*****************************************************************************/
MdlModelStruct *MdlModelNew2(MdlTrimSrfStruct *TrimSrfs,
                             MdlTrimSegStruct *TrimSegs)
{
    MdlModelStruct
        *MdlModel = (MdlModelStruct *) IritMalloc(sizeof(MdlModelStruct));

    MdlModel -> TrimSrfList = TrimSrfs;
    MdlModel -> TrimSegList = TrimSegs;

    MdlModel -> Pnext = NULL;
    MdlModel -> Attr = NULL;

    return MdlModel;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Linearly transforms, in place, given model as specified by Translate and   M
* Scale.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:         Model to transform.		                             M
*   Translate:     Translation factor.                                       M
*   Scale:         Scaling factor.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelTransform, models		                                     M
*****************************************************************************/
void MdlModelTransform(MdlModelStruct *Model,
		       const CagdRType *Translate,
		       CagdRType Scale)
{
    MdlTrimSrfStruct
	*TrimSrfList = Model -> TrimSrfList;
    MdlTrimSegStruct
	*TrimSegList = Model -> TrimSegList;

    /* Transform the geometry. */
    for ( ; TrimSrfList != NULL; TrimSrfList = TrimSrfList -> Pnext)
    CagdSrfTransform(TrimSrfList -> Srf, Translate, Scale);

    /* And remove all Euclidean trimming curves. */
    for ( ; TrimSegList != NULL; TrimSegList = TrimSegList -> Pnext) {
	if (TrimSegList -> EucCrv != NULL) {
	    CagdCrvFree(TrimSegList -> EucCrv);
	    TrimSegList -> EucCrv = NULL;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Transforms, in place, the given model as specified by homogeneous matrix   M
* Mat.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Model:         Model to transform.		                             M
*   Mat:           Homogeneous transformation to apply to TV.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlModelMatTransform, models                                             M
*****************************************************************************/
void MdlModelMatTransform(MdlModelStruct *Model, CagdMType Mat)
{
    MdlTrimSrfStruct
	*TrimSrfList = Model -> TrimSrfList;
    MdlTrimSegStruct
	*TrimSegList = Model -> TrimSegList;

    /* Transform the geometry. */
    for ( ; TrimSrfList != NULL; TrimSrfList = TrimSrfList -> Pnext) {
        CagdSrfStruct
	    *TSrf = CagdSrfMatTransform(TrimSrfList -> Srf, Mat);

	CagdSrfFree(TrimSrfList -> Srf);
	TrimSrfList -> Srf = TSrf;
    }

    /* And remove all Euclidean trimming curves. */
    for ( ; TrimSegList != NULL; TrimSegList = TrimSegList -> Pnext) {
	if (TrimSegList -> EucCrv != NULL) {
	    CagdCrvFree(TrimSegList -> EucCrv);
	    TrimSegList -> EucCrv = NULL;
	}
    }
}
