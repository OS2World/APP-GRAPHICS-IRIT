/******************************************************************************
* Trim_ftl.c - default FatalError function for the trim library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, October 94.					      *
******************************************************************************/

#include <stdio.h>
#include "trim_loc.h"

IRIT_STATIC_DATA TrimSetErrorFuncType
    GlblTrimSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Trim_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
TrimSetErrorFuncType TrimSetFatalErrorFunc(TrimSetErrorFuncType ErrorFunc)
{
    TrimSetErrorFuncType
	OldErrorFunc = GlblTrimSetErrorFunc;

    GlblTrimSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Trim_lib errors right here. Provides a default error handler for the  M
* trim library. Gets an error description using TrimDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimFatalError, error handling                                           M
*****************************************************************************/
void TrimFatalError(TrimFatalErrorType ErrID)
{
    if (GlblTrimSetErrorFunc != NULL) {
        GlblTrimSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("TRIM_LIB: %s\n"), TrimDescribeError(ErrID));

    exit(-1);
}
