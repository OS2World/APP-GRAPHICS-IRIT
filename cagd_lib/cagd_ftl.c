/******************************************************************************
* Cagd_ftl.c - default FatalError function for the cagd library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "cagd_loc.h"

IRIT_STATIC_DATA CagdSetErrorFuncType
    GlblCagdSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Cagd_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
CagdSetErrorFuncType CagdSetFatalErrorFunc(CagdSetErrorFuncType ErrorFunc)
{
    CagdSetErrorFuncType
	OldErrorFunc = GlblCagdSetErrorFunc;

    GlblCagdSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Cagd_lib errors right here. Provides a default error handler for the  M
* cagd library. Gets an error description using CagdDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdFatalError, error handling                                           M
*****************************************************************************/
void CagdFatalError(CagdFatalErrorType ErrID)
{
    if (GlblCagdSetErrorFunc != NULL) {
        GlblCagdSetErrorFunc(ErrID);
	return;
    }

    IRIT_WARNING_MSG_PRINTF("CAGD_LIB: %s\n", CagdDescribeError(ErrID));

    exit(-1);
}
