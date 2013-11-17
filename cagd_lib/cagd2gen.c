/******************************************************************************
* Cagd_gen.c - General routines used by all modules of CAGD_lib.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <string.h>
#include "cagd_loc.h"
#include "geom_lib.h"
#include "miscattr.h"

#ifdef DEBUG
#undef CagdCrvFree
#undef CagdCrvFreeList
#undef CagdSrfFree
#undef CagdSrfFreeList
#undef CagdUVFree
#undef CagdUVFreeList
#undef CagdUVArrayFree
#undef CagdPtFree
#undef CagdPtFreeList
#undef CagdSrfPtFree
#undef CagdSrfPtFreeList
#undef CagdPtArrayFree
#undef CagdCtlPtFree
#undef CagdCtlPtFreeList
#undef CagdCtlPtArrayFree
#undef CagdVecFree
#undef CagdVecFreeList
#undef CagdVecArrayFree
#undef CagdPlaneFree
#undef CagdPlaneFreeList
#undef CagdPlaneArrayFree
#undef CagdBBoxFree
#undef CagdBBoxFreeList
#undef CagdBBoxArrayFree
#undef CagdPolylineFree
#undef CagdPolylineFreeList
#undef CagdPolygonFree
#undef CagdPolygonFreeList
#endif /* DEBUG */

/* Control of the linear order surface convertion to polygon: */
IRIT_GLOBAL_DATA CagdLin2PolyType
    _CagdLin2Poly = CAGD_ONE_POLY_PER_COLIN;

IRIT_STATIC_DATA CagdPlgErrorFuncType
    PolygonErrFunc = NULL;

IRIT_GLOBAL_DATA CagdBType
    _CagdSrfMakeOnlyTri = FALSE;
IRIT_GLOBAL_DATA CagdSrfMakeTriFuncType
    _CagdSrfMakeTriFunc = NULL;
IRIT_GLOBAL_DATA CagdSrfMakeRectFuncType
    _CagdSrfMakeRectFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of vector structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   VecList:       To be copied.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A duplicated list of vectors.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecCopyList, copy                                                    M
