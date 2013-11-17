/******************************************************************************
* User_err.c - handler for all user library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/

#include "irit_sm.h"
#include "user_loc.h"

typedef struct UserErrorStruct {
    UserFatalErrorType ErrorNum;
    char *ErrorDesc;
} UserErrorStruct;

IRIT_STATIC_DATA UserErrorStruct ErrMsgs[] =
{
    { USER_ERR_WRONG_SRF,		IRIT_EXP_STR("Provided surface type is wrong") },
    { USER_ERR_MISSING_ATTRIB,		IRIT_EXP_STR("Missing attribute that must be here") },
    { USER_ERR_WRONG_ANGLE,		IRIT_EXP_STR("Given angle is out of range") },
    { USER_ERR_INVALID_PLANE,		IRIT_EXP_STR("Invalid plane equation") },
    { USER_ERR_RATIONAL_NO_SUPPORT,	IRIT_EXP_STR("Rational function is not supported") },
    { USER_ERR_NON_CRV_OBJ_IN_FONT,     IRIT_EXP_STR("A non curve object found in font") },
    { USER_ERR_NO_ADJ_INFO,		IRIT_EXP_STR("No polygonal adjacency information found") },
    { USER_ERR_NO_NRML_INFO,		IRIT_EXP_STR("No polygonal normal information found") },
    { USER_ERR_NO_CRVTR_INFO,		IRIT_EXP_STR("No polygonal curvature information found") },
    { USER_ERR_EXPCT_REG_TRIANG,	IRIT_EXP_STR("Expecting regular triangles only") },
    { USER_ERR_EXPCT_POLY_OBJ,		IRIT_EXP_STR("Expecting a poly object") },
    { USER_ERR_EXPCT_SRF_OBJ,		IRIT_EXP_STR("Expecting a surface object") },
    { USER_ERR_EXPCT_VRTX_NRMLS,	IRIT_EXP_STR("Expecting vertices' normal") },
    { USER_ERR_EXPCT_VRTX_UVS,		IRIT_EXP_STR("Expecting vertices' UV params") },
    { USER_ERR_WRONG_CTLPT_INDEX,	IRIT_EXP_STR("Wrong control point index") },
    { USER_ERR_INVALID_SIZE,		IRIT_EXP_STR("Wrong size prescribed") },
    { USER_ERR_INVALID_CURVE,		IRIT_EXP_STR("Wrong curve prescribed") },
    { USER_ERR_INVALID_SURFACE,		IRIT_EXP_STR("Wrong surface prescribed") },
    { USER_ERR_INVALID_DIR,		IRIT_EXP_STR("Wrong direction prescribed") },
    { USER_ERR_INVALID_IMAGE_SIZE,	IRIT_EXP_STR("Wrong image(s) size") },

    { USER_ERR_NC_MIX_CRVS_PLLNS,	IRIT_EXP_STR("Cannot mix curves and polyline") },
    { USER_ERR_NC_INVALID_PARAM,	IRIT_EXP_STR("Invalid NC tool path parameter") },
    { USER_ERR_NC_INVALID_INTER,	IRIT_EXP_STR("Invalid NC intersections detected") },
    { USER_ERR_NC_NO_POLYLINES,		IRIT_EXP_STR("2D pockets cannot be formed ot of polylines") },

    { USER_ERR_UNDEFINE_ERR,		NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this user library as well as other users. Raised error will  M
* cause an invokation of UserFatalError function which decides how to handle M
* this error. UserFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserDescribeError, error handling                                        M
*****************************************************************************/
const char *UserDescribeError(UserFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return IRIT_EXP_STR("Undefined error");
}
