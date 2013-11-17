/******************************************************************************
* RRInter.c - ruled-ruled surface/ring-ring surface intersection.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, February 98    				      *
******************************************************************************/

#include "symb_loc.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"

#define	DELTA_EPS	   IRIT_UEPS
#define	DDELTA_EPS	   IRIT_UEPS
#define CONTOUR_EPS	   1e-8     /* Level above zero to actually contour. */
#define TRIM_EPS	   1e-3
#define	SELF_EPS	   1e-6			 /* Ignore |u-v| < SELF_EPS. */

/*
#define	PRINT_PARALLEL_CASE
#define	PRINT_DELTAS
#define	PRINT_PERF
*/

IRIT_STATIC_DATA int
    GlblSelfTest = 0;

static int SolveForST(CagdRType u,
		      CagdRType v,
		      CagdCrvStruct *C1,
		      CagdCrvStruct *C2,
		      CagdCrvStruct *D1,
		      CagdCrvStruct *D2,
		      CagdRType *s,
		      CagdRType *t);
static CagdSrfStruct *SymbRuledRuledAux(CagdCrvStruct **C1,
					CagdCrvStruct **C2,
					CagdCrvStruct **D1,
					CagdCrvStruct **D2,
					CagdSrfStruct **C1Srf,
					CagdSrfStruct **C2Srf,
					CagdSrfStruct **D1Srf,
					CagdSrfStruct **D2Srf,
					CagdSrfStruct **TSrf1,
					CagdSrfStruct **TSrf2,
					CagdSrfStruct **TSrf3);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curve of two ruled surfaces:                   M
*                                                                            M
*        S1(u, t) = t C1(u) + (1-t) C2(u)				     V
*        S2(v, s) = s D1(v) + (1-s) D2(s)				     V
*                                                                            M
* Then S1(u, t) = S2(v, s) yields,                                           M
*                                                                            M
* C1(u) - D1(v) = s (C1(u) - C2(u)) + t (D2(v) - D1(v))                      V
*									     N
* or solve for the zero set of the determinant of,			     M
*                                                                            M
*                | C1(u) - D1(u) |					     V
*  Gamma(u, v) = | C1(u) - C2(u) | = 0					     V
*                | D1(v) - D2(v) |					     V
*                                                                            *
* PARAMETERS:                                                                M
*   C1, C2:  The two curves forming the first ruled surface.                 M
*   D1, D2:  The two curves forming the second ruled surface.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Gamma(u, v).					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRuledRuledZeroSetFunc                                                M
*****************************************************************************/
CagdSrfStruct *SymbRuledRuledZeroSetFunc(CagdCrvStruct *C1,
					 CagdCrvStruct *C2,
					 CagdCrvStruct *D1,
					 CagdCrvStruct *D2)
{
    CagdSrfStruct *C1Srf, *C2Srf, *D1Srf, *D2Srf, *TSrf1, *TSrf2, *TSrf3,
        *ZeroSetFunc = SymbRuledRuledAux(&C1, &C2, &D1, &D2,
					 &C1Srf, &C2Srf, &D1Srf, &D2Srf,
					 &TSrf1, &TSrf2, &TSrf3);

    CagdCrvFree(C1);
    CagdCrvFree(C2);
    CagdCrvFree(D1);
    CagdCrvFree(D2);

    CagdSrfFree(C1Srf);
    CagdSrfFree(C2Srf);
    CagdSrfFree(D1Srf);
    CagdSrfFree(D2Srf);

    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);

    return ZeroSetFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curve of two ruled surfaces:                   M
*                                                                            M
*        S1(u, t) = t C1(u) + (1-t) C2(u)				     V
*        S2(v, s) = s D1(v) + (1-s) D2(s)				     V
*                                                                            M
* Then S1(u, t) = S2(v, s) yields,                                           M
*                                                                            M
* C1(u) - D1(v) = s (C1(u) - C2(u)) + t (D2(v) - D1(v))                      V
*									     N
* or solve for the zero set of the determinant of,			     M
*                                                                            M
*                  | C1(u) - D1(u) |					     V
*   Gamma(u, v) =  | C1(u) - C2(u) | = 0				     V
*                  | D1(v) - D2(v) |					     V
*                                                                            *
* PARAMETERS:                                                                M
*   C1, C2:      The two curves forming the first ruled surface.             M
*   D1, D2:      The two curves forming the second ruled surface.            M
*   Tolerance:   Accuracy of zero set computation as part of the solution.   M
*                Value of 10 is a good start.  If Tolerance is negative      M
*		 the ruled surfaces are assumed infinite (and absolute value M
*		 of Tolerance is employed).  Otherwise, the ruled surface is M
*		 bound between C1 and C2 and between D1 and D2 respectively. M
*   PCrvs1, PCrvs2:  The parametric domains of the intersection curves, in   M
*		 the two surfaces.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Intersection curves in Euclidean space.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRuledRuledIntersection                                               M
*****************************************************************************/
CagdCrvStruct *SymbRuledRuledIntersection(CagdCrvStruct *C1,
					  CagdCrvStruct *C2,
					  CagdCrvStruct *D1,
					  CagdCrvStruct *D2,
					  CagdRType Tolerance,
					  CagdCrvStruct **PCrvs1,
					  CagdCrvStruct **PCrvs2)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };   /* A scalar srf - only X. */
    CagdBType
	InfiniteRuledSrfs = Tolerance < 0.0;
    CagdSrfStruct *C1Srf, *C2Srf, *D1Srf, *D2Srf, *TSrf,
	*TSrf1, *TSrf2, *TSrf3;
    IPPolygonStruct *Cntrs, *Cntr;
    CagdCrvStruct
	*E3Crvs;
    CagdRType CMin, CMax, DMin, DMax;

#   ifdef PRINT_PERF
	CagdRType StartTime, CrntTime;
	StartTime = IritCPUTime(FALSE);
