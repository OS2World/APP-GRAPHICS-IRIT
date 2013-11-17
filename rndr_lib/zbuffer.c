/*****************************************************************************
* Z buffer creation,manipulation and access                                  *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "rndr_loc.h"
#include "zbuffer.h"
#include "vis_maps.h"
#include "filt.h"
#include "polyline.h"

#define RNDR_COLOR_QUANTIZE(Clr, n)       ((int) (Clr * n)) / ((IrtRType) n);
#define RNDR_UV_VALUES "uvvals"

static void ZBufferCalcColors(IRndrZBufferStruct *Buffer);
static int ThisLittleEndianHardware(void);
static IRndrZPointStruct *AddPoint(IRndrZBufferStruct *Buffer,
                              int x,
                              int y,
                              IRndrInterpolStruct *i);
static void PolyEdgeIncr(IRndrEdgeStruct *PEdge);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize a newly created z-buffer.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:     IN, OUT, Pointer to the z-buffer.                            M
*   Scene:      IN, Pointer to the related scene object.                     M
*   SuperSize:  IN, Super sampling size.                                     M
*   ColorQuantization:  IN, non zero to quantize the generated colors to     M
*               ColorQuantization levels of colors.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  0 if successfull.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferInit, initialize, create buffer                                   M
*****************************************************************************/
int ZBufferInit(IRndrZBufferStruct *Buffer,
                IRndrSceneStruct *Scene,
                int SuperSize,
                int ColorQuantization)
{
    IRndrZListStruct **z, Dummy;
    int x, y;

    assert(Scene -> SizeX >= 1 && Scene -> SizeY >= 1);

    Dummy.First.Next = NULL;
    Dummy.First.z = (IRndrZDepthType) RNDR_FAREST_Z;
    Dummy.First.Transp = (IRndrZTranspType) 0;
    Dummy.First.Triangle = NULL;

    Buffer -> Scene = Scene;

    Buffer -> TargetSizeX = Scene -> SizeX / SuperSize;
    Buffer -> TargetSizeY = Scene -> SizeY / SuperSize;

    Buffer -> SizeX = Scene -> SizeX;
    Buffer -> SizeY = Scene -> SizeY;

    Buffer -> ColorQuantization = ColorQuantization;

    IRIT_PT_COPY(&Buffer -> BackgroundColor, Scene -> BackgroundColor);

    if (SuperSize > 1) {
        Buffer -> Filter = RNDR_MALLOC(IRndrFilterType, 1);
        Buffer -> Filter -> SuperSize = SuperSize;
        Buffer -> Filter -> Name = NULL;
    }
    else
        Buffer -> Filter = NULL;

    z = RNDR_MALLOC(IRndrZListStruct *, Buffer -> SizeY);
    RNDR_SET_COL_FROM_REAL(Dummy.First.Color, Buffer -> BackgroundColor);

    for (y = 0; y < Buffer -> SizeY; y++) {
        z[y] = RNDR_MALLOC(IRndrZListStruct, Buffer -> SizeX);
        for (x = 0; x < Buffer -> SizeX; x++) {
            memcpy(&z[y][x], &Dummy, sizeof(IRndrZListStruct));
        }
    }
    Buffer -> z = z;
    Buffer -> ColorsValid = FALSE;
    Buffer -> UseTransparency = 0;
    Buffer -> AccessMode = ZBUFFER_ACCESS_FILTERED;
    Buffer -> PointsAlloc =
        FastAllocInit(sizeof(IRndrZPointStruct),
                      sizeof(IRndrZPointStruct) << 10, 2, 0);

    Buffer -> LineColors  = RNDR_MALLOC(IRndrColorType,
                                        Buffer -> TargetSizeX + SuperSize);
    Buffer -> LineAlpha   = RNDR_MALLOC(IrtBType,
                                        Buffer -> TargetSizeX + SuperSize);
    Buffer -> LinePixels  = RNDR_MALLOC(IrtImgPixelStruct,
                                        Buffer -> TargetSizeX + SuperSize);

    Buffer -> ZBufCmp = IRNDR_ZBUFFER_GREATER;

    Buffer -> ZPol = NULL;
    Buffer -> PreZCmpClbk = NULL;
    Buffer -> ZPassClbk = NULL;
    Buffer -> ZFailClbk = NULL;

    Buffer -> ImgSetType = IrtImgWriteSetType;
    Buffer -> ImgOpen = IrtImgWriteOpenFile;
    Buffer -> ImgWriteLine = IrtImgWritePutLine;
    Buffer -> ImgClose = IrtImgWriteCloseFile;

    Buffer -> VisMap = NULL;
    Buffer -> DoVisMapScan = FALSE;
    Buffer -> ScanContinuousDegenTriangleForVisMap = FALSE;
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the context of the z-buffer.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:   IN, OUT, Pointer to the z-buffer.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferClear, clear buffer                                               M
*****************************************************************************/
void ZBufferClear(IRndrZBufferStruct *Buffer)
{
    int x, y;
    IRndrZListStruct Dummy;

    Dummy.Stencil = 0;
    Dummy.First.Next = NULL;
    Dummy.First.z = (IRndrZDepthType) RNDR_FAREST_Z;
    Dummy.First.Transp = (IRndrZTranspType) 0;
    Dummy.First.Triangle = NULL;
    for (y = 0; y < Buffer -> SizeY; y++) {
        for (x = 0; x < Buffer -> SizeX; x++) {
            memcpy(&Buffer -> z[y][x], &Dummy, sizeof(IRndrZListStruct));
        }
    }
    FastAllocDestroy(Buffer -> PointsAlloc);
    Buffer -> PointsAlloc = FastAllocInit(sizeof(IRndrZPointStruct),
                                          sizeof(IRndrZPointStruct) << 10,
                                          2, 0);

    Buffer -> ColorsValid = FALSE;
    IRndrVMClear(Buffer -> VisMap);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Manually adds a single pixel.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:    IN, OUT, pointer to z-buffer.                                 M
*   x:         IN, the column number.                                        M
*   y:         IN, the line number.                                          M
*   z:         IN, the pixel's depth.                                        M
*   Transparency: IN, the pixel's transparency value.                        M
*   Color:     IN, the new color of pixel at (x, y).                         M
*   Triangle:  IN, The triangle which created the added point.               *
*   ClbkData:  IN, data to be transfered to call back functions if any.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferPutPixel, put pixel                                               M
*****************************************************************************/
void ZBufferPutPixel(IRndrZBufferStruct *Buffer,
                     int x,
                     int y,
                     IrtRType z,
                     IrtRType Transparency,
                     IRndrColorType Color,
                     IPPolygonStruct *Triangle,
                     VoidPtr ClbkData)
{
    IRndrZPointStruct
        *Point = NULL;
    IRndrZListStruct *CurrZ;

    assert(x >= 0 && x < Buffer -> SizeX && y >= 0 && y < Buffer -> SizeY);

    if (Buffer -> PreZCmpClbk) {
        IRndrColorType PrevColor;

        CurrZ = &Buffer -> z[y][x];

        RNDR_SET_REAL_FROM_COL(PrevColor, CurrZ -> First.Color);
        Buffer -> PreZCmpClbk(x, y, PrevColor, CurrZ -> First.z, ClbkData);
    }

    if (Buffer -> UseTransparency) {
        IRndrInterpolStruct Interpol;

        Interpol.z = z;
        Interpol.HasColor = FALSE;
        Point = AddPoint(Buffer, x, y, &Interpol);
    }
    else {
        CurrZ = &Buffer -> z[y][x];

        if (Buffer -> ZPol && Buffer -> ZPol(x, y, CurrZ -> First.z, z)) {
            Point = &CurrZ -> First;
        }
        else if (!Buffer -> ZPol) {
            switch (Buffer -> ZBufCmp) {
                case IRNDR_ZBUFFER_NEVER:
                    break;
                case IRNDR_ZBUFFER_LESS:
                    if (z < CurrZ -> First.z)
                        Point = &CurrZ -> First;
                    break;
                case IRNDR_ZBUFFER_LEQUAL:
                    if (z <= CurrZ -> First.z)
                        Point = &CurrZ -> First;
                    break;
                default:
                case IRNDR_ZBUFFER_GREATER:
                    if (z > CurrZ -> First.z)
                        Point = &CurrZ -> First;
                    break;
                case IRNDR_ZBUFFER_GEQUAL:
                    if (z >= CurrZ -> First.z)
                        Point = &CurrZ -> First;
                    break;
                case IRNDR_ZBUFFER_NOTEQUAL:
                    if (z != CurrZ -> First.z)
                        Point = &CurrZ -> First;
                    break;
                case IRNDR_ZBUFFER_ALWAYS:
                    Point = &CurrZ -> First;
                    break;
            }
        }
        else {
            if (Buffer -> ZFailClbk)
                Buffer -> ZFailClbk(x, y, Color, CurrZ -> First.z, ClbkData);
            return;
        }

        if (Buffer -> ZPassClbk)
            Buffer -> ZPassClbk(x, y, Color, CurrZ -> First.z, ClbkData);
    }

    if (Point != NULL) {
        Point -> Transp = (IRndrZTranspType) Transparency;
        Point -> z = (IRndrZDepthType) z;
        Point -> Triangle = Triangle;
        RNDR_SET_COL_FROM_REAL(Point -> Color, Color);
    }

    Buffer -> ColorsValid = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan converts a triangle object into the z-buffer.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:    IN, OUT, pointer to the z-buffer.                             M
*   Tri:       IN, pointer to the Triangle object.                           M
*   ClbkData:  IN, data to be transfered to call back functions if any.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferScanTri, scan convert                                             M
*****************************************************************************/
void ZBufferScanTri(IRndrZBufferStruct *Buffer,
                    IRndrTriangleStruct *Tri,
                    VoidPtr ClbkData)
{
    IRIT_STATIC_DATA IRndrIntensivityStruct
        *Intens[3] = { NULL, NULL, NULL };
    int x, y, i, dx, HasAlphaImage, PrevXStart, PrevXEnd;
    IRndrEdgeStruct
        **Edges = Tri -> SortedEdge;
    IRndrZPointStruct *NewPoint;
    IRndrZListStruct *CurrZ;
    IRndrInterpolStruct DeltaVal, Val, TmpVal;

    Buffer -> ColorsValid = FALSE;
    if (!Intens[0]) { /* Space recycling. */
        for (i = 0; i < 3; i++) {
            Intens[i] = RNDR_MALLOC(IRndrIntensivityStruct,
                                    RNDR_MAX_LIGHTS_NUM);
        }
    }
    DeltaVal.i = Intens[0];
    Val.i = Intens[1];
    TmpVal.i = Intens[2];

    if (!Edges[1]) {
        _IRndrReportError(IRIT_EXP_STR("No right edge in triangle.\n"));
        return;
    }

    PrevXStart = Edges[0] -> x;
    PrevXEnd = Edges[1] -> x;
    /* Start scan conversion. */
    for (y = Tri -> YMin; y <= Tri -> YMax && y < Buffer -> SizeY; y++) {
        if (Edges[2]) {
            if (!RNDR_IN(y, Edges[0] -> YMin,
                         Edges[0] -> YMin + Edges[0] -> dy - 1)) {
                if (Edges[0] -> x == Edges[2] -> x) {
                    Edges[0] = Edges[2];
                }
            }
            if (!RNDR_IN(y, Edges[1] -> YMin,
                         Edges[1] -> YMin + Edges[1] -> dy - 1)) {
                if (Edges[1] -> x == Edges[2] -> x) {
                    Edges[1] = Edges[2];
                }
            }
        }

        /* Scan over dx. */
        if (y >= 0) {
            int XStart = Edges[0] -> x, 
                XEnd = Edges[1] -> x;

            /*   Degen triangle in certain angle may be scanned as dots    */
            /* isntead of lines. In visibility map it's crucial they will  */
            /* be scanned as lines so we make sure they are (Also, in      */
            /* visibility map all the interpolation values are not         */
            /* required and therefore ignored.                             */
            if (Buffer -> ScanContinuousDegenTriangleForVisMap) {
                if (PrevXEnd < XStart - 1) 
                    XStart = PrevXEnd + 1;
                else if (PrevXStart > XEnd + 1) 
                    XEnd = PrevXStart - 1;
            }
            PrevXStart = XStart;
            PrevXEnd = XEnd; 

            /* Edge[0] is the start of interpolation. */
            InterpolCopy(&Val, &Edges[0] -> Value);

            /* Dx is scan line length. */
            dx = XEnd - XStart;
            if (dx < 0) {
                _IRndrReportError(IRIT_EXP_STR("dx < 0, dx = %d\n"), dx);
            }

            InterpolDelta(&DeltaVal, &Edges[1] -> Value, &Edges[0] -> Value, dx);

            HasAlphaImage = Tri -> Object -> Txtr.PrmImage &&
                            Tri -> Object -> Txtr.PrmImage -> Alpha;

            for (x = XStart; x <= XEnd; InterpolIncr(&Val, &DeltaVal), x++) {
                IRndrColorType NewColor;

                if (!RNDR_IN(x, 0, Buffer -> SizeX - 1)) {
                    /* Not inside of Image. */
                    continue;
                }

                /* VisMap section: scan convetion for visibility map. */
                if (Buffer -> DoVisMapScan) {
                    IrtPtType XYZ;
                    /*   Prior to calling this function for the visibility */
                    /* map we have switched xy values with uv values.      */
		    /*   Therefore, XYZ holds the xyz values while x and y */
                    /* holds the uv values.                                */
                    XYZ[0] = Val.u;           /* Actually point's x value. */
                    XYZ[1] = Val.v;           /* Actually point's y value. */
                    XYZ[2] = Val.z;
/* This was visibility map code. It was replaced by another algorithm. */
#ifdef VM_USE_OLD_CODE
                    XYZ[2] = IRIT_MIN(Val.z + Tri -> dz, Tri -> ZMax);
#endif
                    IRndrVMRelocatePtIntoTriangle(Tri -> Poly, XYZ);
                    IRndrVMPutPixel(Buffer -> VisMap, x, y, XYZ, 
                                    Tri -> Validity, Tri -> IsBackFaced, 
                                    Tri -> Poly);
                    continue;
                }

                CurrZ = &Buffer -> z[y][x];

                NewPoint = NULL;
                if (Buffer -> UseTransparency || Buffer -> VisMap) {
                    /* When visiblity map is used the transparency machanizm */
                    /* is used when doing the regular scan (as oppose to UV  */
                    /* scan when Buffer -> DoVisMapScan is TRUE).            */
                    NewPoint = AddPoint(Buffer, x, y, &Val);
                }
                else {
                    if (Buffer -> PreZCmpClbk) {
                        IRndrColorType PrevColor;

                        RNDR_SET_REAL_FROM_COL(PrevColor,
                                               CurrZ -> First.Color);
                        Buffer -> PreZCmpClbk(x, y,
                                              PrevColor, CurrZ -> First.z,
                                              ClbkData);
                    }

                    if (StencilTest(&Buffer -> StencilCfg,
                                    CurrZ -> Stencil)) {
                        if (Buffer -> ZPol) {
                            if (Buffer -> ZPol(x, y, CurrZ -> First.z,
                                               Val.z)) {
                                NewPoint = &CurrZ -> First;
                            }
                        }
                        else {
                            switch (Buffer -> ZBufCmp) {
                                case IRNDR_ZBUFFER_NEVER:
                                    break;
                                case IRNDR_ZBUFFER_LESS:
                                    if (Val.z < CurrZ -> First.z)
                                        NewPoint = &CurrZ -> First;
                                    break;
                                case IRNDR_ZBUFFER_LEQUAL:
                                    if (Val.z <= CurrZ -> First.z)
                                        NewPoint = &CurrZ -> First;
                                    break;
                                default:
                                case IRNDR_ZBUFFER_GREATER:
                                    if (Val.z > CurrZ -> First.z)
                                        NewPoint = &CurrZ -> First;
                                    break;
                                case IRNDR_ZBUFFER_GEQUAL:
                                    if (Val.z >= CurrZ -> First.z)
                                        NewPoint = &CurrZ -> First;
                                    break;
                                case IRNDR_ZBUFFER_NOTEQUAL:
                                    if (Val.z != CurrZ -> First.z)
                                        NewPoint = &CurrZ -> First;
                                    break;
                                case IRNDR_ZBUFFER_ALWAYS:
                                    NewPoint = &CurrZ -> First;
                                    break;
                            }
                        }
                    }
                    else {
                        StencilOpFail(&Buffer -> StencilCfg,
                                      &CurrZ -> Stencil);
                    }
                }

                if (NewPoint) {
                    NewPoint -> Transp =
                                  (IRndrZTranspType) Tri -> Object -> Transp;
                    NewPoint -> z = (IRndrZDepthType) Val.z;
                    NewPoint -> Triangle = Tri -> Poly;

                    InterpolCopy(&TmpVal, &Val);
                    TriangleColorEval(Tri -> Poly, x, y, Tri -> Object,
                                      Buffer -> Scene, &TmpVal, NewColor);

                    /* If image has Alpha - use as transparency factor. */
                    if (HasAlphaImage)
                        NewPoint -> Transp =
                                (IRndrZTranspType) (1.0 - NewColor[3]);

                    RNDR_SET_COL_FROM_REAL(NewPoint -> Color, NewColor);
                    if (!Buffer -> UseTransparency) {
                        StencilOpZPass(&Buffer -> StencilCfg,
                                       &CurrZ -> Stencil);
                        if (Buffer -> ZPassClbk)
                          Buffer -> ZPassClbk(x, y, NewColor,
                                              CurrZ -> First.z,
                                              ClbkData);
                    }
                }
                else if (!Buffer -> UseTransparency) {
                    StencilOpZFail(&Buffer -> StencilCfg,
                                   &CurrZ -> Stencil);
                    if (Buffer -> ZFailClbk) {
                        IRndrColorType PrevColor;

                        RNDR_SET_REAL_FROM_COL(PrevColor,
                                               CurrZ -> First.Color);
                        Buffer -> ZFailClbk(x, y, PrevColor,
                                            CurrZ -> First.z,
                                            ClbkData);
                    }
                }
            } /* End x iteration. */
        } /* End if y >= 0. */

        /* Advance to next line. */
        PolyEdgeIncr(Edges[0]);
        PolyEdgeIncr(Edges[1]);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scan converts a triagle object into the z-buffer, for the visibility map.M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:    IN, OUT, pointer to the z-buffer.                             M
*   Tri:       IN, pointer to the Triangle object.                           M
*   ClbkData:  IN, data to be transfered to call back functions if any.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   ZBufferScanTri                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferScanVMTri                                                         M
*****************************************************************************/
void ZBufferScanVMTri(IRndrZBufferStruct *Buffer,
                      IRndrTriangleStruct *Tri,
                      VoidPtr ClbkData)
{
    Buffer -> DoVisMapScan = TRUE;
    ZBufferScanTri(Buffer, Tri, ClbkData);
    Buffer -> DoVisMapScan = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Release the memory taken by the z-buffer.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:   IN,OUT, pointer to the z-buffer.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferRelease, memory free, release                                     M
*****************************************************************************/
void ZBufferRelease(IRndrZBufferStruct *Buffer)
{
    int i, y;

    for (y = 0; y < Buffer -> SizeY; ++y) {
        RNDR_FREE(Buffer -> z[y]);
    }
    RNDR_FREE(Buffer -> z);

    FastAllocDestroy(Buffer -> PointsAlloc);
    RNDR_FREE(Buffer -> LineColors);
    RNDR_FREE(Buffer -> LineAlpha);
    RNDR_FREE(Buffer -> LinePixels);

    if (Buffer -> Filter != NULL) {
        for (i = 0; i < Buffer -> Filter -> SuperSize; i++)
            RNDR_FREE(Buffer -> Filter -> FilterData[i]);
        RNDR_FREE(Buffer -> Filter -> FilterData);
        RNDR_FREE(Buffer -> Filter);
    }

    IRndrVMRelease(Buffer -> VisMap);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Calulate the final color of all points, using transparncy attributes.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Buffer:       IN, OUT,  The z-buffer.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ZBufferCalcColors(IRndrZBufferStruct *Buffer)
{
    int x, y;
    IRndrColorType c, ResultColor;
    IRndrZPointStruct *p;
    IrtRType t, s;

    if (Buffer -> UseTransparency && Buffer -> ColorsValid == FALSE) {
        for (y = 0; y < Buffer -> SizeY; ++y) {
            for (x = 0; x < Buffer -> SizeX; x++) {
                IRIT_PT_COPY(ResultColor, Buffer -> BackgroundColor);
                /* The first link is always background - it has deepest Z. */
                p = Buffer -> z[y][x].First.Next;
                while (p != NULL) {
                    /* Ignore too close Z values. */
                    if (p -> Next != NULL && IRIT_APX_EQ_EPS(p -> z, p -> Next -> z,
                                                        RNDR_ZBUF_SAME_EPS)) {
                        p = p -> Next;
                        continue;
                    }

                    RNDR_SET_REAL_FROM_COL(c, p -> Color);
                    t = p -> Transp;
                    s = 1 - t;
                    ResultColor[RNDR_RED_CLR]  =
                        s * c[RNDR_RED_CLR] + t * ResultColor[RNDR_RED_CLR];
                    ResultColor[RNDR_GREEN_CLR]  =
                        s * c[RNDR_GREEN_CLR] + t * ResultColor[RNDR_GREEN_CLR];
                    ResultColor[RNDR_BLUE_CLR]  =
                        s * c[RNDR_BLUE_CLR] + t * ResultColor[RNDR_BLUE_CLR];
                    p = p -> Next;
                }
                RNDR_SET_COL_FROM_REAL(Buffer -> z[y][x].First.Color,
                                       ResultColor);
            }
        }
    }
    Buffer -> ColorsValid = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test little vs. big endian style of packing bytes.              *
*   Test is done by placing a none zero byte into the first place of a zero  *
* integer.                                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE/FALSE for little/big endian style.                           *
*****************************************************************************/
static int ThisLittleEndianHardware(void)
{
    IRIT_STATIC_DATA int
        Style = -1;

    if (Style < 0) {
        int i = 0;
        char
            *c = (char *) &i;
    
        *c = 1;

        /* i == 16777216 on HPUX, SUN, SGI etc. */
        /* i == 1 on IBM PC based systems (OS2/Windows NT). */
        Style = i == 1;
    }

    return Style;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets call back functions to set image type, open an image, save a row,   M
* and close the image, for ZBufferSaveFile.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:        IN, OUT, pointer to the z-buffer.                         M
*   ImgSetType:    IN, Image setting file type function                      M
*   ImgOpen:       IN, Function to open an image file.                       M
*   ImgWriteLine:  IN, Function to write one row (Vec of RGB & vec of Alpha).M
*   ImgClose:      IN, Function to close an image file.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferSaveFileCB, save, persist                                         M
*****************************************************************************/
void ZBufferSaveFileCB(IRndrZBufferStruct *Buffer,
                       IRndrImgSetTypeFuncType ImgSetType,
                       IRndrImgOpenFuncType ImgOpen,
                       IRndrImgWriteLineFuncType ImgWriteLine,
                       IRndrImgCloseFuncType ImgClose)
{
    Buffer -> ImgSetType = ImgSetType;
    Buffer -> ImgOpen = ImgOpen;
    Buffer -> ImgWriteLine = ImgWriteLine;
    Buffer -> ImgClose = ImgClose;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saves the context of the z-buffer into a file or to the functions        M
* pointed by ZBufferSaveFileCB.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:        IN, OUT, pointer to the z-buffer.                         M
*   BaseDirectory: IN, the directory where the file is to be saved.          M
*   OutFileName:   IN, the file name.                                        M
*   FileType:      IN, the file type.                                        M
*   DataType:      IN, where to save color/z-depth/stencil data.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferSaveFile, save, persist                                           M
*****************************************************************************/
void ZBufferSaveFile(IRndrZBufferStruct *Buffer,
                     const char *BaseDirectory,
                     const char *OutFileName,
                     const char *FileType,
                     IRndrZBufferDataType DataType)
{
    int x, y, SizeX, SizeY, SuperSize,
        OldAccessMode = Buffer -> AccessMode;
    IRndrColorType Color,
        *Colors = Buffer -> LineColors;
    const char *ImageTypeStr;
    IrtBType
        *Alpha = Buffer -> LineAlpha;
    IrtImgPixelStruct
        *Pixels = Buffer -> LinePixels;

    ImageTypeStr = FileType ? FileType 
                            : (OutFileName ? strrchr(OutFileName, '.') + 1 
                                           : "ppm");
    SuperSize = Buffer -> Filter ? Buffer -> Filter -> SuperSize : 1;
    SizeX = Buffer -> TargetSizeX;
    SizeY = Buffer -> TargetSizeY;
    Buffer -> AccessMode = ZBUFFER_ACCESS_FILTERED;

    if (DataType == ZBUFFER_DATA_COLOR)
        ZBufferCalcColors(Buffer);
    if (Buffer -> ImgSetType(ImageTypeStr) == IRIT_IMAGE_UNKNOWN_TYPE)
        _IRndrReportFatal(IRIT_EXP_STR("Image type \"%s\" is unknown.\n"),
                          ImageTypeStr);
    if (!Buffer -> ImgOpen((const char **) &BaseDirectory, OutFileName,
                           TRUE, SizeX, SizeY))
        _IRndrReportFatal(IRIT_EXP_STR("Failed to open output image file \"%s\".\n"),
                          OutFileName);

    for (y = 0; y < SizeY; y++) {
        if (DataType == ZBUFFER_DATA_COLOR) {
            ZBufferGetLineColorAlpha(Buffer, 0, SizeX - 1, y, Colors);
        }

        for (x = 0; x < SizeX; x++) {
            if (DataType == ZBUFFER_DATA_COLOR) {
                IRIT_PT4D_COPY(&Color, Colors[x]);

                if (Buffer -> ColorQuantization > 0) {
                    int cq = Buffer -> ColorQuantization;

                    Color[RNDR_RED_CLR] =
                        RNDR_COLOR_QUANTIZE(Color[RNDR_RED_CLR], cq);
                    Color[RNDR_GREEN_CLR] =
                        RNDR_COLOR_QUANTIZE(Color[RNDR_GREEN_CLR], cq);
                    Color[RNDR_BLUE_CLR] =
                        RNDR_COLOR_QUANTIZE(Color[RNDR_BLUE_CLR], cq);
                }

                Pixels[x].r = RNDR_REAL_TO_BYTE(Color[RNDR_RED_CLR]);
                Pixels[x].g = RNDR_REAL_TO_BYTE(Color[RNDR_GREEN_CLR]);
                Pixels[x].b = RNDR_REAL_TO_BYTE(Color[RNDR_BLUE_CLR]);
                Alpha[x] = RNDR_REAL_TO_BYTE(Color[RNDR_ALPHA_CLR]);
            }
            else if (DataType == ZBUFFER_DATA_VIS_MAP) {
	        int RetCode;
                IrtRType Res,
                    **FilterData = (Buffer -> Filter) ?
                                Buffer -> Filter -> FilterData : NULL;
                IRndrVisibleValidityType Validity;
  
                Alpha[x] = 255;

		RetCode = IRndrVMGetLine(Buffer -> VisMap, x, x, y,FilterData, 
					 &Res, &Validity);
                assert(RetCode);
                if (Validity != IRNDR_VISMAP_VALID_OK) {
                    const IrtImgPixelStruct *Color;

                    switch (Validity) {
                        case IRNDR_VISMAP_VALID_TANGENT:
                            Color = &RNDR_VISMAP_TANGENT_COLOR;
                            break;
                        case IRNDR_VISMAP_VALID_POOR_AR:
                            Color = &RNDR_VISMAP_POOR_AR_COLOR;
                            break;
                        case IRNDR_VISMAP_VALID_DEGEN:
                            Color = &RNDR_VISMAP_DEGEN_COLOR;
                            break;
                        default:
                            Color = NULL;
                            assert(0);
                            break;
                    }
                    memcpy(&Pixels[x], Color, sizeof(IrtImgPixelStruct));
                }
                else if (Res < 0) { /* IRNDR_VISMAP_FILL_EMPTY */
                    memcpy(&Pixels[x], &RNDR_VISMAP_EMPTY_COLOR, 
                        sizeof(IrtImgPixelStruct));
                }
                else if (Res == 0) { /* IRNDR_VISMAP_FILL_MAPPED */
                    memcpy(&Pixels[x], &RNDR_VISMAP_MAPPED_COLOR, 
                        sizeof(IrtImgPixelStruct));
                }
                else  {
                    /* Visible pixel - dark green color. */
                    Pixels[x].r = (int)(Res * RNDR_VISMAP_VISIBLE_COLOR.r);
                    Pixels[x].g = (int)(Res * RNDR_VISMAP_VISIBLE_COLOR.g);
                    Pixels[x].b = (int)(Res * RNDR_VISMAP_VISIBLE_COLOR.b);
                }
             }

            /* Stencil or depth. */
            else {
                float DataZ;
                char *Bytes;
                int i, DataI,
                    Len = 0;

                if (DataType == ZBUFFER_DATA_ZDEPTH) {
                    DataZ = (float) (Buffer ->
                                      z[y * SuperSize][x * SuperSize].First.z);
                    Bytes = (char *) &DataZ;
                    Len = sizeof(DataZ);
                }
                else {
                    DataI = (int) (Buffer ->
                                      z[y * SuperSize][x * SuperSize].Stencil);
                    Bytes = (char *) &DataI;
                    Len = sizeof(DataI);
                }

                if (ThisLittleEndianHardware()) {
                    i = 0;
                    Pixels[x].r = i < Len ? Bytes[i++] : 0;
                    Pixels[x].g = i < Len ? Bytes[i++] : 0;
                    Pixels[x].b = i < Len ? Bytes[i++] : 0;
                    Alpha[x]    = i < Len ? Bytes[i++] : 0;
                }
                else {
                    i = Len - 1;
                    Pixels[x].r = i >= 0 ? Bytes[i--] : 0;
                    Pixels[x].g = i >= 0 ? Bytes[i--] : 0;
                    Pixels[x].b = i >= 0 ? Bytes[i--] : 0;
                    Alpha[x]    = i >= 0 ? Bytes[i--] : 0;
                }
            }
        }
        Buffer -> ImgWriteLine(Alpha, Pixels);
    }
    Buffer -> AccessMode = OldAccessMode;
    Buffer -> ImgClose();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   For a polygon point on the current scan line, allocates and inserts into *
*   sorted list data structure element describing it. It is used when trans- *
*   parancy mode is on to store information about all polygons owning the    *
*   same point on the scan line.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Buffer:   IN OUT, pointer to the z-buffer.                               *
*   x:        IN, cloumn number.                                             *
*   y:        IN, row number.                                                *
*   i:        IN, interpolation value for the point.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   IRndrZPointStruct * : the newly created point.                           *
*****************************************************************************/
static IRndrZPointStruct *AddPoint(IRndrZBufferStruct *Buffer,
                                   int x,
                                   int y,
                                   IRndrInterpolStruct *i)
{
    IRndrZPointStruct *New,
        *p = NULL;
    IRndrZListStruct *z;

    assert(x >= 0 && x < Buffer -> SizeX && y >= 0 && y < Buffer -> SizeY);

    z = &Buffer -> z[y][x];

    /* Find the place in sorted list. */
    if (Buffer -> ZPol) {
        for (p = &z -> First;
             p -> Next != NULL && Buffer -> ZPol(x, y, p -> Next -> z, i -> z);
             p = p -> Next);
    }
    else {
        int Loop = TRUE;

        for (p = &z -> First; p -> Next != NULL; p = p -> Next) {
            switch (Buffer -> ZBufCmp) {
                case IRNDR_ZBUFFER_NEVER:
                    Loop = FALSE;
                    break;
                case IRNDR_ZBUFFER_LESS:
                    if (i -> z >= p -> Next -> z)
                        Loop = FALSE;
                    break;
                case IRNDR_ZBUFFER_LEQUAL:
                    if (i -> z > p -> Next -> z)
                        Loop = FALSE;
                    break;
                default:
                case IRNDR_ZBUFFER_GREATER:
                    if (i -> z <= p -> Next -> z)
                        Loop = FALSE;
                    break;
                case IRNDR_ZBUFFER_GEQUAL:
                    if (i -> z < p -> Next -> z)
                        Loop = FALSE;
                    break;
                case IRNDR_ZBUFFER_NOTEQUAL:
                    if (i -> z == p -> Next -> z)
                        Loop = FALSE;
                    break;
                case IRNDR_ZBUFFER_ALWAYS:
                    break;
            }
            if (!Loop)
                break;
        }
    }

    New = FastAllocNew(Buffer -> PointsAlloc);
    New -> Next = p -> Next;
    p -> Next = New;
    return New;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieves color information of a specific line.                          M
*   The line should be allocated by the caller.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:   IN, OUT, pointer to the z-buffer                               M
*   x0:   IN, minimal x coordinate.                                          M
*   x1:   IN, maximal x coordinate.                                          M
*   y:    IN, line number.                                                   M
*   Result:  OUT, the color values of the line.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferGetLineColorAlpha, color access                                   M
*****************************************************************************/
void ZBufferGetLineColorAlpha(IRndrZBufferStruct *Buffer,
                              int x0,
                              int x1,
                              int y,
                              IRndrColorType *Result)
{
    int x, SizeX, SizeY;
    IRndrColorType Color;

    assert(x0 >= 0 && x0 < x1 && x1 < Buffer -> SizeX &&
           y >= 0 && y < Buffer -> SizeY);

    ZBufferCalcColors(Buffer);
    if (!Buffer -> Filter || Buffer -> AccessMode == ZBUFFER_ACCESS_RAW) {
        for (x = x0; x <= x1; x++, Result++) {
            RNDR_SET_REAL_FROM_COL(*Result, Buffer -> z[y][x].First.Color);
            if (Buffer -> UseTransparency)
                (*Result)[3] =
                    Buffer -> z[y][x].First.Next == NULL ? 0.0 : 1.0;
            else
                (*Result)[3] =
                    Buffer -> z[y][x].First.z != Buffer -> BackgroundDepth;
        }
    }
    else {
        int SuperSize = Buffer -> Filter -> SuperSize;
        IrtRType
            **Filter = Buffer -> Filter -> FilterData,
            SuperScale = 1.0 / IRIT_SQR(SuperSize);

        for (x = x0; x <= x1; x++, Result++) {
            IRIT_PT4D_RESET(*Result);

            for (SizeY = 0; SizeY < SuperSize; SizeY++) {
                for (SizeX = 0; SizeX < SuperSize; SizeX++) {
                    IRndrZPointStruct
                        *z = &Buffer -> z[y * SuperSize + SizeY]
                                         [x * SuperSize + SizeX].First;

                    RNDR_SET_REAL_FROM_COL(Color, z -> Color);
                    IRIT_PT_SCALE(Color, Filter[SizeY][SizeX]);
                    IRIT_PT_ADD(*Result, *Result, Color);
                    if (Buffer -> UseTransparency)
                        (*Result)[3] += z -> Next == NULL ? 0.0 : 1.0;
                    else
                        (*Result)[3] += z -> z != Buffer -> BackgroundDepth;
                }
            }
            (*Result)[3] *= SuperScale;
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrives z information of a specific line.                               M
*   The line should be allocated by the caller.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:   IN, OUT, pointer to the z-buffer                               M
*   x0:   IN, minimal x coordinate.                                          M
*   x1:   IN, maximal x coordinate.                                          M
*   y:    IN, line number.                                                   M
*   Result:  OUT, the z values of the line.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: whether operation succeded.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferGetLineDepth, z depth access                                      M
*****************************************************************************/
int ZBufferGetLineDepth(IRndrZBufferStruct *Buffer,
                        int x0,
                        int x1,
                        int y,
                        IrtRType *Result)
{
    int x, SizeX, SizeY;

    assert(x0 >= 0 && x0 <= x1 && x1 < Buffer -> SizeX &&
           y >= 0 && y < Buffer -> SizeY);

    if (Buffer -> UseTransparency)
        return 0;

    if (!Buffer -> Filter || Buffer -> AccessMode == ZBUFFER_ACCESS_RAW) {
        for (x = x0; x <= x1; x++, Result++) {
            *Result = Buffer -> z[y][x].First.z;
        }
    }
    else {
        /* Compute average of supersampled points. */
        int SuperSize = Buffer -> Filter -> SuperSize;
        IrtRType
            **Filter = Buffer -> Filter -> FilterData;

        for (x = x0; x <= x1; x++, Result++) {
            *Result = 0;

            for (SizeY = 0; SizeY < SuperSize; SizeY++) {
                for (SizeX = 0; SizeX < SuperSize; SizeX++) {
                    *Result += Buffer -> z[y * SuperSize + SizeY]
                                          [x * SuperSize + SizeX].First.z *
                                                        Filter[SizeY][SizeX];
                }
            }
        }
    }
    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrives stencil information of a specific line.                         M
*   The line should be allocated by the caller.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:   IN, OUT, pointer to the z-buffer.                              M
*   x0:   IN, minimal x coordinate.                                          M
*   x1:   IN, maximal x coordinate.                                          M
*   y:    IN, line number.                                                   M
*   Result:  OUT, the stencil values of the line.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: whether operation succeded.                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferGetLineStencil, stencil access                                    M
*****************************************************************************/
int ZBufferGetLineStencil(IRndrZBufferStruct *Buffer,
                          int x0,
                          int x1,
                          int y,
                          int *Result)
{
    int x, SizeX, SizeY, Stencil;

    assert(x0 >= 0 && x0 < x1 && x1 < Buffer -> SizeX &&
           y >= 0 && y < Buffer -> SizeY);

    if (!Buffer -> Filter || Buffer -> AccessMode == ZBUFFER_ACCESS_RAW) {
        for (x = x0; x <= x1; x++, Result++) {
            *Result = Buffer -> z[y][x].Stencil;
        }
    }
    else {
        /* Compute maximum of super sampled points. */
        int SuperSize = Buffer -> Filter -> SuperSize;

        for (x = x0; x <= x1; x++, Result++) {
            *Result = 0;

            for (SizeY = 0; SizeY < SuperSize; SizeY++) {
                for (SizeX = 0; SizeX < SuperSize; SizeX++) {
                    Stencil =
                        Buffer -> z[y * SuperSize + SizeY]
                                   [x * SuperSize + SizeX].Stencil;
                    if (Stencil > *Result) {
                        *Result = Stencil;
                    }
                }
            }
        }
    }
    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Routine to set the filter before any antialias                          M
*    processing could be done.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:  Pointer to the z-buffer.                                        M
*   FilterName: String representing the filter name.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferSetFilter, antialias, AntialiasLine, Utah filter package          M
*****************************************************************************/
void ZBufferSetFilter(IRndrZBufferStruct *Buffer, char *FilterName)
{
    int i, j, SuperSize;
    IrtRType x, y, r;
    Filt *f;

    if (!FilterName || !Buffer -> Filter )
        return;
    SuperSize = Buffer -> Filter -> SuperSize;

    if (!(f = filt_find(FilterName))) {
        f = filt_find(FilterName = "sinc");
        _IRndrReportWarning(IRIT_EXP_STR("unknown filter name, %s used.\n"),
                            FilterName);
    }
    Buffer -> Filter -> FilterData = RNDR_MALLOC(IrtRType *, SuperSize);
    for (i = 0; i < SuperSize; i++) {
        Buffer -> Filter -> FilterData[i] = RNDR_MALLOC(IrtRType, SuperSize);
    }
    Buffer -> Filter -> TotalWeight = 0;
    if (f -> windowme) {
        f -> supp = 1;
        f = filt_window(f, "hanning");
    }
    r = f -> supp / M_SQRT2;
    for (i = 0; i < SuperSize; i++) {
        y = (i + 1) * 2 * r / (SuperSize + 1) - r;
        for (j = 0; j < SuperSize; j++) {
            x = (j + 1) * 2 * r / (SuperSize + 1) - r;
            Buffer -> Filter -> TotalWeight  +=
                (Buffer -> Filter -> FilterData[i][j] =
                filt_func(f, (sqrt(IRIT_SQR(y) + IRIT_SQR(x)))));
        }
    }
    for (i = 0; i < SuperSize; i++) {
        for (j = 0; j < SuperSize; j++) {
            Buffer -> Filter -> FilterData[i][j] /=
                Buffer -> Filter -> TotalWeight;
            /* Normalize the filter. */
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Routine to clear the z depth information                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:  Pointer to the z-buffer.                                        M
*   ClearZ:  Depth to clear the ZBuffer to.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferClearDepth, Z coordinate, depth, clear                            M
*****************************************************************************/
void ZBufferClearDepth(IRndrZBufferStruct *Buffer, IRndrZDepthType ClearZ)
{
    int x, y;

    for (y = 0; y < Buffer -> SizeY; y++) {
        for (x = 0; x < Buffer -> SizeX; x++) {
           Buffer -> z[y][x].First.z = ClearZ;
        }
    }

    Buffer -> BackgroundDepth = ClearZ;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Routine to clear the Stencil information                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:  Pointer to the z-buffer.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferClearStencil, clear, Stencil                                      M
*****************************************************************************/
void ZBufferClearStencil(IRndrZBufferStruct *Buffer)
{
    int x, y;

    for (y = 0; y < Buffer -> SizeY; y++) {
        for (x = 0; x < Buffer -> SizeX; x++) {
            Buffer -> z[y][x].Stencil = 0;
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Routine to clear the color information.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Buffer:  Pointer to the z-buffer.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ZBufferClearColor, clear, background, color, reset                       M
*****************************************************************************/
void ZBufferClearColor(IRndrZBufferStruct *Buffer)
{
    int x, y;
    IRndrColorType bg;

    IRIT_PT_COPY(bg, Buffer -> BackgroundColor);
    for (y = 0; y < Buffer -> SizeY; y++) {
        for (x = 0; x < Buffer -> SizeX; x++) {
           IRIT_PT_COPY(Buffer -> z[y][x].First.Color, bg);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Increments all interpolation values and coordinate of intersection       *
*   with respect to the next scan line.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   PEdge:    IN, pointer to the polygon edge on the current scan-line.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PolyEdgeIncr(IRndrEdgeStruct *PEdge)
{
    int Sign = IRIT_SIGN(PEdge -> dx),
        Numerator = abs(PEdge -> dx),
        Denumerator = PEdge -> dy;

    InterpolIncr(&PEdge -> Value, &PEdge -> dValue);

    if (Numerator < Denumerator) {                         /* Slope is > 1. */
        PEdge -> Inc += Numerator;
    }
    else {
        if (Denumerator != 0) {
            PEdge -> x += PEdge -> dx / Denumerator;
            PEdge -> Inc += Numerator % Denumerator;
        }
    }

    if (PEdge -> Inc > Denumerator) {
        PEdge -> x += Sign;
        PEdge -> Inc -= Denumerator;
    }
}
