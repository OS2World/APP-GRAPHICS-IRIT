/******************************************************************************
* Bzr_Pwr.c - Bezier to power basis conversion.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jun. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

#define CAGD_BINOM_COEF(n, i)		CagdIChooseK(i, n)

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts the given curve from Bezier basis functions to a Power basis      M
* functions. Using:							     M
*									     M
*	  n								     V
*	  __								     V
*  n	  \	j-i n	j   j						     V
* B (t) = /  (-1)  ( ) ( ) t						     V
*  i	  --	    j   i						     V
*	 j=i								     V
*					     n-i			     V
* Which can be derived by expanding the (1-t)    term in bezier basis	     V
* function definition as:						     V
*	  								     V
* 	   n-i  							     V
*           __  							     V
*      n-i  \  n-i      j						     V
* (1-t)   = / (   ) (-t)	using binomial expansion.		     V
*	    --  j							     V
*	   j=0								     V
*                                               			     M
* This routine simply take the weight of each Bezier basis function B(t) and M
* spread it into the different power basis t^j function scaled by:	     M
*                                               			     M
*	j-i n	j							     V
*   (-1)   ( ) ( )                                             		     V
*           j   i                                 			     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To convert into Power basis function representation.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Same geometry, but in the Power basis.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtBzr2BspCrv, CagdCnvrtBsp2BzrCrv, CagdCnvrtPwr2BzrCrv            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBzr2PwrCrv, power basis, conversion                             M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtBzr2PwrCrv(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, l,
	n = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType *PwrP, *BzrP;
    CagdCrvStruct *NewCrv;

    if (!CAGD_IS_BEZIER_CRV(Crv))
	return NULL;

    NewCrv = CagdCrvNew(CAGD_CPOWER_TYPE, Crv -> PType, n);
    NewCrv -> Order = n;

    for (l = IsNotRational; l <= MaxCoord; l++) {
	PwrP = NewCrv -> Points[l];
	BzrP = Crv -> Points[l];
	IRIT_ZAP_MEM(PwrP, sizeof(CagdRType) * n);

	for (i = 0; i < n; i++) {
	    for (j = i; j < n; j++) {
		PwrP[j] += BzrP[i] * CAGD_BINOM_COEF(n - 1, j) *
				     CAGD_BINOM_COEF(j, i) *
						(((j - i) & 0x01) ? -1 : 1);
	    }
	}
    }

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts the given curve from Power basis functions to Bezier basis        M
* functions. Using:							     M
*									     M
*      n    j								     V
*      __  ( )								     V
*   i  \    i	 n							     V
*  t = /  ----- B (t)							     V
*      --   n	 j							     V
*      j=i ( )								     V
*	    i								     V
*                                               			     M
* This routine simply take the weight of each Power basis function t^i and   M
* spread it into the different basis basis function B(t) scaled by:	     M
*                                               			     M
*     j   / n								     V
*    ( ) / ( )                                             		     V
*     i /   i	                                 			     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To convert to Bezier basis functions.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Same geometry, in the Bezier basis functions.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtBzr2BspCrv, CagdCnvrtBsp2BzrCrv, CagdCnvrtBzr2PwrCrv            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPwr2BzrCrv, power basis, conversion                             M
*****************************************************************************/
CagdCrvStruct *CagdCnvrtPwr2BzrCrv(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j, l,
	n = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType *PwrP, *BzrP;
    CagdCrvStruct *NewCrv;

    if (!CAGD_IS_POWER_CRV(Crv))
	return NULL;

    NewCrv = BzrCrvNew(n, Crv -> PType);

    for (l = IsNotRational; l <= MaxCoord; l++) {
	PwrP = Crv -> Points[l];
	BzrP = NewCrv -> Points[l];
	IRIT_ZAP_MEM(BzrP, sizeof(CagdRType) * n);

	for (i = 0; i < n; i++) {
	    for (j = i; j < n; j++) {
		BzrP[j] += PwrP[i] * CAGD_BINOM_COEF(j, i) /
				     CAGD_BINOM_COEF(n - 1, i);
	    }
	}
    }

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts the given surface from Bezier basis functions to a Power basis    M
* functions. Using:							     M
*									     M
*         n                                                                  V
*         __                                                                 V
*  n      \      p-i n   p   p                                               V
* B (t) = /  (-1)   ( ) ( ) t                                                V
*  i      --         p   i                                                   V
*         p=i                                                                V
*                                                                            V
* or                                                                         M
*                n   m                                                       V
*                __  __                                                      V
*  n     m       \   \     p-i n   p     q-j  m   q   p  q                   V
* B (u) B (v) =  /   /  (-1)  ( ) ( ) (-1)   ( ) ( ) u  v                    V
*  i     j       --  --        p   i          q   j                          V
*               p=i q=j                                                      V
*                                                                            V
*                                               			     M
* This routine simply take the weight of each product of two Bezier basis    M
* functions Bi(u) Bj(v) and spread it into the different power basis u^j v^k M
* functions scaled by:							     M
*                                               			     M
*      p-i n   p     q-j  m   q                                              V
*   (-1)  ( ) ( ) (-1)   ( ) ( )                                             V
*          p   i          q   j                                              V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To convert into Power basis function representation.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Same geometry, but in the Power basis.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBzr2PwrSrf, power basis, conversion                             M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtBzr2PwrSrf(const CagdSrfStruct *Srf)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int p, q, i, j, l, Pq, Ij,
	n = Srf -> ULength,
	m = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *PwrP, *BzrP;
    CagdSrfStruct *NewSrf;

    if (!CAGD_IS_BEZIER_SRF(Srf))
	return NULL;

    NewSrf = CagdSrfNew(CAGD_SPOWER_TYPE, Srf -> PType, n, m);
    NewSrf -> UOrder = NewSrf -> ULength;
    NewSrf -> VOrder = NewSrf -> VLength;

    for (l = IsNotRational; l <= MaxCoord; l++) {
	BzrP = Srf -> Points[l];
	PwrP = NewSrf -> Points[l];
	IRIT_ZAP_MEM(PwrP, sizeof(CagdRType) * n * m);

	for (j = 0, Ij = 0; j < m; j++)
	    for (i = 0; i < n; i++, Ij++)
		for (q = j; q < m; q++)
		    for (p = i, Pq = p + n * q; p < n; p++, Pq++)
			PwrP[Pq] += BzrP[Ij]
			            * CAGD_BINOM_COEF(n - 1, p) *
				      CAGD_BINOM_COEF(p, i) *
						(((p - i) & 0x01) ? -1 : 1)
			            * CAGD_BINOM_COEF(m - 1, q) *
				      CAGD_BINOM_COEF(q, j) *
						(((q - j) & 0x01) ? -1 : 1);
    }

    CAGD_PROPAGATE_ATTR(NewSrf, Srf);

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts the given surface from Power basis functions to Bezier basis      M
* functions. Using:							     M
*                                                                            M
*               u -->                                                        V
*     +----------------------+                                               V
*     |P0                Pi-1|                                               V
*   v |Pi               P2i-1|  Parametric space orientation - control mesh. V
*   | |                      |                                               V
*   v |Pn-i              Pn-1|                                               V
*     +----------------------+                                               V
*                                                                            V
*                                                                            V
*         n   m    i    j                                                    V
*         __  __  ( )  ( )                                                   V
*   p q   \   \    p    q    n     m                                         V
*  u v =  /   /  ---------- B (u) B (v)                                      V
*         --  --   n    m    i     j                                         V
*         i=p j=q ( )  ( )                                                   V
*                  p    q                                                    V
*                                                                            V
*                                                                            V
*         i   j    i    j                                                    V
*         __  __  ( )  ( )                                                   V
*         \   \    p    q                                                    V
*  C    = /   /  ---------- A                                                V
*   ij    --  --   n    m    pq                                              V
*         p=0 q=0 ( )  ( )                                                   V
*                  p    q                                                    V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To convert into Bezier basis function representation.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Same geometry, but in the Bezier basis.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtPwr2BzrSrf, power basis, conversion                             M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtPwr2BzrSrf(const CagdSrfStruct *Srf)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int p, q, i, j, l, Pq, Ij,
	n = Srf -> ULength,
	m = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *PwrP, *BzrP;
    CagdSrfStruct *NewSrf;

    if (!CAGD_IS_POWER_SRF(Srf))
	return NULL;

    NewSrf = BzrSrfNew(n, m, Srf -> PType);

    for (l = IsNotRational; l <= MaxCoord; l++) {
	PwrP = Srf -> Points[l];
	BzrP = NewSrf -> Points[l];
	IRIT_ZAP_MEM(BzrP, sizeof(CagdRType) * n * m);

	for (q = 0, Pq = 0; q < m; q++)
	    for (p = 0; p < n; p++, Pq++)
		for (j = q; j < m; j++)
		    for (i = p, Ij = i + n * j; i < n; i++, Ij++)
			BzrP[Ij] += PwrP[Pq]
			    * (CAGD_BINOM_COEF(i, p) /
			       CAGD_BINOM_COEF(n - 1, p))
			    * (CAGD_BINOM_COEF(j, q) /
			       CAGD_BINOM_COEF(m - 1, q));
    }

    CAGD_PROPAGATE_ATTR(NewSrf, Srf);

    return NewSrf;
}
