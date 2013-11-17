/******************************************************************************
* Trng_err.c - handler for all trng library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "trng_loc.h"

typedef struct TrngErrorStruct {
    TrngFatalErrorType ErrorNum;
    char *ErrorDesc;
} TrngErrorStruct;

IRIT_STATIC_DATA TrngErrorStruct ErrMsgs[] =
{
    { TRNG_ERR_DIR_NOT_VALID,	IRIT_EXP_STR("Dir is not valid") },
    { TRNG_ERR_UNDEF_GEOM,	IRIT_EXP_STR("Undefined geometry type") },
    { TRNG_ERR_WRONG_DOMAIN,	IRIT_EXP_STR("Given parameter is not in domain") },
    { TRNG_ERR_WRONG_ORDER,	IRIT_EXP_STR("Provided order is wrong") },
    { TRNG_ERR_BSPLINE_NO_SUPPORT, IRIT_EXP_STR("Bspline basis type is not supported") },
    { TRNG_ERR_GREGORY_NO_SUPPORT, IRIT_EXP_STR("Gregory type is not supported") },

    { TRNG_ERR_UNDEFINE_ERR,	NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this trng library as well as other users. Raised error will  M
* cause an invokation of TrngFatalError function which decides how to handle M
* this error. TrngFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngDescribeError, error handling                                        M
*****************************************************************************/
const char *TrngDescribeError(TrngFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return IRIT_EXP_STR("Undefined error");
}
