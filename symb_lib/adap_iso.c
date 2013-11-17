/******************************************************************************
* Adap_iso.c - adaptive isoline surface extraction algorithm.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 92.					      *
******************************************************************************/

#include "allocate.h"
#include "geom_lib.h"
#include "symb_loc.h"

#define DEBUG_ADAP_ISO_RECT_RGNS

#define SIMILAR_PARAM_VAL	1e-4
#define OFST_SAMPLING_RATE	30
#define OFST_CRV_ORDER		3
#define OFST_MAX_ITERS		50
#define OFST_EPS_ITERS		1e-6

#define CONVEX_BLEND(v1, v2, b1, b2, t) { \
		CagdRType \
		    Blend = -b1 / ( b2 - b1 ); \
		t = v2 * Blend + v1 * (1.0 - Blend); \
	    }

typedef void (*SymbAdapIsoExtractRRFuncType)(CagdRType,
					     CagdRType,
					     CagdRType,
					     CagdRType);

IRIT_STATIC_DATA int
    GlblMinSubdivLevel = 1;
IRIT_STATIC_DATA CagdSrfDirType
    GlblAdapIsoRRDir = CAGD_CONST_U_DIR;
IRIT_STATIC_DATA CagdRType
    GlblAdapIsoWeightPtWidth = 0.1,
    GlblAdapIsoWeightPtScale = 0.0,
    GlblAdapIsoRRDt = 1.0;
IRIT_STATIC_DATA CagdPType
    GlblAdapIsoWeightPt = { 0.0, 0.0, 0.0 };
IRIT_STATIC_DATA IPPolygonStruct
    *GlblAdapIsoRRPls = NULL;
IRIT_STATIC_DATA const CagdSrfStruct
    *GlblAdapIsoRRSrf = NULL;
IRIT_STATIC_DATA SymbAdapIsoExtractRRFuncType
    GlblAdapIsoRRFuncCB = NULL;

static CagdCrvStruct *ChainAdapIsoCurves(CagdCrvStruct *MidCrv,
					 CagdCrvStruct *AdapIso1,
					 CagdCrvStruct *AdapIso2);
static CagdCrvStruct *ChainAdapIsoCurves2(CagdCrvStruct *MidCrv1,
					  CagdCrvStruct *MidCrv2,
					  CagdCrvStruct *AdapIso1,
					  CagdCrvStruct *AdapIso2,
					  CagdCrvStruct *AdapIso3);
static CagdCrvStruct *SymbAdapIsoExtractAux(int Level,
					    const CagdSrfStruct *Srf,
					    const CagdSrfStruct *NSrf,
					    SymbAdapIsoDistSqrFuncType
					                      AdapIsoDistFunc,
					    CagdCrvStruct *Crv1,
					    CagdCrvStruct *NCrv1,
					    CagdCrvStruct *Crv2,
					    CagdCrvStruct *NCrv2,
					    CagdRType Crv1Param,
					    CagdRType Crv2Param,
					    CagdSrfDirType Dir,
					    CagdRType AITol,
					    CagdBType FullIso,
					    CagdBType SinglePath);
static CagdCrvStruct *CopyRegionFromCrv(const CagdCrvStruct *Crv,
					CagdRType TMin,
					CagdRType TMax);
static CagdRType ComputeMidParam(CagdRType Param1,
				 CagdRType Param2,
				 CagdSrfDirType Dir,
				 const CagdSrfStruct *Srf);

static IPPolygonStruct *SymbAdapIsoExtractRRGenRects(const CagdCrvStruct *Crv1,
						     const CagdCrvStruct *Crv2,
						     CagdRType Dt);
static void SymbAdapIsoGenerateRectRgns(CagdRType Crv1Param,
					CagdRType Crv2Param,
					CagdRType TMin,
					CagdRType TMax);
static CagdCrvStruct *AdapIsoDistWeightPt(int Level,
					  CagdCrvStruct *Crv1,
					  CagdCrvStruct *NCrv1,
					  CagdCrvStruct *Crv2,
					  CagdCrvStruct *NCrv2);