#   endif /* PRINT_PERF */

    CagdCrvDomain(C1, &CMin, &CMax); /* Assume C1 and C2 have the same Dom. */
    CagdCrvDomain(D1, &DMin, &DMax); /* Assume D1 and D2 have the same Dom. */

    Tolerance = IRIT_FABS(Tolerance);

    *PCrvs1 = NULL;
    *PCrvs2 = NULL;
    E3Crvs = NULL;

    /* C1(u) - D1(v) = s (C1(u) - C2(u)) + t (D2(v) - D1(v))		    */
    /* T1            = s        T2       + t        T3			    */
    /* TSrf = Gamma(u,v)						    */
    TSrf = SymbRuledRuledAux(&C1, &C2, &D1, &D2,
			     &C1Srf, &C2Srf, &D1Srf, &D2Srf,
			     &TSrf1, &TSrf2, &TSrf3);

    CagdSrfFree(C1Srf);
    CagdSrfFree(C2Srf);
    CagdSrfFree(D1Srf);
    CagdSrfFree(D2Srf);

#   ifndef NO_RRI_TRIVIAL_REJECT

    if (!InfiniteRuledSrfs) { /* Trivial Reject */
	CagdSrfStruct *ASrf, *BSrf, *DSrf, *ESrf, *FSrf;
	CagdSrfStruct *Denom, *Numer;
	CagdBBoxStruct BBox;

	/* A = <T2, T2>							*/
        /* B = <T2, T3>							*/
        /* D = <T3, T3>							*/
        /* E = <T1, T2>							*/
        /* F = <T1, T3>							*/
        /* 								*/
        /* | A  B | | s | = | E |					*/
        /* | B  D | | t | = | F |					*/
	ASrf = SymbSrfDotProd(TSrf2, TSrf2);
	BSrf = SymbSrfDotProd(TSrf2, TSrf3);
	DSrf = SymbSrfDotProd(TSrf3, TSrf3);
	ESrf = SymbSrfDotProd(TSrf2, TSrf1);
	FSrf = SymbSrfDotProd(TSrf3, TSrf1);

	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
	CagdSrfFree(TSrf3);

	/* Make sure the s and t parameter might be between zero and one. */

	/* | s | =   1   | D  -B || E |					*/
	/* |   |   ----- |       ||   |					*/
	/* | t | = AD-BB |-B   A || F |					*/
	TSrf1 = SymbSrfMult(ASrf, DSrf);
	TSrf2 = SymbSrfMult(BSrf, BSrf);
	CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
	Denom = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);

	TSrf1 = SymbSrfMult(DSrf, ESrf);
	TSrf2 = SymbSrfMult(BSrf, FSrf);
	CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
	Numer = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);

	CagdMakeSrfsCompatible(&Denom, &Numer, TRUE, TRUE, TRUE, TRUE);
	TSrf1 = SymbSrfMergeScalar(Denom, Numer, NULL, NULL);
	CagdSrfFree(Numer);
	CagdSrfBBox(TSrf1, &BBox);
	CagdSrfFree(TSrf1);

	/* If s cannot be between zero and one - quit. */
	if (BBox.Min[0] > 1.0 || BBox.Max[0] < 0.0) {
	    CagdSrfFree(Denom);
	    CagdSrfFree(ASrf);
	    CagdSrfFree(BSrf);
	    CagdSrfFree(DSrf);
	    CagdSrfFree(ESrf);
	    CagdSrfFree(FSrf);

	    CagdCrvFree(C1);
	    CagdCrvFree(C2);
	    CagdCrvFree(D1);
	    CagdCrvFree(D2);

	    return NULL;
	}

	TSrf1 = SymbSrfMult(ASrf, FSrf);
	TSrf2 = SymbSrfMult(BSrf, ESrf);
	CagdMakeSrfsCompatible(&TSrf1, &TSrf2, TRUE, TRUE, TRUE, TRUE);
	Numer = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);

	CagdMakeSrfsCompatible(&Denom, &Numer, TRUE, TRUE, TRUE, TRUE);
	TSrf1 = SymbSrfMergeScalar(Denom, Numer, NULL, NULL);
	CagdSrfFree(Numer);
	CagdSrfBBox(TSrf1, &BBox);
	CagdSrfFree(TSrf1);
	/* If s cannot be between zero and one - quit. */
	if (BBox.Min[0] > 1.0 || BBox.Max[0] < 0.0) {
	    CagdSrfFree(Denom);
	    CagdSrfFree(ASrf);
	    CagdSrfFree(BSrf);
	    CagdSrfFree(DSrf);
	    CagdSrfFree(ESrf);
	    CagdSrfFree(FSrf);

	    CagdCrvFree(C1);
	    CagdCrvFree(C2);
	    CagdCrvFree(D1);
	    CagdCrvFree(D2);

	    return NULL;
	}

	CagdSrfFree(Denom);
	CagdSrfFree(ASrf);
	CagdSrfFree(BSrf);
	CagdSrfFree(DSrf);
	CagdSrfFree(ESrf);
	CagdSrfFree(FSrf);
    } /* Trivial Reject */
    else {
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
	CagdSrfFree(TSrf3);
    }
