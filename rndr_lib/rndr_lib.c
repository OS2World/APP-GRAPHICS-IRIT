/*****************************************************************************
*    Rendering library implementation.                                       *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "rndr_loc.h"
#include "rndr_lib.h"
#include "zbuffer.h"
#include "vis_maps.h"

typedef enum IRndrModeType {
    INC_MODE_NONE,
    INC_MODE_OBJ,
    INC_MODE_PLL,
    INC_MODE_LIGHT
} IRndrModeType;

typedef struct IRndrStruct {
    IRndrZBufferStruct ZBuf;
    IRndrSceneStruct Scene;
    IRndrObjectStruct Obj;
    IRndrTriangleStruct Tri;
    IRndrLineSegmentStruct Seg;
    IRndrModeType Mode;
    IRndrVMStruct VisMap;
} IRndrStruct;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a new Rendering context, and returns a handle to it.             M
*                                                                            *
* PARAMETERS:                                                                M
*   SizeX:              IN, the width of the z-buffer.                       M
*   SizeY:              IN, the height of the z-buffer.                      M
*   SuperSampSize:      IN, the super-sample size.                           M
*   ColorQuantization:  IN, non zero to quantize the generated colors to     M
*		        ColorQuantization levels of colors.		     M
*   UseTransparency:    IN, whether tarnsparency is on.                      M
*   BackfaceCulling:    IN, whether to use back-face culling.                M
*   BackgrCol:          IN, the background color.                            M
*   AmbientLight:       IN, the abient light factor.                         M
*   VisMap:             IN, TRUE to create a visibility map image.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrPtrType: a handle to the newly created z-buffer.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrInitialize, create, initialize, z-buffer                            M
*****************************************************************************/
IRndrPtrType IRndrInitialize(int SizeX,
                             int SizeY,
                             int SuperSampSize,
                             int ColorQuantization,
                             IrtBType UseTransparency,
                             IrtBType BackfaceCulling,
                             IRndrColorType BackgrCol,
                             IrtRType AmbientLight,
			     int VisMap)
{
    IRndrPtrType Rend;

    Rend = RNDR_MALLOC(IRndrStruct, 1);
    Rend -> Scene.SizeX = SizeX * SuperSampSize;
    Rend -> Scene.SizeY = SizeY * SuperSampSize;
    Rend -> Scene.ShadeModel = IRNDR_SHADING_PHONG;
    Rend -> Scene.BackFace = BackfaceCulling;
    Rend -> Scene.Ambient = AmbientLight;
    IRIT_PT_COPY(&Rend -> Scene.BackgroundColor, BackgrCol);
    SceneSetMatrices(&Rend -> Scene, NULL, NULL, NULL);

    /* VisMap section: sets background color to white. */
    if (VisMap) {
        IRndrColorType 
            White = {1.0, 1.0, 1.0, 1.0};
        
        IRIT_PT_COPY(&Rend -> Scene.BackgroundColor, &White);
    }

    /* All the initalizations are done once and for all. */
    ZBufferInit(&Rend -> ZBuf, &Rend -> Scene, SuperSampSize,
		ColorQuantization);
    Rend -> ZBuf.UseTransparency = UseTransparency;

    LineSegmentInit(&Rend -> Seg, NULL);
    LightListInitEmpty(&Rend -> Scene.Lights);
    TriangleInit(&Rend -> Tri);
    ObjectInit(&Rend -> Obj);
    Rend -> Mode = INC_MODE_NONE;

    IRndrStencilCmpFunc(Rend, IRNDR_STENCIL_ALWAYS, 0, 0);
    IRndrStencilOp(Rend,
		   IRNDR_STENCIL_KEEP, IRNDR_STENCIL_INCR, IRNDR_STENCIL_INCR);

    IRndrClearDepth(Rend, (IRndrZDepthType) RNDR_FAREST_Z);

    return Rend;
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
*   IRndrDestroy, destroy, dispose, free, release                            M
*****************************************************************************/
void IRndrDestroy(IRndrPtrType Rend)
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
*   Clear depth information in the rendering context.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:    IN,OUT, the rendering context.                                  M
*   ClearZ:  IN, Depth to clear the ZBuffer to.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrClearDepth, clear, reset, depth, Z coordinate                       M
*****************************************************************************/
void IRndrClearDepth(IRndrPtrType Rend, IRndrZDepthType ClearZ)
{
    ZBufferClearDepth(&Rend -> ZBuf, ClearZ);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clear stencil information in the rendering context.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN,OUT, the rendering context.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrClearStencil, clear, reset, stencil                                 M
*****************************************************************************/
void IRndrClearStencil(IRndrPtrType Rend)
{
    ZBufferClearStencil(&Rend -> ZBuf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reset color information to the registered background color.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN,OUT, the rendering context.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrClearColor, clear, reset, background, color                         M
*****************************************************************************/
void IRndrClearColor(IRndrPtrType Rend)
{
    ZBufferClearColor(&Rend -> ZBuf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds a new light source.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT the rendering context.                                   M
*   Type:   IN, the light type ( POINT, VECTOR )                             M
*   Where:  IN, the light position.                                          M
*   Color:  IN, the light's color.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrAddLightSource, add light source, list                              M
*****************************************************************************/
void IRndrAddLightSource(IRndrPtrType Rend,
                         IRndrLightType Type,
                         IrtPtType Where,
                         IRndrColorType Color)
{
    IRndrLightStruct
        *Light = RNDR_MALLOC(IRndrLightStruct, 1);
    Light -> Type = Type;
    IRIT_PT_COPY(Light -> Where, Where);
    IRIT_PT_COPY(Light -> Color, Color);
    LightListAdd(&Rend -> Scene.Lights, Light);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Changes the filter, used for antialiasing.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:       IN,OUT, the rendering context.                               M
*   FilterName: IN, the filter's name.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetFilter, Filter, anti-aliasing                                    M
*****************************************************************************/
void IRndrSetFilter(IRndrPtrType Rend, char *FilterName)
{
    ZBufferSetFilter(&Rend -> ZBuf, FilterName);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Changes the shading model.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:        IN, OUT, the rendering context.                             M
*   ShadeModel:  IN, the new shading model (FLAT,GOURAUD,PHONG,NONE).        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrShadingType:  Old shading model.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetShadeModel, shading, light model                                 M
*****************************************************************************/
IRndrShadingType IRndrSetShadeModel(IRndrPtrType Rend,
				    IRndrShadingType ShadeModel)
{
    IRndrShadingType
	OldShadeModel = Rend -> Scene.ShadeModel;

    if (ShadeModel < IRNDR_SHADING_LAST) {
        Rend -> Scene.ShadeModel = ShadeModel;
    }

    return OldShadeModel;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the view and perspective matrices.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:     IN,OUT, the rendering context.                                 M
*   ViewMat:  IN, the view matrix.                                           M
*   PrspMat:  IN, the perspective matrix, NULL if parallel projection.       M
*   ScrnMat:  IN, the mapping to the screen or NULL if scale [-1,+1] to      M
*	      image size.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetViewPrsp, view, perspective, matrix context                      M
*****************************************************************************/
void IRndrSetViewPrsp(IRndrPtrType Rend,
		      IrtHmgnMatType ViewMat,
		      IrtHmgnMatType PrspMat,
		      IrtHmgnMatType ScrnMat)
{
    SceneSetMatrices(&Rend -> Scene, ViewMat, PrspMat, ScrnMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrives the 6 clipping planes, defining the viewing frastrum.           M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:        IN, the rendering context.                                  M
*   ClipPlanes:  OUT, a pointer to the 6 user allocated planes.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetClippingPlanes, clipping, viewing frastrum                       M
*****************************************************************************/
void IRndrGetClippingPlanes(IRndrPtrType Rend, IrtPlnType *ClipPlanes)
{
    int i, j;

    for (i = 0; i < 3; i++)
        for (j = 0; j < 2; j++)
            SceneGetClippingPlane(&Rend -> Scene, i, j, ClipPlanes[2 * i + j]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the near and far XY clipping planes, defining the viewing frustum.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*   ZNear: IN, the (negation of the) z-coordinate of the near clipping plane.M
*   ZFar:  IN, the (negation of the) z-coordinate of the far clipping plane. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetZBounds, clipping, viewing frastrum                              M
*****************************************************************************/
void IRndrSetZBounds(IRndrPtrType Rend, IrtRType ZNear, IrtRType ZFar)
{
    SceneSetZClippingPlanes(&Rend -> Scene, ZNear, ZFar);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the Polyline parameters, used for line drawing.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:         IN,OUT, the rendering context.                             M
*   MinWidth:     IN, the width of the line at Z = Znear.                    M
*   MaxWidth:     IN, the width of the line at Z = Zfar.                     M
*   ZNear, ZFar:  IN, as stated above.usually the expected scene width.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetPllParams, polyline line depth cue                               M
*****************************************************************************/
void IRndrSetPllParams(IRndrPtrType Rend,
                        IrtRType MinWidth,
                        IrtRType MaxWidth,
                        IrtRType ZNear,
                        IrtRType ZFar)
{
    IrtRType
        ScaleFactor = Rend -> Scene.SizeX * 0.5;
    IRndrPolylineOptionsStruct PllOptions;

    PllOptions.MinWidth = MinWidth * ScaleFactor;
    PllOptions.MaxWidth = MaxWidth * ScaleFactor;
    PllOptions.ZNear = ZNear;
    PllOptions.ZFar = ZFar;

    LineSegmentSetOptions(&Rend -> Seg, &PllOptions);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer access mode (original super smapled data or filtered). M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:        IN, OUT, the rendering context.                             M
*   UseRawMode:  IN, whether the access mode is RAW (otherwise filtered).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtBType:  Old raw mode.                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetRawMode, antialising access mode raw                             M
*****************************************************************************/
IrtBType IRndrSetRawMode(IRndrPtrType Rend, IrtBType UseRawMode)
{
    IrtBType
	OldRawMode = Rend -> ZBuf.AccessMode == ZBUFFER_ACCESS_RAW;

    Rend -> ZBuf.AccessMode = UseRawMode ?
        ZBUFFER_ACCESS_RAW : ZBUFFER_ACCESS_FILTERED;

    return OldRawMode;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer comparison function.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:    IN,OUT, the rendering context.                                  M
*   ZCmpPol: IN, the comparison function (linear order).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrZCmpPolicyFuncType:  Old comparison function.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetZCmpPolicy, comparison, z-buffer, sort                           M
*****************************************************************************/
IRndrZCmpPolicyFuncType IRndrSetZCmpPolicy(IRndrPtrType Rend,
					   IRndrZCmpPolicyFuncType ZCmpPol)
{
    IRndrZCmpPolicyFuncType
	OldZCmpPol = Rend -> ZBuf.ZPol;

    Rend -> ZBuf.ZPol = ZCmpPol;

    return OldZCmpPol;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer comparison method.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:    IN,OUT, the rendering context.                                  M
*   ZCmp:    IN, the comparison method.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrZBufferCmpType:  Old comparison method.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetZCmp, comparison, z-buffer, sort                                 M
*****************************************************************************/
IRndrZBufferCmpType IRndrSetZCmp(IRndrPtrType Rend, IRndrZBufferCmpType ZCmp)
{
    IRndrZBufferCmpType
	OldZCmp = Rend -> ZBuf.ZBufCmp;

    Rend -> ZBuf.ZBufCmp = ZCmp;

    return OldZCmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer pre comparison function callback function.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   PixelClbk: IN, the callback function.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IRndrPixelClbkFuncType:  Old callbakc function.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetPreZCmpClbk, comparison pre  callback z-buffer                   M
*****************************************************************************/
IRndrPixelClbkFuncType IRndrSetPreZCmpClbk(IRndrPtrType Rend,
					   IRndrPixelClbkFuncType PixelClbk)
{
    IRndrPixelClbkFuncType
	OldPixelClbk = Rend -> ZBuf.PreZCmpClbk;

    Rend -> ZBuf.PreZCmpClbk = PixelClbk;

    return OldPixelClbk;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer post comparison function callback function.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:                   IN,OUT, the rendering context.                   M
*   ZPassClbk, ZFailClbk:   IN, the callback functions called on             M
*                           success/failure.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSetPostZCmpClbk, comparison, z-buffer, post                         M
*****************************************************************************/
void IRndrSetPostZCmpClbk(IRndrPtrType Rend,
			  IRndrPixelClbkFuncType ZPassClbk,
			  IRndrPixelClbkFuncType ZFailClbk)
{
    Rend -> ZBuf.ZPassClbk = ZPassClbk;
    Rend -> ZBuf.ZFailClbk = ZFailClbk;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer stencil test function.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*   SCmp:  IN, stencil test comparison type.                                 M
*   Ref:   IN, stencil test refernce value.                                  M
*   Mask:  IN, stencil test mask.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrStencilCmpFunc, stencil buffer, OpenGL, glStencilFunc               M
*****************************************************************************/
void IRndrStencilCmpFunc(IRndrPtrType Rend,
			 IRndrStencilCmpType SCmp,
			 int Ref,
			 unsigned Mask)
{
    IRndrStencilCfgStruct
        *SCfg = &Rend -> ZBuf.StencilCfg;

    SCfg -> SCmp = SCmp;
    SCfg -> Ref = Ref;
    SCfg -> Mask = Mask;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the z-buffer stencil operations.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT, the rendering context.                                  M
*   Fail:   IN, stencil test failure operation.                              M
*   ZFail:  IN, Z-test failure operation.                                    M
*   ZPass:  IN, Z-test pass operation.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrStencilOp, stencil buffer, OpenGL, glStencilOp                      M
*****************************************************************************/
void IRndrStencilOp(IRndrPtrType Rend,
                    IRndrStencilOpType Fail,
                    IRndrStencilOpType ZFail,
                    IRndrStencilOpType ZPass)
{
    IRndrStencilCfgStruct
        *SCfg = &Rend -> ZBuf.StencilCfg;

    SCfg -> OpFail = Fail;
    SCfg -> OpZFail = ZFail;
    SCfg -> OpZPass = ZPass;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the Irit object to be scan converted.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:       IN, OUT, the rendering context.                              M
*   Object:     IN, the object to be scanned.                                M
*   NoShading:  IN, if TRUE, ignore shading on this one.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrBeginObject, scan irit object, z-buffer                             M
*****************************************************************************/
void IRndrBeginObject(IRndrPtrType Rend,
		      IPObjectStruct *Object,
		      int NoShading)
{
    Rend -> Obj.noShade = NoShading;

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
*   IRndrPutTriangle, scan triangle, polygon z-buffer                        M
*****************************************************************************/
void IRndrPutTriangle(IRndrPtrType Rend, IPPolygonStruct *Triangle)
{
    /* Degenerate triangles aren't scanned except for UVZ space of           */
    /* visibility map.                                                       */
    if (AttrGetIntAttrib(Triangle -> Attr, IP_ATTRIB_DEGEN_POLY) == 
                                                            IP_ATTR_BAD_INT) {
        if (IPVrtxListLen(Triangle -> PVertex) != 3) {
	    assert(0);
	    return;
	}

        if (Rend -> Mode != INC_MODE_OBJ) {
            _IRndrReportError(IRIT_EXP_STR("IRndrPutTriangle() not during object scan.\n"));
        }
        if (TriangleSet(&Rend -> Tri, Triangle, &Rend -> Obj, &Rend -> Scene)) {
            ZBufferScanTri(&Rend -> ZBuf, &Rend -> Tri, NULL);
        }
    }

    /* VisMap section: scan conversion of triangle to visibility map.        */
    /* IRndrVMPutTriangle switches between xy values and uv values (vertex ->*/
    /* Coord[1,2] switched with vertex -> Attr => "uvvals") and initializes  */
    /* triangle structrure.                                                  */
    /* ZBufferScanVMTri scans convert again the triangle (it was already scan*/
    /* converted above for the z buffer) but this time interpolates the z    */
    /* value over the u and v values (for this purpose we switch between xy  */
    /* and uv). The xyz value for each uv value is stored in VisMap and also */
    /* validity (IRndrVisibleValidityType) and fill value                    */
    /* (IRndrVisibleFillType) is set to mapped or empty                      */
    /* (for invalid triangle).                                               */
    /* Unvalid triangles (degenerated, tangent, poor aspect ratio) just      */
    /* mark all their pixel with the same unvalid value.                     */
    if (Rend -> ZBuf.VisMap != NULL &&
        IRndrVMPutTriangle(&Rend -> VisMap, &Rend -> Tri,
			   &Rend -> Scene, &Rend -> Obj, Triangle)) {
        ZBufferScanVMTri(&Rend -> ZBuf, &Rend -> Tri, NULL);
    }
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
*   IRndrEndObject, scan, irit object, z-buffer                              M
*****************************************************************************/
void IRndrEndObject(IRndrPtrType Rend)
{
    Rend -> Mode = INC_MODE_NONE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Begin drawing a line.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrBeginPll, new line, start                                           M
*****************************************************************************/
void IRndrBeginPll(IRndrPtrType Rend)
{
    LineSegmentStart(&Rend -> Seg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the next vertex of the line.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:    IN, OUT, the rendering context.                                 M
*   Vertex:  IN, the next vertex of the line.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrPutPllVertex, line vertex lineto                                    M
*****************************************************************************/
void IRndrPutPllVertex(IRndrPtrType Rend, IPVertexStruct *Vertex)
{
    int i;
    IrtRType Coord[4];

    VertexTransform(Vertex, &Rend -> Scene.Matrices, &Rend -> Obj, Coord);
    LineSegmentSet(&Rend -> Seg, Coord);
    for (i = 0; i < Rend -> Seg.TrianglesNum; i++) {
        if (TriangleSet(&Rend -> Tri,
			LineSegmentGetTri(&Rend -> Seg, i),
			&Rend -> Obj, &Rend -> Scene)) {
	    ZBufferScanTri(&Rend -> ZBuf, &Rend -> Tri, NULL);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Marks the end of the line .                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrEndPll, line end                                                    M
*****************************************************************************/
void IRndrEndPll(IRndrPtrType Rend)
{
    int i;

    Rend -> Mode = INC_MODE_NONE;
    LineSegmentEnd(&Rend -> Seg);
    for (i = 0; i < Rend -> Seg.TrianglesNum; i++) {
        if (TriangleSet(&Rend -> Tri,
			LineSegmentGetTri(&Rend -> Seg, i),
                        &Rend -> Obj, &Rend -> Scene)) {
	    ZBufferScanTri(&Rend -> ZBuf, &Rend -> Tri, NULL);
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Manually adds a single pixel.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:         IN, OUT, the rendering context.                            M
*   x:            IN, the column number.                                     M
*   y:            IN, the line number.                                       M
*   z:            IN, the pixel's depth.                                     M
*   Transparency: IN, the pixel's transparency value.                        M
*   Color:        IN, the new color of pixel at (x, y).                      M
*   Triangle:     IN, The triangle which created the added point.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrPutPixel, put pixel                                                 M
*****************************************************************************/
void IRndrPutPixel(IRndrPtrType Rend,
                   int x,
                   int y,
                   IrtRType z,
                   IrtRType Transparency,
                   IRndrColorType Color,
                   IPPolygonStruct *Triangle)
{
    ZBufferPutPixel(&Rend -> ZBuf, x, y, z, Transparency, Color, Triangle, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve a pixel's color (and alpha) from the z-buffer.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*   x:     IN, the column number.                                            M
*   y:     IN, the line number.                                              M
*   Result:  OUT, the user allocated buffer to hold the result.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetPixelColorAlpha, z-buffer, line color information                M
*****************************************************************************/
void IRndrGetPixelColorAlpha(IRndrPtrType Rend,
			     int x,
			     int y,
			     IRndrColorType *Result)
{
    ZBufferGetLineColorAlpha(&Rend -> ZBuf, x, x, y, Result);
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
*   IRndrGetPixelDepth, z-buffer, line depth                                 M
*****************************************************************************/
void IRndrGetPixelDepth(IRndrPtrType Rend,
                        int x,
                        int y,
                        IrtRType *Result)
{
    ZBufferGetLineDepth(&Rend -> ZBuf, x, x, y, Result);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve a pixel's stencil from the z-buffer.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:  IN, OUT, the rendering context.                                   M
*   x:     IN, the column number.                                            M
*   y:     IN, the line number.                                              M
*   Result:  OUT, the user allocated buffer to hold the result.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetPixelStencil, z-buffer line stencil                              M
*****************************************************************************/
void IRndrGetPixelStencil(IRndrPtrType Rend,
                          int x,
                          int y,
                          int *Result)
{
    ZBufferGetLineStencil(&Rend -> ZBuf, x, x, y, Result);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve color (and alpha) data from the z-buffer.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT, the rendering context.                                  M
*   y:      IN, the line number.                                             M
*   Result: OUT, the user allocated buffer to hold the result.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetLineColorAlpha, z-buffer, line color information                 M
*****************************************************************************/
void IRndrGetLineColorAlpha(IRndrPtrType Rend, int y, IRndrColorType *Result)
{
    ZBufferGetLineColorAlpha(&Rend -> ZBuf, 0,
			     Rend -> ZBuf.AccessMode == ZBUFFER_ACCESS_RAW ?
			         Rend -> ZBuf.SizeX - 1 :
			         Rend -> ZBuf.TargetSizeX - 1,
			     y, Result);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve z-coordinate data from the z-buffer.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT, the rendering context.                                  M
*   y:      IN, the line number.                                             M
*   Result: OUT, the user allocated buffer to hold the result.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetLineDepth, z-buffer, line Z information                          M
*****************************************************************************/
void IRndrGetLineDepth(IRndrPtrType Rend, int y, IrtRType *Result)
{
    ZBufferGetLineDepth(&Rend -> ZBuf, 0,
			Rend -> ZBuf.AccessMode == ZBUFFER_ACCESS_RAW ?
			    Rend -> ZBuf.SizeX - 1 :
			    Rend -> ZBuf.TargetSizeX - 1,
			y, Result);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Retrieve stencil data from the z-buffer.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:   IN, OUT, the rendering context.                                  M
*   y:      IN, the line number.                                             M
*   Result: OUT, the user allocated buffer to hold the result.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetLineStencil, z-buffer, line stencil information                  M
*****************************************************************************/
void IRndrGetLineStencil(IRndrPtrType Rend, int y, int *Result)
{
    ZBufferGetLineStencil(&Rend -> ZBuf, 0,
			  Rend -> ZBuf.AccessMode == ZBUFFER_ACCESS_RAW ?
			      Rend -> ZBuf.SizeX - 1 :
			      Rend -> ZBuf.TargetSizeX - 1,
			  y, Result);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets teh call back functions to invoked when saving files.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:          IN, OUT, the rendering context.                           M
*   ImgSetType:    IN, Image setting file type function			     M
*   ImgOpen:       IN, Function to open an image file.			     M
*   ImgWriteLine:  IN, Function to write one row (Vec of RGB & vec of Alpha).M
*   ImgClose:      IN, Function to close an image file.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSaveFileCB, z-buffer save dump                                      M
*****************************************************************************/
void IRndrSaveFileCB(IRndrPtrType Rend,
		     IRndrImgSetTypeFuncType ImgSetType,
		     IRndrImgOpenFuncType ImgOpen,
		     IRndrImgWriteLineFuncType ImgWriteLine,
		     IRndrImgCloseFuncType ImgClose)
{
    ZBufferSaveFileCB(&Rend -> ZBuf,
		      ImgSetType, ImgOpen, ImgWriteLine, ImgClose);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Save the color info of the z-buffer into a file .                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:           IN, OUT, the rendering context.                          M
*   BaseDirectory:  IN, the directory to save the file in.                   M
*   OutFileName:    IN, the file name.                                       M
*   Type:           IN, the file type.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSaveFile, z-buffer save dump                                        M
*****************************************************************************/
void IRndrSaveFile(IRndrPtrType Rend,
		   const char *BaseDirectory,
		   const char *OutFileName,
		   const char *Type)
{
    ZBufferSaveFile(&Rend -> ZBuf, BaseDirectory,
		    OutFileName, Type, ZBUFFER_DATA_COLOR);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Save the z coordinate values of the z-buffer into a file.                M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:           IN, OUT, the rendering context.                          M
*   BaseDirectory:  IN, the directory to save the file in.                   M
*   OutFileName:    IN, the file name.                                       M
*   Type:           IN, the file type.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSaveFileDepth, z-buffer save dump                                   M
*****************************************************************************/
void IRndrSaveFileDepth(IRndrPtrType Rend,
			const char *BaseDirectory,
			const char *OutFileName,
			const char *Type)
{
    ZBufferSaveFile(&Rend -> ZBuf, BaseDirectory,
		    OutFileName, Type, ZBUFFER_DATA_ZDEPTH);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Save  the context of the z-buffer into a file .                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:          IN, OUT, the rendering context.                           M
*   BaseDirectory: IN, the directory to save the file in.                    M
*   OutFileName:   IN, the file name.                                        M
*   Type:          IN, the file type.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSaveFileStencil, z-buffer save dump                                 M
*****************************************************************************/
void IRndrSaveFileStencil(IRndrPtrType Rend,
			  const char *BaseDirectory,
			  const char *OutFileName,
			  const char *Type)
{
    ZBufferSaveFile(&Rend -> ZBuf, BaseDirectory,
		    OutFileName, Type, ZBUFFER_DATA_STENCIL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the scene struct.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:          IN, the rendering context.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   struct IRndrSceneStruct *:   The scene struct.                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrGetScene			                                     M
*****************************************************************************/
struct IRndrSceneStruct *IRndrGetScene(IRndrPtrType Rend)
{
    return &Rend -> Scene;
} 

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Save the context of the UV-map into a file .                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:          IN, OUT, the rendering context.                           M
*   BaseDirectory: IN, the directory to save the file in.                    M
*   OutFileName:   IN, the file name.                                        M
*   Type:          IN, the file type.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrSaveFileVisMap, z-buffer save dump                                  M
*****************************************************************************/
void IRndrSaveFileVisMap(IRndrPtrType Rend,
			 const char *BaseDirectory,
			 const char *OutFileName,
			 const char *Type)
{
    ZBufferSaveFile(&Rend -> ZBuf, BaseDirectory,
		    OutFileName, Type, ZBUFFER_DATA_VIS_MAP);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Enabling visibility map in z-buffer.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   Objects:   IN, Rendered object list.                                     M
*   SuperSize: IN, filter sample super size.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if successful.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVisMapEnable, visibility map, UV                                    M
*****************************************************************************/
int IRndrVisMapEnable(IRndrPtrType Rend, 
                      IPObjectStruct *Objects,
                      int SuperSize)
{
    int retval = IRndrVMInit(&Rend -> VisMap, &Rend -> Scene, SuperSize);
	
    if (retval == 0) {
        Rend -> ZBuf.VisMap = &Rend -> VisMap;
        IRndrVMSetLimits(&Rend -> VisMap, Objects);
        Rend -> ZBuf.ScanContinuousDegenTriangleForVisMap = TRUE;
    }
    return retval;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Wrapper function for scaning visibility map.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVisMapScan, scan visibility map                                     M
*****************************************************************************/
void IRndrVisMapScan(IRndrPtrType Rend)
{
    IRndrVMScan(&Rend -> VisMap, &Rend -> ZBuf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   wrapper function for setting critic tangency angle.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   CosAng:    IN, value of critic cosinus value of angle between normal and M
*              view vector.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVisMapSetTanAngle                                                   M
*****************************************************************************/
void IRndrVisMapSetTanAngle(IRndrPtrType Rend, IrtRType CosAng)
{
    IRndrVMSetTanAngle(&Rend -> VisMap, CosAng);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   wrapper function for setting critic aspect ratio.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   CriticAR:  IN, value of critic aspect ratio value, the ratio between     M
*              largest and smallest edge of a triangle.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVisMapSetCriticAR                                                   M
*****************************************************************************/
void IRndrVisMapSetCriticAR(IRndrPtrType Rend, IrtRType CriticAR)
{
    IRndrVMSetCriticAR(&Rend -> VisMap, CriticAR);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   wrapper function for setting delation iterations number for white color  M
*   hiding in visibility maps.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Rend:      IN, OUT, the rendering context.                               M
*   Dilation:  IN, the amount of iterations to do the dilation algorithm.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVisMapSetDilation                                                   M
*****************************************************************************/
void IRndrVisMapSetDilation(IRndrPtrType Rend, int Dilation)
{
    IRndrVMSetDilation(&Rend -> VisMap, Dilation);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Wrapper function to get the domain of the given object (Object which     M
* aren't polygons are ignored.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:                   IN - The object to get its domain. Objects which M
*                                aren't polygons are ignored.                M
*   UMin, UMax, VMin, VMax: OUT - The domain of the object.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: FALSE if no uv value was found.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IRndrVisMapGetObjDomain                                                  M
*****************************************************************************/
int IRndrVisMapGetObjDomain(IPObjectStruct *PObj, 
                            IrtRType *UMin, 
                            IrtRType *UMax, 
                            IrtRType *VMin, 
                            IrtRType *VMax)
{
    return IRndrVMGetObjDomain(PObj, UMin, UMax, VMin, VMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Wrapper function for spreading UV values domains.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:                The geometric object.                               M
*   MapWidth, MapHeight: The dimensions of the visibility map.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IRndrVMPrepareUVValuesOfGeoObj                                           M
*                                                                            M
* KEYWORDS:                                                                  M
*   IRndrVisMapPrepareUVValuesOfGeoObj                                       M
*****************************************************************************/
void IRndrVisMapPrepareUVValuesOfGeoObj(IPObjectStruct *PObj, 
                                        int MapWidth, 
                                        int MapHeight)
{
    IRndrVMPrepareUVValuesOfGeoObj(PObj, MapWidth, MapHeight);
}

