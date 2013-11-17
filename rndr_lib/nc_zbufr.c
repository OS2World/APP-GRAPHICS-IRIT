/*****************************************************************************
*    Z buffer for NC applications.                                           *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber		          Ver 0.4, Sep. 2006         *
*****************************************************************************/

#include "rndr_loc.h"
#include "rndr_lib.h"
#include "zbuffer.h"

typedef enum INCModeType {
    INC_MODE_NONE,
    INC_MODE_OBJ,
    INC_MODE_PLL,
    INC_MODE_LIGHT
} INCModeType;

typedef struct INCZBufferStruct {
    IRndrZBufferStruct ZBuf;
    IRndrSceneStruct Scene;
    IRndrObjectStruct Obj;
    IRndrTriangleStruct Tri;
    IRndrLineSegmentStruct Seg;
    INCModeType Mode;
    IrtRType ClbkZ, ZPixelsRemoved;
    int GridSizeX, GridSizeY;
    int ActiveRegionXMin, ActiveRegionYMin,
        ActiveRegionXMax, ActiveRegionYMax;
} INCZBufferStruct;

static void INCRndrResetPixelsActiveDomain(INCZBufferPtrType Rend);
static int INCRndrGetPixelsActiveDomain(INCZBufferPtrType Rend,
					int *XMin,
					int *YMin,
					int *XMax,
					int *YMax,
					IrtRType *ZPixelsRemoved);
static void INCRndrUpdateActiveDomainPreClbk(int x,
					     int y,
					     IRndrColorType Color,
					     IrtRType Z,
					     VoidPtr ClbkData);
static void INCRndrUpdateActiveDomainPostClbk(int x,
					      int y,
					      IRndrColorType Color,
					      IrtRType Z,
					      VoidPtr ClbkData);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new NC (Numerically controled) Rendering context, and returns  M
