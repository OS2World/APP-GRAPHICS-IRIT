/******************************************************************************
* Smp_Skel.c - computation of simple curve/surface skeletons.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, August 98    				      *
******************************************************************************/

#include "symb_loc.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"

#define CONTOUR_EPS	   1e-8     /* Level above zero to actually contour. */
#define SRF_SCALE_FACTOR   1	    /* Scaling factor for better contouring. */
#define NORMAL_SAME_DIR	   0.9999
#define MINIMAL_VALID_ARC_LEN 1e-4

#define SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf, Mat) {  \
			CagdSrfStruct *_TSrf = CagdSrfMatTransform(Srf, Mat); \
			CagdSrfFree(Srf); \
			Srf = _TSrf; } 

IRIT_STATIC_DATA CagdRType
    GlblOrigin[3] = { 0.0, 0.0, 0.0 },
    GlblXYPlane[4] = { 0.0, 0.0, 1.0, 0.0 };

static CagdSrfStruct *SymbSrfIsolateRational(CagdSrfStruct **Srf,
					     int RetWeights);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector curve on a sphere of a point and a curve, both     M
* assumed to be on the unit sphere                                           M
*   The following assumtion are made and must be met for proper answer:      M
* 1. Both the curve and the point are indeed on the unit sphere.	     M
* 2. Both the curve and the point are on the northern hemisphere.  That is   M
*    the Z coefficients of all points on both Pt and Crv are positive.       M
*   The end result is NOT a curve on the sphere but rather a rational curve  M
* in the Z = 1 plane whose central projection onto the sphere (i.e.          M
* normalization) would yield the proper bisector on the unit sphere.	     M
*   Let P be the bisector point. Then, the following must be satisified:     M
*      < P, Pt > = < P, C(t) >   (Equality of angular distance)              V
*      < P - C(t), C'(t) > = 0   (orthogonality of distance measure)         V
*      < P, (0, 0, 1) > = 1      (containment in the Z = 1 plane, or Pz = 1) V
*   Note we have only two unknowns (Px, Py) as Pz equals 1.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:        A point on the unit sphere, on the northern hemisphere.       M
*   CCrv:      A curve on the unit sphere, on the northern hemisphere.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The bisector curve on Z = 1 plane to be centrally     M
*		projected onto the unit sphere as the spherical bisector.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCrvBisectOnSphere				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPtCrvBisectOnSphere, bisectors, skeleton                             M
*****************************************************************************/
CagdCrvStruct *SymbPtCrvBisectOnSphere(const CagdPType Pt,
				       const CagdCrvStruct *CCrv)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(CCrv -> PType);
    CagdPType Trans;
    CagdCrvStruct *DCrv, *DCrvE3, *CrvE3, *A11, *A12, *A21, *A22, *B1, *B2,
	*TCrv1, *TCrv2, *TCrv3, *TCrv4, *DCrvW, *DCrvX, *DCrvY, *DCrvZ,
        *CrvW, *CrvX, *CrvY, *CrvZ,
        *Crv = CagdCoerceCrvTo(CCrv, IsRational ? CAGD_PT_P3_TYPE
					        : CAGD_PT_E3_TYPE, FALSE);

    DCrv = CagdCrvDerive(Crv);
    if (IsRational) {
	SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
	SymbCrvSplitScalar(DCrv, &DCrvW, &DCrvX, &DCrvY, &DCrvZ);
	CrvE3 = SymbCrvMergeScalar(NULL, CrvX, CrvY, CrvZ);
	DCrvE3 = SymbCrvMergeScalar(NULL, DCrvX, DCrvY, DCrvZ);

	A21 = SymbCrvMult(DCrvX, CrvW);
	A22 = SymbCrvMult(DCrvY, CrvW);

	TCrv1 = SymbCrvDotProd(CrvE3, DCrvE3);
	TCrv2 = SymbCrvMult(DCrvZ, CrvW);
	B2 = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);

	IRIT_PT_COPY(Trans, Pt);
	IRIT_PT_SCALE(Trans, -1.0);
	CagdCrvTransform(Crv, Trans, 1.0);
	SymbCrvSplitScalar(Crv, &TCrv1, &A11, &A12, &B1);
	CagdCrvTransform(B1, NULL, -1.0);
	CagdCrvFree(TCrv1);

	CagdCrvFree(CrvW);
	CagdCrvFree(CrvX);
	CagdCrvFree(CrvY);
	CagdCrvFree(CrvZ);
	CagdCrvFree(DCrvW);
	CagdCrvFree(DCrvX);
	CagdCrvFree(DCrvY);
	CagdCrvFree(DCrvZ);
	CagdCrvFree(CrvE3);
	CagdCrvFree(DCrvE3);
    }
    else {
	SymbCrvSplitScalar(DCrv, &TCrv1, &A21, &A22, &TCrv2);

	TCrv1 = SymbCrvDotProd(Crv, DCrv);
	B2 = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);

	IRIT_PT_COPY(Trans, Pt);
	IRIT_PT_SCALE(Trans, -1.0);
	CagdCrvTransform(Crv, Trans, 1.0);
	SymbCrvSplitScalar(Crv, &TCrv1, &A11, &A12, &B1);
	CagdCrvTransform(B1, NULL, -1.0);
    }
    CagdCrvFree(Crv);
    CagdCrvFree(DCrv);

    /* Solve the equations. */
    TCrv1 = SymbCrvDeterminant2(A11, A12, A21, A22);
    TCrv2 = SymbCrvDeterminant2(B1,  A12, B2,  A22);
    TCrv3 = SymbCrvDeterminant2(A11, B1,  A21, B2);

    CagdCrvFree(A11);
    CagdCrvFree(A12);
    CagdCrvFree(A21);
    CagdCrvFree(A22);
    CagdCrvFree(B1);
    CagdCrvFree(B2);

    CagdMakeCrvsCompatible(&TCrv1, &TCrv2, TRUE, TRUE);
    CagdMakeCrvsCompatible(&TCrv1, &TCrv3, TRUE, TRUE);
    CagdMakeCrvsCompatible(&TCrv2, &TCrv3, TRUE, TRUE);

    TCrv4 = SymbCrvMergeScalar(TCrv1, TCrv2, TCrv3, NULL);
    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);
    CagdCrvFree(TCrv3);

    TCrv1 = CagdCoerceCrvTo(TCrv4, CAGD_PT_P3_TYPE, FALSE);
    CagdCrvFree(TCrv4);

    IRIT_PT_RESET(Trans);
    Trans[2] = 1.0;
    CagdCrvTransform(TCrv1, Trans, 1.0);	    /* Bring to Z = 1 plane. */

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(TCrv1 -> Points, TCrv1 -> PType,
			   TCrv1 -> Length, TRUE);

    return TCrv1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector on a sphere between a point and a curve on the     M
