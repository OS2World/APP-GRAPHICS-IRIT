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
#include "geom_lib.h"
#include "allocate.h"
#include "attribut.h"
#include "objects.h"
#include "ip_cnvrt.h"
#include "user_lib.h"
#include "freeform.h"

#define IRIT_MSC_DEF_SUBDIV_EPS	0.05
#define IRIT_MSC_DEF_NUMER_EPS	1e-6
#define IRIT_MSS_DEF_SUBDIV_EPS	0.5
#define IRIT_MSS_DEF_NUMER_EPS	1e-6
#define IRIT_CONTOUR_EPS	1e-10

IRIT_STATIC_DATA int
    GlblCntrFFValidate = FALSE;
IRIT_STATIC_DATA IrtRType
    GlblCntrFFValidateCosAngle = -1.0;
IRIT_STATIC_DATA IrtVecType
    GlblCntrFFValidateDir = { 0.0, 0.0, 1.0 };

static IPPolygonStruct *CnvrtCntrsToPolygons(IPObjectStruct *Cntrs);
static IPObjectStruct *GetControlTVMesh(IPObjectStruct *LstLstObjList,
					int UOrder,
					int VOrder,
					int WOrder,
					TrivGeomType GType,
					char **ErrStr);
static int ContourFreeformValidateCntrPt(const CagdSrfStruct *Srf,
					 IrtRType u,
					 IrtRType v);
