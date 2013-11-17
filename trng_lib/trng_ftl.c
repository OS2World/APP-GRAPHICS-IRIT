/******************************************************************************
* Trng_ftl.c - default FatalError function for the trng library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include <stdio.h>
#include "trng_loc.h"

IRIT_STATIC_DATA TrngSetErrorFuncType
    GlblTrngSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Trng_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
TrngSetErrorFuncType TrngSetFatalErrorFunc(TrngSetErrorFuncType ErrorFunc)
{
    TrngSetErrorFuncType
	OldErrorFunc = GlblTrngSetErrorFunc;

    GlblTrngSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Trng_lib errors right here. Provides a default error handler for the  M
* trng library. Gets an error description using TrngDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngFatalError, error handling                                           M
*****************************************************************************/
void TrngFatalError(TrngFatalErrorType ErrID)
{
    if (GlblTrngSetErrorFunc != NULL) {
        GlblTrngSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("TRNG_LIB: %s\n"), TrngDescribeError(ErrID));

    exit(-1);
}
