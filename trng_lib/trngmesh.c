/******************************************************************************
* TrngMesh.c - Extract control mesh of triangular surfcae as set of polylines.*
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a polyline representation to the control mesh of the triangular   M
* surface								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:  To compute a polyline representation for its control mesh.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A polyline representing TriSrf's control mesh.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrf2CtrlMesh, bbox, bounding box                                  M
*****************************************************************************/
CagdPolylineStruct *TrngTriSrf2CtrlMesh(const TrngTriangSrfStruct *TriSrf)
{
    int i, j, k, l, Index,
	Length = TriSrf -> Length;
    TrngTriangSrfStruct
	*E3TriSrf = TrngCoerceTriSrfTo(TriSrf, CAGD_PT_E3_TYPE);
    CagdRType
    	**Points = E3TriSrf -> Points;
    CagdPolylineStruct
	*PolyList = NULL;

    for (i = 0; i < Length; i++) {
	CagdPolylineStruct
	    *Poly1 = CagdPolylineNew(Length - i),
	    *Poly2 = CagdPolylineNew(Length - i),
	    *Poly3 = CagdPolylineNew(Length - i);

	for (j = 0; j < Length - i; j++) {
	    k = Length - i - j - 1;

	    Index = TRNG_MESH_IJK(TriSrf, i, j, k);
	    for (l = 0; l < 3; l++)
		Poly1 -> Polyline[j].Pt[l] = Points[l + 1][Index];

	    Index = TRNG_MESH_IJK(TriSrf, j, k, i);
	    for (l = 0; l < 3; l++)
		Poly2 -> Polyline[j].Pt[l] = Points[l + 1][Index];

	    Index = TRNG_MESH_IJK(TriSrf, k, i, j);
	    for (l = 0; l < 3; l++)
		Poly3 -> Polyline[j].Pt[l] = Points[l + 1][Index];
	}

	IRIT_LIST_PUSH(Poly1, PolyList);
	IRIT_LIST_PUSH(Poly2, PolyList);
	IRIT_LIST_PUSH(Poly3, PolyList);
    }
	
    TrngTriSrfFree(E3TriSrf);

    return PolyList;
}