#   else
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
#   endif /* NO_RRI_TRIVIAL_REJECT */


    /* Computes the zero set of the equation as contours Gamma(u,v) = 0. */
    Cntrs = UserCntrSrfWithPlane(TSrf, Plane, Tolerance);
    CagdSrfFree(TSrf);

    /* Gets the st ruling parameters and save in the normal slots. */
    for (Cntr = Cntrs; Cntr != NULL;  Cntr = Cntr -> Pnext) {

	IPVertexStruct *V, *Pre, Vs;
	CagdRType s, t;

	Vs.Pnext = Cntr -> PVertex;
	Pre = &Vs;

	for (V = Vs.Pnext; V != NULL; Pre = V, V = V -> Pnext) {

	    /* Make sure we are on the boundary. */
	    if (V -> Coord[1] < 0.0)
		V -> Coord[1] = 0.0;
	    else if (V -> Coord[1] > 1.0)
		V -> Coord[1] = 1.0;
	    if (V -> Coord[2] < 0.0)
		V -> Coord[2] = 0.0;
	    else if (V -> Coord[2] > 1.0)
		V -> Coord[2] = 1.0;
	
	    switch (SolveForST(V -> Coord[1], V -> Coord[2],
			       C1, C2, D1, D2, &s, &t)) {
	        case -1:				     /* Common line. */
#		    ifdef PRINT_PARALLEL_CASE
		        IRIT_INFO_MSG_PRINTF("Common line:(u,v) = (%.12f, %.12f)\n",
				V -> Coord[1], V -> Coord[2]);
#		    endif /* PRINT_PARALLEL_CASE */

		    if (V -> Pnext != NULL)
			Cntr -> Pnext = IPAllocPolygon(0, V -> Pnext,
						       Cntr -> Pnext);
		    Pre -> Pnext = NULL;
		    IPFreeVertex(V);
		    V = Pre;
		    break;

		case -2:		    /* Parallel/Opposit: Not common. */
#		    ifdef PRINT_PARALLEL_CASE
		        IRIT_INFO_MSG_PRINTF(
				"Redundant line:(u,v) = (%.12f, %.12f)\n",
				V -> Coord[1], V -> Coord[2]);
#		    endif /* PRINT_PARALLEL_CASE */

		    if (V -> Pnext != NULL)
			Cntr -> Pnext = IPAllocPolygon(0, V -> Pnext,
						       Cntr -> Pnext);
		    Pre -> Pnext = NULL;
		    IPFreeVertex(V);
		    V = Pre;
		    break;

	        case 0:
	        default:
		    V -> Normal[0] = 0.0;
		    V -> Normal[1] = s;
		    V -> Normal[2] = t;
		    break;
	    }

	    Cntr -> PVertex = Vs.Pnext;

	}
    }

    if (!InfiniteRuledSrfs) {
	/* Gets the st ruling parameters and clip between zero and one. */
	for (Cntr = Cntrs; Cntr != NULL;  Cntr = Cntr -> Pnext) {
	    IPVertexStruct *V, *VPrev;
	    CagdRType SPrev, TPrev;

	    if (!Cntr -> PVertex)
		continue;

	    VPrev = Cntr -> PVertex;
	    SPrev = VPrev -> Normal[1];
	    TPrev = VPrev -> Normal[2];

	    for (V = VPrev -> Pnext; V != NULL; ) {
		CagdRType
	            s = V -> Normal[1],
		    t = V -> Normal[2];
		CagdBType
		    IsSTPrevInDomain = SPrev >= 0.0 && SPrev <= 1.0 &&
				       TPrev >= 0.0 && TPrev <= 1.0,
		    IsSTInDomain = s >= 0.0 && s <= 1.0 &&
				   t >= 0.0 && t <= 1.0,
		    EnterDomain = !IsSTPrevInDomain && IsSTInDomain,
		    ExitDomain = IsSTPrevInDomain && !IsSTInDomain;

		/* Check for edges that crosses the boundary and clip them. */

		if (EnterDomain || ExitDomain) {
 		    CagdRType r, r2;

		    /* Only one of the two parameters is out. */

		    if (EnterDomain)
			r = 0.0, r2 = 0.0;
		    else
			r = 1.0, r2 = 1.0;

		    if (s < 0.0 || SPrev < 0.0)
			r = -SPrev / (s - SPrev);
		    else if (s > 1.0 || SPrev > 1.0)
			r = (1.0 - SPrev) / (s - SPrev);

		    if (t < 0.0 || TPrev < 0.0)
			r2 = -TPrev / (t - TPrev);
		    else if (t > 1.0 || TPrev > 1.0)
			r2 = (1.0 - TPrev) / (t - TPrev);

		    if (EnterDomain) {
		        IPVertexStruct *VTmp, *VTmp2;

		    	if (r2 > r)
			   r = r2;

			IRIT_PT_BLEND(VPrev -> Coord,
				 V -> Coord, VPrev -> Coord, r);
			IRIT_VEC_BLEND(VPrev -> Normal,
				  V -> Normal, VPrev -> Normal, r);

			/* Make sure we are on the boundary. */
			if (s < 0.0)
			    VPrev -> Normal[1] = 0.0;
			else if (s > 1.0)
			    VPrev -> Normal[1] = 1.0;
			if (t < 0.0)
			    VPrev -> Normal[2] = 0.0;
			else if (t > 1.0)
			    VPrev -> Normal[2] = 1.0;

			/* Purge all vertices prior to this one. */
			for (VTmp = Cntr -> PVertex; VTmp != VPrev; ) {
			     VTmp2 = VTmp -> Pnext;
			     IPFreeVertex(VTmp);
			     VTmp = VTmp2;
			 }
			Cntr -> PVertex = VPrev;
			VPrev = V;
			V = V -> Pnext;
		    }
		    else { /* ExitDomain */
			if (r2 < r)
			    r = r2;

			IRIT_PT_BLEND(V -> Coord,
				 V -> Coord, VPrev -> Coord, r);
			IRIT_VEC_BLEND(V -> Normal,
				 V -> Normal, VPrev -> Normal, r);

			/* Make sure we are on the boundary. */
			if (s < 0.0)
			    V -> Normal[1] = 0.0;
			else if (s > 1.0)
			    V -> Normal[1] = 1.0;
			if (t < 0.0)
			    V -> Normal[2] = 0.0;
			else if (t > 1.0)
			    V -> Normal[2] = 1.0;

			/* Make a new contour starting from V -> Pnext. */
			if (V -> Pnext != NULL) {
			    Cntr -> Pnext = IPAllocPolygon(0, V -> Pnext,
							   Cntr -> Pnext);
			    V -> Pnext = NULL;
			    V = NULL;
			}
		    }
		}
		else {
		    VPrev = V;
		    V = V -> Pnext;
		}
		SPrev = s;
		TPrev = t;
	    }		    
	}
    }

    /* Gets the st ruling parameters and forms the intersection curves. */
    for (Cntr = Cntrs; Cntr != NULL;  Cntr = Cntr -> Pnext) {
	int i,
	    Len = IPVrtxListLen(Cntr -> PVertex);
	CagdCrvStruct *Crv;
	CagdRType **PPoints1, **PPoints2, **E3Points;
	IPVertexStruct *V;

	if (Len < 2)
	    continue;

	if (!InfiniteRuledSrfs) {

	    /* Make sure all coordinates are properly clipped. */
	    for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
		if (V -> Normal[1] < -TRIM_EPS ||
		    V -> Normal[1] > 1.0 + TRIM_EPS ||
		    V -> Normal[2] < -TRIM_EPS ||
		    V -> Normal[2] > 1.0 + TRIM_EPS) {
		    break;
		}
	    }
	    if (V != NULL)
	        continue;
	}

	Crv = BspCrvNew(Len, 2, CAGD_PT_E2_TYPE);
	IRIT_LIST_PUSH(Crv, *PCrvs1);
	Crv = BspCrvNew(Len, 2, CAGD_PT_E2_TYPE);
	IRIT_LIST_PUSH(Crv, *PCrvs2);
	Crv = BspCrvNew(Len, 2, CAGD_PT_E3_TYPE);
	IRIT_LIST_PUSH(Crv, E3Crvs);

	PPoints1 = (*PCrvs1) -> Points;
	PPoints2 = (*PCrvs2) -> Points;
	E3Points = E3Crvs -> Points;

	BspKnotUniformOpen(Len, 2, (*PCrvs1) -> KnotVector);
	BspKnotUniformOpen(Len, 2, (*PCrvs2) -> KnotVector);
	BspKnotUniformOpen(Len, 2, E3Crvs -> KnotVector);

	for (V = Cntr -> PVertex, i = 0; V != NULL; V = V -> Pnext, i++) {
	    CagdRType *R;
	    CagdPType Pt1, Pt2, Srf1Pt, Srf2Pt;

	    V -> Coord[1] = IRIT_BOUND(V -> Coord[1], 0.0, 1.0);
	    V -> Coord[2] = IRIT_BOUND(V -> Coord[2], 0.0, 1.0);


	    PPoints1[1][i] = CMin + (CMax - CMin) * V -> Coord[1];
	    PPoints1[2][i] = V -> Normal[1];			   /* [0, 1] */

	    PPoints2[1][i] = DMin + (DMax - DMin) * V -> Coord[2];
	    PPoints2[2][i] = V -> Normal[2];			   /* [0, 1] */

	    R = CagdCrvEval(C1, V -> Coord[1]);
	    CagdCoerceToE3(Pt1, &R, -1, C1 -> PType);
	    R = CagdCrvEval(C2, V -> Coord[1]);
	    CagdCoerceToE3(Pt2, &R, -1, C2 -> PType);
	    IRIT_PT_BLEND(Srf1Pt, Pt2, Pt1, V -> Normal[1]);

	    R = CagdCrvEval(D1, V -> Coord[2]);
	    CagdCoerceToE3(Pt1, &R, -1, D1 -> PType);
	    R = CagdCrvEval(D2, V -> Coord[2]);
	    CagdCoerceToE3(Pt2, &R, -1, D2 -> PType);
	    IRIT_PT_BLEND(Srf2Pt, Pt2, Pt1, V -> Normal[2]);

	    /* Should be the same - take their average. */
	    IRIT_PT_BLEND(Pt1, Srf1Pt, Srf2Pt, 0.5);

	    E3Points[1][i] = Pt1[0];
	    E3Points[2][i] = Pt1[1];
	    E3Points[3][i] = Pt1[2];
	}
    }

    CagdCrvFree(C1);
    CagdCrvFree(C2);
    CagdCrvFree(D1);
    CagdCrvFree(D2);

    IPFreePolygonList(Cntrs);

