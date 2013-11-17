/******************************************************************************
* Mvar_dbg.c - Provide a routine to print multi variate objects to stderr.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include "mvar_loc.h"

static void MvarETDbgAux(const MvarExprTreeStruct *ET, int Depth);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints multi-variates to stderr. Should be linked to programs for          M
* debugging purposes, so multi-variates may be inspected from a debugger.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A multi-variate - to be printed to stderr.  		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarDbg, debugging                                                       M
*****************************************************************************/
void MvarDbg(const void *Obj)
{
    char
	*ErrorMsg = "?";
    const MvarMVStruct
	*MV = (const MvarMVStruct *) Obj;
    MvarGeomType 
	GType = MV -> GType;

    switch (GType) {
	case MVAR_POWER_TYPE:
	case MVAR_BEZIER_TYPE:
	case MVAR_BSPLINE_TYPE:
	    MvarMVWriteToFile3(MV, stderr, 0, IRIT_EXP_STR("MvarDbg"),
			       &ErrorMsg);
	    break;
	default:
	    ErrorMsg = "Undefined geometry";
	    break;
    }

    if (ErrorMsg)
	fprintf(stderr, IRIT_EXP_STR("MvarDbg Error: %s\n"), ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints multivariate expression tree to stderr. Should be linked to         M
* programs for debugging purposes, so multi-variates may be inspected from a M
* debugger.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ET:       A multivariate expression tree - to be printed to stderr.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarETDbg, debugging                                                     M
*****************************************************************************/
void MvarETDbg(const MvarExprTreeStruct *ET)
{
    MvarETDbgAux(ET, 0);
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxiliary function to function MvarETDbg				     *
*****************************************************************************/
static void MvarETDbgAux(const MvarExprTreeStruct *ET, int Depth)
{
    const char
        *NodeStr = "?";

    switch (ET -> NodeType) {
	case MVAR_ET_NODE_LEAF:
	    fprintf(stderr,
		    "******** Mvar Expression Tree, Depth = %d (Leaf, Ref = %d) *******\n",
		    Depth, ET -> IsRef);
	    MvarDbg(ET -> MV);
	    break;
	case MVAR_ET_NODE_ADD:
	case MVAR_ET_NODE_SUB:
	case MVAR_ET_NODE_MULT:
	    switch (ET -> NodeType) {
		case MVAR_ET_NODE_ADD:
		    NodeStr = "ADD";
		    break;
		case MVAR_ET_NODE_SUB:
		    NodeStr = "SUB";
		    break;
		case MVAR_ET_NODE_MULT:
		    NodeStr = "MULT";
		    break;
		default:
		    assert(0);
	    }
	    fprintf(stderr,
		    "******** Mvar Expression Tree, Depth = %d (%s) *******\n",
		    Depth, NodeStr);
	    MvarETDbgAux(ET -> Left, Depth + 1);
	    MvarETDbgAux(ET -> Right, Depth + 1);
	    break;
	case MVAR_ET_NODE_EXP:
	case MVAR_ET_NODE_LOG:
	    switch(ET -> NodeType) {
		case MVAR_ET_NODE_EXP:
		    NodeStr = "EXP";
		    break;
		case MVAR_ET_NODE_LOG:
		    NodeStr = "LOG";
		    break;
		default:
		    assert(0);
	    }
	    fprintf(stderr,
		    "******** Mvar Expression Tree, Depth = %d (%s) *******\n",
		    Depth, NodeStr);
	    MvarETDbgAux(ET -> Left, Depth + 1);
	    break;
	default:
	    fprintf(stderr, IRIT_EXP_STR("MvarETDbg Error: Undefined ET Node type\n"));
	    break;
    }
}
