/******************************************************************************
* Prsr_ftl.c - default FatalError function for the prsr library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "prsr_loc.h"

IRIT_STATIC_DATA IPSetErrorFuncType
    IPGlblPrsrSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by Prsr_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPSetErrorFuncType:  Old error function reference.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetFatalErrorFunc, error handling                                      M
*****************************************************************************/
IPSetErrorFuncType IPSetFatalErrorFunc(IPSetErrorFuncType ErrorFunc)
{
    IPSetErrorFuncType
	OldErrorFunc = IPGlblPrsrSetErrorFunc;

    IPGlblPrsrSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Prsr_lib errors right here. Provides a default error handler for the  M
* prsr library. Gets an error description using PrsrDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFatalError, error handling 	                                     M
*****************************************************************************/
void IPFatalError(IPFatalErrorType ErrID)
{
    if (IPGlblPrsrSetErrorFunc != NULL) {
        IPGlblPrsrSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("PRSR_LIB: %s\n"), IPDescribeError(ErrID));

    exit(-1);
}
