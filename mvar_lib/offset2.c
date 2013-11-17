/******************************************************************************
* Offset2.c - computes offset approximation to curves and surfaces.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 2006.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "mvar_loc.h"
#include "user_lib.h"

#define MVAR_OFF_CONTOUR_EPS	0.3*2.05949e-8 /* Level above 0 to contour. */
#define MVAR_OFF_SRF_SAMPLES			100
#define MVAR_OFFSET_TRIM_NUMERIC_IMPROVEMENT	TRUE

typedef struct MvarMVsOffsetSrfSamplesStruct {
    CagdRType U, V;
    CagdPType E3Pt;
} MvarMVsOffsetSrfSamplesStruct;

IRIT_STATIC_DATA CagdRType
    GlblSubdivTol = 0.0,
    GlblNumerTol = 0.0,
    GlblTrimAmountEpsSqr = 0.0;
IRIT_STATIC_DATA CagdPType
    GlblLastOSCntr = { IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY };
IRIT_STATIC_DATA CagdSrfStruct
    const *GlblOrigSrf = NULL;
IRIT_STATIC_DATA CagdSrfStruct
    const *GlblOffsetSrf = NULL;
IRIT_STATIC_DATA VoidPtr
    GlblSrfHierarchy = NULL;

IRIT_STATIC_DATA MvarMVsOffsetSrfSamplesStruct
    GlblSrfSamples[MVAR_OFF_SRF_SAMPLES][MVAR_OFF_SRF_SAMPLES];

#if defined(ultrix) && defined(mips)
static int RealParamSortCmpr(VoidPtr VReal1, VoidPtr VReal2);
#else
static int RealParamSortCmpr(const VoidPtr VReal1, const VoidPtr VReal2);
#endif /* ultrix && mips (no const support) */

static int MvarMVsZerosOffsetCallBack(MvarMVStruct ***MVs,
				      MvarConstraintType **Constraints,
				      int *NumOfMVs,
				      int *NumOfZeroMVs,
				      int Depth);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two reals for sorting purposes.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   VReal1, VReal2:  Two pointers to real values.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two real values.         *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int RealParamSortCmpr(VoidPtr VReal1, VoidPtr VReal2)
