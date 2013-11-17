/******************************************************************************
* Trng_dbg.c - Provide a routine to print Triangular surfaces to stderr.      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints triangular surface to stderr. Should be linked to programs for      M
* debugging purposes, so triangular surfaces may be inspected from a         M
* debugger.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A triangular surface - to be printed to stderr.  	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngDbg, debugging                                                       M
*****************************************************************************/
void TrngDbg(void *Obj)
{
    char
        *ErrorMsg = "?";
    TrngTriangSrfStruct
	*TriSrf = (TrngTriangSrfStruct *) Obj;
    TrngGeomType 
	GType = TriSrf -> GType;

    switch (GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	case TRNG_TRISRF_BSPLINE_TYPE:
	    TrngTriSrfWriteToFile3(TriSrf, stderr, 0, IRIT_EXP_STR("TrngDbg"),
				   &ErrorMsg);
	    break;
	case TRNG_UNDEF_TYPE:
	    break;
        default:
	    assert(0);

    }

    if (ErrorMsg)
	fprintf(stderr, IRIT_EXP_STR("TrngDbg Error: %s\n"), ErrorMsg);
}
