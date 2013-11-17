/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, May 1995    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to compute the offset of a polygon/line, in the XY plame.	     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "allocate.h"
#include "geom_loc.h"

#define GM_MINIMUM_MITER_SCALE 0.01		   /* A miter scale of 100. */
#define GM_MAX_ANGLE_BLEND_NRML	180.0

static IrtRType GMPolyOffsetAmountConstant(IrtRType *Coord);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Default offset amount estimating routine - returns a constant scaling    *
* factor of one.                                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Coord:    Of point as XYZ values.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  scaling factor of offset amount.                              *
*****************************************************************************/
static IrtRType GMPolyOffsetAmountConstant(IrtRType *Coord)
{
    return 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the offset amount to be a function of the depth Z value by scaling  M
* with 1/Z.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Coord:    Of point as XYZ values.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  scaling factor of offset amount.                              M
*                                                                            *
* SEE ALSO:								     M
*   GMPolyOffset							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyOffsetAmountDepth                                                  M
*****************************************************************************/
IrtRType GMPolyOffsetAmountDepth(const IrtRType *Coord)
{
    return 1.0 / (IRIT_EPS + IRIT_FABS(Coord[2] + 10.0));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the offset of a given polygon/line in the XY plane by Ofst.     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:        To compute its offset in the XY plane.                      M
*   IsPolygon:   TRUE for a polygon, FALSE for a polyline.                   M
*   Ofst:        Amount of offset.				             M
*   AmountFunc:  Scale the offset amount according to this function. A NULL  M
*		 here will use a constant scaling factor of one.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Offset of Poly by Ofst amount.                      M
*                                                                            *
* SEE ALSO:								     M
*   GMPolyOffsetAmountDepth, GMPolyOffset3D				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyOffset                                                             M
*****************************************************************************/
IPPolygonStruct *GMPolyOffset(const IPPolygonStruct *Poly,
			      int IsPolygon,
			      IrtRType Ofst,
			      GMPolyOffsetAmountFuncType AmountFunc)
{
    IrtRType R;
    IrtVecType Dir, DirAux, PrevDir;
    IPPolygonStruct
        *PolyOfst = IPAllocPolygon(Poly -> Tags,
				   IPCopyVertexList(Poly -> PVertex), NULL);
    IPVertexStruct
	*V = Poly -> PVertex,
	*Vnext = V -> Pnext,
	*VO = PolyOfst -> PVertex;

    if (AmountFunc == NULL)
	AmountFunc = GMPolyOffsetAmountConstant;

    IRIT_PT_SUB(PrevDir, Vnext -> Coord, V -> Coord);
    PrevDir[2] = 0.0; 				     /* Make sure Z is zero. */
    IRIT_PT_NORMALIZE(PrevDir);
    if (!IsPolygon) {
	/* Handle the first vertex, if a polyline. */
	R = AmountFunc(VO -> Coord) * Ofst;
	VO -> Coord[0] += PrevDir[1] * R;
	VO -> Coord[1] -= PrevDir[0] * R;
    }

    /* Iterate through the rest of the points.  For a polygon the list is    */
    /* assumed circular.  For a polyline the list is assumed NULL terminated.*/
    V = Vnext;
    Vnext = Vnext -> Pnext;
    VO = VO -> Pnext;
    if (Vnext != NULL) {
	do {
	    IRIT_PT_SUB(Dir, Vnext -> Coord, V -> Coord);
	    Dir[2] = 0.0; 			     /* Make sure Z is zero. */
	    IRIT_PT_NORMALIZE(Dir);
	    IRIT_PT_ADD(DirAux, Dir, PrevDir);
	    IRIT_PT_NORMALIZE(DirAux);

	    /* Compute the miter joint's scaling factor of 1/sin(alpha). */
	    R = sin(acos(-IRIT_DOT_PROD(Dir, PrevDir)) * 0.5);
	    if (R < GM_MINIMUM_MITER_SCALE)
	        R = GM_MINIMUM_MITER_SCALE;

	    R = AmountFunc(VO -> Coord) * Ofst / R;
	    VO -> Coord[0] += DirAux[1] * R;
	    VO -> Coord[1] -= DirAux[0] * R;

	    V = Vnext;
	    Vnext = Vnext -> Pnext;
	    VO = VO -> Pnext;

	    IRIT_PT_COPY(PrevDir, Dir);
	}
	while (Vnext != NULL && V != Poly -> PVertex -> Pnext);
    }
    else
        IRIT_PT_COPY(Dir, PrevDir);

    if (!IsPolygon) {
	/* Do the last point. */
	R = AmountFunc(VO -> Coord) * Ofst;
	VO -> Coord[0] += Dir[1] * R;
	VO -> Coord[1] -= Dir[0] * R;
    }
    else {
        IRIT_PLANE_COPY(PolyOfst -> Plane, Poly -> Plane);
    }

    return PolyOfst;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the offset of a given polygon object in R^3 by Ofst.	     M
*   Computation is done by moving all vertices in the nomal direction by     M
* ofst amount.  If Poly does not contain vertex normals, they are estimated. M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:        To compute its offset in R^3.		                     M
*   Ofst:        Amount of offset.				             M
*   ForceSmoothing:  True to force normal smoothing.			     M
*   MiterEdges:  True to take into consideration the dihedral angle.         M
*   AmountFunc:  Scale the offset amount according to this function. A NULL  M
*		 here will use a constant scaling factor of one.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Offset of Poly by Ofst amount.                      M
*                                                                            *
* SEE ALSO:								     M
*   GMPolyOffsetAmountDepth, GMPolyOffset				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyOffset3D                                                           M
*****************************************************************************/
IPPolygonStruct *GMPolyOffset3D(const IPPolygonStruct *Poly,
				IrtRType Ofst,
				int ForceSmoothing,
				int MiterEdges,
				GMPolyOffsetAmountFuncType AmountFunc)
{
    int HasVNrmls = TRUE;
    const IPPolygonStruct *Pl;
    IPPolygonStruct *Poly2, *Pl2;

    for (Pl = Poly; HasVNrmls && Pl != NULL; Pl = Pl -> Pnext) {
        const IPVertexStruct
	    *V = Pl -> PVertex;

	do {
	    if (!IP_HAS_NORMAL_VRTX(V))
	        HasVNrmls = FALSE;
	    else
	        V = V -> Pnext;
	}
	while (HasVNrmls && V != NULL && V != Pl -> PVertex);
    }

    Poly2 = IPCopyPolygonList(Poly);   /* We are about to modify the input. */

    if (ForceSmoothing || !HasVNrmls) {
	/* Estimate the normals locally. */
        GMBlendNormalsToVertices(Poly2, GM_MAX_ANGLE_BLEND_NRML);
    }

    for (Pl2 = Poly2; Pl2 != NULL; Pl2 = Pl2 -> Pnext) {
        IrtRType R, D;
	IrtVecType N;
        IPVertexStruct
	    *V = Pl2 -> PVertex;

	do {
	    R = AmountFunc == NULL ? Ofst : AmountFunc(V -> Coord) * Ofst;

	    if (MiterEdges) {
	        D = AttrGetRealAttrib(V -> Attr, "_CosNrmlMaxDeviation");
		if (!IP_ATTR_IS_BAD_REAL(D)) {
		    R /= IRIT_MAX(D, IRIT_EPS);
		}
	    }

	    IRIT_VEC_SCALE2(N, V -> Normal, R);
	    IRIT_PT_ADD(V -> Coord, V -> Coord, N);
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl2 -> PVertex);

	/* Update the new plane of the offseted polygons. */
	IRIT_VEC_COPY(N, Pl2 -> Plane);
	IPUpdatePolyPlane(Pl2);
	if (IRIT_DOT_PROD(N, Pl2 -> Plane) < 0.0) {
	    IRIT_PLANE_SCALE(Pl2 -> Plane, -1.0);
	}
    }

    return Poly2;
}
