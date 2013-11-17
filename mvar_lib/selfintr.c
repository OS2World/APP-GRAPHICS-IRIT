/******************************************************************************
* SelfIntr.c - self intersection of varieties.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by gershon Elber, Feb 2006.                                         *
******************************************************************************/

#include "irit_sm.h"
#include "mvar_loc.h"

#define MVAR_SRF_BZR_FROM_BSP_DOMAIN(Srf, UMin, UMax, VMin, VMax) { \
	        float *UV; \
	        UV = AttrGetUVAttrib(Srf -> Attr, "BspDomainMin"); \
		assert(UV != NULL); \
	        UMin = UV[0]; \
	        VMin = UV[1]; \
	        UV = AttrGetUVAttrib(Srf -> Attr, "BspDomainMax"); \
		assert(UV != NULL); \
	        UMax = UV[0]; \
	        VMax = UV[1]; \
	    }

#define MVAR_KNOT_IN_ORIG_KV_CRV(t, Crv, Eps) \
    IRIT_APX_EQ_EPS(Crv -> KnotVector[BspKnotLastIndexLE(Crv -> KnotVector, \
						 CAGD_CRV_PT_LST_LEN(Crv) + \
						     Crv -> Order,  \
						 t)], t, Eps)

IRIT_STATIC_DATA CagdCrvStruct const
    *GlblSelfInterCrvOrig = NULL;

static int MvarMVsZerosDelUMinusVCB(MvarMVStruct ***MVs,
				    MvarConstraintType **Constraints,
				    int *NumOfMVs,
				    int *NumOfZeroMVs,
				    int Depth);
static void MVarMVAccumulate(MvarMVStruct **MVAccum, MvarMVStruct *MVNew);
static MvarPtStruct *MvarFilterSltnsZeroCnstrns(MvarPtStruct *MVPts,
						MvarMVStruct **MVs,
						int MinMVs,
						int MaxMVs,
						IrtRType Tol);
static MvarPtStruct *MvarPtsMapAndAppend(MvarPtStruct *MVPts,
					 CagdSrfStruct *BzrSrf1,
					 CagdSrfStruct *BzrSrf2,
					 MvarPtStruct *MVPtsAll);
static MvarPtStruct *MvarBspSrfSelfInterPurgePt(MvarPtStruct *MVPts,
						IrtRType U1,
						IrtRType V1,
						IrtRType U2,
						IrtRType V2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes a (u - v) factor from the given scalar multivariate which is     M
* assumed to be a bivariate S(u, v).                                         M
*   Note that typically a Bspline surface will not have (u - v) in all its   M
* patches so use this function with care - this function does not verify     M
* this existence.  It is more common to have (u - v) only along symmetric    M
* diagonal patches of the Bspline surface, after symbolic operations like    M
* C1(u) - C2(v).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   Bivariate (rep. as multivriate) to factor out a (u - v) term from. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: Factored out bivariate, (rep. as a multivriate).         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV, BzrSrfFactorUMinusV, MvarMV4VarFactorUMinusVRMinusT,M
*   MvarMV3VarFactorUMinusV						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVBivarFactorUMinusV                                                 M
*****************************************************************************/
MvarMVStruct *MvarMVBivarFactorUMinusV(const MvarMVStruct *MV)
{
    CagdSrfStruct *FctrSrf,
        *Srf = MvarMVToSrf(MV);
    MvarMVStruct *FctrMV;

    assert(MV -> Dim == 2);

    if (CAGD_IS_BEZIER_SRF(Srf))
	FctrSrf = BzrSrfFactorUMinusV(Srf);
    else if (CAGD_IS_BSPLINE_SRF(Srf))
	FctrSrf = BspSrfFactorUMinusV(Srf);
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_BEZ_OR_BSP_EXPECTED);
	return NULL;
    }

#   ifdef DEBUG
    {
        CagdSrfStruct *UMinusV, *MultSrf;

        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestUMinVResult, FALSE) {
	    /* Lets see if product with (u - v) recovers the original. */
	    if (CAGD_IS_BSPLINE_SRF(Srf)) {
	        CagdRType UMin, UMax, VMin, VMax;

	        UMinusV = BspSrfNew(2, 2, 2, 2, CAGD_PT_E1_TYPE);

		CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

		BspKnotUniformOpen(2, 2, UMinusV -> UKnotVector);
		BspKnotAffineTransOrder2(UMinusV -> UKnotVector,
					 UMinusV -> UOrder,
					 UMinusV -> UOrder +
					     UMinusV -> ULength,
					 UMin, UMax);

		BspKnotUniformOpen(2, 2, UMinusV -> VKnotVector);
		BspKnotAffineTransOrder2(UMinusV -> VKnotVector,
					 UMinusV -> VOrder,
					 UMinusV -> VOrder +
					     UMinusV -> VLength,
					 VMin, VMax);

		UMinusV -> Points[1][0] =  UMin - VMin;
		UMinusV -> Points[1][1] =  UMax - VMin;
		UMinusV -> Points[1][2] =  UMin - VMax;
		UMinusV -> Points[1][3] =  UMax - VMax;
	    }
	    else {
	        UMinusV = BzrSrfNew(2, 2, CAGD_PT_E1_TYPE);
		UMinusV -> Points[1][0] =  0.0;
		UMinusV -> Points[1][1] =  1.0;
		UMinusV -> Points[1][2] = -1.0;
		UMinusV -> Points[1][3] =  0.0;
	    }

	    MultSrf = SymbSrfMult(FctrSrf, UMinusV);
	    fprintf(stderr, "Multivar UMinusV factor is %s\n",
		    CagdSrfsSame(Srf, MultSrf, IRIT_EPS) ? "Same"
							 : "Different");
	    CagdSrfFree(UMinusV);
	    CagdSrfFree(MultSrf);
	}
    }
#   endif /* DEBUG */

    FctrMV = MvarSrfToMV(FctrSrf);
    CagdSrfFree(FctrSrf);
    CagdSrfFree(Srf);

    return FctrMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes a (u - v) factor from the given scalar four multivariate which   M
