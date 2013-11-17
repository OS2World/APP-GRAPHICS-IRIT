/******************************************************************************
* Symbolic.c - Generic symbolic computation.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 92.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "symb_loc.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"

#define SYMB_CRV_AREA_APPROX_TOL 1e-6

static CagdCrvStruct *SymbCrvAddSubAux(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2,
				       CagdBType OperationAdd);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - add them coordinatewise.				     M
*   The two curves are promoted to same point type before the multiplication M
* can take place. Furthermore, order and continuity are matched as well.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curve to add up coordinatewise.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The summation of Crv1 + Crv2 coordinatewise.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvSub, SymbCrvMult                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvAdd, addition, symbolic computation                               M
*****************************************************************************/
CagdCrvStruct *SymbCrvAdd(const CagdCrvStruct *Crv1,
			  const CagdCrvStruct *Crv2)
{
    CagdRType TMin1, TMax1, TMin2, TMax2;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);
    if (!IRIT_APX_EQ(TMin1, TMin2) || !IRIT_APX_EQ(TMax1, TMax2)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	return NULL;
    }

    return SymbCrvAddSubAux(Crv1, Crv2, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - subtract them coordinatewise.			     M
*   The two curves are promoted to same point type before the multiplication M
* can take place. Furthermore, order and continuity are matched as well.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curve to subtract coordinatewise.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The difference of Crv1 - Crv2 coordinatewise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvAdd, SymbCrvMult                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvSub, subtraction, symbolic computation                            M
*****************************************************************************/
CagdCrvStruct *SymbCrvSub(const CagdCrvStruct *Crv1, const CagdCrvStruct *Crv2)
{
    CagdRType TMin1, TMax1, TMin2, TMax2;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);
    if (!IRIT_APX_EQ(TMin1, TMin2) || !IRIT_APX_EQ(TMax1, TMax2)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	return NULL;
    }

    return SymbCrvAddSubAux(Crv1, Crv2, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - multiply them coordinatewise.			     M
*   The two curves are promoted to same point type before the multiplication M
* can take place.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curve to multiply coordinatewise.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The product of Crv1 * Crv2 coordinatewise.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvInvert, SymbCrvMultScalar      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMult, product, symbolic computation                               M
*****************************************************************************/
CagdCrvStruct *SymbCrvMult(const CagdCrvStruct *Crv1,
			   const CagdCrvStruct *Crv2)
{
    CagdCrvStruct
	*ProdCrv = NULL;

    if (Crv1 -> GType == CAGD_CBEZIER_TYPE &&
	Crv2 -> GType == CAGD_CBEZIER_TYPE)
	ProdCrv = BzrCrvMult(Crv1, Crv2);
    else if ((Crv1 -> GType == CAGD_CBEZIER_TYPE ||
	      Crv1 -> GType == CAGD_CBSPLINE_TYPE) &&
	     (Crv2 -> GType == CAGD_CBEZIER_TYPE ||
	      Crv2 -> GType == CAGD_CBSPLINE_TYPE))
	ProdCrv = BspCrvMult(Crv1, Crv2);
    else
	SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);

    return ProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar curve, returns a scalar curve representing the reciprocal   M
* values, by making it rational (if was not one) and flipping the numerator  M
* and the denominator.					       		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A scalar curve to compute a reciprocal value for.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A rational scalar curve that is equal to the          M
*                      reciprocal value of Crv.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvMult, SymbCrvMultScalar        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvInvert, division, symbolic computation, reciprocal value          M
*****************************************************************************/
CagdCrvStruct *SymbCrvInvert(const CagdCrvStruct *Crv)
{
    int i;
    CagdRType *R;
    CagdCrvStruct
	*NewCrv = CagdPeriodicCrvNew(Crv -> GType, CAGD_PT_P1_TYPE,
				     Crv -> Length, Crv -> Periodic);
    NewCrv -> Order = Crv -> Order;

    switch (Crv -> PType) {
	case CAGD_PT_E1_TYPE:
	    CAGD_GEN_COPY(NewCrv -> Points[0], Crv -> Points[1],
			  sizeof(CagdRType) * Crv -> Length);
	    for (i = 0, R = NewCrv -> Points[1];
		 i < Crv -> Length;
		 i++)
		*R++ = 1.0;
	    break;
	case CAGD_PT_P1_TYPE:
	    CAGD_GEN_COPY(NewCrv -> Points[0], Crv -> Points[1],
			  sizeof(CagdRType) * Crv -> Length);
	    CAGD_GEN_COPY(NewCrv -> Points[1], Crv -> Points[0],
			  sizeof(CagdRType) * Crv -> Length);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNSUPPORT_PT);
	    break;
    }

    if (CAGD_IS_BSPLINE_CRV(Crv)) {
	NewCrv -> KnotVector = BspKnotCopy(NULL, Crv -> KnotVector,
				   Crv -> Length + Crv -> Order +
				   (Crv -> Periodic ? Crv -> Order - 1 : 0));
    }

    CAGD_PROPAGATE_ATTR(NewCrv, Crv);

    return NewCrv;

}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve, scale it by Scale.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      A curve to scale by magnitude Scale.                           M
*   Scale:    Scaling factor.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curves scaled by Scale compared to Crv.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvMult, SymbCrvMultScalar        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvScalarScale, scaling, symbolic computation                        M
*****************************************************************************/
CagdCrvStruct *SymbCrvScalarScale(const CagdCrvStruct *Crv, CagdRType Scale)
{
    int i, j;
    CagdRType *R;
    CagdCrvStruct
	*TCrv = CagdCrvCopy(Crv);

    for (j = 1; j <= CAGD_NUM_OF_PT_COORD(TCrv -> PType); j++)
	for (i = 0, R = TCrv -> Points[j]; i < TCrv -> Length; i++)
	    *R++ *= Scale;

    return TCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - computes their dot product.			     M
*   Returned curve is a scalar curve representing the dot product of the     M
* two given curves.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curve to multiply and compute a dot product for.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A scalar curve representing the dot product of        M
*                      Crv1 . Crv2.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvScalarScale, SymbCrvVecDotProd, SymbCrvMult, SymbCrvMultScalar    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvDotProd, product, dot product, symbolic computation               M
*****************************************************************************/
CagdCrvStruct *SymbCrvDotProd(const CagdCrvStruct *Crv1,
			      const CagdCrvStruct *Crv2)
{
    CagdCrvStruct *PCrvW, *PCrvX, *PCrvY, *PCrvZ, *TCrv1, *TCrv2, *DotProdCrv,
	*ProdCrv = SymbCrvMult(Crv1, Crv2);

    SymbCrvSplitScalar(ProdCrv, &PCrvW, &PCrvX, &PCrvY, &PCrvZ);
    CagdCrvFree(ProdCrv);

    if (PCrvY != NULL) {
	TCrv1 = SymbCrvAdd(PCrvX, PCrvY);
	CagdCrvFree(PCrvX);
	CagdCrvFree(PCrvY);
    }
    else
	TCrv1 = PCrvX;

    if (PCrvZ != NULL) {
	TCrv2 = SymbCrvAdd(TCrv1, PCrvZ);
	CagdCrvFree(TCrv1);
	CagdCrvFree(PCrvZ);
	TCrv1 = TCrv2;
    }

    DotProdCrv = SymbCrvMergeScalar(PCrvW, TCrv1, NULL, NULL);
    CagdCrvFree(PCrvW);
    CagdCrvFree(TCrv1);

    return DotProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a vector - computes their dot product.                   M
*   Returned curve is a scalar curve representing the dot product.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  Curve to multiply and compute a dot product for.                   M
*   Vec:  Vector to project Crv onto.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A scalar curve representing the dot product of        M
*                      Crv . Vec.                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvMult, SymbCrvMultScalar, SymbCrvCrossProd         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvVecDotProd, product, dot product, symbolic computation            M
*****************************************************************************/
CagdCrvStruct *SymbCrvVecDotProd(const CagdCrvStruct *Crv,
				 const CagdVType Vec)
{
    CagdCrvStruct *PCrvW, *PCrvX, *PCrvY, *PCrvZ, *TCrv, *DotProdCrv;

    SymbCrvSplitScalar(Crv, &PCrvW, &PCrvX, &PCrvY, &PCrvZ);

    TCrv = SymbCrvScalarScale(PCrvX, Vec[0]);
    CagdCrvFree(PCrvX);
    PCrvX = TCrv;

    if (PCrvY != NULL) {
	TCrv = SymbCrvScalarScale(PCrvY, Vec[1]);
	CagdCrvFree(PCrvY);
	PCrvY = TCrv;

	TCrv = SymbCrvAdd(PCrvX, PCrvY);
        CagdCrvFree(PCrvX);
        CagdCrvFree(PCrvY);

	PCrvX = TCrv;
    }

    if (PCrvZ != NULL) {
	TCrv = SymbCrvScalarScale(PCrvZ, Vec[2]);
	CagdCrvFree(PCrvZ);
	PCrvZ = TCrv;

	TCrv = SymbCrvAdd(PCrvX, PCrvZ);
        CagdCrvFree(PCrvX);
        CagdCrvFree(PCrvZ);

	PCrvX = TCrv;
    }

    DotProdCrv = SymbCrvMergeScalar(PCrvW, PCrvX, NULL, NULL);
    CagdCrvFree(PCrvW);
    CagdCrvFree(PCrvX);

    return DotProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - a vector curve Crv1 and a scalar curve Crv2, multiply   M
* all Crv1's coordinates by the scalar curve Crv2.			     M
*   Returned curve is a curve representing the product of the two given	     M
* curves.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  Two curve to multiply.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve representing the product of Crv1 and Crv2.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvMult, SymbCrvCrossProd,        M
*   SymbSrfMultScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMultScalar, product, symbolic computation  		             M
*****************************************************************************/
CagdCrvStruct *SymbCrvMultScalar(const CagdCrvStruct *Crv1,
				 const CagdCrvStruct *Crv2)
{
    CagdCrvStruct *PCrv1W, *PCrv1X, *PCrv1Y, *PCrv1Z,
		  *PCrv2W, *PCrv2X, *PCrv2Y, *PCrv2Z,
		  *TCrv, *ProdCrv;

    SymbCrvSplitScalar(Crv1, &PCrv1W, &PCrv1X, &PCrv1Y, &PCrv1Z);
    SymbCrvSplitScalar(Crv2, &PCrv2W, &PCrv2X, &PCrv2Y, &PCrv2Z);

    TCrv = SymbCrvMult(PCrv1X, PCrv2X);
    CagdCrvFree(PCrv1X);
    PCrv1X = TCrv;

    if (PCrv1Y != NULL) {
	TCrv = SymbCrvMult(PCrv1Y, PCrv2X);
	CagdCrvFree(PCrv1Y);
	PCrv1Y = TCrv;
    }

    if (PCrv1Z != NULL) {
	TCrv = SymbCrvMult(PCrv1Z, PCrv2X);
	CagdCrvFree(PCrv1Z);
	PCrv1Z = TCrv;
    }

    if (PCrv1W != NULL && PCrv2W != NULL) {
	TCrv = SymbCrvMult(PCrv1W, PCrv2W);
	CagdCrvFree(PCrv1W);
	PCrv1W = TCrv;
    }
    else if (PCrv2W != NULL) {
	PCrv1W = PCrv2W;
	PCrv2W = NULL;
    }

    ProdCrv = SymbCrvMergeScalar(PCrv1W, PCrv1X, PCrv1Y, PCrv1Z);

    CagdCrvFree(PCrv1W);
    CagdCrvFree(PCrv1X);
    CagdCrvFree(PCrv1Y);
    CagdCrvFree(PCrv1Z);
    CagdCrvFree(PCrv2W);
    CagdCrvFree(PCrv2X);
    CagdCrvFree(PCrv2Y);
    CagdCrvFree(PCrv2Z);

    return ProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - computes their cross product.			     M
*   Returned curve is a scalar curve representing the cross product of the   M
* two given curves.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2:  Two curve to multiply and compute a cross product for.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A scalar curve representing the cross product of      M
*                      Crv1 x Crv2.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvMult, SymbCrvMultScalar        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrossProd, product, cross product, symbolic computation           M
*****************************************************************************/
CagdCrvStruct *SymbCrvCrossProd(const CagdCrvStruct *CCrv1,
				const CagdCrvStruct *CCrv2)
{
    CagdCrvStruct *Crv1W, *Crv1X, *Crv1Y, *Crv1Z, 
		  *Crv2W, *Crv2X, *Crv2Y, *Crv2Z,
		  *TCrv1, *TCrv2, *CrossProdCrv,
		  *PCrvW, *PCrvX, *PCrvY, *PCrvZ, *Crv1, *Crv2;

    if (CCrv1 -> GType != CAGD_PT_E3_TYPE &&
	CCrv1 -> GType != CAGD_PT_P3_TYPE) {
	Crv1 = CagdCoerceCrvTo(CCrv1,
			       CAGD_IS_RATIONAL_CRV(CCrv1) ? CAGD_PT_P3_TYPE
							   : CAGD_PT_E3_TYPE,
			       FALSE);
	SymbCrvSplitScalar(Crv1, &Crv1W, &Crv1X, &Crv1Y, &Crv1Z);
	CagdCrvFree(Crv1);
    }
    else
	SymbCrvSplitScalar(CCrv1, &Crv1W, &Crv1X, &Crv1Y, &Crv1Z);

    if (CCrv2 -> GType != CAGD_PT_E3_TYPE &&
	CCrv2 -> GType != CAGD_PT_P3_TYPE) {
	Crv2 = CagdCoerceCrvTo(CCrv2,
			       CAGD_IS_RATIONAL_CRV(CCrv2) ? CAGD_PT_P3_TYPE
							   : CAGD_PT_E3_TYPE,
			       FALSE);
        SymbCrvSplitScalar(Crv2, &Crv2W, &Crv2X, &Crv2Y, &Crv2Z);
	CagdCrvFree(Crv2);
    }
    else
	SymbCrvSplitScalar(CCrv2, &Crv2W, &Crv2X, &Crv2Y, &Crv2Z);

    /* Cross product X axis. */
    TCrv1 = SymbCrvMult(Crv1Y, Crv2Z);
    TCrv2 = SymbCrvMult(Crv2Y, Crv1Z);
    PCrvX = SymbCrvSub(TCrv1, TCrv2);
    CagdCrvFree(TCrv2);
    CagdCrvFree(TCrv1);

    /* Cross product Y axis. */
    TCrv1 = SymbCrvMult(Crv1Z, Crv2X);
    TCrv2 = SymbCrvMult(Crv2Z, Crv1X);
    PCrvY = SymbCrvSub(TCrv1, TCrv2);
    CagdCrvFree(TCrv2);
    CagdCrvFree(TCrv1);

    /* Cross product Z axis. */
    TCrv1 = SymbCrvMult(Crv1X, Crv2Y);
    TCrv2 = SymbCrvMult(Crv2X, Crv1Y);
    PCrvZ = SymbCrvSub(TCrv1, TCrv2);
    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);

    /* Cross product W axis. */
    if (Crv1W || Crv2W) {
	if (Crv1W == NULL)
	    PCrvW = CagdCrvCopy(Crv2W);
	else if (Crv2W == NULL)
	    PCrvW = CagdCrvCopy(Crv1W);
	else
	    PCrvW = SymbCrvMult(Crv1W, Crv2W);
    }
    else
	PCrvW = NULL;

    CagdCrvFree(Crv1X);
    CagdCrvFree(Crv1Y);
    CagdCrvFree(Crv1Z);
    CagdCrvFree(Crv1W);

    CagdCrvFree(Crv2X);
    CagdCrvFree(Crv2Y);
    CagdCrvFree(Crv2Z);
    CagdCrvFree(Crv2W);

    if (!CagdMakeCrvsCompatible(&PCrvX, &PCrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvX, &PCrvZ, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvY, &PCrvZ, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvW, &PCrvX, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvW, &PCrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvW, &PCrvZ, TRUE, TRUE))
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

    CrossProdCrv = SymbCrvMergeScalar(PCrvW, PCrvX, PCrvY, PCrvZ);
    CagdCrvFree(PCrvX);
    CagdCrvFree(PCrvY);
    CagdCrvFree(PCrvZ);
    CagdCrvFree(PCrvW);

    return CrossProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a vector - computes their cross product.		     M
*   Returned curve is a scalar curve representing the cross product of   M
* the curve and vector.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  Curve to multiply and compute a cross product for.               M
*   Vec:  Vector to cross product Crv with.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A vector curve representing the cross product of    M
*                      Crv x Vec.                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvScalarScale, SymbCrvMultScalar,M
*   SymbCrvInvert, SymbCrvCrossProd                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvVecCrossProd, product, cross product, symbolic computation        M
*****************************************************************************/
CagdCrvStruct *SymbCrvVecCrossProd(const CagdCrvStruct *Crv,
				   const CagdVType Vec)
{
    CagdCrvStruct *CrvW, *CrvX, *CrvY, *CrvZ, *TCrv,
		  *TCrv1, *TCrv2, *CrossProdCrv,
	*PCrvW = NULL,
	*PCrvX = NULL,
	*PCrvY = NULL,
	*PCrvZ = NULL;

    if (Crv -> GType != CAGD_PT_E3_TYPE && Crv -> GType != CAGD_PT_P3_TYPE) {
	TCrv = CagdCoerceCrvTo(Crv,
			       CAGD_IS_RATIONAL_CRV(Crv) ? CAGD_PT_P3_TYPE
							 : CAGD_PT_E3_TYPE,
			       FALSE);
	SymbCrvSplitScalar(TCrv, &CrvW, &CrvX, &CrvY, &CrvZ);
	CagdCrvFree(TCrv);
    }
    else
	SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);

    if (CrvX == NULL || CrvY == NULL)
	SYMB_FATAL_ERROR(SYMB_ERR_NO_CROSS_PROD);

    /* Cross product X axis. */
    TCrv1 = CagdCrvCopy(CrvY);
    CagdCrvTransform(TCrv1, NULL, Vec[2]);
    if (CrvZ != NULL) {
	TCrv2 = CagdCrvCopy(CrvZ);
	CagdCrvTransform(TCrv2, NULL, Vec[1]);

	PCrvX = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv2);
	CagdCrvFree(TCrv1);
    }
    else
	PCrvX = TCrv1;

    /* Cross product Y axis. */
    TCrv2 = CagdCrvCopy(CrvX);
    CagdCrvTransform(TCrv2, NULL, Vec[2]);
    if (CrvZ != NULL) {
	TCrv1 = CagdCrvCopy(CrvZ);
	CagdCrvTransform(TCrv1, NULL, Vec[0]);

	PCrvY = SymbCrvSub(TCrv1, TCrv2);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
    }
    else
	PCrvY = TCrv2;

    /* Cross product Z axis. */
    TCrv1 = CagdCrvCopy(CrvX);
    CagdCrvTransform(TCrv1, NULL, Vec[1]);
    TCrv2 = CagdCrvCopy(CrvY);
    CagdCrvTransform(TCrv2, NULL, Vec[0]);

    PCrvZ = SymbCrvSub(TCrv1, TCrv2);
    CagdCrvFree(TCrv1);
    CagdCrvFree(TCrv2);

    /* Cross product W axis. */
    if (CrvW != NULL)
	PCrvW = CagdCrvCopy(CrvW);

    CagdCrvFree(CrvX);
    CagdCrvFree(CrvY);
    CagdCrvFree(CrvZ);
    CagdCrvFree(CrvW);

    if (!CagdMakeCrvsCompatible(&PCrvX, &PCrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvX, &PCrvZ, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvY, &PCrvZ, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvW, &PCrvX, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvW, &PCrvY, TRUE, TRUE) ||
	!CagdMakeCrvsCompatible(&PCrvW, &PCrvZ, TRUE, TRUE))
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

    CrossProdCrv = SymbCrvMergeScalar(PCrvW, PCrvX, PCrvY, PCrvZ);
    CagdCrvFree(PCrvX);
    CagdCrvFree(PCrvY);
    CagdCrvFree(PCrvZ);
    CagdCrvFree(PCrvW);

    return CrossProdCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two curves - multiply them using the quotient product rule:	     M
*  X = X1 W2 +/- X2 W1							     V
*   All provided curves are assumed to be non rational scalar curves.	     M
*   Returned is a non rational scalar curve (CAGD_PT_E1_TYPE).		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1X:     Numerator of first curve.                                     M
*   Crv1W:     Denominator of first curve.  Can be NULL.                     M
*   Crv2X:     Numerator of second curve.                                    M
*   Crv2W:     Denominator of second curve.  Can be NULL.                    M
*   OperationAdd:   TRUE for addition, FALSE for subtraction.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The result of  Crv1X Crv2W +/- Crv2X Crv1W.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDotProd, SymbCrvVecDotProd, SymbCrvMult, SymbCrvMultScalar        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvRtnlMult, product, symbolic computation                           M
*****************************************************************************/
CagdCrvStruct *SymbCrvRtnlMult(const CagdCrvStruct *Crv1X,
			       const CagdCrvStruct *Crv1W,
			       const CagdCrvStruct *Crv2X,
			       const CagdCrvStruct *Crv2W,
			       CagdBType OperationAdd)
{
    CagdCrvStruct *CTmp1, *CTmp2, *CTmp3;

    if (Crv1X == NULL || Crv2X == NULL)
	return NULL;

    CTmp1 = Crv2W == NULL ? CagdCrvCopy(Crv1X) : SymbCrvMult(Crv1X, Crv2W);
    CTmp2 = Crv1W == NULL ? CagdCrvCopy(Crv2X) : SymbCrvMult(Crv2X, Crv1W);

    if (!CagdMakeCrvsCompatible(&CTmp1, &CTmp2, FALSE, FALSE))
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

    CTmp3 = SymbCrvAddSubAux(CTmp1, CTmp2, OperationAdd);
    CagdCrvFree(CTmp1);
    CagdCrvFree(CTmp2);

    return CTmp3;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Given two curve - add or subtract them as prescribed by OperationAdd,	     *
* coordinatewise.			      				     *
*   The two curves are promoted to same type, point type, and order before   *
* addition can take place.						     *
*   Returned is a curve representing their sum or difference.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   CCrv1, CCrv2:  Two curve to subtract coordinatewise.                     *
*   OperationAdd:  TRUE of addition, FALSE for subtraction.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The summation or difference of Crv1 and Crv2.         *
*****************************************************************************/
static CagdCrvStruct *SymbCrvAddSubAux(const CagdCrvStruct *CCrv1,
				       const CagdCrvStruct *CCrv2,
				       CagdBType OperationAdd)
{
    CagdBType
        SameWeights = FALSE,
	Crv1Rational = CAGD_IS_RATIONAL_CRV(CCrv1),
	Crv2Rational = CAGD_IS_RATIONAL_CRV(CCrv2),
	NoneRational = !Crv1Rational && !Crv2Rational,
	BothRational = Crv1Rational && Crv2Rational;
    int i;
    CagdCrvStruct *SumCrv, *Crv1, *Crv2;
    CagdRType **Points1, **Points2;

    /* Make the two curves have the same point type and curve type. */
    Crv1 = CagdCrvCopy(CCrv1);
    Crv2 = CagdCrvCopy(CCrv2);
    if ((Crv1 -> Periodic ^ Crv2 -> Periodic) ||
	!CagdMakeCrvsCompatible(&Crv1, &Crv2, NoneRational, NoneRational))
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

    Points1 = Crv1 -> Points;
    Points2 = Crv2 -> Points;

    /* Check if both are rational with identical weights. */
    if (BothRational &&
	Crv1 -> Order == Crv2 -> Order &&
	(Crv1 -> GType != CAGD_CBSPLINE_TYPE ||
	 (Crv1 -> Length == Crv2 -> Length &&
	  BspKnotVectorsSame(Crv1 -> KnotVector, Crv2 -> KnotVector,
			     Crv1 -> Order + Crv1 -> Length, IRIT_EPS)))) {
	/* Maybe the weights are identical, in which we can still add the   */
	/* the respective control polygons.				    */
	for (i = 0; i < Crv1 -> Length; i++)
	    if (!IRIT_APX_EQ(Points1[0][i], Points2[0][i]))
		break;
	if (i >= Crv1 -> Length)
	    SameWeights = TRUE;
    }

    if (NoneRational || SameWeights) {
        if (!CagdMakeCrvsCompatible(&Crv1, &Crv2, TRUE, TRUE))
	    SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

        SumCrv = CagdPeriodicCrvNew(Crv1 -> GType, Crv1 -> PType,
				    Crv1 -> Length, Crv1 -> Periodic);
	SumCrv -> Order = Crv1 -> Order;
	if (CAGD_IS_BSPLINE_CRV(SumCrv)) {
	    SumCrv -> KnotVector = BspKnotCopy(NULL, Crv1 -> KnotVector,
					       Crv1 -> Order +
					           CAGD_CRV_PT_LST_LEN(Crv1));
	}

	/* Simply add the respective control polygons. */
	SymbMeshAddSub(SumCrv -> Points, Crv1 -> Points, Crv2 -> Points,
		       SumCrv -> PType, SumCrv -> Length, OperationAdd);
    }
    else {
	CagdCrvStruct *Crv1W, *Crv1X, *Crv1Y, *Crv1Z, *Crv2W, *Crv2X,
		*Crv2Y, *Crv2Z, *SumCrvW, *SumCrvX, *SumCrvY, *SumCrvZ;

	/* Weights are different. Must use the addition of rationals         */
	/* rule ( we invoke SymbCrvMult here):			             */
	/*							 	     */
	/*  x1     x2   x1 w2 +/- x2 w1				             */
	/*  -- +/- -- = ---------------				             */
	/*  w1     w2        w1 w2					     */
	/*								     */
	SymbCrvSplitScalar(Crv1, &Crv1W, &Crv1X, &Crv1Y, &Crv1Z);
	SymbCrvSplitScalar(Crv2, &Crv2W, &Crv2X, &Crv2Y, &Crv2Z);

	SumCrvW = SymbCrvMult(Crv1W, Crv2W);
	SumCrvX = SymbCrvRtnlMult(Crv1X, Crv1W, Crv2X, Crv2W, OperationAdd);
	SumCrvY = SymbCrvRtnlMult(Crv1Y, Crv1W, Crv2Y, Crv2W, OperationAdd);
	SumCrvZ = SymbCrvRtnlMult(Crv1Z, Crv1W, Crv2Z, Crv2W, OperationAdd);
	CagdCrvFree(Crv1W);
	CagdCrvFree(Crv1X);
	CagdCrvFree(Crv1Y);
	CagdCrvFree(Crv1Z);
	CagdCrvFree(Crv2W);
	CagdCrvFree(Crv2X);
	CagdCrvFree(Crv2Y);
	CagdCrvFree(Crv2Z);

	if (!CagdMakeCrvsCompatible(&SumCrvW, &SumCrvX, TRUE, TRUE) ||
	    (SumCrvY != NULL &&
	     !CagdMakeCrvsCompatible(&SumCrvW, &SumCrvY, TRUE, TRUE)) ||
	    (SumCrvZ != NULL &&
	     !CagdMakeCrvsCompatible(&SumCrvW, &SumCrvZ, TRUE, TRUE)))
	    SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);

	SumCrv = SymbCrvMergeScalar(SumCrvW, SumCrvX, SumCrvY, SumCrvZ);
	CagdCrvFree(SumCrvW);
	CagdCrvFree(SumCrvX);
	CagdCrvFree(SumCrvY);
	CagdCrvFree(SumCrvZ);
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    return SumCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a planar curve, compute its enclosed area field curve.		     M
*   This has little meaning unless Crv is closed, in which by evaluation     M
* the resulting area field curve at the end points, the area enclosed by     M
* Crv can be computed.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to compute area field curve for.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The area field curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvEnclosedAreaEval                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvEnclosedArea, area, symbolic computation                          M
*****************************************************************************/
CagdCrvStruct *SymbCrvEnclosedArea(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *CrvX, *CrvY, *CrvZ, *CrvW, *DCrvX, *DCrvY,
	*CTmp1, *CTmp2, *CTmp3;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    return NULL;
    }

    /* Using Green's theorem, if C(t1) = C(t2) the area enclosed is equal to */
    /*									     */
    /*	   t2								     */
    /*	    /								     */
    /*	1  |								     */
    /*  -  | -x'(t) y(t) + x(t) y'(t) dt				     */
    /*	2  |								     */
    /*    /								     */
    /*    t1								     */
    /*									     */
    SymbCrvSplitScalar(Crv, &CrvW, &CrvX, &CrvY, &CrvZ);
    if (CrvZ)
        CagdCrvFree(CrvZ);
    if (CrvW) {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	CagdCrvFree(CrvW);
    }

    DCrvX = CagdCrvDerive(CrvX);
    DCrvY = CagdCrvDerive(CrvY);
    CTmp1 = SymbCrvMult(CrvX, DCrvY);
    CTmp2 = SymbCrvMult(DCrvX, CrvY);
    CagdCrvFree(CrvX);
    CagdCrvFree(CrvY);
    CagdCrvFree(DCrvX);
    CagdCrvFree(DCrvY);
    CTmp3 = SymbCrvSub(CTmp1, CTmp2);
    CagdCrvFree(CTmp1);
    CagdCrvFree(CTmp2);
    CTmp1 = CagdCrvIntegrate(CTmp3);
    CagdCrvFree(CTmp3);

    /* Scale by a half. */
    CagdCrvTransform(CTmp1, NULL, 0.5);

    return CTmp1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a planar curve, compute its enclosed area.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to compute area for.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The area.	  		                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvEnclosedArea                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvEnclosedAreaEval, area, symbolic computation                      M
*****************************************************************************/
CagdRType SymbCrvEnclosedAreaEval(const CagdCrvStruct *Crv)
{
    CagdRType A;

    if (Crv -> Order == 2) {
	/* Convert a polygon and compute. */
	CagdPolylineStruct
	    *CagdPl = CagdCnvrtLinBspCrv2Polyline(Crv);
	IPPolygonStruct
	    *Pl = IPCagdPllns2IritPllns(CagdPl);	  /* And free CagdPl. */

	IPUpdatePolyPlane(Pl);

	A = GMPolyOnePolyArea(Pl);

	IPFreePolygon(Pl);
    }
    else if (CAGD_IS_RATIONAL_CRV(Crv)) { /* Approximate. */
        CagdPolylineStruct *CagdPoly;
	IPPolygonStruct *Pl;
	CagdBBoxStruct BBox;
	CagdRType
	    Tol = SYMB_CRV_AREA_APPROX_TOL;

	CagdCrvBBox(Crv, &BBox);
	Tol *= IRIT_MAX(BBox.Max[0] - BBox.Min[0], BBox.Max[1] - BBox.Min[1]);

	CagdPoly = SymbCrv2Polyline(Crv, Tol,
				    SYMB_CRV_APPROX_TOLERANCE, TRUE);
	Pl = IPCagdPllns2IritPllns(CagdPoly);           /* And Free CagdPoly. */
	IPUpdatePolyPlane(Pl);

	A = GMPolyOnePolyArea(Pl);

	IPFreePolygon(Pl);
    }
    else {
        CagdRType TMin, TMax, *R;
	CagdCrvStruct
	    *Area = SymbCrvEnclosedArea(Crv);

	CagdCrvDomain(Crv, &TMin, &TMax);
	
	R = CagdCrvEval(Area, TMax);
	/* Make it positive for CW curves in XY plane, like closed polys. */
	A = CAGD_IS_RATIONAL_CRV(Area) ? -R[1] / R[0] : -R[1];

	CagdCrvFree(Area);
    }

    return A;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two control polygons/meshes - add them coordinate wise.		     M
* If mesh is rational, weights are assumed identical and are just copied.    M
*                                                                            *
* PARAMETERS:                                                                M
*   DestPoints:    Where addition or difference result should go to.         M
*   Points1:       First control polygon/mesh.                               M
*   Points2:       Second control polygon/mesh.                              M
*   PType:         Type of points we are dealing with.                       M
*   Size:          Length of each vector in Points1/2.                       M
*   OperationAdd:  TRUE of addition, FALSE for subtraction.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSub, SymbSrfAdd                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbMeshAddSub, addition, subtraction, symbolic computation              M
*****************************************************************************/
void SymbMeshAddSub(CagdRType **DestPoints,
		    CagdRType * const *Points1,
		    CagdRType * const *Points2,
		    CagdPointType PType,
		    int Size,
		    CagdBType OperationAdd)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i, j,
	NumCoords = CAGD_NUM_OF_PT_COORD(PType);

    for (i = 1; i <= NumCoords; i++) {
	CagdRType
	    *DPts = DestPoints[i];
	const CagdRType
	    *Pts1 = Points1[i],
	    *Pts2 = Points2[i];

	if (OperationAdd)
	    for (j = 0; j < Size; j++)
	        *DPts++ = *Pts1++ + *Pts2++;
	else
	    for (j = 0; j < Size; j++)
	        *DPts++ = *Pts1++ - *Pts2++;
    }

    if (IsRational) {     /* Copy the weights (should be identical in both). */
	CagdRType
	    *DPts = DestPoints[0];
	const CagdRType
	    *Pts1 = Points1[0],
	    *Pts2 = Points2[0];

	for (j = 0; j < Size; j++) {
	    if (!IRIT_APX_EQ(*Pts1, *Pts2))
		SYMB_FATAL_ERROR(SYMB_ERR_W_NOT_SAME);
	    *DPts++ = *Pts1++;
	    Pts2++;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve, splits it to its scalar component curves.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      Curve to split.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct **: A vector of scalar curves - components of Crv.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSplitScalar, SymbCrvMergeScalarN                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvSplitScalarN, split, symbolic computation                         M
*****************************************************************************/
CagdCrvStruct **SymbCrvSplitScalarN(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Length = Crv -> Length,
	NumCoords = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct
	**Crvs = (CagdCrvStruct **) IritMalloc(sizeof(CagdCrvStruct *)
							  * CAGD_MAX_PT_SIZE);

    IRIT_ZAP_MEM(Crvs, sizeof(CagdCrvStruct *) * CAGD_MAX_PT_SIZE);

    for (i = IsNotRational; i <= NumCoords; i++) {
	Crvs[i] = CagdPeriodicCrvNew(Crv -> GType, CAGD_PT_E1_TYPE, Length,
				     Crv -> Periodic);
	Crvs[i] -> Order = Crv -> Order;
	if (Crv -> KnotVector != NULL)
	    Crvs[i] -> KnotVector = BspKnotCopy(NULL, Crv -> KnotVector,
						Crv -> Order +
						    CAGD_CRV_PT_LST_LEN(Crv));

	SYMB_GEN_COPY(Crvs[i] -> Points[1], Crv -> Points[i],
		      sizeof(CagdRType) * Length);
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve splits it to its scalar component curves. Ignores all	     M
* dimensions beyond the third, Z, dimension.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      Curve to split.                                                M
*   CrvW:     The weight component of Crv, if have any.                      M
*   CrvX:     The X component of Crv.                                        M
*   CrvY:     The Y component of Crv, if have any.                           M
*   CrvZ:     The Z component of Crv, if have any.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSplitScalar, SymbCrvMergeScalar                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvSplitScalar, split, symbolic computation                          M
*****************************************************************************/
void SymbCrvSplitScalar(const CagdCrvStruct *Crv,
			CagdCrvStruct **CrvW,
			CagdCrvStruct **CrvX,
			CagdCrvStruct **CrvY,
			CagdCrvStruct **CrvZ)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Length = Crv -> Length,
	NumCoords = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct
	*Crvs[CAGD_MAX_PT_SIZE];

    for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
	Crvs[i] = NULL;

    for (i = IsNotRational; i <= NumCoords; i++) {
	Crvs[i] = CagdPeriodicCrvNew(Crv -> GType, CAGD_PT_E1_TYPE, Length,
				     Crv -> Periodic);
	Crvs[i] -> Order = Crv -> Order;
	if (Crv -> KnotVector != NULL)
	    Crvs[i] -> KnotVector = BspKnotCopy(NULL, Crv -> KnotVector,
						Crv -> Order +
						    CAGD_CRV_PT_LST_LEN(Crv));

	SYMB_GEN_COPY(Crvs[i] -> Points[1], Crv -> Points[i],
		      sizeof(CagdRType) * Length);
    }

    *CrvW = Crvs[0];
    *CrvX = Crvs[1];
    *CrvY = Crvs[2];
    *CrvZ = Crvs[3];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a vector of scalar curves, treat them as coordinates into a new    M
* vector curve.							     M
*   Assumes at least CrvVec is not NULL in which a scalar curve is returned. M
*   Assumes CrvVec[i]/CrvW are either E1 or P1 in which the weights are      M
* assumed to be identical and can be ignored if CrvW exists and copied.        M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvW:     The weight component of new constructed curve, if exist,     M
*	      NULL if none.						M
*   CrvVec:   A vector of scalar curves.			            M
*   NumCrvs:  Number of curves in CrvVec.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A new curve constructed from given scalar curves.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfMergeScalar, SymbCrvSplitScalarN                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMergeScalarN, merge, symbolic computation                         M
*****************************************************************************/
CagdCrvStruct *SymbCrvMergeScalarN(const CagdCrvStruct *CrvW, 
				   const CagdCrvStruct **CrvVec, 
				   int NumCrvs)
{
    CagdBType
	WeightCopied = FALSE,
	IsRational = CrvW != NULL;
    int i, Length,
	NumCoords = NumCrvs;
    CagdPointType
	PType = CAGD_MAKE_PT_TYPE(IsRational, NumCoords);
    CagdCrvStruct *Crvs[CAGD_MAX_PT_SIZE], *Crv;

    Crvs[0] = CrvW == NULL ? NULL : CagdCrvCopy(CrvW);
    for (i = 1; i < NumCoords + 1; i++)
	Crvs[i] = CagdCrvCopy(CrvVec[i - 1]);

    for (i = 0; i < NumCoords + 1; i++) {
	int j;

	for (j = i + 1; j < NumCoords + 1; j++)
	    if (Crvs[i] != NULL && Crvs[j] != NULL)
	        CagdMakeCrvsCompatible(&Crvs[i], &Crvs[j], TRUE, TRUE);
    }

    Length = Crvs[1] -> Length;
    Crv = CagdPeriodicCrvNew(Crvs[1] -> GType, PType, Length, Crvs[1] -> Periodic);

    Crv -> Order = Crvs[1] -> Order;
    if (Crvs[1] -> KnotVector != NULL)
	Crv -> KnotVector = BspKnotCopy(NULL, Crvs[1] -> KnotVector,
					Crvs[1] -> Order +
					    CAGD_CRV_PT_LST_LEN(Crvs[1]));

    for (i = !IsRational; i < NumCoords + 1; i++) {
	if (Crvs[i] != NULL) {
	    if (Crvs[i] -> PType != CAGD_PT_E1_TYPE) {
		if (Crvs[i] -> PType != CAGD_PT_P1_TYPE)
		    SYMB_FATAL_ERROR(SYMB_ERR_SCALAR_EXPECTED);
		else if (CrvW == NULL && WeightCopied == FALSE) {
		    SYMB_GEN_COPY(Crv -> Points[0], Crvs[i] -> Points[0],
				  sizeof(CagdRType) * Length);
		    WeightCopied = TRUE;
		}
	    }

	    SYMB_GEN_COPY(Crv -> Points[i], Crvs[i] -> Points[1],
			  sizeof(CagdRType) * Length);
	}
    }

    for (i = 0; i < NumCoords + 1; i++)
	CagdCrvFree(Crvs[i]);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of scalar curves, treat them as coordinates into a new curve.  M
*   Assumes at least CrvX is not NULL in which a scalar curve is returned.   M
*   Assumes CrvX/Y/Z/W are either E1 or P1 in which the weights are assumed  M
* to be identical and can be ignored if CrvW exists or copied otheriwse.     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvW:     The weight component of new constructed curve, if have any.    M
*   CrvX:     The X component of new constructed curve.                      M
*   CrvY:     The Y component of new constructed curve, if have any.         M
*   CrvZ:     The Z component of new constructed curve, if have any.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A new curve constructed from given scalar curves.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfMergeScalar, SymbCrvSplitScalar                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMergeScalar, merge, symbolic computation                          M
*****************************************************************************/
CagdCrvStruct *SymbCrvMergeScalar(const CagdCrvStruct *CrvW,
				  const CagdCrvStruct *CrvX,
				  const CagdCrvStruct *CrvY,
				  const CagdCrvStruct *CrvZ)
{
    CagdBType
	WeightCopied = FALSE,
	IsRational = CrvW != NULL;
    int i, Length,
	NumCoords = (CrvX != NULL) + (CrvY != NULL) + (CrvZ != NULL);
    CagdPointType
	PType = CAGD_MAKE_PT_TYPE(IsRational, NumCoords);
    CagdCrvStruct *Crvs[CAGD_MAX_PT_SIZE], *Crv;

    Crvs[0] = CrvW == NULL ? NULL : CagdCrvCopy(CrvW);
    Crvs[1] = CrvX == NULL ? NULL : CagdCrvCopy(CrvX);
    Crvs[2] = CrvY == NULL ? NULL : CagdCrvCopy(CrvY);
    Crvs[3] = CrvZ == NULL ? NULL : CagdCrvCopy(CrvZ);

    for (i = 0; i < 4; i++) {
	int j;

	for (j = i + 1; j < 4; j++)
	    if (Crvs[i] != NULL && Crvs[j] != NULL)
	        CagdMakeCrvsCompatible(&Crvs[i], &Crvs[j], TRUE, TRUE);
    }
    Length = CrvX -> Length;
    Crv = CagdPeriodicCrvNew(CrvX -> GType, PType, Length, CrvX -> Periodic);

    Crv -> Order = CrvX -> Order;
    if (CrvX -> KnotVector != NULL)
	Crv -> KnotVector = BspKnotCopy(NULL, CrvX -> KnotVector,
					CrvX -> Order +
					    CAGD_CRV_PT_LST_LEN(CrvX));

    for (i = !IsRational; i <= NumCoords; i++) {
	if (Crvs[i] != NULL) {
	    if (Crvs[i] -> PType != CAGD_PT_E1_TYPE) {
		if (Crvs[i] -> PType != CAGD_PT_P1_TYPE)
		    SYMB_FATAL_ERROR(SYMB_ERR_SCALAR_EXPECTED);
		else if (CrvW == NULL && WeightCopied == FALSE) {
		    SYMB_GEN_COPY(Crv -> Points[0], Crvs[i] -> Points[0],
				  sizeof(CagdRType) * Length);
		    WeightCopied = TRUE;
		}
	    }

	    SYMB_GEN_COPY(Crv -> Points[i], Crvs[i] -> Points[1],
			  sizeof(CagdRType) * Length);
	}
    }

    for (i = 0; i < 4; i++)
	CagdCrvFree(Crvs[i]);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Promote a scalar curve to two dimensions by moving the scalar axis to be   M
* the Y axis and adding monotone X axis.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      Scalar curve to promose to a two dimensional one.              M
*   Min:      Minimum of new monotone X axis.                                M
*   Max:      Maximum of new monotone X axis.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A two dimensional curve.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPrmtSclrSrfTo3D                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPrmtSclrCrvTo2D, promotion, conversion, symbolic computation         M
*****************************************************************************/
CagdCrvStruct *SymbPrmtSclrCrvTo2D(const CagdCrvStruct *Crv,
				   CagdRType Min,
				   CagdRType Max)
{
    int i,
	Length = Crv -> Length;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    CagdRType *Points, *WPoints,
	Step = (Max - Min) / (Length - 1);
    CagdCrvStruct
	*PrmtCrv = CagdCoerceCrvTo(Crv, IsRational ? CAGD_PT_P2_TYPE
						   : CAGD_PT_E2_TYPE, FALSE);

    /* Copy X (original scalar data) to Y. */
    IRIT_GEN_COPY(PrmtCrv -> Points[2], PrmtCrv -> Points[1],
	     sizeof(CagdRType) * Length);

    Points = PrmtCrv -> Points[1];
    WPoints = IsRational ? PrmtCrv -> Points[0] : NULL;
    for (i = 0; i < Length; i++)
	*Points++ = (Min + Step * i) * (IsRational ? *WPoints++ : 1.0);

    return PrmtCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a control polygon/mesh, computes the extremum values of them all.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:       To scan for extremum values.				     M
*   Length:       Length of each vector in Points.                           M
*   FindMinimum:  TRUE for minimum, FALSE for maximum.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding PType point with the extremum values of   M
*                 each axis independently.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbExtremumCntPtVals, extremum values, symbolic computation             M
*****************************************************************************/
CagdRType *SymbExtremumCntPtVals(CagdRType * const *Points,
				 int Length,
				 CagdBType FindMinimum)
{
    IRIT_STATIC_DATA CagdRType Extremum[CAGD_MAX_PT_SIZE];
    int i, j;

    for (i = 1; Points[i] != NULL && i <= CAGD_MAX_PT_COORD; i++) {
	const CagdRType
	    *R = Points[0],
	    *P = Points[i];

	Extremum[i] = FindMinimum ? IRIT_INFNTY : -IRIT_INFNTY;

	for (j = 0; j < Length; j++) {
	    CagdRType
		Val = R == NULL ? *P++ : *P++ / *R++;

	    if (FindMinimum) {
		if (Extremum[i] > Val)
		    Extremum[i] = Val;
	    }
	    else {
		if (Extremum[i] < Val)
		    Extremum[i] = Val;
	    }
	}
    }

    return Extremum;
}

