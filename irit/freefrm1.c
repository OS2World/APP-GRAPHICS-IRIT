/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to provide the required interfact for the cagd library for the    *
* free form surfaces and curves.					     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "allocate.h"
#include "attribut.h"
#include "objects.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "freeform.h"

#define KNOT_VEC_TOL	1e-4

static IPObjectStruct *GetControlMesh(IPObjectStruct *LstObjList,
				      int UOrder,
				      int VOrder,
				      CagdGeomType GType,
				      char **ErrStr);
static IPObjectStruct *GetControlPoly(IPObjectStruct *PtObjList,
				      int Order,
				      CagdGeomType GType,
				      char **ErrStr);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object DrawCtlPt.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Value of DrawCtlPt object.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetDrawCtlPt                                                             M
*****************************************************************************/
int GetDrawCtlPt(void)
{
    int DrawCtlPt;
    IPObjectStruct
	*PObj = IritDBGetObjByName("DRAWCTLPT");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name DRAWCTLPT is defined");
	DrawCtlPt = 0;
    }
    else
	DrawCtlPt = IRIT_REAL_TO_INT(PObj -> U.R);

    return DrawCtlPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object FourPerFlat.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Value of Flat4Ply object.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetFourPerFlat                                                           M
*****************************************************************************/
int GetFourPerFlat(void)
{
    int FourPerFlat;
    IPObjectStruct
	*PObj = IritDBGetObjByName("FLAT4PLY");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name FLAT4PLY is defined");
	FourPerFlat = 0;
    }
    else
	FourPerFlat = IRIT_REAL_TO_INT(PObj -> U.R);

    return FourPerFlat;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object Poly_Approx_opt.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Value of Poly_Approx_opt object.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetPolyApproxOptimal                                                     M
*****************************************************************************/
int GetPolyApproxOptimal(void)
{
    int PolyApproxOpt;
    IPObjectStruct
	*PObj = IritDBGetObjByName("POLY_APPROX_OPT");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name POLY_APPROX_OPT is defined");
	PolyApproxOpt = 0;
    }
    else
	PolyApproxOpt = IRIT_REAL_TO_INT(PObj -> U.R);

    return PolyApproxOpt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object Poly_Approx_Uv.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Value of Poly_Approx_Uv object.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetPolyApproxUV                                                          M
*****************************************************************************/
int GetPolyApproxUV(void)
{
    int PolyApproxUV;
    IPObjectStruct
	*PObj = IritDBGetObjByName("POLY_APPROX_UV");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name POLY_APPROX_UV is defined");
	PolyApproxUV = 0;
    }
    else
	PolyApproxUV = IRIT_REAL_TO_INT(PObj -> U.R);

    return PolyApproxUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object Poly_Approx_Tri.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Value of Poly_Approx_Uv object.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetPolyApproxTri                                                         M
*****************************************************************************/
int GetPolyApproxTri(void)
{
    int PolyApproxTri;
    IPObjectStruct
	*PObj = IritDBGetObjByName("POLY_APPROX_TRI");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name POLY_APPROX_TRI is defined");
	PolyApproxTri = 0;
    }
    else
	PolyApproxTri = IRIT_REAL_TO_INT(PObj -> U.R);

    return PolyApproxTri;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object Poly_Approx_Tol.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:        Value of Poly_Approx_Tol object.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetPolyApproxTol                                                         M
*****************************************************************************/
IrtRType GetPolyApproxTol(void)
{
    IrtRType PolyApproxTol;
    IPObjectStruct
	*PObj = IritDBGetObjByName("POLY_APPROX_TOL");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name POLY_APPROX_TOL is defined");
	PolyApproxTol = 0.1;
    }
    else
	PolyApproxTol = PObj -> U.R;

    return PolyApproxTol;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to fetch the boolean object Poly_Merge_Coplanar.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Value of Poly_Merge_Coplanar object.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetPolyMergeCoplanar	                                             M
*****************************************************************************/
int GetPolyMergeCoplanar(void)
{
    int PolyMergeCoplanar;
    IPObjectStruct
	*PObj = IritDBGetObjByName("POLY_MERGE_COPLANAR");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name POLY_MERGE_COPLANAR is defined");
	PolyMergeCoplanar = 0;
    }
    else
	PolyMergeCoplanar = IRIT_REAL_TO_INT(PObj -> U.R);

    return PolyMergeCoplanar;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy the control mesh lists to a surface control mesh.          *
