/******************************************************************************
* TrivMrph.c - A simple tool to morph between two compatible trivariates.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 97.					      *
******************************************************************************/

#include "triv_loc.h"
#include "misc_lib.h"
#include "geom_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two compatible trivariates (See function TrivMakeTVsCompatible),     M
* computes a convex blend between them according to Blend which must be      M
* between zero and one.							     M
*   Returned is the new blended trivariate.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV1, TV2:  The two trivariates to blend.                                 M
*   Blend:       A parameter between zero and one                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   TV2 * Blend + TV1 * (1 - Blend).                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTwoCrvsMorphing, SymbTwoCrvsMorphingCornerCut,			     M
*   SymbTwoCrvsMorphingMultiRes, SymbTwoSrfsMorphing, TrivMakeTVsCompatible  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTwoTVsMorphing, morphing                                             M
*****************************************************************************/
TrivTVStruct *TrivTwoTVsMorphing(const TrivTVStruct *TV1,
				 const TrivTVStruct *TV2,
				 CagdRType Blend)
{
    int i, j,
	MaxAxis = CAGD_NUM_OF_PT_COORD(TV1 -> PType),
	ULength = TV1 -> ULength,
	VLength = TV1 -> VLength,
	WLength = TV1 -> WLength,
	UOrder = TV1 -> UOrder,
	VOrder = TV1 -> VOrder,
	WOrder = TV1 -> WOrder;
    CagdRType **NewPoints,
	* const *Points1 = TV1 -> Points,
	* const *Points2 = TV2 -> Points,
	Blend1 = 1.0 - Blend;
    TrivTVStruct *NewTV;

    if (TV1 -> PType != TV2 -> PType ||
	TV1 -> GType != TV2 -> GType ||
	UOrder != TV2 -> UOrder ||
	VOrder != TV2 -> VOrder ||
	WOrder != TV2 -> WOrder ||
	ULength != TV2 -> ULength ||
	VLength != TV2 -> VLength ||
	WLength != TV2 -> WLength) {
	TRIV_FATAL_ERROR(TRIV_ERR_TVS_INCOMPATIBLE);
	return NULL;
    }
	
    NewTV = TrivTVNew(TV1 -> GType, TV1 -> PType, ULength, VLength, WLength);
    NewTV -> UOrder = UOrder;
    NewTV -> VOrder = VOrder;
    NewTV -> WOrder = WOrder;
    NewPoints = NewTV -> Points;
    if (TV1 -> UKnotVector != NULL)
	NewTV -> UKnotVector = BspKnotCopy(NULL, TV1 -> UKnotVector,
					   ULength + UOrder);
    if (TV1 -> VKnotVector != NULL)
	NewTV -> VKnotVector = BspKnotCopy(NULL, TV1 -> VKnotVector,
					   VLength + VOrder);
    if (TV1 -> WKnotVector != NULL)
	NewTV -> WKnotVector = BspKnotCopy(NULL, TV1 -> WKnotVector,
					   WLength + WOrder);

    for (i = !CAGD_IS_RATIONAL_PT(TV1 -> PType); i <= MaxAxis; i++) {
	CagdRType
	    *Pts1 = &Points1[i][0],
	    *Pts2 = &Points2[i][0],
	    *NewPts = &NewPoints[i][0];

	for (j = ULength * VLength * WLength - 1; j >= 0; j--)
	    *NewPts++ = *Pts1++ * Blend1 + *Pts2++ * Blend;
    }

    return NewTV;
}
