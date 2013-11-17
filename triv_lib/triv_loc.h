/******************************************************************************
* Triv_loc.h - header file for the TRIV library.			      *
* This header is local to the library - it is NOT external.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#ifndef TRIV_LOC_H
#define TRIV_LOC_H

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "cagd_lib.h"

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call TrivFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define TRIV_FATAL_ERROR(Msg)	TrivFatalError(Msg)

#include "triv_lib.h"		     /* Include the extrenal header as well. */

#endif /* TRIV_LOC_H */
