/******************************************************************************
* Tv0Jacob.c - approximate the zeros of the jacobian on the given trivariate  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct 99.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "mvar_lib.h"
#include "mrchcube.h"
#include "user_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a piecewise linear polygonal approximation to the zeros of the  M
* Jacobian of the given trivariate function.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:           Trivariate to derives its zero jacobian.                   M
*   Euclidean:    TRUE to evaluate into Euclidean space, FALSE to return     M
*		  the polygonal approximation in the parametric space of TV. M
*   SkipRate:     Of data in the volume.  1 skips nothing, 2 every second,   M
*		  etc.							     M
*   Fineness:     Of trivariate global refinment level, 0 for no ref.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The approximation of the zeros of the Jacobian, in   M
*		  all three axes.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserTrivarZeros                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserTVZeroJacobian                                                       M
*****************************************************************************/
IPObjectStruct *UserTVZeroJacobian(const TrivTVStruct *TV,
				   CagdBType Euclidean,
				   int SkipRate,
				   const CagdRType Fineness[3])
{
    MvarMVStruct
        *Mv = MvarTVToMV(TV),
	*DuMv = MvarMVDerive(Mv, 0),
	*DvMv = MvarMVDerive(Mv, 1),
	*DwMv = MvarMVDerive(Mv, 2),
	*MvTmp1 = MvarMVCrossProd(DuMv, DvMv),
	*MvTmp2 = MvarMVDotProd(MvTmp1, DwMv);
    TrivTVStruct
	*TVTmp = MvarMVToTV(MvTmp2);
    IPObjectStruct
        *JZero = UserTrivarZeros(TVTmp, Euclidean ? TV : NULL,
				 SkipRate, Fineness);

    MvarMVFree(Mv);
    MvarMVFree(DuMv);
    MvarMVFree(DvMv);
    MvarMVFree(DwMv);
    MvarMVFree(MvTmp1);
    MvarMVFree(MvTmp2);

    TrivTVFree(TVTmp);

    return JZero;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Approximate the zero set of a trivariate function.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:           Trivariate function to approximate its zero set.           M
*   TVEuclidean:  If provided, use this trivariate to evaluate into	     M
*		  Euclidean space.					     M
*   SkipRate:     Of data in the volume.  1 skips nothing, 2 every second,   M
*		  etc.							     M
*   Fineness:     Of trivariate global refinment level, 0 for no ref., in    M
*		  all three axes.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The approximation of the zeros of the trivariate.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserTVZeroJacobian                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserTrivarZeros                                                          M
*****************************************************************************/
IPObjectStruct *UserTrivarZeros(const TrivTVStruct *TV,
				const TrivTVStruct *TVEuclidean,
				int SkipRate,
				const CagdRType Fineness[3])
{
    int Len;
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax, *NewKvs, CpFineness[3];
    TrivTVStruct
	*TVTmp = TRIV_IS_BEZIER_TV(TV) ? TrivCnvrtBzr2BspTV(TV)
				       : TrivTVCopy(TV);
    IrtPtType CubeDim;
    IPObjectStruct *JZero;
    TrivTVStruct *TVTmp2;

    IRIT_GEN_COPY(CpFineness, Fineness, sizeof(CagdRType) * 3);

    /* Refine the trivariate as requested. */
    while (CpFineness[0]-- > 0) {
	Len = TVTmp -> ULength + TVTmp -> UOrder;
	NewKvs = BspKnotDoubleKnots(TVTmp -> UKnotVector, &Len,
				    TVTmp -> UOrder);
	TVTmp2 = TrivTVRefineAtParams(TVTmp, TRIV_CONST_U_DIR, FALSE,
				      NewKvs, Len);
	IritFree(NewKvs);
	TrivTVFree(TVTmp);
	TVTmp = TVTmp2;
    }
    while (CpFineness[1]-- > 0) {
	Len = TVTmp -> VLength + TVTmp -> VOrder;
	NewKvs = BspKnotDoubleKnots(TVTmp -> VKnotVector, &Len,
				    TVTmp -> VOrder);
	TVTmp2 = TrivTVRefineAtParams(TVTmp, TRIV_CONST_V_DIR, FALSE,
				      NewKvs, Len);
	IritFree(NewKvs);
	TrivTVFree(TVTmp);
	TVTmp = TVTmp2;
    }
    while (CpFineness[2]-- > 0) {
	Len = TVTmp -> WLength + TVTmp -> WOrder;
	NewKvs = BspKnotDoubleKnots(TVTmp -> WKnotVector, &Len,
				    TVTmp -> WOrder);
	TVTmp2 = TrivTVRefineAtParams(TVTmp, TRIV_CONST_W_DIR, FALSE,
				      NewKvs, Len);
	IritFree(NewKvs);
	TrivTVFree(TVTmp);
	TVTmp = TVTmp2;
    }

    TrivTVDomain(TVTmp, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);
    CubeDim[0] = (UMax - UMin) * SkipRate / TVTmp -> ULength;
    CubeDim[1] = (VMax - VMin) * SkipRate / TVTmp -> VLength;
    CubeDim[2] = (WMax - WMin) * SkipRate / TVTmp -> WLength;

    JZero = MCExtractIsoSurface2(TVTmp, 1, FALSE, CubeDim, SkipRate, 1.0, 0.0);
    TrivTVFree(TVTmp);
    
    if (TVEuclidean != NULL) {
        IPPolygonStruct *Pl;
	TrivTVStruct *DuTv, *DvTv;

	/* Prepare the partial derivative functions for normal computation. */
	DuTv = TrivTVDeriveScalar(TVEuclidean, TRIV_CONST_U_DIR);
	DvTv = TrivTVDeriveScalar(TVEuclidean, TRIV_CONST_V_DIR);

        /* Evaluate the set over the given, input, trivariate. */
        for (Pl = JZero -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    CagdRType *R;
	    IPVertexStruct
		*V = Pl -> PVertex;

	    do {
	        CagdVType Du, Dv;

		/* Compute normal. */
		R = TrivTVEval(DuTv, V -> Coord[0] + UMin,
			             V -> Coord[1] + VMin,
			             V -> Coord[2] + WMin);
		CagdCoerceToE3(Du, &R, -1, DuTv -> PType);
		R = TrivTVEval(DvTv, V -> Coord[0] + UMin,
			             V -> Coord[1] + VMin,
			             V -> Coord[2] + WMin);
		CagdCoerceToE3(Dv, &R, -1, DvTv -> PType);
		IRIT_CROSS_PROD(V -> Normal, Du, Dv);
		IRIT_VEC_NORMALIZE(V -> Normal);
		IP_SET_NORMAL_VRTX(V);

		/* Compute position. */
	        R = TrivTVEval(TVEuclidean, V -> Coord[0] + UMin,
			                    V -> Coord[1] + VMin,
			                    V -> Coord[2] + WMin);
		CagdCoerceToE3(V -> Coord, &R, -1, TVEuclidean -> PType);

	        V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);

	    /* Update plane normal and reverse if needs to. */
	    IPUpdatePolyPlane(Pl);
	    if (IRIT_DOT_PROD(Pl -> PVertex -> Normal, Pl -> Plane) < 0) {
	        Pl -> PVertex = IPReverseVrtxList2(Pl -> PVertex);
		IRIT_PT_SCALE(Pl -> Plane, -1.0);
		Pl -> Plane[3] *= -1.0;
	    }
	    IP_SET_PLANE_POLY(Pl);
	}

	TrivTVFree(DuTv);
	TrivTVFree(DvTv);
    }

    return JZero;
}
