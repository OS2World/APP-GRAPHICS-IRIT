/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 2002   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to parse strings as valid IRIT syntax.			     *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "program.h"
#include "allocate.h"
#include "attribut.h"
#include "inptprsg.h"
#include "inptprsl.h"

#define IP_EXPLICIT_MAX_DEGREE 100

static IPObjectStruct *IPProcessExplicitParseTree(ParseTree *PTree,
						  int NumVars);
static int IPProcessExpPTreeIAux(int *Degrees, ParseTree *PTree, int NumVars);
static int IPProcessExpPTreeIAux2(int *Degrees, ParseTree *PTree, int NumVars);
static int IPProcessExpPTreeIIAux(ParseTree *PTree,
				  int NumVars,
				  int Sign,
				  MvarMVStruct *MV);
static int IPProcessExpPTreeIIAux2(int *Degrees,
				   IrtRType *Coef,
				   ParseTree *PTree,
				   int NumVars);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts a regular expression with A..Z as variables into explicit       M
* multivariate function.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:        Expected dimension of the multivariate.                      M
*   Expr:       To convert, for example "A^2 + B^2".                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A multivariate Bezier representing the above.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptEvalMVExplicit                                                       M
*****************************************************************************/
IPObjectStruct *InptEvalMVExplicit(IrtRType *Dim, const char *Expr)
{
    char Name[2];
    int i, NumVars;
    ParseTree *PTree;
    IPObjectStruct
	*PRetObj = NULL,
	*VarList = NULL;

    InptPrsrQueueInputLine(Expr);
    if (strchr(Expr, ';') == NULL)
        InptPrsrQueueInputLine(";");

    for (NumVars = 26; NumVars > 0; NumVars--)
	if (strchr(Expr, 'a' + NumVars - 1) != NULL ||
	    strchr(Expr, 'A' + NumVars - 1) != NULL)
	    break;

    if (NumVars == 0) {
	IRIT_NON_FATAL_ERROR("MVExplicit: No variables in expression");
	return NULL;
    }
    else if (NumVars > IRIT_REAL_PTR_TO_INT(Dim)) {
	IRIT_NON_FATAL_ERROR("MVExplicit: Found variable larger than dimension");
	return NULL;
    }
    NumVars = IRIT_REAL_PTR_TO_INT(Dim);

    /* Allocate the variables for further use and binding. */
    Name[1] = 0;
    for (i = 0; i < NumVars; i++) {
	IrtRType
	    R = 1.0;

	Name[0] = 'A' + i;
	VarList = IPGenNumObject(Name, &R, VarList);
	VarList -> Count = 1;
    }
    IritDBPush(VarList);

    PTree = InptPrsrGenInputParseTree();	     /* Generate parse tree. */

    if (PTree != NULL) {
	PRetObj = IPProcessExplicitParseTree(PTree, NumVars);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintExplicitTree, FALSE) {
	        char Str[IP_EXPR_MAX_STRING_LEN];

		InptPrsrPrintTree(PTree, Str, IP_EXPR_MAX_STRING_LEN);

		IRIT_INFO_MSG_PRINTF("Expr (%d vars): %s\n", NumVars, Str);
	    }
	}
#	endif /* DEBUG */

	InptPrsrFreeTree(PTree);		     /* Not needed any more. */
    }

    /* Free the allocated variables. */
    IritDBPop(TRUE);

    return PRetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process the expression tree with A..Z variables.  The expressions are    *
