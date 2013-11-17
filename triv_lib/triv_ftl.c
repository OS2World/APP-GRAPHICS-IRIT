/******************************************************************************
* Triv_ftl.c - default FatalError function for the triv library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "triv_loc.h"

IRIT_STATIC_DATA TrivSetErrorFuncType
    GlblTrivSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Triv_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
TrivSetErrorFuncType TrivSetFatalErrorFunc(TrivSetErrorFuncType ErrorFunc)
{
    TrivSetErrorFuncType
	OldErrorFunc = GlblTrivSetErrorFunc;

    GlblTrivSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Triv_lib errors right here. Provides a default error handler for the  M
* triv library. Gets an error description using TrivDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivFatalError, error handling                                           M
*****************************************************************************/
void TrivFatalError(TrivFatalErrorType ErrID)
{
    if (GlblTrivSetErrorFunc != NULL) {
        GlblTrivSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("TRIV_LIB: %s\n"), TrivDescribeError(ErrID));

    exit(-1);
}