* a handle to it.						             M
*   Specified are the required sizes of the ZBuffer, the grid size to impose M
* over the XY ZBuffer, and the stock dimensions.  The grid will be used to   M
* efficiently fetch ZBuffer data at the granularity of grid cells instead of M
* the entire ZBuffer every time.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ZBufSizeX:          IN, the width of the z-buffer, in pixels.            M
*   ZBufSizeY:          IN, the height of the z-buffer, in pixels.           M
*   GridSizeX:          IN, the grid X size to divide the z-buffer into.     M
*   GridSizeY:          IN, the grid Y size to divide the z-buffer into.     M
*   XYZMin:             IN, the minimum corner of volume to consider.        M
*   XYZMax:             IN, the maximum corner of volume to consider.        M
*   BottomMaxZ:         IN, TRUE if bottom is maximal Z value, FALSE if      M
*			    bottom should capture minimal Z values.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   INCZBufferPtrType: A handle to the newly created NC z-buffer.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrInitialize, create, initialize, z-buffer                          M
*****************************************************************************/
INCZBufferPtrType INCRndrInitialize(int ZBufSizeX,
				    int ZBufSizeY,
				    int GridSizeX,
				    int GridSizeY,
				    IrtPtType XYZMin,
				    IrtPtType XYZMax,
				    int BottomMaxZ)
{
    int i;
    INCZBufferPtrType Rend;
    IrtHmgnMatType Mat;
    IRndrStencilCfgStruct *SCfg;

    Rend = RNDR_MALLOC(INCZBufferStruct, 1);
    Rend -> Scene.SizeX = ZBufSizeX;
    Rend -> Scene.SizeY = ZBufSizeY;
    Rend -> Scene.ShadeModel = IRNDR_SHADING_NONE;
    Rend -> Scene.BackFace = FALSE;
    Rend -> GridSizeX = GridSizeX;
    Rend -> GridSizeY = GridSizeY;
    Rend -> ActiveRegionXMin = Rend -> ActiveRegionYMin = IRIT_MAX_INT;
    Rend -> ActiveRegionXMax = Rend -> ActiveRegionYMax = -1;
    Rend -> ZPixelsRemoved = 0.0;
    SceneSetMatrices(&Rend -> Scene, NULL, NULL, NULL);

    /* All the initalizations are done once and for all. */
    ZBufferInit(&Rend -> ZBuf, &Rend -> Scene, 1, 0);
    Rend -> ZBuf.UseTransparency = FALSE;

    LineSegmentInit(&Rend -> Seg, NULL);
    LightListInitEmpty(&Rend -> Scene.Lights);
    TriangleInit(&Rend -> Tri);
    ObjectInit(&Rend -> Obj);
    Rend -> Mode = INC_MODE_NONE;

    /* Essentially disabling the stencil buffer. */
    SCfg = &Rend -> ZBuf.StencilCfg;
    SCfg -> SCmp = IRNDR_STENCIL_ALWAYS;
    SCfg -> Ref = 0;
    SCfg -> Mask = 0;
    SCfg -> OpFail = IRNDR_STENCIL_KEEP;
    SCfg -> OpZFail = IRNDR_STENCIL_KEEP;
    SCfg -> OpZPass = IRNDR_STENCIL_KEEP;

    /* We actually in need of keeping the smallest Z values. */
    if (BottomMaxZ) {
        ZBufferClearDepth(&Rend -> ZBuf, (IRndrZDepthType) XYZMax[2]);
	Rend -> ZBuf.ZBufCmp = IRNDR_ZBUFFER_LESS;
    }
    else {
        ZBufferClearDepth(&Rend -> ZBuf, (IRndrZDepthType) XYZMin[2]);
	Rend -> ZBuf.ZBufCmp = IRNDR_ZBUFFER_GREATER;
    }

    /* We need to record any instance of Zbuffer update here. */
    Rend -> ZBuf.PreZCmpClbk = INCRndrUpdateActiveDomainPreClbk;
    Rend -> ZBuf.ZPassClbk = INCRndrUpdateActiveDomainPostClbk;

    SceneSetZClippingPlanes(&Rend -> Scene, XYZMin[2], XYZMax[2]);

    /* Map the specified domain to [-1, +1] in X and Y. */
    MatGenUnitMat(Mat);
    Mat[0][0] = 2.0 / (XYZMax[0] - XYZMin[0]);
    Mat[1][1] = 2.0 / (XYZMax[1] - XYZMin[1]);

    Mat[0][3] = -Mat[0][0] * XYZMin[0] - 1.0;
    Mat[1][3] = -Mat[1][1] * XYZMin[1] - 1.0;
    SceneSetMatrices(&Rend -> Scene, Mat, NULL, NULL);

    /* Coerce the boundary of the Z buffer to go back to bbox min height. */
    for (i = 0; i < ZBufSizeX; i++) {
        INCRndrPutPixel(Rend, i, 0, XYZMin[2]);
        INCRndrPutPixel(Rend, i, ZBufSizeY - 1, XYZMin[2]);
    }
    for (i = 1; i < ZBufSizeY; i++) {
        INCRndrPutPixel(Rend, 0, i, XYZMin[2]);
        INCRndrPutPixel(Rend, ZBufSizeX - 1, i, XYZMin[2]);
    }

    return Rend;
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
*   INCRndrSetZCmp, comparison, z-buffer, sort                               M
*****************************************************************************/
IRndrZBufferCmpType INCRndrSetZCmp(INCZBufferPtrType Rend,
				   IRndrZBufferCmpType ZCmp)
{
    IRndrZBufferStruct
        *ZBuffer = &Rend -> ZBuf;
    IRndrZBufferCmpType
	OldZCmp = ZBuffer -> ZBufCmp;

    ZBuffer -> ZBufCmp = ZCmp;

    return OldZCmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Dispose of a the rendering context.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN,OUT, the rendering context.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrDestroy, destroy, dispose, free, release                          M
*****************************************************************************/
void INCRndrDestroy(INCZBufferPtrType Rend)
{
    ZBufferRelease(&Rend -> ZBuf);
    LineSegmentRelease(&Rend -> Seg);
    SceneRelease(&Rend -> Scene);
    TriangleRelease(&Rend -> Tri);
    ObjectRelease(&Rend -> Obj);

    RNDR_FREE(Rend);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the Irit object to be scan converted.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:       IN, OUT, the rendering context.                              M
*   Object:     IN, the object to be scanned.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrBeginObject, scan irit object, z-buffer                           M
*****************************************************************************/
void INCRndrBeginObject(INCZBufferPtrType Rend, IPObjectStruct *Object)
{
    Rend -> Obj.noShade = TRUE;

    if (IP_IS_POLYLINE_OBJ(Object)) {
        AttrSetObjectIntAttrib(Object, "_TRANSFORMED", TRUE);
        Rend -> Obj.Transformed = TRUE;
        Rend -> Mode = INC_MODE_PLL;
    }
    else {
        Rend -> Mode = INC_MODE_OBJ;
    }
    ObjectSet(&Rend -> Obj, Object, &Rend -> Scene);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan converts a triangle polygon.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   Triangle:  IN, the triangle to be scanned.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrPutTriangle, scan triangle, polygon z-buffer                      M
*****************************************************************************/
void INCRndrPutTriangle(INCZBufferPtrType Rend,
			IPPolygonStruct *Triangle)
{
    if (IPVrtxListLen(Triangle -> PVertex) != 3)
	return;

    if (Rend -> Mode != INC_MODE_OBJ) {
        _IRndrReportError(IRIT_EXP_STR("IRndrPutTriangle() not during object scan.\n"));
    }

    if (TriangleSet(&Rend -> Tri, Triangle, &Rend -> Obj, &Rend -> Scene)) {
        ZBufferScanTri(&Rend -> ZBuf, &Rend -> Tri, Rend);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan convert a depth Mask into the Zbuffer.  The Mask is a 2D square of  M
* depth values that is centered around Pos.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   PosXY:     IN, XY location where to place the center of the mask in the  M
*		   Z buffer.						     M
*   PosZ:      IN, The depth to place the mask at.			     M
*   Mask:      IN, The 2D square array of depth values.                      M
*   MaskXSize: IN, X size of Mask.                                           M
*   MaskYSize: IN, Y size of Mask.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Amount of material removed by this call in Pixels^2*Z        M
*		volume units.	                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrPutMask, scan mask, polygon z-buffer                              M
*****************************************************************************/
IrtRType INCRndrPutMask(INCZBufferPtrType Rend,
			int *PosXY,
			IrtRType PosZ,
			IrtRType *Mask,
			int MaskXSize,
			int MaskYSize)
{
    IRIT_STATIC_DATA int
	LastPosXY[2] = { -1, -1 };
    IRIT_STATIC_DATA IrtRType
	LastPosZ = IRIT_INFNTY;
    int x, x1, x2, y, y1, y2;
    IrtRType
	Volume = 0.0;
    IRndrZBufferStruct
        *ZBuffer = &Rend -> ZBuf;

    if (IRIT_APX_EQ(LastPosZ, PosZ) &&
	LastPosXY[0] == PosXY[0] &&
	LastPosXY[1] == PosXY[1])
	return 0.0;
    LastPosZ = PosZ;
    LastPosXY[0] = PosXY[0];
    LastPosXY[1] = PosXY[1];

    x1 = PosXY[0] - MaskXSize / 2;
    x2 = x1 + MaskXSize - 1;
    y1 = PosXY[1] - MaskYSize / 2;
    y2 = y1 + MaskYSize - 1;

    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {
	    if (x < 0 ||
		x >= ZBuffer -> SizeX ||
		y < 0 ||
		y >= ZBuffer -> SizeY)
	        Mask++;  /* Skip the location that is outside the Z Buffer. */
	    else {
	        IRndrZListStruct
		    *CurrZ = &ZBuffer -> z[y][x];

		if (*Mask++ != IRIT_INFNTY &&
		    CurrZ -> First.z > PosZ + Mask[-1]) {
		    Volume += CurrZ -> First.z - (PosZ + Mask[-1]);

		    if (Rend -> ActiveRegionXMin > x)
		        Rend -> ActiveRegionXMin = x;
		    if (Rend -> ActiveRegionXMax < x)
		        Rend -> ActiveRegionXMax = x;

		    if (Rend -> ActiveRegionYMin > y)
		        Rend -> ActiveRegionYMin = y;
		    if (Rend -> ActiveRegionYMax < y)
		        Rend -> ActiveRegionYMax = y;

		    CurrZ -> First.z = (IRndrZDepthType) (PosZ + Mask[-1]);
		}
	    }
	}
    }

    Rend -> ZPixelsRemoved += Volume;

    return Volume;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Marks the end of  the object scaning.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend: IN, OUT, the rendering context.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrEndObject, scan, irit object, z-buffer                            M
*****************************************************************************/
void INCRndrEndObject(INCZBufferPtrType Rend)
{
    Rend -> Mode = INC_MODE_NONE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Manually adds a single pixel.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*   x:     IN, the column number.                                            M
*   y:     IN, the line number.                                              M
*   z:     IN, the pixel's depth.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrPutPixel, put pixel                                               M
*****************************************************************************/
void INCRndrPutPixel(INCZBufferPtrType Rend, int x, int y, IrtRType z)
{
    IRIT_STATIC_DATA IRndrColorType
	Color = { 0.0, 0.0, 0.0, 0.0 };

    assert(x >= 0 && x < Rend -> Scene.SizeX &&
	   y >= 0 && y < Rend -> Scene.SizeY);

    ZBufferPutPixel(&Rend -> ZBuf, x, y, z, 0.0, Color, NULL, Rend);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve a pixel's depth from the z-buffer.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend: IN,OUT, the rendering context.                                     M
*   x:    IN, the column number.                                             M
*   y:    IN, the line number.                                               M
*   Result: OUT, the user allocated buffer to hold the result.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrGetPixelDepth, z-buffer, line depth                               M
*****************************************************************************/
void INCRndrGetPixelDepth(INCZBufferPtrType Rend,
			  int x,
			  int y,
			  IrtRType *Result)
{
    assert(x >= 0 && x < Rend -> Scene.SizeX &&
	   y >= 0 && y < Rend -> Scene.SizeY);

    ZBufferGetLineDepth(&Rend -> ZBuf, x, x, y, Result);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve z-coordinate data from the z-buffer.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT, the rendering context.                                  M
*   x1, x2: IN, the x-range to fetch along the line.                         M
*   y:      IN, the line number.                                             M
*   ZValues: OUT, a user allocated buffer to hold the result.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrGetLineDepth, z-buffer, line Z information                        M
*****************************************************************************/
void INCRndrGetLineDepth(INCZBufferPtrType Rend,
			 int x1,
			 int x2,
			 int y,
			 IrtRType *ZValues)
{
    assert(x1 >= 0 && x1 <= x2 && x2 < Rend -> Scene.SizeX &&
	   y >= 0 && y < Rend -> Scene.SizeY);

    ZBufferGetLineDepth(&Rend -> ZBuf, x1, x2, y, ZValues);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the grid size and maximal cell size, in pixels of the z-buffer.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:           		IN, OUT, the rendering context.              M
*   GridSizeX, GridSizeY:	OUT, the grid dimensions.		     M
*   GridCellXSize, GridCellYSize:  OUT, cell sizes.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrGetZbufferGridCellMaxSize, z-buffer                               M
*****************************************************************************/
void INCRndrGetZbufferGridCellMaxSize(INCZBufferPtrType Rend,
				      int *GridSizeX,
				      int *GridSizeY,
				      int *GridCellXSize,
				      int *GridCellYSize)
{
    *GridCellXSize = 2 + Rend -> Scene.SizeX / (*GridSizeX = Rend -> GridSizeX);
    *GridCellYSize = 2 + Rend -> Scene.SizeY / (*GridSizeY = Rend -> GridSizeY);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetches the z-buffer regions under the requested grid cell.              M
* Returned is a linear buffer of as many pixels in the requested grid cell,  M
* ordered by rows.  See INCRndrGetZbufferGridCellMaxSize to get maximal grid M
* cell size.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:       IN, OUT, the rendering context.                              M
*   GridCellX, GridCellY:  IN, the grid cell to fetch its Z values.          M
*   ZValues:    OUT, a user allocated buffer to hold the result.             M
*   XMin, YMin: OUT, minimal dimensions of fetched grid cell.		     M
*   XMax, YMax: OUT, maximal dimensions of fetched grid cell.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if succesful, FALSE otherwise (out of grid range).            M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrGetZbufferGridCell, z-buffer                                      M
*****************************************************************************/
int INCRndrGetZbufferGridCell(INCZBufferPtrType Rend,
			      int GridCellX,
			      int GridCellY,
			      IrtRType *ZValues,
			      int *XMin,
			      int *YMin,
			      int *XMax,
			      int *YMax)
{
    int y;
    IrtRType *Z;

    if (GridCellX < 0 ||
	GridCellX >= Rend -> GridSizeX ||
	GridCellY < 0 ||
	GridCellY >= Rend -> GridSizeY)
        return FALSE;

    /* Compute the dimensions of this grid cell, in pixels. */
    *XMin = GridCellX * Rend -> Scene.SizeX / Rend -> GridSizeX;
    *YMin = GridCellY * Rend -> Scene.SizeY / Rend -> GridSizeY;

    if (GridCellX == Rend -> GridSizeX - 1)
        *XMax = Rend -> Scene.SizeX - 1;
    else
        *XMax = *XMin + Rend -> Scene.SizeX / Rend -> GridSizeX;

    if (GridCellY == Rend -> GridSizeY - 1)
        *YMax = Rend -> Scene.SizeY - 1;
    else
        *YMax = *YMin + Rend -> Scene.SizeY / Rend -> GridSizeY;

    for (y = *YMin, Z = ZValues; y <= *YMax; y++) {
	INCRndrGetLineDepth(Rend, *XMin, *XMax, y, Z);
	Z += *XMax - *XMin + 1;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Maps the given pixel coordinates to indices of the cell that contains    M
* these pixel coordinates.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN,OUT, the rendering context.                                    M
*   X, Y:  IN,OUT, pixel coordinates to map to the cell indices they are in. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if we found the right cell.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrMapPixelsToCells                                                  M
*****************************************************************************/
int INCRndrMapPixelsToCells(INCZBufferPtrType Rend, int *X, int *Y)
{
    IrtRType
	CellSizeX = Rend -> Scene.SizeX / Rend -> GridSizeX,
	CellSizeY = Rend -> Scene.SizeY / Rend -> GridSizeY;

    if (*X < 0 ||
	*X >= Rend -> Scene.SizeX ||
	*Y < 0 ||
	*Y >= Rend -> Scene.SizeY) {
        return FALSE;
    }

    *X = (int) (*X / CellSizeX);
    *Y = (int) (*Y / CellSizeY);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the active cells - a rectangular (XMin, YMin) :; (XMax, YMax)       M
* indices of cells where the Z-buffer was updated in, since the last call.   M
*   This function also resets all cells to inactive state		     M 
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:                 IN,OUT, the rendering context.                     M
*   MinCellX, MinCellY:   OUT, Minimum indices of active cells.              M
*   MaxCellX, MaxCellY:   OUT, Maximum indices of active cells.              M
*   ZPixelsRemoved:	  OUT, number of pixels removed since last call.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if we do have active cells.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   INCRndrGetActiveCells                                                    M
*****************************************************************************/
int INCRndrGetActiveCells(INCZBufferPtrType Rend,
			  int *MinCellX,
			  int *MinCellY,
			  int *MaxCellX,
			  int *MaxCellY,
			  IrtRType *ZPixelsRemoved)
{
    int PixelXMin, PixelYMin, PixelXMax, PixelYMax;

    INCRndrGetPixelsActiveDomain(Rend, &PixelXMin, &PixelYMin,
				 &PixelXMax, &PixelYMax, ZPixelsRemoved);

    /* Map the pixel coordinates to cell indices, playing its safe by      */
    /* expanding the active region by 1 pixel in all directions.	   */
    PixelXMin = IRIT_MAX(PixelXMin - 1, 0);
    PixelYMin = IRIT_MAX(PixelYMin - 1, 0);
    INCRndrMapPixelsToCells(Rend, &PixelXMin, &PixelYMin);
    *MinCellX = PixelXMin;
    *MinCellY = PixelYMin;

    PixelXMax = IRIT_MIN(PixelXMax + 1, Rend -> Scene.SizeX - 1);
    PixelYMax = IRIT_MIN(PixelYMax + 1, Rend -> Scene.SizeY - 1);
    INCRndrMapPixelsToCells(Rend, &PixelXMax, &PixelYMax);
    *MaxCellX = PixelXMax;
    *MaxCellY = PixelYMax;

    /* Render inactive all cells. */
    INCRndrResetPixelsActiveDomain(Rend);

    return (*MinCellX <= *MaxCellX) && (*MinCellY <= *MaxCellY);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Resets the active domain - a rectangular (XMin, YMin) :; (XMax, YMax)    *
* domain where the Z-buffer is updated in.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:    IN,OUT, the rendering context.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void INCRndrResetPixelsActiveDomain(INCZBufferPtrType Rend)
{
    Rend -> ActiveRegionXMin = Rend -> ActiveRegionYMin = IRIT_MAX_INT;
    Rend -> ActiveRegionXMax = Rend -> ActiveRegionYMax = -IRIT_MAX_INT;
    Rend -> ZPixelsRemoved = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets the active domain - a rectangular (XMin, YMin) :; (XMax, YMax)      *
* domain where the Z-buffer was updated in, since the last reset.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Rend:                    IN,OUT, the rendering context.                  *
*   XMin, YMin, XMax, YMax:  OUT, the rectangular bbox that was modified.    *
*   ZPixelsRemoved:	     OUT, number of pixels removed since last call.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if a valid, FALSE if not (no update since last reset).     *
*****************************************************************************/
static int INCRndrGetPixelsActiveDomain(INCZBufferPtrType Rend,
					int *XMin,
					int *YMin,
					int *XMax,
					int *YMax,
					IrtRType *ZPixelsRemoved)
{
    *XMin = Rend -> ActiveRegionXMin;
    *YMin = Rend -> ActiveRegionYMin;
    *XMax = Rend -> ActiveRegionXMax;
    *YMax = Rend -> ActiveRegionYMax;

    *ZPixelsRemoved = Rend -> ZPixelsRemoved;

    return (*XMin <= *XMax) && (*YMin <= *YMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A call back function from inside the Z-buffer on every update that is    *
* made into it, before it is made.  Keeps original Z values.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:  IN, Original location in the Z buffer.                            *
*   Color: IN, Original color - ignored here.				     *
*   Z:     IN, Updated Depth - copied.					     *
*   ClbkData:  IN, Call back data - passed from original code all way back.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void INCRndrUpdateActiveDomainPreClbk(int x,
					     int y,
					     IRndrColorType Color,
					     IrtRType Z,
					     VoidPtr ClbkData)
{
    INCZBufferPtrType
	Rend = (INCZBufferPtrType) ClbkData;

    assert(x >= 0 && x < Rend -> Scene.SizeX &&
	   y >= 0 && y < Rend -> Scene.SizeY);

    Rend -> ClbkZ = Z;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A call back function from inside the Z-buffer on every update that is    *
* made into it, after a pixel was modified.				     *
*   Will capture this info into the active domain.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:  IN, Updated location in the Z buffer.                             *
*   Color: IN, New color - ignored here.				     *
*   Z:     IN, New Depth.						     *
*   ClbkData:  IN, Call back data - passed from original code all way back.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void INCRndrUpdateActiveDomainPostClbk(int x,
					      int y,
					      IRndrColorType Color,
					      IrtRType Z,
					      VoidPtr ClbkData)
{
    INCZBufferPtrType
	Rend = (INCZBufferPtrType) ClbkData;

    assert(x >= 0 && x < Rend -> Scene.SizeX &&
	   y >= 0 && y < Rend -> Scene.SizeY);

    if (Rend -> ActiveRegionXMin > x)
	Rend -> ActiveRegionXMin = x;
    if (Rend -> ActiveRegionXMax < x)
	Rend -> ActiveRegionXMax = x;

    if (Rend -> ActiveRegionYMin > y)
	Rend -> ActiveRegionYMin = y;
    if (Rend -> ActiveRegionYMax < y)
	Rend -> ActiveRegionYMax = y;

    Rend -> ZPixelsRemoved += Rend -> ClbkZ > Z ? Rend -> ClbkZ - Z : 0;
}
