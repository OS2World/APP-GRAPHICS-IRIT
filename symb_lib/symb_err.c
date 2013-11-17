/******************************************************************************
* Symb_err.c - handler for all symb library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/

#include "symb_loc.h"

typedef struct SymbErrorStruct {
    SymbFatalErrorType ErrorNum;
    char *ErrorDesc;
} SymbErrorStruct;

IRIT_STATIC_DATA SymbErrorStruct ErrMsgs[] =
{
    { SYMB_ERR_WRONG_SRF,		IRIT_EXP_STR("Provided surface type is wrong") },
    { SYMB_ERR_BZR_CRV_EXPECT,		IRIT_EXP_STR("Bezier curve is expected") },
    { SYMB_ERR_BZR_SRF_EXPECT,		IRIT_EXP_STR("Bezier surface is expected") },
    { SYMB_ERR_BSP_CRV_EXPECT,		IRIT_EXP_STR("Bspline curve is expected") },
    { SYMB_ERR_BSP_SRF_EXPECT,		IRIT_EXP_STR("Bspline surface is expected") },
    { SYMB_ERR_RATIONAL_EXPECTED,	IRIT_EXP_STR("Rational Crv/Srf expected.") },
    { SYMB_ERR_NO_CROSS_PROD,		IRIT_EXP_STR("No cross product for scalar surface") },
    { SYMB_ERR_POWER_NO_SUPPORT,	IRIT_EXP_STR("Power basis type is not supported") },
    { SYMB_ERR_CRV_FAIL_CMPT,		IRIT_EXP_STR("Cannot make curves compatible") },
    { SYMB_ERR_SRF_FAIL_CMPT,		IRIT_EXP_STR("Cannot make surfaces compatible") },
    { SYMB_ERR_UNDEF_CRV,		IRIT_EXP_STR("Undefined curve type") },
    { SYMB_ERR_UNDEF_SRF,		IRIT_EXP_STR("Undefined surface type") },
    { SYMB_ERR_UNDEF_GEOM,		IRIT_EXP_STR("Undefined geometry type") },
    { SYMB_ERR_OUT_OF_RANGE,		IRIT_EXP_STR("Data is out of range.") },
    { SYMB_ERR_DIR_NOT_CONST_UV,	IRIT_EXP_STR("Dir is not one of CONST_U/V_DIR") },
    { SYMB_ERR_REPARAM_NOT_MONOTONE,	IRIT_EXP_STR("Reparametrization is not monotone") },
    { SYMB_ERR_BSPLINE_NO_SUPPORT,	IRIT_EXP_STR("Bspline basis type is not supported") },
    { SYMB_ERR_WRONG_PT_TYPE,		IRIT_EXP_STR("Provided point type is wrong") },
    { SYMB_ERR_ONLY_2D_OR_3D,		IRIT_EXP_STR("Only two or three dimensions are supported") },
    { SYMB_ERR_ONLY_2D,			IRIT_EXP_STR("Only two dimensions are supported") },
    { SYMB_ERR_ONLY_3D,			IRIT_EXP_STR("Only three dimensions are supported") },
    { SYMB_ERR_RATIONAL_NO_SUPPORT,	IRIT_EXP_STR("Rational function is not supported") },
    { SYMB_ERR_SRFS_INCOMPATIBLE,	IRIT_EXP_STR("Surfaces for requested operation are incompatible") },
    { SYMB_ERR_CRVS_INCOMPATIBLE,	IRIT_EXP_STR("Curves for requested operation are incompatible") },
    { SYMB_ERR_CANNOT_COMP_NORMAL,	IRIT_EXP_STR("Cannot compute normal") },
    { SYMB_ERR_TOO_COMPLEX,		IRIT_EXP_STR("Too complex") },
    { SYMB_ERR_UNSUPPORT_PT,		IRIT_EXP_STR("Unsupported point type") },
    { SYMB_ERR_W_NOT_SAME,		IRIT_EXP_STR("Weights are not identical") },
    { SYMB_ERR_SCALAR_EXPECTED,		IRIT_EXP_STR("Scalar entity expected") },
    { SYMB_ERR_POLY_CONST_SRF,		IRIT_EXP_STR("Constant surfaces cannot be converted to polygons") },
    { SYMB_ERR_COPLANAR_GEOMETRY,	IRIT_EXP_STR("Coplanar geometry") },
    { SYMB_ERR_ILLEGAL_PARAMETERS,	IRIT_EXP_STR("Illegal/Invalid parameters") },
    { SYMB_ERR_INCONSIST_EDGE_BHOLE,    IRIT_EXP_STR("Inconsistent edge list in black hole fill") },
    { SYMB_ERR_BIARC_FIT_FAIL,		IRIT_EXP_STR("Biarc fitting failed") },
    { SYMB_ERR_SPL_PROD_FAILED,		IRIT_EXP_STR("Interpolating spline product failed") },
    { SYMB_ERR_MATCH_FAILED,		IRIT_EXP_STR("Matching procedure failed") },
    { SYMB_ERR_MINIMUM_LINEAR,		IRIT_EXP_STR("Minimum order expected linear") },
    { SYMB_ERR_DIV_ZERO,		IRIT_EXP_STR("Division by zero detected!") },
    { SYMB_ERR_INVALID_AXIS,		IRIT_EXP_STR("Invalid axis specification") },
    { SYMB_ERR_WRONG_KNOT_INDEX,	IRIT_EXP_STR("Invalid knot index requested") },
    { SYMB_ERR_SUBDIV_TREE_BEZ_ONLY,	IRIT_EXP_STR("Subdivision tree support for Bezier curves only") },
    { SYMB_ERR_IDENTICAL_ZERO_DATA,	IRIT_EXP_STR("Data that is identically zero detected") },

    { SYMB_ERR_UNDEFINE_ERR,		NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this symb library as well as other users. Raised error will  M
* cause an invokation of SymbFatalError function which decides how to handle M
* this error. SymbFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbDescribeError, error handling                                        M
*****************************************************************************/
const char *SymbDescribeError(SymbFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return IRIT_EXP_STR("Undefined error");
}
