/******************************************************************************
* TrivMesh.c - Extract trivariate control mesh as a set of polylines.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts the control mesh of a surface as a list of polylines.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Trivar:     To extract a control mesh from.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  The control mesh of Srf.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTV2CtrlMesh, control mesh                                            M
*****************************************************************************/
CagdPolylineStruct *TrivTV2CtrlMesh(const TrivTVStruct *Trivar)
{
    int i, j, k,
	ULength = Trivar -> ULength + (Trivar -> UPeriodic != FALSE),
	VLength = Trivar -> VLength + (Trivar -> VPeriodic != FALSE),
	WLength = Trivar -> WLength + (Trivar -> WPeriodic != FALSE);
    CagdRType
	* const *TrivarP = Trivar -> Points;
    CagdPolylnStruct *NewPolyline;
    CagdPolylineStruct *P,
	*PList = NULL;

    for (k = 0; k < WLength; k++) {	   /* Generate the rows of the mesh. */
	for (j = 0; j < VLength; j++) {
	    P = CagdPolylineNew(ULength);
	    NewPolyline = P -> Polyline;

	    for (i = 0; i < ULength; i++) {
		CagdCoerceToE3(NewPolyline -> Pt, TrivarP,
			       TRIV_MESH_UVW(Trivar, i % Trivar -> ULength,
					             j % Trivar -> VLength,
					             k % Trivar -> WLength),
			       Trivar -> PType);
		NewPolyline++;
	    }
	    IRIT_LIST_PUSH(P, PList);
	}
    }

    for (k = 0; k < WLength; k++) {	   /* Generate the cols of the mesh. */
	for (i = 0; i < ULength; i++) {
	    P = CagdPolylineNew(VLength);
	    NewPolyline = P -> Polyline;

	    for (j = 0; j < VLength; j++) {
		CagdCoerceToE3(NewPolyline -> Pt, TrivarP,
			       TRIV_MESH_UVW(Trivar, i % Trivar -> ULength,
					             j % Trivar -> VLength,
					             k % Trivar -> WLength),
			       Trivar -> PType);
		NewPolyline++;
	    }
	    IRIT_LIST_PUSH(P, PList);
	}
    }

    for (i = 0; i < ULength; i++) {	 /* Generate the depths of the mesh. */
	for (j = 0; j < VLength; j++) {
	    P = CagdPolylineNew(WLength);
	    NewPolyline = P -> Polyline;

	    for (k = 0; k < WLength; k++) {
		CagdCoerceToE3(NewPolyline -> Pt, TrivarP,
			       TRIV_MESH_UVW(Trivar, i % Trivar -> ULength,
					             j % Trivar -> VLength,
					             k % Trivar -> WLength),
			       Trivar -> PType);
		NewPolyline++;
	    }
	    IRIT_LIST_PUSH(P, PList);
	}
    }

    return PList;
}
