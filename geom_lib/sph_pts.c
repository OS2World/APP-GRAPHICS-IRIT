/******************************************************************************
* Sph_Pts.c - distribute uniformly points on a sphere.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 1996.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a honey comb distribution of points on a sphere. Result is      M
* an approximation.						             M
*                                                                            *
* PARAMETERS:                                                                M
*   HoneyCombSize:      Size of honey comb.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A point list object.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointCoverOfUnitHemiSphere                                             M
*****************************************************************************/
IPObjectStruct *GMPointCoverOfUnitHemiSphere(IrtRType HoneyCombSize)
{
    IrtRType
	Cos30 = cos(M_PI / 6),
	Alpha = 2 * asin(HoneyCombSize / 2);
    int x, y,
	d = (int) (20 * M_PI / Alpha);
    IPPolygonStruct *Poly;
    IPVertexStruct *V,
	*VHead = NULL;
    IPObjectStruct *PObj;

    for (y = -d; y <= d; y++) {
	for (x = -d; x <= d; x++) {
	    IrtRType CoordZ = 0, DAlpha,
		CoordX = x + (y & 0x01 ? 0.5 : 0.0),
		CoordY = Cos30 * y,
		Len = sqrt(IRIT_SQR(CoordX) + IRIT_SQR(CoordY));

	    if (IRIT_APX_EQ(Len, 0.0))
	        Len = IRIT_EPS;

	    if ((DAlpha = Len * Alpha) < M_PI_DIV_2) {
		IrtRType
		    NewLen = tan(DAlpha);

		/* Angular span fits the southern hemisphere. */
		CoordX *= NewLen / Len;
		CoordY *= NewLen / Len;

		Len = sqrt(IRIT_SQR(CoordX) + IRIT_SQR(CoordY) + 1.0);
		CoordX /= Len;
		CoordY /= Len;
		CoordZ = 1.0 / Len;

		V = IPAllocVertex2(VHead);
		VHead = V;
		V -> Coord[0] = CoordX;
		V -> Coord[1] = CoordY;
		V -> Coord[2] = CoordZ;
	    }
	}
    }

    Poly = IPAllocPolygon(0, VHead, NULL);

    PObj = IPGenPOLYObject(Poly);
    IP_SET_POINTLIST_OBJ(PObj);
    return PObj;
}
