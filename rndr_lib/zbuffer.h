/******************************************************************************
* ZBuffer.h - header file for the ZBuffer in the RNDR library.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef IRNDR_ZBUFFER_H
#define IRNDR_ZBUFFER_H

#include "rndr_loc.h"
#include "triangle.h"
#include "scene.h"
#include "fstalloc.h"
#include "stencil.h"

#define ZBUFFER_ACCESS_FILTERED 0
#define ZBUFFER_ACCESS_RAW 1

typedef enum IRndrZBufferDataType {
    ZBUFFER_DATA_COLOR,
    ZBUFFER_DATA_ZDEPTH,
    ZBUFFER_DATA_STENCIL,
    ZBUFFER_DATA_VIS_MAP
} IRndrZBufferDataType;

typedef struct IRndrFilterType {
    int SuperSize;
    IrtRType **FilterData;
    char *Name;
    IrtRType TotalWeight;
} IRndrFilterType;

typedef float IRndrZTranspType;

/* Every scan line is represented in z-buffer by array of z-slots, mainly    */
/* formed by the linked list of z-points set up by different Triangles       */
/* projection.                                                               */
typedef struct IRndrZPointStruct {
    struct IRndrZPointStruct *Next; /* Link to next z-point at same location.*/
    IRndrPixelType Color;
    IRndrZDepthType z;
    IRndrZTranspType Transp;                         /* Transparancy factor. */
    IPPolygonStruct *Triangle;     /* The triangle which created this point. */
} IRndrZPointStruct;

typedef struct IRndrZListStruct {
    IRndrZPointStruct First;/* Head of linked list, contains nearest z-point.*/
    int Stencil;
} IRndrZListStruct;

typedef struct IRndrZBufferStruct {
    IRndrZListStruct **z;
    int SizeX;
    int SizeY;
    int TargetSizeX;
    int TargetSizeY;
    IRndrZBufferCmpType ZBufCmp;

    FastAllocType PointsAlloc;
    IrtBType ColorsValid;
    int AccessMode;
    struct IRndrFilterType *Filter;
    int UseTransparency;
    IRndrColorType BackgroundColor;
    IrtRType BackgroundDepth;
    IRndrSceneStruct *Scene;
    int ColorQuantization;
    struct IRndrVMStruct *VisMap;
    int DoVisMapScan;   /* Flags if scan convention is done over UV coords. */
    int ScanContinuousDegenTriangleForVisMap;

    /* Temporary line calculation buffers. */
    IRndrColorType *LineColors;
    IrtBType *LineAlpha;
    IrtImgPixelStruct *LinePixels;

    /* Callbacks. */
    IRndrZCmpPolicyFuncType ZPol;
    IRndrPixelClbkFuncType PreZCmpClbk;
    IRndrPixelClbkFuncType ZPassClbk;
    IRndrPixelClbkFuncType ZFailClbk;

    IRndrStencilCfgStruct StencilCfg;

    IRndrImgSetTypeFuncType ImgSetType;
    IRndrImgOpenFuncType ImgOpen;
    IRndrImgWriteLineFuncType ImgWriteLine;
    IRndrImgCloseFuncType ImgClose;
} IRndrZBufferStruct;

int  ZBufferInit(IRndrZBufferStruct *Buffer,
                 IRndrSceneStruct *Scene,
                 int SuperSize,
		 int ColorQuantization);
void ZBufferClear(IRndrZBufferStruct *Buffer);
void ZBufferSetFilter(IRndrZBufferStruct *Buffer, char *FilterName);
void ZBufferSetBackgrZ(IRndrZBufferStruct *Buffer, IrtRType BackgrZ);
void ZBufferPutPixel(IRndrZBufferStruct *Buffer,
                     int x,
                     int y,
                     IrtRType z,
                     IrtRType Transparency,
                     IRndrColorType Color,
                     IPPolygonStruct *Triangle,
		     VoidPtr ClbkData);
void ZBufferScanTri(IRndrZBufferStruct *Buffer,
                    IRndrTriangleStruct *f,
		    VoidPtr ClbkData);
void ZBufferScanVMTri(IRndrZBufferStruct *Buffer,
		      IRndrTriangleStruct *Tri,
		      VoidPtr ClbkData);
int  ZBufferGetLineDepth(IRndrZBufferStruct *Buffer,
                         int x0,
			 int x1,
			 int y,
                         IrtRType *Result);
void ZBufferGetLineColorAlpha(IRndrZBufferStruct *Buffer,
			      int x0,
			      int x1,
			      int y,
			      IRndrColorType *Result);
int  ZBufferGetLineStencil(IRndrZBufferStruct *Buffer,
                           int x0,
			   int x1,
			   int y,
                           int *Result);
void ZBufferSaveFileCB(IRndrZBufferStruct *Buffer,
		       IRndrImgSetTypeFuncType ImgSetType,
		       IRndrImgOpenFuncType ImgOpen,
		       IRndrImgWriteLineFuncType ImgWriteLine,
		       IRndrImgCloseFuncType ImgClose);
void ZBufferSaveFile(IRndrZBufferStruct *Buffer,
                     const char *BaseDirectory,
                     const char *OutFileName,
                     const char *FileType,
                     IRndrZBufferDataType DataType);
void ZBufferClearColor(IRndrZBufferStruct *Buffer);
void ZBufferClearDepth(IRndrZBufferStruct *Buffer, IRndrZDepthType ClearZ);
void ZBufferClearStencil(IRndrZBufferStruct *Buffer);
void ZBufferRelease(IRndrZBufferStruct *Buffer);

#endif /* IRNDR_ZBUFFER_H */