*   The surface is allocated here as well.				     *
*   Returns the surface if o.k., otherwise NULL.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   LstObjList: A list object of lists of control points.                    *
*   UOrder:     U order of surface.                                          *
*   VOrder:     V order of surface.                                          *
*   GType:      Geometry type - Bezier, Bspline etc.                         *
*   ErrStr:     If an error, detected, this is initialized with description. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A surface object if successful, NULL otherwise.      *
*****************************************************************************/
static IPObjectStruct *GetControlMesh(IPObjectStruct *LstObjList,
				      int UOrder,
				      int VOrder,
				      CagdGeomType GType,
				      char **ErrStr)
{
    int i, j, k, PtSize,
	NumVertices = 0,
	NumVerticesFirst = -1,
        NumLists = 0;
    CagdRType **r;
    IrtRType *v;
    IPObjectStruct *SrfObj, *LstObj, *PtObj;
    CagdPointType
	PtType = CAGD_PT_E1_TYPE;

    if (!IP_IS_OLST_OBJ(LstObjList))
	IRIT_FATAL_ERROR("SURFACE: Not object list object!");

    while ((LstObj = IPListObjectGet(LstObjList, NumLists)) != NULL) {
	if (!IP_IS_OLST_OBJ(LstObj)) {
	    *ErrStr = IRIT_EXP_STR("Non list object found in list");
	    return NULL;
	}

        NumVertices = -1;
        while ((PtObj = IPListObjectGet(LstObj, ++NumVertices)) != NULL) {
	    if (!IP_IS_CTLPT_OBJ(PtObj) &&
		!IP_IS_POINT_OBJ(PtObj) &&
		!IP_IS_VEC_OBJ(PtObj)) {
		*ErrStr = IRIT_EXP_STR("Non point object found in list");
		return NULL;
	    }
	}
	if ((PtType = IPCoerceCommonSpace(LstObj, PtType)) == CAGD_PT_NONE) {
	    *ErrStr = "";
	    return NULL;
	}

	if (NumLists++ == 0)
	    NumVerticesFirst = NumVertices;
        else if (NumVerticesFirst != NumVertices) {
	    *ErrStr = IRIT_EXP_STR("Different size of point lists");
	    return NULL;
	}
    }

    /* Coerce all points to a common space, in place. */
    for (NumLists = 0;
	 (LstObj = IPListObjectGet(LstObjList, NumLists)) != NULL;
	 NumLists++)
	if (IPCoercePtsListTo(LstObj, PtType) == CAGD_PT_NONE) {
	    *ErrStr = "";
	    return NULL;
	}

    SrfObj = IPGenSRFObject(NULL);
    switch (GType) {
	case CAGD_SPOWER_TYPE:
	    SrfObj -> U.Srfs = CagdSrfNew(CAGD_SPOWER_TYPE, PtType,
					  NumVertices, NumLists);
	    SrfObj -> U.Srfs -> UOrder = SrfObj -> U.Srfs -> ULength;
	    SrfObj -> U.Srfs -> VOrder = SrfObj -> U.Srfs -> VLength;
	    break;
	case CAGD_SBEZIER_TYPE:
	    SrfObj -> U.Srfs = BzrSrfNew(NumVertices, NumLists, PtType);
	    SrfObj -> U.Srfs -> UOrder = SrfObj -> U.Srfs -> ULength;
	    SrfObj -> U.Srfs -> VOrder = SrfObj -> U.Srfs -> VLength;
	    break;
	case CAGD_SBSPLINE_TYPE:
	    SrfObj -> U.Srfs = BspSrfNew(NumVertices, NumLists,
					 UOrder, VOrder, PtType);
	    break;
	default:
	    break;
    }
    PtSize = CAGD_IS_RATIONAL_PT(PtType) + CAGD_NUM_OF_PT_COORD(PtType);

    for (r = SrfObj -> U.Srfs -> Points, i = 0; i < NumLists; i++) {
	LstObj = IPListObjectGet(LstObjList, i);

        for (j = 0; j < NumVertices; j++) {
	    IPObjectStruct
		*VObj = IPListObjectGet(LstObj, j);

	    v = VObj -> U.CtlPt.Coords;

	    if (CAGD_IS_RATIONAL_PT(PtType))
		for (k = 0; k < PtSize; k++)
		    r[k][i * NumVertices + j] = *v++;
	    else
		for (k = 1; k <= PtSize; k++)
		    r[k][i * NumVertices + j] = *++v;
        }
    }

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy the control polygon to a curve's control polygon.          *
*   The curve is allocated here as well.				     *
*   Returns the surface if o.k., otherwise NULL.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtObjList:  A list object of control points.   	  	             *
*   Order:      Order of curve.	                                	     *
*   GType:      Geometry type - Bezier, Bspline etc.                         *
*   ErrStr:     If an error, detected, this is initialized with description. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A curve object if successful, NULL otherwise.        *
*****************************************************************************/
static IPObjectStruct *GetControlPoly(IPObjectStruct *PtObjList,
				      int Order,
				      CagdGeomType GType,
				      char **ErrStr)
{
    int i, j, PtSize,
        NumVertices = -1;
    CagdRType **r;
    IrtRType *v;
    IPObjectStruct *CrvObj, *PtObj;
    CagdPointType PtType;

    *ErrStr = NULL;

    if (!IP_IS_OLST_OBJ(PtObjList))
	IRIT_FATAL_ERROR("CURVE: Not object list object!");

    while ((PtObj = IPListObjectGet(PtObjList, ++NumVertices)) != NULL) {
	if (!IP_IS_CTLPT_OBJ(PtObj) &&
	    !IP_IS_POINT_OBJ(PtObj) &&
	    !IP_IS_VEC_OBJ(PtObj)) {
	    *ErrStr = IRIT_EXP_STR("Non point object found in list");
	    return NULL;
	}
    }
	
    /* Coerce all points to a common space, in place. */
    if ((PtType = IPCoercePtsListTo(PtObjList, CAGD_PT_E1_TYPE))
							== CAGD_PT_NONE) {
	*ErrStr = "";
	return NULL;
    }

    CrvObj = IPAllocObject("", IP_OBJ_CURVE, NULL);
    switch (GType) {
	case CAGD_CPOWER_TYPE:
	    CrvObj -> U.Crvs = CagdCrvNew(CAGD_CPOWER_TYPE, PtType,
					  NumVertices);
	    CrvObj -> U.Crvs -> Order = CrvObj -> U.Crvs -> Length;
	    break;
	case CAGD_CBEZIER_TYPE:
	    CrvObj -> U.Crvs = BzrCrvNew(NumVertices, PtType);
	    CrvObj -> U.Crvs -> Order = CrvObj -> U.Crvs -> Length;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    CrvObj -> U.Crvs = BspCrvNew(NumVertices, Order, PtType);
	    break;
	default:
	    break;
    }
    PtSize = CAGD_IS_RATIONAL_PT(PtType) + CAGD_NUM_OF_PT_COORD(PtType);

    for (r = CrvObj -> U.Crvs -> Points, i = 0; i < NumVertices; i++) {
	IPObjectStruct
	    *VObj = IPListObjectGet(PtObjList, i);

	v = VObj -> U.CtlPt.Coords;

	if (CAGD_IS_RATIONAL_PT(PtType))
	    for (j = 0; j < PtSize; j++)
		r[j][i] = *v++;
	else
	    for (j = 1; j <= PtSize; j++)
		r[j][i] = *++v;
    }

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy the list of knots into the knot vector provided.           *
*   Returns KnotVector if o.k., NULL otherwise (sets ErrStr to description). *
*   Length can hold the requested length (in ctlpts) in case a uniform KV is *
* requested. Length will return the actual length of the KV constructed.     *
*                                                                            *
* PARAMETERS:                                                                *
*   KntObjList: A list of knots (numeric values).                            *
*   Order:      Order of geometry to use this knot vector.                   *
*   Length:     Expected length of knot vector.                              *
*   ErrStr:     If an error, detected, this is initialized with description. *
*   Validate:   TRUE, if needs to validate the knot sequence.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   Allocated knot vector, or NULL if error.                  *
*****************************************************************************/
CagdRType *GetKnotVector(IPObjectStruct *KntObjList,
			 int Order,
			 int *Length,
			 char **ErrStr,
			 int Validate)
{
    int NumKnots = 0,
	RequestLength = *Length;
    CagdRType *KnotVector;
    IPObjectStruct *KntObj;

    *ErrStr = NULL;

    if (!IP_IS_OLST_OBJ(KntObjList))
	IRIT_FATAL_ERROR("KNOT: Not object list object!");

    *Length = IPListObjectLength(KntObjList);
    KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * *Length);

    while ((KntObj = IPListObjectGet(KntObjList, NumKnots)) != NULL &&
	   NumKnots < *Length) {
	if (!IP_IS_NUM_OBJ(KntObj)) {
	    *ErrStr = IRIT_EXP_STR("Non numeric object found in list");
	    return NULL;
	}

	KnotVector[NumKnots++] = KntObj -> U.R;
    }

    if (NumKnots == 1 && KnotVector[0] < KV_MIN_LEGAL) {
	int KVType = IRIT_REAL_TO_INT(KnotVector[0]);

	IritFree(KnotVector);

	*Length = RequestLength + Order;

	switch (KVType) {
	    case KV_UNIFORM_OPEN:
		KnotVector = BspKnotUniformOpen(RequestLength, Order, NULL);
		break;
	    case KV_UNIFORM_DISCONT_OPEN:
		KnotVector = BspKnotDiscontUniformOpen(RequestLength, Order,
						       NULL);
		break;
	    case KV_UNIFORM_FLOAT:
		KnotVector = BspKnotUniformFloat(RequestLength, Order, NULL);
		break;
	    case KV_UNIFORM_PERIODIC:
		KnotVector = BspKnotUniformPeriodic(RequestLength + Order - 1,
						    Order, NULL);
		*Length += Order - 1;
		break;
	    default:
		*ErrStr = IRIT_EXP_STR("Invalid knot value");
		IRIT_WARNING_MSG_PRINTF("Knot = %10.6g (%d)",
			                KnotVector[0], KVType);
		return NULL;
	}
    }
    else if (NumKnots != *Length) {
	*ErrStr = IRIT_EXP_STR("Wrong knot vector length");
	return NULL;
    }

    if (Validate)
        BspKnotVerifyKVValidity(KnotVector, Order, *Length - Order,
				KNOT_VEC_TOL);

    return KnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier surface geometric object defined by a list of M
* lists of vertices.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   LstObjList: A list object of lists of control points.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier surface object if successful, NULL otherwise. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenPowerSurfaceObject                                                    M
*****************************************************************************/
IPObjectStruct *GenPowerSurfaceObject(IPObjectStruct *LstObjList)
{
    char *ErrStr;
    IPObjectStruct
	*SrfObj = GetControlMesh(LstObjList, -1, -1, CAGD_SPOWER_TYPE, &ErrStr);

    if (SrfObj == NULL) {
	IRIT_NON_FATAL_ERROR2("SPOWER: %s, empty object result.\n", ErrStr);
    }

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Power curve geometric object defined by a list of    M
* vertices.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:  A list object of control points.   	  	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Power curve object if successful, NULL otherwise.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenPowerCurveObject                                                      M
*****************************************************************************/
IPObjectStruct *GenPowerCurveObject(IPObjectStruct *PtObjList)
{
    char *ErrStr;
    IPObjectStruct
	*CrvObj = GetControlPoly(PtObjList, -1, CAGD_CPOWER_TYPE, &ErrStr);

    if (CrvObj == NULL) {
	IRIT_NON_FATAL_ERROR2("CPOWER: %s\n, empty object result.", ErrStr);
    }

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier surface geometric object defined by a list of M
* lists of vertices.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   LstObjList: A list object of lists of control points.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier surface object if successful, NULL otherwise. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBezierSurfaceObject                                                   M
*****************************************************************************/
IPObjectStruct *GenBezierSurfaceObject(IPObjectStruct *LstObjList)
{
    char *ErrStr;
    IPObjectStruct
	*SrfObj = GetControlMesh(LstObjList, -1, -1, CAGD_SBEZIER_TYPE, &ErrStr);

    if (SrfObj == NULL) {
	IRIT_NON_FATAL_ERROR2("SBEZIER: %s, empty object result.\n", ErrStr);
    }

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier curve geometric object defined by a list of   M
* vertices.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:  A list object of control points.   	  	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier curve object if successful, NULL otherwise.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBezierCurveObject                                                     M
*****************************************************************************/
IPObjectStruct *GenBezierCurveObject(IPObjectStruct *PtObjList)
{
    char *ErrStr;
    IPObjectStruct
	*CrvObj = GetControlPoly(PtObjList, -1, CAGD_CBEZIER_TYPE, &ErrStr);

    if (CrvObj == NULL) {
	IRIT_NON_FATAL_ERROR2("CBEZIER: %s\n, empty object result.", ErrStr);
    }

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bspline surface geometric object defined by a list   M
* of lists of vertices.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   RUOrder:     U order of surface.                                         M
*   RVOrder:     V order of surface.                                         M
*   LstObjList:  A list object of lists of control points.                   M
*   KntObjList:  A list of knots (numeric values).                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bspline surface object if successful, NULL otherwise.M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBsplineSurfaceObject                                                  M
*****************************************************************************/
IPObjectStruct *GenBsplineSurfaceObject(IrtRType *RUOrder,
					IrtRType *RVOrder,
					IPObjectStruct *LstObjList,
					IPObjectStruct *KntObjList)
{
    int Len1, Len2,
	UOrder = IRIT_REAL_PTR_TO_INT(RUOrder),
	VOrder = IRIT_REAL_PTR_TO_INT(RVOrder);
    char *ErrStr;
    IPObjectStruct
	*SrfObj = GetControlMesh(LstObjList, UOrder, VOrder,
					  CAGD_SBSPLINE_TYPE, &ErrStr);

    if (SrfObj == NULL) {
	IRIT_NON_FATAL_ERROR2("SBSPLINE: Ctl mesh, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (!IP_IS_OLST_OBJ(KntObjList) || IPListObjectLength(KntObjList) != 2) {
	IPFreeObject(SrfObj);
	IRIT_NON_FATAL_ERROR("SBSPLINE: Exactly two knot vectors expected");
	return NULL;
    }

    if (SrfObj -> U.Srfs -> ULength < SrfObj -> U.Srfs -> UOrder ||
	SrfObj -> U.Srfs -> VLength < SrfObj -> U.Srfs -> VOrder) {
	IPFreeObject(SrfObj);
	IRIT_NON_FATAL_ERROR("SBSPLINE: Surface mesh length smaller than order.");
	return NULL;
    }

    IritFree(SrfObj -> U.Srfs -> UKnotVector);
    SrfObj -> U.Srfs -> UKnotVector = NULL;
    IritFree(SrfObj -> U.Srfs -> VKnotVector);
    SrfObj -> U.Srfs -> VKnotVector = NULL;
    Len1 = SrfObj -> U.Srfs -> ULength;
    Len2 = SrfObj -> U.Srfs -> VLength;
    if ((SrfObj -> U.Srfs -> UKnotVector =
	 GetKnotVector(IPListObjectGet(KntObjList, 0), UOrder,
		       &Len1, &ErrStr, TRUE)) == NULL ||
	(SrfObj -> U.Srfs -> VKnotVector =
	 GetKnotVector(IPListObjectGet(KntObjList, 1), VOrder,
		       &Len2, &ErrStr, TRUE)) == NULL) {
	IPFreeObject(SrfObj);
	IRIT_NON_FATAL_ERROR2("SBSPLINE: Knot vectors, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (Len1 != SrfObj -> U.Srfs -> ULength + UOrder) {
	if (Len1 == SrfObj -> U.Srfs -> ULength + UOrder + UOrder - 1)
	    SrfObj -> U.Srfs -> UPeriodic = TRUE;
	else {
	    IPFreeObject(SrfObj);
	    IRIT_NON_FATAL_ERROR("Wrong knot vector length");
	    return NULL;
	}
    }
    if (Len2 != SrfObj -> U.Srfs -> VLength + VOrder) {
	if (Len2 == SrfObj -> U.Srfs -> VLength + VOrder + VOrder - 1)
	    SrfObj -> U.Srfs -> VPeriodic = TRUE;
	else {
	    IPFreeObject(SrfObj);
	    IRIT_NON_FATAL_ERROR("Wrong knot vector length");
	    return NULL;
	}
    }

    return SrfObj;

}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bspline curve geometric object defined by a list     M
* of vertices.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ROrder:      Order of surface.                                           M
*   PtObjList:   A list object of control points.   	  	             M
*   KntObjList:  A list of knots (numeric values).                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bspline curve object if successful, NULL otherwise.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBsplineCurveObject                                                    M
*****************************************************************************/
IPObjectStruct *GenBsplineCurveObject(IrtRType *ROrder,
				      IPObjectStruct *PtObjList,
				      IPObjectStruct *KntObjList)
{
    int Len,
	Order = IRIT_REAL_PTR_TO_INT(ROrder);
    char *ErrStr;
    IPObjectStruct
	*CrvObj = GetControlPoly(PtObjList, Order, CAGD_CBSPLINE_TYPE, &ErrStr);

    if (CrvObj == NULL) {
	IRIT_NON_FATAL_ERROR2("CBSPLINE: Ctl polygon, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (!IP_IS_OLST_OBJ(KntObjList)) {
	IPFreeObject( CrvObj);
	IRIT_NON_FATAL_ERROR("CBSPLINE: Exactly one knot vector expected");
	return NULL;
    }

    if (CrvObj -> U.Crvs -> Length < CrvObj -> U.Crvs -> Order) {
	IPFreeObject(CrvObj);
	IRIT_NON_FATAL_ERROR("CBSPLINE: Curve polygon length smaller than order.");
	return NULL;
    }

    IritFree(CrvObj -> U.Crvs -> KnotVector);
    CrvObj -> U.Crvs -> KnotVector = NULL;
    Len = CrvObj -> U.Crvs -> Length;
    if ((CrvObj -> U.Crvs -> KnotVector =
	 GetKnotVector(KntObjList, Order, &Len, &ErrStr, TRUE)) == NULL) {
	IPFreeObject(CrvObj);
	IRIT_NON_FATAL_ERROR2("CBSPLINE: Knot vector, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (Len != CrvObj -> U.Crvs -> Length + Order) {
	if (Len == CrvObj -> U.Crvs -> Length + Order + Order - 1)
	    CrvObj -> U.Crvs -> Periodic = TRUE;
	else {
	    IPFreeObject(CrvObj);
	    IRIT_NON_FATAL_ERROR("CBSPLINE: Wrong knot vector length");
	    return NULL;
	}
    }

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to approximate a surface of revolution by rotating the given       M
* cross section along Z axes, as a cubic POLYNOMIAL surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:    A curve torotate around the Zaxis.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A surface of revolution by rotating Cross around Z.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSURFPREVObject                                                        M
*****************************************************************************/
IPObjectStruct *GenSURFPREVObject(IPObjectStruct *Cross)
{
    if (IP_IS_CRV_OBJ(Cross)) {
	if (CAGD_NUM_OF_PT_COORD(Cross -> U.Crvs -> PType) < 3) {
	    IRIT_NON_FATAL_ERROR("SurfRev: cross-section perpendicular to Z. Empty object result");
	    return NULL;
	}

	return IPGenSRFObject(CagdSurfaceRevPolynomialApprox(Cross -> U.Crvs));
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to subdivide a surface into two in specified direction (1 or 2)    M
* and specified parameter value.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:       Surface (possibly a trimmed surface) to subdivide.         M
*   RDir:         Direction of subdivision. Either U or V.                   M
*   ParamVal:     Parameter value at which subdivision should occur.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object of two surface objects, result of the   M
*		       subdivision.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DivideSurfaceObject                                                      M
*****************************************************************************/
IPObjectStruct *DivideSurfaceObject(IPObjectStruct *SrfObj,
				    IrtRType *RDir,
				    IrtRType *ParamVal)
{
    int Dir = IRIT_REAL_PTR_TO_INT(RDir);
    IPObjectStruct *SrfList,
	*Srf1 = NULL,
	*Srf2 = NULL;

    if (IP_IS_SRF_OBJ(SrfObj)) {
	CagdSrfStruct *Srf;

        if ((Srf = CagdSrfSubdivAtParam(SrfObj -> U.Srfs, *ParamVal,
					(CagdSrfDirType) Dir)) == NULL)
	    return NULL;

	Srf1 = IPGenSRFObject(Srf);
	IP_ATTR_SAFECOPY_ATTRS(Srf1 -> Attr, SrfObj -> Attr);
	Srf2 = IPGenSRFObject(Srf -> Pnext);
	IP_ATTR_SAFECOPY_ATTRS(Srf2 -> Attr, SrfObj -> Attr);

	Srf -> Pnext = NULL;
    }
    else if (IP_IS_TRIMSRF_OBJ(SrfObj)) {
	TrimSrfStruct *Srf;

        if ((Srf = TrimSrfSubdivAtParam(SrfObj -> U.TrimSrfs, *ParamVal,
					(CagdSrfDirType) Dir)) == NULL)
	    return NULL;

	Srf1 = IPGenTRIMSRFObject(Srf);
	IP_ATTR_SAFECOPY_ATTRS(Srf1 -> Attr, SrfObj -> Attr);

	if (Srf -> Pnext != NULL) {
	    Srf2 = IPGenTRIMSRFObject(Srf -> Pnext);
	    IP_ATTR_SAFECOPY_ATTRS(Srf2 -> Attr, SrfObj -> Attr);

	    Srf -> Pnext = NULL;
	}
	else
	    Srf2 = NULL;
    }

    SrfList = IPGenLISTObject(Srf1);
    IPListObjectInsert(SrfList, 1, Srf2);
    IPListObjectInsert(SrfList, 2, NULL);

    return SrfList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract a surface region in specified direction (1 or 2) and    M
* specified parameter values.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:      Surface to extract a region from.                           M
*   RDir:        Direction of region extraction. Either U or V.              M
*   ParamVal1:   Parameter of beginning of region.                           M
*   ParamVal2:   Parameter of end of region.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A region of SrfObj,                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   RegionFromSurfaceObject                                                  M
*****************************************************************************/
IPObjectStruct *RegionFromSurfaceObject(IPObjectStruct *SrfObj, 
					IrtRType *RDir,
					IrtRType *ParamVal1,
					IrtRType *ParamVal2)
{
    int Dir = IRIT_REAL_PTR_TO_INT(RDir);

    if (IP_IS_SRF_OBJ(SrfObj)) {
        CagdSrfStruct
	    *Srf = CagdSrfRegionFromSrf(SrfObj -> U.Srfs,
					*ParamVal1, *ParamVal2,
					(CagdSrfDirType) Dir);

	if (Srf == NULL)
	    return NULL;

	SrfObj = IPGenSRFObject(Srf);
    }
    else if (IP_IS_TRIMSRF_OBJ(SrfObj)) {
	TrimSrfStruct
	    *TSrf = TrimSrfRegionFromTrimSrf(SrfObj -> U.TrimSrfs,
					*ParamVal1, *ParamVal2,
					(CagdSrfDirType) Dir);

	if (TSrf == NULL)
	    return NULL;

	SrfObj = IPGenTRIMSRFObject(TSrf);
    }

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to subdivide a curve into two in specified parameter value.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:       Curve to subdivide.                                        M
*   ParamVal:     Parameter value at which subdivision should occur.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object of two curve objects, result of the     M
*		       subdivision.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DivideCurveObject                                                        M
*****************************************************************************/
IPObjectStruct *DivideCurveObject(IPObjectStruct *CrvObj, IrtRType *ParamVal)
{
    CagdCrvStruct
	*Crv = CagdCrvSubdivAtParam(CrvObj -> U.Crvs, *ParamVal);
    IPObjectStruct *Crv1, *Crv2, *CrvList;

    if (Crv == NULL)
	return NULL;

    Crv1 = IPGenCRVObject(Crv);
    IP_ATTR_SAFECOPY_ATTRS(Crv1 -> Attr, CrvObj -> Attr);

    Crv2 = IPGenCRVObject(Crv -> Pnext);
    IP_ATTR_SAFECOPY_ATTRS(Crv2 -> Attr, CrvObj -> Attr);

    Crv -> Pnext = NULL;

    CrvList = IPGenLISTObject(Crv1);
    IPListObjectInsert(CrvList, 1, Crv2);
    IPListObjectInsert(CrvList, 2, NULL);

    return CrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract a curve region in specified parameter values.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:      Curve to extract a region from.                             M
*   ParamVal1:   Parameter of beginning of region.                           M
*   ParamVal2:   Parameter of end of region.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A region of CrvObj,                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   RegionFromCurveObject                                                    M
*****************************************************************************/
IPObjectStruct *RegionFromCurveObject(IPObjectStruct *CrvObj,
				      IrtRType *ParamVal1,
				      IrtRType *ParamVal2)
{
    CagdCrvStruct
	*Crv = CagdCrvRegionFromCrv(CrvObj -> U.Crvs, *ParamVal1, *ParamVal2);

    if (Crv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to refine a surface in specified direction (1 or 2) and knot       M
* vector.								     M
*   If, however, Replace is non zero, KnotsObj REPLACES current vector.      M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Surface to refine in direction RDir.                         M
*   RDir:       Direction of refinement. Either U or V,                      M
*   RReplace:   If TRUE KnotsObj will replace the RDir knot vector of SrfObj.M
*		Otherwise, the knots in KnotsObj will be added to it.	     M
*   KnotsObj:   A list of knots.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A refined surface, or a surface with a replaced knot M
*                       vector.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   RefineSurfaceObject                                                      M
*****************************************************************************/
IPObjectStruct *RefineSurfaceObject(IPObjectStruct *SrfObj,
				    IrtRType *RDir,
				    IrtRType *RReplace,
				    IPObjectStruct *KnotsObj)
{
    int n,
	Replace = IRIT_REAL_PTR_TO_INT(RReplace),
	Dir = IRIT_REAL_PTR_TO_INT(RDir);
    char *ErrStr;
    CagdRType
	*t = GetKnotVector(KnotsObj, 0, &n, &ErrStr, FALSE);
    CagdSrfStruct *RefSrf;
    IPObjectStruct *RefSrfObj;

    if (t == NULL) {
	IPFreeObject(SrfObj);
	IRIT_NON_FATAL_ERROR2("REFINE: %s, empty object result.\n", ErrStr);
	return NULL;
    }
    RefSrf = CagdSrfRefineAtParams(SrfObj -> U.Srfs, (CagdSrfDirType) Dir,
				   Replace, t, n);
    IritFree(t);
    if (RefSrf == NULL)
	return NULL;

    RefSrfObj = IPGenSRFObject(RefSrf);
    IP_ATTR_SAFECOPY_ATTRS(RefSrfObj -> Attr, SrfObj -> Attr);

    return RefSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to refine a curve.						     M
*   If, however, Replace is non zero, KnotsObj REPLACES current vector.      M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:     Curve to refine.                     			     M
*   RReplace:   If TRUE KnotsObj will replace the knot vector of CrvObj.     M
*		Otherwise, the knots in KnotsObj will be added to it.	     M
*   KnotsObj:   A list of knots.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A refined curve, or a curve with a replaced knot     M
*                       vector.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   RefineCurveObject                                                        M
*****************************************************************************/
IPObjectStruct *RefineCurveObject(IPObjectStruct *CrvObj,
				  IrtRType *RReplace,
				  IPObjectStruct *KnotsObj)
{
    int n,
	Replace = IRIT_REAL_PTR_TO_INT(RReplace);
    char *ErrStr;
    CagdRType
	*t = GetKnotVector(KnotsObj, 0, &n, &ErrStr, FALSE);
    CagdCrvStruct *RefCrv;
    IPObjectStruct *RefCrvObj;

    if (t == NULL) {
	IPFreeObject(CrvObj);
	IRIT_NON_FATAL_ERROR2("REFINE: %s, empty object result.\n", ErrStr);
	return NULL;
    }
    RefCrv = CagdCrvRefineAtParams(CrvObj -> U.Crvs, Replace, t, n);
    IritFree(t);
    if (RefCrv == NULL)
	return NULL;

    RefCrvObj = IPGenCRVObject(RefCrv);
    IP_ATTR_SAFECOPY_ATTRS(RefCrvObj -> Attr, RefCrv -> Attr);

    return RefCrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate surface in specified parameter values.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface (possibly trimmed) to evaluate at (u, v).             M
*   u, v:      Parameter values to evaluate at.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A control point of the same type SrfObj has.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalSurfaceObject                                                        M
*****************************************************************************/
IPObjectStruct *EvalSurfaceObject(IPObjectStruct *SrfObj,
				  IrtRType *u,
				  IrtRType *v)
{
    CagdRType *Pt;
    CagdSrfStruct *Srf;

    if (IP_IS_SRF_OBJ(SrfObj))
	Srf = SrfObj -> U.Srfs;
    else if (IP_IS_TRIMSRF_OBJ(SrfObj))
	Srf = SrfObj -> U.TrimSrfs -> Srf;
    else
	return NULL;

    Pt = CagdSrfEval(Srf, *u, *v);
    return IPGenCTLPTObject(Srf -> PType, Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate curve in specified parameter value.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Surface to evaluate at t.                                     M
*   t:         Parameter value to evaluate at.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A control point of the same type CrvObj has.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalCurveObject                                                          M
*****************************************************************************/
IPObjectStruct *EvalCurveObject(IPObjectStruct *CrvObj, IrtRType *t)
{
    CagdRType
	*Pt = CagdCrvEval(CrvObj -> U.Crvs, *t);
    IPObjectStruct
	*CtlPtObj = IPGenCTLPTObject(CrvObj -> U.Crvs -> PType, Pt);

    return CtlPtObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the derivative surface in Dir of SrfObj.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to differentiate.                                     M
*   Dir:       Direction of differentiation. Either U or V.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A differentiated surface.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   DeriveSurfaceObject                                                      M
*****************************************************************************/
IPObjectStruct *DeriveSurfaceObject(IPObjectStruct *SrfObj, IrtRType *Dir)
{
    CagdSrfStruct
	*DerivSrf = CagdSrfDerive(SrfObj -> U.Srfs,
				  (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir));
    IPObjectStruct
	*DerivSrfObj = IPGenSRFObject(DerivSrf);

    return DerivSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the integral surface of SrfObj.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to integrate.                                         M
*   Dir:       Direction of differentiation. Either U or V.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Integrated surface.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IntegrateSurfaceObject                                                   M
*****************************************************************************/
IPObjectStruct *IntegrateSurfaceObject(IPObjectStruct *SrfObj, IrtRType *Dir)
{
    CagdSrfStruct
	*IntgSrf = CagdSrfIntegrate(SrfObj -> U.Srfs,
				    (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir));
    IPObjectStruct
	*IntgSrfObj = IPGenSRFObject(IntgSrf);

    return IntgSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the derivative curve of CrvObj.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to differentiate.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A differentiated curve.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   DeriveCurveObject                                                        M
*****************************************************************************/
IPObjectStruct *DeriveCurveObject(IPObjectStruct *CrvObj)
{
    CagdCrvStruct
	*DerivCrv = CagdCrvDerive(CrvObj -> U.Crvs);
    IPObjectStruct
	*DerivCrvObj = IPGenCRVObject(DerivCrv);

    return DerivCrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the integral curve of CrvObj.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to integrate.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Integrated curve.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IntegrateCurveObject                                                     M
*****************************************************************************/
IPObjectStruct *IntegrateCurveObject(IPObjectStruct *CrvObj)
{
    CagdCrvStruct
	*IntgCrv = CagdCrvIntegrate(CrvObj -> U.Crvs);
    IPObjectStruct
	*IntgCrvObj = IPGenCRVObject(IntgCrv);

    return IntgCrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to Moebius transform a curve with a transformation factor of C.    M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to Moebius transform.                                   M
*   C:         The moebius transformation factor with C == 1 uniform.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Transformed curve.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MoebiusCurveTrans                                                        M
*****************************************************************************/
IPObjectStruct *MoebiusCurveTrans(IPObjectStruct *CrvObj, IrtRType *C)
{
    CagdCrvStruct *TransCrv;
    IPObjectStruct *TransCrvObj;

    TransCrv = CagdCrvMoebiusTransform(CrvObj -> U.Crvs, *C);

    TransCrvObj = IPGenCRVObject(TransCrv);

    return TransCrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to Moebius transform a surface with a transformation factor of C.  M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to Moebius transform.                                 M
*   C:         The moebius transformation factor with C == 1 uniform.        M
*   RDir:      Direction of tangent, Either U or V.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Transformed surface.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MoebiusSurfaceTrans                                                      M
*****************************************************************************/
IPObjectStruct *MoebiusSurfaceTrans(IPObjectStruct *SrfObj,
				    IrtRType *C,
				    IrtRType *RDir)
{
    CagdSrfDirType
	Dir = (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(RDir);
    CagdSrfStruct *TransSrf;
    IPObjectStruct *TransSrfObj;

    TransSrf = CagdSrfMoebiusTransform(SrfObj -> U.Srfs, *C, Dir);

    TransSrfObj = IPGenSRFObject(TransSrf);

    return TransSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the normal surface of SrfObj.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to compute a normal vector field for.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A vector field surface representing SrfObj's normal. M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceNormalObject                                                      M
*****************************************************************************/
IPObjectStruct *SurfaceNormalObject(IPObjectStruct *SrfObj)
{
    CagdSrfStruct
	*NormalSrf = SymbSrfNormalSrf(SrfObj -> U.Srfs);
    IPObjectStruct
	*NormalSrfObj = IPGenSRFObject(NormalSrf);

    return NormalSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the normal curve of CrvObj.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Surface to compute a normal vector field for.  The magnitude  M
*	       of this vector field equals the curvature.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A vector field surface representing CrvObj's normal. M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveNormalObject                                                        M
*****************************************************************************/
IPObjectStruct *CurveNormalObject(IPObjectStruct *CrvObj)
{
    CagdCrvStruct
	*NormalCrv = SymbCrv3DCurvatureNormal(CrvObj -> U.Crvs);
    IPObjectStruct
	*NormalCrvObj = IPGenCRVObject(NormalCrv);

    return NormalCrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate surface's normal in specified parameter values.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:   Surface (possibly trimmed) to evaluate its normal at (u, v).   M
*   u, v:     Parameters at which to evaluate SrfObj's normal.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A vector object representing the normal of SrfObj     M
*                      at (u, v).                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   NormalSurfaceObject                                                      M
*****************************************************************************/
IPObjectStruct *NormalSurfaceObject(IPObjectStruct *SrfObj,
				    IrtRType *u,
				    IrtRType *v)
{
    int i;
    IrtRType V[3];
    CagdVecStruct *Vec;
    IPObjectStruct *NormalObj;

    if (IP_IS_SRF_OBJ(SrfObj))
	Vec = CagdSrfNormal(SrfObj -> U.Srfs, *u, *v, TRUE);
    else if (IP_IS_TRIMSRF_OBJ(SrfObj))
	Vec = CagdSrfNormal(SrfObj -> U.TrimSrfs -> Srf, *u, *v, TRUE);
    else
        Vec = NULL;

    if (Vec == NULL) {
	IRIT_NON_FATAL_ERROR("SNORMAL: unable to compute normal to the surface");
	return NULL;
    }

    for (i = 0; i < 3; i++)
	V[i] = Vec -> Vec[i];

    NormalObj = IPGenVECObject(&V[0], &V[1], &V[2]);

    return NormalObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate curve's normal in specified parameter values.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:   Curve (possibly trimmed) to evaluate its normal at (u, v).     M
*   t:         Parameter to evaluate CrvObj at.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A vector object representing the normal of CrvObj     M
*                      at (u, v).                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   NormalCurveObject                                                        M
*****************************************************************************/
IPObjectStruct *NormalCurveObject(IPObjectStruct *CrvObj, IrtRType *t)
{
    int i;
    IrtRType V[3];
    CagdVecStruct
        *Vec = CagdCrvNormal(CrvObj -> U.Crvs, *t, TRUE);
    IPObjectStruct *NormalObj;

    if (Vec == NULL) {
	IRIT_NON_FATAL_ERROR("CNORMAL: unable to compute normal to the curve");
	return NULL;
    }

    for (i = 0; i < 3; i++)
	V[i] = Vec -> Vec[i];

    NormalObj = IPGenVECObject(&V[0], &V[1], &V[2]);

    return NormalObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate surface's tangent in specified parameter value and dir.M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:   Surface (possibly trimmed) to evaluate its tangent at (u, v).  M
*   RDir:     Direction of tangent, Either U or V.                           M
*   u, v:     Parameters at which to evaluate SrfObj's tangent.              M
*   Normalize: TRUE to normalize the vector, FALSE to return actual          M
*	      derivative magnitude.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A vector object representing the tangent of SrfObj   M
*                       at (u, v) in direction RDir.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TangentSurfaceObject                                                     M
*****************************************************************************/
IPObjectStruct *TangentSurfaceObject(IPObjectStruct *SrfObj,
				     IrtRType *RDir,
				     IrtRType *u,
				     IrtRType *v,
				     IrtRType *Normalize)
{
    int i,
	Dir = IRIT_REAL_PTR_TO_INT(RDir);
    IrtRType V[3];
    CagdVecStruct *Vec;
    IPObjectStruct *TangentObj;

    if (IP_IS_SRF_OBJ(SrfObj))
	Vec = CagdSrfTangent(SrfObj -> U.Srfs, *u, *v,
			     (CagdSrfDirType) Dir, IRIT_REAL_PTR_TO_INT(Normalize));
    else if (IP_IS_TRIMSRF_OBJ(SrfObj))
	Vec = CagdSrfTangent(SrfObj -> U.TrimSrfs -> Srf, *u, *v,
			     (CagdSrfDirType) Dir, IRIT_REAL_PTR_TO_INT(Normalize));
    else
	Vec = NULL;

    if (Vec == NULL) {
	IRIT_NON_FATAL_ERROR("STANGENT: unable to compute tangent to the surface");
	return NULL;
    }

    for (i = 0; i < 3; i++)
	V[i] = Vec -> Vec[i];

    TangentObj = IPGenVECObject(&V[0], &V[1], &V[2]);

    return TangentObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to evaluate curve's tangent in specified parameter value.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to evaluate its tangent at t.                           M
*   t:         Parameter to evaluate CrvObj at.                              M
*   Normalize: TRUE to normalize the vector, FALSE to return actual          M
*	       derivative magnitude.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A vector object representing the tangent of CrvObj.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TangentCurveObject                                                       M
*****************************************************************************/
IPObjectStruct *TangentCurveObject(IPObjectStruct *CrvObj,
				   IrtRType *t,
				   IrtRType *Normalize)
{
    int i;
    IrtRType V[3];
    CagdVecStruct
	*Vec = CagdCrvTangent(CrvObj -> U.Crvs, *t,
			      IRIT_REAL_PTR_TO_INT(Normalize));
    IPObjectStruct *TangentObj;

    if (Vec == NULL) {
	IRIT_NON_FATAL_ERROR("CTANGENT: unable to compute tangent to the curve");
	return NULL;
    }

    for (i = 0; i < 3; i++)
	V[i] = Vec -> Vec[i];

    TangentObj = IPGenVECObject(&V[0], &V[1], &V[2]);

    return TangentObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract an isoparametric curve out of a (triangular) surface.   M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Tensor product or triangular surface to extract an	     M
*		isoparametric curve from.			             M
*   RDir:       Direction of extraction. Either U or V (or W for triangular).M
*   ParamVal:   Parameter value of isoparametric curve.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A curve object which is an isoparametric curve of     M
*                      SrfObj.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveFromSurface                                                         M
*****************************************************************************/
IPObjectStruct *CurveFromSurface(IPObjectStruct *SrfObj,
				 IrtRType *RDir,
				 IrtRType *ParamVal)
{
    int Dir = IRIT_REAL_PTR_TO_INT(RDir);
    CagdCrvStruct *Crv;
    IPObjectStruct *CrvObj;

    if (IP_IS_SRF_OBJ(SrfObj)) {
	Crv = CagdCrvFromSrf(SrfObj -> U.Srfs, *ParamVal,
			     (CagdSrfDirType) Dir);
    }
    else if (IP_IS_TRISRF_OBJ(SrfObj)) {
	Crv = TrngCrvFromTriSrf(SrfObj -> U.TriSrfs, *ParamVal,
				(TrngTriSrfDirType) Dir);
    }
    else {
	IRIT_FATAL_ERROR("CSURFACE: Undefined surface type to process.\n");
	return NULL;
    }

    if (Crv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract an isoparametric curve out of a surface mesh.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Surface to extract an isoparametric curve from itsmesh.      M
*   RDir:       Direction of extraction. Either U or V.                      M
*   RIndex:     Index into SrfObj's mesh.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A curve object which is not necessarily in SrfObj.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveFromSrfMesh                                                         M
*****************************************************************************/
IPObjectStruct *CurveFromSrfMesh(IPObjectStruct *SrfObj,
				 IrtRType *RDir,
				 IrtRType *RIndex)
{
    int Dir = IRIT_REAL_PTR_TO_INT(RDir),
	Index = IRIT_REAL_PTR_TO_INT(RIndex);
    CagdCrvStruct
	*Crv = CagdCrvFromMesh(SrfObj -> U.Srfs, Index, (CagdSrfDirType) Dir);
    IPObjectStruct *CrvObj;

    if (Crv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extract the geometry of the control mesh/polygon of the given freeform.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A freeform geometry to extract its control mesh/polygon.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A polyline object representing the control mesh.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MeshGeometry2Polylines                                                   M
*****************************************************************************/
IPObjectStruct *MeshGeometry2Polylines(IPObjectStruct *PObj)
{
    CagdPolylineStruct
	*CagdMesh = NULL;

    switch (PObj -> ObjType) {
	case IP_OBJ_CURVE:
	    CagdMesh = CagdCrv2CtrlPoly(PObj -> U.Crvs);
	    break;
	case IP_OBJ_SURFACE:
	    CagdMesh = CagdSrf2CtrlMesh(PObj -> U.Srfs);
	    break;
        case IP_OBJ_TRIMSRF:
	    CagdMesh = CagdSrf2CtrlMesh(PObj -> U.TrimSrfs -> Srf);
	    break;
	case IP_OBJ_TRIVAR:
	    CagdMesh = TrivTV2CtrlMesh(PObj -> U.Trivars);
	    break;
	case IP_OBJ_TRISRF:
	    CagdMesh = TrngTriSrf2CtrlMesh(PObj -> U.TriSrfs);
	    break;
        default:
	    assert(0);
    }

    if (CagdMesh != NULL) {
	IPPolygonStruct
	    *IritMesh = IPCagdPllns2IritPllns(CagdMesh);

	return IPGenPOLYLINEObject(IritMesh);
    }
    else {
	IRIT_NON_FATAL_ERROR("FFMESH: unable to compute control mesh for given object");
	return NULL;
    }
}


