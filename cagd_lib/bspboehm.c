/******************************************************************************
* BspBoehm.c - Implements Boehm single knot insertion routine.		      *
* Based on:								      *
* "Recursive proof of Boehm's knot insertion technique", by Phillip J Barry   *
* Ronald N Goldman, CAD, Volume 20 number 4 1988, pp 181-182.		      *
* The original paper by Boehm W. is also referenced there.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve refined at t (t is inserted as a new knot in Crv).     M
*   If however the multiplicity of t in the current knot vector is equal     M
* (or greater!?) to the degree or t is not in the curve's parametric domain, M
* no new knot is insert and NULL is returned instead.			     M
*   Control mesh is updated as follows (P is old ctl polygon, Q is new):     M
*   Let Index be the last knot in old knot vector less than t and	     M
* let j be j = Index - order + 1. Also let k be the curve order. Then,	     M
*									     M
*  Case 1: Q(i) = P(i), i <= j						     V
*									     V
*		  t     -    t(i)        t(i+k-1)  -   t		     V
*  case 2: Q(i) = --------------- P(i) + --------------- P(i-1), j<i<=Index  V
*		  t(i+k-1) - t(i)        t(i+k-1) - t(i)		     V
*									     V
*  case 3: Q(i) = P(i-1), Index < i					     V
*									     M
*  Note: Altough works, this is not the optimal way to insert many knot!     M
*  See also the BspKnotEvalAlpha set of routines.			     M
*									     M
* For more see:                                                              M
* "Recursive proof of Boehm's knot insertion technique", by Phillip J Barry  M
* Ronald N Goldman, CAD, Volume 20 number 4 1988, pp 181-182.		     M
* Which also references the original 1980 paper by Boehm.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To refine by adding a new knot with value equal to t. If Crv   M
*             is a periodic curve, it is first unwrapped to a float end      M
*             condition curve.                                               M
*   t:        New knot to insert into Crv.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The refined curve.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvKnotInsertNSame, BspCrvKnotInsertNDiff, BspSrfKnotInsert           M
*   BspKnotEvalAlphaCoef					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvKnotInsert, refinement, knot insertion                             M
*****************************************************************************/
CagdCrvStruct *BspCrvKnotInsert(const CagdCrvStruct *Crv, CagdRType t)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    CagdRType *KnotVector;
    int i, j, Len, KVLen, Index,
	k = Crv -> Order,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *RefinedCrv;
    CagdRType **NewPoints;
    CagdRType * const *Points;
    CagdCrvStruct *CpCrv;
    
    if (CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    KnotVector = Crv -> KnotVector;
    Len = Crv -> Length;
    KVLen = k + Len;
    Index = BspKnotLastIndexL(KnotVector, KVLen, t);
    RefinedCrv = CagdCrvNew(Crv -> GType, Crv -> PType, Len + 1);
    Points = Crv -> Points;
    NewPoints = RefinedCrv -> Points;

    if (!BspKnotParamInDomain(KnotVector, Len, k, FALSE, t))
	CAGD_FATAL_ERROR(CAGD_ERR_T_NOT_IN_CRV);

    /* Update the rest of the RefinedCrv data structure (easy part). */
    RefinedCrv -> Order = k;
    RefinedCrv -> KnotVector = BspKnotInsertOne(Crv -> KnotVector, k, Len, t);

    /* Case 1: Copy all points upto (Index - Order + 1) as is. */
    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(NewPoints[i], Points[i],
		      sizeof(CagdRType) * (Index - k + 2));

    /* Case 2: Convex blend of exactly 2 points. */
    for (j = Index - k + 2; j <= Index; j++)
	for (i = IsNotRational; i <= MaxCoord; i++)
	    NewPoints[i][j] =
	       ((t - KnotVector[j]) * Points[i][j] +
		(KnotVector[j + k - 1] - t) * Points[i][j - 1]) /
				      (KnotVector[j + k - 1] - KnotVector[j]);

    /* Case 3: Copy all points upto the end. */
    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(&NewPoints[i][Index + 1],
		      &Points[i][Index],
		      sizeof(CagdRType) * (Len - Index));

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return RefinedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface refined at t (t is inserted as a new knot in Srf)    M
* in parametric directipn Dir. See BspCrvKnotInsert for the mathematical     M
* background of this knot insertion algorithm.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To refine by adding a new knot with value equal to t. If Srf   M
*             is a periodic curve, it is first unwrapped to a float end      M
*             condition curve.                                               M
*   Dir:      Of refinement, either U or V.                                  M
*   t:        New knot to insert into Srf.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The refined surface.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfKnotInsertNSame, BspSrfKnotInsertNDiff, BspCrvKnotInsert           M
*   BspKnotEvalAlphaCoef					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfKnotInsert, refinement, knot insertion                             M
*****************************************************************************/
CagdSrfStruct *BspSrfKnotInsert(const CagdSrfStruct *Srf,
				CagdSrfDirType Dir,
				CagdRType t)
{
    int Row, Col, ULength, VLength, UOrder, VOrder;
    CagdCrvStruct *Crv, *RefCrv;
    CagdSrfStruct *CpSrf,
	*RefSrf = NULL;

    if (CAGD_IS_PERIODIC_SRF(Srf))
	Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;
    
    ULength = Srf -> ULength;
    VLength = Srf -> VLength;
    UOrder = Srf -> UOrder;
    VOrder = Srf -> VOrder;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    RefSrf = BspSrfNew(ULength + 1, VLength, UOrder, VOrder,
							      Srf -> PType);
	    BspKnotCopy(RefSrf -> VKnotVector, Srf -> VKnotVector,
			Srf -> VLength + Srf -> VOrder);

	    for (Row = 0; Row < VLength; Row++) {
		Crv = BspSrfCrvFromMesh(Srf, Row, CAGD_CONST_V_DIR);
		RefCrv = BspCrvKnotInsert(Crv, t);

		if (Row == 0) {		  /* Figure out refined knot vector. */
		    BspKnotCopy(RefSrf -> UKnotVector, RefCrv -> KnotVector,
				RefCrv -> Length + RefCrv -> Order);
		}

		CagdCrvToMesh(RefCrv, Row, CAGD_CONST_V_DIR, RefSrf);

		CagdCrvFree(Crv);
		CagdCrvFree(RefCrv);
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    RefSrf = BspSrfNew(ULength, VLength + 1, UOrder, VOrder,
							     Srf -> PType);
	    BspKnotCopy(RefSrf -> UKnotVector, Srf -> UKnotVector,
			Srf -> ULength + Srf -> UOrder);

	    for (Col = 0; Col < ULength; Col++) {
		Crv = BspSrfCrvFromMesh(Srf, Col, CAGD_CONST_U_DIR);
		RefCrv = BspCrvKnotInsert(Crv, t);

		if (Col == 0) {		  /* Figure out refined knot vector. */
		    BspKnotCopy(RefSrf -> VKnotVector, RefCrv -> KnotVector,
				RefCrv -> Length + RefCrv -> Order);
		}

		CagdCrvToMesh(RefCrv, Col, CAGD_CONST_U_DIR, RefSrf);

		CagdCrvFree(Crv);
		CagdCrvFree(RefCrv);
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return RefSrf;
}
