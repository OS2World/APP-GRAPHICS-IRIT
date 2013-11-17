/******************************************************************************
* Mvar_err.c - handler for all mvar library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include "mvar_loc.h"

typedef struct MvarErrorStruct {
    MvarFatalErrorType ErrorNum;
    char *ErrorDesc;
} MvarErrorStruct;

IRIT_STATIC_DATA MvarErrorStruct ErrMsgs[] =
{
    { MVAR_ERR_DIR_NOT_VALID,	   IRIT_EXP_STR("Dir is not valid") },
    { MVAR_ERR_UNDEF_CRV,	   IRIT_EXP_STR("Undefined curve type") },
    { MVAR_ERR_UNDEF_SRF,	   IRIT_EXP_STR("Undefined surface type") },
    { MVAR_ERR_UNDEF_CRV,	   IRIT_EXP_STR("Undefined curve type") },
    { MVAR_ERR_UNDEF_MVAR,	   IRIT_EXP_STR("Undefined multi-variate type") },
    { MVAR_ERR_UNDEF_GEOM,	   IRIT_EXP_STR("Undefined geometry type") },
    { MVAR_ERR_GEOM_NO_SUPPORT,    IRIT_EXP_STR("Given geometry is not supported") },
    { MVAR_ERR_RATIONAL_NO_SUPPORT,IRIT_EXP_STR("Rational function is not supported") },
    { MVAR_ERR_RATIONAL_EXPECTED,  IRIT_EXP_STR("Rational function expected") },
    { MVAR_ERR_WRONG_ORDER,	   IRIT_EXP_STR("Provided order is wrong") },
    { MVAR_ERR_KNOT_NOT_ORDERED,   IRIT_EXP_STR("Provided knots are not in ascending order") },
    { MVAR_ERR_NUM_KNOT_MISMATCH,  IRIT_EXP_STR("Number of knots does not match") },
    { MVAR_ERR_INDEX_NOT_IN_MESH,  IRIT_EXP_STR("Index is out of mesh range") },
    { MVAR_ERR_POWER_NO_SUPPORT,   IRIT_EXP_STR("Power basis type is not supported") },
    { MVAR_ERR_WRONG_DOMAIN,	   IRIT_EXP_STR("Given parameter is not in domain") },
    { MVAR_ERR_INCONS_DOMAIN,	   IRIT_EXP_STR("Inconsistent domain (must be between zero and length)") },
    { MVAR_ERR_SCALAR_PT_EXPECTED, IRIT_EXP_STR("A scalar field multivariate is expected.") },
    { MVAR_ERR_INVALID_AXIS,       IRIT_EXP_STR("Invalid axis specification.") },
    { MVAR_ERR_NO_CLOSED_POLYGON,  IRIT_EXP_STR("Failed to form a closed polygon.") },
    { MVAR_ERR_TWO_INTERSECTIONS,  IRIT_EXP_STR("Should have found two intersections only.") },
    { MVAR_ERR_NO_MATCH_PAIR,      IRIT_EXP_STR("Cannot find matching pairs.") },
    { MVAR_ERR_FAIL_READ_FILE,	   IRIT_EXP_STR("Failed to read from given file.") },
    { MVAR_ERR_INVALID_STROKE_TYPE,IRIT_EXP_STR("Invalid stroke type requested.") },
    { MVAR_ERR_READ_FAIL,	   IRIT_EXP_STR("Failed to read from file") },
    { MVAR_ERR_MVS_INCOMPATIBLE,   IRIT_EXP_STR("Mvariates are incompatible") },
    { MVAR_ERR_PT_OR_LEN_MISMATCH, IRIT_EXP_STR("PtType or Length mismatch") },
    { MVAR_ERR_TOO_FEW_PARAMS,	   IRIT_EXP_STR("Not enough parameters to compute") },
    { MVAR_ERR_TOO_MANY_PARAMS,	   IRIT_EXP_STR("Too many parameters to compute") },
    { MVAR_ERR_FAIL_CMPT,	   IRIT_EXP_STR("Failed multivariate compatibility") },
    { MVAR_ERR_NO_CROSS_PROD,      IRIT_EXP_STR("Must be at least E3 for a Cross product" )}, 
    { MVAR_ERR_BEZIER_EXPECTED,    IRIT_EXP_STR("Multivariate Bezier expected") },
    { MVAR_ERR_BSPLINE_EXPECTED,   IRIT_EXP_STR("Multivariate Bspline expected") },
    { MVAR_ERR_BEZ_OR_BSP_EXPECTED,IRIT_EXP_STR("Multivariate Bezier or Bspline expected") },
    { MVAR_ERR_SAME_GTYPE_EXPECTED,IRIT_EXP_STR("Multivariate of same geom type expected") },
    { MVAR_ERR_SAME_PTYPE_EXPECTED,IRIT_EXP_STR("Multivariate of same point type expected") },
    { MVAR_ERR_ONE_OR_THREE_EXPECTED,IRIT_EXP_STR("Expected either one or three multivariate(s)") },
    { MVAR_ERR_POWER_EXPECTED,     IRIT_EXP_STR("Multivariate power basis expected") },
    { MVAR_ERR_MSC_TOO_FEW_OBJ,    IRIT_EXP_STR("Attempting to compute spanning circle to too few entities") },
    { MVAR_ERR_MSC_FAILED,	   IRIT_EXP_STR("Minimum spanning circle failed") },
    { MVAR_ERR_MSS_INCONSISTENT_NUM_OBJ,IRIT_EXP_STR("Inconsistent number of input multivariates") },
    { MVAR_ERR_SCALAR_EXPECTED,    IRIT_EXP_STR("A scalar field expected") },
    { MVAR_ERR_DIM_TOO_HIGH,       IRIT_EXP_STR("Dimension of multivariate too high") },
    { MVAR_ERR_INVALID_MV,	   IRIT_EXP_STR("Multivariate is invalid") },
    { MVAR_ERR_CANNT_MIX_BSP_BEZ,  IRIT_EXP_STR("Cannot mix Bspline and Bezier MV constraints") },
    { MVAR_ERR_CH_FAILED,	   IRIT_EXP_STR("Convex Hull computation failed") },
    { MVAR_ERR_MSC_CURVES,	   IRIT_EXP_STR("MSC computation of curves failed") },
    { MVAR_ERR_ONLY_2D,		   IRIT_EXP_STR("2D Geometry was expected") },
    { MVAR_ERR_ONLY_3D,		   IRIT_EXP_STR("3D Geometry was expected") },
    { MVAR_ERR_2D_OR_3D,	   IRIT_EXP_STR("2D or 3D Geometry was expected") },
    { MVAR_ERR_1D_OR_3D,	   IRIT_EXP_STR("1D or 3D Geometry was expected") },
    { MVAR_ERR_WRONG_INDEX,	   IRIT_EXP_STR("Wrong index") },
    { MVAR_ERR_MSC_TOO_FEW_PTS,    IRIT_EXP_STR("Too few points to use in minimum spanning cone") },
    { MVAR_ERR_ET_DFRNT_DOMAINS,   IRIT_EXP_STR("Given two expression trees do not share domains") },
    { MVAR_ERR_SRF_NOT_ADJ,	   IRIT_EXP_STR("Surfaces are not adjacent") },
    { MVAR_ERR_CURVATURE_CONT,	   IRIT_EXP_STR("Curvature continuous input expected") },

    { MVAR_ERR_UNDEFINE_ERR,	NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this mvar library as well as other users. Raised error will  M
* cause an invokation of MvarFatalError function which decides how to handle M
* this error. MvarFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarDescribeError, error handling                                        M
*****************************************************************************/
const char *MvarDescribeError(MvarFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return "Undefined error";
}
