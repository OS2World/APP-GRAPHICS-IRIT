/******************************************************************************
* Lights.h - header file for lights processing in the RNDR library.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_LIGHTS_H_
#define _IRNDR_LIGHTS_H_

#include "rndr_loc.h"
#include "interpol.h"

/* Data structure describing light source. */
typedef struct IRndrLightStruct {
    int Type;                                          /* Light source type. */
    IrtPtType Where;                     /* Light source position or vector. */
    IRndrColorType Color;                      /* Color of the light source. */
} IRndrLightStruct;

/* We use array of Light source objects. */
typedef struct IRndrLightListStruct {
    int n;                                       /* Number of light sources. */
    IRndrLightStruct *Src;                 /* Array of light source objects. */
} IRndrLightListStruct;

void LightListInitEmpty(IRndrLightListStruct *Lights);

void LightListAdd(IRndrLightListStruct *Lights, IRndrLightStruct *NewSrc);

void LightListInitDefault(IRndrLightListStruct *Lights);

#endif /* _IRNDR_LIGHTS_H_ */
