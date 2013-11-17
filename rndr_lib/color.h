/******************************************************************************
* Color.h - header file for color handling in the RNDR library.               *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_COLOR_H_
#define _IRNDR_COLOR_H_

#include "rndr_loc.h"
#include "interpol.h"
#include "object.h"
#include "scene.h"

void LightIntensivity(IRndrLightStruct *l,
                      const IrtPtType p,
                      const IrtNrmlType n,
                      const IRndrObjectStruct *o,
                      IRndrSceneStruct *Scene,
                      IRndrIntensivityStruct *i);

void TriangleColorEval(IPPolygonStruct *Poly,
                       int x,
                       int y,
                       IRndrObjectStruct *o,
                       IRndrSceneStruct *Scene,
                       IRndrInterpolStruct *Value,
                       IRndrColorType r);

#endif /* _IRNDR_COLOR_H_ */
