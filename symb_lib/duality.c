/******************************************************************************
* Duality.c - Compute the dual of a given curve or a surface.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 02.					      *
******************************************************************************/

#include "symb_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the dual of the given curve.  The dual curve is a mapping of    M
* the tangent lines of Crv (for which Crv is the envelop of) to points in    M
* the dual space.							     M
*   Duality is derived by computing the tangent line "Ax + By + C = 0" to    M
* curve Crv and mapping this line to homogeneous point (A/C, B/C).           M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   The curve to compute its dual.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The dual curve.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDual                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvDual                                                              M
*****************************************************************************/
CagdCrvStruct *SymbCrvDual(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *A, *B, *C, *TCrv1, *TCrv2, *TCrv3,
	*CrvX, *CrvY, *CrvZ, *CrvW, *DCrvX, *DCrvY, *DCrvZ, *DCrvW;

    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);

    TCrv1 = CagdCrvDeriveScalar(Crv);

    SymbCrvSplitScalar(TCrv1, &DCrvW, &DCrvX, &DCrvY, &DCrvZ);
    CagdCrvFree(TCrv1);

    if (CrvW != NULL) {
	TCrv1 = SymbCrvMult(DCrvX, CrvW);
	TCrv2 = SymbCrvMult(CrvX, DCrvW);
	B = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);

	TCrv1 = SymbCrvMult(DCrvY, CrvW);
	TCrv2 = SymbCrvMult(CrvY, DCrvW);
	TCrv3 = SymbCrvSub(TCrv1, TCrv2);
	A = SymbCrvScalarScale(TCrv3, -1);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	CagdCrvFree(TCrv3);
    }
    else {
        B = DCrvX;
	DCrvX = NULL;

        A = SymbCrvScalarScale(DCrvY, -1);
    }

    TCrv1 = SymbCrvMult(A, CrvX);
    TCrv2 = SymbCrvMult(B, CrvY);
    TCrv3 = SymbCrvAdd(TCrv1, TCrv2);
    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);
    C = SymbCrvScalarScale(TCrv3, -1);
    CagdCrvFree(TCrv3);

    if (CrvW != NULL) {
	TCrv1 = SymbCrvMult(A, CrvW);
	CagdCrvFree(A);
	A = TCrv1;

	TCrv1 = SymbCrvMult(B, CrvW);
	CagdCrvFree(B);
	B = TCrv1;
    }

    CagdCrvFree(CrvW);
    CagdCrvFree(CrvX);
    CagdCrvFree(CrvY);
    CagdCrvFree(CrvZ);

    CagdCrvFree(DCrvW);
    CagdCrvFree(DCrvX);
    CagdCrvFree(DCrvY);
    CagdCrvFree(DCrvZ);

    if (!CagdMakeCrvsCompatible(&A, &C, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&B, &C, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&A, &B, TRUE, TRUE))
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

    TCrv1 = SymbCrvMergeScalar(C, A, B, NULL);
    CagdCrvFree(A);
    CagdCrvFree(B);
    CagdCrvFree(C);

    return TCrv1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Computes the dual of the given syrface.  The dual curve is a mapping of   M
* the tangent planes of Srf (for which Srf is the envelop of) to points in   M
* the dual space.							     M
*   Duality is derived by computing the tangent plane "Ax + By + Cz + D = 0" M
* to surface Srf and mapping this plane to homogeneous point (A/D, B/D, C/D).M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   The surface to compute its dual.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The dual surface.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDual                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfDual                                                              M
*****************************************************************************/
CagdSrfStruct *SymbSrfDual(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *A, *B, *C, *D, *Nrml, *TSrf1, *TSrf2, *TSrf3,
	*SrfX, *SrfY, *SrfZ, *SrfW;

    Nrml = SymbSrfNormalSrf(Srf);
    SymbSrfSplitScalar(Nrml, &TSrf1, &A, &B, &C);
    CagdSrfFree(Nrml);
    CagdSrfFree(TSrf1);

    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);

    TSrf1 = SymbSrfMult(A, SrfX);
    TSrf2 = SymbSrfMult(B, SrfY);
    TSrf3 = SymbSrfAdd(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    if (SrfZ != NULL) {
	TSrf1 = SymbSrfMult(C, SrfZ);
	TSrf2 = SymbSrfAdd(TSrf3, TSrf1);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf3);
	TSrf3 = TSrf2;
    }
    D = SymbSrfScalarScale(TSrf3, -1);
    CagdSrfFree(TSrf3);

    if (SrfW != NULL) {
	TSrf1 = SymbSrfMult(A, SrfW);
	CagdSrfFree(A);
	A = TSrf1;

	TSrf1 = SymbSrfMult(B, SrfW);
	CagdSrfFree(B);
	B = TSrf1;

	TSrf1 = SymbSrfMult(C, SrfW);
	CagdSrfFree(C);
	C = TSrf1;
    }

    CagdSrfFree(SrfW);
    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(SrfZ);

    if (!CagdMakeSrfsCompatible(&A, &D, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&B, &D, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&C, &D, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&A, &B, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&A, &C, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&B, &C, TRUE, TRUE, TRUE, TRUE))
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

    TSrf1 = SymbSrfMergeScalar(D, A, B, C);
    CagdSrfFree(A);
    CagdSrfFree(B);
    CagdSrfFree(C);
    CagdSrfFree(D);

    return TSrf1;
}