* is assumed to be a 4-variate F(u, v, r, t).  The u, v parameters are the   M
* first two parameters of the 4-variate.  The (u - v) terms are factored     M
* out along the diagonal of the third/fourth parameters which are assumed    M
* to be symmetric as well.		                                     M
*   Note that typically a Bspline 4-var will not have (u - v) in all its     M
* all its patches so use this function with care - this function does not    M
* verify this existence.  It is more common to have (u - v) only along       M
* symmetric diagonal patches of the Bsplines, after symbolic                 M
* operations like S1(u, v) - S2(r, t).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   4-variate (rep. as multivariate) to factor out (u - v) term from.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: Factored out 4-variate, (rep. as a multivriate).         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV, BzrSrfFactorUMinusV, MvarMVBivarFactorUMinusV,      M
*   MvarMV4VarFactorUMinusR, MvarMV3VarFactorUMinusV			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMV4VarFactorUMinusV                                                  M
*****************************************************************************/
MvarMVStruct *MvarMV4VarFactorUMinusV(const MvarMVStruct *MV)
{
    int k, Len, BivarMeshSize;
    CagdSrfStruct *FctrSrf, *RaisedFctrSrf,
	*Srf = NULL;
    MvarMVStruct *FctrMV;

    assert(MV -> Dim == 4);
    assert(MV -> Lengths[2] == MV -> Lengths[3]);

    FctrMV = MvarMVCopy(MV);

    BivarMeshSize = MV -> Lengths[0] * MV -> Lengths[1];

    for (k = Len = 0; k < MV -> Lengths[2]; k++) {
        if (k == 0)
	    Srf = MvarMVToSrf(MV);
	else {
	    /* Copy the next bivariate mesh from the MV onto our Srf. */
	    Len += BivarMeshSize * (1 + MV -> Lengths[2]);
	    CAGD_GEN_COPY(Srf -> Points[1], &FctrMV -> Points[1][Len],
			  sizeof(CagdRType) * BivarMeshSize);
	}

	FctrSrf = BspSrfFactorUMinusV(Srf);
	/* Now degree raise FctrSrf back. */
	RaisedFctrSrf = CagdSrfDegreeRaiseN(FctrSrf,
					    FctrSrf -> UOrder + 1,
					    FctrSrf -> VOrder + 1);
	CagdSrfFree(FctrSrf);

	CAGD_GEN_COPY(&FctrMV -> Points[1][Len],
		      RaisedFctrSrf -> Points[1],
		      sizeof(CagdRType) * BivarMeshSize);
	CagdSrfFree(RaisedFctrSrf);
    }

    CagdSrfFree(Srf);

    return FctrMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Removes a (u - r) factor from the given scalar four multivariate which   M
* is assumed to be a 4-variate S(u, v, r, t)	                             M
*   Note that typically a Bspline 4-var will not have (u - r) in all its     M
* patches so use this function with care - this function does not verify     M
* this existence.  It is more common to have (u - r) only along symmetric    M
* diagonal patches of the Bsplines, after symbolic  operations like          M
* S1(u, v) - S2(r, t).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   4-variate (rep. as multivariate) to factor out (u - r) term from.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: Factored out bivariate, (rep. as a multivriate).         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV, BzrSrfFactorUMinusV, MvarMVBivarFactorUMinusV,      M
*   MvarMV4VarFactorUMinusV, MvarMV3VarFactorUMinusV			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMV4VarFactorUMinusR                                                  M
*****************************************************************************/
MvarMVStruct *MvarMV4VarFactorUMinusR(const MvarMVStruct *MV)
{
    MvarMVStruct *FctrMVTmp1, *FctrMVTmp2;

    assert(MV -> Dim == 4);

    /* Reverse v with r, factor out the (u - r) term, and reverse back. */
    FctrMVTmp1 = MvarMVReverse(MV, 1, 2);
    FctrMVTmp2 = MvarMV4VarFactorUMinusV(FctrMVTmp1);
    MvarMVFree(FctrMVTmp1);
    FctrMVTmp1 = MvarMVReverse(FctrMVTmp2, 1, 2);
    MvarMVFree(FctrMVTmp2);

    return FctrMVTmp1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of the MV zeros code to remove (u - v) terms. Invokes *
* the removal function iff is a Bezier patch along the diagonal.	     *
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
static int MvarMVsZerosDelUMinusVCB(MvarMVStruct ***MVs,
				    MvarConstraintType **Constraints,
				    int *NumOfMVs,
				    int *NumOfZeroMVs,
				    int Depth)
{
    int i;
    CagdRType UMin, UMax, VMin, VMax;
    MvarMVStruct *MV;

    MvarMVDomain((*MVs)[0], &UMin, &UMax, 0);
    MvarMVDomain((*MVs)[0], &VMin, &VMax, 1);

    /* If not a symmetric domain or not a Bezier patch, do nothing. */
    if (!IRIT_APX_EQ(UMin, VMin) ||
	!IRIT_APX_EQ(UMax, VMax) ||
	(*MVs)[0] -> Lengths[0] != (*MVs)[0] -> Orders[0] ||
	(*MVs)[0] -> Lengths[1] != (*MVs)[0] -> Orders[1])
	return FALSE;

    /* Make sure this domain seats between original knots. */
    if (!MVAR_KNOT_IN_ORIG_KV_CRV(UMin, GlblSelfInterCrvOrig, IRIT_EPS) ||
	!MVAR_KNOT_IN_ORIG_KV_CRV(UMax, GlblSelfInterCrvOrig, IRIT_EPS))
        return FALSE;

    /* Remove the (u - v) term. */
    for (i = 0; i < *NumOfZeroMVs; i++) {
        MV = MvarMVBivarFactorUMinusV((*MVs)[i]);

	MV -> PAux = (*MVs)[i] -> PAux;
	(*MVs)[i] -> PAux = NULL;

	MV -> PAux2 = (*MVs)[i] -> PAux2;
	(*MVs)[i] -> PAux2 = NULL;

	MV -> Attr = (*MVs)[i] -> Attr;
	(*MVs)[i] -> Attr = NULL;

	MvarMVFree((*MVs)[i]);
	(*MVs)[i] = MV;
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes antipodal points in the given curve - pairs of points C(t)      M
* and C(r) such that (t and r are two independent parameters of same curve): M
*									     M
*            < C(t) - C(r), dC(t)/dt > = 0,				     V
*            < C(t) - C(r), dC(r)/dr > = 0.				     V
*                                                                            M
*   Direct attempt to solve this set of constraints is bound to be slow as   M
* all points in Crv satisfy these equations when t == r.  The key is in      M
* adding a third inequality constaint of the form			     M
*                                                                            M
*            < C'(t), C'(r) > < 0.					     V
*                                                                            M
*   Antipodal points must exists if Crv self intersect in a closed loop and  M
* hence can help in detecting self-intersections.  Further, the diameter of  M
* Crv could be easily deduced from the antipodal points.		     M
*   Note this function also captures the self-intersection locations	     M
* C(t) = C(r), for which the dot product of the tangents is negative.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         To detect its antipodal points.                            M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Antipodal points, as points in E2 (r, t). 	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MVarCrvDiameter, SymbCrvDiameter, MvarSrfAntipodalPoints,                M
*   MvarHFDistAntipodalCrvCrvC1						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvAntipodalPoints                                                   M
*****************************************************************************/
MvarPtStruct *MvarCrvAntipodalPoints(const CagdCrvStruct *CCrv,
				     CagdRType SubdivTol,
				     CagdRType NumericTol)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *DCrv, *Crv;
    MvarPtStruct *AntiPodalPoints, *Pt, *RetPts;
    MvarConstraintType Constrs[3];
    MvarMVStruct *MVCrv1, *MVCrv2, *MVCrvDiff, *MVDrCrv, *MVDtCrv, *MVTemp,
        *MVs[3];

    if (CAGD_IS_RATIONAL_CRV(CCrv))
        MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);

    /* Make sure curve has open end condition. */
    Crv = CagdCnvrtBsp2OpenCrv(CCrv);

    CagdCrvDomain(Crv, &TMin, &TMax);
    if (TMin != 0.0 || TMax != 1.0) {
	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
				 Crv -> Length + Crv -> Order,
				 0.0, 1.0);
    }

    DCrv = CagdCrvDerive(Crv);

    /* Crv(t) - Crv(r). */
    MVTemp = MvarCrvToMV(Crv);
    MVCrv1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVCrv2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);

    MVCrvDiff = MvarMVSub(MVCrv1, MVCrv2);
    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);

    /* DrCrv, DtCrv. */
    MVTemp = MvarCrvToMV(DCrv);
    MVDrCrv = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVDtCrv = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);
    CagdCrvFree(DCrv);

    /* < Crv(r) - Crv(t), DrCrv >. */
    MVs[0] = MvarMVDotProd(MVCrvDiff, MVDrCrv);

    /* < Crv(r) - Crv(t), DtCrv >. */
    MVs[1] = MvarMVDotProd(MVCrvDiff, MVDtCrv);

    MvarMVFree(MVCrvDiff);

    /* < DCrv(r), DCrv(t) >. */
    MVs[2] = MvarMVDotProd(MVDrCrv, MVDtCrv);
    MvarMVFree(MVDrCrv);
    MvarMVFree(MVDtCrv);

    Constrs[0] = Constrs[1] = MVAR_CNSTRNT_ZERO;
    Constrs[2] = MVAR_CNSTRNT_NEGATIVE;

    AntiPodalPoints = MvarMVsZeros(MVs, Constrs, 3, SubdivTol, NumericTol);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    MvarMVFree(MVs[2]);

    CagdCrvFree(Crv);

    /* Purge redundant solutions. */
    NumericTol = IRIT_FABS(NumericTol);
    for (RetPts = NULL; AntiPodalPoints != NULL; ) {
	IRIT_LIST_POP(Pt, AntiPodalPoints);
	if (Pt -> Pt[0] > Pt -> Pt[1] &&
	    !IRIT_APX_EQ_EPS(Pt -> Pt[0], Pt -> Pt[1], NumericTol * 10)) {
	    MvarPtFree(Pt);
	}
	else
	    IRIT_LIST_PUSH(Pt, RetPts);
    }

    if (TMin != 0.0 || TMax != 1.0) {
        CagdRType
	    Dt = TMax - TMin;

        /* Map solution back to original domain. */
        for (Pt = RetPts; Pt != NULL; Pt = Pt -> Pnext) {
	    Pt -> Pt[0] = Pt -> Pt[0] * Dt + TMin;
	    Pt -> Pt[1] = Pt -> Pt[1] * Dt + TMin;
	}
    }

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes antipodal points in the given surface - pairs of points S(u, v) M
* and S(s, t) such that ((u, v) and (s, t) are two independent params of the M
* same srf):								     M
*									     M
*            < S(u, v) - S(s, t), dS(u, v)/du > = 0,			     V
*            < S(u, v) - S(s, t), dS(u, v)/dv > = 0,			     V
*            < N(s, t), dS(u, v)/du > = 0,				     V
*            < N(s, t), dS(u, v)/dv > = 0.				     V
*                                                                            M
*   Direct attempt to solve this set of constraints is bound to be slow as   M
* all points in Srf satisfy these equations when (u, v) == (s, t).  The key  M
* is in adding a fifth inequality constaint of the form			     M
*                                                                            M
*            < N(u, v), N(s, t) > < 0.					     V
*                                                                            M
*   Antipodal points must exists if Srf self intersect in a closed loop and  M
* hence can help in detecting sefl-intersections. Further, the diameter of   M
* Srf could be easily deduced from the antipodal points.		     M
*   Original version of this function was written by Diana Pekerman,	     M
* Technion, Israel.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To detect its antipodal points.                            M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   Antipodal points, as points in E4 (u, v, s, t).        M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvAntipodalPoints, MvarHFDistAntipodalSrfSrfC1                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfAntipodalPoints                                                   M
*****************************************************************************/
MvarPtStruct *MvarSrfAntipodalPoints(const CagdSrfStruct *Srf,
				     CagdRType SubdivTol,
				     CagdRType NumericTol)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *DuSrf, *DvSrf, *NrmlSrf, *CpSrf;
    MvarPtStruct *AntiPodalPoints, *Pt, *RetPts;
    MvarConstraintType Constrs[5];
    MvarMVStruct *MVSrf1, *MVSrf2, *MVSrfDiff, *MVDuSrf, *MVDvSrf,
	*MVDsSrf, *MVDtSrf, *MVTemp, *MVTemp1, *MVTemp2, *MVs[5];

    if (CAGD_IS_RATIONAL_SRF(Srf))
        MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);

    if (CAGD_IS_BSPLINE_SRF(Srf) && !BspSrfHasOpenEC(Srf))
        Srf = CpSrf = BspSrfOpenEnd(Srf);
    else
        CpSrf = NULL;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    if (UMin != 0.0 || UMax != 1.0 || VMin != 0.0 || VMax != 1.0) {
	if (CpSrf == NULL)
	    Srf = CpSrf = CagdSrfCopy(Srf);

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
				 Srf -> ULength + Srf -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
				 Srf -> VLength + Srf -> VOrder,
				 0.0, 1.0);
    }

    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
    NrmlSrf = SymbSrfCrossProd(DuSrf, DvSrf);

    /* Srf(u, v) - Srf(s, t). */
    MVTemp = MvarSrfToMV(Srf);
    MVSrf1 = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVSrf2 = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);

    MVSrfDiff = MvarMVSub(MVSrf1, MVSrf2);
    MvarMVFree(MVSrf1);
    MvarMVFree(MVSrf2);

    /* DuSrf, DsSrf. */
    MVTemp = MvarSrfToMV(DuSrf);
    MVDuSrf = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVDsSrf = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);
    CagdSrfFree(DuSrf);

    /* DvSrf, DtSrf. */
    MVTemp = MvarSrfToMV(DvSrf);
    MVDvSrf = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVDtSrf = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);
    CagdSrfFree(DvSrf);

    /* < NrmlSrf(u,v), NrmlSrf(s,t) >. */
    MVTemp = MvarSrfToMV(NrmlSrf);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MVs[4] = MvarMVDotProd(MVTemp1, MVTemp2);

    /* < Srf(u,v) - Srf(s,t), DuSrf >. */
    MVs[0] = MvarMVDotProd(MVSrfDiff, MVDuSrf);
    /* < Srf(u,v) - Srf(s,t), DvSrf >. */
    MVs[1] = MvarMVDotProd(MVSrfDiff, MVDvSrf);
    /* NrmlSrf(s,t), DuSrf >. */
    MVs[2] = MvarMVDotProd(MVTemp2, MVDuSrf);
    /* NrmlSrf(s,t), DvSrf >. */
    MVs[3] = MvarMVDotProd(MVTemp2, MVDvSrf);

    MvarMVFree(MVSrfDiff);
    MvarMVFree(MVDuSrf);
    MvarMVFree(MVDvSrf);
    MvarMVFree(MVDsSrf);
    MvarMVFree(MVDtSrf);
    MvarMVFree(MVTemp);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);
    CagdSrfFree(NrmlSrf);

    Constrs[0] = Constrs[1] = Constrs[2] = Constrs[3] = MVAR_CNSTRNT_ZERO;
    Constrs[4] = MVAR_CNSTRNT_NEGATIVE;

    AntiPodalPoints = MvarMVsZeros(MVs, Constrs, 5, SubdivTol, NumericTol);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    MvarMVFree(MVs[2]);
    MvarMVFree(MVs[3]);
    MvarMVFree(MVs[4]);

    if (CpSrf != NULL)
        CagdSrfFree(CpSrf);

    /* Purge redundant solutions. */
    NumericTol = IRIT_FABS(NumericTol);
    for (RetPts = NULL; AntiPodalPoints != NULL; ) {
	IRIT_LIST_POP(Pt, AntiPodalPoints);
	if (Pt -> Pt[0] > Pt -> Pt[2] ||
	    (IRIT_APX_EQ_EPS(Pt -> Pt[0], Pt -> Pt[2], NumericTol * 10) &&
	     IRIT_APX_EQ_EPS(Pt -> Pt[1], Pt -> Pt[3], NumericTol * 10))) {
	    MvarPtFree(Pt);
	}
	else
	    IRIT_LIST_PUSH(Pt, RetPts);
    }

    if (UMin != 0.0 || UMax != 1.0 || VMin != 0.0 || VMax != 1.0) {
        CagdRType
	    Du = UMax - UMin,
	    Dv = VMax - VMin;

        /* Map solution back to original domain. */
        for (Pt = RetPts; Pt != NULL; Pt = Pt -> Pnext) {
	    Pt -> Pt[0] = Pt -> Pt[0] * Du + UMin;
	    Pt -> Pt[1] = Pt -> Pt[1] * Dv + VMin;
	    Pt -> Pt[2] = Pt -> Pt[2] * Du + UMin;
	    Pt -> Pt[3] = Pt -> Pt[3] * Dv + VMin;
	}
    }

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes self intersection points for given curve using the following    M
* constraints r and t are two independent params of the same crv):	     M
*									     M
*            x(r) - x(t) = 0,						     V
*            y(r) - y(t) = 0.						     V
*                                                                            M
*   Direct attempt to solve this set of constraints is bound to be slow as   M
* all points in Crv satisfy these equations when r == t.  The key is in      M
* adding a third inequality constaint of the form			     M
*                                                                            M
*                 < N(r), N(t) >					     V
*            ---------------------- < Cos(Angle),		             V
*            || N(r) ||  || N(t) ||					     V
*                                                                            M
*   Where Cos(Angle) is a provided constant that prescribes the minimal      M
* angle the curve is expected to intersect at.  The closer Cos(Angle) to     M
* one the more work this function will have to do in order to isolate the    M
* self intersection points.  The above expression is not rational and so,    M
* we use a logical or of the following two expressions:			     M
*                                                                            M
*                 < N(r), N(t) >^2					     V
*            -------------------------- < Cos^2(Angle),		             V
*            || N(r) ||^2 || N(t) ||^2					     V
*     or                                                                     M
*                                                                            M
*                 < N(r), N(t) > < 0.					     V
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         To detect its self intersection points.                    M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*   MinNrmlDeviation:   At the intersection points.  Zero for 90 degrees     M
*	minimum deviation, positive for smaller minimal deviation and	     M
*	negative for a larger minimal deviation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Self intersection points, as points in E4 (r, t).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvAntipodalPoints, MvarSrfSelfInterNrmlDev                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvSelfInterNrmlDev                                                  M
*****************************************************************************/
MvarPtStruct *MvarCrvSelfInterNrmlDev(const CagdCrvStruct *CCrv,
				      CagdRType SubdivTol,
				      CagdRType NumericTol,
				      CagdRType MinNrmlDeviation)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *NrmlCrv, *Crv;
    MvarPtStruct *SelfInterPoints, *Pt, *RetPts;
    MvarConstraintType Constrs[3];
    MvarMVStruct *MVTemp, *MVTemp1, *MVTemp2, *MVTemp3, *MVTemp4, *MVs[3];

    if (CAGD_NUM_OF_PT_COORD(CCrv -> PType) < 2) {
	MVAR_FATAL_ERROR(MVAR_ERR_ONLY_2D);
	return NULL;
    }

    if (CAGD_IS_BEZIER_CRV(CCrv))
        Crv = CagdCnvrtBzr2BspCrv(CCrv);
    else
        Crv = CagdCnvrtBsp2OpenCrv(CCrv);

    CagdCrvDomain(Crv, &TMin, &TMax);
    if (TMin != 0.0 || TMax != 1.0) {
        CagdCrvStruct
	    *TCrv = CagdCrvCopy(Crv);         /* Must be a B-spline curve. */

	CagdCrvFree(Crv);
	Crv = TCrv;

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
				 Crv -> Length + Crv -> Order,
				 0.0, 1.0);
    }

    NrmlCrv = CagdCrv2DNormalField(Crv);

    /* The first X, Y constraints. */
    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    if (CrvW != NULL)
        CagdCrvFree(CrvW);    
    if (CrvZ != NULL)
        CagdCrvFree(CrvZ);    

    /* X */
    MVTemp = MvarCrvToMV(CrvX);
    CagdCrvFree(CrvX);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);

    MVs[0] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* Y */
    MVTemp = MvarCrvToMV(CrvY);
    CagdCrvFree(CrvY);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);

    MVs[1] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* The angle between normals' constraint: < N(t), N(r) >  <  MinNrmlDev. */
    MVTemp = MvarCrvToMV(NrmlCrv);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);
    MVs[2] = MvarMVDotProd(MVTemp1, MVTemp2);
    if (MinNrmlDeviation >= 90) {
        /* Use just the constraint of < N(t), N(r) > < 0. */
	MvarMVFree(MVTemp1);
	MvarMVFree(MVTemp2);
    }
    else {
        CagdRType t;
	MvarMVStruct *MVMerge[MVAR_MAX_PT_SIZE];

        /* Implement both constraints in same MV function, with the         */
	/* semantics of a logical or between them.		            */
	IRIT_ZAP_MEM(MVMerge, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
	MVMerge[1] = MVs[2];

        MVTemp3 = MvarMVDotProd(MVTemp1, MVTemp1);
        MVTemp4 = MvarMVDotProd(MVTemp2, MVTemp2);
	MvarMVFree(MVTemp1);
	MvarMVFree(MVTemp2);
	t = cos(IRIT_DEG2RAD(MinNrmlDeviation));
	t = IRIT_SQR(t);
	MVTemp1 = MvarMVMult(MVTemp3, MVTemp4);
	MvarMVFree(MVTemp3);
	MvarMVFree(MVTemp4);
	MvarMVTransform(MVTemp1, NULL, t);

	MVTemp2 = MvarMVMult(MVMerge[1], MVMerge[1]);

	MVMerge[2] = MvarMVSub(MVTemp2, MVTemp1);

	MVs[2] = MvarMVMergeScalar(MVMerge);
	MvarMVFree(MVMerge[1]);
	MvarMVFree(MVMerge[2]);
    }

    Constrs[0] = Constrs[1] = MVAR_CNSTRNT_ZERO;
    Constrs[2] = MVAR_CNSTRNT_NEGATIVE;

    SelfInterPoints = MvarMVsZeros(MVs, Constrs, 3, SubdivTol, NumericTol);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    MvarMVFree(MVs[2]);

    CagdCrvFree(Crv);

    /* Purge redundant solutions. */
    NumericTol = IRIT_FABS(NumericTol);
    for (RetPts = NULL; SelfInterPoints != NULL; ) {
	IRIT_LIST_POP(Pt, SelfInterPoints);
	if (Pt -> Pt[0] > Pt -> Pt[1] ||
	    (NumericTol < SubdivTol &&
	     IRIT_APX_EQ_EPS(Pt -> Pt[0], Pt -> Pt[1], NumericTol * 10))) {
	    MvarPtFree(Pt);
	}
	else
	    IRIT_LIST_PUSH(Pt, RetPts);
    }

    if (TMin != 0.0 || TMax != 1.0) {
        CagdRType
	    Dt = TMax - TMin;

        /* Map solution back to original domain. */
        for (Pt = RetPts; Pt != NULL; Pt = Pt -> Pnext) {
	    Pt -> Pt[0] = Pt -> Pt[0] * Dt + TMin;
	    Pt -> Pt[1] = Pt -> Pt[1] * Dt + TMin;
	}
    }

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes self intersection points for given curve using the following    M
* constraints r and t are two independent params of the same crv:	     M
*									     M
*            x(r) - x(t) = 0,						     V
*            y(r) - y(t) = 0.						     V
*                                                                            M
*   Direct attempt to solve this set of constraints is bound to be slow as   M
* all points in Crv satisfy these equations when r == t.  The key here is    M
* to remove all (u - v) factors off diagonal Bezier patches of the above.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          To detect its self intersection points.                    M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Self intersection points, as points in E4 (r, t).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvAntipodalPoints, MvarSrfSelfInterNrmlDev,			     M
*   MvarCrvSelfInterNrmlDev			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvSelfInterDiagFactor                                               M
*****************************************************************************/
MvarPtStruct *MvarCrvSelfInterDiagFactor(const CagdCrvStruct *Crv,
					 CagdRType SubdivTol,
					 CagdRType NumericTol)
{
    CagdBType OldDomainReduce;
    CagdRType TMin, TMax;
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *CpCrv;
    MvarPtStruct *SelfInterPoints, *Pt, *RetPts;
    MvarConstraintType Constrs[3];
    MvarMVStruct *MVTemp, *MVTemp1, *MVTemp2, *MVs[2];
    MvarMVsZerosSubdivCallBackFunc OldCallBack;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2) {
	MVAR_FATAL_ERROR(MVAR_ERR_ONLY_2D);
	return NULL;
    }

    if (CAGD_IS_BEZIER_CRV(Crv))
        Crv = CpCrv = CagdCnvrtBzr2BspCrv(Crv);
    else if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv))
        Crv = CpCrv = BspCrvOpenEnd(Crv);
    else
        CpCrv = NULL;

    CagdCrvDomain(Crv, &TMin, &TMax);
    if (TMin != 0.0 || TMax != 1.0) {
        CagdCrvStruct
	    *TCrv = CagdCrvCopy(Crv);         /* Must be a B-spline curve. */

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	Crv = CpCrv = TCrv;

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Crv -> KnotVector, Crv -> Order,
				 Crv -> Length + Crv -> Order,
				 0.0, 1.0);
    }

    /* The first X, Y constraints. */
    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    if (CrvW != NULL)
        CagdCrvFree(CrvW);    
    if (CrvZ != NULL)
        CagdCrvFree(CrvZ);    

    /* X */
    MVTemp = MvarCrvToMV(CrvX);
    CagdCrvFree(CrvX);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);

    MVs[0] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* Y */
    MVTemp = MvarCrvToMV(CrvY);
    CagdCrvFree(CrvY);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 2, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 2, 1);
    MvarMVFree(MVTemp);

    MVs[1] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    Constrs[0] = Constrs[1] = MVAR_CNSTRNT_ZERO;

    OldCallBack = MvarMVsZerosSetCallBackFunc(MvarMVsZerosDelUMinusVCB);
    /* So we can track domains - when subdividing over knots in CB func. */
    OldDomainReduce = MvarMVsZerosDomainReduction(FALSE);

    GlblSelfInterCrvOrig = Crv;

    SelfInterPoints = MvarMVsZeros(MVs, Constrs, 2, SubdivTol, NumericTol);

    MvarMVsZerosSetCallBackFunc(OldCallBack);
    MvarMVsZerosDomainReduction(OldDomainReduce);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    if (CpCrv != NULL)
        CagdCrvFree(CpCrv);

    /* Purge redundant solutions. */
    NumericTol = IRIT_FABS(NumericTol);
    for (RetPts = NULL; SelfInterPoints != NULL; ) {
	IRIT_LIST_POP(Pt, SelfInterPoints);
	if (Pt -> Pt[0] > Pt -> Pt[1] ||
	    (NumericTol < SubdivTol &&
	     IRIT_APX_EQ_EPS(Pt -> Pt[0], Pt -> Pt[1], NumericTol * 10))) {
	    MvarPtFree(Pt);
	}
	else
	    IRIT_LIST_PUSH(Pt, RetPts);
    }

    if (TMin != 0.0 || TMax != 1.0) {
        CagdRType
	    Dt = TMax - TMin;

        /* Map solution back to original domain. */
        for (Pt = RetPts; Pt != NULL; Pt = Pt -> Pnext) {
	    Pt -> Pt[0] = Pt -> Pt[0] * Dt + TMin;
	    Pt -> Pt[1] = Pt -> Pt[1] * Dt + TMin;
	}
    }

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes self intersection points for given surface using the following  M
* constraints (u, v) and (s, t) are two independent params of the same srf:  M
*									     M
*            x(u, v) - x(s, t) = 0,					     V
*            y(u, v) - y(s, t) = 0,					     V
*            z(u, v) - z(s, t) = 0.					     V
*                                                                            M
*   Direct attempt to solve this set of constraints is bound to be slow as   M
* all points in Srf satisfy these equations when (u, v) == (s, t).  The key  M
* is in adding a fourth inequality constaint of the form		     M
*                                                                            M
*                 < N(u, v), N(s, t) >					     V
*            ---------------------------- < Cos(Angle),                      V
*            || N(u, v) ||  || N(s, t) ||				     V
*                                                                            M
*   Where Cos(Angle) is a provided constant that prescribes the minimal      M
* angle the surface is expected to intersect at.  The closer Cos(Angle) to   M
* one the more work this function will have to do in order to isolate the    M
* self intersection curve.  The above expression is not rational and so,     M
* we use a logical or of the following two expressions:			     M
*                                                                            M
*                 < N(u, v), N(s, t) >^2				     V
*            ------------------------------- < Cos^2(Angle),                 V
*            || N(u, v) ||^2 || N(s, t) ||^2				     V
*     or                                                                     M
*                                                                            M
*                 < N(u, v), N(s, t) > < 0.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To detect its self intersecting points.                    M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*   MinNrmlDeviation:  The minimal angle teh surfaces are suppose to         M
*		  intersect at.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Self intersection points, as points in E4 (u, v, s, t). M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfAntipodalPoints, MvarCrvSelfInterNrmlDev                          M
*   MvarSrfSelfInterDiagFactor 				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSelfInterNrmlDev                                                  M
*****************************************************************************/
MvarPtStruct *MvarSrfSelfInterNrmlDev(const CagdSrfStruct *Srf,
				      CagdRType SubdivTol,
				      CagdRType NumericTol,
				      CagdRType MinNrmlDeviation)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *NrmlSrf, *CpSrf;
    MvarPtStruct *SelfInterPoints, *Pt, *RetPts;
    MvarConstraintType Constrs[4];
    MvarMVStruct *MVTemp, *MVTemp1, *MVTemp2, *MVTemp3, *MVTemp4, *MVs[4];

    if (CAGD_NUM_OF_PT_COORD(Srf -> PType) != 3) {
	MVAR_FATAL_ERROR(MVAR_ERR_ONLY_3D);
	return NULL;
    }

    if (CAGD_IS_BSPLINE_SRF(Srf) && !BspSrfHasOpenEC(Srf))
	Srf = CpSrf = BspSrfOpenEnd(Srf);
    else
        CpSrf = NULL;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    if (UMin != 0.0 || UMax != 1.0 || VMin != 0.0 || VMax != 1.0) {
	if (CpSrf == NULL)
	    Srf = CpSrf = CagdSrfCopy(Srf);

	/* Force a domain of [0,..,1] for simplicity. */
	BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
				 Srf -> ULength + Srf -> UOrder,
				 0.0, 1.0);
	BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
				 Srf -> VLength + Srf -> VOrder,
				 0.0, 1.0);
    }

    NrmlSrf = SymbSrfNormalSrf(Srf);

    /* The first X, Y, Z constraints. */
    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
    if (SrfW != NULL)
        CagdSrfFree(SrfW);    

    /* X */
    MVTemp = MvarSrfToMV(SrfX);
    CagdSrfFree(SrfX);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);

    MVs[0] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* Y */
    MVTemp = MvarSrfToMV(SrfY);
    CagdSrfFree(SrfY);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);

    MVs[1] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* Z */
    MVTemp = MvarSrfToMV(SrfZ);
    CagdSrfFree(SrfZ);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);

    MVs[2] = MvarMVSub(MVTemp1, MVTemp2);
    MvarMVFree(MVTemp1);
    MvarMVFree(MVTemp2);

    /* The angle between normals' constraint: <N(u,v), N(s,t)> < MinNrmlDev. */
    MVTemp = MvarSrfToMV(NrmlSrf);
    MVTemp1 = MvarPromoteMVToMV2(MVTemp, 4, 0);
    MVTemp2 = MvarPromoteMVToMV2(MVTemp, 4, 2);
    MvarMVFree(MVTemp);
    MVs[3] = MvarMVDotProd(MVTemp1, MVTemp2);

    if (MinNrmlDeviation >= 90) {
        /* Use just the constraint of < N(u,v), N(s,t) > < 0. */
    }
    else {
        CagdRType t;
	MvarMVStruct *MVMerge[MVAR_MAX_PT_SIZE];

        /* Implement both constraints in same MV function, with the         */
	/* semantics of a logical or between them.		            */
	IRIT_ZAP_MEM(MVMerge, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
	MVMerge[1] = MVs[3];

        MVTemp3 = MvarMVDotProd(MVTemp1, MVTemp1);
        MVTemp4 = MvarMVDotProd(MVTemp2, MVTemp2);
	MvarMVFree(MVTemp1);
	MvarMVFree(MVTemp2);
	t = cos(IRIT_DEG2RAD(MinNrmlDeviation));
	t = IRIT_SQR(t);
	MVTemp1 = MvarMVMult(MVTemp3, MVTemp4);
	MvarMVFree(MVTemp3);
	MvarMVFree(MVTemp4);
	MvarMVTransform(MVTemp1, NULL, t);

	MVTemp2 = MvarMVMult(MVMerge[1], MVMerge[1]);

	MVMerge[2] = MvarMVSub(MVTemp2, MVTemp1);

	MVs[3] = MvarMVMergeScalar(MVMerge);
	MvarMVFree(MVMerge[1]);
	MvarMVFree(MVMerge[2]);
    }

    Constrs[0] = Constrs[1] = Constrs[2] = MVAR_CNSTRNT_ZERO;
    Constrs[3] = MVAR_CNSTRNT_NEGATIVE;

    SelfInterPoints = MvarMVsZeros(MVs, Constrs, 4, SubdivTol, NumericTol);

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);
    MvarMVFree(MVs[2]);
    MvarMVFree(MVs[3]);

    if (CpSrf != NULL)
        CagdSrfFree(CpSrf);

    /* Purge redundant solutions. */
    NumericTol = IRIT_FABS(NumericTol);
    for (RetPts = NULL; SelfInterPoints != NULL; ) {
	IRIT_LIST_POP(Pt, SelfInterPoints);
	if (IRIT_APX_EQ_EPS(Pt -> Pt[0], Pt -> Pt[2], NumericTol * 10) &&
	    IRIT_APX_EQ_EPS(Pt -> Pt[1], Pt -> Pt[3], NumericTol * 10)) {
	    MvarPtFree(Pt);
	}
	else
	    IRIT_LIST_PUSH(Pt, RetPts);
    }

    if (UMin != 0.0 || UMax != 1.0 || VMin != 0.0 || VMax != 1.0) {
        CagdRType
	    Du = UMax - UMin,
	    Dv = VMax - VMin;

        /* Map solution back to original domain. */
        for (Pt = RetPts; Pt != NULL; Pt = Pt -> Pnext) {
	    Pt -> Pt[0] = Pt -> Pt[0] * Du + UMin;
	    Pt -> Pt[1] = Pt -> Pt[1] * Dv + VMin;
	    Pt -> Pt[2] = Pt -> Pt[2] * Du + UMin;
	    Pt -> Pt[3] = Pt -> Pt[3] * Dv + VMin;
	}
    }

    return RetPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Adds MVNew to MVAccum, in place, and then free MVNew.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   MVAccum:   Multivariate to add he new function MVNew.                    *
