/******************************************************************************
* Bool_err.c - handler for all bool library fatal errors.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/


#include <stdio.h>
#include "irit_sm.h"
#include "allocate.h"
#include "bool_loc.h"

typedef struct BoolErrorStruct {
    BoolFatalErrorType ErrorNum;
    char *ErrorDesc;
} BoolErrorStruct;

IRIT_STATIC_DATA BoolErrorStruct ErrMsgs[] =
{
    { BOOL_ERR_NO_POLY_OBJ,	    IRIT_EXP_STR("Operation on non polygonal object(s)") },
    { BOOL_ERR_NO_BOOL_OP_SUPPORT,  IRIT_EXP_STR("Undefined Boolean operation") },
    { BOOL_ERR_NO_MATCH_POINT,	    IRIT_EXP_STR("Failed to find matching point") },
    { BOOL_ERR_NO_ELMNT_TO_DEL,	    IRIT_EXP_STR("Element to delete not found") },
    { BOOL_ERR_SORT_INTER_LIST,	    IRIT_EXP_STR("Failed to sort intersection list") },
    { BOOL_ERR_FIND_VERTEX_FAILED,  IRIT_EXP_STR("Failed to find vertex") },
    { BOOL_ERR_NO_COPLANAR_VRTX,    IRIT_EXP_STR("Failed to find non coplanar point") },
    { BOOL_ERR_NO_OPEN_LOOP,	    IRIT_EXP_STR("None open loop") },
    { BOOL_ERR_NO_NEWER_VERTEX,	    IRIT_EXP_STR("Failed to find newer vertex") },
    { BOOL_ERR_NO_INTERSECTION,	    IRIT_EXP_STR("Failed to find intersection") },
    { BOOL_ERR_LOOP_LESS_3_VRTCS,   IRIT_EXP_STR("Closed loop with fewer than 3 vertices") },
    { BOOL_ERR_NO_INVERSE_MAT,	    IRIT_EXP_STR("Inverse matrix does not exists") },
    { BOOL_ERR_ADJ_STACK_OF,	    IRIT_EXP_STR("Adjacency stack overflow, object too large") },
    { BOOL_ERR_CIRC_VRTX_LST,	    IRIT_EXP_STR("Vertex list must be circular for proper adjacencies") },
    { BOOL_ERR_NO_2D_OP_SUPPORT,    IRIT_EXP_STR("Unsupported 2D Boolean operation") },
    { BOOL_ERR_NO_PLLN_MATCH,	    IRIT_EXP_STR("Failed to match polylines") },
    { BOOL_ERR_DISJ_PROP_ERR,	    IRIT_EXP_STR("Disjoint propagation error") },
    { BOOL_ERR_EMPTY_POLY_OBJ,      IRIT_EXP_STR("Empty polygon object was detected") },

    { BOOL_ERR_UNDEFINE_ERR,	    NULL }
};

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a string describing a the given error. Errors can be raised by     M
* any member of this bool library as well as other users. Raised error will  M
* cause an invocation of BoolFatalError function which decides how to handle M
* this error. BoolFatalError can for example, invoke this routine with the   M
* error type, print the appropriate message and quit the program.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorNum:   Type of the error that was raised.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   const char *:     A string describing the error type.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolDescribeError, error handling                                        M
*****************************************************************************/
const char *BoolDescribeError(BoolFatalErrorType ErrorNum)
{
    int i = 0;

    for ( ; ErrMsgs[i].ErrorDesc != NULL; i++)
	if (ErrorNum == ErrMsgs[i].ErrorNum)
	    return ErrMsgs[i].ErrorDesc;

    return "Undefined error";
}
