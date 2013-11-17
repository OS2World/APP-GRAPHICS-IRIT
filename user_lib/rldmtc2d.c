/*****************************************************************************
*   A best matching of a ruled surface to a given surface, using 2D matching.*
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 0.1, Feb 2010.    *
*****************************************************************************/

#include "user_loc.h"
#include "geom_lib.h"

#define USER_RULED_FIT_INVALID_WEIGHT	3.01060e8
#define USER_RULED_FIT_EPS		1e-8

typedef struct UserDynPrgmStruct {
    CagdRType Weight;			    /* The weight of this IJ event. */
    CagdRType Accum;	    /* The accumulated weights until this location. */
    int PrevIdx[2][2]; /* Indices of previous cell of the optimal solution. */
} UserDynPrgmStruct;

typedef struct UserEvalSrfStruct {
    CagdUVType UV;
    CagdPType Pt;
} UserEvalSrfStruct;

IRIT_STATIC_DATA int
    GLblUserRuledFitIsClosed = FALSE;
IRIT_STATIC_DATA CagdSrfStruct
    *GlblUserRuledFitSrf = NULL;
IRIT_STATIC_DATA UserEvalSrfStruct
    **GlblUserRuledFitSamplesMat = NULL;

static UserEvalSrfStruct **UserRuledMatchSampleSrf(const CagdSrfStruct *Srf,
						   CagdSrfBndryType Bndry,
						   int NumSamples,
						   CagdBType *CrvClosed);
static void UserRuledMatchFreeSamples(UserEvalSrfStruct **Mat, int NumSamples);

static CagdCrvStruct *UserRuledSrfMatchDynProg(UserDynPrgmStruct **DP,
					       int Samples,
					       int Shift,
					       CagdBType IsClosed,
					       CagdRType *Error,
					       CagdRType *MaxError);
static CagdSrfStruct *UserRuledSrfBuildSrf(const CagdSrfStruct *Srf,
					   CagdSrfDirType RulingDir,
					   CagdCrvStruct *RuledFit);
static UserDynPrgmStruct ****UserRuledMatchAllocDP(int Samples);
static void UserRuledMatchEvalDP(UserDynPrgmStruct ****DP,
				 int i,
				 int j,
				 int k,
				 int l);