*   MVNew:     New function to add to MVAccum.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MVarMVAccumulate(MvarMVStruct **MVAccum, MvarMVStruct *MVNew)
{
    MvarMVStruct
        *MVTmp = MvarMVAdd(*MVAccum, MVNew);

    MvarMVFree(*MVAccum);
    MvarMVFree(MVNew);

    *MVAccum = MVTmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a 2-variate Bezier function, Srf, that is to be subtracted from    M
* itself as,								     M
*          F(u1, u2, u3, u4) = Srf(u1, u2) - Srf(u3, u4),		     V
* computes a decomposition for F as					     M
*          F(u1, u2, u3, u4) = (u1 - u3) G(u1, u2, u3, u4) +		     V
*			       (u2 - u4) H(u1, u2, u3, u4),		     V
* that is known to always exist.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             The 2-variate to subtract against itself and decompose. M
*   U1MinusU3Factor: The G function above.                                   M
*   U2MinusU4Factor: The H function above.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrSelfInter4VarDecomp 	                                             M
*****************************************************************************/
void MvarBzrSelfInter4VarDecomp(const CagdSrfStruct *Srf,
				MvarMVStruct **U1MinusU3Factor,
				MvarMVStruct **U2MinusU4Factor)
{
    CagdPointType
	PType = Srf -> PType;
    MvarMVStruct *MVTmp1;

    if (Srf -> UOrder == 2 && Srf -> VOrder == 2) {  /* A bilinear surface. */
	int k;
        CagdSrfStruct *Srf1, *Srf2;
	CagdRType **Pts1, **Pts2,
	    * const *Pts = Srf -> Points;

        Srf1 = BzrSrfNew(1, 2, PType);
	Pts1 = Srf1 -> Points;

        Srf2 = BzrSrfNew(2, 1, PType);
	Pts2 = Srf2 -> Points;

	for (k = !CAGD_IS_RATIONAL_PT(PType);
	     k <= CAGD_NUM_OF_PT_COORD(PType);
	     k++) {
	    CagdRType
		A2 = Pts[k][1] - Pts[k][0],
		A3 = Pts[k][2] - Pts[k][0],
		A4 = Pts[k][0] + Pts[k][3] - (Pts[k][1] + Pts[k][2]);

	    Pts1[k][0] = A2;
	    Pts1[k][1] = A2 + A4;

	    Pts2[k][0] = A3;
	    Pts2[k][1] = A3 + A4;
	}

	MVTmp1 = MvarSrfToMV(Srf1);
	*U1MinusU3Factor = MvarPromoteMVToMV2(MVTmp1, 4, 2);
	CagdSrfFree(Srf1);
	MvarMVFree(MVTmp1);

	MVTmp1 = MvarSrfToMV(Srf2);
	*U2MinusU4Factor = MvarPromoteMVToMV2(MVTmp1, 4, 0);
	CagdSrfFree(Srf2);
	MvarMVFree(MVTmp1);
    }
    else {
        CagdSrfStruct *S11, *S12, *S21, *S22, *S1, *S2, *S3, *S4,
		      *STmp1, *STmp2;
	MvarMVStruct *MVTmp2, *MVTmp3, *MVTmp4, *MVTmp5;

        /* Compute (1-u)(1-v) S11 + (1-u)v S12 + u(1-v) S21 + uv S22. */
        BzrSrfFactorLowOrders(Srf, &S11, &S12, &S21, &S22);

	/* Convert to S1 + u S2 + v S3 + uv S4. */
	S1 = S11;
	S2 = SymbSrfSub(S12, S11);
	S3 = SymbSrfSub(S21, S11);
	STmp1 = SymbSrfAdd(S11, S22);
	STmp2 = SymbSrfAdd(S21, S12);
	S4 = SymbSrfSub(STmp1, STmp2);

	CagdSrfFree(STmp1);
	CagdSrfFree(STmp2);
	CagdSrfFree(S12);
	CagdSrfFree(S21);
	CagdSrfFree(S22);

	/* 1st recursion call on S1 term. */
	MvarBzrSelfInter4VarDecomp(S1, U1MinusU3Factor, U2MinusU4Factor);

	/* 2nd recursion call on S2 term. */
	MvarBzrSelfInter4VarDecomp(S2, &MVTmp1, &MVTmp2);

	/* Multiply by u and add to U1MinusU3Factor, U2MinusU4Factor. */
	MVTmp3 = MvarBzrLinearInOneDir(4, 0, (MvarPointType) PType); /* u1. */
	MVarMVAccumulate(U1MinusU3Factor, MvarMVMult(MVTmp1, MVTmp3));
	MVarMVAccumulate(U2MinusU4Factor, MvarMVMult(MVTmp2, MVTmp3));
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp3);

	/* And add the second term. */
	MVTmp1 = MvarSrfToMV(S2);
	MVTmp2 = MvarPromoteMVToMV2(MVTmp1, 4, 2);
	MVarMVAccumulate(U1MinusU3Factor, MVTmp2);
	MvarMVFree(MVTmp1);

	/* 3rd recursion call on S3 term. */
	MvarBzrSelfInter4VarDecomp(S3, &MVTmp1, &MVTmp2);

	/* Multiply by u and add to U1MinusU3Factor, U2MinusU4Factor. */
	MVTmp3 = MvarBzrLinearInOneDir(4, 1, (MvarPointType) PType);/* u2. */
	MVarMVAccumulate(U1MinusU3Factor, MvarMVMult(MVTmp1, MVTmp3));
	MVarMVAccumulate(U2MinusU4Factor, MvarMVMult(MVTmp2, MVTmp3));
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp3);

	/* And add the second term. */
	MVTmp1 = MvarSrfToMV(S3);
	MVTmp2 = MvarPromoteMVToMV2(MVTmp1, 4, 2);
	MVarMVAccumulate(U2MinusU4Factor, MVTmp2);
	MvarMVFree(MVTmp1);

	/* 4th recursion call on S4 term. */
	MVTmp1 = MvarSrfToMV(S4);

	MVTmp2 = MvarPromoteMVToMV2(MVTmp1, 4, 0);
	MVTmp3 = MvarBzrLinearInOneDir(4, 0, (MvarPointType) PType); /* u1. */
	MVarMVAccumulate(U2MinusU4Factor, MvarMVMult(MVTmp2, MVTmp3));

	MVTmp4 = MvarPromoteMVToMV2(MVTmp1, 4, 2);
	MVTmp5 = MvarBzrLinearInOneDir(4, 3, (MvarPointType) PType); /* u4. */
	MVarMVAccumulate(U1MinusU3Factor, MvarMVMult(MVTmp4, MVTmp5));

	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp4);

	MvarBzrSelfInter4VarDecomp(S4, &MVTmp1, &MVTmp2);
	MVTmp4 = MvarMVMult(MVTmp3, MVTmp5); /* u1 * u4. */
	MvarMVFree(MVTmp3);
	MvarMVFree(MVTmp5);

	MVarMVAccumulate(U1MinusU3Factor, MvarMVMult(MVTmp1, MVTmp4));
	MVarMVAccumulate(U2MinusU4Factor, MvarMVMult(MVTmp2, MVTmp4));
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp4);

	CagdSrfFree(S1);
	CagdSrfFree(S2);
	CagdSrfFree(S3);
	CagdSrfFree(S4);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes self intersection points for a given surface S using the        M
