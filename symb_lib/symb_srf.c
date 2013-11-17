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

static CagdSrfStruct *SymbSrfAddSubAux(const CagdSrfStruct *Srf1,
				       const CagdSrfStruct *Srf2,
				       CagdBType OperationAdd);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces - add them coordinatewise.				     M
*   The two surfaces are promoted to same point type before the		     M
* multiplication can take place. Furthermore, order and continuity are	     M
* matched as well.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surface to add up coordinatewise.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The summation of Srf1 + Srf2 coordinatewise.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSub, SymbMeshAddSub, SymbSrfMult                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfAdd, addition, symbolic computation                               M
*****************************************************************************/
CagdSrfStruct *SymbSrfAdd(const CagdSrfStruct *Srf1,
			  const CagdSrfStruct *Srf2)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    if (!IRIT_APX_EQ(UMin1, UMin2) || !IRIT_APX_EQ(UMax1, UMax2) ||
	!IRIT_APX_EQ(VMin1, VMin2) || !IRIT_APX_EQ(VMax1, VMax2)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	return NULL;
    }

    return SymbSrfAddSubAux(Srf1, Srf2, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces - subtract them coordinatewise.			     M
*   The two surfaces are promoted to same point type before the		     M
* multiplication can take place. Furthermore, order and continuity are	     M
* matched as well.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surface to subtract coordinatewise.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The difference of Srf1 - Srf2 coordinatewise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfAdd, SymbMeshAddSub, SymbSrfMult                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSub, subtraction, symbolic computation                            M
*****************************************************************************/
CagdSrfStruct *SymbSrfSub(const CagdSrfStruct *Srf1,
			  const CagdSrfStruct *Srf2)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    if (!IRIT_APX_EQ(UMin1, UMin2) || !IRIT_APX_EQ(UMax1, UMax2) ||
	!IRIT_APX_EQ(VMin1, VMin2) || !IRIT_APX_EQ(VMax1, VMax2)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	return NULL;
    }

    return SymbSrfAddSubAux(Srf1, Srf2, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces - multiply them coordinatewise.			     M
*   The two surfaces are promoted to same point type before the 	     M
* multiplication can take place.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surface to multiply coordinatewise.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The product of Srf1 * Srf2 coordinatewise.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfScalarScale, SymbSrfMultScalar,M
*   SymbSrfInvert							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMult, product, symbolic computation                               M
*****************************************************************************/
CagdSrfStruct *SymbSrfMult(const CagdSrfStruct *Srf1,
			   const CagdSrfStruct *Srf2)
{
    CagdSrfStruct
	*ProdSrf = NULL;

    if (Srf1 -> GType == CAGD_SBEZIER_TYPE &&
	Srf2 -> GType == CAGD_SBEZIER_TYPE)
	ProdSrf = BzrSrfMult(Srf1, Srf2);
    else if ((Srf1 -> GType == CAGD_SBEZIER_TYPE ||
	      Srf1 -> GType == CAGD_SBSPLINE_TYPE) &&
	     (Srf2 -> GType == CAGD_SBEZIER_TYPE ||
	      Srf2 -> GType == CAGD_SBSPLINE_TYPE))
	ProdSrf = BspSrfMult(Srf1, Srf2);
    else
	SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);

    return ProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar surface, returns a scalar surface representing the	     M
* reciprocal values, by making it rational (if was not one) and flipping     M
* the numerator and the denominator.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       A scalar surface to compute a reciprocal value for.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A rational scalar surface that is equal to the        M
*                      reciprocal value of Srf.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfScalarScale, SymbSrfMultScalar,M
*   SymbSrfMult, SymbSrfCrossProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfInvert, division, symbolic computation, reciprocal value          M
*****************************************************************************/
CagdSrfStruct *SymbSrfInvert(const CagdSrfStruct *Srf)
{
    int i;
    CagdRType *R;
    CagdSrfStruct
	*NewSrf = CagdPeriodicSrfNew(Srf -> GType, CAGD_PT_P1_TYPE,
				     Srf -> ULength, Srf -> VLength,
				     Srf -> UPeriodic, Srf -> VPeriodic);
    NewSrf -> UOrder = Srf -> UOrder;
    NewSrf -> VOrder = Srf -> VOrder;

    switch (Srf -> PType) {
	case CAGD_PT_E1_TYPE:
	    CAGD_GEN_COPY(NewSrf -> Points[0], Srf -> Points[1],
			  sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);
	    for (i = 0, R = NewSrf -> Points[1];
		 i < Srf -> ULength * Srf -> VLength;
		 i++)
		*R++ = 1.0;
	    break;
	case CAGD_PT_P1_TYPE:
	    CAGD_GEN_COPY(NewSrf -> Points[0], Srf -> Points[1],
			  sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);
	    CAGD_GEN_COPY(NewSrf -> Points[1], Srf -> Points[0],
			  sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNSUPPORT_PT);
	    break;
    }

    if (CAGD_IS_BSPLINE_SRF(Srf)) {
        NewSrf -> UKnotVector = BspKnotCopy(NULL, Srf -> UKnotVector,
				   Srf -> ULength + Srf -> UOrder +
				   (Srf -> UPeriodic ? Srf -> UOrder - 1 : 0));
	NewSrf -> VKnotVector = BspKnotCopy(NULL, Srf -> VKnotVector,
				   Srf -> VLength + Srf -> VOrder +
				   (Srf -> VPeriodic ? Srf -> VOrder - 1 : 0));
    }

    CAGD_PROPAGATE_ATTR(NewSrf, Srf);

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface, scale it by Scale.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      A surface to scale by magnitude Scale.                         M
*   Scale:    Scaling factor.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A surfaces scaled by Scale compared to Srf.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfMult, SymbSrfMultScalar,       M
*   SymbSrfInvert, SymbSrfCrossProd                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfScalarScale, scaling, symbolic computation                        M
*****************************************************************************/
CagdSrfStruct *SymbSrfScalarScale(const CagdSrfStruct *Srf, CagdRType Scale)
{
    int i;
    CagdRType *R;
    CagdSrfStruct
	*TSrf = CagdSrfCopy(Srf);

    switch (TSrf -> PType) {
	case CAGD_PT_P1_TYPE:
	case CAGD_PT_E1_TYPE:
	    for (i = 0, R = TSrf -> Points[1];
		 i < TSrf -> ULength * TSrf -> VLength;
		 i++)
		*R++ *= Scale;
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNSUPPORT_PT);
	    break;
    }

    return TSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surface - a vector curve Srf1 and a scalar curve Srf2, multiply  M
* all Srf1's coordinates by the scalar curve Srf2.			     M
*   Returned surface is a surface representing the product of the two given  M
* surfaces.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surfaces to multiply.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A surface representing the product of Srf1 and Srf2.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfMult, SymbSrfCrossProd,        M
*   SymbCrvMultScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMultScalar, product, symbolic computation  		             M
*****************************************************************************/
CagdSrfStruct *SymbSrfMultScalar(const CagdSrfStruct *Srf1,
				 const CagdSrfStruct *Srf2)
{
    CagdSrfStruct *PSrf1W, *PSrf1X, *PSrf1Y, *PSrf1Z,
		  *PSrf2W, *PSrf2X, *PSrf2Y, *PSrf2Z,
		  *TSrf, *ProdSrf;

    SymbSrfSplitScalar(Srf1, &PSrf1W, &PSrf1X, &PSrf1Y, &PSrf1Z);
    SymbSrfSplitScalar(Srf2, &PSrf2W, &PSrf2X, &PSrf2Y, &PSrf2Z);

    TSrf = SymbSrfMult(PSrf1X, PSrf2X);
    CagdSrfFree(PSrf1X);
    PSrf1X = TSrf;

    if (PSrf1Y != NULL) {
	TSrf = SymbSrfMult(PSrf1Y, PSrf2X);
	CagdSrfFree(PSrf1Y);
	PSrf1Y = TSrf;
    }

    if (PSrf1Z != NULL) {
	TSrf = SymbSrfMult(PSrf1Z, PSrf2X);
	CagdSrfFree(PSrf1Z);
	PSrf1Z = TSrf;
    }

    if (PSrf1W != NULL && PSrf2W != NULL) {
	TSrf = SymbSrfMult(PSrf1W, PSrf2W);
	CagdSrfFree(PSrf1W);
	PSrf1W = TSrf;
    }
    else if (PSrf2W != NULL) {
	PSrf1W = PSrf2W;
	PSrf2W = NULL;
    }

    ProdSrf = SymbSrfMergeScalar(PSrf1W, PSrf1X, PSrf1Y, PSrf1Z);

    CagdSrfFree(PSrf1W);
    CagdSrfFree(PSrf1X);
    CagdSrfFree(PSrf1Y);
    CagdSrfFree(PSrf1Z);
    CagdSrfFree(PSrf2W);
    CagdSrfFree(PSrf2X);
    CagdSrfFree(PSrf2Y);
    CagdSrfFree(PSrf2Z);

    return ProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces - computes their dot product.			     M
*   Returned surface is a scalar surface representing the dot product of the M
* two given surfaces.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surface to multiply and compute a dot product for.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A scalar surface representing the dot product of      M
*                      Srf1 . Srf2.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfMult, SymbSrfVecDotProd, SymbSrfScalarScale, SymbSrfMultScalar,   M
*   SymbSrfInvert, SymbSrfCrossProd, SymbSrfVecCrossProd                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfDotProd, product, dot product, symbolic computation               M
*****************************************************************************/
CagdSrfStruct *SymbSrfDotProd(const CagdSrfStruct *Srf1,
			      const CagdSrfStruct *Srf2)
{
    CagdSrfStruct *PSrfW, *PSrfX, *PSrfY, *PSrfZ, *TSrf1, *TSrf2, *DotProdSrf,
	*ProdSrf = SymbSrfMult(Srf1, Srf2);

    SymbSrfSplitScalar(ProdSrf, &PSrfW, &PSrfX, &PSrfY, &PSrfZ);
    CagdSrfFree(ProdSrf);

    if (PSrfY != NULL) {
	TSrf1 = SymbSrfAdd(PSrfX, PSrfY);
	CagdSrfFree(PSrfX);
	CagdSrfFree(PSrfY);
    }
    else
	TSrf1 = PSrfX;

    if (PSrfZ != NULL) {
	TSrf2 = SymbSrfAdd(TSrf1, PSrfZ);
	CagdSrfFree(TSrf1);
	CagdSrfFree(PSrfZ);
	TSrf1 = TSrf2;
    }

    DotProdSrf = SymbSrfMergeScalar(PSrfW, TSrf1, NULL, NULL);
    CagdSrfFree(PSrfW);
    CagdSrfFree(TSrf1);

    return DotProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and a vector - computes their dot product.                 M
*   Returned surface is a scalar surface representing the dot product.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:  Surface to multiply and compute a dot product for.                 M
*   Vec:  Vector to project Srf onto.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A scalar surface representing the dot product of      M
*                      Srf . Vec.                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfMult, SymbSrfScalarScale, SymbSrfMultScalar,      M
*   SymbSrfInvert, SymbSrfCrossProd, SymbSrfVecCrossProd                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfVecDotProd, product, dot product, symbolic computation            M
*****************************************************************************/
CagdSrfStruct *SymbSrfVecDotProd(const CagdSrfStruct *Srf,
				 const CagdVType Vec)
{
    CagdSrfStruct *PSrfW, *PSrfX, *PSrfY, *PSrfZ, *TSrf, *DotProdSrf;

    SymbSrfSplitScalar(Srf, &PSrfW, &PSrfX, &PSrfY, &PSrfZ);

    TSrf = SymbSrfScalarScale(PSrfX, Vec[0]);
    CagdSrfFree(PSrfX);
    PSrfX = TSrf;

    if (PSrfY != NULL) {
	TSrf = SymbSrfScalarScale(PSrfY, Vec[1]);
	CagdSrfFree(PSrfY);
	PSrfY = TSrf;

	TSrf = SymbSrfAdd(PSrfX, PSrfY);
        CagdSrfFree(PSrfX);
        CagdSrfFree(PSrfY);

	PSrfX = TSrf;
    }

    if (PSrfZ != NULL) {
	TSrf = SymbSrfScalarScale(PSrfZ, Vec[2]);
	CagdSrfFree(PSrfZ);
	PSrfZ = TSrf;

	TSrf = SymbSrfAdd(PSrfX, PSrfZ);
        CagdSrfFree(PSrfX);
        CagdSrfFree(PSrfZ);

	PSrfX = TSrf;
    }

    DotProdSrf = SymbSrfMergeScalar(PSrfW, PSrfX, NULL, NULL);
    CagdSrfFree(PSrfW);
    CagdSrfFree(PSrfX);

    return DotProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces - computes their cross product.			     M
*   Returned surface is a scalar surface representing the cross product of   M
* the two given surfaces.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surface to multiply and compute a cross product for.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A vectir surface representing the cross product of    M
*                      Srf1 x Srf2.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfScalarScale, SymbSrfMultScalar,M
*   SymbSrfInvert, SymbSrfVecCrossProd                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfCrossProd, product, cross product, symbolic computation           M
*****************************************************************************/
CagdSrfStruct *SymbSrfCrossProd(const CagdSrfStruct *Srf1,
				const CagdSrfStruct *Srf2)
{
    CagdSrfStruct *Srf1W, *Srf1X, *Srf1Y, *Srf1Z,
		  *Srf2W, *Srf2X, *Srf2Y, *Srf2Z,
		  *TSrf1, *TSrf2, *CrossProdSrf,
	*PSrfW = NULL,
	*PSrfX = NULL,
	*PSrfY = NULL,
	*PSrfZ = NULL;

    SymbSrfSplitScalar(Srf1, &Srf1W, &Srf1X, &Srf1Y, &Srf1Z);
    SymbSrfSplitScalar(Srf2, &Srf2W, &Srf2X, &Srf2Y, &Srf2Z);

    if (Srf1X == NULL || Srf1Y == NULL || Srf2X == NULL || Srf2Y == NULL)
	SYMB_FATAL_ERROR(SYMB_ERR_NO_CROSS_PROD);

    /* Cross product X axis. */
    TSrf1 = Srf2Z ? SymbSrfMult(Srf1Y, Srf2Z) : NULL;
    TSrf2 = Srf1Z ? SymbSrfMult(Srf2Y, Srf1Z) : NULL;

    if (TSrf1) {
	if (TSrf2) {
	    PSrfX = SymbSrfSub(TSrf1, TSrf2);
	    CagdSrfFree(TSrf2);
	}
	CagdSrfFree(TSrf1);
    }

    /* Cross product Y axis. */
    TSrf1 = Srf1Z ? SymbSrfMult(Srf1Z, Srf2X) : NULL;
    TSrf2 = Srf2Z ? SymbSrfMult(Srf2Z, Srf1X) : NULL;
    if (TSrf1) {
	if (TSrf2) {
	    PSrfY = SymbSrfSub(TSrf1, TSrf2);
	    CagdSrfFree(TSrf2);
	}
	CagdSrfFree(TSrf1);
    }

    /* Cross product Z axis. */
    TSrf1 = SymbSrfMult(Srf1X, Srf2Y);
    TSrf2 = SymbSrfMult(Srf2X, Srf1Y);
    PSrfZ = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    /* Cross product W axis. */
    if (Srf1W || Srf2W) {
	if (Srf1W == NULL)
	    PSrfW = CagdSrfCopy(Srf2W);
	else if (Srf2W == NULL)
	    PSrfW = CagdSrfCopy(Srf1W);
	else
	    PSrfW = SymbSrfMult(Srf1W, Srf2W);
    }

    CagdSrfFree(Srf1X);
    CagdSrfFree(Srf1Y);
    CagdSrfFree(Srf1Z);
    CagdSrfFree(Srf1W);

    CagdSrfFree(Srf2X);
    CagdSrfFree(Srf2Y);
    CagdSrfFree(Srf2Z);
    CagdSrfFree(Srf2W);

    if (PSrfX == NULL && PSrfZ != NULL) {    /* We had planar 2D surface(s). */
        PSrfX = CagdSrfCopy(PSrfZ);
	IRIT_ZAP_MEM(PSrfX -> Points[1],
		PSrfX -> ULength * PSrfX -> VLength * sizeof(CagdRType));
    }

    if (PSrfY == NULL && PSrfZ != NULL) {    /* We had planar 2D surface(s). */
        PSrfY = CagdSrfCopy(PSrfZ);
	IRIT_ZAP_MEM(PSrfY -> Points[1],
		PSrfY -> ULength * PSrfY -> VLength * sizeof(CagdRType));
    }

    if (!CagdMakeSrfsCompatible(&PSrfW, &PSrfX, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&PSrfW, &PSrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&PSrfW, &PSrfZ, TRUE, TRUE, TRUE, TRUE))
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

    CrossProdSrf = SymbSrfMergeScalar(PSrfW, PSrfX, PSrfY, PSrfZ);
    CagdSrfFree(PSrfX);
    CagdSrfFree(PSrfY);
    CagdSrfFree(PSrfZ);
    CagdSrfFree(PSrfW);

    return CrossProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and a vector - computes their cross product.		     M
*   Returned surface is a scalar surface representing the cross product of   M
* the surface and vector.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:  Surface to multiply and compute a cross product for.               M
*   Vec:  Vector to cross product Srf with.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A vector surface representing the cross product of    M
*                      Srf x Vec.                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfScalarScale, SymbSrfMultScalar,M
*   SymbSrfInvert, SymbSrfCrossProd                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfVecCrossProd, product, cross product, symbolic computation        M
*****************************************************************************/
CagdSrfStruct *SymbSrfVecCrossProd(const CagdSrfStruct *Srf,
				   const CagdVType Vec)
{
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ,
		  *TSrf1, *TSrf2, *CrossProdSrf,
	*PSrfW = NULL,
	*PSrfX = NULL,
	*PSrfY = NULL,
	*PSrfZ = NULL;

    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);

    if (SrfX == NULL || SrfY == NULL)
	SYMB_FATAL_ERROR(SYMB_ERR_NO_CROSS_PROD);

    /* Cross product X axis. */
    TSrf1 = CagdSrfCopy(SrfY);
    CagdSrfTransform(TSrf1, NULL, Vec[2]);
    if (SrfZ != NULL) {
	TSrf2 = CagdSrfCopy(SrfZ);
	CagdSrfTransform(TSrf2, NULL, Vec[1]);

        PSrfX = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf2);
	CagdSrfFree(TSrf1);
    }
    else
        PSrfX = TSrf1;

    /* Cross product Y axis. */
    TSrf2 = CagdSrfCopy(SrfX);
    CagdSrfTransform(TSrf2, NULL, Vec[2]);
    if (SrfZ != NULL) {
	TSrf1 = CagdSrfCopy(SrfZ);
	CagdSrfTransform(TSrf1, NULL, Vec[0]);

	PSrfY = SymbSrfSub(TSrf1, TSrf2);
	CagdSrfFree(TSrf1);
	CagdSrfFree(TSrf2);
    }
    else
        PSrfX = TSrf2;

    /* Cross product Z axis. */
    TSrf1 = CagdSrfCopy(SrfX);
    CagdSrfTransform(TSrf1, NULL, Vec[1]);
    TSrf2 = CagdSrfCopy(SrfY);
    CagdSrfTransform(TSrf2, NULL, Vec[0]);
    PSrfZ = SymbSrfSub(TSrf1, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    /* Cross product W axis. */
    if (SrfW != NULL)
	PSrfW = CagdSrfCopy(SrfW);

    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(SrfZ);
    CagdSrfFree(SrfW);

    if (PSrfX == NULL && PSrfZ != NULL) {    /* We had planar 2D surface(s). */
        PSrfX = CagdSrfCopy(PSrfZ);
	IRIT_ZAP_MEM(PSrfX -> Points[1],
		PSrfX -> ULength * PSrfX -> VLength * sizeof(CagdRType));
    }

    if (PSrfY == NULL && PSrfZ != NULL) {    /* We had planar 2D surface(s). */
        PSrfY = CagdSrfCopy(PSrfZ);
	IRIT_ZAP_MEM(PSrfY -> Points[1],
		PSrfY -> ULength * PSrfY -> VLength * sizeof(CagdRType));
    }

    if (!CagdMakeSrfsCompatible(&PSrfW, &PSrfX, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&PSrfW, &PSrfY, TRUE, TRUE, TRUE, TRUE) ||
	!CagdMakeSrfsCompatible(&PSrfW, &PSrfZ, TRUE, TRUE, TRUE, TRUE))
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

    CrossProdSrf = SymbSrfMergeScalar(PSrfW, PSrfX, PSrfY, PSrfZ);
    CagdSrfFree(PSrfX);
    CagdSrfFree(PSrfY);
    CagdSrfFree(PSrfZ);
    CagdSrfFree(PSrfW);

    return CrossProdSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two surfaces - multiply them using the quotient product rule:	     M
*  X = X1 W2 +/- X2 W1							     V
*   All provided surfaces are assumed to be non rational scalar surfaces.    M
*   Returned is a non rational scalar surface (CAGD_PT_E1_TYPE).	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1X:     Numerator of first surface.                                   M
*   Srf1W:     Denominator of first surface.  Can be NULL.                   M
*   Srf2X:     Numerator of second surface.                                  M
*   Srf2W:     Denominator of second surface.  Can be NULL.                  M
*   OperationAdd:   TRUE for addition, FALSE for subtraction.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The result of  Srf1X Srf2W +/- Srf2X Srf1W.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDotProd, SymbSrfVecDotProd, SymbSrfScalarScale, SymbSrfMultScalar,M
*   SymbSrfInvert                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfRtnlMult, product, symbolic computation                           M
*****************************************************************************/
CagdSrfStruct *SymbSrfRtnlMult(const CagdSrfStruct *Srf1X,
			       const CagdSrfStruct *Srf1W,
			       const CagdSrfStruct *Srf2X,
			       const CagdSrfStruct *Srf2W,
			       CagdBType OperationAdd)
{
    CagdSrfStruct *STmp1, *STmp2, *STmp3;

    if (Srf1X == NULL || Srf2X == NULL)
	return NULL;

    STmp1 = Srf2W == NULL ? CagdSrfCopy(Srf1X) : SymbSrfMult(Srf1X, Srf2W);
    STmp2 = Srf1W == NULL ? CagdSrfCopy(Srf2X) : SymbSrfMult(Srf2X, Srf1W);

    if (!CagdMakeSrfsCompatible(&STmp1, &STmp2, FALSE, FALSE, FALSE, FALSE))
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

    STmp3 = SymbSrfAddSubAux(STmp1, STmp2, OperationAdd);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);

    return STmp3;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface - compute its unnormalized norrmal vectorfield surface, as M
* the cross product if this partial derivatives.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To compute an unnormalized normal vector field for.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A vector field representing the unnormalized normal   M
*                      vector field of Srf.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfNormalSrf, normal, symbolic computation                           M
*****************************************************************************/
CagdSrfStruct *SymbSrfNormalSrf(const CagdSrfStruct *Srf)
{
    CagdSrfStruct
	*SrfDU = CagdSrfDerive(Srf, CAGD_CONST_U_DIR),
	*SrfDV = CagdSrfDerive(Srf, CAGD_CONST_V_DIR),
	*NormalSrf = SymbSrfCrossProd(SrfDV, SrfDU);

    CagdSrfFree(SrfDU);
    CagdSrfFree(SrfDV);

    return NormalSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Given two surface - add or subtract them as prescribed by OperationAdd,    *
* coordinatewise.			      				     *
*   The two surfaces are promoted to same type, point type, and order before *
* addition can take place.						     *
*   Returned is a surface representing their sum or difference.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   CSrf1, CSrf2:  Two surface to subtract coordinatewise.                   *
*   OperationAdd:  TRUE of addition, FALSE for subtraction.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:   The summation or difference of Srf1 and Srf2.         *
*****************************************************************************/
static CagdSrfStruct *SymbSrfAddSubAux(const CagdSrfStruct *CSrf1,
				       const CagdSrfStruct *CSrf2,
				       CagdBType OperationAdd)
{
    CagdBType
        SameWeights = FALSE,
	Srf1Rational = CAGD_IS_RATIONAL_SRF(CSrf1),
	Srf2Rational = CAGD_IS_RATIONAL_SRF(CSrf2),
	NoneRational = !Srf1Rational && !Srf2Rational,
	BothRational = Srf1Rational && Srf2Rational;
    int i, Len;
    CagdSrfStruct *SumSrf, *Srf1, *Srf2;
    CagdRType **Points1, **Points2;

    /* Make the two surfaces have the same point type and surface type. */
    Srf1 = CagdSrfCopy(CSrf1);
    Srf2 = CagdSrfCopy(CSrf2);
    if ((Srf1 -> UPeriodic ^ Srf2 -> UPeriodic) ||
	(Srf1 -> VPeriodic ^ Srf2 -> VPeriodic) ||
	!CagdMakeSrfsCompatible(&Srf1, &Srf2, NoneRational, NoneRational,
					      NoneRational, NoneRational))
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

    Points1 = Srf1 -> Points;
    Points2 = Srf2 -> Points;

    /* Check if both are rational with identical weights. */
    if (BothRational &&
	Srf1 -> UOrder == Srf2 -> UOrder &&
	Srf1 -> VOrder == Srf2 -> VOrder &&
	(Srf1 -> GType != CAGD_CBSPLINE_TYPE ||
	 (Srf1 -> ULength == Srf2 -> ULength &&
	  Srf1 -> VLength == Srf2 -> VLength &&
	  BspKnotVectorsSame(Srf1 -> UKnotVector, Srf2 -> UKnotVector,
			     Srf1 -> UOrder + Srf1 -> ULength, IRIT_EPS) &&
	  BspKnotVectorsSame(Srf1 -> VKnotVector, Srf2 -> VKnotVector,
			     Srf1 -> VOrder + Srf1 -> VLength, IRIT_EPS)))) {
	/* Maybe the weights are identical, in which we can still add the   */
	/* the respective control polygons.				    */
	for (i = 0; i < Srf1 -> ULength * Srf1 -> VLength; i++)
	    if (!IRIT_APX_EQ(Points1[0][i], Points2[0][i]))
		break;
	if (i >= Srf1 -> ULength * Srf1 -> VLength)
	    SameWeights = TRUE;
    }

    if (NoneRational || SameWeights) {
        if (!CagdMakeSrfsCompatible(&Srf1, &Srf2, TRUE, TRUE, TRUE, TRUE))
	    SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

	Len = Srf1 -> ULength * Srf1 -> VLength;
	SumSrf = CagdPeriodicSrfNew(Srf1 -> GType, Srf1 -> PType,
				    Srf1 -> ULength, Srf1 -> VLength,
				    Srf1 -> UPeriodic, Srf1 -> VPeriodic);
	SumSrf -> UOrder = Srf1 -> UOrder;
	SumSrf -> VOrder = Srf1 -> VOrder;
	if (CAGD_IS_BSPLINE_SRF(SumSrf)) {
	    SumSrf -> UKnotVector = BspKnotCopy(NULL, Srf1 -> UKnotVector,
						Srf1 -> UOrder +
						    CAGD_SRF_UPT_LST_LEN(Srf1));
	    SumSrf -> VKnotVector = BspKnotCopy(NULL, Srf1 -> VKnotVector,
						Srf1 -> VOrder +
						    CAGD_SRF_VPT_LST_LEN(Srf1));
	  }

	/* Simply add the respective control polygons. */
	SymbMeshAddSub(SumSrf -> Points, Srf1 -> Points, Srf2 -> Points,
		       SumSrf -> PType, Len, OperationAdd);
    }
    else {
        CagdSrfStruct *Srf1W, *Srf1X, *Srf1Y, *Srf1Z, *Srf2W, *Srf2X,
		*Srf2Y, *Srf2Z, *SumSrfW, *SumSrfX, *SumSrfY, *SumSrfZ;

	/* Weights are different. Must use the addition of rationals         */
	/* rule ( we invoke SymbSrfMult here):				     */
	/*								     */
	/*  x1     x2   x1 w2 +/- x2 w1					     */
	/*  -- +/- -- = ---------------					     */
	/*  w1     w2        w1 w2					     */
	/*								     */
	SymbSrfSplitScalar(Srf1, &Srf1W, &Srf1X, &Srf1Y, &Srf1Z);
	SymbSrfSplitScalar(Srf2, &Srf2W, &Srf2X, &Srf2Y, &Srf2Z);

	SumSrfW = SymbSrfMult(Srf1W, Srf2W);
	SumSrfX = SymbSrfRtnlMult(Srf1X, Srf1W, Srf2X, Srf2W, OperationAdd);
	SumSrfY = SymbSrfRtnlMult(Srf1Y, Srf1W, Srf2Y, Srf2W, OperationAdd);
	SumSrfZ = SymbSrfRtnlMult(Srf1Z, Srf1W, Srf2Z, Srf2W, OperationAdd);
	CagdSrfFree(Srf1W);
	CagdSrfFree(Srf1X);
	CagdSrfFree(Srf1Y);
	CagdSrfFree(Srf1Z);
	CagdSrfFree(Srf2W);
	CagdSrfFree(Srf2X);
	CagdSrfFree(Srf2Y);
	CagdSrfFree(Srf2Z);

	if (!CagdMakeSrfsCompatible(&SumSrfW, &SumSrfX,
				    TRUE, TRUE, TRUE, TRUE) ||
	    (SumSrfY != NULL &&
	     !CagdMakeSrfsCompatible(&SumSrfW, &SumSrfY,
				     TRUE, TRUE, TRUE, TRUE)) ||
	    (SumSrfZ != NULL &&
	     !CagdMakeSrfsCompatible(&SumSrfW, &SumSrfZ,
				     TRUE, TRUE, TRUE, TRUE)))
	    SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);

	SumSrf = SymbSrfMergeScalar(SumSrfW, SumSrfX, SumSrfY, SumSrfZ);
	CagdSrfFree(SumSrfW);
	CagdSrfFree(SumSrfX);
	CagdSrfFree(SumSrfY);
	CagdSrfFree(SumSrfZ);
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return SumSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface, splits it to its scalar component surfaces.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      Surface to split.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct **:   A vector of scalar surfaces - components of Srf.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSplitScalar, SymbSrfMergeScalarN                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSplitScalarN, split, symbolic computation                         M
*****************************************************************************/
CagdSrfStruct **SymbSrfSplitScalarN(const CagdSrfStruct *Srf)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	NumCoords = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	**Srfs = (CagdSrfStruct **) IritMalloc(sizeof(CagdSrfStruct *)
							  * CAGD_MAX_PT_SIZE);

    IRIT_ZAP_MEM(Srfs, sizeof(CagdSrfStruct *) * CAGD_MAX_PT_SIZE);

    for (i = IsNotRational; i <= NumCoords; i++) {
	Srfs[i] = CagdPeriodicSrfNew(Srf -> GType, CAGD_PT_E1_TYPE,
				     ULength, VLength,
				     Srf -> UPeriodic, Srf -> VPeriodic);
	Srfs[i] -> UOrder = Srf -> UOrder;
	Srfs[i] -> VOrder = Srf -> VOrder;
	if (Srf -> UKnotVector != NULL)
	    Srfs[i] -> UKnotVector = BspKnotCopy(NULL, Srf -> UKnotVector,
						 CAGD_SRF_UPT_LST_LEN(Srf)
						     + Srf -> UOrder);
	if (Srf -> VKnotVector != NULL)
	    Srfs[i] -> VKnotVector = BspKnotCopy(NULL, Srf -> VKnotVector,
						 CAGD_SRF_VPT_LST_LEN(Srf)
						     + Srf -> VOrder);

	SYMB_GEN_COPY(Srfs[i] -> Points[1], Srf -> Points[i],
		      sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);
    }

    return Srfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface splits it to its scalar component surfaces. Ignores all    M
* dimensions beyond the third, Z, dimension.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      Surface to split.                                              M
*   SrfW:     The weight component of Srf, if have any.                      M
*   SrfX:     The X component of Srf.                                        M
*   SrfY:     The Y component of Srf, if have any.                           M
*   SrfZ:     The Z component of Srf, if have any.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfMergeScalar, SymbSrfSplitScalar, SymbSrfSplitScalarN              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSplitScalar, split, symbolic computation                          M
*****************************************************************************/
void SymbSrfSplitScalar(const CagdSrfStruct *Srf,
			CagdSrfStruct **SrfW,
			CagdSrfStruct **SrfX,
			CagdSrfStruct **SrfY,
			CagdSrfStruct **SrfZ)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	NumCoords = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	*Srfs[CAGD_MAX_PT_SIZE];

    for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
	Srfs[i] = NULL;

    for (i = IsNotRational; i <= NumCoords; i++) {
	Srfs[i] = CagdPeriodicSrfNew(Srf -> GType, CAGD_PT_E1_TYPE,
				     ULength, VLength,
				     Srf -> UPeriodic, Srf -> VPeriodic);
	Srfs[i] -> UOrder = Srf -> UOrder;
	Srfs[i] -> VOrder = Srf -> VOrder;
	if (Srf -> UKnotVector != NULL)
	    Srfs[i] -> UKnotVector = BspKnotCopy(NULL, Srf -> UKnotVector,
						 CAGD_SRF_UPT_LST_LEN(Srf)
						     + Srf -> UOrder);
	if (Srf -> VKnotVector != NULL)
	    Srfs[i] -> VKnotVector = BspKnotCopy(NULL, Srf -> VKnotVector,
						 CAGD_SRF_VPT_LST_LEN(Srf)
						     + Srf -> VOrder);

	SYMB_GEN_COPY(Srfs[i] -> Points[1], Srf -> Points[i],
		      sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);
    }

    *SrfW = Srfs[0];
    *SrfX = Srfs[1];
    *SrfY = Srfs[2];
    *SrfZ = Srfs[3];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a vector of scalar surfaces, treat them as coordinates into a new    M
* vector surface.							     M
*   Assumes at least SrfVec is not NULL in which a scalar surface is	     M
* returned.								     M
*   Assumes SrfVec[i]/SrfW are either E1 or P1 in which the weights are      M
* assumed to be identical and can be ignored if SrfW exists and copied.      M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfW:     The weight component of new constructed surface, if exist,     M
*	      NULL if none.						     M
*   SrfVec:   A vector of scalar surfaces.			             M
*   NumSrfs:  Number of surfaces in SrfVec.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A new surface constructed from given scalar surfaces. M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfMergeScalar, SymbSrfSplitScalarN                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMergeScalarN, merge, symbolic computation                         M
*****************************************************************************/
CagdSrfStruct *SymbSrfMergeScalarN(const CagdSrfStruct *SrfW, 
				   const CagdSrfStruct **SrfVec, 
				   int NumSrfs)
{
    CagdBType
	WeightCopied = FALSE,
	IsRational = SrfW != NULL;
    int i, ULength, VLength,
	NumCoords = NumSrfs;
    CagdPointType
	PType = CAGD_MAKE_PT_TYPE(IsRational, NumCoords);
    CagdSrfStruct *Srfs[CAGD_MAX_PT_SIZE], *Srf;

    Srfs[0] = SrfW == NULL ? NULL : CagdSrfCopy(SrfW);
    for (i = 1; i < NumCoords + 1; i++)
	Srfs[i] = CagdSrfCopy(SrfVec[i - 1]);

    for (i = 0; i < NumCoords + 1; i++) {
	int j;

	for (j = i + 1; j < NumCoords + 1; j++)
	    if (Srfs[i] != NULL && Srfs[j] != NULL)
	        CagdMakeSrfsCompatible(&Srfs[i], &Srfs[j],
				       TRUE, TRUE, TRUE, TRUE);
    }

    ULength = Srfs[1] -> ULength;
    VLength = Srfs[1] -> VLength;
    Srf = CagdPeriodicSrfNew(Srfs[1] -> GType, PType, ULength, VLength,
			     Srfs[1] -> UPeriodic, Srfs[1] -> VPeriodic);

    Srf -> UOrder = Srfs[1] -> UOrder;
    Srf -> VOrder = Srfs[1] -> VOrder;
    if (Srfs[1] -> UKnotVector != NULL)
	Srf -> UKnotVector = BspKnotCopy(NULL, Srfs[1] -> UKnotVector,
					 CAGD_SRF_UPT_LST_LEN(Srfs[1])
						  + Srfs[1] -> UOrder);
    if (Srfs[1] -> VKnotVector != NULL)
	Srf -> VKnotVector = BspKnotCopy(NULL, Srfs[1] -> VKnotVector,
					 CAGD_SRF_VPT_LST_LEN(Srfs[1])
					          + Srfs[1] -> VOrder);

    for (i = !IsRational; i <= NumCoords; i++) {
	if (Srfs[i] != NULL) {
	    if (Srfs[i] -> PType != CAGD_PT_E1_TYPE) {
		if (Srfs[i] -> PType != CAGD_PT_P1_TYPE)
		    SYMB_FATAL_ERROR(SYMB_ERR_SCALAR_EXPECTED);
		else if (Srfs[0] == NULL && WeightCopied == FALSE) {
		    SYMB_GEN_COPY(Srf -> Points[0], Srfs[i] -> Points[0],
				  sizeof(CagdRType) * ULength * VLength);
		    WeightCopied = TRUE;
		}
	    }

	    SYMB_GEN_COPY(Srf -> Points[i], Srfs[i] -> Points[1],
			  sizeof(CagdRType) * ULength * VLength);
	}
    }

    for (i = 0; i < NumCoords + 1; i++)
	CagdSrfFree(Srfs[i]);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of scalar surfaces, treat them as coordinates into a new	     M
* surface.								     M
*   Assumes at least SrfX is not NULL in which a scalar surface is returned. M
*   Assumes SrfX/Y/Z/W are either E1 or P1 in which the weights are assumed  M
* to be identical and can be ignored if SrfW exists or copied otherwise.     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfW:     The weight component of new constructed surface, if have any.  M
*   SrfX:     The X component of new constructed surface.                    M
*   SrfY:     The Y component of new constructed surface, if have any.       M
*   SrfZ:     The Z component of new constructed surface, if have any.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A new surface constructed from given scalar surfaces. M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSplitScalar, SymbCrvMergeScalar                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfMergeScalar, merge, symbolic computation                          M
*****************************************************************************/
CagdSrfStruct *SymbSrfMergeScalar(const CagdSrfStruct *SrfW,
				  const CagdSrfStruct *SrfX,
				  const CagdSrfStruct *SrfY,
				  const CagdSrfStruct *SrfZ)
{
    CagdBType
	WeightCopied = FALSE,
	IsRational = SrfW != NULL;
    int i, ULength, VLength,
	NumCoords = (SrfX != NULL) + (SrfY != NULL) + (SrfZ != NULL);
    CagdPointType
	PType = CAGD_MAKE_PT_TYPE(IsRational, NumCoords);
    CagdSrfStruct *Srfs[CAGD_MAX_PT_SIZE], *Srf;

    Srfs[0] = SrfW == NULL ? NULL : CagdSrfCopy(SrfW);
    Srfs[1] = SrfX == NULL ? NULL : CagdSrfCopy(SrfX);
    Srfs[2] = SrfY == NULL ? NULL : CagdSrfCopy(SrfY);
    Srfs[3] = SrfZ == NULL ? NULL : CagdSrfCopy(SrfZ);

    for (i = 0; i < 4; i++) {
	int j;

	for (j = i + 1; j < 4; j++)
	    if (Srfs[i] != NULL && Srfs[j] != NULL)
	        CagdMakeSrfsCompatible(&Srfs[i], &Srfs[j],
				       TRUE, TRUE, TRUE, TRUE);
    }

    ULength = Srfs[1] -> ULength;
    VLength = Srfs[1] -> VLength;
    Srf = CagdSrfNew(Srfs[1] -> GType, PType, ULength, VLength);

    Srf -> UOrder = Srfs[1] -> UOrder;
    Srf -> VOrder = Srfs[1] -> VOrder;
    if (Srfs[1] -> UKnotVector != NULL)
	Srf -> UKnotVector = BspKnotCopy(NULL, Srfs[1] -> UKnotVector,
					 ULength + Srfs[1] -> UOrder);
    if (Srfs[1] -> VKnotVector != NULL)
	Srf -> VKnotVector = BspKnotCopy(NULL, Srfs[1] -> VKnotVector,
					 VLength + Srfs[1] -> VOrder);

    for (i = !IsRational; i <= NumCoords; i++) {
	if (Srfs[i] != NULL) {
	    if (Srfs[i] -> PType != CAGD_PT_E1_TYPE) {
		if (Srfs[i] -> PType != CAGD_PT_P1_TYPE)
		    SYMB_FATAL_ERROR(SYMB_ERR_SCALAR_EXPECTED);
		else if (Srfs[0] == NULL && WeightCopied == FALSE) {
		    SYMB_GEN_COPY(Srf -> Points[0], Srfs[i] -> Points[0],
				  sizeof(CagdRType) * ULength * VLength);
		    WeightCopied = TRUE;
		}
	    }

	    SYMB_GEN_COPY(Srf -> Points[i], Srfs[i] -> Points[1],
			  sizeof(CagdRType) * ULength * VLength);
	}
    }

    for (i = 0; i < 4; i++)
	CagdSrfFree(Srfs[i]);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Promote a scalar surface to three dimensions by moving the scalar axis to  M
* be the Z axis and adding monotone X and Y axes.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      Surface to promote from one to three dimensions.               M
*   UMin:     Minimum of new monotone X axis.                                M
*   UMax:     Maximum of new monotone X axis.                                M
*   VMin:     Minimum of new monotone Y axis.                                M
*   VMax:     Maximum of new monotone Y axis.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    A three dimensional surface.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPrmtSclrCrvTo2D                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPrmtSclrSrfTo3D, promotion, conversion, symbolic computation         M
*****************************************************************************/
CagdSrfStruct *SymbPrmtSclrSrfTo3D(const CagdSrfStruct *Srf,
				   CagdRType UMin,
				   CagdRType UMax,
				   CagdRType VMin,
				   CagdRType VMax)
{
    int i, j,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_SRF(Srf);
    CagdRType *Points, *WPoints,
	UStep = (UMax - UMin) / (ULength - 1),
	VStep = (VMax - VMin) / (VLength - 1);
    CagdSrfStruct
	*PrmtSrf = CagdCoerceSrfTo(Srf, IsRational ? CAGD_PT_P3_TYPE
						   : CAGD_PT_E3_TYPE, FALSE);

    /* Copy X (original scalar data) to Z. */
    CAGD_GEN_COPY(PrmtSrf -> Points[3], PrmtSrf -> Points[1],
		  sizeof(CagdRType) * ULength * VLength);

    Points = PrmtSrf -> Points[1];
    WPoints = IsRational ? PrmtSrf -> Points[0] : NULL;
    for (j = 0; j < VLength; j++)
	for (i = 0; i < ULength; i++)
	    *Points++ = (UMin + UStep * i) *
		(IsRational ? *WPoints++ : 1.0);

    Points = PrmtSrf -> Points[2];
    WPoints = IsRational ? PrmtSrf -> Points[0] : NULL;
    for (j = 0; j < VLength; j++)
	for (i = 0; i < ULength; i++)
	    *Points++ = (VMin + VStep * j) *
		(IsRational ? *WPoints++ : 1.0);

    return PrmtSrf;
}
