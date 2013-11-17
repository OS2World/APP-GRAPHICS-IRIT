/******************************************************************************
* Trim_err.c - handler for all trim library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, October 94.					      *
******************************************************************************/

#include "trim_loc.h"

typedef struct TrimErrorStruct {
    TrimFatalErrorType ErrorNum;
    char *ErrorDesc;
} TrimErrorStruct;

IRIT_STATIC_DATA TrimErrorStruct ErrMsgs[] =
{
    { TRIM_ERR_TRIM_CRV_E2,	IRIT_EXP_STR("Trimming curve must have E2 point type.") },
    { TRIM_ERR_BSPLINE_EXPECT,	IRIT_EXP_STR("Bspline representation expected.") },
    { TRIM_ERR_BZR_BSP_EXPECT,	IRIT_EXP_STR("Bezier or Bspline representation expected.") },
    { TRIM_ERR_DIR_NOT_CONST_UV,IRIT_EXP_STR("Dir is not one of CONST_U/V_DIR.") },
    { TRIM_ERR_ODD_NUM_OF_INTER,IRIT_EXP_STR("Odd # of intersections with trimming crvs.") },
    { TRIM_ERR_TCRV_ORIENT,     IRIT_EXP_STR("Improper trimmingcurve orientations.") },
    { TRIM_ERR_INCONSISTENT_CNTRS, IRIT_EXP_STR("Inconsistent contours detected.") },
    { TRIM_ERR_FAIL_MERGE_TRIM_SEG, IRIT_EXP_STR("Failed to merge trimming curves' segments.") },
    { TRIM_ERR_INVALID_TRIM_SEG, IRIT_EXP_STR("Invalid trimming segment found in polygonal approximation.") },
    { TRIM_ERR_INCON_PLGN_CELL, IRIT_EXP_STR("Inconsistent cell in polygonization.") },
    { TRIM_ERR_TRIM_TOO_COMPLEX,IRIT_EXP_STR("Trimming curves too complex.") },
    { TRIM_ERR_TRIMS_NOT_LOOPS, IRIT_EXP_STR("Trimming curves do not form loops.") },
    { TRIM_ERR_LINEAR_TRIM_EXPECT, IRIT_EXP_STR("Linear trimming curve expected.") },
    { TRIM_ERR_NO_INTERSECTION, IRIT_EXP_STR("Failed to derive ray loop intersection.") },
    { TRIM_ERR_POWER_NO_SUPPORT,IRIT_EXP_STR("Power basis type is not supported") },
    { TRIM_ERR_UNDEF_SRF,	IRIT_EXP_STR("Undefined surface type") },
    { TRIM_ERR_TRIM_OPEN_LOOP,  IRIT_EXP_STR("Trimming loop was found open") },

    { TRIM_ERR_UNDEFINE_ERR,	NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this trim library as well as other users. Raised error will  M
* cause an invokation of TrimFatalError function which decides how to handle M
* this error. TrimFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimDescribeError, error handling                                        M
*****************************************************************************/
const char *TrimDescribeError(TrimFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return IRIT_EXP_STR("Undefined error");
}
