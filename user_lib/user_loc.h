/******************************************************************************
* User_loc.h - header file for the user interaction library.		      *
* This header is also the interface header to the world.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 95.					      *
******************************************************************************/

#ifndef USER_LOC_H
#define USER_LOC_H

/******************************************************************************
* This macro is called when the library has detected an unrecoverable error.  *
* Default action is to call UserFatalError, but you may want to reroute this  *
* to invoke your handler and recover yourself (by long jump for example).     *
******************************************************************************/
#define USER_FATAL_ERROR(Msg)	UserFatalError(Msg)

#include "user_lib.h"

#endif /* USER_LOC_H */
