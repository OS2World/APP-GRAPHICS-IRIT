/*****************************************************************************
*   A best matching of a ruled surface to a given surface.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 0.1, Feb 2010.    *
*****************************************************************************/

#include "user_loc.h"
#include "geom_lib.h"

#define USER_RULED_FIT_EPS	1e-8
#define USER_RULED_SRF_MOD_SAMPLES(i) ((i + Shift + Samples) % Samples)

typedef struct UserDynPrgmStruct {
    CagdRType Weight;			    /* The weight of this IJ event. */
    CagdRType Accum;	    /* The accumulated weights until this location. */
    int PrevIJ[2];     /* Indices of previous cell of the optimal solution. */
} UserDynPrgmStruct;

typedef struct UserEvalSrfStruct {
    CagdUVType UV;
    CagdPType Pt;
} UserEvalSrfStruct;

static UserEvalSrfStruct *UserRuledMatchSampleSrf(const CagdSrfStruct *Srf,
						  CagdSrfBndryType Bndry,
						  int NumSamples,
						  CagdBType *CrvClosed);
static CagdCrvStruct *UserRuledSrfMatchDynProg(UserDynPrgmStruct **DP,
					       int Samples,
					       int Shift,
					       CagdBType IsClosed,
					       CagdRType *Error,
					       CagdRType *MaxError);
static CagdSrfStruct *UserRuledSrfBuildSrf(const CagdSrfStruct *Srf,
					   CagdSrfDirType RulingDir,
					   CagdCrvStruct *RuledFit);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Uniformly sample NumSamples along the specified boundary of surface Srf. *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:            To samples its boundary.                                 *
*   Bndry:          Boundary to sample.                                      *
*   NumSamples:     To uniformly sample this boundary curve of Srf.          *
*   CrvClosed:      Set to TRUE if curve is a closed curve, FALSE otherwise. *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserEvalSrfStruct *:  A vector, allocated dtnamically, of NumSamples     *
*                   samples along the requested boundary (UV and E3 data).   *
*****************************************************************************/
static UserEvalSrfStruct *UserRuledMatchSampleSrf(const CagdSrfStruct *Srf,
						  CagdSrfBndryType Bndry,
						  int NumSamples,
						  CagdBType *CrvClosed)
{
    int i, Dir, OtherDir;
    CagdRType OtherT;
    CagdCrvStruct *Crv;
    CagdRType UMin, UMax, VMin, VMax, TMin, TMax, t, Dt;
    UserEvalSrfStruct *Vec;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (Bndry) {
	case CAGD_U_MIN_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, UMin, CAGD_CONST_U_DIR);
	    TMin = VMin;
	    TMax = VMax;
	    Dir = 1;
	    OtherDir = 0;
	    OtherT = UMin;
	    break;
	case CAGD_U_MAX_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, UMax, CAGD_CONST_U_DIR);
	    TMin = VMin;
	    TMax = VMax;
	    Dir = 1;
	    OtherDir = 0;
	    OtherT = UMax;
	    break;
	case CAGD_V_MIN_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, VMin, CAGD_CONST_V_DIR);
	    TMin = UMin;
	    TMax = UMax;
	    Dir = 0;
	    OtherDir = 1;
	    OtherT = VMin;
	    break;
        case CAGD_V_MAX_BNDRY:
	    Crv = CagdCrvFromSrf(Srf, VMax, CAGD_CONST_V_DIR);
	    TMin = UMin;
	    TMax = UMax;
	    Dir = 0;
	    OtherDir = 1;
	    OtherT = VMax;
	    break;
        default:
	    assert(0);
	    return NULL;
    }
    Dt = TMax - TMin;

    *CrvClosed = CagdIsClosedCrv(Crv);

    /* Sample the possibly expanded curve, Crv. */
    assert(NumSamples > 2);
    Vec = (UserEvalSrfStruct *) IritMalloc(sizeof(UserEvalSrfStruct)
					                        * NumSamples);
    IRIT_ZAP_MEM(Vec, sizeof(UserEvalSrfStruct) * NumSamples);
    Dt = Dt / (NumSamples - 1) - IRIT_UEPS;

    for (t = TMin, i = 0;
	 t < TMax && i < NumSamples;
	 t += Dt, i++) {
        CagdRType
	    *R = CagdCrvEval(Crv, t);

	assert(i < NumSamples);
	CagdCoerceToE3(Vec[i].Pt, &R, -1, Crv -> PType);
	if (CAGD_IS_BSPLINE_CRV(Crv)) {	         /* Round to nearest knot. */
	    int Idx = BspKnotLastIndexLE(Crv -> KnotVector, Crv -> Length, t);

	    if (IRIT_APX_EQ_EPS(t, Crv -> KnotVector[Idx], USER_RULED_FIT_EPS))
		t = Crv -> KnotVector[Idx];
	    else if (IRIT_APX_EQ_EPS(t, Crv -> KnotVector[Idx + 1],
				     USER_RULED_FIT_EPS))
		t = Crv -> KnotVector[Idx + 1];
	}
	Vec[i].UV[Dir] = t;
	Vec[i].UV[OtherDir] = OtherT;

#	ifdef DEBUG_RULED_FIT_VECS
	    if (i == 0)
	        printf("****************** Vec: *****************\n");

	    printf("t = %f  OtherT = %f,  Pt = (%f,  %f,  %f)\n",
		   Vec[i].UV[Dir], Vec[i].UV[OtherDir],
		   Vec[i].Pt[0], Vec[i].Pt[1], Vec[i].Pt[2]);
#	endif /* DEBUG_RULED_FIT_VECS */
    }
    
    CagdCrvFree(Crv);

    return Vec;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Perform the dyn. prog. search on the given 2D table.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   DPTable:  The 2D array of the dynamic programming table.                 *