* following constraints ((u, v) and (s, t) are two independent params of the M
* same srf):								     M
*									     M
*            x(u, v) - x(s, t) = 0,					     V
*            y(u, v) - y(s, t) = 0,					     V
*            z(u, v) - z(s, t) = 0.					     V
*                                                                            M
*   Direct attempt to solve this set of constraints is bound to be slow as   M
* all points in Crv satisfy these equations when u == v and s == t.  The key M
* here is to remove all (u - s) (and possibly (v - t)) factors off diagonal  M
* Bezier patches of the above, using a decomposition of the Bezier patches   M
* as (u1 - u3) G(u1, u2, u3, u4) + (u2 - u4) H(u1, u2, u3, u4).		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To detect its self intersecting points.                    M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  Self intersection points, as points in E4 (u, v, s, t). M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfAntipodalPoints, MvarCrvSelfInterNrmlDev                          M
*   MvarCrvSelfInterDiagFactor, MvarSrfSelfInterDiagFactor2                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfSelfInterDiagFactor                                               M
*****************************************************************************/
MvarPtStruct *MvarSrfSelfInterDiagFactor(const CagdSrfStruct *Srf,
					 CagdRType SubdivTol,
					 CagdRType NumericTol)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return MvarBzrSrfSelfInterDiagFactor(Srf, SubdivTol, NumericTol);
	case CAGD_SBSPLINE_TYPE:
	    return MvarBspSrfSelfInterDiagFactor(Srf, SubdivTol, NumericTol);
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    break;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Filters and purge solution points that are not satisfying equality       *
* constraints MVs, fron index MinMvs to index MaxMVs.			     *
*   This test is to make sure numerical accuracy is achieved for the 	     *
* ZERO_SUBDIV constraints as well that are ignored in the numeric improvment *
* stage.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVPts:  Solution points to filer.					     *
*   MVs:    Constraints to test the solution points against.                 *
*   MinMVs: Minimal constraint index to examine.                             *
*   MaxMVs: Maximal constraint index to examine.                             *
*   Tol:    Tolerance the solution point must satisfy.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:   Filtered points that satysify the given constraints    *
*****************************************************************************/
static MvarPtStruct *MvarFilterSltnsZeroCnstrns(MvarPtStruct *MVPts,
						MvarMVStruct **MVs,
						int MinMVs,
						int MaxMVs,
						IrtRType Tol)
{
    int i;
    MvarPtStruct *MVPt,
        *MVFiltered = NULL;

    while (MVPts != NULL) {
        IRIT_LIST_POP(MVPt, MVPts);

	for (i = MinMVs; i <= MaxMVs; i++) {
	    IrtRType
	        *R = MvarMVEval(MVs[i], MVPt -> Pt);

	    if (IRIT_FABS(R[1]) > Tol)
	        break;
	}

	if (i <= MaxMVs) {
	    MvarPtFree(MVPt);
	}
	else {
	    IRIT_LIST_PUSH(MVPt, MVFiltered);
	}
    }

    return MVFiltered;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a Bezier surface, S, compute its self intersections, if any, by    M
* computing a (u1 - u3) G(u1, u2, u3, u4) + (u2 - u4) H(u1, u2, u3, u4)      M
* decomposition for the 4-variate function of S(u1, u2)-S(u3, u4).	     M
*   The self intersection is then derived as the solution of:		     M
*         Gx Hy - Gy Hx = 0,						     V
*         Gx Hz - Gz Hx = 0,						     V
*         (u1 - u3) Gx + (u2 - u4) Hx = 0.				     V
*   The first two equations ensure the G an H are parallel while the last    M
* guarantee a zero point in X (which is therefore a zero point in Y an Z.)   M
*   Due to signularities in these equations, all six equations of the form:  M
*         Gx Hy - Gy Hx = 0,						     V
*         Gx Hz - Gz Hx = 0,						     V
*         Gy Hz - Gz Hy = 0,						     V
*         (u1 - u3) Gx + (u2 - u4) Hx = 0,				     V
*         (u1 - u3) Gy + (u2 - u4) Hy = 0,				     V
*         (u1 - u3) Gz + (u2 - u4) Hz = 0,				     V
* are employed during the subdivision stage.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          The surface to derive its self intersections.		     M
*   SubdivTol:    Tolerance of the first zero set finding subdivision stage. M
*   NumericTol:   Tolerance of the second zero set finding numeric stage.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *: List of self intersection points, as (u, v) parameter    M
*		    pairs into the surfaces' domain. Points in R^4.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBzrSelfInter4VarDecomp                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrSrfSelfInterDiagFactor                                            M
*****************************************************************************/
MvarPtStruct *MvarBzrSrfSelfInterDiagFactor(const CagdSrfStruct *Srf,
					    CagdRType SubdivTol,
					    CagdRType NumericTol)
{
    IRIT_STATIC_DATA MvarConstraintType
	Constraints[6] = { MVAR_CNSTRNT_ZERO,
			   MVAR_CNSTRNT_ZERO,
			   MVAR_CNSTRNT_ZERO,
			   MVAR_CNSTRNT_ZERO_SUBDIV,
			   MVAR_CNSTRNT_ZERO_SUBDIV,
			   MVAR_CNSTRNT_ZERO_SUBDIV };
    int i;
    MvarPtStruct *MVPts;
    CagdSrfStruct *CpSrf;
    MvarMVStruct *U1_U3Fctr, *U2_U4Fctr, *MVs[6],
	*U1_U3FctrXYZ[MVAR_MAX_PT_SIZE], *U2_U4FctrXYZ[MVAR_MAX_PT_SIZE],
	*MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4;

    if (!CAGD_IS_BEZIER_SRF(Srf)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BEZIER_EXPECTED);
	return NULL;
    }
    if (Srf -> UOrder != Srf -> VOrder) {
        /* Degree raise the lower order to the higher order one, as some of */
	/* the recursion code assumes UOrder == VOrder (was not really      */
        /* necessary but was simpler to implement).			    */
        Srf = CpSrf = BzrSrfDegreeRaiseN(Srf,
					 IRIT_MAX(Srf -> UOrder, Srf -> VOrder),
					 IRIT_MAX(Srf -> UOrder, Srf -> VOrder));
    }
    else
	CpSrf = NULL;

    /* COmpute the self-intersection decomposition. */
    MvarBzrSelfInter4VarDecomp(Srf, &U1_U3Fctr, &U2_U4Fctr);
    IRIT_GEN_COPY(U1_U3FctrXYZ, MvarMVSplitScalar(U1_U3Fctr),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    IRIT_GEN_COPY(U2_U4FctrXYZ, MvarMVSplitScalar(U2_U4Fctr),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MvarMVFree(U1_U3Fctr);
    MvarMVFree(U2_U4Fctr);

    /* Build (u1 - u3) and (u2 - u4). */
    MVTmp1 = MvarBzrLinearInOneDir(4, 0, MVAR_PT_E1_TYPE); /* u1. */
    MVTmp2 = MvarBzrLinearInOneDir(4, 2, MVAR_PT_E1_TYPE); /* u3. */
    MVTmp3 = MvarMVSub(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarBzrLinearInOneDir(4, 1, MVAR_PT_E1_TYPE); /* u2. */
    MVTmp2 = MvarBzrLinearInOneDir(4, 3, MVAR_PT_E1_TYPE); /* u4. */
    MVTmp4 = MvarMVSub(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    /* Build the constraints.  Note the order matters as only the first     */
    /* will participate in the second numeric improvmen tstage.		    */
    MVs[0] = MvarMVCrossProd2D(U1_U3FctrXYZ[1], U1_U3FctrXYZ[2],
			       U2_U4FctrXYZ[1], U2_U4FctrXYZ[2]);
    MVs[1] = MvarMVCrossProd2D(U1_U3FctrXYZ[1], U1_U3FctrXYZ[3],
			       U2_U4FctrXYZ[1], U2_U4FctrXYZ[3]);

    MVTmp1 = MvarMVMult(U1_U3FctrXYZ[3], MVTmp3);
    MVTmp2 = MvarMVMult(U2_U4FctrXYZ[3], MVTmp4);
    MVs[2] = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMult(U1_U3FctrXYZ[1], MVTmp3);
    MVTmp2 = MvarMVMult(U2_U4FctrXYZ[1], MVTmp4);
    MVs[3] = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMult(U1_U3FctrXYZ[2], MVTmp3);
    MVTmp2 = MvarMVMult(U2_U4FctrXYZ[2], MVTmp4);
    MVs[4] = MvarMVAdd(MVTmp1, MVTmp2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVs[5] = MvarMVCrossProd2D(U1_U3FctrXYZ[2], U1_U3FctrXYZ[3],
			       U2_U4FctrXYZ[2], U2_U4FctrXYZ[3]);

    for (i = 1; i < 4; i++) {
	MvarMVFree(U1_U3FctrXYZ[i]);
	MvarMVFree(U2_U4FctrXYZ[i]);
    }
    /* Solve the constraints. */
    MVPts = MvarMVsZeros(MVs, Constraints, 6, SubdivTol, NumericTol);

    /* keep only those points that are satisfying all 6 equations.  The     */
    /* first 3 were also verified by the solver.  Verify last 3 here.       */
    if (NumericTol < 0.0)
	MVPts = MvarFilterSltnsZeroCnstrns(MVPts, MVs, 3, 5, -NumericTol);

    for (i = 0; i < 6; i++)
        MvarMVFree(MVs[i]);

    if (CpSrf != NULL)
        CagdSrfFree(CpSrf);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   COmputes the intersection locations of two adjacent surfaces, that share M
* an edge (a boundary curve).  No intersections locations along the shared   M
* edge are returned.  This case is common for adjacent patches in a B-spline M
* surface.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:   The two adjacent surfaces to intersect.                    M
*   Srf1Bndry:    Boundary of Srf1 that is shared with Srf2.  Srf2 Boundary  M
*		  Must be the reciprocal boundary.  That is, if SrfBndry is  M
*		  UMin, Srf2's boundary will be UMax.			     M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumericTol:   Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *: List of intersection points, as (u, v) parameter         M
*		    pairs into the two surfaces' domains. Points in R^4.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBspSrfSelfInterDiagFactor, MvarBzrSrfSelfInterDiagFactor,	     M
*   BzrSrfFactorExtremeRowCol						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarAdjacentSrfSrfInter                                                  M
*****************************************************************************/
MvarPtStruct *MvarAdjacentSrfSrfInter(const CagdSrfStruct *Srf1,
				      const CagdSrfStruct *Srf2,
				      CagdSrfBndryType Srf1Bndry,
				      CagdRType SubdivTol,
				      CagdRType NumericTol)
{
    IRIT_STATIC_DATA MvarConstraintType
	Constraints[6] = { MVAR_CNSTRNT_ZERO,
			   MVAR_CNSTRNT_ZERO,
			   MVAR_CNSTRNT_ZERO,
			   MVAR_CNSTRNT_ZERO_SUBDIV,
			   MVAR_CNSTRNT_ZERO_SUBDIV,
			   MVAR_CNSTRNT_ZERO_SUBDIV };
    int i;
    CagdSrfStruct *Srfs1[4], *Srfs2[4], *Srf1r, *Srf2r;
    MvarMVStruct *MVs[6], *TMV, *MVs1[4], *MVs2[4];
    MvarPtStruct *MVPts;

    if (!CAGD_IS_BEZIER_SRF(Srf1) || !CAGD_IS_BEZIER_SRF(Srf2)) {
        MVAR_FATAL_ERROR(MVAR_ERR_BEZIER_EXPECTED);
	return NULL;
    }
    if (CAGD_IS_RATIONAL_SRF(Srf1) || CAGD_IS_RATIONAL_SRF(Srf2)) {
        MVAR_FATAL_ERROR(MVAR_ERR_RATIONAL_NO_SUPPORT);
        return NULL;
    }
    if (CAGD_NUM_OF_PT_COORD(Srf1 -> PType) < 3 ||
	CAGD_NUM_OF_PT_COORD(Srf2 -> PType) < 3) {
        MVAR_FATAL_ERROR(MVAR_ERR_ONLY_3D);
        return NULL;
    }

    switch (Srf1Bndry) {
	case CAGD_U_MIN_BNDRY:
	    Srf1r = BzrSrfFactorExtremeRowCol(Srf1, CAGD_U_MIN_BNDRY);
	    Srf2r = BzrSrfFactorExtremeRowCol(Srf2, CAGD_U_MAX_BNDRY);
	    break;
	case CAGD_U_MAX_BNDRY:
	    Srf1r = BzrSrfFactorExtremeRowCol(Srf1, CAGD_U_MAX_BNDRY);
	    Srf2r = BzrSrfFactorExtremeRowCol(Srf2, CAGD_U_MIN_BNDRY);
	    break;
	case CAGD_V_MIN_BNDRY:
	    Srf1r = BzrSrfFactorExtremeRowCol(Srf1, CAGD_V_MIN_BNDRY);
	    Srf2r = BzrSrfFactorExtremeRowCol(Srf2, CAGD_V_MAX_BNDRY);
	    break;
	case CAGD_V_MAX_BNDRY:
	    Srf1r = BzrSrfFactorExtremeRowCol(Srf1, CAGD_V_MAX_BNDRY);
	    Srf2r = BzrSrfFactorExtremeRowCol(Srf2, CAGD_V_MIN_BNDRY);
	    break;
	default:
	    Srf1r = Srf2r = NULL;
	    assert(0);
    }

    /* Solve for the intersection using 6 equations. 3 from the XYZ        */
    /* equality constraints of the original surface, and 3 from parallel   */
    /* constraints over the reduced functions.			           */

    /* Convert to multivariates the original surfaces. */
    SymbSrfSplitScalar(Srf1, &Srfs1[0], &Srfs1[1], &Srfs1[2], &Srfs1[3]);
    SymbSrfSplitScalar(Srf2, &Srfs2[0], &Srfs2[1], &Srfs2[2], &Srfs2[3]);
    MVs1[0] = NULL;
    for (i = 1; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs1[i]);
	CagdSrfFree(Srfs1[i]);
	MVs1[i] = MvarPromoteMVToMV2(TMV, 4, 0);
	MvarMVFree(TMV);
    }
    MVs2[0] = NULL;
    for (i = 1; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs2[i]);
	CagdSrfFree(Srfs2[i]);
	MVs2[i] = MvarPromoteMVToMV2(TMV, 4, 2);
	MvarMVFree(TMV);
    }

    /* Build the Srf1 == Srf2 equality constraint. */
    MVs[0] = MvarMVSub(MVs1[1], MVs2[1]);
    MVs[1] = MvarMVSub(MVs1[2], MVs2[2]);
    MVs[2] = MvarMVSub(MVs1[3], MVs2[3]);

    for (i = 1; i < 4; i++) {
        if (MVs1[i] != NULL)
	    MvarMVFree(MVs1[i]);
        if (MVs2[i] != NULL)
	    MvarMVFree(MVs2[i]);
    }

    /* Convert to multivariates the reduced surfaces. */
    SymbSrfSplitScalar(Srf1r, &Srfs1[0], &Srfs1[1], &Srfs1[2], &Srfs1[3]);
    SymbSrfSplitScalar(Srf2r, &Srfs2[0], &Srfs2[1], &Srfs2[2], &Srfs2[3]);
    CagdSrfFree(Srf1r);
    CagdSrfFree(Srf2r);
    MVs1[0] = NULL;
    for (i = 1; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs1[i]);
	CagdSrfFree(Srfs1[i]);
	MVs1[i] = MvarPromoteMVToMV2(TMV, 4, 0);
	MvarMVFree(TMV);
    }
    MVs2[0] = NULL;
    for (i = 1; i < 4; i++) {
	TMV = MvarSrfToMV(Srfs2[i]);
	CagdSrfFree(Srfs2[i]);
	MVs2[i] = MvarPromoteMVToMV2(TMV, 4, 2);
	MvarMVFree(TMV);
    }

    /* Add the parallel constraints. */
    MVs[3] = MvarMVCrossProd2D(MVs1[1], MVs1[2], MVs2[1], MVs2[2]);
    MVs[4] = MvarMVCrossProd2D(MVs1[1], MVs1[3], MVs2[1], MVs2[3]);
    MVs[5] = MvarMVCrossProd2D(MVs1[2], MVs1[3], MVs2[2], MVs2[3]);

    for (i = 1; i < 4; i++) {
        if (MVs1[i] != NULL)
	    MvarMVFree(MVs1[i]);
        if (MVs2[i] != NULL)
	    MvarMVFree(MVs2[i]);
    }

    /* Solve! */
    MVPts = MvarMVsZeros(MVs, Constraints, 6, SubdivTol, NumericTol);

    /* keep only those points that are satisfying all 6 equations.  The    */
    /* first 3 were also verified by the solver.  Verify last 3 here.      */
    if (NumericTol < 0.0)
        MVPts = MvarFilterSltnsZeroCnstrns(MVPts, MVs, 3, 5, -NumericTol);

    for (i = 0; i < 6; i++)
        MvarMVFree(MVs[i]);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Maps the computes set of solution points to the real (Bspline) domain of *
* the two intersected patches as they are represented as beziers now.        *
*   Then, merge the mapped point to the master list of solution points.      *
*                                                                            *
* PARAMETERS:                                                                *
*   MVPts:    List of points to map and merge.                               *
*   BzrSrf1:  First surface in the intersection test.                        *
*   BzrSrf2:  Second surface in the intersection test.                       *
*   MVPtsAll: Master list of solution points to merge into once done.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  Returned appended master list.                          *
*****************************************************************************/
static MvarPtStruct *MvarPtsMapAndAppend(MvarPtStruct *MVPts,
					 CagdSrfStruct *BzrSrf1,
					 CagdSrfStruct *BzrSrf2,
					 MvarPtStruct *MVPtsAll)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2,
        DU1, DV1, DU2, DV2;
    MvarPtStruct *MVPt;

    if (MVPts == NULL)
        return MVPtsAll;

    MVAR_SRF_BZR_FROM_BSP_DOMAIN(BzrSrf1, UMin1, UMax1, VMin1, VMax1);
    DU1 = UMax1 - UMin1;
    DV1 = VMax1 - VMin1;

    MVAR_SRF_BZR_FROM_BSP_DOMAIN(BzrSrf2, UMin2, UMax2, VMin2, VMax2);
    DU2 = UMax2 - UMin2;
    DV2 = VMax2 - VMin2;

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        MVPt -> Pt[0] = MVPt -> Pt[0] * DU1 + UMin1;
        MVPt -> Pt[1] = MVPt -> Pt[1] * DV1 + VMin1;
        MVPt -> Pt[3] = MVPt -> Pt[3] * DU2 + UMin2;
        MVPt -> Pt[4] = MVPt -> Pt[4] * DV2 + VMin2;
    }

    return CagdListAppend(MVPts, MVPtsAll);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Purge the given (U1, V1, U2, V2) point from the points list.             *
*                                                                            *
* PARAMETERS:                                                                *
*   MVPts:           List of point to pruge the point from.                  *
*   U1, V1, U2, V2:  The coordinate of the point to purge.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:    List of point, after the point was purged.           *
*****************************************************************************/
static MvarPtStruct *MvarBspSrfSelfInterPurgePt(MvarPtStruct *MVPts,
						IrtRType U1,
						IrtRType V1,
						IrtRType U2,
						IrtRType V2)
{
    MvarPtStruct *MVPt, *MVPtNext;

    if (MVPts == NULL)
        return NULL;

    if (IRIT_APX_EQ(MVPts -> Pt[0], U1) &&
	IRIT_APX_EQ(MVPts -> Pt[1], V1) &&
	IRIT_APX_EQ(MVPts -> Pt[2], U2) &&
	IRIT_APX_EQ(MVPts -> Pt[3], V2)) {
        MVPt = MVPts;
        MVPts = MVPts -> Pnext;
        MvarPtFree(MVPt);
    }
    else {
        for (MVPt = MVPts, MVPtNext = MVPts -> Pnext;
	     MVPtNext != NULL;
	     MVPt = MVPtNext, MVPtNext = MVPtNext -> Pnext) {
	    if (IRIT_APX_EQ(MVPtNext -> Pt[0], U1) &&
		IRIT_APX_EQ(MVPtNext -> Pt[1], V1) &&
		IRIT_APX_EQ(MVPtNext -> Pt[2], U2) &&
		IRIT_APX_EQ(MVPtNext -> Pt[3], V2)) {
	        MVPt -> Pnext = MVPtNext -> Pnext;
		MvarPtFree(MVPtNext);
		break;
	    }
	}
    }

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a Bspline surface, S, compute its self intersections, if any, by   M
* dividing it at all internal knots and examining all patches against all    M
* other patches.  Diagonal patches should also be examined against           M
* themselves with the aid of MvarBzrSrfSelfInterDiagFactor.		     M
*   Adjacent patches are sharing an edge (a curve) and hence spacial care    M
* is taken in those cases to eliminate and ignore that shared curve, in	     M
* MvarAdjacentSrfSrfInter.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          The surface to derive its self intersections.		     M
*   SubdivTol:    Tolerance of the first zero set finding subdivision stage. M
*   NumericTol:   Tolerance of the second zero set finding numeric stage.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *: List of self intersection points, as (u, v) parameter    M
*		    pairs into the surfaces' domain. Points in R^4.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*    MvarAdjacentSrfSrfInter, MvarBzrSrfSelfInterDiagFactor		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspSrfSelfInterDiagFactor                                            M
*****************************************************************************/
MvarPtStruct *MvarBspSrfSelfInterDiagFactor(const CagdSrfStruct *Srf,
					    CagdRType SubdivTol,
					    CagdRType NumericTol)
{
    if (CAGD_IS_BEZIER_SRF(Srf)) {
        return MvarBzrSrfSelfInterDiagFactor(Srf, SubdivTol, NumericTol);
    }
    else if (CAGD_IS_BSPLINE_SRF(Srf)) {
        CagdBType UClosed, VClosed;
        CagdRType UMin, UMax, VMin, VMax, UMin1, UMax1, VMin1, VMax1;
	CagdCrvStruct *Crv1, *Crv2;
        CagdSrfStruct *TSrf1, *TSrf2,
	    *Srfs = CagdCnvrtBsp2BzrSrf(Srf);
	MvarPtStruct *MVPts,
	    *MVPtsAll = NULL;

	/* Lets see of the surface is closed. */
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	Crv1 = CagdCrvFromSrf(Srf, UMin, CAGD_CONST_U_DIR);
	Crv2 = CagdCrvFromSrf(Srf, UMax, CAGD_CONST_U_DIR);
	UClosed = CagdCrvsSame(Crv1, Crv2, IRIT_EPS);
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	Crv1 = CagdCrvFromSrf(Srf, VMin, CAGD_CONST_V_DIR);
	Crv2 = CagdCrvFromSrf(Srf, VMax, CAGD_CONST_V_DIR);
	VClosed = CagdCrvsSame(Crv1, Crv2, IRIT_EPS);
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);

	/* Now examine of intersections among all patches.  Every patches   */
	/* should also be examined against itself.			    */
	while (Srfs != NULL) {
	    IRIT_LIST_POP(TSrf1, Srfs);

	    /* Examine for self intersections against itself. */
	    MVPts = MvarBzrSrfSelfInterDiagFactor(TSrf1, SubdivTol,
						  NumericTol);

	    /* Points were computed in a Bezier space of [0, 1]^2.  We  */
	    /* better map them to the B-spline domain of this patches   */
	    MVPtsAll = MvarPtsMapAndAppend(MVPts, TSrf1, TSrf1, MVPtsAll);

	    /* Examine this patch against all other patches. */
	    MVAR_SRF_BZR_FROM_BSP_DOMAIN(TSrf1, UMin1, UMax1, VMin1, VMax1);
	    for (TSrf2 = Srfs; TSrf2 != NULL; TSrf2 = TSrf2 -> Pnext) {
	        CagdRType UMin2, UMax2, VMin2, VMax2;

		MVAR_SRF_BZR_FROM_BSP_DOMAIN(TSrf2,
					     UMin2, UMax2, VMin2, VMax2);

		if (IRIT_APX_EQ(UMin1, UMin2) && IRIT_APX_EQ(UMax1, UMax2)) {
		    /* Surfaces share the same U Domain. */
		    if (IRIT_APX_EQ(VMax1, VMin2) ||/* TSrf1 is below TSrf2 in V.*/
			(VClosed &&
			 IRIT_APX_EQ(VMax1, VMax) &&/* Same - across closed srf. */
			 IRIT_APX_EQ(VMin2, VMin)))
		        MVPts = MvarAdjacentSrfSrfInter(TSrf1, TSrf2,
							CAGD_V_MAX_BNDRY,
							SubdivTol, NumericTol);
		    else if (IRIT_APX_EQ(VMax2, VMin1) ||/* TSrf1 above TSrf2, U.*/
			     (VClosed &&
			      IRIT_APX_EQ(VMax2, VMax) &&  /* Same - closed srf. */
			      IRIT_APX_EQ(VMin1, VMin)))
		        MVPts = MvarAdjacentSrfSrfInter(TSrf1, TSrf2,
							CAGD_V_MIN_BNDRY,
							SubdivTol, NumericTol);
		    else  /* No common boundary curve between two surfaces. */
		        MVPts = MvarSrfSrfInter(TSrf1, TSrf2, SubdivTol,
						NumericTol, FALSE);
		}
		else if (IRIT_APX_EQ(VMin1, VMin2) && IRIT_APX_EQ(VMax1, VMax2)) {
		    /* Surface share the same V Domain. */
		    if (IRIT_APX_EQ(UMax1, UMin2) ||/* TSrf1 is below TSrf2 in U.*/
			(UClosed &&
			 IRIT_APX_EQ(UMax1, UMax) &&/* Same - across closed srf. */
			 IRIT_APX_EQ(UMin2, UMin)))
		        MVPts = MvarAdjacentSrfSrfInter(TSrf1, TSrf2,
							CAGD_U_MAX_BNDRY,
							SubdivTol, NumericTol);
		    else if (IRIT_APX_EQ(UMax2, UMin1) ||/* TSrf1 above TSrf2, U.*/
			     (UClosed &&
			      IRIT_APX_EQ(UMax2, UMax) &&  /* Same - closed srf. */
			      IRIT_APX_EQ(UMin1, UMin)))
		         MVPts = MvarAdjacentSrfSrfInter(TSrf1, TSrf2,
							CAGD_U_MIN_BNDRY,
							SubdivTol, NumericTol);
		    else  /* No common boundary curve between two surfaces. */
		        MVPts = MvarSrfSrfInter(TSrf1, TSrf2, SubdivTol,
						NumericTol, FALSE);
		}
		else {    /* No common boundary curve between two surfaces. */
		    MVPts = MvarSrfSrfInter(TSrf1, TSrf2, SubdivTol,
					    NumericTol, FALSE);

		    /* Filter corner points if two patches share a corner.  */
		    if (IRIT_APX_EQ(UMin1, UMax2) ||
			(UClosed &&
			 IRIT_APX_EQ(UMin1, UMin) &&       /* Same - closed srf. */
			 IRIT_APX_EQ(UMax2, UMax))) {
		        if (IRIT_APX_EQ(VMin1, VMax2) ||
			    (VClosed &&
			     IRIT_APX_EQ(VMin1, VMin) &&
			     IRIT_APX_EQ(VMax2, VMax)))
			    MVPts = MvarBspSrfSelfInterPurgePt(MVPts,
							       0, 0, 1, 1);
			else if (IRIT_APX_EQ(VMax1, VMin2) ||
				 (VClosed &&
				  IRIT_APX_EQ(VMax1, VMax) &&
				  IRIT_APX_EQ(VMin2, VMin)))
			    MVPts = MvarBspSrfSelfInterPurgePt(MVPts,
							       0, 1, 1, 0);
		    }
		    else if (IRIT_APX_EQ(UMax1, UMin2) ||
			     (UClosed &&
			      IRIT_APX_EQ(UMax1, UMax) &&  /* Same - closed srf. */
			      IRIT_APX_EQ(UMin2, UMin))) {
		        if (IRIT_APX_EQ(VMin1, VMax2) ||
			    (VClosed &&
			     IRIT_APX_EQ(VMin1, VMin) &&
			     IRIT_APX_EQ(VMax2, VMax)))
			    MVPts = MvarBspSrfSelfInterPurgePt(MVPts,
							       1, 0, 0, 1);
			else if (IRIT_APX_EQ(VMax1, VMin2) ||
				 (VClosed &&
				  IRIT_APX_EQ(VMax1, VMax) &&
				  IRIT_APX_EQ(VMin2, VMin)))
			    MVPts = MvarBspSrfSelfInterPurgePt(MVPts,
							       1, 1, 0, 0);
		    }
		}

		/* Points were computed in a Bezier space of [0, 1]^2.  We  */
		/* better map them to the B-spline domain of this patches   */
		MVPtsAll = MvarPtsMapAndAppend(MVPts, TSrf1, TSrf2, MVPtsAll);
	    }

	    CagdSrfFree(TSrf1);
	}

	return MVPtsAll;
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_BEZ_OR_BSP_EXPECTED);
	return NULL;
    }
}

#ifdef DEBUG_TEST_SELF_INTER_4VAR_DECOMP

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A test program for the Bezier self-intersection decomposition code.      *
*****************************************************************************/
void main(int argc, char **argv)
{
    int i, j;
    IPObjectStruct
        *PObj = IPGetDataFiles(&argv[1], 1, TRUE, TRUE);
    CagdSrfStruct
        *Srf = PObj -> U.Srfs;
    CagdPointType
	PType = Srf -> PType;
    MvarMVStruct *U1MinusU3Factor, *U2MinusU4Factor,
	*MVTmp1, *MVTmp2, *MVTmp3, *MVTmp4, *MVTmp5, *MVTmp6;

    for (i = 0; i < 100; i++) {
        /* COmpute the self-intersection decomposition. */
        MvarBzrSelfInter4VarDecomp(Srf, &U1MinusU3Factor, &U2MinusU4Factor);

	if (i == 0) {
	    MvarPtStruct *MVPt,
	        *MVPts = MvarSrfSelfInterDiagFactor(Srf, 0.01, -1e-8);

	    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
	        CagdRType
		    *R = CagdSrfEval(Srf, MVPt -> Pt[0], MVPt -> Pt[1]);

		fprintf(stderr, "[OBJECT NONE\n    [POINT %f  %f  %f]\n]\n",
			R[1], R[2], R[3]);
	    }
	}

	/* Compute a 4-variate Srf(u1, u2) - Srf(u3, u4) to compare to. */
	MVTmp1 = MvarSrfToMV(Srf);
	MVTmp2 = MvarPromoteMVToMV2(MVTmp1, 4, 0);
	MVTmp3 = MvarPromoteMVToMV2(MVTmp1, 4, 2);
	MVTmp6 = MvarMVSub(MVTmp2, MVTmp3);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp3);

	/* Now test the result by multiplying the first by (u1 - u3) and    */
	/* the second by u2 - u4, and summing the two expressions up.       */
	MVTmp1 = MvarBzrLinearInOneDir(4, 0, PType); /* u1. */
	MVTmp2 = MvarBzrLinearInOneDir(4, 2, PType); /* u3. */
	MVTmp3 = MvarMVSub(MVTmp1, MVTmp2);
	MVTmp4 = MvarMVMult(MVTmp3, U1MinusU3Factor);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp3);

	MVTmp1 = MvarBzrLinearInOneDir(4, 1, PType); /* u2. */
	MVTmp2 = MvarBzrLinearInOneDir(4, 3, PType); /* u4. */
	MVTmp3 = MvarMVSub(MVTmp1, MVTmp2);
	MVTmp5 = MvarMVMult(MVTmp3, U2MinusU4Factor);
	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp2);
	MvarMVFree(MVTmp3);

	MVTmp1 = MvarMVAdd(MVTmp4, MVTmp5);
	MvarMVFree(MVTmp4);
	MvarMVFree(MVTmp5);

	fprintf(stderr, "Decomposition (Order = %d) computed the %s answer.\n",
		Srf -> UOrder,
		MvarMVsSame(MVTmp1, MVTmp6, IRIT_EPS) ? "right" : "WRONG");

	MvarMVFree(MVTmp1);
	MvarMVFree(MVTmp6);

	MvarMVFree(U1MinusU3Factor);
	MvarMVFree(U2MinusU4Factor);

	/* Allocate a random surface. */
	if (i > 0)
	    CagdSrfFree(Srf);
	Srf = BzrSrfNew(i % 5 + 2, i % 5 + 2, PType = CAGD_PT_E1_TYPE);
	for (j = Srf -> UOrder * Srf -> VOrder - 1; j >= 0; j--)
	    Srf -> Points[1][j] = IritRandom(-1.0, 1.0);
    }
}

#endif /* DEBUG_TEST_SELF_INTER_4VAR_DECOMP */
