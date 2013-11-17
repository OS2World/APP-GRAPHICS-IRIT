/******************************************************************************
* CCnvHul.c - computation of convex hull of a freeform curve.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, January 96  				      *
* Modification by Hee-Seok Heo, August 98				      *
******************************************************************************/

#include "irit_sm.h"
#include "symb_loc.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"

/*
#define TEST_PERF_CH_PREPROC
#define NO_OPEN_END_PT_TEST
#define PARTIAL_ZEROSET_TEST
#define NO_MERGE_CH
#define TEST_PERF_CH2_PREPROC
#define DUMP_CH_DOMAINS
#define	DUMP_CH_CNTR_DOMAINS
*/

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugL1CHPreproc, FALSE);
IRIT_SET_DEBUG_PARAMETER(_DebugL2CHPreproc, FALSE);
#endif /* DEBUG */

#define	CRV_EXTREM_SET_EPS	1e-6
#define	MIN_MAX_EPS		1e-6
#define	NORMAL_ZERO_EPS		1e-6
#define	DISTANCE_EPS		1e-6
#define	CRV_CONST_SET_EPS	1e-6
#define	REJECT_SEG_EPS		0.0
#define ADJUST_PARAM_EPS        1e-3
#define CNTR_DIAGONAL_EPS	1e-3

typedef struct SymbCHSortPt {
    int I;
    CagdPtStruct *Pt;
} SymbCHSortPt;

typedef	struct	SymbSortDomainInfo {
    CagdRType N[2];
    CagdPtStruct *Pt;
} SymbSortDomainInfo;

IRIT_STATIC_DATA IrtPlnType
    GlblPlane = { 1.0, 0.0, 0.0, 0.0 }; /* Used to contour surfaces. */

static CagdCrvStruct *SymbCHCrvListInit(CagdCrvStruct *Crvs);
static CagdSrfStruct *SymbPtCnvxHullF3Srf(CagdPType Pt,
					  CagdCrvStruct *NCrv,
					  CagdCrvStruct *VCrv);
static CagdSrfStruct *SymbCrvCnvxHullF3Srf(CagdCrvStruct *UCrv,
					   CagdCrvStruct *VCrv);
static void InsertInvalidDomainIntoListAux(CagdPtStruct *Pt,
					   CagdRType TMin,
					   CagdRType TMax);
static void InsertInvalidDomainIntoList(CagdPtStruct **PtList,
					CagdRType TMin,
					CagdRType TMax);
static void InsertValidDomainIntoListAux(CagdPtStruct *Pt,
				 	 CagdRType TMin,
					 CagdRType TMax);
static void InsertValidDomainIntoList(CagdPtStruct **PtList,
				      CagdRType TMin,
				      CagdRType TMax);
static void ConnectDomainEps(CagdPtStruct **PtList, CagdRType Eps);
static void DeleteSmallDomainEps(CagdPtStruct **PtList, CagdRType Eps);
static void ProcessContours(IPPolygonStruct *Cntrs,
			    CagdPtStruct **PtInCHDomain);
static CagdPtStruct *SymbCHSortPtList(CagdPtStruct *PtList, int Indx);
static void SymbCHCrv2DTransform(CagdCrvStruct *Crv,
				 CagdRType Tx,
				 CagdRType Ty,
				 CagdRType Rc,
				 CagdRType Rs);
static void CrvDirectionalExtremPoints(CagdCrvStruct *Crvs,
				       CagdVType Vec,
				       CagdPtStruct **PtMax,
				       CagdPtStruct **PtMin);
static void CrvExtremPolygon(CagdCrvStruct *Crvs,
			     int NDir,
			     CagdPtStruct **ExtList);
static void CrvExternalDomainFromLine(CagdCrvStruct *Crvs,
				      CagdVType Pa,
				      CagdVType Pb,
				      CagdVType Na,
				      CagdVType Nb,
				      CagdPtStruct **PtList);
static void IncludeExtremePoints(CagdPtStruct **PtList,
				 CagdPtStruct *ExtList,
				 CagdCrvStruct *Crvs);
static CagdRType GetAngle(CagdRType x, CagdRType y);
static CagdPtStruct *SortValidDomains(CagdPtStruct *PtList,
				      CagdPtStruct *ExtList,
				      CagdCrvStruct *Crvs);
static CagdCrvStruct *CnvrtBspline2BezierKnotCrv(CagdCrvStruct *Crv);
static CagdCrvStruct *E2LineCrvNew(CagdPType P0,
				   CagdPType P1,
				   CagdRType K0,
				   CagdRType K1);
static CagdCrvStruct *CHCrvsFromCHDomain(CagdCrvStruct *Crvs,
					 CagdPtStruct *PtInCHDomain,
					 int SetOrient);
static IPPolygonStruct *SymbCHPtTestZeroSetAux(CagdPType Pt,
					       CagdCrvStruct *NCrv,
					       CagdCrvStruct *Crvs,
					       CagdCrvStruct *Lines,
					       CagdRType FineNess);
static IPPolygonStruct *SymbCHTestZeroSetAux(CagdCrvStruct *Crvs,
					     CagdCrvStruct *Lines,
					     CagdRType FineNess);
static IPPolygonStruct *SymbCHTestZeroSet(CagdCrvStruct *Crvs,
					  CagdRType FineNess,
					  CagdPtStruct *PtOrderedList);
#ifdef PARTIAL_ZEROSET_TEST
static IPPolygonStruct *SymbCHTestZeroSetWithExtremLine(CagdCrvStruct *Crvs,
							CagdPType Pa,
							CagdPType Pb,
							CagdRType FineNess,
							CagdPtStruct *PtList);
#endif /* PARTIAL_ZEROSET_TEST */
static CagdPtStruct *SymbCHCrvOpenPt(CagdCrvStruct *Crvs, CagdPtStruct *PtList);
static CagdPtStruct *SymbCHTestOpenPoints(CagdPtStruct *Pts,
					  CagdRType FineNess,
					  CagdPtStruct *PtOrderedList,
					  CagdCrvStruct *Crvs);

#if defined(ultrix) && defined(mips)
static int CompSymbCHSortPt(VoidPtr P1, VoidPtr P2);
#else
static int CompSymbCHSortPt(const VoidPtr P1, const VoidPtr P2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a freeform curve, compute its diameter as a function.	             M
* If the curve is a convex, probably as a result of a convex hull	     M
* computation of an original curve, the matching will be one to one.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to process its diameter function.                     M
*   FineNess:  Of numeric search for the zero set (for surface subdivision). M
*	       A positive value (10 is a good start).			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  Contours of the matched parallel lines on Crv.       M
*	       Each vertex will hold two parameter values on Crv.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvPtTangents, SymbCrvDiameterMinMax                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvDiameter                                                          M
*****************************************************************************/
IPPolygonStruct *SymbCrvDiameter(const CagdCrvStruct *Crv, CagdRType FineNess)
{
    IPPolygonStruct *Cntrs, *Cntr, *PrevC;
    CagdCrvStruct
	*DCrv = CagdCrvDerive(Crv);
    CagdSrfStruct *Srf, *Srf1, *Srf2, *SrfT,*SrfR,
	*SrfRW, *SrfRX, *SrfRY, *SrfRZ, *SrfTW, *SrfTX, *SrfTY, *SrfTZ;

    /* Make sure the domain is zero to one. */
    BspKnotAffineTrans(DCrv -> KnotVector,
		       DCrv -> Length + DCrv -> Order,
		       -DCrv -> KnotVector[0],
		       1.0 / (DCrv -> KnotVector[DCrv -> Length +
						 DCrv -> Order - 1] -
			      DCrv -> KnotVector[0]));

    SrfT = CagdPromoteCrvToSrf(DCrv, CAGD_CONST_U_DIR),
    SrfR = CagdPromoteCrvToSrf(DCrv, CAGD_CONST_V_DIR);
    CagdCrvFree(DCrv);

    SymbSrfSplitScalar(SrfT, &SrfTW, &SrfTX, &SrfTY, &SrfTZ);
    CagdSrfFree(SrfT);
    if (SrfTW != NULL)
	CagdSrfFree(SrfTW);
    if (SrfTZ != NULL)
	CagdSrfFree(SrfTZ);

    SymbSrfSplitScalar(SrfR, &SrfRW, &SrfRX, &SrfRY, &SrfRZ);
    CagdSrfFree(SrfR);
    if (SrfRW != NULL)
	CagdSrfFree(SrfRW);
    if (SrfRZ != NULL)
	CagdSrfFree(SrfRZ);

    Srf1 = SymbSrfMult(SrfTX, SrfRY);
    CagdSrfFree(SrfTX);
    CagdSrfFree(SrfRY);
    Srf2 = SymbSrfMult(SrfTY, SrfRX);
    CagdSrfFree(SrfTY);
    CagdSrfFree(SrfRX);
    Srf = SymbSrfSub(Srf1, Srf2);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

#   ifdef DUMP_DIAMETER_SYMBOLIC_FUNC
	IPStdoutObject(IPGenSRFObject(Srf), FALSE); /* Also a memory leak... */
#   endif /* DUMP_DIAMETER_SYMBOLIC_FUNC */

    Cntrs = UserCntrSrfWithPlane(Srf, GlblPlane, FineNess);

    CagdSrfFree(Srf);

    /* Eliminate points below or on the diagonal. */
    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V, *PrevV;
	CagdRType TMin, TMax, Dt;

	CagdCrvDomain(Crv, &TMin, &TMax);
	Dt = TMax - TMin;

	for (V = Cntr -> PVertex, PrevV = NULL; V != NULL; ) {
	    /* Consider only points that are off the diagonal. */
	    if (IRIT_APX_EQ_EPS(V -> Coord[1], V -> Coord[2],
				CNTR_DIAGONAL_EPS) ||
		V -> Coord[1] < V -> Coord[2]) {
		/* Delete this vertex. */
		if (PrevV != NULL) {
		    V = V -> Pnext;
		    IPFreeVertex(PrevV -> Pnext);
		    PrevV -> Pnext = V;
		}
		else {
		    Cntr -> PVertex = V -> Pnext;
		    IPFreeVertex(V);
		    V = Cntr -> PVertex;
		}
	    }
	    else {
		V -> Coord[0] = TMin + Dt * V -> Coord[1];
		V -> Coord[0] = IRIT_BOUND(V -> Coord[0], TMin, TMax);
		V -> Coord[1] = TMin + Dt * V -> Coord[2];
		V -> Coord[1] = IRIT_BOUND(V -> Coord[1], TMin, TMax);
		V -> Coord[2] = 0.0;
		PrevV = V;
		V = V -> Pnext;
	    }
	}
    }

    /* Eliminate empty contours. */
    for (Cntr = Cntrs, PrevC = NULL; Cntr != NULL; ) {
	if (Cntr -> PVertex == NULL) {
	    if (PrevC != NULL) {
		Cntr = Cntr -> Pnext;
		IPFreePolygon(PrevC -> Pnext);
		PrevC -> Pnext = Cntr;
	    }
	    else {
		Cntrs = Cntrs -> Pnext;
		IPFreePolygon(Cntr);
		Cntr = Cntrs;
	    }
	}
	else {
	    PrevC = Cntr;
	    Cntr = Cntr -> Pnext;
	}
    }

#   ifdef DUMP_DIAMETER_CONTOURS
	IPStdoutObject(IPGenPOLYObject(Cntrs), FALSE); /* Also a mem leak... */
#   endif /* DUMP_DIAMETER_CONTOURS */

    return Cntrs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the maximum or minimum diameter out of diameter matched list    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   To compute its diameter.                                          M
*   Cntrs: Output of SymbCrvDiameter - the matched paraller tangents.        M
*   Min:   TRUE of minimum diameter, FALSE for maximum diameter.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *: Two parameter values on Crv of tangent lines extreme value. M
*		 Returns an address to a statically allocated point.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDiameter                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvDiameterMinMax                                                    M
*****************************************************************************/
CagdRType *SymbCrvDiameterMinMax(const CagdCrvStruct *Crv,
				 IPPolygonStruct *Cntrs,
				 int Min)
{
    IRIT_STATIC_DATA CagdRType Params[3];
    IPPolygonStruct *Cntr;
    CagdRType
	ExtremeDist = Min ? IRIT_INFNTY : -IRIT_INFNTY;

    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V;

	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
	    CagdRType Dist, *R;
	    CagdPType Pt1, Pt2;

	    R = CagdCrvEval(Crv, V -> Coord[0]);
	    CagdCoerceToE3(Pt1, &R, -1, Crv -> PType);
	    R = CagdCrvEval(Crv, V -> Coord[1]);
	    CagdCoerceToE3(Pt2, &R, -1, Crv -> PType);

	    Dist = IRIT_PT_PT_DIST(Pt1, Pt2);
	    if (Min) {
		if (Dist < ExtremeDist) {
		    ExtremeDist = Dist;
		    IRIT_PT_COPY(Params, V -> Coord);
		}
	    }
	    else {
		if (Dist > ExtremeDist) {
		    ExtremeDist = Dist;
		    IRIT_PT_COPY(Params, V -> Coord);
		}
	    }
	}
    }

    return Params;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Initialize a given CrvList to calculate their convex hull               *
