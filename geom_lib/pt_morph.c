/******************************************************************************
* Pt_Morth.c - metamorphosis of polygons.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 1998.					      *
******************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "geom_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a blend of the given two polyhedra as 't Pl1 + (1-t) Pl2'.      M
*   The two polyhedra are assumed to be of the same topology: same number of M
* (ordered) polygons and same number of vertices in each corresponding       M
* polygon.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl1, Pl2:    The two polygonal object to meta-morph.                     M
*   t:		 Linear blending factor: (1-t) * Pl1 + t * Pl2.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:     Blended polyhedra.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolygonalMorphosis                                                     M
*****************************************************************************/
IPPolygonStruct *GMPolygonalMorphosis(const IPPolygonStruct *Pl1,
				      const IPPolygonStruct *Pl2,
				      IrtRType t)
{
    IPPolygonStruct *Pl1c, *PlFirst;

    if (IPPolyListLen(Pl1) != IPPolyListLen(Pl2)) {
        GEOM_FATAL_ERROR(GEOM_ERR_UNEQUAL_NUM_OF_POLYS);
	return NULL;
    }

    Pl1c = PlFirst = IPCopyPolygonList(Pl1);
    for ( ; Pl1c != NULL ; Pl1c = Pl1c -> Pnext, Pl2 = Pl2 -> Pnext) {
        IPVertexStruct
	    *V1 = Pl1c -> PVertex;
        const IPVertexStruct
	    *V2 = Pl2 -> PVertex;

	do {
	    IRIT_PT_BLEND(V1 -> Coord, V2 -> Coord, V1 -> Coord, t);

	    V1 = V1 -> Pnext;
	    V2 = V2 -> Pnext;
        }
	while (V1 != Pl1c -> PVertex &&
	       V1 != NULL &&
	       V2 != Pl2 -> PVertex &&
	       V2 != NULL);

	if ((V1 != Pl1c -> PVertex && V1 != NULL) ||
	    (V2 != Pl2 -> PVertex && V2 != NULL)) {
	    GEOM_FATAL_ERROR(GEOM_ERR_UNEQUAL_NUM_OF_VRTXS);
	    return NULL;
	}
    }

    return PlFirst;
}
