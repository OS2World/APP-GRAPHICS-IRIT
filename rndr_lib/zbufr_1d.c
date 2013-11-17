/*****************************************************************************
*    1D Z buffer applications.	                                             *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber		          Ver 0.4, Apr. 2007         *
*****************************************************************************/

#include "rndr_loc.h"
#include "rndr_lib.h"
#include "zbuffer.h"

typedef struct IRndrZBuffer1DLineStruct {
    IrtRType x1, z1, x2, z2;
} IRndrZBuffer1DLineStruct;

typedef struct IRndrZBuffer1DStruct {
    IrtRType *ZBuf;
    IRndrZBufferCmpType ZBufCmp;
    IrtRType XMin, XMax, Dx;		     /* Real range of X in Zbuffer. */
    int Size;					   /* Width of 1D Z buffer. */
    int *ZBufLnSegIndex;/* Place to keep scan converted ZBuf line indicess. */
    int CurrentLine;	    /* Actual number of lines kept in Lines vector. */
    int MaxNumOfLines;     /* Maximal number of lines we can hold in Lines. */
    IRndrZBuffer1DLineStruct *Lines;     /* Keeps all scan converted lines. */
} IRndrZBuffer1DStruct;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new 1D Zuffer, and returns a handle to it.	             M
*   Specified are the required length of the ZBuffer, and the real 	     M
* dimensions.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZBuf1DSize:         IN, the length of the 1D z-buffer, in pixels.        M
*   XMin, XMax:         IN, the min/maximum real dimension to consider.      M
*   ZMin, ZMax:         IN, the min/maximum depth to consider.		     M
*   BottomMaxZ:         IN, TRUE if bottom is maximal Z value, FALSE if      M
*			    bottom should capture minimal Z values.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrZBuffer1DPtrType: A handle to the newly created 1D z-buffer.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DInitialize, create, initialize, z-buffer                          M
*****************************************************************************/
IRndrZBuffer1DPtrType IRndr1DInitialize(int ZBuf1DSize,
					IrtRType XMin,
					IrtRType XMax,
					IrtRType ZMin,
					IrtRType ZMax,
					int BottomMaxZ)
{
    int i;
    IRndrZBuffer1DPtrType Rend;

    assert(ZBuf1DSize > 1);

    Rend = RNDR_MALLOC(IRndrZBuffer1DStruct, 1);

    Rend -> ZBuf = RNDR_MALLOC(IrtRType, ZBuf1DSize + 1);

    Rend -> XMin = XMin;
    Rend -> XMax = XMax;
    Rend -> Dx = XMax - XMin;

    Rend -> ZBufLnSegIndex = RNDR_MALLOC(int, ZBuf1DSize + 1);
    for (i = 0; i < ZBuf1DSize; i++)
        Rend -> ZBufLnSegIndex[i] = -1;

    Rend -> MaxNumOfLines = 1000;
    Rend -> CurrentLine = 0;
    Rend -> Lines = RNDR_MALLOC(IRndrZBuffer1DLineStruct,
				Rend -> MaxNumOfLines);

    Rend -> Size = ZBuf1DSize;

    IRndr1DClearDepth(Rend, BottomMaxZ ? ZMax : ZMin);
    Rend -> ZBufCmp = BottomMaxZ ? IRNDR_ZBUFFER_LESS
				 : IRNDR_ZBUFFER_GREATER;

    return Rend;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Resets the 1D z-buffer depth.	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:     IN,OUT, the rendering context.                                 M
*   ClearZ:   IN, the new depth to reset the zbuffer to.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void						                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DClearDepth, comparison, z-buffer, sort                            M
*****************************************************************************/
void IRndr1DClearDepth(IRndrZBuffer1DPtrType Rend, IrtRType ClearZ)
{
    int i;

    for (i = 0; i < Rend -> Size; i++)
        Rend -> ZBuf[i] = ClearZ;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the NC z-buffer comparison method.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:    IN,OUT, the rendering context.                                  M
*   ZCmp:    IN, the comparison method.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrZBufferCmpType:  Old comparison method.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DSetZCmp, comparison, z-buffer, sort                               M
*****************************************************************************/
IRndrZBufferCmpType IRndr1DSetZCmp(IRndrZBuffer1DPtrType Rend,
				   IRndrZBufferCmpType ZCmp)
{
    IRndrZBufferCmpType
	OldZCmp = Rend -> ZBufCmp;

    Rend -> ZBufCmp = ZCmp;

    return OldZCmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dispose of the rendering context.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN,OUT, the rendering context.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DDestroy, destroy, dispose, free, release                          M
*****************************************************************************/
void IRndr1DDestroy(IRndrZBuffer1DPtrType Rend)
{
    RNDR_FREE(Rend -> ZBuf);
    RNDR_FREE(Rend -> ZBufLnSegIndex);
    RNDR_FREE(Rend -> Lines);

    RNDR_FREE(Rend);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan converts a polyline.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   Pl:        IN, the polyline to scan convert.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DPutPolyline, scan polyline, z-buffer		                     M
*****************************************************************************/
void IRndr1DPutPolyline(IRndrZBuffer1DPtrType Rend, IPPolygonStruct *Pl)
{
    IPVertexStruct *V;

    if (Pl -> PVertex == NULL || Pl -> PVertex -> Pnext == NULL)
        return;

    for (V = Pl -> PVertex; V -> Pnext != NULL; V = V -> Pnext) {
        IRndr1DPutLine(Rend, V -> Coord[0], V -> Coord[2],
		       V -> Pnext -> Coord[0], V -> Pnext -> Coord[2]);
    }      
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan converts a line.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:              IN, OUT, the rendering context.                       M
*   x1, z1, x2, z2:    IN, the line to scan convert.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DPutLine, scan polyline, z-buffer		                     M
*****************************************************************************/
void IRndr1DPutLine(IRndrZBuffer1DPtrType Rend,
		    IrtRType x1,
		    IrtRType z1,
		    IrtRType x2,
		    IrtRType z2)
{
    IrtRType Dz, z;
    int x,
	Size1 = Rend -> Size - 1,
	x1i = (int) (0.5 + Size1 * (x1 - Rend -> XMin) / Rend -> Dx),
	x2i = (int) (0.5 + Size1 * (x2 - Rend -> XMin) / Rend -> Dx);

    if (x1i > x2i) {
        IRIT_SWAP(int, x1i, x2i);
        IRIT_SWAP(IrtRType, z1, z2);
    }

    if (x1i < x2i)
        Dz = (z2 - z1) / (x2i - x1i);
    else
        Dz = 0.0;
    z = z1;

    /* Make sure we are within bounds. */
    if (x2i < 0 || x1i >= Rend -> Size)
        return; /* We are completely out. */
    if (x1i < 0) {
        z += Dz * -x1i;
	x1i = 0;
    }
    else if (x2i > Rend -> Size)
        x2i = Rend -> Size;

    /* Scan convert. */
    switch (Rend -> ZBufCmp) {
        default:
            assert(0);
	case IRNDR_ZBUFFER_LESS:
	    for (x = x1i; x <= x2i; x++, z += Dz) {
	        if (Rend -> ZBuf[x] > z) {
		    Rend -> ZBuf[x] = z;
		    Rend -> ZBufLnSegIndex[x] = Rend -> CurrentLine;
		}
	    }
	    break;
 	case IRNDR_ZBUFFER_GREATER:
	    for (x = x1i; x <= x2i; x++, z += Dz) {
	        if (Rend -> ZBuf[x] < z) {
		    Rend -> ZBuf[x] = z;
		    Rend -> ZBufLnSegIndex[x] = Rend -> CurrentLine;
		}
	    }
    }

    /* Increase the buffers that keeps all scan converted lines if needs to. */
    if (Rend -> CurrentLine >= Rend -> MaxNumOfLines - 1) {
        Rend -> Lines = (IRndrZBuffer1DLineStruct *)
	    IritRealloc(Rend -> Lines,
		sizeof(IRndrZBuffer1DLineStruct) * Rend -> MaxNumOfLines,
		sizeof(IRndrZBuffer1DLineStruct) * Rend -> MaxNumOfLines * 2);
        Rend -> MaxNumOfLines *= 2;
    }
    Rend -> Lines[Rend -> CurrentLine].x1 = x1;       /* And keep this line. */
    Rend -> Lines[Rend -> CurrentLine].z1 = z1;
    Rend -> Lines[Rend -> CurrentLine].x2 = x2;
    Rend -> Lines[Rend -> CurrentLine++].z2 = z2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Manually adds a single pixel.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*   x:     IN, the column number.                                            M
*   z:     IN, the pixel's depth.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DPutPixel, put pixel                                               M
*****************************************************************************/
void IRndr1DPutPixel(IRndrZBuffer1DPtrType Rend, int x, IrtRType z)
{
    if (x < 0 && x >= Rend -> Size)
	return;

    switch (Rend -> ZBufCmp) {
        default:
            assert(0);
	case IRNDR_ZBUFFER_LESS:
	    if (Rend -> ZBuf[x] > z) {
	        Rend -> ZBuf[x] = z;
		Rend -> ZBufLnSegIndex[x] = Rend -> CurrentLine;
	    }
	    break;
 	case IRNDR_ZBUFFER_GREATER:
	    if (Rend -> ZBuf[x] < z) {
	        Rend -> ZBuf[x] = z;
		Rend -> ZBufLnSegIndex[x] = Rend -> CurrentLine;
	    }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve a pixel's depth from the z-buffer.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend: IN,OUT, the rendering context.                                     M
*   x:    IN, the column number.                                             M
*   Result: OUT, the user allocated buffer to hold the result.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DGetPixelDepth, z-buffer, line depth                               M
*****************************************************************************/
void IRndr1DGetPixelDepth(IRndrZBuffer1DPtrType Rend, int x, IrtRType *Result)
{
    assert(x >= 0 && x < Rend -> Size);

    *Result = Rend -> ZBuf[x];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve z-coordinate data from the z-buffer.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT, the rendering context.                                  M
*   x1, x2: IN, the x-range to fetch along the line.                         M
*   ZValues: OUT, a user allocated buffer to hold the result.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DGetLineDepth, z-buffer, line Z information                        M
*****************************************************************************/
void IRndr1DGetLineDepth(IRndrZBuffer1DPtrType Rend,
			 int x1,
			 int x2,
			 IrtRType *ZValues)
{
    int i, x;

    assert(x1 >= 0 && x1 <= x2 && x2 < Rend -> Size);

    for (x = x1, i = 0; x <= x2; )
        ZValues[i++] = Rend -> ZBuf[x++];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve the z-buffer envelope as one polyline from XMin to XMax.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:         IN, the rendering context.	                             M
*   MergeInters:  If TRUE, adjacent linear segments as detected in the       M
*		  1D Z buffer are merged whenever possible.		     *
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  The retrieved envelope as a polyline.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DUpperEnvAsPolyline, z-buffer, line Z information                  M
*****************************************************************************/
IPPolygonStruct *IRndr1DUpperEnvAsPolyline(IRndrZBuffer1DPtrType Rend,
					   int MergeInters)
{
    int i = 0;
    IrtRType
        Dx = (Rend -> XMax - Rend -> XMin) / (Rend -> Size - 1.0);
    IPVertexStruct
	*V = NULL;
    IPPolygonStruct *Pl;

    while (i < Rend -> Size) {
        int IIdx, j;

        /* Find out how many pixels this line index covers. */
        for (IIdx = Rend -> ZBufLnSegIndex[i], j = i + 1;
	     j < Rend -> Size && Rend -> ZBufLnSegIndex[j] == IIdx;
	     j++);

	if (j-- == i) {
	    /* Only a single pixel for this line segment. */
	    V = IPAllocVertex2(V);
	    V -> Coord[0] = Rend -> XMin + i * Dx;
	    V -> Coord[1] = 0.0;
	    V -> Coord[2] = Rend -> ZBuf[i];
	    AttrSetIntAttrib(&V -> Attr, "_SegIdx", Rend -> ZBufLnSegIndex[j]);
	}
	else {
	    /* Insert two vertices - end location of this segment. */
	    V = IPAllocVertex2(V);
	    V -> Coord[0] = Rend -> XMin + i * Dx;
	    V -> Coord[1] = 0.0;
	    V -> Coord[2] = Rend -> ZBuf[i];
	    AttrSetIntAttrib(&V -> Attr, "_SegIdx", Rend -> ZBufLnSegIndex[i]);

	    V = IPAllocVertex2(V);
	    V -> Coord[0] = Rend -> XMin + j * Dx;
	    V -> Coord[1] = 0.0;
	    V -> Coord[2] = Rend -> ZBuf[j];
	    AttrSetIntAttrib(&V -> Attr, "_SegIdx", Rend -> ZBufLnSegIndex[j]);
	}

	i = j + 1;
    }

    Pl = IPAllocPolygon(0, IPReverseVrtxList2(V), NULL);

    return IRndr1DFilterCollinearEdges(Rend, Pl, MergeInters);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Filters the resulting polyline for collinear edges.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:         IN, the rendering context.                                 M
*   Pl:           IN, the polyline to filer for collinear edges, in place.   M
*   MergeInters:  If TRUE, adjacent linear segments as detected in the       M
*		  1D Z buffer are merged whenever possible.		     *
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  The filtered polyline.		                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndr1DFilterCollinearEdges, z-buffer, line Z information                M
*****************************************************************************/
IPPolygonStruct *IRndr1DFilterCollinearEdges(IRndrZBuffer1DPtrType Rend,
					     IPPolygonStruct *Pl,
					     int MergeInters)
{
    IrtRType
        Dx = (Rend -> XMax - Rend -> XMin) / (Rend -> Size - 1.0);
    IPVertexStruct *V2,
	*V = Pl -> PVertex;

    if (MergeInters && V != NULL) {
        while (V -> Pnext != NULL) {
	    V2 = V -> Pnext;

	    /* If two neighboring points are closer than Z-buffer resolution */
	    /* merge the two points into one.				     */
	    if (IRIT_FABS(V2 -> Coord[0] - V -> Coord[0]) < Dx * 2.0 &&
		IRIT_FABS(V2 -> Coord[2] - V -> Coord[2]) < Dx * 2.0) {
	        V -> Coord[0] = (V -> Coord[0] + V2 -> Coord[0]) * 0.5;
	        V -> Coord[2] = (V -> Coord[2] + V2 -> Coord[2]) * 0.5;
		V -> Pnext = V2 -> Pnext;
		IPFreeVertex(V2);
	    }
	    else
	        V = V2;
	}
    }

    /* Finally remove collinear points. */
    V = Pl -> PVertex;
    if (V != NULL && V -> Pnext != NULL) {
        while (V -> Pnext -> Pnext != NULL) {
	    if (GMCollinear3Pts(V -> Coord, V -> Pnext -> Coord,
				V -> Pnext -> Pnext -> Coord)) {
	        V2 = V -> Pnext;
		V -> Pnext = V2 -> Pnext;
		IPFreeVertex(V2);
	    }
	    else
	        V = V -> Pnext;
	}
    }

    return Pl;
}
