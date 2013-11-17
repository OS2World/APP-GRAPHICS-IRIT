/******************************************************************************
* Cagd_err.c - handler for all cagd library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

typedef struct CagdErrorStruct {
    CagdFatalErrorType ErrorNum;
    char *ErrorDesc;
} CagdErrorStruct;

IRIT_STATIC_DATA CagdErrorStruct ErrMsgs[] =
{
    { CAGD_ERR_180_ARC,			IRIT_EXP_STR("Attempt to construct a 180 degrees arc") },
    { CAGD_ERR_AFD_NO_SUPPORT,		IRIT_EXP_STR("No support for such AFD operation.") },
    { CAGD_ERR_ALLOC_ERR,		IRIT_EXP_STR("Memory allocation error") },
    { CAGD_ERR_BSPLINE_NO_SUPPORT,	IRIT_EXP_STR("Bspline basis type is not supported") },
    { CAGD_ERR_BZR_CRV_EXPECT,		IRIT_EXP_STR("Bezier curve is expected") },
    { CAGD_ERR_BZR_SRF_EXPECT,		IRIT_EXP_STR("Bezier surface is expected") },
    { CAGD_ERR_BSP_CRV_EXPECT,		IRIT_EXP_STR("Bspline curve is expected") },
    { CAGD_ERR_BSP_SRF_EXPECT,		IRIT_EXP_STR("Bspline surface is expected") },
    { CAGD_ERR_CRV_FAIL_CMPT,		IRIT_EXP_STR("Cannot make curves compatible") },
    { CAGD_ERR_CRVS_INCOMPATIBLE,	IRIT_EXP_STR("Curves for requested operation are incompatible") },
    { CAGD_ERR_CUBIC_EXPECTED,		IRIT_EXP_STR("Cubic polynomial expected.") },
    { CAGD_ERR_DEGEN_ALPHA,		IRIT_EXP_STR("Degenerated Alpha matrix") },
    { CAGD_ERR_DIR_NOT_CONST_UV,	IRIT_EXP_STR("Dir is not one of CONST_U/V_DIR") },
    { CAGD_ERR_DIR_NOT_VALID,		IRIT_EXP_STR("Dir is not valid") },
    { CAGD_ERR_INDEX_NOT_IN_MESH,	IRIT_EXP_STR("Index is out of mesh range") },
    { CAGD_ERR_KNOT_NOT_ORDERED,	IRIT_EXP_STR("Provided knots are not in ascending order") },
    { CAGD_ERR_LIN_NO_SUPPORT,		IRIT_EXP_STR("Order below linear is not supported") },
    { CAGD_ERR_NO_CROSS_PROD,		IRIT_EXP_STR("No cross product for scalar surface") },
    { CAGD_ERR_NOT_ENOUGH_MEM,		IRIT_EXP_STR("Not enough memory, exit") },
    { CAGD_ERR_NOT_IMPLEMENTED,		IRIT_EXP_STR("Not implemented") },
    { CAGD_ERR_NUM_KNOT_MISMATCH,	IRIT_EXP_STR("Number of knots does not match") },
    { CAGD_ERR_OUT_OF_RANGE,		IRIT_EXP_STR("Data is out of range.") },
    { CAGD_ERR_PARSER_STACK_OV,		IRIT_EXP_STR("Parser Internal stack overflow..") },
    { CAGD_ERR_POWER_NO_SUPPORT,	IRIT_EXP_STR("Power basis type is not supported") },
    { CAGD_ERR_PT_OR_LEN_MISMATCH,	IRIT_EXP_STR("PtType or Length mismatch") },
    { CAGD_ERR_POLYNOMIAL_EXPECTED,	IRIT_EXP_STR("Polynomial Crv/Srf expected.") },
    { CAGD_ERR_RATIONAL_EXPECTED,	IRIT_EXP_STR("Rational Crv/Srf expected.") },
    { CAGD_ERR_SCALAR_EXPECTED,		IRIT_EXP_STR("Scalar entity expected") },
    { CAGD_ERR_SRF_FAIL_CMPT,		IRIT_EXP_STR("Cannot make surfaces compatible") },
    { CAGD_ERR_SRFS_INCOMPATIBLE,	IRIT_EXP_STR("Surfaces for requested operation are incompatible") },
    { CAGD_ERR_UNDEF_CRV,		IRIT_EXP_STR("Undefined curve type") },
    { CAGD_ERR_UNDEF_SRF,		IRIT_EXP_STR("Undefined surface type") },
    { CAGD_ERR_UNDEF_GEOM,		IRIT_EXP_STR("Undefined geometry type") },
    { CAGD_ERR_UNSUPPORT_PT,		IRIT_EXP_STR("Unsupported point type") },
    { CAGD_ERR_T_NOT_IN_CRV,		IRIT_EXP_STR("Given t is not in curve's parametric domain") },
    { CAGD_ERR_U_NOT_IN_SRF,		IRIT_EXP_STR("Given u is not in u surface's parametric domain") },
    { CAGD_ERR_V_NOT_IN_SRF,		IRIT_EXP_STR("Given v is not in v surface's parametric domain") },
    { CAGD_ERR_WRONG_DOMAIN,		IRIT_EXP_STR("Given parameter is not in domain") },
    { CAGD_ERR_W_NOT_SAME,		IRIT_EXP_STR("Weights are not identical") },
    { CAGD_ERR_W_ZERO,			IRIT_EXP_STR("Weights are zero") },
    { CAGD_ERR_WRONG_CRV,		IRIT_EXP_STR("Provided curve type is wrong") },
    { CAGD_ERR_WRONG_INDEX,		IRIT_EXP_STR("Provided index is wrong") },
    { CAGD_ERR_WRONG_ORDER,		IRIT_EXP_STR("Provided order is wrong") },
    { CAGD_ERR_WRONG_SRF,		IRIT_EXP_STR("Provided surface type is wrong") },
    { CAGD_ERR_WRONG_PT_TYPE,		IRIT_EXP_STR("Provided point type is wrong") },
    { CAGD_ERR_CANNOT_COMP_VEC_FIELD,	IRIT_EXP_STR("Cannot compute vec field/normal") },
    { CAGD_ERR_CANNOT_COMP_NORMAL,	IRIT_EXP_STR("Cannot compute normal") },
    { CAGD_ERR_REPARAM_NOT_MONOTONE,	IRIT_EXP_STR("Reparametrization is not monotone") },
    { CAGD_ERR_RATIONAL_NO_SUPPORT,	IRIT_EXP_STR("Rational function is not supported") },
    { CAGD_ERR_NO_SOLUTION,		IRIT_EXP_STR("No solution") },
    { CAGD_ERR_TOO_COMPLEX,		IRIT_EXP_STR("Too complex") },
    { CAGD_ERR_REF_LESS_ORIG,		IRIT_EXP_STR("Refined object smaller than original") },
    { CAGD_ERR_ONLY_2D_OR_3D,		IRIT_EXP_STR("Only two or three dimensions are supported") },
    { CAGD_ERR_ONLY_1D_TO_3D,		IRIT_EXP_STR("Only one to three dimensions are supported") },
    { CAGD_ERR_ONLY_2D,			IRIT_EXP_STR("Only two dimensions are supported") },
    { CAGD_ERR_DOMAIN_TOO_SMALL,	IRIT_EXP_STR("Parametric domain is too small") },
    { CAGD_ERR_PERIODIC_EXPECTED,	IRIT_EXP_STR("Periodic geometry expected") },
    { CAGD_ERR_PERIODIC_NO_SUPPORT,	IRIT_EXP_STR("Periodic geometry is not supported") },
    { CAGD_ERR_OPEN_EC_EXPECTED,	IRIT_EXP_STR("Open end conditions expected") },
    { CAGD_ERR_POLYGON_EXPECTED,	IRIT_EXP_STR("A polygon was expected") },
    { CAGD_ERR_POLYSTRIP_EXPECTED,	IRIT_EXP_STR("A polygonal strip was expected") },
    { CAGD_ERR_SWEEP_AXIS_TOO_COMPLEX,	IRIT_EXP_STR("Sweep axis too complex") },
    { CAGD_ERR_INVALID_CONIC_COEF,	IRIT_EXP_STR("Invalid conic section coefficients") },
    { CAGD_ERR_HYPERBOLA_NO_SUPPORT,	IRIT_EXP_STR("Hyperbola is not supported") },
    { CAGD_ERR_WRONG_DERIV_ORDER,	IRIT_EXP_STR("Wrong order of derivative") },
    { CAGD_ERR_NO_TOL_TEST_FUNC,	IRIT_EXP_STR("No tolerance testing routine") },
    { CAGD_ERR_NO_KV_FOUND,		IRIT_EXP_STR("No knot vector found, possible a Bezier") },
    { CAGD_ERR_WRONG_SIZE,		IRIT_EXP_STR("Wrong specification of size") },
    { CAGD_ERR_INVALID_CRV,	        IRIT_EXP_STR("Invalid curve detected") },
    { CAGD_ERR_INVALID_SRF,	        IRIT_EXP_STR("Invalid surface detected") },
    { CAGD_ERR_C0_KV_DETECTED,		IRIT_EXP_STR("C0 discontinuous KV detected") },

    { CAGD_ERR_UNDEFINE_ERR,		NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this cagd library as well as other users. Raised error will  M
* cause an invokation of CagdFatalError function which decides how to handle M
* this error. CagdFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdDescribeError, error handling                                        M
*****************************************************************************/
const char *CagdDescribeError(CagdFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return "Undefined error";
}
