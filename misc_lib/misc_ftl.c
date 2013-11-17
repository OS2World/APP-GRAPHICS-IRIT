/******************************************************************************
* Misc_ftl.c - default FatalError function for the misc library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "misc_loc.h"

IRIT_STATIC_DATA MiscSetErrorFuncType
    GlblMiscSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Misc_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   MiscSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
MiscSetErrorFuncType MiscSetFatalErrorFunc(MiscSetErrorFuncType ErrorFunc)
{
    MiscSetErrorFuncType
	OldErrorFunc = GlblMiscSetErrorFunc;

    GlblMiscSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Misc_lib errors right here. Provides a default error handler for the  M
* misc library. Gets an error description using MiscDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*   ErrDesc:    Possibly, an additional description on error.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscFatalError, error handling                                           M
*****************************************************************************/
void MiscFatalError(MiscFatalErrorType ErrID, char *ErrDesc)
{
    if (GlblMiscSetErrorFunc != NULL) {
        GlblMiscSetErrorFunc(ErrID, ErrDesc);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("MISC_LIB: %s%s\n"),
	    MiscDescribeError(ErrID), ErrDesc);

    exit(-1);
}
