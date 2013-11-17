/******************************************************************************
* Rndr_lib.h - header file for the RNDR library.			      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef RNDR_LIB_H
#define RNDR_LIB_H

#include <irit_sm.h>
#include <iritprsr.h>

typedef enum IRndrLightType {
    IRNDR_LIGHT_POINT,
    IRNDR_LIGHT_VECTOR,
    IRNDR_LIGHT_LAST
} IRndrLightType;

typedef enum IRndrShadingType {
    IRNDR_SHADING_FLAT,
    IRNDR_SHADING_GOURAUD,
    IRNDR_SHADING_PHONG,
    IRNDR_SHADING_NONE,
    IRNDR_SHADING_LAST
} IRndrShadingType;

typedef enum IRndrZBufferCmpType {
    IRNDR_ZBUFFER_NEVER,
    IRNDR_ZBUFFER_LESS,
    IRNDR_ZBUFFER_LEQUAL,
    IRNDR_ZBUFFER_GREATER,
    IRNDR_ZBUFFER_GEQUAL,
    IRNDR_ZBUFFER_NOTEQUAL,
    IRNDR_ZBUFFER_ALWAYS
} IRndrZBufferCmpType;

typedef enum IRndrStencilCmpType {
    IRNDR_STENCIL_NEVER,
    IRNDR_STENCIL_LESS,
    IRNDR_STENCIL_LEQUAL,
    IRNDR_STENCIL_GREATER,
    IRNDR_STENCIL_GEQUAL,
    IRNDR_STENCIL_EQUAL,
    IRNDR_STENCIL_NOTEQUAL,
    IRNDR_STENCIL_ALWAYS
} IRndrStencilCmpType;

typedef enum IRndrStencilOpType {
    IRNDR_STENCIL_KEEP,
    IRNDR_STENCIL_ZERO,
    IRNDR_STENCIL_REPLACE,
    IRNDR_STENCIL_INCR,
    IRNDR_STENCIL_DECR,
    IRNDR_STENCIL_INVERT
} IRndrStencilOpType;

typedef enum IRndrVisibleFillType {
    IRNDR_VISMAP_FILL_EMPTY = 0,
    IRNDR_VISMAP_FILL_MAPPED,
    IRNDR_VISMAP_FILL_VISIBLE,
    IRNDR_VISMAP_FILL_DILATE_MAPPED,
    IRNDR_VISMAP_FILL_DILATE_VISIBLE
} IRndrVisibleFillType;

typedef enum IRndrVisibleValidityType {
    IRNDR_VISMAP_VALID_OK,
    IRNDR_VISMAP_VALID_TANGENT,
    IRNDR_VISMAP_VALID_POOR_AR,
    IRNDR_VISMAP_VALID_DEGEN,
    IRNDR_VISMAP_VALID_COUNT /* Number of validity types. */
} IRndrVisibleValidityType;

typedef IrtRType IRndrColorType[4];			  /* Space for RGBA. */
typedef float IRndrZDepthType;
typedef struct IRndrStruct *IRndrPtrType;
typedef struct INCZBufferStruct *INCZBufferPtrType;
typedef struct IRndrZBuffer1DStruct *IRndrZBuffer1DPtrType;
typedef IrtBType (*IRndrZCmpPolicyFuncType)(int x,
					    int y,
					    IrtRType OldZ,
					    IrtRType NewZ);
typedef void (*IRndrPixelClbkFuncType)(int x,
				       int y,
				       IRndrColorType Color,
				       IrtRType Z,
				       VoidPtr ClbkData);
typedef IrtImgImageType (*IRndrImgSetTypeFuncType)(const char *ImageType);
typedef int (*IRndrImgOpenFuncType)(const char **argv,
				    const char *FName,
				    int Alpha,
				    int XSize,
				    int YSize);
typedef void (*IRndrImgWriteLineFuncType)(IrtBType *Alpha,
					  IrtImgPixelStruct *Pixels);
typedef void (*IRndrImgCloseFuncType)(void);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Colors for visibility map. */
extern const IrtImgPixelStruct RNDR_VISMAP_EMPTY_COLOR;
extern const IrtImgPixelStruct RNDR_VISMAP_MAPPED_COLOR;
extern const IrtImgPixelStruct RNDR_VISMAP_VISIBLE_COLOR;
extern const IrtImgPixelStruct RNDR_VISMAP_TANGENT_COLOR;
extern const IrtImgPixelStruct RNDR_VISMAP_POOR_AR_COLOR;
extern const IrtImgPixelStruct RNDR_VISMAP_DEGEN_COLOR;

/* Construct a new IRndrer. */
IRndrPtrType IRndrInitialize(int SizeX,
                             int SizeY,
                             int SuperSampSize,
			     int ColorQuantization,
                             IrtBType UseTransparency,
                             IrtBType BackfaceCulling,
                             IRndrColorType BackgrCol,
                             IrtRType AmbientLight,
			     int VisMap);

/* Dispose of an IRender. */
void IRndrDestroy(IRndrPtrType Rend);

