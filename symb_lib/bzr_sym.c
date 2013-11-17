/******************************************************************************
* Bzr_Sym.c - Bezier symbolic computation.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 92.					      *
******************************************************************************/

#include "geom_lib.h"
#include "symb_loc.h"

#define PINDEX_IJ(i,j)         P[(i) + (j) * (m+1)]
#define QINDEX_IJ(i,j)         Q[(i) + (j) * m]

typedef struct SymbApproxSubdivNodeStruct {
    struct SymbApproxSubdivNodeStruct *Left, *Right;  /* Left & right subd. */
    CagdRType TMin, TMax;                       /* Domain of current curve. */
    int OneQuad;       /* In piecewise quadratic, TRUE signals one segment. */
} SymbApproxSubdivNodeStruct;

IRIT_STATIC_DATA SymbApproxLowDegStateType
    GlblSymbApproxLowDegState = SYMB_APPROX_LOW_DEG_NO_TREE;
IRIT_STATIC_DATA SymbApproxSubdivNodeStruct
    *GlblSymbApproxSubdivTree = NULL;

static void BzrSrfFactorBilinearAux0(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n);
static void BzrSrfFactorBilinearAux3(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n);
static void BzrSrfFactorBilinearAux1(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n);
static void BzrSrfFactorBilinearAux2(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n);
static void BzrApproxLowDegFreeTree(SymbApproxSubdivNodeStruct *Tree);
static CagdCrvStruct *ApproxCrvsAsCubics(
				      const CagdCrvStruct *Crv,
				      CagdRType Tol2,
				      SymbApproxSubdivNodeStruct *SubdivTree);
static CagdCrvStruct *ApproxCrvsAsQuadratics(
				      const CagdCrvStruct *Crv,
				      CagdRType Tol2,
				      SymbApproxSubdivNodeStruct *SubdivTree);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bezier curves - multiply them coordinatewise.		     M
