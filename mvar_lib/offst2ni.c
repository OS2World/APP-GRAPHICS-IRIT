/******************************************************************************
* Offst2ni.c -    Improves self-intersections of offset surfaces using        *
*                 numerical tracing method.                                   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Joon-Kyung Seong, Sep. 2006.				      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "mvar_loc.h"
#include "user_lib.h"
#include "allocate.h"
#include "geom_lib.h"

/* Global variables for a numerical tracing. */
IRIT_STATIC_DATA CagdSrfStruct
    *GlblSrf1 = NULL,
    *GlblSrf1u = NULL,
    *GlblSrf1v = NULL,
    *GlblSrf1uu = NULL,
    *GlblSrf1uv = NULL,
    *GlblSrf1vv = NULL,
    *GlblSrf1uuu = NULL,
    *GlblSrf1uuv = NULL,
    *GlblSrf1uvv = NULL,
    *GlblSrf1vvv = NULL;
IRIT_STATIC_DATA CagdSrfStruct
    *GlblSrf2 = NULL,
    *GlblSrf2u = NULL,
    *GlblSrf2v = NULL,
    *GlblSrf2uu = NULL,
    *GlblSrf2uv = NULL,
    *GlblSrf2vv = NULL,
    *GlblSrf2uuu = NULL,
    *GlblSrf2uuv = NULL,
    *GlblSrf2uvv = NULL,
    *GlblSrf2vvv = NULL;
IRIT_STATIC_DATA CagdPType GlblSrf1Pt0, GlblSrf2Pt0, GlblR0;
IRIT_STATIC_DATA CagdVType GlblTangent, GlblNormal, GlblBiNormal,
    GlblXu, GlblXv, GlblXuu, GlblXuv, GlblXvv, GlblXuuu, GlblXuuv, GlblXuvv,
    GlblXvvv, GlblNx, GlblYu, GlblYv, GlblYuu, GlblYuv, GlblYvv, GlblYuuu,
    GlblYuuv, GlblYuvv, GlblYvvv, GlblNy;
IRIT_STATIC_DATA CagdUVType GlblU1, GlblU1p, GlblU1pp, GlblU1ppp,
    GlblU2, GlblU2p, GlblU2pp, GlblU2ppp;
IRIT_STATIC_DATA CagdRType GlblKappa, GlblKappa_p, GlblTau,
    GlblUmin1, GlblUmax1, GlblVmin1, GlblVmax1, GlblUmin2, GlblUmax2,
    GlblVmin2, GlblVmax2, GlblSameUVTol;

#define MVAR_OFST_NI_E3_TO_VEC(E, V) (V[0] = E[1], V[1] = E[2], V[2] = E[3])
#define MVAR_OFST_NI_E3_TO_PT(E, Pt) (Pt[0] = E[1], Pt[1] = E[2], Pt[2] = E[3])
#define MVAR_OFST_NI_E2_TO_UV(P, Uv) (Uv[0] = P[1], Uv[1] = P[2])

/* Function declarations. */

static CagdCrvStruct *NumericalImprovePolylines(IPPolygonStruct *Pl1, 
						IPPolygonStruct *Pl2, 
						const CagdSrfStruct *OffSrf,
						CagdRType NumerTol,
						IrtRType SeedFactor);
static MvarPtStruct *NumericalImproveVertices(IrtRType *V1, 
					      IrtRType *V2, 
					      const CagdSrfStruct *OffSrf,
					      CagdRType NumerTol, 
					      CagdRType OrigDistance);
static void InitGlobalVars(const CagdSrfStruct *X, const CagdSrfStruct *Y);
static void FreeGlobalVars(void);
static CagdCrvStruct *TraceIntersectionCrv(IrtRType *U1,
					   IrtRType *U2,
					   IrtRType Epsilon,
					   int LocalSelfIntr);
static void EvalCompAtCurParam(void);
static void GetFirstOrderComponents(CagdBType Reverse);
static void GetSecondOrderComponents(void);
static void GetThirdOrderComponents(void);
static CagdCrvStruct *SSIApxInterCrv(void);
static IrtRType SSIStepDistance(CagdCrvStruct **C, IrtRType Tol);
static CagdBType SSIPointRefine(IrtRType Tol);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Improve self-intersection curves of offset surfaces                      M
*   using numerical methods.                                                 M
*                                                                            M
* PARAMETERS:                                                                M
*   Plls:        Connected list of polylines to be improved.                 M
*   OffSrf:      The offset surface approximation.      		     M
*   SubdivTol:   Accuracy of computation.				     M
*   NumerTol:    NumerTol tolerance.        				     M
*   Euclidean:   If TRUE, returned data is in the Euclidean space of	     M
*		 OffSrf.  FALSE for parametric space of OffSrf.	 	     M
*   SameUVTol:   Tolerance for termination condition around singular points. M
*									     M
* RETURN VALUE:							 	     M
*   IPObjectStruct *:  A list of self inter. curves, on the offset surface.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfTrimGlblOffsetSelfInterNI                                         M
*****************************************************************************/
IPObjectStruct *MvarSrfTrimGlblOffsetSelfInterNI(IPPolygonStruct *Plls, 
						 const CagdSrfStruct *OffSrf, 
						 CagdRType SubdivTol, 
						 CagdRType NumerTol,
						 int Euclidean,
						 CagdRType SameUVTol)
{
    CagdCrvStruct
	*SSICrv = NULL,
	*SSICrvList = NULL,
	*SSICrvListUV = NULL;
    IPPolygonStruct *Pl, *MatchPl;

    GlblSameUVTol = SameUVTol;

    /* Initialize global variables for a tracing method Recall that this is */
    /* self-intersection computing problem.				    */
    InitGlobalVars(OffSrf, OffSrf);

    for (Pl = Plls; Pl != NULL; Pl = Pl -> Pnext)
	AttrSetIntAttrib(&Pl -> Attr, "Improved", 0);

    for (Pl = Plls; Pl != NULL; Pl = Pl -> Pnext) {
	IPPolygonStruct
	    *MatchedPl = NULL;
	CagdRType *R, Pt1[3], Pt2[3], Dist,
	    MatchSmall = IRIT_INFNTY;
	IPVertexStruct *V;
	int ImprovedPolyline = AttrGetIntAttrib(Pl -> Attr, "Improved");

	/* Already improved. */
	if (ImprovedPolyline)
	    continue;

	R = CagdSrfEval(OffSrf, Pl -> PVertex -> Coord[0],
				Pl -> PVertex -> Coord[1]);
	CagdCoerceToE3(Pt1, &R, -1, OffSrf -> PType);

	for (MatchPl = Pl -> Pnext;
	     MatchPl != NULL;
	     MatchPl = MatchPl -> Pnext) {
	    CagdRType
		Small = IRIT_INFNTY;

	    ImprovedPolyline = AttrGetIntAttrib(MatchPl -> Attr, "Improved");
	    if (ImprovedPolyline)
		continue;

	    /* Find the matching pair. */
	    for (V = MatchPl -> PVertex; V != NULL; V = V -> Pnext) {
		R = CagdSrfEval(OffSrf, V -> Coord[0], V -> Coord[1]);
		CagdCoerceToE3(Pt2, &R, -1, OffSrf -> PType);
		Dist = IRIT_PT_PT_DIST_SQR(Pt1, Pt2);
		if (Dist < Small)
		    Small = Dist;
	    }

	    if (Small < MatchSmall) {
		MatchSmall = Small;
		MatchedPl = MatchPl;
	    }
	}

	if (MatchSmall > 0.1) {
	    /* Find the matching pair inside the polyline itself. */
	    for (V = Pl -> PVertex -> Pnext; V != NULL; V = V -> Pnext) {
		if (IRIT_PT_APX_EQ_E2_EPS(V -> Coord, Pl -> PVertex -> Coord, 0.01))
		    continue;
		R = CagdSrfEval(OffSrf, V -> Coord[0], V -> Coord[1]);
		CagdCoerceToE3(Pt2, &R, -1, OffSrf -> PType);
		Dist = IRIT_PT_PT_DIST_SQR(Pt1, Pt2);
		if (Dist < MatchSmall)
		  MatchSmall = Dist;
	    }

	    if (MatchSmall > 0.1)
		fprintf(stderr, "No match: never happens.\n");
	    MatchedPl = Pl;
	}

	/* Numerically improve two matched polylines. */
	SSICrv = NumericalImprovePolylines(Pl, MatchedPl, OffSrf,
					   NumerTol, 2.0);
	if (SSICrv == NULL)
	    SSICrv = NumericalImprovePolylines(Pl, MatchedPl,
					       OffSrf, NumerTol, 3.0);

	if (SSICrv != NULL) {
	    IRIT_LIST_PUSH(SSICrv -> Pnext -> Pnext, SSICrvList);
	    IRIT_LIST_PUSH(SSICrv -> Pnext, SSICrvListUV);
	    IRIT_LIST_PUSH(SSICrv, SSICrvListUV);
	    AttrSetIntAttrib(&MatchedPl -> Attr, "Improved", 1);
	}
    }
    IPFreePolygonList(Plls);

    FreeGlobalVars();

    /* Connect a list of mvar points into a list of polylines and return it. */
    if (Euclidean) {
	CagdCrvFreeList(SSICrvListUV);
	return IPGenCRVObject(SSICrvList);
    }
    else {
	CagdCrvFreeList(SSICrvList);
	return IPGenCRVObject(SSICrvListUV);
    }
}

