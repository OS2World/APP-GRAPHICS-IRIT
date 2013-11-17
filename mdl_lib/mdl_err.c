/******************************************************************************
* Mdl_err.c - handler for all Model library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 96.					      *
******************************************************************************/

#include "mdl_loc.h"

typedef struct MdlErrorStruct {
    MdlFatalErrorType ErrorNum;
    char *ErrorDesc;
} MdlErrorStruct;

static MdlErrorStruct ErrMsgs[] =
{
    { MDL_ERR_PTR_REF,			IRIT_EXP_STR("Invalid reference pointer") },
    { MDL_ERR_TSEG_NO_SRF,		IRIT_EXP_STR("Trimming segment with no surfaces") },
    { MDL_ERR_BOOL_MERGE_FAIL,		IRIT_EXP_STR("Booleans failed in trimming segs merger") },
    { MDL_ERR_TSEG_NOT_FOUND,		IRIT_EXP_STR("Failed to find trimming segment index in MODEL") },
    { MDL_ERR_TSRF_NOT_FOUND,		IRIT_EXP_STR("Failed to find surface index in MODEL") },
    { MDL_ERR_FP_ERROR,			IRIT_EXP_STR("Floaing point error") },
    { MDL_ERR_BOOL_DISJOINT,		IRIT_EXP_STR("Objects might be disjoint - no intersections") },
    { MDL_ERR_BOOL_GET_REF,		IRIT_EXP_STR("Getting other reference of intersection curve failed") },
    { MDL_ERR_BOOL_CLASSIFY_FAIL,       IRIT_EXP_STR("Classification of curve failed (Curve too small!?)") },
    { MDL_ERR_BOOL_UVMATCH_FAIL,	IRIT_EXP_STR("Matching of neighboring UV curves failed (Try to tighten tolerances)") },

    { MDL_ERR_UNDEFINE_ERR,		NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this mdl library as well as other users. Raised error will   M
* cause an invokation of MdlFatalError function which decides how to handle  M
* this error. MdlFatalError can for example, invoke this routine with the    M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlDescribeError, error handling                                         M
*****************************************************************************/
const char *MdlDescribeError(MdlFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return "Undefined error";
}
