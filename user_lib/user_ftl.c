/******************************************************************************
* User_ftl.c - default FatalError function for the user library.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, April. 93.					      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "user_loc.h"

IRIT_STATIC_DATA UserSetErrorFuncType
    GlblUserSetErrorFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by User_lib.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserSetErrorFuncType:  Old error function reference.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
UserSetErrorFuncType UserSetFatalErrorFunc(UserSetErrorFuncType ErrorFunc)
{
    UserSetErrorFuncType
	OldErrorFunc = GlblUserSetErrorFunc;

    GlblUserSetErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap User_lib errors right here. Provides a default error handler for the  M
* user library. Gets an error description using UserDescribeError, prints it M
* and exit the program using exit.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFatalError, error handling                                           M
*****************************************************************************/
void UserFatalError(UserFatalErrorType ErrID)
{
    if (GlblUserSetErrorFunc != NULL) {
        GlblUserSetErrorFunc(ErrID);
	return;
    }

    fprintf(stderr, IRIT_EXP_STR("USER_LIB: %s\n"), UserDescribeError(ErrID));

    exit(-1);
}
