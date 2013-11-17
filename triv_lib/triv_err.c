/******************************************************************************
* Triv_err.c - handler for all triv library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/

#include "triv_loc.h"

typedef struct TrivErrorStruct {
    TrivFatalErrorType ErrorNum;
    char *ErrorDesc;
} TrivErrorStruct;

IRIT_STATIC_DATA TrivErrorStruct ErrMsgs[] =
{
    { TRIV_ERR_DIR_NOT_VALID, 	IRIT_EXP_STR("Dir is not valid") },
    { TRIV_ERR_UNDEF_CRV,	IRIT_EXP_STR("Undefined curve type") },
    { TRIV_ERR_UNDEF_SRF,	IRIT_EXP_STR("Undefined surface type") },
    { TRIV_ERR_UNDEF_CRV,	IRIT_EXP_STR("Undefined curve type") },
    { TRIV_ERR_UNDEF_TRIVAR,	IRIT_EXP_STR("Undefined trivariate type") },
    { TRIV_ERR_UNDEF_GEOM,	IRIT_EXP_STR("Undefined geometry type") },
    { TRIV_ERR_UNSUPPORT_PT,    IRIT_EXP_STR("Unsupported point type") },
    { TRIV_ERR_RATIONAL_NO_SUPPORT, IRIT_EXP_STR("Rational function is not supported") },
    { TRIV_ERR_WRONG_ORDER,	IRIT_EXP_STR("Provided order is wrong") },
    { TRIV_ERR_KNOT_NOT_ORDERED,IRIT_EXP_STR("Provided knots are not in ascending order") },
    { TRIV_ERR_NUM_KNOT_MISMATCH,IRIT_EXP_STR("Number of knots does not match") },
    { TRIV_ERR_INDEX_NOT_IN_MESH,IRIT_EXP_STR("Index is out of mesh range") },
    { TRIV_ERR_POWER_NO_SUPPORT,IRIT_EXP_STR("Power basis type is not supported") },
    { TRIV_ERR_WRONG_DOMAIN,	IRIT_EXP_STR("Given parameter is not in domain") },
    { TRIV_ERR_INCONS_DOMAIN,	IRIT_EXP_STR("Inconsistent domain (must be between zero and length)") },
    { TRIV_ERR_DIR_NOT_CONST_UVW, IRIT_EXP_STR("Given direction is not U, V or W") },
    { TRIV_ERR_SCALAR_PT_EXPECTED,IRIT_EXP_STR("A scalar field trivariate is expected.") },
    { TRIV_ERR_INVALID_AXIS,     IRIT_EXP_STR("Invalid axis specification.") },
    { TRIV_ERR_NO_CLOSED_POLYGON,IRIT_EXP_STR("Failed to form a closed polygon.") },
    { TRIV_ERR_TWO_INTERSECTIONS,IRIT_EXP_STR("Should have found two intersections only.") },
    { TRIV_ERR_NO_MATCH_PAIR,    IRIT_EXP_STR("Cannot find matching pairs.") },
    { TRIV_ERR_2_OR_4_INTERS,    IRIT_EXP_STR("Only two or four intersections in a face.") },
    { TRIV_ERR_FAIL_FIND_PT,	 IRIT_EXP_STR("Failed to find next point.") },
    { TRIV_ERR_FAIL_READ_FILE,	 IRIT_EXP_STR("Failed to read from given file.") },
    { TRIV_ERR_INVALID_STROKE_TYPE,IRIT_EXP_STR("Invalid stroke type requested.") },
    { TRIV_ERR_READ_FAIL,	 IRIT_EXP_STR("Failed to read from file") },
    { TRIV_ERR_TVS_INCOMPATIBLE, IRIT_EXP_STR("Trivariates are in compatible") },
    { TRIV_ERR_PT_OR_LEN_MISMATCH,IRIT_EXP_STR("PtType or Length mismatch") },
    { TRIV_ERR_BSP_TV_EXPECT,	IRIT_EXP_STR("Bspline trivariate is expected") },
    { TRIV_ERR_PERIODIC_EXPECTED,IRIT_EXP_STR("Periodic geometry expected") },
    { TRIV_ERR_UNSUPPORT_DERIV,  IRIT_EXP_STR("Unsupported derivative") },

    { TRIV_ERR_UNDEFINE_ERR,	NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this triv library as well as other users. Raised error will  M
* cause an invokation of TrivFatalError function which decides how to handle M
* this error. TrivFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivDescribeError, error handling                                        M
*****************************************************************************/
const char *TrivDescribeError(TrivFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return IRIT_EXP_STR("Undefined error");
}
