/*****************************************************************************
* Default warning error handler for irit.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, April 1993  *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "irit_sm.h"
#include "misc_loc.h"

IRIT_STATIC_DATA IritWarningMsgFuncType
    GlblIritWarningMsgFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the warning function of irit.		                             M
*                                                                            *
* PARAMETERS:                                                                M
*   WrnMsgFunc:      New function to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritWarningMsgFuncType:  Old function reference.	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSetWarningMsgFunc, error handling                                    M
*****************************************************************************/
IritWarningMsgFuncType IritSetWarningMsgFunc(IritWarningMsgFuncType WrnMsgFunc)
{
    IritWarningMsgFuncType
	OldErrorFunc = GlblIritWarningMsgFunc;

    GlblIritWarningMsgFunc = WrnMsgFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit warnings.			     M
*   This function just prints the given warning message.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message to print.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritWarningMsgPrintf, IritFatalError                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritWarningMsg, warning trap, error trap                                 M
*****************************************************************************/
void IritWarningMsg(const char *Msg)
{
    IRIT_STATIC_DATA char
	Line[IRIT_LINE_LEN_VLONG] = { 0 };

    if (IritLineHasCntrlChar(Msg)) {
        if (strchr(Msg, '\b') == NULL && Line[0] != 0) {
	    if (GlblIritWarningMsgFunc)
	        GlblIritWarningMsgFunc(Line);
	    else
		fprintf(stderr, IRIT_EXP_STR("Irit Warning: %s"), Line);

	    Line[0] = 0;
	}
        if (GlblIritWarningMsgFunc)
	    GlblIritWarningMsgFunc(Msg);
	else
	    fprintf(stderr, "%s", Msg);

	return;
    }

    /* Concat with accumulated message so far. */
    strncat(Line, Msg, IRIT_LINE_LEN_VLONG - 1);

    if (strlen(Line) > IRIT_LINE_LEN_VLONG - 2) {
        Line[IRIT_LINE_LEN_VLONG - 2] = 0;

        if (GlblIritWarningMsgFunc)
	    GlblIritWarningMsgFunc(IRIT_EXP_STR("Irit Warning buffer overflow:\n"));
	else
	    fprintf(stderr, IRIT_EXP_STR("Irit Warning buffer overflow:\n"));

        if (GlblIritWarningMsgFunc)
	    GlblIritWarningMsgFunc(Line);
	else
	    fprintf(stderr, IRIT_EXP_STR("Irit Warning: %s\n"), Line);
	Line[0] = 0;
    }
}