* sphere.  The returned result is a piecewise linear curve on the sphere.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:           A point on the unit sphere, on the northern hemisphere.    M
*   CCrv:         A curve on the unit sphere, on the northern hemisphere.    M
*   Tolerance:    Accuracy of computation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A piecewise linear curve approximating the bisector    M
*	        of Crv1 and Crv2 on the sphere.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere, SymbCrvCrvBisectOnSphere                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPtCrvBisectOnSphere2, bisectors, skeleton                            M
*****************************************************************************/
CagdCrvStruct *SymbPtCrvBisectOnSphere2(const CagdPType Pt,
					const CagdCrvStruct *CCrv,
					CagdRType Tolerance)
{
    int i, Length;
    CagdRType **Points;
    CagdCrvStruct *TCrv, *Crv,
	*RetList = NULL,
	*BisectCrv = SymbPtCrvBisectOnSphere(Pt, CCrv);
    CagdPolylineStruct
	*BisectPoly = SymbCrv2Polyline(BisectCrv, Tolerance,
				       SYMB_CRV_APPROX_UNIFORM, TRUE);

    CagdCrvFree(BisectCrv);

    /* Convert to a piecewise linear curve and normalize. */
    Crv = CagdCnvrtPolyline2LinBspCrv(BisectPoly);
    Points = Crv -> Points;
    Length = Crv -> Length;

    for (i = 0; i < Length; i++) {
	CagdRType
	    Len = sqrt(IRIT_SQR(Points[1][i]) +
		       IRIT_SQR(Points[2][i]) +
		       IRIT_SQR(Points[3][i]));

	if (Len > IRIT_PT_NORMALIZE_ZERO) {
	    Points[1][i] /= Len;
	    Points[2][i] /= Len;
	    Points[3][i] /= Len;
	}
    }

    CagdPolylineFree(BisectPoly);

    while (TRUE) {
	CagdRType
	    **Points = Crv -> Points;

	for (i = 1; i < Length; i++) {
	    if (Points[1][i - 1] * Points[1][i] +
		Points[2][i - 1] * Points[2][i] +
		Points[3][i - 1] * Points[3][i] < 0.0) {
		/* Need to split curve here. */
		if (i > 1) {
		    TCrv = CagdCrvRegionFromCrv(Crv, 0, (i - 1.0) / Length);
		    IRIT_LIST_PUSH(TCrv, RetList);
		}
		if (i < Length - 1) {
		    TCrv = CagdCrvRegionFromCrv(Crv, (i + 0.5) / Length,
						1.0);
		    BspKnotUniformOpen(TCrv -> Length, TCrv -> Order,
				       TCrv -> KnotVector);
		}
	        else
		    TCrv = NULL;

		CagdCrvFree(Crv);
		Crv = TCrv;
		Length = Crv -> Length;
		break;
	    }
	}

	if (i >= Crv -> Length) {
	    IRIT_LIST_PUSH(Crv, RetList);
	    break;
	}
    }

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(Crv -> Points, Crv -> PType, Crv -> Length, TRUE);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector on a sphere between two curves, both are assumed   M
* to be on the unit sphere.                                                  M
*   The end result is a bivariate function NOT a curve on the sphere but     M
* rather a rational surface in the (s, t) parameter space of Crv1(s) and     M
* Crv2(t) whose zero set provides the correspondancing bisector in the       M
* parametric space.							     M
*   Let P be the bisector point. Then, the following must vanish:	     M
*      < P, C1(t) > = < P, C2(t) > (Equality of angular distance)            V
*      < P - C1(t), C1'(t) > = 0   (orthogonality of distance measure)       V
*      < P - C2(t), C2'(t) > = 0   (orthogonality of distance measure)       V
*   Then the returned result is the determinant of these three equations     M
* that must vanish.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2:  Two curves on the unit sphere.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The bisector surface bivariate function whose zero    M
*	set provides the bisector correspondance in the parameteric space.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere2, SymbPtCrvBisectOnSphere3	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrvBisectOnSphere, bisectors, skeleton                            M
*****************************************************************************/
CagdSrfStruct *SymbCrvCrvBisectOnSphere(const CagdCrvStruct *CCrv1,
					const CagdCrvStruct *CCrv2)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(CCrv1 -> PType);
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2,
	*A11, *A12, *A13, *A21, *A22, *A23, *A31, *A32, *A33, *TSrf1, *TSrf2;
    CagdCrvStruct
	*Crv1 = CagdCoerceCrvTo(CCrv1, IsRational ? CAGD_PT_P3_TYPE
					          : CAGD_PT_E3_TYPE, FALSE),
	*Crv2 = CagdCoerceCrvTo(CCrv2, IsRational ? CAGD_PT_P3_TYPE
					          : CAGD_PT_E3_TYPE, FALSE);

    Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR);
    Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    BspKnotAffineTrans2(Srf1 -> VKnotVector, Srf1 -> VLength + Srf1 -> VOrder,
			VMin2, VMax2);
    BspKnotAffineTrans2(Srf2 -> UKnotVector, Srf2 -> ULength + Srf2 -> UOrder,
			UMin1, UMax1);

    TSrf1 = SymbSrfSub(Srf1, Srf2);
    DSrf1 = CagdSrfDerive(Srf1, CAGD_CONST_U_DIR);
    DSrf2 = CagdSrfDerive(Srf2, CAGD_CONST_V_DIR);
    if (IsRational) {
	SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
	CagdSrfFree(TSrf2);
	SymbSrfSplitScalar(DSrf1, &TSrf2, &A21, &A22, &A23);
	CagdSrfFree(TSrf2);
	SymbSrfSplitScalar(DSrf2, &TSrf2, &A31, &A32, &A33);
	CagdSrfFree(TSrf2);
    }
    else {
	SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
	SymbSrfSplitScalar(DSrf1, &TSrf2, &A21, &A22, &A23);
	SymbSrfSplitScalar(DSrf2, &TSrf2, &A31, &A32, &A33);
    }
    CagdSrfFree(TSrf1);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf2);

    /* Solve the equations. */
    TSrf1 = SymbSrfDeterminant3(A11, A12, A13,
				A21, A22, A23,
				A31, A32, A33);

    CagdSrfFree(A11);
    CagdSrfFree(A12);
    CagdSrfFree(A13);
    CagdSrfFree(A21);
    CagdSrfFree(A22);
    CagdSrfFree(A23);
    CagdSrfFree(A31);
    CagdSrfFree(A32);
    CagdSrfFree(A33);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(TSrf1 -> Points, TSrf1 -> PType,
			   TSrf1 -> ULength * TSrf1 -> VLength, TRUE);

    return TSrf1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector on a sphere between two curves on the sphere.      M
* The returned result is a piecewise linear curve on the sphere.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   Two curves on the unit sphere.                             M
*   Tolerance:    Accuracy of computation.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of piecewise linear curves approximating the    M
*	       bisectors of Crv1 and Crv2 on the sphere.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere, SymbCrvCrvBisectOnSphere,                       M
*   SymbCrvCrvBisectOnSphere3				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrvBisectOnSphere2, bisectors, skeleton                           M
*****************************************************************************/
CagdCrvStruct *SymbCrvCrvBisectOnSphere2(const CagdCrvStruct *Crv1,
					 const CagdCrvStruct *Crv2,
					 CagdRType Tolerance)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };    /* A scalar srf - only X. */
    int Len;
    CagdSrfStruct
	*BisectSrf = SymbCrvCrvBisectOnSphere(Crv1, Crv2);
    IPVertexStruct *V;
    IPPolygonStruct *Cntrs, *Cntr, *PrevCntr;
    CagdCrvStruct
	*PLCrvList = NULL;
    CagdBBoxStruct BBoxTmp, BBox;

    CagdSrfTransform(BisectSrf, NULL, SRF_SCALE_FACTOR);

    /* Computes the zero set of the equation as contours. */
    Cntrs = UserCntrSrfWithPlane(BisectSrf, Plane, Tolerance);
    CagdSrfFree(BisectSrf);

    CagdCrvBBox(Crv1, &BBox);
    CagdCrvBBox(Crv2, &BBoxTmp);
    CagdMergeBBox(&BBox, &BBoxTmp);

    /* Get the st parameters and convert to the bisector's position. */
    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct
	    *VPrev = NULL;
	int i;

	Len = IPVrtxListLen(Cntr -> PVertex);
	if (Len < 2)
	    continue;
 
	for (V = Cntr -> PVertex, i = 0;
	     V != NULL;
	     VPrev = V, V = V -> Pnext, i++) {
	    CagdRType *R, t1, t2;
	    CagdPType Pos1, Pos2, Inter1, Inter2;
	    CagdVType Nrml1, Nrml2;
	    CagdVecStruct *Vec;

	    R = CagdCrvEval(Crv1, V -> Coord[1]);
	    CagdCoerceToE3(Pos1, &R, -1, Crv1 -> PType);
	    Vec = CagdCrvTangent(Crv1, V -> Coord[1], FALSE);
	    IRIT_CROSS_PROD(Nrml1, Vec -> Vec, Pos1);
	    IRIT_VEC_NORMALIZE(Nrml1);

	    R = CagdCrvEval(Crv2, V -> Coord[2]);
	    CagdCoerceToE3(Pos2, &R, -1, Crv2 -> PType);
	    Vec = CagdCrvTangent(Crv2, V -> Coord[2], FALSE);
	    IRIT_CROSS_PROD(Nrml2, Vec -> Vec, Pos2);
	    IRIT_VEC_NORMALIZE(Nrml2);

	    if (IRIT_FABS(IRIT_DOT_PROD(Nrml1, Nrml2) > NORMAL_SAME_DIR)) {
		if (V -> Pnext != NULL) {
		    /* Break the contour here. */
		    Cntr -> Pnext = IPAllocPolygon(0, V -> Pnext,
						   Cntr -> Pnext);
		}
		IPFreeVertex(V);
		if (VPrev != NULL)
		    VPrev -> Pnext = NULL;

		break;			    /* No more work on this polygon. */
	    }
	    else {
		GM2PointsFromLineLine(Pos1, Nrml1, Pos2, Nrml2,
				      Inter1, &t1, Inter2, &t2);

		IRIT_PT_BLEND(V -> Coord, Inter1, Inter2, 0.5);
		IRIT_PT_NORMALIZE(V -> Coord);
		if (VPrev != NULL &&
		    IRIT_DOT_PROD(VPrev -> Coord, V -> Coord) < 0.0) {
		    IRIT_PT_SCALE(V -> Coord, -1.0);
		}
	    }
	}
    }

    /* Filter out contours that are too short. */
    for (PrevCntr = NULL, Cntr = Cntrs; Cntr != NULL; ) {
	CagdBType
	    Delete = FALSE;
	CagdRType
	    Len = 0.0;
	IPVertexStruct
	    *V = Cntr -> PVertex;

	if (V != NULL) {
	    for ( ; V -> Pnext != NULL; V = V -> Pnext) {
	        CagdPType Pt;
		IRIT_PT_SUB(Pt, V -> Coord, V -> Pnext -> Coord);

		Len += IRIT_PT_LENGTH(Pt);
		if (Len > MINIMAL_VALID_ARC_LEN)
		    break;
	    }
	    Delete = Len < MINIMAL_VALID_ARC_LEN;
	}
	else
	    Delete = TRUE;

	if (Delete) {
	    if (PrevCntr == NULL) {
		Cntrs = Cntrs -> Pnext;
		Cntr -> Pnext = NULL;
		IPFreePolygon(Cntr);
		Cntr = Cntrs;
	    }
	    else {
		PrevCntr -> Pnext = Cntr -> Pnext;
		Cntr -> Pnext = NULL;
		IPFreePolygon(Cntr);
		Cntr = PrevCntr -> Pnext;
	    }
	}
	else {
	    PrevCntr = Cntr;
	    Cntr = Cntr -> Pnext;
	}
    }

    if (Cntrs == NULL)
	return NULL;

    /* Make an antipodal copy of all contours and place as well into output. */
    Cntr = IPGetLastPoly(Cntrs);
    Cntr -> Pnext = IPCopyPolygonList(Cntrs);
    for (Cntr = Cntr -> Pnext; Cntr != NULL; Cntr = Cntr -> Pnext) {
	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext)
	    IRIT_PT_SCALE(V -> Coord, -1.0);
    }

    Cntrs = GMMergePolylines(Cntrs, IRIT_EPS);

    /* Convert to curves while ignoring too short contours. */
    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	Len = IPVrtxListLen(Cntr -> PVertex);
	if (Len >= 2) {
	    CagdCrvStruct
		*PLCrv = IPPolyline2Curve(Cntr, 2);

	    if (CagdCrvArcLenPoly(PLCrv) < MINIMAL_VALID_ARC_LEN) {
		CagdCrvFree(PLCrv);
	    }
	    else
		IRIT_LIST_PUSH(PLCrv, PLCrvList);
	}
    }

    IPFreePolygonList(Cntrs);

    return PLCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector of two cone surfaces sharing an apex at the origin M
