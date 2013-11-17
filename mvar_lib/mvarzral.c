/******************************************************************************
* MvarZrAl.c - Tools to synthesize MV constraints with some ease.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jul. 09.					      *
******************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_lib.h"
#include "mvar_loc.h"

#define MVAR_ZR_ALG_INIT_NUM_PARAMS	10
#define MV_ZR_RET_STR(SetAndInc)  MvarZrRetStr(MVZrAlg, Depth, SetAndInc, &Idx)
  

typedef enum {
    MVAR_ZR_ALG_PARAM_EXPR,
    MVAR_ZR_ALG_PARAM_VAR
} MvarZrAlgParamType;

typedef enum {
    MVAR_ZR_ALG_TYPE_NONE,
    MVAR_ZR_ALG_TYPE_NUMBER,
    MVAR_ZR_ALG_TYPE_NUMVAR,
    MVAR_ZR_ALG_TYPE_MVVAR,
    MVAR_ZR_ALG_TYPE_MV
} MvarZrAlgType;

typedef int (*MvarZrAlgEpilogueFuncType)(void *MVZrAlg);

typedef struct MvarZrAlgParamStruct {
    MvarZrAlgParamType Type;
    char *Name;
    char *Expr;
    CagdRType DmnMin, DmnMax;
    CagdRType Val;
    MvarMVStruct *MV;
} MvarZrAlgParamStruct;

typedef struct MvarZrAlgSetupStruct {
    int MaxNumOfTempParam;           /* Maximal number of temporaries used. */
    int NumOfCodeGens;  /* Number of invocations of code generation (eqns). */
    int AllocParams, NumParams;
    FILE *f;                  /* If dumps to a file, keep here a reference. */
    MvarZrAlgEpilogueFuncType MvarZrAlgEpilogueFunc;
    struct MvarZrAlgParamStruct *Params;
} MvarZrAlgSetupStruct;

typedef struct MvarZrAlgMVConstStruct {
    MvarZrAlgType Type;
    MvarMVStruct *MV;
    IrtRType Const;
    char Name[IRIT_LINE_LEN];
} MvarZrAlgMVConstStruct;

IRIT_STATIC_DATA int
    MvarZrAlgGenTempIdx = 1;

static void MvarZrAlgVerifyParamSpace(MvarZrAlgSetupStruct *ZrAlg);
static char *MvarZrAlgSubstString(const char *S,
				  const char *Src,
				  const char *Dst);
static char *MvarZrAlgSubstAssignments(void *MVZrAlg, const char *Expr);
static MvarZrAlgParamStruct *MvarZrAkgFetchParam(void *MVZrAlg,
						 const char *Name);
static const char *MvarZrRetStr(void *MVZrAlg,
				int Depth,
				CagdBType Increment,
				int *Index);
static const char *MvarZrAlgGenMVCodeAux(void *MVZrAlg,
					 int Depth,
					 IritE2TExprNodeStruct *ET,
					 FILE *f,
					 MvarZrAlgMVConstStruct *MVConst);