*****************************************************************************/
CagdVecStruct *CagdVecCopyList(const CagdVecStruct *VecList)
{
    CagdVecStruct *VecTemp, *NewVecList;

    if (VecList == NULL)
	return NULL;
    VecTemp = NewVecList = CagdVecCopy(VecList);
    VecList = VecList -> Pnext;
    while (VecList) {
	VecTemp -> Pnext = CagdVecCopy(VecList);
	VecTemp = VecTemp -> Pnext;
	VecList = VecList -> Pnext;
    }
    return NewVecList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of plane structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PlaneList:       To be copied.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPlaneStruct *:  A duplicated list of planes.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneCopyList, copy                                                  M
*****************************************************************************/
CagdPlaneStruct *CagdPlaneCopyList(const CagdPlaneStruct *PlaneList)
{
    CagdPlaneStruct *PlaneTemp, *NewPlaneList;

    if (PlaneList == NULL)
	return NULL;
    PlaneTemp = NewPlaneList = CagdPlaneCopy(PlaneList);
    PlaneList = PlaneList -> Pnext;
    while (PlaneList) {
	PlaneTemp -> Pnext = CagdPlaneCopy(PlaneList);
	PlaneTemp = PlaneTemp -> Pnext;
	PlaneList = PlaneList -> Pnext;
    }
    return NewPlaneList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of bbox structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   BBoxList:       To be copied.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBBoxStruct *:  A duplicated list of bbox's.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxCopyList, copy                                                   M
*****************************************************************************/
CagdBBoxStruct *CagdBBoxCopyList(const CagdBBoxStruct *BBoxList)
{
    CagdBBoxStruct *BBoxTemp, *NewBBoxList;

    if (BBoxList == NULL)
	return NULL;
    BBoxTemp = NewBBoxList = CagdBBoxCopy(BBoxList);
    BBoxList = BBoxList -> Pnext;
    while (BBoxList) {
	BBoxTemp -> Pnext = CagdBBoxCopy(BBoxList);
	BBoxTemp = BBoxTemp -> Pnext;
	BBoxList = BBoxList -> Pnext;
    }
    return NewBBoxList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of polyline structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:       To be copied.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A duplicated list of polylines.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolylineCopyList, copy                                               M
*****************************************************************************/
CagdPolylineStruct *CagdPolylineCopyList(const CagdPolylineStruct *PolyList)
{
    CagdPolylineStruct *PolyTemp, *NewPolyList;

    if (PolyList == NULL)
	return NULL;
    PolyTemp = NewPolyList = CagdPolylineCopy(PolyList);
    PolyList = PolyList -> Pnext;
    while (PolyList) {
	PolyTemp -> Pnext = CagdPolylineCopy(PolyList);
	PolyTemp = PolyTemp -> Pnext;
	PolyList = PolyList -> Pnext;
    }
    return NewPolyList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies a list of polygon structures.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:       To be copied.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A duplicated list of polygons.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonCopyList, copy                                                M
*****************************************************************************/
CagdPolygonStruct *CagdPolygonCopyList(const CagdPolygonStruct *PolyList)
{
    CagdPolygonStruct *PolyTemp, *NewPolyList;

    if (PolyList == NULL)
	return NULL;
    PolyTemp = NewPolyList = CagdPolygonCopy(PolyList);
    PolyList = PolyList -> Pnext;
    while (PolyList) {
	PolyTemp -> Pnext = CagdPolygonCopy(PolyList);
	PolyTemp = PolyTemp -> Pnext;
	PolyList = PolyList -> Pnext;
    }
    return NewPolyList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a curve structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvFree, free                                                        M
*****************************************************************************/
void CagdCrvFree(CagdCrvStruct *Crv)
{
    if (Crv == NULL)
	return;

    if (Crv -> KnotVector != NULL)
	IritFree(Crv -> KnotVector);

    IP_ATTR_FREE_ATTRS(Crv -> Attr);

#ifdef CAGD_MALLOC_STRUCT_ONCE
    {
#   ifdef DEBUG
        int i,
	    MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);

        /* Make sure all control points are allocated as one vector. */
	for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType) + 1; i <= MaxAxis; i++) {
	    if ((Crv -> Points[i] - Crv -> Points[i - 1]) != Crv -> Length)
	        CAGD_FATAL_ERROR(CAGD_ERR_INVALID_CRV);
	}
#   endif /* DEBUG */

	IritFree(Crv);
    }
#else
    {
        int i,
	    MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);

	for (i = !CAGD_IS_RATIONAL_CRV(Crv); i <= MaxAxis; i++)
	    IritFree(Crv -> Points[i]);

	IritFree(Crv);
    }
#endif /* CAGD_MALLOC_STRUCT_ONCE */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a curve structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:  To be deallocated.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvFreeList, free                                                    M
*****************************************************************************/
void CagdCrvFreeList(CagdCrvStruct *CrvList)
{
    CagdCrvStruct *CrvTemp;

    while (CrvList) {
	CrvTemp = CrvList -> Pnext;
	CagdCrvFree(CrvList);
	CrvList = CrvTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a surface sstructure.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfFree, free                                                        M
*****************************************************************************/
void CagdSrfFree(CagdSrfStruct *Srf)
{
    if (Srf == NULL)
	return;

#ifdef CAGD_MALLOC_STRUCT_ONCE
    {
#   ifdef DEBUG
        int i,
	    MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);

        /* Make sure all control points are allocated as one vector. */
	for (i = !CAGD_IS_RATIONAL_PT(Srf -> PType) + 1; i <= MaxAxis; i++) {
	    if ((Srf -> Points[i] - Srf -> Points[i - 1]) !=
					    Srf -> ULength * Srf -> VLength)
		CAGD_FATAL_ERROR(CAGD_ERR_INVALID_SRF);
	}
#   endif /* DEBUG */
    }
#else
    {
        int i,
	    MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);

	for (i = !CAGD_IS_RATIONAL_SRF(Srf); i <= MaxAxis; i++)
	    IritFree(Srf -> Points[i]);
    }
#endif /* CAGD_MALLOC_STRUCT_ONCE */

    if (Srf -> UKnotVector != NULL)
	IritFree(Srf -> UKnotVector);
    if (Srf -> VKnotVector != NULL)
	IritFree(Srf -> VKnotVector);

    IP_ATTR_FREE_ATTRS(Srf -> Attr);

    CagdSrfFreeCache(Srf);

    IritFree(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a surface cache structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:  To  deallocate its cache.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfFreeCache, free                                                   M
*****************************************************************************/
void CagdSrfFreeCache(CagdSrfStruct *Srf)
{
    if (Srf -> PAux != NULL) {
	CagdSrfEvalCacheStruct
	    *SrfEvalCache = (CagdSrfEvalCacheStruct *) Srf -> PAux;

        CagdCrvFree(SrfEvalCache -> IsoSubCrv);
        IritFree(SrfEvalCache -> VBasisFunc);
	IritFree(SrfEvalCache);
	Srf -> PAux = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a surface structure list:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:  To be deallocated.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfFreeList, free                                                    M
*****************************************************************************/
void CagdSrfFreeList(CagdSrfStruct *SrfList)
{
    CagdSrfStruct *SrfTemp;

    while (SrfList) {
	SrfTemp = SrfList -> Pnext;
	CagdSrfFree(SrfList);
	SrfList = SrfTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a UV structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   UV:       To be deallocated.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVFree, free                                                         M
*****************************************************************************/
void CagdUVFree(CagdUVStruct *UV)
{
    if (UV == NULL)
	return;

    IP_ATTR_FREE_ATTRS(UV -> Attr);

    IritFree(UV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a UV structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVList:  To be deallocated.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVFreeList, free                                                     M
*****************************************************************************/
void CagdUVFreeList(CagdUVStruct *UVList)
{
    CagdUVStruct *UVTemp;

    while (UVList) {
	UVTemp = UVList -> Pnext;
	CagdUVFree(UVList);
	UVList = UVTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of UV structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   UVArray:     To be deallocated.                                          M
*   Size:        Of the deallocated array.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdUVArrayFree, free                                                    M
*****************************************************************************/
void CagdUVArrayFree(CagdUVStruct *UVArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(UVArray[i].Attr);

    IritFree(UVArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a point structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:       To be deallocated.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtFree, free                                                         M
*****************************************************************************/
void CagdPtFree(CagdPtStruct *Pt)
{
    if (Pt == NULL)
	return;

    IP_ATTR_FREE_ATTRS(Pt -> Attr);

    IritFree(Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a point structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:  To be deallocated.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtFreeList, free                                                     M
*****************************************************************************/
void CagdPtFreeList(CagdPtStruct *PtList)
{
    CagdPtStruct *PtTemp;

    while (PtList) {
	PtTemp = PtList -> Pnext;
	CagdPtFree(PtList);
	PtList = PtTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a point structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfPt:       To be deallocated.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfPtFree, free                                                      M
*****************************************************************************/
void CagdSrfPtFree(CagdSrfPtStruct *SrfPt)
{
    if (SrfPt == NULL)
	return;

    IP_ATTR_FREE_ATTRS(SrfPt -> Attr);

    IritFree(SrfPt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a point structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfPtList:  To be deallocated.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfPtFreeList, free                                                  M
*****************************************************************************/
void CagdSrfPtFreeList(CagdSrfPtStruct *SrfPtList)
{
    CagdSrfPtStruct *SrfPtTemp;

    while (SrfPtList) {
	SrfPtTemp = SrfPtList -> Pnext;
	CagdSrfPtFree(SrfPtList);
	SrfPtList = SrfPtTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of Pt structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtArray:     To be deallocated.                                          M
*   Size:        Of the deallocated array.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtArrayFree, free                                                    M
*****************************************************************************/
void CagdPtArrayFree(CagdPtStruct *PtArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(PtArray[i].Attr);

    IritFree(PtArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a CtlPt structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   CtlPt:       To be deallocated.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtFree, free                                                      M
*****************************************************************************/
void CagdCtlPtFree(CagdCtlPtStruct *CtlPt)
{
    if (CtlPt == NULL)
	return;

    IP_ATTR_FREE_ATTRS(CtlPt -> Attr);

    IritFree(CtlPt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a CtlPt structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CtlPtList:  To be deallocated.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtFreeList, free                                                  M
*****************************************************************************/
void CagdCtlPtFreeList(CagdCtlPtStruct *CtlPtList)
{
    CagdCtlPtStruct *CtlPtTemp;

    while (CtlPtList) {
	CtlPtTemp = CtlPtList -> Pnext;
	CagdCtlPtFree(CtlPtList);
	CtlPtList = CtlPtTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of CtlPt structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CtlPtArray:     To be deallocated.                                       M
*   Size:           Of the deallocated array.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCtlPtArrayFree, free                                                 M
*****************************************************************************/
void CagdCtlPtArrayFree(CagdCtlPtStruct *CtlPtArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(CtlPtArray[i].Attr);

    IritFree(CtlPtArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a vector structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:       To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecFree, free                                                        M
*****************************************************************************/
void CagdVecFree(CagdVecStruct *Vec)
{
    if (Vec == NULL)
	return;

    IP_ATTR_FREE_ATTRS(Vec -> Attr);

    IritFree(Vec);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a vector structure list:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecList:  To be deallocated.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecFreeList, free                                                    M
*****************************************************************************/
void CagdVecFreeList(CagdVecStruct *VecList)
{
    CagdVecStruct *VecTemp;

    while (VecList) {
	VecTemp = VecList -> Pnext;
	CagdVecFree(VecList);
	VecList = VecTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of vector structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecArray:     To be deallocated.                                         M
*   Size:         Of the deallocated array.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdVecArrayFree, free                                                   M
*****************************************************************************/
void CagdVecArrayFree(CagdVecStruct *VecArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(VecArray[i].Attr);

    IritFree(VecArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a plane structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Plane:       To be deallocated.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneFree, free                                                      M
*****************************************************************************/
void CagdPlaneFree(CagdPlaneStruct *Plane)
{
    if (Plane == NULL)
	return;

    IP_ATTR_FREE_ATTRS(Plane -> Attr);

    IritFree(Plane);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a plane structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlaneList:  To be deallocated.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneFreeList, free                                                  M
*****************************************************************************/
void CagdPlaneFreeList(CagdPlaneStruct *PlaneList)
{
    CagdPlaneStruct *PlaneTemp;

    while (PlaneList) {
	PlaneTemp = PlaneList -> Pnext;
	CagdPlaneFree(PlaneList);
	PlaneList = PlaneTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of plane structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PlaneArray:     To be deallocated.                                       M
*   Size:           Of the deallocated array.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneArrayFree, free                                                 M
*****************************************************************************/
void CagdPlaneArrayFree(CagdPlaneStruct *PlaneArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(PlaneArray[i].Attr);

    IritFree(PlaneArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a BBox structure.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox:       To be deallocated.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxFree, free                                                       M
*****************************************************************************/
void CagdBBoxFree(CagdBBoxStruct *BBox)
{
    if (BBox == NULL)
	return;

    IP_ATTR_FREE_ATTRS(BBox -> Attr);

    IritFree(BBox);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a BBox structure list:				     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBoxList:  To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxFreeList, free                                                   M
*****************************************************************************/
void CagdBBoxFreeList(CagdBBoxStruct *BBoxList)
{
    CagdBBoxStruct *BBoxTemp;

    while (BBoxList) {
	BBoxTemp = BBoxList -> Pnext;
	CagdBBoxFree(BBoxList);
	BBoxList = BBoxTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees an array of BBox structure.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBoxArray:    To be deallocated.                                         M
*   Size:         Of the deallocated array.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBBoxArrayFree, free                                                  M
*****************************************************************************/
void CagdBBoxArrayFree(CagdBBoxStruct *BBoxArray, int Size)
{
    int i;

    for (i = 0; i < Size; i++)
	IP_ATTR_FREE_ATTRS(BBoxArray[i].Attr);

    IritFree(BBoxArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a polyline structure.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:       To be deallocated.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolylineFree, free                                                   M
*****************************************************************************/
void CagdPolylineFree(CagdPolylineStruct *Poly)
{
    if (Poly == NULL)
	return;

    IritFree(Poly -> Polyline);
    IP_ATTR_FREE_ATTRS(Poly -> Attr);

    IritFree(Poly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a polyline structure list:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:  To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolylineFreeList, free                                               M
*****************************************************************************/
void CagdPolylineFreeList(CagdPolylineStruct *PolyList)
{
    CagdPolylineStruct *PolyTemp;

    while (PolyList) {
	PolyTemp = PolyList -> Pnext;
	CagdPolylineFree(PolyList);
	PolyList = PolyTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees all slots of a polygon structure.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:       To be deallocated.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonFree, free                                                    M
*****************************************************************************/
void CagdPolygonFree(CagdPolygonStruct *Poly)
{
    if (Poly == NULL)
	return;

    IP_ATTR_FREE_ATTRS(Poly -> Attr);

    if (Poly -> PolyType == CAGD_POLYGON_TYPE_POLYSTRIP) {
	IritFree(Poly -> U.PolyStrip.StripPt);
	IritFree(Poly -> U.PolyStrip.StripNrml);
	IritFree(Poly -> U.PolyStrip.StripUV);
    }

    IritFree(Poly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates and frees a polygon structure list:			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:  To be deallocated.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonFreeList, free                                                M
*****************************************************************************/
void CagdPolygonFreeList(CagdPolygonStruct *PolyList)
{
    CagdPolygonStruct *PolyTemp;

    while (PolyList) {
	PolyTemp = PolyList -> Pnext;
	CagdPolygonFree(PolyList);
	PolyList = PolyTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses a list of cagd library objects, in place.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   List:       To be reversed.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Reversed list.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdListReverse, reverse                                                 M
*****************************************************************************/
VoidPtr CagdListReverse(VoidPtr List)
{
    CagdGenericStruct
	*OldHead = (CagdGenericStruct *) List,
	*NewHead = NULL;

    if (OldHead == NULL || OldHead -> Pnext == NULL)
        return (VoidPtr) OldHead;

    while (OldHead) {
	CagdGenericStruct
	    *TmpStruct = OldHead -> Pnext;

	OldHead -> Pnext = NewHead;
	NewHead = OldHead;

	OldHead = TmpStruct;
    }

    return (VoidPtr) NewHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the last element of given list of cagd library objects.            M
*                                                                            *
* PARAMETERS:                                                                M
*   List:       To return its last element.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Last element.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdListLast		                                             M
*****************************************************************************/
VoidPtr CagdListLast(VoidPtr List)
{
    CagdGenericStruct
	*Head = (CagdGenericStruct *) List;

    if (Head == NULL)
	return NULL;

    for ( ; Head -> Pnext != NULL; Head = Head -> Pnext);

    return (VoidPtr) Head;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the element previous to given Item in List of cagd library objs.   M
*                                                                            *
* PARAMETERS:                                                                M
*   List:       To seek the previous element to Item.                        M
*   Item:	Item to seek its prev.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:    Previous item to Item or NULL if not found (or Item is the   M
*               first item in List).			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdListPrev		                                             M
*****************************************************************************/
VoidPtr CagdListPrev(VoidPtr List, VoidPtr Item)
{
    CagdGenericStruct *Prev,
	*Head = (CagdGenericStruct *) List;

    for (Prev = NULL;
	 Head != NULL && Head != Item;
	 Prev = Head, Head = Head -> Pnext);

    return (VoidPtr) Prev;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Inserts a new element NewElement into an ordered list List.  Ordering    M
* is prescribed by CompFunc.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   List:        The list to update, in place. Can be empty list.	     M
*   NewElement:  New element to insert, in place into List.		     M
*   CompFunc:    A comparison function.  Gets two elements of the list and   M
*		 compare and return a positive, zero, or negative values     M
*		 if first elements is smaller, equal, larger than second     M
*		 element, strcmp style.					     M
*   InsertEqual: If TRUE, a new item that is found to be equal to an item in M
*		 the list will be insert anyway. If FALSE, NULL is returned  M
*		 and no modification is made to the list, if equal.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:	 The updated list.  NULL if !InsertEqual and found equality. M
*		 If NULL is returned, no modification was made to List.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdListInsert                                                           M
*****************************************************************************/
VoidPtr CagdListInsert(VoidPtr List,
		       VoidPtr NewElement,
		       CagdCompFuncType CompFunc,
		       CagdBType InsertEqual)
{
    int CompVal;
    CagdGenericStruct
	*GList = (CagdGenericStruct *) List,
	*GNewElement = (CagdGenericStruct *) NewElement;

    if (GList == NULL)
        GList = GNewElement;
    else if ((CompVal = CompFunc(GList, GNewElement)) > 0 ||
	     (CompVal == 0 && InsertEqual)) {
	/* Put new element as first. */
	GNewElement -> Pnext = GList;
	GList = GNewElement;
    }
    else if (CompVal == 0 && !InsertEqual) {
	return NULL;
    }
    else {
	CagdGenericStruct
	    *Last = GList,
	    *Trace = Last -> Pnext;

	while (Trace != NULL && CompFunc(Trace, GNewElement) < 0) {
	    Last = Trace;
	    Trace = Last -> Pnext;
	}

	if (Trace != NULL &&
	    CompFunc(Trace, GNewElement) == 0 &&
	    !InsertEqual)
	    return NULL;
	else {
	    Last -> Pnext = GNewElement;
	    GNewElement -> Pnext = Trace;
	}
    }

    return (VoidPtr) GList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the length of a list.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   List:          List of cagd objects.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           Length of list.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdListLength                                                           M
*****************************************************************************/
int CagdListLength(const VoidPtr List)
{
    CagdGenericStruct
	*Head = (CagdGenericStruct *) List;
    int i;

    for (i = 0; Head != NULL; Head = Head -> Pnext, i++);

    return i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Appends two lists, in place.	                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   List1, List2:  Two lists of cagd objects to append, in place.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:       Appended list.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdListAppend                                                           M
*****************************************************************************/
VoidPtr CagdListAppend(VoidPtr List1, VoidPtr List2)
{
    CagdGenericStruct *Head1;

    if (List1 == NULL)
	return List2;
    else if (List2 == NULL)
        return List1;

    for (Head1 = (CagdGenericStruct *) List1;
	 Head1 -> Pnext != NULL;
	 Head1 = Head1 -> Pnext);
    Head1 -> Pnext = (CagdGenericStruct *) List2;

    return List1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transform, in place, to given curve Crv as specified by  M
* Translate and Scale.							     M
*   Each control point is first translated by Translate and then scaled by   M
* Scale.		                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         To be affinely transformed.                                 M
*   Translate:   Translation amount, NULL for non.                           M
*   Scale:       Scaling amount.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfTransform, CagdTransform, CagdCrvMatTransform, CagdCrvRotateToXY  M
*   CagdCrvScale							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvTransform, scaling, translation, transformations                  M
*****************************************************************************/
void CagdCrvTransform(CagdCrvStruct *Crv,
		      const CagdRType *Translate,
		      CagdRType Scale)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
    	    CagdTransform(Crv -> Points,
			  Crv -> Length,
    			  CAGD_NUM_OF_PT_COORD(Crv -> PType),
    			  !CAGD_IS_RATIONAL_CRV(Crv),
			  Translate,
			  Scale);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies a nonuniform scaling transform, in place, to given curve Crv as    M
* specified by Scale.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         To be nonuniformly scaled.                                  M
*   Scale:       Scaling amount.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfTransform, CagdTransform, CagdCrvMatTransform, CagdCrvRotateToXY  M
*   CagdCrvTransform							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvScale, scaling, transformations 		                     M
*****************************************************************************/
void CagdCrvScale(CagdCrvStruct *Crv, const CagdRType *Scale)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
    	    CagdScale(Crv -> Points, Crv -> Length,
		      CAGD_NUM_OF_PT_COORD(Crv -> PType),
		      Scale);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transform, in place, to given surface Srf as specified   M
* by Translate and Scale.						     M
*   Each control point is first translated by Translate and then scaled by   M
* Scale.		                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To be affinely transformed.                                 M
*   Translate:   Translation amount, NULl for non.                           M
*   Scale:       Scaling amount.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTransform, CagdTransform, CagdSrfMatTransform                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfTransform, scaling, translation, transformations                  M
*****************************************************************************/
void CagdSrfTransform(CagdSrfStruct *Srf,
		      const CagdRType *Translate,
		      CagdRType Scale)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
	    CagdTransform(Srf -> Points,
	    		  Srf -> ULength * Srf -> VLength,
	                  CAGD_NUM_OF_PT_COORD(Srf -> PType),
			  !CAGD_IS_RATIONAL_SRF(Srf),
		          Translate,
        	          Scale);
	    break;
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies a nonuniform scaling transform, in place, to given curve Srf as    M
* specified by Scale.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To be nonuniformly scaled.                                  M
*   Scale:       Scaling amount.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTransform, CagdTransform, CagdSrfMatTransform, CagdSrfRotateToXY  M
*   CagdSrfTransform							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfScale, scaling, transformations 		                     M
*****************************************************************************/
void CagdSrfScale(CagdSrfStruct *Srf, const CagdRType *Scale)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
    	    CagdScale(Srf -> Points, Srf -> ULength * Srf -> VLength,
		      CAGD_NUM_OF_PT_COORD(Srf -> PType),
		      Scale);
	    break;
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transform, in place, to given set of points Points which M
* as array of vectors, each vector of length Len.                            M
*   Array Points optionally contains (if !IsNotRational) in Points[0] the    M
* weights coefficients and in Points[i] the coefficients of axis i, up to    M
* and include MaxCoord (X = 1, Y = 2, etc.).				     M
* Points are translated and scaled as prescribed by Translate and Scale.     M
*   Each control point is first translated by Translate and then scaled by   M
* Scale.		                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:         To be affinely transformed. Array of vectors.            M
*   Len:            Of vectors of Points.                                    M
*   MaxCoord:       Maximum number of coordinates to be found in Points.     M
*		    At most 3 - R^3.					     M
*   IsNotRational:  Do we have weights as vector Points[0]?                  M
*   Translate:      Translation amount, NULL for non.                        M
*   Scale:          Scaling amount.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfTransform, CagdCrvTransform, CagdTransform                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdTransform, scaling, translation, transformations                     M
*****************************************************************************/
void CagdTransform(CagdRType **Points,
		   int Len,
		   int MaxCoord,
		   CagdBType IsNotRational,
		   const CagdRType *Translate,
		   CagdRType Scale)
{
    int i, j;

    if (MaxCoord > 3)
        MaxCoord = 3;

    if (Translate == NULL) {
        for (i = 1; i <= MaxCoord; i++)
	    for (j = 0; j < Len; j++)
	        Points[i][j] *= Scale;
    }
    else {
        if (IsNotRational)
	    for (i = 1; i <= MaxCoord; i++)
	        for (j = 0; j < Len; j++)
		    Points[i][j] = (Points[i][j] + Translate[i - 1]) * Scale;
	else
	    for (i = 1; i <= MaxCoord; i++)
	        for (j = 0; j < Len; j++)
		    Points[i][j] = (Points[i][j] +
				    Translate[i - 1] * Points[W][j]) * Scale;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies a scale transform, in place, to given set of points Points which   M
* as array of vectors, each vector of length Len.                            M
*   Array Points optionally contains (if !IsNotRational) in Points[0] the    M
* weights coefficients and in Points[i] the coefficients of axis i, up to    M
* and include MaxCoord (X = 1, Y = 2, etc.).				     M
* Points are scaled as prescribed by Scale.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:         To be affinely transformed. Array of vectors.            M
*   Len:            Of vectors of Points.                                    M
*   MaxCoord:       Maximum number of coordinates to be found in Points.     M
*   Scale:          Scaling amount.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfScale, CagdCrvScale, CagdTransform		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdScale, scaling, transformations			                     M
*****************************************************************************/
void CagdScale(CagdRType **Points,
	       int Len,
	       int MaxCoord,
	       const CagdRType *Scale)
{
    int i, j;

    for (i = 1; i <= MaxCoord; i++)
	for (j = 0; j < Len; j++)
	    Points[i][j] *= Scale[i - 1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an homogeneous transformation, to the given curve Crv as specified M
* by homogeneous transformation Mat.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be transformed.                                            M
*   Mat:       Defining the transformation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Returned transformed curve.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdTransform, CagdSrfMatTransform, CagdMatTransform, CagdCrvRotateToXY  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvMatTransform, scaling, rotation, translation, transformations     M
*****************************************************************************/
CagdCrvStruct *CagdCrvMatTransform(const CagdCrvStruct *Crv,
				   CagdMType Mat)
{
    CagdCrvStruct *NewCrv;

    switch (Crv -> GType) {
	case CAGD_CPOWER_TYPE:
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
	    switch (Crv -> PType) {               /* Make sure input is 3D. */
		case CAGD_PT_E1_TYPE:
		case CAGD_PT_E2_TYPE:
		    NewCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);
		    break;
		case CAGD_PT_P1_TYPE:
		case CAGD_PT_P2_TYPE:
		    NewCrv = CagdCoerceCrvTo(Crv, CAGD_PT_P3_TYPE, FALSE);
		    break;
		default:
		    NewCrv = CagdCrvCopy(Crv);
		    break;
	    }

	    CagdMatTransform(NewCrv -> Points,
    			     NewCrv -> Length,
                	     CAGD_NUM_OF_PT_COORD(NewCrv -> PType),
			     !CAGD_IS_RATIONAL_CRV(NewCrv),
		             Mat);
	    break;
	default:
	    NewCrv = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an homogeneous transformation, to the given surface Srf as         M
* specified by homogeneous transformation Mat.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be transformed.                                            M
*   Mat:       Defining the transformation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Returned transformed surface.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdTransform, CagdCrvMatTransform, CagdMatTransform                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfMatTransform, scaling, rotation, translation, transformations     M
*****************************************************************************/
CagdSrfStruct *CagdSrfMatTransform(const CagdSrfStruct *Srf,
				   CagdMType Mat)
{
    CagdSrfStruct *NewSrf;

    switch (Srf -> GType) {
	case CAGD_SPOWER_TYPE:
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
	    switch (Srf -> PType) {               /* Make sure input is 3D. */
		case CAGD_PT_E1_TYPE:
		case CAGD_PT_E2_TYPE:
		    NewSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);
		    break;
		case CAGD_PT_P1_TYPE:
		case CAGD_PT_P2_TYPE:
		    NewSrf = CagdCoerceSrfTo(Srf, CAGD_PT_P3_TYPE, FALSE);
		    break;
		default:
		    NewSrf = CagdSrfCopy(Srf);
		    break;
	    }

	    CagdMatTransform(NewSrf -> Points,
    			     NewSrf -> ULength * NewSrf -> VLength,
        	             CAGD_NUM_OF_PT_COORD(NewSrf -> PType),
			     !CAGD_IS_RATIONAL_SRF(NewSrf),
		             Mat);
	    break;
	default:
	    NewSrf = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    break;
    }

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an homogeneous transformation, in place, to given set of points    M
* Points which as array of vectors, each vector of length Len.               M
*   Array Points optionally contains (if !IsNotRational) in Points[0] the    M
* weights coefficients and in Points[i] the coefficients of axis i, up to    M
* and include MaxCoord (X = 1, Y = 2, etc.).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:         To be affinely transformed. Array of vectors.            M
*   Len:            Of vectors of Points.                                    M
*   MaxCoord:       Maximum number of coordinates to be found in Points.     M
*   IsNotRational:  Do we have weights as vector Points[0]?                  M
*   Mat:            Defining the transformation.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdTransform, CagdSrfMatTransform, CagdCrvMatTransform                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMatTransform, scaling, rotation, translation, transformations        M
*****************************************************************************/
void CagdMatTransform(CagdRType **Points,
		      int Len,
		      int MaxCoord,
		      CagdBType IsNotRational,
		      CagdMType Mat)
{
    int i, j;
    CagdRType P[4], Q[4];

    if (MaxCoord > 3)
	MaxCoord = 3;

    if (IsNotRational)
	for (i = 0; i < Len; i++) {
	    for (j = 1; j <= MaxCoord; j++)
		P[j - 1] = Points[j][i];

	    /* Zero unused coords. */
	    for (j = MaxCoord + 1; j < 4; j++)
		P[j - 1] = 0.0;

            MatMultPtby4by4(Q, P, Mat);

	    for (j = 1; j <= MaxCoord; j++)
		Points[j][i] = Q[j - 1];
        }
    else
	for (i = 0; i < Len; i++) {
	    for (j = 1; j <= MaxCoord; j++)
		P[j - 1] = Points[j][i];
	    P[3] = Points[W][i];

	    /* Zero unused coords. */
	    for (j = MaxCoord + 1; j < 4; j++)
		P[j - 1] = 0.0;

            MatMultWVecby4by4(Q, P, Mat);

	    for (j = 1; j <= MaxCoord; j++)
		Points[j][i] = Q[j - 1];
	    Points[W][i] = Q[3];
        }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Normalize in place the given curve so its maximal coefficient            M
* is of unit size.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv: Curve to normalize in place its coefficients.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Normalized curve.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfUnitMaxCoef                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvUnitMaxCoef                                                       M
*****************************************************************************/
CagdCrvStruct *CagdCrvUnitMaxCoef(CagdCrvStruct *Crv)
{
    int i,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType Scl[CAGD_MAX_PT_COORD], 
	MaxVal = IRIT_UEPS;
    CagdBBoxStruct BBox;

    CagdCrvBBox(Crv, &BBox);

    for (i = 0; i < MaxCoord; i++) {
	CagdRType
	    R = IRIT_MAX(IRIT_FABS(BBox.Min[i]), IRIT_FABS(BBox.Max[i]));

	if (MaxVal < R)
	    MaxVal = R;
    }
    for (i = 0; i < MaxCoord; i++)
        Scl[i] = 1.0 / MaxVal;

    CagdCrvScale(Crv, Scl);

    return Crv;
}
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Normalize in place the given surface so its maximal coefficient          M
* is of unit size.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf: Surface to normalize in place its coefficients.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Normalized surface.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvUnitMaxCoef                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfUnitMaxCoef                                                       M
*****************************************************************************/
CagdSrfStruct *CagdSrfUnitMaxCoef(CagdSrfStruct *Srf)
{
    int i,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType Scl[CAGD_MAX_PT_COORD], 
	MaxVal = IRIT_UEPS;
    CagdBBoxStruct BBox;

    CagdSrfBBox(Srf, &BBox);

    for (i = 0; i < MaxCoord; i++) {
	CagdRType
	    R = IRIT_MAX(IRIT_FABS(BBox.Min[i]), IRIT_FABS(BBox.Max[i]));

	if (MaxVal < R)
	    MaxVal = R;
    }
    for (i = 0; i < MaxCoord; i++)
        Scl[i] = 1.0 / MaxVal;

    CagdSrfScale(Srf, Scl);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a rotation matrix to rotate the given (hopefully planar) curve  M
* to the XY plane, in place.						     M
* If the curve is not planar, the rotation is heuristic and is not optimal   M
* in any sense.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute a matrix that rotate (and possibly translate) Crv   M
*             to the XY plane.					             M
*   Mat:      Defining the transformation.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if reasonably successfull, FALSE if failed.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTransform, CagdCrvMatTransform, CagdCrvRotateToXY                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvRotateToXYMat                                                     M
*****************************************************************************/
int CagdCrvRotateToXYMat(const CagdCrvStruct *Crv, IrtHmgnMatType Mat)
{
    int i, l;
    IrtHmgnMatType TmpMat;
    CagdCrvStruct *TCrv,
	*E3Crv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);
    CagdRType *R,
	ZAverage = 0.0,
	**Points = E3Crv -> Points;
    CagdVType V1, V2, V;

    /* Estimate the normal of the plane where the curve resides. */
    IRIT_PT_RESET(V);
    for (i = 0; i < 3; i++)
	V1[i] = Points[i + 1][1] - Points[i + 1][0];
    for (l = 2; l < E3Crv -> Length; l++) {
	for (i = 0; i < 3; i++)
	    V2[i] = Points[i + 1][l] - Points[i + 1][l - 1];
	if (IRIT_PT_EQ_ZERO(V)) {
	    IRIT_CROSS_PROD(V, V1, V2);
	}
	else {
	    CagdVType VTmp;
	    IRIT_CROSS_PROD(VTmp, V1, V2);

	    if (IRIT_DOT_PROD(V, VTmp) < 0.0) {
		IRIT_PT_SUB(V, V, VTmp);
	    }
	    else {
		IRIT_PT_ADD(V, V, VTmp);
	    }
	}
    }

    if (IRIT_PT_EQ_ZERO(V)) {
	CagdCrvFree(E3Crv);
	return FALSE;
    }

    /* Rotate the curve so that V follows the Z axis. */
    if (V[2] < 0.0)
	IRIT_PT_SCALE(V, -1.0); /* Make sure we are in northern hemisphere. */
    GMGenRotateMatrix(Mat, V);

    /* Translates in Z to the XY plane. */
    TCrv = CagdCrvMatTransform(E3Crv, Mat);
    CagdCrvFree(E3Crv);
    E3Crv = TCrv;

    for (i = 0, R = E3Crv -> Points[3]; i < E3Crv -> Length; i++)
	ZAverage += *R++;

    MatGenMatTrans(0.0, 0.0, -ZAverage / E3Crv -> Length, TmpMat);
    MatMultTwo4by4(Mat, Mat, TmpMat);

    CagdCrvFree(E3Crv);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Rotates the given (hopefully planar) curve to the XY plane.		     M
* If the curve is not planar, the rotation is heuristic and is not optimal   M
* in any sense.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To rotate, to the XY plane.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Rotated Crv if reasonably successfull, NULL if failed. M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvTransform, CagdCrvMatTransform, CagdCrvRotateToXYMat              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvRotateToXY                                                        M
*****************************************************************************/
CagdCrvStruct *CagdCrvRotateToXY(const CagdCrvStruct *Crv)
{
    IrtHmgnMatType Mat;

    if (CagdCrvRotateToXYMat(Crv, Mat)) {
	return CagdCrvMatTransform(Crv, Mat);
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the given control points have poles - that is have both  M
* negative and positive weights.					     N
*                                                                            *
* PARAMETERS:                                                                M
*   Points:      Control points to consider.                                 M
*   Len:         Number of points in Points.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if has poles, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdAllWeightsNegative, CagdAllWeightsSame                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPointsHasPoles                                                       M
*****************************************************************************/
CagdBType CagdPointsHasPoles(CagdRType * const *Points, int Len)
{
    int i;
    const CagdRType
	*Weights = Points[0];
    CagdBType
	Positive = FALSE,
	Negative = FALSE;

    if (Weights == NULL)
	return FALSE;

    for (i = 0; i < Len; i++, Weights++) {
	if (*Weights >= 0)
	    Positive = TRUE;
	if (*Weights <= 0)
	    Negative = TRUE;
    }

    return Positive & Negative;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if the given control points has negative weights.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:      Control points to consider and possibly modify in place.    M
*   PType:	 Input point type, as given in Points.			     M
*   Len:         Number of points in Points.                                 M
*   Flip:        If TRUE, flips all weights (and points coefficients) so we  M
*		 end up with positive weights only.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if original has negative weights, FALSE otherwise.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPointsHasPoles, CagdAllWeightsSame                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdAllWeightsNegative                                                   M
*****************************************************************************/
CagdBType CagdAllWeightsNegative(CagdRType * const *Points,
				 CagdPointType PType,
				 int Len,
				 CagdBType Flip)
{
    int i, j;
    CagdRType
	*Weights = Points[0];
    CagdBType
	Positive = FALSE,
	Negative = FALSE;

    if (Weights == NULL)
	return FALSE;

    for (i = 0; i < Len; i++, Weights++) {
	if (*Weights > 0)
	    Positive = TRUE;
	if (*Weights < 0)
	    Negative = TRUE;
    }

    if (!Negative)
	return FALSE;

    if (Flip && !Positive && Negative) {
	for (j = 0; j <= CAGD_NUM_OF_PT_COORD(PType); j++) {
	    CagdRType
		*R = Points[j];

	    for (i = 0; i < Len; i++, R++)
		*R = -*R;
	}
    }

    return Negative;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if given control points has idential weights throughout.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:      Control points to consider.				     M
*   Len:         Number of points in Points.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if all weights are the same, FALSE otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdPointsHasPoles, CagdAllWeightsNegative                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdAllWeightsSame                                                       M
*****************************************************************************/
CagdBType CagdAllWeightsSame(CagdRType * const *Points, int Len)
{
    int i;
    CagdRType W0;
    CagdRType const
	*Weights = Points[0];

    if (Weights == NULL)
	return TRUE;

    W0 = *Weights++;
    for (i = 1; i < Len; i++, Weights++) {
        if (!IRIT_APX_EQ(W0, *Weights))
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the polygon approximation error function.  The error function       M
* will return a negative value if this triangle must be purged or otherwise  M
* a non negative error measure.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPlgErrorFuncType:  Old value of function.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPolygonSetErrFunc                                                    M
*****************************************************************************/
CagdPlgErrorFuncType CagdPolygonSetErrFunc(CagdPlgErrorFuncType Func)
{
    CagdPlgErrorFuncType
	OldFunc = PolygonErrFunc;

    PolygonErrFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the call back function to generate triangles.  The function will be M
* invoked with each triangle in the polygonal approximation.                 M
*   Default call back function used is CagdMakeTriangle.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfMakeTriFuncType:  Old value of function.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfSetMakeRectFunc, CagdSrf2Polygons, CagdSrfAdap2Polygons,          M
*   CagdMakeTriangle, CagdSrfSetMakeOnlyTri				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfSetMakeTriFunc                                                    M
*****************************************************************************/
CagdSrfMakeTriFuncType CagdSrfSetMakeTriFunc(CagdSrfMakeTriFuncType Func)
{
    CagdSrfMakeTriFuncType
	OldFunc = _CagdSrfMakeTriFunc;

    _CagdSrfMakeTriFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the call back function to generate rectangles.  The function will   M
* be invoked with each rectangle in the polygonal approximation.             M
*   Default call back function used is CagdMakeRectangle.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfMakeRectFuncType:  Old value of function.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfSetMakeTriFunc, CagdSrf2Polygons, CagdSrfAdap2Polygons,           M
*   CagdMakeRectangle, CagdSrfSetMakeOnlyTri				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfSetMakeRectFunc                                                   M
*****************************************************************************/
CagdSrfMakeRectFuncType CagdSrfSetMakeRectFunc(CagdSrfMakeRectFuncType Func)
{
    CagdSrfMakeRectFuncType
	OldFunc = _CagdSrfMakeRectFunc;

    _CagdSrfMakeRectFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets a flag to control if only triangles are to be generated from the    M
* tesselation code.  If TRUE only triangular polygons will be in the output  M
* set									     M
*                                                                            *
* PARAMETERS:                                                                M
*   OnlyTri:     TRUE for triangles only, FALSE otherwise.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  Old value of flag.		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfSetMakeRectFunc, CagdSrf2Polygons, CagdSrfAdap2Polygons,          M
*   CagdMakeTriangle, CagdSrfSetMakeTriFunc				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfSetMakeOnlyTri                                                    M
*****************************************************************************/
CagdBType CagdSrfSetMakeOnlyTri(CagdBType OnlyTri)
{
    CagdBType
	OldOnlyTri = _CagdSrfMakeOnlyTri;

    _CagdSrfMakeOnlyTri = OnlyTri;

    return OldOnlyTri;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create one triangular polygon, given its vertices.		     M
* Returns NULL if Triangle is degenerated.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ComputeNormals:   If TRUE then use Nl? parameters. Nl? are valid.        M
*   ComputeUV:        If TRUE then use UV? parameters. UV? are valid.        M
*   Pt1, Pt2, Pt3:    Euclidean locations of vertices.                       M
*   Nl1, Nl2, Nl3:    Optional Normals of vertices (if ComputeNormals).      M
*   UV1, UV2, UV3:    Optional UV parametric location of vertices (if        M
*                     ComputeUV).                                            M
*   GenPoly:          Returns TRUE if a polygon was generated, FALSE         M
*		      otherwise.  Note this function can return NULL and     M
*		      still generate a polygon as a call back for            M
*		      CagdSrf2Polygons.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A polygonal triangle structure, or NULL if points  M
*                         are collinear.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMakeRectangle, CagdSrf2Polygons, CagdSrfAdap2Polygons	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMakeTriangle                                                         M
*****************************************************************************/
CagdPolygonStruct *CagdMakeTriangle(CagdBType ComputeNormals,
				    CagdBType ComputeUV,
				    const CagdRType *Pt1,
				    const CagdRType *Pt2,
				    const CagdRType *Pt3,
				    const CagdRType *Nl1,
				    const CagdRType *Nl2,
				    const CagdRType *Nl3,
				    const CagdRType *UV1,
				    const CagdRType *UV2,
				    const CagdRType *UV3,
				    CagdBType *GenPoly)
{
    CagdPolygonStruct *Poly;

    if (GMCollinear3Pts(Pt1, Pt2, Pt3) ||
	(PolygonErrFunc != NULL &&
	 PolygonErrFunc(Pt1, Pt2, Pt3) < 0.0)) {
        *GenPoly = FALSE;
	return NULL;
    }
    else
        *GenPoly = TRUE;

    Poly = CagdPolygonNew(3);

    IRIT_PT_COPY(Poly -> U.Polygon[0].Pt, Pt1);
    IRIT_PT_COPY(Poly -> U.Polygon[1].Pt, Pt2);
    IRIT_PT_COPY(Poly -> U.Polygon[2].Pt, Pt3);

    if (ComputeNormals) {
	CagdVType Nrml;

	if (Nl1 == NULL || Nl2 == NULL || Nl3 == NULL) {
	    CagdVType V1, V2;

	    /* Estimate a normal from the triangle itself. */
	    IRIT_VEC_SUB(V1, Pt1, Pt2);
	    IRIT_VEC_SUB(V2, Pt2, Pt3);
	    IRIT_CROSS_PROD(Nrml, V1, V2);
	    IRIT_VEC_NORMALIZE(Nrml);

	    if (Nl1 == NULL)
		Nl1 = Nrml;
	    if (Nl2 == NULL)
		Nl2 = Nrml;
	    if (Nl3 == NULL)
		Nl3 = Nrml;
	}

	IRIT_PT_COPY(Poly -> U.Polygon[0].Nrml, Nl1);
	IRIT_PT_COPY(Poly -> U.Polygon[1].Nrml, Nl2);
	IRIT_PT_COPY(Poly -> U.Polygon[2].Nrml, Nl3);
    }
    else {
	IRIT_VEC_RESET(Poly -> U.Polygon[0].Nrml);
	IRIT_VEC_RESET(Poly -> U.Polygon[1].Nrml);
	IRIT_VEC_RESET(Poly -> U.Polygon[2].Nrml);
    }

    if (ComputeUV) {
	IRIT_UV_COPY(Poly -> U.Polygon[0].UV, UV1);
	IRIT_UV_COPY(Poly -> U.Polygon[1].UV, UV2);
	IRIT_UV_COPY(Poly -> U.Polygon[2].UV, UV3);
    }
    else {
	IRIT_UV_RESET(Poly -> U.Polygon[0].UV);
	IRIT_UV_RESET(Poly -> U.Polygon[1].UV);
	IRIT_UV_RESET(Poly -> U.Polygon[2].UV);
    }

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create one triangular polygon, given its vertices.		     M
* Returns NULL if Triangle is degenerated.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ComputeNormals:      If TRUE then use Nl? parameters. Nl? are valid.     M
*   ComputeUV:           If TRUE then use UV? parameters. UV? are valid.     M
*   Pt1, Pt2, Pt3, Pt4:  Euclidean locations of vertices.                    M
*   Nl1, Nl2, Nl3, Nl4:  Optional Normals of vertices (if ComputeNormals).   M
*   UV1, UV2, UV3, UV4:  Optional UV parametric location of vertices (if     M
*                        ComputeUV).                                         M
*   GenPoly:          Returns TRUE if a polygon was generated, FALSE         M
*		      otherwise.  Note this function can return NULL and     M
*		      still generate a polygon as a call back for            M
*		      CagdSrf2Polygons.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A polygonal rectangle structure, or NULL if points M
*                         are collinear.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMakeTriangle, CagdSrf2Polygons, CagdSrfAdap2Polygons	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMakeRectangle                                                        M
*****************************************************************************/
CagdPolygonStruct *CagdMakeRectangle(CagdBType ComputeNormals,
				     CagdBType ComputeUV,
				     const CagdRType *Pt1,
				     const CagdRType *Pt2,
				     const CagdRType *Pt3,
				     const CagdRType *Pt4,
				     const CagdRType *Nl1,
				     const CagdRType *Nl2,
				     const CagdRType *Nl3,
				     const CagdRType *Nl4,
				     const CagdRType *UV1,
				     const CagdRType *UV2,
				     const CagdRType *UV3,
				     const CagdRType *UV4,
				     CagdBType *GenPoly)
{
    CagdPolygonStruct *Poly;

    if (GMCollinear3Pts(Pt1, Pt2, Pt3) ||
	_CagdSrfMakeOnlyTri ||
	(PolygonErrFunc != NULL &&
	 PolygonErrFunc(Pt1, Pt2, Pt3) < 0.0)) {
        *GenPoly = FALSE;
	return NULL;
    }
    else
        *GenPoly = TRUE;

    Poly = CagdPolygonNew(4);

    IRIT_PT_COPY(Poly -> U.Polygon[0].Pt, Pt1);
    IRIT_PT_COPY(Poly -> U.Polygon[1].Pt, Pt2);
    IRIT_PT_COPY(Poly -> U.Polygon[2].Pt, Pt3);
    IRIT_PT_COPY(Poly -> U.Polygon[3].Pt, Pt4);

    if (ComputeNormals) {

	CagdVType Nrml;
	if (Nl1 == NULL || Nl2 == NULL || Nl3 == NULL || Nl4 == NULL) {
	    CagdVType V1, V2;

	    /* Estimate a normal from the polygon itself. */
	    IRIT_VEC_SUB(V1, Pt1, Pt2);
	    IRIT_VEC_SUB(V2, Pt2, Pt3);
	    IRIT_CROSS_PROD(Nrml, V1, V2);
	    IRIT_VEC_NORMALIZE(Nrml);

	    if (Nl1 == NULL)
		Nl1 = Nrml;
	    if (Nl2 == NULL)
		Nl2 = Nrml;
	    if (Nl3 == NULL)
		Nl3 = Nrml;
	    if (Nl4 == NULL)
		Nl4 = Nrml;
	}

	IRIT_PT_COPY(Poly -> U.Polygon[0].Nrml, Nl1);
	IRIT_PT_COPY(Poly -> U.Polygon[1].Nrml, Nl2);
	IRIT_PT_COPY(Poly -> U.Polygon[2].Nrml, Nl3);
	IRIT_PT_COPY(Poly -> U.Polygon[3].Nrml, Nl4);
    }
    else {
	IRIT_VEC_RESET(Poly -> U.Polygon[0].Nrml);
	IRIT_VEC_RESET(Poly -> U.Polygon[1].Nrml);
	IRIT_VEC_RESET(Poly -> U.Polygon[2].Nrml);
	IRIT_VEC_RESET(Poly -> U.Polygon[3].Nrml);
    }

    if (ComputeUV) {
	IRIT_UV_COPY(Poly -> U.Polygon[0].UV, UV1);
	IRIT_UV_COPY(Poly -> U.Polygon[1].UV, UV2);
	IRIT_UV_COPY(Poly -> U.Polygon[2].UV, UV3);
	IRIT_UV_COPY(Poly -> U.Polygon[3].UV, UV4);
    }
    else {
	IRIT_UV_RESET(Poly -> U.Polygon[0].UV);
	IRIT_UV_RESET(Poly -> U.Polygon[1].UV);
	IRIT_UV_RESET(Poly -> U.Polygon[2].UV);
	IRIT_UV_RESET(Poly -> U.Polygon[3].UV);
    }

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the way (co)linear surfaces are converted into polygons.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Lin2Poly:  Specification.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSetLinear2Poly, polygonization, polygonal approximation              M
*****************************************************************************/
void CagdSetLinear2Poly(CagdLin2PolyType Lin2Poly)
{
    _CagdLin2Poly = Lin2Poly;
}