* represented as their generating curves on a unit sphere.		     M
*   Let P be the bisector point. Then, the following must vanish:	     M
*      < P, C1(t) > = < P, C2(t) > (Equality of angular distance)            V
*      < P - C1(t), C1'(t) > = 0   (orthogonality of distance measure)       V
*      < P - C2(t), C2'(t) > = 0   (orthogonality of distance measure)       V
*   Then, the returned is the solution of the above equations.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2: two curves on the unit sphere.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The bisector surface function.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere, SymbPtCrvBisectOnSphere,                        M
*   SymbPtCrvBisectOnSphere2				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrvBisectOnSphere3, bisectors, skeleton                           M
*****************************************************************************/
CagdSrfStruct *SymbCrvCrvBisectOnSphere3(const CagdCrvStruct *CCrv1,
					 const CagdCrvStruct *CCrv2)
{
    CagdBType
	IsRational1 = CAGD_IS_RATIONAL_PT(CCrv1 -> PType),
	IsRational2 = CAGD_IS_RATIONAL_PT(CCrv2 -> PType),
	IsRational = IsRational1 && IsRational2;
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv1, *Crv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2, *B1, *B2, *B3,
	*A11, *A12, *A13, *A21, *A22, *A23, *A31, *A32, *A33,
	*TSrf1, *TSrf2, *TSrf3, *TSrf4, *RetVal;

    if (IsRational1 != IsRational2) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	return NULL;
    }

    Crv1 = CagdCoerceCrvTo(CCrv1, IsRational ? CAGD_PT_P3_TYPE
					     : CAGD_PT_E3_TYPE, FALSE);
    Crv2 = CagdCoerceCrvTo(CCrv2, IsRational ? CAGD_PT_P3_TYPE
					     : CAGD_PT_E3_TYPE, FALSE);

    Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR);
    Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    BspKnotAffineTrans2(Srf1 -> VKnotVector, Srf1 -> VLength + Srf1 -> VOrder,
			VMin2, VMax2);
    BspKnotAffineTrans2(Srf2 -> UKnotVector, Srf2 -> ULength + Srf2 -> UOrder,
			UMin1, UMax1);

    TSrf1 = SymbSrfSub(Srf1, Srf2);
    DSrf1 = CagdSrfDerive(Srf1, CAGD_CONST_U_DIR);
    DSrf2 = CagdSrfDerive(Srf2, CAGD_CONST_V_DIR);
    if (IsRational) {
	SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
	CagdSrfFree(TSrf2);

	SymbSrfSplitScalar(DSrf1, &TSrf2, &A21, &A22, &A23);
	CagdSrfFree(TSrf2);
	CagdSrfFree(DSrf1);
	DSrf1 = SymbSrfMergeScalar(NULL, A21, A22, A23);

	SymbSrfSplitScalar(DSrf2, &TSrf2, &A31, &A32, &A33);
	CagdSrfFree(TSrf2);
	CagdSrfFree(DSrf2);
	DSrf2 = SymbSrfMergeScalar(NULL, A31, A32, A33);
    }
    else {
	SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
	SymbSrfSplitScalar(DSrf1, &TSrf2, &A21, &A22, &A23);
	SymbSrfSplitScalar(DSrf2, &TSrf2, &A31, &A32, &A33);
    }

    B1 = BspSrfNew(1, 1, 1, 1, CAGD_PT_E1_TYPE);           /* Identical zero. */
    B1 -> Points[1][0] = 0;
    B1 -> UKnotVector[0] = B1 -> VKnotVector[0] = 0.0;
    B1 -> UKnotVector[1] = B1 -> VKnotVector[1] = 1.0;
    BspKnotAffineTrans2(B1 -> VKnotVector, 2, VMin2, VMax2);
    BspKnotAffineTrans2(B1 -> UKnotVector, 2, UMin1, UMax1);

    B2 = SymbSrfDotProd(Srf1, DSrf1);
    B3 = SymbSrfDotProd(Srf2, DSrf2);

    CagdSrfFree(TSrf1);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf2);

    /* Solve the equations. */
    TSrf1 = SymbSrfDeterminant3(A11, A12, A13,
				A21, A22, A23,
				A31, A32, A33);

    TSrf2 = SymbSrfDeterminant3(B1, A12, A13,
				B2, A22, A23,
				B3, A32, A33);

    TSrf3 = SymbSrfDeterminant3(A11, B1, A13,
				A21, B2, A23,
				A31, B3, A33);

    TSrf4 = SymbSrfDeterminant3(A11, A12, B1,
				A21, A22, B2,
				A31, A32, B3);

    CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf4, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf2, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf2, &TSrf4, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf3, &TSrf4, TRUE, TRUE, TRUE, TRUE);

    RetVal = SymbSrfMergeScalar(TSrf1, TSrf2, TSrf3, TSrf4);

    CagdSrfFree(B1);
    CagdSrfFree(B2);
    CagdSrfFree(B3);

    CagdSrfFree(A11);
    CagdSrfFree(A12);
    CagdSrfFree(A13);
    CagdSrfFree(A21);
    CagdSrfFree(A22);
    CagdSrfFree(A23);
    CagdSrfFree(A31);
    CagdSrfFree(A32);
    CagdSrfFree(A33);

    /* Make sure weights are all positive, if all weights same sign. */
    CagdAllWeightsNegative(RetVal -> Points, RetVal -> PType,
			   RetVal -> ULength * RetVal -> VLength, TRUE);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between the XY plane and a point.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:      Direction of line from origin.                                  M
