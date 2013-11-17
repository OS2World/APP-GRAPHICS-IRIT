/******************************************************************************
* Geom_err.c - handler for all geom library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 98.					      *
******************************************************************************/

#include "geom_loc.h"

typedef struct GeomErrorStruct {
    GeomFatalErrorType ErrorNum;
    char *ErrorDesc;
} GeomErrorStruct;

IRIT_STATIC_DATA GeomErrorStruct ErrMsgs[] =
{
    { GEOM_ERR_NO_OGL_SUPPORT,       IRIT_EXP_STR("Open GL is not supported in this environment") },
    { GEOM_ERR_OGL_NO_X_SERVER,      IRIT_EXP_STR("OGL: Failed to access X server") },
    { GEOM_ERR_X_NO_OGL_SERVER,      IRIT_EXP_STR("OGL: X server with no OGL support") },
    { GEOM_ERR_X_NO_VISUAL,          IRIT_EXP_STR("OGL: X server failed in visual allocation") },
    { GEOM_ERR_X_NO_GCONTEXT,        IRIT_EXP_STR("OGL: X failed to create graphics context") },
    { GEOM_ERR_CH_STACK_OVERFLOW,    IRIT_EXP_STR("Stack overflow for convex hull computation") },
    { GEOM_ERR_CH_STACK_UNDERFLOW,   IRIT_EXP_STR("Stack underflow for convex hull computation") },
    { GEOM_ERR_NO_INSTANCE_ORIGIN,   IRIT_EXP_STR("Failed to find instance's origin") },
    { GEOM_ERR_ANIM_MAT_OR_CRV,      IRIT_EXP_STR("Only matrics and curves are supported in animation") },
    { GEOM_ERR_UNKNOWN_ANIM_CRVS,    IRIT_EXP_STR("Unknown animation curve is ignored") },
    { GEOM_ERR_NO_ANIM_CRVS,         IRIT_EXP_STR("No animation attributes were found") },
    { GEOM_ERR_UNEQUAL_NUM_OF_POLYS, IRIT_EXP_STR("Unequal number of polygons is given") },
    { GEOM_ERR_UNEQUAL_NUM_OF_VRTXS, IRIT_EXP_STR("Unequal number of vertices is given") },
    { GEOM_ERR_TOO_MANY_ADJACENCIES, IRIT_EXP_STR("More than one adjacency detected") },
    { GEOM_ERR_NO_IRIT_PATH,	     IRIT_EXP_STR("IRIT_PATH env. not set. Cannot load irit font") },
    { GEOM_ERR_INVALID_FONT,	     IRIT_EXP_STR("Unable to read font or invalid format") },
    { GEOM_ERR_MSC_TOO_FEW_PTS,	     IRIT_EXP_STR("Too few points to use in minimum spanning circ/cone/sphere") },
    { GEOM_ERR_MSC_COLIN_CIRC,	     IRIT_EXP_STR("Collinear pts in attempt to derive min span circ/cone/sphere") },
    { GEOM_ERR_TRIANGLES_ONLY,	     IRIT_EXP_STR("Expecting only triangles at this time") },
    { GEOM_ERR_INVALID_POLYGON,	     IRIT_EXP_STR("Invalid polygon (or NULL terminated poly)") },
    { GEOM_ERR_VRTX_MTCH_FAILED,     IRIT_EXP_STR("Matching of vertices failed") },
    { GEOM_ERR_EXPCT_POLYHEDRA,      IRIT_EXP_STR("Expecting a polyhedra model") },
    { GEOM_ERR_EXPCT_POLYLINE,       IRIT_EXP_STR("Expecting a polyline model") },
    { GEOM_ERR_EXPCT_LIST_OBJ,       IRIT_EXP_STR("Expecting a list object") },
    { GEOM_ERR_EXPCT_TWO_PTS,        IRIT_EXP_STR("Expected at least two points") },
    { GEOM_ERR_PROJ_FAILED,	     IRIT_EXP_STR("Failed to find a projection plane") },
    { GEOM_ERR_DECIM_BDRY_FAILED,    IRIT_EXP_STR("Decimation for given object's boundaries failed") },
    { GEOM_ERR_OPEN_OBJ_VOL_COMP,    IRIT_EXP_STR("Open object detected in volume computation") },
    { GEOM_ERR_NO_INV_MAT,	     IRIT_EXP_STR("No inverse matrix exists") },
    { GEOM_ERR_NO_POLY_PLANE,	     IRIT_EXP_STR("No polygon plane detected") },
    { GEOM_ERR_NO_VRTX_NRML,	     IRIT_EXP_STR("No vertex normals detected") },
    { GEOM_ERR_REGULAR_POLY,	     IRIT_EXP_STR("Regular model expected") },
    { GEOM_ERR_REORIENT_STACK_OF,    IRIT_EXP_STR("Reorient stack overflow, object too large") },
    { GEOM_ERR_DISJOINT_PARTS,	     IRIT_EXP_STR("Polygonal object with disjoint parts detected") },
    { GEOM_ERR_VRTX_MUST_HAVE_NRML,  IRIT_EXP_STR("Vertices must have normals") },
    { GEOM_ERR_MISS_VRTX_IDX,	     IRIT_EXP_STR("Vertices must have index") },
    { GEOM_ERR_CMPLX_T_JUNC,	     IRIT_EXP_STR("Complex T junction detected and ignored") },

    { GEOM_ERR_UNDEFINE_ERR,	     NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this geom library as well as other users. Raised error will  M
* cause an invokation of GeomFatalError function which decides how to handle M
* this error. GeomFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GeomDescribeError, error handling                                        M
*****************************************************************************/
const char *GeomDescribeError(GeomFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return "Undefined error";
}
