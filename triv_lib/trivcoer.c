/******************************************************************************
* TrivCoer.c - Handle point coercions/conversions.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a list of trivariates to point type PType.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To coerce to a new point type PType.                           M
*   PType:    New point type for TV.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   New trivariates with PType as their point type.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCoerceTVsTo, coercion                                                M
*****************************************************************************/
TrivTVStruct *TrivCoerceTVsTo(const TrivTVStruct *TV, CagdPointType PType)
{
    TrivTVStruct *CoercedTV,
        *CoercedTVs = NULL;

    for ( ; TV != NULL; TV = TV -> Pnext) {
        CoercedTV = TrivCoerceTVTo(TV, PType);
	IRIT_LIST_PUSH(CoercedTV, CoercedTVs);
    }

    return CagdListReverse(CoercedTVs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a trivariate to point type PType.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To coerce to a new point type PType.                           M
*   PType:    New point type for TV.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   A new trivariate with PType as its point type.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivCoerceTVTo, coercion                                                 M
*****************************************************************************/
TrivTVStruct *TrivCoerceTVTo(const TrivTVStruct *TV, CagdPointType PType)
{
    TrivTVStruct
	*NewTV = TrivTVCopy(TV);

    CagdCoercePointsTo(NewTV -> Points,
		       NewTV -> ULength * NewTV -> VLength * NewTV -> WLength,
		       NewTV -> PType, PType);

    NewTV -> PType = PType;
    return NewTV;
}
