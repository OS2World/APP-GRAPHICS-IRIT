/******************************************************************************
* Triv_dbg.c - Provide a routine to print Trivariate objects to stderr.       *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec. 94.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints trivariates stderr. Should be linked to programs for debugging      M
* purposes, so trivariates may be inspected from a debugger.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A trivariate - to be printed to stderr.  		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivDbg, debugging                                                       M
*****************************************************************************/
void TrivDbg(const void *Obj)
{
    char
	*ErrorMsg = NULL;
    const TrivTVStruct
	*TV = (const TrivTVStruct *) Obj;
    TrivGeomType 
	GType = TV -> GType;

    switch (GType) {
	case TRIV_TVBEZIER_TYPE:
	case TRIV_TVBSPLINE_TYPE:
	    TrivTVWriteToFile3(TV, stderr, 0, IRIT_EXP_STR("TrivDbg"),
			       &ErrorMsg);
	    break;
        default:
	    ErrorMsg = "Undefined geometry";
	    break;
    }

    if (ErrorMsg)
	fprintf(stderr, IRIT_EXP_STR("TrivDbg Error: %s\n"), ErrorMsg);
}
