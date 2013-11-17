/******************************************************************************
* SymbSPly.c - Adaptive surface to polygons approximation.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March. 93.					      *
******************************************************************************/

#include "symb_loc.h"
#include "geom_lib.h"

typedef enum {
    UMIN_BOUNDARY = 1,
    UMAX_BOUNDARY = 2,
    VMIN_BOUNDARY = 4,
    VMAX_BOUNDARY = 8
} PatchBoundaryType;

typedef struct PointDependStruct {
    struct PointDependStruct *Pnext;
    struct RectangularStruct *Rect;
    int OtherIndex, Index;
} PointDependStruct;

typedef struct RectangularStruct {
    struct RectangularStruct *Pnext;
    CagdPType Point[4];	/* 4 corners, not always on Srf due to "black holes".*/
    CagdVType Normal[4];
    CagdUVType UV[4];
    PointDependStruct *Depend[4];  /* Points that depends on these 4 points. */
    CagdBType Valid;/* TRUE if to be converted to polygons, FALSE otherwise. */
} RectangularStruct;

typedef struct EdgeStruct {
    struct EdgeStruct *Pnext;
    struct RectangularStruct *Rect;
    int Index;
} EdgeStruct;

typedef struct {
    struct EdgeStruct *Left;		/* UMin */
    struct EdgeStruct *Right;		/* UMax */
    struct EdgeStruct *Top;		/* VMin */
    struct EdgeStruct *Bottom;		/* VMax */
} ConnectivityStruct;

IRIT_STATIC_DATA int
    GlblFoundPartialShare = FALSE;
IRIT_STATIC_DATA RectangularStruct
    *GlblRectList = NULL;
IRIT_STATIC_DATA CagdSrfStruct
    *GlblCrvtrSqrSrf = NULL,
    *GlblUIsoCrvtrSqrSrf = NULL,
    *GlblVIsoCrvtrSqrSrf = NULL,
    *GlblOrigSrf = NULL,
    *GlblNormalSrf = NULL;

static CagdRType SymbMaxDistCrvLine(const CagdCrvStruct *Crv);
static CagdRType ExtremumNodeValue(const CagdSrfStruct *Srf,
				   CagdRType UMin, CagdRType UMax,
				   CagdRType VMin, CagdRType VMax,
				   CagdSrfDirType Dir,
				   CagdRType *MaxVal);
static ConnectivityStruct SymbSrf2OptimalPolygonsAux(
			        CagdSrfStruct *Srf,
				int SubdivDepth,
				CagdRType Tolerance,
				SymbPlSubdivStrategyType SubdivDirStrategy,
				SymbPlErrorFuncType SrfPolyApproxError);
static ConnectivityStruct MergeConnectivityEdge(
				ConnectivityStruct *Connectivity1,
				ConnectivityStruct *Connectivity2,
				CagdSrfDirType Dir);
static void UpdateBlackHoles(EdgeStruct *L1,
			     EdgeStruct *L2,
			     CagdSrfDirType Dir);
static int UpdateOneBlackHole(RectangularStruct *Rect1,
			      int Index1a,
			      RectangularStruct *Rect2,
			      int Index2a,
			      CagdSrfDirType Dir);
static void MakePointDepend(RectangularStruct *DependRect,
			    int DependIndex,
			    RectangularStruct *MasterRect,
			    int Master1Index,
			    int Master2Index);
static void PropagateDependency(RectangularStruct *Rect, int Index);
static void ProjectPointOnLine(CagdPType Pt,
			       CagdPType Pt1,
			       CagdPType Pt2,
			       CagdPType Nrml,
			       CagdPType Nrml1,
			       CagdPType Nrml2);
static EdgeStruct *ChainEdgeLists(EdgeStruct *L1, EdgeStruct *L2);
static CagdPolygonStruct *MakeTriangle(CagdPType Pt1,
				       CagdVType Nrml1,
				       CagdUVType UV1,
				       CagdPType Pt2,
				       CagdVType Nrml2,
				       CagdUVType UV2,
				       CagdPType Pt3,
				       CagdVType Nrml3,
				       CagdUVType UV3,
				       CagdBType ComputeUV,
				       CagdBType ComputeNrmls);