static IPObjectStruct *KnotVectorToListObj(CagdRType *KV, int KVLen);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a trimmed surface, given the parametric surface and its	     M
* trimming curves.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:         To construct the trimmed surface with.                   M
*   TrimmedCrvsObj: To trim srf with. No real validity testing is performed. M
*		    Can be NULL. Create a structure of trimmed surface,      M
*		    untrimmed.						     M
*   RHasTopLvlTrim: FALSE if needs to add a top level rectangular boundary.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A trimmed surface object.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTrimmedSurface                                                        M
*****************************************************************************/
IPObjectStruct *GenTrimmedSurface(IPObjectStruct *SrfObj,
				  IPObjectStruct *TrimmedCrvsObj,
				  IrtRType *RHasTopLvlTrim)
{
    CagdCrvStruct
	*Crvs = NULL;
    CagdBType
	HasTopLvlTrim = IRIT_REAL_PTR_TO_INT(RHasTopLvlTrim);

    if (IP_IS_CRV_OBJ(TrimmedCrvsObj)) {
	Crvs = CagdCrvCopyList(TrimmedCrvsObj -> U.Crvs);
    }
    else if (IP_IS_OLST_OBJ(TrimmedCrvsObj)) {
	int NumCrvs = 0;
	IPObjectStruct *CrvObj;

	while ((CrvObj = IPListObjectGet(TrimmedCrvsObj, NumCrvs++)) != NULL) {
	    CagdCrvStruct *Crv;

	    if (!IP_IS_CRV_OBJ(CrvObj)) {
		IRIT_NON_FATAL_ERROR("Non curve object as a trimmed curve");
		return NULL;
	    }

	    Crv = CagdCrvCopy(CrvObj -> U.Crvs);
	    IRIT_LIST_PUSH(Crv, Crvs);
	}
    }

    return IPGenTRIMSRFObject(TrimSrfNew2(CagdSrfCopy(SrfObj -> U.Srfs),
					Crvs, HasTopLvlTrim));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a set of trimmed surfaces, given (a list) of curves or	     M
* polylines in the parametric space of SrfObj.				     M
*									     *
* PARAMETERS:                                                                M
*   SrfObj:      To construct the trimmed surfaces from.                     M
*   Cntrs:       A polygon/polyline/curve or a list of those.  Each such     M
*		 entity must be either closed or start and end on the        M
*		 boundary of the surface.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of trimmed surfaces object.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTrimmedSurfaces2                                                      M
*****************************************************************************/
IPObjectStruct *GenTrimmedSurfaces2(IPObjectStruct *SrfObj,
				    IPObjectStruct *Cntrs)
{
    IPPolygonStruct
	*PolyCntrs = CnvrtCntrsToPolygons(Cntrs);
    TrimSrfStruct
	*TSrfs = TrimSrfsFromContours(SrfObj -> U.Srfs, PolyCntrs);

    IPFreePolygonList(PolyCntrs);

    if (TSrfs == NULL)
	return NULL;
    else if (TSrfs -> Pnext == NULL)
        return IPGenTRIMSRFObject(TSrfs);
    else {
        IPObjectStruct
	    *ListObj = IPGenLISTObject(NULL);
	int Count = 0;

        while (TSrfs != NULL) {
	    TrimSrfStruct
		*TSrf = TSrfs;

	    TSrfs = TSrfs -> Pnext;
	    TSrf -> Pnext = NULL;
	    IPListObjectInsert(ListObj, Count++, IPGenTRIMSRFObject(TSrf));
	}

	IPListObjectInsert(ListObj, Count, NULL);
	return ListObj;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert the given contours which can be a curve/polygon/polyline or a    *
* list of them.                                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Cntrs:     A curve/polygon/polyline or a list of them.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   A list of polyline/polygon.                         *
*****************************************************************************/
static IPPolygonStruct *CnvrtCntrsToPolygons(IPObjectStruct *Cntrs)
{
    if (IP_IS_POLY_OBJ(Cntrs)) {
        return IPCopyPolygonList(Cntrs -> U.Pl);
    }
    else if (IP_IS_CRV_OBJ(Cntrs)) {
        int Resolution = GetResolution(FALSE);
	IrtRType
	    RelResolution = AttrGetObjectRealAttrib(Cntrs, "resolution");

	if (Resolution < MIN_FREE_FORM_RES)
	    Resolution = MIN_FREE_FORM_RES;
	if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	    Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

        return IPCurve2Polylines(Cntrs -> U.Crvs, Resolution,
				 SYMB_CRV_APPROX_UNIFORM);
    }
    else if (IP_IS_OLST_OBJ(Cntrs)) {
	IPObjectStruct *PObj;
	int i = 0;
	IPPolygonStruct *Pl,
	    *Pls = NULL;

        while ((PObj = IPListObjectGet(Cntrs, i++)) != NULL) {
	    if ((Pl = CnvrtCntrsToPolygons(PObj)) != NULL) {
	        Pls = IPAppendPolyLists(Pl, Pls);
	    }
	}

	return Pls;
    }
    else {
	IRIT_NON_FATAL_ERROR("TRMSRFS: expected polylines/gons, curves, or a list of them\n");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts the parametric surface of a trimmed surface.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimmedSrfObj:   To extract parametric surface from.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Extracted parametric surface.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetSrfFromTrimmedSrf                                                     M
*****************************************************************************/
IPObjectStruct *GetSrfFromTrimmedSrf(IPObjectStruct *TrimmedSrfObj)
{
    return IPGenSRFObject(CagdSrfCopy(TrimmedSrfObj -> U.TrimSrfs -> Srf));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts trimming curves of trimming surface, either in parametric or    M
* Euclidean space.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimmedSrfObj:   To extract trimming curves from.                        M
*   RParamSpace:     TRUE for parametric space, FALSE for Euclidean space.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Extracted trimming curves.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetTrimCrvsFromTrimmedSrf                                                M
*****************************************************************************/
IPObjectStruct *GetTrimCrvsFromTrimmedSrf(IPObjectStruct *TrimmedSrfObj,
					  IrtRType *RParamSpace)
{
    int Count = 0,
	ParamSpace = IRIT_REAL_PTR_TO_INT(RParamSpace);
    CagdCrvStruct
	*Crvs = TrimGetTrimmingCurves(TrimmedSrfObj -> U.TrimSrfs,
				      ParamSpace, TRUE);
    IPObjectStruct
	*ListObj = IPGenLISTObject(NULL);

    while (Crvs) {
	CagdCrvStruct
	    *Crv = Crvs;

	Crvs = Crvs -> Pnext;
	Crv -> Pnext = NULL;
	IPListObjectInsert(ListObj, Count++, IPGenCRVObject(Crv));
    }

    IPListObjectInsert(ListObj, Count, NULL);
    return ListObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy the control mesh lists to a trivariate control mesh.       *
*   The trivariate is allocated here as well.				     *
*   Returns the trivariate if o.k., otherwise NULL.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   LstLstObjList:  A list object of lists of lists of control points.       *
*   UOrder:     U order of trivar.                                           *
*   VOrder:     V order of trivar.                                           *
*   WOrder:     V order of trivar.                                           *
*   GType:      Geometry type - Bezier, Bspline etc.                         *
*   ErrStr:     If an error, detected, this is initialized with description. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A trivar object if successful, NULL otherwise.       *
*****************************************************************************/
static IPObjectStruct *GetControlTVMesh(IPObjectStruct *LstLstObjList,
					int UOrder,
					int VOrder,
					int WOrder,
					TrivGeomType GType,
					char **ErrStr)
{
    int i, j, k, l, PtSize, NumPlane,
	NumVertices = 0,
	NumVerticesFirst = -1,
        NumLists = 0,
	NumListsFirst = -1,
        NumListLists = 0;
    CagdRType **r;
    IrtRType *v;
    IPObjectStruct *TrivarObj, *LstObjList, *ObjList, *PtObj;
    CagdPointType
	PtType = CAGD_PT_E1_TYPE;

    while ((LstObjList = IPListObjectGet(LstLstObjList,
					 NumListLists)) != NULL) {
	if (!IP_IS_OLST_OBJ(LstObjList)) {
	    *ErrStr = IRIT_EXP_STR("Non list object found in list");
	    return NULL;
	}

	while ((ObjList = IPListObjectGet(LstObjList, NumLists)) != NULL) {
	    if (!IP_IS_OLST_OBJ(ObjList)) {
		*ErrStr = IRIT_EXP_STR("Non list object found in list");
		return NULL;
	    }

	    NumVertices = -1;
	    while ((PtObj = IPListObjectGet(ObjList, ++NumVertices)) != NULL) {
		if (!IP_IS_CTLPT_OBJ(PtObj) &&
		    !IP_IS_POINT_OBJ(PtObj) &&
		    !IP_IS_VEC_OBJ(PtObj)) {
		    *ErrStr = IRIT_EXP_STR("Non point object found in list");
		    return NULL;
		}
	    }
	    if ((PtType = IPCoerceCommonSpace(ObjList, PtType)) ==
								CAGD_PT_NONE) {
		*ErrStr = "";
		return NULL;
	    }

	    if (NumLists++ == 0 && NumListLists == 0)
		NumVerticesFirst = NumVertices;
	    else if (NumVerticesFirst != NumVertices) {
		*ErrStr = IRIT_EXP_STR("Different size of point lists");
		return NULL;
	    }
	}

	if (NumListLists++ == 0)
	    NumListsFirst = NumLists;
	else if (NumListsFirst != NumLists) {
	    *ErrStr = IRIT_EXP_STR("Different size of list lists");
	    return NULL;
	}
    }

    /* Coerce all points to a common space, in place. */
    while ((LstObjList = IPListObjectGet(LstLstObjList, NumListLists)) != NULL)
	while ((ObjList = IPListObjectGet(LstObjList, NumLists)) != NULL)
	    if (IPCoercePtsListTo(ObjList, PtType) == CAGD_PT_NONE) {
		*ErrStr = "";
		return NULL;
	    }

    if (NumVertices < 2 || NumLists < 2 || NumListLists < 2) {
	*ErrStr = IRIT_EXP_STR("Less than 2 points in a row/col/depth");
	return NULL;
    }

    TrivarObj = IPGenTRIVARObject(NULL);
    switch (GType) {
	case TRIV_TVBEZIER_TYPE:
	    TrivarObj -> U.Trivars = TrivBzrTVNew(NumVertices, NumLists,
						  NumListLists, PtType);
	    break;
	case TRIV_TVBSPLINE_TYPE:
	    TrivarObj -> U.Trivars = TrivBspTVNew(NumVertices, NumLists,
						  NumListLists, UOrder,
						  VOrder, WOrder, PtType);
	    break;
	default:
	    break;
    }
    PtSize = CAGD_IS_RATIONAL_PT(PtType) + CAGD_NUM_OF_PT_COORD(PtType);

    NumPlane = NumVertices * NumLists;
    for (r = TrivarObj -> U.Trivars -> Points, i = 0; i < NumListLists; i++) {
	LstObjList = IPListObjectGet(LstLstObjList, i);

        for (j = 0; j < NumLists; j++) {
	    ObjList = IPListObjectGet(LstObjList, j);

	    for (k = 0; k < NumVertices; k++) {
		IPObjectStruct
		    *VObj = IPListObjectGet(ObjList, k);

		v = VObj -> U.CtlPt.Coords;

		if (CAGD_IS_RATIONAL_PT(PtType))
		    for (l = 0; l < PtSize; l++)
			r[l][i * NumPlane + j * NumVertices + k] = *v++;
		else
		    for (l = 1; l <= PtSize; l++)
			r[l][i * NumPlane + j * NumVertices + k] = *++v;
	    }
	}
    }

    return TrivarObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier trivar geometric object defined by a list of  M
* lists of lists of control points.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   LstLstObjList: A list object of lists of lists of control points.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier trivar object if successful, NULL otherwise.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBezierTrivarObject                                                    M
*****************************************************************************/
IPObjectStruct *GenBezierTrivarObject(IPObjectStruct *LstLstObjList)
{
    char *ErrStr;
    IPObjectStruct
	*TrivObj = GetControlTVMesh(LstLstObjList, -1, -1, -1,
				    TRIV_TVBEZIER_TYPE, &ErrStr);

    if (TrivObj == NULL) {
	IRIT_NON_FATAL_ERROR2("TBEZIER: %s, empty object result.\n", ErrStr);
    }

    return TrivObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bspline trivar geometric object defined by a list    M
* of lists of lists of control points.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   RUOrder:        U order of trivar.                                       M
*   RVOrder:        V order of trivar.                                       M
*   RWOrder:        W order of trivar.                                       M
*   LstLstObjList:  A list object of lists of control points.                M
*   KntObjList:     A list of knots (numeric values).                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bspline trivar object if successful, NULL otherwise. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBsplineTrivarObject                                                   M
*****************************************************************************/
IPObjectStruct *GenBsplineTrivarObject(IrtRType *RUOrder,
				       IrtRType *RVOrder,
				       IrtRType *RWOrder,
				       IPObjectStruct *LstLstObjList,
				       IPObjectStruct *KntObjList)
{
    int Len1, Len2, Len3,
	UOrder = IRIT_REAL_PTR_TO_INT(RUOrder),
	VOrder = IRIT_REAL_PTR_TO_INT(RVOrder),
	WOrder = IRIT_REAL_PTR_TO_INT(RWOrder);
    char *ErrStr;
    IPObjectStruct
	*TrivObj = GetControlTVMesh(LstLstObjList, UOrder, VOrder, WOrder,
				    TRIV_TVBSPLINE_TYPE, &ErrStr);

    if (TrivObj == NULL) {
	IRIT_NON_FATAL_ERROR2("TBSPLINE: Ctl mesh, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (!IP_IS_OLST_OBJ(KntObjList) || IPListObjectLength(KntObjList) != 3) {
	IPFreeObject(TrivObj);
	IRIT_NON_FATAL_ERROR("TBSPLINE: Exactly three knot vectors expected");
	return NULL;
    }

    if (TrivObj -> U.Trivars -> ULength < TrivObj -> U.Trivars -> UOrder ||
	TrivObj -> U.Trivars -> VLength < TrivObj -> U.Trivars -> VOrder ||
	TrivObj -> U.Trivars -> WLength < TrivObj -> U.Trivars -> WOrder) {
	IPFreeObject(TrivObj);
	IRIT_NON_FATAL_ERROR("TBSPLINE: Trivar mesh length smaller than order.");
	return NULL;
    }

    IritFree(TrivObj -> U.Trivars -> UKnotVector);
    TrivObj -> U.Trivars -> UKnotVector = NULL;
    IritFree(TrivObj -> U.Trivars -> VKnotVector);
    TrivObj -> U.Trivars -> VKnotVector = NULL;
    IritFree(TrivObj -> U.Trivars -> WKnotVector);
    TrivObj -> U.Trivars -> WKnotVector = NULL;
    Len1 = TrivObj -> U.Trivars -> ULength;
    Len2 = TrivObj -> U.Trivars -> VLength;
    Len3 = TrivObj -> U.Trivars -> WLength;
    if ((TrivObj -> U.Trivars -> UKnotVector =
	 GetKnotVector(IPListObjectGet(KntObjList, 0), UOrder,
		       &Len1, &ErrStr, TRUE)) == NULL ||
	(TrivObj -> U.Trivars -> VKnotVector =
	 GetKnotVector(IPListObjectGet(KntObjList, 1), VOrder,
		       &Len2, &ErrStr, TRUE)) == NULL ||
	(TrivObj -> U.Trivars -> WKnotVector =
	 GetKnotVector(IPListObjectGet(KntObjList, 2), WOrder,
		       &Len3, &ErrStr, TRUE)) == NULL) {
	IPFreeObject(TrivObj);
	IRIT_NON_FATAL_ERROR2("TBSPLINE: Knot vectors, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (Len1 != TrivObj -> U.Trivars -> ULength + UOrder) {
	if (Len1 == TrivObj -> U.Trivars -> ULength + UOrder + UOrder - 1)
	    TrivObj -> U.Trivars -> UPeriodic = TRUE;
	else {
	    IPFreeObject(TrivObj);
	    IRIT_NON_FATAL_ERROR("Wrong knot vector length");
	    return NULL;
	}
    }
    if (Len2 != TrivObj -> U.Trivars -> VLength + VOrder) {
	if (Len2 == TrivObj -> U.Trivars -> VLength + VOrder + VOrder - 1)
	    TrivObj -> U.Trivars -> VPeriodic = TRUE;
	else {
	    IPFreeObject(TrivObj);
	    IRIT_NON_FATAL_ERROR("Wrong knot vector length");
	    return NULL;
	}
    }
    if (Len3 != TrivObj -> U.Trivars -> WLength + WOrder) {
	if (Len3 == TrivObj -> U.Trivars -> WLength + WOrder + WOrder - 1)
	    TrivObj -> U.Trivars -> WPeriodic = TRUE;
	else {
	    IPFreeObject(TrivObj);
	    IRIT_NON_FATAL_ERROR("Wrong knot vector length");
	    return NULL;
	}
    }

    return TrivObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate a trivariate function, at the prescribed parametric location.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:    Trivariate to evaluate.                                        M
*   u, v, w:  Parametric location to evaluate at.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A control point of the same type TV has.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalTrivarObject                                                         M
*****************************************************************************/
IPObjectStruct *EvalTrivarObject(IPObjectStruct *TVObj,
				 IrtRType *u,
				 IrtRType *v,
				 IrtRType *w)
{
    CagdRType
	*Pt = TrivTVEval(TVObj -> U.Trivars, *u, *v, *w);
    IPObjectStruct
	*CtlPtObj = IPGenCTLPTObject(TVObj -> U.Trivars -> PType, Pt);

    return CtlPtObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts an isoparametric surface out of the trivariate.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:     Trivariate to extract surface from.                           M
*   RDir:      Direction of extarction. One of U, V, W.                      M
*   ParamVal:  Parameter value of trivariate.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An isoparametric surface.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceFromTrivar                                                        M
*****************************************************************************/
IPObjectStruct *SurfaceFromTrivar(IPObjectStruct *TVObj,
				  IrtRType *RDir,
				  IrtRType *ParamVal)
{
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    CagdSrfStruct
	*Srf = TrivSrfFromTV(TVObj -> U.Trivars, *ParamVal, Dir, FALSE);

    if (Srf == NULL)
	return NULL;

    return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts an isoparametric surface out of the trivariate.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:     Trivariate to extract surface from.                           M
*   RDir:      Direction of extarction. One of U, V, W.                      M
*   RIndex:    Index inside the mesh in prescribed direction.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An isoparametric surface.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceFromTrivMesh                                                      M
*****************************************************************************/
IPObjectStruct *SurfaceFromTrivMesh(IPObjectStruct *TVObj,
				    IrtRType *RDir,
				    IrtRType *RIndex)
{
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir),
	Index = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RIndex);
    CagdSrfStruct
	*Srf = TrivSrfFromMesh(TVObj -> U.Trivars, Index, (TrivTVDirType) Dir);

    if (Srf == NULL)
	return NULL;

    return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to subdivide a trivariate function into two in specified direction M
* and specified parameter value.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:        Trivariate to subdivide.                                   M
*   RDir:         Direction of subdivision. Either U, V, or W.               M
*   ParamVal:     Parameter value at which subdivision should occur.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object of two trivar objects, result of the    M
*		       subdivision.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DivideTrivarObject                                                       M
*****************************************************************************/
IPObjectStruct *DivideTrivarObject(IPObjectStruct *TVObj,
				    IrtRType *RDir,
				    IrtRType *ParamVal)
{
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    TrivTVStruct
	*TV = TrivTVSubdivAtParam(TVObj -> U.Trivars, *ParamVal, Dir);
    IPObjectStruct *TV1, *TV2, *TVList;

    if (TV == NULL)
	return NULL;

    TV1 = IPGenTRIVARObject(TV);
    TV1 -> Attr = IP_ATTR_COPY_ATTRS(TVObj -> Attr);
    TV2 = IPGenTRIVARObject(TV -> Pnext);
    TV2 -> Attr = IP_ATTR_COPY_ATTRS(TVObj -> Attr);

    TV -> Pnext = NULL;

    TVList = IPGenLISTObject(TV1);
    IPListObjectInsert(TVList, 1, TV2);
    IPListObjectInsert(TVList, 2, NULL);

    return TVList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract a region of a trivariate in specified direction         M
* and specified parameter values.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:       Trivariate to extract a region from.                        M
*   RDir:        Direction of region extraction. Either U or V or W.         M
*   ParamVal1:   Parameter of beginning of region.                           M
*   ParamVal2:   Parameter of end of region.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A region of TVObj,                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   RegionFromTrivarObject                                                   M
*****************************************************************************/
IPObjectStruct *RegionFromTrivarObject(IPObjectStruct *TVObj, 
					IrtRType *RDir,
					IrtRType *ParamVal1,
					IrtRType *ParamVal2)
{
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    TrivTVStruct
	*TV = TrivTVRegionFromTV(TVObj -> U.Trivars,
				 *ParamVal1, *ParamVal2, Dir);

    if (TV == NULL)
	return NULL;

    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to refine a trivariate function in specified direction             M
* and knot vector.							     M
*   If, however, Replace is non zero, KnotsObj REPLACES current vector.      M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:      Trivariate to refine in direction RDir.                      M
*   RDir:       Direction of refinement. Either U or V or W.                 M
*   RReplace:   If TRUE KnotsObj will replace the RDir knot vector of TVObj. M
*		Otherwise, the knots in KnotsObj will be added to it.	     M
*   KnotsObj:   A list of knots.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A refined trivar, or a trivar with a replaced knot   M
*                       vector.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   RefineTrivarObject                                                       M
*****************************************************************************/
IPObjectStruct *RefineTrivarObject(IPObjectStruct *TVObj,
				    IrtRType *RDir,
				    IrtRType *RReplace,
				    IPObjectStruct *KnotsObj)
{
    int n,
	Replace = IRIT_REAL_PTR_TO_INT(RReplace);
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    char *ErrStr;
    CagdRType
	*t = GetKnotVector(KnotsObj, 0, &n, &ErrStr, FALSE);
    TrivTVStruct *RefTV;
    IPObjectStruct *RefTVObj;

    if (t == NULL) {
	IPFreeObject(TVObj);
	IRIT_NON_FATAL_ERROR2("REFINE: %s, empty object result.\n", ErrStr);
	return NULL;
    }
    RefTV = TrivTVRefineAtParams(TVObj -> U.Trivars, Dir, Replace, t, n);
    IritFree(t);
    if (RefTV == NULL)
	return NULL;

    RefTVObj = IPGenTRIVARObject(RefTV),
    RefTVObj -> Attr = IP_ATTR_COPY_ATTRS(TVObj -> Attr);

    return RefTVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to differentiate a trivariate function in Dir of SrfObj.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TVObj:     Trivar to differentiate.                                      M
*   Dir:       Direction of differentiation. Either U or V or W.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A differentiated trivar.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   DeriveTrivarObject                                                       M
*****************************************************************************/
IPObjectStruct *DeriveTrivarObject(IPObjectStruct *TVObj, IrtRType *Dir)
{
    TrivTVStruct
	*DerivTV = TrivTVDerive(TVObj -> U.Trivars,
				(TrivTVDirType) IRIT_REAL_PTR_TO_INT(Dir));
    IPObjectStruct
	*DerivTVObj = IPGenTRIVARObject(DerivTV);

    return DerivTVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interpolates/least squares fit a three dimensional grid.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Either a Trivariate to interpolate its control points or a     M
8	      list of scattered points to interpolate/least squares fit.     M
*   DULength, DVLength, DWLength:  Lengths of created-interpolated trivar.   M
*   DUOrder, DVOrder, DWOrder:     Orders of created-interpolated trivar.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Interpolating trivar, with same order as original.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   InterpolateTrivar                                                        M
*****************************************************************************/
IPObjectStruct *InterpolateTrivar(IPObjectStruct *PObj,
				  IrtRType *DULength,
				  IrtRType *DVLength,
				  IrtRType *DWLength,
				  IrtRType *DUOrder,
				  IrtRType *DVOrder,
				  IrtRType *DWOrder)
{
    TrivTVStruct
	*NewTV = NULL;

    if (IP_IS_TRIVAR_OBJ(PObj))
        NewTV = TrivTVInterpPts(PObj -> U.Trivars,
				IRIT_REAL_PTR_TO_INT(DULength),
				IRIT_REAL_PTR_TO_INT(DVLength),
				IRIT_REAL_PTR_TO_INT(DWLength),
				IRIT_REAL_PTR_TO_INT(DUOrder),
				IRIT_REAL_PTR_TO_INT(DVOrder),
				IRIT_REAL_PTR_TO_INT(DWOrder));
    else if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0,
	    NumCoords = 4;
	CagdCtlPtStruct *CtlPt,
	    *CtlPtHead = NULL;
	IPObjectStruct *Obj, *PtObj;

	/* Validate the input. */
	while ((Obj = IPListObjectGet(PObj, i++)) != NULL) {
	    if (!IP_IS_CTLPT_OBJ(Obj) &&
		!IP_IS_POINT_OBJ(Obj) &&
		!IP_IS_VEC_OBJ(Obj)) {
		return NULL;
	    }
	    if (IP_IS_CTLPT_OBJ(Obj) &&
		NumCoords < CAGD_NUM_OF_PT_COORD(Obj -> U.CtlPt.PtType))
	        NumCoords = CAGD_NUM_OF_PT_COORD(Obj -> U.CtlPt.PtType);
	}

	/* Collect all points of scattered type interpolation/approximation. */
	IPCoercePtsListTo(PObj, CAGD_MAKE_PT_TYPE(FALSE, NumCoords));

	for (i = 0; (PtObj = IPListObjectGet(PObj, i)) != NULL; i++) {
	    CtlPt = CagdCtlPtCopy(&PtObj -> U.CtlPt);
	    IRIT_LIST_PUSH(CtlPt, CtlPtHead);
	}

	NewTV = TrivTVInterpScatPts(CtlPtHead, 
				    IRIT_REAL_PTR_TO_INT(DUOrder),
				    IRIT_REAL_PTR_TO_INT(DVOrder),
				    IRIT_REAL_PTR_TO_INT(DWOrder),
				    IRIT_REAL_PTR_TO_INT(DULength),
				    IRIT_REAL_PTR_TO_INT(DVLength),
				    IRIT_REAL_PTR_TO_INT(DWLength),
				    NULL, NULL, NULL);

	CagdCtlPtFreeList(CtlPtHead);
    }

    if (NewTV == NULL)
	return NULL;

    return IPGenTRIVARObject(NewTV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes moments of freeform curves.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to compute moment for.                                  M
*   RMoment:   Order of moment.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An E3 vector representing the approximated moment.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeCrvMoments, moments                                               M
*****************************************************************************/
IPObjectStruct *ComputeCrvMoments(IPObjectStruct *CrvObj, IrtRType *RMoment)
{
    CagdPType Pt;
    CagdVType Vec;
    int Moment = IRIT_REAL_PTR_TO_INT(RMoment);

    CagdCrvFirstMoments(CrvObj -> U.Crvs, 100, Pt, Vec);

    switch (Moment) {
	case 0:
	    return IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]);
	case 1:
	    return IPGenVECObject(&Vec[0], &Vec[1], &Vec[2]);
	default:
	    IRIT_NON_FATAL_ERROR("Moment: Only moments of order zero or one");
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a trivariate out of the provided surface list.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:     A list of surfaces to approximate a trivariate through.     M
*   OtherOrder:  Other, third, order of trivariate.                          M
*   OtherEC:     Other, third, end condition of trivariate.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A trivariate approximately traversing through the    M
*			given surfaces.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTVFromSrfsObject                                                      M
*****************************************************************************/
IPObjectStruct *GenTVFromSrfsObject(IPObjectStruct *SrfList,
				    IrtRType *OtherOrder,
				    IrtRType *OtherEC)
{
    int i,
	NumSrfs = 0;
    IPObjectStruct *TVObj, *SrfObj;
    TrivTVStruct *TV;
    CagdSrfStruct *Srf,
	*Srfs = NULL;

    if (!IP_IS_OLST_OBJ(SrfList))
	IRIT_FATAL_ERROR("TFROMSRF: Not object list object!");

    while ((SrfObj = IPListObjectGet(SrfList, NumSrfs)) != NULL) {
	if (!IP_IS_SRF_OBJ(SrfObj)) {
	    IRIT_NON_FATAL_ERROR("TFROMSRF: List contains non surface object(s).");
	    return NULL;
	}
	if (SrfObj -> U.Srfs -> Pnext != NULL) {
	    IRIT_NON_FATAL_ERROR("TFROMSRF: nested surface lists are disallowed.");
	    return NULL;
	}
	NumSrfs++;
    }

    /* Chain all surfaces into a single list and invoke the TV constructor: */
    for (i = 0; i < NumSrfs; i++) {
	SrfObj = IPListObjectGet(SrfList, i);
	Srf = CagdSrfCopy(SrfObj -> U.Srfs);
	IRIT_LIST_PUSH(Srf, Srfs);
    }

    Srfs = CagdListReverse(Srfs);

    switch (IRIT_REAL_PTR_TO_INT(OtherEC)) {
	case KV_UNIFORM_OPEN:
	    TV = TrivTVFromSrfs(Srfs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				CAGD_END_COND_OPEN, NULL);
	    break;
	case KV_UNIFORM_FLOAT:
	    TV = TrivTVFromSrfs(Srfs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				CAGD_END_COND_FLOAT, NULL);
	    break;
	case KV_UNIFORM_PERIODIC:
	    TV = TrivTVFromSrfs(Srfs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				CAGD_END_COND_PERIODIC, NULL);
	    break;
	default:
	    assert(0);
	    TV = NULL;
    }

    CagdSrfFreeList(Srfs);

    if (TV == NULL)
	return NULL;

    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a trivariate out of the provided surface list.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:     A list of surfaces to approximate a trivariate through.     M
*   OtherOrder:  Other, third, order of trivariate.                          M
*   OtherEC:     Other, third, end condition of trivariate.                  M
*   OtherParam:  Parametrization type in other direction, i.e. chord length  M
*                or uniform, etc.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A trivariate approximately traversing through the    M
*			given surfaces.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTVFromSrfsObject                                                      M
*****************************************************************************/
IPObjectStruct *GenTVInterpolateSrfsObject(IPObjectStruct *SrfList,
					   IrtRType *OtherOrder,
					   IrtRType *OtherEC,
					   IrtRType *OtherParam)
{
    int i,
	NumSrfs = 0;
    IPObjectStruct *TVObj, *SrfObj;
    TrivTVStruct *TV;
    CagdSrfStruct *Srf,
	*Srfs = NULL;

    if (!IP_IS_OLST_OBJ(SrfList))
	IRIT_FATAL_ERROR("TINTPSRF: Not object list object!");

    while ((SrfObj = IPListObjectGet(SrfList, NumSrfs)) != NULL) {
	if (!IP_IS_SRF_OBJ(SrfObj)) {
	    IRIT_NON_FATAL_ERROR("TINTPSRF: List contains non surface object(s).");
	    return NULL;
	}
	if (SrfObj -> U.Srfs -> Pnext != NULL) {
	    IRIT_NON_FATAL_ERROR("TINTPSRF: nested surface lists are disallowed.");
	    return NULL;
	}
	NumSrfs++;
    }

    /* Chain all surfaces into a single list and invoke the TV constructor: */
    for (i = 0; i < NumSrfs; i++) {
	SrfObj = IPListObjectGet(SrfList, i);
	Srf = CagdSrfCopy(SrfObj -> U.Srfs);
	IRIT_LIST_PUSH(Srf, Srfs);
    }

    Srfs = CagdListReverse(Srfs);

    switch (IRIT_REAL_PTR_TO_INT(OtherEC)) {
	case KV_UNIFORM_OPEN:
	    TV = TrivTVInterpolateSrfs(Srfs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				       CAGD_END_COND_OPEN,
				       IRIT_REAL_PTR_TO_INT(OtherParam), NULL);
	    break;
	case KV_UNIFORM_FLOAT:
	    TV = TrivTVInterpolateSrfs(Srfs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				       CAGD_END_COND_FLOAT,
				       IRIT_REAL_PTR_TO_INT(OtherParam), NULL);
	    break;
	case KV_UNIFORM_PERIODIC:
	    TV = TrivTVInterpolateSrfs(Srfs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				       CAGD_END_COND_PERIODIC,
				       IRIT_REAL_PTR_TO_INT(OtherParam), NULL);
	    break;
	default:
	    assert(0);
	    TV = NULL;
    }

    CagdSrfFreeList(Srfs);

    if (TV == NULL)
	return NULL;

    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a trivar of revolution.	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Z axis forming a trivar of revolution.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A trivar of revolution.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTVREVObject                                                           M
*****************************************************************************/
IPObjectStruct *GenTVREVObject(IPObjectStruct *Cross)
{
    if (IP_IS_OLST_OBJ(Cross)) {
        int i;
        IPObjectStruct *PTmp,
	    *PRetVal = IPGenLISTObject(NULL);

	for (i = 0; (PTmp = IPListObjectGet(Cross, i)) != NULL; i++) {
	    IPListObjectAppend(PRetVal, GenTVREVObject(PTmp));
	}
	return PRetVal;
    }
    else if (IP_IS_SRF_OBJ(Cross)) {
        return IPGenTRIVARObject(TrivTVOfRev(Cross -> U.Srfs));
    }
    else {
        assert(0);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Generates a polynomial approximation of a trivar of revolution.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Cross:     To rotate around the Z axis forming an approximation of a     M
*              trivar of revolution.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A polynomial approximation of a trivar of revolution. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTVPREVObject                                                          M
*****************************************************************************/
IPObjectStruct *GenTVPREVObject(IPObjectStruct *Cross)
{
    if (IP_IS_OLST_OBJ(Cross)) {
        int i;
        IPObjectStruct *PTmp,
	    *PRetVal = IPGenLISTObject(NULL);

	for (i = 0; (PTmp = IPListObjectGet(Cross, i)) != NULL; i++) {
	    IPListObjectAppend(PRetVal, GenTVPREVObject(PTmp));
	}
	return PRetVal;
    }
    else if (IP_IS_SRF_OBJ(Cross)) {
        return IPGenTRIVARObject(TrivTVOfRevPolynomialApprox(Cross -> U.Srfs));
    }
    else {
        assert(0);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the zero of the jacobian of the given trivariate.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Tv:        Trivariate to compute the zero of its Jacobian.               M
*   REuclid:   If TRUE result in Euclidean space.  If FALSE, Tv param space. M
*   RSkipRate: Of data in the volume.  1 skips nothing, 2 every second, etc. M
*   RFineness: Number of double-knots refinements, zero for none, of all     M
*	       three axes of the trivariate if a list of three numbers.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Piecewise linear polygonal approximation of result.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivZeroJacobian                                                         M
*****************************************************************************/
IPObjectStruct *TrivZeroJacobian(IPObjectStruct *Tv,
				 IrtRType *REuclid,
				 IrtRType *RSkipRate,
				 IPObjectStruct *RFineness)
{
    int Euclid = IRIT_REAL_PTR_TO_INT(REuclid),
	SkipRate = IRIT_REAL_PTR_TO_INT(RSkipRate);
    CagdRType Fineness[3];

    Fineness[0] = Fineness[1] = Fineness[2] = 0.0;
    if (IP_IS_NUM_OBJ(RFineness)) {
	Fineness[0] = Fineness[1] = Fineness[2] = RFineness -> U.R;
    }
    else if (IP_IS_OLST_OBJ(RFineness) && IPListObjectLength(RFineness) == 3) {
	IPObjectStruct
	    *PNum1 = IPListObjectGet(RFineness, 0),
	    *PNum2 = IPListObjectGet(RFineness, 1),
	    *PNum3 = IPListObjectGet(RFineness, 2);

	if (IP_IS_NUM_OBJ(PNum1) &&
	    IP_IS_NUM_OBJ(PNum2) &&
	    IP_IS_NUM_OBJ(PNum3)) {

	    Fineness[0] = PNum1 -> U.R;
	    Fineness[1] = PNum2 -> U.R;
	    Fineness[2] = PNum3 -> U.R;
	}
    }

    return UserTVZeroJacobian(Tv -> U.Trivars, Euclid, SkipRate, Fineness);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the kernel of a freeform simple closed curve.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to compute its kernel.			             M
*   RGamma:    Angular deviation for gamma-kernel computation.  Zero for     M
*	       regular Kernel, positive/negative degrees for gamma-kernel.   M
*   REuclid:   TRUE result in Euclidean space.  If FALSE, Tv param space.    M
*   RFineness: If RMode == 0/1, number of double-knots refinements, zero     M
*	           for none, of all three axes of the trivariate if a list   M
*		   of three numbers.					     M
*	       If RMode == 2, a numeric value that controls the extent of    M
*	       the surfaces/trivars.					     M
*   RMode:     0 for the gamma kernel.					     M
*	       1 to extract silhouette samples only out of the trivar.       M
*              2 for gamma kernel surfaces/trivariates.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Piecewise linear polygonal approximation of result or M
*		the trivariate function itself if RFineness is negative.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvKernel		                                                     M
*****************************************************************************/
IPObjectStruct *CrvKernel(IPObjectStruct *Crv,
			  IrtRType *RGamma,
			  IrtRType *REuclid,
			  IPObjectStruct *RFineness,
			  IrtRType *RMode)
{
    int Euclid = IRIT_REAL_PTR_TO_INT(REuclid),
	Mode = IRIT_REAL_PTR_TO_INT(RMode);
    CagdRType Fineness[3], UMin, UMax, VMin, VMax, WMin, WMax;
    IrtHmgnMatType Mat;

    switch (Mode) {
	default:
	case 0:
	    {
	        MvarMVStruct
		    *Mv = MVarCrvGammaKernel(Crv -> U.Crvs, *RGamma);
		TrivTVStruct
		    *Tv = MvarMVToTV(Mv);
		IPObjectStruct *RetObj, *TObj;

		MvarMVFree(Mv);

		Fineness[0] = Fineness[1] = Fineness[2] = 0.0;
		if (IP_IS_NUM_OBJ(RFineness)) {
		    if (RFineness -> U.R < 0)
		        return IPGenTRIVARObject(Tv);

		    Fineness[0] = Fineness[1] = Fineness[2] = RFineness -> U.R;
		}
		else if (IP_IS_OLST_OBJ(RFineness) && 
			 IPListObjectLength(RFineness) == 3) {
		    IPObjectStruct
		        *PNum1 = IPListObjectGet(RFineness, 0),
		        *PNum2 = IPListObjectGet(RFineness, 1),
		        *PNum3 = IPListObjectGet(RFineness, 2);

		    if (IP_IS_NUM_OBJ(PNum1) &&
			IP_IS_NUM_OBJ(PNum2) &&
			IP_IS_NUM_OBJ(PNum3)) {
		        Fineness[0] = PNum1 -> U.R;
			Fineness[1] = PNum2 -> U.R;
			Fineness[2] = PNum3 -> U.R;
		    }
		    else {
		        IRIT_FATAL_ERROR("CRVKERNEL: Three fineness numbers expected!");
			return NULL;
		    }
		}
		else {
		    IRIT_FATAL_ERROR("CRVKERNEL: Wrong fineness setting!");
		    return NULL;
		}

		TObj = UserTrivarZeros(Tv, Euclid ? Tv : NULL, 1, Fineness);

		TrivTVDomain(Tv, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);
		MatGenMatTrans(UMin, VMin, 0.0, Mat);
		RetObj = GMTransformObject(TObj, Mat);
		IPFreeObject(TObj);

		TrivTVFree(Tv);
		return RetObj;
	    }
	case 1:
	    if (IP_IS_OLST_OBJ(RFineness) && 
		IPListObjectLength(RFineness) == 2) {
	        IPObjectStruct
	            *PNum1 = IPListObjectGet(RFineness, 0),
	            *PNum2 = IPListObjectGet(RFineness, 1);

		if (IP_IS_NUM_OBJ(PNum1) && IP_IS_NUM_OBJ(PNum2))
		    return MVarCrvKernelSilhouette(Crv -> U.Crvs, *RGamma,
						   PNum1 -> U.R, PNum2 -> U.R);
		else {
		    IRIT_FATAL_ERROR("CRVKERNEL: Two fineness numbers expected!");
		    return NULL;
		}
	    }
	    else {
	        IRIT_FATAL_ERROR("CRVKERNEL: Two fineness numbers expected!");
		return NULL;
	    }
	    break;
	case 2:
	    {
	        CagdRType
		    Extent =  IP_IS_NUM_OBJ(RFineness) ? RFineness -> U.R : 1;
	        MvarMVStruct
		    *Mv = MVarCrvGammaKernelSrf(Crv -> U.Crvs,
						Extent, *RGamma);

		return Mv == NULL ? NULL : IPGenMULTIVARObject(Mv);
	    }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the kernel of a freeform simple closed surface.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       A surface to compute its kernel.			             M
*   Fineness:  Tolerance of surface approximation as well as parabolic       M
*	       curves approximation.					     M
*   RSkipRate: Step size over the parabolic points, 1 to process them all.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A polygonal approximation of the kernel.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfKernel		                                                     M
*****************************************************************************/
IPObjectStruct *SrfKernel(IPObjectStruct *Srf,
			  IrtRType *Fineness,
			  IrtRType *RSkipRate)
{
    IPObjectStruct
	*PObj = UserSrfKernel(Srf -> U.Srfs, *Fineness,
			      IRIT_REAL_PTR_TO_INT(RSkipRate));

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the diameter of a freeform simple closed curve.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to compute its kernel.			             M
*   RSubEps:   Subdivision epsilon.					     M
*   RNumEps:   Numeric marching tolerance.				     M
*   RMinMaxAll:  0 for minimal diameter, 1 for maximal diameter, 2 for all.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Parameter (pair of) values along the curve with the   M
		requested diameter (or a list of such parameters).	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvDiameter		                                                     M
*****************************************************************************/
IPObjectStruct *CrvDiameter(IPObjectStruct *Crv,
			    IrtRType *RSubEps,
			    IrtRType *RNumEps,
			    IrtRType *RMinMaxAll)
{
    int MinMaxAll = IRIT_REAL_PTR_TO_INT(RMinMaxAll);
    IPObjectStruct *PObj,
	*Pairs = MVarCrvDiameter(Crv -> U.Crvs, *RSubEps, *RNumEps);

    switch (MinMaxAll) {
	case 2:
	    /* Convert the computed points to our form. */
	    return Pairs;
	case 0:
	case 1:
	    {
		int i;
		CagdRType MinParam[2], MaxParam[2], *R, d,
		    MinDist = IRIT_INFNTY,
		    MaxDist = -IRIT_INFNTY,
		    Zero = 0.0;
		CagdPType Pt1, Pt2;
		CagdCrvStruct
		    *TCrv = Crv -> U.Crvs;

		for (i = 0; (PObj = IPListObjectGet(Pairs, i)) != NULL; i++) {
		    R = CagdCrvEval(TCrv, PObj -> U.Pt[0]);
		    CagdCoerceToE3(Pt1, &R, -1, TCrv -> PType);
		    R = CagdCrvEval(TCrv, PObj -> U.Pt[1]);
		    CagdCoerceToE3(Pt2, &R, -1, TCrv -> PType);

		    d = IRIT_PT_PT_DIST_SQR(Pt1, Pt2);
		    if (MinDist > d) {
		        MinDist = d;
			MinParam[0] = PObj -> U.Pt[0];
			MinParam[1] = PObj -> U.Pt[1];
		    }
		    if (MaxDist < d) {
		        MaxDist = d;
			MaxParam[0] = PObj -> U.Pt[0];
			MaxParam[1] = PObj -> U.Pt[1];
		    }
		}

		if (MinMaxAll == 1)
		    PObj = IPGenPTObject(&MaxParam[0], &MaxParam[1], &Zero);
		else
		    PObj = IPGenPTObject(&MinParam[0], &MinParam[1], &Zero);

		IPFreeObject(Pairs);
		return PObj;
	    }
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a cubic Hermite curve or surface out of the given two positional M
* and two directional constraints. The constraints can be either	     M
* points/vectors where a cubic Bezier curve is constructed or four curves    M
* where a surface is constructed.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos1:    Starting position.                                              M
*   Pos2:    End position.                                                   M
*   Dir1:    Tangent at starting position.                                   M
*   Dir2:    Tangent at end position.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A curve or surface satisfying these four constraints. M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenHermiteObject, Hermite                                                M
*****************************************************************************/
IPObjectStruct *GenHermiteObject(IPObjectStruct *Pos1,
				 IPObjectStruct *Pos2,
				 IPObjectStruct *Dir1,
				 IPObjectStruct *Dir2)
{
    IPObjectStruct *PObj;

    if (IP_IS_CRV_OBJ(Pos1) &&
	IP_IS_CRV_OBJ(Pos2) &&
	IP_IS_CRV_OBJ(Dir1) &&
	IP_IS_CRV_OBJ(Dir2)) {
	CagdSrfStruct
	    *Srf = CagdCubicHermiteSrf(Pos1 -> U.Crvs, Pos2 -> U.Crvs,
				       Dir1 -> U.Crvs, Dir2 -> U.Crvs);

	PObj = Srf != NULL ? IPGenSRFObject(Srf) : NULL;
    }
    else if ((IP_IS_POINT_OBJ(Pos1) || IP_IS_VEC_OBJ(Pos1)) &&
	     (IP_IS_POINT_OBJ(Pos2) || IP_IS_VEC_OBJ(Pos2)) &&
	     (IP_IS_POINT_OBJ(Dir1) || IP_IS_VEC_OBJ(Dir1)) &&
	     (IP_IS_POINT_OBJ(Dir2) || IP_IS_VEC_OBJ(Dir2))) {
	CagdCrvStruct
	    *Crv = CagdCubicHermiteCrv(Pos1 -> U.Pt, Pos2 -> U.Pt,
				       Dir1 -> U.Vec, Dir2 -> U.Vec);

	PObj = Crv != NULL ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_FATAL_ERROR("HERMITE: Invalid parameters!");
	PObj = NULL;
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a blending surface that interpolates the given two positional    M
* and two directional constraints.   In addition, the constructed surface    M
* will follow the CrossSec in Normal direction.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos1:    Starting position.                                              M
*   Pos2:    End position.                                                   M
*   Dir1:    Tangent at starting position.                                   M
*   Dir2:    Tangent at end position.                                        M
*   CrossSec:   2D shape of the cross section of the blend.	             M
*	     Must satisfy (C(t) as CrossSecShape, t in [0, 1]),	             M
*	     C(0) = (-1, 0), C(1) = (1,0), C'(0) = (0, 0), C'(1) = (0, 0).   M
*   Normal:  A unit vector field orthogonal to Pos1Crv - Pos2Crv.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The blended surface constructed as follows: 	     M
*		       Let S(t) = ( Pos1(t) + Pos2(t) ) / 2		     M
*		       Let D(t) = ( Pos2(t) - Pos1(t) ) / 2		     M
*		       Then S(t, r) = H01(r) * Dir1Crv(t) +		     M
*				      H11(r) * Dir2Crv(t) + 		     M
*				      S(t) + D(t) * CrossSecShape_x(r)       M
*				      Normal(t) * CrossSecShape_y(r)	     M
*		       where H are the cubic Hermite functions for the two   M
*		       tangent fields.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBlendHermiteObject, Hermite                                           M
*****************************************************************************/
IPObjectStruct *GenBlendHermiteObject(IPObjectStruct *Pos1,
				      IPObjectStruct *Pos2,
				      IPObjectStruct *Dir1,
				      IPObjectStruct *Dir2,
				      IPObjectStruct *CrossSec,
				      IPObjectStruct *Normal)
{
    CagdSrfStruct
	*Srf = SymbShapeBlendSrf(Pos1 -> U.Crvs, Pos2 -> U.Crvs,
				 Dir1 -> U.Crvs, Dir2 -> U.Crvs,
				 CrossSec -> U.Crvs, Normal -> U.Crvs);

    return Srf != NULL ? IPGenSRFObject(Srf) : NULL;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a surface, C^1 tangent to given surface and has the           M
* prescribed cross section shape.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     Surface to construct the blended shape on, with C^1 continuity. M
*   UVCrv:   The curve along which to blend the formed shae, in the	     M
*	     parametric domain of the surface.  Assumed to be in Srf.	     M
*   CrossSecShape:  The cross section of this blended shape.		     M
*   TanScale:       Scale factor of derived tangent fields.		     M
*   Width:   Of swept shape, in parametric space units.			     M
*   Height:  Of swept shape, in Euclidean space units a scaling factor       M
*	     number of a scalar curve.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A newly form swept shape with CrossSecShape as       M
*		approximated cross section, along UVCrv.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBlendHermiteOnSrfObject, Hermite                                      M
*****************************************************************************/
IPObjectStruct *GenBlendHermiteOnSrfObject(IPObjectStruct *Srf,
					   IPObjectStruct *UVCrv,
					   IPObjectStruct *CrossSecShape,
					   IrtRType *TanScale,
					   IPObjectStruct *Width,
					   IPObjectStruct *Height)
{
    CagdSrfStruct *RetSrf;

    if (IP_IS_CRV_OBJ(Width)) {
        if (IP_IS_CRV_OBJ(Height))
	    RetSrf = SymbShapeBlendOnSrf(Srf -> U.Srfs, UVCrv -> U.Crvs,
					 CrossSecShape -> U.Crvs, *TanScale,
					 1.0, Width -> U.Crvs,
					 1.0, Height -> U.Crvs);
	else /* A number. */
	    RetSrf = SymbShapeBlendOnSrf(Srf -> U.Srfs, UVCrv -> U.Crvs,
					 CrossSecShape -> U.Crvs, *TanScale,
					 1.0, Width -> U.Crvs,
					 Height -> U.R, NULL);
    }
    else {
        if (IP_IS_CRV_OBJ(Height))
	    RetSrf = SymbShapeBlendOnSrf(Srf -> U.Srfs, UVCrv -> U.Crvs,
					 CrossSecShape -> U.Crvs, *TanScale,
					 Width -> U.R, NULL,
					 1.0, Height -> U.Crvs);
	else /* A number. */
	    RetSrf = SymbShapeBlendOnSrf(Srf -> U.Srfs, UVCrv -> U.Crvs,
					 CrossSecShape -> U.Crvs, *TanScale,
					 Width -> U.R, NULL,
					 Height -> U.R, NULL);
    }

    return RetSrf != NULL ? IPGenSRFObject(RetSrf) : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reparametrize the second curve into the "best" match to the first curve. M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv1, PCrv2:  The two curve to establish matching between.              M
*   RReduce:       Accuracy of matching, the larger the better.              M
*   RSampleSet:    Number of samples of tangents.			     M
*   RReparamOrder: Order of reparametrization curve.			     M
*   RRotate:       If one would like to apply rotation.			     M
*   RNorm:         1 for ruled norm, 2 for morph norm.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: Reparametrized crv2 to match crv1.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatchTwoCurves                                                           M
*****************************************************************************/
IPObjectStruct *MatchTwoCurves(IPObjectStruct *PCrv1,
			       IPObjectStruct *PCrv2,
			       IrtRType *RReduce,
			       IrtRType *RSampleSet,
			       IrtRType *RReparamOrder,
			       IrtRType *RRotate,
			       IrtRType *RNorm)
{
    int AllowNegativeNorm,
	Reduce = IRIT_REAL_PTR_TO_INT(RReduce),
	SampleSet = IRIT_REAL_PTR_TO_INT(RSampleSet),
	Rotate = IRIT_REAL_PTR_TO_INT(RRotate),
	ReparamOrder = IRIT_REAL_PTR_TO_INT(RReparamOrder),
	Norm = IRIT_REAL_PTR_TO_INT(RNorm);
    CagdMatchNormFuncType
        MatchNorm = NULL;
    CagdCrvStruct *Crv;

    switch (IRIT_ABS(Norm)) {
	case 1:
	    MatchNorm = CagdMatchRuledNorm;
	    break;
	case 2:
	    MatchNorm = CagdMatchMorphNorm;
	    break;
	case 4:
	    MatchNorm = CagdMatchBisectorNorm;
	    break;
	case 3:
	default:
	    MatchNorm = CagdMatchDistNorm;
	    break;
    }
    AllowNegativeNorm = Norm < 0;

    Crv = CagdMatchingTwoCurves(PCrv1 -> U.Crvs, PCrv2 -> U.Crvs,
				Reduce, SampleSet, ReparamOrder,
				Rotate, AllowNegativeNorm, FALSE, MatchNorm);

    if (Crv == NULL) {
	IRIT_WNDW_PUT_STR("FFMATCH: failed to match the two curves.");
	return NULL;
    }

    return IPGenCRVObject(Crv);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of ContourFreeform (UserCntrSrfWithPlane actually) to *
* verify validity of a contour point.					     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   Surface foreach the contour is computed.                          *
*   u, v:  Parametric location of the contour point.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if valid, FALSE otherwise.                                   *
*****************************************************************************/
static int ContourFreeformValidateCntrPt(const CagdSrfStruct *Srf,
					 IrtRType u,
					 IrtRType v)
{
    CagdVecStruct
	*V = CagdSrfNormal(Srf, u, v, TRUE);

    return IRIT_DOT_PROD(GlblCntrFFValidateDir, V -> Vec) >=
						 GlblCntrFFValidateCosAngle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Contours a freeform surface according to prescribed Plane.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Freefrom object to contour.  If a freefom suface, Object is   M
*	       approximated using global resolution level, before contoured. M
*   Plane:     To contour PObj with.					     M
*   PSrfEvalObj: If non NULL and PObj is a scalar surface, the resulting     M
*	       contoured data is mapped onto this surface.		     M
*   PValidationObj:  A list object holding the contour points' validation    M
*	       information (or, if not a list object, no validation used) -  M
*	       list(vector V, angle A) - validates only contour points were  M
*	       the normal of the surface there form an angle of less than A  M
*	       degrees with V.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A polyline approximation of the contoured data, if    M
*		input is a 2-manifold, or a list of points if the input is a M
*		1-manifold.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ContourFreeform                                                          M
*****************************************************************************/
IPObjectStruct *ContourFreeform(IPObjectStruct *PObj,
				IrtPlnType Plane,
				IPObjectStruct *PSrfEvalObj,
				IPObjectStruct *PValidationObj)
{
    IPPolygonStruct *Cntrs, *Cntr;
    IPVertexStruct *V, *VList;
    IPObjectStruct *CntrObj;

    if (IP_IS_CRV_OBJ(PObj)) {
        CagdLType Line;
        CagdPtStruct *Pts, *Pt;

	Line[0] = Plane[0];      /* Ignore the Z coefficient of the plane. */
	Line[1] = Plane[1];
	Line[2] = Plane[3];

	Pts = SymbLclDistCrvLine(PObj -> U.Crvs, Line, IRIT_CONTOUR_EPS,
				 TRUE, FALSE);
	for (VList = NULL, Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	    V = IPAllocVertex2(VList);
	    VList = V;
	    V -> Coord[0] = Pt -> Pt[0];
	    V -> Coord[1] = V -> Coord[2] = 0.0;
	}
	Cntrs = IPAllocPolygon(0, VList, NULL);
	CntrObj = IPGenPOINTLISTObject(Cntrs);
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
        int Resolution = GetResolution(FALSE);
	IrtRType
	    RelResolution = AttrGetObjectRealAttrib(PObj, "resolution");

        if (Resolution < MIN_FREE_FORM_RES)
	    Resolution = MIN_FREE_FORM_RES;
	if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	    Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

	GlblCntrFFValidate = FALSE;
	if (PValidationObj != NULL && IP_IS_OLST_OBJ(PValidationObj)) {
	    IPObjectStruct
	        *PTmp1 = IPListObjectGet(PValidationObj, 0),
	        *PTmp2 = IPListObjectGet(PValidationObj, 1);

	    if (IP_IS_VEC_OBJ(PTmp1) && IP_IS_NUM_OBJ(PTmp2)) {
	        GlblCntrFFValidateCosAngle = cos(IRIT_DEG2RAD(PTmp2 -> U.R));
		IRIT_VEC_COPY(GlblCntrFFValidateDir, PTmp1 -> U.Vec);
		IRIT_VEC_NORMALIZE(GlblCntrFFValidateDir);
		GlblCntrFFValidate = TRUE;
	    }
	}

	Cntrs = UserCntrSrfWithPlane(PObj -> U.Srfs, Plane, Resolution);

	if (Cntrs == NULL)
	    return NULL;

	if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) == 1) {
	    if (PSrfEvalObj != NULL) {
	        Cntrs = UserCntrEvalToE3(PSrfEvalObj -> U.Srfs, Cntrs,
			 GlblCntrFFValidate ? ContourFreeformValidateCntrPt
					    : NULL);
	    }
	    else {
	        for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
		    for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
		        V -> Coord[0] = V -> Coord[1];
			V -> Coord[1] = V -> Coord[2];
			V -> Coord[2] = 0.0;
		    }
		}
	    }
        }

	CntrObj = IPGenPOLYLINEObject(Cntrs);
    }
    else if IP_IS_POLY_OBJ(PObj) {
        CntrObj = BooleanCONTOUR(PObj, Plane);
    }
    else {
	assert(0);
	CntrObj = NULL;
    }

    return CntrObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests for inclusion of Pt in a given polygon or polyhedra.               M
*   If PObj holds a single polygon, it is assumed a 2D polygon and the test  M
* is conducted in XY plane against this polygon only.  Otherwise a full test M
* in 3D is conducted.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Polygon/polyhedra to test for point inclusion.               M
*   Pt:         Point position.		                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  TRUE if Pt inside polygon, FALSE otherwise.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolygonPtInclusion                                                       M
*****************************************************************************/
IPObjectStruct *PolygonPtInclusion(IPObjectStruct *PObj, IrtPtType Pt)
{
    return IPGenNUMValObject(PObj -> U.Pl -> Pnext == NULL ?
			         GMPolygonPointInclusion(PObj -> U.Pl, Pt) :
			         GMPolygonPointInclusion3D(PObj -> U.Pl, Pt));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests for inclusion of Pt in a given planar curve.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Curve to test for point inclusion.                           M
*   Pt:         Point position.		                                     M
*   Epsilon:    Accuracy of computation.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  TRUE if Pt inside curve, FALSE otherwise.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurvePtInclusion                                                         M
*****************************************************************************/
IPObjectStruct *CurvePtInclusion(IPObjectStruct *PObj,
				 IrtPtType Pt,
				 IrtRType *Epsilon)
{
    return IPGenNUMValObject(SymbCrvPointInclusion(PObj -> U.Crvs, Pt,
						   *Epsilon));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the number of intersections of a ray and a polygon, in the XY   M
* plane.							             M
* The RayOrigin is the point the ray originates from, and the ray can be     M
* X parallel (RayDir = 0) or Y parallel (RayDir = 1).  			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Polygon to shoot a ray against.                              M
*   RayOrigin:  Origin position of ray.                                      M
*   RayDir:     Direction of ray, 0 for X dir, 1 for Y dir.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The number of intersections of the ray with the poly. M
*                                                                            *
* KEYWORDS:                                                                  M
*   PolygonRayIntersect                                                      M
*****************************************************************************/
IPObjectStruct *PolygonRayIntersect(IPObjectStruct *PObj,
				    IrtPtType RayOrigin,
				    IrtRType *RayDir)
{
    int NumInter = GMPolygonRayInter(PObj -> U.Pl, RayOrigin,
				     IRIT_REAL_PTR_TO_INT(RayDir));

    return IPGenNUMValObject(NumInter);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection point of a ray and a freeform surface.         M
*   The surface is approximated by polygons that are then tested for the     M
* intersection.								     M
*   If RayDir is (0, 0, 0), the closest point on the surfaces to RayOrigin   M
* is computed instead.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrfObj:    Surface to shoot a ray against.                              M
*   RayOrigin:  Origin position of ray.                                      M
*   RayDir:     Direction of ray.	                                     M
*   Tolerance:  Of computation.  Zero to signal end of use of this surface.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A point holding the UV values (in XY) of the first    M
*		ray surface intersection if was one, invalud coordinates     M
*		otherwise.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceRayIntersect                                                      M
*****************************************************************************/
IPObjectStruct *SurfaceRayIntersect(IPObjectStruct *PSrfObj,
				    IrtPtType RayOrigin,
				    IrtVecType RayDir,
				    IrtRType *Tolerance)
{
    IRIT_STATIC_DATA VoidPtr
	Handle = NULL;
    IRIT_STATIC_DATA IrtPtType
        PtInfty = { IRIT_INFNTY, IRIT_INFNTY, 1.0 };
    CagdUVType UV;
    IPObjectStruct *PObj;

    if (*Tolerance <= 0.0) {
        if (Handle != NULL) {
	    IntrSrfHierarchyFreePreprocess(Handle);
	    Handle = NULL;
	}
	PObj = IPGenPTObject(&PtInfty[0], &PtInfty[1], &PtInfty[2]);
    }
    else {
        if (Handle == NULL)
	    Handle = IntrSrfHierarchyPreprocessSrf(PSrfObj -> U.Srfs,
						   *Tolerance);

	if ((IRIT_PT_EQ_ZERO(RayDir) &&
	     IntrSrfHierarchyTestPt(Handle, RayOrigin, TRUE, UV)) ||
	    (!IRIT_PT_EQ_ZERO(RayDir) &&
	     IntrSrfHierarchyTestRay(Handle, RayOrigin, RayDir, UV))) {
	    IRIT_STATIC_DATA IrtRType
	        R = 0.0;

	    PObj = IPGenPTObject(&UV[0], &UV[1], &R);
	}
	else {
	    PObj = IPGenPTObject(&PtInfty[0], &PtInfty[1], &PtInfty[2]);
	}
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Splits a given freeform into its scalar componenets.  Returns a list of  M
* all the components as E1 freeforms.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:     To split into its individial coefficients.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of coefficients as freeforms.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   FreeFormMergeScalar                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormSplitScalar                                                      M
*****************************************************************************/
IPObjectStruct *FreeFormSplitScalar(IPObjectStruct *FreeForm)
{
    IPObjectStruct
	*RetVal = NULL;
    int j,
	i = 0;

    if (IP_IS_CRV_OBJ(FreeForm)) {
	CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ;

	SymbCrvSplitScalar(FreeForm -> U.Crvs, &CrvW, &CrvX, &CrvY, &CrvZ);
	RetVal = IPGenLISTObject(NULL);
	if (CrvW)
	    IPListObjectInsert(RetVal, i++, IPGenCRVObject(CrvW));
	if (CrvX)
	    IPListObjectInsert(RetVal, i++, IPGenCRVObject(CrvX));
	if (CrvY)
	    IPListObjectInsert(RetVal, i++, IPGenCRVObject(CrvY));
	if (CrvZ)
	    IPListObjectInsert(RetVal, i++, IPGenCRVObject(CrvZ));
	IPListObjectInsert(RetVal, i++, NULL);
    }
    else if (IP_IS_SRF_OBJ(FreeForm)) {
	CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ;

	SymbSrfSplitScalar(FreeForm -> U.Srfs, &SrfW, &SrfX, &SrfY, &SrfZ);
	RetVal = IPGenLISTObject(NULL);
	if (SrfW)
	    IPListObjectInsert(RetVal, i++, IPGenSRFObject(SrfW));
	if (SrfX)
	    IPListObjectInsert(RetVal, i++, IPGenSRFObject(SrfX));
	if (SrfY)
	    IPListObjectInsert(RetVal, i++, IPGenSRFObject(SrfY));
	if (SrfZ)
	    IPListObjectInsert(RetVal, i++, IPGenSRFObject(SrfZ));
	IPListObjectInsert(RetVal, i++, NULL);
    }
    else if (IP_IS_MVAR_OBJ(FreeForm)) {
	MvarMVStruct *MVScalars[MVAR_MAX_PT_SIZE];

	MVAR_SPLIT_SCALARS(FreeForm -> U.MultiVars, MVScalars);

	RetVal = IPGenLISTObject(NULL);
	if (MVScalars[0])
	    IPListObjectInsert(RetVal, i++, IPGenMULTIVARObject(MVScalars[0]));
	for (j = 1; j <= MVAR_MAX_PT_COORD && MVScalars[j] != NULL; j++) {
	    if (MVScalars[j])
		IPListObjectInsert(RetVal, i++,
				   IPGenMULTIVARObject(MVScalars[j]));
	    else
		break;
	}
	IPListObjectInsert(RetVal, i++, NULL);
    }
    else
	IRIT_FATAL_ERROR("FFSplit: Invalid freeform to split!");

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Merges a given list of freeforms into a single freeform with point type  M
* PtType. All the components in ScalarFreeFormList must be E1.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ScalarFreeFormList: To merge into a single freeform of point type RPType.M
*   RPType:		To merge this list of scalar freeforms into.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A single freeforms of type RPType.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   FreeFormSplitScalar                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormMergeScalar                                                      M
*****************************************************************************/
IPObjectStruct *FreeFormMergeScalar(IPObjectStruct *ScalarFreeFormList,
				    IrtRType *RPType)
{
    IPObjStructType
	ObjType;
    IPObjectStruct *PObj,
	*RetVal = NULL;
    int i, j,
	PType = IRIT_REAL_PTR_TO_INT(RPType);

    ObjType = IPListObjectGet(ScalarFreeFormList, 0) -> ObjType;
    if (ObjType != IP_OBJ_CURVE &&
	ObjType != IP_OBJ_SURFACE &&
	ObjType != IP_OBJ_MULTIVAR) {
	IRIT_NON_FATAL_ERROR("FFMerge: Only curves, surfaces, or multivariates!");
	return NULL;
    }

    for (i = 1; (PObj = IPListObjectGet(ScalarFreeFormList, i)) != NULL; i++) {
	if (PObj -> ObjType != ObjType) {
	    IRIT_NON_FATAL_ERROR("FFMerge: Different freeforms in list!");
	    return NULL;
	}
    }

    if (CAGD_IS_RATIONAL_PT(PType))
        i--;
    if (i != CAGD_NUM_OF_PT_COORD(PType)) {
	IRIT_NON_FATAL_ERROR("FFMerge: Incorrect number of freeforms in list!");
	return NULL;
    }

    if (ObjType == IP_OBJ_CURVE) {
	CagdCrvStruct *Crv, *Crvs[CAGD_MAX_PT_SIZE];

	i = j = 0;
	if (CAGD_IS_RATIONAL_PT(PType))
	    Crvs[i++] = IPListObjectGet(ScalarFreeFormList, j++) -> U.Crvs;
	else
	    Crvs[i++] = NULL;

	while ((PObj = IPListObjectGet(ScalarFreeFormList, j++)) != NULL &&
	       i < CAGD_MAX_PT_SIZE)
	    Crvs[i++] = PObj -> U.Crvs;

	while (i < CAGD_MAX_PT_SIZE)
	    Crvs[i++] = NULL;

	if ((Crv = SymbCrvMergeScalar(Crvs[0], Crvs[1],
				      Crvs[2], Crvs[3])) != NULL)
	    RetVal = IPGenCRVObject(Crv);
    }
    else if (ObjType == IP_OBJ_SURFACE) {
	CagdSrfStruct *Srf, *Srfs[CAGD_MAX_PT_SIZE];

	i = j = 0;
	if (CAGD_IS_RATIONAL_PT(PType))
	    Srfs[i++] = IPListObjectGet(ScalarFreeFormList, j++) -> U.Srfs;
	else
	    Srfs[i++] = NULL;

	while ((PObj = IPListObjectGet(ScalarFreeFormList, j++)) != NULL &&
	       i < CAGD_MAX_PT_SIZE)
	    Srfs[i++] = PObj -> U.Srfs;

	while (i < CAGD_MAX_PT_SIZE)
	    Srfs[i++] = NULL;

	if ((Srf = SymbSrfMergeScalar(Srfs[0], Srfs[1],
				      Srfs[2], Srfs[3])) != NULL)
	    RetVal = IPGenSRFObject(Srf);
    }
    else if (ObjType == IP_OBJ_MULTIVAR) {
	MvarMVStruct *MV, *MVs[MVAR_MAX_PT_SIZE];

	i = j = 0;
	if (MVAR_IS_RATIONAL_PT(PType))
	    MVs[i++] = IPListObjectGet(ScalarFreeFormList, j++) -> U.MultiVars;
	else
	    MVs[i++] = NULL;

	while ((PObj = IPListObjectGet(ScalarFreeFormList, j++)) != NULL &&
	       i < MVAR_MAX_PT_SIZE)
	    MVs[i++] = PObj -> U.MultiVars;

	while (i < MVAR_MAX_PT_SIZE)
	    MVs[i++] = NULL;

	if ((MV = MvarMVMergeScalar(MVs)) != NULL)
	    RetVal = IPGenMULTIVARObject(MV);
    }
    else
	IRIT_FATAL_ERROR("FFSplit: Invalid freeform to split!");

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the point type of a given freeform.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To return its point type.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A numeric object with the point type of Freeform.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormIrtPtType                                                        M
*****************************************************************************/
IPObjectStruct *FreeFormIrtPtType(IPObjectStruct *FreeForm)
{
    IrtRType
        R = 0.0;

    if (IP_IS_CRV_OBJ(FreeForm))
	R = FreeForm -> U.Crvs -> PType;
    else if (IP_IS_SRF_OBJ(FreeForm))
	R = FreeForm -> U.Srfs -> PType;
    else if (IP_IS_TRIVAR_OBJ(FreeForm))
	R = FreeForm -> U.Trivars -> PType;
    else if (IP_IS_TRIMSRF_OBJ(FreeForm))
	R = FreeForm -> U.TrimSrfs -> Srf -> PType;
    else if (IP_IS_TRISRF_OBJ(FreeForm))
	R = FreeForm -> U.TriSrfs -> PType;
    else if (IP_IS_MVAR_OBJ(FreeForm))
	R = FreeForm -> U.MultiVars -> PType;
    else
	IRIT_FATAL_ERROR("FFPtType: Invalid freeform to extract point type from!");

    return IPGenNUMValObject(R);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the point type of a given freeform.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To return its point type.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A numeric object with the point type of Freeform.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormGeomType                                                         M
*****************************************************************************/
IPObjectStruct *FreeFormGeomType(IPObjectStruct *FreeForm)
{
    int GType;

    if (IP_IS_CRV_OBJ(FreeForm))
	GType = FreeForm -> U.Crvs -> GType;
    else if (IP_IS_SRF_OBJ(FreeForm))
	GType = FreeForm -> U.Srfs -> GType;
    else if (IP_IS_TRIVAR_OBJ(FreeForm))
	GType = FreeForm -> U.Trivars -> GType;
    else if (IP_IS_TRIMSRF_OBJ(FreeForm))
	GType = FreeForm -> U.TrimSrfs -> Srf -> GType;
    else if (IP_IS_TRISRF_OBJ(FreeForm))
	GType = FreeForm -> U.TriSrfs -> GType;
    else if (IP_IS_MVAR_OBJ(FreeForm))
	GType = FreeForm -> U.MultiVars -> GType;
    else {
	IRIT_FATAL_ERROR("FFGType: Invalid freeform to extract geometry type from!");
	GType = -1;
    }

    switch (GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_SBEZIER_TYPE:
	case TRIV_TVBEZIER_TYPE:
	case MVAR_BEZIER_TYPE:
	case TRNG_TRISRF_BEZIER_TYPE:
	    return IPGenNUMValObject(FF_BEZIER_TYPE);

	case CAGD_CBSPLINE_TYPE:
	case CAGD_SBSPLINE_TYPE:
	case TRIV_TVBSPLINE_TYPE:
	case MVAR_BSPLINE_TYPE:
	case TRNG_TRISRF_BSPLINE_TYPE:
	    return IPGenNUMValObject(FF_BSPLINE_TYPE);

	case CAGD_CPOWER_TYPE:
	case CAGD_SPOWER_TYPE:
	case TRIV_TVPOWER_TYPE:
	case MVAR_POWER_TYPE:
	    return IPGenNUMValObject(FF_POWER_TYPE);

	default:
	    IRIT_FATAL_ERROR("FFGType: Invalid geometry type!");
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the order(s) of a given freeform.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To return its order(s).                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with the order(s) of Freeform.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormOrder                                                            M
*****************************************************************************/
IPObjectStruct *FreeFormOrder(IPObjectStruct *FreeForm)
{
    int i = 0;
    IPObjectStruct
	*RetVal = IPGenLISTObject(NULL);

    if (IP_IS_CRV_OBJ(FreeForm))
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Crvs -> Order));
    else if (IP_IS_SRF_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Srfs -> UOrder));
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Srfs -> VOrder));
    }
    else if (IP_IS_TRIVAR_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Trivars -> UOrder));
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Trivars -> VOrder));
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Trivars -> WOrder));
    }
    else if (IP_IS_TRIMSRF_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
		 IPGenNUMValObject(FreeForm -> U.TrimSrfs -> Srf -> UOrder));
        IPListObjectInsert(RetVal, i++,
		 IPGenNUMValObject(FreeForm -> U.TrimSrfs -> Srf -> VOrder));
    }
    else if (IP_IS_TRISRF_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
			 IPGenNUMValObject(FreeForm -> U.TriSrfs -> Order));
    }
    else if (IP_IS_MVAR_OBJ(FreeForm)) {
	MvarMVStruct
	    *MV = FreeForm -> U.MultiVars;
	
	for ( ; i < MV -> Dim; i++)
	    IPListObjectInsert(RetVal, i,
			       IPGenNUMValObject(MV -> Orders[i]));
    }
    else {
	IRIT_FATAL_ERROR("FFOrder: Invalid freeform to extract order(s) from!");
	IPFreeObject(RetVal);
	return NULL;
    }

    IPListObjectInsert(RetVal, i++, NULL);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the control polygon/mesh size(s) of a given freeform.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To return its control polygon/mesh size(s).                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with the control polygon/mesh size(s)   M
*			of Freeform.				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormMeshSize                                                         M
*****************************************************************************/
IPObjectStruct *FreeFormMeshSize(IPObjectStruct *FreeForm)
{
    int i = 0;
    IPObjectStruct
	*RetVal = IPGenLISTObject(NULL);

    if (IP_IS_CRV_OBJ(FreeForm))
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Crvs -> Length));
    else if (IP_IS_SRF_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Srfs -> ULength));
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.Srfs -> VLength));
    }
    else if (IP_IS_TRIVAR_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
			  IPGenNUMValObject(FreeForm -> U.Trivars -> ULength));
        IPListObjectInsert(RetVal, i++,
			  IPGenNUMValObject(FreeForm -> U.Trivars -> VLength));
        IPListObjectInsert(RetVal, i++,
			  IPGenNUMValObject(FreeForm -> U.Trivars -> WLength));
    }
    else if (IP_IS_TRIMSRF_OBJ(FreeForm)) {
        IPListObjectInsert(RetVal, i++,
		 IPGenNUMValObject(FreeForm -> U.TrimSrfs -> Srf -> ULength));
        IPListObjectInsert(RetVal, i++,
		 IPGenNUMValObject(FreeForm -> U.TrimSrfs -> Srf -> VLength));
    }
    else if (IP_IS_TRISRF_OBJ(FreeForm))
        IPListObjectInsert(RetVal, i++,
			   IPGenNUMValObject(FreeForm -> U.TriSrfs -> Length));
    else if (IP_IS_MVAR_OBJ(FreeForm)) {
	MvarMVStruct
	    *MV = FreeForm -> U.MultiVars;
	
	for ( ; i < MV -> Dim; i++)
	    IPListObjectInsert(RetVal, i,
			       IPGenNUMValObject(MV -> Lengths[i]));
    }
    else {
	IRIT_FATAL_ERROR("FFMSize: Invalid freeform to extract length(s) from!");
	IPFreeObject(RetVal);
	return NULL;
    }

    IPListObjectInsert(RetVal, i++, NULL);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns on list object created out of the given knot vector sequence.    *
*                                                                            *
* PARAMETERS:                                                                *
*   KV:   	To knot vector.						     *
*   KVLen:	The length of KV					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A list object with the knot vector.                   *
*****************************************************************************/
static IPObjectStruct *KnotVectorToListObj(CagdRType *KV, int KVLen)
{
    int i;
    IPObjectStruct
	*RetVal = IPGenLISTObject(NULL);

    for (i = 0; i < KVLen; i++)
        IPListObjectInsert(RetVal, i, IPGenNUMValObject(KV[i]));

    IPListObjectInsert(RetVal, i++, NULL);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the knot vector(s) of a given freeform.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To return its knot vector(s).				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with the knot vector(s) of Freeform.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormKnotVector                                                       M
*****************************************************************************/
IPObjectStruct *FreeFormKnotVector(IPObjectStruct *FreeForm)
{
    int i = 0;
    IPObjectStruct
	*RetVal = IPGenLISTObject(NULL);

    if (IP_IS_CRV_OBJ(FreeForm) && CAGD_IS_BSPLINE_CRV(FreeForm -> U.Crvs))
        IPListObjectInsert(RetVal, i++,
		KnotVectorToListObj(FreeForm -> U.Crvs -> KnotVector,
				    FreeForm -> U.Crvs -> Order +
				      CAGD_CRV_PT_LST_LEN(FreeForm -> U.Crvs)));
    else if (IP_IS_SRF_OBJ(FreeForm) &&
	     CAGD_IS_BSPLINE_SRF(FreeForm -> U.Srfs)) {
        IPListObjectInsert(RetVal, i++,
		KnotVectorToListObj(FreeForm -> U.Srfs -> UKnotVector,
				    FreeForm -> U.Srfs -> UOrder +
				      CAGD_SRF_UPT_LST_LEN(FreeForm -> U.Srfs)));
        IPListObjectInsert(RetVal, i++,
		KnotVectorToListObj(FreeForm -> U.Srfs -> VKnotVector,
				    FreeForm -> U.Srfs -> VOrder +
				      CAGD_SRF_VPT_LST_LEN(FreeForm -> U.Srfs)));
    }
    else if (IP_IS_TRIVAR_OBJ(FreeForm) &&
	     TRIV_IS_BSPLINE_TV(FreeForm -> U.Trivars)) {
        IPListObjectInsert(RetVal, i++,
		KnotVectorToListObj(FreeForm -> U.Trivars -> UKnotVector,
				    FreeForm -> U.Trivars -> UOrder +
				      TRIV_TV_UPT_LST_LEN(FreeForm -> U.Trivars)));
        IPListObjectInsert(RetVal, i++,
		 KnotVectorToListObj(FreeForm -> U.Trivars -> VKnotVector,
				     FreeForm -> U.Trivars -> VOrder +
				      TRIV_TV_VPT_LST_LEN(FreeForm -> U.Trivars)));
        IPListObjectInsert(RetVal, i++,
		 KnotVectorToListObj(FreeForm -> U.Trivars -> WKnotVector,
				     FreeForm -> U.Trivars -> WOrder +
				      TRIV_TV_WPT_LST_LEN(FreeForm -> U.Trivars)));
    }
    else if (IP_IS_TRIMSRF_OBJ(FreeForm) &&
	     CAGD_IS_BSPLINE_SRF(FreeForm -> U.TrimSrfs -> Srf)) {
        IPListObjectInsert(RetVal, i++,
	    KnotVectorToListObj(FreeForm -> U.TrimSrfs -> Srf -> UKnotVector,
				FreeForm -> U.TrimSrfs -> Srf -> UOrder +
				  CAGD_SRF_UPT_LST_LEN(FreeForm -> U.TrimSrfs
						                      -> Srf)));
        IPListObjectInsert(RetVal, i++,
	    KnotVectorToListObj(FreeForm -> U.TrimSrfs -> Srf -> VKnotVector,
				FreeForm -> U.TrimSrfs -> Srf -> VOrder +
				  CAGD_SRF_VPT_LST_LEN(FreeForm -> U.TrimSrfs
						                      -> Srf)));
    }
    else if (IP_IS_TRISRF_OBJ(FreeForm) &&
	TRNG_IS_BSPLINE_TRISRF(FreeForm -> U.TriSrfs)) {
        IPListObjectInsert(RetVal, i++,
		KnotVectorToListObj(FreeForm -> U.TriSrfs -> KnotVector,
				    FreeForm -> U.TriSrfs -> Order +
				        FreeForm -> U.TriSrfs -> Length));
    }
    else if (IP_IS_MVAR_OBJ(FreeForm)) {
	MvarMVStruct
	    *MV = FreeForm -> U.MultiVars;
	
	for ( ; i < MV -> Dim; i++)
	    IPListObjectInsert(RetVal, i,
		       KnotVectorToListObj(MV -> KnotVectors[i],
					   MV -> Orders[i] +
					     MVAR_MVAR_ITH_PT_LST_LEN(MV, i)));
    }
    else {
	IRIT_FATAL_ERROR("FFKntVec: Invalid freeform to extract knot vector(s) from!");
	IPFreeObject(RetVal);
	return NULL;
    }

    IPListObjectInsert(RetVal, i++, NULL);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the control points of a given freeform.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForm:   To return its control points.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with the control points of Freeform.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeFormControlPoints                                                    M
*****************************************************************************/
IPObjectStruct *FreeFormControlPoints(IPObjectStruct *FreeForm)
{
    int i, Length;
    CagdPointType PType;
    IPObjectStruct
	*RetVal = IPGenLISTObject(NULL);
    CagdRType **Points;

    if (IP_IS_CRV_OBJ(FreeForm)) {
	PType = FreeForm -> U.Crvs -> PType;
	Points = FreeForm -> U.Crvs -> Points;
	Length = FreeForm -> U.Crvs -> Length;
    }
    else if (IP_IS_SRF_OBJ(FreeForm)) {
	PType = FreeForm -> U.Srfs -> PType;
	Points = FreeForm -> U.Srfs -> Points;
	Length = FreeForm -> U.Srfs -> ULength * FreeForm -> U.Srfs -> VLength;
    }
    else if (IP_IS_TRIVAR_OBJ(FreeForm)) {
	PType = FreeForm -> U.Trivars -> PType;
	Points = FreeForm -> U.Trivars -> Points;
	Length = FreeForm -> U.Trivars -> ULength *
	         FreeForm -> U.Trivars -> VLength *
	         FreeForm -> U.Trivars -> WLength;
    }
    else if (IP_IS_TRIMSRF_OBJ(FreeForm)) {
	PType = FreeForm -> U.TrimSrfs -> Srf -> PType;
	Points = FreeForm -> U.TrimSrfs -> Srf -> Points;
	Length = FreeForm -> U.TrimSrfs -> Srf -> ULength *
	         FreeForm -> U.TrimSrfs -> Srf -> VLength;
    }
    else if (IP_IS_TRISRF_OBJ(FreeForm)) {
	PType = FreeForm -> U.TriSrfs -> PType;
	Points = FreeForm -> U.TriSrfs -> Points;
	Length = TRNG_TRISRF_MESH_SIZE(FreeForm -> U.TriSrfs);
    }
    else if (IP_IS_MVAR_OBJ(FreeForm)) {
	PType = (CagdPointType) FreeForm -> U.MultiVars -> PType;
	Points = FreeForm -> U.MultiVars -> Points;
	Length = MVAR_CTL_MESH_LENGTH(FreeForm -> U.MultiVars);
    }
    else {
	IRIT_FATAL_ERROR("FFCtlPts: Invalid freeform to extract control points from!");
	IPFreeObject(RetVal);
	return NULL;
    }

    /* Copy the points themslves into the object list. */
    for (i = 0; i < Length; i++) {
	int j;
	CagdRType Coords[CAGD_MAX_PT_SIZE];

	if (CAGD_IS_RATIONAL_PT(PType))
	    Coords[0] = Points[0][i];
	for (j = 1; j <= CAGD_NUM_OF_PT_COORD(PType); j++)
	    Coords[j] = Points[j][i];
	
        IPListObjectInsert(RetVal, i, IPGenCTLPTObject(PType, Coords));
    }

    IPListObjectInsert(RetVal, i++, NULL);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the convex hull of the given data set, which can be either      M
* curves or polygon/polyline/points, all planar.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Geom:        A curve or a polygon/line/points.                           M
*   FineNess:    If curve is given, the accuracy of the numeric computation. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An object representing the convex hull of Geom.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   ComputeMinSpanCirc, CrvTwoTangents, CrvPointTangents                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeConvexHull, convex hull                                           M
*****************************************************************************/
IPObjectStruct *ComputeConvexHull(IPObjectStruct *Geom, IrtRType *FineNess)
{
    if (IP_IS_CRV_OBJ(Geom)) {
	CagdCrvStruct
	   *CHCrv = SymbCrvCnvxHull(Geom -> U.Crvs, *FineNess);

	return CHCrv == NULL ? NULL : IPGenCRVObject(CHCrv);
    }
    else if (IP_IS_SRF_OBJ(Geom) || IP_IS_OLST_OBJ(Geom)) {
        IRIT_NON_FATAL_ERROR("Convex hull of surfaces is not supported");
	return NULL;
    }
    else if (IP_IS_POLY_OBJ(Geom)) {
	int i,
	    Circular = FALSE;
	IPVertexStruct *VLast, *VHead,
	    *V = Geom -> U.Pl -> PVertex;
	int Len = IPVrtxListLen(V);
	IrtE2PtStruct
	    *DTPts = (IrtE2PtStruct *) IritMalloc(sizeof(IrtE2PtStruct) * Len);

	for (i = 0; i < Len; i++, V = V -> Pnext) {
	    DTPts[i].Pt[0] = V -> Coord[0];
	    DTPts[i].Pt[1] = V -> Coord[1];
	}

	if (!GMConvexHull(DTPts, &Len)) {
	    IritFree(DTPts);
	    return NULL;
	}

	/* Make an object of same type: point list/polygon/polyline. */
	Geom = IPCopyObject(NULL, Geom, TRUE);
	IP_CAT_OBJ_NAME(Geom, "CH");

	/* Copy the convex hull set into it. */
	V = VHead = Geom -> U.Pl -> PVertex;
	if (IP_IS_POLYGON_OBJ(Geom)) {
	    VLast = IPGetLastVrtx(V);
	    Circular = (VLast -> Pnext != NULL);
	    VLast -> Pnext = NULL;
	}

	for (i = 0; i < Len && V != NULL; ) {
	    V -> Coord[0] = DTPts[i].Pt[0];
	    V -> Coord[1] = DTPts[i].Pt[1];
	    V -> Coord[2] = 0;

	    if (IP_IS_POLYGON_OBJ(Geom))
		IRIT_PT_COPY(V -> Normal, Geom -> U.Pl -> Plane);

	    if (++i < Len)
		V = V -> Pnext;
	}

	IritFree(DTPts);
	if (V != NULL) {
	    IPFreeVertexList(V -> Pnext);
	    V -> Pnext = NULL;
	}
	if (Circular) {
	    VLast = IPGetLastVrtx(VHead);
	    VLast -> Pnext = VHead;
	}

	return Geom;
    }
    else
	IRIT_FATAL_ERROR("CNVXHULL: Invalid type of input geometry!");

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimum spanning cone (MSCONE) of the given data set, which M
* are either vector (in R^3) or ctlpts (in R^n) objects.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Geom:        A list of vector objects. 		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An object representing the MSCONE of Geom.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   ComputeConvexHull, ComputeMinSpanCirc                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeMinSpanCone                                                       M
*****************************************************************************/
IPObjectStruct *ComputeMinSpanCone(IPObjectStruct *Geom)
{
    if (IP_IS_OLST_OBJ(Geom)) {
	int MVDim = 0,
	    i = 0,
	    Len = 0,
	    CreateConeSrf = FALSE,
	    UseAvg = !IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(Geom,
								"UseAvg")),
	    ListLen = IPListObjectLength(Geom);
	IrtRType
	    Angle = 0.0;
	CagdPointType
	    PtType = CAGD_PT_NONE;
	IPObjStructType
	    ObjType = IP_OBJ_UNDEF;
	IrtVecType V, ConeCenter,
	    *DTVecs = NULL;
	MvarNormalConeStruct
	    *MVCone = NULL;
	MvarVecStruct
	    *MVVecs = NULL;
	IPObjectStruct *Obj,
	    *MSC = NULL;

	if (ListLen < 1) {
	    IRIT_NON_FATAL_ERROR("Empty input MSCone list");
	    return NULL;
	}

	/* Process the input list of vector objects or ctlpt objects. */
	while ((Obj = IPListObjectGet(Geom, Len++)) != NULL) {
	    if (i == 0) {
	        ObjType = Obj -> ObjType;

		if (IP_IS_VEC_OBJ(Obj))
		    DTVecs = (IrtVecType *) IritMalloc(sizeof(IrtVecType)
						                   * ListLen);
		else if (IP_IS_CTLPT_OBJ(Obj)) {
		    PtType = Obj -> U.CtlPt.PtType;
		    MVDim = CAGD_NUM_OF_PT_COORD(PtType);
		    MVVecs = MvarVecArrayNew(ListLen, MVDim);
		}
		else {
		    IRIT_NON_FATAL_ERROR("Only vectors/ctlpts expected in MSCone list");
		    return NULL;
		}
	    }
	    else if (ObjType != Obj -> ObjType) {
		IRIT_NON_FATAL_ERROR("Different object types in MSCone list");
		if (DTVecs != NULL)
		    IritFree(DTVecs);
		if (MVVecs != NULL)
		    MvarVecArrayFree(MVVecs, ListLen);
		return NULL;
	    }

	    if (IP_IS_VEC_OBJ(Obj)) {
		DTVecs[i][0] = Obj -> U.Vec[0];
		DTVecs[i][1] = Obj -> U.Vec[1];
		DTVecs[i++][2] = Obj -> U.Vec[2];
	    }
	    else if (IP_IS_CTLPT_OBJ(Obj)) {
	        IRIT_GEN_COPY(MVVecs[i].Vec, &Obj -> U.CtlPt.Coords[1],
			      MVDim * sizeof(CagdRType));
		MVVecs[i++].Dim = MVDim;
	    }
	}

	if (DTVecs != NULL) {
	    if (UseAvg) {
	        if (!GMMinSpanConeAvg(DTVecs, FALSE, i, ConeCenter, &Angle)) {
		    IritFree(DTVecs);
		    return NULL;
		}
	    }
	    else {
		if (!GMMinSpanCone(DTVecs, FALSE, i, ConeCenter, &Angle)) {
		    IritFree(DTVecs);
		    return NULL;
		}
	    }

	    IritFree(DTVecs);

	    CreateConeSrf = TRUE;
	}
	else if (MVVecs != NULL) {
	    MVCone = MvarNormalConeNew(MVDim);

	    if (UseAvg) {
	        if (!MvarMinSpanConeAvg(MVVecs, FALSE, ListLen, MVCone)) {
		    IritFree(MVVecs);
		    return NULL;
		}
	    }
	    else {
		if (!MvarMinSpanCone(MVVecs, FALSE, ListLen, MVCone)) {
		    IritFree(MVVecs);
		    return NULL;
		}
	    }

	    IritFree(MVVecs);

	    if ((CreateConeSrf = (MVDim == 3)) != FALSE) {
	        IRIT_PT_COPY(ConeCenter, MVCone -> ConeAxis -> Vec);
		Angle = acos(MVCone -> ConeAngleCosine);
	    }
	}
	else {
	    assert(0);
	}

	if (CreateConeSrf) {		   /* Build viewable cone geometry. */
	    IRIT_STATIC_DATA CagdPtStruct
	        PtOrigin = { NULL, NULL, { 0, 0, 0 } };
	    CagdPtStruct PtConeAxis;
	    IrtHmgnMatType Mat, Mat2;
	    IPObjectStruct *PTmp;

	    V[0] = V[1] = 0.0;
	    V[2] = -1.0;
	    PTmp = IPGenSRFObject(CagdPrimConeSrf(V,
						  tan(Angle > M_PI_DIV_2
						      ? M_PI - Angle : Angle),
						  1.0, TRUE,
						  CAGD_PRIM_CAPS_NONE));
	    if (Angle > M_PI_DIV_2) {
	        IRIT_PT_COPY(V, ConeCenter);
		IRIT_PT_SCALE(V, -1.0);
	        GMGenMatrixZ2Dir(Mat, V);
	    }
	    else
	        GMGenMatrixZ2Dir(Mat, ConeCenter);
	    MatGenMatRotX(-1.0, 0.0, Mat2);
	    MatMultTwo4by4(Mat2, Mat2, Mat);

	    MSC = IPGenLISTObject(GMTransformObject(PTmp, Mat2));
	    IPFreeObject(PTmp);

	    /* Place an axis to the cone. */
	    IRIT_PT_COPY(PtConeAxis.Pt, ConeCenter);
	    IRIT_PT_SCALE(PtConeAxis.Pt, 1.5);
	    PTmp = IPGenCRVObject(CagdMergePtPt(&PtOrigin, &PtConeAxis));
	    IPListObjectAppend(MSC, PTmp);

	    AttrSetObjectRealAttrib(MSC, "angle", Angle);
	    AttrSetObjectObjAttrib(MSC, "center",
				   IPGenPTObject(&ConeCenter[0],
						 &ConeCenter[1],
						 &ConeCenter[2]),
				   FALSE);
	}
	else { /* M hyper cone in R^n - return a list object of axis/angle. */
	    MSC = IPGenLISTObject(IPGenCTLPTObject(PtType,
					     &MVCone -> ConeAxis -> Vec[-1]));
	    IPListObjectAppend(MSC,
			  IPGenNUMValObject(acos(MVCone -> ConeAngleCosine))); 
	}

	if (MVCone != NULL)
	    MvarNormalConeFree(MVCone);

	return MSC;
    }
    else
	IRIT_FATAL_ERROR("MSCone: Invalid type of input geometry, expecting list of objects!");

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimum spanning circle (MSC) of the given data set, which  M
* can be polygon/polyline/points, all planar.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Geom:        A polygon/line/points/curves. 		                     M
*   Tols:        Of computation (only if curves, nil otherwise).  If         M
*		 specified, can be either the Nuerical Tolerance or a list   M
*		 of two numbers (Subdiv, Numeric) tolernces.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An object representing the MSC of Geom.  The center M
*			 and radius of the MSC are placed as "center" and    M
*			 "radius" attribute on this returned object.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   ComputeConvexHull, ComputeMinSpanCone	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeMinSpanCirc, minimum spanning circle                              M
*****************************************************************************/
IPObjectStruct *ComputeMinSpanCirc(IPObjectStruct *Geom, IPObjectStruct *Tols)
{
    if (IP_IS_POLY_OBJ(Geom)) {
	IrtRType Radius;
	IPVertexStruct
	    *V = Geom -> U.Pl -> PVertex;
	int i,
	    Len = IPVrtxListLen(V);
	IrtE2PtStruct Center,
	    *DTPts = (IrtE2PtStruct *) IritMalloc(sizeof(IrtE2PtStruct) * Len);
	IPObjectStruct
	    *MSC = NULL;
	CagdPtStruct CagdCenter;

	if (Len < 1) {
	    IRIT_NON_FATAL_ERROR("Empty input MSCirc list");
	    return NULL;
	}

	for (i = 0; i < Len; i++, V = V -> Pnext) {
	    DTPts[i].Pt[0] = V -> Coord[0];
	    DTPts[i].Pt[1] = V -> Coord[1];
	}

	if (!GMMinSpanCirc(DTPts, Len, &Center, &Radius)) {
	    IritFree(DTPts);
	    return NULL;
	}

	IritFree(DTPts);

	CagdCenter.Pt[0] = Center.Pt[0];
	CagdCenter.Pt[1] = Center.Pt[1];
	CagdCenter.Pt[2] = 0.0;
	MSC = IPGenCRVObject(BspCrvCreateCircle(&CagdCenter, Radius));
	
	AttrSetObjectRealAttrib(MSC, "radius", Radius);
	AttrSetObjectObjAttrib(MSC, "center",
			       IPGenPTObject(&CagdCenter.Pt[0],
					     &CagdCenter.Pt[1],
					     &CagdCenter.Pt[2]),
			       FALSE);
	return MSC;
    }
    else if (IP_IS_OLST_OBJ(Geom)) {
	int Len = 0,
	    ListLen = IPListObjectLength(Geom);
	IPObjectStruct *Obj,
	    *MSC = NULL;

	if (ListLen < 1) {
	    IRIT_NON_FATAL_ERROR("Empty input MSCirc list");
	    return NULL;
	}

	/* Should hold curve objs. */
	while ((Obj = IPListObjectGet(Geom, Len++)) != NULL) {
	    if (!IP_IS_CRV_OBJ(Obj)) {
		IRIT_NON_FATAL_ERROR("Non curve object in MSCirc list");
		return NULL;
	    }
	}

	/* Min Spanning circle of curves. */
	if (Geom != NULL) {
	    CagdRType Radius, SubdivTol, NumerTol;
	    CagdPType Center;
	    CagdPtStruct CagdCenter;

	    if (IP_IS_OLST_OBJ(Tols)) {
	        IPObjectStruct
		    *PObj1 = IPListObjectGet(Tols, 0),
		    *PObj2 = IPListObjectGet(Tols, 1);

		if (IP_IS_NUM_OBJ(PObj1))
		    SubdivTol = PObj1 -> U.R;
		else
		    SubdivTol = IRIT_MSC_DEF_SUBDIV_EPS;

		if (IP_IS_NUM_OBJ(PObj2))
		    NumerTol = PObj2 -> U.R;
		else
		    NumerTol = IRIT_MSC_DEF_NUMER_EPS;
	    }
	    else if (IP_IS_NUM_OBJ(Tols)) {
	        NumerTol = Tols -> U.R;
		SubdivTol = IRIT_MSC_DEF_SUBDIV_EPS;
	    }
	    else {
		NumerTol = IRIT_MSC_DEF_NUMER_EPS;
		SubdivTol = IRIT_MSC_DEF_SUBDIV_EPS;
	    }

	    MvarMinSpanCirc(Geom, Center, &Radius, SubdivTol, NumerTol);

	    CagdCenter.Pt[0] = Center[0];
	    CagdCenter.Pt[1] = Center[1];
	    CagdCenter.Pt[2] = 0.0;
	    MSC = IPGenCRVObject(BspCrvCreateCircle(&CagdCenter, Radius));

	    AttrSetObjectRealAttrib(MSC, "radius", Radius);
	    AttrSetObjectObjAttrib(MSC, "center",
				   IPGenPTObject(&CagdCenter.Pt[0],
						 &CagdCenter.Pt[1],
						 &CagdCenter.Pt[2]),
				   FALSE);
	}
	else
	    IRIT_NON_FATAL_ERROR("Non curve object in MSCirc list");

	return MSC;
    }
    else
	IRIT_FATAL_ERROR("MSCirc: Invalid type of input geometry!");

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the minimum spanning sphere (MSS) of the given data set, which  M
* can be points.		                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Geom:        Points to derive MSS for. 		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An object representing the MSS of Geom.  The center M
*			 and radius of the MSS are placed as "center" and    M
*			 "radius" attribute on this returned object.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   ComputeConvexHull, ComputeMinSpanCone, ComputeMinSpanCirc                M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeMinSpanSphere, minimum spanning sphere                            M
*****************************************************************************/
IPObjectStruct *ComputeMinSpanSphere(IPObjectStruct *Geom)
{
    if (IP_IS_OLST_OBJ(Geom)) {
	int Len = 0,
	    ListLen = IPListObjectLength(Geom);
	IPObjectStruct *Obj;
	IrtE3PtStruct Cent,
	    *DTPts = NULL;
	IPObjectStruct *MSS;
	CagdRType Radius;
	IrtVecType SphereCenter;

	if (ListLen < 1) {
	    IRIT_NON_FATAL_ERROR("Empty input MSSphere list");
	    return NULL;
	}

	/* Verify the input. */
	while ((Obj = IPListObjectGet(Geom, Len)) != NULL) {
	    if (IP_IS_POINT_OBJ(Obj)) {
	        if (DTPts == NULL)
		    DTPts = (IrtE3PtStruct *)
		        IritMalloc(sizeof(IrtE3PtStruct) * ListLen);
		
		IRIT_PT_COPY(DTPts[Len].Pt, Obj -> U.Pt);
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Non point object in MSShpere list");
		if (DTPts != NULL)
		    IritFree(DTPts);
		return NULL;
	    }
	    Len++;
	}

	/* Computing MSS of Points. */
	if (DTPts != NULL) {
	    if (!GMMinSpanSphere(DTPts, Len, &Cent, &Radius)) {
	        IritFree(DTPts);
		return NULL;
	    }
	    IritFree(DTPts);

	    SphereCenter[0] = Cent.Pt[0];
	    SphereCenter[1] = Cent.Pt[1];
	    SphereCenter[2] = Cent.Pt[2];

	    MSS = IPGenSRFObject(CagdPrimSphereSrf(SphereCenter, 
						   Radius, FALSE));
	    
	    AttrSetObjectRealAttrib(MSS, "radius", Radius);
	    AttrSetObjectObjAttrib(MSS, "center",
				   IPGenPTObject(&SphereCenter[0],
						 &SphereCenter[1],
						 &SphereCenter[2]),
				   FALSE);
	    return MSS;
	}
	else
	    return NULL;
    }
    else
	IRIT_FATAL_ERROR("MSSphere: Invalid type of input geometry!");

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all the lines through Pt that are also tangent to the given     M
* curve Crv.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To compute all tangent lines to it through Pt.               M
*   TanPt:      Where all tangent lines should go throug.                    M
*   FineNess:   The accuracy of the numeric computation.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with all parameter values of tangents.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   ComputeConvexHull, CrvTwoTangents                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvPointTangents                                                         M
*****************************************************************************/
IPObjectStruct *CrvPointTangents(IPObjectStruct *Crv,
				 IrtPtType TanPt,
				 IrtRType *FineNess)
{
    int Count = 0;
    CagdPtStruct *Pt,
        *Pts = SymbCrvPtTangents(Crv -> U.Crvs, TanPt, *FineNess);
    IPObjectStruct
        *RetVal = IPGenLISTObject(NULL);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext)
        IPListObjectInsert(RetVal, Count++, IPGenNUMObject(&Pt -> Pt[0]));
    IPListObjectInsert(RetVal, Count, NULL);

    CagdPtFreeList(Pts);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all lines that are tangent to Crv at TWO different locations    M
* on Crv.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to compute mutual tangent lines.                      M
*   FineNess:    The accuracy of the numeric computation.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with all parameter value pairs of the   M
*		       tangents.  Each point object in list will contain     M
*		       the pair of parameter values at the X and Y	     M
*		       coordinates.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   ComputeConvexHull, CrvPointTangents, CircTangentsTwoCrvs,                M
*   SymbTangentToCrvAtTwoPts						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvTwoTangents                                                           M
*****************************************************************************/
IPObjectStruct *CrvTwoTangents(IPObjectStruct *Crv, IrtRType *FineNess)
{
    int Count = 0;
    IrtRType
	R = 0.0;
    CagdPtStruct *Pt,
        *Pts = SymbTangentToCrvAtTwoPts(Crv -> U.Crvs, *FineNess);
    IPObjectStruct
        *RetVal = IPGenLISTObject(NULL);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext)
        IPListObjectInsert(RetVal, Count++,
			   IPGenPTObject(&Pt -> Pt[0], &Pt -> Pt[1], &R));
    IPListObjectInsert(RetVal, Count, NULL);

    CagdPtFreeList(Pts);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all lines that are tangent to Crv at TWO different locations    M
* on Crv.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to find the circle that is tangent to both. M
*   Radius:       Of the circle that is tangent to Crv1/2.		     M
*   Tol:	  Tolerance of approximation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with the center locations of the        M
*		       computed circles.  Each center points has a "Params"  M
*		       attribute that holds the two parameters on original   M
*		       curves where the circle is tangent two the curves.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   CrvTwoTangents, CrvPointTangents                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CircTangentsTwoCrvs                                                      M
*****************************************************************************/
IPObjectStruct *CircTangentsTwoCrvs(IPObjectStruct *Crv1,
				    IPObjectStruct *Crv2,
				    IrtRType *Radius,
				    IrtRType *Tol)
{
    int Count = 0;
    IrtRType
	R = 0.0;
#ifdef CIRC_TAN_2CRVS_SYMB_LIB
    CagdPtStruct *Pt,
        *Pts = SymbCircTanTo2Crvs(Crv1 -> U.Crvs, Crv2 -> U.Crvs,
				  *Radius, *Tol);
    IPObjectStruct *PObj,
        *RetVal = IPGenLISTObject(NULL);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        float
	    *UV = AttrGetUVAttrib(Pt -> Attr, "Params");

        PObj = IPGenPTObject(&Pt -> Pt[0], &Pt -> Pt[1], &R);
        IPListObjectInsert(RetVal, Count++, PObj);
	
	AttrSetObjectUVAttrib(PObj, "Params", UV[0], UV[1]);
    }

    IPListObjectInsert(RetVal, Count, NULL);

    CagdPtFreeList(Pts);

    return RetVal;
#else
    MvarPtStruct *Pt,
        *Pts = MvarCircTanTo2Crvs(Crv1 -> U.Crvs, Crv2 -> U.Crvs,
				  *Radius, *Tol);
    IPObjectStruct *PObj,
        *RetVal = IPGenLISTObject(NULL);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        PObj = IPGenPTObject(&Pt -> Pt[2], &Pt -> Pt[3], &R);
        IPListObjectInsert(RetVal, Count++, PObj);
	
	AttrSetObjectUVAttrib(PObj, "Params", Pt -> Pt[0], Pt -> Pt[1]);
    }

    IPListObjectInsert(RetVal, Count, NULL);

    MvarPtFreeList(Pts);

    return RetVal;
#endif /* CIRC_TAN_2CRVS_SYMB_LIB */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all plane that are tangent to SrfList at three locations.       M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:       One or three surfaces to compute self or mutual tangents. M
*   Orientation:   0 for no effect, -1 or +1 for a request to get opposite   M
*		   or similar normal orientation bi tangencies only.         M
*   SubdivTol:     The accuracy of the subdivision computation.              M
*   NumericTol:    The accuracy of the numeric computation.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object with all parameter value pairs of the   M
*		       tangents.  Each point object in list will contain     M
*		       the six parameter values for the three surfaces.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CrvTwoTangents, CrvPointTangents                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfThreeTangents                                                         M
*****************************************************************************/
IPObjectStruct *SrfThreeTangents(IPObjectStruct *SrfList,
				 IrtRType *Orientation,
				 IrtRType *SubdivTol,
				 IrtRType *NumericTol)
{
    int Count = 0;
    IPObjectStruct
	*PObj1 = IPListObjectGet(SrfList, 0),
	*PObj2 = IPListObjectGet(SrfList, 1),
	*PObj3 = IPListObjectGet(SrfList, 2);
    MvarMVStruct *MV1, *MV2, *MV3;
    MvarPtStruct *Pt, *Pts;
    IPObjectStruct
        *RetVal = IPGenLISTObject(NULL);

    if (PObj1 == NULL || !IP_IS_SRF_OBJ(PObj1)) {
	IRIT_WNDW_PUT_STR("SRF3TANS: Expected a list of surfaces.");
	return NULL;
    }
    else
	MV1 = MvarSrfToMV(PObj1 -> U.Srfs);

    if (PObj2 == NULL) {
	MV2 = NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj2)) {
	MV2 = MvarSrfToMV(PObj2 -> U.Srfs);
    }
    else {
	MvarMVFree(MV1);
	IRIT_WNDW_PUT_STR("SRF3TANS: Expected a list of surfaces.");
	return NULL;
    }

    if (PObj2 == NULL || PObj3 == NULL) {
	MV3 = NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj3)) {
	MV3 = MvarSrfToMV(PObj3 -> U.Srfs);
    }
    else {
	MvarMVFree(MV1);
	if (MV2 != NULL)
	    MvarMVFree(MV2);
	IRIT_WNDW_PUT_STR("SRF3TANS: Expected a list of surfaces.");
	return NULL;
    }

    Pts = MvarMVTriTangents(MV1, MV2, MV3, IRIT_REAL_PTR_TO_INT(Orientation),
			    *SubdivTol, *NumericTol);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext)
        IPListObjectInsert(RetVal, Count++,
			   IPGenCTLPTObject(CAGD_PT_E6_TYPE, &Pt -> Pt[-1]));
    IPListObjectInsert(RetVal, Count, NULL);

    MvarPtFreeList(Pts);

    MvarMVFree(MV1);
    if (MV2 != NULL)
	MvarMVFree(MV2);
    if (MV3 != NULL)
	MvarMVFree(MV3);

    return RetVal;
}