*    Crv0     -> Crv1    -> ...                                              *
*    0.0~1.0     1.1~2.1                                                     *
* PARAMETERS:                                                                *
*   Crvs:  To compute their convex hull.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  To compute their convex hull			     *
*			 (Open, Normal Knot, BspCrv).			     *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCrvCnvxHull                                                          *
*****************************************************************************/
static CagdCrvStruct *SymbCHCrvListInit(CagdCrvStruct *Crvs)
{
    CagdRType Start;
    CagdCrvStruct *Crv, *TCrv,
	*CrvList = NULL;

    for (Start = 0.0, Crv = Crvs; Crv; Crv = Crv -> Pnext, Start += 1.1) {
	/* Make sure the given curve is open end conditioned curve. */
	if (CAGD_IS_BEZIER_CRV(Crv))
	    TCrv = CagdCnvrtBzr2BspCrv(Crv);
	else if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv))
	    TCrv = BspCrvOpenEnd(Crv);
	else
	    TCrv = CagdCrvCopy(Crv);

	/* Make sure the domain is zero to one. */
	BspKnotAffineTrans(TCrv -> KnotVector,
			TCrv -> Length + TCrv -> Order,
			Start - TCrv -> KnotVector[0],
			1.0 / (TCrv -> KnotVector[TCrv -> Length +
				TCrv -> Order - 1] -
			TCrv -> KnotVector[0]));

	IRIT_LIST_PUSH(TCrv, CrvList);
    }

    return CagdListReverse(CrvList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate S(u,v) = <C(v) - Pt , N(u)> >= 0,   for all r.	             *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:         Pt.							     *
*   NCrv:       N(u).				                             *
*   VCrv:       C(v).				                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *: S(u,v) SrfList.                                         *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCrvCnvxHull                                                          *
*****************************************************************************/
static CagdSrfStruct *SymbPtCnvxHullF3Srf(CagdPType Pt,
					  CagdCrvStruct *NCrv,
					  CagdCrvStruct *VCrv)
{
    CagdPType Translate;
    CagdRType	UMin, UMax, VMin, VMax;
    CagdSrfStruct *Srf, *NSrf, *Srf1, *Srf2,
	*SrfW, *SrfX, *SrfY, *SrfZ, *NSrfW, *NSrfX, *NSrfY, *NSrfZ;

    CagdCrvDomain(NCrv, &UMin, &UMax);
    CagdCrvDomain(VCrv, &VMin, &VMax);
    IRIT_PT_SCALE2(Translate, Pt, -1.0);

    NSrf = CagdPromoteCrvToSrf(NCrv, CAGD_CONST_U_DIR);
    BspKnotAffineTrans(NSrf -> VKnotVector,
                       NSrf -> VLength + NSrf -> VOrder,
                       VMin - NSrf -> VKnotVector[0],
                       (VMax - VMin) / (NSrf -> VKnotVector[NSrf -> VLength +
                              NSrf -> VOrder - 1] - NSrf -> VKnotVector[0]) );

    SymbSrfSplitScalar(NSrf, &NSrfW, &NSrfX, &NSrfY, &NSrfZ);
    CagdSrfFree(NSrf);
    if (NSrfW != NULL)
	CagdSrfFree(NSrfW);
    if (NSrfZ != NULL)
	CagdSrfFree(NSrfZ);

    VCrv = CagdCrvCopy(VCrv);
    CagdCrvTransform(VCrv, Translate, 1.0);
    Srf1 = CagdPromoteCrvToSrf(VCrv, CAGD_CONST_V_DIR);
    CagdCrvFree(VCrv);
    BspKnotAffineTrans(Srf1 -> UKnotVector,
                       Srf1 -> ULength + Srf1 -> UOrder,
                       UMin - Srf1 -> UKnotVector[0],
                       (UMax - UMin) / (Srf1 -> UKnotVector[Srf1 -> ULength +
                              Srf1 -> UOrder - 1] - Srf1 -> UKnotVector[0]) );
    SymbSrfSplitScalar(Srf1, &SrfW, &SrfX, &SrfY, &SrfZ);
    CagdSrfFree(Srf1);
    if (SrfW != NULL)
	CagdSrfFree(SrfW);
    if (SrfZ != NULL)
	CagdSrfFree(SrfZ);

    Srf1 = SymbSrfMult(SrfX, NSrfX);
    CagdSrfFree(SrfX);
    CagdSrfFree(NSrfX);
    Srf2 = SymbSrfMult(SrfY, NSrfY);
    CagdSrfFree(SrfY);
    CagdSrfFree(NSrfY);
    Srf = SymbSrfAdd(Srf1, Srf2);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

#   ifdef DUMP_CH_SYMBOLIC_FUNC
	IPStdoutObject(IPGenSRFObject(Srf), FALSE);    /* Also a mem leak... */
#   endif /* DUMP_CH_SYMBOLIC_FUNC */

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate S(u,v) = (C(v) - C(u)) x C'(u) >= 0,   for all r.	             *
*                                                                            *
* PARAMETERS:                                                                *
*   UCrv:       C(u).				                             *
*   VCrv:       C(v).				                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *: S(u,v) SrfList.                                         *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCrvCnvxHull                                                          *
*****************************************************************************/
static CagdSrfStruct *SymbCrvCnvxHullF3Srf(CagdCrvStruct *UCrv,
					   CagdCrvStruct *VCrv)
{
    CagdRType	UMin, UMax, VMin, VMax;
    CagdCrvStruct *DCrv;
    CagdSrfStruct *Srf, *DSrf, *SrfT, *SrfR, *Srf1, *Srf2,
	*SrfW, *SrfX, *SrfY, *SrfZ, *DSrfW, *DSrfX, *DSrfY, *DSrfZ;

    CagdCrvDomain(UCrv, &UMin, &UMax);
    CagdCrvDomain(VCrv, &VMin, &VMax);
    DCrv = CagdCrvDerive(UCrv);
    DSrf = CagdPromoteCrvToSrf(DCrv, CAGD_CONST_U_DIR);
    BspKnotAffineTrans(DSrf -> VKnotVector,
                       DSrf -> VLength + DSrf -> VOrder,
                       VMin - DSrf -> VKnotVector[0],
                       (VMax - VMin) / (DSrf -> VKnotVector[DSrf -> VLength +
                              DSrf -> VOrder - 1] - DSrf -> VKnotVector[0]) );
    CagdCrvFree(DCrv);

    SymbSrfSplitScalar(DSrf, &DSrfW, &DSrfX, &DSrfY, &DSrfZ);
    CagdSrfFree(DSrf);
    if (DSrfW != NULL)
	CagdSrfFree(DSrfW);
    if (DSrfZ != NULL)
	CagdSrfFree(DSrfZ);

    SrfT = CagdPromoteCrvToSrf(UCrv, CAGD_CONST_U_DIR);
    BspKnotAffineTrans(SrfT -> VKnotVector,
                       SrfT -> VLength + SrfT -> VOrder,
                       VMin - SrfT -> VKnotVector[0],
                       (VMax - VMin) / (SrfT -> VKnotVector[SrfT -> VLength +
                              SrfT -> VOrder - 1] - SrfT -> VKnotVector[0]) );
    SrfR = CagdPromoteCrvToSrf(VCrv, CAGD_CONST_V_DIR);
    BspKnotAffineTrans(SrfR -> UKnotVector,
                       SrfR -> ULength + SrfR -> UOrder,
                       UMin - SrfR -> UKnotVector[0],
                       (UMax - UMin) / (SrfR -> UKnotVector[SrfR -> ULength +
                              SrfR -> UOrder - 1] - SrfR -> UKnotVector[0]) );

    Srf1 = SymbSrfSub(SrfR, SrfT);
    CagdSrfFree(SrfR);
    CagdSrfFree(SrfT);
    SymbSrfSplitScalar(Srf1, &SrfW, &SrfX, &SrfY, &SrfZ);
    CagdSrfFree(Srf1);
    if (SrfW != NULL)
	CagdSrfFree(SrfW);
    if (SrfZ != NULL)
	CagdSrfFree(SrfZ);

    Srf1 = SymbSrfMult(SrfX, DSrfY);
    CagdSrfFree(SrfX);
    CagdSrfFree(DSrfY);
    Srf2 = SymbSrfMult(SrfY, DSrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(DSrfX);
    Srf = SymbSrfSub(Srf1, Srf2);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

#   ifdef DUMP_CH_SYMBOLIC_FUNC
	IPStdoutObject(IPGenSRFObject(Srf), FALSE);    /* Also a mem leak... */
#   endif /* DUMP_CH_SYMBOLIC_FUNC */

    return Srf;
}

/*****************************************************************************
* AUXILIARY:   		                                                     *
*   Auxiliary function for  InsertInvalidDomainIntoList                      *
*   Pre-condition Pt -> Pt[1] < TMin <= Pt -> Pnext -> Pt[1]                 * 
*****************************************************************************/
static void InsertInvalidDomainIntoListAux(CagdPtStruct *Pt,
					   CagdRType TMin,
					   CagdRType TMax)
{
    CagdPtStruct *Pt1;

    if (!Pt -> Pnext || TMax < Pt -> Pnext -> Pt[0])
	return;

    if (Pt -> Pnext -> Pt[0] < TMin) {
	Pt1 = CagdPtNew();
	Pt1 -> Pt[0] = Pt -> Pnext -> Pt[0];
	Pt1 -> Pt[1] = TMin;
	Pt1 -> Pnext = Pt -> Pnext;
	Pt -> Pnext = Pt1;
	Pt = Pt -> Pnext;
    }

    if (TMax < Pt -> Pnext -> Pt[1])
	Pt -> Pnext -> Pt[0] = TMax;
    else {
	Pt1 = Pt -> Pnext;
	Pt -> Pnext = Pt -> Pnext -> Pnext;
	CagdPtFree(Pt1);
	InsertInvalidDomainIntoListAux(Pt, TMin, TMax);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts one new invalid domain from TMin to TMax to the list of valid    *
* domains PtList.			                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:      Current list of valid domains, to update in place.          *
*   TMin, TMax:  New invalid domain to insert.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertInvalidDomainIntoList(CagdPtStruct **PtList,
					CagdRType TMin,
					CagdRType TMax)
{
    CagdPtStruct Pts, *Pt;

    if (TMax <= TMin)
	return;

    Pts.Pnext = *PtList;

    for (Pt = &Pts;
	 Pt -> Pnext && TMin > Pt -> Pnext -> Pt[1];
	 Pt = Pt -> Pnext);

    InsertInvalidDomainIntoListAux(Pt, TMin, TMax);

    *PtList = Pts.Pnext;
}

/*****************************************************************************
* AUXILIARY:   		                                                     *
*   Auxiliary function for InsertValidDomainIntoList                         *
*   Pre-condition Pt -> Pt[1] < TMin <= Pt -> Pnext -> Pt[1]                 * 
*****************************************************************************/
static void InsertValidDomainIntoListAux(CagdPtStruct *Pt,
				 	 CagdRType TMin,
					 CagdRType TMax)
{
    CagdPtStruct *Pt1;

    if (!Pt -> Pnext || TMax < Pt -> Pnext -> Pt[0]) {
	Pt1 = CagdPtNew();
	Pt1 -> Pt[0] = TMin;
	Pt1 -> Pt[1] = TMax;
	Pt1 -> Pnext = Pt -> Pnext;
	Pt -> Pnext = Pt1;
    }
    else {
	Pt1 = Pt -> Pnext;
	Pt -> Pnext = Pt -> Pnext -> Pnext;
	TMin = IRIT_MIN(TMin, Pt1 -> Pt[0]);
	TMax = IRIT_MAX(TMax, Pt1 -> Pt[1]);
	CagdPtFree(Pt1);
	InsertValidDomainIntoListAux(Pt, TMin, TMax);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts one new valid domain from TMin to TMax to the list of valid      *
* domains PtList.			                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:      Current list of valid domains, to update in place.          *
*   TMin, TMax:  New valid domain to insert.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertValidDomainIntoList(CagdPtStruct **PtList,
				      CagdRType TMin,
				      CagdRType TMax)
{
    CagdPtStruct Pts, *Pt;

    if (TMax <= TMin)
	return;

    Pts.Pnext = *PtList;

    for (Pt = &Pts;
	 Pt -> Pnext && TMin > Pt -> Pnext -> Pt[1];
	 Pt = Pt -> Pnext);

    InsertValidDomainIntoListAux(Pt, TMin, TMax);

    *PtList = Pts.Pnext;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Connect domains which have little distance.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:      Current list of valid domains, to update in place.          *
*   Eps:	 Connect Domains in the distance Eps			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ConnectDomainEps(CagdPtStruct **PtList, CagdRType Eps)
{
    CagdPtStruct *Pt, *Pt1;

    if (!(*PtList))
	return;

    Pt = (*PtList);
    while(Pt -> Pnext) {
	if (Pt -> Pnext -> Pt[0] - Pt -> Pt[1] < Eps) {
	    Pt -> Pt[1] = Pt -> Pnext -> Pt[1];
	    Pt1 = Pt -> Pnext;
	    Pt -> Pnext = Pt -> Pnext -> Pnext;
	    CagdPtFree(Pt1);
	}
	else
	    Pt = Pt -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Delete small domains in current vaild domains                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:      Current list of valid domains, to update in place.          *
*   Eps:	 Threshold of a small domain 	 			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DeleteSmallDomainEps(CagdPtStruct **PtList, CagdRType Eps)
{
    CagdPtStruct Pts, *Pt, *Pt1;

    Pts.Pnext = *PtList;
    Pt = &Pts;
    while(Pt -> Pnext) {
	if (Pt -> Pnext -> Pt[1] - Pt -> Pnext -> Pt[0] < Eps) {
	    Pt1 = Pt -> Pnext;
	    Pt -> Pnext = Pt -> Pnext -> Pnext;
	    CagdPtFree(Pt1);
	}
	else
	    Pt = Pt -> Pnext;
    }

    *PtList = Pts.Pnext;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the u domain of the given Cntrs that signals the domain that is *
* not in the CH. the rest of the domain is hence, the the CH. 		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Cntrs:    the (almost) zero set of the symbolic functional. The contours *
*	      has the Height (Z), U, and V as the XYZ in this order.	     *
*   PtInCHDomain:  Points in domain.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void ProcessContours(IPPolygonStruct *Cntrs,
			    CagdPtStruct **PtInCHDomain)
{
    IPPolygonStruct *Cntr;

    /* We start with the entire zero to one domain as in the CH. */
    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V;
	CagdRType
	    TMin = IRIT_INFNTY,
	    TMax = -IRIT_INFNTY;

	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
	    /* Consider only points that are off the diagonal. */
	    if (!IRIT_APX_EQ_EPS(V -> Coord[1], V -> Coord[2],
				 CNTR_DIAGONAL_EPS)) {
		if (TMin > V -> Coord[1])
		    TMin = V -> Coord[1];
		if (TMax < V -> Coord[1])
		    TMax = V -> Coord[1];
	    }
	}

	if (TMin < TMax) {
#	    ifdef DUMP_CH_CNTR_DOMAINS
	        IRIT_INFO_MSG_PRINTF("TMin = %f, TMax = %f\n", TMin, TMax);
#	    endif /* DUMP_CH_CNTR_DOMAINS */

	    InsertInvalidDomainIntoList(PtInCHDomain, TMin, TMax);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorting key function for qsort (see below).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   P1, P2:  Two point to sort out and compute their relative order.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    1, 0, -1 based on the relation between P1 and P2.                *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int CompSymbCHSortPt(VoidPtr P1, VoidPtr P2)
#else
static int CompSymbCHSortPt(const VoidPtr P1, const VoidPtr P2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
        V1 = ((SymbCHSortPt *) P1) -> Pt -> Pt[((SymbCHSortPt *) P1) -> I],
    	V2 = ((SymbCHSortPt *) P2) -> Pt -> Pt[((SymbCHSortPt *) P2) -> I];
    
    if (V1 < V2)
	return -1;
    else if (V1 > V2)
	return 1;
    else
	return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sorts the point list.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:     Points to sort.                                              *
*   Indx:       Index of values to sort.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *:  Sorted points.                                          *
*****************************************************************************/
static CagdPtStruct *SymbCHSortPtList(CagdPtStruct *PtList, int Indx)
{
    SymbCHSortPt *SymbCHSortPtArray, *SPt;
    CagdPtStruct *Pt;
    int Cnt;

    for (Cnt = 0, Pt = PtList; Pt; Pt = Pt -> Pnext, Cnt++);
    if (!Cnt)
	return NULL;

    SPt = SymbCHSortPtArray =
	(SymbCHSortPt *) IritMalloc(Cnt * sizeof(SymbCHSortPt));

    for (Pt = PtList; Pt; SPt -> Pt = Pt, Pt = Pt -> Pnext, SPt++) {
	SPt -> I = Indx;
	SPt -> Pt = Pt;
    }

    qsort(SymbCHSortPtArray, Cnt, sizeof(SymbCHSortPt), CompSymbCHSortPt);
    PtList = NULL;

    while (Cnt--) {
	IRIT_LIST_PUSH(SymbCHSortPtArray[Cnt].Pt , PtList);
    }

    IritFree(SymbCHSortPtArray);

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   | Cx' | = | Rc  -Rs | | Cx + Tx |                                        *
*   | Cy' |   | Rs   Rc | | Cy + Ty |                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:             To be transformed. (Can be rational).                   *
*   Tx, Ty, Rc, Rs:  Transformations parameters.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SymbCHCrv2DTransform(CagdCrvStruct *Crv,
				 CagdRType Tx,
				 CagdRType Ty,
				 CagdRType Rc,
				 CagdRType Rs)
{
    int i,
        Length = Crv -> Length;
    CagdRType x, y,
        *PtX = Crv -> Points[1],
        *PtY = Crv -> Points[2];

    for (i = 0; i < Length; i++) {
	x = *PtX + Tx;
	y = *PtY + Ty;
	*PtX++ = Rc * x - Rs * y;
	*PtY++ = Rs * x + Rc * y;
    } 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find extreme points for the given direction vector.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       To compute their convex hull.                                *
*		Crv0(t00~t01) -> Crv1(t10~t11) -> ... [ti1 <= t(i+1)0]	     *
*   Vec:        Normalized direction vector				     *
*   PtList:     Current list of valid domains, to update in place.           *
*   PtMax:      Maximum points which are found in this function.             *
*   PtMin:      Minimum points which are found in this function.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvDirectionalExtremPoints(CagdCrvStruct *Crvs,
				       CagdVType Vec,
				       CagdPtStruct **PtMax,
				       CagdPtStruct **PtMin)
{
    CagdRType *P, MinV, MaxV, TMin, TMax;
    CagdCrvStruct *Crv1, *Crv;
    CagdPtStruct *Pt, *Pt1, *Pt2, *PtExtm;
    CagdMType Mat;

    MatGenMatRotZ(Vec[0], -Vec[1], Mat);

    for (PtExtm = NULL, Crv = Crvs; Crv; Crv = Crv -> Pnext) {
	Crv1 = CagdCrvCopy(Crv);
	CagdCrvDomain(Crv1, &TMin, &TMax);
	SymbCHCrv2DTransform(Crv1, 0.0, 0.0, Vec[0], -Vec[1]);

	Pt1 = SymbCrvExtremSet(Crv1, 1, CRV_EXTREM_SET_EPS, TRUE);

	for (Pt2 = NULL, Pt = Pt1; Pt; Pt = Pt -> Pnext)
	    InsertValidDomainIntoList(&Pt2,
				      Pt -> Pt[0] - ADJUST_PARAM_EPS,
				      Pt -> Pt[0] + ADJUST_PARAM_EPS);
	InsertValidDomainIntoList(&Pt2, TMin, TMin + ADJUST_PARAM_EPS);
	if (!CagdIsClosedCrv(Crv))
	    InsertValidDomainIntoList(&Pt2, TMax - ADJUST_PARAM_EPS, TMax);
	CagdPtFreeList(Pt1);

	for (Pt = Pt2; Pt; Pt = Pt -> Pnext) {
	    Pt -> Pt[0] = (Pt -> Pt[0] + Pt -> Pt[1]) * 0.5;
	    P = CagdCrvEval(Crv1, Pt -> Pt[0]);
	    Pt -> Pt[1] = CAGD_IS_RATIONAL_CRV(Crv1) ? P[1] / P[0] : P[1];
	}
 
	if (Pt2) {
	    Pt = Pt2;
	    IRIT_LIST_LAST_ELEM(Pt);
	    Pt->Pnext = PtExtm;
	    PtExtm = Pt2;
	}
	
	CagdCrvFree(Crv1);
    }

    CagdCrvFree(Crv);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
	IRIT_INFO_MSG("Start Partial Extrem Point List\n");
	for (Pt = PtExtm; Pt; Pt=Pt -> Pnext)
	    IRIT_INFO_MSG_PRINTF("	Pt: %lf\n", Pt -> Pt[0]);
	IRIT_INFO_MSG("End Partial Extrem Point List\n");
    }
#   endif /* DEBUG */

    /* Only use mininum and maximum values. */

    MinV = IRIT_INFNTY;
    MaxV = -IRIT_INFNTY;

    for (Pt = PtExtm; Pt; Pt = Pt -> Pnext) {
	if (Pt -> Pt[1] < MinV)
	    MinV = Pt -> Pt[1];
	if (Pt -> Pt[1] > MaxV)
	    MaxV = Pt -> Pt[1];
    }

    /* 1.2 select global extreme points. */
    Pt = PtExtm;
    while (Pt) {
	if (IRIT_APX_EQ_EPS(Pt -> Pt[1], MinV, MIN_MAX_EPS)) {
	    Pt -> Pt[1] = -Vec[0];
	    Pt -> Pt[2] = -Vec[1];
	    Pt1 = Pt;
	    Pt = Pt -> Pnext;
	    IRIT_LIST_PUSH(Pt1, (*PtMin));
	}
	else if (IRIT_APX_EQ_EPS(Pt -> Pt[1], MaxV, MIN_MAX_EPS)) {
	    Pt -> Pt[1] = Vec[0];
	    Pt -> Pt[2] = Vec[1];
	    Pt1 = Pt;
	    Pt = Pt -> Pnext;
	    IRIT_LIST_PUSH(Pt1, (*PtMax));
	}
	else {
	    Pt1 = Pt;
	    Pt = Pt -> Pnext;
	    CagdPtFree(Pt1);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate the extrem polygon for the given number of directions          *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       To compute its convex hull.                                  *
*   NDir:      Num. of the dir where we check trivial rejection.             *
*   ExList:  Extrem points which are found in this function.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvExtremPolygon(CagdCrvStruct *Crvs,
			     int NDir,
			     CagdPtStruct **ExtList)
{
    int	i;
    CagdRType Teta, DTeta;
    CagdPtStruct *Pt, *PtMax, *PtMin;
    CagdVType Vec;

    *ExtList = NULL;
    PtMax = NULL;
    PtMin = NULL;

    DTeta = M_PI / (CagdRType) NDir;
	
    /* 1. For each direction, select extreme points. */
    for (Teta = 0.0, i = 0; i < NDir; i++, Teta += DTeta) {
	IRIT_VEC_SET(Vec, cos(Teta), sin(Teta), 0.0);

#	ifdef DEBUG
	IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
	    IRIT_INFO_MSG_PRINTF(
		    "\nTest Normal Direction : (Cos, Sin) = (%lf, %lf)\n",
		    Vec[0], Vec[1]);
	}
#	endif /* DEBUG */

	CrvDirectionalExtremPoints(Crvs, Vec, &PtMax, &PtMin);
    }

    if (!PtMax || !PtMin) {
	IRIT_WARNING_MSG("CrvExtremPolygon: No extrem points\n");
	return;
    }

    /* PtExtm0 -> PtExtm1 -> ... -> PtExtmN -> PtExtm0 (repeat first). */
    (*ExtList) = Pt = CagdListReverse(PtMax);
    IRIT_LIST_LAST_ELEM(Pt);
    Pt -> Pnext = CagdListReverse(PtMin);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugL1CHPreproc) {
	IRIT_INFO_MSG("Global Extrem Point List\n");
	for (Pt = *ExtList; Pt; Pt=Pt -> Pnext)
	    IRIT_INFO_MSG_PRINTF("	Pt: %lf (%lf, %lf)\n",
				Pt -> Pt[0], Pt -> Pt[1], Pt -> Pt[2]);
	IRIT_INFO_MSG("End Global Extrem Point List\n");
    }
#   endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Select the external region from one line of the extreme polygon.         *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs: To compute their convex hull.		                             *
*   Pa:	  One extreme point.						     *
*   Pb:	  The other extreme point.					     *
*   Na:   One extreme normal.						     *
*   Nb:	  The other extreme normal.					     *
*   PtList:  List of valid domains, to update in place.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CrvExternalDomainFromLine(CagdCrvStruct *Crvs,
				      CagdVType Pa,
				      CagdVType Pb,
				      CagdVType Na,
				      CagdVType Nb,
				      CagdPtStruct **PtList)
{
    int IsClosed;
    CagdRType TMin, TMax, T1, T2, Len, Scale, *P;
    CagdCrvStruct *Crv1, *Crv;
    CagdPtStruct *PtIntx, *Pt;
    CagdVType Pm, Vab, Pam, N, Nab;

    /* Vab  = (Pb - Pa) / | Pb - Pa |. */
    IRIT_VEC_SUB(Vab, Pb, Pa);
    Len = IRIT_VEC_LENGTH(Vab);
    if (Len <  DISTANCE_EPS)
	return;
    Scale = 1.0 / Len;
    IRIT_VEC_SCALE(Vab, Scale);

    /* N = (Na + Nb) / | Na + Nb |. */
    IRIT_VEC_ADD(N, Na, Nb);
    Len = IRIT_VEC_LENGTH(N);
    if (Len < NORMAL_ZERO_EPS)
	return;
    Scale = 1.0 / Len;
    IRIT_VEC_SCALE(N, Scale);

    /* Nab = (-Vab[1], Vab[0]) orthogonal to Vab. */
    IRIT_VEC_SET(Nab, -Vab[1], Vab[0], 0.0);
    Len = IRIT_DOT_PROD(Nab, N);
    if (Len < -NORMAL_ZERO_EPS) {	   /* Test if Nab is proper normal. */
	IRIT_VEC_SET(Nab, Vab[1], -Vab[0], 0.0);
    }
    else if (Len < NORMAL_ZERO_EPS)
	return;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
	IRIT_INFO_MSG_PRINTF("Vab : (%lf, %lf)\n", Vab[0], Vab[1]);
	IRIT_INFO_MSG_PRINTF("PRE ORIGIN: (%lf, %lf) PRE VECTOR: (%lf, %lf)\n",
			     Pa[0], Pa[1], Pb[0], Pb[1]);
    }
#   endif /* DEBUG */

    for (Crv = Crvs; Crv; Crv = Crv -> Pnext) {
	IsClosed = CagdIsClosedCrv(Crv);
	CagdCrvDomain(Crv, &TMin, &TMax);
	Crv1 = CagdCrvCopy(Crv);
	SymbCHCrv2DTransform(Crv1, -Pa[0], -Pa[1], Vab[0], -Vab[1]);

	/* Find intersection between Crv and one line of Extreme Polygon. */
	PtIntx = SymbCrvConstSet(Crv1, 2, CRV_CONST_SET_EPS, 0.0, TRUE);
	CagdCrvFree(Crv1);

	if (!PtIntx) {		  /* Test if the crv is totally external. */
	    CagdPType  M;

	    CagdCoerceToE3(Pm, Crv -> Points, 0, Crv -> PType);
	    CagdCoerceToE3(M, Crv -> Points, Crv -> Length - 1, Crv -> PType);
	    IRIT_PT_ADD(Pm, M, Pm);
	    CagdCrvDomain(Crv, &T1, &T2);
	    P = CagdCrvEval(Crv, (T1 + T2) * 0.5);
	    CagdCoerceToE3(M, &P, -1, Crv -> PType);
	    IRIT_PT_ADD(Pm, M, Pm);
	    IRIT_PT_SCALE(Pm, 1.0 / 3.0);/* Pm: cntr of C(T0),C(T1),C(T0.5).*/
	    
	    IRIT_VEC_SUB(Pam, Pm, Pa);

	    if (IRIT_DOT_PROD(Pam, Nab) > -REJECT_SEG_EPS)
		InsertValidDomainIntoList(PtList, T1, T2);

	    continue;
	}

	/* Sort intersection points: Result[PtIntx]. */
	PtIntx = SymbCHSortPtList(PtIntx, 0);

	if (IsClosed) {
	    Pt = PtIntx;
	    IRIT_LIST_LAST_ELEM(Pt);
	    Pt -> Pnext = CagdPtCopy(PtIntx);
	    Pt -> Pnext -> Pt[0] += TMax - TMin;
	    Pt -> Pnext -> Pnext = NULL;
	}
	else {
	    Pt = PtIntx;
	    if (Pt -> Pt[0] > TMin) {
		PtIntx = CagdPtNew();
		PtIntx -> Pt[0] = TMin;
		PtIntx -> Pnext = Pt;
	    }
	    IRIT_LIST_LAST_ELEM(Pt);
	    if (Pt -> Pt[0] < TMax) {
		Pt -> Pnext = CagdPtNew();
		Pt -> Pnext -> Pt[0] = TMax;
		Pt -> Pnext -> Pnext = NULL;
	    }
	}

#	ifdef DEBUG
	IRIT_IF_DEBUG_ON_PARAMETER(_DebugL1CHPreproc) {
	    IRIT_INFO_MSG("Start Intersection  List\n");
	    for (Pt = PtIntx; Pt; Pt=Pt -> Pnext)
		IRIT_INFO_MSG_PRINTF("	Pt: %lf\n", Pt -> Pt[0]);
	    IRIT_INFO_MSG("End Intersection List\n\n");
	}
#	endif /* DEBUG */

	for (Pt = PtIntx; Pt && Pt -> Pnext; Pt = Pt -> Pnext) {
	    T1 = Pt -> Pt[0];
	    T2 = Pt -> Pnext -> Pt[0];

	    P = CagdCrvEval(Crv, (T1 + T2) * 0.5 > TMax ?
			(T1 + T2) * 0.5 - (TMax - TMin) : (T1 + T2) * 0.5);
	    CagdCoerceToE3(Pm, &P, -1, Crv -> PType);
	    if (T1 != TMin) {
		IRIT_VEC_SUB(Pam, Pm, Pa);
	    }
	    else {
	    	IRIT_VEC_SUB(Pam, Pm, Pb); /* Open Curve. */
	    }

	    if (IRIT_DOT_PROD(Pam, Nab) > -REJECT_SEG_EPS) {
		if (T2 > TMax) {
		    InsertValidDomainIntoList(PtList, T1, TMax);
		    InsertValidDomainIntoList(PtList, TMin, T2 - (TMax-TMin) );
		}
		else
		    InsertValidDomainIntoList(PtList, T1, T2);
	    }
	}

	CagdPtFreeList(PtIntx);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extreme Points should be included in ConvexHull                          *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:    Current list of valid domains, to update in place.            *
*   ExtList:   Extreme points.                                               *
*   Crvs: To compute their convex hull.		                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IncludeExtremePoints(CagdPtStruct **PtList,
				 CagdPtStruct *ExtList,
				 CagdCrvStruct *Crvs)
{
    CagdRType TMin, TMax;
    CagdPtStruct *EPt;
    CagdCrvStruct *Crv;

    for (EPt = ExtList; EPt; EPt = EPt -> Pnext) {
	for (Crv = Crvs; Crv; Crv = Crv -> Pnext) {
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (TMin <= EPt -> Pt[0] && EPt -> Pt[0] <= TMax) {
		TMin = IRIT_MAX(EPt -> Pt[0] - ADJUST_PARAM_EPS, TMin);
		TMax = IRIT_MIN(EPt -> Pt[0] + ADJUST_PARAM_EPS, TMax);
		InsertValidDomainIntoList(PtList, TMin, TMax);
	    	break;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Evaluate the point at T from CrvList.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:      CrvList which have mutually exclusive domain.                 *
*   T:         Parameter.                                                    *
*   Pt:        Evaluated Point.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void SymbCHCrvListEvalE3(CagdCrvStruct *Crvs,
			 CagdRType T,
			 CagdPType Pt)
{
    CagdRType TMin, TMax, *P;
    CagdCrvStruct *Crv;

    for (Crv = Crvs; Crv; Crv = Crv -> Pnext) {
	CagdCrvDomain(Crv, &TMin, &TMax);
	if (TMin <= T && T <= TMax) {
	    P = CagdCrvEval(Crv, T);
	    CagdCoerceToE3(Pt, &P, -1, Crv -> PType);
	    return;
	}
    }

    IRIT_WARNING_MSG_PRINTF("SymbCHCrvListEvalE3: [%.14lf] is out of domain\n",
			    T);
    SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute angle of a given xy coordinate.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:    Given coordinate.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Computed angle.                                             *
*****************************************************************************/
static CagdRType GetAngle(CagdRType x, CagdRType y)
{
    CagdRType Sv, Vax, Vay;

    Sv = sqrt(x * x + y * y);

    if (Sv < IRIT_EPS)
        return  0.0;

    Vax = x / Sv;
    Vay = y / Sv;

    if (Vax > 0.0 && Vay > 0.0)
        return atan(Vay / Vax);
    else if (Vax > IRIT_EPS)
        return atan(Vay / Vax) + M_PI + M_PI;
    else if (Vax < -IRIT_EPS)
        return M_PI + atan(Vay / Vax);
    else if (Vay > IRIT_EPS)
        return M_PI * 0.5;
    else if (Vay < -IRIT_EPS)
        return M_PI + M_PI * 0.5;
    else
        return 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Arrange valid domains in the coundclockwise normal direction.            *
*			(With C0Continuity!)				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:    Current list of valid domains, to update in place.            *
*   ExtList:   Extreme points.                                               *
*   Crvs:       To compute their convex hull.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct * : Arranged domains                                        *
*                                                                            *
*****************************************************************************/
static CagdPtStruct *SortValidDomains(CagdPtStruct *PtList,
				      CagdPtStruct *ExtList,
				      CagdCrvStruct *Crvs)
{
    SymbSortDomainInfo	*PtIArray, *PtI;
    CagdPtStruct *Pt, *PtE;
    int i, Cnt, Orient;
    CagdPType Center;

    for (Cnt = 0, Pt = PtList; Pt; Pt = Pt -> Pnext, Cnt++);
    if (!Cnt)
	return NULL;

    PtI = PtIArray = (SymbSortDomainInfo *)
				IritMalloc(Cnt * sizeof(SymbSortDomainInfo));
    Pt = PtList;
    IRIT_PT_SET(Center, 0.0, 0.0, 0.0);
    for (i = 0; i < Cnt; i++) {
	CagdPType Point;

	PtI -> N[0] = 0.0;
	PtI -> N[1] = 0.0;
	PtI -> Pt = Pt;
	SymbCHCrvListEvalE3(Crvs, Pt -> Pt[0], Point);
	IRIT_PT_ADD(Center, Center, Point);
	SymbCHCrvListEvalE3(Crvs, Pt -> Pt[1], Point);
	IRIT_PT_ADD(Center, Center, Point);
	PtI++;
	Pt = Pt -> Pnext;
    }
    IRIT_PT_SCALE(Center, 0.5/Cnt);
	
    for (PtE = ExtList; PtE; PtE = PtE -> Pnext) {
	for (PtI = PtIArray, i = 0; i < Cnt; i++, PtI++)
	    if (PtI -> Pt -> Pt[0] <= PtE -> Pt[0] &&
	        PtE -> Pt[0] <= PtI -> Pt -> Pt[1]) {
		PtI -> N[0] += PtE -> Pt[1];
		PtI -> N[1] += PtE -> Pt[2];
		break;
	    }
    }
    
    for (PtI = PtIArray, i = 0; i < Cnt; i++, PtI++) {
	CagdRType Size;
	CagdVType Pa, Pb, Pm, T, T2, N;

	SymbCHCrvListEvalE3(Crvs, PtI -> Pt -> Pt[0], Pa);
	SymbCHCrvListEvalE3(Crvs, PtI -> Pt -> Pt[1], Pb);

	if (PtI -> N[0] == 0.0 && PtI -> N[1] == 0.0) {
	    /* Estimate normal direction. */
	
	    CagdCrvStruct *Crv;
	    CagdRType TMin, TMax;

	    for (Crv = Crvs; Crv; Crv = Crv -> Pnext) {
		CagdCrvDomain(Crv, &TMin, &TMax);
		if (TMin <= PtI -> Pt -> Pt[0] && PtI -> Pt -> Pt[1] <= TMax)
			break;
	    }
	    if (!Crv) {
    		IRIT_WARNING_MSG_PRINTF("SortValidDomains: [%.14lf %.14lf] is out of domain\n",
				       PtI -> Pt -> Pt[0], PtI -> Pt -> Pt[1]);
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	    }

#	    ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugL1CHPreproc) {
		    IRIT_INFO_MSG_PRINTF("Estimate one normal of CHCrv[%lf %lf]\n",
				       PtI -> Pt -> Pt[0], PtI -> Pt -> Pt[1]);
		}
#	    endif /* DEBUG */

	    if (PtI -> Pt -> Pt[0] == TMin && CagdIsClosedCrv(Crv)) {
		int k;
		SymbSortDomainInfo *PtI2;
		    
    		for (PtI2 = PtIArray, k = 0; k < Cnt; k++, PtI2++)
		    if (PtI2 -> Pt -> Pt[1] == TMax)
			break;
		if (PtI2)
		    SymbCHCrvListEvalE3(Crv, IRIT_MIN(PtI2 -> Pt -> Pt[0]
					+ ADJUST_PARAM_EPS, TMax), Pa);
	    }
	    if (PtI -> Pt -> Pt[1] == TMax && CagdIsClosedCrv(Crv)) {
		int k;
		SymbSortDomainInfo *PtI2;
		    
    		for (PtI2 = PtIArray, k = 0; k < Cnt; k++, PtI2++)
		    if (PtI2 -> Pt -> Pt[0] == TMin)
			break;
		if (PtI2)
		    SymbCHCrvListEvalE3(Crv,
					IRIT_MAX(PtI2 -> Pt -> Pt[1]
						     -ADJUST_PARAM_EPS, TMin),
					Pb);
	    }

	    IRIT_VEC_SUB(T, Pb, Pa);
	    Size = IRIT_VEC_LENGTH(T);
	    if (Size > IRIT_EPS) {
		Size = 1.0 / Size;
		IRIT_VEC_SCALE(T, Size);
		IRIT_VEC_SET(N, T[1], -T[0], 0.0);
		SymbCHCrvListEvalE3(Crv,
			(PtI -> Pt -> Pt[0] + PtI -> Pt -> Pt[1]) * 0.5, Pm);
		IRIT_VEC_SUB(T, Pm, Center);
	        Size = IRIT_VEC_LENGTH(T);
		if (Size > IRIT_EPS) {
		    Size = 1.0 / Size;
		    IRIT_VEC_SCALE(T, Size);
		    Size = IRIT_DOT_PROD(T, N);
		    if (Size > 0.0) {
			PtI -> N[0] = N[0];
			PtI -> N[1] = N[1];
		    }
		    else if (Size < 0.0) {
			PtI -> N[0] = -N[0];
			PtI -> N[1] = -N[1];
		    }
		}
	    }
	}

	/* Estimate orientation						*/
	/* Zvalue{(N)X(Pb-Pa)} > 0 Positive orientation		        */
	/*                     < 0 Negative orientation.		*/
	Orient = 0;
	if (PtI -> N[0] != 0.0 || PtI -> N[1] != 0.0) {
	    IRIT_VEC_SET(T, PtI -> N[0], PtI -> N[1], 0.0);
	    IRIT_VEC_SUB(T2, Pb, Pa);
	    Size = IRIT_VEC_LENGTH(T2);
	    if (Size > IRIT_EPS) {
		Size = 1.0 / Size;
		IRIT_VEC_SCALE(T2, Size);
		IRIT_CROSS_PROD(N, T, T2);
		if (N[2] > 0.0)
		    Orient = 1;
		else if (N[2] < 0.0)
		    Orient = -1;
		else
		    Orient = 0;
	    }
	}

	if (PtI -> N[0] == 0.0 && PtI -> N[1] == 0.0)
	    IRIT_WARNING_MSG("Can't estimate one normal of CHCrv\n");
	if (!Orient)
	    IRIT_WARNING_MSG("Can't estimate one orientation of CHCrv\n");

	if (Orient < 0) {
	    IRIT_SWAP(CagdRType, PtI -> Pt -> Pt[0], PtI -> Pt -> Pt[1]);
	}

	PtI -> Pt -> Pt[2] = GetAngle(PtI -> N[0], PtI -> N[1]);
    }

    PtList = SymbCHSortPtList(PtList, 2);

    IritFree(PtIArray);

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a Bspline curve into a set of Bezier curves by subdividing the    *
* Bspline curve at all its internal knots.				     *
*   Returned is a list of subdivided curves.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:     A Bspline curve to convert to a Bezier curve.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   A list of Bezier-knot curves representing the Bspline *
*                      curve Crv.					     *
*****************************************************************************/
static CagdCrvStruct *CnvrtBspline2BezierKnotCrv(CagdCrvStruct *Crv)
{
    CagdBType
	NewCrv = FALSE;
    int i, Order, Length;
    CagdRType LastT, *KnotVector;
    CagdCrvStruct *OrigCrv,
	*BezierCrvs = NULL;

    if (Crv -> GType != CAGD_CBSPLINE_TYPE) {
	SYMB_FATAL_ERROR(SYMB_ERR_ILLEGAL_PARAMETERS);
	return NULL;
    }

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv)) {
	Crv = BspCrvOpenEnd(Crv);
	NewCrv = TRUE;
    }

    Order = Crv -> Order,
    Length = Crv -> Length;
    KnotVector = Crv -> KnotVector;
    OrigCrv = Crv;

    for (i = Length - 1, LastT = KnotVector[Length]; i >= Order; i--) {
    	CagdRType
    	    t = KnotVector[i];
    	    
	if (!IRIT_APX_EQ(LastT, t)) {
    	    CagdCrvStruct
		*Crvs = BspCrvSubdivAtParam(Crv, t);

    	    if (Crv != OrigCrv)
    	        CagdCrvFree(Crv);

    	    Crvs -> Pnext -> Pnext = BezierCrvs;
    	    BezierCrvs = Crvs -> Pnext;

    	    Crv = Crvs;
    	    Crv -> Pnext = NULL;

	    LastT = t;
    	}
    }

    if (Crv == OrigCrv) {
	/* No interior knots in this curve - just copy it: */
	BezierCrvs = CagdCrvCopy(Crv);
    }
    else {
    	Crv -> Pnext = BezierCrvs;
    	BezierCrvs = Crv;
    }

    if (NewCrv)
	CagdCrvFree(Crv);

    return BezierCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generate Line Segment from P0 to P1 with parameter domain [K0 K1]        *
*									     *
* PARAMETERS:                                                                *
*   P0:         Start Point.                                                 *
*   Pb:         End Point.                                                   *
*   K0:         Start Knot.                                                  *
*   K1:         End Knot.                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:                                                         *
*****************************************************************************/
static CagdCrvStruct *E2LineCrvNew(CagdPType P0,
				   CagdPType P1,
				   CagdRType K0,
				   CagdRType K1)
{
    CagdCrvStruct *Line;

    Line = BspCrvNew(2, 2, CAGD_PT_E2_TYPE);
    Line -> KnotVector[0] = Line -> KnotVector[1] = K0; /* Avoid dup */
    Line -> KnotVector[2] = Line -> KnotVector[3] = K1;
    Line -> Points[1][0] = P0[0];
    Line -> Points[1][1] = P1[0];
    Line -> Points[2][0] = P0[1];
    Line -> Points[2][1] = P1[1];
    Line -> Pnext = NULL;

    return Line;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* the proper sub-domains that are in CH are extracted from Crv and returned  *
* as a list.				                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:           To extract the domain of them on the CH.                 *
*   PtInCHDomain:  Points in domain.					     *
*   SetOrient:  If True, reverse curve seg. according to the orientation.    *
*                 (Pt->Pt[1] < Pt->Pt[0] means the reverse orientation)      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  A list of sub curves of Crv that are on the CH.        *
*****************************************************************************/
static CagdCrvStruct *CHCrvsFromCHDomain(CagdCrvStruct *Crvs,
					 CagdPtStruct *PtInCHDomain,
					 int SetOrient)
{
    CagdCrvStruct *Crv, *TCrv, *CrvList = NULL;
    CagdPtStruct *Pt;
    CagdRType TMin, TMax;

    for (Pt = PtInCHDomain; Pt != NULL; Pt = Pt -> Pnext) {
	for (Crv = Crvs; Crv; Crv = Crv -> Pnext) {

	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (Pt -> Pt[0] <= Pt -> Pt[1]) {
		TMin = IRIT_MAX(Pt -> Pt[0], TMin);
		TMax = IRIT_MIN(Pt -> Pt[1], TMax);
	    }
	    else {
		TMin = IRIT_MAX(Pt -> Pt[1], TMin);
		TMax = IRIT_MIN(Pt -> Pt[0], TMax);
	    }

	    if (TMax - TMin < IRIT_EPS)
		continue;

	    TCrv = CagdCrvRegionFromCrv(Crv, TMin, TMax);

	    if (SetOrient && Pt -> Pt[0] > Pt -> Pt[1]) {
		CagdCrvStruct *T2Crv = CagdCrvReverse(TCrv);

		CagdCrvFree(TCrv);
		TCrv = T2Crv;
	    }

#	    ifdef DUMP_CH_CRV_DOMAINS
		/* Also a memory leak... */
		IPStdoutObject(IPGenCRVObject(TCrv), FALSE);
#	    endif /* DUMP_CH_CRV_DOMAINS */

	    IRIT_LIST_PUSH(TCrv, CrvList);
	}
    }

    return CagdListReverse(CrvList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate the zero level of S(u,v) = <C(v) - Pt , N(u)>                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:      To compute its cnvex hull.					     *
*   NCrv:    Noraml crv at Pt.						     *
*   Crvs:       To compute their convex hull.                                *
*   Lines:      Boundary lines of convex hull.                               *
*   FineNess:  Of numeric search for the zero set (for surface subdivision). *
*	       A positive value (10 is a good start).			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: ZeroSet Contours                                      *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCrvCnvxHull                                                          *
*****************************************************************************/
static IPPolygonStruct *SymbCHPtTestZeroSetAux(CagdPType Pt,
					       CagdCrvStruct *NCrv,
					       CagdCrvStruct *Crvs,
					       CagdCrvStruct *Lines,
					       CagdRType FineNess)
{
    IPPolygonStruct CntrHead, *CntrEnd;
    CagdCrvStruct TCrvs, *VCrv, *TCrv, *TCrvEnd, *Line;
    CagdSrfStruct *Srf;

#   ifdef TEST_PERF_CH2_PREPROC
	CagdRType  Time, PreTime;

	PreTime = IritCPUTime(FALSE);
#   endif /* TEST_PERF_CH2_PREPROC */

    TCrvs.Pnext = NULL;
    TCrvEnd = &TCrvs;
    for (TCrv = Crvs; TCrv; TCrv = TCrv -> Pnext) {
	TCrvEnd -> Pnext = CnvrtBspline2BezierKnotCrv(TCrv);
	IRIT_LIST_LAST_ELEM(TCrvEnd);
    }

    CntrHead.Pnext = NULL;
    CntrEnd = &CntrHead;

    for (VCrv = TCrvs.Pnext; VCrv; VCrv = VCrv -> Pnext) {
	Srf = SymbPtCnvxHullF3Srf(Pt, NCrv, VCrv);

    	CntrEnd -> Pnext = UserCntrSrfWithPlane(Srf, GlblPlane, FineNess);
	IRIT_LIST_LAST_ELEM(CntrEnd);

    	CagdSrfFree(Srf);
    }
    for (Line = Lines; Line; Line = Line -> Pnext) {
	Srf = SymbPtCnvxHullF3Srf(Pt, NCrv, Line);

    	CntrEnd -> Pnext = UserCntrSrfWithPlane(Srf, GlblPlane, FineNess);
	IRIT_LIST_LAST_ELEM(CntrEnd);

    	CagdSrfFree(Srf);
    }

    CagdCrvFreeList(TCrvs.Pnext);

#   ifdef DUMP_CH_CONTOURS
	/* Also a mem leak... */
	IPStdoutObject(IPGenPOLYObject(CntrHead.Pnext), FALSE);
#   endif /* DUMP_CH_CONTOURS */

#   ifdef TEST_PERF_CH2_PREPROC
	Time = IritCPUTime(FALSE);
	IRIT_INFO_MSG_PRINTF("	SymbCHPtTestZeroSetAux... %f sec\n",
			     Time - PreTime);
#   endif /* TEST_PERF_CH2_PREPROC */
    
    return CntrHead.Pnext;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate the zero level set.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:      To compute their convex hull.                                 *
*   Lines:     Boundary lines of convex hull.                                *
*   FineNess:  Of numeric search for the zero set (for surface subdivision). *
*	       A positive value (10 is a good start).			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: ZeroSet Contours                                      *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCrvCnvxHull                                                          *
*****************************************************************************/
static IPPolygonStruct *SymbCHTestZeroSetAux(CagdCrvStruct *Crvs,
					     CagdCrvStruct *Lines,
					     CagdRType FineNess)
{
    IPPolygonStruct CntrHead, *CntrEnd;
    CagdCrvStruct TCrvs, *UCrv, *VCrv, *TCrv, *TCrvEnd, *Line;
    CagdSrfStruct *Srf;

#   ifdef TEST_PERF_CH2_PREPROC
	CagdRType  Time, PreTime;

	PreTime = IritCPUTime(FALSE);
#   endif /* TEST_PERF_CH2_PREPROC */


#   ifdef DEBUG
    CagdDbg(Crvs);
    CagdDbg(Lines);
#   endif /* DEBUG */
    
    TCrvs.Pnext = NULL;
    TCrvEnd = &TCrvs;
    for (TCrv = Crvs; TCrv; TCrv = TCrv -> Pnext) {
	TCrvEnd -> Pnext = CnvrtBspline2BezierKnotCrv(TCrv);
	IRIT_LIST_LAST_ELEM(TCrvEnd);
    }

    CntrHead.Pnext = NULL;
    CntrEnd = &CntrHead;

    for (UCrv = TCrvs.Pnext; UCrv; UCrv = UCrv -> Pnext) {
	for (VCrv = TCrvs.Pnext; VCrv; VCrv = VCrv -> Pnext) {
	    Srf = SymbCrvCnvxHullF3Srf(UCrv, VCrv);
		
    	    CntrEnd -> Pnext = UserCntrSrfWithPlane(Srf, GlblPlane, FineNess);
	    IRIT_LIST_LAST_ELEM(CntrEnd);

    	    CagdSrfFree(Srf);
	}

	for (Line = Lines; Line; Line = Line -> Pnext) {
	    Srf = SymbCrvCnvxHullF3Srf(UCrv, Line);

    	    CntrEnd -> Pnext = UserCntrSrfWithPlane(Srf, GlblPlane, FineNess);
	    IRIT_LIST_LAST_ELEM(CntrEnd);

    	    CagdSrfFree(Srf);
	}
    }

    CagdCrvFreeList(TCrvs.Pnext);

#   ifdef DUMP_CH_CONTOURS
	/* Also a mem leak... */
	IPStdoutObject(IPGenPOLYObject(CntrHead.Pnext), FALSE);
#   endif /* DUMP_CH_CONTOURS */

#   ifdef TEST_PERF_CH2_PREPROC
	Time = IritCPUTime(FALSE);
	IRIT_INFO_MSG_PRINTF("	SymbCHTestZeroSetAux... %f sec\n",
				Time - PreTime);
#   endif /* TEST_PERF_CH2_PREPROC */
    
    return CntrHead.Pnext;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   After connecting Crvs (assuming no self intersection) with lines,        *
* calculate the zero level set.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:      To compute their convex hull.                                 *
*   FineNess:  Of numeric search for the zero set (for surface subdivision). *
*	       A positive value (10 is a good start).			     *
*   PtOrderedList:  Ordered points in domain.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: ZeroSet Contours                                      *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCHTestZeroSetAux                                                     *
*****************************************************************************/
static IPPolygonStruct *SymbCHTestZeroSet(CagdCrvStruct *Crvs,
					  CagdRType FineNess,
					  CagdPtStruct *PtOrderedList)
{
    IPPolygonStruct *Cntrs;
    CagdCrvStruct *LineEnd, Lines;
    CagdPtStruct *Pt;
    CagdPType P0, P1, Pa;

    if (!Crvs || !PtOrderedList)
	return NULL;

    Crvs = CHCrvsFromCHDomain(Crvs, PtOrderedList, FALSE);

#   ifdef DEBUG
     CagdDbg(Crvs);
#   endif /* DEBUG */
    
    if (!Crvs)
	return NULL;

    Lines.Pnext = NULL;
    LineEnd = &Lines;

    Pt = PtOrderedList;
    SymbCHCrvListEvalE3(Crvs, Pt -> Pt[0], Pa);
    SymbCHCrvListEvalE3(Crvs, Pt -> Pt[1], P0);

    for (Pt = Pt -> Pnext; Pt; Pt = Pt -> Pnext) {
	SymbCHCrvListEvalE3(Crvs, Pt -> Pt[0], P1);

	LineEnd -> Pnext = E2LineCrvNew(P0, P1, -2.0, -1.0);
	LineEnd = LineEnd -> Pnext;

	SymbCHCrvListEvalE3(Crvs, Pt -> Pt[1], P0);
    }

    LineEnd -> Pnext = E2LineCrvNew(P0, Pa, -2.0, -1.0);
    LineEnd -> Pnext -> Pnext = NULL;

    Cntrs = SymbCHTestZeroSetAux(Crvs, Lines.Pnext, FineNess);
    CagdCrvFreeList(Crvs);
    CagdCrvFreeList(Lines.Pnext);

#   ifdef DEBUG
    IPPolygonDbg(Cntrs);
#   endif /* DEBUG */

    return Cntrs;
}

#ifdef PARTIAL_ZEROSET_TEST
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate the zero level set for local region which is external from     *
*   one extrem line of the extrem polygon and  insert the region into        *
*   invalid domain.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       To compute their convex hull.                                *
*   Pa:         One extrem Point.                                            *
*   Pb:         The other extrem Point.                                      *
*   FineNess:  Of numeric search for the zero set (for surface subdivision). *
*	       A positive value (10 is a good start).			     *
*   PtList:  Points in domain.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: ZeroSet Contours                                      *
*                                                                            *
* SEE ALSO:                                                                  *
*   SymbCHTestZeroSetAux                                                     *
*****************************************************************************/
static IPPolygonStruct *SymbCHTestZeroSetWithExtremLine(CagdCrvStruct *Crvs,
							CagdPType Pa,
							CagdPType Pb,
							CagdRType FineNess,
							CagdPtStruct *PtList)
{
    IPPolygonStruct *Cntrs;
    CagdCrvStruct *Crv, *LineEnd, Lines;
    CagdPType P0, P1;

    if (!Crvs || !PtList)
	return NULL;

    Crvs = CHCrvsFromCHDomain(Crvs, PtList, FALSE);

    if (!Crvs)
	return NULL;

    Lines.Pnext = NULL;
    LineEnd = &Lines;

    LineEnd -> Pnext = E2LineCrvNew(Pa, Pb, -2.0, -1.0);
    LineEnd = LineEnd -> Pnext;

    IRIT_PT_COPY(P0, Pa);

    for (Crv = Crvs; Crv; Crv = Crv->Pnext) {
	CagdCoerceToE3(P1, Crv -> Points, 0, Crv -> PType);

	LineEnd -> Pnext = E2LineCrvNew(P0, P1, -2.0, -1.0);
	LineEnd = LineEnd -> Pnext;

	CagdCoerceToE3(P0, Crv -> Points, Crv -> Length - 1, Crv -> PType);
    }

    LineEnd -> Pnext = E2LineCrvNew(P0, Pb, -2.0, -1.0);
    LineEnd -> Pnext -> Pnext = NULL;

    Cntrs = SymbCHTestZeroSetAux(Crvs, Lines.Pnext, FineNess);
    CagdCrvFreeList(Crvs);
    CagdCrvFreeList(Lines.Pnext);

    return Cntrs;
}
#endif /* PARTIAL_ZEROSET_TEST */

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Find out the open-end points from the domain 'PtList'		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:     To compute their convex hull.                                  *
*   PtList:   Points in domain.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *: Open-end points.                                         *
*									     *
*****************************************************************************/
static CagdPtStruct *SymbCHCrvOpenPt(CagdCrvStruct *Crvs, CagdPtStruct *PtList)
{
    CagdPtStruct *Pt, *NewPt,
	*OpenPt = NULL;
    CagdCrvStruct *Crv;
    CagdRType TMin, TMax;
    CagdPType Point;

    for (Crv = Crvs; Crv; Crv = Crv -> Pnext) {
	CagdCrvDomain(Crv, &TMin, &TMax);

	for (Pt = PtList; Pt; Pt = Pt -> Pnext) {
	    if (Pt -> Pt[0] <= TMin && TMin <= Pt -> Pt[1]) {
		CagdCoerceToE3(Point, Crv -> Points, 0, Crv -> PType);

#		ifdef DEBUG
	            IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
		        printf("OpenPt1 [%lf %lf]\n", Point[0], Point[1]);
		    }
#		endif /* DEBUG */

		NewPt = CagdPtNew();
		NewPt -> Pt[0] = TMin;
		NewPt -> Pt[1] = Point[0];
		NewPt -> Pt[2] = Point[1];
		IRIT_LIST_PUSH(NewPt, OpenPt);
		break;
	    }
	}

	if (!CagdIsClosedCrv(Crv) ) {
	    for (Pt = PtList; Pt; Pt = Pt -> Pnext) {
		if (Pt -> Pt[0] <= TMax && TMax <= Pt -> Pt[1]) {
		    CagdCoerceToE3(Point, Crv -> Points,
				   Crv -> Length - 1, Crv -> PType);

#		    ifdef DEBUG
	                IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
			    printf("OpenPt2 [%lf %lf]\n", Point[0], Point[1]);
			}
#		    endif /* DEBUG */

		    NewPt = CagdPtNew();
		    NewPt -> Pt[0] = TMax;
		    NewPt -> Pt[1] = Point[0];
		    NewPt -> Pt[2] = Point[1];
		    IRIT_LIST_PUSH(NewPt, OpenPt);
		    break;
		}
	    }
	}
    }

    return OpenPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if 'Pts' are out of the convex hull defined by			     *
*   the domain 'PtOrderedList'						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pts:       to be tested.						     *
*   FineNess:  Of numeric search for the zero set (for surface subdivision). *
*	       A positive value (10 is a good start).			     *
*   PtOrderedList:  Ordered points in domain.				     *
*   Crvs:       To compute their convex hull.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*                                                                            *
*****************************************************************************/
static CagdPtStruct *SymbCHTestOpenPoints(CagdPtStruct *Pts,
					  CagdRType FineNess,
					  CagdPtStruct *PtOrderedList,
					  CagdCrvStruct *Crvs)
{
    CagdPtStruct *Pt, *Pt1, *NewPt, *TestPt = NULL, *OpenPt = NULL;
    CagdCrvStruct *LineEnd, Lines, *NCrv;
    CagdPType P0, P1, Pa, Center;
    int Cnt;

#   ifdef DEBUG
        IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
	    printf("SymbCHTestOpenPoints...\n");
	}
#   endif /* DEBUG */

    for (Pt = Pts; Pt; Pt = Pt -> Pnext) {
	for (Pt1 = PtOrderedList; Pt1; Pt1 = Pt1 -> Pnext)
	    if (Pt1 -> Pt[0] <= Pt -> Pt[0] && Pt -> Pt[0] <= Pt1 -> Pt[1])
		break;

        if (!Pt1) {
	    NewPt = CagdPtCopy(Pt);
	    IRIT_LIST_PUSH(NewPt, TestPt);
	}
    }

    if (!TestPt)
	return NULL;

    Crvs = CHCrvsFromCHDomain(Crvs, PtOrderedList, FALSE);

    if (!Crvs)
	return NULL;

    Lines.Pnext = NULL;
    LineEnd = &Lines;

    Pt = PtOrderedList;
    SymbCHCrvListEvalE3(Crvs, Pt -> Pt[0], Pa);
    SymbCHCrvListEvalE3(Crvs, Pt -> Pt[1], P0);

    IRIT_PT_SET(Center, 0.0, 0.0, 0.0);
    Cnt = 0;

    for (Pt = Pt -> Pnext; Pt; Pt = Pt -> Pnext) {
	SymbCHCrvListEvalE3(Crvs, Pt -> Pt[0], P1);

	LineEnd -> Pnext = E2LineCrvNew(P0, P1, -2.0, -1.0);
	LineEnd = LineEnd -> Pnext;
	IRIT_PT_ADD(Center, Center, P0); Cnt++;
	IRIT_PT_ADD(Center, Center, P1); Cnt++;

	SymbCHCrvListEvalE3(Crvs, Pt -> Pt[1], P0);
    }

    LineEnd -> Pnext = E2LineCrvNew(P0, Pa, -2.0, -1.0);
    LineEnd -> Pnext -> Pnext = NULL;
    IRIT_PT_ADD(Center, Center, P0); Cnt++;
    IRIT_PT_ADD(Center, Center, Pa); Cnt++;
    IRIT_PT_SCALE(Center, 1.0/Cnt);

    NCrv = BspCrvNew(3, 2, CAGD_PT_E2_TYPE);
    NCrv -> KnotVector[0] = NCrv -> KnotVector[1] = 0.0;
    NCrv -> KnotVector[2] = 0.5;
    NCrv -> KnotVector[3] = NCrv -> KnotVector[4] = 1.0;
    NCrv -> Points[1][0] =  1.0;
    NCrv -> Points[2][0] =  -IRIT_EPS;
    NCrv -> Points[1][1] =  0.0;
    NCrv -> Points[2][1] =  1.0;
    NCrv -> Points[1][2] = -1.0;
    NCrv -> Points[2][2] =  -IRIT_EPS;

    for (Pt = TestPt; Pt; Pt = Pt -> Pnext) {
	CagdPType Point;
	CagdPtStruct *PtList;
	IPPolygonStruct *Cntrs;

	IRIT_PT_SET(Point, Pt -> Pt[1], Pt -> Pt[2], 0.0);

	Cntrs = SymbCHPtTestZeroSetAux(Point, NCrv, Crvs,
				       Lines.Pnext, FineNess);

        PtList = CagdPtNew();
	PtList -> Pt[0] = 0.0;
	PtList -> Pt[1] = 1.0;
        ProcessContours(Cntrs, &PtList);
	IPFreePolygonList(Cntrs);
	ConnectDomainEps(&PtList, ADJUST_PARAM_EPS);
	DeleteSmallDomainEps(&PtList, IRIT_EPS);

#	ifdef DUMP_CH_DOMAINS
	IRIT_INFO_MSG("OpenPt Domain\n");
	for (Pt1 = PtList; Pt1 != NULL; Pt1 = Pt1 -> Pnext)
            IRIT_INFO_MSG_PRINTF("OpenPt domain at [%.16lf : %.16lf]\n",
				 Pt1 -> Pt[0], Pt1 -> Pt[1]);
#	endif /* DUMP_CH_DOMAINS */

	if (PtList) {
	    CagdRType *P, MaxLen,
		T = -IRIT_INFNTY;
	    CagdPType N, C;

	    for (MaxLen = 0.0, Pt1 = PtList; Pt1; Pt1 = Pt1 -> Pnext)  {
		if (Pt1 -> Pt[1] - Pt1 -> Pt[0] > MaxLen) {
		    MaxLen = Pt1 -> Pt[1] - Pt1 -> Pt[0];
		    T = (Pt1 -> Pt[0] + Pt1 -> Pt[1]) * 0.5;
		}
	    }

	    P = CagdCrvEval(NCrv, T);
	    CagdCoerceToE3(N, &P, -1, NCrv -> PType);
	    IRIT_PT_NORMALIZE(N);

#	    ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugL2CHPreproc) {
		    printf("N = (%lf %lf)\n", N[0], N[1]);
		}
#	    endif /* DEBUG */

	    IRIT_VEC_SUB(C, Point, Center);
	    if (IRIT_DOT_PROD(N, C) < 0.0) {
		IRIT_VEC_SCALE(N, -1.0);
	    }

	    NewPt = CagdPtCopy(Pt);
	    Pt -> Pt[1] = N[0];
	    Pt -> Pt[2] = N[1];
	    IRIT_LIST_PUSH(NewPt, OpenPt);

	    CagdPtFreeList(PtList);
	}

    }

    CagdCrvFree(NCrv);
    CagdCrvFreeList(Lines.Pnext);
    CagdCrvFreeList(Crvs);
    CagdPtFreeList(TestPt);

    return OpenPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the convex hull of C0 freeform planar curves, in the            *
* XY plane.  The convex hull is computed by symbolically isolating the non   *
* negative set (in t) of:						     *
*                                                                            *
*		C'(t) x (C(r) - C(t)) >= 0,   for all r.		     *
*                                                                            *
* Note the above equation yields a scalar value since C(t) is planar. The    *
* resulting set in t contains all the subdomain in C(t) that is on the       *
* convex hull of C(t).  Connecting these pieces with straight lines yeilds   *
* the final convex hull curve.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       To compute their convex hull.                                *
*   FineNess:  Of numeric search for the zero set (for surface subdivision). *
*	       A positive value (10 is a good start).			     *
*   NDir: the number of  directions for trivial Rejection (8,16 is a good)   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   A curve representing the convex hull of Crvs.         *
*****************************************************************************/
CagdCrvStruct *SymbCrvListCnvxHullVaDir(CagdCrvStruct *Crvs,
					CagdRType FineNess,
					int NDir)
{
    CagdPtStruct *PtExtm, *PtList, *InCHDomain, *Pt, *PtE, PtS,
	*InOrderedCHDomain, *OpenPt, OpenPts;
    IPPolygonStruct *Cntrs;
    CagdCrvStruct *TCrv, *CHCrv, *CrvDomainsInCH;
    CagdVType Pa, Pb, Na, Nb;

#   ifdef TEST_PERF_CH_PREPROC
        CagdRType Time, PreTime, LocalPreTime,
	    TrivialRejectTime = 0,
	    ZeroSetTime = 0;

        PreTime = IritCPUTime(FALSE);

        IRIT_INFO_MSG_PRINTF(
                "SymbCrvListCnvxHullVaDir(Crv, FinNess[%lf], NDir[%d])\n",
                FineNess, NDir);
#   endif /* TEST_PERF_CH_PREPROC */

    Crvs = SymbCHCrvListInit(Crvs);

#   ifdef TEST_PERF_CH_PREPROC
        LocalPreTime = IritCPUTime(FALSE);
#   endif /* TEST_PERF_CH_PREPROC */

    CrvExtremPolygon(Crvs, NDir, &PtExtm);

#   ifdef TEST_PERF_CH_PREPROC
        Time = IritCPUTime(FALSE);
	TrivialRejectTime += Time - LocalPreTime;
#   endif /* TEST_PERF_CH_PREPROC */

    InCHDomain = NULL;
    Pt = PtExtm;
    IRIT_LIST_LAST_ELEM(Pt);
    SymbCHCrvListEvalE3(Crvs, Pt -> Pt[0], Pa);
    IRIT_VEC_SET(Na, Pt -> Pt[1], Pt -> Pt[2], 0.0);

    OpenPts.Pnext = NULL;
    OpenPt = &OpenPts;

    for (PtE = PtExtm; PtE; PtE = PtE -> Pnext) {
	SymbCHCrvListEvalE3(Crvs, PtE -> Pt[0], Pb);
	IRIT_VEC_SET(Nb, PtE -> Pt[1], PtE -> Pt[2], 0.0);

#   ifdef TEST_PERF_CH_PREPROC
	LocalPreTime = IritCPUTime(FALSE);
#   endif /* TEST_PERF_CH_PREPROC */

	PtList = NULL;
	CrvExternalDomainFromLine(Crvs, Pa, Pb, Na, Nb, &PtList);

#   ifndef NO_OPEN_PT_TEST
        OpenPt -> Pnext = SymbCHCrvOpenPt(Crvs, PtList);
        IRIT_LIST_LAST_ELEM(OpenPt);
#   endif /* NO_OPEN_PT_TEST */


#   ifdef TEST_PERF_CH_PREPROC
	Time = IritCPUTime(FALSE);
	TrivialRejectTime += Time - LocalPreTime;
#   endif /* TEST_PERF_CH_PREPROC */

#   ifdef DUMP_CH_DOMAINS
	for (Pt = PtList; Pt != NULL; Pt = Pt -> Pnext)
	    IRIT_INFO_MSG_PRINTF("Pre Valid domain at [%.16lf : %.16lf]\n",
				 Pt -> Pt[0], Pt -> Pt[1]);
#   endif /* DUMP_CH_DOMAINS */

#   ifdef PARTIAL_ZEROSET_TEST
#	ifdef TEST_PERF_CH_PREPROC
	    LocalPreTime = IritCPUTime(FALSE);
#	endif /* TEST_PERF_CH_PREPROC */

	ConnectDomainEps(&PtList, ADJUST_PARAM_EPS);
	DeleteSmallDomainEps(&PtList, IRIT_EPS);
	Cntrs = SymbCHTestZeroSetWithExtremLine(Crvs, Pa, Pb, FineNess, PtList);
	ProcessContours(Cntrs, &PtList);
	IPFreePolygonList(Cntrs);


#       ifdef TEST_PERF_CH_PREPROC
            Time = IritCPUTime(FALSE);
	    ZeroSetTime += Time - LocalPreTime;
#	endif /* TEST_PERF_CH_PREPROC */
#   endif /* PARTIAL_ZEROSET_TEST */

	for (Pt = PtList; Pt; Pt = Pt -> Pnext)
	    InsertValidDomainIntoList(&InCHDomain, Pt -> Pt[0], Pt -> Pt[1]);
	CagdPtFreeList(PtList);

	IRIT_PT_COPY(Pa, Pb);
	IRIT_VEC_COPY(Na, Nb);
    }

#   if !defined(PARTIAL_ZEROSET_TEST) && !defined(NO_GLOBAL_ZEROSET_TEST)
#	ifdef TEST_PERF_CH_PREPROC
            LocalPreTime = IritCPUTime(FALSE);
#	endif /* TEST_PERF_CH_PREPROC */

	IncludeExtremePoints(&InCHDomain, PtExtm, Crvs);
	ConnectDomainEps(&InCHDomain, ADJUST_PARAM_EPS);
	DeleteSmallDomainEps(&InCHDomain, IRIT_EPS);
	InOrderedCHDomain =
	    SortValidDomains(CagdPtCopyList(InCHDomain), PtExtm, Crvs);
	Cntrs = SymbCHTestZeroSet(Crvs, FineNess, InOrderedCHDomain);
	ProcessContours(Cntrs, &InCHDomain);
	IPFreePolygonList(Cntrs);
	CagdPtFreeList(InOrderedCHDomain);

#	ifdef TEST_PERF_CH_PREPROC
            Time = IritCPUTime(FALSE);
	    ZeroSetTime += Time - LocalPreTime;
#	endif /* TEST_PERF_CH_PREPROC */
#   endif /* !PARTIAL_ZEROSET_TEST && !NO_GLOBAL_ZEROSET_TEST */

    IncludeExtremePoints(&InCHDomain, PtExtm, Crvs);
    ConnectDomainEps(&InCHDomain, ADJUST_PARAM_EPS);
    DeleteSmallDomainEps(&InCHDomain, IRIT_EPS);

#   ifndef NO_OPEN_END_PT_TEST

    InOrderedCHDomain =
	SortValidDomains(CagdPtCopyList(InCHDomain), PtExtm, Crvs);
    OpenPt = SymbCHTestOpenPoints(OpenPts.Pnext,
			FineNess, InOrderedCHDomain, Crvs);
    CagdPtFreeList(InOrderedCHDomain);
    CagdPtFreeList(OpenPts.Pnext);

    Pt = PtExtm;
    IRIT_LIST_LAST_ELEM(Pt);
    OpenPts.Pnext = OpenPt;

    if (OpenPt && OpenPt -> Pnext) {
	CagdPtStruct *PtQ, *PtQEnd, *Top;
	int Cnt;

	for (Cnt = 0, PtQ = OpenPt; PtQ; PtQ = PtQ -> Pnext, Cnt++);
	PtQ = PtQEnd = OpenPt;
	IRIT_LIST_LAST_ELEM(PtQEnd);

	OpenPts.Pnext = NULL;
	OpenPt = &OpenPts;

	while (Cnt--) {
	    CagdPtStruct
	        *OpenInCHDomain = CagdPtCopyList(InCHDomain);

	    PtQEnd -> Pnext = NULL;
	    IncludeExtremePoints(&OpenInCHDomain, PtQ -> Pnext, Crvs);
	    ConnectDomainEps(&OpenInCHDomain, ADJUST_PARAM_EPS);
	    DeleteSmallDomainEps(&OpenInCHDomain, IRIT_EPS);

	    Pt -> Pnext = PtQ -> Pnext;
	    InOrderedCHDomain = SortValidDomains(OpenInCHDomain, PtExtm, Crvs);
	    Top = CagdPtCopy(PtQ);
	    Top -> Pnext = NULL;
            OpenPt -> Pnext = SymbCHTestOpenPoints(Top, FineNess,
				InOrderedCHDomain, Crvs);
	    IRIT_LIST_LAST_ELEM(OpenPt);
	    CagdPtFree(Top);
	    CagdPtFreeList(InOrderedCHDomain);

	    PtQEnd -> Pnext = PtQ;
	    PtQ = PtQ -> Pnext;
	    PtQEnd = PtQEnd -> Pnext;

	}

	PtQEnd -> Pnext = NULL;
	CagdPtFreeList(PtQ);
    }

    if (OpenPts.Pnext) {
	IncludeExtremePoints(&InCHDomain, OpenPts.Pnext, Crvs);
	ConnectDomainEps(&InCHDomain, ADJUST_PARAM_EPS);
	DeleteSmallDomainEps(&InCHDomain, IRIT_EPS);
	Pt -> Pnext = OpenPts.Pnext;
    }

#   endif /* NO_OPEN_END_PT_TEST */

#   ifdef DUMP_CH_DOMAINS
        for (Pt = InCHDomain; Pt != NULL; Pt = Pt -> Pnext)
            IRIT_INFO_MSG_PRINTF("Valid domain at [%.16lf : %.16lf]\n",
				 Pt -> Pt[0], Pt -> Pt[1]);
#   endif /* DUMP_CH_DOMAINS */

    InOrderedCHDomain = SortValidDomains(InCHDomain, PtExtm, Crvs);

#   ifdef DUMP_CH_DOMAINS
    for (Pt = InOrderedCHDomain; Pt != NULL; Pt = Pt -> Pnext)
	IRIT_INFO_MSG_PRINTF("Sorted Valid domain at [%.16lf : %.16lf]\n",
				Pt -> Pt[0], Pt -> Pt[1]);
#   endif /* DUMP_CH_DOMAINS */

    CrvDomainsInCH = CHCrvsFromCHDomain(Crvs, InOrderedCHDomain, TRUE);

    CagdCrvFreeList(Crvs);
    CagdPtFreeList(PtExtm);
    CagdPtFreeList(InOrderedCHDomain);

#   ifdef NO_MERGE_CH
        CHCrv = CrvDomainsInCH;
#   else  /* NO_MERGE_CH */
        CHCrv = CagdMergeCrvList(CrvDomainsInCH, TRUE);
	if ( !CagdIsClosedCrv(CHCrv) ) {
	    CagdCoerceToE3(PtS.Pt, CHCrv -> Points, 0, CHCrv -> PType);
	    TCrv = CagdMergeCrvPt(CHCrv, &PtS);
	    CagdCrvFree(CHCrv);
	    CHCrv = TCrv;
	}
	CagdCrvFreeList(CrvDomainsInCH);
#   endif /* NO_MERGE_CH */

#   ifdef TEST_PERF_CH_PREPROC
        Time = IritCPUTime(FALSE);
        IRIT_INFO_MSG_PRINTF("Total SymbCrvListCnvxHullVaDir... %f sec\n",
			     Time - PreTime);
        IRIT_INFO_MSG_PRINTF("	TrivialRejection... %f sec\n",
			     TrivialRejectTime);
        IRIT_INFO_MSG_PRINTF("	ZeroSetTest... %f sec\n", ZeroSetTime);
#   endif /* TEST_PERF_CH_PREPROC */

    return CHCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the convex hull of C1 freeform planar curves, in the            M
* XY plane.  The convex hull is computed by symbolically isolating the non   M
* negative set (in t) of:						     M
*                                                                            M
*		C'(t) x (C(r) - C(t)) >= 0,   for all r.		     V
*                                                                            M
* Note the above equation yields a scalar value since C(t) is planar. The    M
* resulting set in t contains all the subdomain in C(t) that is on the       M
* convex hull of C(t).  Connecting these pieces with straight lines yeilds   M
* the final convex hull curve.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:       To compute their convex hull.                                M
*   FineNess:  Of numeric search for the zero set (for surface subdivision). M
*	       A positive value (10 is a good start).			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve representing the convex hull of Crvs.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPtTangents, SymbCrvDiameter                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvListCnvxHull                                                      M
*****************************************************************************/
CagdCrvStruct *SymbCrvListCnvxHull(CagdCrvStruct *Crvs, CagdRType FineNess)
{
    return SymbCrvListCnvxHullVaDir(Crvs, FineNess, 8);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the convex hull of a C1 freeform planar curve, in the           M
* XY plane.  The convex hull is computed by symbolically isolating the non   M
* negative set (in t) of:						     M
*                                                                            M
*		C'(t) x (C(r) - C(t)) >= 0,   for all r.		     V
*                                                                            M
* Note the above equation yields a scalar value since C(t) is planar. The    M
* resulting set in t contains all the subdomain in C(t) that is on the       M
* convex hull of C(t).  Connecting these pieces with straight lines yeilds   M
* the final convex hull curve.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute its convex hull.                                   M
*   FineNess:  Of numeric search for the zero set (for surface subdivision). M
*	       A positive value (10 is a good start).			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve representing the convex hull of Crv.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPtTangents, SymbCrvDiameter                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCnvxHull                                                          M
*****************************************************************************/
CagdCrvStruct *SymbCrvCnvxHull(const CagdCrvStruct *Crv, CagdRType FineNess)
{
    CagdCrvStruct *TCrv, *RetCrv;

    TCrv = CagdCrvCopy(Crv);
    TCrv -> Pnext = NULL;

    RetCrv =  SymbCrvListCnvxHullVaDir(TCrv, FineNess, 8);

    CagdCrvFree(TCrv);

    return RetCrv;
}