#else
static int RealParamSortCmpr(const VoidPtr VReal1, const VoidPtr VReal2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
	*R1 = (CagdRType *) VReal1,
	*R2 = (CagdRType *) VReal2;

    return IRIT_SIGN(*R1 - *R2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Trims regions in the offset curve OffCrv that are closer than TrimAmount M
* to original Crv, bu compute all self intersections in the offset curve,    M
* splitting the offset curve at those intersections, and purges offset curve M
* segments that are too close to original curve.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          Original curve, assumed to be C1 self intersection free.   M
*   OffCrv:       The offset curve approximation.                            M
*   TrimAmount:   The trimming distance.  A fraction smaller than the offset M
*		  amount.						     M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of curve segments that are valid, after the     M
*		      trimming process took place.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvTrimGlblOffsetSelfInter                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvTrimGlblOffsetSelfInter                                           M
*****************************************************************************/
CagdCrvStruct *MvarCrvTrimGlblOffsetSelfInter(CagdCrvStruct *Crv,
					      const CagdCrvStruct *OffCrv,
					      CagdRType TrimAmount,
					      CagdRType SubdivTol,
					      CagdRType NumericTol)
{
    int i, j, n, Proximity;
    CagdRType *Params;
    CagdPtStruct *CPt, *CPts;
    MvarPtStruct *MVPt,
        *MVPts = MvarCrvSelfInterDiagFactor(OffCrv, SubdivTol, NumericTol);
    CagdCrvStruct *TCrv, *Crvs, *RetCrvs;

    if (MVPts == NULL)
	return CagdCrvCopy(OffCrv);

    Params = (CagdRType *)
        IritMalloc(sizeof(CagdRType) * (n = CagdListLength(MVPts) * 2));
    for (MVPt = MVPts, i = 0; MVPt != NULL; MVPt = MVPt -> Pnext) {
        Params[i++] = MVPt -> Pt[0];
	Params[i++] = MVPt -> Pt[1];
    }
    MvarPtFreeList(MVPts);

    /* Sort the parameters and filter out too-close neighbors. */
    qsort(Params, n, sizeof(CagdRType), RealParamSortCmpr);
    for (j = 1, i = 0; j < n; j++) {
        if (!IRIT_APX_EQ_EPS(Params[i], Params[j], NumericTol))
	    i++;

	Params[i] = Params[j];
    }
    n = i + 1;

    /* Convert to a form we can subdivide with and subdivide into pieces. */
    CPts = NULL;
    for (i = 0; i < n; i++) {
        CPt = CagdPtNew();
        CPt -> Pt[0] = Params[i]; 
	IRIT_LIST_PUSH(CPt, CPts);
    }
    IritFree(Params);
    CPts = CagdListReverse(CPts);

    Crvs = CagdCrvSubdivAtParams(OffCrv, CPts, NumericTol, &Proximity);
    CagdPtFreeList(CPts);

    /* Now examine all the pieces if they are too close to the original or  */
    /* or not, and purge the segment if so.				    */
    RetCrvs = NULL;
    while (Crvs != NULL) {
        CagdRType MinDist;

        IRIT_LIST_POP(TCrv, Crvs);

	MVPt = MvarCrvCrvMinimalDist(TCrv, Crv, &MinDist, FALSE, NumericTol);
	MvarPtFree(MVPt);

	if (MinDist < TrimAmount) {
	    CagdCrvFree(TCrv);
	}
	else {
	    IRIT_LIST_PUSH(TCrv, RetCrvs);
	}
    }

    return CagdMergeCrvList2(CagdListReverse(RetCrvs), SubdivTol);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of the MV zeros code to test if the current offset    *
* surface region is too close to the original surface as a whole, in which   *
* case the subdivision could be stopped.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:          Current multivariate regions, during the subdivision.      *
*   Constraints:  Type of constraints MVs ares.				     *
*   NumOfMVs:     Total number of constraints in MVs.			     *
*   NumOfZeroMVs: The first NumOfZeroMVs constraints are zero constraints.   *
*   Depth:        Of the subdivision tree.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if the subdivision should be stop, FALSE otherwise.        *
*****************************************************************************/
static int MvarMVsZerosOffsetCallBack(MvarMVStruct ***MVs,
				      MvarConstraintType **Constraints,
				      int *NumOfMVs,
				      int *NumOfZeroMVs,
				      int Depth)
{
    IRIT_STATIC_DATA CagdUVType
        LastMinUV = { IRIT_INFNTY, IRIT_INFNTY };
    int i;
    CagdRType OSUMin, OSUMax, OSVMin, OSVMax, *R, **Points;
    CagdSrfStruct *TSrf, *OffSrfRegion;
    CagdPType OSCntr, SCntr, CPt;
    CagdUVType MinUV;

    MvarMVDomain((*MVs)[0], &OSUMin, &OSUMax, 0);
    MvarMVDomain((*MVs)[0], &OSVMin, &OSVMax, 1);

    /* Find a middle point on the offset surface and find its closest      */
    /* point on the original surface.					   */
    R = CagdSrfEval(GlblOffsetSrf, (OSUMin + OSUMax) * 0.5,
		                   (OSVMin + OSVMax) * 0.5);
    CagdCoerceToE3(OSCntr, &R, -1, GlblOffsetSrf -> PType);
    if (IRIT_PT_APX_EQ(OSCntr, GlblLastOSCntr)) {
        IRIT_UV_COPY(MinUV, LastMinUV);
    }
    else {
	IntrSrfHierarchyTestPt(GlblSrfHierarchy, OSCntr, TRUE, MinUV);
    }
    IRIT_PT_COPY(GlblLastOSCntr, OSCntr);
    IRIT_UV_COPY(LastMinUV, MinUV);

    R = CagdSrfEval(GlblOrigSrf, MinUV[0], MinUV[1]);
    CagdCoerceToE3(SCntr, &R, -1, GlblOrigSrf -> PType);

    /* If the closest point is too far - quit now. */
    if (IRIT_PT_PT_DIST_SQR(OSCntr, SCntr) > GlblTrimAmountEpsSqr)
        return FALSE;

    /* Extract the relevant region from the offset surface. */
    TSrf = CagdSrfRegionFromSrf(GlblOffsetSrf, OSUMin, OSUMax,
				CAGD_CONST_U_DIR);
    OffSrfRegion = CagdSrfRegionFromSrf(TSrf, OSVMin, OSVMax,
				CAGD_CONST_V_DIR);
    CagdSrfFree(TSrf);

    for (i = OffSrfRegion -> ULength * OffSrfRegion -> VLength - 1,
					    Points = OffSrfRegion -> Points;
	 i >= 0;
	 i--) {
        CagdCoerceToE3(CPt, Points, i, OffSrfRegion -> PType);
        if (IRIT_PT_PT_DIST_SQR(SCntr, CPt) > GlblTrimAmountEpsSqr) {
	    CagdSrfFree(OffSrfRegion);
	    return FALSE;
	}
    }

    CagdSrfFree(OffSrfRegion);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   trims regions in the offset surface OffSrf that are closer than	     M
* TrimAmount to original Srf.  TrimAmount should be a fraction smaller than  M
* the offset amount itself.  See also:                                       M
* Joon-Kyung Seong, Gershon Elber, and Myung-Soo Kim. ``Trimming Local and   M
* Global Self-intersections in Offset Curves and Surfaces using Distance     M
* Maps.'' Computer Aided Design, Vol 38, No 3, pp 183-193, March 2006.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          Original surface.                                          M
*   OffSrf:       The offset surface approximation.                          M
*   TrimAmount:   The trimming distance.  A fraction smaller than the offset M
*		  amount.						     M
*   Validate:     If TRUE, compute only points along the self intersections  M
*		  that are valid and serve to delineate between valid and    M
*		  invalid (to be purged) offset surface regions.	     M
*   Euclidean:    If TRUE, returned data is in the Euclidean space of	     M
*		  OffSrf.  FALSE for parametric space of OffSrf.	     M
*   SubdivTol:    Accuracy of computation.				     M
*   NumerTol:     If smaller that SubdivTol, a numerical improvment stage is M
*		  applied by the solver.				     M
*   NumerImp:     If TRUE, a final stage of numerical marching over the      M
*		  surface is applied to improve the solution even further.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of self inter. curves, on the offset surface.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfTrimGlblOffsetSelfInter                                           M
*****************************************************************************/
IPObjectStruct *MvarSrfTrimGlblOffsetSelfInter(CagdSrfStruct *Srf,
					       const CagdSrfStruct *OffSrf,
					       CagdRType TrimAmount,
					       int Validate,
					       int Euclidean,
					       CagdRType SubdivTol,
					       CagdRType NumerTol,
					       CagdBType NumerImp)
{
    IRIT_STATIC_DATA CagdPType
        Translate = { 0.0, 0.0, 0.0 };
    MvarMVStruct *MVCnstrns[3],
	*DistSqr = MvarMVDistSrfSrf(OffSrf, Srf, 1),
	*DuDistSqr = MvarMVDerive(DistSqr, 2),
	*DvDistSqr = MvarMVDerive(DistSqr, 3);
    MvarConstraintType Constraints[3];
    MvarPtStruct *Pts;
    IPObjectStruct *PObjPlls;
    MvarMVsZerosSubdivCallBackFunc OldCallBack;

    /* Make the contouring a zero-level contouring. */
    Translate[0] = -IRIT_SQR(MVAR_OFF_CONTOUR_EPS + TrimAmount);
    MvarMVTransform(DistSqr, Translate, 1.0);

    /* Substitute the first 3 constraints in. */
    MVCnstrns[0] = DistSqr;
    MVCnstrns[1] = DuDistSqr;
    MVCnstrns[2] = DvDistSqr;

    if (Validate) {
        int i, j;
	CagdRType *R, UMin, UMax, VMin, VMax, Du, Dv;

	GlblSrfHierarchy = IntrSrfHierarchyPreprocessSrf(Srf, SubdivTol * 0.1);
	GlblLastOSCntr[0] = 
	    GlblLastOSCntr[1] = 
		GlblLastOSCntr[2] = IRIT_INFNTY;

	/* Sample the original surface uniformly as a rough distance map. */
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	Du = (UMax - UMin - IRIT_UEPS) / MVAR_OFF_SRF_SAMPLES;
	Dv = (VMax - VMin - IRIT_UEPS) / MVAR_OFF_SRF_SAMPLES;

        for (i = 0; i < MVAR_OFF_SRF_SAMPLES; i++) {
	    for (j = 0; j < MVAR_OFF_SRF_SAMPLES; j++) {
	        R = CagdSrfEval(Srf, UMin + Du * i, VMin + Dv * j);
		CagdCoerceToE3(GlblSrfSamples[i][j].E3Pt, &R, -1,
			       Srf -> PType);
		GlblSrfSamples[i][j].U = UMin + Du * i;
		GlblSrfSamples[i][j].V = VMin + Dv * j;
	    }
	}

	GlblOrigSrf = Srf;
	GlblOffsetSrf = OffSrf;
	GlblSubdivTol = SubdivTol;
	GlblNumerTol = NumerTol;
	GlblTrimAmountEpsSqr = IRIT_SQR(MVAR_OFF_CONTOUR_EPS + TrimAmount);
	OldCallBack = MvarMVsZerosSetCallBackFunc(MvarMVsZerosOffsetCallBack);
    }

    /* Solve for the points. */
    Constraints[0] =
        Constraints[1] =
	    Constraints[2] = MVAR_CNSTRNT_ZERO;
    Pts = MvarMVsZeros(MVCnstrns, Constraints, 3, SubdivTol, NumerTol);

    if (Validate) {
        GlblOrigSrf = NULL;
	GlblOffsetSrf = NULL;
	MvarMVsZerosSetCallBackFunc(OldCallBack);
	IntrSrfHierarchyFreePreprocess(GlblSrfHierarchy);
    }

#ifdef MVAR_OFFSET_TRIM_NUMERIC_IMPROVEMENT
    {
        int i;
        IPPolygonStruct *Plls;
	IrtRType
	    *ParamDomain = (IrtRType *) IritMalloc(sizeof(IrtRType) * 3 * 2);

	for (i = 0; i < 3; i++) {
	    IrtRType Max, Min;

	    MvarMVDomain(DistSqr, &Min, &Max, i);
	    ParamDomain[i * 2] = Min;
	    ParamDomain[i * 2 + 1] = Max;
	}

	Plls = MvarCnvrtMVPtsToPolys2(Pts, SubdivTol, 3, ParamDomain);

	/* Numerical improving. */
	if (NumerImp) {
	    PObjPlls = MvarSrfTrimGlblOffsetSelfInterNI(Plls, OffSrf,
							SubdivTol, NumerTol,
							Euclidean, SubdivTol);
	}
	else if (Euclidean) {
	    IPVertexStruct *V;
	    IPPolygonStruct *Pl;

	    for (Pl = Plls; Pl != NULL; Pl = Pl -> Pnext) {
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    CagdRType
		        *R = CagdSrfEval(OffSrf, V -> Coord[0], V -> Coord[1]);

		    CagdCoerceToE3(V -> Coord, &R, -1, OffSrf -> PType);
		}
	    }
	    PObjPlls = IPGenPOLYLINEObject(Plls);
	}
	else
	    PObjPlls = IPGenPOLYLINEObject(Plls);

	IritFree(ParamDomain);
    }
#else
    /* Merge into polylines and evalute to Euclidean space (offset surface). */
    if (Euclidean) {
        MvarMVStruct
	    *MVTemp1 = MvarSrfToMV(OffSrf),
	    *MVTemp2 = MvarPromoteMVToMV2(MVTemp1, 4, 0);

	MvarMVFree(MVTemp1);

	if (CAGD_IS_BSPLINE_SRF(OffSrf) && CAGD_IS_BSPLINE_SRF(Srf)) {
	    CagdRType UMin, UMax, VMin, VMax;

	    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	    BspKnotAffineTrans2(MVTemp2 -> KnotVectors[2],
				MVTemp2 -> Lengths[2] +
			            MVTemp2 -> Orders[2], UMin, UMax);
	    BspKnotAffineTrans2(MVTemp2 -> KnotVectors[3],
				MVTemp2 -> Lengths[3] +
			            MVTemp2 -> Orders[3], VMin, VMax);
	}

	PObjPlls = MvarCnvrtMVPtsToPolys(Pts, MVTemp2, SubdivTol * 2);
	MvarMVFree(MVTemp2);
    }
    else {
        PObjPlls = MvarCnvrtMVPtsToPolys(Pts, NULL, SubdivTol * 2);
    }
#endif /* MVAR_OFFSET_TRIM_NUMERIC_IMPROVEMENT */

    MvarMVFree(DistSqr);
    MvarMVFree(DuDistSqr);
    MvarMVFree(DvDistSqr);

    return PObjPlls;
}
