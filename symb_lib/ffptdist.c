/******************************************************************************
* FfPtDist.c - Point distribution on freeforms.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "symb_loc.h"

#define PT_ON_SRF_FAIL_RATIO	1000

IRIT_STATIC_DATA CagdUVType
    *GlblDistUV = NULL;
IRIT_STATIC_DATA CagdRType
    *GlblDistProb = NULL;
IRIT_STATIC_DATA int
    GlblDistSize = 0;

#if defined(ultrix) && defined(mips)
static int CompareReal(VoidPtr PReal1, VoidPtr PReal2);
#else
static int CompareReal(const VoidPtr PReal1, const VoidPtr PReal2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a stocastically uniform distribution of points on a curve.      M
*   n points are placed at approximately equal distance from each other      M
* along Crv's arc length.  This distribution converges to a uniform          M
* distribution as n approached infinity.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           To place n points along, uniformly.                       M
*   ParamUniform:  If TRUE, produces a distribution uniform in parametric    M
*		   space. If FALSE, uniform in Euclidean space.		     M
*   n:             Number of points to distribute along Crv.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A dynamically allocated vector of size n, of the parameter M
*                 values of the distributed points.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbUniformAprxPtOnSrfDistrib                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbUniformAprxPtOnCrvDistrib, uniform distribution                      M
*****************************************************************************/
CagdRType *SymbUniformAprxPtOnCrvDistrib(const CagdCrvStruct *Crv,
					 CagdBType ParamUniform,
					 int n)
{
    int i,
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct
	*DCrv = CagdCrvDerive(Crv),
	*DCrvMagSqr = SymbCrvDotProd(DCrv, DCrv);
    CagdRType TMin, TMax,
	MaxDMag = -IRIT_INFNTY,
	*Dist = (CagdRType *) IritMalloc(n * sizeof(CagdRType)),
	*DPts = DCrvMagSqr -> Points[1];

    CagdCrvFree(DCrv);

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* Compute bounds on expected speeds of the curve. */
    for (i = 0; i < DCrvMagSqr -> Length; i++) {
	if (MaxDMag < DPts[i])
	    MaxDMag = DPts[i];
    }
    MaxDMag = sqrt(MaxDMag);

    i = 0;
    do {
	CagdRType
	    t = IritRandom(TMin, TMax),
	    *R = CagdCrvEval(DCrvMagSqr, t);

	if (IsRational)
	    R[1] /= R[0];

	if (ParamUniform || IritRandom(0.0, 1.0) < sqrt(R[1]) / MaxDMag)
	    Dist[i++] = t;
    }
    while (i < n);

    qsort(Dist, n, sizeof(CagdRType), CompareReal);

    return Dist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a stocastically uniform distribution of points on a surface.    M
*   n points are placed at approximately equal distance from each other on   M
* Srf's surface.  This distribution converges to a uniform distribution as   M
* n approached infinity.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           To place n points on, uniformly.                          M
*   ParamUniform:  If TRUE, produces a distribution uniform in parametric    M
*		   space. If FALSE, uniform in Euclidean space.		     M
*   n:             Number of points to distribute along Srf.                 M
*   EvalImportance:  Optional function to evaluate the importance of each    M
*		   selected points and if returning FALSE, that point is     M
*		   purged.  NULL to disable.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVType *:  A dynamically allocated vector of size n, of parameter    M
*                values of the distributed points.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbUniformAprxPtOnCrvDistrib                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbUniformAprxPtOnSrfDistrib, uniform distribution                      M
*****************************************************************************/
CagdUVType *SymbUniformAprxPtOnSrfDistrib(
		        const CagdSrfStruct *Srf,
			CagdBType ParamUniform,
			int n,
			SymbUniformAprxSrfPtImportanceFuncType EvalImportance)
{
    int i, MaxPtsTested,
	j = 0,
	IsRational = CAGD_IS_RATIONAL_SRF(Srf);
    CagdSrfStruct
	*DSrf = SymbSrfNormalSrf(Srf),
	*DSrfMagSqr = SymbSrfDotProd(DSrf, DSrf);
    CagdRType UMin, UMax, VMin, VMax,
	MaxDMag = -IRIT_INFNTY,
	*DPts = DSrfMagSqr -> Points[1];
    CagdUVType
	*Dist = (CagdUVType *) IritMalloc(n * sizeof(CagdUVType));

    CagdSrfFree(DSrf);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Compute bounds on expected differential area elements of the surface. */
    for (i = 0; i < DSrfMagSqr -> ULength * DSrfMagSqr -> VLength; i++) {
	if (MaxDMag < DPts[i])
	    MaxDMag = DPts[i];
    }
    MaxDMag = sqrt(MaxDMag);

    MaxPtsTested = PT_ON_SRF_FAIL_RATIO * n;

    i = 0;
    do {
	CagdRType
	    u = IritRandom(UMin, UMax),
	    v = IritRandom(VMin, VMax),
	    *R = CagdSrfEval(DSrfMagSqr, u, v);

	if (IsRational)
	    R[1] /= R[0];

	if (ParamUniform || IritRandom(0.0, 1.0) < sqrt(R[1]) / MaxDMag) {
	    if (EvalImportance == NULL || EvalImportance(Srf, u, v)) {
	        Dist[i][0] = u;
		Dist[i++][1] = v;
	    }
	}

	/* Make sure we do not stay here forever due to fails of the        */
	/* callback function EvalImportance...				    */
	if (j++ >= MaxPtsTested) {
	    while (i < n) {
		Dist[i][0] = -IRIT_INFNTY;
		Dist[i++][1] = -IRIT_INFNTY;
	    }
	    break;
	}
    }
    while (i < n);

    CagdSrfFree(DSrfMagSqr);

    return Dist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepares a uniform distribution of points on surface Srf.		     M
*   This function is invoked in preparation of several calls to function     M
* SymbUniformAprxPtOnSrfGetDistrib that return a uniform Euclidean           M
* distributions that is consistent with the area differentials found.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To place n points on its parametric space, uniformly.          M
*   n:        Number of points to distribute along Srf.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbUniformAprxPtOnCrvDistrib, SymbUniformAprxPtOnSrfDistrib,            M
*   SymbUniformAprxPtOnSrfGetDistrib					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbUniformAprxPtOnSrfPrepDistrib, uniform distribution                  M
*****************************************************************************/
void SymbUniformAprxPtOnSrfPrepDistrib(const CagdSrfStruct *Srf, int n)
{
    int i = 0;
    CagdRType UMin, UMax, VMin, VMax;

    if (GlblDistUV != NULL) {
	IritFree(GlblDistUV);
	IritFree(GlblDistProb);
    }

    GlblDistUV = (CagdUVType *) IritMalloc(n * sizeof(CagdUVType));
    GlblDistProb = (CagdRType *) IritMalloc(n * sizeof(CagdRType));

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    for (i = 0; i < n; i++) {
	CagdRType
	    u = IritRandom(UMin, UMax),
	    v = IritRandom(VMin, VMax);

	GlblDistUV[i][0] = u;
	GlblDistUV[i][1] = v;
	GlblDistProb[i] = IritRandom(0.0, 1.0);
    }

    GlblDistSize = n;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a uniform distribution of points on the surface Srf.            M
*   The points are placed at approximately equal distance from each other on M
* Srf's Euclidean space.  A subset of the n points that were selected via    M
* the last invokation of SymbUniformAprxPtOnSrfPrepDistrib is returned, such M
* that the points are at equal distance, approximately.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To place points on its parametric space, uniformly.            M
*	      This surface must have the same parameter domain as Srf in the M
*	      last invokation of SymbUniformAprxPtOnSrfPrepDistrib.	     M
*   n:        Returns actual number of UV locations in the returned vector.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVType *:  A dynamically allocated vector of at most n parameter     M
*		 values of the distributed points.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbUniformAprxPtOnCrvDistrib, SymbUniformAprxPtOnSrfDistrib,            M
*   SymbUniformAprxPtOnSrfPrepDistrib					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbUniformAprxPtOnSrfGetDistrib, uniform distribution                   M
*****************************************************************************/
CagdUVType *SymbUniformAprxPtOnSrfGetDistrib(const CagdSrfStruct *Srf, int *n)
{
    int i, j;
    CagdSrfStruct
	*DSrf = SymbSrfNormalSrf(Srf),
	*DSrfMagSqr = SymbSrfDotProd(DSrf, DSrf);
    CagdRType UMin, UMax, VMin, VMax,
	MaxDMag = -IRIT_INFNTY,
	*DPts = DSrfMagSqr -> Points[1];
    CagdUVType
	*Dist = (CagdUVType *) IritMalloc(GlblDistSize * sizeof(CagdUVType));

    CagdSrfFree(DSrf);
    if (CAGD_IS_RATIONAL_SRF(Srf)) {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	CagdSrfFree(DSrfMagSqr);
	return NULL;
    }

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Compute bounds on expected differential area elements of the surface. */
    for (i = 0; i < DSrfMagSqr -> ULength * DSrfMagSqr -> VLength; i++) {
	if (MaxDMag < DPts[i])
	    MaxDMag = DPts[i];
    }
    MaxDMag = sqrt(MaxDMag);

    for (i = j = 0; i < GlblDistSize; i++) {
	CagdRType
	    *R = CagdSrfEval(DSrfMagSqr, GlblDistUV[i][0], GlblDistUV[i][1]);

	if (GlblDistProb[i] < sqrt(R[1]) / MaxDMag) {
	    Dist[j][0] = GlblDistUV[i][0];
	    Dist[j++][1] = GlblDistUV[i][1];
	}
    }

    *n = j;

    CagdSrfFree(DSrfMagSqr);

    return Dist;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two real numbers for sorting purposes.                *
*                                                                            *
* PARAMETERS:                                                                *
*   PReal1, PReal2:  Two pointers to real numbers.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two reals.               *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int CompareReal(VoidPtr PReal1, VoidPtr PReal2)
#else
static int CompareReal(const VoidPtr PReal1, const VoidPtr PReal2)
#endif /* ultrix && mips (no const support) */
{
    CagdRType
	Diff = (*((CagdRType *) PReal1)) - (*((CagdRType *) PReal2));

    return IRIT_SIGN(Diff);
}
