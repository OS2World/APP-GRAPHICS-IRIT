/******************************************************************************
* TrngCoer.c - Handle point coercesions/conversions.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a list of triangular surfaces to point type PType.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   To coerce to a new point type PType.                           M
*   PType:    New point type for TriSrf.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:   New triangular surfaces with PType as their     M
*			     point type.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngCoerceTriSrfsTo, coercion                                            M
*****************************************************************************/
TrngTriangSrfStruct *TrngCoerceTriSrfsTo(const TrngTriangSrfStruct *TriSrf,
					 CagdPointType PType)
{
    TrngTriangSrfStruct *CoercedTriSrf,
        *CoercedTriSrfs = NULL;

    for ( ; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
        CoercedTriSrf = TrngCoerceTriSrfTo(TriSrf, PType);
	IRIT_LIST_PUSH(CoercedTriSrf, CoercedTriSrfs);
    }

    return CagdListReverse(CoercedTriSrfs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a triangular surface to point type PType.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CTriSrf:   To coerce to a new point type PType.                          M
*   PType:     New point type for TriSrf.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:   A new trngariate with PType as its point type.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngCoerceTriSrfTo, coercion                                             M
*****************************************************************************/
TrngTriangSrfStruct *TrngCoerceTriSrfTo(const TrngTriangSrfStruct *CTriSrf,
					CagdPointType PType)
{
    TrngTriangSrfStruct
	*TriSrf = TrngTriSrfCopy(CTriSrf);

    CagdCoercePointsTo(TriSrf -> Points, TRNG_TRISRF_MESH_SIZE(TriSrf),
		       TriSrf -> PType, PType);

    TriSrf -> PType = PType;
    return TriSrf;
}