/*****************************************************************************
* DESCRIPTION:							  	     *
*   Given two matching polylines, improve them into a real intersection.     *
*									     *
* PARAMETERS:								     *
*   Pl1/Pl2:      Polyline to be improved.				     *
*   OffSrf:       The offset surface approximation.      		     *
*   NumerTol:     NumerTol tolerance.					     *
*   SeedFactor:   Determine starting point.				     *
*									     *
* RETURN VALUE:							 	     *
*   CagdCrvStruct:    Improved list of polylines.			     *
*****************************************************************************/
static CagdCrvStruct *NumericalImprovePolylines(IPPolygonStruct *Pl1, 
						IPPolygonStruct *Pl2, 
						const CagdSrfStruct *OffSrf, 
						CagdRType NumerTol,
						IrtRType SeedFactor)
{
    IPVertexStruct *V1, *V2, *MatchV;
    MvarPtStruct *ImprovedPt;
    CagdCrvStruct *SSICrv;
    IrtRType *R, Pt1[3],
	Small = IRIT_INFNTY,
	Epsilon = 0.0001;               /* Epsilon for a numerical tracing. */
    int LocalSelfIntr = FALSE;

    /* Check whether this polyline is loop or not. */
    V1 = V2 = Pl1 -> PVertex;
    IRIT_LIST_LAST_ELEM(V2);
    if (!IRIT_PT_APX_EQ_E2_EPS(V1 -> Coord, V2 -> Coord, 0.01) ||
	Pl1 == Pl2) {
	int i,
	    Size = CagdListLength(Pl1 -> PVertex),
	    NewStart = (int) (Size / SeedFactor);

	for (i = 0, V1 = Pl1 -> PVertex; i < NewStart; i++)
	    V1 = V1 -> Pnext;
	LocalSelfIntr = TRUE;
    }

    /* Improve the seed point. */
    R = CagdSrfEval(OffSrf, V1 -> Coord[0], V1 -> Coord[1]);
    CagdCoerceToE3(Pt1, &R, -1, OffSrf -> PType);

    MatchV = NULL;
    for (V2 = Pl2 -> PVertex; V2 != NULL; V2 = V2 -> Pnext) {
	CagdRType Pt2[3], Dist;

	if (IRIT_PT_APX_EQ_E2_EPS(V1 -> Coord, V2 -> Coord, 0.03))
	    continue;

	R = CagdSrfEval(OffSrf, V2 -> Coord[0], V2 -> Coord[1]);
	CagdCoerceToE3(Pt2, &R, -1, OffSrf -> PType);
	Dist = IRIT_PT_PT_DIST_SQR(Pt1, Pt2);
	if (Dist < Small) {
	    Small = Dist;
	    MatchV = V2;
	}
    }

    /* Numerically improve two vertices. */
    ImprovedPt = NumericalImproveVertices(V1 -> Coord, MatchV -> Coord,
					  OffSrf, NumerTol, Small);

    /* Numerically tracing real self-intersections from the starting point. */
    if (ImprovedPt != NULL)
	SSICrv = TraceIntersectionCrv(ImprovedPt -> Pt,
				      ImprovedPt -> Pnext -> Pt, 
				      Epsilon, LocalSelfIntr);
    else {
	fprintf(stderr, "Failed to improve a single point");
	return NULL;
    }
	
    return SSICrv;
}	    

