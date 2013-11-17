/******************************************************************************
* Scene.h - header file for the scene processing in the RNDR library.         *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_SCENE_H_
#define _IRNDR_SCENE_H_

#include "rndr_loc.h"
#include "lights.h"

typedef struct IRndrMatrixContextStruct {
    IrtHmgnMatType ViewMat; /* Comulative matrix of viewing transformations. */
    IrtHmgnMatType ViewInvMat;             /* Inverse of accumulated matrix. */
    IrtPtType  Viewer;                      /* Viewer position or direction. */
    IrtHmgnMatType TransMat;
    IrtHmgnMatType ScreenMat;
    int ParallelProjection;
} IRndrMatrixContextStruct;

typedef struct IRndrSceneStruct {
    int SizeX;
    int SizeY;
    IRndrMatrixContextStruct Matrices;
    IRndrLightListStruct Lights;
    IRndrColorType BackgroundColor;
    IrtRType Ambient;
    int ShadeModel;			  /* Type of shading model to apply. */
    int BackFace;	       /* Flag directing to remove back faced flats. */
    IrtRType ZNear;                                      /* Clipping planes. */
    IrtRType ZFar;
    int ZFarValid;
    IrtRType XMin, XMax, YMin, YMax;
} IRndrSceneStruct;

void SceneSetMatrices(IRndrSceneStruct *Scene,
                      IrtHmgnMatType ViewMat,
                      IrtHmgnMatType PrspMat,
		      IrtHmgnMatType ScrnMat);

void SceneGetClippingPlane(IRndrSceneStruct *Scene,
                           int Axis,
                           int Min,
                           IrtPlnType Result);

void SceneSetZClippingPlanes(IRndrSceneStruct *Scene,
                             IrtRType ZNear,
                             IrtRType ZFar);

void SceneRelease(IRndrSceneStruct *Scene);

struct IRndrMatrixContextStruct *SceneGetMatrices(IRndrSceneStruct *Scene);

#endif /* _IRNDR_SCENE_H_ */
