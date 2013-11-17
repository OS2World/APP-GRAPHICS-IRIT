/*****************************************************************************
* A facility to convert line segments into triangles.                        *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "polyline.h"
#include <geom_lib.h>

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize the segment structure, using the PolylineOptions.             M
*   Should be called when object is created.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:           IN, OUT, pointer to the line segment.                     M
*   PolyOptions:   IN, the polyline options structure.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentInit, Initialize Options                                      M
*****************************************************************************/
void LineSegmentInit(IRndrLineSegmentStruct *Seg,
		     IRndrPolylineOptionsStruct *PolyOptions)
{
    Seg -> TriVertex[2] = IPAllocVertex2(NULL);
    Seg -> TriVertex[1] = IPAllocVertex2(Seg -> TriVertex[2]);
    Seg -> TriVertex[0] = IPAllocVertex2(Seg -> TriVertex[1]);

    Seg -> Tri = IPAllocPolygon(0, Seg -> TriVertex[0], NULL);

    Seg -> Tri -> Plane[RNDR_X_AXIS] = 0.0;
    Seg -> Tri -> Plane[RNDR_Y_AXIS] = 0.0;
    Seg -> Tri -> Plane[RNDR_Z_AXIS] = -1.0;
    Seg -> Tri -> Plane[RNDR_W_AXIS] = 0.0;
    IPUpdateVrtxNrml(Seg -> Tri, Seg -> Tri -> Plane);
    Seg -> TrianglesNum = 0;

    if (PolyOptions)
        LineSegmentSetOptions(Seg, PolyOptions);
    else {
        Seg -> k = 0;
        Seg -> PolyOptions.MaxWidth =
	    Seg -> PolyOptions.MinWidth = 0.01;
        Seg -> PolyOptions.ZNear =
	    Seg -> PolyOptions.ZFar = 0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Changes the PolyOptions.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:           IN, OUT, pointer to the line segment.                     M
*   PolyOptions:   IN, the polyline options structure.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentSetOptions,Initialize Options                                 M
*****************************************************************************/
void LineSegmentSetOptions(IRndrLineSegmentStruct *Seg,
			   IRndrPolylineOptionsStruct *PolyOptions)
{
    if (!PolyOptions)
	return;

    memcpy(&Seg -> PolyOptions, PolyOptions,
	   sizeof(IRndrPolylineOptionsStruct));

    Seg -> k = IRIT_APX_EQ(PolyOptions -> ZNear, PolyOptions -> ZFar) ?
        0 : (PolyOptions -> MaxWidth - PolyOptions -> MinWidth) /
        (PolyOptions -> ZNear - PolyOptions -> ZFar);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees the memory allocated by the object.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:   IN, OUT, pointer to the line segment.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentRelease, Release free Options                                 M
*****************************************************************************/
void LineSegmentRelease(IRndrLineSegmentStruct *Seg)
{
    IPFreePolygon(Seg -> Tri);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Begins a new line.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:   IN, OUT, pointer to the line segment.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentStart, begin start                                            M
*****************************************************************************/
void LineSegmentStart(IRndrLineSegmentStruct *Seg)
{
    Seg -> NumVertex = -1;
    Seg -> SharpBend = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the next point for the line.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:      IN, OUT, pointer to the line segment.                          M
*   Vertex:   IN, the new point.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentSet, add point line-to                                        M
*****************************************************************************/
void LineSegmentSet(IRndrLineSegmentStruct *Seg, IrtPtType4 Vertex)
{
    IrtRType Width, Length;
    IrtPtType d;
    IrtPtType4 LastPoint;

    Seg -> NumVertex++;
    if (Seg -> NumVertex == 0) {
        IRIT_PT_COPY4(Seg -> LastPoint, Vertex);
        Seg -> TrianglesNum = 0;
        return;
    }
    IRIT_PT2D_SUB(d, Vertex, Seg -> LastPoint);

    Length = sqrt(IRIT_SQR(d[RNDR_X_AXIS]) + IRIT_SQR(d[RNDR_Y_AXIS]));

    if (IRIT_APX_EQ(Length, 0)) {
        Seg -> TrianglesNum = 0;
        return;                                 /* Segment has zero length.  */
    }
    IRIT_PT2D_SCALE(d, 1 / Length);

    if (Seg -> NumVertex == 1) {
        IRIT_PT_COPY4(LastPoint, Seg -> LastPoint);
        Width = (LastPoint[RNDR_Z_AXIS] - Seg -> PolyOptions.ZFar)
				    * Seg -> k + Seg -> PolyOptions.MinWidth;

        IRIT_PT_COPY4(Seg -> Vertex[2], LastPoint);
        Seg -> Vertex[2][RNDR_X_AXIS] -= Width * d[RNDR_Y_AXIS];
        Seg -> Vertex[2][RNDR_Y_AXIS] += Width * d[RNDR_X_AXIS];

        Seg -> Normal[2][RNDR_X_AXIS] = -d[RNDR_Y_AXIS];
        Seg -> Normal[2][RNDR_Y_AXIS] =  d[RNDR_X_AXIS];
        Seg -> Normal[2][RNDR_Z_AXIS] = -0.5;


        IRIT_PT_COPY4(Seg -> Vertex[3], LastPoint);
        Seg -> Vertex[3][RNDR_X_AXIS] += Width * d[RNDR_Y_AXIS];
        Seg -> Vertex[3][RNDR_Y_AXIS] -= Width * d[RNDR_X_AXIS];

        Seg -> Normal[3][RNDR_X_AXIS] =  d[RNDR_Y_AXIS];
        Seg -> Normal[3][RNDR_Y_AXIS] = -d[RNDR_X_AXIS];
        Seg -> Normal[3][RNDR_Z_AXIS] = -0.5;

        Seg -> TrianglesNum = 0;
        /* Don't return triangle. */
    }
    else {
        IrtVecType Dir;

        IRIT_PT_COPY4(LastPoint, Seg -> LastPoint);

	if (Seg -> SharpBend) {
	    Seg -> SharpBend = FALSE;
	    IRIT_PT_COPY4(Seg -> Vertex[0], Seg -> Vertex[3]);
	    IRIT_PT_COPY4(Seg -> Vertex[1], Seg -> Vertex[2]);

	    IRIT_VEC_COPY(Seg -> Normal[0], Seg -> Normal[3]);
	    IRIT_VEC_COPY(Seg -> Normal[1], Seg -> Normal[2]);
	}
	else {
	    IRIT_PT_COPY4(Seg -> Vertex[0], Seg -> Vertex[2]);
	    IRIT_PT_COPY4(Seg -> Vertex[1], Seg -> Vertex[3]);

	    IRIT_VEC_COPY(Seg -> Normal[0], Seg -> Normal[2]);
	    IRIT_VEC_COPY(Seg -> Normal[1], Seg -> Normal[3]);
	}

        Width = (LastPoint[RNDR_Z_AXIS] - Seg -> PolyOptions.ZFar)
			            * Seg -> k + Seg -> PolyOptions.MinWidth;

	if (IRIT_DOT_PROD_2D(Seg -> LastDelta, d) > 0.0) {
	    IRIT_VEC2D_ADD(Dir, Seg -> LastDelta, d);
	}
	else {
	    Seg -> SharpBend = TRUE;

	    IRIT_VEC2D_SUB(Dir, Seg -> LastDelta, d);
	}
	IRIT_VEC2D_NORMALIZE(Dir);

        IRIT_PT_COPY4(Seg -> Vertex[2], LastPoint);
        Seg -> Vertex[2][RNDR_X_AXIS] -= Width * Dir[RNDR_Y_AXIS];
        Seg -> Vertex[2][RNDR_Y_AXIS] += Width * Dir[RNDR_X_AXIS];

        IRIT_PT_COPY4(Seg -> Vertex[3], LastPoint);
        Seg -> Vertex[3][RNDR_X_AXIS] += Width * Dir[RNDR_Y_AXIS];
        Seg -> Vertex[3][RNDR_Y_AXIS] -= Width * Dir[RNDR_X_AXIS];

	IRIT_VEC_RESET(Seg -> Normal[2]);
	Seg -> Normal[2][RNDR_X_AXIS] -= Dir[RNDR_Y_AXIS];
        Seg -> Normal[2][RNDR_Y_AXIS] += Dir[RNDR_X_AXIS];
        Seg -> Normal[2][RNDR_Z_AXIS] = -0.5;

	IRIT_VEC_RESET(Seg -> Normal[3]);
	Seg -> Normal[3][RNDR_X_AXIS] += Dir[RNDR_Y_AXIS];
        Seg -> Normal[3][RNDR_Y_AXIS] -= Dir[RNDR_X_AXIS];
        Seg -> Normal[3][RNDR_Z_AXIS] = -0.5;

        /* Two new triangles can be returned. */
        Seg -> TrianglesNum = 2;
    }

    IRIT_PT_COPY4(Seg -> LastPoint, Vertex);
    IRIT_PT_COPY(Seg -> LastDelta, d);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Ends a line. Should be called when after the last point was added.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:   IN, OUT, pointer to the line segment.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentEnd, line termination                                         M
*****************************************************************************/
void LineSegmentEnd(IRndrLineSegmentStruct *Seg)
{
    /* We simulate as if there is a new vertex in the tangent direction. */
    IrtPtType4 Pt;

    IRIT_PT_COPY4(Pt, Seg -> LastPoint);
    Pt[0] += Seg -> LastDelta[0];
    Pt[1] += Seg -> LastDelta[1];
    LineSegmentSet(Seg, Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieves the triangles compromising the current segment.                M
*                                                                            *
* PARAMETERS:                                                                M
*   Seg:      IN, OUT, pointer to the line segment.                          M
*   NumTri:   IN, the number of triangle, should be < TrianglesNum.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The triangle.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineSegmentGetTri, triangulation                                         M
*****************************************************************************/
IPPolygonStruct *LineSegmentGetTri(IRndrLineSegmentStruct *Seg, int NumTri)
{
    int Map[3], i;

    if (NumTri >= Seg -> TrianglesNum) {
        return NULL;
    }

    if (NumTri < 2) {
	Map[0] = 0;
	Map[1] = NumTri == 0 ? 1 : 3;
	Map[2] = NumTri == 0 ? 3 : 2;
    }

    for (i = 0; i < 3; i++) {
        IRIT_PT_COPY(Seg -> TriVertex[i] -> Coord, Seg -> Vertex[Map[i]]);
        IRIT_VEC_COPY(Seg -> TriVertex[i] -> Normal, Seg -> Normal[Map[i]]);
	IRIT_VEC_NORMALIZE(Seg -> TriVertex[i] -> Normal);

        AttrSetRealAttrib(&Seg -> TriVertex[i] ->  Attr,
			  "_1/W", Seg -> Vertex[Map[i]][3]);
    }

    return Seg -> Tri;
}
