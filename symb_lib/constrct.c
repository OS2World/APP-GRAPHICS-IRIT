/******************************************************************************
* Constrct.c - construction of surfaces using algebraic tools.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 98					      *
******************************************************************************/

#include "symb_loc.h"

static CagdSrfStruct *CagdPromoteCrvToSrfWithOtherCrv(const CagdCrvStruct
						                         *Crv,
						      CagdSrfDirType Dir,
						      const CagdCrvStruct
						                   *OtherCrv);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Promotes a curve into a surface along Dir, and set the otehr dir's	     *
* domain to follow OtherCrv domain.                                          *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:      To promote int a surface.                                      *
*   Dir:      of promotion, U or V.                                          *
*   OtherCrv: To govern the pther domain of the constructed surface.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CgadSrfStruct *:   Promoted surface.                                     *
*****************************************************************************/
static CagdSrfStruct *CagdPromoteCrvToSrfWithOtherCrv(const CagdCrvStruct
						                         *Crv,
						      CagdSrfDirType Dir,
						      const CagdCrvStruct
						                    *OtherCrv)
{
    CagdRType TMin, TMax;
    CagdSrfStruct
        *Srf = CagdPromoteCrvToSrf(Crv, Dir);

    CagdCrvDomain(OtherCrv, &TMin, &TMax);

    if (!IRIT_APX_EQ(TMin, 0.0) || !IRIT_APX_EQ(TMax, 1.0)) {
        if (CAGD_IS_BEZIER_SRF(Srf)) {
	    CagdSrfStruct
	        *TSrf = CagdCnvrtBzr2BspSrf(Srf);

	    CagdSrfFree(Srf);
	    Srf = TSrf;
	}

	switch (Dir) {
	    case CAGD_CONST_U_DIR:
		BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
					 CAGD_SRF_VPT_LST_LEN(Srf)
					     + Srf -> VOrder, TMin, TMax);
		break;
	    case CAGD_CONST_V_DIR:
		BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
					 CAGD_SRF_UPT_LST_LEN(Srf)
					     + Srf -> UOrder, TMin, TMax);
		break;
	    default:
	        assert(0);
	}
    }

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds up algebraically the given two curves, C1(r) and C2(t).  The result M
* is a bivariate surface S(r, t) = C1(r) + C2(t).                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:    Two curves to sum algebraically.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    A surface represent their sum, algebraically.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSwungAlgSumSrf, SymbAlgebraicProdSrf	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbAlgebraicSumSrf                                                      M
*****************************************************************************/
CagdSrfStruct *SymbAlgebraicSumSrf(const CagdCrvStruct *Crv1,
				   const CagdCrvStruct *Crv2)
{
    CagdSrfStruct *Srf,
        *Srf1 = CagdPromoteCrvToSrfWithOtherCrv(Crv1, CAGD_CONST_U_DIR, Crv2),
        *Srf2 = CagdPromoteCrvToSrfWithOtherCrv(Crv2, CAGD_CONST_V_DIR, Crv1);

    if (!CagdMakeSrfsCompatible(&Srf1, &Srf2, TRUE, TRUE, TRUE, TRUE) ||
	(Srf = SymbSrfAdd(Srf1, Srf2)) == NULL) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	return NULL;
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Multiply up algebraically the given two curves, C1(r) and C2(t).  The    M
* result is a bivariate surface S(r, t) = C1(r) C2(t).                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:    Two curves to multiply algebraically.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    A surface represent their product, algebraically.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbAlgebraicSumSrf, SymbSwungAlgSumSrf	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbAlgebraicProdSrf                                                     M
*****************************************************************************/
CagdSrfStruct *SymbAlgebraicProdSrf(const CagdCrvStruct *Crv1,
				    const CagdCrvStruct *Crv2)
{
    CagdSrfStruct *Srf,
        *Srf1 = CagdPromoteCrvToSrfWithOtherCrv(Crv1, CAGD_CONST_U_DIR, Crv2),
        *Srf2 = CagdPromoteCrvToSrfWithOtherCrv(Crv2, CAGD_CONST_V_DIR, Crv1);

    if (!CagdMakeSrfsCompatible(&Srf1, &Srf2, TRUE, TRUE, TRUE, TRUE) ||
	(Srf = SymbSrfMult(Srf1, Srf2)) == NULL) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	return NULL;
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds up algebraically the given two curves, C1(r) and C2(t), as swung    M
* surfaces (The NURBs book', by Piegl and Tiller, pp 455):		     M
*                                                                            M
*     S(r, t) = (x1(r) x2(t), x1(r) y2(t), y1(r))                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:    Two curves to sum algebraically, forming a swung surface. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    A surface represent their swung algebraic sum.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbAlgebraicProdSrf, SymbAlgebraicSumSrf		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSwungAlgSumSrf                                                       M
*****************************************************************************/
CagdSrfStruct *SymbSwungAlgSumSrf(const CagdCrvStruct *Crv1,
				  const CagdCrvStruct *Crv2)
{
    CagdSrfStruct *Srf, *SrfW, *SrfX, *SrfY, *SrfZ, *Srf1, *Srf2,
        *Srf1W, *Srf1X, *Srf1Y, *Srf1Z, *Srf2W, *Srf2X, *Srf2Y, *Srf2Z;

    if (CAGD_NUM_OF_PT_COORD(Crv1 -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(Crv1 -> PType) > 3 ||
	CAGD_NUM_OF_PT_COORD(Crv2 -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(Crv2 -> PType) > 3) {
	SYMB_FATAL_ERROR(SYMB_ERR_ONLY_2D_OR_3D);
	return NULL;
    }

    Srf1 = CagdPromoteCrvToSrfWithOtherCrv(Crv1, CAGD_CONST_U_DIR, Crv2);
    Srf2 = CagdPromoteCrvToSrfWithOtherCrv(Crv2, CAGD_CONST_V_DIR, Crv1);

    if (!CagdMakeSrfsCompatible(&Srf1, &Srf2, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);
	return NULL;
    }

    SymbSrfSplitScalar(Srf1, &Srf1W, &Srf1X, &Srf1Y, &Srf1Z);
    SymbSrfSplitScalar(Srf2, &Srf2W, &Srf2X, &Srf2Y, &Srf2Z);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    SrfX = SymbSrfMult(Srf1X, Srf2X);
    SrfY = SymbSrfMult(Srf1X, Srf2Y);
    SrfZ = Srf2W == NULL ? CagdSrfCopy(Srf1Y) : SymbSrfMult(Srf1Y, Srf2W);
    SrfW = Srf2W == NULL ? Srf1W
	                 : (Srf1W == NULL ? Srf2W : SymbSrfMult(Srf1W, Srf2W));
    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf2Y);
    if (Srf1Z != NULL)
	CagdSrfFree(Srf1Z);
    if (Srf2Z != NULL)
	CagdSrfFree(Srf2Z);
    if (Srf1W != NULL)
	CagdSrfFree(Srf1W);
    if (Srf2W != NULL)
	CagdSrfFree(Srf2W);

    if (!CagdMakeSrfsCompatible(&SrfX, &SrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfX, &SrfZ, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfY, &SrfZ, TRUE, TRUE, TRUE, TRUE) ||
	(SrfW != NULL &&
	 (!CagdMakeSrfsCompatible(&SrfW, &SrfX, TRUE, TRUE, TRUE, TRUE) ||
	  !CagdMakeSrfsCompatible(&SrfW, &SrfY, TRUE, TRUE, TRUE, TRUE) ||
	  !CagdMakeSrfsCompatible(&SrfW, &SrfZ, TRUE, TRUE, TRUE, TRUE)))) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	return NULL;
    }

    Srf = SymbSrfMergeScalar(SrfW, SrfX, SrfY, SrfZ);

    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(SrfZ);
    if (SrfW != NULL)
	CagdSrfFree(SrfW);

    return Srf;
}