static CagdRType AdapIsoDistWeightPtEval(CagdPType Pt);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets minimum level of subdivision forced in the adaptive iso extraction.   M
*                                                                            *
* PARAMETERS:                                                                M
*   MinLevel:    At least that many subdivision will occur.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSetAdapIsoExtractMinLevel, adaptive isocurves                        M
*****************************************************************************/
void SymbSetAdapIsoExtractMinLevel(int MinLevel)
{
    GlblMinSubdivLevel = MinLevel;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a valid coverage set of isolines from the given surface in the    M
* given direction and epsilon.						     M
*   If FullIso is TRUE, all extracted isocurves are spanning the entire      M
* parametric domain.							     M
*   If SinglePath is TRUE, the entire coverage is going to be a single       M
* curve.								     M
*   If NSrf != NULL, every second curve will be a vector field curve         M
* representing the unnormalized normal for the previous Euclidean curve.     M
* This mode disable the SinglePath mode.				     M
*   See also function SymbSetAdapIsoExtractMinLevel.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute adaptive isocurve coverage form                   M
*   NSrf:       Normal vector field defining the normals of Srf.             M
*   AdapIsoDistFunc: Optional function to invoke with the two adjacent       M
*		isoparametric curves of the coverage to evaluate the         M
*		distance between them.					     M
*   Dir:        Direction of adaptive isocurve extraction. Either U or V.    M
*   Eps:        Tolerance of adaptive isocurve cuverage. For every point P   M
*               on Srf there will be a point Q in one of the extracted       M
*               isocurves such the |P - Q| < Eps.			     M
*   FullIso:    Do we want all isocurves to span the entire domain?          M
*   SinglePath: Do we want a single curve through them all?                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of curves representing the computed adaptive    M
*                     isocurve coverage for surface Srf. If normal field,    M
*                     NSrf, is prescribed, normal curves are concatenated    M
*                     alternatingly in this list.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbAdapIsoExtractRectRgns, SymbAdapIsoSetWeightPt                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbAdapIsoExtract, adaptive isocurves                                   M
*****************************************************************************/
CagdCrvStruct *SymbAdapIsoExtract(const CagdSrfStruct *Srf,
				  const CagdSrfStruct *NSrf,
				  SymbAdapIsoDistSqrFuncType AdapIsoDistFunc,
				  CagdSrfDirType Dir,
				  CagdRType Eps,
				  CagdBType FullIso,
				  CagdBType SinglePath)
{
    CagdRType Crv1Param, Crv2Param;
    CagdCrvStruct *Crv1, *Crv2, *NCrv1, *NCrv2, *AllAdapIso, *TCrv;
    CagdSrfStruct *CpSrf;

    if (NSrf != NULL)
	SinglePath = FALSE;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = CpSrf = CagdCnvrtBzr2BspSrf(Srf);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    CpSrf = NULL;
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_WRONG_SRF);
	    return NULL;
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Crv1Param = Srf -> GType == CAGD_SBSPLINE_TYPE ? 
		Srf -> UKnotVector[0] + IRIT_UEPS : IRIT_UEPS;
	    Crv2Param = Srf -> GType == CAGD_SBSPLINE_TYPE ? 
		Srf -> UKnotVector[Srf -> ULength +
				   Srf -> UOrder - 1] - IRIT_UEPS :
		1.0 - IRIT_UEPS;
	    break;
	case CAGD_CONST_V_DIR:
	    Crv1Param = Srf -> GType == CAGD_SBSPLINE_TYPE ? 
		Srf -> VKnotVector[0] + IRIT_UEPS : IRIT_UEPS;
	    Crv2Param = Srf -> GType == CAGD_SBSPLINE_TYPE ? 
		Srf -> VKnotVector[Srf -> VLength +
				   Srf -> VOrder - 1] - IRIT_UEPS :
		1.0 - IRIT_UEPS;
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_DIR_NOT_CONST_UV);
	    Crv1Param = 0.0;
	    Crv2Param = 1.0;
	    break;
    }

    Crv1 = CagdCrvFromSrf(Srf, Crv1Param, Dir);
    AttrSetRealAttrib(&Crv1 -> Attr, "IsoParam", Crv1Param);
    AttrSetIntAttrib(&Crv1 -> Attr, "Level", 0);
    Crv2 = CagdCrvFromSrf(Srf, Crv2Param, Dir);
    AttrSetRealAttrib(&Crv2 -> Attr, "IsoParam", Crv2Param);
    AttrSetIntAttrib(&Crv2 -> Attr, "Level", 0);

    if (NSrf != NULL) {
	NCrv1 = CagdCrvFromSrf(NSrf, Crv1Param, Dir);
	NCrv2 = CagdCrvFromSrf(NSrf, Crv2Param, Dir);
    }
    else
	NCrv1 = NCrv2 = NULL;

    /* Compute the adaptive iso curves. */
    GlblAdapIsoRRSrf = Srf;
    AllAdapIso = SymbAdapIsoExtractAux(1, Srf, NSrf, AdapIsoDistFunc,
				       Crv1, NCrv1, Crv2, NCrv2,
				       Crv1Param, Crv2Param,
				       Dir, Eps * Eps, FullIso, SinglePath);

    /* Chain first and last iso curves that always span the entire domain. */
    if (AllAdapIso != NULL) {
	Crv1 -> Pnext = AllAdapIso;
	TCrv = CagdListLast(AllAdapIso);
	TCrv -> Pnext = Crv2;
    }
    else
	Crv1 -> Pnext = Crv2;

    if (NSrf != NULL) {
	NCrv1 -> Pnext = Crv1 -> Pnext;
	Crv1 -> Pnext = NCrv1;
	NCrv2 -> Pnext = Crv2 -> Pnext;
	Crv2 -> Pnext = NCrv2;
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return Crv1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Chain all given curves into one linked list in order:                    *
* (AdapIso1, MidCrv, AdapIso2).					             *
*   Note all Adaptive isocurves AdapIso? can be NULL.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   MidCrv:    Interior curve.                                               *
*   AdapIso1:  Adap isocurves computed recursively between Crv1 and MidCrv.  *
*   AdapIso2:  Adap isocurves computed recursively between MidCrv and Crv2.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:     One list of chained curves.                         *
*****************************************************************************/
static CagdCrvStruct *ChainAdapIsoCurves(CagdCrvStruct *MidCrv,
					 CagdCrvStruct *AdapIso1,
					 CagdCrvStruct *AdapIso2)
{
    CagdCrvStruct *AdapIso;

    if (AdapIso1 != NULL) {
        ((CagdCrvStruct *) CagdListLast(AdapIso1)) -> Pnext = MidCrv;
	AdapIso = AdapIso1;
    }
    else
        AdapIso = MidCrv;

    MidCrv -> Pnext = AdapIso2;

    return AdapIso;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Chain all given curves into one linked list in order:                    *
* (AdapIso1, MidCrv1, AdapIso2, MidCrv2, AdapIso2).		             *
*   Note all Adaptive isocurves AdapIso? can be NULL.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   MidCrv1:   First interior curve.                                         *
*   MidCrv2:   Second interior curve.                                        *
*   AdapIso1:  Adap isocurves computed recursively between Crv1 and MidCrv1. *
*   AdapIso2:  Adap isos computed recursively between MidCrv1 and MidCrv2.   *
*   AdapIso3:  Adap isocurves computed recursively between MidCrv2 and Crv2  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:     One list of chained curves.                         *
*****************************************************************************/
static CagdCrvStruct *ChainAdapIsoCurves2(CagdCrvStruct *MidCrv1,
					  CagdCrvStruct *MidCrv2,
					  CagdCrvStruct *AdapIso1,
					  CagdCrvStruct *AdapIso2,
					  CagdCrvStruct *AdapIso3)
{
    CagdCrvStruct *AdapIso;

    if (AdapIso1 != NULL) {
        ((CagdCrvStruct *) CagdListLast(AdapIso1)) -> Pnext = MidCrv1;
	AdapIso = AdapIso1;
    }
    else
        AdapIso = MidCrv1;

    if (AdapIso2 != NULL) {
        MidCrv1 -> Pnext = AdapIso2;

	((CagdCrvStruct *) CagdListLast(AdapIso2)) -> Pnext = MidCrv2;

	MidCrv2 -> Pnext = AdapIso3;
    }
    else
        MidCrv1 -> Pnext = MidCrv2;

    return AdapIso;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* An auxiliary function of SymbAdapIsoExtract. Computes the distance square  *
* between the given two curves, extract the regions that are far than that   *
* and recursively invoke this function with the sub-curves.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Level:         Of recursion.                                             *
*   Srf:           This adaptive isocurve coverage is computed for.          *
*   NSrf:          Normal vectorfield of Srf.                                *
*   AdapIsoDistFunc: Optional function to invoke with the two adjacent       *
*		   isoparametric curves of the coverage to evaluate the      *
*		   distance between them.				     *
*   Crv1:          First curve to handle.                                    *
*   NCrv1:         Normal vector field of first curve Crv1.                  *
*   Crv2:          Second curve to handle.                                   *
*   NCrv2:         Normal vector field of second curve Crv2.                 *
*   Crv1Param:     Parameter along the other direction of Srf, of Crv1.      *
*   Crv2Param:     Parameter along the other direction of Srf, of Crv2.      *
*   Dir:           Direction of adaptive isocurve extraction.                *
*   AITol:          Tolerance of adaptive isocurve extraction.               *
*   FullIso:       Do we want all isocurves to span all parametric domain?   *
*   SinglePath:    Do we want everything in a single path?                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  A list of isocurves covering Srf to within AITol.      *
*****************************************************************************/
static CagdCrvStruct *SymbAdapIsoExtractAux(int Level,
					    const CagdSrfStruct *Srf,
					    const CagdSrfStruct *NSrf,
					    SymbAdapIsoDistSqrFuncType
					                      AdapIsoDistFunc,
					    CagdCrvStruct *Crv1,
					    CagdCrvStruct *NCrv1,
					    CagdCrvStruct *Crv2,
					    CagdCrvStruct *NCrv2,
					    CagdRType Crv1Param,
					    CagdRType Crv2Param,
					    CagdSrfDirType Dir,
					    CagdRType AITol,
					    CagdBType FullIso,
					    CagdBType SinglePath)
{
    int i, KVLen;
    CagdPointType PType;
    CagdRType Dist, *KV, LastT, DistCrv2Min, DistCrv2Max,
	**Points, *PtsW, *PtsX, LastDist,
	*Nodes = NULL;
    CagdBType CloseEnough, LastCloseEnough;
    CagdCrvStruct *TCrv, *DistCrv2,
	*AllAdapIso = NULL;

    if (AdapIsoDistFunc != NULL) { /* Call function to compute distance sqr. */
	DistCrv2 = AdapIsoDistFunc(Level, Crv1, NCrv1, Crv2, NCrv2);
    }
    else if (GlblAdapIsoWeightPtScale != 0.0) {
	DistCrv2 = AdapIsoDistWeightPt(Level, Crv1, NCrv1, Crv2, NCrv2);
    }
    else {	       /* Symbolically compute the distance square function. */
	CagdCrvStruct
	    *DiffCrv = SymbCrvSub(Crv1, Crv2);

	DistCrv2 = SymbCrvDotProd(DiffCrv, DiffCrv);
	CagdCrvFree(DiffCrv);
    }
    PType = DistCrv2 -> PType;

    /* Simple heuristic how much to refine DistCrv2: */
    KVLen = DistCrv2 -> Length * 2;
    DistCrv2Min = DistCrv2 -> KnotVector[0];
    DistCrv2Max = DistCrv2 -> KnotVector[DistCrv2 -> Order +
					 DistCrv2 -> Length - 1];
    KV = (CagdRType *) IritMalloc(sizeof(CagdRType) * KVLen);
    for (i = 0; i < KVLen; i++)
	KV[i] = DistCrv2Min + (i + 1) * (DistCrv2Max - DistCrv2Min) /
								(KVLen + 1);

    TCrv = BspCrvKnotInsertNDiff(DistCrv2, FALSE, KV, KVLen);
    IritFree(KV);
    CagdCrvFree(DistCrv2);
    DistCrv2 = TCrv;

    Nodes = CagdCrvNodes(DistCrv2);
    LastT = Nodes[0];
    Points = DistCrv2 -> Points,
    PtsW = Points[SYMB_W],
    PtsX = Points[SYMB_X],
    LastDist = (PType == CAGD_PT_E1_TYPE ? PtsX[0]
					 : (PtsX[0] / PtsW[0])) - AITol;
    LastCloseEnough = LastDist < 0.0 && GlblMinSubdivLevel <= Level;

    for (i = 1; i < DistCrv2 -> Length; i++) {
        CagdRType t;
	int KnotIndex,
	    KVLength = Crv1 -> Length + Crv1 -> Order;

	Dist = (PType == CAGD_PT_E1_TYPE ? PtsX[i]
					 : (PtsX[i] / PtsW[i])) - AITol;
	CloseEnough = Dist < 0.0 && GlblMinSubdivLevel <= Level;

	if (CloseEnough == LastCloseEnough)
	    t = Nodes[DistCrv2 -> Length - 1];
	else
	    CONVEX_BLEND(Nodes[i - 1], Nodes[i], LastDist, Dist, t);

	/* If parameter t is close "enough" to an existing knot. use it. */
	KnotIndex = BspKnotLastIndexLE(Crv1 -> KnotVector, KVLength, t);
	if (KnotIndex >= 0 &&
	    t - Crv1 -> KnotVector[KnotIndex] < SIMILAR_PARAM_VAL)
	    t = Crv1 -> KnotVector[KnotIndex];

	/* Are we generating rectangles (see SymbAdapIsoExtractRectRgns)? */
	if (GlblAdapIsoRRFuncCB != NULL &&
	    LastCloseEnough &&
	    (i == DistCrv2 -> Length - 1 || !CloseEnough)) {
	    /* Generate rectangles from LastT to t. */
	    GlblAdapIsoRRFuncCB(Crv1Param, Crv2Param, LastT,
			        i == DistCrv2 -> Length - 1 ? Nodes[i] : t);
	}

	if (CloseEnough != LastCloseEnough ||
	    (i == DistCrv2 -> Length - 1 && !LastCloseEnough)) {
	    if (t - LastT > SIMILAR_PARAM_VAL &&
		(CloseEnough ||
		 (i == DistCrv2 -> Length - 1 && !LastCloseEnough))) {
		/* We are at the end of a region that is not close enough. */
		if (SinglePath) {
		    CagdRType
		        MidParam1 = (Crv1Param * 2.0 + Crv2Param) / 3.0,
			MidParam2 = (Crv1Param + Crv2Param * 2.0) / 3.0;
		    CagdCrvStruct
		        *AdapIso1, *AdapIso2, *AdapIso3, *AdapIso,
			*MidCrv1 = CagdCrvFromSrf(Srf, MidParam1, Dir),
			*MidCrv2 = CagdCrvFromSrf(Srf, MidParam2, Dir);

		    AttrSetRealAttrib(&MidCrv1 -> Attr, "IsoParam", MidParam1);
		    AttrSetIntAttrib(&MidCrv1 -> Attr, "Level", Level);
		    AttrSetRealAttrib(&MidCrv2 -> Attr, "IsoParam", MidParam2);
		    AttrSetIntAttrib(&MidCrv1 -> Attr, "Level", Level);
    
		    if (FullIso) {
			AdapIso1 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 Crv1, NULL,
							 MidCrv1, NULL,
							 Crv1Param, MidParam1,
							 Dir, AITol,
							 FullIso, SinglePath);
			AdapIso2 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 MidCrv1, NULL,
							 MidCrv2, NULL,
							 MidParam1, MidParam2,
							 Dir, AITol,
							 FullIso, SinglePath);
			AdapIso3 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 MidCrv2, NULL,
							 Crv2, NULL,
							 MidParam2, Crv2Param,
							 Dir, AITol,
							 FullIso, SinglePath);

			AdapIso = ChainAdapIsoCurves2(MidCrv1, MidCrv2,
						      AdapIso1, AdapIso2,
						      AdapIso3);

			/* Chain these isolines to the entire set.*/
			AllAdapIso = CagdListAppend(AllAdapIso, AdapIso);

			break; /* Only one (entire domain) region! */
		    }
		    else {
			CagdCrvStruct
			    *Region1 = CopyRegionFromCrv(Crv1, LastT, t),
			    *Region2 = CopyRegionFromCrv(Crv2, LastT, t),
			    *MidRegion1 = CopyRegionFromCrv(MidCrv1, LastT, t),
			    *MidRegion2 = CopyRegionFromCrv(MidCrv2, LastT, t);

			CagdCrvFree(MidCrv1);
			CagdCrvFree(MidCrv2);

			AdapIso1 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 Region1, NULL,
							 MidRegion1, NULL,
							 Crv1Param, MidParam1,
							 Dir, AITol,
							 FullIso, SinglePath);
			AdapIso2 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 MidRegion1, NULL,
							 MidRegion2, NULL,
							 MidParam1, MidParam2,
							 Dir, AITol,
							 FullIso, SinglePath);
			AdapIso3 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 MidRegion2, NULL,
							 Region2, NULL,
							 MidParam2, Crv2Param,
							 Dir, AITol,
							 FullIso, SinglePath);

			AdapIso = ChainAdapIsoCurves2(MidRegion1, MidRegion2,
						      AdapIso1, AdapIso2,
						      AdapIso3);

			/* Chain these isolines to the entire set.*/
			AllAdapIso = CagdListAppend(AllAdapIso, AdapIso);

			CagdCrvFree(Region1);
			CagdCrvFree(Region2);
		    }
		}
		else { /* Return all the isocurves as a list of curves. */
		    CagdRType
		        MidParam = ComputeMidParam(Crv1Param, Crv2Param,
						   Dir, Srf);
		    CagdCrvStruct *AdapIso1, *AdapIso2, *AdapIso,
			*MidCrv = CagdCrvFromSrf(Srf, MidParam, Dir),
			*MidNCrv = NSrf ? CagdCrvFromSrf(NSrf, MidParam, Dir)
					: NULL;

		    AttrSetRealAttrib(&MidCrv -> Attr, "IsoParam", MidParam);
		    AttrSetIntAttrib(&MidCrv -> Attr, "Level", Level);

		    if (FullIso) {
			AdapIso1 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 Crv1, NCrv1,
							 MidCrv, MidNCrv,
							 Crv1Param, MidParam,
							 Dir, AITol,
							 FullIso, SinglePath);
			AdapIso2 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 MidCrv, MidNCrv,
							 Crv2, NCrv2,
							 MidParam, Crv2Param,
							 Dir, AITol,
							 FullIso, SinglePath);

			AdapIso = ChainAdapIsoCurves(MidCrv,
						     AdapIso1, AdapIso2);

			if (NSrf != NULL) {
			    /* Chain the normal fields after the geometry. */
			    MidNCrv -> Pnext =  MidCrv -> Pnext;
			    MidCrv -> Pnext = MidNCrv;
			}

			/* Chain these isolines to the entire set.*/
			AllAdapIso = CagdListAppend(AllAdapIso, AdapIso);

			break; /* Only one (entire domain) region! */
		    }
		    else {
			CagdCrvStruct
			    *NRegion1 = NULL,
			    *NRegion2 = NULL,
			    *MidNRegion = NULL,
			    *Region1 = CopyRegionFromCrv(Crv1, LastT, t),
			    *Region2 = CopyRegionFromCrv(Crv2, LastT, t),
			    *MidRegion = CopyRegionFromCrv(MidCrv, LastT, t);

			CagdCrvFree(MidCrv);

			if (NSrf) {
			    NRegion1 = CopyRegionFromCrv(NCrv1, LastT, t);
			    NRegion2 = CopyRegionFromCrv(NCrv2, LastT, t);
			    MidNRegion = CopyRegionFromCrv(MidNCrv, LastT, t);
			    CagdCrvFree(MidNCrv);
			}

			AdapIso1 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 Region1, NRegion1,
							 MidRegion, MidNRegion,
							 Crv1Param, MidParam,
							 Dir, AITol,
							 FullIso, SinglePath);
			AdapIso2 = SymbAdapIsoExtractAux(Level + 1, Srf, NSrf,
							 AdapIsoDistFunc,
							 MidRegion, MidNRegion,
							 Region2, NRegion2,
							 MidParam, Crv2Param,
							 Dir, AITol,
							 FullIso, SinglePath);

			AdapIso = ChainAdapIsoCurves(MidRegion,
						     AdapIso1, AdapIso2);

			if (NSrf != NULL) {
			    /* Chain the normal fields after the geometry. */
			    MidNRegion -> Pnext =  MidNRegion -> Pnext;
			    MidNRegion -> Pnext = MidNRegion;
			}

			/* Chain these isolines to the entire set.*/
			AllAdapIso = CagdListAppend(AllAdapIso, AdapIso);

			CagdCrvFree(Region1);
			CagdCrvFree(Region2);
			if (NSrf != NULL) {
			    CagdCrvFree(NRegion1);
			    CagdCrvFree(NRegion2);
			}
		    }
		}
	    }

	    /* We are at the beginning of a region not close enough. */
	    LastT = t;
	}

	LastDist = Dist;
	LastCloseEnough = CloseEnough;
    }

    CagdCrvFree(DistCrv2);
    IritFree(Nodes);

    if (SinglePath) { /* Add connecting isocurves into the existing curves. */
	/* Not implemented yet. */
    }

    return AllAdapIso;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extracts a sub region of the given curve, but also copies its attributes.*
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:        To extracts a region from.                                   *
*   TMin, TMax: Domain of region.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Extracted region.                                     *
*****************************************************************************/
static CagdCrvStruct *CopyRegionFromCrv(const CagdCrvStruct *Crv,
					CagdRType TMin,
					CagdRType TMax)
{
    CagdCrvStruct
	*Region = CagdCrvRegionFromCrv(Crv, TMin, TMax);

    IP_ATTR_FREE_ATTRS(Region -> Attr);
    Region -> Attr = IP_ATTR_COPY_ATTRS(Crv -> Attr);

    return Region;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Estimates a parameter between Param1 and Param2 in Srf in Dir. Selects   *
* interior knots before selecting an average between parameters.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Param1, Param2:  Two parameters to find in between.                      *
*   Dir:             Direction in Srf of Param1/2.                           *
*   Srf:             Surface to explore.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:       A parameter between Param1 and Param2.                  *
*****************************************************************************/
static CagdRType ComputeMidParam(CagdRType Param1,
				 CagdRType Param2,
				 CagdSrfDirType Dir,
				 const CagdSrfStruct *Srf)
{
    CagdRType t, *KV;
    int KVLen, Index1, Index2;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    KV = Srf -> UKnotVector;
	    KVLen = Srf -> UOrder + Srf -> ULength;
	    break;
	case CAGD_CONST_V_DIR:
	    KV = Srf -> VKnotVector;
	    KVLen = Srf -> VOrder + Srf -> VLength;
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_DIR_NOT_CONST_UV);
	    KV = NULL;
	    KVLen = 0;
    }

    Index1 = BspKnotLastIndexLE(KV, KVLen, Param1);
    Index2 = BspKnotLastIndexLE(KV, KVLen, Param2);

    if (Index1 < 0 || Index2 < 0 || Index1 >= KVLen || Index2 >= KVLen) {
	SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
	return -IRIT_INFNTY;
    }

    t = KV[(Index1 + Index2) >> 1];
    if (t < Param1 ||
	t > Param2 ||
	IRIT_APX_EQ(t, Param1) ||
	IRIT_APX_EQ(t, Param2))
	t = (Param1 + Param2) * 0.5;

    return t;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute adaptive rectangular regions' tiles between Crv1 and Crv2 with   *