static int MvarZrAlgGenMVCodeEpilogue(void *MVZrAlg);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates and allocate the structure to manipulate algebraic expressions.  M
*                                                                            *
* PARAMETERS:                                                                M
*   None			                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *:  The allocated algebraic expressions structure.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZrAlgCreate                                                          M
*****************************************************************************/
void *MvarZrAlgCreate()
{
    MvarZrAlgSetupStruct
	*MVZrAlg = (MvarZrAlgSetupStruct *)
				    IritMalloc(sizeof(MvarZrAlgSetupStruct));

    MVZrAlg -> MaxNumOfTempParam = 0;
    MVZrAlg -> NumOfCodeGens = 0;
    MVZrAlg -> NumParams = 0;
    MVZrAlg -> AllocParams = MVAR_ZR_ALG_INIT_NUM_PARAMS;

    MVZrAlg -> f = NULL;
    MVZrAlg -> MvarZrAlgEpilogueFunc = NULL;

    MVZrAlg -> Params = (MvarZrAlgParamStruct *)
				IritMalloc(MVAR_ZR_ALG_INIT_NUM_PARAMS *
					   sizeof(MvarZrAlgParamStruct));

    return MVZrAlg;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates the structure to manipulate algebraic expressions.           M
*                                                                            *
* PARAMETERS:                                                                M
*   MVZrAlg:   Structure to deallocate.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZrAlgDelete                                                          M
*****************************************************************************/
void MvarZrAlgDelete(void *MVZrAlg)
{
    int i;
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    MvarZrAlgParamStruct
        *Params = ZrAlg -> Params;

    if (ZrAlg -> MvarZrAlgEpilogueFunc != NULL)
        ZrAlg -> MvarZrAlgEpilogueFunc(MVZrAlg);

    for (i = 0; i < ZrAlg -> NumParams; i++) {
        if (Params -> Name != NULL)
	    IritFree(Params -> Name);
        if (Params -> Expr != NULL)
	    IritFree(Params -> Expr);
        if (Params -> MV != NULL)
	    MvarMVFree(Params -> MV);
    }
    IritFree(ZrAlg -> Params);

    IritFree(ZrAlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verify we have a space to insert another assignment.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVZrAlg:   structure to verify existence of space for a new assignment.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   None                                                                     *
*****************************************************************************/
static void MvarZrAlgVerifyParamSpace(MvarZrAlgSetupStruct *ZrAlg)
{
    if (ZrAlg -> NumParams >= ZrAlg -> AllocParams) {
	ZrAlg -> Params = (MvarZrAlgParamStruct *)
	    IritRealloc(ZrAlg -> Params,
			ZrAlg -> AllocParams * sizeof(MvarZrAlgParamStruct),
			ZrAlg -> AllocParams * 2 *
					        sizeof(MvarZrAlgParamStruct));
        ZrAlg -> AllocParams *= 2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert an expression assignment into the structure.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   MVZrAlg:   Structure to add a new assignmen to.                          M
*   Name:      Of new assignment.                                            M
*   Expr:      The expression.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZrAlgAssignExpr                                                      M
*****************************************************************************/
int MvarZrAlgAssignExpr(void *MVZrAlg, const char *Name, const char *Expr)
{
    int i;
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    MvarZrAlgParamStruct *Param,
        *Params = ZrAlg -> Params;

    if (IritStrIStr(Expr, Name) != NULL) {
        /* No recursive definition allowed. */
        return FALSE;
    }

    for (i = 0; i < ZrAlg -> NumParams; i++) {
        if (stricmp(Name, Params[i].Name) == 0)
	    return FALSE;
    }

    MvarZrAlgVerifyParamSpace(ZrAlg);

    Param = &ZrAlg -> Params[ZrAlg -> NumParams++];
    Param -> Name = IritStrdup(Name);
    Param -> Expr = IritStrdup(Expr);
    Param -> DmnMin = 0.0;
    Param -> DmnMax = 0.0;
    Param -> Val = 0.0;
    Param -> MV = NULL;
    Param -> Type = MVAR_ZR_ALG_PARAM_EXPR;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert a numeric variable into the structure.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVZrAlg:   Structure to add a new assignmen to.                          M
*   Name:      Of new assignment.                                            M
*   Val:       The numeric value.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZrAlgAssignNumVar                                                    M
*****************************************************************************/
int MvarZrAlgAssignNumVar(void *MVZrAlg, const char *Name, CagdRType Val)
{
    int i;
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    MvarZrAlgParamStruct *Param,
        *Params = ZrAlg -> Params;

    for (i = 0; i < ZrAlg -> NumParams; i++) {
        if (stricmp(Name, Params[i].Name) == 0)
	    return FALSE;
    }

    MvarZrAlgVerifyParamSpace(ZrAlg);

    Param = &ZrAlg -> Params[ZrAlg -> NumParams++];
    Param -> Name = IritStrdup(Name);
    Param -> Expr = NULL;
    Param -> DmnMin = 0.0;
    Param -> DmnMax = 0.0;
    Param -> Val = Val;
    Param -> MV = NULL;
    Param -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Insert an MV variable assignment into the structure.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVZrAlg:   Structure to add a new assignment to.                         M
*   Name:      Name of this variable.                                        M
*   DmnMin, DmnMax:   Domain of this variable.                               M
*   MV:      ZrAlg  Multivariate representing this variable.  Optional.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZrAlgAssignMVVar                                                     M
*****************************************************************************/
int MvarZrAlgAssignMVVar(void *MVZrAlg,
			 const char *Name,
			 CagdRType DmnMin,
			 CagdRType DmnMax,
			 const MvarMVStruct *MV)
{
    int i;
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    MvarZrAlgParamStruct *Param,
        *Params = ZrAlg -> Params;

    for (i = 0; i < ZrAlg -> NumParams; i++) {
        if (stricmp(Name, Params[i].Name) == 0)
	    return FALSE;
    }

    MvarZrAlgVerifyParamSpace(ZrAlg);

    Param = &ZrAlg -> Params[ZrAlg -> NumParams++];
    Param -> Name = IritStrdup(Name);
    Param -> Expr = NULL;
    Param -> DmnMin = DmnMin;
    Param -> DmnMax = DmnMax;
    Param -> Val = 0.0;
    Param -> MV = MvarMVCopy(MV);
    Param -> Type = MVAR_ZR_ALG_TYPE_MVVAR;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds a parameter by name or NULL if error.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   MVZrAlg:   Structure to fetch a parameter from.                          *
*   char *:    Parameter to seek by name.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarZrAlgParamStruct *:  Found parameter or NULL if none.                *
*****************************************************************************/
static MvarZrAlgParamStruct *MvarZrAkgFetchParam(void *MVZrAlg,
						 const char *Name)
{
    int i;
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    MvarZrAlgParamStruct
        *Params = ZrAlg -> Params;

    for (i = 0; i < ZrAlg -> NumParams; i++) {
        if (stricmp(Name, Params[i].Name) == 0)
	    return &Params[i];
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A find (Src) and replace (into Dst) function, all instances.             *
*                                                                            *
* PARAMETERS:                                                                *
*   s:     Input string.						     *
*   src:   Pattern to look for in S and substitute with Dst.		     *
*   Dst:   Replacement patter for Src.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:  new string, allocated dynamically.                              *
*****************************************************************************/
static char *MvarZrAlgSubstString(const char *s,
				  const char *Src,
				  const char *Dst)
{
    char *p, *DstParen,
        *NewS = IritStrdup(s);

    /* Add parenthesis to substituted expression to be on the safe size. */
    DstParen = IritMalloc((unsigned int) (strlen(Dst) + 3));
    sprintf(DstParen, "(%s)", Dst);

    while (IritStrIStr(NewS, Src) != NULL) {
        p = IritSubstStr(NewS, Src, DstParen, TRUE);
	IritFree(NewS);
	NewS = p;
    }

    IritFree(DstParen);

    return NewS;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Substitute in all assignments into expression Expr.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   MVZrAlg:   Structure to add a new assignment to.                         *
*   Expr:      To substitute in assignments from MVZrAlg.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:                                                                  *
*****************************************************************************/
static char *MvarZrAlgSubstAssignments(void *MVZrAlg, const char *Expr)
{
    int i;
    char *p,
        *NewExpr = IritStrdup(Expr);
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    MvarZrAlgParamStruct
        *Params = ZrAlg -> Params;

    for (i = ZrAlg -> NumParams - 1; i >= 0; i--) {
        if (Params[i].Type == MVAR_ZR_ALG_PARAM_EXPR) {
	    p = MvarZrAlgSubstString(NewExpr, Params[i].Name, Params[i].Expr);
	    IritFree(NewExpr);
	    NewExpr = p;
	}
    }

    return NewExpr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generate multivariate C Code sequence into designated file that          M
* represent the given expression Expr:				             M
* 1. Using MVZrAlg, perform all possible substitutions.			     M
* 2. Parse the result into a binary tree and then synthesize the C code to   M
*    build a multivar with variables at the leaves fetched from MVZrAlg.     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVZrAlg:   Structure of variables and expressions.                       M
*   Expr:      To parse into a multivariate C Code.			     M
*   f:         Destination of synthesized C code.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if successful, FALSE otherwise.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZrAlgGenMVCode                                                       M
*****************************************************************************/
int MvarZrAlgGenMVCode(void *MVZrAlg, const char *Expr, FILE *f)
{
    char *NewExpr;
    const char *RetVal;
    MvarZrAlgSetupStruct
        *ZrAlg = MVZrAlg;
    IritE2TExprNodeStruct *ET;
    MvarZrAlgMVConstStruct MVConst;

    /* Sets up the function to invoke when done. */
    ZrAlg -> MvarZrAlgEpilogueFunc = MvarZrAlgGenMVCodeEpilogue;
    ZrAlg -> f = f;

    /* Stage 1 - substitute in all expressions. */
    NewExpr = MvarZrAlgSubstAssignments(MVZrAlg, Expr);

    /* Stage 2 - convert to an expression tree and parse. */
    ET = IritE2TExpr2Tree(NewExpr);

    fprintf(f, "\n\n    /* Expression: \"%s\". */\n    /* Expanded: \"%s\". */\n    {\n", Expr, NewExpr);

    IritFree(NewExpr);

    /* Traverse the built tree. */
    MvarZrAlgGenTempIdx = 1;
    RetVal = MvarZrAlgGenMVCodeAux(MVZrAlg, 0, ET, f, &MVConst);

    fprintf(f, "    }\n");

    ZrAlg -> MaxNumOfTempParam = IRIT_MAX(ZrAlg -> MaxNumOfTempParam,
					  MvarZrAlgGenTempIdx);
    ZrAlg -> NumOfCodeGens++;

    IritE2TFreeTree(ET);

    return RetVal != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a string representing temporary variable.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   MVZrAlg:   Structure of variables and expressions.                       *
*   Depth:     Of recursion.						     *
*   SetAndIncrement: TRUE to update Index and increment global temporary     *
8	       indices, FALSE to leave Index as is.			     *
*   Index:     Of temporary variable to use.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   const:                                                                   *
*****************************************************************************/
static const char *MvarZrRetStr(void *MVZrAlg,
				int Depth,
				CagdBType SetAndIncrement,
				int *Index)
{
    IRIT_STATIC_DATA char TempVar[IRIT_LINE_LEN_LONG];
    MvarZrAlgSetupStruct
        *ZrAlg = (MvarZrAlgSetupStruct *) MVZrAlg;

    if (Depth == 0)
        sprintf(TempVar, "MVs[%d]", ZrAlg -> NumOfCodeGens);
    else {
        if (SetAndIncrement)
	    *Index = MvarZrAlgGenTempIdx++;

        sprintf(TempVar, "MVT%d", *Index);
    }

    return TempVar;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traverses the created tree and synthesizes MV code that represent it.    *
*                                                                            *
* PARAMETERS:                                                                *
*   MVZrAlg:   Structure of variables and expressions.                       *
*   Depth:     Of recursion.						     *
*   ET:        Parsed expression tree.                                       *
*   f:         File where to dump the MV code.                               *
*   MVConst:   Result is to be returned here, either as a number or MV.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   const char *:   NULL if error or name of variable/parameter.             *
*****************************************************************************/
static const char *MvarZrAlgGenMVCodeAux(void *MVZrAlg,
					 int Depth,
					 IritE2TExprNodeStruct *ET,
					 FILE *f,
					 MvarZrAlgMVConstStruct *MVConst)
{
    const char *MVRStr, *MVLStr;
    int Idx = -1;
    MvarZrAlgParamStruct *Param;
    MvarZrAlgMVConstStruct MVConstL, MVConstR;
    IrtRType
        ConstRet = 0.0;

    if (!ET)
        return NULL;

    MVConst -> Type = MVAR_ZR_ALG_TYPE_NONE;
    MVConst -> Name[0] = 0;
    MVConst -> MV = NULL;

    switch (ET -> NodeKind) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	    assert(0);     /* We do not support non-algebraic expressions. */
	    return NULL;

	case IRIT_E2T_SQR:
	case IRIT_E2T_UNARMINUS:
	    if ((MVRStr = MvarZrAlgGenMVCodeAux(MVZrAlg, Depth + 1,
						ET -> Right, f,
						&MVConstR)) == NULL)
	        return NULL;

	    switch (ET -> NodeKind) {
	        case IRIT_E2T_SQR:
		    switch (MVConstR.Type) {
		        case MVAR_ZR_ALG_TYPE_MV:
		        case MVAR_ZR_ALG_TYPE_MVVAR:
			    fprintf(f, "\t%s = MvarMVMult(%s, %s);\n",
				    MV_ZR_RET_STR(TRUE), MVRStr, MVRStr);
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMVAR:
			    fprintf(f, "\t%s = IRIT_SQR(%s);\n",
				    MV_ZR_RET_STR(TRUE), MVRStr);
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMBER:
			    ConstRet = IRIT_SQR(MVConstR.Const);
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
			    break;
		        default:
			    assert(0);
		    }
		    break;
	        case IRIT_E2T_UNARMINUS:
		    switch (MVConstR.Type) {
		        case MVAR_ZR_ALG_TYPE_MV:
		        case MVAR_ZR_ALG_TYPE_MVVAR:
			    fprintf(f, "\t%s = MvarMVScalarScale(%s, -1.0);\n",
				    MV_ZR_RET_STR(TRUE), MVRStr);
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMVAR:
			    fprintf(f, "\t%s = -%s;\n",
				    MV_ZR_RET_STR(TRUE), MVRStr);
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMBER:
			    ConstRet = -MVConstR.Const;
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
			    break;
		        default:
			    assert(0);
		    }
		    break;
	        default:
		    assert(0);
		    return NULL;
	    }

	    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_MV)
	        fprintf(f, "\tMvarMVFree(%s);\n", MVRStr);

	    MVConst -> Const = ConstRet;
	    break;

	case IRIT_E2T_DIV:
	case IRIT_E2T_MINUS:
	case IRIT_E2T_MULT:
	case IRIT_E2T_PLUS:
	case IRIT_E2T_POWER:
	    if ((MVLStr = MvarZrAlgGenMVCodeAux(MVZrAlg, Depth + 1,
						ET -> Left, f,
						&MVConstL)) == 0 ||
		(MVRStr = MvarZrAlgGenMVCodeAux(MVZrAlg, Depth + 1,
						ET -> Right, f,
						&MVConstR)) == 0)
	        return NULL;

	    switch (ET -> NodeKind) {
	        case IRIT_E2T_DIV:
		    assert(0);
		    return NULL;
	        case IRIT_E2T_MINUS:
		    switch (MVConstL.Type) {
		        case MVAR_ZR_ALG_TYPE_MV:
		        case MVAR_ZR_ALG_TYPE_MVVAR:
			    switch (MVConstR.Type) {
			        case MVAR_ZR_ALG_TYPE_MV:
			        case MVAR_ZR_ALG_TYPE_MVVAR:
				    fprintf(f, "\t%s = MvarMVSub(%s, %s);\n",
					    MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        case MVAR_ZR_ALG_TYPE_NUMBER:
			        case MVAR_ZR_ALG_TYPE_NUMVAR:
				    fprintf(f, "\t%s = MvarMVCopy(%s);\n",
					    MV_ZR_RET_STR(TRUE), MVLStr);
				    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
				        fprintf(f, "\tR = -%s;\n", MVRStr);
				    else
				        fprintf(f, "\tR = %f;\n", -MVConstR.Const);
				    fprintf(f, "\tMvarMVTransform(%s, &R, 1.0);\n",
					    MV_ZR_RET_STR(FALSE));
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        default:
				    assert(0);
			    }
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMBER:
			case MVAR_ZR_ALG_TYPE_NUMVAR:
			    switch (MVConstR.Type) {
			        case MVAR_ZR_ALG_TYPE_MV:
			        case MVAR_ZR_ALG_TYPE_MVVAR:
				    fprintf(f, "\t%s = MvarMVScalarScale(%s, -1.0);\n",
					    MV_ZR_RET_STR(TRUE), MVRStr);
				    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
				        fprintf(f, "\tR = %s;\n", MVLStr);
				    else
				        fprintf(f, "\tR = %f;\n", MVConstL.Const);
				    fprintf(f, "\tMvarMVTransform(%s, &R, 1.0);\n",
					    MV_ZR_RET_STR(FALSE));
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        case MVAR_ZR_ALG_TYPE_NUMBER:
			        case MVAR_ZR_ALG_TYPE_NUMVAR:
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;
				    if (MVConstL.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
					if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
					    fprintf(f, "\t%s = %s - %s;\n",
						    MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
					else {
					    fprintf(f, "\t%s = %s - %f;\n",
						    MV_ZR_RET_STR(TRUE), MVLStr, MVConstR.Const);
					}
				    }
				    else {
				        if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
					    fprintf(f, "\t%s = %f - %s;\n",
						    MV_ZR_RET_STR(TRUE), MVConstL.Const, MVRStr);
					}
					else {
					    ConstRet = MVConstL.Const - MVConstR.Const;
					    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
					}
				    }
				    break;
			        default:
				    assert(0);
				    break;
			    }
			    break;
			default:
			    assert(0);
			    break;
		    }
		    break;
	        case IRIT_E2T_MULT:
		    switch (MVConstL.Type) {
		        case MVAR_ZR_ALG_TYPE_MV:
		        case MVAR_ZR_ALG_TYPE_MVVAR:
			    switch (MVConstR.Type) {
				case MVAR_ZR_ALG_TYPE_MV:
				case MVAR_ZR_ALG_TYPE_MVVAR:
				    fprintf(f, "\t%s = MvarMVMult(%s, %s);\n",
					    MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        case MVAR_ZR_ALG_TYPE_NUMBER:
			        case MVAR_ZR_ALG_TYPE_NUMVAR:
				    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
				        fprintf(f, "\t%s = MvarMVScalarScale(%s, %s);\n",
						MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
				    else
				        fprintf(f, "\t%s = MvarMVScalarScale(%s, %f);\n",
						MV_ZR_RET_STR(TRUE), MVLStr, MVConstR.Const);
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        default:
				    assert(0);
			    }
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMBER:
		        case MVAR_ZR_ALG_TYPE_NUMVAR:
			    switch (MVConstR.Type) {
				case MVAR_ZR_ALG_TYPE_MV:
				case MVAR_ZR_ALG_TYPE_MVVAR:
				    if (MVConstL.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
				        fprintf(f, "\t%s = MvarMVScalarScale(%s, %s);\n",
						MV_ZR_RET_STR(TRUE), MVRStr, MVLStr);
				    else
				        fprintf(f, "\t%s = MvarMVScalarScale(%s, %f);\n",
						MV_ZR_RET_STR(TRUE), MVRStr, MVConstL.Const);
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        case MVAR_ZR_ALG_TYPE_NUMBER:
			        case MVAR_ZR_ALG_TYPE_NUMVAR:
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;
				    if (MVConstL.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
					if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
					    fprintf(f, "\t%s = %s * %s;\n",
						    MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
					else {
					    fprintf(f, "\t%s = %s * %f;\n",
						    MV_ZR_RET_STR(TRUE), MVLStr, MVConstR.Const);
					}
				    }
				    else {
				        if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
					    fprintf(f, "\t%s = %f * %s;\n",
						    MV_ZR_RET_STR(TRUE), MVConstL.Const, MVRStr);
					}
					else {
					    ConstRet = MVConstL.Const * MVConstR.Const;
					    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
					}
				    }
				    break;
			        default:
				    assert(0);
			    }
			    break;
			default:
			    assert(0);
			    break;
		    }
		    break;
	        case IRIT_E2T_PLUS:
		    switch (MVConstL.Type) {
		        case MVAR_ZR_ALG_TYPE_MV:
		        case MVAR_ZR_ALG_TYPE_MVVAR:
			    switch (MVConstR.Type) {
				case MVAR_ZR_ALG_TYPE_MV:
				case MVAR_ZR_ALG_TYPE_MVVAR:
				    fprintf(f, "\t%s = MvarMVAdd(%s, %s);\n",
					    MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        case MVAR_ZR_ALG_TYPE_NUMBER:
			        case MVAR_ZR_ALG_TYPE_NUMVAR:
				    fprintf(f, "\t%s = MvarMVCopy(%s);\n",
					    MV_ZR_RET_STR(TRUE), MVLStr);
				    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
				        fprintf(f, "\tR = %s;\n", MVRStr);
				    else
					fprintf(f, "\tR = %f;\n", MVConstR.Const);
				    fprintf(f, "\tMvarMVTransform(%s, &R, 1.0);\n",
					    MV_ZR_RET_STR(FALSE));
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        default:
				    assert(0);
			    }
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMBER:
		        case MVAR_ZR_ALG_TYPE_NUMVAR:
			    switch (MVConstR.Type) {
			        case MVAR_ZR_ALG_TYPE_MV:
				case MVAR_ZR_ALG_TYPE_MVVAR:
				    fprintf(f, "\t%s = MvarMVCopy(%s);\n",
					    MV_ZR_RET_STR(TRUE), MVRStr);
				    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
				        fprintf(f, "\tR = %s;\n", MVLStr);
				    else
				        fprintf(f, "\tR = %f;\n", MVConstL.Const);
				    fprintf(f, "\tMvarMVTransform(%s, &R, 1.0);\n",
					    MV_ZR_RET_STR(FALSE));
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
				    break;
			        case MVAR_ZR_ALG_TYPE_NUMBER:
			        case MVAR_ZR_ALG_TYPE_NUMVAR:
				    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;
				    if (MVConstL.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
					if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR)
					    fprintf(f, "\t%s = %s + %s;\n",
						    MV_ZR_RET_STR(TRUE), MVLStr, MVRStr);
					else {
					    fprintf(f, "\t%s = %s + %f;\n",
						    MV_ZR_RET_STR(TRUE), MVLStr, MVConstR.Const);
					}
				    }
				    else {
				        if (MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
					    fprintf(f, "\t%s = %f + %s;\n",
						    MV_ZR_RET_STR(TRUE), MVConstL.Const, MVRStr);
					}
					else {
					    ConstRet = MVConstL.Const + MVConstR.Const;
					    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
					}
				    }
				    break;
			        default:
				    assert(0);
			    }
			    break;
			default:
			    assert(0);
			    break;
		    }
		    break;
	        case IRIT_E2T_POWER:
		    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_MV ||
			MVConstR.Type == MVAR_ZR_ALG_TYPE_MVVAR ||
			MVConstR.Type == MVAR_ZR_ALG_TYPE_NUMVAR) {
		        /* We only support integer/constant powers. */
		        return NULL;
		    }

		    switch (MVConstL.Type) {
		        case MVAR_ZR_ALG_TYPE_MV:
			case MVAR_ZR_ALG_TYPE_MVVAR:
			    fprintf(f, "\t%s = MvarMVCopy(%s);\n",
				    MV_ZR_RET_STR(TRUE), MVLStr);
			    fprintf(f, "\tfor (i = 1; i < %d; i++) {\n",
				    (int) MVConstR.Const);
			    fprintf(f, "\t    MvarMVStruct\n\t\t*MVTmp = MvarMVMult(%s, %s);\n",
				    MV_ZR_RET_STR(FALSE), MVLStr);

			    fprintf(f, "\t    MvarMVFree(%s);\n",
				    MV_ZR_RET_STR(FALSE));
			    fprintf(f, "\t    %s = MVTmp;\n",
				    MV_ZR_RET_STR(FALSE));
			    fprintf(f, "\t}\n");
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_MV;
			    break;
		        case MVAR_ZR_ALG_TYPE_NUMBER:
			    ConstRet = pow(MVConstL.Const, MVConstR.Const);
			    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
			    break;
		        default:
			    assert(0);
			    break;
		    }
		    break;
	        default:
		    assert(0);
		    return NULL;
	    }

	    if (MVConstR.Type == MVAR_ZR_ALG_TYPE_MV)
		fprintf(f, "\tMvarMVFree(%s);\n", MVRStr);
	    if (MVConstL.Type == MVAR_ZR_ALG_TYPE_MV)
		fprintf(f, "\tMvarMVFree(%s);\n", MVLStr);

	    MVConst -> Const = ConstRet;
	    break;

	case IRIT_E2T_NUMBER:
	    MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMBER;
	    MVConst -> Const = ET -> RData;
	    break;
	    
	case IRIT_E2T_PARAMETER:
	    Param = MvarZrAkgFetchParam(MVZrAlg, ET -> SData);
	    if (Param != NULL &&
		Param -> Type == MVAR_ZR_ALG_TYPE_NUMVAR)
	        MVConst -> Type = MVAR_ZR_ALG_TYPE_NUMVAR;
	    else
	        MVConst -> Type = MVAR_ZR_ALG_TYPE_MVVAR;
	    strncpy(MVConst -> Name, ET -> SData, IRIT_LINE_LEN - 1);
	    break;

        default:
	    assert(0);
	    return NULL;	                      /* Never gets here... */
    }

    if (MVConst -> Type == MVAR_ZR_ALG_TYPE_MV) {
        /* We should have set this one to a real index. */
	assert(Depth == 0 || Idx != -1);
	sprintf(MVConst -> Name, MV_ZR_RET_STR(FALSE));
    }

    return MVConst -> Name;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints the epilogue of the MVCode into the designated file               *
*                                                                            *
* PARAMETERS:                                                                *
*   MVZrAlg:   Structure of variables and expressions.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.                          *
*****************************************************************************/
static int MvarZrAlgGenMVCodeEpilogue(void *MVZrAlg)
{
    int i;
    MvarZrAlgSetupStruct
	*ZrAlg = (MvarZrAlgSetupStruct *) MVZrAlg;
    FILE
        *f = ZrAlg -> f;

    if (f == NULL)
        return FALSE;

    fprintf(f, "\n\n");
    for (i = 0; i < ZrAlg -> NumOfCodeGens; i++)
        fprintf(f, "    Constrs[%d] = MVAR_CNSTRNT_ZERO;\n", i);

    /* Solve the equations. */
    fprintf(f, "\n    MVPts = MvarMVsZeros(MVs, Constrs, %d, SubdivTol, NumericTol);\n\n",
	    ZrAlg -> NumOfCodeGens);

    for (i = 0; i < ZrAlg -> NumOfCodeGens; i++)
        fprintf(f, "    MvarMVFree(MVs[%d]);\n", i);

    fprintf(f, "}\n");

    /* Provide local variables' declaration at the end (move to start)... */
    fprintf(f, "\n    /* Local variables - move to begining of functions. */\n");
    fprintf(f, "    int i;\n");
    fprintf(f, "    CagdRType R;\n");
    fprintf(f, "    MvarPtStruct *MVPts;\n");
    fprintf(f, "    MvarConstraintType Constrs[%d];\n",
	    ZrAlg -> NumOfCodeGens);
    fprintf(f, "    MvarMVStruct *MVs[%d],\n\t", ZrAlg -> NumOfCodeGens);

    for (i = 1; i < ZrAlg -> MaxNumOfTempParam; i++) {
        fprintf(f, " *MVT%d%c", i,
		i == ZrAlg -> MaxNumOfTempParam - 1 ? ';' : ',');
	if (i % 8 == 0)
	    fprintf(f, "\n\t");
    }

    return TRUE;
}
