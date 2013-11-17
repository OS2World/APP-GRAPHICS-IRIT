/******************************************************************************
* Bbox.c - computes bounding boxes for objects.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 1993.					      *
******************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "geom_loc.h"

#define RESET_BBOX(Bbox) { \
		      Bbox.Min[0] = Bbox.Min[1] = Bbox.Min[2] = IRIT_INFNTY; \
		      Bbox.Max[0] = Bbox.Max[1] = Bbox.Max[2] = -IRIT_INFNTY; }

#define UNIT_BBOX(Bbox) { \
		      Bbox.Min[0] = Bbox.Min[1] = Bbox.Min[2] = 0.0; \
		      Bbox.Max[0] = Bbox.Max[1] = Bbox.Max[2] = 1.0; }

#define SET_IF_LESS_THAN(Val, NewVal)     { if (NewVal < Val) Val = NewVal; }
#define SET_IF_GREATER_THAN(Val, NewVal)  { if (NewVal > Val) Val = NewVal; }
#define SET_PT_IF_LESS_THAN(Pt, NewPt)    { SET_IF_LESS_THAN(Pt[0], NewPt[0]) \
					    SET_IF_LESS_THAN(Pt[1], NewPt[1]) \
					    SET_IF_LESS_THAN(Pt[2], NewPt[2]) }
#define SET_PT_IF_GREATER_THAN(Pt, NewPt) \
				       { SET_IF_GREATER_THAN(Pt[0], NewPt[0]) \
					 SET_IF_GREATER_THAN(Pt[1], NewPt[1]) \
					 SET_IF_GREATER_THAN(Pt[2], NewPt[2]) }
IRIT_STATIC_DATA int
    GlblBBoxInvisibleData = FALSE;
IRIT_STATIC_DATA const IPObjectStruct
    *GlblBBObjList = NULL;
IRIT_STATIC_DATA GMBBBboxStruct
    GlblBbox;

static void GMBBComputeBboxObjectAux(IPObjectStruct *CPObj,
				     IrtHmgnMatType Mat);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls whether or not to include invisible geometry in the bbox        M
* computations.  Invisible geometry is tagged using "invisible" attribs.     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBoxInvisibles:  TRUE to include invisible geometry in bbox computation. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old state of invisible geometry's bbox computation.            M
*                                                                            *
* SEE ALSO:                                                                  M
*    GMBBComputeBboxObject                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBSetBBoxInvisibles                                                    M
*****************************************************************************/
int GMBBSetBBoxInvisibles(int BBoxInvisibles)
{
    int OldVal = GlblBBoxInvisibleData;

    GlblBBoxInvisibleData = BBoxInvisibles;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box of a given object of any type.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To compute a bounding box for.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:   A pointer to a statically allocated bounding box     M
*                       holding bounding box information on PObj.            M
*                                                                            *
* SEE ALSO:                                                                  M
*    GMBBSetBBoxInvisibles                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBComputeBboxObject, bounding box                                      M
*****************************************************************************/
GMBBBboxStruct *GMBBComputeBboxObject(const IPObjectStruct *PObj)
{
    int UpdatedGlblBBObjList = FALSE,
	OldTraverseCopyObjState = IPTraverseObjectCopy(TRUE);
    IrtHmgnMatType Mat;
    const IPObjectStruct *OldVal;

    if (GlblBBObjList == NULL) {
	OldVal = GMBBSetGlblBBObjList(PObj);
	UpdatedGlblBBObjList = TRUE;	
    }

    MatGenUnitMat(Mat);

    RESET_BBOX(GlblBbox);

    /* Const-safe as GMBBComputeBboxObjectAux is used here. */
    IPTraverseObjHierarchy((IPObjectStruct *) PObj,
			   (IPObjectStruct *) GlblBBObjList,
			   GMBBComputeBboxObjectAux, Mat, FALSE);

    if (UpdatedGlblBBObjList)
	GMBBSetGlblBBObjList(OldVal);

    IPTraverseObjectCopy(OldTraverseCopyObjState);

    return &GlblBbox;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Auxiliary function of GMBBComputeBboxObject.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   CPObj:     To compute a bounding box for.                                *
*   Mat:       Transformation matrix to apply to PObj.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void GMBBComputeBboxObjectAux(IPObjectStruct *CPObj,
				     IrtHmgnMatType Mat)
{
    IRIT_STATIC_DATA GMBBBboxStruct Bbox;
    IPObjectStruct *PObjTmp;
    GMBBBboxStruct *PBbox2;
    CagdBBoxStruct CagdBbox;
    MvarBBoxStruct MvarBbox;
    CagdRType *R;
    CagdPType Pt;
    IPObjectStruct *PObj;

    RESET_BBOX(Bbox);

    PObj = GMTransformObject(CPObj, Mat);

    switch (PObj -> ObjType) {
	case IP_OBJ_POLY:
	    Bbox = *GMBBComputePolyListBbox(PObj -> U.Pl);
	    break;
	case IP_OBJ_CTLPT:
	    R = PObj -> U.CtlPt.Coords;
	    CagdCoercePointTo(Pt, CAGD_PT_E3_TYPE,
			      &R, -1, PObj -> U.CtlPt.PtType);
	    Bbox = *GMBBComputePointBbox(Pt);
	    break;
	case IP_OBJ_POINT:
	    Bbox = *GMBBComputePointBbox(PObj -> U.Pt);
	    break;
	case IP_OBJ_VECTOR:
	    Bbox = *GMBBComputePointBbox(PObj -> U.Vec);
	    break;
	case IP_OBJ_STRING:
	    if ((PObjTmp = AttrGetObjectObjAttrib(PObj, "_geometry")) != NULL)
	        GMBBComputeBboxObjectAux(PObjTmp, FALSE);
	    break;
	case IP_OBJ_CURVE:
	    CagdCrvListBBox(PObj -> U.Crvs, &CagdBbox);
	    IRIT_GEN_COPY(Bbox.Min, CagdBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, CagdBbox.Max, 3 * sizeof(IrtRType));
	    break;
	case IP_OBJ_SURFACE:
	    CagdSrfListBBox(PObj -> U.Srfs, &CagdBbox);
	    IRIT_GEN_COPY(Bbox.Min, CagdBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, CagdBbox.Max, 3 * sizeof(IrtRType));	
	    break;
	case IP_OBJ_TRIMSRF:
	    CagdSrfListBBox(PObj -> U.TrimSrfs -> Srf, &CagdBbox);
	    IRIT_GEN_COPY(Bbox.Min, CagdBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, CagdBbox.Max, 3 * sizeof(IrtRType));	
	    break;
	case IP_OBJ_TRIVAR:
	    TrivTVListBBox(PObj -> U.Trivars, &CagdBbox);
	    IRIT_GEN_COPY(Bbox.Min, CagdBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, CagdBbox.Max, 3 * sizeof(IrtRType));	
	    break;
	case IP_OBJ_TRISRF:
	    TrngTriSrfListBBox(PObj -> U.TriSrfs, &CagdBbox);
	    IRIT_GEN_COPY(Bbox.Min, CagdBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, CagdBbox.Max, 3 * sizeof(IrtRType));	
	    break;
	case IP_OBJ_MODEL:
	    MdlModelListBBox(PObj -> U.Mdls, &CagdBbox);
	    IRIT_GEN_COPY(Bbox.Min, CagdBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, CagdBbox.Max, 3 * sizeof(IrtRType));	
	    break;
	case IP_OBJ_MULTIVAR:
	    MvarMVListBBox(PObj -> U.MultiVars, &MvarBbox);
	    IRIT_GEN_COPY(Bbox.Min, MvarBbox.Min, 3 * sizeof(IrtRType));
	    IRIT_GEN_COPY(Bbox.Max, MvarBbox.Max, 3 * sizeof(IrtRType));	
	    break;
	default:
	    break;
    }

    PBbox2 = GMBBMergeBbox(&GlblBbox, &Bbox);
    IRIT_GEN_COPY(&GlblBbox, PBbox2, sizeof(GMBBBboxStruct));

    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box of a list of objects of any type.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To compute a bounding box for.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:   A pointer to a statically allocated bounding box     M
*                       holding bounding box information on objects PObj.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBComputeBboxObjectList, bounding box                                  M
*****************************************************************************/
GMBBBboxStruct *GMBBComputeBboxObjectList(const IPObjectStruct *PObj)
{
    IRIT_STATIC_DATA GMBBBboxStruct Bbox;
    GMBBBboxStruct *PBbox;
    const IPObjectStruct
	*OldVal = GMBBSetGlblBBObjList(PObj);

    RESET_BBOX(Bbox);
    PBbox = &Bbox;

    for ( ; PObj != NULL; PObj = PObj -> Pnext) {
	PBbox = GMBBMergeBbox(PBbox, GMBBComputeBboxObject(PObj));
	IRIT_GEN_COPY(&Bbox, PBbox, sizeof(GMBBBboxStruct));
	PBbox = &Bbox;
    }

    GMBBSetGlblBBObjList(OldVal);

    return PBbox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the global list object to search instances at.      		     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBObjList:      Global object list to search instances at.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   const IPObjectStruct *:   Old global object list.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBSetGlblBBObjList, bounding box      	                             M
*****************************************************************************/
const IPObjectStruct *GMBBSetGlblBBObjList(const IPObjectStruct *BBObjList)
{
    const IPObjectStruct
	*OldVal = GlblBBObjList;

    GlblBBObjList = BBObjList;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box of a polygon/polyline/pointlist object.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:     To compute a bounding box for.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:   A pointer to a statically allocated bounding box     M
*                       holding bounding box information on PPoly.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBComputeOnePolyBbox, bounding box                                     M
*****************************************************************************/
GMBBBboxStruct *GMBBComputeOnePolyBbox(const IPPolygonStruct *PPoly)
{
    IRIT_STATIC_DATA GMBBBboxStruct Bbox;
    const IPVertexStruct
	*V = PPoly -> PVertex;

    RESET_BBOX(Bbox);

    do {
	SET_PT_IF_LESS_THAN(Bbox.Min, V -> Coord);
	SET_PT_IF_GREATER_THAN(Bbox.Max, V -> Coord);
	V = V -> Pnext;
    }
    while (V != NULL && V != PPoly -> PVertex);

    return &Bbox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of polygon/polyline/pointlist objects.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PPoly:     To compute a bounding box for.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:   A pointer to a statically allocated bounding box     M
*                       holding bounding box information on PPoly list.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBComputePolyListBbox, bounding box                                    M
*****************************************************************************/
GMBBBboxStruct *GMBBComputePolyListBbox(const IPPolygonStruct *PPoly)
{
    IRIT_STATIC_DATA GMBBBboxStruct Bbox;

    RESET_BBOX(Bbox);

    for ( ; PPoly != NULL; PPoly = PPoly -> Pnext) {
	const IPVertexStruct
	    *V = PPoly -> PVertex;

	if (V != NULL) {
	    do {
	        SET_PT_IF_LESS_THAN(Bbox.Min, V -> Coord);
		SET_PT_IF_GREATER_THAN(Bbox.Max, V -> Coord);
		V = V -> Pnext;
	    }
	    while (V != NULL && V != PPoly -> PVertex);
	}
    }

    return &Bbox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box of a point object.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:     To compute a bounding box for.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:   A pointer to a statically allocated bounding box     M
*                       holding bounding box information on Pt.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBComputePointBbox, bounding box                                       M
*****************************************************************************/
GMBBBboxStruct *GMBBComputePointBbox(const IrtRType *Pt)
{
    IRIT_STATIC_DATA GMBBBboxStruct Bbox;

    IRIT_PT_COPY(Bbox.Min, Pt);
    IRIT_PT_COPY(Bbox.Max, Pt);

    return &Bbox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges (union) given two bounding boxes into one.			     M
* Either Bbox1 or Bbox2 can be pointing to the static area Bbox used herein. M
*                                                                            *
* PARAMETERS:                                                                M
*   Bbox1:      First bounding box to union up.                              M
*   Bbox2:      Second bounding box to union up.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMBBBboxStruct *:    A unioned bounding box the contains both BBox1 and  M
*                        BBox2.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMBBMergeBbox, bounding box                                              M
*****************************************************************************/
GMBBBboxStruct *GMBBMergeBbox(const GMBBBboxStruct *Bbox1,
			      const GMBBBboxStruct *Bbox2)
{
    IRIT_STATIC_DATA GMBBBboxStruct Bbox;
    int i;

    /* Make sure first Bbox is in Bbox and Bbox2 holds the other one. */
    if (Bbox2 == &Bbox) {
	Bbox2 = Bbox1;
    }
    else {
	IRIT_GEN_COPY(&Bbox, Bbox1, sizeof(GMBBBboxStruct));
    }

    /* Compare the two Bbox's and update. */
    for (i = 0; i < 3; i++) {
	if (Bbox.Min[i] > Bbox2 -> Min[i])
	    Bbox.Min[i] = Bbox2 -> Min[i];
	if (Bbox.Max[i] < Bbox2 -> Max[i])
	    Bbox.Max[i] = Bbox2 -> Max[i];
    }

    return &Bbox;
}