* Dt rounded steps.						             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2: The two curves to tile rectangles between (in the common     *
*               domain.                                                      *
*   Dt:         The parametric domain step size for each rectangle.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  The computed rectangles, as UV coordinates in Srf.   *
*****************************************************************************/
static IPPolygonStruct *SymbAdapIsoExtractRRGenRects(const CagdCrvStruct *Crv1,
						     const CagdCrvStruct *Crv2,
						     CagdRType Dt)
{
    IrtRType t, TMin1, TMax1, TMin2, TMax2, TMin, TMax, Iso1, Iso2;
    IPPolygonStruct
        *Pls = NULL;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);
    TMin = IRIT_MAX(TMin1, TMin2);
    TMax = IRIT_MIN(TMax1, TMax2);
    assert(TMin < TMax);

    TMin = Dt * ((int) (TMin / Dt));
    TMax = Dt * ((int) (TMax / Dt));
    if (TMin >= TMax)
        return NULL;

    Iso1 = AttrGetRealAttrib(Crv1 -> Attr, "IsoParam");
    Iso2 = AttrGetRealAttrib(Crv2 -> Attr, "IsoParam");

    for (t = TMin; !IRIT_APX_EQ(t, TMax); ) {
        CagdRType UsedDt,
	    DtDivFactor = 1.0;
        IPVertexStruct
	    *V4 = IPAllocVertex2(NULL),
	    *V3 = IPAllocVertex2(V4),
	    *V2 = IPAllocVertex2(V3),
	    *V1 = IPAllocVertex2(V2);

	/* if we have a weighted points - derive weight. */
	if (GlblAdapIsoWeightPtScale != 0.0) {
	    CagdRType *R, W;
	    CagdPType Pt;

	    R = CagdSrfEval(GlblAdapIsoRRSrf,
			    (Iso1 + Iso2) * 0.5, IRIT_MIN(t + Dt * 0.5, TMax));
	    CagdCoerceToE3(Pt, &R, -1, GlblAdapIsoRRSrf -> PType);

	    W = AdapIsoDistWeightPtEval(Pt);
	    while (W > 4.0) {
	        W *= 0.25;
		DtDivFactor *= 0.5;
	    }
	}
	if (TMax - t < Dt * DtDivFactor)
	    UsedDt = TMax - t;
	else
	    UsedDt = Dt * DtDivFactor;

        /* Create the rectangles from t to t + Dt between Crv1 and Crv2.  */
	V1 -> Coord[0] = Iso1;
	V1 -> Coord[1] = t;

	V2 -> Coord[0] = Iso1;
	V2 -> Coord[1] = t + UsedDt;

	V3 -> Coord[0] = Iso2;
	V3 -> Coord[1] = t + UsedDt;

	V4 -> Coord[0] = Iso2;
	V4 -> Coord[1] = t;

	V1 -> Coord[2] = V2 -> Coord[2] = V3 -> Coord[2] = V4 -> Coord[2] = 0.0;

	if (GlblAdapIsoRRDir == CAGD_CONST_V_DIR) {
	    IPVertexStruct *V;

	    for (V = V1; V != NULL; V = V -> Pnext) {
		IRIT_SWAP(IrtRType, V -> Coord[0], V -> Coord[1]);
	    }
	}

	Pls = IPAllocPolygon(0, V1, Pls);

	t += UsedDt;
    }

    return Pls;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process the rectangles into the desired output format.                   *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   OrigSrf:    To compute adaptive rectangular regions' tiles for.          *
*   Srf:        Normalized OrigSrf to [0, 1]^2 domain.		             *
*   Rects:      The Rectangles as computed, in UV space of Srf.              *
*   OutputType: 0 for UV coordinates in original surfaces.		     *
*               1 for polygonal rectangles in Euclidean space.		     *
*               2 for precise curved patches in Euclidean space.	     *
*               3 for surface patches in Euclidean space.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:                                                        *
*****************************************************************************/
static IPObjectStruct *SymbAdapIsoExtractRRCreateOutput(
						const CagdSrfStruct *OrigSrf,
						const CagdSrfStruct *Srf,
						const IPPolygonStruct *Rects,
						int OutputType)
{
    int CircVLists = IPSetPolyListCirc(FALSE);
    IrtRType UMin, UMax, VMin, VMax, Du, Dv;
    IPObjectStruct *RetObj, *PTmp;
    const IPPolygonStruct *Pl;
    IPVertexStruct *V;

    IPSetPolyListCirc(CircVLists);			  /* Recover state. */

    CagdSrfDomain(OrigSrf, &UMin, &UMax, &VMin, &VMax);
    Du = UMax - UMin;
    Dv = VMax - VMin;

    switch (OutputType) {
        default:
        case 0:
	    PTmp = IPGenPOLYObject(IPCopyPolygonList(Rects));

	    /* Bring the UV values from [0, 1]^2 to real domain. */
	    if (UMin != 0.0 || UMax != 1.0 || VMin != 0.0 || VMax != 1.0) {
		for (Pl = PTmp -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
		    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		        V -> Coord[0] = UMin + V -> Coord[0] * Du;
		        V -> Coord[1] = VMin + V -> Coord[1] * Dv;
		    }

		    if (CircVLists)/* Close VLists to a loop if so desired. */
		        IPGetLastVrtx(Pl -> PVertex) -> Pnext = Pl -> PVertex;
		}
	    }

	    RetObj = IPLnkListToListObject(PTmp -> U.Pl, IP_OBJ_POLY);

	    PTmp -> U.Pl = NULL;
	    IPFreeObject(PTmp);
	    break;
        case 1:
	    PTmp = IPGenPOLYObject(IPCopyPolygonList(Rects));

	    /* Evaluate to Euclidean rectangles in 3-space. */
	    for (Pl = PTmp -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    CagdRType
		        *R = CagdSrfEval(Srf, V -> Coord[0], V -> Coord[1]);

		    CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
		}

		if (CircVLists)    /* Close VLists to a loop if so desired. */
		    IPGetLastVrtx(Pl -> PVertex) -> Pnext = Pl -> PVertex;
	    }

	    RetObj = IPLnkListToListObject(PTmp -> U.Pl, IP_OBJ_POLY);

	    PTmp -> U.Pl = NULL;
	    IPFreeObject(PTmp);
	    break;
        case 2:
	    RetObj = IPGenLISTObject(NULL);

	    /* Extract surface patches. */
	    for (Pl = Rects; Pl != NULL; Pl = Pl -> Pnext) {
	        IPVertexStruct
		    *V1 = Pl -> PVertex,
		    *V2 = V1 -> Pnext -> Pnext;
	        CagdSrfStruct
		    *TSrf = SymbComposeSrfPatch(Srf,
						V1 -> Coord,
						V1 -> Pnext -> Coord,
						V2 -> Pnext -> Coord,
						V2 -> Coord);
		CagdCrvStruct
		    **BCrvs = CagdBndryCrvsFromSrf(TSrf);

		BCrvs[0] -> Pnext = BCrvs[1];
		BCrvs[1] -> Pnext = BCrvs[2];
		BCrvs[2] -> Pnext = BCrvs[3];

		IPListObjectAppend(RetObj, IPGenCRVObject(BCrvs[0]));
	    }
	    break;
       case 3:
	    PTmp = IPGenSRFObject(NULL);

	    /* Extract surface patches. */
	    for (Pl = Rects; Pl != NULL; Pl = Pl -> Pnext) {
	        IPVertexStruct
		    *V1 = Pl -> PVertex,
		    *V2 = V1 -> Pnext -> Pnext;
	        CagdSrfStruct
		    *TSrf = SymbComposeSrfPatch(Srf,
						V1 -> Coord,
						V1 -> Pnext -> Coord,
						V2 -> Pnext -> Coord,
						V2 -> Coord);

		IRIT_LIST_PUSH(TSrf, PTmp -> U.Srfs);
	    }

	    RetObj = IPLnkListToListObject(PTmp -> U.Srfs, IP_OBJ_SURFACE);

	    PTmp -> U.Srfs = NULL;
	    IPFreeObject(PTmp);
	    break;
    }

    return RetObj; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of SymbAdapIsoExtractAux to generate rectangles.      *