*   Size:    Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbCylinPointBisect, SymbConePointBisect, SymbSpherePointBisect,        M
*   SymbTorusPointBisect,						     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbConeLineBisect, SymbSphereLineBisect				     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPlanePointBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbPlanePointBisect(const CagdPType Pt, CagdRType Size)
{
    CagdSrfStruct *XYPlane, *BisectSrf;

    Size *= 2 * IRIT_MAX(IRIT_FABS(Pt[0]), IRIT_FABS(Pt[1])) + 1.0;
    XYPlane = CagdPrimPlaneSrf(-Size, -Size, Size, Size, 0.0);

    BisectSrf = SymbSrfPtBisectorSrf3D(XYPlane, Pt);

    CagdSrfFree(XYPlane);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a cylinder and a point.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CylPt:   Point on axis of cylinder.                                      M
*   CylDir:  Direction of cylinder.					     M
*   CylRad:  Radius of cylinder.					     M
*   Pt:      Direction of line from origin.				     M
*   Size:    Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbPlanePointBisect, SymbConePointBisect, SymbSpherePointBisect,        M
*   SymbTorusPointBisect,						     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbConeLineBisect, SymbSphereLineBisect				     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCylinPointBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbCylinPointBisect(const CagdPType CylPt,
				    const CagdVType CylDir,
				    CagdRType CylRad,
				    const CagdPType Pt,
				    CagdRType Size)
{
    IrtHmgnMatType Mat;
    CagdVType Dir;
    CagdPType Pos;
    CagdSrfStruct *Cylin, *BisectSrf;

    /* Create a canonic cylinder, and bring to right position/orientation. */
    Pos[0] = Pos[1] = 0.0;
    Pos[2] = -Size;
    Cylin = CagdPrimCylinderSrf(Pos, CylRad, Size, TRUE, CAGD_PRIM_CAPS_NONE);

    IRIT_PT_COPY(Dir, CylDir);
    IRIT_PT_NORMALIZE(Dir);
    GMGenMatrixZ2Dir(Mat, Dir);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Cylin, Mat);
    CagdSrfTransform(Cylin, CylPt, 1.0);

    BisectSrf = SymbSrfPtBisectorSrf3D(Cylin, Pt);

    CagdSrfFree(Cylin);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a cone and a point.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeApex:   Apex point of cone.	                                     M
*   ConeDir:    Direction of cone axes.                                      M
*   ConeAngle:  Spanning angle of the cone, in degrees.                      M
*   Pt:         Direction of line from origin.			             M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbSpherePointBisect,       M
*   SymbTorusPointBisect,						     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbConeLineBisect, SymbSphereLineBisect				     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConePointBisect, bisectors, skeleton                                 M
*****************************************************************************/
CagdSrfStruct *SymbConePointBisect(const CagdPType ConeApex,
				   const CagdVType ConeDir,
				   CagdRType ConeAngle,
				   const CagdPType Pt,
				   CagdRType Size)
{
    IrtHmgnMatType Mat;
    CagdRType Radius;
    CagdVType Dir;
    CagdSrfStruct *Cone, *BisectSrf;

    /* Create a canonic cone, and bring to right position/orientation. */
    Radius = tan(IRIT_DEG2RAD(ConeAngle)) * Size;
    Cone = CagdPrimConeSrf(GlblOrigin, Radius, Size, TRUE,
			   CAGD_PRIM_CAPS_NONE);

    /* Brings cone's apex to origin. */
    IRIT_PT_RESET(Dir);
    Dir[2] = -Size;
    CagdSrfTransform(Cone, Dir, 1.0);

    /* Orient and position. */
    IRIT_PT_COPY(Dir, ConeDir);
    IRIT_PT_NORMALIZE(Dir);
    IRIT_PT_SCALE(Dir, -1.0);
    GMGenMatrixZ2Dir(Mat, Dir);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Cone, Mat);
    CagdSrfTransform(Cone, ConeApex, 1.0);

    BisectSrf = SymbSrfPtBisectorSrf3D(Cone, Pt);

    CagdSrfFree(Cone);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a sphere and a point.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr: Center location of the sphere.                                  M
*   SprRad:  Radius of sphere.						     M
*   Pt:      Direction of line from origin.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbTorusPointBisect,						     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbConeLineBisect, SymbSphereLineBisect				     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect2, SymbConeCylinBisect,         M
*   SymbCylinCylinBisect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSpherePointBisect, bisectors, skeleton                               M
*****************************************************************************/
CagdSrfStruct *SymbSpherePointBisect(const CagdPType SprCntr,
				     CagdRType SprRad,
				     const CagdPType Pt)
{
    IRIT_STATIC_DATA CagdPType
	Origin = { 0.0, 0.0, 0.0 };
    CagdVType Dir;
    CagdSrfStruct *SphereAux, *Sphere, *BisectSrf;
    IrtHmgnMatType Mat1, Mat2;

    /* Create a canonic sphere, rotate so north pole points at Pt, and      */
    /* bring the sphere to right, SprCntr, position.			    */
    SphereAux = CagdPrimSphereSrf(Origin, SprRad, TRUE);
    IRIT_VEC_SUB(Dir, Pt, SprCntr);
    GMGenMatrixZ2Dir(Mat1, Dir);
    MatGenMatTrans(SprCntr[0], SprCntr[1], SprCntr[2], Mat2);
    MatMultTwo4by4(Mat1, Mat1, Mat2);
    Sphere = CagdSrfMatTransform(SphereAux, Mat1);
    CagdSrfFree(SphereAux);

    BisectSrf = SymbSrfPtBisectorSrf3D(Sphere, Pt);

    CagdSrfFree(Sphere);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a torus and a point.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrsCntr:     Center of constructed torus.                                M
*   TrsDir:      Axis of symmetry of constructed torus.                      M
*   TrsMajorRad: Major radius of constructed torus.                          M
*   TrsMinorRad: Minor radius of constructed torus.                          M
*   Pt:          Direction of line from origin.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect,						     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbConeLineBisect, SymbSphereLineBisect				     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTorusPointBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbTorusPointBisect(const CagdVType TrsCntr,
				    const CagdVType TrsDir,
				    CagdRType TrsMajorRad,
				    CagdRType TrsMinorRad,
				    const CagdPType Pt)
{
    IrtHmgnMatType Mat;
    CagdVType Dir;
    CagdSrfStruct *Torus, *BisectSrf;

    /* Create a canonic torus, and bring to right position/orientation. */
    Torus = CagdPrimTorusSrf(GlblOrigin, TrsMajorRad, TrsMinorRad, TRUE);

    IRIT_PT_COPY(Dir, TrsDir);
    IRIT_PT_NORMALIZE(Dir);
    GMGenMatrixZ2Dir(Mat, Dir);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Torus, Mat);
    CagdSrfTransform(Torus, TrsCntr, 1.0);

    BisectSrf = SymbSrfPtBisectorSrf3D(Torus, Pt);

    CagdSrfFree(Torus);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between the XY plane and a line emanating   M
* from the origin in direction V.  					     M
*                                                                            *
* PARAMETERS:                                                                M
*   LineDir: Direction of line from origin.  Must be in northern hemisphere. M
*   Size:    Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect,				     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPlaneLineBisect, bisectors, skeleton                                 M
*****************************************************************************/
CagdSrfStruct *SymbPlaneLineBisect(const CagdVType LineDir, CagdRType Size)
{
    CagdVType V;
    CagdCrvStruct *BisectCrvs,
	*Equator = BspCrvCreateUnitCircle();
    CagdSrfStruct
	*BisectSrfs = NULL;

    IRIT_PT_COPY(V, LineDir);
    IRIT_VEC_NORMALIZE(V);

    BisectCrvs = SymbPtCrvBisectOnSphere(V, Equator);
    CagdCrvFree(Equator);

    while (BisectCrvs) {
	CagdCrvStruct *ZeroCrv,
	    *NextCrv = BisectCrvs -> Pnext;
	CagdSrfStruct *Srf;

	ZeroCrv = CagdCrvCopy(BisectCrvs);
	CagdCrvTransform(ZeroCrv, NULL, 0.0);
	Srf = CagdRuledSrf(BisectCrvs, ZeroCrv, 2, 2);
	CagdSrfTransform(Srf, NULL, Size);

	IRIT_LIST_PUSH(Srf, BisectSrfs);

	CagdCrvFree(BisectCrvs);
	CagdCrvFree(ZeroCrv);
	BisectCrvs = NextCrv;
    }

    return BisectSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a cone and a line through its apex. M
*   Assumes the cone's apex is at the origin.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeDir:    Direction of cone axes.  Must be in the northern hemisphere. M
*   ConeAngle:  Spanning angle of the cone, in degrees.                      M
*   LineDir:    Direction of line from the origin.  Must be in the northern  M
*		hemisphere.			                             M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPtCrvBisectOnSphere,                                                 M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbSphereLineBisect, SymbPlaneLineBisect,				     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConeLineBisect, bisectors, skeleton                                  M
*****************************************************************************/
CagdSrfStruct *SymbConeLineBisect(const CagdVType ConeDir,
				  CagdRType ConeAngle,
				  const CagdVType LineDir,
				  CagdRType Size)
{
    CagdRType Radius;
    CagdVType V;
    CagdCrvStruct *BisectCrvs, *Circ, *TCrv;
    CagdSrfStruct
	*BisectSrfs = NULL;
    CagdPtStruct Center;
    IrtHmgnMatType Mat;

    Radius = sin(IRIT_DEG2RAD(ConeAngle));
    Center.Pt[0] = 0;
    Center.Pt[1] = 0;
    Center.Pt[2] = cos(IRIT_DEG2RAD(ConeAngle));
    Circ = BspCrvCreateCircle(&Center, Radius);
    IRIT_PT_COPY(V, ConeDir);
    IRIT_VEC_NORMALIZE(V);
    GMGenMatrixZ2Dir(Mat, V);
    TCrv = CagdCrvMatTransform(Circ, Mat);
    CagdCrvFree(Circ);
    Circ = TCrv;

    IRIT_PT_COPY(V, LineDir);
    IRIT_VEC_NORMALIZE(V);

    BisectCrvs = SymbPtCrvBisectOnSphere(V, Circ);
    CagdCrvFree(Circ);

    while (BisectCrvs) {
	CagdCrvStruct *ZeroCrv,
	    *NextCrv = BisectCrvs -> Pnext;
	CagdSrfStruct *Srf;

	ZeroCrv = CagdCrvCopy(BisectCrvs);
	CagdCrvTransform(ZeroCrv, NULL, 0.0);
	Srf = CagdRuledSrf(BisectCrvs, ZeroCrv, 2, 2);
	CagdSrfTransform(Srf, NULL, Size);

	IRIT_LIST_PUSH(Srf, BisectSrfs);

	CagdCrvFree(BisectCrvs);
	CagdCrvFree(ZeroCrv);
	BisectCrvs = NextCrv;
    }

    return BisectSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a sphere and a line.		     M
*   The computation is reduced to that of a bisector between a point and a   M
* cylinder, that has a rational form.  The line is assumed to be the Z axis. M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr:    Center location of the sphere.                               M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbPlaneLineBisect,				     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect, SymbSpherePlaneBisect,        M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSphereLineBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbSphereLineBisect(const CagdPType SprCntr,
				    CagdRType SprRad,
				    CagdRType Size)
{
    CagdVType Center;
    CagdSrfStruct *Cylin, *BisectSrf;
    CagdRType
	Scale = Size * (IRIT_FABS(SprCntr[2]) * 2.0 + 2.0);

    Center[0] = Center[1] = 0.0;
    Center[2] = -Scale;

    Cylin = CagdPrimCylinderSrf(Center, SprRad, Scale * 2.0, 1,
				CAGD_PRIM_CAPS_NONE);
    BisectSrf = SymbSrfPtBisectorSrf3D(Cylin, SprCntr);

    CagdSrfFree(Cylin);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a sphere and the XY plane.	     M
*   The computation is reduced to that of a bisector between a point and a   M
* plane, that has a rational form.  Only the portion for which Z > 0 should  M
* be considered in the output.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr:    Center location of the sphere.  Must be in northern	     M
*		hemisphere (positive Z coefficient).			     M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbConePlaneBisect, SymbCylinPlaneBisect,				     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSpherePlaneBisect, bisectors, skeleton                               M
*****************************************************************************/
CagdSrfStruct *SymbSpherePlaneBisect(const CagdPType SprCntr,
				     CagdRType SprRad,
				     CagdRType Size)
{
    CagdSrfStruct *Plane, *BisectSrf;
    CagdRType
	Scale = Size * (IRIT_FABS(SprCntr[2]) * 2.0 + 2.0);

    Plane = CagdPrimPlaneSrf(-Scale, -Scale, Scale, Scale, -SprRad);

    BisectSrf = SymbSrfPtBisectorSrf3D(Plane, SprCntr);

    CagdSrfFree(Plane);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a cylinder and the XY plane.	     M
*   The computation is reduced to that of a bisector between a line and a    M
* plane, that has a rational form.  Only the portion for which Z > 0 should  M
* be considered in the output.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   CylPt:      Point on axis of cylinder.                                   M
*   CylDir:     Direction of cylinder. Must be in the northern hemisphere    M
*		(positive Z coefficient).				     M
*   CylRad:     Radius of cylinder.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbConePlaneBisect, SymbSpherePlaneBisect, 			     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCylinPlaneBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbCylinPlaneBisect(const CagdPType CylPt,
				    const CagdVType CylDir,
				    CagdRType CylRad,
				    CagdRType Size)
{
    CagdRType t;
    CagdPType XYPt;
    CagdSrfStruct *BisectSrf;

    if (!GMPointFromLinePlane(CylPt, CylDir, GlblXYPlane, XYPt, &t)) {
	SYMB_FATAL_ERROR(SYMB_ERR_COPLANAR_GEOMETRY);
	return NULL;
    }

    BisectSrf = SymbPlaneLineBisect(CylDir, Size);

    /* Compensate for the fact that the apex is at depth CylRad as both the  */
    /* plane and the cylinder were offset by CylRad. Hence, we need to       */
    /* translate the bisector along the cylinder axes to create a Z change   */
    /* CylRad.								     */
    XYPt[0] -= CylDir[0] * CylRad / CylDir[2];
    XYPt[1] -= CylDir[1] * CylRad / CylDir[2];
    XYPt[2] = -CylRad;
    CagdSrfTransform(BisectSrf, XYPt, 1.0);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a cone and the XY plane.	     M
*   The computation is reduced to that of a bisector between a line and a    M
* cone, that has a rational form.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeApex:   Apex point of cone.	                                     M
*   ConeDir:    Direction of cylinder. Must be in the northern hemisphere.   M
*   ConeAngle:  Angular span of cone, in degrees.			     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbSpherePlaneBisect,			     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbTorusSphereBisect, SymbConeConeBisect, SymbConeConeBisect2,	     M
*   SymbConeCylinBisect, SymbCylinCylinBisect				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConePlaneBisect, bisectors, skeleton                                 M
*****************************************************************************/
CagdSrfStruct *SymbConePlaneBisect(const CagdPType ConeApex,
				   const CagdVType ConeDir,
				   CagdRType ConeAngle,
				   CagdRType Size)
{
    IRIT_STATIC_DATA CagdVType
      ZAxis = { 0.0, 0.0, 1.0 };
    CagdRType Off, t;
    CagdPType XYPt, Apex;
    CagdVType Dir;
    CagdSrfStruct *BisectSrf;

    if (!GMPointFromLinePlane(ConeApex, ConeDir, GlblXYPlane, XYPt, &t)) {
	SYMB_FATAL_ERROR(SYMB_ERR_COPLANAR_GEOMETRY);
	return NULL;
    }

    IRIT_PT_COPY(Dir, ConeDir);
    IRIT_PT_NORMALIZE(Dir);
    BisectSrf = SymbConeLineBisect(ZAxis, 90 + ConeAngle, Dir, Size);

    /* Compensate for the fact that the apex is at depth ConeApex[2] and was */
    /* assumed to be at depth Zero.  The cone was offset until its apex met  */
    /* the offseted XYPlane.  For offset Off, we move along the cone axis    */
    /* amounts equal to (Off / sin(ConeAngle)) which in turns changes the Z  */
    /* height of the apex by (Off / sin(ConeAngle)) * ConeDir[2] (assuming   */
    /* ConeDir is a normalized vector or ConeDir[2] is the projection of     */
    /* ConeDir on the Z axis.  Then we have:				     */
    /*        (Off / sin(ConeAngle)) * ConeDir[2] + Off = ConeApex[2]	     */
    /* or     Off = ConeApex[2] / (1 + ConeDir[2] / sin(ConeAngle))          */
    IRIT_PT_COPY(Apex, ConeApex);
    Off = Apex[2] / (1.0 + Dir[2] / sin(IRIT_DEG2RAD(ConeAngle)));
    t = Off / sin(IRIT_DEG2RAD(ConeAngle));
    Apex[0] -= t * Dir[0];
    Apex[1] -= t * Dir[1];
    Apex[2] -= t * Dir[2];
    CagdSrfTransform(BisectSrf, Apex, 1.0);

    return BisectSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a cylinder and a sphere.	     M
*   The computation is reduced to that of a bisector between a point and a   M
* cylinder, that has a rational form.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   CylPt:      Point on axis of cylinder.                                   M
*   CylDir:     Direction of cylinder. Must be in the northern hemisphere    M
*		(positive Z coefficient).				     M
*   CylRad:     Radius of cylinder.					     M
*   SprCntr:    Center location of the sphere.  Must be in northern	     M
*		hemisphere (positive Z coefficient).			     M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbSphereSphereBisect, SymbConeSphereBisect, SymbTorusSphereBisect,     M
*   SymbConeConeBisect, SymbConeConeBisect2, SymbConeCylinBisect,            M
*   SymbCylinCylinBisect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCylinSphereBisect, bisectors, skeleton                               M
*****************************************************************************/
CagdSrfStruct *SymbCylinSphereBisect(const CagdPType CylPt,
				     const CagdVType CylDir,
				     CagdRType CylRad,
				     const CagdPType SprCntr,
				     CagdRType SprRad,
				     CagdRType Size)
{
    return SymbCylinPointBisect(CylPt, CylDir, IRIT_FABS(CylRad + SprRad),
				SprCntr, Size);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between wto spheres.			     M
*   The computation is reduced to that of a bisector between a point and a   M
* sphere, that has a rational form.   	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   SprCntr1:   Center location of the first sphere.			     M
*   SprRad1:    Radius of first sphere.					     M
*   SprCntr2:   Center location of the second sphere.			     M
*   SprRad2:    Radius of second sphere.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbConeSphereBisect, SymbTorusSphereBisect,      M
*   SymbConeConeBisect, SymbConeConeBisect2, SymbConeCylinBisect,            M
*   SymbCylinCylinBisect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSphereSphereBisect, bisectors, skeleton                              M
*****************************************************************************/
CagdSrfStruct *SymbSphereSphereBisect(const CagdVType SprCntr1,
				      CagdRType SprRad1,
				      const CagdVType SprCntr2,
				      CagdRType SprRad2)
{
    if (SprRad1 > SprRad2)
	return SymbSpherePointBisect(SprCntr1, SprRad1 - SprRad2, SprCntr2);
    else
	return SymbSpherePointBisect(SprCntr1, SprRad2 - SprRad1, SprCntr2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a sphere and a cone.		     M
*   The computation is reduced to that of a bisector between a point and a   M
* cone, that has a rational form.      	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   ConeApex:   Apex point of cone.	                                     M
*   ConeDir:    Direction of cone axes.                                      M
*   ConeAngle:  Spanning angle of the cone, in degrees.                      M
*   SprCntr:    Center location of the sphere.				     M
*   SprRad:     Radius of sphere.					     M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbTorusSphereBisect,    M
*   SymbConeConeBisect, SymbConeConeBisect2, SymbConeCylinBisect,            M
*   SymbCylinCylinBisect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConeSphereBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbConeSphereBisect(const CagdPType ConeApex,
				    const CagdVType ConeDir,
				    CagdRType ConeAngle,
				    const CagdPType SprCntr,
				    CagdRType SprRad,
				    CagdRType Size)
{
    int i;
    CagdRType t;
    CagdVType Dir;
    CagdPType Apex;

    /* Offset of a cone by SprRad would simply move the apex of the cone by */
    /* ConeApex += ConeDir * (-SprRad / sin(ConeAngle)).	            */
    t = -SprRad / sin(IRIT_DEG2RAD(ConeAngle));
    IRIT_PT_COPY(Dir, ConeDir);
    IRIT_PT_NORMALIZE(Dir);
    for (i = 0; i < 3; i++)
	Apex[i] = ConeApex[i] + Dir[i] * t;

    return SymbConePointBisect(Apex, Dir, ConeAngle, SprCntr, Size);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between two cones sharing an apex. The      M
* apex is assumed to be at the origin.					     M
*   The computation is reduced to that of a bisector between a line and a    M
* cone, that has a rational form.      	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cone1Dir:   Direction of first cone axes.                                M
*   Cone1Angle: Spanning angle of the first cone, in degrees.                M
*   Cone2Dir:   Direction of second cone axes.                               M
*   Cone2Angle: Spanning angle of the second cone, in degrees.               M
*   Size:       Portion of result as it is infinite.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbTorusSphereBisect,    M
*   SymbConeSphereBisect, SymbConeConeBisect2, SymbConeCylinBisect,          M
*   SymbCylinCylinBisect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConeConeBisect, bisectors, skeleton                                  M
*****************************************************************************/
CagdSrfStruct *SymbConeConeBisect(const CagdVType Cone1Dir,
				  CagdRType Cone1Angle,
				  const CagdVType Cone2Dir,
				  CagdRType Cone2Angle,
				  CagdRType Size)
{
    if (Cone1Angle > Cone2Angle)
	return SymbConeLineBisect(Cone1Dir, Cone1Angle - Cone2Angle,
				  Cone2Dir, Size);
    else
	return SymbConeLineBisect(Cone2Dir, Cone2Angle - Cone1Angle,
				  Cone1Dir, Size);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between a torus and a sphere.		     M
*   The computation is reduced to that of a bisector between a point and a   M
* torus, that has a rational form.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   TrsCntr:     Center of constructed torus.                                M
*   TrsDir:      Axis of symmetry of constructed torus.                      M
*   TrsMajorRad: Major radius of constructed torus.                          M
*   TrsMinorRad: Minor radius of constructed torus.                          M
*   SprCntr:     Center location of the sphere.  Must be in northern	     M
*		 hemisphere (positive Z coefficient).			     M
*   SprRad:      Radius of sphere.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbConeConeBisect, SymbConeConeBisect2, SymbConeCylinBisect,            M
*   SymbCylinCylinBisect						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTorusSphereBisect, bisectors, skeleton                               M
*****************************************************************************/
CagdSrfStruct *SymbTorusSphereBisect(const CagdVType TrsCntr,
				     const CagdVType TrsDir,
				     CagdRType TrsMajorRad,
				     CagdRType TrsMinorRad,
				     const CagdVType SprCntr,
				     CagdRType SprRad)
{
    return SymbTorusPointBisect(TrsCntr, TrsDir, TrsMajorRad,
				TrsMinorRad + SprRad, SprCntr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a rational surface Srf, remove its W factors and return them in    *
*  the scalar surface WSrf.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:          Surface to remove its rational factors, in place.          *
*   RetWeights:   TRUE to return the weights, FALSE to purge.                *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:   The WSrf (weights) if requested, NULL otherwise.      *
*****************************************************************************/
static CagdSrfStruct *SymbSrfIsolateRational(CagdSrfStruct **Srf,
					     int RetWeights)
{
    CagdSrfStruct *XSrf, *YSrf, *ZSrf, *WSrf;

    SymbSrfSplitScalar(*Srf, &WSrf, &XSrf, &YSrf, &ZSrf);

    CagdSrfFree(*Srf);

    *Srf = SymbSrfMergeScalar(NULL, XSrf, YSrf, ZSrf);

    CagdSrfFree(XSrf);
    CagdSrfFree(YSrf);
    CagdSrfFree(ZSrf);

    if (RetWeights)
	return WSrf;
    else {
	CagdSrfFree(WSrf);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between two cylinders. 		     M
*   Let C1(u), T1(u), and N1(u) and C2(v), T2(v), and N2(v) be the cylinders M
* cross section, cross section unit tangent field and cross section unit     M
* normal field.   Ci(u) can be derived as a transformed circle.  Ti(u) and   M
* Ni(u) are unit circles rotated to the proper orientation CyliDir.          M
*   Finally, note that Ci(u), Ti(u), and Ni(u) are all rational.             M
*   Then, the bisector is computed as the solution of the following three    M
* linear equations:							     M
*									     M
*   < B - C1(u), T1(u) > = 0						     V
*									     V
*   < B - C2(v), T2(v) > = 0						     V
*									     V
*   < B, N1(u) - N2(v) > = < C1(u), N1(u) > - < C2(v), N2(v) >		     V
*									     M
* The first two constraints the bisector to be on the normal plane of the    M
* generators of the two cylinders that are fixed along the generator (the    M
* straight lines of the cylinder).  The last constraint make sure the        M
* bisector is on the plane that bisects the two tangent planes of the two    M
* cylinders.								     M
*   This computation is following the bisectors of two developables,         M
* presented in,								     M
* "Geometric Properties of Bisector Surfaces", by Martin Peternell,	     M
* Graphical Models, Volume 62, No. 3, May 2000.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cyl1Pos:     A point on the axis of the first cylinder.                  M
*   Cyl1Dir:     The direction of the first cylinder.	                     M
*   Cyl1Rad:     Radius of first cylinder.		                     M
*   Cyl2Pos:     A point on the axis of the second cylinder.                 M
*   Cyl2Dir:     The direction of the second cylinder.	                     M
*   Cyl2Rad:     Radius of second cylinder.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbConeConeBisect, SymbConeConeBisect2, SymbConeCylinBisect,	     M
*   									     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCylinCylinBisect, bisectors, skeleton                                M
*****************************************************************************/
CagdSrfStruct *SymbCylinCylinBisect(const CagdVType Cyl1Pos,
				    const CagdVType Cyl1Dir,
				    CagdRType Cyl1Rad,
				    const CagdVType Cyl2Pos,
				    const CagdVType Cyl2Dir,
				    CagdRType Cyl2Rad)
{
    IrtHmgnMatType Mat, Mat2;
    CagdCrvStruct *NrmlCrv, *TanCrv,
	*Crv = BspCrvCreateUnitCircle();
    CagdSrfStruct *Srf1, *NrmlSrf1, *TanSrf1, *Srf2, *NrmlSrf2, *TanSrf2,
	*W1, *W2, *TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5,
	*A11, *A12, *A13, *A21, *A22, *A23, *A31, *A32, *A33, *b1, *b2, *b3;

    BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
			     Crv -> Order + Crv -> Length, 0.0, 1.0);
    NrmlCrv = CagdCrvCopy(Crv);
    MatGenMatRotZ1(M_PI_DIV_2, Mat);
    TanCrv = CagdCrvMatTransform(Crv, Mat);

    /* We now have a circle Crv, its normal field as NrmlCrv, and its       */
    /* tangent field as TanCrv.						    */

    /* Prepare these three vector fields for Cylinder 1: */
    Srf1 = CagdPromoteCrvToSrf(Crv, CAGD_CONST_U_DIR);
    NrmlSrf1 = CagdPromoteCrvToSrf(NrmlCrv, CAGD_CONST_U_DIR);
    TanSrf1 = CagdPromoteCrvToSrf(TanCrv, CAGD_CONST_U_DIR);

    GMGenMatrixZ2Dir(Mat, Cyl1Dir);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(TanSrf1, Mat);
    SymbSrfIsolateRational(&TanSrf1, FALSE);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(NrmlSrf1, Mat);
    SymbSrfIsolateRational(&NrmlSrf1, FALSE);

    MatGenMatUnifScale(-Cyl1Rad, Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);
    MatGenMatTrans(Cyl1Pos[0], Cyl1Pos[1], Cyl1Pos[2], Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf1, Mat);
    W1 = SymbSrfIsolateRational(&Srf1, TRUE);

    /* Prepare these three vector fields for Cylinder 2: */
    Srf2 = CagdPromoteCrvToSrf(Crv, CAGD_CONST_V_DIR);
    NrmlSrf2 = CagdPromoteCrvToSrf(NrmlCrv, CAGD_CONST_V_DIR);
    TanSrf2 = CagdPromoteCrvToSrf(TanCrv, CAGD_CONST_V_DIR);

    GMGenMatrixZ2Dir(Mat, Cyl2Dir);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(TanSrf2, Mat);
    SymbSrfIsolateRational(&TanSrf2, FALSE);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(NrmlSrf2, Mat);
    SymbSrfIsolateRational(&NrmlSrf2, FALSE);

    MatGenMatUnifScale(-Cyl2Rad, Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);
    MatGenMatTrans(Cyl2Pos[0], Cyl2Pos[1], Cyl2Pos[2], Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf2, Mat);
    W2 = SymbSrfIsolateRational(&Srf2, TRUE);

    CagdCrvFree(Crv);
    CagdCrvFree(TanCrv);
    CagdCrvFree(NrmlCrv);

    /* Construct the linear system to solve for the bisector. */
    b1 = SymbSrfDotProd(Srf1, TanSrf1);
    TSrf1 = SymbSrfMultScalar(TanSrf1, W1);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
    CagdSrfFree(TSrf1);

    b2 = SymbSrfDotProd(Srf2, TanSrf2);
    TSrf1 = SymbSrfMultScalar(TanSrf2, W2);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &A21, &A22, &A23);
    CagdSrfFree(TSrf1);

    TSrf1 = SymbSrfMult(W2, W2);
    TSrf2 = SymbSrfMultScalar(NrmlSrf1, TSrf1);
    CagdSrfFree(TSrf1);
    TSrf3 = SymbSrfDotProd(Srf1, TSrf2);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(W1, W1);
    TSrf2 = SymbSrfMultScalar(NrmlSrf2, TSrf1);
    CagdSrfFree(TSrf1);
    TSrf4 = SymbSrfDotProd(Srf2, TSrf2);
    CagdSrfFree(TSrf2);
    b3 = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);
    TSrf1 = SymbSrfMultScalar(NrmlSrf1, W2);
    TSrf2 = SymbSrfMultScalar(NrmlSrf2, W1);
    TSrf3 = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(W1, W2);
    TSrf4 = SymbSrfMultScalar(TSrf3, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf3);
    SymbSrfSplitScalar(TSrf4, &TSrf1, &A31, &A32, &A33);
    CagdSrfFree(TSrf4);

    CagdSrfFree(W1);
    CagdSrfFree(W2);
    CagdSrfFree(Srf1);
    CagdSrfFree(TanSrf1);
    CagdSrfFree(NrmlSrf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(TanSrf2);
    CagdSrfFree(NrmlSrf2);

    /* And solve the linear system. */
    TSrf1 = SymbSrfDeterminant3(A11, A12, A13,
				A21, A22, A23,
				A31, A32, A33);
    TSrf2 = SymbSrfDeterminant3(b1, A12, A13,
				b2, A22, A23,
				b3, A32, A33);
    TSrf3 = SymbSrfDeterminant3(A11, b1, A13,
				A21, b2, A23,
				A31, b3, A33);
    TSrf4 = SymbSrfDeterminant3(A11, A12, b1,
				A21, A22, b2,
				A31, A32, b3);
    CagdSrfFree(b1);
    CagdSrfFree(b2);
    CagdSrfFree(b3);
    CagdSrfFree(A11);
    CagdSrfFree(A12);
    CagdSrfFree(A13);
    CagdSrfFree(A21);
    CagdSrfFree(A22);
    CagdSrfFree(A23);
    CagdSrfFree(A31);
    CagdSrfFree(A32);
    CagdSrfFree(A33);

    CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf4, TRUE, TRUE, TRUE, TRUE);

    TSrf5 = SymbSrfMergeScalar(TSrf1, TSrf2, TSrf3, TSrf4);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    return TSrf5;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between two cones in general position.      M
*   Let C1(u), T1(u), and N1(u) and C2(v), T2(v), and N2(v) be cones'        M
* cross sections, cross sections' unit tangent field and cross sections'     M
* unit normal field.   Ci(u) can be derived as a transformed circle.  Ti(u)  M
* are unit circles rotated to the proper orientation ConeiDir and Ni(u) is   M
* a circle on the unit sphere with the proper orientation.	             M
*   Finally, note that Ci(u), Ti(u), and Ni(u) are all rational.   	     M
*   Then, the bisector is computed as the solution of the following three    M
* linear equations:							     M
*									     M
*   < B - C1(u), T1(u) > = 0						     V
*									     V
*   < B - C2(v), T2(v) > = 0						     V
*									     V
*   < B, N1(u) - N2(v) > = < C1(u), N1(u) > - < C2(v), N2(v) >		     V
*									     M
* The first two constraints the bisector to be on the normal plane of the    M
* generators of the two cones that are fixed along the generator (the        M
* straight lines of the cone).  The last constraint make sure the            M
* bisector is on the plane that bisects the two tangent planes of the two    M
* cones.								     M
*   This computation is following the bisectors of two developables,         M
* presented in,								     M
* "Geometric Properties of Bisector Surfaces", by Martin Peternell,	     M
* Graphical Models, Volume 62, No. 3, May 2000.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cone1Pos:     The apex point of the first cone.	                     M
*   Cone1Dir:     The direction of the first cone.	                     M
*   Cone1Angle:   Spanning angle of the first cone, in degrees.              M
*   Cone2Pos:     The apex point of the the second cone.	             M
*   Cone2Dir:     The direction of the second cone.	                     M
*   Cone2Angle:   Spanning angle of the second cone, in degrees.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbConeConeBisect, SymbCylinCylinBisect, SymbConeCylinBisect,	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConeConeBisect2, bisectors, skeleton                                 M
*****************************************************************************/
CagdSrfStruct *SymbConeConeBisect2(const CagdVType Cone1Pos,
				   const CagdVType Cone1Dir,
				   CagdRType Cone1Angle,
				   const CagdVType Cone2Pos,
				   const CagdVType Cone2Dir,
				   CagdRType Cone2Angle)
{
    IrtHmgnMatType Mat, Mat2, Mat3;
    CagdRType
	Tan1Angle = tan(-IRIT_DEG2RAD(Cone1Angle)),
	Tan2Angle = tan(-IRIT_DEG2RAD(Cone2Angle));
    CagdCrvStruct *NrmlCrv, *TanCrv,
	*Crv = BspCrvCreateUnitCircle();
    CagdSrfStruct *Srf1, *NrmlSrf1, *TanSrf1, *Srf2, *NrmlSrf2, *TanSrf2,
	*W1, *W2, *TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5,
	*A11, *A12, *A13, *A21, *A22, *A23, *A31, *A32, *A33, *b1, *b2, *b3;

    BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
			     Crv -> Order + Crv -> Length, 0.0, 1.0);
    NrmlCrv = CagdCrvCopy(Crv),
    MatGenMatRotZ1(M_PI_DIV_2, Mat);
    TanCrv = CagdCrvMatTransform(Crv, Mat);

    /* We now have a circle Crv, its normal field as NrmlCrv, and its       */
    /* tangent field as TanCrv.						    */

    /* Prepare these three vector fields for Cone 1: */
    Srf1 = CagdPromoteCrvToSrf(Crv, CAGD_CONST_U_DIR);
    NrmlSrf1 = CagdPromoteCrvToSrf(NrmlCrv, CAGD_CONST_U_DIR);
    TanSrf1 = CagdPromoteCrvToSrf(TanCrv, CAGD_CONST_U_DIR);

    GMGenMatrixZ2Dir(Mat, Cone1Dir);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(TanSrf1, Mat);
    SymbSrfIsolateRational(&TanSrf1, FALSE);

    MatGenMatUnifScale(1 / sqrt(1 + IRIT_SQR(Tan1Angle)), Mat2);
    MatMultTwo4by4(Mat2, Mat2, Mat);
    MatGenMatTrans(0.0, 0.0, -Tan1Angle, Mat3);
    MatMultTwo4by4(Mat3, Mat3, Mat2);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(NrmlSrf1, Mat3);
    SymbSrfIsolateRational(&NrmlSrf1, FALSE);

    MatGenMatTrans(0.0, 0.0, 1.0, Mat2);
    MatMultTwo4by4(Mat, Mat2, Mat);
    MatGenMatUnifScale(Tan1Angle, Mat2);
    MatMultTwo4by4(Mat, Mat2, Mat);
    MatGenMatTrans(Cone1Pos[0], Cone1Pos[1], Cone1Pos[2], Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf1, Mat);
    W1 = SymbSrfIsolateRational(&Srf1, TRUE);

    /* Prepare these three vector fields for Cone 2: */
    Srf2 = CagdPromoteCrvToSrf(Crv, CAGD_CONST_V_DIR);
    NrmlSrf2 = CagdPromoteCrvToSrf(NrmlCrv, CAGD_CONST_V_DIR);
    TanSrf2 = CagdPromoteCrvToSrf(TanCrv, CAGD_CONST_V_DIR);

    GMGenMatrixZ2Dir(Mat, Cone2Dir);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(TanSrf2, Mat);
    SymbSrfIsolateRational(&TanSrf2, FALSE);

    MatGenMatUnifScale(1 / sqrt(1 + IRIT_SQR(Tan2Angle)), Mat2);
    MatMultTwo4by4(Mat2, Mat2, Mat);
    MatGenMatTrans(0.0, 0.0, -Tan2Angle, Mat3);
    MatMultTwo4by4(Mat3, Mat3, Mat2);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(NrmlSrf2, Mat3);
    SymbSrfIsolateRational(&NrmlSrf2, FALSE);

    MatGenMatTrans(0.0, 0.0, 1.0, Mat2);
    MatMultTwo4by4(Mat, Mat2, Mat);
    MatGenMatUnifScale(Tan2Angle, Mat2);
    MatMultTwo4by4(Mat, Mat2, Mat);
    MatGenMatTrans(Cone2Pos[0], Cone2Pos[1], Cone2Pos[2], Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf2, Mat);
    W2 = SymbSrfIsolateRational(&Srf2, TRUE);

    CagdCrvFree(Crv);
    CagdCrvFree(TanCrv);
    CagdCrvFree(NrmlCrv);

    /* Construct the linear system to solve for the bisector. */
    b1 = SymbSrfDotProd(Srf1, TanSrf1);
    TSrf1 = SymbSrfMultScalar(TanSrf1, W1);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
    CagdSrfFree(TSrf1);

    b2 = SymbSrfDotProd(Srf2, TanSrf2);
    TSrf1 = SymbSrfMultScalar(TanSrf2, W2);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &A21, &A22, &A23);
    CagdSrfFree(TSrf1);

    TSrf1 = SymbSrfMult(W2, W2);
    TSrf2 = SymbSrfMultScalar(NrmlSrf1, TSrf1);
    CagdSrfFree(TSrf1);
    TSrf3 = SymbSrfDotProd(Srf1, TSrf2);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(W1, W1);
    TSrf2 = SymbSrfMultScalar(NrmlSrf2, TSrf1);
    CagdSrfFree(TSrf1);
    TSrf4 = SymbSrfDotProd(Srf2, TSrf2);
    CagdSrfFree(TSrf2);
    b3 = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);
    TSrf1 = SymbSrfMultScalar(NrmlSrf1, W2);
    TSrf2 = SymbSrfMultScalar(NrmlSrf2, W1);
    TSrf3 = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(W1, W2);
    TSrf4 = SymbSrfMultScalar(TSrf3, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf3);
    SymbSrfSplitScalar(TSrf4, &TSrf1, &A31, &A32, &A33);
    CagdSrfFree(TSrf4);

    CagdSrfFree(W1);
    CagdSrfFree(W2);
    CagdSrfFree(Srf1);
    CagdSrfFree(TanSrf1);
    CagdSrfFree(NrmlSrf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(TanSrf2);
    CagdSrfFree(NrmlSrf2);

    /* And solve the linear system. */
    TSrf1 = SymbSrfDeterminant3(A11, A12, A13,
				A21, A22, A23,
				A31, A32, A33);
    TSrf2 = SymbSrfDeterminant3(b1, A12, A13,
				b2, A22, A23,
				b3, A32, A33);
    TSrf3 = SymbSrfDeterminant3(A11, b1, A13,
				A21, b2, A23,
				A31, b3, A33);
    TSrf4 = SymbSrfDeterminant3(A11, A12, b1,
				A21, A22, b2,
				A31, A32, b3);
    CagdSrfFree(b1);
    CagdSrfFree(b2);
    CagdSrfFree(b3);
    CagdSrfFree(A11);
    CagdSrfFree(A12);
    CagdSrfFree(A13);
    CagdSrfFree(A21);
    CagdSrfFree(A22);
    CagdSrfFree(A23);
    CagdSrfFree(A31);
    CagdSrfFree(A32);
    CagdSrfFree(A33);

    CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf4, TRUE, TRUE, TRUE, TRUE);

    TSrf5 = SymbSrfMergeScalar(TSrf1, TSrf2, TSrf3, TSrf4);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    return TSrf5;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the bisector surface between two cones in general position.      M
*   Let C1(u), T1(u), and N1(u) and C2(v), T2(v), and N2(v) be cones'        M
* cross sections, cross sections' unit tangent field and cross sections'     M
* unit normal field.   Ci(u) can be derived as a transformed circle.  Ti(u)  M
* are unit circles rotated to the proper orientation ConeiDir and Ni(u) is   M
* a circle on the unit sphere with the proper orientation.		     M
*   Finally, note that Ci(u), Ti(u), and Ni(u) are all rational.             M
*   Then, the bisector is computed as the solution of the following three    M
* linear equations:							     M
*									     M
*   < B - C1(u), T1(u) > = 0						     V
*									     V
*   < B - C2(v), T2(v) > = 0						     V
*									     V
*   < B, N1(u) - N2(v) > = < C1(u), N1(u) > - < C2(v), N2(v) >		     V
*									     M
* The first two constraints the bisector to be on the normal plane of the    M
* generators of the two cylinders that are fixed along the generator (the    M
* straight lines of the cylinder).  The last constraint make sure the        M
* bisector is on the plane that bisects the two tangent planes of the two    M
* cylinders.								     M
*   This computation is following the bisectors of two developables,         M
* presented in,								     M
* "Geometric Properties of Bisector Surfaces", by Martin Peternell,	     M
* Graphical Models, Volume 62, No. 3, May 2000.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cone1Pos:     The apex point of the first cone.	                     M
*   Cone1Dir:     The direction of the first cone.	                     M
*   Cone1Angle:   Spanning angle of the first cone, in degrees.              M
*   Cyl2Pos:     A point on the axis of the second cylinder.                 M
*   Cyl2Dir:     The direction of the second cylinder.	                     M
*   Cyl2Rad:     Radius of second cylinder.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed bisector surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPlanePointBisect, SymbCylinPointBisect, SymbConePointBisect,         M
*   SymbSpherePointBisect, SymbTorusPointBisect,			     M
*   SymbConeLineBisect, SymbSphereLineBisect, SymbPlaneLineBisect,	     M
*   SymbCylinPlaneBisect, SymbConePlaneBisect, SymbSpherePlaneBisect,	     M
*   SymbCylinSphereBisect, SymbSphereSphereBisect, SymbConeSphereBisect,     M
*   SymbConeConeBisect, SymbCylinCylinBisect, SymbConeConeBisect2,	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbConeCylinBisect, bisectors, skeleton                                 M
*****************************************************************************/
CagdSrfStruct *SymbConeCylinBisect(const CagdVType Cone1Pos,
				   const CagdVType Cone1Dir,
				   CagdRType Cone1Angle,
				   const CagdVType Cyl2Pos,
				   const CagdVType Cyl2Dir,
				   CagdRType Cyl2Rad)
{
    IrtHmgnMatType Mat, Mat2, Mat3;
    CagdRType
	Tan1Angle = tan(IRIT_DEG2RAD(Cone1Angle));
    CagdCrvStruct *NrmlCrv, *TanCrv,
	*Crv = BspCrvCreateUnitCircle();
    CagdSrfStruct *Srf1, *NrmlSrf1, *TanSrf1, *Srf2, *NrmlSrf2, *TanSrf2,
	*W1, *W2, *TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5,
	*A11, *A12, *A13, *A21, *A22, *A23, *A31, *A32, *A33, *b1, *b2, *b3;

    BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
			     Crv -> Order + Crv -> Length, 0.0, 1.0);
    NrmlCrv = CagdCrvCopy(Crv),
    MatGenMatRotZ1(M_PI_DIV_2, Mat);
    TanCrv = CagdCrvMatTransform(Crv, Mat);

    /* We now have a circle Crv, its normal field as NrmlCrv, and its       */
    /* tangent field as TanCrv.						    */

    /* Prepare these three vector fields for Cone 1: */
    Srf1 = CagdPromoteCrvToSrf(Crv, CAGD_CONST_U_DIR);
    NrmlSrf1 = CagdPromoteCrvToSrf(NrmlCrv, CAGD_CONST_U_DIR);
    TanSrf1 = CagdPromoteCrvToSrf(TanCrv, CAGD_CONST_U_DIR);

    GMGenMatrixZ2Dir(Mat, Cone1Dir);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(TanSrf1, Mat);
    SymbSrfIsolateRational(&TanSrf1, FALSE);

    MatGenMatUnifScale(1 / sqrt(1 + IRIT_SQR(Tan1Angle)), Mat2);
    MatMultTwo4by4(Mat2, Mat2, Mat);
    MatGenMatTrans(0.0, 0.0, -Tan1Angle, Mat3);
    MatMultTwo4by4(Mat3, Mat3, Mat2);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(NrmlSrf1, Mat3);
    SymbSrfIsolateRational(&NrmlSrf1, FALSE);

    MatGenMatTrans(0.0, 0.0, 1.0, Mat2);
    MatMultTwo4by4(Mat, Mat2, Mat);
    MatGenMatUnifScale(Tan1Angle, Mat2);
    MatMultTwo4by4(Mat, Mat2, Mat);
    MatGenMatTrans(Cone1Pos[0], Cone1Pos[1], Cone1Pos[2], Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf1, Mat);
    W1 = SymbSrfIsolateRational(&Srf1, TRUE);

    /* Prepare these three vector fields for Cylinder 2: */
    Srf2 = CagdPromoteCrvToSrf(Crv, CAGD_CONST_V_DIR);
    NrmlSrf2 = CagdPromoteCrvToSrf(NrmlCrv, CAGD_CONST_V_DIR);
    TanSrf2 = CagdPromoteCrvToSrf(TanCrv, CAGD_CONST_V_DIR);

    GMGenMatrixZ2Dir(Mat, Cyl2Dir);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(TanSrf2, Mat);
    SymbSrfIsolateRational(&TanSrf2, FALSE);
    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(NrmlSrf2, Mat);
    SymbSrfIsolateRational(&NrmlSrf2, FALSE);

    MatGenMatUnifScale(-Cyl2Rad, Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);
    MatGenMatTrans(Cyl2Pos[0], Cyl2Pos[1], Cyl2Pos[2], Mat2);
    MatMultTwo4by4(Mat, Mat, Mat2);

    SMP_SKEL_SRF_MAT_TRANS_IN_PLACE(Srf2, Mat);
    W2 = SymbSrfIsolateRational(&Srf2, TRUE);

    CagdCrvFree(Crv);
    CagdCrvFree(TanCrv);
    CagdCrvFree(NrmlCrv);

    /* Construct the linear system to solve for the bisector. */
    b1 = SymbSrfDotProd(Srf1, TanSrf1);
    TSrf1 = SymbSrfMultScalar(TanSrf1, W1);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &A11, &A12, &A13);
    CagdSrfFree(TSrf1);

    b2 = SymbSrfDotProd(Srf2, TanSrf2);
    TSrf1 = SymbSrfMultScalar(TanSrf2, W2);
    SymbSrfSplitScalar(TSrf1, &TSrf2, &A21, &A22, &A23);
    CagdSrfFree(TSrf1);

    TSrf1 = SymbSrfMult(W2, W2);
    TSrf2 = SymbSrfMultScalar(NrmlSrf1, TSrf1);
    CagdSrfFree(TSrf1);
    TSrf3 = SymbSrfDotProd(Srf1, TSrf2);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(W1, W1);
    TSrf2 = SymbSrfMultScalar(NrmlSrf2, TSrf1);
    CagdSrfFree(TSrf1);
    TSrf4 = SymbSrfDotProd(Srf2, TSrf2);
    CagdSrfFree(TSrf2);
    b3 = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);
    TSrf1 = SymbSrfMultScalar(NrmlSrf1, W2);
    TSrf2 = SymbSrfMultScalar(NrmlSrf2, W1);
    TSrf3 = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(W1, W2);
    TSrf4 = SymbSrfMultScalar(TSrf3, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf3);
    SymbSrfSplitScalar(TSrf4, &TSrf1, &A31, &A32, &A33);
    CagdSrfFree(TSrf4);

    CagdSrfFree(W1);
    CagdSrfFree(W2);
    CagdSrfFree(Srf1);
    CagdSrfFree(TanSrf1);
    CagdSrfFree(NrmlSrf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(TanSrf2);
    CagdSrfFree(NrmlSrf2);

    /* And solve the linear system. */
    TSrf1 = SymbSrfDeterminant3(A11, A12, A13,
				A21, A22, A23,
				A31, A32, A33);
    TSrf2 = SymbSrfDeterminant3(b1, A12, A13,
				b2, A22, A23,
				b3, A32, A33);
    TSrf3 = SymbSrfDeterminant3(A11, b1, A13,
				A21, b2, A23,
				A31, b3, A33);
    TSrf4 = SymbSrfDeterminant3(A11, A12, b1,
				A21, A22, b2,
				A31, A32, b3);
    CagdSrfFree(b1);
    CagdSrfFree(b2);
    CagdSrfFree(b3);
    CagdSrfFree(A11);
    CagdSrfFree(A12);
    CagdSrfFree(A13);
    CagdSrfFree(A21);
    CagdSrfFree(A22);
    CagdSrfFree(A23);
    CagdSrfFree(A31);
    CagdSrfFree(A32);
    CagdSrfFree(A33);

    CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf3, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf1, &TSrf4, TRUE, TRUE, TRUE, TRUE);

    TSrf5 = SymbSrfMergeScalar(TSrf1, TSrf2, TSrf3, TSrf4);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);

    return TSrf5;
}
