/******************************************************************************
* Trng_sub.c - subdivision of triangular surfaces.                            *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a triangulare surface - subdivides it into two three sub-surfaces at M
* given parametric values u, v, w.					     M
*    Returns pointer to a list of two trngmed surfaces, at most. It can very M
* well may happen that the subdivided surface is completely trimmed out and  M
* hence nothing is returned for it.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrngSrf:  To subdivide at the prescibed parameter value t.               M
*   t:        The parameter to subdivide the curve Crv at.                   M
*   Dir:      Direction of subdivision. Either U or V.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:  The subdivided surfaces. Usually two, but can    M
*		      have only one, if other is totally trimmed away.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngSrfSubdivAtParam, subdivision                                        M
*****************************************************************************/
TrngTriangSrfStruct *TrngSrfSubdivAtParam(TrngTriangSrfStruct *TrngSrf,
					  CagdRType t,
					  CagdSrfDirType Dir)
{
    return NULL;
}
