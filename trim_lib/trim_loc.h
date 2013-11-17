/******************************************************************************
* Trim_loc.h - header file for the TRIMolic library.			      *
* This library is closely related to cagd_lib and should be linked with it.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Nov. 94.					      *
******************************************************************************/

#ifndef TRIM_LOC_H
#define TRIM_LOC_H

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"

#ifdef IRIT_DOUBLE
#define TRIM_ISO_PARAM_PERTURB 1.23456789e-8
#else
#define TRIM_ISO_PARAM_PERTURB 1.23456789e-4
#endif /* IRIT_DOUBLE */

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call TrimFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define TRIM_FATAL_ERROR(Msg)	TrimFatalError(Msg)

#include "trim_lib.h"

IRIT_GLOBAL_DATA_HEADER CagdRType _TrimUVCrvApproxTolSamples;
IRIT_GLOBAL_DATA_HEADER SymbCrvApproxMethodType _TrimUVCrvApproxMethod;
IRIT_GLOBAL_DATA_HEADER CagdBType _TrimEuclidComposedFromUV;

#endif /* TRIM_LOC_H */
