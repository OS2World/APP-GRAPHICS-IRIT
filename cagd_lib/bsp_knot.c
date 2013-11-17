/******************************************************************************
* Bsp_knot.c - Various bspline routines to handle knot vectors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

#define KNOT_IS_THE_SAME 1e-10

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given curve has no interior knot open end KV.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To check for KV that mimics Bezier polynomial curve.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if same as Bezier curve, FALSE otherwise.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvHasBezierKV, conversion                                            M
*****************************************************************************/
CagdBType BspCrvHasBezierKV(const CagdCrvStruct *Crv)
{
    return BspKnotHasBezierKV(Crv -> KnotVector, Crv -> Length, Crv -> Order);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given surface has no interior knot open end KVs.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To check for KVs that mimics Bezier polynomial surface.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if same as Bezier surface, FALSE otherwise.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfHasBezierKVs, conversion                                           M
*****************************************************************************/
CagdBType BspSrfHasBezierKVs(const CagdSrfStruct *Srf)
{
    return
	BspKnotHasBezierKV(Srf -> UKnotVector, Srf -> ULength, Srf -> UOrder) &&
	BspKnotHasBezierKV(Srf -> VKnotVector, Srf -> VLength, Srf -> VOrder);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given knot vector of length (Len + Order) has no      M
* interior knots and it has an open end conditions.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:   To check for open end and no interior knots conditions.    M
*   Len:          Of control mesh of this knot vector.                       M
*   Order:        Of curve/surface the exploits this knot vector.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if has open end conditions and no interior knots,     M
*                 FALSE otherwise.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotHasBezierKV, knot vectors, conversion                             M
*****************************************************************************/
CagdBType BspKnotHasBezierKV(const CagdRType *KnotVector, int Len, int Order)
{
    return Len == Order && BspKnotHasOpenEC(KnotVector, Len, Order);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline curve has open end coditions.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To check for open end conditions.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if curve has open end conditions, FALSE otherwise.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvHasOpenEC, open end conditions                                     M
*****************************************************************************/
CagdBType BspCrvHasOpenEC(const CagdCrvStruct *Crv)
{
    return BspKnotHasOpenEC(Crv -> KnotVector, Crv -> Length, Crv -> Order);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline surface has open end coditions.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To check for open end conditions.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if surface has open end conditions, FALSE otherwise.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfHasOpenEC, open end conditions                                     M
*****************************************************************************/
CagdBType BspSrfHasOpenEC(const CagdSrfStruct *Srf)
{
    return
	BspKnotHasOpenEC(Srf -> UKnotVector, Srf -> ULength, Srf -> UOrder) &&
	BspKnotHasOpenEC(Srf -> VKnotVector, Srf -> VLength, Srf -> VOrder);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline surface has open end coditions in the   M
* specified direction.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To check for open end conditions.                              M
*   Dir:      Either the U or the V parametric direction.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if surface has open end conditions, FALSE otherwise.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfHasOpenECDir, open end conditions                                  M
*****************************************************************************/
CagdBType BspSrfHasOpenECDir(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    return BspKnotHasOpenEC(Srf -> UKnotVector, Srf -> ULength,
				    Srf -> UOrder);
	case CAGD_CONST_V_DIR:
	    return BspKnotHasOpenEC(Srf -> VKnotVector, Srf -> VLength,
				    Srf -> VOrder);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given knot vector of length (Len + Order) has open    M
* end conditions.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:   To check for open end condition.                           M
*   Len:          Of control mesh of this knot vector.                       M
*   Order:        Of curve/surface the exploits this knot vector.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if KV has open end conditions.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotHasOpenEC, knot vectors, open end conditions                      M
*****************************************************************************/
CagdBType BspKnotHasOpenEC(const CagdRType *KnotVector, int Len, int Order)
{
    int i,
        l = Len + Order - 1;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    switch (Order) {
	case 1:
	    return TRUE;
	case 2:
	    return IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[1], IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(KnotVector[l], KnotVector[l - 1], IRIT_UEPS);
	case 3:
	    return IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[1], IRIT_UEPS) &&
	           IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[2], IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(KnotVector[l], KnotVector[l - 1], IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(KnotVector[l], KnotVector[l - 2], IRIT_UEPS);
	    break;
	case 4:
	    return IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[1], IRIT_UEPS) &&
	           IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[2], IRIT_UEPS) &&
	           IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[3], IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(KnotVector[l], KnotVector[l - 1], IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(KnotVector[l], KnotVector[l - 2], IRIT_UEPS) &&
		   IRIT_APX_EQ_EPS(KnotVector[l], KnotVector[l - 3], IRIT_UEPS);
	    break;
	default:
	    for (i = 0; ++i < Order; )
	       if (!IRIT_APX_EQ_EPS(KnotVector[0], KnotVector[i], IRIT_UEPS))
		    return FALSE;

	    for (i = l; --i >= Len; )
	       if (!IRIT_APX_EQ_EPS(KnotVector[l],
			       KnotVector[i], IRIT_UEPS))
		    return FALSE;
	    break;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff t is in the parametric domain as define by the knot       M
* vector KnotVector, its length Len, and the order Order.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To verify t is indeed in.                                 M
*   Len:           Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   Order:         Order of curve/surface using KnotVector.                  M
*   Periodic:      TRUE if this KnotVector is periodic.                      M
*   t:             Parametric value to verify.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE, if t is contained in the parametric domain, FALSE    M
*                 otherwise.                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotParamInDomain, parametric domain, knot vectors                    M
*****************************************************************************/
CagdBType BspKnotParamInDomain(const CagdRType *KnotVector,
			       int Len,
			       int Order,
			       CagdBType Periodic,
			       CagdRType t)
{
    CagdRType r1, r2;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    r1 = KnotVector[Order - 1];
    r2 = KnotVector[Len + (Periodic ? Order - 1 : 0)];

    return (r1 < t || IRIT_APX_EQ_EPS(r1, t, IRIT_UEPS))
        && (r2 > t || IRIT_APX_EQ_EPS(r2, t, IRIT_UEPS));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the index of the last knot which is less than or equal to t in the M
* given knot vector KnotVector of length Len.  IRIT_APX_EQ_UEPS is used in        M
* equality.								     M
*   Parameter t is assumed to be in the parametric domain for the knot       M
* vector.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To search for a knot with the LE relation to t.           M
*   Len:           Of knot vector. This is not the length of the             M
*                  curve/surface using this KnotVector.                      M
*   t:             The parameter value to search for the LE relation.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           Index of last knot in KnotVector that is LE t, or -1 if   M
*                  t is below the first knot.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotLastIndexLE, knot vectors                                         M
*****************************************************************************/
int BspKnotLastIndexLE(const CagdRType *KnotVector, int Len, CagdRType t)
{
    int i,
	Step = Len >> 1,
        StartIndex = 0;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    /* Do a rough binary search. */
    while (Step > 2) {
	if (KnotVector[StartIndex + Step] <= t ||
	    IRIT_APX_EQ_EPS(KnotVector[StartIndex + Step], t, IRIT_UEPS))
	    StartIndex += Step;
	Step >>= 1;
    }

    /* Find the exact location. */
    for (i = StartIndex, KnotVector = &KnotVector[StartIndex];
	 i < Len && (*KnotVector <= t ||
		     IRIT_APX_EQ_EPS(*KnotVector, t, IRIT_UEPS));
	 i++, KnotVector++);

    return i - 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the index of the last knot which is less t in the given knot       M
* vector KnotVector of length Len. IRIT_APX_EQ_EPS is used for equality.          M
*   Parameter t is assumed to be in the parametric domain for the knot       M
* vector.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To search for a knot with the L relation to t.            M
*   Len:           Of knot vector. This is not the length of the             M
*                  curve/surface using this KnotVector.                      M
*   t:             The parameter value to search for the L relation.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           Index of last knot in KnotVector that is less than t or   M
*                  -1 if t is below the first knot.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotLastIndexL, knot vectors                                          M
*****************************************************************************/
int BspKnotLastIndexL(const CagdRType *KnotVector, int Len, CagdRType t)
{
    int i,
	Step = Len >> 1,
        StartIndex = 0;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    /* Do a rough binary search. */
    while (Step > 2) {
	if (KnotVector[StartIndex + Step] < t &&
	    !IRIT_APX_EQ_EPS(KnotVector[StartIndex + Step], t, IRIT_UEPS))
	    StartIndex += Step;
	Step >>= 1;
    }

    /* Find the exact location. */
    for (i = StartIndex, KnotVector = &KnotVector[StartIndex];
	 i < Len && *KnotVector < t && !IRIT_APX_EQ_EPS(*KnotVector, t, IRIT_UEPS);
	 i++, KnotVector++);

    return i - 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the index of the first knot which is greater than t in the given   M
* knot vector KnotVector of length Len. IRIT_APX_EQ_EPS is used for equality.     M
*   Parameter t is assumed to be in the parametric domain for the knot       M
* vector.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To search for a knot with the G relation to t.            M
*   Len:           Of knot vector. This is not the length of the             M
*                  curve/surface using this KnotVector.                      M
*   t:             The parameter value to search for the G relation.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           Index of first knot in KnotVector that is greater than t  M
*                  or Len if t is greater than last knot in KnotVector.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotFirstIndexG, knot vectors                                         M
*****************************************************************************/
int BspKnotFirstIndexG(const CagdRType *KnotVector, int Len, CagdRType t)
{
    int i,
	Step = Len >> 1,
        StartIndex = Len - 1;
    const CagdRType *KV;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    /* Do a rough binary search. */
    while (Step > 2) {
	if (KnotVector[StartIndex - Step] > t &&
	    !IRIT_APX_EQ_EPS(KnotVector[StartIndex - Step], t, IRIT_UEPS))
	    StartIndex -= Step;
	Step >>= 1;
    }

    /* Find the exact location. */
    for (i = StartIndex, KV = &KnotVector[StartIndex];
	 i >= 0 && *KV > t && !IRIT_APX_EQ_EPS(*KV, t, IRIT_UEPS);
	 i--, KV--);

    return i + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a uniform periodic knot vector for Len Control points and order    M
* Order. The actual length of the KV is Len + Order + Order - 1.	     M
* If KnotVector is NULL it is being allocated dynamically.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:          Of control polygon/mesh of curve/surface that is to use    M
*                 this knot vector.                                          M
*   Order:        Of the curve/surface that is to use this knot vector.      M
*   KnotVector:   If new knot vector is to be saved here, otherwise a new    M
*                 space is allocated.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  The created uniform periodic knot vector, either newly     M
*                 allocated or given in Knotvector and just initialized.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotUniformPeriodic, knot vectors, periodic end conditions, end       M
*   conditions 								     M
*****************************************************************************/
CagdRType *BspKnotUniformPeriodic(int Len, int Order, CagdRType *KnotVector)
{
    int i,
	KVLen = Len + Order + Order - 1;
    CagdRType *KV,
	InteriorKnots = 1 + Len - Order;

    if (KnotVector == NULL)
	KV = KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * KVLen);
    else
	KV = KnotVector;

    for (i = -Order + 1; i < Len + Order; i++)
	*KV++ = i / InteriorKnots;

    return KnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a uniform floating knot vector for Len Control points and order    M
* Order. The actual length of the KV is Len + Order.			     M
* If KnotVector is NULL it is being allocated dynamically.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:          Of control polygon/mesh of curve/surface that is to use    M
*                 this knot vector.                                          M
*   Order:        Of the curve/surface that is to use this knot vector.      M
*   KnotVector:   If new knot vector is to be saved here, otherwise a new    M
*                 space is allocated.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  The created uniform floating knot vector, either newly     M
*                 allocated or given in Knotvector and just initialized.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotUniformFloat, knot vectors, floating end conditions, end          M
*   conditions								     M
*****************************************************************************/
CagdRType *BspKnotUniformFloat(int Len, int Order, CagdRType *KnotVector)
{
    int i;
    CagdRType *KV,
	InteriorKnots = 1 + Len - Order;

    if (KnotVector == NULL)
	KV = KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
								(Len + Order));
    else
	KV = KnotVector;

    for (i = -Order + 1; i <= Len; i++)
	*KV++ = i / InteriorKnots;

    return KnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a uniform open knot vector for Len Control points and order        M
* Order. The actual length of the KV is Len + Order.			     M
* If KnotVector is NULL it is being allocated dynamically.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:          Of control polygon/mesh of curve/surface that is to use    M
*                 this knot vector.                                          M
*   Order:        Of the curve/surface that is to use this knot vector.      M
*   KnotVector:   If new knot vector is to be saved here, otherwise a new    M
*                 space is allocated.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  The created uniform open knot vector, either newly         M
*                 allocated or given in Knotvector and just initialized.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotUniformOpen, knot vectors, open end conditions, end               M
*   conditions 								     M
*****************************************************************************/
CagdRType *BspKnotUniformOpen(int Len, int Order, CagdRType *KnotVector)
{
    int i, j;
    CagdRType
	InteriorKnots = 1 + Len - Order;
    CagdRType *KV;

    if (KnotVector == NULL)
	KV = KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
								(Len + Order));
    else
	KV = KnotVector;

    for (i = 0; i < Order; i++)
	*KV++ = 0.0;
    for (i = 1, j = Len - Order; i <= j; )
	*KV++ = i++ / InteriorKnots;
    for (j = 0; j < Order; j++)
	*KV++ = i / InteriorKnots;

    return KnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a discontinuous uniform open knot vector for Len Control points    M
* and order Order. The actual length of the KV is Len + Order.		     M
*   The generated sequence is of the form "0 0 0 0 1 1 1 2 2 2 ... n n n n"  M
* and Hence Len + Order must equal 2 * Order + 3 * x * (Order - 1).	     M
*   If KnotVector is NULL it is being allocated dynamically.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:          Of control polygon/mesh of curve/surface that is to use    M
*                 this knot vector.                                          M
*   Order:        Of the curve/surface that is to use this knot vector.      M
*   KnotVector:   If new knot vector is to be saved here, otherwise a new    M
*                 space is allocated.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  The created uniform open knot vector, either newly         M
*                 allocated or given in Knotvector and just initialized.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotDiscontUniformOpen, knot vectors, open end conditions,	     M
* end conditions 							     M
*****************************************************************************/
CagdRType *BspKnotDiscontUniformOpen(int Len, int Order, CagdRType *KnotVector)
{
    int i, j;
    CagdRType
	InteriorKnots = (Len - Order) / (Order - 1);
    CagdRType *KV;

    if (KnotVector == NULL)
	KV = KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
								(Len + Order));
    else
	KV = KnotVector;

    /* Make sure we have the proper length. */
    if (InteriorKnots != (int) InteriorKnots)
	CAGD_FATAL_ERROR(CAGD_ERR_OUT_OF_RANGE);

    for (i = 0; i < Order; i++)
	*KV++ = 0.0;
    for (i = 1; i <= InteriorKnots; i++)
        for (j = 0; j < Order - 1; j++)
	    *KV++ = i / (InteriorKnots + 1);
    for (j = 0; j < Order; j++)
	*KV++ = 1.0;

    return KnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Tests the given knot vector for uniformity and open/float end conditions.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:          Of control polygon/mesh of curve/surface that is using     M
*                 this knot vector.                                          M
*   Order:        Of the curve/surface that is using this knot vector.       M
*   KnotVector:   The knot vector to verify.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdEndConditionType:  CAGD_END_COND_GENERAL if general knot vector, or  M
*		CAGD_END_COND_OPEN/FLOAT/PERIODIC if knot vector is uniform  M
8		with open/float/periodic end conditions.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspIsKnotUniform, knot vectors, end conditions 			     M
*****************************************************************************/
CagdEndConditionType BspIsKnotUniform(int Len,
				      int Order,
				      const CagdRType *KnotVector)
{
    int i;
    CagdBType
	OpenEC = TRUE,
	FloatEC = TRUE;
    CagdRType const *KV;
    CagdRType Step;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    KV = &KnotVector[Order - 1];
    Step = KV[1] - KV[0];

    /* Verify that all interior knots are equally spaced. */
    for (i = 1, KV++; i <= Len - Order; i++, KV++)
	if (!IRIT_APX_EQ(Step, KV[1] - KV[0]))
	    return CAGD_END_COND_GENERAL;

    /* Test initial end condition. */
    for (i = 0, KV = KnotVector; i <= Order - 2; i++, KV++) {
	if (!IRIT_APX_EQ(Step, KV[1] - KV[0]))
	    FloatEC = FALSE;
	if (!IRIT_APX_EQ(KV[1], KV[0]))
	    OpenEC = FALSE;
    }

    /* Test final end condition. */
    for (i = 0, KV = &KnotVector[Len]; i <= Order - 2; i++, KV++) {
	if (!IRIT_APX_EQ(Step, KV[1] - KV[0]))
	    FloatEC = FALSE;
	if (!IRIT_APX_EQ(KV[1], KV[0]))
	    OpenEC = FALSE;
    }

    if (FloatEC)
	return CAGD_END_COND_FLOAT;
    else if (OpenEC)
	return CAGD_END_COND_OPEN;
    else
	return CAGD_END_COND_GENERAL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Tests the given knot vector for discontinuous uniformity and open/float    M
* end conditions.  That is all interior knots are of multiplicity Order-1    M
* and are uniformly spaced.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Len:          Of control polygon/mesh of curve/surface that is using     M
*                 this knot vector.                                          M
*   Order:        Of the curve/surface that is using this knot vector.       M
*   KnotVector:   The knot vector to verify.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdEndConditionType:  CAGD_END_COND_GENERAL if general knot vector, or  M
*		CAGD_END_COND_OPEN/FLOAT/PERIODIC if knot vector is uniform  M
*		with open/float/periodic end conditions.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspIsKnotDiscontUniform, knot vectors, end conditions 		     M
*****************************************************************************/
CagdEndConditionType BspIsKnotDiscontUniform(int Len,
					     int Order,
					     const CagdRType *KnotVector)
{
    int i, j;
    CagdBType
	OpenEC = TRUE,
	FloatEC = TRUE;
    CagdRType const *KV;
    CagdRType Step;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    KV = &KnotVector[Order];
    Step = KV[0] - KV[-1];

    /* Verify that all interior knots are equally spaced and of multiplicity */
    /* of Order-1.							     */
    for (i = 1; i <= Len - Order; i += Order - 1, KV += Order - 1) {
	for (j = 1; j < Order - 1; j++)
	    if (!IRIT_APX_EQ(KV[j], KV[j - 1]))
	        return CAGD_END_COND_GENERAL;

	if (!IRIT_APX_EQ(Step, KV[Order - 1] - KV[Order - 2]))
	    return CAGD_END_COND_GENERAL;
    }

    /* Test initial end condition. */
    for (i = 0, KV = KnotVector; i <= Order - 2; i++, KV++) {
	if (!IRIT_APX_EQ(Step, KV[1] - KV[0]))
	    FloatEC = FALSE;
	if (!IRIT_APX_EQ(KV[1], KV[0]))
	    OpenEC = FALSE;
    }

    /* Test final end condition. */
    for (i = 0, KV = &KnotVector[Len]; i <= Order - 2; i++, KV++) {
	if (!IRIT_APX_EQ(Step, KV[1] - KV[0]))
	    FloatEC = FALSE;
	if (!IRIT_APX_EQ(KV[1], KV[0]))
	    OpenEC = FALSE;
    }

    if (FloatEC)
	return CAGD_END_COND_FLOAT;
    else if (OpenEC)
	return CAGD_END_COND_OPEN;
    else
	return CAGD_END_COND_GENERAL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies a scale transformation to the given knot vector. Note scale        M
* transformation on the knot vector does not change the Bspline curve.	     M
*   Knot vector is scaled by Scale as KV[i] = KV[i] * Scale.                 M
*   Scaling is taken place in place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:     To affinely transform.                                   M
*   Len:            Of knot vector. This is not the length of the curve or   M
*                   surface using this knot vector.                          M
*   Scale:          Amount to scale the knot vector.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotAffineTrans, BspKnotAffineTrans2                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotScale, knot vectors, affine transformation                        M
*****************************************************************************/
void BspKnotScale(CagdRType *KnotVector, int Len, CagdRType Scale)
{
    int i;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    for (i = 0; i < Len; i++)
	KnotVector[i] = KnotVector[i] * Scale;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transformation to the given knot vector. Note affine     M
* transformation on the knot vector does not change the Bspline curve.	     M
*   Knot vector is translated by Translate amount and scaled by Scale as     M
*  KV[i] = (KV[i] - KV[0]) * Scale + (KV[0] + Translate).                    V
*   All transformation as taken place in place.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:     To affinely transform.                                   M
*   Len:            Of knot vector. This is not the length of the curve or   M
*                   surface using this knot vector.                          M
*   Translate:      Amount to translate the knot vector.                     M
*   Scale:          Amount to scale the knot vector.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotScale, BspKnotAffineTrans2, BspKnotAffineTransOrder               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAffineTrans, knot vectors, affine transformation                  M
*****************************************************************************/
void BspKnotAffineTrans(CagdRType *KnotVector,
			int Len,
			CagdRType Translate,
			CagdRType Scale)
{
    int i;
    CagdRType KV0;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    KV0 = KnotVector[0];

    for (i = 0; i < Len; i++)
	KnotVector[i] = (KnotVector[i] - KV0) * Scale + KV0 + Translate;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transformation to the given knot vector. Note affine     M
* transformation on the knot vector does not change the Bspline curve.	     M
*   Knot vector is translated and scaled so as to span the domain from       M
* MinVal top MaxVal. This works for open end condition curves only.	     M
*  KV[i] = (KV[i] - KV[0]) * Scale + MinVal,                                 V
* where Scale = (MaxVal - MinVal) / (KV[Len - 1] - KV[0]).		     M
*   All transformation as taken place in place.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:     To affinely transform.                                   M
*   Len:            Of knot vector. This is not the length of the curve or   M
*                   surface using this knot vector.                          M
*   MinVal, MaxVal: New parametric domain of knot vector.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotScale, BspKnotAffineTrans, BspKnotAffineTransOrder2               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAffineTrans2, knot vectors, affine transformation                 M
*****************************************************************************/
void BspKnotAffineTrans2(CagdRType *KnotVector,
			 int Len,
			 CagdRType MinVal,
			 CagdRType MaxVal)
{
    int i;
    CagdRType KV0, KVn, Scale;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    KV0 = KnotVector[0],
    KVn = KnotVector[Len - 1],
    Scale = (MaxVal - MinVal) / (KVn - KV0);

    for (i = 0; i < Len; i++)
	KnotVector[i] = (KnotVector[i] - KV0) * Scale + MinVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transformation to the given knot vector. Note affine     M
* transformation on the knot vector does not change the Bspline curve.	     M
*   Knot vector is translated by Translate amount and scaled by Scale as     M
*  KV[i] = (KV[i] - KV[Order-1]) * Scale + (KV[Order-1] + Translate).        V
*   All transformation as taken place in place.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:     To affinely transform.                                   M
*   Order:	    Order of the space using this knot vector.               M
*   Len:            Of knot vector. This is not the length of the curve or   M
*                   surface using this knot vector.                          M
*   Translate:      Amount to translate the knot vector.                     M
*   Scale:          Amount to scale the knot vector.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotScale, BspKnotAffineTrans2                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAffineTransOrder, knot vectors, affine transformation             M
*****************************************************************************/
void BspKnotAffineTransOrder(CagdRType *KnotVector,
			     int Order,
			     int Len,
			     CagdRType Translate,
			     CagdRType Scale)
{
    int i;
    CagdRType KV0;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    KV0 = KnotVector[Order - 1];

    for (i = 0; i < Len; i++)
	KnotVector[i] = (KnotVector[i] - KV0) * Scale + KV0 + Translate;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Applies an affine transformation to the given knot vector. Note affine     M
* transformation on the knot vector does not change the Bspline curve.	     M
*   Knot vector is translated and scaled so as to span the domain from       M
* MinVal top MaxVal. This works for open end condition curves only.	     M
*   KV[i] = (KV[i] - KV[Order - 1]) * Scale + MinVal,                        V
* where Scale = (MaxVal - MinVal) / (KV[Len - Order] - KV[Order - 1]).	     M
*   All transformation as taken place in place.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:     To affinely transform.                                   M
*   Order:	    Order of the space using this knot vector.               M
*   Len:            Of knot vector. This is not the length of the curve or   M
*                   surface using this knot vector.                          M
*   MinVal, MaxVal: New parametric domain of knot vector.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotScale, BspKnotAffineTrans2                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAffineTransOrder2, knot vectors, affine transformation            M
*****************************************************************************/
void BspKnotAffineTransOrder2(CagdRType *KnotVector,
			      int Order,
			      int Len,
			      CagdRType MinVal,
			      CagdRType MaxVal)
{
    int i;
    CagdRType KV0, KVn, Scale;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    KV0 = KnotVector[Order - 1],
    KVn = KnotVector[Len - Order],
    Scale = (MaxVal - MinVal) / (KVn - KV0);

    for (i = 0; i < Len; i++)
	KnotVector[i] = (KnotVector[i] - KV0) * Scale + MinVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an identical copy of a given knot vector KnotVector of length Len. M
*                                                                            *
* PARAMETERS:                                                                M
*   DstKV:        Destination address or NULL for a whole new copy.          M
*   SrcKV:        Knot vector to duplicate                                   M
*   Len:          Length of knot vector. This is not the length of the curve M
*                 or surface using this knot vector.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  The duplicated (destination) knot vector.                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotCopy, allocation, knot vectors                                    M
*****************************************************************************/
CagdRType *BspKnotCopy(CagdRType *DstKV, const CagdRType *SrcKV, int Len)
{
    CagdRType
	*NewKnotVector =
	    DstKV ? DstKV : (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);

#ifdef DEBUG
    if (SrcKV == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    CAGD_GEN_COPY(NewKnotVector, SrcKV, sizeof(CagdRType) * Len);

    return NewKnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverse a knot vector of length Len.	Reversing of knot vector keeps the   M
* knots monotonically non-decreasing as well as the parametric domain. Only  M
* the spaces between the knots are being flipped. For example the knot       M
* vector:                                                                    M
*                        [0 0 0 0 1 2 2 6 6 6 6]                             V
* is reversed to be:                                                         M
*                        [0 0 0 0 4 4 5 6 6 6 6]                             V
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:   Knot vector to be reversed.                                M
*   Len:          Length of knot vector. This is not the length of the curve M
*                 or surface using this knot vector.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  The reversed knot vector.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotReverse, knot vectors, reverse                                    M
*****************************************************************************/
CagdRType *BspKnotReverse(const CagdRType *KnotVector, int Len)
{
    int i;
    CagdRType *NewKnotVector, t;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    NewKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len),
    t = KnotVector[Len - 1] + KnotVector[0];

    for (i = 0; i < Len; i++)
	NewKnotVector[i] = t - KnotVector[Len - i - 1];

    return NewKnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a knot vector for a freeform that will fit the freeform if it   M
* was degree raised to NewOrder.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:       Current knot vector of freeform.                               M
*   Len:      Length of the freeform - number of control points.             M
*   Order:    Order of the freeform.                                         M
*   NewOrder: New order of the freeform.                                     M
*   NewLen:   New length of (dynamically) allocated knot vector.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:    A new knot vector, allocated dynamically, that would fit M
*		    this same freeform if degree raised.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotDegreeRaisedKV                                                    M
*****************************************************************************/
CagdRType *BspKnotDegreeRaisedKV(const CagdRType *KV,
				 int Len,
				 int Order,
				 int NewOrder,
				 int *NewLen)
{
    int i, j, k, Mult,
	DOrder = NewOrder - Order + 1;
    CagdRType *NewKV;

#ifdef DEBUG
    if (KV == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    /* This allocation is likely to be overestimating... */
    NewKV = (CagdRType *) IritMalloc(sizeof(CagdRType) *
				                  (Len * DOrder + Order) * 2);

    for (i = j = Mult = 0; i < Len + Order - 1; i++) {
        if (IRIT_APX_EQ_EPS(KV[i], KV[i + 1], IRIT_UEPS))
	    Mult++;
	else {
	    for (k = 0; k < Mult + DOrder; k++) {
	        NewKV[j++] = KV[i];
	    }
	    Mult = 0;
	}
    }
    if (Mult > 0 || Order == 1) {
        for (k = 0; k < Mult + DOrder; k++) {
	    NewKV[j++] = KV[i];
	}
    }
    *NewLen = j;

    return NewKV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a knot vector that contains all the knots in KnotVector1 that are  M
* not in KnotVector2.							     M
*   NewLen is set to new KnotVector length.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector1:   First knot vector.                                        M
*   Len1:          Length of KnotVector1. This is not the length of the      M
*                  curve or surface using this knot vector.                  M
*   KnotVector2:   Second knot vector.                                       M
*   Len2:          Length of KnotVector2. This is not the length of the      M
*                  curve or surface using this knot vector.                  M
*   NewLen:        To save the size of the knot vector that contains the     M
*                  computed subset of KnotVector1 / KnotVector2.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The subset of knot in KnotVector1 that are not in         M
*                  KnotVector2 (KnotVector1 / KnotVector2).                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotSubtrTwo, knot vectors, compatibility, refinement                 M
*****************************************************************************/
CagdRType *BspKnotSubtrTwo(const CagdRType *KnotVector1,
			   int Len1,
			   const CagdRType *KnotVector2,
			   int Len2,
			   int *NewLen)
{
    int i = 0,
	j = 0;
    CagdRType
	*NewKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len1),
	*t = NewKnotVector;

#ifdef DEBUG
    if (KnotVector1 == NULL || KnotVector2 == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    *NewLen = 0;
    while (i < Len1 && j < Len2) {
	if (IRIT_APX_EQ_EPS(KnotVector1[i], KnotVector2[j], IRIT_UEPS)) {
	    i++;
	    j++;
	}
	else if (KnotVector1[i] > KnotVector2[j]) {
	    j++;
	}
	else {
	    /* Current KnotVector1 value is less than current KnotVector2: */
	    *t++ = KnotVector1[i++];
	    (*NewLen)++;
	}
    }

    return NewKnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two knot vector KnotVector1 and KnotVector2 of length Len1 and Len2 M
* respectively into one. If Mult is not zero then knot multiplicity is       M
* tested not to be larger than Mult value.				     M
*   NewLen is set to new KnotVector length.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector1:   First  knot vector.                                       M
*   Len1:          Length of KnotVector1. This is not the length of the      M
*                  curve or surface using this knot vector.                  M
*   KnotVector2:   Second knot vector.                                       M
*   Len2:          Length of KnotVector2. This is not the length of the      M
*                  curve or surface using this knot vector.                  M
*   Mult:          Maximum multiplicity to allow in merged knot vector.      M
*   NewLen:        To save the size of the knot vector that contains the     M
*                  merged knot vectors.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The merged knot verctor (KnotVector1 U KnotVector2).      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotMergeTwo, knot vectors, compatibility, refinement                 M
*****************************************************************************/
CagdRType *BspKnotMergeTwo(const CagdRType *KnotVector1,
			   int Len1,
			   const CagdRType *KnotVector2,
			   int Len2,
			   int Mult,
			   int *NewLen)
{
    int i = 0,
	j = 0,
	k = 0,
        m = 0,
	Len = Len1 + Len2;
    CagdRType t,
	*NewKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);

#ifdef DEBUG
    if (KnotVector1 == NULL || KnotVector2 == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    if (Mult == 0)
	Mult = Len + 1;

    while (i < Len1 && j < Len2) {
	if (KnotVector1[i] < KnotVector2[j]) {
	    /* Use value from KnotVector1: */
	    t = KnotVector1[i++];
	}
	else {
	    /* Use value from KnotVector2: */
	    t = KnotVector2[j++];
	}

	if (k == 0 || (k > 0 &&
		       !IRIT_APX_EQ_EPS(NewKnotVector[k - 1], t, IRIT_UEPS))) {
	    NewKnotVector[k++] = t;
	    m = 1;
	}
	else if (m < Mult) {
	    NewKnotVector[k++] = t;
	    m++;
	}
    }

    while (i < Len1)
	NewKnotVector[k++] = KnotVector1[i++];
    while (j < Len2)
	NewKnotVector[k++] = KnotVector1[j++];

    /* It should be noted that k <= Len so there is a chance some of the new */
    /* KnotVector space will not be used (it is not memory leak!). If you    */
    /* really care that much - copy it to a new allocated vector of size k   */
    /* and return it while freeing the original of size Len.		     */
    *NewLen = k;

    return NewKnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two knot vector KnotVector1 and KnotVector2 of length Len1 and Len2 M
* respectively into one, from geometries of orders Order1 and Order2.	     M
*   Merged knot vector is for order ResOrder so that the resulting curve can M
* represent the discontinuities in both geometries.		             M
*   Assumes both knot vectors are open end spanning the same domain.         M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector1:   First  knot vector.                                       M
*   Len1:          Length of KnotVector1. This is not the length of the      M
*                  curve or surface using this knot vector.                  M
*   Order1:        Order of first knot vector's geometry.                    M
*   KnotVector2:   Second knot vector.                                       M
*   Len2:          Length of KnotVector2. This is not the length of the      M
*                  curve or surface using this knot vector.                  M
*   Order2:        Order of second knot vector's geometry.                   M
*   ResOrder:      Expected order of geometry that will use the merged       M
*                  knot vector.                                              M
*   NewLen:        To save the size of the knot vector that contains the     M
*                  merged knot vectors.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The merged knot verctor (KnotVector1 U KnotVector2).      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotContinuityMergeTwo, knot vectors, compatibility, refinement       M
*****************************************************************************/
CagdRType *BspKnotContinuityMergeTwo(const CagdRType *KnotVector1,
				     int Len1,
				     int Order1,
				     const CagdRType *KnotVector2,
				     int Len2,
				     int Order2,
				     int ResOrder,
				     int *NewLen)
{
    int l1, l2, m, cont,
	i = 0,
	j = 0,
	k = 0,
	Len = (Len1 + Len2 + 1) * (1 + ResOrder);
    CagdRType t,
	*NewKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * Len);

#ifdef DEBUG
    if (KnotVector1 == NULL || KnotVector2 == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    while (i < Len1 && j < Len2) {
	if (IRIT_APX_EQ_EPS(KnotVector1[i], KnotVector2[j], IRIT_UEPS)) {
	    /* Compute multiplicity and continuity. */
	    for (l1 = 1;
		 i + l1 < Len1 &&
		 IRIT_APX_EQ_EPS(KnotVector1[i], KnotVector1[i + l1], IRIT_UEPS);
		 l1++);
	    for (l2 = 1;
		 j + l2 < Len2 &&
		 IRIT_APX_EQ_EPS(KnotVector2[j], KnotVector2[j + l2], IRIT_UEPS);
		 l2++);
	    cont = IRIT_MIN(Order1 - l1, Order2 - l2) - 1;

	    /* Use value from KnotVector1: */
	    t = KnotVector1[i];
	    i += l1;
	    j += l2;
	}
	else if (KnotVector1[i] < KnotVector2[j]) {
	    /* Compute multiplicity and continuity. */
	    for (l1 = 1;
		 i + l1 < Len1 &&
		 IRIT_APX_EQ_EPS(KnotVector1[i], KnotVector1[i + l1], IRIT_UEPS);
		 l1++);
	    cont = Order1 - l1 - 1;

	    /* Use value from KnotVector1: */
	    t = KnotVector1[i];
	    i += l1;
	}
	else {
	    /* Compute multiplicity and continuity. */
	    for (l2 = 1;
		 j + l2 < Len2 &&
		 IRIT_APX_EQ_EPS(KnotVector2[j], KnotVector2[j + l2], IRIT_UEPS);
		 l2++);
	    cont = Order2 - l2 - 1;

	    /* Use value from KnotVector2: */
	    t = KnotVector2[j];
	    j += l2;
	}

	for (m = 0; m < ResOrder - cont - 1; m++)
	    NewKnotVector[k++] = t;
    }

    /* It should be noted that k <= Len so there is a chance some of the new */
    /* KnotVector space will not be used (it is not memory leak!). If you    */
    /* really care that much - copy it to a new allocated vector of size k   */
    /* and return it while freeing the original of size Len.		     */
    *NewLen = k;

    return NewKnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a new knot vector from the given KnotVector that includes knot     M
* values in the middle of any two adjacent knots that are different in       M
* value.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To average out.                                           M
*   Len:           Length of KnotVector. This is not the length of the       M
*                  curve or surface using this knot vector.                  M
*                  Len is updated in the end to the length of the returned   M
*		   vector.						     M
*   Order:	   Order of freeform geometry using this knot sequence.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The averaged knot vector of length (Len - Ave + 1).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotNodes                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotDoubleKnots, knot vectors, node values                            M
*****************************************************************************/
CagdRType *BspKnotDoubleKnots(const CagdRType *KnotVector, int *Len, int Order)
{
    int i, j;
    CagdRType
	*NewKnots = (CagdRType *) IritMalloc(sizeof(CagdRType) * *Len);

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    for (i = Order - 1, j = 0; i < *Len - Order; i++)
        if (KnotVector[i] < KnotVector[i + 1] - IRIT_EPS)
	    NewKnots[j++] = (KnotVector[i] + KnotVector[i + 1]) * 0.5;

    *Len = j;

    return NewKnots;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates a new knot vector from the given KnotVector that averages Ave	     M
* consequetive knots.							     M
*  Resulting vector will have (Len - Ave + 1) elements.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To average out.                                           M
*   Len:           Length of KnotVector. This is not the length of the       M
*                  curve or surface using this knot vector.                  M
*   Ave:           How many knots to average each time.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The averaged knot vector of length (Len - Ave + 1).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotNodes                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAverage, knot vectors, node values                                M
*****************************************************************************/
CagdRType *BspKnotAverage(const CagdRType *KnotVector, int Len, int Ave)
{
    int i,
	AveLen = Len - Ave + 1;
    CagdRType
	Ave1 = 1.0 / Ave,
	*AveVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * AveLen);

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    if (Ave > Len || Ave < 1)
	CAGD_FATAL_ERROR(CAGD_ERR_OUT_OF_RANGE);

    /* Compute the first average value. */
    for (AveVector[0] = 0.0, i = 0; i < Ave; i++)
	AveVector[0] += KnotVector[i];

    for (i = 1; i < AveLen; i++)
	AveVector[i] = AveVector[i - 1] + KnotVector[i + Ave - 1]
					- KnotVector[i - 1];

    for (i = 0; i < AveLen; i++)
	AveVector[i] *= Ave1;

    return AveVector;
}

/******************************************************************************
* DESCRIPTION:                                                               M
* Creates a new vector with the given KnotVector Node values. The given      M
* knot vector is assumed to have open end conditions.			     M
*   The nodes are the approximated parametric value associated with the each M
* control point. Therefore for a knot vector of length Len and order Order   M
* there are Len - Order control points and therefore nodes.		     M
*   Nodes are defined as (k = Order - 1 or degree):			     M
*	    								     M
*	  i+k								     V
*	   -			 	First Node N(i = 0)		     V
*	   \								     V
*          / KnotVector(j)		Last Node  N(i = Len - k - 2)	     V
*	   -								     V
*        j=i+1								     V
* N(i) = -----------------						     V
*	        k							     V
*	    								     M
* PARAMETERS:                                                                M
*   KnotVector:    To average out as nodes.                                  M
*   Len:           Length of KnotVector. This is not the length of the       M
*                  curve or surface using this knot vector.                  M
*   Order:         Of curve or surface that exploits this knot vector.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The nodes computed for the given knot vector.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotAverage, BspKnotPeriodicNodes                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotNodes, knot vectors, node values                                  M
*****************************************************************************/
CagdRType *BspKnotNodes(const CagdRType *KnotVector, int Len, int Order)
{
    int i,
	k = IRIT_MAX(Order - 1, 1),
	NodeLen = Len - Order;
    CagdRType const *KV;
    CagdRType TMin, TMax, *NodeVector, *NV,
	k1 = 1.0 / k;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    TMin = KnotVector[k];
    TMax = KnotVector[NodeLen];
    NodeVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * NodeLen);

    /* Compute the first average value. */
    for (NodeVector[0] = 0.0, i = 1; i <= k; i++)
	NodeVector[0] += IRIT_BOUND(KnotVector[i], TMin, TMax);

    for (i = 1, KV = &KnotVector[1], NV = NodeVector;
	 i < NodeLen;
	 i++, KV++, NV++)
	NV[1] = NV[0] + IRIT_BOUND(KV[k], TMin, TMax) -
		        IRIT_BOUND(KV[0], TMin, TMax);

    for (i = 0, NV = NodeVector; i < NodeLen; i++)
	*NV++ *= k1;

    return NodeVector;
}

/******************************************************************************
* DESCRIPTION:                                                               M
* Creates a new vector with the given KnotVector Node values. The given      M
* knot vector is assumed to have periodic end conditions.		     M
*   The nodes are the approximated parametric value associated with the each M
* control point. Therefore for a knot vector of length Len and order Order   M
* there are Len - Order control points and therefore nodes.		     M
*   Nodes are defined as (k = Order - 1 or degree):			     M
*	    								     M
*	  i+k								     V
*	   -			 	First Node N(i = 0)		     V
*	   \								     V
*          / KnotVector(j)		Last Node  N(i = Len - k - 2)	     V
*	   -								     V
*        j=i+1								     V
* N(i) = -----------------						     V
*	        k							     V
*	    								     M
* PARAMETERS:                                                                M
*   KnotVector:    To average out as nodes.                                  M
*   Len:           Length of periodic KnotVector. This is not the length of  M
*                  the curve or surface using this knot vector.              M
*   Order:         Of curve or surface that exploits this knot vector.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The nodes computed for the given knot vector.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotAverage, BspKnotNodes                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotPeriodicNodes, knot vectors, node values                          M
*****************************************************************************/
CagdRType *BspKnotPeriodicNodes(const CagdRType *KnotVector,
				int Len,
				int Order)
{
    int i,
	k = IRIT_MAX(Order - 1, 1),
	NodeLen = Len - Order;
    CagdRType const *KV;
    CagdRType TMin, TMax, *NodeVector, *NV,
	k1 = 1.0 / k;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    TMin = KnotVector[k];
    TMax = KnotVector[NodeLen];
    NodeLen -= Order - 1;     /* We have Order-1 less control points. */
    NodeVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * NodeLen);

    /* Compute the first average value. */
    for (NodeVector[0] = 0.0, i = 1; i <= k; i++)
        NodeVector[0] += KnotVector[i];

    for (i = 1, KV = &KnotVector[1], NV = NodeVector;
	 i < NodeLen;
	 i++, KV++, NV++)
	NV[1] = NV[0] + KV[k] - KV[0];

    for (i = 0, NV = NodeVector; i < NodeLen; i++, NV++) {
	*NV *= k1;
	if (*NV < TMin)
	    *NV = TMax - (TMin - *NV);
	else if (*NV >= TMax)
	    *NV = TMin + (*NV - TMax);
    }

    return NodeVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the nodes of a freeform curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute node values for.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *: Node values of the given curve.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvNodes, node values                                                M
*****************************************************************************/
CagdRType *CagdCrvNodes(const CagdCrvStruct *Crv)
{
    int i,
        Order = Crv -> Order,
	Length = CAGD_CRV_PT_LST_LEN(Crv);
    CagdRType *NodeVector;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    NodeVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * Order);
	    for (i = 0; i < Order; i++)
	        NodeVector[i] = i / ((CagdRType) (Order - 1));
	    break;
	case CAGD_CBSPLINE_TYPE:
	    if (CAGD_IS_PERIODIC_CRV(Crv))
	        NodeVector = BspKnotPeriodicNodes(Crv -> KnotVector,
						  CAGD_CRV_PT_LST_LEN(Crv) +
						                       Order,
						  Order);
	    else
	        NodeVector = BspKnotNodes(Crv -> KnotVector, Length + Order,
					  Order);
	    break;
	default:
	    NodeVector = NULL;
	    break;
    }

    return NodeVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the nodes of a freeform surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute node values for.                                    M
*   Dir:      Either the U or the V parametric direction.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *: Node values of the given surface and given parametric       M
*                direction.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfNodes, node values                                                M
*****************************************************************************/
CagdRType *CagdSrfNodes(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    int i,
        Order = Dir == CAGD_CONST_U_DIR ? Srf -> UOrder : Srf -> VOrder,
	Length = Dir == CAGD_CONST_U_DIR ? Srf -> ULength : Srf -> VLength;
    CagdRType *NodeVector;

    if (Dir != CAGD_CONST_U_DIR && Dir != CAGD_CONST_V_DIR) {
        CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	return NULL;
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    NodeVector = (CagdRType *) IritMalloc(sizeof(CagdRType) * Order);
	    for (i = 0; i < Order; i++)
	        NodeVector[i] = i / ((CagdRType) (Order - 1));
	    break;
	case CAGD_SBSPLINE_TYPE:
	    switch (Dir) {
	        default:
		    assert(0);
	        case CAGD_CONST_U_DIR:
		    if (CAGD_IS_UPERIODIC_SRF(Srf))
			NodeVector =
			    BspKnotPeriodicNodes(Srf -> UKnotVector,
						 CAGD_SRF_UPT_LST_LEN(Srf),
						 Order);
		    else
		        NodeVector = BspKnotNodes(Srf -> UKnotVector,
						  Length + Order, Order);
		    break;
	        case CAGD_CONST_V_DIR:
		    if (CAGD_IS_VPERIODIC_SRF(Srf))
			NodeVector =
			    BspKnotPeriodicNodes(Srf -> VKnotVector,
						 CAGD_SRF_VPT_LST_LEN(Srf),
						 Order);
		    else
		        NodeVector = BspKnotNodes(Srf -> VKnotVector,
						  Length + Order, Order);
		    break;
	    }
	    break;
	default:
	    NodeVector = NULL;
	    break;
    }

    return NodeVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds the parameter value with the largest coefficient of the curve using  M
* nodes values to estimate the coefficients' parameters.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           To compute the parameter node value of the largest        M
*                  coefficient.                                              M
*   Axis:          Which axis should we search for maximal coefficient?      M
*                  1 for X, 2 for Y, etc.                                    M
*   MaxVal:        The coefficient itself will be place herein.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:     The node parameter value of the detected maximal          M
*                  coefficient.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvMaxCoefParam, extremum                                             M
*****************************************************************************/
CagdRType BspCrvMaxCoefParam(const CagdCrvStruct *Crv,
			     int Axis,
			     CagdRType *MaxVal)
{
    int i,
	Index = 0,
	Length = Crv -> Length,
	Order = Crv -> Order;
    CagdRType
	*Points = Crv -> Points[Axis],
	R = *Points,
        *Nodes = BspKnotNodes(Crv -> KnotVector, Length + Order, Order);

    for (i = 0; i < Length; i++, Points++) {
	if (*Points > R) {
	    R = *Points;
	    Index = i;
	}
    }
    *MaxVal = R;

    R = Nodes[Index];
    IritFree(Nodes);

    return R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Finds the parameter value with the largest coefficient of the surface      M
* using nodes values to estimate the coefficients' parameters.		     M
* Returns a pointer to a static array of two elements holding U and V.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           To compute the parameter node value of the largest        M
*                  coefficient.                                              M
*   Axis:          Which axis should we search for maximal coefficient?      M
*                  1 for X, 2 for Y, etc.                                    M
*   MaxVal:        The coefficient itself will be place herein.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The node UV parameter values of the detected maximal      M
*                  coefficient.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfMaxCoefParam, extremum                                             M
*****************************************************************************/
CagdRType *BspSrfMaxCoefParam(const CagdSrfStruct *Srf,
			      int Axis,
			      CagdRType *MaxVal)
{
    IRIT_STATIC_DATA CagdRType UV[2];
    int i,
	UIndex = 0,
	VIndex = 0,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder;
    CagdRType
	*Points = Srf -> Points[Axis],
	R = *Points,
        *UNodes = BspKnotNodes(Srf -> UKnotVector, ULength + UOrder, UOrder),
        *VNodes = BspKnotNodes(Srf -> VKnotVector, VLength + VOrder, VOrder);

    for (i = 0; i < ULength * VLength; i++, Points++) {
	if (*Points > R) {
	    R = *Points;
	    UIndex = i % ULength;
	    VIndex = i / ULength;
	}
    }
    *MaxVal = R;

    UV[0] = UNodes[UIndex];
    UV[1] = VNodes[VIndex];
    IritFree(UNodes);
    IritFree(VNodes);
    return UV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new vector with t inserted as a new knot. Attempt is made to   M
* make sure t is in the knot vector domain.				     M
*   No test is made for the current multiplicity of knot t in KnotVector.    M
*   This function only constructs a refined knot vector and does not         M
* compute the actual refined coefficients.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To insert new knot t in.                                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Len:           Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   t:             The new knot t to insert.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   A new knot vector larger by one than KnotVector that      M
*                  contains t.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotInsertOne, knot vectors, knot insertion, refinement               M
*****************************************************************************/
CagdRType *BspKnotInsertOne(const CagdRType *KnotVector,
			    int Order,
			    int Len,
			    CagdRType t)
{
    int Mult = BspKnotFindMult(KnotVector, Order, Len, t) + 1;

    return BspKnotInsertMult(KnotVector, Order, &Len, t, Mult);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Inserts Mult knots with value t into the knot vector KnotVector.	     M
*   Attempt is made to make sure t in knot vector domain.		     M
*   If a knot equal to t (up to IRIT_APX_EQ) already exists with multiplicity i   M
* only (Mult - i) knot are being inserted into the new knot vector.	     M
*   Len is updated to the resulting knot vector.			     M
*   It is possible to DELETE a knot using this routine by specifying         M
* multiplicity less then current multiplicity!				     M
*   This function only constructs a refined knot vector and does not         M
* compute the actual refined coefficients.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To insert new knot t in.                                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Len:           Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   t:             The new knot t to insert.                                 M
*   Mult:          The multiplicity that this knot should have in resulting  M
*                  knot vector.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   A new knot vector derived from KnotVector that has        M
*                  a mltiplicity of exacly Mult at the knot t.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotInsertMult, knot vectors, knot insertion, refinement              M
*****************************************************************************/
CagdRType *BspKnotInsertMult(const CagdRType *KnotVector,
			     int Order,
			     int *Len,
			     CagdRType t,
			     int Mult)
{
    int i, CurrentMult, NewLen, FirstIndex;
    CagdRType *NewKnotVector;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    if (!BspKnotParamInDomain(KnotVector, *Len, Order, FALSE, t))
	CAGD_FATAL_ERROR(CAGD_ERR_T_NOT_IN_CRV);

    CurrentMult = BspKnotFindMult(KnotVector, Order, *Len, t);
    NewLen = *Len + Mult - CurrentMult;
    NewKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
    							(NewLen + Order));
    FirstIndex = BspKnotLastIndexL(KnotVector, *Len + Order, t) + 1;

    /* Copy all the knot before the knot t. */
    CAGD_GEN_COPY(NewKnotVector, KnotVector, sizeof(CagdRType) * FirstIndex);

    /* Insert Mult knot of value t. */
    for (i = FirstIndex; i < FirstIndex + Mult; i++)
	NewKnotVector[i] = t;

    /* And copy the second part. */
    CAGD_GEN_COPY(&NewKnotVector[FirstIndex + Mult],
		  &KnotVector[FirstIndex + CurrentMult],
		  sizeof(CagdRType) * (*Len + Order - FirstIndex - Mult + 1));

    *Len = NewLen;
    return NewKnotVector;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the multiplicity of knot t in knot vector KnotVector, zero if      M
* none.                                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To test multiplicity of knot value t at.                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Len:           Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Len + Order).     M
*   t:             The knot to verify the multiplicity of.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           Multiplicity of t in KnotVector.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotFindMult, knot vectors                                            M
*****************************************************************************/
int BspKnotFindMult(const CagdRType *KnotVector,
		    int Order,
		    int Len,
		    CagdRType t)
{
    int LastIndex, FirstIndex;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    if (!BspKnotParamInDomain(KnotVector, Len, Order, FALSE, t))
	CAGD_FATAL_ERROR(CAGD_ERR_T_NOT_IN_CRV);

    FirstIndex = BspKnotLastIndexL(KnotVector, Len + Order, t) + 1;

    for (LastIndex = FirstIndex;
	 LastIndex < Len && IRIT_APX_EQ_EPS(KnotVector[LastIndex], t, IRIT_UEPS);
	 LastIndex++);

    return LastIndex - FirstIndex;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the multiplicity/value vectors of the given knot sequence. For    M
* example: (0, 0, 0, 1, 2, 2, 3, 3, 3) will be converted to:		     M
* KnotValue of (0, 1, 2, 3) and KnotMult of (3, 1, 2, 3).		     M
*   KnotValue and KnotMult are assumed big enough vectors to hold the result.M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To derive its multiplicity/value vectors.                 M
*   Len:           Length of the KnotVector.				     M
*   KnotValues:    Vector of the unique values found in Knotvector.          M
*   KnotMultiplicities:   Multiplicity of unique values found in Knotvector. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Size of vectors KnotValues/KnotMultiplicities.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotsMultiplicityVector, knot vectors                                 M
*****************************************************************************/
int BspKnotsMultiplicityVector(const CagdRType *KnotVector,
			       int Len,
			       CagdRType *KnotValues,
			       int *KnotMultiplicities)
{
    int i, j;

    KnotValues[0] = KnotVector[0];
    KnotMultiplicities[0] = 1;
    for (i = 1, j = 0; i < Len; i++) {
        if (IRIT_APX_EQ_EPS(KnotVector[i], KnotValues[j], IRIT_UEPS))
	    KnotMultiplicities[j]++;
	else {
	    KnotValues[++j] = KnotVector[i];
	    KnotMultiplicities[j] = 1;
	}
    }

    return j + 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector to a potential C0 discontinuity.		     M
*   Returns TRUE if found one and set t to its parameter value.		     M
*   Assumes knot vector has open end condition.				     M
*   A knot vector with multiplicity of a knot of (Order) can be C0           M
* discontinuous at that knot. However, this is only a necessary condition    M
* for C0 discontinuity in the geometry.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To test for potential C0 dicontinuities.                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Length:        Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   t:             Where to put the parameter value (knot) that can be C0    M
*                  discontinuous.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a potential C0 discontinuity, FALSE         M
*		   otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfKnotC1Discont, BspKnotC1Discont, BspKnotC2Discont,	             M
*   BspKnotAllC1Discont						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotC0Discont, knot vectors, continuity, discontinuity                M
*****************************************************************************/
CagdBType BspKnotC0Discont(const CagdRType *KnotVector,
			   int Order,
			   int Length,
			   CagdRType *t)
{
    int i, j;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    if (Length < 2) /* Constant order... */
	return FALSE;

    /* Search for discontinuities. */
    for (i = Order; i < Length; ) {
        for (j = i + 1;
	     j < Length && IRIT_APX_EQ_EPS(KnotVector[j], KnotVector[i], IRIT_UEPS);
	     j++);
	if (j - i >= Order) {
	    *t = KnotVector[i];
	    return TRUE;
	}
	i = j;
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector to a potential C1 discontinuity.		     M
*   Returns TRUE if found one and set t to its parameter value.		     M
*   Assumes knot vector has open end condition.				     M
*   A knot vector with multiplicity of a knot of (Order - 1) can be C1       M
* discontinuous at that knot. However, this is only a necessary condition    M
* for C1 discontinuity in the geometry.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To test for potential C1 dicontinuities.                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Length:        Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   t:             Where to put the parameter value (knot) that can be C1    M
*                  discontinuous.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a potential C1 discontinuity, FALSE         M
*		   otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfKnotC1Discont, BspKnotC0Discont, BspKnotC2Discont                  M
*   BspKnotAllC1Discont							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotC1Discont, knot vectors, continuity, discontinuity                M
*****************************************************************************/
CagdBType BspKnotC1Discont(const CagdRType *KnotVector,
			   int Order,
			   int Length,
			   CagdRType *t)
{
    int i, Count;
    CagdRType LastT;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    LastT = KnotVector[0] - 1.0;

    /* Search for discontinuities from the center of KV to its ends. */
    for (i = IRIT_MAX(Order, (Length >> 1)), Count = 0; i < Length; i++) {
	if (IRIT_APX_EQ_EPS(LastT, KnotVector[i], IRIT_UEPS))
	    Count++;
	else {
	    Count = 1;
	    LastT = KnotVector[i];
	}

	if (Count >= Order - 1) {
	    *t = LastT;
	    return TRUE;
	}
    }

    /* Search for discontinuities from the center of KV to its beginning. */
    for (i = IRIT_MIN(IRIT_MAX(Order, (Length >> 1)) + Order - 2, Length - 1),
								    Count = 0;
	 i >= Order;
	 i--) {
	if (IRIT_APX_EQ_EPS(LastT, KnotVector[i], IRIT_UEPS))
	    Count++;
	else {
	    Count = 1;
	    LastT = KnotVector[i];
	}

	if (Count >= Order - 1) {
	    *t = LastT;
	    return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector to a potential C2 discontinuity.		     M
*   Returns TRUE if found one and set t to its parameter value.		     M
*   Assumes knot vector has open end condition.				     M
*   A knot vector with multiplicity of a knot of (Order - 1) can be C2       M
* discontinuous at that knot. However, this is only a necessary condition    M
* for C2 discontinuity in the geometry.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To test for potential C1 dicontinuities.                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Length:        Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   t:             Where to put the parameter value (knot) that can be C1    M
*                  discontinuous.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a potential C2 discontinuity, FALSE         M
*		   otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfKnotC1Discont, BspKnotC0Discont, BspKnotC1Discont                  M
*   BspKnotAllC1Discont							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotC2Discont, knot vectors, continuity, discontinuity                M
*****************************************************************************/
CagdBType BspKnotC2Discont(const CagdRType *KnotVector,
			   int Order,
			   int Length,
			   CagdRType *t)
{
    int i, Count;
    CagdRType LastT;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    LastT = KnotVector[0] - 1.0;

    /* Search for discontinuities from the center of KV to its ends. */
    for (i = IRIT_MAX(Order, (Length >> 1)), Count = 0; i < Length; i++) {
	if (IRIT_APX_EQ_EPS(LastT, KnotVector[i], IRIT_UEPS))
	    Count++;
	else {
	    Count = 1;
	    LastT = KnotVector[i];
	}

	if (Count >= Order - 2) {
	    *t = LastT;
	    return TRUE;
	}
    }

    /* Search for discontinuities from the center of KV to its beginning. */
    for (i = IRIT_MIN(IRIT_MAX(Order, (Length >> 1)) + Order - 2, Length - 1),
								    Count = 0;
	 i >= Order;
	 i--) {
	if (IRIT_APX_EQ_EPS(LastT, KnotVector[i], IRIT_UEPS))
	    Count++;
	else {
	    Count = 1;
	    LastT = KnotVector[i];
	}

	if (Count >= Order - 2) {
	    *t = LastT;
	    return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector for all potential C0 discontinuity. Returns    M
* a vector holding the parameter values of the potential C0 discontinuities, M
* NULL of none found.							     M
*   Sets n to length of returned vector.				     M
*   Assumes knot vector has open end condition.				     M
*   A knot vector with multiplicity of a knot of Order can be C0             M
* discontinuous at that knot. However, this is only a necessary condition    M
* for C0 discontinuity in the geometry.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To test for potential C0 dicontinuities.                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Length:        Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   n:             Length of returned vector - number of potential C0        M
*                  discontinuities found.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Vector holding all parametr values with potential C0      M
*                  discontinuities.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAllC0Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdRType *BspKnotAllC0Discont(const CagdRType *KnotVector,
			       int Order,
			       int Length,
			       int *n)
{
    int i, Count,
	C0DiscontCount = 0;
    CagdRType LastT, *C0Disconts;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    LastT = KnotVector[0] - 1.0,
    C0Disconts = (CagdRType *) IritMalloc(sizeof(CagdRType) * Length);

    for (i = Order, Count = 0; i < Length; i++) {
	if (IRIT_APX_EQ_EPS(LastT, KnotVector[i], IRIT_UEPS))
	    Count++;
	else {
	    Count = 1;
	    LastT = KnotVector[i];
	}

	if (Count >= Order) {
	    C0Disconts[C0DiscontCount++] = LastT;
	    Count = 0;
	}
    }

    if ((*n = C0DiscontCount) > 0) {
	return C0Disconts;
    }
    else {
	IritFree(C0Disconts);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector for all potential C1 discontinuity. Returns    M
* a vector holding the parameter values of the potential C1 discontinuities, M
* NULL of none found.							     M
*   Sets n to length of returned vector.				     M
*   Assumes knot vector has open end condition.				     M
*   A knot vector with multiplicity of a knot of (Order - 1) can be C1       M
* discontinuous at that knot. However, this is only a necessary condition    M
* for C1 discontinuity in the geometry.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   KnotVector:    To test for potential C1 dicontinuities.                  M
*   Order:         Of geometry that exploits KnotVector.                     M
*   Length:        Length of curve/surface using KnotVector. This is NOT the M
*                  length of KnotVector which is equal to (Length + Order).  M
*   n:             Length of returned vector - number of potential C1        M
*                  discontinuities found.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Vector holding all parametr values with potential C1      M
*                  discontinuities.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotAllC1Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdRType *BspKnotAllC1Discont(const CagdRType *KnotVector,
			       int Order,
			       int Length,
			       int *n)
{
    int i, Count,
	C1DiscontCount = 0;
    CagdRType LastT, *C1Disconts;

#ifdef DEBUG
    if (KnotVector == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    LastT = KnotVector[0] - 1.0,
    C1Disconts = (CagdRType *) IritMalloc(sizeof(CagdRType) * Length);

    for (i = Order, Count = 0; i < Length; i++) {
	if (IRIT_APX_EQ_EPS(LastT, KnotVector[i], IRIT_UEPS))
	    Count++;
	else {
	    Count = 1;
	    LastT = KnotVector[i];
	}

	if (Count >= Order - 1) {
	    C1Disconts[C1DiscontCount++] = LastT;
	    Count = 0;
	}
    }

    if ((*n = C1DiscontCount) > 0) {
	return C1Disconts;
    }
    else {
	IritFree(C1Disconts);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to determine where to sample along the provided paramtric domain, M
* given the C1 discontinuities along it.				     M
*   Returns a vector of length NumSamples.				     M
*   If C1Disconts != NULL (NumC1Disconts > 0), C1Discont is being freed.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PMin:            Minimum of parametric domain.                           M
*   PMax:            Maximum of parametric domain.                           M
*   NumSamples:      To allocate for the vector of samples.                  M
*   C1Disconts:      A vector of potential C1 discontinuities in the         M
*                    (PMin, PMax) domain. This vector is freed by this       M
*                    routine, if it is not NULL.                             M
*   NumC1Disconts:   Length of C1Discont. if zero then C1Discont == NULL.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:    A vector of the suggested set of sampling locations.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotParamValues, piecewise linear approximation, knot vectors         M
*****************************************************************************/
CagdRType *BspKnotParamValues(CagdRType PMin,
			      CagdRType PMax,
			      int NumSamples,
			      CagdRType *C1Disconts,
			      int NumC1Disconts)
{
    int i;
    CagdRType Step, *Samples;

    if (NumSamples <= 0) {
        if (C1Disconts != NULL)
	    IritFree(C1Disconts);

        return NULL;
    }

    Samples = (CagdRType *) IritMalloc(sizeof(CagdRType) * NumSamples);

    Samples[0] = PMin;
    if (NumSamples <= 1)
	return Samples;
    Samples[NumSamples - 1] = PMax;
    if (NumSamples <= 2)
	return Samples;

    if (NumC1Disconts == NumSamples - 2) {
    	/* Same number of C1 discontinuities and samples. Grab them all.     */
	for (i = 0; i < NumSamples - 2; i++)
    	    Samples[i + 1] = C1Disconts[i];
    }
    else if (NumC1Disconts > NumSamples - 2) {
    	/* More C1 discontinuities than we are sampling. Grab a subset of    */
	/* The discontinuities as the sampling set.			     */
    	Step = ((CagdRType) (NumC1Disconts - 1)) / (NumSamples - 2)
								- IRIT_UEPS;
	for (i = 0; i < NumSamples - 2; i++)
    	    Samples[i + 1] = C1Disconts[(int) (i * Step)];
    }
    else {
        int j, k,
	    *SamplesPerInterval = (int *)
	                        IritMalloc(sizeof(int) * (NumC1Disconts + 1));

	/* Compute how many samples we should have per discont. interval. */
	j = (NumSamples - 2 - NumC1Disconts) / (NumC1Disconts + 1);
	for (i = 0; i <= NumC1Disconts; i++)
	    SamplesPerInterval[i] = j + 1;  /* One more for the C1 discont. */
	j = NumSamples - 2 - NumC1Disconts - j * (NumC1Disconts + 1);

	/* Distribute j, the left over fraction, over all intervals. */
	for (i = 0; i <= NumC1Disconts && j-- > 0; i++)
	    SamplesPerInterval[i]++;

	/* More samples than C1 discontinuites. Uniformly distribute the C1  */
	/* discontinuites between the samples and linearly interpolate the   */
	/* sample values in between.					     */
	for (i = -1, k = 0; i < NumC1Disconts; i++) {
	    IrtRType
	        IntervalStart = i < 0 ? PMin : C1Disconts[i],
	        IntervalEnd = i + 1 >= NumC1Disconts ? PMax : C1Disconts[i + 1];

	    for (j = 0; j < SamplesPerInterval[i + 1]; j++) {
		CagdRType
		    t = j / ((CagdRType) SamplesPerInterval[i + 1]);

		Samples[k++] = IntervalStart * (1.0 - t) + IntervalEnd * t;
	    }
	}
	IritFree(SamplesPerInterval);

	assert(k == NumSamples - 1);             /* k should point at PMax. */
    }

    if (C1Disconts != NULL)
	IritFree(C1Disconts);

    return Samples;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a knot vector, make sure adjacent knots that are close "enough" are  M
* actually identical. Important for robustness of subdiv/refinement algs.    M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:        Knot vector to make robust, in place.                         M
*   Len:       Length of knot vector KV.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if the knot sequence has been modified.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotVerifyKVValidity                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotMakeRobustKV, knot vectors                                        M
*****************************************************************************/
int BspKnotMakeRobustKV(CagdRType *KV, int Len)
{
    int RetVal = FALSE;
    CagdRType
	*KVLast = KV + Len;

#ifdef DEBUG
    if (KV == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    while (++KV < KVLast) {
	if (*KV < KV[-1]) {
	    *KV = KV[-1];
	    RetVal = TRUE;
	}
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compare the two knot vectors for similarity.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   KV1, KV2:  The two knot vectors to compare.                              M
*   Len:       Length of knot vectors.                                       M
*   Eps:       Tolerance of equality.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if knot vectors are the same, FALSE otehrwise.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCtlPointsSame, CagdCrvsSame, CagdSrfsSame                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotVectorsSame                                                       M
*****************************************************************************/
CagdBType BspKnotVectorsSame(const CagdRType *KV1,
			     const CagdRType *KV2,
			     int Len,
			     CagdRType Eps)
{
    int i;

#ifdef DEBUG
    if (KV1 == NULL || KV2 == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    for (i = 0; i < Len; i++)
	if (!IRIT_APX_EQ_EPS(KV1[i], KV2[i], Eps))
	    return FALSE;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the two ends (knots outside the curve's domain) to match the      M
* same spacing as the inner knots on the other end...  Updates KV in place.  M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:      To update its knots outside the entity's domain.                M
*   Order:   Of the entity.                                                  M
*   Len:     Such that Len + Order equals the number of knots in KV.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotVerifyPeriodicKV                                                  M
*****************************************************************************/
void BspKnotVerifyPeriodicKV(CagdRType *KV, int Order, int Len)
{
    int i;

#ifdef DEBUG
    if (KV == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    for (i = Order; i < Order + Order - 1; i++) {
	CagdRType
	    Diff = KV[i] - KV[i - 1];

	KV[Len + 1 + i - Order] = KV[Len + i - Order] + Diff;
    }
    for (i = Len; i > Len - Order + 1; i--) {
	CagdRType
	    Diff = KV[i] - KV[i - 1];

	KV[i - Len + Order - 2] = KV[i - Len + Order - 1] - Diff;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Verify that the given knot sequence is a valid one upto tolerance Tol.   M
* That is no more than order knots are similar upto Tol and that the knot    M
* sequence is monotone.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   KV:      To update its knots outside the entity's domain.                M
*   Order:   Of the entity.                                                  M
*   Len:     Such that Len + Order equals the number of knots in KV.         M
*   Tol:     Tolerance to consider two knots the same.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if valid, FALSE if failed to validate.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspKnotVerifyKVValidity                                                  M
*****************************************************************************/
int BspKnotVerifyKVValidity(CagdRType *KV, int Order, int Len, CagdRType Tol)
{
    int i;

#ifdef DEBUG
    if (KV == NULL)
	CAGD_FATAL_ERROR(CAGD_ERR_NO_KV_FOUND);
#endif /* DEBUG */

    for (i = 1; i < Len + Order; i++) {
	if (KV[i] < KV[i - 1]) {
	    if (IRIT_APX_EQ_EPS(KV[i], KV[i - 1], Tol))
		KV[i] = KV[i - 1];                   /* Make them the same. */
	    else {
	        /* The input is not even monotone - mark this KV as invalid. */
	        return FALSE;
	    }
	}
    }

    /* We have a monotone knot sequence.  Make sure the end location has     */
    /* multiplicity of lower or equal to the order.			     */
    for (i = Len; i >= Order; i--) {
	if (KV[i] - KV[i - 1] < Tol)
	    KV[i - 1] = KV[i] - Tol;
	else
	    break;
    }

    /* Verify monotonicity. */
    for (i = 1; i < Len + Order; i++) {
	if (KV[i] < KV[i - 1])
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a monotone vector of reals, spread (almost) equal items so they    M
* are MinDist apart while keeping the minimal and maximal values the same.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:     Vector of reals to spread (almost) equal items in.              M
*	     Modified in place.						     M
*   Len:     Length of vector Vec.                                           M
*   MinDist: The minimal distance two adjacent item should have.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE if cannot be done (MinDist too large). M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspVecSpreadEqualItems                                                   M
*****************************************************************************/
int BspVecSpreadEqualItems(CagdRType *Vec, int Len, CagdRType MinDist)
{
    int Attempt, i, j;
    CagdRType Min, Max;

    if (Len < 2)
        return TRUE;

    Min = Vec[0];
    Max = Vec[Len - 1];

    for (Attempt = 0; Attempt < 3; Attempt++) {              /* DO 3 tries. */
        /* Do a forward pass. */
        for (i = 1; i < Len; i++) {
	    if (Vec[i] - Vec[i - 1] < MinDist) {
	        if (Vec[i - 1] + MinDist > Max)
		    break;
		else
		    Vec[i] = Vec[i - 1] + (MinDist + IRIT_EPS);
	    }
	}

        /* Do a backward pass. */
        for (j = Len - 1; j > 1; j--) {
	    if (Vec[j] - Vec[j - 1] < MinDist) {
	        if (Vec[j] - MinDist < Min)
		    break;
		else
		    Vec[j - 1] = Vec[j] - (MinDist + IRIT_EPS);
	    }
	}

	if (i >= Len && j <= 1)
	    break;  /* Vec is monotone with MinDist between adjacent items. */
    }

    return Attempt < 3;
}