/* Clearing functions. */
void IRndrClearDepth(IRndrPtrType Rend, IRndrZDepthType ClearZ);
void IRndrClearStencil(IRndrPtrType Rend);
void IRndrClearColor(IRndrPtrType Rend);

/* Specify light sources (must be called only before any scan conversion
takes place). */
void IRndrAddLightSource(IRndrPtrType Rend,
                         IRndrLightType Type,
                         IrtPtType Where,
                         IRndrColorType Color);

/* Configure IRndrer (at initialization or on-the-fly). */
void IRndrSetFilter(IRndrPtrType Rend,
                    char *FilterName);
IRndrShadingType IRndrSetShadeModel(IRndrPtrType Rend,
				    IRndrShadingType ShadeModel);
void IRndrSetViewPrsp(IRndrPtrType Rend,
                      IrtHmgnMatType ViewMat,
                      IrtHmgnMatType PrspMat,
		      IrtHmgnMatType ScrnMat);
void IRndrSetPllParams(IRndrPtrType Rend,
                       IrtRType MinWidth,
                       IrtRType MaxWidth,
                       IrtRType ZNear,
                       IrtRType ZFar);
IrtBType IRndrSetRawMode(IRndrPtrType Rend, IrtBType UseRawMode);
IRndrZCmpPolicyFuncType IRndrSetZCmpPolicy(IRndrPtrType Rend,
					   IRndrZCmpPolicyFuncType ZCmpPol);
IRndrZBufferCmpType IRndrSetZCmp(IRndrPtrType Rend, IRndrZBufferCmpType ZCmp);
IRndrPixelClbkFuncType IRndrSetPreZCmpClbk(IRndrPtrType Rend,
					   IRndrPixelClbkFuncType PixelClbk);
void IRndrSetPostZCmpClbk(IRndrPtrType Rend,
                          IRndrPixelClbkFuncType ZPassClbk,
                          IRndrPixelClbkFuncType ZFailClbk);

/* Stencil operations, OpenGL-style. */
void IRndrStencilCmpFunc(IRndrPtrType Rend,
			 IRndrStencilCmpType SCmp,
			 int Ref,
			 unsigned Mask);
void IRndrStencilOp(IRndrPtrType Rend,
                    IRndrStencilOpType Fail,
                    IRndrStencilOpType ZFail,
                    IRndrStencilOpType ZPass);

/* Object scan conversion. */
void IRndrBeginObject(IRndrPtrType Rend,
                      IPObjectStruct *Object,
		      int NoShading);
void IRndrPutTriangle(IRndrPtrType Rend, IPPolygonStruct *Triangle);
void IRndrEndObject(IRndrPtrType Rend);

/* Polyline scan conversion. */
void IRndrBeginPll(IRndrPtrType Rend);
void IRndrPutPllVertex(IRndrPtrType Rend, IPVertexStruct *Vertex);
void IRndrEndPll(IRndrPtrType Rend);

/* "Manual" pixel rendering. */
void IRndrPutPixel(IRndrPtrType Rend,
                   int x,
                   int y,
                   IrtRType z,
                   IrtRType Transparency,
                   IRndrColorType Color,
                   IPPolygonStruct *Triangle);

/* Z Buffer Access - Pixel Resolution. */
void IRndrGetPixelColorAlpha(IRndrPtrType Rend,
			     int x,
			     int y,
			     IRndrColorType *Result);
void IRndrGetPixelDepth(IRndrPtrType Rend,
                        int x,
                        int y,
                        IrtRType *Result);
void IRndrGetPixelStencil(IRndrPtrType Rend,
                          int x,
                          int y,
                          int *Result);

/* Z Buffer Access - Line Resolution. */
void IRndrGetLineColorAlpha(IRndrPtrType Rend,
			    int y,
			    IRndrColorType *Result);
void IRndrGetLineDepth(IRndrPtrType Rend, int y, IrtRType *Result);
void IRndrGetLineStencil(IRndrPtrType Rend, int y, int *Result);

/* Clipping support. */
void IRndrGetClippingPlanes(IRndrPtrType Rend, IrtPlnType *ClipPlanes);
void IRndrSetZBounds(IRndrPtrType Rend, IrtRType ZNear, IrtRType ZFar);

/* Dump z-buffer contents to a file. */
void IRndrSaveFileCB(IRndrPtrType Rend,
		     IRndrImgSetTypeFuncType ImgSetType,
		     IRndrImgOpenFuncType ImgOpen,
		     IRndrImgWriteLineFuncType ImgWriteLine,
		     IRndrImgCloseFuncType ImgClose);
void IRndrSaveFile(IRndrPtrType Rend,
                   const char *BaseDirectory,
                   const char *OutFileName,
                   const char *Type);
void IRndrSaveFileDepth(IRndrPtrType Rend,
			const char *BaseDirectory,
			const char *OutFileName,
			const char *Type);
void IRndrSaveFileStencil(IRndrPtrType Rend,
			  const char *BaseDirectory,
			  const char *OutFileName,
			  const char *Type);