static EdgeStruct *MakeEdge(RectangularStruct *Rect, int Index);

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to bound the maximal deviation of a curve from a line connecting   *
* its end points. Works for closed curves as well.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:        To compute a bound on its straightness.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:    Straightness deviation measure.                            *
*****************************************************************************/
static CagdRType SymbMaxDistCrvLine(const CagdCrvStruct *Crv)
{
    int i;
    CagdRType *R,
	MaxVal = 0.0;
    CagdPtStruct Pt1, Pt2;
    CagdCrvStruct *Line, *Diff, *DiffSqr;

    CagdCoerceToE3(Pt1.Pt, Crv -> Points, 0, Crv -> PType);
    CagdCoerceToE3(Pt2.Pt, Crv -> Points, Crv -> Length - 1, Crv -> PType);

    Line = CagdMergePtPt(&Pt1, &Pt2);
    if (CAGD_IS_BSPLINE_CRV(Crv)) {
	CagdCrvStruct
	    *CTmp = CagdCnvrtBzr2BspCrv(Line);

	CagdCrvFree(Line);
	Line = CTmp;
	BspKnotAffineTrans(Line -> KnotVector, Line -> Length + Line -> Order,
			   Crv -> KnotVector[0] - Line -> KnotVector[0],
			   (Crv -> KnotVector[Crv -> Length +
					      Crv -> Order - 1] -
			    Crv -> KnotVector[0]) /
			   (Line -> KnotVector[Line -> Length +
					       Line -> Order - 1] -
			    Line -> KnotVector[0]));
    }
    Diff = SymbCrvSub(Crv, Line);
    CagdCrvFree(Line);
    DiffSqr = SymbCrvDotProd(Diff, Diff);
    CagdCrvFree(Diff);
    Diff = CagdCoerceCrvTo(DiffSqr, CAGD_PT_E1_TYPE, FALSE);
    CagdCrvFree(DiffSqr);
    for (i = 0, R = Diff -> Points[1]; i < Diff -> Length; i++, R++) {
	if (MaxVal < *R)
	    MaxVal = *R;
    }
    return sqrt(MaxVal);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the scalar field of k1^2 + k2^2 (k1, k2 are principal   M
* curvatures) for the surface Srf, into GlblCrvtrSqrSrf.		     M
*   This scalar field is used by SymbSrf2OptPolysCurvatureError function.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute the curvature bound for as an optional preprocess M
*		for function SymbSrf2OptPolysCurvatureError.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2OptPolysCurvatureErrorPrep, polygonization, surface		     M
*   approximation							     M
*****************************************************************************/
void SymbSrf2OptPolysCurvatureErrorPrep(const CagdSrfStruct *Srf)
{
    if (CAGD_IS_BEZIER_SRF(Srf))
	GlblOrigSrf = CagdCnvrtBzr2BspSrf(Srf);
    else
	GlblOrigSrf = CagdSrfCopy(Srf);

    GlblCrvtrSqrSrf = SymbSrfCurvatureUpperBound(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to estimate the curvature of the patch using k1^2 + k2^2.          M
*   Assumes the availability of the GlblCrvtrSqrSrf for Srf.		     M
*   This estimate is too loose and in fact is not recommended!		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To estimate curvature for.                                  M
*   Dir:         Currently not used.                                         M
*   SubdivLevel: Subdivision level of surface.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Curvature estimated.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2OptPolysCurvatureError, polygonization, surface approximation    M
*****************************************************************************/
CagdRType SymbSrf2OptPolysCurvatureError(CagdSrfStruct *Srf,
					 CagdSrfDirType Dir,
					 int SubdivLevel)
{
    CagdRType UMin, UMax, VMin, VMax, *R, MaxCurvatureSqr, Size, RetVal, TmpR;
    CagdSrfStruct *TSrf1, *TSrf2;
    CagdCrvStruct *AuxCrv, *Crv;
    CagdBBoxStruct BBox;
    int i,
	Neighbors = AttrGetIntAttrib(Srf -> Attr, "_Neighbors");

    RetVal = AttrGetRealAttrib(Srf -> Attr, "_SrfCurvature");
    if (!IP_ATTR_IS_BAD_REAL(RetVal))
	return RetVal;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    TSrf1 = CagdSrfRegionFromSrf(GlblCrvtrSqrSrf, UMin, UMax, CAGD_CONST_U_DIR);
    TSrf2 = CagdSrfRegionFromSrf(TSrf1, VMin, VMax, CAGD_CONST_V_DIR);
    CagdSrfFree(TSrf1);
    TSrf1 = CagdCoerceSrfTo(TSrf2, CAGD_PT_E1_TYPE, FALSE);
    CagdSrfFree(TSrf2);

    for (i = 0, R = TSrf1 -> Points[1], MaxCurvatureSqr = 0.0;
	 i < TSrf1 -> ULength * TSrf1 -> VLength;
	 i++, R++)
	if (MaxCurvatureSqr < *R)
	    MaxCurvatureSqr = *R;
    CagdSrfFree(TSrf1);

    CagdSrfBBox(Srf, &BBox);
    for (i = 0, Size = 0.0; i < 3; i++)
	Size += BBox.Max[i] - BBox.Min[i];
    
    RetVal = sqrt(MaxCurvatureSqr) * Size;
    
    /* Now compute bounds on boundary, if on boundary. */
    if (Neighbors & UMIN_BOUNDARY) {
	AuxCrv = CagdCrvFromMesh(Srf, 0, CAGD_CONST_U_DIR),
	Crv = CagdCrvRegionFromCrv(AuxCrv, VMin, VMax);
	TmpR = SymbMaxDistCrvLine(Crv);
	RetVal = IRIT_MAX(RetVal, TmpR);
	CagdCrvFree(AuxCrv);
	CagdCrvFree(Crv);
    }
    if (Neighbors & UMAX_BOUNDARY) {
	AuxCrv = CagdCrvFromMesh(Srf, Srf -> ULength - 1,
				 CAGD_CONST_U_DIR);
	Crv = CagdCrvRegionFromCrv(AuxCrv, VMin, VMax);
	TmpR = SymbMaxDistCrvLine(Crv);
	RetVal = IRIT_MAX(RetVal, TmpR);
	CagdCrvFree(AuxCrv);
	CagdCrvFree(Crv);
    }
    if (Neighbors & VMIN_BOUNDARY) {
	AuxCrv = CagdCrvFromMesh(Srf, 0, CAGD_CONST_V_DIR);
	Crv = CagdCrvRegionFromCrv(AuxCrv, UMin, UMax);
	TmpR = SymbMaxDistCrvLine(Crv);
	RetVal = IRIT_MAX(RetVal, TmpR);
	CagdCrvFree(AuxCrv);
	CagdCrvFree(Crv);
    }
    if (Neighbors & VMAX_BOUNDARY) {
	AuxCrv = CagdCrvFromMesh(Srf, Srf -> VLength - 1,
				 CAGD_CONST_V_DIR);
	Crv = CagdCrvRegionFromCrv(AuxCrv, UMin, UMax);
	TmpR = SymbMaxDistCrvLine(Crv);
	RetVal = IRIT_MAX(RetVal, TmpR);
	CagdCrvFree(AuxCrv);
	CagdCrvFree(Crv);
    }

    AttrSetRealAttrib(&Srf -> Attr, "_SrfCurvature", RetVal);
    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to estimate the curvature of the patch using a bilinear approx.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To estimate curvature for.                                  M
*   Dir:         Currently not used.                                         M
*   SubdivLevel: Subdivision level of surface.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Curvature estimated.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2OptPolysBilinPolyError, polygonization, surface approximation    M
*****************************************************************************/
CagdRType SymbSrf2OptPolysBilinPolyError(CagdSrfStruct *Srf,
					 CagdSrfDirType Dir,
					 int SubdivLevel)
{
    int i;
    CagdRType UMin, UMax, VMin, VMax, *R, MaxDiffSqr, RetVal, Bilin2PolyErr;
    CagdPtStruct Pt00, Pt01, Pt10, Pt11;
    CagdSrfStruct *BilinSrf, *TSrf2,
	*TSrf1 = CagdSrfCopy(Srf);
    IrtPlnType Plane;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    R = CagdSrfEval(Srf, UMin, VMin);
    CagdCoerceToE3(Pt00.Pt, &R, -1, Srf -> PType);
    R = CagdSrfEval(Srf, UMin, VMax);
    CagdCoerceToE3(Pt01.Pt, &R, -1, Srf -> PType);
    R = CagdSrfEval(Srf, UMax, VMin);
    CagdCoerceToE3(Pt10.Pt, &R, -1, Srf -> PType);
    R = CagdSrfEval(Srf, UMax, VMax);
    CagdCoerceToE3(Pt11.Pt, &R, -1, Srf -> PType);

    if (GMPlaneFrom3Points(Plane, Pt00.Pt, Pt01.Pt, Pt10.Pt))
	Bilin2PolyErr = GMDistPointPlane(Pt11.Pt, Plane) * 0.5;
    else
	Bilin2PolyErr = 0.0;

    BilinSrf = CagdBilinearSrf(&Pt00, &Pt10, &Pt01, &Pt11);

    CagdMakeSrfsCompatible(&TSrf1, &BilinSrf, TRUE, TRUE, TRUE, TRUE);

    TSrf2 = SymbSrfSub(TSrf1, BilinSrf);
    CagdSrfFree(TSrf1);
    CagdSrfFree(BilinSrf);
    TSrf1 = SymbSrfDotProd(TSrf2, TSrf2);
    CagdSrfFree(TSrf2);

    TSrf2 = CagdCoerceSrfTo(TSrf1, CAGD_PT_E1_TYPE, FALSE);
    CagdSrfFree(TSrf1);

    for (i = 0, R = TSrf2 -> Points[1], MaxDiffSqr = 0.0;
	 i < TSrf2 -> ULength * TSrf2 -> VLength;
	 i++, R++)
	if (MaxDiffSqr < *R)
	    MaxDiffSqr = *R;
    CagdSrfFree(TSrf2);

    RetVal = sqrt(MaxDiffSqr) + Bilin2PolyErr;
    
    AttrSetRealAttrib(&Srf -> Attr, "_SrfCurvature", RetVal);
    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to precompute the scalar field of kn^u and kn^v (the normal        M
* curvatures in the iso parametric directions).				     M
*   These scalar fields are used to determined the prefered subdivision	     M
* location of Srf.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute the curvature bound in the isoparametric direction. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2OptPolysIsoDirCurvatureErrorPrep, polygonization, surface	     M
*   approximation							     M
*****************************************************************************/
void SymbSrf2OptPolysIsoDirCurvatureErrorPrep(const CagdSrfStruct *Srf)
{
    if (CAGD_IS_BEZIER_SRF(Srf))
	GlblOrigSrf = CagdCnvrtBzr2BspSrf(Srf);
    else
	GlblOrigSrf = CagdSrfCopy(Srf);

    GlblUIsoCrvtrSqrSrf = SymbSrfIsoDirNormalCurvatureBound(Srf,
							    CAGD_CONST_U_DIR);
    GlblVIsoCrvtrSqrSrf = SymbSrfIsoDirNormalCurvatureBound(Srf,
							    CAGD_CONST_V_DIR);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single surface to a set of triangles approximating    M
* it.									     M
*   FineNess is controlled via a function that returns an error measure      M
* SrfPolyApproxError that is guaranteed to be less than Tolerance.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:               To convert and approximate using triangles.           M
*   Tolerance:         Accuracy control.                                     M
*   SubdivDirStrategy: Alternatively in U and V, direction that minimizes    M
*		       the error, etc.					     M
*   SrfPolyApproxErr:  Using bilinear curvature estimate, k1^2 + k2^2	     M
*		       estimate, etc.  Bounds the error call back function.  M
*		       If this function returns a negative value, this whole M
*		       patch is invalidated and no polygons will be created  M
*		       for it. 						     M
*   ComputeNormals:    Do we want normals to be computed as well?            M
*   FourPerFlat:       If TRUE, four triangle per flat surface patch are     M
*                      created, otherwise only two.			     M
*   ComputeUV:         Do we want UV parameter values with the vertices of   M
*                      the triangles?					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  Resulting polygons that approximates Srf.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2OptimalPolygons, approximation, conversion                       M
*****************************************************************************/
CagdPolygonStruct *SymbSrf2OptimalPolygons(
				CagdSrfStruct *Srf,
				CagdRType Tolerance,
				SymbPlSubdivStrategyType SubdivDirStrategy,
				SymbPlErrorFuncType SrfPolyApproxErr,
				CagdBType ComputeNormals,
				CagdBType FourPerFlat,
				CagdBType ComputeUV)
{
    int i,
	NewSrf = FALSE;
    CagdPolygonStruct
	*Polys = NULL;

    GlblNormalSrf = ComputeNormals ? SymbSrfNormalSrf(Srf) : NULL;

    if (CAGD_IS_BEZIER_SRF(Srf)) {
	NewSrf = TRUE;
	Srf = CagdCnvrtBzr2BspSrf(Srf);
    }

    GlblRectList = NULL;
    GlblFoundPartialShare = FALSE;

    AttrSetIntAttrib(&Srf -> Attr, "_Neighbors",
		     UMIN_BOUNDARY | UMAX_BOUNDARY |
		     VMIN_BOUNDARY | VMAX_BOUNDARY);

    SymbSrf2OptimalPolygonsAux(Srf, 0, Tolerance, SubdivDirStrategy,
			       SrfPolyApproxErr);

    /* Convert every rectangular domain into two or four polygons. */
    while (GlblRectList != NULL) {
	RectangularStruct
	    *Rect = GlblRectList;

	if (Rect -> Valid) {
	    if (FourPerFlat) {
		CagdPType MiddlePoint;
		CagdVType MiddleNormal;
		CagdUVType MiddleUV;
		CagdRType *R;

		MiddleUV[0] = (Rect -> UV[0][0] + Rect -> UV[2][0]) * 0.5;
		MiddleUV[1] = (Rect -> UV[0][1] + Rect -> UV[2][1]) * 0.5;
		R = CagdSrfEval(Srf, MiddleUV[0], MiddleUV[1]);
		CagdCoerceToE3(MiddlePoint, &R, -1, Srf -> PType);
		if (GlblNormalSrf != NULL)
		    CagdEvaluateSurfaceVecField(MiddleNormal, GlblNormalSrf,
						MiddleUV[0], MiddleUV[1]);

		for (i = 0; i < 4; i++) {
		    CagdPolygonStruct
			*Poly = MakeTriangle(Rect -> Point[i],
					         Rect -> Normal[i],
					         Rect -> UV[i],
					     Rect -> Point[(i + 1) & 0x11],
					         Rect -> Normal[(i + 1) & 0x11],
					         Rect -> UV[(i + 1) & 0x11],
					     MiddlePoint,
					         MiddleNormal,
					         MiddleUV,
					     ComputeUV, ComputeNormals);

		    if (Poly != NULL)
			IRIT_LIST_PUSH(Poly, Polys);
		}
	    }
	    else {
		CagdPolygonStruct
		    *Poly1 = MakeTriangle(Rect -> Point[0],
				              Rect -> Normal[0],
				              Rect -> UV[0],
					  Rect -> Point[1],
				              Rect -> Normal[1],
				              Rect -> UV[1],
					  Rect -> Point[2],
				              Rect -> Normal[2],
				              Rect -> UV[2],
					  ComputeUV, ComputeNormals),
		    *Poly2 = MakeTriangle(Rect -> Point[0],
				              Rect -> Normal[0],
				              Rect -> UV[0],
					  Rect -> Point[2],
				              Rect -> Normal[2],
				              Rect -> UV[2],
					  Rect -> Point[3],
				              Rect -> Normal[3],
				              Rect -> UV[3],
					  ComputeUV, ComputeNormals);

		if (Poly1 != NULL)
		    IRIT_LIST_PUSH(Poly1, Polys);
		if (Poly2 != NULL)
		    IRIT_LIST_PUSH(Poly2, Polys);
	    }
	}

	GlblRectList = GlblRectList -> Pnext;

	/* Free the rectangular structure. */
	for (i = 0; i < 4; i++) {
	    PointDependStruct
		*Depend = Rect -> Depend[i];

	    while (Depend != NULL) {
		PointDependStruct
		    *NextDepend = Depend -> Pnext;

		IritFree(Depend);
		Depend = NextDepend;
	    }
	}
	IritFree(Rect);
    }

    CagdSrfFree(GlblNormalSrf);
    GlblNormalSrf = NULL;

    if (NewSrf)
	CagdSrfFree(Srf);

    if (GlblCrvtrSqrSrf != NULL) {
	CagdSrfFree(GlblCrvtrSqrSrf);
	GlblCrvtrSqrSrf = NULL;
    }
    if (GlblUIsoCrvtrSqrSrf != NULL) {
	CagdSrfFree(GlblUIsoCrvtrSqrSrf);
	GlblUIsoCrvtrSqrSrf = NULL;
    }
    if (GlblVIsoCrvtrSqrSrf != NULL) {
	CagdSrfFree(GlblVIsoCrvtrSqrSrf);
	GlblVIsoCrvtrSqrSrf = NULL;
    }
    if (GlblOrigSrf != NULL) {
	CagdSrfFree(GlblOrigSrf);
	GlblOrigSrf = NULL;
    }

    return Polys;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computs the extremum curvature location to subdivide at.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf: 			Surface to consider.                         *
*   UMin, UMax, VMin, VMax:     Domain to consider in Srf.                   *
*   Dir:                        Direction of subdivision planned.            *
*   MaxVal:                     Where extremum curvature value is saved.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:    The subdivision parameter in direction Dir suggested       *
*****************************************************************************/
static CagdRType ExtremumNodeValue(const CagdSrfStruct *Srf,
				   CagdRType UMin,
				   CagdRType UMax,
				   CagdRType VMin,
				   CagdRType VMax,
				   CagdSrfDirType Dir,
				   CagdRType *MaxVal)
{
    CagdRType *UV;
    CagdSrfStruct
	*TSrf1 = CagdSrfRegionFromSrf(Srf, UMin, UMax, CAGD_CONST_U_DIR),
	*TSrf2 = CagdSrfRegionFromSrf(TSrf1, VMin, VMax, CAGD_CONST_V_DIR);

    CagdSrfFree(TSrf1);
    TSrf1 = CagdCoerceSrfTo(TSrf2, CAGD_PT_E1_TYPE, FALSE);
    CagdSrfFree(TSrf2);

    UV = BspSrfMaxCoefParam(TSrf1, 1, MaxVal);
    CagdSrfFree(TSrf1);

    if (IRIT_APX_EQ(UV[0], UMin))
	UV[0] = (2.0 * UMin + UMax) / 3.0;
    if (IRIT_APX_EQ(UV[0], UMax))
	UV[0] = (2.0 * UMax + UMin) / 3.0;
    if (IRIT_APX_EQ(UV[1], VMin))
	UV[1] = (2.0 * VMin + VMax) / 3.0;
    if (IRIT_APX_EQ(UV[1], VMax))
	UV[1] = (2.0 * VMax + VMin) / 3.0;

    return Dir == CAGD_CONST_U_DIR ? UV[0] : UV[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a single surface to set of rectangular domains	     *
* approximating it.							     *
*   FineNess is control via a function that returns an error measure	     *
* SrfPolyApproxError that is guaranteed to be less than Tolerance.	     *
*   The returned Connectivity structure is used to fill in "black holes".    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:               To convert and approximate using triangles.           *
*   SubdivDepth:       Depth of recursion (subdivision) of this function.    *
*   Tolerance:         Accuracy control.                                     *
*   SubdivDirStrategy: Alternatively in U and V, direction that minimizes    *
*		       the error, etc.					     *
*   SrfPolyApproxErr:  Using bilinear curvature estimate, k1^2 + k2^2	     *
*		       estimate, etc.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   ConnectivityStruct:   Subdivided regions with adjacency topology.        *
*****************************************************************************/
static ConnectivityStruct SymbSrf2OptimalPolygonsAux(
				CagdSrfStruct *Srf,
				int SubdivDepth,
				CagdRType Tolerance,
				SymbPlSubdivStrategyType SubdivDirStrategy,
				SymbPlErrorFuncType SrfPolyApproxErr)
{
    CagdRType UMin, UMax, VMin, VMax, u, v,
	RetTol = 1.0;
    int UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	Neighbors = AttrGetIntAttrib(Srf -> Attr, "_Neighbors");
    CagdBType
	HasUDiscont = Srf -> UKnotVector != NULL &&
		    BspKnotC1Discont(Srf -> UKnotVector, UOrder, ULength, &u),
	HasVDiscont = Srf -> VKnotVector != NULL &&
		    BspKnotC1Discont(Srf -> VKnotVector, VOrder, VLength, &v);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

#ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDomain, FALSE)
	    IRIT_INFO_MSG_PRINTF(
		    "Umin = %lf, Umax = %lf, Vmin = %lf, Vmax = %lf (%d, %d)\n",
		    UMin, UMax, VMin, VMax, HasUDiscont, HasVDiscont);
    }
#endif /* DEBUG */

    if (!(HasUDiscont || HasVDiscont) &&
	(UMax - UMin < IRIT_EPS * 5 ||
	 VMax - VMin < IRIT_EPS * 5 ||
	 (RetTol = SrfPolyApproxErr(Srf, CAGD_NO_DIR, SubdivDepth))
								< Tolerance)) {
	int i;
	ConnectivityStruct Connectivity;
	RectangularStruct
	    *Rect = (RectangularStruct *) IritMalloc(sizeof(RectangularStruct));

	Rect -> Valid = RetTol >= 0.0;

	CagdSrfDomain(Srf,
		      &Rect -> UV[0][0], &Rect -> UV[2][0],
		      &Rect -> UV[0][1], &Rect -> UV[2][1]);
	Rect -> UV[1][0] = Rect -> UV[2][0];
	Rect -> UV[1][1] = Rect -> UV[0][1];
	Rect -> UV[3][0] = Rect -> UV[0][0];
	Rect -> UV[3][1] = Rect -> UV[2][1];

	for (i = 0; i < 4; i++) {
	    CagdRType
		*R = CagdSrfEval(Srf, Rect -> UV[i][0], Rect -> UV[i][1]);

	    CagdCoerceToE3(Rect -> Point[i], &R, -1, Srf -> PType);

	    if (GlblNormalSrf != NULL)
	        CagdEvaluateSurfaceVecField(Rect -> Normal[i], GlblNormalSrf,
					    Rect -> UV[i][0],
					    Rect -> UV[i][1]);

	    Rect -> Depend[i] = NULL;
	}
	IRIT_LIST_PUSH(Rect, GlblRectList);

	Connectivity.Left = MakeEdge(Rect, 3);
	Connectivity.Right = MakeEdge(Rect, 1);
	Connectivity.Top = MakeEdge(Rect, 0);
	Connectivity.Bottom = MakeEdge(Rect, 2);
	return Connectivity;
    }
    else {
	int NeighborU1 = Neighbors & ~UMAX_BOUNDARY,
	    NeighborU2 = Neighbors & ~UMIN_BOUNDARY,
	    NeighborV1 = Neighbors & ~VMAX_BOUNDARY,
	    NeighborV2 = Neighbors & ~VMIN_BOUNDARY;
	CagdRType UDiv, VDiv, ErrU1, ErrU2, ErrV1, ErrV2,
	    MaxUIsoCrvtr, MaxVIsoCrvtr;
	CagdSrfStruct *SrfU1, *SrfU2, *SrfV1, *SrfV2;
	ConnectivityStruct Connectivity, Connectivity1, Connectivity2;
	CagdSrfDirType SubdivDir;

	if (HasUDiscont) {
	    NeighborU1 = Neighbors | UMAX_BOUNDARY;
	    NeighborU2 = Neighbors | UMIN_BOUNDARY;
	    UDiv = u;
	}
	else if (GlblUIsoCrvtrSqrSrf != NULL)
	    UDiv = ExtremumNodeValue(GlblVIsoCrvtrSqrSrf,
				     UMin, UMax, VMin, VMax,
				     CAGD_CONST_U_DIR, &MaxVIsoCrvtr);
	else if (ULength > UOrder)
	    UDiv = Srf -> UKnotVector[(Srf -> ULength + Srf -> UOrder) >> 1];
	else
	    UDiv = (UMin + UMax) * 0.5;

	if (HasVDiscont) {
	    NeighborV1 = Neighbors | VMAX_BOUNDARY;
	    NeighborV2 = Neighbors | VMIN_BOUNDARY;
	    VDiv = v;
	}
	else if (GlblVIsoCrvtrSqrSrf != NULL)
	    VDiv = ExtremumNodeValue(GlblUIsoCrvtrSqrSrf,
				     UMin, UMax, VMin, VMax,
				     CAGD_CONST_V_DIR, &MaxUIsoCrvtr);
	else if (VLength > VOrder)
	    VDiv = Srf -> VKnotVector[(Srf -> VLength + Srf -> VOrder) >> 1];
	else
	    VDiv = (VMin + VMax) * 0.5;
	
	SrfU1 = CagdSrfSubdivAtParam(Srf, UDiv, CAGD_CONST_U_DIR);
	SrfU2 = SrfU1 -> Pnext;
	AttrSetIntAttrib(&SrfU1 -> Attr, "_Neighbors", NeighborU1);
	AttrSetIntAttrib(&SrfU2 -> Attr, "_Neighbors", NeighborU2);
	
	SrfV1 = CagdSrfSubdivAtParam(Srf, VDiv, CAGD_CONST_V_DIR);
	SrfV2 = SrfV1 -> Pnext;
	AttrSetIntAttrib(&SrfV1 -> Attr, "_Neighbors", NeighborV1);
	AttrSetIntAttrib(&SrfV2 -> Attr, "_Neighbors", NeighborV2);

	if (HasUDiscont)
	    SubdivDir = CAGD_CONST_U_DIR;
	else if (HasVDiscont)
	    SubdivDir = CAGD_CONST_V_DIR;
	else {
	    if (GlblUIsoCrvtrSqrSrf != NULL && GlblVIsoCrvtrSqrSrf != NULL)
	        SubdivDir = MaxUIsoCrvtr > MaxVIsoCrvtr ? CAGD_CONST_V_DIR
							: CAGD_CONST_U_DIR;

	    switch (SubdivDirStrategy) {
		case SYMB_SUBDIV_STRAT_MIN_MAX:
		    ErrU1 = SrfPolyApproxErr(SrfU1, CAGD_CONST_U_DIR,
					     SubdivDepth);
		    ErrU2 = SrfPolyApproxErr(SrfU2, CAGD_CONST_U_DIR,
					     SubdivDepth);
		    ErrV1 = SrfPolyApproxErr(SrfV1, CAGD_CONST_V_DIR,
					     SubdivDepth);
		    ErrV2 = SrfPolyApproxErr(SrfV2, CAGD_CONST_V_DIR,
					     SubdivDepth);
		    SubdivDir = IRIT_MAX(ErrU1, ErrU2) > IRIT_MAX(ErrV1, ErrV2)
			                ? CAGD_CONST_V_DIR : CAGD_CONST_U_DIR;
		    break;
		default:
		case SYMB_SUBDIV_STRAT_ALTERNATE:
		    SubdivDir = SubdivDepth % 2 ?
			CAGD_CONST_U_DIR : CAGD_CONST_V_DIR;
		    break;
	    }
	}

	if (SubdivDir == CAGD_CONST_V_DIR) {
	    /* Use the V subdivision. */
	    Connectivity1 = SymbSrf2OptimalPolygonsAux(SrfV1, SubdivDepth + 1,
						       Tolerance,
						       SubdivDirStrategy,
						       SrfPolyApproxErr);
	    Connectivity2 = SymbSrf2OptimalPolygonsAux(SrfV2, SubdivDepth + 1,
						       Tolerance,
						       SubdivDirStrategy,
						       SrfPolyApproxErr);

	    Connectivity = MergeConnectivityEdge(&Connectivity1,
						 &Connectivity2,
						 CAGD_CONST_V_DIR);
	}
	else {
	    /* Use the U subdivision. */
	    Connectivity1 = SymbSrf2OptimalPolygonsAux(SrfU1, SubdivDepth + 1,
						       Tolerance,
						       SubdivDirStrategy,
						       SrfPolyApproxErr);
	    Connectivity2 = SymbSrf2OptimalPolygonsAux(SrfU2, SubdivDepth + 1,
						       Tolerance,
						       SubdivDirStrategy,
						       SrfPolyApproxErr);

	    Connectivity = MergeConnectivityEdge(&Connectivity1,
						 &Connectivity2,
						 CAGD_CONST_U_DIR);
	}
	CagdSrfFree(SrfU1);
	CagdSrfFree(SrfU2);
	CagdSrfFree(SrfV1);
	CagdSrfFree(SrfV2);
	return Connectivity;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Merges two connectivity information into one, destructively.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Connectivity1, Connectivity2:  The two regions to merge.		     *
*   Dir:                           Direction of merge. Either U or V.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   ConnectivityStruct:   Merged structure.				     *
*****************************************************************************/
static ConnectivityStruct MergeConnectivityEdge(
					ConnectivityStruct *Connectivity1,
					ConnectivityStruct *Connectivity2,
					CagdSrfDirType Dir)
{
    ConnectivityStruct Connectivity;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Connectivity.Left = Connectivity1 -> Left;
	    Connectivity.Right = Connectivity2 -> Right;
	    Connectivity.Top = ChainEdgeLists(Connectivity1 -> Top,
					      Connectivity2 -> Top);
	    Connectivity.Bottom = ChainEdgeLists(Connectivity1 -> Bottom,
						 Connectivity2 -> Bottom);
	    UpdateBlackHoles(Connectivity1 -> Right, Connectivity2 -> Left,
			     CAGD_CONST_U_DIR);
	    break;
	case CAGD_CONST_V_DIR:
	    Connectivity.Top = Connectivity1 -> Top;
	    Connectivity.Bottom = Connectivity2 -> Bottom;
	    Connectivity.Right = ChainEdgeLists(Connectivity1 -> Right,
						Connectivity2 -> Right);
	    Connectivity.Left = ChainEdgeLists(Connectivity1 -> Left,
					       Connectivity2 -> Left);
	    UpdateBlackHoles(Connectivity1 -> Bottom, Connectivity2 -> Top,
			     CAGD_CONST_V_DIR);
	    break;
	default:
	    Connectivity.Top =
		Connectivity.Bottom =
		    Connectivity.Right =
		        Connectivity.Left = NULL;

	    SYMB_FATAL_ERROR(SYMB_ERR_DIR_NOT_CONST_UV);
    }
    return Connectivity;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Test for black holes between two adjacent edges and compensate as needed.  *
*                                                                            *
* PARAMETERS:                                                                *
*   L1, L2:  Two edges to search and fill black holes.                       *
*   Dir:     Direction of edge. Either U or V.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateBlackHoles(EdgeStruct *L1,
			     EdgeStruct *L2,
			     CagdSrfDirType Dir)
{
    while (L1 != NULL && L2 != NULL) {
	int Inc,
	    Index1 = L1 -> Index,
	    Index2 = L2 -> Index;
	RectangularStruct
	    *Rect1 = L1 -> Rect,
	    *Rect2 = L2 -> Rect;

	Inc = UpdateOneBlackHole(Rect1, Index1, Rect2, Index2, Dir);

	if (Inc & 0x01)
	    L1 = L1 -> Pnext;
	if (Inc & 0x02)
	    L2 = L2 -> Pnext;
    }

    if (L1 != NULL || L2 != NULL)
	SYMB_FATAL_ERROR(SYMB_ERR_INCONSIST_EDGE_BHOLE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Compares the two edges prescribed by the rectangular their are in and the  *
* index within the rectangular for collinearity and close black hole if any. *
*   Returns two bit flags which edge should be incremented.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Rect1, Index1a:   Index of first edge and rectangular region it is in.   *
*   Rect2, Index2a:   Index of second edge and rectangular region it is in.  *
*   Dir:              Direction of test for black hole. Either U or V.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:              Which edge should be incremented. First bit first edge *
*                     second bit second edge.                                *
*****************************************************************************/
static int UpdateOneBlackHole(RectangularStruct *Rect1,
			      int Index1a,
			      RectangularStruct *Rect2,
			      int Index2a,
			      CagdSrfDirType Dir)
{
    int Index1b = (Index1a + 1) % 4,
	Index2b = (Index2a + 1) % 4,
	UVDir = Dir == CAGD_CONST_U_DIR ? 1 : 0;
    CagdRType *P1a, *P1b, *P2a, *P2b, *N1a, *N1b, *N2a, *N2b,
	R1a = Rect1 -> UV[Index1a][UVDir],
	R1b = Rect1 -> UV[Index1b][UVDir],
	R2a = Rect2 -> UV[Index2a][UVDir],
	R2b = Rect2 -> UV[Index2b][UVDir];

    if (IRIT_APX_EQ(R1a, R2a) && IRIT_APX_EQ(R2a, R2b)) {
	/* Eliminate the trivial case of no black hole. */
	return 0x03;
    }

    /* Make sure points are sorted in edge. */
    if (R1a > R1b) {
	IRIT_SWAP(CagdRType, R1a, R1b);
	IRIT_SWAP(int, Index1a, Index1b);
    }
    P1a = Rect1 -> Point[Index1a];
    P1b = Rect1 -> Point[Index1b];
    N1a = Rect1 -> Normal[Index1a];
    N1b = Rect1 -> Normal[Index1b];

    if (R2a > R2b) {
	IRIT_SWAP(CagdRType, R2a, R2b);
	IRIT_SWAP(int, Index2a, Index2b);
    }
    P2a = Rect2 -> Point[Index2a];
    P2b = Rect2 -> Point[Index2b];
    N2a = Rect2 -> Normal[Index2a];
    N2b = Rect2 -> Normal[Index2b];

    if (R1a <= R2a && R1b >= R2b) {
	/* Edge 1 contains edge 2. */
	if (!IRIT_APX_EQ(R2a, R1a)) {
	    ProjectPointOnLine(P2a, P1a, P1b, N2a, N1a, N1b);
	    MakePointDepend(Rect2, Index2a, Rect1, Index1a, Index1b);
	    MakePointDepend(Rect2, Index2a, Rect1, Index1b, Index1a);
	    PropagateDependency(Rect2, Index2a);
	}
	if (!IRIT_APX_EQ(R2b, R1b)) {
	    ProjectPointOnLine(P2b, P1a, P1b, N2b, N1a, N1b);
	    MakePointDepend(Rect2, Index2b, Rect1, Index1a, Index1b);
	    MakePointDepend(Rect2, Index2b, Rect1, Index1b, Index1a);
	    PropagateDependency(Rect2, Index2b);
	}

	return IRIT_APX_EQ(R1b, R2b) ? 0x03 : 0x02;
    }
    else if (R2a <= R1a && R2b >= R1b) {
	/* Edge 2 contains edge 1. */
	if (!IRIT_APX_EQ(R1a, R2a)) {
	    ProjectPointOnLine(P1a, P2a, P2b, N1a, N2a, N2b);
	    MakePointDepend(Rect1, Index1a, Rect2, Index2b, Index2a);
	    MakePointDepend(Rect1, Index1a, Rect2, Index2a, Index2b);
	    PropagateDependency(Rect1, Index1a);
	}
	if (!IRIT_APX_EQ(R1b, R2b)) {
	    ProjectPointOnLine(P1b, P2a, P2b, N1b, N2a, N2b);
	    MakePointDepend(Rect1, Index1b, Rect2, Index2b, Index2a);
	    MakePointDepend(Rect1, Index1b, Rect2, Index2a, Index2b);
	    PropagateDependency(Rect1, Index1b);
	}

	return IRIT_APX_EQ(R1b, R2b) ? 0x03 : 0x01;
    }
    else {
	/* Two edges only share part of their domain. Rarely happens. */
	if (GlblFoundPartialShare == FALSE) {
	    IRIT_WARNING_MSG("Warning: Partial adjacency share detected\n");
	    GlblFoundPartialShare = TRUE;
	}

	if (R1a < R2a) {
	    if (R1b < R2b) {
		ProjectPointOnLine(P2a, P1a, P2b, N2a, N1a, N2b);
		ProjectPointOnLine(P1b, P1a, P2b, N1b, N1a, N2b);
	    }
	    else {
		ProjectPointOnLine(P2a, P1a, P1b, N2a, N1a, N1b);
		ProjectPointOnLine(P2b, P1a, P1b, N2b, N1a, N1b);
	    }
	}
	else {
	    if (R1b < R2b) {
		ProjectPointOnLine(P1a, P2a, P2b, N1a, N2a, N2b);
		ProjectPointOnLine(P1b, P2a, P2b, N1b, N2a, N2b);
	    }
	    else {
		ProjectPointOnLine(P1a, P2a, P1b, N1a, N2a, N1b);
		ProjectPointOnLine(P2b, P2a, P1b, N2b, N2a, N1b);
	    }
	}

	return R1b > R2b ? 0x02 : 0x01;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Make Depend point depend on the two Master points.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   DependRect, DependIndex:  Point to become dependent on 1 and 2.	     *
*   MasterRect, Master1Index, Master2Index:  The points to depend on.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MakePointDepend(RectangularStruct *DependRect,
			    int DependIndex,
			    RectangularStruct *MasterRect,
			    int Master1Index,
			    int Master2Index)
{
    PointDependStruct
	*PtDepend1 = (PointDependStruct *) IritMalloc(sizeof(PointDependStruct)),
	*PtDepend2 = (PointDependStruct *) IritMalloc(sizeof(PointDependStruct));

    PtDepend1 -> Rect = PtDepend2 -> Rect = DependRect;
    PtDepend1 -> Index = PtDepend2 -> Index = DependIndex;
    PtDepend1 -> OtherIndex = Master2Index;
    PtDepend2 -> OtherIndex = Master1Index;

    PtDepend1 -> Pnext = MasterRect -> Depend[Master1Index];
    MasterRect -> Depend[Master1Index] = PtDepend1;
    PtDepend2 -> Pnext = MasterRect -> Depend[Master2Index];
    MasterRect -> Depend[Master2Index] = PtDepend2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Propagate through dependency information the fact the this point changed.  *
*                                                                            *
* PARAMETERS:                                                                *
*   Rect Index:   "this point".						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PropagateDependency(RectangularStruct *Rect, int Index)
{
    PointDependStruct *PtDepend;

    for (PtDepend = Rect -> Depend[Index];
	 PtDepend != NULL;
	 PtDepend = PtDepend -> Pnext) {
	ProjectPointOnLine(PtDepend -> Rect -> Point[PtDepend -> Index],
			   Rect -> Point[Index],
			   Rect -> Point[PtDepend -> OtherIndex],
			   PtDepend -> Rect -> Normal[PtDepend -> Index],
			   Rect -> Normal[Index],
			   Rect -> Normal[PtDepend -> OtherIndex]);
	PropagateDependency(PtDepend -> Rect, PtDepend -> Index);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Update Pt to the closest point on line Pt1Pt2.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:            Point to be projected on line Pt1Pt2, closing black hole. *
*   Pt1, Pt2:      The line to project Pt on.				     *
*   Nrml:          To be computed for Pt from normals at Pt1, Pt2.           *
*   Nrml1, Nrml2:  Normals at Pt1 and Pt2.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProjectPointOnLine(CagdPType Pt,
			       CagdPType Pt1,
			       CagdPType Pt2,
			       CagdPType Nrml,
			       CagdPType Nrml1,
			       CagdPType Nrml2)
{
    CagdRType t;
    CagdVType Vec;
    CagdPType NewPt;

    IRIT_PT_SUB(Vec, Pt1, Pt2);

    GMPointFromPointLine(Pt, Pt1, Vec, NewPt);
    IRIT_PT_COPY(Pt, NewPt);

    if (GlblNormalSrf != NULL) {
	t = sqrt(IRIT_PT_PT_DIST_SQR(Pt, Pt2) / IRIT_PT_PT_DIST_SQR(Pt1, Pt2));
	IRIT_PT_BLEND(Nrml, Nrml1, Nrml2, t);
	IRIT_PT_NORMALIZE(Nrml);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Concatenates two edge lists, destructively.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   L1, L2:   The two edge list to concatenate.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   EdgeStruct *:   Concatenated edge list.                                  *
*****************************************************************************/
static EdgeStruct *ChainEdgeLists(EdgeStruct *L1, EdgeStruct *L2)
{
    EdgeStruct
	*L = L1;

    if (L1 == NULL)
	return L2;
    else {
	while (L1 -> Pnext != NULL)
	    L1 = L1 -> Pnext;
	L1 -> Pnext = L2;

	return L;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Makes a single triangle from given data.				     *
* Returns NULL if Triangle is degenerated.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Nrml1, UV1:   Coefficients of first vertex.			     *
*   Pt2, Nrml2, UV2:   Coefficients of second vertex.			     *
*   Pt3, Nrml3, UV3:   Coefficients of third vertex.			     *
*   ComputeUV:         Do we have valid UV information in UV??               *
*   ComputeNrmls:      Do we have valid normal information in Nrml??         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:    Constructed triangle, or NULL if degenerate.     *
*****************************************************************************/
static CagdPolygonStruct *MakeTriangle(CagdPType Pt1,
				       CagdVType Nrml1,
				       CagdUVType UV1,
				       CagdPType Pt2,
				       CagdVType Nrml2,
				       CagdUVType UV2,
				       CagdPType Pt3,
				       CagdVType Nrml3,
				       CagdUVType UV3,
				       CagdBType ComputeUV,
				       CagdBType ComputeNrmls)
{
    int i;
    CagdPolygonStruct *Poly;

   if (GMCollinear3Pts(Pt1, Pt2, Pt3))
	return NULL;

    Poly = CagdPolygonNew(3);

    IRIT_PT_COPY(Poly -> U.Polygon[0].Pt, Pt1);
    IRIT_PT_COPY(Poly -> U.Polygon[1].Pt, Pt2);
    IRIT_PT_COPY(Poly -> U.Polygon[2].Pt, Pt3);

    if (ComputeUV) {
	IRIT_UV_COPY(Poly -> U.Polygon[0].UV, UV1);
	IRIT_UV_COPY(Poly -> U.Polygon[1].UV, UV2);
	IRIT_UV_COPY(Poly -> U.Polygon[2].UV, UV3);
    }
    else {
	for (i = 0; i < 3; i++)
	    IRIT_UV_RESET(Poly -> U.Polygon[i].UV);
    }

    if (ComputeNrmls) {
	IRIT_PT_COPY(Poly -> U.Polygon[0].Nrml, Nrml1);
	IRIT_PT_COPY(Poly -> U.Polygon[1].Nrml, Nrml2);
	IRIT_PT_COPY(Poly -> U.Polygon[2].Nrml, Nrml3);
    }
    else {
	for (i = 0; i < 3; i++)
	    IRIT_PT_RESET(Poly -> U.Polygon[i].Nrml);
    }

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Makes a single edge from given data.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Rect, Index:    Given the rectangle and the index inside...		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   EdgeStruct *:   ...Fetch out the requrested egde.                        *
*****************************************************************************/
static EdgeStruct *MakeEdge(RectangularStruct *Rect, int Index)
{
    EdgeStruct
	*Edge = (EdgeStruct *) IritMalloc(sizeof(EdgeStruct));

    Edge -> Rect = Rect;
    Edge -> Index = Index;
    Edge -> Pnext = NULL;

    return Edge;
}