#   ifdef PRINT_PERF
	CrntTime = IritCPUTime(FALSE);
	IRIT_INFO_MSG_PRINTF(
		"> Time spent for RuledRuledSrfIntersection : %lf sec\n",
		CrntTime - StartTime);
#   endif /* PRINT_PERF */

    return E3Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the self intersection curve of a ruled surfaces:                M
*                                                                            M
* PARAMETERS:                                                                M
*   C1, C2:  The two curves forming the ruled surface.                       M
*   Tolerance:   Accuracy of zero set computation as part of the solution.   M
*                Value of 10 is a good start.  If Tolerance is negative      M
*		 the ruled surfaces are assumed infinite (and absolute value M
*		 of Tolerance is employed).  Otherwise, the ruled surface is M
*		 bound between C1 and C2 and between D1 and D2 respectively. M
*   PCrvs1, PCrvs2:  The parametric domains of the intersection curves, in   M
*		 the two surfaces.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Intersection curves in Euclidean space.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRuledSelfIntersection                                                M
*****************************************************************************/
CagdCrvStruct *SymbRuledSelfIntersection(CagdCrvStruct *C1,
					 CagdCrvStruct *C2,
					 CagdRType Tolerance,
					 CagdCrvStruct **PCrvs1,
					 CagdCrvStruct **PCrvs2)
{
    CagdCrvStruct *Ret;

    GlblSelfTest = 1;
    Ret = SymbRuledRuledIntersection(C1, C2, C1, C2,
				     Tolerance, PCrvs1, PCrvs2);
    GlblSelfTest = 0;

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Solves for the parameters along the ruling direction.                    *
* Solves,								     *
*                                                                            *
* C1(u) - D1(v) = s (C1(u) - C2(u)) + t (D2(v) - D1(v))                      *
* T1            = s        T2       + t        T3                            *
*                                                                            *
* A = T2 * T2                                                                *
* B = T2 * T3                                                                *
* D = T3 * T3                                                                *
* E = T1 * T2                                                                *
* F = T1 * T3                                                                *
*                                                                            *
* | A  B | | s | = | E |                                                     *
* | B  D | | t | = | F |                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   u:          Parameter along the curves in the first ruled surface.       *
*   v:          Parameter along the curves in the second ruled surface.      *
*   C1, C2:  The two curves forming the first ruled surface.                 *
*   D1, D2:  The two curves forming the second ruled surface.                *
*   s, t:       To solve for.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int SolveForST(CagdRType u,
		      CagdRType v,
		      CagdCrvStruct *C1,
		      CagdCrvStruct *C2,
		      CagdCrvStruct *D1,
		      CagdCrvStruct *D2,
		      CagdRType *s,
		      CagdRType *t)
{
    CagdPType C1Pt, C2Pt, D1Pt, D2Pt, T1, T2, T3, NT2, NT3, NT2xNT3;
    CagdRType *R, Delta;

    if (GlblSelfTest && u < v + SELF_EPS)
	return -1;

    R = CagdCrvEval(C1, u);
    CagdCoerceToE3(C1Pt, &R, -1, C1 -> PType);
    R = CagdCrvEval(C2, u);
    CagdCoerceToE3(C2Pt, &R, -1, C2 -> PType);

    R = CagdCrvEval(D1, v);
    CagdCoerceToE3(D1Pt, &R, -1, D1 -> PType);
    R = CagdCrvEval(D2, v);
    CagdCoerceToE3(D2Pt, &R, -1, D2 -> PType);

    IRIT_PT_SUB(T1, C1Pt, D1Pt);
    IRIT_PT_SUB(T2, C1Pt, C2Pt);
    IRIT_PT_SUB(T3, D2Pt, D1Pt);
    IRIT_PT_COPY(NT2, T2);
    IRIT_PT_NORMALIZE(NT2);
    IRIT_PT_COPY(NT3, T3);
    IRIT_PT_NORMALIZE(NT3);
    IRIT_CROSS_PROD(NT2xNT3, NT2, NT3);
    Delta = IRIT_DOT_PROD(NT2xNT3, NT2xNT3);		   /* Delta = sin^2. */

#   ifdef PRINT_DELTAS
    {
	CagdPType	T1xNT2, T1xNT3;
	CagdRType	Vdelta1, Vdelta2;

	IRIT_INFO_MSG_PRINTF("(uvDd1d2)=(%.8f %.8f %.16f", u, v, Delta);
	IRIT_CROSS_PROD(T1xNT2, T1, NT2);
	IRIT_CROSS_PROD(T1xNT3, T1, NT3);
	Vdelta1 = IRIT_DOT_PROD(T1xNT2, T1xNT2);
	Vdelta2 = IRIT_DOT_PROD(T1xNT3, T1xNT3);
	IRIT_INFO_MSG_PRINTF(" %.16f %.16f)\n", Vdelta1, Vdelta2);
    }
#   endif /* PRINT_DELTAS */

    if (IRIT_APX_EQ_EPS(Delta, 0.0, DELTA_EPS)) {	      /* Delta == 0. */
	CagdPType T1xNT2, T1xNT3;
	CagdRType Vdelta;

	IRIT_CROSS_PROD(T1xNT2, T1, NT2);
	IRIT_CROSS_PROD(T1xNT3, T1, NT3);
	Vdelta = IRIT_DOT_PROD(T1xNT2, T1xNT2) + IRIT_DOT_PROD(T1xNT3, T1xNT3);

#	ifdef PRINT_PARALLEL_CASE
	    IRIT_INFO_MSG_PRINTF(
	   	">>>>> Delta = %.16f, delta1+2 = %.16f <<<<<<<\n",
	   	Delta, Vdelta);
#	endif /* PRINT_PARALLEL_CASE */

	if (IRIT_APX_EQ_EPS(Vdelta, 0.0, DDELTA_EPS)) {
	    return -1;					     /* Common line. */
	}
	else {
	    return -2;					  /* Redundant case. */
	}
    }
    else {
	CagdRType A, B, D, E, F, Det;

	/* Delta neq 0 --> Det neq 0. */

	A = IRIT_DOT_PROD(T2, T2);
	B = IRIT_DOT_PROD(T2, T3);
	D = IRIT_DOT_PROD(T3, T3);
	E = IRIT_DOT_PROD(T1, T2);
	F = IRIT_DOT_PROD(T1, T3);

	Det = A * D - B * B;

	*s = (E * D - F * B) / Det;
	*t = (A * F - B * E) / Det;
    }

    return 0;
}

/*****************************************************************************
* AUXILIARY:                                                                 *
*   Auxiliary function to compute the gamma(u,v).		             *
*****************************************************************************/
static CagdSrfStruct *SymbRuledRuledAux(CagdCrvStruct **C1,
					CagdCrvStruct **C2,
					CagdCrvStruct **D1,
					CagdCrvStruct **D2,
					CagdSrfStruct **C1Srf,
					CagdSrfStruct **C2Srf,
					CagdSrfStruct **D1Srf,
					CagdSrfStruct **D2Srf,
					CagdSrfStruct **TSrf1,
					CagdSrfStruct **TSrf2,
					CagdSrfStruct **TSrf3)
{
    CagdSrfStruct *TSrf0,
	*Det11, *Det12, *Det13, *Det21, *Det22, *Det23, *Det31, *Det32, *Det33;

    if (CAGD_NUM_OF_PT_COORD((*C1) -> PType) < 3 ||
	CAGD_NUM_OF_PT_COORD((*C2) -> PType) < 3 ||
	CAGD_NUM_OF_PT_COORD((*D1) -> PType) < 3 ||
	CAGD_NUM_OF_PT_COORD((*D2) -> PType) < 3) {
	SYMB_FATAL_ERROR(SYMB_ERR_ONLY_3D);
	return NULL;
    }

    if (CAGD_IS_BSPLINE_CRV(*C1) ||
	CAGD_IS_BSPLINE_CRV(*C2) ||
	CAGD_IS_BSPLINE_CRV(*D1) ||
	CAGD_IS_BSPLINE_CRV(*D2)) {
	if (CAGD_IS_BEZIER_CRV(*C1))
	    *C1 = CagdCnvrtBzr2BspCrv(*C1);
	else
	    *C1 = CagdCrvCopy(*C1);
	if (CAGD_IS_BEZIER_CRV(*C2))
	    *C2 = CagdCnvrtBzr2BspCrv(*C2);
	else
	    *C2 = CagdCrvCopy(*C2);
	if (CAGD_IS_BEZIER_CRV(*D1))
	    *D1 = CagdCnvrtBzr2BspCrv(*D1);
	else
	    *D1 = CagdCrvCopy(*D1);
	if (CAGD_IS_BEZIER_CRV(*D2))
	    *D2 = CagdCnvrtBzr2BspCrv(*D2);
	else
	    *D2 = CagdCrvCopy(*D2);

	BspKnotAffineTrans2((*C1) -> KnotVector,
			    (*C1) -> Length + (*C1) -> Order, 0.0, 1.0);
	BspKnotAffineTrans2((*C2) -> KnotVector,
			    (*C2) -> Length + (*C2) -> Order, 0.0, 1.0);
	BspKnotAffineTrans2((*D1) -> KnotVector,
			    (*D1) -> Length + (*D1) -> Order, 0.0, 1.0);
	BspKnotAffineTrans2((*D2) -> KnotVector,
			    (*D2) -> Length + (*D2) -> Order, 0.0, 1.0);
    }
    else {
	*C1 = CagdCrvCopy(*C1);
	*C2 = CagdCrvCopy(*C2);
	*D1 = CagdCrvCopy(*D1);
	*D2 = CagdCrvCopy(*D2);
    }

    
    *C1Srf = CagdPromoteCrvToSrf(*C1, CAGD_CONST_U_DIR);
    *C2Srf = CagdPromoteCrvToSrf(*C2, CAGD_CONST_U_DIR);

    *D1Srf = CagdPromoteCrvToSrf(*D1, CAGD_CONST_V_DIR);
    *D2Srf = CagdPromoteCrvToSrf(*D2, CAGD_CONST_V_DIR);

    CagdMakeSrfsCompatible(C1Srf, D1Srf, TRUE, TRUE, TRUE, TRUE);
    *TSrf1 = SymbSrfSub(*C1Srf, *D1Srf);
    SymbSrfSplitScalar(*TSrf1, &TSrf0, &Det11, &Det12, &Det13);
    if (TSrf0 != NULL)
        CagdSrfFree(TSrf0);

    CagdMakeSrfsCompatible(C1Srf, C2Srf, TRUE, TRUE, TRUE, TRUE);
    *TSrf2 = SymbSrfSub(*C1Srf, *C2Srf);
    SymbSrfSplitScalar(*TSrf2, &TSrf0, &Det21, &Det22, &Det23);
    if (TSrf0 != NULL)
        CagdSrfFree(TSrf0);

    CagdMakeSrfsCompatible(D2Srf, D1Srf, TRUE, TRUE, TRUE, TRUE);
    *TSrf3 = SymbSrfSub(*D2Srf, *D1Srf);
    SymbSrfSplitScalar(*TSrf3, &TSrf0, &Det31, &Det32, &Det33);
    if (TSrf0 != NULL)
        CagdSrfFree(TSrf0);

    TSrf0 = SymbSrfDeterminant3(Det11, Det12, Det13,
				Det21, Det22, Det23,
				Det31, Det32, Det33);

    CagdSrfFree(Det11);
    CagdSrfFree(Det12);
    CagdSrfFree(Det13);
    CagdSrfFree(Det21);
    CagdSrfFree(Det22);
    CagdSrfFree(Det23);
    CagdSrfFree(Det31);
    CagdSrfFree(Det32);
    CagdSrfFree(Det33);


    return TSrf0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curve of two ring surfaces:                    M
*                                                                            M
*        S1(u, t) = C1(u) + Circ1(t) r1(u)				     V
*        S2(v, s) = C2(v) + Circ2(s) r2(v)				     V
*                                                                            M
* where Circ1 and Circ2 are oriented to be in the normal plane of C1/C2.     M
* The intersection is derived from the zero set of the function that is      M
* computed by SymbRingRingZeroSetFunc.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   C1, r1:      The two curves prescribing the first ring surface. Must be  M
*	         integral curves.					     M
*   C2, r2:      The two curves prescribing the second ring surface. Must be M
*	         integral curves.					     M
*   Tolerance:   Accuracy of zero set computation as part of the solution.   M
*                Value of 10 is a good start.				     M
*   PCrvs1, PCrvs2:  The parametric domains of the intersection curves, in   M
*		 the two surfaces.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Intersection curves in Euclidean space.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRingRingZeroSetFunc			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRingRingIntersection                                                 M
*****************************************************************************/
CagdCrvStruct *SymbRingRingIntersection(CagdCrvStruct *C1,
					CagdCrvStruct *r1,
					CagdCrvStruct *C2,
					CagdCrvStruct *r2,
					CagdRType Tolerance,
					CagdCrvStruct **PCrvs1,
					CagdCrvStruct **PCrvs2)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, CONTOUR_EPS };   /* A scalar srf - only X. */
    CagdCrvStruct *n1, *n2;
    CagdSrfStruct *TSrf;
    IPPolygonStruct *Cntrs, *Cntr;
    CagdCrvStruct *E3Crvs;
    CagdRType C1Min, C1Max, C2Min, C2Max;

#   ifdef PRINT_PERF
	CagdRType StartTime, CrntTime;
	StartTime = IritCPUTime(FALSE);
#   endif /* PRINT_PERF */

    *PCrvs1 = NULL;
    *PCrvs2 = NULL;
    E3Crvs = NULL;

    TSrf = SymbRingRingZeroSetFunc(C1, r1, C2, r2);

    /* Compute the zero set of the equation as contours Gamma(u,v) = 0. */
    Cntrs = UserCntrSrfWithPlane(TSrf, Plane, Tolerance);
    CagdSrfFree(TSrf);

    CagdCrvDomain(C1, &C1Min, &C1Max); /* Assume C1 and r1 have same Domain. */
    CagdCrvDomain(C2, &C2Min, &C2Max); /* Assume C2 and r2 have same Domain. */

    n1 = CagdCrvDerive(C1);
    n2 = CagdCrvDerive(C2);

    /* Form the intersection curves. */
    for (Cntr = Cntrs; Cntr != NULL;  Cntr = Cntr -> Pnext) {
	int i,
	    Len = IPVrtxListLen(Cntr -> PVertex);
	CagdCrvStruct *Crv;
	CagdRType **PPoints1, **PPoints2, **E3Points;
	IPVertexStruct *V;

	if (Len < 2)
	    continue;

	/* Create pair of curves as each solution occurs twice. */
	Crv = BspCrvNew(Len, 2, CAGD_PT_E2_TYPE);
	IRIT_LIST_PUSH(Crv, *PCrvs1);

	Crv = BspCrvNew(Len, 2, CAGD_PT_E2_TYPE);
	IRIT_LIST_PUSH(Crv, *PCrvs2);

	Crv = BspCrvNew(Len, 2, CAGD_PT_E3_TYPE);
	IRIT_LIST_PUSH(Crv, E3Crvs);

	BspKnotUniformOpen(Len, 2, (*PCrvs1) -> KnotVector);
	BspKnotUniformOpen(Len, 2, (*PCrvs2) -> KnotVector);
	BspKnotUniformOpen(Len, 2, E3Crvs -> KnotVector);

	PPoints1 = (*PCrvs1) -> Points;
	PPoints2 = (*PCrvs2) -> Points;
	E3Points = E3Crvs -> Points;

	for (V = Cntr -> PVertex, i = 0; V != NULL; V = V -> Pnext, i++) {
	    CagdRType *R, Rad1, Rad2, Err1, Err2;
	    CagdPType Cntr1, Cntr2, Inter1, Inter2;
	    CagdVType Nrml1, Nrml2;

	    V -> Coord[1] = IRIT_BOUND(V -> Coord[1], C1Min, C1Max);
	    V -> Coord[2] = IRIT_BOUND(V -> Coord[2], C2Min, C2Max);

	    R = CagdCrvEval(C1, V -> Coord[1]);
	    CagdCoerceToE3(Cntr1, &R, -1, C1 -> PType);
	    R = CagdCrvEval(C2, V -> Coord[2]);
	    CagdCoerceToE3(Cntr2, &R, -1, C2 -> PType);

	    R = CagdCrvEval(n1, V -> Coord[1]);
	    CagdCoerceToE3(Nrml1, &R, -1, C1 -> PType);
	    R = CagdCrvEval(n2, V -> Coord[2]);
	    CagdCoerceToE3(Nrml2, &R, -1, C2 -> PType);

	    R = CagdCrvEval(r1, V -> Coord[1]);
	    CagdCoercePointTo(&Rad1, CAGD_PT_E1_TYPE, &R, -1, C1 -> PType);
	    R = CagdCrvEval(r2, V -> Coord[2]);
	    CagdCoercePointTo(&Rad2, CAGD_PT_E1_TYPE, &R, -1, C2 -> PType);

	    /* Compute the intersections of the two circle is E3. */
	    if (GM2PointsFromCircCirc3D(Cntr1, Nrml1, Rad1, Cntr2, Nrml2, Rad2,
					Inter1, Inter2) == 0) {
	        SYMB_FATAL_ERROR(SYMB_ERR_TOO_COMPLEX);
	    }

	    /* Test against the distance to the second circle. */
	    Err1 = IRIT_FABS(IRIT_PT_PT_DIST(Inter1, Cntr2) - Rad2);
	    Err2 = IRIT_FABS(IRIT_PT_PT_DIST(Inter2, Cntr2) - Rad2);
	    if (Err1 > Err2)
		IRIT_PT_COPY(Inter1, Inter2);

	    /* Update the first solution. */
	    PPoints1[1][i] = V -> Coord[1];
	    PPoints1[2][i] = 0.0;     /* Should contain parameter in 0..2Pi. */

	    PPoints2[1][i] = V -> Coord[2];
	    PPoints2[2][i] = 0.0;     /* Should contain parameter in 0..2Pi. */

	    E3Points[1][i] = Inter1[0];
	    E3Points[2][i] = Inter1[1];
	    E3Points[3][i] = Inter1[2];
	}
    }

    CagdCrvFree(n1);
    CagdCrvFree(n2);
    IPFreePolygonList(Cntrs);

#   ifdef PRINT_PERF
	CrntTime = IritCPUTime(FALSE);
	IRIT_INFO_MSG_PRINTF(
		"> Time spent for RingRingSrfIntersection : %lf sec\n",
		CrntTime - StartTime);
#   endif /* PRINT_PERF */

    return E3Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curve of two ring surfaces:                    M
*                                                                            M
*        S1(u, t) = C1(u) + Circ1(t) r1(u)				     V
*        S2(v, s) = C2(v) + Circ2(s) r2(v)				     V
*                                                                            M
* where Circ1 and Circ2 are oriented to be in the normal plane of C1/C2.     M
*                                                                            M
* Let n1(u) and n2(v) be the normals of the normal plane of C1/C2,	     M
* n1(u) = C1'(u), n2(v) = C2'(v).  Then, solve for x(u, v), y(u, v), z(u, v) M
*                                                                            M
*  |     n1(u)     | | x |     | < n1(u), C1(u) >                      |     V
*  |               | |   |     |                                       |     V
*  |     n2(v)     | | y |  =  | < n2(v), C2(v) >                      |     V
*  |               | |   |     |                                       |     V
*  | C1(u) - C2(v) | | z |     | ( < C1(u), C1(u) > - < C2(v), C2(v) > |     V
*  |               | |   |     |             + r2^2(v) - r1^2(u) ) / 2 |     V
*                                                                            M
*  Lets P(u, v) = (x(u, v), y(u, v), z(u, v)).  Find the zero set of         M
*                                                                            M
*          F(u, v):  < P(u, v) - C1(u), P(u, v) - C1(u) > - r1^2(u) = 0,     M
*                                                                            M
*  or, alternatively, the zero set of,                                       M
*                                                                            M
*          F(u, v):  < P(u, v) - C2(v), P(u, v) - C2(v) > - r2^2(v) = 0.     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   C1, r1:  The two curves prescribing the first ring surface. Must be      M
*	     integral curves.						     M
*   C2, r2:  The two curves prescribing the second ring surface. Must be     M
*	     integral curves.					             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   F(u, v).						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbRingRingIntersection                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbRingRingZeroSetFunc                                                  M
*****************************************************************************/
CagdSrfStruct *SymbRingRingZeroSetFunc(CagdCrvStruct *C1,
				       CagdCrvStruct *r1,
				       CagdCrvStruct *C2,
				       CagdCrvStruct *r2)
{
    CagdRType TMin, TMax, t1, t2;
    CagdSrfStruct *TSrf0, *TSrf1, *TSrf2, *TSrf3, *TSrf4,
	*C1Srf, *C2Srf, *r1Srf, *r2Srf, *n1Srf, *n2Srf, *b1, *b2, *b3,
	*Det11, *Det12, *Det13, *Det21, *Det22, *Det23, *Det31, *Det32, *Det33;

    if (CAGD_NUM_OF_PT_COORD(C1 -> PType) < 3 ||
	CAGD_NUM_OF_PT_COORD(C2 -> PType) < 3) {
	SYMB_FATAL_ERROR(SYMB_ERR_ONLY_3D);
	return NULL;
    }

    if (CAGD_IS_RATIONAL_CRV(C1) ||
	CAGD_IS_RATIONAL_CRV(r1) ||
	CAGD_IS_RATIONAL_CRV(C2) ||
	CAGD_IS_RATIONAL_CRV(r2)) {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    if (CAGD_IS_BSPLINE_CRV(C1) ||
	CAGD_IS_BSPLINE_CRV(C2) ||
	CAGD_IS_BSPLINE_CRV(r1) ||
	CAGD_IS_BSPLINE_CRV(r2)) {
	if (CAGD_IS_BEZIER_CRV(C1))
	    C1 = CagdCnvrtBzr2BspCrv(C1);
	else
	    C1 = CagdCrvCopy(C1);
	if (CAGD_IS_BEZIER_CRV(C2))
	    C2 = CagdCnvrtBzr2BspCrv(C2);
	else
	    C2 = CagdCrvCopy(C2);
	if (CAGD_IS_BEZIER_CRV(r1))
	    r1 = CagdCnvrtBzr2BspCrv(r1);
	else
	    r1 = CagdCrvCopy(r1);
	if (CAGD_IS_BEZIER_CRV(r2))
	    r2 = CagdCnvrtBzr2BspCrv(r2);
	else
	    r2 = CagdCrvCopy(r2);

	CagdCrvDomain(C1, &TMin, &TMax);
	CagdCrvDomain(r1, &t1, &t2);
	if (!IRIT_APX_EQ(TMin, t1) || !IRIT_APX_EQ(TMax, t2)) {
	    SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	    return NULL;
	}
	CagdCrvDomain(C2, &TMin, &TMax);
	CagdCrvDomain(r2, &t1, &t2);
	if (!IRIT_APX_EQ(TMin, t1) || !IRIT_APX_EQ(TMax, t2)) {
	    SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	    return NULL;
	}	
    }
    else {
	C1 = CagdCrvCopy(C1);
	C2 = CagdCrvCopy(C2);
	r1 = CagdCrvCopy(r1);
	r2 = CagdCrvCopy(r2);
    }

    /* Promote C1/r1 and C2/r2 to surfaces and compute n1/n2. */
    C1Srf = CagdPromoteCrvToSrf(C1, CAGD_CONST_U_DIR);
    n1Srf = CagdSrfDerive(C1Srf, CAGD_CONST_U_DIR);
    r1Srf = CagdPromoteCrvToSrf(r1, CAGD_CONST_U_DIR);

    C2Srf = CagdPromoteCrvToSrf(C2, CAGD_CONST_V_DIR);
    n2Srf = CagdSrfDerive(C2Srf, CAGD_CONST_V_DIR);
    r2Srf = CagdPromoteCrvToSrf(r2, CAGD_CONST_V_DIR);

    CagdCrvFree(C1);
    CagdCrvFree(r1);
    CagdCrvFree(C2);
    CagdCrvFree(r2);

    /* Compute the A matrix in the 'Ax = b' linear system. */
    SymbSrfSplitScalar(n1Srf, &TSrf1, &Det11, &Det12, &Det13);
    SymbSrfSplitScalar(n2Srf, &TSrf1, &Det21, &Det22, &Det23);
    TSrf2 = SymbSrfSub(C1Srf, C2Srf);
    SymbSrfSplitScalar(TSrf2, &TSrf1, &Det31, &Det32, &Det33);
    CagdSrfFree(TSrf2);

    /* Compute the b vector in the 'Ax = b' linear system. */
    b1 = SymbSrfDotProd(n1Srf, C1Srf);
    b2 = SymbSrfDotProd(n2Srf, C2Srf);
    TSrf1 = SymbSrfDotProd(C1Srf, C1Srf);
    TSrf2 = SymbSrfDotProd(C2Srf, C2Srf);
    TSrf3 = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfMult(r1Srf, r1Srf);
    TSrf2 = SymbSrfMult(r2Srf, r2Srf);
    TSrf4 = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    b3 = SymbSrfSub(TSrf3, TSrf4);
    CagdSrfFree(TSrf3);
    CagdSrfFree(TSrf4);
    CagdSrfTransform(b3, NULL, 0.5);

    /* Solve the linear system. */
    TSrf0 = SymbSrfDeterminant3(Det11, Det12, Det13,
				Det21, Det22, Det23,
				Det31, Det32, Det33);
    TSrf1 = SymbSrfDeterminant3(b1,    Det12, Det13,
				b2,    Det22, Det23,
				b3,    Det32, Det33);
    TSrf2 = SymbSrfDeterminant3(Det11, b1,    Det13,
				Det21, b2,    Det23,
				Det31, b3,    Det33);
    TSrf3 = SymbSrfDeterminant3(Det11, Det12, b1,
				Det21, Det22, b2,
				Det31, Det32, b3);
    CagdSrfFree(Det11);
    CagdSrfFree(Det12);
    CagdSrfFree(Det13);
    CagdSrfFree(Det21);
    CagdSrfFree(Det22);
    CagdSrfFree(Det23);
    CagdSrfFree(Det31);
    CagdSrfFree(Det32);
    CagdSrfFree(Det33);
    CagdSrfFree(b1);
    CagdSrfFree(b2);
    CagdSrfFree(b3);
  
    /* Compute F(u, v) as "< P(u, v) - C1(u), P(u, v) - C1(u) > - r1^2(u)". */
    TSrf4 = SymbSrfMergeScalar(NULL, TSrf1, TSrf2, TSrf3);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);
    TSrf1 = SymbSrfMultScalar(C1Srf, TSrf0);

    TSrf2 = SymbSrfSub(TSrf4, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf4);
    TSrf4 = SymbSrfDotProd(TSrf2, TSrf2);
    CagdSrfFree(TSrf2);

    TSrf2 = SymbSrfMult(r1Srf, TSrf0);
    CagdSrfFree(TSrf0);
    TSrf3 = SymbSrfMult(TSrf2, TSrf2);
    CagdSrfFree(TSrf2);
    TSrf1 = SymbSrfSub(TSrf4, TSrf3);
    CagdSrfFree(TSrf4);
    CagdSrfFree(TSrf3);

    CagdSrfFree(C1Srf);
    CagdSrfFree(n1Srf);
    CagdSrfFree(r1Srf);
    CagdSrfFree(C2Srf);
    CagdSrfFree(n2Srf);
    CagdSrfFree(r2Srf);

    return TSrf1;
}