*   Generate rectangles from TMin to TMax between isocurves at surface       *
* parameters Crv1Param and Crv2Param.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1Param, Crv2Param:  The isoparametric values of the adjacent curves. *
*   TMin, TMax:            The shared domain of the adjacent curves.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void					                             *
*****************************************************************************/
static void SymbAdapIsoGenerateRectRgns(CagdRType Crv1Param,
					CagdRType Crv2Param,
					CagdRType TMin,
					CagdRType TMax)
{
    CagdCrvStruct
        *Crv1 = CagdCrvFromSrf(GlblAdapIsoRRSrf, Crv1Param, GlblAdapIsoRRDir),
        *Crv2 = CagdCrvFromSrf(GlblAdapIsoRRSrf, Crv2Param, GlblAdapIsoRRDir),
        *Crv1Rgn = CagdCrvRegionFromCrv(Crv1, TMin, TMax),
        *Crv2Rgn = CagdCrvRegionFromCrv(Crv2, TMin, TMax);
    IPPolygonStruct *Pls;

#ifdef DEBUG_ADAP_ISO_RR_DUMP
    fprintf(stderr, "Params = %f  %f,  T = %f  %f\n", Crv1Param, Crv2Param, TMin, TMax);
#endif /* DEBUG_ADAP_ISO_RR_DUMP */

    AttrSetRealAttrib(&Crv1Rgn -> Attr, "IsoParam", Crv1Param);
    AttrSetRealAttrib(&Crv2Rgn -> Attr, "IsoParam", Crv2Param);
    Pls = SymbAdapIsoExtractRRGenRects(Crv1Rgn, Crv2Rgn, GlblAdapIsoRRDt);

    GlblAdapIsoRRPls = IPAppendPolyLists(Pls, GlblAdapIsoRRPls);
    
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(Crv1Rgn);
    CagdCrvFree(Crv2Rgn);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a valid coverage set of rectangular regions of roughly equal size M
* to the given surface in the given direction Dir and size Size.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To compute adaptive rectangular regions' tiles for.          M
*   Dir:        Direction of adaptive isocurve extraction. Either U or V.    M
*   Size:       Rough size of the edges of the generated rectangles.         M
*   Smoothing:  Number of low pass smoothing steps to apply, 0 to disable.   M
*   OutputType: 0 for UV coordinates in original surfaces.		     M
*               1 for polygonal rectangles in Euclidean space.		     M
*               2 for surface patches in Euclidean space.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of rectangles, as OutputType sets.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbAdapIsoExtract, SymbAdapIsoSetWeightPt                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbAdapIsoExtractRectRgns, adaptive isocurves                           M
*****************************************************************************/
IPObjectStruct *SymbAdapIsoExtractRectRgns(const CagdSrfStruct *Srf,
					   CagdSrfDirType Dir,
					   CagdRType Size,
					   int Smoothing,
					   int OutputType)
{
    IrtRType Dt, UMin, UMax, VMin, VMax,
        AccumParam = 0.0,
        AccumDist = 0.0;
    CagdSrfStruct *NewSrf;
    const CagdSrfStruct
	*OrigSrf = Srf;
    CagdCrvStruct *Crv, *AdapIso;
    IPPolygonStruct
        *AllPls = NULL;
    IPObjectStruct *RetObj, *PObj, *PTmp;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = NewSrf = CagdCnvrtBzr2BspSrf(Srf);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    NewSrf = NULL;
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_WRONG_SRF);
	    return NULL;
    }

    /* Reparameterize the domain to be [0, 1]^2. */
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    if (UMin != 0.0 || UMax != 1.0 || VMin != 0.0 || VMax != 1.0) {
        if (NewSrf == NULL)
	    Srf = NewSrf = CagdSrfCopy(Srf);
        CagdSrfSetDomain(NewSrf, 0.0, 1.0, 0.0, 1.0);
    }
    else
        NewSrf = NULL;

    /* Compute adap iso to estimate parameter step size. */
    if ((AdapIso = SymbAdapIsoExtract(Srf, NULL, NULL, Dir, Size,
				      FALSE, FALSE)) == NULL) {
        if (NewSrf != NULL)
	    CagdSrfFree(NewSrf);

        return NULL;
    }

    /* Estimate parameter step size to get a Size step in Euclidean space. */
    for (Crv = AdapIso; Crv != NULL; Crv = Crv -> Pnext) {
        CagdRType TMin, TMax;

        CagdCrvDomain(Crv, &TMin, &TMax);
	AccumParam += TMax - TMin;
	AccumDist += CagdCrvArcLenPoly(Crv);
    }
    Dt = Size * AccumParam / AccumDist;
    /* Round Dt to an integer division of the domain */
    Dt = (1.0 - IRIT_EPS) / ((int) (1.0 / Dt + 0.5));
    CagdCrvFreeList(AdapIso);

    /* Generate the rectangles. */
    GlblAdapIsoRRFuncCB = SymbAdapIsoGenerateRectRgns;
    GlblAdapIsoRRDir = Dir;
    GlblAdapIsoRRDt = Dt;
    GlblAdapIsoRRPls = NULL;
    if ((AdapIso = SymbAdapIsoExtract(Srf, NULL, NULL, Dir, Size,
				      FALSE, FALSE)) == NULL) {
        if (NewSrf != NULL)
	    CagdSrfFree(NewSrf);

        return NULL;
    }
    GlblAdapIsoRRFuncCB = NULL;
    CagdCrvFreeList(AdapIso);
    AllPls = GlblAdapIsoRRPls;
    GlblAdapIsoRRPls = NULL;

    GMVrtxListToCircOrLin(AllPls, TRUE);
    PObj = IPGenPOLYObject(AllPls);

    /* Handle T junctions in the generated rectangles. */
    PTmp = GMRegularizePolyModel(PObj, FALSE);
    IPFreeObject(PObj);
    PObj = GMConvertPolysToRectangles(PTmp);

    /* Smooth the data if so desired. */
    if (Smoothing > 0)
        PObj = GMPolyMeshSmoothing(PObj, Smoothing);

    AllPls = PObj -> U.Pl;
    PObj -> U.Pl = NULL;
    IPFreeObject(PObj);
    GMVrtxListToCircOrLin(AllPls, FALSE);

    /* Process the rectangles in UV space into real geometry. */
    RetObj = SymbAdapIsoExtractRRCreateOutput(OrigSrf, Srf, AllPls,
					      OutputType);
    IPFreePolygonList(AllPls);

    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a distance function that also takes into account an influence   *
