/*****************************************************************************
* Default fatal error handler for irit.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, April 1993  *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "irit_sm.h"
#include "misc_loc.h"

IRIT_STATIC_DATA IritFatalMsgFuncType
    GlblIritFatalMsgFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the warning function of irit.		                             M
*                                                                            *
* PARAMETERS:                                                                M
*   FatalMsgFunc:      New function to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritFatalMsgFuncType:  Old function reference.	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
IritFatalMsgFuncType IritSetFatalErrorFunc(IritFatalMsgFuncType FatalMsgFunc)
{
    IritFatalMsgFuncType
	OldErrorFunc = GlblIritFatalMsgFunc;

    GlblIritFatalMsgFunc = FatalMsgFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit fatal errors.			     M
*   This function just prints the given error message and die.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message to print.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritWarningMsg, IritInformationMsg, IritFatalErrorPrintf                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritFatalError, error trap                                               M
*****************************************************************************/
void IritFatalError(const char *Msg)
{
    if (GlblIritFatalMsgFunc)
        GlblIritFatalMsgFunc(Msg);
    else
        fprintf(stderr, IRIT_EXP_STR("Irit Fatal Error: %s\n"), Msg);

    exit(-1);
}
