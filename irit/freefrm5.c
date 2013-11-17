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
#include "user_lib.h"
#include "geom_lib.h"
#include "objects.h"
#include "mrchcube.h"
#include "mdl_lib.h"
#include "freeform.h"

static IPObjectStruct *GetControlTriSrfMesh(IPObjectStruct *PtObjList,
					    int Length,
					    int Order,
					    TrngGeomType GType,
					    char **ErrStr);
static int Skel2D2PrimsCnvrt(IPObjectStruct *Obj, MvarSkel2DPrimStruct *Prim);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the regions of planar curve Crv that are visible from a view    M
* point (View[2] == 1) or direction (View[2] == 0) View (x, y, w).  Return   M
* is a list of visible curve segments.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Planar curve to compute its visible regions from View.        M
*   View:      As (x, y, w) where w == 0 for parallel view direction and     M
*	       w = 1 for a point view.					     M
*   Tol:       Of computation. 						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of visible curve segments of CrvObj.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvVisibleRegions                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveVisibility                                                          M
*****************************************************************************/
IPObjectStruct *CurveVisibility(IPObjectStruct *CrvObj,
				IrtVecType View,
				IrtRType *Tol)
{
    int Count = 0;
    CagdCrvStruct *Crv,
	*Crvs = UserCrvVisibleRegions(CrvObj -> U.Crvs, View, *Tol);
    IPObjectStruct *RetObj;

    if (Crvs == NULL) {
	IRIT_NON_FATAL_ERROR("CVisible: no visible curve segments detected!");
	return NULL;
    }

    RetObj = IPGenLISTObject(NULL);
    while (Crvs != NULL) {
	IRIT_LIST_POP(Crv, Crvs);
	IPListObjectInsert(RetObj, Count++, IPGenCRVObject(Crv));
    }

    IPListObjectInsert(RetObj, Count, NULL);
    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Decomposes a freeform surface into regions, represented as trimmed       M
* surfaces.  Each region has normals that deviate up to a certain amount     M
* from a prescribed viewing direction.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:        To decompose                                              M
*   Resolution:    Of polygon approximation of SrfObj.                       M
*   ConeSize:      tangent of the angular opening of the cone of normals.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  List of trimmed surfaces, each representing a         M
*		       Region that has set of normals inside the same cone.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   VisibConeDecomposition                                                   M
*****************************************************************************/
IPObjectStruct *VisibConeDecomposition(IPObjectStruct *SrfObj,
				       IrtRType *Resolution,
				       IrtRType *ConeSize)
{
    int Count = 0;
    IPObjectStruct
	*TrimmedSrfObjs = UserSrfVisibConeDecomp(SrfObj -> U.Srfs,
						 *Resolution,
						 *ConeSize),
	*ListObj = IPGenLISTObject(NULL);

    while (TrimmedSrfObjs) {
	IPObjectStruct
	    *TrimmedSrfObj = TrimmedSrfObjs;

	TrimmedSrfObjs = TrimmedSrfObjs -> Pnext;
	TrimmedSrfObj -> Pnext = NULL;
	IPListObjectInsert(ListObj, Count++, TrimmedSrfObj);
    }

    IPListObjectInsert(ListObj, Count, NULL);
    return ListObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the orthogonal/parallel/angular map of a planar curve.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Planar curve to compute the map for.			     M
*   Tolerance: Of computation.	Netagtive to return the 2D function whose    M
*	       zero set is the map.					     M
*   Angle:     0 for parallel maps, 90 for orthogonal maps, or general       M
*	       for general angles.					     M
*   RDiagExtreme:  If non zero, extracts the diagonal extremes of the OM.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The orthogonal map in [D x D] square domain where   M
*		D is Crv's domain if Tolerance > 0, or the function whose    M
*		zero set is the Orthogonal map if Tolerance <= 0.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvAngleMap, UserCrvOMDiagExtreme                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvAngleMap                                                              M
*****************************************************************************/
IPObjectStruct *CrvAngleMap(IPObjectStruct *CrvObj,
			    IrtRType *Tolerance,
			    IrtRType *Angle,
			    IrtRType *RDiagExtreme)
{
    IPObjectStruct
	*PObj = UserCrvAngleMap(CrvObj -> U.Crvs, *Tolerance, *Angle);

    if (IRIT_REAL_PTR_TO_INT(RDiagExtreme)) {
        IPObjectStruct
	    *DiagExtrm = UserCrvOMDiagExtreme(CrvObj -> U.Crvs, PObj,
					      IRIT_REAL_PTR_TO_INT(RDiagExtreme));

	IPFreeObject(PObj);

	return DiagExtrm;
    }
    else
        return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the view of a planar curve, < N(t), ViewCrv(Theta) > = 0,	     M
* N(t) being the normal field of Crv(t).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Planar curve to compute the map for.			     M
*   ViewCrvObj:  Planar viewing direction curve in vector space.	     M
*   SubTol:    Used only if TrimInvisible TRUE to control the subdivision    M
*	       Tolerance of the solver.	  If negative, the solution set is   M
*              returned as a cloud of points.				     M
*   NumTol:    Of computation.  If negative the function M whose zero set is M
*	       the view map, is returned.	 			     M
*   TrimInvisible:  If TRUE, trim the regions that are invisible from V.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The view map in [D x (0, 360)] rectangle domain     M
*		where D is Crv's domain if Tolerance > 0, or the function    M
*	        whose zero set is the Orthogonal map if Tolerance <= 0.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvAngleMap, UserCrvOMDiagExtreme                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvViewMap                                                               M
*****************************************************************************/
IPObjectStruct *CrvViewMap(IPObjectStruct *CrvObj,
			   IPObjectStruct *ViewCrvObj,
			   IrtRType *SubTol,
			   IrtRType *NumTol,
			   IrtRType *TrimInvisible)
{
    IPObjectStruct
	*PObj = UserCrvViewMap(CrvObj -> U.Crvs, ViewCrvObj -> U.Crvs,
			       *SubTol, *NumTol,
			       IRIT_REAL_PTR_TO_INT(TrimInvisible));

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes points that covers the (hemi) sphere as a honeycomb.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:    of honeycomb mesh - distance between adjacent points.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The list of points on the hemisphere.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   PointCoverOfHemiSphere                                                   M
*****************************************************************************/
IPObjectStruct *PointCoverOfHemiSphere(IrtRType *Size)
{
    int Count = 0;
    IPObjectStruct
	*PObj = GMPointCoverOfUnitHemiSphere(*Size),
	*PObjList = IPGenLISTObject(NULL);
    IPVertexStruct
	*V = PObj -> U.Pl -> PVertex;

    for ( ; V != NULL; V = V -> Pnext) {
	IPListObjectInsert(PObjList, Count++,
			   IPGenPTObject(&V -> Coord[0],
					 &V -> Coord[1],
					 &V -> Coord[2]));
    }

    IPFreeObject(PObj);

    IPListObjectInsert(PObjList, Count, NULL);

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a distribution of points in the parametric space of the free    M
* form that is statistically uniform.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeFormObj:    To compute the distribution for.                         M
*   RParamUniform:  If TRUE, produces a distribution uniform in parametric   M
*		    space. If FALSE, uniform in Euclidean space.	     M
*   RNumOfPts:      Number of points to place along the freeform object.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list of points, each represented as the parameter  M
*		value in the freeform object.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeformPointDistrib, uniform distribution                               M
*****************************************************************************/
IPObjectStruct *FreeformPointDistrib(IPObjectStruct *FreeFormObj,
				     IrtRType *RParamUniform,
				     IrtRType *RNumOfPts)
{
    int i,
	ParamUniform = IRIT_REAL_PTR_TO_INT(RParamUniform),
	NumOfPts = IRIT_REAL_PTR_TO_INT(RNumOfPts);
    CagdRType
	Zero = 0.0;
    IPObjectStruct
	*PObjList = IPGenLISTObject(NULL);

    if (IP_IS_CRV_OBJ(FreeFormObj)) {
	CagdRType
	    *R = SymbUniformAprxPtOnCrvDistrib(FreeFormObj -> U.Crvs,
					       ParamUniform, NumOfPts);

	for (i = 0; i < NumOfPts; i++)
	    IPListObjectInsert(PObjList, i,
			       IPGenPTObject(&R[i], &Zero, &Zero));
	IPListObjectInsert(PObjList, i, NULL);

	IritFree(R);
    }
    else if (IP_IS_SRF_OBJ(FreeFormObj)) {
	CagdUVType
	    *UV = SymbUniformAprxPtOnSrfDistrib(FreeFormObj -> U.Srfs,
						ParamUniform, NumOfPts,
						NULL);

	for (i = 0; i < NumOfPts; i++)
	    IPListObjectInsert(PObjList, i,
			       IPGenPTObject(&UV[i][0], &UV[i][1], &Zero));
	IPListObjectInsert(PObjList, i, NULL);

	IritFree(UV);
    }
    else {
	IRIT_FATAL_ERROR("FfPtDist: Invalid freeform to have uniform distribution for!");
	return NULL;
    }

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to copy the control mesh to a triangular surface's control mesh.   *
*   The triangular surface is allocated here as well.			     *
*   Returns the triangular surface if o.k., otherwise NULL.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtObjList:  A list object of control points.   	  	             *
*   Length:     Length of triangular surface (edge).                         *
*   Order:      Order of triangular surface.                           	     *
*   GType:      Geometry type - Bezier, Bspline etc.                         *
*   ErrStr:     If an error, detected, this is initialized with description. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A triangular surface object if successful, NULL	     *
*		otherwise.						     *
*****************************************************************************/
static IPObjectStruct *GetControlTriSrfMesh(IPObjectStruct *PtObjList,
					    int Length,
					    int Order,
					    TrngGeomType GType,
					    char **ErrStr)
{
    int i, j, PtSize,
        NumVertices = -1;
    CagdRType **r;
    IrtRType *v;
    IPObjectStruct *TriSrfObj, *PtObj;
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

    if (NumVertices < 2) {
	*ErrStr = IRIT_EXP_STR("Less than 2 points");
	return NULL;
    }
    if (TRNG_LENGTH_MESH_SIZE(Length) +
	    (GType == TRNG_TRISRF_GREGORY_TYPE ? 3 : 0) != NumVertices) {
	*ErrStr = IRIT_EXP_STR("Mismatch in number of vertices");
	return NULL;
    }

    /* Coerce all points to a common space, in place. */
    if ((PtType = IPCoercePtsListTo(PtObjList, CAGD_PT_E1_TYPE))
							== CAGD_PT_NONE) {
	*ErrStr = "";
	return NULL;
    }

    switch (GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	    TriSrfObj = IPGenTRISRFObject(TrngBzrTriSrfNew(Length, PtType));
	    break;
	case TRNG_TRISRF_BSPLINE_TYPE:
	    TriSrfObj = IPGenTRISRFObject(TrngBspTriSrfNew(Length, Order,
							   PtType));
	    break;
	case TRNG_TRISRF_GREGORY_TYPE:
	    TriSrfObj = IPGenTRISRFObject(TrngGrgTriSrfNew(Length, PtType));
	    break;
	default:
	    *ErrStr = IRIT_EXP_STR("Bezier, Bspline or Gregory trisrf expected");
	    return NULL;
    }
    PtSize = CAGD_IS_RATIONAL_PT(PtType) + CAGD_NUM_OF_PT_COORD(PtType);

    for (r = TriSrfObj -> U.TriSrfs -> Points, i = 0; i < NumVertices; i++) {
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

    return TriSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier triangular surface geometric object defined   M
*  by a list of control points.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   RLength:     Length of triangular surface (edge).                        M
*   PtObjList: A list object of control points.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier triangualr surface object if successful, NULL M
*		otherwise.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBezierTriSrfObject                                                    M
*****************************************************************************/
IPObjectStruct *GenBezierTriSrfObject(IrtRType *RLength,
				      IPObjectStruct *PtObjList)
{
    int Length = IRIT_REAL_PTR_TO_INT(RLength);
    char *ErrStr;
    IPObjectStruct
	*TriSrfObj = GetControlTriSrfMesh(PtObjList, Length, -1,
					  TRNG_TRISRF_BEZIER_TYPE, &ErrStr);

    if (TriSrfObj == NULL) {
	IRIT_NON_FATAL_ERROR2("TSBEZIER: Ctl mesh, %s, empty object result.\n",
			      ErrStr);
    }

    return TriSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bspline triangular surface geometric object defined  M
* by a list of control points.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   RLength:     Length of triangular surface (edge).                        M
*   ROrder:      Order of triangular surface.                                M
*   PtObjList:   A list object of control points.		             M
*   KntObjList:  A list of knots (numeric values).                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bspline triangular surface object if successful,     M
*		NULL otherwise.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBsplineTriSrfObject                                                   M
*****************************************************************************/
IPObjectStruct *GenBsplineTriSrfObject(IrtRType *RLength,
				       IrtRType *ROrder,
				       IPObjectStruct *PtObjList,
				       IPObjectStruct *KntObjList)
{
    int Len,
	Length = IRIT_REAL_PTR_TO_INT(RLength),
	Order = IRIT_REAL_PTR_TO_INT(ROrder);
    char *ErrStr;
    IPObjectStruct
	*TriSrfObj = GetControlTriSrfMesh(PtObjList, Length, Order,
					  TRNG_TRISRF_BSPLINE_TYPE, &ErrStr);

    if (TriSrfObj == NULL) {
	IRIT_NON_FATAL_ERROR2("TSBSPLINE: Ctl mesh, %s, empty object result.\n",
			      ErrStr);
	return NULL;
    }

    if (TriSrfObj -> U.TriSrfs -> Length < TriSrfObj -> U.TriSrfs -> Order) {
	IPFreeObject(TriSrfObj);
	IRIT_NON_FATAL_ERROR("TBSPLINE: TriSrf mesh length smaller than order.");
	return NULL;
    }

    IritFree(TriSrfObj -> U.TriSrfs -> KnotVector);
    TriSrfObj -> U.TriSrfs -> KnotVector = NULL;
    Len = TriSrfObj -> U.TriSrfs -> Length;
    if ((TriSrfObj -> U.TriSrfs -> KnotVector =
	 GetKnotVector(KntObjList, Order, &Len, &ErrStr, TRUE)) == NULL) {
	IPFreeObject(TriSrfObj);
	IRIT_NON_FATAL_ERROR2("TSBSPLINE: Knot vector, %s, empty object result.\n",
			     ErrStr);
	return NULL;
    }

    if (Len != TriSrfObj -> U.TriSrfs -> Length + Order) {
	IPFreeObject(TriSrfObj);
	IRIT_NON_FATAL_ERROR("Wrong knot vector length");
	return NULL;
    }

    return TriSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a Bezier triangular surface geometric object defined   M
*  by a list of control points.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   RLength:     Length of triangular surface (edge).                        M
*   PtObjList: A list object of control points.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A Bezier triangualr surface object if successful, NULL M
*		otherwise.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenGregoryTriSrfObject                                                   M
*****************************************************************************/
IPObjectStruct *GenGregoryTriSrfObject(IrtRType *RLength,
				       IPObjectStruct *PtObjList)
{
    int Length = IRIT_REAL_PTR_TO_INT(RLength);
    char *ErrStr;
    IPObjectStruct
	*TriSrfObj = GetControlTriSrfMesh(PtObjList, Length, -1,
					  TRNG_TRISRF_GREGORY_TYPE, &ErrStr);

    if (TriSrfObj == NULL) {
	IRIT_NON_FATAL_ERROR2("TSGREGORY: Ctl mesh, %s, empty object result.\n",
			      ErrStr);
    }

    return TriSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate a triangular surface function, at the prescribed parametric     M
* location.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfObj:    Triangular Surface to evaluate.                            M
*   u, v, w:      Parametric location to evaluate at.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A control point of the same type TriSrf has.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalTriSrfObject                                                         M
*****************************************************************************/
IPObjectStruct *EvalTriSrfObject(IPObjectStruct *TriSrfObj,
				 IrtRType *u,
				 IrtRType *v,
				 IrtRType *w)
{
    CagdRType *Pt;
    IPObjectStruct *CtlPtObj;

    if (!IRIT_APX_EQ(*u + *v + *w, 1.0)) {
	IRIT_NON_FATAL_ERROR("Barycentric coordinates of triangular surface must sum to one");
	return NULL;
    }

    Pt = TrngTriSrfEval(TriSrfObj -> U.TriSrfs, *u, *v, *w);
    CtlPtObj = IPGenCTLPTObject(TriSrfObj -> U.TriSrfs -> PType, Pt);

    return CtlPtObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the normal of a point on a triangular surface function, at the  M
* prescribed parametric location.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfObj:    Triangular Surface to evaluate normal at a point.          M
*   u, v, w:      Parametric location to evaluate at.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A control point of the same type TriSrf has.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   NormalTriSrfObject                                                       M
*****************************************************************************/
IPObjectStruct *NormalTriSrfObject(IPObjectStruct *TriSrfObj,
				   IrtRType *u,
				   IrtRType *v,
				   IrtRType *w)
{
    IPObjectStruct *NormalObj;
    CagdVecStruct *Nrml;

    if (!IRIT_APX_EQ(*u + *v + *w, 1.0)) {
	IRIT_NON_FATAL_ERROR("Barycentric coordinates of triangular surface must sum to one");
	return NULL;
    }

    Nrml = TrngTriSrfNrml(TriSrfObj -> U.TriSrfs, *u, *v);
    NormalObj = IPGenVECObject(&Nrml -> Vec[0], &Nrml -> Vec[1],
			       &Nrml -> Vec[2]);

    return NormalObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to differentiate a triangular surface function in Dir of SrfObj.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfObj:    TriSrf to differentiate.                                   M
*   Dir:          Direction of differentiation. Either U or V or W.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A differentiated TriSrf.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   DeriveTriSrfObject                                                       M
*****************************************************************************/
IPObjectStruct *DeriveTriSrfObject(IPObjectStruct *TriSrfObj, IrtRType *Dir)
{
    TrngTriangSrfStruct
	*DerivTriSrf = TrngTriSrfDerive(TriSrfObj -> U.TriSrfs,
				   (TrngTriSrfDirType) IRIT_REAL_PTR_TO_INT(Dir));
    IPObjectStruct
	*DerivTriSrfObj = IPGenTRISRFObject(DerivTriSrf);

    return DerivTriSrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a three dimensional envelope of the offset propagation crom     M
* curve Crv.  The nevelope height (also offset distance) will be set to      M
* Height, and will be approximated within Tolerance accuracy.                M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:        Curve to compute the envelope offset.                     M
*   Height:        Height of enevlope offset.                                M
*   Tolerance:     Accuracy of envelope offset.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The evelope offset. One surface object for an open   M
*		curve, two surfaces in a list for a closed curve.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveEnvelopeOffset, offset                                              M
*****************************************************************************/
IPObjectStruct *CurveEnvelopeOffset(IPObjectStruct *CrvObj,
				    IrtRType *Height,
				    IrtRType *Tolerance)
{
    CagdSrfStruct
	*EnvSrf1 = SymbEnvOffsetFromCrv(CrvObj -> U.Crvs, *Height, *Tolerance);

    if (EnvSrf1 == NULL)
        return NULL;
    else if (EnvSrf1 -> Pnext == NULL) {
	return IPGenSRFObject(EnvSrf1);
    }
    else {
	IPObjectStruct
	    *PObjList = IPGenLISTObject(IPGenSRFObject(EnvSrf1));

	IPListObjectInsert(PObjList, 1, IPGenSRFObject(EnvSrf1 -> Pnext));
	EnvSrf1 -> Pnext = NULL;
	IPListObjectInsert(PObjList, 2, NULL);

	return PObjList;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector on a sphere of two curves or a curve and a point,  M
* to within a prescribed Tolerance.                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Objs:           Either a list of two curves or a list of a point and a   M
*		    curve.  Geometry must be on the unit sphere.             M
*   Tolerance:      Accuracy of bisector computation, or scaling of the	     M
*		    ruling direction in the curve/point bisector.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List object of bisector curves.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SphericalBisector, bisectors, skeleton                                   M
*****************************************************************************/
IPObjectStruct *SphericalBisector(IPObjectStruct *Objs, IrtRType *Tolerance)
{
    int i;
    CagdCrvStruct *Crv;
    IPObjectStruct *RetVal, *Obj1, *Obj2;

    if (!IP_IS_OLST_OBJ(Objs) ||
	(Obj1 = IPListObjectGet(Objs, 0)) == NULL ||
	(Obj2 = IPListObjectGet(Objs, 1)) == NULL) {
	IRIT_NON_FATAL_ERROR("Expecting a list of two curves or a point and a curve.");
	return NULL;
    }

    if (IP_IS_CRV_OBJ(Obj1) && IP_IS_CRV_OBJ(Obj2)) {
	if (IRIT_APX_EQ(*Tolerance, 0.0)) {
	    CagdSrfStruct
	        *BisectSrf = SymbCrvCrvBisectOnSphere3(Obj1 -> U.Crvs,
						       Obj2 -> U.Crvs);

	    if (BisectSrf == NULL)
		return NULL;

	    RetVal = IPGenSRFObject(BisectSrf);
	}
	else if (*Tolerance > 0.0) {
	    CagdCrvStruct
	        *BisectCrvs = SymbCrvCrvBisectOnSphere2(Obj1 -> U.Crvs,
							Obj2 -> U.Crvs,
							*Tolerance);
	    if (BisectCrvs == NULL)
		return NULL;
	    else if (BisectCrvs -> Pnext == NULL)
		RetVal = IPGenCRVObject(BisectCrvs);
	    else {
		RetVal = IPGenLISTObject(NULL);
		for (i = 0; BisectCrvs != NULL; ) {
		    Crv = BisectCrvs;
		    BisectCrvs = BisectCrvs -> Pnext;
		    Crv -> Pnext = NULL;
		    IPListObjectInsert(RetVal, i++, IPGenCRVObject(Crv));
		}
		IPListObjectInsert(RetVal, i, NULL);
	    }
	}
	else {
	    CagdSrfStruct
	        *BisectSrf = SymbCrvCrvBisectOnSphere(Obj1 -> U.Crvs,
						      Obj2 -> U.Crvs);

	    if (BisectSrf == NULL)
		return NULL;

	    RetVal = IPGenSRFObject(BisectSrf);
	}
    }
    else if ((IP_IS_CRV_OBJ(Obj1) && IP_IS_POINT_OBJ(Obj2)) ||
	     (IP_IS_CRV_OBJ(Obj2) && IP_IS_POINT_OBJ(Obj1))) {
        CagdCrvStruct *BisectCrvs;

	if (IP_IS_CRV_OBJ(Obj1) && IP_IS_POINT_OBJ(Obj2)) {
	    IRIT_SWAP(IPObjectStruct *, Obj1, Obj2);
	}

	if (*Tolerance > 0.0)
	    BisectCrvs = SymbPtCrvBisectOnSphere2(Obj1 -> U.Pt, Obj2 -> U.Crvs,
						  *Tolerance);
	else
	    BisectCrvs = SymbPtCrvBisectOnSphere(Obj1 -> U.Pt, Obj2 -> U.Crvs);

	if (BisectCrvs == NULL)
	    return NULL;
	else if (BisectCrvs -> Pnext == NULL)
	    RetVal = IPGenCRVObject(BisectCrvs);
	else {
	    RetVal = IPGenLISTObject(NULL);
	    for (i = 0; BisectCrvs != NULL; ) {
		Crv = BisectCrvs;
		BisectCrvs = BisectCrvs -> Pnext;
		Crv -> Pnext = NULL;
		IPListObjectInsert(RetVal, i++, IPGenCRVObject(Crv));
	    }
	    IPListObjectInsert(RetVal, i, NULL);
	}
    }
    else {
	IRIT_NON_FATAL_ERROR("Expecting a list of two curves or a point and a curve.");
	return NULL;
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of the XY plane and a point.			     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:        Point to bisect with.			                     M
*   Size:      Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A single surface representing the bisector.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PlanePointBisector                                                       M
*****************************************************************************/
IPObjectStruct *PlanePointBisector(IrtPtType Pt, IrtRType *Size)
{
   CagdSrfStruct
	*Srf = SymbPlanePointBisect(Pt, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cylinder and a point.			     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   CylPt:   Point on axis of cylinder.                                      M
*   CylDir:  Direction of cylinder. Must be in the northern hemisphere       M
*	     (positive Z coefficient).					     M
*   CylRad:  Radius of cylinder.					     M
*   Pt:      Point to bisect with.			                     M
*   Size:    Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A single surface representing the bisector.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCylinPointBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CylinPointBisector                                                       M
*****************************************************************************/
IPObjectStruct *CylinPointBisector(IrtPtType CylPt,
				   IrtVecType CylDir,
				   IrtRType *CylRad,
				   IrtPtType Pt,
				   IrtRType *Size)
{
   CagdSrfStruct
	*Srf = SymbCylinPointBisect(CylPt, CylDir, *CylRad, Pt, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cone and a point.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeApex:   Apex point of cone.	                                     M
*   ConeDir:    Direction of cone; must be in the northern hemisphere.       M
*   ConeAngle:  Spanning angle of cone, in degrees.                          M
*   Pt:         Point to bisect with.			                     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConePointBisect                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConePointBisector                                                        M
*****************************************************************************/
IPObjectStruct *ConePointBisector(IrtPtType ConeApex,
				  IrtVecType ConeDir,
				  IrtRType *ConeAngle,
				  IrtPtType Pt,
				  IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbConePointBisect(ConeApex, ConeDir, *ConeAngle, Pt, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a sphere and a point.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr:    Center location of the sphere.                               M
*   SprRad:     Radius of sphere.					     M
*   Pt:         Point to bisect with.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSpherePointBisect                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SpherePointBisector                                                      M
*****************************************************************************/
IPObjectStruct *SpherePointBisector(IrtPtType SprCntr,
				    IrtRType *SprRad,
				    IrtPtType Pt)
{
    CagdSrfStruct
	*Srf = SymbSpherePointBisect(SprCntr, *SprRad, Pt);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a torus a point.				     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   TrsCntr:     Center of constructed torus.                                M
*   TrsDir:      Axis of symmetry of constructed torus.                      M
*   TrsMajorRad: Major radius of constructed torus.                          M
*   TrsMinorRad: Minor radius of constructed torus.                          M
*   Pt:          Point to bisect with.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A single surface representing the bisector.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTorusPointBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TorusPointBisector                                                       M
*****************************************************************************/
IPObjectStruct *TorusPointBisector(IrtPtType TrsCntr,
				   IrtVecType TrsDir,
				   IrtRType *TrsMajorRad,
				   IrtRType *TrsMinorRad,
				   IrtPtType Pt)
{
   CagdSrfStruct
	*Srf = SymbTorusPointBisect(TrsCntr, TrsDir, *TrsMajorRad,
				    *TrsMinorRad, Pt);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of the XY and a line through the origin            M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   LineDir:    Direction of line; must be in the northern hemisphere.       M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlaneLineBisect                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   PlaneLineBisector                                                        M
*****************************************************************************/
IPObjectStruct *PlaneLineBisector(IrtVecType LineDir, IrtRType *Size)
{
   CagdSrfStruct
	*Srf = SymbPlaneLineBisect(LineDir, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cone and a line through its apex.  The apex   M
* is assumed to be at the origin.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeDir:    Direction of cone; must be in the northern hemisphere.       M
*   ConeAngle:  Spanning angle of cone, in degrees.                          M
*   LineDir:    Direction of line; must be in the northern hemisphere.       M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConeLineBisect                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConeLineBisector                                                         M
*****************************************************************************/
IPObjectStruct *ConeLineBisector(IrtVecType ConeDir,
				 IrtRType *ConeAngle,
				 IrtVecType LineDir,
				 IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbConeLineBisect(ConeDir, *ConeAngle, LineDir, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a sphere and a line.  The line is assumed to    M
* be the Z axis.		                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr:    Center location of the sphere.                               M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSphereLineBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SphereLineBisector                                                       M
*****************************************************************************/
IPObjectStruct *SphereLineBisector(IrtVecType SprCntr,
				   IrtRType *SprRad,
				   IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbSphereLineBisect(SprCntr, *SprRad, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a sphere and XY plane.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr:    Center location of the sphere.                               M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSpherePlaneBisect                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SpherePlaneBisector                                                      M
*****************************************************************************/
IPObjectStruct *SpherePlaneBisector(IrtPtType SprCntr,
				    IrtRType *SprRad,
				    IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbSpherePlaneBisect(SprCntr, *SprRad, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cylinder and XY plane.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CylPt:      Point on axis of cylinder.                                   M
*   CylDir:     Direction of cylinder. Must be in the northern hemisphere.   M
*   CylRad:     Radius of cylinder.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCylinPlaneBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CylinPlaneBisector                                                       M
*****************************************************************************/
IPObjectStruct *CylinPlaneBisector(IrtPtType CylPt,
				   IrtVecType CylDir,
				   IrtRType *CylRad,
				   IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbCylinPlaneBisect(CylPt, CylDir, *CylRad, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cone and the XY plane.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeApex:   Apex point of cone.	                                     M
*   ConeDir:    Direction of cone; must be in the northern hemisphere.       M
*   ConeAngle:  Spanning angle of cone, in degrees.                          M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConePlaneBisect                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConePlaneBisector                                                        M
*****************************************************************************/
IPObjectStruct *ConePlaneBisector(IrtPtType ConeApex,
				  IrtVecType ConeDir,
				  IrtRType *ConeAngle,
				  IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbConePlaneBisect(ConeApex, ConeDir, *ConeAngle, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of two spheres.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr1:   Center location of the sphere.                               M
*   SprRad1:    Radius of sphere.					     M
*   SprCntr2:   Center location of the sphere.                               M
*   SprRad2:    Radius of sphere.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSphereSphereBisect                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SphereSphereBisector                                                     M
*****************************************************************************/
IPObjectStruct *SphereSphereBisector(IrtPtType SprCntr1,
				     IrtRType *SprRad1,
				     IrtPtType SprCntr2,
				     IrtRType *SprRad2)
{
    CagdSrfStruct
	*Srf = SymbSphereSphereBisect(SprCntr1, *SprRad1, SprCntr2, *SprRad2);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cylinder and a sphere.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CylPt:      Point on axis of cylinder.                                   M
*   CylDir:     Direction of cylinder. Must be in the northern hemisphere.   M
*   CylRad:     Radius of cylinder.					     M
*   SprCntr:    Center location of the sphere.                               M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCylinSphereBisect                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CylinSphereBisector                                                      M
*****************************************************************************/
IPObjectStruct *CylinSphereBisector(IrtPtType CylPt,
				    IrtVecType CylDir,
				    IrtRType *CylRad,
				    IrtPtType SprCntr,
				    IrtRType *SprRad,
				    IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbCylinSphereBisect(CylPt, CylDir, *CylRad,
				     SprCntr, *SprRad, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of two cones sharing an apex.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Cone1Dir:   Direction of first cone; must be in the northern hemisphere. M
*   Cone1Angle: Spanning angle of first cone, in degrees.                    M
*   Cone2Dir:   Direction of second cone; must be in the northern hemisphere.M
*   Cone2Angle: Spanning angle of second cone, in degrees.                   M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConeConeBisect                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConeConeBisector                                                         M
*****************************************************************************/
IPObjectStruct *ConeConeBisector(IrtVecType Cone1Dir,
				 IrtRType *Cone1Angle,
				 IrtVecType Cone2Dir,
				 IrtRType *Cone2Angle,
				 IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbConeConeBisect(Cone1Dir, *Cone1Angle,
				  Cone2Dir, *Cone2Angle, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of two cones in general position.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Cone1Pt:      Apex point of the first cone.			             M
*   Cone1Dir:     Axes of the first cone.				     M
*   Cone1Angle:   Opening angle of the first cone, in degrees.		     M
*   Cone2Pt:      Apex point of the second cone.			     M
*   Cone2Dir:     Axes of the second cone.				     M
*   Cone2Angle:   Opening angle of the second cone, in degrees.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The bisector surface.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConeCylinBisect                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConeConeBisector2                                                        M
*****************************************************************************/
IPObjectStruct *ConeConeBisector2(IrtPtType Cone1Pt,
				  IrtVecType Cone1Dir,
				  IrtRType *Cone1Angle,
				  IrtPtType Cone2Pt,
				  IrtVecType Cone2Dir,
				  IrtRType *Cone2Angle)
{
    CagdSrfStruct
	*Srf = SymbConeConeBisect2(Cone1Pt, Cone1Dir, *Cone1Angle,
				   Cone2Pt, Cone2Dir, *Cone2Angle);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cone and a sphere.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeApex:   Apex point of cone.	                                     M
*   ConeDir:    Direction of cone; must be in the northern hemisphere.       M
*   ConeAngle:  Spanning angle of cone, in degrees.                          M
*   SprCntr:    Center location of the sphere.                               M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConeSphereBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConeSphereBisector                                                       M
*****************************************************************************/
IPObjectStruct *ConeSphereBisector(IrtPtType ConeApex,
				   IrtVecType ConeDir,
				   IrtRType *ConeAngle,
				   IrtPtType SprCntr,
				   IrtRType *SprRad,
				   IrtRType *Size)
{
    CagdSrfStruct
	*Srf = SymbConeSphereBisect(ConeApex, ConeDir, *ConeAngle,
					      SprCntr, *SprRad, *Size);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a torus and a sphere.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   TrsCntr:     Center of constructed torus.                                M
*   TrsDir:      Axis of symmetry of constructed torus.                      M
*   TrsMajorRad: Major radius of constructed torus.                          M
*   TrsMinorRad: Minor radius of constructed torus.                          M
*   SprCntr:     Center location of the sphere.                              M
*   SprRad:      Radius of sphere.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Either a single or a list of surfaces,              M
*		representing the bisectors.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTorusSphereBisect                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TorusSphereBisector                                                      M
*****************************************************************************/
IPObjectStruct *TorusSphereBisector(IrtPtType TrsCntr,
				    IrtVecType TrsDir,
				    IrtRType *TrsMajorRad,
				    IrtRType *TrsMinorRad,
				    IrtPtType SprCntr,
				    IrtRType *SprRad)
{
    CagdSrfStruct
	*Srf = SymbTorusSphereBisect(TrsCntr, TrsDir, *TrsMajorRad,
					*TrsMinorRad, SprCntr, *SprRad);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of two cylinders in general position.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Cyl1Pt:    A point on the axis of the first cylinder.                    M
*   Cyl1Dir:   Direction of first cylinder.				     M
*   Cyl1Rad:   Radius of first cylinder.				     M
*   Cyl2Pt:    A point on the axis of the second cylinder.                   M
*   Cyl2Dir:   Direction of second cylinder.				     M
*   Cyl2Rad:   Radius of second cylinder.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The bisector surface.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCylinCylinBisect                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CylinCylinBisector                                                       M
*****************************************************************************/
IPObjectStruct *CylinCylinBisector(IrtPtType Cyl1Pt,
				   IrtVecType Cyl1Dir,
				   IrtRType *Cyl1Rad,
				   IrtPtType Cyl2Pt,
				   IrtVecType Cyl2Dir,
				   IrtRType *Cyl2Rad)
{
    CagdSrfStruct
	*Srf = SymbCylinCylinBisect(Cyl1Pt, Cyl1Dir, *Cyl1Rad,
				    Cyl2Pt, Cyl2Dir, *Cyl2Rad);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of a cone and a cylinder.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Cone1Pt:      Apex point of the cone.			             M
*   Cone1Dir:     Axes of the cone.					     M
*   Cone1Angle:   Opening angle of the cone, in degrees.		     M
*   Cyl2Pt:       A point on the axis of the second cylinder.                M
*   Cyl2Dir:      Direction of second cylinder.				     M
*   Cyl2Rad:      Radius of second cylinder.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The bisector surface.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbConeCylinBisect                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   ConeCylinBisector                                                        M
*****************************************************************************/
IPObjectStruct *ConeCylinBisector(IrtPtType Cone1Pt,
				  IrtVecType Cone1Dir,
				  IrtRType *Cone1Angle,
				  IrtPtType Cyl2Pt,
				  IrtVecType Cyl2Dir,
				  IrtRType *Cyl2Rad)
{
    CagdSrfStruct
	*Srf = SymbConeCylinBisect(Cone1Pt, Cone1Dir, *Cone1Angle,
				   Cyl2Pt, Cyl2Dir, *Cyl2Rad);

    if (Srf == NULL)
	return NULL;
    else
	return IPGenSRFObject(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector curves for a given freeform planar (E2) curve or   M
*   curves or curve/pt, to within a prescribed Tolerance.  Employs the three M
* F1/F2/F3 functions from:						     M
* Gershon Elber and Myung Soo Kim.  ``Bisector Curves of Planar Rational     M
* Curves.''  CAD, Vol 30, No 14, pp 1089-1096, December 1998.                M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:         Curve to compute the bisector curves for. Either a       M
*                   single curve (self bisectors) or a list of two curves    M
*		    or a list of a curve and a point. Assumes curves to be   M
*                   in E2 form. Uses SymbCrvPtBisectorCrv2D for crv/pt.      M
*   RZeroSet:       If TRUE, returns the zero-set surface. Returns the       M
*                   bisector curve if it is FALSE.                           M
*   RBisectFunc:    1, 2 to use normal fields, tangent fields.		     M
*		    Uses F1 and F2.					     M
*		    3 for bisector surface of two planar curves as           M
*		    a zero set, using solution of normal intersection pt.    M
*                   Uses F3.                                                 M
*                   RBisectFunc is ignored for crv/pt.                       M
*   Tolerance:      Accuracy of bisector computation                         M
*   RNumerImprove:  If TRUE, numerical improvement is to be applied.	     M
*   RSameNormal:    If TRUE, the bisector should be oriented for inner or    M
*		    outer side of the curves, with respect to their normals. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List object of bisector curves.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveBisectorSkel2D, bisectors, skeleton                                 M
*****************************************************************************/
IPObjectStruct *CurveBisectorSkel2D(IPObjectStruct *CrvObj,
				    IrtRType *RZeroSet,
				    IrtRType *RBisectFunc,
				    IrtRType *Tolerance,
				    IrtRType *RNumerImprove,
				    IrtRType *RSameNormal)
{
    CagdCrvStruct
	*Bisectors = NULL;
    IPObjectStruct *PObjList;
    int i,
	ZeroSet = IRIT_REAL_PTR_TO_INT(RZeroSet),
        BisectFunc = IRIT_REAL_PTR_TO_INT(RBisectFunc),
	NumerImprove = IRIT_REAL_PTR_TO_INT(RNumerImprove),
        SameNormal = IRIT_REAL_PTR_TO_INT(RSameNormal);
        
    if (IP_IS_CRV_OBJ(CrvObj)) {
	if (ZeroSet)
	    return IPGenSRFObject(SymbCrvBisectorsSrf(CrvObj -> U.Crvs,
						      BisectFunc));

	else 
	    Bisectors = SymbCrvBisectors(CrvObj -> U.Crvs, BisectFunc, 
					 *Tolerance, NumerImprove, SameNormal,
					 FALSE);
    }
    else if (IP_IS_OLST_OBJ(CrvObj)) {
	IPObjectStruct
	    *CrvObj1 = IPListObjectGet(CrvObj, 0),
	    *Obj2 = IPListObjectGet(CrvObj, 1);

	if (IP_IS_CRV_OBJ(CrvObj1) && IP_IS_CRV_OBJ(Obj2)) {
	    CagdCrvStruct
		*Crv1 = CagdCrvCopy(CrvObj1 -> U.Crvs);
	    CagdSrfStruct *BisectSrf;

	    Crv1 -> Pnext = Obj2 -> U.Crvs;

	    if (ZeroSet) {
	        if (BisectFunc == 1 || BisectFunc == 2) {
		    BisectSrf = SymbCrvBisectorsSrf(Crv1, BisectFunc);
		    CagdCrvFree(Crv1);
		    return IPGenSRFObject(BisectSrf);
	        }
	        else if (BisectFunc == 3) {
		    BisectSrf = SymbCrvBisectorsSrf3(Crv1);
		    CagdCrvFree(Crv1);
		    return IPGenSRFObject(BisectSrf);
	        }
	    }
	    else 
	        Bisectors = SymbCrvBisectors(Crv1, BisectFunc, *Tolerance,
					     NumerImprove, SameNormal, FALSE);

	    CagdCrvFree(Crv1);
	}
	else if (IP_IS_CRV_OBJ(CrvObj1) && IP_IS_POINT_OBJ(Obj2)) {
	    CagdCrvStruct
		*Crv = CrvObj1 -> U.Crvs;
	    CagdRType
		*Pt = Obj2 -> U.Pt;
	    CagdCrvStruct
		*BisectCrv = SymbCrvPtBisectorCrv2D(Crv, Pt, 0.5);

	    return IPGenCRVObject(BisectCrv);
	}
	else {
	    IRIT_NON_FATAL_ERROR("Expecting a list of two curves");
	    return NULL;
	}
    }
    else {
	IRIT_FATAL_ERROR("Expecting either a curve or a list of two curves");
	return NULL;
    }

    PObjList = IPGenLISTObject(NULL);

    i = 0;
    while (Bisectors != NULL) {
	CagdCrvStruct
	    *Crv = Bisectors;

	Bisectors = Bisectors -> Pnext;
	Crv -> Pnext = NULL;

	IPListObjectInsert(PObjList, i++, IPGenCRVObject(Crv));
    }
    IPListObjectInsert(PObjList, i, NULL);

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector curves for given freeform curve(s), to within a    M
* prescribed Tolerance.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:         Curve to compute the bisector curves for. Either a       M
*                   single curve (self bisectors) or a list of two curves    M
*		    or a list of a curve and a point. Curves are assumed to  M
*                   be E3.                      			     M
*   RBisectFunc:    4 for bisector surface of two planar space curves as     M
*		    a zero set, using normals/tangents equalization.         M
*                   Else uses SymbCrvBisectorsSrf3D.                         M
*                   Ignored for Crv/Pt bisector. Uses SymbCrvPtBisectorSrf3D M 
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List object of bisector surfaces.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveBisectorSkel3D, bisectors, skeleton                                 M
*****************************************************************************/
IPObjectStruct *CurveBisectorSkel3D(IPObjectStruct *CrvObj,
				    IrtRType *RBisectFunc)
{
    int BisectFunc = IRIT_REAL_PTR_TO_INT(RBisectFunc);
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv2, *DCrv1, *DCrv2, *Crv1Aux;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2;
        
    if (IP_IS_OLST_OBJ(CrvObj)) {
	IPObjectStruct
	    *CrvObj1 = IPListObjectGet(CrvObj, 0),
	    *Obj2 = IPListObjectGet(CrvObj, 1);

	if (IP_IS_CRV_OBJ(CrvObj1) && IP_IS_CRV_OBJ(Obj2)) {
	    CagdCrvStruct
		*Crv1 = CagdCrvCopy(CrvObj1 -> U.Crvs);
	    CagdSrfStruct *BisectSrf;

	    Crv2 = Crv1 -> Pnext = Obj2 -> U.Crvs;

	    if (BisectFunc == 4) {
		BisectSrf = SymbCrvBisectorsSrf2(Crv1);
		CagdCrvFree(Crv1);
		return IPGenSRFObject(BisectSrf);
	    }
	    else{
	        CagdBType
		    IsRational1 = CAGD_IS_RATIONAL_PT(Crv1 -> PType),
		    IsRational2 = Crv1 -> Pnext ? 
		                   CAGD_IS_RATIONAL_PT(Crv1 -> Pnext -> PType) 
		                   : IsRational1,
		    IsRational = IsRational1 || IsRational2;

		Crv1Aux = CagdCoerceCrvTo(Crv1, IsRational ? CAGD_PT_P3_TYPE
					                   : CAGD_PT_E3_TYPE,
					  FALSE);
		Crv2 = CagdCoerceCrvTo(Crv1 -> Pnext ? Crv1 -> Pnext : Crv2,
				       IsRational ? CAGD_PT_P3_TYPE
					          : CAGD_PT_E3_TYPE, FALSE);

		CagdCrvFree(Crv1);
		Crv1 = Crv1Aux;

		if (CAGD_IS_BEZIER_CRV(Crv1)) {
		    CagdCrvStruct
		      *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

		    CagdCrvFree(Crv1);
		    Crv1 = TCrv;
		}
		if (CAGD_IS_BEZIER_CRV(Crv2)) {
		    CagdCrvStruct
		       *TCrv = CagdCnvrtBzr2BspCrv(Crv2);
		  
		    CagdCrvFree(Crv2);
		    Crv2 = TCrv;
		}

	        DCrv1 = CagdCrvDerive(Crv1);
		DCrv2 = CagdCrvDerive(Crv2);

		Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR);
		Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);
		
		CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
		CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
		
		BspKnotAffineTrans2(Srf1 -> VKnotVector,
				    Srf1 -> VLength + Srf1 -> VOrder,
				    VMin2, VMax2);
		BspKnotAffineTrans2(Srf2 -> UKnotVector,
				    Srf2 -> ULength + Srf2 -> UOrder,
				    UMin1, UMax1);

		DSrf1 = CagdPromoteCrvToSrf(DCrv1, CAGD_CONST_U_DIR);
		DSrf2 = CagdPromoteCrvToSrf(DCrv2, CAGD_CONST_V_DIR);

		BspKnotAffineTrans2(DSrf1 -> VKnotVector,
				    DSrf1 -> VLength + DSrf1 -> VOrder,
				    VMin2, VMax2);
		BspKnotAffineTrans2(DSrf2 -> UKnotVector,
				    DSrf2 -> ULength + DSrf2 -> UOrder,
				    UMin1, UMax1);

		CagdCrvFree(Crv1);
		CagdCrvFree(Crv2);
		CagdCrvFree(DCrv1);
		CagdCrvFree(DCrv2);
		
		BisectSrf = SymbCrvBisectorsSrf3D(Srf1, Srf2, DSrf1, DSrf2, 
						  0.5);
		return IPGenSRFObject(BisectSrf);
	    }
	    
	}
	else if (IP_IS_CRV_OBJ(CrvObj1) && IP_IS_POINT_OBJ(Obj2)) {
	    CagdCrvStruct
		*Crv = CrvObj1 -> U.Crvs;
	    CagdRType
		*Pt = Obj2 -> U.Pt;

	    CagdSrfStruct
		*BisectSrf = SymbCrvPtBisectorSrf3D(Crv, Pt, 0.5);

	    return IPGenSRFObject(BisectSrf);
	}
	else {
	    IRIT_NON_FATAL_ERROR("Expecting a list of two curves");
	    return NULL;
	}
    }
    else {
	IRIT_FATAL_ERROR("Expecting either a curve or a list of two curves");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector curves for a given freeform curve, to within a     M
* prescribed Tolerance.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:         Curve to compute the alpha sector curve/surface for two  M
*                   E3 curves or a point and a curve (E2/E3)                 M
*   Alpha:          Alpha value for calculating the alpha sector or scaling  M
*                   of the ruling direction in the E3 curve/point bisector.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List object of bisector curves/surfaces.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveAlphaSector, bisectors, skeleton                                    M
*****************************************************************************/
IPObjectStruct *CurveAlphaSector(IPObjectStruct *CrvObj, IrtRType *Alpha)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv2, *DCrv1, *DCrv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2;
        
    if (IP_IS_OLST_OBJ(CrvObj)) {
	IPObjectStruct
	    *CrvObj1 = IPListObjectGet(CrvObj, 0),
	    *Obj2 = IPListObjectGet(CrvObj, 1);

	if (IP_IS_CRV_OBJ(CrvObj1) && IP_IS_CRV_OBJ(Obj2)) {
	    CagdCrvStruct *Crv1Aux,
		*Crv1 = CagdCrvCopy(CrvObj1 -> U.Crvs);
	    CagdSrfStruct *BisectSrf;

	    Crv2 = Crv1 -> Pnext = Obj2 -> U.Crvs;

	    if (Crv1 -> PType == CAGD_PT_E3_TYPE &&
		Crv1 -> Pnext -> PType == CAGD_PT_E3_TYPE) {
		CagdBType
		    IsRational1 = CAGD_IS_RATIONAL_PT(Crv1 -> PType),
		    IsRational2 = Crv1 -> Pnext ? 
		                   CAGD_IS_RATIONAL_PT(Crv1 -> Pnext -> PType) 
		                  : IsRational1,
		    IsRational = IsRational1 || IsRational2;
	        Crv1Aux = CagdCoerceCrvTo(Crv1, IsRational ? CAGD_PT_P3_TYPE
				                           : CAGD_PT_E3_TYPE,
					  FALSE);
		Crv2 = CagdCoerceCrvTo(Crv1 -> Pnext ? Crv1 -> Pnext : Crv2,
				       IsRational ? CAGD_PT_P3_TYPE
				                  : CAGD_PT_E3_TYPE, FALSE);
		CagdCrvFree(Crv1);
		Crv1 = Crv1Aux;

		if (CAGD_IS_BEZIER_CRV(Crv1)) {
		    CagdCrvStruct
		        *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

		    CagdCrvFree(Crv1);
		    Crv1 = TCrv;
		}
		if (CAGD_IS_BEZIER_CRV(Crv2)) {
		    CagdCrvStruct
		        *TCrv = CagdCnvrtBzr2BspCrv(Crv2);
		  
		    CagdCrvFree(Crv2);
		    Crv2 = TCrv;
		}
	        DCrv1 = CagdCrvDerive(Crv1);
		DCrv2 = CagdCrvDerive(Crv2);

		Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR);
		Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);

		CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
		CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
		BspKnotAffineTrans2(Srf1 -> VKnotVector, Srf1 -> VLength + Srf1                                    -> VOrder,	VMin2, VMax2);
		BspKnotAffineTrans2(Srf2 -> UKnotVector, Srf2 -> ULength + Srf2                                    -> UOrder,	UMin1, UMax1);

		DSrf1 = CagdPromoteCrvToSrf(DCrv1, CAGD_CONST_U_DIR);
		DSrf2 = CagdPromoteCrvToSrf(DCrv2, CAGD_CONST_V_DIR);

		BspKnotAffineTrans2(DSrf1 -> VKnotVector,
				    DSrf1 -> VLength + DSrf1 -> VOrder,
				    VMin2, VMax2);
		BspKnotAffineTrans2(DSrf2 -> UKnotVector,
				    DSrf2 -> ULength + DSrf2 -> UOrder,
				    UMin1, UMax1);

		CagdCrvFree(Crv1);
		CagdCrvFree(Crv2);
		CagdCrvFree(DCrv1);
		CagdCrvFree(DCrv2);
		BisectSrf = SymbCrvBisectorsSrf3D(Srf1, Srf2, DSrf1, DSrf2, 
						  *Alpha);
		return IPGenSRFObject(BisectSrf);
	    }
	    else {
	        IRIT_NON_FATAL_ERROR("Expecting a list of two E3 curves");
		return NULL;
	    }
	    
	}
	else if (IP_IS_CRV_OBJ(CrvObj1) && IP_IS_POINT_OBJ(Obj2)) {
	    CagdCrvStruct
		*Crv = CrvObj1 -> U.Crvs;
	    CagdRType
		*Pt = Obj2 -> U.Pt;

	    if(Crv -> PType == CAGD_PT_E3_TYPE) {
	        CagdSrfStruct
		    *BisectSrf = SymbCrvPtBisectorSrf3D(Crv, Pt, *Alpha);
		
		return IPGenSRFObject(BisectSrf);
	    }
	    else {
	        CagdCrvStruct
		    *BisectCrv = SymbCrvPtBisectorCrv2D(Crv, Pt, *Alpha);

		return IPGenCRVObject(BisectCrv);
	    }
	}
	else {
	    IRIT_NON_FATAL_ERROR("Expecting a list of two curves");
	    return NULL;
	}
    }
    else {
	IRIT_FATAL_ERROR("Expecting either a curve or a list of two curves");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface for a given freeform surface and a point.  M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Surface to compute the bisector surface for.		     M
*   Pt:		Point to compute the bisector surface for.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The bisector surface.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceBisectorSkel, bisectors, skeleton                                 M
*****************************************************************************/
IPObjectStruct *SurfaceBisectorSkel(IPObjectStruct *SrfObj, IrtPtType Pt)
{
    CagdSrfStruct
	*BisectSrf = SymbSrfPtBisectorSrf3D(SrfObj -> U.Srfs, Pt);

    if (BisectSrf != NULL)
	return IPGenSRFObject(BisectSrf);
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts one point, line, arc, or a curve object into the primitive      *
* data structure of the 2D skeleton computation.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Obj:         Input object to convert from.                               *
*   Prim:        Place for the result.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	TRUE if succesful, FALSE otherwise                           *
*****************************************************************************/
static int Skel2D2PrimsCnvrt(IPObjectStruct *Obj, MvarSkel2DPrimStruct *Prim)
{
    int GeomType = AttrGetObjectIntAttrib(Obj, "GeomType");

    Prim -> Pnext = NULL;

    if (IP_IS_POINT_OBJ(Obj) ||
	IP_IS_VEC_OBJ(Obj) ||
	IP_IS_CTLPT_OBJ(Obj)) {
	IPObjectStruct
	    *TmpObj = IPCoerceObjectTo(Obj, IP_OBJ_POINT);
	
	IRIT_PT_COPY(Prim -> Pt.Pt, TmpObj -> U.Pt);
	Prim -> Type = MVAR_SK2D_PRIM_POINT;

	IPFreeObject(TmpObj);
    }
    else switch (GeomType) {
	case CAGD_GEOM_LINEAR:
	    if (IP_IS_CRV_OBJ(Obj)) {
		Prim -> Type = MVAR_SK2D_PRIM_LINE;
		CagdCoerceToE3(Prim -> Ln.Pt1, Obj -> U.Crvs -> Points, 0,
			       Obj -> U.Crvs -> PType);
		CagdCoerceToE3(Prim -> Ln.Pt2, Obj -> U.Crvs -> Points, 1,
			       Obj -> U.Crvs -> PType);
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Expecting a linear curve.");
		return FALSE;
	    }
	    break;
	case CAGD_GEOM_CIRCULAR:
	    if (IP_IS_CRV_OBJ(Obj)) {
		/* For now handle arcs as regular curves. */
		Prim -> Type = MVAR_SK2D_PRIM_CRV;
		Prim -> Crv.Crv = CagdCrvCopy(Obj -> U.Crvs);
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Expecting a circular/arc curve.");
		return FALSE;
	    }
	    break;
	default:
	    if (IP_IS_CRV_OBJ(Obj)) {
		Prim -> Type = MVAR_SK2D_PRIM_CRV;
		Prim -> Crv.Crv = CagdCrvCopy(Obj -> U.Crvs);
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Expecting a point/line/arc/curve only.");
		return FALSE;
	    }
	    break;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all points that are equadistant from the given three primitives M
* in the plane.                                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj1, Obj2, Obj3:  The three objects to consider.  Can be either a       M
*		       point, a line, and arc, or a curve.		     M
*   OutExtent:         Extent to take the bisector to (a perimeter).         M
*   Eps:               Tolerance of numeric computation stages.              M
*   FineNess:	       Fineness of polygonal approximation in zero set comp. M
*   MZeroTols:	       Subdiv/Numeric tolerances of multivariate solver.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The equadistant locations, as point objects with    M
*		       "Prim1/2/3Pos attributes that provides the respective M
*		       locations on the three primitives.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   Skel2D2PrimsInter                                                        M
*****************************************************************************/
IPObjectStruct *Skel2D2PrimsInter(IPObjectStruct *Obj1,
				  IPObjectStruct *Obj2,
				  IPObjectStruct *Obj3,
				  IrtRType *OutExtent,
				  IrtRType *Eps,
				  IrtRType *FineNess,
				  IPObjectStruct *MZeroTols)
{
    int i;
    CagdRType 
	OldSubdivTol = MvarSkel2DSetMZeroTols(1e-3, 1e-10),
	OldEps = MvarSkel2DSetEpsilon(*Eps),
        OldFineNess = MvarSkel2DSetFineNess(*FineNess),
	OldExtent = MvarSkel2DSetOuterExtent(*OutExtent);
    MvarSkel2DInter3PrimsStruct *Pt, *InterPts;
    MvarSkel2DPrimStruct Prim1, Prim2, Prim3;
    IPObjectStruct *PObjList, *PObj, *PObj2;

    if (IP_IS_OLST_OBJ(MZeroTols)) {
	PObj = IPListObjectGet(MZeroTols, 0);
	PObj2 = IPListObjectGet(MZeroTols, 1);
	if (IP_IS_NUM_OBJ(PObj) && IP_IS_NUM_OBJ(PObj))
	    MvarSkel2DSetMZeroTols(PObj -> U.R, PObj2 -> U.R);
    }

    Skel2D2PrimsCnvrt(Obj1, &Prim1);
    Skel2D2PrimsCnvrt(Obj2, &Prim2);
    Skel2D2PrimsCnvrt(Obj3, &Prim3);
    InterPts = MvarSkel2DInter3Prims(&Prim1, &Prim2, &Prim3);
    if (Prim1.Type == MVAR_SK2D_PRIM_CRV)
	CagdCrvFree(Prim1.Crv.Crv);
    if (Prim2.Type == MVAR_SK2D_PRIM_CRV)
	CagdCrvFree(Prim2.Crv.Crv);
    if (Prim3.Type == MVAR_SK2D_PRIM_CRV)
	CagdCrvFree(Prim3.Crv.Crv);

    MvarSkel2DSetEpsilon(OldEps);
    MvarSkel2DSetFineNess(OldFineNess);
    MvarSkel2DSetOuterExtent(OldExtent);
    MvarSkel2DSetMZeroTols(OldSubdivTol, 1e-10);

    PObjList = IPGenLISTObject(NULL);
    for (Pt = InterPts, i = 0; Pt != NULL; Pt = Pt -> Pnext) {
	IPListObjectInsert(PObjList, i++,
			   PObj = IPGenPTObject(&Pt -> EquadistPoint[0],
						&Pt -> EquadistPoint[1],
						&Pt -> EquadistPoint[2]));
	AttrSetObjectObjAttrib(PObj, "Prim1Pos",
			       IPGenPTObject(&Pt -> PosPrim1[0],
					     &Pt -> PosPrim1[1],
					     &Pt -> PosPrim1[2]), FALSE);
	AttrSetObjectObjAttrib(PObj, "Prim2Pos",
			       IPGenPTObject(&Pt -> PosPrim2[0],
					     &Pt -> PosPrim2[1],
					     &Pt -> PosPrim2[2]), FALSE);
	AttrSetObjectObjAttrib(PObj, "Prim3Pos",
			       IPGenPTObject(&Pt -> PosPrim3[0],
					     &Pt -> PosPrim3[1],
					     &Pt -> PosPrim3[2]), FALSE);
    }
    IPListObjectInsert(PObjList, i, NULL);
    MvarSkel2DInter3PrimsFreeList(InterPts);

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes all center points that are equadistant from the given N         M
* (multivariate) primitives in R^{N-1}.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ListMvs:     List of multivariates to compute hyper tangent sphere for.  M
*   SubdivTol:   Tolerance of the subdivision stage.                         M
*   NumerTol:    Tolerance of the numerical improvement stage.               M
*   UseExprTree: TRUE to use multivariates' expresion trees, FALSE to use    M
*		 regular multivariates.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list of center locations with radius as an	     M
*		"radius" attributes.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Skel2D2PrimsInter                                                        M
*****************************************************************************/
IPObjectStruct *SkelNDPrimsInter(IPObjectStruct *ListMVs,
				 IrtRType *SubdivTol,
				 IrtRType *NumericTol,
				 IrtRType *UseExprTree)
{
    int i,
	ListObjSize = IPListObjectLength(ListMVs);
    IPObjectStruct *PObj, *CtlPts;
    MvarPtStruct *MVPts;
    MvarMVStruct
	**MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) *
								 ListObjSize);

    for (i = 0;
	 (PObj = IPListObjectGet(ListMVs, i)) != NULL && i < ListObjSize;
	 i++) {
        if (!IP_IS_MVAR_OBJ(PObj)) {
	    IritFree(MVs);
	    IRIT_NON_FATAL_ERROR("Expecting multivariates only.");
	    return NULL;
	}

        MVs[i] = PObj -> U.MultiVars;
    }

    MVPts = MvarTanHyperSpheresofNManifolds(MVs, ListObjSize,
					    *SubdivTol, *NumericTol,
					    IRIT_REAL_PTR_TO_INT(UseExprTree));
    IritFree(MVs);

    CtlPts = MvarCnvrtMVPtsToCtlPts(MVPts, 0.0);
    MvarPtFreeList(MVPts);

    return CtlPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a cone that bounds the normals of the given surface.            M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:     An object surface to compute the bounding normal cone for.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Computed normal cone of the given surface.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbNormalConeForSrf                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   EvalSrfNormalsCone                                                       M
*****************************************************************************/
IPObjectStruct *EvalSrfNormalsCone(IPObjectStruct *PSrf)
{
    const SymbNormalConeStruct
	*NCone = SymbNormalConeForSrf(PSrf -> U.Srfs);

    if (NCone != NULL) {
        IPObjectStruct
	    *AxisObj = IPGenVECObject(&NCone -> ConeAxis[0],
				      &NCone -> ConeAxis[1],
				      &NCone -> ConeAxis[2]),
	    *AngleObj = IPGenNUMValObject(NCone -> ConeAngle),
	    *RetObj = IPGenLISTObject(AxisObj);

	IPListObjectInsert(RetObj, 1, AngleObj);
	IPListObjectInsert(RetObj, 2, NULL);

	return RetObj;
    }
    else {
        IRIT_NON_FATAL_ERROR("Failed to create normal cone for given surface");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Voronoi cell for a given set of freeform planar (E2)        M
*   closed curves .							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:         List of Curves to compute the Voronoi cell. The Voronoi  M
*                   cell is computed for the first curve. All the curves     M
*                   assumed to be in E2 form.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List object of Voronoi cell.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeVoronoiCell, bisectors, skeleton                                  M
*****************************************************************************/
IPObjectStruct *ComputeVoronoiCell(IPObjectStruct *CrvObj)
{
    int i,
        j = 1;
    IPObjectStruct *CrvObj1, *Obj2;
    CagdCrvStruct *Crv1Tmp, *Crv1;

    if (IP_IS_OLST_OBJ(CrvObj)) {    
        i = IPListObjectLength(CrvObj);
	CrvObj1 = IPListObjectGet(CrvObj, 0);
	Crv1 = CagdCrvCopy(CrvObj1 -> U.Crvs);
	Crv1Tmp = Crv1;
	while (j < i) {
	    Obj2 = IPListObjectGet(CrvObj, j);
	    Crv1Tmp -> Pnext = CagdCrvCopy(Obj2 -> U.Crvs);
	    Crv1Tmp = Crv1Tmp -> Pnext;
	    j++;
	}
	Obj2 = MvarComputeVoronoiCell(Crv1);
	CagdCrvFreeList(Crv1);
	return Obj2;
    }
    else {
        IRIT_NON_FATAL_ERROR("Expecting curve objects only.");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Builds a belt-curve formed out of a pair of sequences of lines/arcs      M
* around the given set of circles (pulleys), in order.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Pulleys:         A sequence of circles (pulleys), each as (x, y, r).     M
*                    If r is positive is it a CW pulley, otehrwise CCW.      M
*   BeltThickness:   The thickness of the constructed belt.                  M
*   BoundingArcs:    If non zero, bounding arcs are computed for each linear M
*                    segmemnt in belt, for each of the two sides of the belt.M
*   ReturnCrvs:      TRUE to simply return two closed curves with the left   M
*                    and right sides of the belt.  FALSE to return a list    M
*                    with the individual arcs and lines and their attributes.M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Two lists of lines/arcs representing the two sides   M
*                    the belt around the given circles (pulleys).  Each      M
*                    linear segment might hold an optional bounding arc with M
*                    maximal distance of BoundingArcs, if non zero.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeBeltCurve, pulleys   		                             M
*****************************************************************************/
IPObjectStruct *ComputeBeltCurve(IPObjectStruct *Pulleys,
				 IrtRType *BeltThickness,
				 IrtRType *BoundingArcs,
				 IrtRType *ReturnCrvs)
{
    int Intersects;
    const char *Error;
    IPObjectStruct
        *Belt = UserBeltCreate(Pulleys -> U.Pl -> PVertex,
			       *BeltThickness,
			       *BoundingArcs,
			       IRIT_REAL_PTR_TO_INT(ReturnCrvs),
			       &Intersects,
			       &Error);
    if (Belt == NULL) {
        IRIT_NON_FATAL_ERROR(Error);        
	return NULL;
    }

    if (Intersects)
        IRIT_WARNING_MSG("Computed belt self-intersects.");

    return Belt;
}