* of one point (prescribed globally).                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Level:         Of recursion.                                             *
*   Crv1:          First curve to handle.                                    *
*   NCrv1:         Normal vector field of first curve Crv1.                  *
*   Crv2:          Second curve to handle.                                   *
*   NCrv2:         Normal vector field of second curve Crv2.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The distance function.                                *
*****************************************************************************/
static CagdCrvStruct *AdapIsoDistWeightPt(int Level,
					  CagdCrvStruct *Crv1,
					  CagdCrvStruct *NCrv1,
					  CagdCrvStruct *Crv2,
					  CagdCrvStruct *NCrv2)
{
    CagdPType P;
    CagdCrvStruct *SumCrv2PtSqr, *TCrv1, *TCrv2, *F, *Denom, *Numer,
        *Res, *DistCrv2,
        *DiffCrv = SymbCrvSub(Crv1, Crv2);

    DistCrv2 = SymbCrvDotProd(DiffCrv, DiffCrv);
    CagdCrvFree(DiffCrv);

    /* And now build a weight function f around GlblAdapIsoWeightPt as:    */
    /* f = 1 + (GlblAdapIsoWeightPtScale - 1) / (C12(t) + 1),   where      */
    /* C12(t) = || (C1(t) + C2(t)) * 0.5 - GlblAdapIsoWeightPt ||^2.       */
    
    TCrv1 = SymbCrvAdd(Crv1, Crv2);
    TCrv2 = SymbCrvScalarScale(TCrv1, 0.5);
    CagdCrvFree(TCrv1);

    IRIT_PT_COPY(P, GlblAdapIsoWeightPt);
    IRIT_PT_SCALE(P, -1.0);
    CagdCrvTransform(TCrv2, P, 1.0);

    TCrv1 = SymbCrvDotProd(TCrv2, TCrv2);
    CagdCrvFree(TCrv2);

    SumCrv2PtSqr = SymbCrvScalarScale(TCrv1, GlblAdapIsoWeightPtWidth);
    CagdCrvFree(TCrv1);

    Denom = CagdCrvCopy(SumCrv2PtSqr);
    IRIT_PT_RESET(P);
    P[0] = 1.0;
    CagdCrvTransform(Denom, P, 1.0);

    Numer = SumCrv2PtSqr;
    P[0] = GlblAdapIsoWeightPtScale;
    CagdCrvTransform(Numer, P, 1.0);

    F = SymbCrvMergeScalar(Denom, Numer, NULL, NULL);
    CagdCrvFree(Denom);
    CagdCrvFree(Numer);

    /* Multiply F by the original DistCrv2. */
    Res = SymbCrvMultScalar(DistCrv2, F);
    CagdCrvFree(DistCrv2);
    CagdCrvFree(F);

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the distance function weight for a point.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:         Point to compute the distance function for.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   The distance function weight.                               *
*****************************************************************************/
static CagdRType AdapIsoDistWeightPtEval(CagdPType Pt)
{
    CagdRType
        DSqr = IRIT_PT_PT_DIST_SQR(Pt, GlblAdapIsoWeightPt) *
						     GlblAdapIsoWeightPtWidth;

    return (DSqr + GlblAdapIsoWeightPtScale) / (DSqr + 1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the point location to consider using the adaptive iso generation.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:     Point to consider or NULL to disable.                            M
*   Scale:  Scaling factor at Pt compared to a far location from Pt.	     M
*           Must be larger than 1.0 (1.0 has no effect).		     M
*   Width:  Control over the width of the effect.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbAdapIsoDistWeightPt, SymbAdapIsoExtract				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbAdapIsoSetWeightPt                                                   M
*****************************************************************************/
void SymbAdapIsoSetWeightPt(CagdRType *Pt, CagdRType Scale, CagdRType Width)
{
    if (Pt == NULL || Scale < 1.0) {
        GlblAdapIsoWeightPtScale = 0.0;
	return;
    }

    IRIT_PT_COPY(GlblAdapIsoWeightPt, Pt);
    GlblAdapIsoWeightPtScale = Scale;
    GlblAdapIsoWeightPtWidth = Width;
}
