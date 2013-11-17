/*****************************************************************************
* Default information printing handler for irit.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, April 1993  *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "irit_sm.h"
#include "misc_loc.h"

IRIT_STATIC_DATA IritInfoMsgFuncType
    GlblIritInfoMsgFunc = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the warning function of irit.		                             M
*                                                                            *
* PARAMETERS:                                                                M
*   InfoMsgFunc:      New function to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritInfoMsgFuncType:  Old function reference.	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSetInfoMsgFunc, error handling                                       M
*****************************************************************************/
IritInfoMsgFuncType IritSetInfoMsgFunc(IritInfoMsgFuncType InfoMsgFunc)
{
    IritInfoMsgFuncType
	OldErrorFunc = GlblIritInfoMsgFunc;

    GlblIritInfoMsgFunc = InfoMsgFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if the given line contains control chars.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Line: Line to examine for control char's existence.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if this line holds control chars, FALSE otherwise.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritLineHasCntrlChar                                                     M
*****************************************************************************/
int IritLineHasCntrlChar(const char *Line)
{
    int i,
	n = (int) strlen(Line);

    for (i = 0; i < n; i++) {
        if (Line[i] < ' ' && Line[i] != 0x09 /* TAB */)
	    return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit information.			     M
*   This function just prints the given message.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message to print.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritInformationMsgPrintf, IritFatalError, IritWarningMsg                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritInformationMsg, information messages                                 M
*****************************************************************************/
void IritInformationMsg(const char *Msg)
{
    IRIT_STATIC_DATA char
	Line[IRIT_LINE_LEN_VLONG] = { 0 };

    if (IritLineHasCntrlChar(Msg)) {
        if (strchr(Msg, '\b') == NULL && Line[0] != 0) {
	    if (GlblIritInfoMsgFunc)
	        GlblIritInfoMsgFunc(Line);
	    else
		fprintf(stderr, IRIT_EXP_STR("Irit Info: %s"), Line);

	    Line[0] = 0;
	}

        if (GlblIritInfoMsgFunc)
	    GlblIritInfoMsgFunc(Msg);
	else
	    fprintf(stderr, "%s", Msg);

	return;
    }

    /* Concat with accumulated message so far. */
    strncat(Line, Msg, IRIT_LINE_LEN_VLONG - 1);

    if (strlen(Line) > IRIT_LINE_LEN_VLONG - 2) {
        Line[IRIT_LINE_LEN_VLONG - 2] = 0;

        if (GlblIritInfoMsgFunc)
	    GlblIritInfoMsgFunc(IRIT_EXP_STR("Irit Information buffer overflow:\n"));
	else
	    fprintf(stderr, IRIT_EXP_STR("Irit Information buffer overflow:\n"));

        if (GlblIritInfoMsgFunc)
	    GlblIritInfoMsgFunc(Line);
	else
	    fprintf(stderr, IRIT_EXP_STR("Irit Info: %s\n"), Line);
	Line[0] = 0;
    }
}