*   The two curves are promoted to same point type before the multiplication M
* can take place.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2:   The two curves to multiply.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The product Crv1 * Crv2 coordinatewise.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvMultPtsVecs                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvMult, product                                                      M
*****************************************************************************/
CagdCrvStruct *BzrCrvMult(const CagdCrvStruct *CCrv1,
			  const CagdCrvStruct *CCrv2)
{
    IRIT_STATIC_DATA int
	CpPtsLen = 0;
    IRIT_STATIC_DATA CagdRType
	*CpPts1 = NULL,
	*CpPts2 = NULL;
    CagdBType IsNotRational;
    int i, j, ij, l, MaxCoord, IsScalar, ProdOrder,
	Order1 = CCrv1 -> Order,
	Order2 = CCrv2 -> Order,
	Degree1 = Order1 - 1,
        Degree2 = Order2 - 1,
        ProdDegree = Degree1 + Degree2;
    CagdCrvStruct *ProdCrv,
        *Crv1 = CagdCrvCopy(CCrv1),
	*Crv2 = CagdCrvCopy(CCrv2);
    CagdRType **PPoints, **Points1, **Points2;

    if (!CAGD_IS_BEZIER_CRV(Crv1) || !CAGD_IS_BEZIER_CRV(Crv2)) {
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	SYMB_FATAL_ERROR(SYMB_ERR_BZR_CRV_EXPECT);
	return NULL;
    }

    if (Crv1 -> PType != Crv2 -> PType &&
	!CagdMakeCrvsCompatible(&Crv1, &Crv2, FALSE, FALSE)) {
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	return NULL;
    }

    ProdCrv = BzrCrvNew(ProdOrder = Order1 + Order2 - 1, Crv1 -> PType);
    IsNotRational = !CAGD_IS_RATIONAL_CRV(ProdCrv);
    MaxCoord = CAGD_NUM_OF_PT_COORD(ProdCrv -> PType);
    IsScalar = IsNotRational && MaxCoord == 1;

    PPoints = ProdCrv -> Points;
    Points1 = Crv1 -> Points;
    Points2 = Crv2 -> Points;

    for (l = IsNotRational; l <= MaxCoord; l++)
        IRIT_ZAP_MEM(PPoints[l], sizeof(CagdRType) * ProdOrder);

    /* Allocate temporary data, if necessary. */
    if (CpPtsLen < IRIT_MAX(Order1, Order2)) {
        CpPtsLen = IRIT_MAX(Order1, Order2) * 2;
	if (CpPts1 != NULL)
	    IritFree(CpPts1);
	if (CpPts2 != NULL)
	    IritFree(CpPts2);
	CpPts1 = (CagdRType *) IritMalloc(sizeof(CagdRType) * CpPtsLen);
	CpPts2 = (CagdRType *) IritMalloc(sizeof(CagdRType) * CpPtsLen);
    }

    if (ProdOrder < CAGD_MAX_BEZIER_CACHE_ORDER) {             /* Use cache. */
        for (l = IsNotRational; l <= MaxCoord; l++) {
	    CagdRType
	        *PPts = PPoints[l];

	    /* Place the combinatorial coefficients with the control points. */
	    for (i = 0; i < Order1; i++)
	        CpPts1[i] = Points1[l][i] * CagdIChooseKTable[Degree1][i];
	    for (i = 0; i < Order2; i++)
	        CpPts2[i] = Points2[l][i] * CagdIChooseKTable[Degree2][i];

            /* Do the convolution. */
	    for (i = 0; i < Order1; i++) {
		for (j = 0, ij = i; j < Order2; j++, ij++)
		    PPts[ij] += CpPts1[i] * CpPts2[j];
	    }

	    /* Update the denominator combinatorial coefficient. */
	    for (i = 0; i < ProdOrder; i++)
	        PPts[i] /= CagdIChooseKTable[ProdDegree][i];
	}
    }
    else {
        for (l = IsNotRational; l <= MaxCoord; l++) {
	    CagdRType
	        *PPts = PPoints[l];

	    /* Place the combinatorial coefficients with the control points. */
	    for (i = 0; i < Order1; i++)
	        CpPts1[i] = Points1[l][i] * CagdIChooseK(i, Degree1);
	    for (i = 0; i < Order2; i++)
	        CpPts2[i] = Points2[l][i] * CagdIChooseK(i, Degree2);
        
            /* Do the convolution. */
	    for (i = 0; i < Order1; i++) {
		for (j = 0, ij = i; j < Order2; j++, ij++)
		    PPts[ij] += CpPts1[i] * CpPts2[j];
	    }

	    /* Update the denominator combinatorial coefficient. */
	    for (i = 0; i < ProdOrder; i++)
	        PPts[i] /= CagdIChooseK(i, ProdDegree);
	}
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return ProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bezier scalar curves as vectors Ptsi of orders Orderi, multiply  M
* them.									     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts1:    First vector of scalars of first Bezier curve.                  M
*   Order1:  Order of first Bezier curve.				     M
*   Pts2:    Second vector of scalars of second Bezier curve.                M
*   Order2:  Order of second Bezier curve.				     M
*   ProdPts: Result vector of scalars of product Bezier curve.  Result       M
*	     vector is of length Order1+Order2-1.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvMult                                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvMultPtsVecs, product                                               M
*****************************************************************************/
void BzrCrvMultPtsVecs(const CagdRType *Pts1,
		       int Order1,
		       const CagdRType *Pts2,
		       int Order2,
		       CagdRType *ProdPts)
{
    IRIT_STATIC_DATA int
	CpPtsLen = 0;
    IRIT_STATIC_DATA CagdRType
	*CpPts1 = NULL,
	*CpPts2 = NULL;
    int i, j, ij,
	Degree1 = Order1 - 1,
	Degree2 = Order2 - 1,
	ProdOrder = Order1 + Order2 - 1,
	ProdDegree = ProdOrder - 1;

    IRIT_ZAP_MEM(ProdPts, sizeof(CagdRType) * ProdOrder);

    /* Allocate temporary data, if necessary. */
    if (CpPtsLen < IRIT_MAX(Order1, Order2)) {
        CpPtsLen = IRIT_MAX(Order1, Order2) * 2;
	if (CpPts1 != NULL)
	    IritFree(CpPts1);
	if (CpPts2 != NULL)
	    IritFree(CpPts2);
	CpPts1 = (CagdRType *) IritMalloc(sizeof(CagdRType) * CpPtsLen);
	CpPts2 = (CagdRType *) IritMalloc(sizeof(CagdRType) * CpPtsLen);
    }

    if (ProdOrder < CAGD_MAX_BEZIER_CACHE_ORDER) {           /* Use cache. */
        /* Place the combinatorial coefficients with the control points. */
        for (i = 0; i < Order1; i++)
	    CpPts1[i] = Pts1[i] * CagdIChooseKTable[Degree1][i];
        for (i = 0; i < Order2; i++)
	    CpPts2[i] = Pts2[i] * CagdIChooseKTable[Degree2][i];

	/* Do the convolution. */
        for (i = 0; i < Order1; i++) {
	    for (j = 0, ij = i; j < Order2; j++, ij++)
		ProdPts[ij] += CpPts1[i] * CpPts2[j];
	}

        /* Update the denominator combinatorial coefficient. */
        for (i = 0; i < ProdOrder; i++)
	    ProdPts[i] /= CagdIChooseKTable[ProdDegree][i];
    }
    else {
        /* Place the combinatorial coefficients with the control points. */
        for (i = 0; i < Order1; i++)
	    CpPts1[i] = Pts1[i] * CagdIChooseK(i, Degree1);
        for (i = 0; i < Order2; i++)
	    CpPts2[i] = Pts2[i] * CagdIChooseK(i, Degree2);

	/* Do the convolution. */
        for (i = 0; i < Order1; i++) {
	    for (j = 0, ij = i; j < Order2; j++, ij++)
		ProdPts[ij] += CpPts1[i] * CpPts2[j];
	}

        /* Update the denominator combinatorial coefficient. */
        for (i = 0; i < ProdOrder; i++)
	    ProdPts[i] /= CagdIChooseK(i, ProdDegree);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bezier curve lists - multiply them one at a time.		     M
*   Return a Bezier curve lists representing their products.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Lst:    First list of Bezier curves to multiply.                     M
*   Crv2Lst:    Second list of Bezier curves to multiply.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A list of product curves                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvMultList, product                                                  M
*****************************************************************************/
CagdCrvStruct *BzrCrvMultList(const CagdCrvStruct *Crv1Lst,
			      const CagdCrvStruct *Crv2Lst)
{
    const CagdCrvStruct *Crv1, *Crv2;
    CagdCrvStruct
	*ProdCrvTail = NULL,
	*ProdCrvList = NULL;

    for (Crv1 = Crv1Lst, Crv2 = Crv2Lst;
	 Crv1 != NULL && Crv2 != NULL;
	 Crv1 = Crv1 -> Pnext, Crv2 = Crv2 -> Pnext) {
	CagdCrvStruct
	    *ProdCrv = BzrCrvMult(Crv1, Crv2);

	if (ProdCrvList == NULL)
	    ProdCrvList = ProdCrvTail = ProdCrv;
	else {
	    ProdCrvTail -> Pnext = ProdCrv;
	    ProdCrvTail = ProdCrv;
	}
    }

    return ProdCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two Bezier surfaces - multiply them coordinatewise.		     M
*   The two surfaces are promoted to same point type before multiplication   M
* can take place.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf1, CSrf2:   The two surfaces to multiply.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The product Srf1 * Srf2 coordinatewise.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfMult, product                                                      M
*****************************************************************************/
CagdSrfStruct *BzrSrfMult(const CagdSrfStruct *CSrf1,
			  const CagdSrfStruct *CSrf2)
{
    IRIT_STATIC_DATA int
	CpPtsLen = 0;
    IRIT_STATIC_DATA CagdRType
	*CpPts1 = NULL,
	*CpPts2 = NULL;
    CagdBType IsNotRational;
    int i, j, k, l, m, MaxCoord, UDegree, VDegree, UOrder, VOrder,
        Size,
	UOrder1 = CSrf1 -> UOrder,
	VOrder1 = CSrf1 -> VOrder,
	UOrder2 = CSrf2 -> UOrder,
	VOrder2 = CSrf2 -> VOrder,
	UDegree1 = UOrder1 - 1,
	VDegree1 = VOrder1 - 1,
	UDegree2 = UOrder2 - 1,
	VDegree2 = VOrder2 - 1;
    CagdSrfStruct *ProdSrf,
	*Srf1 = CagdSrfCopy(CSrf1),
        *Srf2 = CagdSrfCopy(CSrf2);
    CagdRType **PPoints, **Points1, **Points2;

    if (!CAGD_IS_BEZIER_SRF(CSrf1) || !CAGD_IS_BEZIER_SRF(CSrf2)) {
        CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);
	SYMB_FATAL_ERROR(SYMB_ERR_BZR_SRF_EXPECT);
	return NULL;
    }

    /* Allocate temporary data, if necessary. */
    if (CpPtsLen < IRIT_MAX(UOrder1, UOrder2) * IRIT_MAX(VOrder1, VOrder2)) {
        CpPtsLen = IRIT_MAX(UOrder1, UOrder2) * IRIT_MAX(VOrder1, VOrder2) * 2;
	if (CpPts1 != NULL)
	    IritFree(CpPts1);
	if (CpPts2 != NULL)
	    IritFree(CpPts2);
	CpPts1 = (CagdRType *) IritMalloc(sizeof(CagdRType) * CpPtsLen);
	CpPts2 = (CagdRType *) IritMalloc(sizeof(CagdRType) * CpPtsLen);
    }

    if (Srf1 -> PType != Srf2 -> PType &&
	!CagdMakeSrfsCompatible(&Srf1, &Srf2,
			        FALSE, FALSE, FALSE, FALSE)) {
        CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	return NULL;
    }

    ProdSrf = BzrSrfNew(UOrder = UOrder1 + UOrder2 - 1,
			VOrder = VOrder1 + VOrder2 - 1, Srf1 -> PType);
    IsNotRational = !CAGD_IS_RATIONAL_SRF(ProdSrf);
    MaxCoord = CAGD_NUM_OF_PT_COORD(ProdSrf -> PType);
    Size = UOrder * VOrder;
    UDegree = UOrder - 1;
    VDegree = VOrder - 1;

    PPoints = ProdSrf -> Points;
    Points1 = Srf1 -> Points;
    Points2 = Srf2 -> Points;
    
    for (k = IsNotRational; k <= MaxCoord; k++)
	IRIT_ZAP_MEM(PPoints[k], sizeof(CagdRType) * Size);

    if (UOrder < CAGD_MAX_BEZIER_CACHE_ORDER &&
	VOrder < CAGD_MAX_BEZIER_CACHE_ORDER) {
        for (k = IsNotRational; k <= MaxCoord; k++) {
	    CagdRType *p, *p1, *p2,
	        *PPts = PPoints[k];

	    /* Place the combinatorial coefficients with the control points. */
	    for (p1 = Points1[k], p = CpPts1, j = l = 0; j < VOrder1; j++) {
	        for (i = 0; i < UOrder1; i++, l++)
		    *p++ = *p1++ * CagdIChooseKTable[VDegree1][j]
				 * CagdIChooseKTable[UDegree1][i];
	    }
	    for (p2 = Points2[k], p = CpPts2, j = l = 0; j < VOrder2; j++) {
	        for (i = 0; i < UOrder2; i++, l++)
		    *p++ = *p2++ * CagdIChooseKTable[VDegree2][j]
			  	 * CagdIChooseKTable[UDegree2][i];
	    }

	    /* Do the convolution. */
	    for (j = 0; j < VOrder1; j++) {
	        for (i = 0; i < UOrder1; i++) {
		    CagdRType
			r1 = CpPts1[CAGD_MESH_UV(Srf1, i, j)];

		    p2 = CpPts2;

		    for (m = 0; m < VOrder2; m++) {
		        p = &PPts[CAGD_MESH_UV(ProdSrf, i, j + m)];

			for (l = 0; l++ < UOrder2; )
			    *p++ += r1 * *p2++;
		    }
		}
	    }

	    /* Update the denominator combinatorial coefficient. */
	    for (j = 0; j < VOrder; j++) {
	        p = &PPts[CAGD_MESH_UV(ProdSrf, 0, j)];

	        for (i = 0; i < UOrder; i++)
		    *p++ /= CagdIChooseKTable[UDegree][i] *
			    CagdIChooseKTable[VDegree][j];
	    }
	}
    }
    else {
	/* The original product - easier to follow but not so optimized.     */
	/* 								     */
        /* int il, jm;							     */
	/* 								     */
	/* for (i = 0; i < UOrder1; i++) {				     */
	/*     for (j = 0; j < VOrder1; j++) {				     */
        /*         for (l = 0, il = i; l < UOrder2; l++, il++) {	     */
        /*             for (m = 0, jm = j; m < VOrder2; m++, jm++) {	     */
        /*                 int Index = CAGD_MESH_UV(ProdSrf, il, jm),	     */
        /*                     Index1 = CAGD_MESH_UV(Srf1, i, j),	     */
        /*                     Index2 = CAGD_MESH_UV(Srf2, l, m);	     */
        /*                 CagdRType					     */
        /*                     Coef = CagdIChooseK(i, UDegree1) *	     */
        /*                            CagdIChooseK(l, UDegree2) *	     */
        /*                            CagdIChooseK(j, VDegree1) *	     */
        /*                            CagdIChooseK(m, VDegree2) /	     */
        /*                            (CagdIChooseK(il, UDegree) *	     */
        /*                             CagdIChooseK(jm, VDegree));	     */
        /*								     */
        /*                 for (k = IsNotRational; k <= MaxCoord; k++)	     */
        /*                     PPoints[k][Index] += Coef * 		     */
        /*                         Points1[k][Index1] * Points2[k][Index2];  */
        /*             }						     */
        /*         }							     */
        /*     }							     */
	/* }								     */

        for (k = IsNotRational; k <= MaxCoord; k++) {
	    CagdRType *p, *p1, *p2,
	        *PPts = PPoints[k];

	    /* Place the combinatorial coefficients with the control points. */
	    for (p1 = Points1[k], p = CpPts1, j = l = 0; j < VOrder1; j++) {
	        for (i = 0; i < UOrder1; i++, l++)
		    *p++ = *p1++ * CagdIChooseK(j, VDegree1)
				 * CagdIChooseK(i, UDegree1);
	    }
	    for (p2 = Points2[k], p = CpPts2, j = l = 0; j < VOrder2; j++) {
	        for (i = 0; i < UOrder2; i++, l++)
		    *p++ = *p2++ * CagdIChooseK(j, VDegree2)
				 * CagdIChooseK(i, UDegree2);
	    }

	    /* Do the convolution. */
	    for (j = 0; j < VOrder1; j++) {
	        for (i = 0; i < UOrder1; i++) {
		    CagdRType
			r1 = CpPts1[CAGD_MESH_UV(Srf1, i, j)];

		    p2 = CpPts2;

		    for (m = 0; m < VOrder2; m++) {
		        p = &PPts[CAGD_MESH_UV(ProdSrf, i, j + m)];

		        for (l = 0; l++ < UOrder2; )
			    *p++ += r1 * *p2++;
		    }
		}
	    }

	    /* Update the denominator combinatorial coefficient. */
	    for (j = 0; j < VOrder; j++) {
	        p = &PPts[CAGD_MESH_UV(ProdSrf, 0, j)];

	        for (i = 0; i < UOrder; i++)
		    *p++ /= CagdIChooseK(i, UDegree) *
			    CagdIChooseK(j, VDegree);
	    }
	}
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return ProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Factors out a given bilinear term from a scalar surface, assuming it has M
* this term.								     M
* S(u,v) / {A[0] (1-u)(1-v) + A[1] u(1-v) + A[2] (1-u)v + A[3] uv}	     V
*									     V
* If S(P) = Bilinear(A) * S(Q), then		 			     V
* A[0] (m-i) (n-j) Q[i][j]   + A[1] i (n-j) Q[i-1][j] +			     V
* A[2] (m-i) j     Q[i][j-1] + A[3] i j     Q[i-1][j-1]	= m n P[i][j]	     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To factor out a bilinear term from.                             M
*   A:       Four coefficients of the scalar bilinear. 		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Srf / (u - v).                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfFactorBilinear                                                     M
*****************************************************************************/
CagdSrfStruct *BzrSrfFactorBilinear(const CagdSrfStruct *Srf,
				    const CagdRType *A)
{

    CagdPointType
	PType = Srf -> PType;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i,
        NumCoords = CAGD_NUM_OF_PT_COORD(PType),
	m = Srf -> ULength - 1,
	n = Srf -> VLength - 1;
    CagdSrfStruct
	*FctrSrf = BzrSrfNew(m, n, PType);
    int	Selected;

    /* select the largest absolute value of A[i] */
    if (IRIT_FABS(A[0]) >= IRIT_FABS(A[1])) {
	if (IRIT_FABS(A[0]) >= IRIT_FABS(A[2])) {
	    if (IRIT_FABS(A[0]) >= IRIT_FABS(A[3]))
		Selected = 0;
	    else
		Selected = 3;
	}
	else {
	    if (IRIT_FABS(A[2]) >= IRIT_FABS(A[3]))
		Selected = 2;
	    else
		Selected = 3;
	}
    }
    else {
	if (IRIT_FABS(A[1]) >= IRIT_FABS(A[2])) {
	    if (IRIT_FABS(A[1]) >= IRIT_FABS(A[3]))
		Selected = 1;
	    else
		Selected = 3;
	}
	else {
	    if (IRIT_FABS(A[2]) >= IRIT_FABS(A[3]))
		Selected = 2;
	    else
		Selected = 3;
	}
    }
	
    if (IRIT_FABS(A[Selected]) < IRIT_UEPS) {
	SYMB_FATAL_ERROR(SYMB_ERR_DIV_ZERO);
    }

    for (i = !IsRational; i <= NumCoords; i++) {
        CagdRType
	    *P = Srf -> Points[i],
	    *Q = FctrSrf -> Points[i];

	switch (Selected) {
            case 0:
	        BzrSrfFactorBilinearAux0(P, Q, A, m, n);
		break;
            case 1:
	        BzrSrfFactorBilinearAux1(P, Q, A, m, n);
		break;
            case 2:
	        BzrSrfFactorBilinearAux2(P, Q, A, m, n);
		break;
            case 3:
	        BzrSrfFactorBilinearAux3(P, Q, A, m, n);
		break;
            default:
	        break;
	}
    }

#   ifdef DEBUG
    {
        CagdSrfStruct *Bilin, *MultSrf;

	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestBilinearResult, FALSE) {
	    /* Lets see if product with bilinear recovers the original. */
	    Bilin = BzrSrfNew(2, 2, PType);
	    for (i = !IsRational; i <= NumCoords; i++) {
	        Bilin -> Points[i][0] = A[0];
		Bilin -> Points[i][1] = A[1];
		Bilin -> Points[i][2] = A[2];
		Bilin -> Points[i][3] = A[3];
	    }

	    MultSrf = SymbSrfMult(FctrSrf, Bilin);
	    fprintf(stderr, "Bzr Srf bilinear factor is %s\n",
		    CagdSrfsSame(Srf, MultSrf, IRIT_EPS) ? "Same"
							 : "Different");
	    CagdSrfFree(Bilin);
	    CagdSrfFree(MultSrf);
	}
    }
#   endif /* DEBUG */

    return FctrSrf;
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of BzrSrfFactorBilinear.				     *
*****************************************************************************/
static void BzrSrfFactorBilinearAux0(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n)
{
    int i, j;

    QINDEX_IJ(0,0) = PINDEX_IJ(0,0) / A[0];
    for (i = 1; i < m; i++)
	QINDEX_IJ(i,0) = (m * PINDEX_IJ(i,0) - A[1] * i * QINDEX_IJ(i - 1,0))
							    / (A[0] * (m - i));
    for (j = 1; j < n; j++)
	QINDEX_IJ(0,j) = (n * PINDEX_IJ(0,j) - A[2] * j * QINDEX_IJ(0,j - 1))
							    / (A[0] * (n - j));

    for (i = 1; i < m; i++)
	for (j = 1; j < n; j++)
	    QINDEX_IJ(i,j) = ((m * n) * PINDEX_IJ(i,j)
			- (A[1] * i * (n - j)) * QINDEX_IJ(i - 1, j)
			- (A[2] * (m - i) * j) * QINDEX_IJ(i, j - 1)
			- (A[3] * i * j) * QINDEX_IJ(i - 1, j - 1))
						 / (A[0] * (m - i) * (n - j));
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of BzrSrfFactorBilinear.				     *
*****************************************************************************/
static void BzrSrfFactorBilinearAux3(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n)
{
    int i, j;

    QINDEX_IJ(m-1,n-1) = PINDEX_IJ(m,n) / A[3];
    for (i = m-1; i; i--)
    	QINDEX_IJ(i - 1,n-1) = (m * PINDEX_IJ(i,n) -
			      A[2] * (m - i) * QINDEX_IJ(i,n-1)) / (A[3] * i);
    for (j = n-1; j; j--)
	QINDEX_IJ(m-1,j - 1) = (n * PINDEX_IJ(m,j) -
			      A[1] * (n - j) * QINDEX_IJ(m-1,j)) / (A[3] * j);

    for (i = m-1; i; i--)
	for (j = n-1; j; j--)
	    QINDEX_IJ(i - 1,j - 1) = ((m * n) * PINDEX_IJ(i,j)
			- (A[0] * (m - i) * (n - j)) * QINDEX_IJ(i,j)
			- (A[1] * i * (n - j)) * QINDEX_IJ(i - 1,j)
			- (A[2] * (m - i) * j) * QINDEX_IJ(i,j - 1))
							      / (A[3] * i * j);
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of BzrSrfFactorBilinear.				     *
*****************************************************************************/
static void BzrSrfFactorBilinearAux1(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n)
{
    int i, j;

    QINDEX_IJ(m-1, 0) = PINDEX_IJ(m,0) / A[1];
    for (i = m-1; i; i--)
    	QINDEX_IJ(i - 1,0) = (m * PINDEX_IJ(i,0) -
			    A[0] * (m - i) * QINDEX_IJ(i,0)) / (A[1] * i);
    for (j = 1; j < n; j++)
	QINDEX_IJ(m-1,j) = (n * PINDEX_IJ(m,j) -
			   A[3] * j * QINDEX_IJ(m-1,j - 1)) / (A[1] * (n - j));
    
    for (i = m-1; i; i--)
        for (j = 1; j < n; j++)
	    QINDEX_IJ(i - 1,j) = ((m * n) * PINDEX_IJ(i,j)
			   - (A[0] * (m - i) * (n - j)) * QINDEX_IJ(i, j)
			   - (A[2] * (m - i) * j) * QINDEX_IJ(i, j - 1)
			   - (A[3] * i * j) * QINDEX_IJ(i - 1, j - 1))
							/ (A[1] * i * (n - j));
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function of BzrSrfFactorBilinear.				     *
*****************************************************************************/
static void BzrSrfFactorBilinearAux2(const CagdRType *P,
				     CagdRType *Q,
				     const CagdRType *A,
				     int m,
				     int n)
{
    int i, j;

    QINDEX_IJ(0, n-1) = PINDEX_IJ(0,n) / A[2];
    for (i = 1; i < m; i++)
	QINDEX_IJ(i,n-1) = (m * PINDEX_IJ(i,n) -
			   A[3] * i * QINDEX_IJ(i - 1,n-1)) / (A[2] * (m - i));
    for (j = n - 1; j; j--)
	QINDEX_IJ(0,j - 1) = (n * PINDEX_IJ(0,j) -
			    A[0] * (n - j) * QINDEX_IJ(0,j)) / (A[2] * j);

    for (i = 1; i < m; i++)
        for (j = n-1; j; j--)
	    QINDEX_IJ(i,j - 1) = ((m * n) * PINDEX_IJ(i,j)
			   - (A[0] * (m - i) * (n - j)) * QINDEX_IJ(i, j)
			   - (A[1] * i * (n - j)) * QINDEX_IJ(i - 1, j)
			   - (A[3] * i * j) * QINDEX_IJ(i - 1, j - 1))
							/ (A[2] * (m - i) * j);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Factors out a (u - v) term from a scalar surface, assuming it has one.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To factor out a (u - v) term from.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Srf / (u - v).                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV, BzrSrfFactorBilinear                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfFactorUMinusV                                                      M
*****************************************************************************/
CagdSrfStruct *BzrSrfFactorUMinusV(const CagdSrfStruct *Srf)
{
    int i, j,
	m = Srf -> ULength - 1,
	n = Srf -> VLength - 1;
    CagdSrfStruct
	*FctrSrf = BzrSrfNew(m, n, Srf -> PType);
    CagdRType
        *P = Srf -> Points[1],
        *Q = FctrSrf -> Points[1];

    for (j = 0; j < n; j++)
	Q[j * m] = -n * P[(j + 1) * (m + 1)] / (j + 1);
    
    for (i = 1; i < m; i++)
	for (j = 0; j < n; j++)
	    Q[i + j * m] = ((i * (n - j - 1)) * Q[i - 1 + (j + 1) * m] -
			    (m * n) * P[i + (j + 1) * (m + 1)]) /
				                           ((m - i) * (j + 1));

#   ifdef DEBUG
    {
        CagdSrfStruct *UMinusV, *MultSrf;

	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestUMinVResult, FALSE) {
	    /* Lets see if product with (u - v) recovers the original. */
	    UMinusV = BzrSrfNew(2, 2, CAGD_PT_E1_TYPE);
	    UMinusV -> Points[1][0] =  0.0;
	    UMinusV -> Points[1][1] =  1.0;
	    UMinusV -> Points[1][2] = -1.0;
	    UMinusV -> Points[1][3] =  0.0;

	    MultSrf = SymbSrfMult(FctrSrf, UMinusV);
	    fprintf(stderr, "Bzr Srf UMinusV factor is %s\n",
		    CagdSrfsSame(Srf, MultSrf, IRIT_EPS) ? "Same"
							 : "Different");
	    CagdSrfFree(UMinusV);
	    CagdSrfFree(MultSrf);
	}
    }
#   endif /* DEBUG */

    return FctrSrf;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Derived a reduced Bezier surface out of the given Bezier surface but     M
* performing the following:						     M
* 1. Removing the row/cloumn near the specified surafce boundary, Bndry.     M
* 2. Scaling all other rows/columns by m/i where m is the original order in  M
*    the direction we remove the row/colimn in 1, and i vanish for the	     M
*    row/column index of the removed row/column.	                     M
*   For example, given the Bezier surface:				     M
*                                                                            *
*	 m    n								     V
*       sum  sum Pij Bi,m(u) Bj,n(v),					     V
*       i=0  j=0							     V
*                                                                            *
*   reducing Bndry = CAGD_U_MAX_BNDRY would yeild,		     	     M
*                                                                            *
*	m-1   n	   m							     V
*       sum  sum  --- Pij Bi,m-1(u) Bj,n(v).				     V
*       i=0  j=0  m-i							     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     Surface to compute a reduced form for.                          M
*   Bndry:   Boundary along which to reduce.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Reduced surface, of one degree less in one direction. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarAdjacentSrfSrfInter                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfFactorExtremeRowCol                                                M
*****************************************************************************/
CagdSrfStruct *BzrSrfFactorExtremeRowCol(const CagdSrfStruct *Srf,
					 CagdSrfBndryType Bndry)

{
    CagdPointType
	PType = Srf -> PType;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i, j, k,
        NumCoords = CAGD_NUM_OF_PT_COORD(PType),
        M = Srf -> ULength,
	N = Srf -> VLength,
	N1 = N - 1,
	M1 = M - 1,
	Idx = 0;
    CagdRType **RedPts,
	N1R = N1,
	M1R = M1,
	* const *Pts = Srf -> Points;
    CagdSrfStruct *RedSrf;

    switch (Bndry) {
	case CAGD_U_MIN_BNDRY:
	    Idx = 1;
	case CAGD_U_MAX_BNDRY:
	    /* Copy the control points. */
	    RedSrf = BzrSrfNew(M1, N, PType);
	    RedPts = RedSrf -> Points;
	    for (i = Idx; i < Idx + M1; i++) {
	        IrtRType
		    W = Bndry == CAGD_U_MIN_BNDRY ? M1R / i : M1R / (M1 - i);
	        for (j = 0; j < N; j++) {
		    int SrfIdx = CAGD_MESH_UV(Srf, i, j),
		        RedSrfIdx = CAGD_MESH_UV(RedSrf, i - Idx, j);

		    for (k = !IsRational; k <= NumCoords; k++)
		        RedPts[k][RedSrfIdx] = Pts[k][SrfIdx] * W;
		}
	    }
	    break;
	case CAGD_V_MIN_BNDRY:
	    Idx = 1;
	case CAGD_V_MAX_BNDRY:
	    /* Copy the control points. */
	    RedSrf = BzrSrfNew(M, N1, PType);
	    RedPts = RedSrf -> Points;
	    for (j = Idx; j < Idx + N1; j++) {
	        IrtRType
		    W = Bndry == CAGD_V_MIN_BNDRY ? N1R / j : N1R / (N1 - j);

		for (i = 0; i < M; i++) {
		    int SrfIdx = CAGD_MESH_UV(Srf, i, j),
		        RedSrfIdx = CAGD_MESH_UV(RedSrf, i, j - Idx);

		    for (k = !IsRational; k <= NumCoords; k++)
		        RedPts[k][RedSrfIdx] = Pts[k][SrfIdx] * W;
		}
	    }
	    break;
	default:
	    RedSrf = NULL;
	    assert(0);
    }

    return RedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Factors out a given Bezier surface into four Bezier surfaces of one      M
* order smaller, as (1-u)(1-v) S11 + (1-u)v S12 + u(1-v) S21 + uv S22.       M
*   Srf is assumed to be of orders larger than linear.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To factor out as four surfaces of lower order as,               M
*	     Srf = (1-u)(1-v) S11 + (1-u)v S12 + u(1-v) S21 + uv S22.	     M
*   S11, S12, S21, S22:  The four lower order surfaces to factor out.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfFactorUMinusV, BzrSrfFactorBilinear                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfFactorLowOrders                                                    M
*****************************************************************************/
void BzrSrfFactorLowOrders(const CagdSrfStruct *Srf,
			   CagdSrfStruct **S11,
			   CagdSrfStruct **S12,
			   CagdSrfStruct **S21,
			   CagdSrfStruct **S22)
{
    int i, j, k,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder;
    CagdRType **Pts11, **Pts12, **Pts21, **Pts22,
	* const *Pts = Srf -> Points;
    CagdPointType
	PType = Srf -> PType;

    *S11 = BzrSrfNew(UOrder - 1, VOrder - 1, PType);
    *S12 = BzrSrfNew(UOrder - 1, VOrder - 1, PType);
    *S21 = BzrSrfNew(UOrder - 1, VOrder - 1, PType);
    *S22 = BzrSrfNew(UOrder - 1, VOrder - 1, PType);
    Pts11 = (*S11) -> Points;
    Pts12 = (*S12) -> Points;
    Pts21 = (*S21) -> Points;
    Pts22 = (*S22) -> Points;

    for (i = 0; i < UOrder - 1; i++) {
	for (j = 0; j < VOrder - 1; j++) {
	    for (k = !CAGD_IS_RATIONAL_PT(PType);
		 k <= CAGD_NUM_OF_PT_COORD(PType);
		 k++) {
	        Pts11[k][CAGD_MESH_UV(*S11, i, j)] =
		    Pts[k][CAGD_MESH_UV(Srf, i, j)];
	        Pts12[k][CAGD_MESH_UV(*S12, i, j)] =
		    Pts[k][CAGD_MESH_UV(Srf, i + 1, j)];
	        Pts21[k][CAGD_MESH_UV(*S21, i, j)] =
		    Pts[k][CAGD_MESH_UV(Srf, i, j + 1)];
	        Pts22[k][CAGD_MESH_UV(*S22, i, j)] =
		    Pts[k][CAGD_MESH_UV(Srf, i + 1, j + 1)];
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a rational Bezier curve - computes its derivative curve (Hodograph)  M
* using the quotient rule for differentiation.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Bezier curve to differentiate.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Differentiated rational Bezier curve.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, BspCrvDerive, BspCrvDeriveRational, CagdCrvDerive          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvDeriveRational, derivatives                                        M
*****************************************************************************/
CagdCrvStruct *BzrCrvDeriveRational(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *DCrvW, *DCrvX, *DCrvY, *DCrvZ,
	*TCrv1, *TCrv2, *DeriveCrv;

    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    if (CrvW)
	DCrvW = BzrCrvDerive(CrvW);
    else {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_EXPECTED);
	return NULL;
    }
    
    if (CrvX) {
	DCrvX = BzrCrvDerive(CrvX);

	TCrv1 = BzrCrvMult(DCrvX, CrvW);
	TCrv2 = BzrCrvMult(CrvX, DCrvW);

	CagdCrvFree(CrvX);
	CagdCrvFree(DCrvX);
	CrvX = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }

    if (CrvY) {
	DCrvY = BzrCrvDerive(CrvY);

	TCrv1 = BzrCrvMult(DCrvY, CrvW);
	TCrv2 = BzrCrvMult(CrvY, DCrvW);

	CagdCrvFree(CrvY);
	CagdCrvFree(DCrvY);
	CrvY = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }

    if (CrvZ) {
	DCrvZ = BzrCrvDerive(CrvZ);

	TCrv1 = BzrCrvMult(DCrvZ, CrvW);
	TCrv2 = BzrCrvMult(CrvZ, DCrvW);

	CagdCrvFree(CrvZ);
	CagdCrvFree(DCrvZ);
	CrvZ = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }

    TCrv1 = BzrCrvMult(CrvW, CrvW);
    CagdCrvFree(CrvW);
    CrvW = TCrv1;

    if (!CagdMakeCrvsCompatible(&CrvW, &CrvX, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&CrvW, &CrvZ, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	return NULL;
    }

    DeriveCrv = SymbCrvMergeScalar(CrvW, CrvX, CrvY, CrvZ);

    if (CrvX)
	CagdCrvFree(CrvX);
    if (CrvY)
	CagdCrvFree(CrvY);
    if (CrvZ)
	CagdCrvFree(CrvZ);
    if (CrvW) {
	CagdCrvFree(CrvW);
	CagdCrvFree(DCrvW);
    }

    return DeriveCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a rational Bezier surface - computes its derivative surface in       M
* direction Dir, using the quotient rule for differentiation.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Bezier surface to differentiate.                            M
*   Dir:         Direction of Differentiation. Either U or V.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Differentiated rational Bezier surface.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDerive, BzrSrfDerive, BspSrfDerive, BspSrfDeriveRational          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfDeriveRational, derivatives                                        M
*****************************************************************************/
CagdSrfStruct *BzrSrfDeriveRational(const CagdSrfStruct *Srf,
				    CagdSrfDirType Dir)
{
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *DSrfW, *DSrfX, *DSrfY, *DSrfZ,
	*TSrf1, *TSrf2, *DeriveSrf;

    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
    if (SrfW)
	DSrfW = BzrSrfDerive(SrfW, Dir);
    else {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_EXPECTED);
	return NULL;
    }

    if (SrfX) {
	DSrfX = BzrSrfDerive(SrfX, Dir);

	TSrf1 = BzrSrfMult(DSrfX, SrfW);
	TSrf2 = BzrSrfMult(SrfX, DSrfW);

	CagdSrfFree(SrfX);
	CagdSrfFree(DSrfX);
	SrfX = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    if (SrfY) {
	DSrfY = BzrSrfDerive(SrfY, Dir);

	TSrf1 = BzrSrfMult(DSrfY, SrfW);
	TSrf2 = BzrSrfMult(SrfY, DSrfW);

	CagdSrfFree(SrfY);
	CagdSrfFree(DSrfY);
	SrfY = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    if (SrfZ) {
	DSrfZ = BzrSrfDerive(SrfZ, Dir);

	TSrf1 = BzrSrfMult(DSrfZ, SrfW);
	TSrf2 = BzrSrfMult(SrfZ, DSrfW);

	CagdSrfFree(SrfZ);
	CagdSrfFree(DSrfZ);
	SrfZ = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    
    TSrf1 = BzrSrfMult(SrfW, SrfW);
    CagdSrfFree(SrfW);
    SrfW = TSrf1;

    if (!CagdMakeSrfsCompatible(&SrfW, &SrfX, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&SrfW, &SrfZ, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	return NULL;
    }

    DeriveSrf = SymbSrfMergeScalar(SrfW, SrfX, SrfY, SrfZ);

    if (SrfX)
	CagdSrfFree(SrfX);
    if (SrfY)
	CagdSrfFree(SrfY);
    if (SrfZ)
	CagdSrfFree(SrfZ);
    if (SrfW) {
	CagdSrfFree(SrfW);
	CagdSrfFree(DSrfW);
    }

    return DeriveSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free the subdivision tree.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Tree:   Tree to free.                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BzrApproxLowDegFreeTree(SymbApproxSubdivNodeStruct *Tree)
{
    if (Tree != NULL) {
        BzrApproxLowDegFreeTree(Tree -> Left);
        BzrApproxLowDegFreeTree(Tree -> Right);
        IritFree(Tree);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Control the phases of approximating multiple Bezier curves as lower      M
* order curves, so that all curves are approximated using a similar set      M
* of subdivisions.  A two pass algorithm is devised in which in the first    M
* a subdivision tree is build to hold the union of all subdivisions made     M
* to all the tested curves.  In the second phase, all curves are	     M
* dubdivided the same (following the constructed subdivision tree of phase   M
* one) and returned.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   State: New state to set.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbApproxLowDegStateType:  Old State.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbApproxCrvsAsBzrQuadratics, SymbApproxCrvsAsBzrCubics                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbApproxCrvsLowDegState                                                M
*****************************************************************************/
SymbApproxLowDegStateType SymbApproxCrvsLowDegState(
					      SymbApproxLowDegStateType State)
{
    SymbApproxLowDegStateType
	OldVal = GlblSymbApproxLowDegState; 

    switch (GlblSymbApproxLowDegState = State) {
        case SYMB_APPROX_LOW_DEG_CREATE_TREE:
	    if (GlblSymbApproxSubdivTree != NULL)
	        BzrApproxLowDegFreeTree(GlblSymbApproxSubdivTree);

	    /* Create a new subdivision tree with only the root. */
	    GlblSymbApproxSubdivTree = (SymbApproxSubdivNodeStruct *)
			      IritMalloc(sizeof(SymbApproxSubdivNodeStruct));
	    GlblSymbApproxSubdivTree -> Left = 
	        GlblSymbApproxSubdivTree -> Right = NULL;
	    GlblSymbApproxSubdivTree -> OneQuad = TRUE;
	    GlblSymbApproxSubdivTree -> TMin = 0.0;
	    GlblSymbApproxSubdivTree -> TMax = 1.0;
	    break;
        case SYMB_APPROX_LOW_DEG_TREE_COMPLETE:
	    break;
        case SYMB_APPROX_LOW_DEG_FREE_TREE:
	    BzrApproxLowDegFreeTree(GlblSymbApproxSubdivTree);
	    GlblSymbApproxSubdivTree = NULL;
	    GlblSymbApproxLowDegState = SYMB_APPROX_LOW_DEG_NO_TREE;
	    break;
	default:
	    assert(0);
    }

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a curve - convert it to (possibly) piecewise cubic polynomials.    M
* If the curve is							     M
* 1. A cubic - a copy if it is returned.				     M
* 2. Lower than cubic - a degree raised (to a cubic) curve is returned.      M
* 3. Higher than cubic - a C^1 continuous piecewise cubic polynomials        M
*    approximation is computed for Crv.					     M
* 4. Piecewise polynomial Bspline curve - split into polynomial segments.    M
*									     M
*   In case 3, a list of polynomial cubic curves is returned. Tol is then    M
* used for the distance tolerance error measure for the approximation.	     M
*   If the total length of control polygon is (approximately) more than	     M
* MaxLen, the curve is subdivided until this is not the case.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          To approximate using cubic Bezier polynomials.             M
*   Tol:          Accuracy control.                                          M
*   MaxLen:       Maximum arc length of curve.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of cubic Bezier polynomials approximating Crv.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbApproxCrvAsBzrQuadratics, SymbApproxCrvsLowDegState		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbApproxCrvAsBzrCubics, conversion, approximation                      M
*****************************************************************************/
CagdCrvStruct *SymbApproxCrvAsBzrCubics(const CagdCrvStruct *Crv,
					CagdRType Tol,
					CagdRType MaxLen)
{
    int OldBspInterpMult = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdCrvStruct *TCrv, *CpCrv, *CubicCrvs, *CubicCrvsMaxLen;

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv))
	Crv = CpCrv = BspCrvOpenEnd(Crv);
    else
	CpCrv = NULL;

    switch (Crv -> Order) {
	case 2:
	    CubicCrvs = CagdCrvDegreeRaiseN(Crv, 4);
	    break;
	case 3:
	    CubicCrvs = CagdCrvDegreeRaise(Crv);
	    break;
	case 4:
	    CubicCrvs = CagdCrvCopy(Crv);
	    break;
	default:
	    if (Crv -> Order < 4) {
		BspMultComputationMethod(OldBspInterpMult);
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
		if (CpCrv != NULL)
		    CagdCrvFree(CpCrv);
		return NULL;
	    }

	    CubicCrvs = ApproxCrvsAsCubics(Crv, Tol * Tol,
					   GlblSymbApproxSubdivTree);
	    break;
    }

    CubicCrvsMaxLen = NULL;
    for (TCrv = CubicCrvs; TCrv != NULL; TCrv = TCrv -> Pnext) {
	CagdCrvStruct *BzrCrv;

        if (CAGD_IS_BSPLINE_CRV(CubicCrvs))      /* If degree raised/copied. */
	    BzrCrv = CagdCnvrtBsp2BzrCrv(TCrv);
	else
	    BzrCrv = CagdCrvCopy(TCrv);

	CubicCrvsMaxLen = CagdListAppend(CubicCrvsMaxLen, BzrCrv);
    }
    CagdCrvFreeList(CubicCrvs);
    CubicCrvs = CubicCrvsMaxLen;

    CubicCrvsMaxLen = NULL;
    for (TCrv = CubicCrvs; TCrv != NULL; TCrv = TCrv -> Pnext) {
	CagdCrvStruct
	    *MaxLenCrv = CagdLimitCrvArcLen(TCrv, MaxLen);

	CubicCrvsMaxLen = CagdListAppend(CubicCrvsMaxLen, MaxLenCrv);
    }
    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);
    CagdCrvFreeList(CubicCrvs);
    BspMultComputationMethod(OldBspInterpMult);

    return CubicCrvsMaxLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a set of Bezier curves with order larger than cubic, approximate   *
* them all using piecewise cubic curves, using identical subdivision tree.   *
*   In other words, all curves will have the same topology of subdivision    *
* tree and obviously the same number of cubic curves in the approximation.   *
* This topological similarity is achieved by doing a two phase algorithm in  *
* which in the first the needed subdivisions are detected and saved in a     *
* constructed binary subdivision tree, where in the second phase, the tree   *
* is used to guide the subdivision of all curves.			     *
* See BzrApproxBzrCrvsLowDegState that control the different phases.	     *
*   A C^1 continuous approximation is computed so that the approximation is  *
* at most sqrt(Tol2) away from the given curve.				     *
*   Input curve can be rational, although output is always polynomial.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         To approximate using cubic Bezier curves.                   *
*   Tol2:        Accuracy control (squared).                                 *
*   SubdivTree:  If not NULL, the subdivision tree to update and/or follow   *
*		 based on the status of SymbBzrApproxLowDegStateType.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   List of cubic Bezier curves approximating Crv.        *
*****************************************************************************/
static CagdCrvStruct *ApproxCrvsAsCubics(const CagdCrvStruct *Crv,
					 CagdRType Tol2,
				        SymbApproxSubdivNodeStruct *SubdivTree)
{
    CagdBType
	ApproxOK = TRUE,
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Order = Crv -> Order,
	Length = Crv -> Length,
	MaxCoord = IRIT_MIN(CAGD_NUM_OF_PT_COORD(Crv -> PType), 3);
    CagdPointType
	PType = Crv -> PType,
	CubicPType = CAGD_MAKE_PT_TYPE(FALSE, MaxCoord);
    CagdCrvStruct *DistSqrCrv, *DiffCrv,
	*CubicCrv = BzrCrvNew(4, CubicPType);
    CagdRType E3Pt1[3], E3Pt2[3], E3Pt3[3], E3Pt4[3], Tan1[3], Tan2[3],
	TMin, TMax,
	**CubicPoints = CubicCrv -> Points,
	* const *Points = Crv -> Points;

    CagdCrvDomain(Crv, &TMin, &TMax);

    CagdCoerceToE3(E3Pt1, Points, 0, PType);
    CagdCoerceToE3(E3Pt2, Points, 1, PType);
    CagdCoerceToE3(E3Pt3, Points, Length - 2, PType);
    CagdCoerceToE3(E3Pt4, Points, Length - 1, PType);

    /* End of the two points must match. */
    for (i = 0; i < MaxCoord; i++) {
	CubicPoints[i + 1][0] = E3Pt1[i];
	CubicPoints[i + 1][3] = E3Pt4[i];
    }

    /* Tangents at the end of the two points must match. */
    IRIT_PT_SUB(Tan1, E3Pt2, E3Pt1);
    IRIT_PT_SUB(Tan2, E3Pt4, E3Pt3);
    IRIT_PT_SCALE(Tan1, (Order - 1.0) / 3.0);
    IRIT_PT_SCALE(Tan2, (Order - 1.0) / 3.0);

    for (i = 0; i < MaxCoord; i++) {
	CubicPoints[i + 1][1] = E3Pt1[i] + Tan1[i];
	CubicPoints[i + 1][2] = E3Pt4[i] - Tan2[i];
    }

    if (GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_NO_TREE ||
	GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_CREATE_TREE) {
        /* Compare the two curves by computing the distance square between   */
        /* corresponding parameter values.			 	     */
        if (!IRIT_APX_EQ(TMin, 0.0) || !IRIT_APX_EQ(TMax, 1.0)) {
	    CagdCrvStruct
	        *TCrv = CagdCnvrtBzr2BspCrv(CubicCrv);

	    CagdCrvFree(CubicCrv);
	    CubicCrv = TCrv;
	    BspKnotAffineTransOrder2(CubicCrv -> KnotVector, CubicCrv -> Order,
				     CubicCrv -> Length + CubicCrv -> Order,
				     TMin, TMax);
	}
        DiffCrv = SymbCrvSub(Crv, CubicCrv);
	DistSqrCrv = SymbCrvDotProd(DiffCrv, DiffCrv);
	CagdCrvFree(DiffCrv);
	Points = DistSqrCrv -> Points;
	if (IsNotRational) {
	    CagdRType
	        *Dist = Points[1];

	    for (i = DistSqrCrv -> Order - 1; i >= 0; i--) {
	        if (*Dist++ > Tol2) {
		    ApproxOK = FALSE;
		    break;
		}
	    }		
	}
	else {
	    CagdRType
	        *Dist0 = Points[0],
	        *Dist1 = Points[1];

	    for (i = DistSqrCrv -> Order - 1; i >= 0; i--) {
	        if (*Dist1++ / *Dist0++ > Tol2) {
		    ApproxOK = FALSE;
		    break;
		}
	    }		
	}
	CagdCrvFree(DistSqrCrv);

	if (!ApproxOK &&
	    GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_CREATE_TREE) {
	    /* Create the two subdivision sub nodes. */
	    SubdivTree -> Left = (SymbApproxSubdivNodeStruct *)
			      IritMalloc(sizeof(SymbApproxSubdivNodeStruct));
	    SubdivTree -> Left -> TMin = SubdivTree -> TMin;
	    SubdivTree -> Left -> TMax = (SubdivTree -> TMin +
					      SubdivTree -> TMax) * 0.5;
	    SubdivTree -> Left -> Left = 
	        SubdivTree -> Left -> Right = NULL;
	    SubdivTree -> Left -> OneQuad = TRUE;

	    SubdivTree -> Right = (SymbApproxSubdivNodeStruct *)
			      IritMalloc(sizeof(SymbApproxSubdivNodeStruct));
	    SubdivTree -> Right -> TMin = SubdivTree -> Left -> TMax;
	    SubdivTree -> Right -> TMax = SubdivTree -> TMax;
	    SubdivTree -> Right -> Left = 
	        SubdivTree -> Right -> Right = NULL;
	    SubdivTree -> Right -> OneQuad = TRUE;

	}
    }
    else {
        /* GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_TREE_COMPLETE. */
        ApproxOK = SubdivTree -> Left == NULL && SubdivTree -> Right == NULL;
    }

    if (ApproxOK) {
	return CubicCrv;
    }
    else {
	CagdCrvStruct
	    *Crv1 = CagdCrvSubdivAtParam(Crv, (TMin + TMax) * 0.5),
	    *Crv2 = Crv1 -> Pnext,
	    *Crv1Approx = ApproxCrvsAsCubics(Crv1, Tol2,
			      SubdivTree != NULL ? SubdivTree -> Left : NULL),
	    *Crv2Approx = ApproxCrvsAsCubics(Crv2, Tol2,
			      SubdivTree != NULL ? SubdivTree -> Right : NULL);

	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	CagdCrvFree(CubicCrv);

	return CagdListAppend(Crv1Approx, Crv2Approx);;	
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a curve - convert it to (possibly) piecewise quadratic	     M
* polynomials.								     M
* If the curve is							     M
* 1. A quadratic - a copy if it is returned.				     M
* 2. Lower than quadratic - a degree raised (to a quadratic) curve is	     M
*			    returned.					     M
* 3. Higher than quadratic - a C^1 continuous piecewise quadratic            M
*    polynomial approximation is computed for Crv.			     M
* 4. Piecewise polynomial Bspline curve - split into polynomial segments.    M
*									     M
*   In case 3, a list of polynomial quadratic curves is returned. Tol is     M
* then used for the distance tolerance error measure for the approximation.  M
*   If the total length of control polygon is (approximately) more than      M
* MaxLen, the curve is subdivided until this is not the case.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         To approximate using quadratic Bezier polynomials.         M
*   Tol:          Accuracy control.                                          M
*   MaxLen:       Maximum arc length of curve.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of quadratic Bezier polynomials approximating   M
*		  Crv.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbApproxCrvAsBzrCubics, SymbApproxCrvsLowDegState			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbApproxCrvAsBzrQuadratics, conversion, approximation                  M
*****************************************************************************/
CagdCrvStruct *SymbApproxCrvAsBzrQuadratics(const CagdCrvStruct *CCrv,
					    CagdRType Tol,
					    CagdRType MaxLen)
{
    int OldBspInterpMult = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdCrvStruct *TCrv, *Crv, *QuadraticCrvs, *QuadraticCrvsMaxLen;

    if (CAGD_IS_BSPLINE_CRV(CCrv) && !BspCrvHasOpenEC(CCrv))
	Crv = BspCrvOpenEnd(CCrv);
    else
	Crv = CagdCrvCopy(CCrv);

    switch (Crv -> Order) {
	case 2:
	    QuadraticCrvs = CagdCrvDegreeRaiseN(Crv, 3);
	    break;
	case 3:
	    QuadraticCrvs = CagdCrvCopy(Crv);
	    break;
	default:
	    if (Crv -> Order < 3) {
		BspMultComputationMethod(OldBspInterpMult);
		SYMB_FATAL_ERROR(SYMB_ERR_OUT_OF_RANGE);
		CagdCrvFree(Crv);
		return NULL;
	    }

	    QuadraticCrvs = ApproxCrvsAsQuadratics(Crv, Tol * Tol,
						   GlblSymbApproxSubdivTree);
	    break;
    }

    QuadraticCrvsMaxLen = NULL;
    for (TCrv = QuadraticCrvs; TCrv != NULL; TCrv = TCrv -> Pnext) {
	CagdCrvStruct *BzrCrv;

        if (CAGD_IS_BSPLINE_CRV(QuadraticCrvs))  /* If degree raised/copied. */
	    BzrCrv = CagdCnvrtBsp2BzrCrv(TCrv);
	else
	    BzrCrv = CagdCrvCopy(TCrv);

	QuadraticCrvsMaxLen = CagdListAppend(QuadraticCrvsMaxLen, BzrCrv);
    }
    CagdCrvFreeList(QuadraticCrvs);
    QuadraticCrvs = QuadraticCrvsMaxLen;

    QuadraticCrvsMaxLen = NULL;
    for (TCrv = QuadraticCrvs; TCrv != NULL; TCrv = TCrv -> Pnext) {
	CagdCrvStruct
	    *MaxLenCrv = CagdLimitCrvArcLen(TCrv, MaxLen);

	QuadraticCrvsMaxLen = CagdListAppend(QuadraticCrvsMaxLen, MaxLenCrv);
    }

    CagdCrvFree(Crv);

    CagdCrvFreeList(QuadraticCrvs);
    BspMultComputationMethod(OldBspInterpMult);

    return QuadraticCrvsMaxLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a set of Bezier curves with order larger than quadratic,           *
* approximate them all using piecewise quadratic curves, using an identical  *
* subdivision tree.							     *
*   In other words, all curves will have the same topology of subdivision    *
* tree and obviously the same number of quadratic curves in the		     *
* approximation. This topological similarity is achieved by doing a two	     *
* phase algorithm in which in the first the needed subdivisions are detected *
* and saved in a constructed binary subdivision tree, where in the second    *
* phase, the tree is used to guide the subdivision of all curves.	     *
* See BzrApproxBzrCrvsLowDegState that control the different phases.	     *
*   A C^1 continuous approximation is computed so that the approximation is  *
* at most sqrt(Tol2) away from the given curve, using pairs of quadratics.   *
*   Input curve can be rational, although output is always polynomial.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:         To approximate using quadratuc Bezier curves.               *
*   Tol2:        Accuracy control (squared).                                 *
*   SubdivTree:  If not NULL, the subdivision tree to update and/or follow   *
*		 based on the status of SymbBzrApproxLowDegStateType.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   List of quadratic Bezier curves approximating Crv.    *
*****************************************************************************/
static CagdCrvStruct *ApproxCrvsAsQuadratics(
				       const CagdCrvStruct *Crv,
				       CagdRType Tol2,
				       SymbApproxSubdivNodeStruct *SubdivTree)
{
    CagdBType SingleQuadratic,
	ApproxOK = TRUE,
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Order = Crv -> Order,
	Length = Crv -> Length,
	MaxCoord = IRIT_MIN(CAGD_NUM_OF_PT_COORD(Crv -> PType), 3);
    CagdPointType
	PType = Crv -> PType,
	QuadraticPType = CAGD_MAKE_PT_TYPE(FALSE, MaxCoord);
    CagdCrvStruct *DistSqrCrv, *DiffCrv, *QuadBsp,
	*QuadraticBzr1 = BzrCrvNew(3, QuadraticPType),
	*QuadraticBzr2 = BzrCrvNew(3, QuadraticPType);
    CagdVType E3Pt1, E3Pt2, E3Pt3, E3Pt4, Tan1, Tan2, Chord, V1, V2;
    CagdRType t1, t2,
	Tol = sqrt(Tol2),
	TMin, TMax,
	**QuadraticPoints1 = QuadraticBzr1 -> Points,
	**QuadraticPoints2 = QuadraticBzr2 -> Points,
	* const *Points = Crv -> Points;

    CagdCrvDomain(Crv, &TMin, &TMax);

    CagdCoerceToE3(E3Pt1, Points, 0, PType);
    CagdCoerceToE3(E3Pt2, Points, 1, PType);
    CagdCoerceToE3(E3Pt3, Points, Length - 2, PType);
    CagdCoerceToE3(E3Pt4, Points, Length - 1, PType);

    /* Tangents at the end of the two points must match. */
    IRIT_PT_SUB(Tan1, E3Pt2, E3Pt1);
    IRIT_PT_SUB(Tan2, E3Pt4, E3Pt3);
    IRIT_PT_SUB(Chord, E3Pt4, E3Pt1);

    /* If these four sampled locations are coplanar and Tan1 and Tan2 are on */
    /* the opposite sides of the Chord line, we have no inflections and we   */
    /* can try a single quadratic.					     */
    IRIT_CROSS_PROD(V1, Tan1, Chord);
    IRIT_CROSS_PROD(V2, Tan2, Chord);
    SingleQuadratic = FALSE;
    if (GMCoplanar4Pts(E3Pt1, E3Pt2, E3Pt3, E3Pt4) &&
	IRIT_DOT_PROD(V1, V2) < 0 &&
	GM2PointsFromLineLine(E3Pt1, Tan1, E3Pt4, Tan2, V1, &t1, V2, &t2)) {
        assert(IRIT_PT_APX_EQ(V1, V2));
	SingleQuadratic = TRUE;

	for (i = 0; i < MaxCoord; i++) {
	    /* End of the two points must match. Place mid point at the      */
	    /* intersections of the two tangents, Tan1 and Tan2.	     */
	    QuadraticPoints1[i + 1][0] = E3Pt1[i];
	    QuadraticPoints1[i + 1][1] = V1[i];
	    QuadraticPoints1[i + 1][2] = E3Pt4[i];
	}
    }
    else {
        /* Measure the distance of all the interior control points of Crv to */
        /* Chord and if less than defined by Tol2, still use a single quad.  */
        /*   Note we assume here, Crv is monotone with respect to Chord.     */
        for (i = 1; i < Length - 1; i++) {
	    CagdCoerceToE3(V1, Points, i, PType);
	    if (GMDistPointLine(V1, E3Pt1, Chord) > Tol)
	        break;
	}
	if (i >= Length - 1) {
	    SingleQuadratic = TRUE;

	    for (i = 0; i < MaxCoord; i++) {
	        /* End of the two points must match. Place mid in middle. */
	        QuadraticPoints1[i + 1][0] = E3Pt1[i];
	        QuadraticPoints1[i + 1][1] = (E3Pt1[i] + E3Pt4[i]) * 0.5;
	        QuadraticPoints1[i + 1][2] = E3Pt4[i];
	    }
	}
    }

    if (SingleQuadratic) {
        QuadBsp = CagdCnvrtBzr2BspCrv(QuadraticBzr1);

	CagdCrvFree(QuadraticBzr2);
	QuadraticBzr2 = NULL;
    }
    else {		              /* Try to fit two quadratics together. */
        /* Divide by 2 (quadratic degree) and by 2 since we have two pieces. */
        IRIT_PT_SCALE(Tan1, (Order - 1.0) / 4.0);
	IRIT_PT_SCALE(Tan2, (Order - 1.0) / 4.0);

	for (i = 0; i < MaxCoord; i++) {
	    /* End of the two points must match. */
	    QuadraticPoints1[i + 1][0] = E3Pt1[i];
	    QuadraticPoints2[i + 1][2] = E3Pt4[i];

	    /* Middle points are fully governed from the C^1 continuity. */
	    QuadraticPoints1[i + 1][1] = E3Pt1[i] + Tan1[i];
	    QuadraticPoints2[i + 1][1] = E3Pt4[i] - Tan2[i];

	    /* End point of first cyrve == start point of second curve -    */
	    /* middle point due to the C^1 continuity. constraint.	    */
	    QuadraticPoints1[i + 1][2] =
	        QuadraticPoints2[i + 1][0] =
	            (QuadraticPoints1[i + 1][1] +
		     QuadraticPoints2[i + 1][1]) * 0.5;
	}

        QuadBsp = CagdMergeCrvCrv(QuadraticBzr1, QuadraticBzr2, FALSE);
    }

    if (GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_NO_TREE ||
	GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_CREATE_TREE) {
        /* Compare the two curves by computing the distance square between   */
        /* corresponding parameter values.				     */
	BspKnotAffineTransOrder2(QuadBsp -> KnotVector, QuadBsp -> Order,
				 QuadBsp -> Length + QuadBsp -> Order,
				 TMin, TMax);
	DiffCrv = SymbCrvSub(Crv, QuadBsp);
	DistSqrCrv = SymbCrvDotProd(DiffCrv, DiffCrv);
	CagdCrvFree(DiffCrv);
	Points = DistSqrCrv -> Points;
	if (IsNotRational) {
	    CagdRType
	        *Dist = Points[1];

	    for (i = DistSqrCrv -> Order - 1; i >= 0; i--) {
	        if (*Dist++ > Tol2) {
		    ApproxOK = FALSE;
		    break;
		}
	    }
	}
	else {
	    CagdRType
	        *Dist0 = Points[0],
	        *Dist1 = Points[1];

	    for (i = DistSqrCrv -> Order - 1; i >= 0; i--) {
	        if (*Dist1++ / *Dist0++ > Tol2) {
		    ApproxOK = FALSE;
		    break;
		}
	    }		
	}

	CagdCrvFree(DistSqrCrv);
	CagdCrvFree(QuadBsp);

	if (GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_CREATE_TREE) {
	    /* It is enough if one instance demanded a pair of quadratics. */
	    if (SubdivTree -> OneQuad && !SingleQuadratic)
	        SubdivTree -> OneQuad = FALSE;

	    if (!ApproxOK) {
	        /* Create the two subdivision sub nodes, if not exist. */
	        if (SubdivTree -> Left == NULL) {
		    SubdivTree -> Left = (SymbApproxSubdivNodeStruct *)
			      IritMalloc(sizeof(SymbApproxSubdivNodeStruct));
		    SubdivTree -> Left -> TMin = SubdivTree -> TMin;
		    SubdivTree -> Left -> TMax = (SubdivTree -> TMin +
					      SubdivTree -> TMax) * 0.5;
		    SubdivTree -> Left -> Left = 
		        SubdivTree -> Left -> Right = NULL;
		    SubdivTree -> Left -> OneQuad = TRUE;
		}

		if (SubdivTree -> Right == NULL) {
		    SubdivTree -> Right = (SymbApproxSubdivNodeStruct *)
			      IritMalloc(sizeof(SymbApproxSubdivNodeStruct));
		    SubdivTree -> Right -> TMin = SubdivTree -> Left -> TMax;
		    SubdivTree -> Right -> TMax = SubdivTree -> TMax;
		    SubdivTree -> Right -> Left = 
		        SubdivTree -> Right -> Right = NULL;
		    SubdivTree -> Right -> OneQuad = TRUE;
		}
	    }
	}
    }
    else {
        /* GlblSymbApproxLowDegState == SYMB_APPROX_LOW_DEG_TREE_COMPLETE. */
        ApproxOK = SubdivTree -> Left == NULL && SubdivTree -> Right == NULL;
    }

    if (ApproxOK) {
	QuadraticBzr1 -> Pnext = QuadraticBzr2;
	return QuadraticBzr1;
    }
    else {
	CagdCrvStruct
	    *Crv1 = CagdCrvSubdivAtParam(Crv, (TMin + TMax) * 0.5),
	    *Crv2 = Crv1 -> Pnext,
	    *Crv1Approx = ApproxCrvsAsQuadratics(Crv1, Tol2,
			      SubdivTree != NULL ? SubdivTree -> Left : NULL),
	    *Crv2Approx = ApproxCrvsAsQuadratics(Crv2, Tol2,
			      SubdivTree != NULL ? SubdivTree -> Right : NULL);

	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	CagdCrvFree(QuadraticBzr1);
	if (QuadraticBzr2 != NULL)
	    CagdCrvFree(QuadraticBzr2);

	return CagdListAppend(Crv1Approx, Crv2Approx);
    }
}
