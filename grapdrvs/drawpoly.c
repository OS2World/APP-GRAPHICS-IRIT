/*****************************************************************************
*   Default polyline/gon drawing routine common to graphics drivers.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"

static void IGDrawPolyStrip(IPPolygonStruct *Pl);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Poly object using current modes and transformations.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A poly object to draw.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPoly                                                               M
*****************************************************************************/
void IGDrawPoly(IPObjectStruct *PObj)
{
    IPVertexStruct *V;
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    if (Pl == NULL)
        return;

    if (IGGlblAbortKeyPressed)
	return;

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS) {
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext)
		    IGPlotTo3D(V -> Coord);
	    }
	    else {
	        if (Pl -> PVertex != NULL && Pl -> PVertex -> Pnext != NULL) {
		    IGMoveTo3D(Pl -> PVertex -> Coord);
		    for (V = Pl -> PVertex -> Pnext; V != NULL; V = V -> Pnext)
		        IGLineTo3D(V -> Coord);
		}
	    }
	}
    }
    else if (IP_IS_POINTLIST_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS) {
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    IGPlotTo3D(V -> Coord);
		}
	    }
	    else {
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    int i;
		    IrtPtType Ends[6];
		    IrtRType
		        *Pt = V -> Coord;

		    for (i = 0; i < 6; i++)
		        IRIT_PT_COPY(Ends[i], Pt);

		    Ends[0][0] -= IGGlblPointSize;
		    Ends[1][0] += IGGlblPointSize;
		    Ends[2][1] -= IGGlblPointSize;
		    Ends[3][1] += IGGlblPointSize;
		    Ends[4][2] -= IGGlblPointSize;
		    Ends[5][2] += IGGlblPointSize;

		    for (i = 0; i < 6; i += 2) {
		        IGMoveTo3D(Ends[i]);
			IGLineTo3D(Ends[i+1]);
		    }
		}
	    }
	}
    }
    else if (IP_IS_POLYGON_OBJ(PObj) || IP_SET_POLYSTRIP_OBJ(PObj)) {
	int i, j,
	    PolysFromSrf = AttrGetObjectIntAttrib(PObj, "_srf_polys") == TRUE,
	    NumOfVertices = 0;
	IrtPtType PNormal, VNormal;

	if (IGGlblDrawSurfaceSketch)
	    IGDrawPolygonSketches(PObj);

	if (IGGlblDrawSurfaceSilh || IGGlblDrawSurfaceBndry)
	    IGDrawPolySilhBndry(PObj);

	if ((PolysFromSrf && !IGGlblDrawSurfacePoly) ||
	    (!PolysFromSrf && !IGGlblDrawPolygons))
	    return;

	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct *PrevV;

	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    if (IP_IS_STRIP_POLY(Pl)) {
	        IGDrawPolyStrip(Pl);
		continue;
	    }

	    if (IGGlblBackFaceCull) {
	        IrtRType P[3];

		MatMultVecby4by4(P, Pl -> Plane, IPViewMat);
		if ((P[2] > 0.0) ^ IGGlblFlipNormalOrient)
		    continue;
	    }

	    PrevV = Pl -> PVertex;
	    if (IGGlblDrawPNormal) {
		NumOfVertices = 1;
		IRIT_PT_COPY(PNormal, PrevV -> Coord);
	    }
	    IGMoveTo3D(PrevV -> Coord);

	    for (V = PrevV -> Pnext; V != NULL; PrevV = V, V = V -> Pnext) {
		if (IP_IS_INTERNAL_VRTX(PrevV) && !IGGlblDrawInternal)
		    IGMoveTo3D(V -> Coord);
		else {
		    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS)
		        IGPlotTo3D(V -> Coord);
		    else
		        IGLineTo3D(V -> Coord);
		}

		if (IGGlblDrawPNormal) {
		    for (j = 0; j < 3; j++)
			PNormal[j] += V -> Coord[j];
		    NumOfVertices++;
		}

		if (V -> Pnext == Pl -> PVertex)     /* For circular lists. */
		    break;
	    }
	    if (!IP_IS_INTERNAL_VRTX(PrevV) || IGGlblDrawInternal) {
	        if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS)
		    IGPlotTo3D(Pl -> PVertex -> Coord);
		else
		    IGLineTo3D(Pl -> PVertex -> Coord);
	    }

	    if (IGGlblDrawPNormal && IP_HAS_PLANE_POLY(Pl)) {
		for (i = 0; i < 3; i++)
		    PNormal[i] /= NumOfVertices;
		IGMoveTo3D(PNormal);
		IG_ADD_ORIENT_NRML(PNormal, Pl -> Plane);
		IGLineTo3D(PNormal);
	    }

	    if (IGGlblDrawVNormal) {
		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    if (IP_HAS_NORMAL_VRTX(V)) {
		        IRIT_VEC_COPY(VNormal, V -> Coord);
		        IG_ADD_ORIENT_NRML(VNormal, V -> Normal);
			IGMoveTo3D(V -> Coord);
			IGLineTo3D(VNormal);
		    }

		    if (V -> Pnext == Pl -> PVertex) /* For circular lists. */
		        break;
		}
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw a strip of polygons using current modes and transformations.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:     A poly strip to draw.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawPolyStrip(IPPolygonStruct *Pl)
{
    int i, j;
    IPVertexStruct *Prev1V, *Prev2V,
	*V = Pl -> PVertex;

    /* Line drawing. */
    Prev1V = Pl -> PVertex;
    IGMoveTo3D(Prev1V -> Coord);

    Prev2V = Prev1V -> Pnext;
    IGLineTo3D(Prev2V -> Coord);

    V = Prev2V -> Pnext;

    for ( ; V != NULL; V = V -> Pnext) {
        IGMoveTo3D(V -> Coord);
        IGLineTo3D(Prev1V -> Coord);

        IGMoveTo3D(V -> Coord);
        IGLineTo3D(Prev2V -> Coord);

	Prev1V = Prev2V;
	Prev2V = V;
    }

    if (IGGlblDrawVNormal) {
        IrtVecType VNormal;

	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    if (IP_HAS_NORMAL_VRTX(V)) {
	        for (j = 0; j < 3; j++)
		    VNormal[j] = V -> Coord[j] +
				 V -> Normal[j] * IGGlblNormalSize;

		IGMoveTo3D(V -> Coord);
		IGLineTo3D(VNormal);
	    }
	}
    }

    if (IGGlblDrawPNormal) {
	int j;

	Prev1V = Pl -> PVertex;
	Prev2V = Prev1V -> Pnext;
	for (V = Prev2V -> Pnext, j = 0; V != NULL; V = V -> Pnext, j++) {
	    IrtVecType Center, V1, V2, Nrml;

	    IRIT_PT_COPY(Center, V -> Coord);
	    IRIT_PT_ADD(Center, Center, Prev1V -> Coord);
	    IRIT_PT_ADD(Center, Center, Prev2V -> Coord);
	    IRIT_PT_SCALE(Center, 1.0 / 3.0);

	    IRIT_PT_SUB(V1, Prev1V -> Coord, Prev2V -> Coord);
	    IRIT_PT_SUB(V2, Prev2V -> Coord, V -> Coord);
	    IRIT_CROSS_PROD(Nrml, V1, V2);
	    IRIT_PT_NORMALIZE(Nrml);

	    /* Make sure we have the orientation right - cannot get it right */
	    /* from the polygonal strip itself!				     */
	    if (IP_HAS_NORMAL_VRTX(V)) {
		if (IRIT_DOT_PROD(Nrml, V -> Normal) < 0)
		    IRIT_PT_SCALE(Nrml, -1.0);
	    }
	    else if (j & 0x01) {
		/* Maybe flipped, but at least be consistent thoughout. */
		IRIT_PT_SCALE(Nrml, -1.0);
	    }

	    IGMoveTo3D(Center);
	    for (i = 0; i < 3; i++)
		Center[i] += Nrml[i] * IGGlblNormalSize;
	    IGLineTo3D(Center);

	    Prev1V = Prev2V;
	    Prev2V = V;
	}
    }
}
