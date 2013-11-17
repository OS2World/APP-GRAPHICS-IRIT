/******************************************************************************
* Mdl_ftl.c - default FatalError function for the Model library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "mdl_loc.h"

IRIT_STATIC_DATA MdlSetErrorFuncType
    GlblMdlSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Mdl_lib.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   MdlSetErrorFuncType:  Old error function reference.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlSetFatalErrorFunc, error handling                                     M
*****************************************************************************/
MdlSetErrorFuncType MdlSetFatalErrorFunc(MdlSetErrorFuncType ErrorFunc)
{
    MdlSetErrorFuncType
	OldErrorFunc = GlblMdlSetErrorFunc;

    GlblMdlSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Mdl_lib errors right here. Provides a default error handler for the   M
* mdl library. Gets an error description using MdlDescribeError, prints it   M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlFatalError, error handling                                            M
*****************************************************************************/
void MdlFatalError(MdlFatalErrorType ErrID)
{
    if (GlblMdlSetErrorFunc != NULL) {
        GlblMdlSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, "MDL_LIB: %s\n", MdlDescribeError(ErrID));

    exit(-1);
}