static void UserRuledMatchFreeDP(UserDynPrgmStruct ****DP, int Samples);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Uniformly sample (NumSamples x NumSamples) samples in the interior of    *
* the specified surface Srf.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:            To samples its interior.                                 *
*   Dir:            Parametric direction along which matching will commence. *
*   NumSamples:     To uniformly sample this boundary curve of Srf.          *
*   CrvClosed:      Set to TRUE if curve is a closed curve, FALSE otherwise. *
*                                                                            *
* RETURN VALUE:                                                              *
*   UserEvalSrfStruct *:  A 2D matrix of size (NumSamples x NumSamples),     *
*		 allocated dtnamically, of surface samples (UV and E3 data). *
*****************************************************************************/
static UserEvalSrfStruct **UserRuledMatchSampleSrf(const CagdSrfStruct *Srf,
						   CagdSrfDirType Dir,
						   int NumSamples,
						   CagdBType *CrvClosed)
{
  int i, j;
    CagdRType OtherT;
    CagdCrvStruct *Crv;
    CagdRType UMin, UMax, VMin, VMax, u, v, Du, Dv;
    UserEvalSrfStruct **Mat;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    Du = (UMax - UMin) / (NumSamples - 1) - IRIT_UEPS;
    Dv = (VMax - VMin) / (NumSamples - 1) - IRIT_UEPS;

    *CrvClosed = CagdIsClosedSrf(Srf, Dir);

    /* Sample the surface. */
    assert(NumSamples > 2);
    Mat = (UserEvalSrfStruct **) IritMalloc(sizeof(UserEvalSrfStruct *)
					                        * NumSamples);
    for (i = 0; i < NumSamples; i++) {
        Mat[i] = (UserEvalSrfStruct *) IritMalloc(sizeof(UserEvalSrfStruct)
					                        * NumSamples);
	IRIT_ZAP_MEM(Mat[i], sizeof(UserEvalSrfStruct) * NumSamples);
    }

    switch (Dir) {
        case CAGD_CONST_U_DIR:
	    for (u = UMin, i = 0; u < UMax && i < NumSamples; u += Du, i++) {
	        for (v = VMin, j = 0; v < VMax && j < NumSamples; v += Dv, j++) {
		    CagdRType
		        *R = CagdSrfEval(Srf, u, v);

		    CagdCoerceToE3(Mat[i][j].Pt, &R, -1, Srf -> PType);

		    Mat[i][j].UV[0] = u;
		    Mat[i][j].UV[1] = v;
		}
	    }	       
	    break;
        case CAGD_CONST_V_DIR:
	    for (v = VMin, j = 0; v < VMax && j < NumSamples; v += Dv, j++) {
	        for (u = UMin, i = 0; u < UMax && i < NumSamples; u += Du, i++) {
		    CagdRType
		        *R = CagdSrfEval(Srf, u, v);

		    CagdCoerceToE3(Mat[i][j].Pt, &R, -1, Srf -> PType);

		    Mat[i][j].UV[0] = u;
		    Mat[i][j].UV[1] = v;
		}
	    }	       
	    break;
        default:
	    break;
    }

#define DEBUG_RULED_FIT_MATS
#ifdef DEBUG_RULED_FIT_MATS
    for (i = 0; i < NumSamples; i++) {
        printf("*************** Row %d **************\n", i);
	for (j = 0; j < NumSamples; j++) {
	    printf("UV = [%f, %f],  Pt = (%f,  %f,  %f)\n",
		   Mat[i][j].UV[0], Mat[i][j].UV[1],
		   Mat[i][j].Pt[0], Mat[i][j].Pt[1], Mat[i][j].Pt[2]);
	}
    }
#endif /* DEBUG_RULED_FIT_MATS */

    return Mat;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free the surface's samples matrix.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:            Matrix of samples to free.                               *
*   NumSamples:     Size of this square matrix.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void UserRuledMatchFreeSamples(UserEvalSrfStruct **Mat, int NumSamples)
{
    int i;

    for (i = 0; i < NumSamples; i++)
        IritFree(Mat[i]);

    IritFree(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Perform the dyn. prog. search on the given 2D matrix.                    *
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
static CagdCrvStruct *UserRuledSrfMatchDynProg(UserDynPrgmStruct ****DPTable,
					       int Samples,
					       int Shift,
					       CagdBType IsClosed,
					       CagdRType *Error,
					       CagdRType *MaxError)
{
    int x1, y1, x2, y2, ii, ii1, jj, k;
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
        assert(0);
	return NULL;
    }

    y1 = Samples / 4;
    y2 = Samples * 3 / 4;
    for (x1 = 0; x1 < Samples; x1++) {
	for (x2 = 0; x2 < Samples; x2++) {
	    if (x1 == 0 && x2 == 0) {
		/* No previous line - first entry. */
	        DP[x1][y1][x2][y2].Accum = UserRuledMatchEvalDP(DP, x1, y1,
								    x2, y2);
	    }
	    else {
	        int PrevX1 = -1,
		    PrevY1 = -1,
		    PrevX2 = -1,
		    PrevY2 = -1;
		CagdRType a1, a2, a3,
		    MinPrevAccum = IRIT_INFNTY;

	        /* Examine (x1 - 1, y1, x2, y2), (x1, y1, x2 - 1, y2), or   */
	        /* (x1 - 1, y1, x2 - 1, y2) where yi, i = 1,2, can also     */
	        /* move by one as one of (yi-1, yi, yi+1).                  */
	        for (i = -1; i <= 1; i++) {
		    if (y1 + i < 0 || y1 + i >= Samples)
		        continue;

		    for (j = -1; j <= 1; j++) {
		        if (y2 + j < 0 || y2 + j >= Samples)
			    continue;

		        if (x1 == 0) {
			    /* Propagate from left only. */
			    a1 = DP[x1][y1 + i][x2 - 1][y2 + j].Accum;
			    if (a1 < MinPrevAccum) {
			        PrevX1 = x1;
				PrevY1 = y1 + i;
				PrevX2 = x2 - 1;
				PrevY2 = y2 + j;
				MinPrevAccum = a1;
			    }
			}
			else if (x2 == 0) {
			    /* Propagate from above only. */
			    a1 = DP[x1 - 1][y1 + i][x2][y2 + j].Accum;
			    if (a1 < MinPrevAccum) {
			        PrevX1 = x1 - 1;
				PrevY1 = y1 + i;
				PrevX2 = x2;
				PrevY2 = y2 + j;
				MinPrevAccum = a1;
			    }
			}
			else {
			    a1 = DP[x1 - 1][y1 + i][x2][y2 + j].Accum;
			    a2 = DP[x1][y1 + i][x2 - 1][y2 + j].Accum;
			    a3 = DP[x1 - 1][y1 + i][x2 - 1][y2 + j].Accum;

			    if (a1 < IRIT_MIN(a2, a3) && a1 < MinPrevAccum) {
			        PrevX1 = x1 - 1;
				PrevY1 = y1 + i;
				PrevX2 = x2;
				PrevY2 = y2 + j;
				MinPrevAccum = a1;
			    }
			    else if (a2 < IRIT_MIN(a1, a3) &&
				     a2 < MinPrevAccum) {
			        PrevX1 = x1;
				PrevY1 = y1 + i;
				PrevX2 = x2 - 1;
				PrevY2 = y2 + j;
				MinPrevAccum = a2;
			    }
			    else if (a3 < MinPrevAccum) {
			        PrevX1 = x1;
				PrevY1 = y1 + i;
				PrevX2 = x2 - 1;
				PrevY2 = y2 + j;
				MinPrevAccum = a3;
			    }
			}
		    }
		}

		/* Update the accumulator. */
		if (PrevX1 < 0) {
		    assert(0);
		}
		else {
		    DP[x1][y1][x2][y2].PrevIdx[0][0] = PrevX1;
		    DP[x1][y1][x2][y2].PrevIdx[0][1] = PrevY1;
		    DP[x1][y1][x2][y2].PrevIdx[1][0] = PrevX2;
		    DP[x1][y1][x2][y2].PrevIdx[1][1] = PrevY2;

		    DP[x1][y1][x2][y2].Accum = 
		        DP[PrevX1, PrevY1, PrevX2, PrevY2].Accum +
			UserRuledMatchEvalDP(DP, x1, y1, x2, y2);
		}
	    }	    
	}
    }

    /* Process the solution. */
    *Error = *MaxError = 0.0;

    return NULL;
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
* DESCRIPTION:                                                               *
*   Allocate the 4D dynamic programming (DP) matrix.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   DP:             Matrix for the dynamic programming to allocate.          *
*   Samples:        Size of this square matrix.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static UserDynPrgmStruct ****UserRuledMatchAllocDP(int Samples)
{
    int i, j, k, l;
    UserDynPrgmStruct
        ****DP = (UserDynPrgmStruct ****)
            IritMalloc(sizeof(UserDynPrgmStruct ***) * (Samples + 1));

    for (i = 0; i <= Samples; i++) {
        DP[i] = (UserDynPrgmStruct ***)
	    IritMalloc(sizeof(UserDynPrgmStruct **) * 2 * (Samples + 1));

        for (j = 0; j <= Samples; j++) {
	    DP[i][j] = (UserDynPrgmStruct **)
	        IritMalloc(sizeof(UserDynPrgmStruct *) * 2 * (Samples + 1));

	    for (k = 0; k <= Samples; k++) {
	        DP[i][j][k] = (UserDynPrgmStruct *)
		    IritMalloc(sizeof(UserDynPrgmStruct) * 2 * (Samples + 1));

		for (l = 0; l <= Samples; l++) {
		    /* Too expensive to evaluate now - use lasy evaluation. */
		    DP[i][j][k][l].Weight = USER_RULED_FIT_INVALID_WEIGHT;
		    DP[i][j][k][l].Accum = 0.0;
		    DP[i][j][k][l].PrevIdx[0][0] =
		        DP[i][j][k][l].PrevIdx[0][1] =
		            DP[i][j][k][l].PrevIdx[1][0] =
		                DP[i][j][k][l].PrevIdx[1][1] = -1;
		}
	    }
	}
    }

    return DP;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Lasy evaluate the the 4D dynamic programming (DP) matrix.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:            DP matrix to free.		                             *
*   i, j, k, l:     Indices into Dp to (lasy) evaluate.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void UserRuledMatchEvalDP(UserDynPrgmStruct ****DP,
				 int i,
				 int j,
				 int k,
				 int l)
{
    if (DP[i][j][k][l].Weight == USER_RULED_FIT_INVALID_WEIGHT) {
        DP[i][j][k][l].Weight = MvarSrfLineOneSidedMaxDist(
					GlblUserRuledFitSrf,
					GlblUserRuledFitSamplesMat[i][j].UV,
					GlblUserRuledFitSamplesMat[k][l].UV,
					GLblUserRuledFitIsClosed,
					USER_RULED_FIT_EPS);
    }

    return DP[i][j][k][l].Weight;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free the 4D dynamic programming (DP) matrix.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:            DP matrix to free.		                             *
*   Samples:        Size of thisDP matrix.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void UserRuledMatchFreeDP(UserDynPrgmStruct ****DP, int Samples)
{
    int i, j, k, l;

    for (i = 0; i <= Samples; i++) {
        for (j = 0; j <= Samples; j++) {
	    for (k = 0; k <= Samples; k++)
	        IritFree(DP[i][j][k]);

	    IritFree(DP[i][j]);
	}

	IritFree(DP[i]);
    }

    IritFree(DP);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fit a ruled surface to the given general surface Srf.  The best fit is   M
* found using a dynamic programming search over all possibly 2D rulings,     M
* while each ruling line's distance is measured against the surface.	     M
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
*   UserRuledSrfFit2D                                                        M
*****************************************************************************/
CagdSrfStruct *UserRuledSrfFit2D(const CagdSrfStruct *Srf,
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
    UserEvalSrfStruct **Mat;
    UserDynPrgmStruct ****DP;

    Mat = UserRuledMatchSampleSrf(Srf, RulingDir, Samples, &IsClosed);

    /* Allocate and initialize the 4D dyn. prog. matrix. Note that in X we  */
    /* double the size so we can shift with ease, if closed.		    */
    GLblUserRuledFitIsClosed = IsClosed;
    GlblUserRuledFitSrf = Srf;
    GlblUserRuledFitSamplesMat = Mat;
    DP = UserRuledMatchAllocDP(NumSamples);

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
    UserRuledMatchFreeSamples(Mat, Samples);
    UserRuledMatchFreeDP(DP, Samples);

    return RuledFit;
}