*   Samples:  Size of the DP table.                                          *
*   Shift:    Horizontal shift to start the search at DP.  I.e. starting     *
*             point will be (i, 0) and termination will be (i, samples-1).   *
*   IsClosed: TRUE if the surface is closed.  Shift can be none zero only    *
*             if the surface is closed.					     *
*   Error:    The accumulated total error.                                   *
*   MaxError: Maximal error - maximal deviation of ruled surface from srf.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:                                                         *
*****************************************************************************/
static CagdCrvStruct *UserRuledSrfMatchDynProg(UserDynPrgmStruct **DPTable,
					       int Samples,
					       int Shift,
					       CagdBType IsClosed,
					       CagdRType *Error,
					       CagdRType *MaxError)
{
    int i, j, ii, ii1, jj, k;
    CagdRType *R, *KV, MinDist;
    CagdCrvStruct *ReparamCrv;
    UserDynPrgmStruct **DP;

    if (!IsClosed && Shift > 0) {
        assert(0);
        return NULL;
    }

    if (Shift == 0)
        DP = DPTable;
    else {
        DP = IritMalloc(sizeof(CagdRType *) * Samples);
        for (j = 0; j < Samples; j++)
	    DP[j] = &DPTable[j][Shift];
    }

    for (j = 0; j < Samples; j++) {
	for (i = 0; i < Samples; i++) {
	    DP[j][i].Accum = 0.0;
	    DP[j][i].PrevIJ[0] = DP[j][i].PrevIJ[1] = -1;
	}
    }

#ifdef DEBUG_RULED_FIT_DP_TABLE
    printf("****************** TABLE: *****************\n");
    for (j = 0; j < Samples; j++) {
	for (i = 0; i < Samples; i++) {
	    printf("%7.5f ", DP[j][i].Weight);
	}
	printf("\n");
    }
#endif /* DEBUG_RULED_FIT_DP_TABLE */

    for (j = 0; j < Samples; j++) {
	for (i = 0; i < Samples; i++) {
	    ii = USER_RULED_SRF_MOD_SAMPLES(i);
	    ii1 = USER_RULED_SRF_MOD_SAMPLES(i - 1);

	    if (j == 0) {
		/* No previous line - propagate from left only. */
		if (ii > 0) {
		    DP[j][ii].Accum = DP[j][ii1].Accum + DP[j][ii].Weight;
		    DP[j][ii].PrevIJ[0] = ii1;
		    DP[j][ii].PrevIJ[1] = j;
		}
	    }
	    else if (!IsClosed && ii == 0) {
		/* Propagate from above only. */
		DP[j][ii].Accum = DP[j - 1][ii].Accum + DP[j][ii].Weight;
		DP[j][ii].PrevIJ[0] = ii;
		DP[j][ii].PrevIJ[1] = j - 1;

	    }
	    else {
		/* Examine (i - 1, j), (i, j - 1), (i - 1, j - 1) cells. */
		if (DP[j][ii1].Accum < DP[j - 1][ii].Accum &&
		    DP[j][ii1].Accum < DP[j - 1][ii1].Accum) {
		    /* (i - 1, j) is the minimum. */
		    DP[j][ii].PrevIJ[0] = ii1;
		    DP[j][ii].PrevIJ[1] = j;
		}
		else if (DP[j - 1][ii].Accum < DP[j - 1][ii1].Accum) {
		    /* (i, j - 1) is the minimum. */
		    DP[j][ii].PrevIJ[0] = ii;
		    DP[j][ii].PrevIJ[1] = j - 1;
		}
		else {
		    /* (i - 1, j - 1) is the minimum. */
		    DP[j][ii].PrevIJ[0] = ii1;
		    DP[j][ii].PrevIJ[1] = j - 1;
		}

		/* Update the accumulator. */
		DP[j][ii].Accum = 
			DP[DP[j][ii].PrevIJ[1]][DP[j][ii].PrevIJ[0]].Accum +
			DP[j][ii].Weight;
	    }	    
	}
    }

    /* Process the solution. */
    *Error = *MaxError = 0.0;
    ReparamCrv = BspCrvNew(Samples * 4, 3, CAGD_PT_E1_TYPE);
    R = ReparamCrv -> Points[1];
    KV = ReparamCrv -> KnotVector;
    i = Samples - 1;
    j = Samples - 1;
    KV[Samples * 4 - 1] = j / (Samples - 1.0);
    R[Samples * 4 - 1] = (i + Shift) / (Samples - 1.0);
    for (k = Samples * 4 - 2; k >= 0; k--) {
	ii = DP[j][i].PrevIJ[0];
	jj = DP[j][i].PrevIJ[1];
	KV[k] = jj / (Samples - 1.0);
	R[k] = (ii + Shift) / (Samples - 1.0);

#	ifdef DEBUG_RULED_FIT_MATCH
	    if (k == Samples * 4 - 2)
	        fprintf(stderr, "****************** Match: *****************\n\n[OBJECT PL\n    [POLYLINE XXX\n\t[%d  %d  0]\n", i + Shift, j);

	    fprintf(stderr, "\t[%d  %d  0]\n", ii + Shift, jj );
	    if (ii == 0 && jj == 0)
	        fprintf(stderr, "    ]\n]\n");
#	endif /* DEBUG_RULED_FIT_DP_MATCH */
    
	*Error += DP[j][i].Weight;
	*MaxError = IRIT_MAX(*MaxError, DP[j][i].Weight);
	i = ii;
	j = jj;
	if (i == 0 && j == 0)
	    break;				   /* Got to starting point. */
    };
    assert(k >= 0);

    /* Move the data to origin. */
    j = Samples * 4 - k;
    IRIT_GEN_COPY(&KV[1], &KV[k], sizeof(CagdRType) * j);
    IRIT_GEN_COPY(R, &R[k], sizeof(CagdRType) * j);

    /* Enforce open end conditions. */
    KV[0] = KV[2] = KV[1];
    KV[j + 1] = KV[j + 2] = KV[j];
    ReparamCrv -> Length = j;

    MinDist = 0.01 / Samples;
    do {
        CagdCrvStruct
	    *TCrv = CagdCrvCopy(ReparamCrv);

	if (BspVecSpreadEqualItems(&TCrv -> KnotVector[TCrv -> Order - 1],
				   TCrv -> Length - TCrv -> Order + 2,
				   MinDist) &&
	    BspVecSpreadEqualItems(TCrv -> Points[1], TCrv -> Length,
				   MinDist)) {
	    CagdCrvFree(ReparamCrv);
	    ReparamCrv = TCrv;
	    break;
	}
	else {
	    CagdCrvFree(TCrv);
	    MinDist *= 0.5;
	}
    }
    while (MinDist > 0.001);

    if (Shift != 0)
        IritFree(DP);

#   ifdef DEBUG_RULED_FIT_MATCH
    CagdDbg(ReparamCrv);
#   endif /* DEBUG_RULED_FIT_DP_MATCH */

    return ReparamCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Builds a ruled surface. given the matching parameter values along the    *
* RuledFit matching.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        To fit a ruled surface through.                              *
*   RulingDir:  Either the U or the V direction.		             *
*   ReparamRuledFit:  The matching fit between the two boundaries.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:                                                         *
*****************************************************************************/
static CagdSrfStruct *UserRuledSrfBuildSrf(const CagdSrfStruct *Srf,
					   CagdSrfDirType RulingDir,
					   CagdCrvStruct *ReparamRuledFit)
{
    int CrossBoundary,
	Len = ReparamRuledFit -> Length;
    CagdRType UMin, UMax, VMin, VMax, TMin, TMax,
        *R = ReparamRuledFit -> Points[1];
    CagdCrvStruct *Crv1, *Crv2, *TCrv;
    CagdSrfStruct *RuledSrf;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (RulingDir) {
	case CAGD_CONST_U_DIR:
	    Crv1 = CagdCrvFromSrf(Srf, VMin, CAGD_CONST_V_DIR);
	    Crv2 = CagdCrvFromSrf(Srf, VMax, CAGD_CONST_V_DIR);
	    break;
        case CAGD_CONST_V_DIR:
	    Crv1 = CagdCrvFromSrf(Srf, UMin, CAGD_CONST_U_DIR);
	    Crv2 = CagdCrvFromSrf(Srf, UMax, CAGD_CONST_U_DIR);
	    break;
        default:
	    USER_FATAL_ERROR(USER_ERR_INVALID_DIR);
	    return NULL;
    }

    /* Map the curves to domain [0, 1] as this is our reparam. domain. */
    if (CAGD_IS_BSPLINE_CRV(Crv1))
        BspKnotAffineTransOrder2(Crv1 -> KnotVector, Crv1 -> Order,
			         Crv1 -> Length + Crv2 -> Order, 0.0, 1.0);
    if (CAGD_IS_BSPLINE_CRV(Crv2))
	BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				 Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);

    /* Verify ReparamRuledFit does no exceed the surface domain (Can happen */
    /* if periodic) - if so split at Srf boundary limits & solve by parts.  */
    CrossBoundary = FALSE;
    switch (RulingDir) {
	case CAGD_CONST_U_DIR:
	    TMin = VMin;
	    TMax = VMax;
	    if (R[0] < VMax && R[Len - 1] > VMax)
	        CrossBoundary = TRUE;
	    break;
        case CAGD_CONST_V_DIR:
	    TMin = UMin;
	    TMax = UMax;
	    if (R[0] < UMax && R[Len - 1] > UMax)
	        CrossBoundary = TRUE;
	    break;
        default:
	    USER_FATAL_ERROR(USER_ERR_INVALID_DIR);
	    return NULL;
    }
    if (CrossBoundary) { 				 /* Solve by parts. */
        int i;
        CagdCrvStruct *ReparamCrvs, *TCrv1, *TCrv2;
	CagdPtStruct
	    *Pts = SymbCrvConstSet(ReparamRuledFit, 1, USER_RULED_FIT_EPS,
				   TMax, TRUE);

	assert(Pts -> Pnext == NULL);/* Monotone shape - has only one inter.*/
	ReparamCrvs = CagdCrvSubdivAtParam(ReparamRuledFit, Pts -> Pt[0]);

	/* Map the ReparamCrvs pieces back to the surface domain. */
	R = ReparamCrvs -> Points[1];
	for (i = 0; i < ReparamCrvs -> Length; i++, R++) {
	    if (*R < TMin)
	        *R = TMin;
	    if (*R > TMax)
	        *R = TMax;
	}
	R = ReparamCrvs -> Pnext -> Points[1];
	for (i = 0; i < ReparamCrvs -> Pnext -> Length; i++, R++) {
	    *R -= TMax - TMin;
	    if (*R < TMin)
	        *R = TMin;
	    if (*R > TMax)
	        *R = TMax;
	}

	TCrv1 = SymbComposeCrvCrv(Crv2, ReparamCrvs);
	TCrv2 = SymbComposeCrvCrv(Crv2, ReparamCrvs -> Pnext);

	CagdCrvFreeList(ReparamCrvs);
	TCrv = CagdMergeCrvCrv(TCrv1, TCrv2, FALSE);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	CagdCrvFree(Crv2);
	Crv2 = TCrv;
    }
    else {
        TCrv = SymbComposeCrvCrv(Crv2, ReparamRuledFit);
	CagdCrvFree(Crv2);
	Crv2 = TCrv;
    }

    RuledSrf = CagdRuledSrf(Crv1, Crv2, 2, 2);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return RuledSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fit a ruled surface to the given general surface Srf.  The best fit is   M
* found using a dynamic programming search over all possibly rulings, while  M
* each ruling line's distance is measured against the surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To fit a ruled surface through.                              M
*   RulingDir:  Either the U or the V direction.  This is used only to       M
*               sample Srf and construct the possibly ruling lines.          M
*   ExtndDmn:   Amount to extended the selected sampled boundary curves.     M
*               Zero will not extend and match the ruling from the original  M
*               boundary to its maximum.  Not supported.		     M
*   Samples:    Number of samples to compute the dynamic programming with.   M
*		Typically in the hundreds.  Must be greater than two.	     M
*   Error:      The computed error, following the distance between the       M
*               rulings and the original surface, Srf.  In L2 sense.	     M
*   MaxError:   maximum error detected in L-infinity sense.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    The fitted ruled surface, or NULL if error.          M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserRuledSrfFit                                                          M
*****************************************************************************/
CagdSrfStruct *UserRuledSrfFit(const CagdSrfStruct *Srf,
			       CagdSrfDirType RulingDir,
			       CagdRType ExtndDmn,
			       int Samples,
			       CagdRType *Error,
			       CagdRType *MaxError)
{
    CagdBType Is1Closed, Is2Closed, IsClosed;
    int i, j;
    CagdCrvStruct *RuledMatch;
    CagdSrfStruct *RuledFit;
    UserEvalSrfStruct *Vec1, *Vec2;
    UserDynPrgmStruct **DP;

#ifdef DEBUG_RULED_FIT_LN_SRF_DIST_TEST
    {
        CagdUVType UV1, UV2;
        CagdRType R;

	UV1[0] = 0.3;
	UV1[1] = 0.0;
	UV2[0] = 0.7;
	UV2[1] = 1.0;
	R = MvarSrfLineOneSidedMaxDist(Srf, UV1, UV2, FALSE,
				       USER_RULED_FIT_EPS);
    }
#endif /* DEBUG_RULED_FIT_LN_SRF_DIST_TEST */

    switch (RulingDir) {
       case CAGD_CONST_U_DIR:
	   Vec1 = UserRuledMatchSampleSrf(Srf, CAGD_V_MIN_BNDRY, Samples,
					  &Is1Closed);
	   Vec2 = UserRuledMatchSampleSrf(Srf, CAGD_V_MAX_BNDRY, Samples,
					  &Is2Closed);
	   break;
       case CAGD_CONST_V_DIR:
	   Vec1 = UserRuledMatchSampleSrf(Srf, CAGD_U_MIN_BNDRY, Samples,
					  &Is1Closed);
	   Vec2 = UserRuledMatchSampleSrf(Srf, CAGD_U_MAX_BNDRY, Samples,
					  &Is2Closed);
	   break;
       default:
	   USER_FATAL_ERROR(USER_ERR_INVALID_DIR);
	   return NULL;
    }
    IsClosed = Is1Closed && Is2Closed;

    /* Allocate and initialize the dyn. prog. matrix. Note that in X we    */
    /* double the size so we can shift with ease.			   */
    DP = (UserDynPrgmStruct **) IritMalloc(sizeof(UserDynPrgmStruct *) *
					                      (Samples + 1));
    for (i = 0; i <= Samples; i++)
        DP[i] = (UserDynPrgmStruct *) IritMalloc(sizeof(UserDynPrgmStruct) *
						 2 * (Samples + 1));

    for (i = 0; i < Samples; i++) {
        for (j = 0; j < Samples; j++) {
	    DP[i][j].Weight = MvarSrfLineOneSidedMaxDist(Srf,
							 Vec1[i].UV,
							 Vec2[j].UV,
							 IsClosed ? RulingDir
							         : CAGD_NO_DIR,
							 USER_RULED_FIT_EPS);
#	    ifdef DEBUG_RULED_FIT_LN_SRF_DIST
	    fprintf(stderr, "t1 = %d (%f %f), t2 = %d (%f, %f) :: Weight %f\n",
		    i, Vec1[i].UV[0], Vec1[i].UV[1],
		    j, Vec2[j].UV[0], Vec2[j].UV[1],
		    DP[i][j].Weight);
#	    endif /* DEBUG_RULED_FIT_LN_SRF_DIST */

#	    ifdef DEBUG_RULED_FIT_FORCE_DIAG
	    if (i != j)
	        DP[i][j].Weight = 10000;
#	    endif /* DEBUG_RULED_FIT_FORCE_DIAG */
	}

	/* Duplicate the data in X, if closed. */
	if (IsClosed)
	    IRIT_GEN_COPY(&DP[i][Samples], &DP[i][0],
			  sizeof(UserDynPrgmStruct) * Samples);
    }

    /* Perform the dynamic programming match: */
    if (IsClosed) {
        CagdRType Err, MaxErr;

	*Error = *MaxError = IRIT_INFNTY;

	RuledMatch = NULL;

	for (i = 0; i < Samples; i++) {
	    CagdCrvStruct
	        *RMatch = UserRuledSrfMatchDynProg(DP, Samples, i, TRUE,
						   &Err, &MaxErr);

#	    ifdef DEBUG_RULED_FIT_ERROR_DUMP
	        fprintf(stderr, "Shift %d, Error = %f, Max Error = %f\n",
			i, Err, MaxErr);
#	    endif /* DEBUG_RULED_FIT_ERROR_DUMP */

	    if (Err < *Error) {
	        if (RuledMatch != NULL)
		    CagdCrvFree(RuledMatch);
		RuledMatch = RMatch;
		*Error = Err;
		*MaxError = MaxErr;
#		ifdef DEBUG_RULED_FIT_MATCH
		fprintf(stderr, "Minimal error so far = %f\n", Err);
#		endif /* DEBUG_RULED_FIT_MATCH */

	    }
	    else
	        CagdCrvFree(RMatch);
	}
    }
    else
        RuledMatch = UserRuledSrfMatchDynProg(DP, Samples, 0, FALSE,
					      Error, MaxError);

    RuledFit = UserRuledSrfBuildSrf(Srf, RulingDir, RuledMatch);
    CagdCrvFree(RuledMatch);

    /* Free all aux. data structures. */
    IritFree(Vec1);
    IritFree(Vec2);
    for (i = 0; i <= Samples; i++)
        IritFree(DP[i]);
    IritFree(DP);

    return RuledFit;
}

