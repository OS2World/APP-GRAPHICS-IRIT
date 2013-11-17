/******************************************************************************
* CagdMesh.c - Extract surface control mesh/curve control polygon as polyline *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts the control polygon of a curve as a polyline.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To extract a control polygon from.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  The control polygon of Crv.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrv2CtrlPoly, control polygon                                        M
*****************************************************************************/
CagdPolylineStruct *CagdCrv2CtrlPoly(const CagdCrvStruct *Crv)
{
    int i,
	Length = Crv -> Length + (Crv -> Periodic != FALSE);
    CagdRType
	* const *CrvP = Crv -> Points;
    CagdPolylnStruct *NewPolyline;
    CagdPolylineStruct *P;

    P = CagdPolylineNew(Length);
    NewPolyline = P -> Polyline;

    for (i = 0; i < Length; i++) {
	CagdCoerceToE3(NewPolyline -> Pt, CrvP, i % Crv -> Length,
		       Crv -> PType);
	NewPolyline++;
    }
    return P;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts the control mesh of a surface as a list of polylines.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To extract a control mesh from.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  The control mesh of Srf.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2CtrlMesh, control mesh                                           M
*****************************************************************************/
CagdPolylineStruct *CagdSrf2CtrlMesh(const CagdSrfStruct *Srf)
{
    int i, j,
	ULength = Srf -> ULength + (Srf -> UPeriodic != FALSE),
	VLength = Srf -> VLength + (Srf -> VPeriodic != FALSE);
    CagdRType
	* const *SrfP = Srf -> Points;
    CagdPolylnStruct *NewPolyline;
    CagdPolylineStruct *P,
	*PList = NULL;

    for (j = 0; j < VLength; j++) {	   /* Generate the rows of the mesh. */
	P = CagdPolylineNew(ULength);
	NewPolyline = P -> Polyline;

	for (i = 0; i < ULength; i++) {
	    CagdCoerceToE3(NewPolyline -> Pt, SrfP,
			   CAGD_MESH_UV(Srf, i % Srf -> ULength,
					     j % Srf -> VLength),
			   Srf -> PType);
            NewPolyline++;
	}
	IRIT_LIST_PUSH(P, PList);
    }

    for (i = 0; i < ULength; i++) {	   /* Generate the cols of the mesh. */
	P = CagdPolylineNew(VLength);
	NewPolyline = P -> Polyline;

	for (j = 0; j < VLength; j++) {
	    CagdCoerceToE3(NewPolyline -> Pt, SrfP,
			   CAGD_MESH_UV(Srf, i % Srf -> ULength,
					     j % Srf -> VLength),
			   Srf -> PType);
            NewPolyline++;
	}
	IRIT_LIST_PUSH(P, PList);
    }

    return PList;
}
