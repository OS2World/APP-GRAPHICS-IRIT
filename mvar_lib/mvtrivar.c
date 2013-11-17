/******************************************************************************
* MvTrivar.c - Manipulation of trivariate functions.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov 11.					      *
******************************************************************************/
#include "iritprsr.h"
#include "allocate.h"

#include "mvar_loc.h"

static CagdRType MvarGetUSubdivParam(const CagdSrfStruct *Srf);
static CagdSrfStruct *MvarTrivarBoolSumGenerateCap(CagdCrvStruct *Crv1,
						   CagdCrvStruct *Crv2,
						   CagdCrvStruct *Crv3,
						   CagdCrvStruct *Crv4);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Derives a division parameter in U for srf.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   Surface to fetch the desired division parameter in u.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     Division parameter.                                       *
*****************************************************************************/
static CagdRType MvarGetUSubdivParam(const CagdSrfStruct *Srf)
{
    if (Srf -> ULength > Srf -> UOrder)
        return Srf -> UKnotVector[(Srf -> ULength + Srf -> UOrder) / 2];
    else {
        CagdRType UMin, UMax, VMin, VMax;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	return (UMin + UMax) * 0.5;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the volumetric (trivariate) boolean sum as follows:             M
* 1. Divide Srf into 4 side patches, in U.                                   M
* 2. Compute the volumetric boolean sum for the 4 patches in 1 and 2.        M
*    Two cap patches will be computed on the fly.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to derive a trivaraite volume for its interior.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   A trivariate volume boolean sum of Srf.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarTrivarBoolSum                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarTrivarBoolOne                                                        M
*****************************************************************************/
TrivTVStruct *MvarTrivarBoolOne(const CagdSrfStruct *Srf)
{
    CagdSrfStruct
        *Srf12 = CagdSrfSubdivAtParam(Srf, MvarGetUSubdivParam(Srf),
				      CAGD_CONST_U_DIR),
        *Srf34 = Srf12 -> Pnext,
        *Srf1 = CagdSrfSubdivAtParam(Srf12, MvarGetUSubdivParam(Srf12),
				     CAGD_CONST_U_DIR),
        *Srf2 = Srf1 -> Pnext,
        *Srf3 = CagdSrfSubdivAtParam(Srf34, MvarGetUSubdivParam(Srf34),
				     CAGD_CONST_U_DIR),
	*Srf4 = Srf3 -> Pnext;
    TrivTVStruct *TV;

    CagdSrfFreeList(Srf12);
    Srf1 -> Pnext = Srf3 -> Pnext = NULL;

    TV = MvarTrivarBoolSum(Srf1, Srf2, Srf3, Srf4, NULL, NULL);

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(Srf3);
    CagdSrfFree(Srf4);

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a Boolean sum of the given four curves enclosing some region.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2, Crv3, Crv4:   The four curves' boundary. Freed at the end.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:   Construct Boolean sum surface.                        *
*****************************************************************************/
static CagdSrfStruct *MvarTrivarBoolSumGenerateCap(CagdCrvStruct *Crv1,
						   CagdCrvStruct *Crv2,
						   CagdCrvStruct *Crv3,
						   CagdCrvStruct *Crv4)
{
     CagdCrvStruct
         *Crv3R = CagdCrvReverse(Crv3),
         *Crv4R = CagdCrvReverse(Crv4);
     CagdSrfStruct
         *Srf = CagdBoolSumSrf(Crv1, Crv3R, Crv4R, Crv2);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(Crv3);
    CagdCrvFree(Crv4);
    CagdCrvFree(Crv3R);
    CagdCrvFree(Crv4R);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the volumetric (trivariate) boolean sum as follows:             M
* Srf1/2/3/4 are around the sweep surface and Srf5/6 are its two caps.       M
*   The boolean sum is computed as follows:				     M
*       T0 = Trilinear(4Corners(Srf1), 4Corners(Srf3))			     V
*	T1 = RuledVolume(Srf1, Srf3)					     V
*	T2 = RuledVolume(Srf2, Srf4)					     V
*	T3 = RuledVolume(Srf5, Srf6)					     V
*       T4 = RuledVolume(RuledSrf(Bndry(Srf1, UMin), Bndry(Srf1, UMax)),     V
*                        RuledSrf(Bndry(Srf3, UMin), Bndry(Srf3, UMax)),     V
*       T5 = RuledVolume(RuledSrf(Bndry(Srf2, VMin), Bndry(Srf2, VMax)),     V
*                        RuledSrf(Bndry(Srf4, VMin), Bndry(Srf4, VMax)),     V
*       T6 = RuledVolume(RuledSrf(Bndry(Srf5, VMin), Bndry(Srf5, VMax)),     V
*                        RuledSrf(Bndry(Srf6, VMin), Bndry(Srf6, VMax)),     V
* And the final boolean sum equals:					     V
*      BoolSumVolume = T1 + t2 + t3 - (T4 + t5 + t6) + T0		     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2, Srf3, Srf4, Srf5, Srf6:   Surfaces to derive a trivariate    M
*        volume for its interior.  If Srf5 or Srf6 are NULL, they are        M
*        generated as surface Boolean sum, from V boundaries of Srf1/2/3/4.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   A trivariate volume boolean sum of Srf.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarTrivarBoolOne                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarTrivarBoolSum                                                        M
*****************************************************************************/
TrivTVStruct *MvarTrivarBoolSum(const CagdSrfStruct *Srf1,
				const CagdSrfStruct *Srf2,
			 	const CagdSrfStruct *Srf3,
				const CagdSrfStruct *Srf4,
				const CagdSrfStruct *Srf5,
				const CagdSrfStruct *Srf6)
{
    int i, n;
    CagdRType *R, Min, Max, UMin, UMax, VMin, VMax;
    CagdPtStruct
        Pt000, Pt001, Pt010, Pt011, Pt100, Pt101, Pt110, Pt111;
    CagdCrvStruct *C1, *C2;
    CagdSrfStruct *S1, *S2,
        *NewSrf5 = NULL,
        *NewSrf6 = NULL,
        *Srf3R = CagdSrfReverseDir(Srf3, CAGD_CONST_U_DIR),
        *Srf4R = CagdSrfReverseDir(Srf4, CAGD_CONST_U_DIR);
    TrivTVStruct *TV1, *TV2, *TV3;
    MvarMVStruct *MVs[7], *MVTmp1, *MVTmp2, *MVTmp3;

    if (Srf5 == NULL) {
        Srf5 = NewSrf5 = MvarTrivarBoolSumGenerateCap(
			      CagdCrvFromMesh(Srf1, 0, CAGD_CONST_V_DIR),
			      CagdCrvFromMesh(Srf2, 0, CAGD_CONST_V_DIR),
			      CagdCrvFromMesh(Srf3, 0, CAGD_CONST_V_DIR),
			      CagdCrvFromMesh(Srf4, 0, CAGD_CONST_V_DIR));
    }
    if (Srf6 == NULL) {
        Srf6 = NewSrf6 = MvarTrivarBoolSumGenerateCap(
	      CagdCrvFromMesh(Srf1, Srf1 -> VLength - 1, CAGD_CONST_V_DIR),
	      CagdCrvFromMesh(Srf2, Srf2 -> VLength - 1, CAGD_CONST_V_DIR),
	      CagdCrvFromMesh(Srf3, Srf3 -> VLength - 1, CAGD_CONST_V_DIR),
	      CagdCrvFromMesh(Srf4, Srf4 -> VLength - 1, CAGD_CONST_V_DIR));
    }

#ifdef DEBUG_TRIV_BOOL_SUB_REPARAM
    Srf1 = CagdSrfSetDomain((CagdSrfStruct *) Srf1, 20, 21, 30, 31);
    Srf3R = CagdSrfSetDomain((CagdSrfStruct *) Srf3R, 20, 21, 30, 31);

    Srf2 = CagdSrfSetDomain((CagdSrfStruct *) Srf2, 22, 23, 30, 31);
    Srf4R = CagdSrfSetDomain((CagdSrfStruct *) Srf4R, 22, 23, 30, 31);

    Srf5 = CagdSrfSetDomain((CagdSrfStruct *) Srf5, 20, 21, 22, 23);
    Srf6 = CagdSrfSetDomain((CagdSrfStruct *) Srf6, 20, 21, 22, 23);
#endif /* DEBUG_TRIV_BOOL_SUB_REPARAM */

    TV1 = TrivRuledTV(Srf1, Srf3R, 2, 2);
    TV2 = TrivRuledTV(Srf4R, Srf2, 2, 2);
    TV3 = TrivRuledTV(Srf5,  Srf6, 2, 2);

    /* The three ruled volumes between opposite surfaces. */
    MVs[1] = MvarTVToMV(TV1);
    MVs[2] = MvarTVToMV(TV2);
    MVs[3] = MvarTVToMV(TV3);

    TrivTVFree(TV1);
    TrivTVFree(TV2);
    TrivTVFree(TV3);

    MvarMVDomain(MVs[1], &Min, &Max, 0);
    MvarMVSetDomain(MVs[2], Min, Max, 2, TRUE);
    MvarMVDomain(MVs[1], &Min, &Max, 1);
    MvarMVSetDomain(MVs[3], Min, Max, 2, TRUE);
    MvarMVDomain(MVs[2], &Min, &Max, 0);
    MvarMVSetDomain(MVs[1], Min, Max, 2, TRUE);

    /* Swap axes to make all three the same. */
    MVTmp1 = MvarMVReverse(MVs[1], 1, 2);
    MvarMVFree(MVs[1]);
    MVs[1] = MVTmp1;

    MVTmp1 = MvarMVReverse(MVs[2], 0, 2);
    MVTmp2 = MvarMVReverse(MVTmp1, 1, 2);
    MvarMVFree(MVs[2]);
    MvarMVFree(MVTmp1);
    MVs[2] = MVTmp2;

    /* Mow build the three ruled volumes between opposite ruled surfaces. */
    C1 = CagdCrvFromMesh(Srf1, 0, CAGD_CONST_U_DIR);
    C2 = CagdCrvFromMesh(Srf1, Srf1 -> ULength - 1, CAGD_CONST_U_DIR);
    S1 = CagdRuledSrf(C1, C2, 2, 2);
    CagdCrvFree(C1);
    CagdCrvFree(C2);

    C1 = CagdCrvFromMesh(Srf3R, 0, CAGD_CONST_U_DIR);
    C2 = CagdCrvFromMesh(Srf3R, Srf3R -> ULength - 1, CAGD_CONST_U_DIR);
    S2 = CagdRuledSrf(C1, C2, 2, 2);
    CagdCrvFree(C1);
    CagdCrvFree(C2);

    TV1 = TrivRuledTV(S1, S2, 2, 2);
    CagdSrfFree(S1);
    CagdSrfFree(S2);
    MVs[4] = MvarTVToMV(TV1);
    TrivTVFree(TV1);

    MVTmp1 = MvarMVReverse(MVs[4], 0, 1);		      /* Swap axes. */
    MVTmp2 = MvarMVReverse(MVTmp1, 1, 2);		      /* Swap axes. */
    MvarMVFree(MVs[4]);
    MVs[4] = MVTmp2;


    C1 = CagdCrvFromMesh(Srf4R, 0, CAGD_CONST_V_DIR);
    C2 = CagdCrvFromMesh(Srf4R, Srf4R -> VLength - 1, CAGD_CONST_V_DIR);
    S1 = CagdRuledSrf(C1, C2, 2, 2);
    CagdCrvFree(C1);
    CagdCrvFree(C2);

    C1 = CagdCrvFromMesh(Srf2, 0, CAGD_CONST_V_DIR);
    C2 = CagdCrvFromMesh(Srf2, Srf2 -> VLength - 1, CAGD_CONST_V_DIR);
    S2 = CagdRuledSrf(C1, C2, 2, 2);
    CagdCrvFree(C1);
    CagdCrvFree(C2);

    TV2 = TrivRuledTV(S1, S2, 2, 2);
    CagdSrfFree(S1);
    CagdSrfFree(S2);
    MVs[5] = MvarTVToMV(TV2);
    TrivTVFree(TV2);

    MVTmp1 = MvarMVReverse(MVs[5], 0, 2);		      /* Swap axes. */
    MVTmp2 = MvarMVReverse(MVTmp1, 1, 2);		      /* Swap axes. */
    MvarMVFree(MVs[5]);
    MVs[5] = MVTmp2;


    C1 = CagdCrvFromMesh(Srf5, 0, CAGD_CONST_V_DIR);
    C2 = CagdCrvFromMesh(Srf5, Srf5 -> VLength - 1, CAGD_CONST_V_DIR);
    S1 = CagdRuledSrf(C1, C2, 2, 2);
    CagdCrvFree(C1);
    CagdCrvFree(C2);

    C1 = CagdCrvFromMesh(Srf6, 0, CAGD_CONST_V_DIR);
    C2 = CagdCrvFromMesh(Srf6, Srf6 -> VLength - 1, CAGD_CONST_V_DIR);
    S2 = CagdRuledSrf(C1, C2, 2, 2);
    CagdCrvFree(C1);
    CagdCrvFree(C2);

    TV3 = TrivRuledTV(S1, S2, 2, 2);
    CagdSrfFree(S1);
    CagdSrfFree(S2);
    MVs[6] = MvarTVToMV(TV3);
    TrivTVFree(TV3);

    /* Mow build a trilinear volume between the eight corners. */
    CagdSrfDomain(Srf1, &UMin, &UMax, &VMin, &VMax);
    R = CagdSrfEval(Srf1, UMin, VMin);
    CagdCoerceToE3(Pt000.Pt, &R, -1, Srf1 -> PType);
    R = CagdSrfEval(Srf1, UMax, VMin);
    CagdCoerceToE3(Pt001.Pt, &R, -1, Srf1 -> PType);
    R = CagdSrfEval(Srf1, UMin, VMax);
    CagdCoerceToE3(Pt010.Pt, &R, -1, Srf1 -> PType);
    R = CagdSrfEval(Srf1, UMax, VMax);
    CagdCoerceToE3(Pt011.Pt, &R, -1, Srf1 -> PType);
    CagdSrfDomain(Srf3R, &UMin, &UMax, &VMin, &VMax);
    R = CagdSrfEval(Srf3R, UMin, VMin);
    CagdCoerceToE3(Pt100.Pt, &R, -1, Srf3R -> PType);
    R = CagdSrfEval(Srf3R, UMax, VMin);
    CagdCoerceToE3(Pt101.Pt, &R, -1, Srf3R -> PType);
    R = CagdSrfEval(Srf3R, UMin, VMax);
    CagdCoerceToE3(Pt110.Pt, &R, -1, Srf3R -> PType);
    R = CagdSrfEval(Srf3R, UMax, VMax);
    CagdCoerceToE3(Pt111.Pt, &R, -1, Srf3R -> PType);
    
    TV1 = TrivTrilinearSrf(&Pt000, &Pt001, &Pt010, &Pt011,
			   &Pt100, &Pt101, &Pt110, &Pt111);
    MVs[0] = MvarTVToMV(TV1);
    TrivTVFree(TV1);

    MVTmp1 = MvarMVReverse(MVs[0], 1, 2);		      /* Swap axes. */
    MvarMVFree(MVs[0]);
    MVs[0] = MVTmp1;

    /* Now set the domains of all 7 trivariates to be the same. */
    for (n = 0; n < 7; n++) {
        if (MVAR_IS_BSPLINE_MV(MVs[n]))
	    break;
    }
    if (n < 7) {
        for (n = 0; n < 7; n++) {
	    if (MVAR_IS_BEZIER_MV(MVs[n])) {
	        MVTmp1 = MvarCnvrtBzr2BspMV(MVs[n]);
		MvarMVFree(MVs[n]);
		MVs[n] = MVTmp1;
	    }
	}
	for (i = 0; i < 3; i++) {
	    MvarMVDomain(MVs[1], &Min, &Max, i);

	    MvarMVSetDomain(MVs[0], Min, Max, i, TRUE);
	    for (n = 4; n < 7; n++)
	        MvarMVSetDomain(MVs[n], Min, Max, i, TRUE);
	}
    }

    /* Make all 7 trivairates compatible (Same degree/KV). */
    for (n = 1; n < 7; n++)
        MvarMakeMVsCompatible(&MVs[0], &MVs[n], TRUE, TRUE);
    for (n = 1; n < 7; n++)
        MvarMakeMVsCompatible(&MVs[0], &MVs[n], TRUE, TRUE);

#ifdef DEBUG_TRIV_BOOL_SUB_DUMP
    IPStderrObject(IPGenMULTIVARObject(MVs[1]));
    IPStderrObject(IPGenMULTIVARObject(MVs[2]));
    IPStderrObject(IPGenMULTIVARObject(MVs[3]));

    IPStderrObject(IPGenMULTIVARObject(MVs[4]));
    IPStderrObject(IPGenMULTIVARObject(MVs[5]));
    IPStderrObject(IPGenMULTIVARObject(MVs[6]));

    IPStderrObject(IPGenMULTIVARObject(MVs[0]));
    exit(0);
#endif /* EBUG_TRIV_BOOL_SUB_DUMP */

    /* Time to some all up:						   */
    /* MVs[1] + MVs[2] + MVs[3] - (MVs[4] + MVs[5] + MVs[6]) + MVs[7].     */
    MVTmp1 = MvarMVAdd(MVs[1], MVs[2]);
    MVTmp2 = MvarMVAdd(MVTmp1, MVs[3]);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarMVAdd(MVs[4], MVs[5]);
    MVTmp3 = MvarMVAdd(MVTmp1, MVs[6]);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarMVSub(MVTmp2, MVTmp3);
    MvarMVFree(MVTmp2);
    MvarMVFree(MVTmp3);

    MVTmp2 = MvarMVAdd(MVTmp1, MVs[0]);
    MvarMVFree(MVTmp1);
    TV1 = MvarMVToTV(MVTmp2);
    MvarMVFree(MVTmp2);

    for (n = 1; n < 7; n++)
        MvarMVFree(MVs[n]);
    CagdSrfFree(Srf3R);
    CagdSrfFree(Srf4R);

    return TV1;
}