/*****************************************************************************
* DESCRIPTION:							 	     *
*   Improve two points into a real intersection.			     *
*									     *
* PARAMETERS:							 	     *
*   V1/V2:	  Vertices to be improved.				     *
*   OffSrf:       The offset surface approximation.      		     *
*   NumerTol:     NumerTol tolerance.				 	     *
*   OrigDistance: Original distance.					     *
*									     *
* RETURN VALUE:							  	     *
*   MvarPtStruct:    Improved list of vertices, NULL if failed.	  	     *
*****************************************************************************/
static MvarPtStruct *NumericalImproveVertices(IrtRType *V1, 
					      IrtRType *V2, 
					      const CagdSrfStruct *OffSrf,
					      CagdRType NumerTol, 
					      CagdRType OrigDistance)
{
    IrtPtType p, q, Suq, Svq, Sup, Svp, CurrPt1, CurrPt2,
	NewPt1, NewPt2, Closest_q, Closest_p, IntersectPt, Pt1;
    IrtVecType VPlane, V;
    IrtPlnType Plane_p, Plane_q;
    IrtRType *R, Duv[2], Distance,
	CurrDistance = OrigDistance;
    CagdSrfStruct
	*SrfSu = CagdSrfDerive(OffSrf, CAGD_CONST_U_DIR),
	*SrfSv = CagdSrfDerive(OffSrf, CAGD_CONST_V_DIR);
    int Iteration = 0,
	MaxIteration = 50;
    MvarPtStruct *PtList;

    IRIT_PT_COPY(CurrPt1, V1);
    IRIT_PT_COPY(CurrPt2, V2);
    IRIT_PT_COPY(NewPt1, CurrPt1);
    IRIT_PT_COPY(NewPt2, CurrPt2);

    do {
	R = CagdSrfEval(OffSrf, CurrPt1[0], CurrPt1[1]);
	CagdCoerceToE3(q, &R, -1, OffSrf -> PType);
	R = CagdSrfEval(OffSrf, CurrPt2[0], CurrPt2[1]);
	CagdCoerceToE3(p, &R, -1, OffSrf -> PType);

	/* Compute two tangent planes. */
	R = CagdSrfEval(SrfSu, CurrPt1[0], CurrPt1[1]);
	CagdCoerceToE3(Suq, &R, -1, SrfSu -> PType);
	IRIT_PT_NORMALIZE(Suq);
	R = CagdSrfEval(SrfSv, CurrPt1[0], CurrPt1[1]);
	CagdCoerceToE3(Svq, &R, -1, SrfSv -> PType);
	IRIT_PT_NORMALIZE(Svq);

	R = CagdSrfEval(SrfSu, CurrPt2[0], CurrPt2[1]);
	CagdCoerceToE3(Sup, &R, -1, SrfSu -> PType);
	IRIT_PT_NORMALIZE(Sup);
	R = CagdSrfEval(SrfSv, CurrPt2[0], CurrPt2[1]);
	CagdCoerceToE3(Svp, &R, -1, SrfSv -> PType);
	IRIT_PT_NORMALIZE(Svp);
	
	/* Compute an intersection line between two planes. */
	GMVecCrossProd(Plane_q, Suq, Svq);
	IRIT_PT_NORMALIZE(Plane_q);
	Plane_q[3] = -IRIT_DOT_PROD(Plane_q, q);
	GMVecCrossProd(Plane_p, Sup, Svp);
	IRIT_PT_NORMALIZE(Plane_p);
	Plane_p[3] = -IRIT_DOT_PROD(Plane_p, p);
	GMVecCrossProd(VPlane, Plane_q, Plane_p);
	IRIT_PT_NORMALIZE(VPlane);
	/* Arbitrary point on the line. */
	Pt1[0] = - (Plane_p[2] * Plane_q[3] - Plane_q[2] * Plane_p[3]) / 
	    (Plane_q[0] * Plane_p[2] - Plane_p[0] * Plane_q[2]);
	Pt1[1] = 0.0;
	Pt1[2] = - (Plane_p[0] * Plane_q[3] - Plane_q[0] * Plane_p[3]) / 
	    (Plane_q[2] * Plane_p[0] - Plane_p[2] * Plane_q[0]);

	/* Intersection point. */
	GMPointFromPointLine(q, Pt1, VPlane, Closest_q);
	GMPointFromPointLine(p, Pt1, VPlane, Closest_p);
	IRIT_PT_BLEND(IntersectPt, Closest_q, Closest_p, 0.5);

	/* Update current point. */
	IRIT_PT_SUB(V, IntersectPt, q);
	if (GMFindLinComb2Vecs(Suq, Svq, V, Duv)) {
	    NewPt1[0] = IRIT_BOUND(CurrPt1[0] + 0.05 * Duv[0], 0.0, 1.0);
	    NewPt1[1] = IRIT_BOUND(CurrPt1[1] + 0.05 * Duv[1], 0.0, 1.0);
	}

	IRIT_PT_SUB(V, IntersectPt, p);
	if (GMFindLinComb2Vecs(Sup, Svp, V, Duv)) {
	    NewPt2[0] = IRIT_BOUND(CurrPt2[0] + 0.05 * Duv[0], 0.0, 1.0);
	    NewPt2[1] = IRIT_BOUND(CurrPt2[1] + 0.05 * Duv[1], 0.0, 1.0);
	}
	
	/* Compute a new distance. */
	R = CagdSrfEval(OffSrf, NewPt1[0], NewPt1[1]);
	CagdCoerceToE3(q, &R, -1, OffSrf -> PType);
	R = CagdSrfEval(OffSrf, NewPt2[0], NewPt2[1]);
	CagdCoerceToE3(p, &R, -1, OffSrf -> PType);

	Distance = IRIT_PT_PT_DIST_SQR(p, q);
	
	if (Distance < CurrDistance) {
	    IRIT_PT_COPY(CurrPt1, NewPt1);
	    IRIT_PT_COPY(CurrPt2, NewPt2);
	    CurrDistance = Distance;
	}
	else {
	    /* One last chance. */
	    IRIT_PT_BLEND(NewPt1, NewPt1, CurrPt1, 0.5);
	    IRIT_PT_BLEND(NewPt2, NewPt2, CurrPt2, 0.5);
	    
	    /* Compute a new distance. */
	    R = CagdSrfEval(OffSrf, NewPt1[0], NewPt1[1]);
	    CagdCoerceToE3(q, &R, -1, OffSrf -> PType);
	    R = CagdSrfEval(OffSrf, NewPt2[0], NewPt2[1]);
	    CagdCoerceToE3(p, &R, -1, OffSrf -> PType);
	    Distance = IRIT_PT_PT_DIST_SQR(p, q);

	    if (Distance < CurrDistance) {
	IRIT_PT_COPY(CurrPt1, NewPt1);
	IRIT_PT_COPY(CurrPt2, NewPt2);
	CurrDistance = Distance;
	    }
	    else
	break;
	}

	Iteration++;
    }
    while (Distance > NumerTol && Iteration < MaxIteration);

    /* Free local stuffs. */
    CagdSrfFree(SrfSu);
    CagdSrfFree(SrfSv);

    if (Distance > NumerTol) {
#ifdef DEBUG_MVAR_OFFSET_NI_TRACING
	fprintf(stderr, "Failed: %f     %f     %d\n",
		Distance, OrigDistance, Iteration);
#endif /* DEBUG_MVAR_OFFSET_NI_TRACING */
	return NULL;
    }

    /* Construct a list of multivar points. */
    PtList = MvarPtNew(3);
    IRIT_PT_COPY(PtList -> Pt, CurrPt1);
    PtList -> Pnext = MvarPtNew(3);
    IRIT_PT_COPY(PtList -> Pnext -> Pt, CurrPt2);
    PtList -> Pnext -> Pnext = NULL;

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:							   	     *
*   Initialize global variables for numerical tracing.		   	     *
*									     *
* PARAMETERS:								     *
*   X, Y: Two surfaces to be traced.		     			     *
*									     *
* RETURN VALUE:							  	     *
*   void:								     *
*****************************************************************************/
static void InitGlobalVars(const CagdSrfStruct *X, const CagdSrfStruct *Y)
{
    GlblSrf1 = CagdSrfCopy(X);
    GlblSrf2 = CagdSrfCopy(Y);

    GlblSrf1u = CagdSrfDerive(GlblSrf1, CAGD_CONST_U_DIR);
    GlblSrf1v = CagdSrfDerive(GlblSrf1, CAGD_CONST_V_DIR);
    GlblSrf1uu = CagdSrfDerive(GlblSrf1u, CAGD_CONST_U_DIR);
    GlblSrf1uv = CagdSrfDerive(GlblSrf1u, CAGD_CONST_V_DIR);
    GlblSrf1vv = CagdSrfDerive(GlblSrf1v, CAGD_CONST_V_DIR);
    GlblSrf1uuu = CagdSrfDerive(GlblSrf1uu, CAGD_CONST_U_DIR);
    GlblSrf1uuv = CagdSrfDerive(GlblSrf1uu, CAGD_CONST_V_DIR);
    GlblSrf1uvv = CagdSrfDerive(GlblSrf1uv, CAGD_CONST_V_DIR);
    GlblSrf1vvv = CagdSrfDerive(GlblSrf1vv, CAGD_CONST_V_DIR);

    GlblSrf2u = CagdSrfDerive(GlblSrf2, CAGD_CONST_U_DIR);
    GlblSrf2v = CagdSrfDerive(GlblSrf2, CAGD_CONST_V_DIR);
    GlblSrf2uu = CagdSrfDerive(GlblSrf2u, CAGD_CONST_U_DIR);
    GlblSrf2uv = CagdSrfDerive(GlblSrf2u, CAGD_CONST_V_DIR);
    GlblSrf2vv = CagdSrfDerive(GlblSrf2v, CAGD_CONST_V_DIR);
    GlblSrf2uuu = CagdSrfDerive(GlblSrf2uu, CAGD_CONST_U_DIR);
    GlblSrf2uuv = CagdSrfDerive(GlblSrf2uu, CAGD_CONST_V_DIR);
    GlblSrf2uvv = CagdSrfDerive(GlblSrf2uv, CAGD_CONST_V_DIR);
    GlblSrf2vvv = CagdSrfDerive(GlblSrf2vv, CAGD_CONST_V_DIR);

    GlblUmin1 = GlblVmin1 = GlblUmin2 = GlblVmin2 = 0.0;
    GlblUmax1 = GlblVmax1 = GlblUmax2 = GlblVmax2 = 1.0;
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Free memory of global variables.					     *
*									     *
* PARAMETERS:								     *
*   None						   		     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void FreeGlobalVars(void)
{
    CagdSrfFree(GlblSrf1);
    CagdSrfFree(GlblSrf1u);
    CagdSrfFree(GlblSrf1v);
    CagdSrfFree(GlblSrf1uu);
    CagdSrfFree(GlblSrf1uv);
    CagdSrfFree(GlblSrf1vv);
    CagdSrfFree(GlblSrf1uuu);
    CagdSrfFree(GlblSrf1uuv);
    CagdSrfFree(GlblSrf1uvv);
    CagdSrfFree(GlblSrf1vvv);

    CagdSrfFree(GlblSrf2);
    CagdSrfFree(GlblSrf2u);
    CagdSrfFree(GlblSrf2v);
    CagdSrfFree(GlblSrf2uu);
    CagdSrfFree(GlblSrf2uv);
    CagdSrfFree(GlblSrf2vv);
    CagdSrfFree(GlblSrf2uuu);
    CagdSrfFree(GlblSrf2uuv);
    CagdSrfFree(GlblSrf2uvv);
    CagdSrfFree(GlblSrf2vvv);
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Numerically trace intersection curve.				     *
*									     *
* PARAMETERS:								     *
*   U1, U2:	   Two parameter points of input surfaces.   		     *
*   Epsilon:	   Epsilon for the tracing.				     *
*   LocalSelfIntr: True if this tracing is for local self-intersection.      *
*									     *
* RETURN VALUE:								     *
*   CagdCrvStruct: Traced intersection curves.				     *
*****************************************************************************/
static CagdCrvStruct *TraceIntersectionCrv(IrtRType *U1,
					   IrtRType *U2,
					   IrtRType Epsilon,
					   int LocalSelfIntr)
{
    CagdCrvStruct
        *SSICrv = NULL,
        *SSILocalCrv = NULL,
	*ptrCrv = NULL,
        *SSICrvUx = NULL,
	*ptrCrvUx = NULL,
        *SSICrvUy = NULL,
        *ptrCrvUy = NULL;
    CagdRType s;
    int Iteration = 0;

    IRIT_UV_COPY(GlblU1, U1);
    IRIT_UV_COPY(GlblU2, U2);

    while (TRUE) {
	EvalCompAtCurParam();
	IRIT_PT_BLEND(GlblR0, GlblSrf1Pt0, GlblSrf2Pt0, 0.5);

	/* Local self-intersection. */
	if (IRIT_PT_APX_EQ_E2_EPS(GlblU1, GlblU2, GlblSameUVTol)) {
	    int Length = SSICrv -> Length;

	    SSICrv -> Points[1][Length - 1] = GlblR0[0];
	    SSICrv -> Points[2][Length - 1] = GlblR0[1];
	    SSICrv -> Points[3][Length - 1] = GlblR0[2];
	    Length = SSICrvUx -> Length;
	    SSICrvUx -> Points[1][Length - 1] = GlblU1[0];
	    SSICrvUx -> Points[2][Length - 1] = GlblU1[1];
	    Length = SSICrvUy -> Length;
	    SSICrvUy -> Points[1][Length - 1] = GlblU2[0];
	    SSICrvUy -> Points[2][Length - 1] = GlblU2[1];
	    break;
	}

	GetFirstOrderComponents(FALSE);
	GetSecondOrderComponents();
	GetThirdOrderComponents();

	CagdCrvFreeList(SSILocalCrv);
	SSILocalCrv = SSIApxInterCrv();
	s = SSIStepDistance(&SSILocalCrv, Epsilon);
	if (s != 0.0) {
	    ptrCrv = CagdCrvRegionFromCrv(SSILocalCrv->Pnext->Pnext, 0.0, s);
	    ptrCrvUx = CagdCrvRegionFromCrv(SSILocalCrv->Pnext, 0.0, s);
	    ptrCrvUy = CagdCrvRegionFromCrv(SSILocalCrv, 0.0, s);
	    if (SSICrv) {
	        SSICrv = CagdMergeCrvCrv(SSICrv, ptrCrv, TRUE);
		SSICrvUx = CagdMergeCrvCrv(SSICrvUx, ptrCrvUx, TRUE);
		SSICrvUy = CagdMergeCrvCrv(SSICrvUy, ptrCrvUy, TRUE);
	    } 
	    else {
	        SSICrv = ptrCrv;
		SSICrvUx = ptrCrvUx;
		SSICrvUy = ptrCrvUy;
	    }
	} 
	else {
#	    ifdef DEBUG_MVAR_OFFSET_NI_TRACING
	        fprintf(stderr, "Step size becomes zero.\n");
#	    endif /* DEBUG_MVAR_OFFSET_NI_TRACING */
		break;
	}
	
	/* Local self-intersection. */
	if (IRIT_PT_APX_EQ_E2_EPS(GlblU1, GlblU2, GlblSameUVTol))
	    break;

	/* Loop detection. */
	if (Iteration > 5 && (IRIT_PT_APX_EQ_E2_EPS(GlblU1, U1, Epsilon * 100.0) ||
			      IRIT_PT_APX_EQ_E2_EPS(GlblU2, U2, Epsilon * 100.0)))
	    break;
	SSIPointRefine(Epsilon * 0.1);
	Iteration++;
    }

    /* Loop: return current result. */
    if (SSICrv && (IRIT_PT_APX_EQ_E2_EPS(GlblU1, U1, Epsilon * 100.0) ||
		   IRIT_PT_APX_EQ_E2_EPS(GlblU2, U2, Epsilon * 100.0))) {
	IRIT_LIST_PUSH(SSICrvUx, SSICrv);
	IRIT_LIST_PUSH(SSICrvUy, SSICrv);
	return SSICrv;
    }
    if (SSICrv) {
	SSICrv = CagdCrvReverse(SSICrv);
	SSICrvUx = CagdCrvReverse(SSICrvUx);
	SSICrvUy = CagdCrvReverse(SSICrvUy);
    }
    
    IRIT_UV_COPY(GlblU1, U1);
    IRIT_UV_COPY(GlblU2, U2);

    while (TRUE) {
	EvalCompAtCurParam();
	IRIT_PT_BLEND(GlblR0, GlblSrf1Pt0, GlblSrf2Pt0, 0.5);

	/* Local self-intersection. */
	if (IRIT_PT_APX_EQ_E2_EPS(GlblU1, GlblU2, GlblSameUVTol)) {
	    int Length = SSICrv -> Length;
	    
	    SSICrv -> Points[1][Length - 1] = GlblR0[0];
	    SSICrv -> Points[2][Length - 1] = GlblR0[1];
	    SSICrv -> Points[3][Length - 1] = GlblR0[2];
	    Length = SSICrvUx -> Length;
	    SSICrvUx -> Points[1][Length - 1] = GlblU1[0];
	    SSICrvUx -> Points[2][Length - 1] = GlblU1[1];
	    Length = SSICrvUy -> Length;
	    SSICrvUy -> Points[1][Length - 1] = GlblU2[0];
	    SSICrvUy -> Points[2][Length - 1] = GlblU2[1];

	    break;
	}

	GetFirstOrderComponents(TRUE);
	GetSecondOrderComponents();
	GetThirdOrderComponents();
	
	CagdCrvFreeList(SSILocalCrv);
	SSILocalCrv = SSIApxInterCrv();
	s = SSIStepDistance(&SSILocalCrv, Epsilon);
	if (s != 0.0) {
	    ptrCrv = CagdCrvRegionFromCrv(SSILocalCrv->Pnext->Pnext, 0.0, s);
	    ptrCrvUx = CagdCrvRegionFromCrv(SSILocalCrv->Pnext, 0.0, s);
	    ptrCrvUy = CagdCrvRegionFromCrv(SSILocalCrv, 0.0, s);
	    if (SSICrv) {
	        SSICrv = CagdMergeCrvCrv(SSICrv, ptrCrv, TRUE);
		SSICrvUx = CagdMergeCrvCrv(SSICrvUx, ptrCrvUx, TRUE);
		SSICrvUy = CagdMergeCrvCrv(SSICrvUy, ptrCrvUy, TRUE);
	    } 
	    else {
	        SSICrv = ptrCrv;
		SSICrvUx = ptrCrvUx;
		SSICrvUy = ptrCrvUy;
	    }
	}
	else 
	    break;

	if (IRIT_PT_APX_EQ_E2_EPS(GlblU1, GlblU2, GlblSameUVTol)) 
	    break;
	SSIPointRefine(Epsilon / 100.0);
    }

    if (SSICrv) {
	IRIT_LIST_PUSH(SSICrvUx, SSICrv);
	IRIT_LIST_PUSH(SSICrvUy, SSICrv);
	return SSICrv;
    }
    
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:							 	     *
*   Computes first order components for numerical tracing.		     *
*									     *
* PARAMETERS:								     *
*   Reverse: True if this tracing is for reversed order.		     *
*									     *
* RETURN VALUE:							 	     *
*   void								     *
*****************************************************************************/
static void GetFirstOrderComponents(CagdBType Reverse)
{ 
    CagdRType E, F, G, Det;

    IRIT_CROSS_PROD(GlblTangent, GlblNx, GlblNy);
    IRIT_PT_NORMALIZE(GlblTangent);

    if (Reverse) { 
	IRIT_PT_SCALE(GlblTangent, -1.0);
    }

    /* u', v' */
    E = IRIT_DOT_PROD(GlblXu, GlblXu);
    F = IRIT_DOT_PROD(GlblXu, GlblXv);
    G = IRIT_DOT_PROD(GlblXv, GlblXv);
    Det = E*G - F*F;
    GlblU1p[0] = (G * IRIT_DOT_PROD(GlblXu, GlblTangent) -
		  F * IRIT_DOT_PROD(GlblXv, GlblTangent)) / Det;
    GlblU1p[1] = (E * IRIT_DOT_PROD(GlblXv, GlblTangent) -
		  F * IRIT_DOT_PROD(GlblXu, GlblTangent)) / Det;

    E = IRIT_DOT_PROD(GlblYu, GlblYu);
    F = IRIT_DOT_PROD(GlblYu, GlblYv);
    G = IRIT_DOT_PROD(GlblYv, GlblYv);
    Det = E*G - F*F;
    GlblU2p[0] = (G * IRIT_DOT_PROD(GlblYu, GlblTangent) -
		  F * IRIT_DOT_PROD(GlblYv, GlblTangent)) / Det;
    GlblU2p[1] = (E * IRIT_DOT_PROD(GlblYv, GlblTangent) -
		  F * IRIT_DOT_PROD(GlblYu, GlblTangent)) / Det;

#   ifdef DEBUG_MVAR_OFFSET_NI_TRACING
        fprintf(stderr, "Tangent    : (%f, %f, %f)\n", 
		GlblTangent[0], GlblTangent[1], GlblTangent[2]);
#   endif /* DEBUG_MVAR_OFFSET_NI_TRACING */
}


/*****************************************************************************
* DESCRIPTION:							 	     *
*   Computes second order components for numerical tracing.		     *
*									     *
* PARAMETERS:								     *
*   None								     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void GetSecondOrderComponents(void)
{
    CagdVType PX, PY, V;
    CagdRType mx, my, E, F, G, Det;

    /* PX = Xuu u' u' + 2 Xuv u' v' + Xvv v' v'. */
    PX[0] = GlblXuu[0] * IRIT_SQR(GlblU1p[0]) +
            2.0 * GlblXuv[0] * GlblU1p[0] * GlblU1p[1] +
            GlblXvv[0] * IRIT_SQR(GlblU1p[1]);
    PX[1] = GlblXuu[1] * IRIT_SQR(GlblU1p[0]) +
            2.0 * GlblXuv[1] * GlblU1p[0] * GlblU1p[1] +
            GlblXvv[1] * IRIT_SQR(GlblU1p[1]);
    PX[2] = GlblXuu[2] * IRIT_SQR(GlblU1p[0]) +
            2.0 * GlblXuv[2] * GlblU1p[0] * GlblU1p[1] +
             GlblXvv[2] * IRIT_SQR(GlblU1p[1]);

    /* PY = Yuu u' u' + 2 Yuv u' v' + Yvv v' v'. */
    PY[0] = GlblYuu[0] * IRIT_SQR(GlblU2p[0]) +
            2.0 * GlblYuv[0] * GlblU2p[0] * GlblU2p[1] +
            GlblYvv[0] * IRIT_SQR(GlblU2p[1]);
    PY[1] = GlblYuu[1] * IRIT_SQR(GlblU2p[0]) +
            2.0 * GlblYuv[1] * GlblU2p[0] * GlblU2p[1] +
            GlblYvv[1] * IRIT_SQR(GlblU2p[1]);
    PY[2] = GlblYuu[2] * IRIT_SQR(GlblU2p[0]) +
            2.0 * GlblYuv[2] * GlblU2p[0] * GlblU2p[1] +
            GlblYvv[2] * IRIT_SQR(GlblU2p[1]);

    Det = IRIT_DOT_PROD(GlblNx,GlblNx) * 
          IRIT_DOT_PROD(GlblNy,GlblNy) - IRIT_SQR(IRIT_DOT_PROD(GlblNx,GlblNy));
    mx = (IRIT_DOT_PROD(PX,GlblNx) * IRIT_DOT_PROD(GlblNy,GlblNy)
	- IRIT_DOT_PROD(PY,GlblNy) * IRIT_DOT_PROD(GlblNx,GlblNy))/Det;
    my = (IRIT_DOT_PROD(PY,GlblNy) * IRIT_DOT_PROD(GlblNx,GlblNx)
	- IRIT_DOT_PROD(PX,GlblNx) * IRIT_DOT_PROD(GlblNy,GlblNx))/Det;

    GlblNormal[0] = mx * GlblNx[0] + my * GlblNy[0];
    GlblNormal[1] = mx * GlblNx[1] + my * GlblNy[1];
    GlblNormal[2] = mx * GlblNx[2] + my * GlblNy[2];
    GlblKappa = IRIT_PT_LENGTH(GlblNormal);
    IRIT_PT_NORMALIZE(GlblNormal);

    /* u'', v'', */
    E = IRIT_DOT_PROD(GlblXu, GlblXu);
    F = IRIT_DOT_PROD(GlblXu, GlblXv);
    G = IRIT_DOT_PROD(GlblXv, GlblXv);
    Det = E*G - F*F;
    IRIT_PT_COPY(V, GlblNormal);
    IRIT_PT_SCALE(V, GlblKappa);
    IRIT_PT_SUB(V, V, PX);		     /* V = kappa*Normal - PhiX. */
    GlblU1pp[0] = (G*IRIT_DOT_PROD(GlblXu, V) - F*IRIT_DOT_PROD(GlblXv, V))/Det;
    GlblU1pp[1] = (E*IRIT_DOT_PROD(GlblXv, V) - F*IRIT_DOT_PROD(GlblXu, V))/Det;

    E = IRIT_DOT_PROD(GlblYu, GlblYu);
    F = IRIT_DOT_PROD(GlblYu, GlblYv);
    G = IRIT_DOT_PROD(GlblYv, GlblYv);
    Det = E*G - F*F;
    IRIT_PT_COPY(V, GlblNormal);
    IRIT_PT_SCALE(V, GlblKappa);
    IRIT_PT_SUB(V, V, PY);		     /* V = kappa*Normal - PhiY. */
    GlblU2pp[0] = (G*IRIT_DOT_PROD(GlblYu, V) - F*IRIT_DOT_PROD(GlblYv, V))/Det;
    GlblU2pp[1] = (E*IRIT_DOT_PROD(GlblYv, V) - F*IRIT_DOT_PROD(GlblYu, V))/Det;

    /* Now compute BiNormal vector. */
    if (GlblKappa != 0.0) {
	IRIT_CROSS_PROD(GlblBiNormal, GlblTangent, GlblNormal);
	IRIT_PT_NORMALIZE(GlblBiNormal);
    } 
    else {
	IRIT_PT_RESET(GlblNormal);
	IRIT_PT_RESET(GlblBiNormal);
    }

#   ifdef DEBUG_MVAR_OFFSET_NI_TRACING
        fprintf(stderr, "Normal   : (%f, %f, %f)\n",
		GlblNormal[0], GlblNormal[1], GlblNormal[2]);
        fprintf(stderr, "BiNormal : (%f, %f, %f)\n",
		GlblBiNormal[0], GlblBiNormal[1], GlblBiNormal[2]);
#   endif/* DEBUG_MVAR_OFFSET_NI_TRACING */
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Computes third order components for numerical tracing.		     *
*									     *
* PARAMETERS:								     *
*   None								     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void GetThirdOrderComponents(void)
{
    CagdVType PX, PY, V;
    CagdRType E, F, G, Det;

    IRIT_PT_COPY(PX, GlblXuuu);
    IRIT_PT_SCALE(PX, GlblU1p[0]*GlblU1p[0]*GlblU1p[0]);
    IRIT_PT_COPY(V, GlblXuuv);
    IRIT_PT_SCALE(V, 3.0*GlblU1p[0]*GlblU1p[0]*GlblU1p[1]);
    IRIT_PT_ADD(PX, PX, V);
    IRIT_PT_COPY(V, GlblXuvv);
    IRIT_PT_SCALE(V, 3.0*GlblU1p[0]*GlblU1p[1]*GlblU1p[1]);
    IRIT_PT_ADD(PX, PX, V);
    IRIT_PT_COPY(V, GlblXvvv);
    IRIT_PT_SCALE(V, GlblU1p[1]*GlblU1p[1]*GlblU1p[1]);
    IRIT_PT_ADD(PX, PX, V);
    IRIT_PT_COPY(V, GlblXuu);
    IRIT_PT_SCALE(V, 3.0*GlblU1pp[0]*GlblU1p[0]);
    IRIT_PT_ADD(PX, PX, V);
    IRIT_PT_COPY(V, GlblXuv);
    IRIT_PT_SCALE(V, 3.0*(GlblU1pp[0]*GlblU1p[1] + GlblU1p[0]*GlblU1pp[1]));
    IRIT_PT_ADD(PX, PX, V);
    IRIT_PT_COPY(V, GlblXvv);
    IRIT_PT_SCALE(V, 3.0*GlblU1pp[1]*GlblU1p[1]);
    IRIT_PT_ADD(PX, PX, V);

    IRIT_PT_COPY(PY, GlblYuuu);
    IRIT_PT_SCALE(PY, GlblU2p[0]*GlblU2p[0]*GlblU2p[0]);
    IRIT_PT_COPY(V, GlblYuuv);
    IRIT_PT_SCALE(V, 3.0*GlblU2p[0]*GlblU2p[0]*GlblU2p[1]);
    IRIT_PT_ADD(PY, PY, V);
    IRIT_PT_COPY(V, GlblYuvv);
    IRIT_PT_SCALE(V, 3.0*GlblU2p[0]*GlblU2p[1]*GlblU2p[1]);
    IRIT_PT_ADD(PY, PY, V);
    IRIT_PT_COPY(V, GlblYvvv);
    IRIT_PT_SCALE(V, GlblU2p[1]*GlblU2p[1]*GlblU2p[1]);
    IRIT_PT_ADD(PY, PY, V);
    IRIT_PT_COPY(V, GlblYuu);
    IRIT_PT_SCALE(V, 3.0*GlblU2pp[0]*GlblU2p[0]);
    IRIT_PT_ADD(PY, PY, V);
    IRIT_PT_COPY(V, GlblYuv);
    IRIT_PT_SCALE(V, 3.0*(GlblU2pp[0]*GlblU2p[1] + GlblU2p[0]*GlblU2pp[1]));
    IRIT_PT_ADD(PY, PY, V);
    IRIT_PT_COPY(V, GlblYvv);
    IRIT_PT_SCALE(V, 3.0*GlblU2pp[1]*GlblU2p[1]);
    IRIT_PT_ADD(PY, PY, V);

    /* kappa_p, tau. */
    Det = GlblKappa * (IRIT_DOT_PROD(GlblNormal, GlblNx) *
		       IRIT_DOT_PROD(GlblBiNormal, GlblNy) -
		       IRIT_DOT_PROD(GlblNormal, GlblNy) *
		       IRIT_DOT_PROD(GlblBiNormal, GlblNx));
    if (Det == 0.0) {
	GlblKappa_p = 0.0;
	GlblTau = 0.0;
    } 
    else {
	GlblKappa_p = GlblKappa * (IRIT_DOT_PROD(PX, GlblNx) *
				   IRIT_DOT_PROD(GlblBiNormal, GlblNy) -
				   IRIT_DOT_PROD(PY, GlblNy) *
				   IRIT_DOT_PROD(GlblBiNormal, GlblNx)) / Det;
	GlblTau = (IRIT_DOT_PROD(PX, GlblNx) *\
		   IRIT_DOT_PROD(GlblNormal, GlblNy) -
		   IRIT_DOT_PROD(PY, GlblNy) *
		   IRIT_DOT_PROD(GlblNormal, GlblNx)) / Det;
    }

    /* u''', v'''. */
    E = IRIT_DOT_PROD(GlblXu, GlblXu);
    F = IRIT_DOT_PROD(GlblXu, GlblXv);
    G = IRIT_DOT_PROD(GlblXv, GlblXv);
    Det = E * G - F * F;
    V[0] = GlblKappa_p * GlblNormal[0] - 
           IRIT_SQR(GlblKappa) * GlblTangent[0] - 
           GlblKappa * GlblTau * GlblBiNormal[0] - PX[0];
    V[1] = GlblKappa_p * GlblNormal[1] - 
           IRIT_SQR(GlblKappa) * GlblTangent[1] - 
           GlblKappa * GlblTau * GlblBiNormal[1] - PX[1];
    V[2] = GlblKappa_p * GlblNormal[2] - 
           IRIT_SQR(GlblKappa) * GlblTangent[2] - 
           GlblKappa * GlblTau * GlblBiNormal[2] - PX[2];
    GlblU1ppp[0] = (G * IRIT_DOT_PROD(GlblXu, V) - F * IRIT_DOT_PROD(GlblXv, V)) / Det;
    GlblU1ppp[1] = (E * IRIT_DOT_PROD(GlblXv, V) - F * IRIT_DOT_PROD(GlblXu, V)) / Det;

    E = IRIT_DOT_PROD(GlblYu, GlblYu);
    F = IRIT_DOT_PROD(GlblYu, GlblYv);
    G = IRIT_DOT_PROD(GlblYv, GlblYv);
    Det = E * G - F * F;
    V[0] = GlblKappa_p * GlblNormal[0] -
           IRIT_SQR(GlblKappa) * GlblTangent[0] -
           GlblKappa * GlblTau * GlblBiNormal[0] - PY[0];
    V[1] = GlblKappa_p * GlblNormal[1] -
           IRIT_SQR(GlblKappa) * GlblTangent[1] -
           GlblKappa * GlblTau * GlblBiNormal[1] - PY[1];
    V[2] = GlblKappa_p * GlblNormal[2] -
           IRIT_SQR(GlblKappa) * GlblTangent[2] -
           GlblKappa * GlblTau * GlblBiNormal[2] - PY[2];
    GlblU2ppp[0] = (G * IRIT_DOT_PROD(GlblYu, V) - F * IRIT_DOT_PROD(GlblYv, V)) / Det;
    GlblU2ppp[1] = (E * IRIT_DOT_PROD(GlblYv, V) - F * IRIT_DOT_PROD(GlblYu, V)) / Det;

#   ifdef DEBUG_MVAR_OFFSET_NI_TRACING 
        fprintf(stderr, "kappa   = %f\n", GlblKappa);
        fprintf(stderr, "kappa_p = %f\n", GlblKappa_p);
        fprintf(stderr, "tau     = %f\n\n", GlblTau);
#   endif /* DEBUG_MVAR_OFFSET_NI_TRACING */
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Approximate intersection curve at current point.			     *
*									     *
* PARAMETERS:								     *
*   void								     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static CagdCrvStruct *SSIApxInterCrv(void)
{
    CagdCrvStruct 
	*CrvR3 = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E3_TYPE, 4),
	*CrvUx = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E2_TYPE, 4),
	*CrvUy = CagdCrvNew(CAGD_CPOWER_TYPE, CAGD_PT_E2_TYPE, 4),
	*RtnCrv = NULL,
	*CrvBzr = NULL,
	*CrvBsp = NULL;
    CagdRType **Points, *XPts, *YPts, *ZPts;

    /* Intersection curve. */
    Points = CrvR3->Points;
    XPts = Points[1];
    YPts = Points[2];
    ZPts = Points[3];

    XPts[0] = GlblR0[0];
    XPts[1] = GlblTangent[0];
    XPts[2] = GlblKappa * GlblNormal[0] / 2.0;
    XPts[3] = (GlblKappa_p * GlblNormal[0] -
	       IRIT_SQR(GlblKappa) * GlblTangent[0] -
	       GlblKappa * GlblTau * GlblBiNormal[0]) / 6.0;

    YPts[0] = GlblR0[1];
    YPts[1] = GlblTangent[1];
    YPts[2] = GlblKappa * GlblNormal[1] / 2.0;
    YPts[3] = (GlblKappa_p * GlblNormal[1] -
	       IRIT_SQR(GlblKappa) * GlblTangent[1] -
	       GlblKappa * GlblTau * GlblBiNormal[1]) / 6.0;

    ZPts[0] = GlblR0[2];
    ZPts[1] = GlblTangent[2];
    ZPts[2] = GlblKappa * GlblNormal[2] / 2.0;
    ZPts[3] = (GlblKappa_p * GlblNormal[2] -
	       IRIT_SQR(GlblKappa) * GlblTangent[2] -
	       GlblKappa * GlblTau * GlblBiNormal[2]) / 6.0;

    /* Inverse image of intersection curve on parameter domain of X. */
    Points = CrvUx->Points;
    XPts = Points[1];
    YPts = Points[2];

    XPts[0] = GlblU1[0];
    XPts[1] = GlblU1p[0];
    XPts[2] = GlblU1pp[0] / 2.0;
    XPts[3] = GlblU1ppp[0] / 6.0;

    YPts[0] = GlblU1[1];
    YPts[1] = GlblU1p[1];
    YPts[2] = GlblU1pp[1] / 2.0;
    YPts[3] = GlblU1ppp[1] / 6.0;

    /* Inverse image of intersection curve on parameter domain of Y. */
    Points = CrvUy->Points;
    XPts = Points[1];
    YPts = Points[2];

    XPts[0] = GlblU2[0];
    XPts[1] = GlblU2p[0];
    XPts[2] = GlblU2pp[0] / 2.0;
    XPts[3] = GlblU2ppp[0] / 6.0;

    YPts[0] = GlblU2[1];
    YPts[1] = GlblU2p[1];
    YPts[2] = GlblU2pp[1] / 2.0;
    YPts[3] = GlblU2ppp[1] / 6.0;

    CrvBzr = CagdCnvrtPwr2BzrCrv(CrvR3);
    CagdCrvFree(CrvR3);
    CrvBsp = CagdCnvrtBzr2BspCrv(CrvBzr);
    CagdCrvFree(CrvBzr);
    IRIT_LIST_PUSH(CrvBsp, RtnCrv);

    CrvBzr = CagdCnvrtPwr2BzrCrv(CrvUx);
    CagdCrvFree(CrvUx);
    CrvBsp = CagdCnvrtBzr2BspCrv(CrvBzr);
    CagdCrvFree(CrvBzr);
    IRIT_LIST_PUSH(CrvBsp, RtnCrv);

    CrvBzr = CagdCnvrtPwr2BzrCrv(CrvUy);
    CagdCrvFree(CrvUy);
    CrvBsp = CagdCnvrtBzr2BspCrv(CrvBzr);
    CagdCrvFree(CrvBzr);
    IRIT_LIST_PUSH(CrvBsp, RtnCrv);
    
    return RtnCrv;
}

/*****************************************************************************
* DESCRIPTION:							 	     *
*   Computes a small step distance that is safe.			     *
*									     *
* PARAMETERS:								     *
*   C:	Approximated curves.						     *
*   Epsilon:  Tolerance.						     *
*									     *
* RETURN VALUE:							 	     *
*   IrtRType: Small step distance.					     *
*****************************************************************************/
static IrtRType SSIStepDistance(CagdCrvStruct **C, IrtRType Epsilon)
{
    CagdCrvStruct
	*CrvR3 = NULL,
        *CrvUx = NULL,
        *CrvUy = NULL;
    CagdPType P0, Px, Py;
    CagdUVType uvx, uvy;
    CagdRType
	*P = NULL,
        s = 0.0,
        ds = 0.1;
    CagdRType Txmin, Txmax, Tymin, Tymax, T3min, T3max;

    if (*C == NULL) 
	return 0.0;

    CrvUy = CagdCrvCopy(*C);
    CrvUx = CagdCrvCopy((*C)->Pnext);
    CrvR3 = CagdCrvCopy((*C)->Pnext->Pnext);

    CagdCrvDomain(CrvR3, &T3min, &T3max);
    CagdCrvDomain(CrvUx, &Txmin, &Txmax);
    CagdCrvDomain(CrvUy, &Tymin, &Tymax);

    /* First check the intersection curve direction by Epsilon ball test. */
    P = CagdCrvEval(CrvUx, Epsilon * 0.1);
    MVAR_OFST_NI_E2_TO_UV(P, uvx);
    if (uvx[0] <= GlblUmin1 || uvx[0] >= GlblUmax1 ||
	uvx[1] <= GlblVmin1 || uvx[1] >= GlblVmax1)
	return 0.0;

    P = CagdCrvEval(CrvUy, Epsilon * 0.1);
    MVAR_OFST_NI_E2_TO_UV(P, uvy);
    if (uvy[0] <= GlblUmin2 || uvy[0] >= GlblUmax2 ||
	uvy[1] <= GlblVmin2 || uvy[1] >= GlblVmax2)
	return 0.0;

    /* Determine the safe step distance. */
    while (TRUE) {
	if (s + ds >= Txmin && s + ds <= Txmax &&
	    s + ds >= Tymin && s + ds <= Tymax) {
	    P = CagdCrvEval(CrvUx, s + ds);
	    MVAR_OFST_NI_E2_TO_UV(P, uvx);
	    if (uvx[0] < GlblUmin1 || uvx[0] > GlblUmax1 ||
		uvx[1] < GlblVmin1 || uvx[1] > GlblVmax1) {
	        if (ds < Epsilon * 0.1) 
		    ds = 0.0;
		else {
		    ds /= 2.0;
		    continue;
		}
	    }
	    
	    P = CagdCrvEval(CrvUy, s + ds);
	    MVAR_OFST_NI_E2_TO_UV(P, uvy);
	    if (uvy[0] < GlblUmin2 || uvy[0] > GlblUmax2 ||
		uvy[1] < GlblVmin2 || uvy[1] > GlblVmax2) {
	        if (ds < Epsilon * 0.1) 
		    ds = 0.0;
		else {
		    ds /= 2.0;
		    continue;
		}
	    }
	} 
	else 
	    ds = 0.0;
	
	if (ds < Epsilon / 100.0) {
	    P = CagdCrvEval(CrvUx, s);
	    MVAR_OFST_NI_E2_TO_UV(P, GlblU1);
	    P = CagdSrfEval(GlblSrf1, GlblU1[0], GlblU1[1]);
	    MVAR_OFST_NI_E3_TO_PT(P, GlblSrf1Pt0);
	    
	    P = CagdCrvEval(CrvUy, s);
	    MVAR_OFST_NI_E2_TO_UV(P, GlblU2);
	    P = CagdSrfEval(GlblSrf2, GlblU2[0], GlblU2[1]);
	    MVAR_OFST_NI_E3_TO_PT(P, GlblSrf2Pt0);

	    P = CagdCrvEval(CrvR3, s);
	    MVAR_OFST_NI_E3_TO_PT(P, GlblR0);
	    
#	    ifdef DEBUG_MVAR_OFFSET_NI_TRACING
	        fprintf(stderr, "C : %f %f %f\n",
			GlblR0[0], GlblR0[1], GlblR0[2]);
	        fprintf(stderr, "X : %f %f %f\n",
			GlblSrf1Pt0[0], GlblSrf1Pt0[1], GlblSrf1Pt0[2]);
	        fprintf(stderr, "Y : %f %f %f\n",
			GlblSrf2Pt0[0], GlblSrf2Pt0[1], GlblSrf2Pt0[2]);
	        fprintf(stderr, "Dist : %f %f %f\n",
			IRIT_PT_PT_DIST(GlblR0, GlblSrf1Pt0),
			IRIT_PT_PT_DIST(GlblR0, GlblSrf2Pt0),
			IRIT_PT_PT_DIST(GlblSrf1Pt0, GlblSrf2Pt0));
#	    endif /* DEBUG_MVAR_OFFSET_NI_TRACING */
	    
	    if (IRIT_APX_EQ_EPS(GlblU1[0], GlblUmin1, Epsilon))
	        GlblU1[0] = GlblUmin1;
	    if (IRIT_APX_EQ_EPS(GlblU1[0], GlblUmax1, Epsilon))
	        GlblU1[0] = GlblUmax1;
	    if (IRIT_APX_EQ_EPS(GlblU1[1], GlblVmin1, Epsilon))
	        GlblU1[1] = GlblVmin1;
	    if (IRIT_APX_EQ_EPS(GlblU1[1], GlblVmax1, Epsilon))
	        GlblU1[1] = GlblVmax1;
	    if (IRIT_APX_EQ_EPS(GlblU2[0], GlblUmin2, Epsilon))
	        GlblU2[0] = GlblUmin2;
	    if (IRIT_APX_EQ_EPS(GlblU2[0], GlblUmax2, Epsilon))
	        GlblU2[0] = GlblUmax2;
	    if (IRIT_APX_EQ_EPS(GlblU2[1], GlblVmin2, Epsilon))
	        GlblU2[1] = GlblVmin2;
	    if (IRIT_APX_EQ_EPS(GlblU2[1], GlblVmax2, Epsilon))
	        GlblU2[1] = GlblVmax2;
	    
	    return s;
	} 
	else {
	    P = CagdCrvEval(CrvR3, s + ds);
	    MVAR_OFST_NI_E3_TO_PT(P, P0);
	    P = CagdSrfEval(GlblSrf1, uvx[0], uvx[1]);
	    MVAR_OFST_NI_E3_TO_PT(P, Px);
	    P = CagdSrfEval(GlblSrf2, uvy[0], uvy[1]);
	    MVAR_OFST_NI_E3_TO_PT(P, Py);
	    if (IRIT_PT_PT_DIST(P0, Px) < Epsilon && 
		IRIT_PT_PT_DIST(P0, Py) < Epsilon &&
		IRIT_PT_PT_DIST(Px, Py) < Epsilon)
	        s += ds;
	    else
	        ds *= 0.5;
	} 
    }

    /* Never reaches here. */
    return s;
}

/*****************************************************************************
* DESCRIPTION:								     *
 *   Numerically improve current point into a real intersection point.	     *
*									     *
* PARAMETERS:								     *
*   Epsilon: Accuracy for the refinement.				     *
*									     *
* RETURN VALUE:								     *
*   CagdBType: True if succeed.						     *
*****************************************************************************/
static CagdBType SSIPointRefine(IrtRType Epsilon)
{
    CagdVType xu, xv, yu, yv, nx, ny, dx, dy;
    CagdPType Px, Py, P0;
    CagdUVType ux, uy, uxOld, uyOld;
    CagdRType E, F, G, Det, m1, m2,
	*P = NULL;
    int Counter = 0;

    IRIT_UV_COPY(ux, GlblU1); IRIT_UV_COPY(uxOld, ux);
    IRIT_UV_COPY(uy, GlblU2); IRIT_UV_COPY(uyOld, uy);
    
    /* Get three points, Px, Py, P0. */
    IRIT_PT_COPY(Px, GlblSrf1Pt0);
    IRIT_PT_COPY(Py, GlblSrf2Pt0);
    IRIT_PT_COPY(P0, GlblR0);

    while (TRUE) {
	/* Get xu, xv, nx. */
	P = CagdSrfEval(GlblSrf1u, ux[0], ux[1]);
	MVAR_OFST_NI_E3_TO_VEC(P, xu);
	P = CagdSrfEval(GlblSrf1v, ux[0], ux[1]);
	MVAR_OFST_NI_E3_TO_VEC(P, xv);
	IRIT_CROSS_PROD(nx, xu, xv);
	
	/* Get yu, yv, ny. */
	P = CagdSrfEval(GlblSrf2u, uy[0], uy[1]);
	MVAR_OFST_NI_E3_TO_VEC(P, yu);
	P = CagdSrfEval(GlblSrf2v, uy[0], uy[1]);
	MVAR_OFST_NI_E3_TO_VEC(P, yv);
	IRIT_CROSS_PROD(ny, yu, yv);
	
	IRIT_PT_SUB(dx, Px, P0);
	IRIT_PT_SUB(dy, Py, P0);
	
	/* Update P0. */
	Det = IRIT_DOT_PROD(nx, nx) * IRIT_DOT_PROD(ny, ny) -
				              IRIT_SQR(IRIT_DOT_PROD(nx, ny));
	m1 = (IRIT_DOT_PROD(nx, dx) * IRIT_DOT_PROD(ny, ny) - 
	    IRIT_DOT_PROD(nx, ny) * IRIT_DOT_PROD(ny, dy)) / Det;
	m2 = (IRIT_DOT_PROD(ny, dy) * IRIT_DOT_PROD(nx, nx) -
	    IRIT_DOT_PROD(nx, dx) * IRIT_DOT_PROD(nx, ny)) / Det;
	P0[0] += m1*nx[0] + m2*ny[0];
	P0[1] += m1*nx[1] + m2*ny[1];
	P0[2] += m1*nx[2] + m2*ny[2];
	
	/* Update Ux. */
	IRIT_PT_SUB(dx, P0, Px);
	E = IRIT_DOT_PROD(xu, xu);
	F = IRIT_DOT_PROD(xu, xv);
	G = IRIT_DOT_PROD(xv, xv);
	Det = E*G-F*F;
	m1 = (G*IRIT_DOT_PROD(xu, dx) - F*IRIT_DOT_PROD(xv, dx))/Det;
	m2 = (E*IRIT_DOT_PROD(xv, dx) - F*IRIT_DOT_PROD(xu, dx))/Det;
	ux[0] += m1;
	ux[1] += m2;

	/* Update Uy. */
	IRIT_PT_SUB(dy, P0, Py);
	E = IRIT_DOT_PROD(yu, yu);
	F = IRIT_DOT_PROD(yu, yv);
	G = IRIT_DOT_PROD(yv, yv);
	Det = E*G-F*F;
	m1 = (G*IRIT_DOT_PROD(yu, dy) - F*IRIT_DOT_PROD(yv, dy))/Det;
	m2 = (E*IRIT_DOT_PROD(yv, dy) - F*IRIT_DOT_PROD(yu, dy))/Det;
	uy[0] += m1;
	uy[1] += m2;

	/* Check the domain boundary. */
	if (ux[0] < GlblUmin1) {
	    ux[1] = uxOld[1] + (ux[1] - uxOld[1]) / (ux[0] - uxOld[0]) *
	                                            (ux[0] - GlblUmin1);
	    ux[0] = GlblUmin1;
	}
	if (ux[0] > GlblUmax1) {
	    ux[1] = uxOld[1] + (ux[1] - uxOld[1]) / (ux[0] - uxOld[0]) *
						    (ux[0] - GlblUmax1);
	    ux[0] = GlblUmax1;
	}
	if (ux[1] < GlblVmin1) {
	    ux[0] = uxOld[0] + (ux[0] - uxOld[0]) / (ux[1] - uxOld[1]) *
						    (ux[1] - GlblVmin1);
	    ux[1] = GlblVmin1;
	}
	if (ux[1] > GlblVmax1) {
	    ux[0] = uxOld[0] + (ux[0] - uxOld[0]) / (ux[1] - uxOld[1]) *
						    (ux[1] - GlblVmax1);
	    ux[1] = GlblVmax1;
	}
	
	if (uy[0] < GlblUmin2) {
	    uy[1] = uyOld[1] + (uy[1] - uyOld[1]) / (uy[0] - uyOld[0]) *
						    (uy[0] - GlblUmin2);
	    uy[0] = GlblUmin2;
	}
	if (uy[0] > GlblUmax2) {
	    uy[1] = uyOld[1] + (uy[1] - uyOld[1]) / (uy[0] - uyOld[0]) *
						    (uy[0] - GlblUmax2);
	    uy[0] = GlblUmax2;
	}
	if (uy[1] < GlblVmin2) {
	    uy[0] = uyOld[0] + (uy[0] - uyOld[0]) / (uy[1] - uyOld[1]) *
						    (uy[1] - GlblVmin2);
	    uy[1] = GlblVmin2;
	}
	if (uy[1] > GlblVmax2) {
	    uy[0] = uyOld[0] + (uy[0] - uyOld[0]) / (uy[1] - uyOld[1]) *
						    (uy[1] - GlblVmax2);
	    uy[1] = GlblVmax2;
	}
	
	P = CagdSrfEval(GlblSrf1, ux[0], ux[1]);
	MVAR_OFST_NI_E3_TO_PT(P, Px);
	P = CagdSrfEval(GlblSrf2, uy[0], uy[1]);
	MVAR_OFST_NI_E3_TO_PT(P, Py);
	
	if (IRIT_PT_PT_DIST(Px, Py) < Epsilon &&
	    IRIT_PT_PT_DIST(P0, Px) < Epsilon &&
	    IRIT_PT_PT_DIST(P0, Py) < Epsilon) {
	    IRIT_UV_COPY(GlblU1, ux);
	    IRIT_UV_COPY(GlblU2, uy);
	    IRIT_PT_COPY(GlblR0, P0);
	    IRIT_PT_COPY(GlblSrf1Pt0, Px);
	    IRIT_PT_COPY(GlblSrf2Pt0, Py);
	    
#	    ifdef DEBUG_MVAR_OFFSET_NI_TRACING
	        fprintf(stderr, "Refinement step %d\n", Counter);
	        fprintf(stderr, "X(%f, %f) = (%f, %f, %f)\n",
			GlblU1[0], GlblU1[1], GlblSrf1Pt0[0],
			GlblSrf1Pt0[1], GlblSrf1Pt0[2]);
	        fprintf(stderr, "Y(%f, %f) = (%f, %f, %f)\n",
			GlblU2[0], GlblU2[1], GlblSrf2Pt0[0],
			GlblSrf2Pt0[1], GlblSrf2Pt0[2]);
	        fprintf(stderr, "Distances : %f, %f, %f\n\n",
			IRIT_PT_PT_DIST(GlblR0, GlblSrf1Pt0),
			IRIT_PT_PT_DIST(GlblR0, GlblSrf2Pt0),
			IRIT_PT_PT_DIST(GlblSrf1Pt0, GlblSrf2Pt0));
#	    endif /* DEBUG_MVAR_OFFSET_NI_TRACING */

	    return TRUE;
	} 
	else if (Counter > 40) {
	    IRIT_UV_COPY(GlblU1, ux);
	    IRIT_UV_COPY(GlblU2, uy);
	    IRIT_PT_COPY(GlblR0, P0);
	    IRIT_PT_COPY(GlblSrf1Pt0, Px);
	    IRIT_PT_COPY(GlblSrf2Pt0, Py);

#	    ifdef DEBUG_MVAR_OFFSET_NI_TRACING
	        fprintf(stderr, "Refinement step expired at iteration %d\n",
			Counter);
	        fprintf(stderr, "X(%f, %f) = (%f, %f, %f)\n",
			GlblU1[0], GlblU1[1], GlblSrf1Pt0[0],
			GlblSrf1Pt0[1], GlblSrf1Pt0[2]);
	        fprintf(stderr, "Y(%f, %f) = (%f, %f, %f)\n",
			GlblU2[0], GlblU2[1], GlblSrf2Pt0[0],
			GlblSrf2Pt0[1], GlblSrf2Pt0[2]);
	        fprintf(stderr, "Distances : %f, %f, %f\n\n",
			IRIT_PT_PT_DIST(GlblR0, GlblSrf1Pt0),
			IRIT_PT_PT_DIST(GlblR0, GlblSrf2Pt0),
			IRIT_PT_PT_DIST(GlblSrf1Pt0, GlblSrf2Pt0));
#	    endif /* DEBUG_MVAR_OFFSET_NI_TRACING */
	    
	    return FALSE;
	} 
	else {
	    IRIT_UV_COPY(uxOld, ux);
	    IRIT_UV_COPY(uyOld, uy);
	    Counter++;
	}
    }
}


/*****************************************************************************
* DESCRIPTION:								     *
*   Evaluates all components at current position.			     *
*									     *
* PARAMETERS:								     *
*   None								     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void EvalCompAtCurParam(void)
{
    IRIT_STATIC_DATA IrtRType 
	*P = NULL;

    P = CagdSrfEval(GlblSrf1, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblSrf1Pt0);
    P = CagdSrfEval(GlblSrf1u, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXu);
    P = CagdSrfEval(GlblSrf1v, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXv);
    P = CagdSrfEval(GlblSrf1uu, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXuu);
    P = CagdSrfEval(GlblSrf1uv, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXuv);
    P = CagdSrfEval(GlblSrf1vv, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXvv);
    P = CagdSrfEval(GlblSrf1uuu, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXuuu);
    P = CagdSrfEval(GlblSrf1uuv, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXuuv);
    P = CagdSrfEval(GlblSrf1uvv, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXuvv);
    P = CagdSrfEval(GlblSrf1vvv, GlblU1[0], GlblU1[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblXvvv);
    IRIT_CROSS_PROD(GlblNx, GlblXu, GlblXv);
    IRIT_PT_NORMALIZE(GlblNx);

    P = CagdSrfEval(GlblSrf2, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblSrf2Pt0);
    P = CagdSrfEval(GlblSrf2u, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYu);
    P = CagdSrfEval(GlblSrf2v, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYv);
    P = CagdSrfEval(GlblSrf2uu, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYuu);
    P = CagdSrfEval(GlblSrf2uv, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYuv);
    P = CagdSrfEval(GlblSrf2vv, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYvv);
    P = CagdSrfEval(GlblSrf2uuu, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYuuu);
    P = CagdSrfEval(GlblSrf2uuv, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYuuv);
    P = CagdSrfEval(GlblSrf2uvv, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYuvv);
    P = CagdSrfEval(GlblSrf2vvv, GlblU2[0], GlblU2[1]);
    MVAR_OFST_NI_E3_TO_PT(P, GlblYvvv);
    IRIT_CROSS_PROD(GlblNy, GlblYu, GlblYv);
    IRIT_PT_NORMALIZE(GlblNy);
}



