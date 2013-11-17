/******************************************************************************
* CPwr-Aux.c - Power basis curve auxilary routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

IRIT_STATIC_DATA CagdBType
    GlblDeriveScalar = FALSE;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a pointer to a static data, holding the value of the curve at    M
* given parametric location t. The curve is assumed to be a power basis.     M
*   Evaluation is conducted using the Horner rule.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To evaluate at the given parametric location t.                M
*   t:        The parameter value at which the curve Crv is to be evaluated. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of curve Crv's point type. If for example the curve's      M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvEval, BspCrvEvalAtParam, BzrCrvEvalAtParam,                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvEvalAtParam, evaluation                                            M
*****************************************************************************/
CagdRType *PwrCrvEvalAtParam(const CagdCrvStruct *Crv, CagdRType t)
{
    static CagdRType Buf[CAGD_MAX_PT_COORD];
    CagdBType
        IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
        k = Crv -> Order,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);

    for (j = IsNotRational; j <= MaxCoord; j++) {
        CagdRType const
	    *Pts = Crv -> Points[j];

        Buf[j] = Pts[k - 1];
        for (i = k - 2; i >= 0; i--)
	    Buf[j] = Buf[j] * t + Pts[i];
    } 

    return Buf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, equal to the given curve, differentiated once.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To differentiate.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvDerive, BspCrvDerive, BzrCrvDerivePwr, BspCrvDeriveRational,      M
*   CrvDeriveRational, PwrCrvDeriveScalar				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvDerive, derivatives                                                M
*****************************************************************************/
CagdCrvStruct *PwrCrvDerive(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, j,
	k = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *DerivedCrv;

    if (!GlblDeriveScalar && !IsNotRational) {
	CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    DerivedCrv = PwrCrvNew(IRIT_MAX(1, k - 1), Crv -> PType);

    if (k >= 2) {
        for (j = IsNotRational; j <= MaxCoord; j++) {
	    CagdRType
		*DPts = DerivedCrv -> Points[j],
	        *Pts = Crv -> Points[j];

	    for (i = 0; i < k - 1; i++)
		DPts[i] = (i + 1) * Pts[i + 1];
	}
    }
    else {
	for (j = IsNotRational; j <= MaxCoord; j++)
	    DerivedCrv -> Points[j][0] = 0.0;
    }

    return DerivedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, equal to the given curve, differentiated once.        M
*   For a Euclidean curve this is the same as CagdCrvDerive but for a        M
* rational curve the returned curve is not the vector field but simply the   M
* derivatives of all the curve's coefficients, including the weights.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To differentiate.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   PwrCrvDerive, CagdCrvDerive, PwrCrvDeriveRational, BspCrvDeriveRational  M
*   BspCrvDerive, PwrCrvDeriveScalar, CagdCrvDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvDeriveScalar, derivatives                                          M
*****************************************************************************/
CagdCrvStruct *PwrCrvDeriveScalar(const CagdCrvStruct *Crv)
{
    CagdCrvStruct *TCrv;

    GlblDeriveScalar = TRUE;

    TCrv = PwrCrvDerive(Crv);

    GlblDeriveScalar = FALSE;

    return TCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bezier curve, equal to the integral of the given power       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to integrate.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Integrated curve.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvIntegrate, BzrSrfIntegrate, CagdCrvIntegrate                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvIntegrate, integrals                                               M
*****************************************************************************/
CagdCrvStruct *PwrCrvIntegrate(const CagdCrvStruct *Crv)
{
    int j, k,
	n = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *IntCrv;

    if (CAGD_IS_RATIONAL_CRV(Crv))
	CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);

    IntCrv = PwrCrvNew(n + 1, Crv -> PType);

    for (k = 1; k <= MaxCoord; k++) {
	CagdRType
	    *Points = Crv -> Points[k],
	    *IntPoints = IntCrv -> Points[k];

	IntPoints[0] = 0.0;
	for (j = 1; j < n + 1; j++)
	    IntPoints[j] = Points[j - 1] / j;
    }

    return IntCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, identical to the original but with order NewOrder.    M
*   Degree raise is computed by adding zeros at high order coefs.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise its degree to a NewOrder.                           M
*   NewOrder:   NewOrder for Crv.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve of order NewOrder representing the same        M
*                     geometry as Crv.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDegreeRaise, BzrCrvDegreeRaiseN, PwrCrvDegreeRaise	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvDegreeRaiseN, degree raising                                       M
*****************************************************************************/
CagdCrvStruct *PwrCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int j,
	k = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *RaisedCrv;

    if (k > NewOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedCrv = PwrCrvNew(NewOrder, Crv -> PType);

    for (j = IsNotRational; j <= MaxCoord; j++) {
        int l;

        IRIT_GEN_COPY(RaisedCrv -> Points[j], Crv -> Points[j],
		 k * sizeof(CagdRType));
	for (l = k; l < NewOrder; l++)
	    RaisedCrv -> Points[j][l] = 0.0;
    }

    CAGD_PROPAGATE_ATTR(RaisedCrv, Crv);

    return RaisedCrv; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve, identical to the original but with one degree higher. M
*   Adds one more, highest degree coefficient, that is identically zero.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise its degree by one.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve of one order higher representing the same      M
*                     geometry as Crv.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDegreeRaiseN, BzrCrvDegreeRaise, PwrCrvDegreeRaiseN                M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvDegreeRaise, degree raising                                        M
*****************************************************************************/
CagdCrvStruct *PwrCrvDegreeRaise(const CagdCrvStruct *Crv)
{
    return PwrCrvDegreeRaiseN(Crv, Crv -> Length + 1);
}
