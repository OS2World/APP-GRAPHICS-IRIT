/******************************************************************************
* Object.h - header file for object processing in the RNDR library.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_OBJECT_H_
#define _IRNDR_OBJECT_H_

#include "rndr_loc.h"
#include "texture.h"
#include "polyline.h"
#include "scene.h"

#define AMBIENT_DEFAULT        0.2
#define COSINE_DEFAULT        64
#define SRF_SPECULAR_DEFAULT    0.4
#define SRF_DIFFUSE_DEFAULT    0.4
#define TRANSP_DEFAULT        0

typedef struct IRndrObjectStruct {
    int Power;              /* Power of cosine factor of specular component. */
    IrtRType KSpecular;                       /* Specular coefficient value. */
    IrtRType KDiffuse;                         /* Diffuse coefficient value. */
    IRndrColorType Color;                            /* Color of the object. */
    IRndrTextureStruct Txtr;
    IrtRType Transp;                          /* Transparency of the object. */
    int noShade;                            /* Pure color model (polylines). */
    IPObjectStruct *OriginalIritObject;
    IrtHmgnMatType AnimationMatrix;
    int Transformed;
    int Animated;
    int DoVisMapCalcs;         /* If calculations are done over UV triangle. */
} IRndrObjectStruct;

void ObjectInit(IRndrObjectStruct *PObject);

void ObjectRelease(IRndrObjectStruct *PObject);

IRndrObjectStruct *ObjectSet(IRndrObjectStruct *PObject,
                               IPObjectStruct *Obj,
                               IRndrSceneStruct *Scene);

void VertexTransform(IPVertexStruct *Vertex,
                     IRndrMatrixContextStruct *Matrices,
                     IRndrObjectStruct *o,
                     IrtRType *Result);

#endif /* _IRNDR_OBJECT_H_ */
