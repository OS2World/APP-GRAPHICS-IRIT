/******************************************************************************
* Mvar_ftl.c - default FatalError function for the mvar library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include <stdio.h>
#include "mvar_loc.h"

IRIT_STATIC_DATA MvarSetErrorFuncType
    GlblMvarSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Mvar_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
MvarSetErrorFuncType MvarSetFatalErrorFunc(MvarSetErrorFuncType ErrorFunc)
{
    MvarSetErrorFuncType
	OldErrorFunc = GlblMvarSetErrorFunc;

    GlblMvarSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Mvar_lib errors right here. Provides a default error handler for the  M
* mvar library. Gets an error description using MvarDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarFatalError, error handling                                           M
*****************************************************************************/
void MvarFatalError(MvarFatalErrorType ErrID)
{
    if (GlblMvarSetErrorFunc != NULL) {
        GlblMvarSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("MVAR_LIB: %s\n"), MvarDescribeError(ErrID));

    exit(-1);
}
