/******************************************************************************
* vis_maps.h - header file for the visibility maps rendered using irender.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Eyal Posener, Nov. 2009.                          		      *
******************************************************************************/

#ifndef IRNDR_VISMAPS_H
#define IRNDR_VISMAPS_H

#include "rndr_loc.h"
#include "scene.h"
#include "triangle.h"
#include "object.h"
#include "zbuffer.h"

typedef struct IRndrUVListStruct {
    IrtPtType Coord;
    IRndrVisibleFillType Value;
    IRndrVisibleValidityType Validity;
    int BackFaced;
    IPPolygonStruct *Triangle;     /* The triangle which created this point. */
} IRndrUVListStruct;

typedef struct IRndrVMStruct {
    int SizeU, SizeV, TargetSizeU, TargetSizeV, SuperSize,
        DilationItarations, InvScreenMatValid;

    /* Minimum and maximum values of UV coordinates of the objects. */
    float MaxU, MinU, MaxV, MinV;
    IrtRType CosTanAng, CriticAR;

    /* Matrix accumulate UV information for UV map output. */
    IRndrUVListStruct **UVMap;
    IRndrSceneStruct *Scene;
} IRndrVMStruct;

int IRndrVMInit(IRndrVMStruct *VisMap, IRndrSceneStruct *Scene, int SuperSize);
void IRndrVMSetLimits(IRndrVMStruct *VisMap, IPObjectStruct* Objects);
void IRndrVMSetTanAngle(IRndrVMStruct *VisMap, IrtRType CosAng);
void IRndrVMSetCriticAR(IRndrVMStruct *VisMap, IrtRType CriticAR);
void IRndrVMSetDilation(IRndrVMStruct *VisMap, int Delation);
void IRndrVMClear(IRndrVMStruct *VisMap);
void IRndrVMRelease(IRndrVMStruct *VisMap);
int IRndrVMPutTriangle(IRndrVMStruct *VisMap,
                      IRndrTriangleStruct *RendTri,
                      IRndrSceneStruct *Scene,
                      IRndrObjectStruct *Obj,
                      IPPolygonStruct *Triangle);
void IRndrVMScan(IRndrVMStruct *VisMap, IRndrZBufferStruct *Buff);
void IRndrVMPutPixel(IRndrVMStruct *VisMap, 
		     int u, int v, 
		     IrtPtType xyzVals, 
		     IRndrVisibleValidityType Validity,
		     int BackFaced,
                     IPPolygonStruct *Triangle);
int IRndrVMGetLine(IRndrVMStruct *VisMap,
                   int u0,
                   int u1,
                   int v,
                   IrtRType **FilterCoeff,
                   IrtRType *Result,
                   IRndrVisibleValidityType *Validity);
int IRndrVMGetObjDomain(IPObjectStruct *PObj, 
                        IrtRType *UMin, 
                        IrtRType *UMax, 
                        IrtRType *VMin, 
                        IrtRType *VMax);
void IRndrVMPrepareUVValuesOfGeoObj(IPObjectStruct *PObj, 
                                    int MapWidth, 
                                    int MapHeight);
void IRndrVMRelocatePtIntoTriangle(IPPolygonStruct *Triangle, IrtPtType Pt);
int IRndrVMIsPointInTriangle(IPPolygonStruct *Triangle, 
                             IrtPtType Pt,
                             int Perim,
                             IrtRType *z,
                             IrtRType *s,
                             IrtRType *t);
#endif /* IRNDR_VISMAPS_H */
