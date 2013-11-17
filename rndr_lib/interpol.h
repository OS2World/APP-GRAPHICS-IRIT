/******************************************************************************
* Interpol.h - header file for the DDA interpolation in the RNDR library.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Michael Plavnik and Dimitri Beserb, Mar. 95.		      *
* Modified by David Shafrir and Alex Reicher, Mar 2004.			      *
******************************************************************************/

#ifndef _IRNDR_INTERPOL_H_
#define _IRNDR_INTERPOL_H_

#include "rndr_loc.h"

/* Scan-line algorithm interpolation values.                                 */
/* Add any for which you want to interpolate.                                */
/* Light intensivity components via shading  models used.                    */

typedef struct IRndrIntensivityStruct {
    IrtRType Diff, Spec;
} IRndrIntensivityStruct;

typedef struct IRndrInterpolStruct {
    IrtRType w, z;                           /* Homogenous and Z coordinate. */
    IrtRType u, v;               /* Bivariative texture mapping coordinates. */
    IrtNrmlType n;             /* Normal at the current interpolation point. */
    IrtPtType c;                /* Color at the current interpolation point. */
    IRndrIntensivityStruct *i;      /* Array of intens. for every light src. */
    int IntensSize;
    int HasColor;
} IRndrInterpolStruct;

IRndrInterpolStruct *InterpolCopy(IRndrInterpolStruct *Dst,
				  IRndrInterpolStruct *Src);

IRndrInterpolStruct *InterpolDelta(IRndrInterpolStruct *Dst,
                  IRndrInterpolStruct *v1,
                  IRndrInterpolStruct *v2,
                  IrtRType d);

IRndrInterpolStruct *InterpolIncr(IRndrInterpolStruct *Dst,
				  IRndrInterpolStruct *d);

#endif /* _IRNDR_INTERPOL_H_ */
