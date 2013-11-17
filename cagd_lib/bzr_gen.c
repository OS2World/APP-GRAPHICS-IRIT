/******************************************************************************
* Bzr-Gen.c - Bezier generic routines.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Bezier surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:      Number of control points in the U direction.               M
*   VLength:      Number of control points in the V direction.               M
*   PType:        Type of control points (E2, P3, etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An uninitialized freeform Bezier surface.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfNew, BspPeriodicSrfNew, CagdSrfNew, CagdPeriodicSrfNew, TrimSrfNew M
*   PwrSrfNew								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfNew, allocation                                                    M
*****************************************************************************/
CagdSrfStruct *BzrSrfNew(int ULength, int VLength, CagdPointType PType)
{
    CagdSrfStruct
	*Srf = CagdSrfNew(CAGD_SBEZIER_TYPE, PType, ULength, VLength);

    Srf -> UOrder = Srf -> ULength = ULength;
    Srf -> VOrder = Srf -> VLength = VLength;

    Srf -> UKnotVector = Srf -> VKnotVector = NULL;

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new Bezier curve.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points                                     M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An uninitialized freeform Bezier curve.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvNew, BspPeriodicCrvNew, CagdCrvNew, CagdPeriodicCrvNew, TrimCrvNew M
*   PwrCrvNew								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrvNew, allocation                                                    M
*****************************************************************************/
CagdCrvStruct *BzrCrvNew(int Length, CagdPointType PType)
{
    CagdCrvStruct
	*Crv = CagdCrvNew(CAGD_CBEZIER_TYPE, PType, Length);

    Crv -> Order = Crv -> Length = Length;

    if (Crv -> Order == 2)
	CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_LINEAR);
    else if (Crv -> Order == 1)
	CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_CONST);

    Crv -> KnotVector = NULL;

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates the memory required for a new Power basis surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:      Number of control points in the U direction.               M
*   VLength:      Number of control points in the V direction.               M
*   PType:        Type of control points (E2, P3, etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An uninitialized freeform Power basis surface.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfNew, BspPeriodicSrfNew, CagdSrfNew, CagdPeriodicSrfNew, TrimSrfNew M
*   BzrSrfNew								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrSrfNew, allocation                                                    M
*****************************************************************************/
CagdSrfStruct *PwrSrfNew(int ULength, int VLength, CagdPointType PType)
{
    CagdSrfStruct
	*Srf = CagdSrfNew(CAGD_SPOWER_TYPE, PType, ULength, VLength);

    Srf -> UOrder = Srf -> ULength = ULength;
    Srf -> VOrder = Srf -> VLength = VLength;

    Srf -> UKnotVector = Srf -> VKnotVector = NULL;

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new Power basis curve.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points                                     M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An uninitialized freeform Power basis curve.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvNew, BspPeriodicCrvNew, CagdCrvNew, CagdPeriodicCrvNew, TrimCrvNew M
*   BzrCrvNew								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrCrvNew, allocation                                                    M
*****************************************************************************/
CagdCrvStruct *PwrCrvNew(int Length, CagdPointType PType)
{
    CagdCrvStruct
	*Crv = CagdCrvNew(CAGD_CPOWER_TYPE, PType, Length);

    Crv -> Order = Crv -> Length = Length;

    if (Crv -> Order == 2)
	CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_LINEAR);
    else if (Crv -> Order == 1)
	CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_CONST);

    Crv -> KnotVector = NULL;

    return Crv;
}