* assumed to be consisting of numeric values, variables, and math. ops.      *
*                                                                            *
* PARAMETERS:                                                                *
*   PTree:     To process into its mathematical formulation.		     *
*   NumVars:   Number of variables.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   Constructed multivariate that represents the shape.  *
*****************************************************************************/
static IPObjectStruct *IPProcessExplicitParseTree(ParseTree *PTree,
						  int NumVars)
{
    int *Degrees = (int *) IritMalloc(sizeof(int) * NumVars);

    /* Do first pass - find the maximal degree in each of the variables. */
    IRIT_ZAP_MEM(Degrees, sizeof(int) * NumVars);
    if (IPProcessExpPTreeIAux(Degrees, PTree, NumVars)) {
	int i;
	MvarMVStruct *MV;

	/* Convert to orders and create the MV. */
	for (i = 0; i < NumVars; i++)
	    Degrees[i]++;
	MV = MvarMVNew(NumVars, MVAR_POWER_TYPE, MVAR_PT_E1_TYPE, Degrees);
	IRIT_GEN_COPY(MV -> Orders, MV -> Lengths, sizeof(int) * MV -> Dim);
	IRIT_ZAP_MEM(MV -> Points[1],
		     sizeof(CagdRType) * MVAR_CTL_MESH_LENGTH(MV));
	IritFree(Degrees);

	/* Do second pass - insert the different expressions in. */
	IPProcessExpPTreeIIAux(PTree, NumVars, 1, MV);

	return IPGenMULTIVARObject(MV);
    }

    IritFree(Degrees);

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process the expression tree with A..Z variables.  The expressions are    *
* assumed to be consisting of numeric values, variables, and math. ops.      *
* Specifically, no parenthesis are processed.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Degrees:   To update the maximal degrees in each variables.		     *
*   PTree:     To process into its mathematical formulation.		     *
*   NumVars:   Number of variables.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.			     *
*****************************************************************************/
static int IPProcessExpPTreeIAux(int *Degrees, ParseTree *PTree, int NumVars)
{
    int i, Var, *LocDegrees;
    char *Name;

    switch (PTree -> NodeKind) {
	case IP_TKN_PLUS:
	case IP_TKN_MINUS:
	    /* Recurse on both parts */
	    return IPProcessExpPTreeIAux(Degrees, PTree -> Left, NumVars) &&
		   IPProcessExpPTreeIAux(Degrees, PTree -> Right, NumVars);
	case IP_TKN_NUMBER:
	    break;
	case IP_TKN_PARAMETER:
	    Name = IP_GET_OBJ_NAME(PTree -> PObj);
	    Var = (int) (Name[0] - 'A');
	    if (Var >= 0 && Var < NumVars) {
		Degrees[Var] = IRIT_MAX(1, Degrees[Var]);
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Explicit: Unsupported parameter detected");
	        return FALSE;
	    }
	    break;
	case IP_TKN_MULT:
	case IP_TKN_DIV:
	case IP_TKN_POWER:
	    /* A single term - get its degrees */
	    LocDegrees = (int *) IritMalloc(sizeof(int) * NumVars);
	    IRIT_ZAP_MEM(LocDegrees, sizeof(int) * NumVars);
	    if (!IPProcessExpPTreeIAux2(LocDegrees, PTree, NumVars))
	        return FALSE;
	    for (i = 0; i < NumVars; i++)
	        Degrees[i] = IRIT_MAX(Degrees[i], LocDegrees[i]);
	    IritFree(LocDegrees);
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Explicit: Unsupported expression type detected");
	    return FALSE;

    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process a single term with A..Z variables.  The expressions are assumed  *
* to be consisting of numeric values, variables, and '*', '/' and unary '-'. *
*                                                                            *
* PARAMETERS:                                                                *
*   Degrees:   To update the maximal degrees in each variables.		     *
*   PTree:     To process into its mathematical formulation.		     *
*   NumVars:   Number of variables.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.			     *
*****************************************************************************/
static int IPProcessExpPTreeIAux2(int *Degrees, ParseTree *PTree, int NumVars)
{
    int Var, Pwr;
    char *Name;

    switch (PTree -> NodeKind) {
	case IP_TKN_NUMBER:
	    break;
	case IP_TKN_PARAMETER:
	    Name = IP_GET_OBJ_NAME(PTree -> PObj);
	    Var = (int) (Name[0] - 'A');
	    if (Var >= 0 && Var < NumVars) {
		Degrees[Var]++;
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Explicit: Unsupported parameter detected");
	        return FALSE;
	    }
	    break;
	case IP_TKN_UNARMINUS:
	    return IPProcessExpPTreeIAux2(Degrees, PTree -> Right, NumVars);
	case IP_TKN_POWER:
	    if (PTree -> Left -> NodeKind == IP_TKN_PARAMETER &&
		PTree -> Right -> NodeKind == IP_TKN_NUMBER) {
	        Name = IP_GET_OBJ_NAME(PTree -> Left -> PObj);
		Var = (int) (Name[0] - 'A');
		Pwr = (int) (PTree -> Right -> PObj -> U.R);
		if (Var >= 0 && Var < NumVars &&
		    Pwr >= 0 && Pwr < IP_EXPLICIT_MAX_DEGREE) {
		    Degrees[Var] += Pwr;
		}
		else {
		    IRIT_NON_FATAL_ERROR("Explicit: Invalid power expression");
		    return FALSE;
		}
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Explicit: Invalid power expression");
	        return FALSE;
	    }
	    break;
	case IP_TKN_MULT:
	    /* Recurse on both parts */
	    return IPProcessExpPTreeIAux2(Degrees, PTree -> Left, NumVars) &&
		   IPProcessExpPTreeIAux2(Degrees, PTree -> Right, NumVars);
	case IP_TKN_DIV:
	    /* Recurse on left parts */
	    if (!IPProcessExpPTreeIAux2(Degrees, PTree -> Left, NumVars))
		return FALSE;
	    if (PTree -> Right -> NodeKind != IP_TKN_NUMBER) {
		IRIT_NON_FATAL_ERROR("Explicit: Division by constants only");
		return FALSE;
	    }
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Explicit: Unsupported expression type detected");
	    return FALSE;

    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process the expression tree with A..Z variables.  The expressions are    *
* assumed to be consisting of numeric values, variables, and math. ops.      *
* Specifically, no parenthesis are processed.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PTree:     To process into its mathematical formulation.		     *
*   NumVars:   Number of variables.					     *
*   Sign:      +1 for a positive expression, -1 for a negative expression.   *
*   MV:        The multivariate to update with the coefficients.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.			     *
*****************************************************************************/
static int IPProcessExpPTreeIIAux(ParseTree *PTree,
				  int NumVars,
				  int Sign,
				  MvarMVStruct *MV)
{
    int Indx, *Degrees;
    IrtRType Coef;

    switch (PTree -> NodeKind) {
	case IP_TKN_PLUS:
	case IP_TKN_MINUS:
	    /* Recurse on both parts */
	    IPProcessExpPTreeIIAux(PTree -> Left, NumVars, Sign, MV);
	    IPProcessExpPTreeIIAux(PTree -> Right, NumVars, 
				   PTree -> NodeKind == IP_TKN_PLUS ? 1 : -1,
				   MV);
	    break;
	case IP_TKN_NUMBER:
	case IP_TKN_PARAMETER:
	case IP_TKN_MULT:
	case IP_TKN_DIV:
	case IP_TKN_POWER:
	    /* A single term - get its degrees and numeric constant. */
	    Degrees = (int *) IritMalloc(sizeof(int) * NumVars);
	    IRIT_ZAP_MEM(Degrees, sizeof(int) * NumVars);
	    Coef = Sign;	    /* The numeric coefficient of this term. */

	    IPProcessExpPTreeIIAux2(Degrees, &Coef, PTree, NumVars);

	    /* Update the multivariate. */
	    Indx = MvarGetPointsMeshIndices(MV, Degrees);
	    MV -> Points[1][Indx] += Coef;
	    IritFree(Degrees);
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Explicit: Unsupported expression type detected");
	    return FALSE;

    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process a single term with A..Z variables.  The expressions are assumed  *
* to be consisting of numeric values, variables, and '*', '/' and unary '-'. *
*                                                                            *
* PARAMETERS:                                                                *
*   Degrees:   To degrees of the diffierent variables in ths term.           *
*   Coef:      The numeric coefficient of this term.			     *
*   PTree:     To process into its mathematical formulation.		     *
*   NumVars:   Number of variables.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.			     *
*****************************************************************************/
static int IPProcessExpPTreeIIAux2(int *Degrees,
				   IrtRType *Coef,
				   ParseTree *PTree,
				   int NumVars)
{
    int Var, Pwr;
    char *Name;

    switch (PTree -> NodeKind) {
	case IP_TKN_NUMBER:
	    *Coef *= PTree -> PObj -> U.R;
	    break;
	case IP_TKN_PARAMETER:
	    Name = IP_GET_OBJ_NAME(PTree -> PObj);
	    Var = (int) (Name[0] - 'A');
	    if (Var >= 0 && Var < NumVars) {
		Degrees[Var]++;
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Explicit: Unsupported parameter detected");
	        return FALSE;
	    }
	    break;
	case IP_TKN_UNARMINUS:
	    *Coef = -*Coef;
	    return IPProcessExpPTreeIIAux2(Degrees, Coef,
					   PTree -> Right, NumVars);
	case IP_TKN_POWER:
	    if (PTree -> Left -> NodeKind == IP_TKN_PARAMETER &&
		PTree -> Right -> NodeKind == IP_TKN_NUMBER) {
	        Name = IP_GET_OBJ_NAME(PTree -> Left -> PObj);
		Var = (int) (Name[0] - 'A');
		Pwr = (int) (PTree -> Right -> PObj -> U.R);
		if (Var >= 0 && Var < NumVars &&
		    Pwr >= 0 && Pwr < IP_EXPLICIT_MAX_DEGREE) {
		    Degrees[Var] += Pwr;
		}
		else {
		    IRIT_NON_FATAL_ERROR("Explicit: Invalid power expression");
		    return FALSE;
		}
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Explicit: Invalid power expression");
	        return FALSE;
	    }
	    break;
	case IP_TKN_MULT:
	    /* Recurse on both parts */
	    return IPProcessExpPTreeIIAux2(Degrees, Coef,
					   PTree -> Left, NumVars) &&
		   IPProcessExpPTreeIIAux2(Degrees, Coef,
					   PTree -> Right, NumVars);
	case IP_TKN_DIV:
	    /* Recurse on left parts */
	    if (!IPProcessExpPTreeIIAux2(Degrees, Coef, PTree -> Left, NumVars))
		return FALSE;
	    if (PTree -> Right -> NodeKind == IP_TKN_NUMBER) {
		*Coef /= PTree -> Right -> PObj -> U.R;
	    }
	    else {
		IRIT_NON_FATAL_ERROR("Explicit: Division by constants only");
		return FALSE;
	    }
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("Explicit: Unsupported expression type detected");
	    return FALSE;

    }

    return TRUE;
}
