/******************************************************************************
* Trng_loc.h - header file for the TRNG library.			      *
* This header is local to the library - it is NOT external.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#ifndef TRNG_LOC_H
#define TRNG_LOC_H

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call TrngFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define TRNG_FATAL_ERROR(Msg)	TrngFatalError(Msg)

#include "trng_lib.h"		     /* Include the extrenal header as well. */

#endif /* TRNG_LOC_H */
