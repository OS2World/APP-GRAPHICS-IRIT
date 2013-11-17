/******************************************************************************
* Misc_err.c - handler for all misc library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 04.					      *
******************************************************************************/

#include "misc_loc.h"

typedef struct MiscErrorStruct {
    MiscFatalErrorType ErrorNum;
    char *ErrorDesc;
} MiscErrorStruct;

IRIT_STATIC_DATA MiscErrorStruct ErrMsgs[] =
{
    { MISC_ERR_MALLOC_FAILED,		IRIT_EXP_STR("Failed to malloc dyn. mem.") },
    { MISC_ERR_CONFIG_FILE_NO_FOUND,	IRIT_EXP_STR("Config file was not found: ") },
    { MISC_ERR_CONFIG_ERROR,		IRIT_EXP_STR("Config file error: ") },
    { MISC_ERR_UNKNOWN_CONFIG,		IRIT_EXP_STR("Unknown configuration error.") },

    { MISC_ERR_UNDEFINE_ERR,		NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this misc library as well as other users. Raised error will  M
* cause an invokation of MiscFatalError function which decides how to handle M
* this error. MiscFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscDescribeError, error handling                                        M
*****************************************************************************/
const char *MiscDescribeError(MiscFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return "Undefined error";
}