void IRndrSaveFileVisMap(IRndrPtrType Rend,
			 const char *BaseDirectory,
			 const char *OutFileName,
			 const char *Type);
struct IRndrSceneStruct *IRndrGetScene(IRndrPtrType Rend); 

/* The 1D Z Buffer implementation. */

IRndrZBuffer1DPtrType IRndr1DInitialize(int ZBuf1DSize,
					IrtRType XMin,
					IrtRType XMax,
					IrtRType ZMin,
					IrtRType ZMax,
					int BottomMaxZ);
void IRndr1DClearDepth(IRndrZBuffer1DPtrType Rend, IrtRType ClearZ);
IRndrZBufferCmpType IRndr1DSetZCmp(IRndrZBuffer1DPtrType Rend,
				   IRndrZBufferCmpType ZCmp);
void IRndr1DDestroy(IRndrZBuffer1DPtrType Rend);
void IRndr1DPutPolyline(IRndrZBuffer1DPtrType Rend, IPPolygonStruct *Pl);
void IRndr1DPutLine(IRndrZBuffer1DPtrType Rend,
		    IrtRType x1,
		    IrtRType z1,
		    IrtRType x2,
		    IrtRType z2);
void IRndr1DPutPixel(IRndrZBuffer1DPtrType Rend, int x, IrtRType z);
void IRndr1DGetPixelDepth(IRndrZBuffer1DPtrType Rend, int x, IrtRType *Result);
void IRndr1DGetLineDepth(IRndrZBuffer1DPtrType Rend,
			 int x1,
			 int x2,
			 IrtRType *ZValues);
IPPolygonStruct *IRndr1DUpperEnvAsPolyline(IRndrZBuffer1DPtrType Rend,
					   int MergeInters);
IPPolygonStruct *IRndr1DFilterCollinearEdges(IRndrZBuffer1DPtrType Rend,
					     IPPolygonStruct *Pl,
					     int MergeInters);

/* The NC Z Buffer implementation. */

INCZBufferPtrType INCRndrInitialize(int ZBufSizeX,
				    int ZBufSizeY,
				    int GridSizeX,
				    int GridSizeY,
				    IrtPtType XYZMin,
				    IrtPtType XYZMax,
				    int BottomMaxZ);
IRndrZBufferCmpType INCRndrSetZCmp(INCZBufferPtrType Rend,
				   IRndrZBufferCmpType ZCmp);
void INCRndrDestroy(INCZBufferPtrType Rend);
void INCRndrBeginObject(INCZBufferPtrType Rend, IPObjectStruct *Object);
void INCRndrPutTriangle(INCZBufferPtrType Rend, IPPolygonStruct *Triangle);
IrtRType INCRndrPutMask(INCZBufferPtrType Rend,
			int *PosXY,
			IrtRType PosZ,
			IrtRType *Mask,
			int MaskXSize,
			int MaskYSize);
void INCRndrEndObject(INCZBufferPtrType Rend);
void INCRndrPutPixel(INCZBufferPtrType Rend, int x, int y, IrtRType z);
void INCRndrGetPixelDepth(INCZBufferPtrType Rend,
			  int x,
			  int y,
			  IrtRType *Result);
void INCRndrGetLineDepth(INCZBufferPtrType Rend,
			 int x1,
			 int x2,
			 int y,
			 IrtRType *ZValues);
void INCRndrGetZbufferGridCellMaxSize(INCZBufferPtrType Rend,
				      int *GridSizeX,
				      int *GridSizeY,
				      int *GridCellXSize,
				      int *GridCellYSize);
int INCRndrGetZbufferGridCell(INCZBufferPtrType Rend,
			      int GridCellX,
			      int GridCellY,
			      IrtRType *ZValues,
			      int *XMin,
			      int *YMin,
			      int *XMax,
			      int *YMax);
int INCRndrMapPixelsToCells(INCZBufferPtrType Rend, int *X, int *Y);
int INCRndrGetActiveCells(INCZBufferPtrType Rend,
			  int *MinCellX,
			  int *MinCellY,
			  int *MaxCellX,
			  int *MaxCellY,
			  IrtRType *ZPixelsRemoved);
int IRndrVisMapEnable(IRndrPtrType Rend,
		      IPObjectStruct* Objects,
		      int SuperSize);
void IRndrVisMapScan(IRndrPtrType Rend);
void IRndrVisMapSetTanAngle(IRndrPtrType Rend, IrtRType CosAng);
void IRndrVisMapSetCriticAR(IRndrPtrType Rend, IrtRType CriticAR);
void IRndrVisMapSetDilation(IRndrPtrType Rend, int Dilation);
int IRndrVisMapGetObjDomain(IPObjectStruct *PObj, 
                            IrtRType *UMin, 
                            IrtRType *UMax, 
                            IrtRType *VMin, 
                            IrtRType *VMax);
void IRndrVisMapPrepareUVValuesOfGeoObj(IPObjectStruct *PObj, 
                                        int MapWidth, 
                                        int MapHeight);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* RNDR_LIB_H */
