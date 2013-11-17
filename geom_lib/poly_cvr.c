/******************************************************************************
* Poly_Pts.c - distribute uniformly points on a polygonal object	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb 1997.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_loc.h"
#include "bool_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a uniform distribution of points on a polygonal object.         M
*   If an "Imprt" attribute is found in a polygon then it is used to weigh   M
* the importance of this polygon and hence the number of points that will be M
* allocated to (on) it.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyObj:      Object to compute a uniform point distribution on.         M
*   n:		  Number of points to distribute (estimate).		     M
*   Dir:	  If given - use it as view dependent uniform distribution.  M
*		  Note that if Dir != NULL less than n points will be        M
*		  generated.						     M
*   PlAttr:       If not NULL, the created points are placed as attributes   M
*		  named PlAttr in each polygon in the return (copied) model. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A point list object.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointCoverOfPolyObj	                                             M
*****************************************************************************/
IPObjectStruct *GMPointCoverOfPolyObj(IPObjectStruct *PolyObj,
				      int n,
				      IrtRType *Dir,
				      char *PlAttr)
{
    IPObjectStruct *PObj;
    IrtRType Area;
    IPPolygonStruct *Pl;
    IPVertexStruct
	*PtDistList = NULL;

    for (Pl = PolyObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        if (IPVrtxListLen(Pl ->PVertex) != 3)
	    break;
    }
    if (Pl == NULL)
        PolyObj = IPCopyObject(NULL, PolyObj, FALSE);
    else
        PolyObj = GMConvertPolysToTriangles(PolyObj);

    Area = GMPolyObjectArea(PolyObj);

    for (Pl = PolyObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	IrtVecType V1, V2;
	IrtRType R,
	    NumOfPts = n * (GMPolyOnePolyArea(Pl) / Area) *
			   (Dir == NULL ? 1.0 : IRIT_DOT_PROD(Dir, Pl -> Plane)),
	    *Pt1 = Pl -> PVertex -> Coord,
	    *Pt2 = Pl -> PVertex -> Pnext -> Coord,
	    *Pt3 = Pl -> PVertex -> Pnext -> Pnext -> Coord;
	int i, PolyN;

	R = AttrGetRealAttrib(Pl -> Attr, "Imprt");
	if (!IP_ATTR_IS_BAD_REAL(R))
	    NumOfPts *= R;

	IRIT_PT_SUB(V1, Pt2, Pt1);
	IRIT_PT_SUB(V2, Pt3, Pt1);

	PolyN = (int) NumOfPts;
	if ((NumOfPts - PolyN) > IritRandom(0.0, 1.0))
	    PolyN++;

        /* Generate PolyN points on this triangle. */
        for (i = 0; i < PolyN; i++) {
	    IrtVecType V1a, V2a;
	    IrtRType
		Rand1 = IritRandom(0.0, 1.0),
		Rand2 = IritRandom(0.0, 1.0);
	    IPVertexStruct
		*V = IPAllocVertex2(PtDistList);

	    PtDistList = V;

	    /* Guarantee that we are inside the triangle... */
	    if (Rand1 + Rand2 > 1.0) {
		Rand1 = 1.0 - Rand1;
		Rand2 = 1.0 - Rand2;
	    }

	    IRIT_PT_COPY(V1a, V1);
	    IRIT_PT_COPY(V2a, V2);
	    IRIT_PT_SCALE(V1a, Rand1);
	    IRIT_PT_SCALE(V2a, Rand2);

	    IRIT_PT_COPY(V -> Coord, Pt1);
	    IRIT_PT_ADD(V -> Coord, V -> Coord, V1a);
	    IRIT_PT_ADD(V -> Coord, V -> Coord, V2a);
	}

	if (PlAttr != NULL) {     /* Each polygon gets to keep its own pts. */
	    AttrSetPtrAttrib(&Pl -> Attr, PlAttr, PtDistList);
	    PtDistList = NULL;
	}
    }

    if (PlAttr != NULL)
	return PolyObj;
    else {
	IPFreeObject(PolyObj);

	Pl = IPAllocPolygon(0, PtDistList, NULL);

	PObj = IPGenPOLYObject(Pl);
	IP_SET_POINTLIST_OBJ(PObj);
	return PObj;
    }
}
