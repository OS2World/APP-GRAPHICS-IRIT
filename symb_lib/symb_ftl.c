/******************************************************************************
* Symb_ftl.c - default FatalError function for the symb library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "symb_loc.h"

IRIT_STATIC_DATA SymbSetErrorFuncType
    GlblSymbSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Symb_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
SymbSetErrorFuncType SymbSetFatalErrorFunc(SymbSetErrorFuncType ErrorFunc)
{
    SymbSetErrorFuncType
	OldErrorFunc = GlblSymbSetErrorFunc;

    GlblSymbSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Symb_lib errors right here. Provides a default error handler for the  M
* symb library. Gets an error description using SymbDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbFatalError, error handling                                           M
*****************************************************************************/
void SymbFatalError(SymbFatalErrorType ErrID)
{
    if (GlblSymbSetErrorFunc != NULL) {
        GlblSymbSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("SYMB_LIB: %s\n"), SymbDescribeError(ErrID));

    exit(-1);
}
