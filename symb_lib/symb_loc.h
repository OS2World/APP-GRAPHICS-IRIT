/******************************************************************************
* Symb_loc.h - header file for the SYMBolic library.			      *
* This library is closely related to cagd_lib and should be linked with it.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#ifndef SYMB_LOC_H
#define SYMB_LOC_H

#include "cagd_lib.h"

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call SymbFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define SYMB_FATAL_ERROR(Msg)	SymbFatalError(Msg)

#define SYMB_GEN_COPY(Dst, Src, Size) memcpy((char *) (Dst), (char *) (Src), \
					     Size)

#define SYMB_W 0 /* Positions of points in Points array (see structs below). */
#define SYMB_X 1
#define SYMB_Y 2
#define SYMB_Z 3

#include "symb_lib.h"

#endif /* SYMB_LOC_H */
