/******************************************************************************
* UsrCnvrt.c - general conversions.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 96.					      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "cagd_lib.h"
#include "user_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a list of linear Bspline curves constructed from given polylines.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Polys:      To convert to linear bspline curves.                         M
*   FilterDups: If TRUE, filters out duplicates points in polygon, in        M
*		place.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Linear Bspline curves representing Poly.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtPolyline2LinBspCrv, UserPolyline2LinBsplineCrv                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserPolylines2LinBsplineCrvs, linear curves, conversion                  M
*****************************************************************************/
CagdCrvStruct *UserPolylines2LinBsplineCrvs(const IPPolygonStruct *Polys,
					    CagdBType FilterDups)
{
    CagdCrvStruct
	*Crvs = NULL;
    const IPPolygonStruct *Poly;

    for (Poly = Polys; Poly != NULL; Poly = Poly -> Pnext) {
	CagdCrvStruct
	    *Crv = UserPolyline2LinBsplineCrv(Poly, FilterDups);

	if (Crv != NULL)
	    IRIT_LIST_PUSH(Crv, Crvs);
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a linear Bspline curve constructed from given polyline.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:       To convert to a linear bspline curve.                        M
*   FilterDups: If TRUE, filters out duplicates points, in polygon, in       M
*		place.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A linear Bspline curve representing Poly.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtPolyline2LinBspCrv, UserPolylines2LinBsplineCrvs                M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserPolyline2LinBsplineCrv, linear curves, conversion                    M
*****************************************************************************/
CagdCrvStruct *UserPolyline2LinBsplineCrv(const IPPolygonStruct *Poly,
					  CagdBType FilterDups)
{
    IPVertexStruct *V;
    int i, Length;
    CagdCrvStruct *Crv;
    CagdRType **Points;

    if (FilterDups && Poly -> PVertex != NULL) {
	for (V = Poly -> PVertex; V -> Pnext != NULL; ) {
	    if (IRIT_PT_APX_EQ(V -> Coord, V -> Pnext -> Coord)) {
		IPVertexStruct
		    *VNext = V -> Pnext -> Pnext;

		IPFreeVertex(V -> Pnext);
		V -> Pnext = VNext;
	    }
	    else
		V = V -> Pnext;

	    if (V == Poly -> PVertex)
		break;
	}
    }

    V = Poly -> PVertex;
    if ((Length = IPVrtxListLen(V)) >= 2) {
	Crv = BspCrvNew(Length, 2, CAGD_PT_E3_TYPE);
	Points = Crv -> Points;

	BspKnotUniformOpen(Length, 2, Crv -> KnotVector);
	BspKnotAffineTrans2(Crv -> KnotVector,
			    Crv -> Length + Crv -> Order, 0, 1);

	for (i = 0; i < Length; i++, V = V -> Pnext) {
	    Points[1][i] = V -> Coord[0];
	    Points[2][i] = V -> Coord[1];
	    Points[3][i] = V -> Coord[2];
	}

	return Crv;
    }
    else
	return NULL;
}
