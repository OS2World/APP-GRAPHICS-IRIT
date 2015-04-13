/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* General, Visible to others, definitions for the Input Parser module.	     *
*****************************************************************************/

#ifndef	INPT_PRSR_GH
#define	INPT_PRSR_GH

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "iritprsr.h"

/*****************************************************************************
* Error	numbers	as located during the parsing process:			     *
*****************************************************************************/
typedef enum {
    IPE_NO_ERR = 0,

    IR_ERR_WRONG_SYNTAX,
    IR_ERR_PARAM_EXPECT,
    IR_ERR_ONE_OPERAND,
    IR_ERR_TWO_OPERAND,
    IR_ERR_STACK_OV,
    IR_ERR_PARAM_MATCH,
    IR_ERR_UNDEF_TOKEN,
    IR_ERR_UNDEF_FUNC,
    IR_ERR_NAME_TOO_LONG,
    IR_ERR_PARAM_FUNC,
    IR_ERR_NO_PARAM_FUNC,
    IR_ERR_STR_TOO_LONG,

/*****************************************************************************
* Error	as located during the evaluation process:			     *
*****************************************************************************/

    IE_ERR_FATAL_ERROR,
    IE_ERR_DIV_BY_ZERO,
    IE_ERR_NO_OBJ_METHOD,
    IE_ERR_TYPE_MISMATCH,
    IE_ERR_ASSIGN_LEFT_OP,
    IE_ERR_MIXED_OBJ,
    IE_ERR_IP_OBJ_UNDEFINED,
    IE_ERR_NO_ASSIGNMENT,
    IE_ERR_FP_ERROR,
    IE_ERR_NUM_PRM_MISMATCH,
    IE_ERR_MAT_POWER,
    IE_ERR_FREE_SIMPLE,
    IE_ERR_MODIF_ITER_VAR,
    IE_ERR_BOOLEAN_ERR,
    IE_ERR_OUT_OF_RANGE,
    IE_ERR_DATA_PRSR_ERROR,
    IE_ERR_USER_FUNC_NO_RETVAL,
    IE_ERR_INCOMPARABLE_TYPES,
    IE_ERR_ONLYEQUALITY_TEST,
    IE_ERR_IF_HAS_NO_COND,
    IE_ERR_IP_USERFUNC_DUP_VAR,
    IE_ERR_IP_USERFUNC_TOO_MANY_PRMS,
    IE_ERR_UNDEF_INSTANCE
} InptPrsrEvalErrType;

/*****************************************************************************
* The expression parse tree node definition:				     *
*****************************************************************************/
typedef	struct ParseTree {
    struct ParseTree *Right, *Left;
    int NodeKind;
    IPObjectStruct *PObj;
    struct UserDefinedFuncDefType *UserFunc;
} ParseTree;

typedef void (*InptPrsrPrintTreeFuncType)(ParseTree *Root,
					  char *Str,
					  int StrLen);

/*****************************************************************************
* The global (visible to others) function prototypes:			     *
*****************************************************************************/
char *InptPrsrQueryCurrentFile(void);

int InptPrsrInputParser(InptPrsrPrintTreeFuncType PrintFunc);
void InptPrsrQueueInputLine(const char *Line);

/* If the above returns NULL object the following might be called to find    */
/* What went wrong (In the parsing stage, or in the evaluation stage.	     */
InptPrsrEvalErrType InptPrsrParseError(char **Message);
InptPrsrEvalErrType InptPrsrEvalError(char **Message);

int InptPrsrHasQueuedInput(void);
void InptPrsrUnGetC(char c);
char InptPrsrGetC(int InString, int);
void InptPrsrFileInclude(const char *FileName);/*Push files to include stack.*/
void InptPrsrFlushToEndOfExpr(int FlushStdin);

void InptEvalFreeFunc(const char *FuncName);
void InptEvalDeleteAllFuncs(void);
int InptPrsrDebugFuncLevel(int DebugFuncLevel);
int InptPrsrSetEchoSource(int EchoSource);

IrtRType InptPrsrSetCmpObjEps(IrtRType NewEps);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	/* INPT_PRSR_GH */
