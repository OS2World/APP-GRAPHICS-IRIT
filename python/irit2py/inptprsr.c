/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to convert infix expression given as	ascii stream sequence into   *
* a binary tree, and evaluate it.					     *
*   All the objects are handled the same but the numerical one, which is     *
* moved as a IrtRType and not as an object (only internally within this	     *
* module) as it is frequently used and consumes much less memory this way.   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "program.h"
#include "allocate.h"
#include "ctrl-brk.h"
#include "inptprsg.h"
#include "inptprsl.h"
#include "objects.h"
#include "overload.h"

IRIT_STATIC_DATA InptPrsrEvalErrType
    IPGlblParseError = IPE_NO_ERR;
IRIT_STATIC_DATA UserDefinedFuncDefType
    *IPGlblUserFunc = NULL;

/* Operator preceeding parser stack, and stack pointer: */
IRIT_STATIC_DATA ParseTree
    **Stack = NULL;
IRIT_STATIC_DATA int
    ParserStackSize = 0,
    ParserStackPointer = 0;

IRIT_GLOBAL_DATA char IPGlblCharData[INPUT_LINE_LEN];   /* Used for parse & eval. */
IRIT_GLOBAL_DATA int InptPrsrLastToken;

#ifdef DEBUG1
    IRIT_GLOBAL_DATA int MaxStackPointer = 0;  /* Measure maximum depth of stack. */
#endif /* DEBUG1 */

static ParseTree *OperatorPrecedence(void);
static int TestPreceeding(int Token1, int Token2);
static char *UpdateCharErrorAux(int Token, ParseTree *Node);
static int GetToken(IrtRType *Data);
static int GetVarFuncToken(char *Token, IrtRType *Data);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module routine - generate parse tree and then tries to evaluate it.   M
*   Returns TRUE if successful, otherwise check IPGlblParseError/EvalError.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PrintFunc:	 If not NULL invoked with parse tree, otherwise evaluated.   M
*		 If invoked, can have a second and third string and string-  M
*		 length to print the expression to.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         TRUE if successful, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrInputParser                                                      M
*****************************************************************************/
int InptPrsrInputParser(InptPrsrPrintTreeFuncType PrintFunc)
{
    ParseTree *PTree;

    if (GlblFatalError) {
	GlblFatalError = FALSE;
	InptPrsrFlushToEndOfExpr(FALSE);  /* Close all include files if any. */
	return TRUE;
    }

    if (Stack == NULL) {				      /* First time. */
	ParserStackSize = IP_INIT_PARSER_STACK;
	Stack = IritMalloc(ParserStackSize * sizeof(ParseTree *));
    }

    PTree = InptPrsrGenInputParseTree();	     /* Generate parse tree. */

    if (IPGlblParseError == IPE_NO_ERR) {
	if (PrintFunc == NULL) {
	    if (InptPrsrTypeCheck(PTree, 0) == ERROR_EXPR) {/* Type checking.*/
	        InptPrsrFreeTree(PTree);	     /* Not needed any more. */
		/* Close all include files and flush stdin.*/
		InptPrsrFlushToEndOfExpr(TRUE);
		return FALSE;
	    }

	    if (strnicmp(InptPrsrQueryCurrentFile(), "iritinit", 8) != 0)
	        InptPrsrPrintTree(PTree, NULL, 0);

	    InptPrsrEvalTree(PTree, 0);			     /* Evaluate it. */
	    if (IPGlblEvalError != IPE_NO_ERR) {
		/* Close all include files and flush stdin.*/
	        InptPrsrFlushToEndOfExpr(TRUE);
		return FALSE;
	    }
	}
	else {
	    PrintFunc(PTree, NULL, 0);
	}
    }
    else {
        /* Close all include files and flush stdin. */
	InptPrsrFlushToEndOfExpr(TRUE);
	return FALSE;
    }

    InptPrsrFreeTree(PTree);			     /* Not needed any more. */

    return !(IPGlblParseError || IPGlblEvalError);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert the expression from stream f into a binary tree.        M
*   Algorithm: Using operator precedence with the following grammer:         M
* EXPR    ::= EXPR    |  EXPR + EXPR    |  EXPR - EXPR                       V
* EXPR    ::= EXPR    |  EXPR * EXPR    |  EXPR / EXPR                       V
* EXPR    ::= EXPR    |  EXPR ^ EXPR                                         V
* EXPR    ::= EXPR    |  EXPR , EXPR    |  EXPR = EXPR                       V
* EXPR    ::= NUMBER  |  -EXPR          |  (EXPR)        |  FUNCTION         V
* FUCTION ::= FUNC(EXPR , EXPR , ...)					     V
*   Where FUNC might be function like arithmetics (SIN, COS etc.).	     M
*   Note that FUNC might have more than one operand, separated by ','.	     M
*                                                                            M
*   Note the stream is terminated by semicolon character ';'.		     M
*                                                                            M
*   Left associativity for +, -, *, /, ^.                                    M
*   Precedence of operators is as usual:                                     M
*     <Highest> {unar minus}   {^}   {*, /}   {+, -} <Lowest>		     M
*                                                                            M
*   Returns NULL if an error was found, and error is in IPGlblParseError     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   ParseTree *:   Constructed parsed tree.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrGenInputParseTree                                                M
*****************************************************************************/
ParseTree *InptPrsrGenInputParseTree(void)
{
    ParseTree *Root;
    int i;

    InptPrsrLastToken = 0;	/* Used to hold last token read from stream. */
    IPGlblParseError = IPE_NO_ERR;		     /* No errors so far ... */

    Root = OperatorPrecedence();

    if (IPGlblParseError) {
	/* Free partialy allocated tree. */
	for (i = 0; i <= ParserStackPointer; i++)
	    InptPrsrFreeTree(Stack[i]);
    	return NULL;						  /* Error ! */
    }
    else
	return Root;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to allocate new ParseTree expression node.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   ParseTree *:   Allocate a ParseTree node.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   ExprMalloc                                                               M
*****************************************************************************/
ParseTree *ExprMalloc(void)
{
    ParseTree *p;

    p = (ParseTree *) IritMalloc(sizeof(ParseTree));
    p -> Right = p -> Left = NULL;
    p -> NodeKind = IP_OBJ_UNDEF;
    p -> PObj = NULL;
    p -> UserFunc = NULL;
    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to free one expression node.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Ptr:       ParseTree to free.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ExprFree                                                                 M
*****************************************************************************/
void ExprFree(ParseTree *Ptr)
{
    IritFree(Ptr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to actually parse using operator precedence.                       *
* Few Notes:                                                                 *
* 1. Parse the input with the help of GetToken routine. Input is redirected  *
*    using the FileStack.						     *
* 2. All tokens must be in the range of 0 ... IP_PRSR_MAX_TOKEN-1 as we use  *
*    the numbers above it (adding IP_PRSR_MAX_TOKEN) to deactivate them in   *
*    the handle searching (i.e. when they were reduced to sub.-expression).  *
* 3. Returns NULL pointer in case of an error.				     *
* 4. See "Compilers - principles, techniques and tools" by Aho, Sethi &      *
*    Ullman,   pages 207-210.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   ParseTree *:   Returned parsed tree.                                     *
*****************************************************************************/
static ParseTree *OperatorPrecedence(void)
{
    int Token, LowHandle, Temp1, Temp2;
    IrtRType Data;

#   ifdef DEBUG1
	MaxStackPointer = 0;
#   endif /* DEBUG1 */

    ParserStackPointer = 0;

    /* Push the start symbol on stack (node pointer points on tos): */
    Stack[ParserStackPointer] = ExprMalloc();
    Stack[ParserStackPointer] -> NodeKind = IP_TKN_START;
    Stack[ParserStackPointer] -> Right =
	Stack[ParserStackPointer] -> Left = NULL;

    Token = GetToken(&Data);      /* Get one look ahead token to start with. */

    while (TRUE) {
        if (IPGlblParseError)
	    return NULL;

	/* Find top active token ( < IP_PRSR_MAX_TOKEN). */
        Temp1 = ParserStackPointer;
        while (Stack[Temp1] -> NodeKind >= IP_PRSR_MAX_TOKEN)
	    Temp1--;
        /* Now test to see if the new token completes an handle: */
        if (TestPreceeding(Stack[Temp1] -> NodeKind, Token)) {
            switch (Token) {
		case IP_TKN_CLOSPARA:
                    if (Stack[Temp1] -> NodeKind == IP_TKN_OPENPARA) {
			ExprFree(Stack[Temp1]);		 /* Free open paran. */
			Stack[Temp1] = NULL;
			/* If a parameter is introduced instead of function  */
			/* it will be reduced already against "(" and it     */
			/* probably was missspelled function...		     */
                        if (Stack[Temp1 - 1] -> NodeKind ==
					IP_TKN_PARAMETER + IP_PRSR_MAX_TOKEN) {
			    strcpy(IPGlblCharData,
				   IP_GET_OBJ_NAME(Stack[Temp1 - 1] -> PObj));
			    IPGlblParseError = IR_ERR_UNDEF_FUNC;
			    return NULL;
			}

			if (IP_IS_USER_FUNCTION(Stack[Temp1 - 1] ->
						                   NodeKind)) {
			    if (ParserStackPointer - Temp1 == 1) {
				Stack[ParserStackPointer] -> NodeKind -=
							     IP_PRSR_MAX_TOKEN;
				Stack[Temp1 - 1] -> NodeKind +=
							     IP_PRSR_MAX_TOKEN;
				Stack[Temp1 - 1] -> Right =
				    Stack[ParserStackPointer];
				ParserStackPointer -= 2;
			    }
			    else {
				Stack[Temp1 - 1] -> NodeKind +=
							     IP_PRSR_MAX_TOKEN;
				Stack[Temp1 - 1] -> Right = NULL;
				ParserStackPointer--;
			    }
			}
                        else if (IP_IS_NO_PARAM_FUNC(Stack[Temp1 - 1] ->
						                   NodeKind)) {
			    if (ParserStackPointer - Temp1 == 1) {
			        UpdateCharError("",
						Stack[Temp1 - 1] -> NodeKind,
						Stack[Temp1 - 1]);
				IPGlblParseError = IR_ERR_NO_PARAM_FUNC;
				return NULL;
			    }
			    Stack[Temp1 - 1] -> NodeKind += IP_PRSR_MAX_TOKEN;
			    Stack[Temp1 - 1] -> Right = NULL;
			    ParserStackPointer--;
                        }
                        else if (IP_IS_FUNCTION(Stack[Temp1 - 1] ->
						                   NodeKind)) {
			    if (ParserStackPointer - Temp1 != 1) {
			        UpdateCharError("",
						Stack[Temp1 - 1] -> NodeKind,
						Stack[Temp1 - 1]);
			        IPGlblParseError = IR_ERR_PARAM_FUNC;
			        return NULL;
			    }
			    Stack[ParserStackPointer] -> NodeKind -=
							     IP_PRSR_MAX_TOKEN;
			    Stack[Temp1 - 1] -> NodeKind += IP_PRSR_MAX_TOKEN;
			    Stack[Temp1 - 1] -> Right =
					Stack[ParserStackPointer];
			    ParserStackPointer -= 2;
	                }
                        else {
			    if (ParserStackPointer - Temp1 != 1) {
			        IPGlblParseError = IR_ERR_PARAM_MATCH;
			        return NULL;
			    }
                            Stack[Temp1] = Stack[ParserStackPointer--];
			}
                        Token = GetToken(&Data);       /* Get another token. */
                        continue;
		    }
		    else if (Stack[Temp1] -> NodeKind == IP_TKN_START) {
			/* No match for this one! */
                        IPGlblParseError = IR_ERR_PARAM_MATCH;
			return NULL;
		    }
		    break;
                case IP_TKN_END:
                    if (Stack[Temp1] -> NodeKind == IP_TKN_START) {
                        if (ParserStackPointer != 1) {
                            IPGlblParseError = IR_ERR_WRONG_SYNTAX;
			    return NULL;
			}
			InptPrsrFreeTree(Stack[Temp1]); /* The IP_TKN_START. */
			Stack[1] -> NodeKind -= IP_PRSR_MAX_TOKEN;
			return Stack[1];
		    }
		}

            Temp2 = Temp1 - 1;		  /* Find the lower bound of handle. */
            while (Temp2 >= 0 && Stack[Temp2] -> NodeKind >= IP_PRSR_MAX_TOKEN)
		Temp2--;
            LowHandle = Temp2 + 1;
            if (LowHandle < 1) {                  /* No low bound was found. */
                IPGlblParseError = IR_ERR_WRONG_SYNTAX;
	        return NULL;			 /* We ignore data till now. */
            }
	    switch (ParserStackPointer - LowHandle + 1) {
		case 1:
	          /* Its a scalar one - mark as used (add IP_PRSR_MAX_TOKEN).*/
		    switch (Stack[ParserStackPointer] -> NodeKind) {
			case IP_TKN_NUMBER:
			case IP_TKN_PARAMETER:
			case IP_TKN_STRING:
		            Stack[ParserStackPointer] -> NodeKind +=
							     IP_PRSR_MAX_TOKEN;
			    break;
			default:
			    UpdateCharError("Found ",
					 Stack[ParserStackPointer] -> NodeKind,
					 Stack[ParserStackPointer]);
			    IPGlblParseError = IR_ERR_PARAM_EXPECT;
			    return NULL;
		    }
		    break;
		case 2: /* Its a monadic operator - create the subtree. */
		    switch (Stack[ParserStackPointer - 1] -> NodeKind) {
		        case IP_TKN_BOOL_NOT:
		        case IP_TKN_UNARMINUS:
		            Stack[ParserStackPointer - 1] -> Right =
						Stack[ParserStackPointer];
		            Stack[ParserStackPointer] -> NodeKind -=
							     IP_PRSR_MAX_TOKEN;
		            Stack[ParserStackPointer - 1] -> NodeKind +=
							     IP_PRSR_MAX_TOKEN;
		            ParserStackPointer--;
		            break;
		        case IP_TKN_OPENPARA:
			    IPGlblParseError = IR_ERR_PARAM_MATCH;
			    return NULL;
		        default:
			    if (IP_IS_AN_OPERATOR(Stack[ParserStackPointer] ->
					                            NodeKind))
				UpdateCharError("Found ",
					Stack[ParserStackPointer] -> NodeKind,
				        Stack[ParserStackPointer]);
			    else
				UpdateCharError("Found ",
				    Stack[ParserStackPointer - 1] -> NodeKind,
				    Stack[ParserStackPointer - 1]);

			    IPGlblParseError = IR_ERR_ONE_OPERAND;
			    return NULL;
		    }
		    break;
		case 3: /* Its a diadic operator - create the subtree. */
		    switch (Stack[ParserStackPointer - 1] -> NodeKind) {
		        case IP_TKN_PLUS:
		        case IP_TKN_MINUS:
		        case IP_TKN_MULT:
		        case IP_TKN_DIV:
		        case IP_TKN_POWER:
		        case IP_TKN_COMMA:
		        case IP_TKN_EQUAL:
		        case IP_TKN_CMP_EQUAL:
		        case IP_TKN_CMP_NOTEQUAL:
		        case IP_TKN_CMP_LSEQUAL:
		        case IP_TKN_CMP_GTEQUAL:
		        case IP_TKN_CMP_LESS:
		        case IP_TKN_CMP_GREAT:
		        case IP_TKN_BOOL_AND:
		        case IP_TKN_BOOL_OR:
		        case IP_TKN_COLON:
		            Stack[ParserStackPointer - 1] -> Right =
                                  Stack[ParserStackPointer];
                            Stack[ParserStackPointer - 1] -> Left =
                                  Stack[ParserStackPointer - 2];
		            Stack[ParserStackPointer - 2] -> NodeKind -=
							     IP_PRSR_MAX_TOKEN;
		            Stack[ParserStackPointer] -> NodeKind -=
							     IP_PRSR_MAX_TOKEN;
		            Stack[ParserStackPointer - 1] -> NodeKind +=
							     IP_PRSR_MAX_TOKEN;
		            Stack[ParserStackPointer - 2] =
						Stack[ParserStackPointer - 1];
		            ParserStackPointer -= 2;
                            break;
                        default:
			    UpdateCharError("Found Operator ",
				    Stack[ParserStackPointer - 1] -> NodeKind,
				    Stack[ParserStackPointer - 1]);
			    IPGlblParseError = IR_ERR_TWO_OPERAND;
			    return NULL;
		    }
		    break;
		default:
		    IPGlblParseError = IR_ERR_WRONG_SYNTAX;
		    return NULL;
	    }
        }
        else {		 /* Push that token on stack - it is not handle yet. */
	    Stack[++ParserStackPointer] = ExprMalloc();

#	    ifdef DEBUG1
		if (MaxStackPointer < ParserStackPointer)
		    MaxStackPointer = ParserStackPointer;
#	    endif /* DEBUG1 */

            if (ParserStackPointer >= ParserStackSize - 10) {
	        int Size = ParserStackSize * sizeof(ParseTree *);

		/* Reallocate the stack. */
		ParserStackSize <<= 1;
		Stack = IritRealloc(Stack, Size, Size << 1);
	    }
            if ((Stack[ParserStackPointer] -> NodeKind = Token)
							     == IP_USERINSTDEF)
		Stack[ParserStackPointer] -> UserFunc = IPGlblUserFunc;
            Stack[ParserStackPointer] -> PObj = NULL;
	    Stack[ParserStackPointer] -> Right =
	    Stack[ParserStackPointer] -> Left = NULL;
	    if (Token == IP_TKN_NUMBER) {
		Stack[ParserStackPointer] -> PObj = IPGenNUMValObject(Data);
		Stack[ParserStackPointer] -> PObj -> Count++;
	    }
	    else if (Token == IP_TKN_PARAMETER) {
		if ((Stack[ParserStackPointer] -> PObj =
				 IritDBGetObjByName(IPGlblCharData)) == NULL) {
		    /*   Its new one - allocate memory for it. Create a      */
		    /* numeric object as a reasonable default.		     */
		    Stack[ParserStackPointer] -> PObj =
			IPAllocObject(IPGlblCharData, IP_OBJ_UNDEF, NULL);
		    IritDBInsertObject(Stack[ParserStackPointer] -> PObj,
				       FALSE);
		}
		Stack[ParserStackPointer] -> PObj -> Count++;
	    }
	    else if (Token == IP_TKN_STRING) {
		Stack[ParserStackPointer] -> PObj =
		    IPGenSTRObject(IPGlblCharData);
		Stack[ParserStackPointer] -> PObj -> Count++;
	    }
            Token = GetToken(&Data);	   /* And get new token from stream. */
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test precedence of two tokens. returns 0, <0 or >0 according to *
* comparison results.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Token1, Token2:   Tokens to compare.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:         <0, 0, >0 comparison's result.                              *
*****************************************************************************/
static int TestPreceeding(int Token1, int Token2)
{
    int Preced1 = 0,
	Preced2 = 0;

    if ((Token1 >= IP_PRSR_MAX_TOKEN) || (Token2 >= IP_PRSR_MAX_TOKEN))
	return FALSE;					 /* Ignore sub-expr. */

    if (IP_IS_FUNCTION(Token1))
	Preced1 = 160;
    else {
    	switch (Token1) {
	    case IP_TKN_START:
	    case IP_TKN_END:
	        Preced1 = 10;
    		break;
	    case IP_TKN_OPENPARA:
    		Preced1 = 20;
    		break;
    	    case IP_TKN_COMMA:
	        Preced1 = 30;
    		break;
    	    case IP_TKN_COLON:
	        Preced1 = 40;
    		break;
	    case IP_TKN_BOOL_AND:
	    case IP_TKN_BOOL_OR:
		Preced1 = 50;
		break;
    	    case IP_TKN_EQUAL:
	    case IP_TKN_CMP_EQUAL:
	    case IP_TKN_CMP_NOTEQUAL:
	    case IP_TKN_CMP_LSEQUAL:
	    case IP_TKN_CMP_GTEQUAL:
	    case IP_TKN_CMP_LESS:
	    case IP_TKN_CMP_GREAT:
	        Preced1 = 55;
    		break;
	    case IP_TKN_PLUS:
	    case IP_TKN_MINUS:
    		Preced1 = 80;
    		break;
	    case IP_TKN_MULT:
	    case IP_TKN_DIV:
    		Preced1 = 100;
    		break;
	    case IP_TKN_POWER:
    		Preced1 = 120;
    		break;
	    case IP_TKN_UNARMINUS:
	    case IP_TKN_BOOL_NOT:
    		Preced1 = 125;
    		break;
    	    case IP_TKN_NUMBER:
    	    case IP_TKN_PARAMETER:
    	    case IP_TKN_STRING:
	    case IP_TKN_CLOSPARA:
    		Preced1 = 180;
    		break;

    	}
    }

    if (IP_IS_FUNCTION(Token2))
	Preced2 = 150;
    else {
    	switch (Token2) {
	    case IP_TKN_START:
	    case IP_TKN_END:
    		Preced2 = 0;
    		break;
	    case IP_TKN_CLOSPARA:
    		Preced2 = 15;
    		break;
	    case IP_TKN_COMMA:
    		Preced2 = 30;
    		break;
	    case IP_TKN_COLON:
    		Preced2 = 40;
    		break;
	    case IP_TKN_BOOL_AND:
	    case IP_TKN_BOOL_OR:
		Preced2 = 50;
		break;
	    case IP_TKN_EQUAL:
	    case IP_TKN_CMP_EQUAL:
	    case IP_TKN_CMP_NOTEQUAL:
	    case IP_TKN_CMP_LSEQUAL:
	    case IP_TKN_CMP_GTEQUAL:
	    case IP_TKN_CMP_LESS:
	    case IP_TKN_CMP_GREAT:
    		Preced2 = 55;
    		break;
	    case IP_TKN_PLUS:
	    case IP_TKN_MINUS:
    		Preced2 = 70;
    		break;
	    case IP_TKN_MULT:
	    case IP_TKN_DIV:
    		Preced2 = 90;
    		break;
	    case IP_TKN_POWER:
    		Preced2 = 110;
    		break;
	    case IP_TKN_UNARMINUS:
	    case IP_TKN_BOOL_NOT:
    		Preced2 = 130;
    		break;
	    case IP_TKN_NUMBER:
	    case IP_TKN_PARAMETER:
	    case IP_TKN_STRING:
	    case IP_TKN_OPENPARA:
    		Preced2 = 170;
    		break;

    	}
    }

    return Preced1 - Preced2 > 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to provide a description, gievn Token and optionally a Node.       *
*                                                                            *
* PARAMETERS:                                                                *
*   Token:      With the fault.                                              *
*   Node:       Optional node at fault.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:     Description of error.                                        *
*****************************************************************************/
static char *UpdateCharErrorAux(int Token, ParseTree *Node)
{
    IRIT_STATIC_DATA char TmpStr[IRIT_LINE_LEN];
    char *TokenChar = NULL;

    if (Token > IP_PRSR_MAX_TOKEN)
	Token -= IP_PRSR_MAX_TOKEN;

    if (IP_IS_NUM_FUNCTION(Token))
	TokenChar = NumFuncTable[Token - IP_NUM_FUNC_OFFSET].FuncName;
    else if (IP_IS_OBJ_FUNCTION(Token))
	TokenChar = ObjFuncTable[Token - IP_OBJ_FUNC_OFFSET].FuncName;
    else if (IP_IS_GEN_FUNCTION(Token))
	TokenChar = GenFuncTable[Token - IP_GEN_FUNC_OFFSET].FuncName;
    else {
    	switch (Token) {
    	    case IP_TKN_PLUS:
    		TokenChar = "+";
    		break;
    	    case IP_TKN_MINUS:
    		TokenChar = "-";
    		break;
	    case IP_TKN_MULT:
    		TokenChar = "*";
    		break;
	    case IP_TKN_DIV:
    		TokenChar = "/";
    		break;
	    case IP_TKN_POWER:
    		TokenChar = "^";
    		break;
	    case IP_TKN_UNARMINUS:
    		TokenChar = "(Unary) -";
    		break;
	    case IP_TKN_EQUAL:
    		TokenChar = "=";
    		break;
	    case IP_TKN_COMMA:
    		TokenChar = ",";
    		break;
	    case IP_TKN_COLON:
    		TokenChar = ":";
    		break;
	    case IP_TKN_SEMICOLON:
    		TokenChar = ";";
    		break;
	    case IP_TKN_CMP_EQUAL:
    		TokenChar = "==";
    		break;
	    case IP_TKN_CMP_NOTEQUAL:
    		TokenChar = "!=";
    		break;
	    case IP_TKN_CMP_LSEQUAL:
    		TokenChar = "<=";
    		break;
	    case IP_TKN_CMP_GTEQUAL:
    		TokenChar = ">=";
    		break;
	    case IP_TKN_CMP_LESS:
    		TokenChar = "<";
    		break;
	    case IP_TKN_CMP_GREAT:
    		TokenChar = ">";
    		break;
	    case IP_TKN_BOOL_AND:
    		TokenChar = "&&";
		break;
	    case IP_TKN_BOOL_OR:
    		TokenChar = "||";
		break;
	    case IP_TKN_BOOL_NOT:
    		TokenChar = "!";
		break;
	    case IP_TKN_OPENPARA:
		TokenChar = "(";
		break;
	    case IP_TKN_CLOSPARA:
		TokenChar = ")";
		break;
	    case IP_TKN_NUMBER:
		if (Node && Node -> PObj && IP_IS_NUM_OBJ(Node -> PObj)) {
		    sprintf(TmpStr, "%g", Node -> PObj -> U.R);
		    TokenChar = TmpStr;
		}
		else
		    TokenChar = "Numeric";
		break;
	    case IP_TKN_PARAMETER:
		if (Node && Node -> PObj && IP_VALID_OBJ_NAME(Node -> PObj))
		    TokenChar = IP_GET_OBJ_NAME(Node -> PObj);
		else
		    TokenChar = "Parameter";
		break;
	    case IP_TKN_STRING:
		if (Node && Node -> PObj && IP_IS_STR_OBJ(Node -> PObj)) {
		    TokenChar = Node -> PObj -> U.Str;
		}
		else
		    TokenChar = "String";
		break;
	    case IP_USERINSTDEF:
	    case IP_USERFUNCDEF:
	    case IP_USERPROCDEF:
		TokenChar = "UserFunc";
		break;
	    default:
    		sprintf(TmpStr, "Token %d\n", Token);
		TokenChar = TmpStr;
		break;
    	}
    }

    return TokenChar;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to update the character error message according to StrMsg & Token  M
*   Node is optional and if not NULL, will be used to get better details.    M
*                                                                            *
* PARAMETERS:                                                                M
*   StrMsg:   Some description of error.                                     M
*   Token:    Token at fault.                                                M
*   Node:     Optional Node at fault.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UpdateCharError                                                          M
*****************************************************************************/
void UpdateCharError(char *StrMsg, int Token, ParseTree *Node)
{
#ifdef IRIT_QUIET_STRINGS
    IPGlblCharData[0] = 0;
#else
    char InputLine[INPUT_LINE_LEN],
	*TokenChar = UpdateCharErrorAux(Token, Node);

    InptPrsrPrintTree(Node, InputLine, INPUT_LINE_LEN);
    sprintf(IPGlblCharData, "%s\"%s\",\n\tExpression: %s",
	    StrMsg, TokenChar, InputLine);
    if (IP_IS_AN_OPERATOR(Token) && Node) {
	if (Node -> Left) {
	    char
		*StrType = InptPrsrTypeToStr(InptPrsrTypeCheck(Node -> Left,
							       1));

	    sprintf(&IPGlblCharData[strlen(IPGlblCharData)],
		    "\n\tLeft Operand: \"%s\" (%s type)",
		    UpdateCharErrorAux(Node -> Left -> NodeKind,
				       Node -> Left),
		    StrType);
	}
	if (Node -> Right) {
	    char
		*StrType = InptPrsrTypeToStr(InptPrsrTypeCheck(Node -> Right,
							       1));

	    sprintf(&IPGlblCharData[strlen(IPGlblCharData)],
		    "\n\tRight Operand: \"%s\" (%s type)",
		    UpdateCharErrorAux(Node -> Right -> NodeKind,
				       Node -> Right),
		    StrType);
	}
    }
#endif /* !IRIT_QUIET_STRINGS */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to get the next token out of the expression.                       *
*   Returns next token found and sets Data to the returned value (if any).   *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:      Real numbers will be saved herein.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       Numeric value of next token.                                  *
*****************************************************************************/
static int GetToken(IrtRType *Data)
{
    int i,
	RetVal = IP_TKN_NONE;
    char c;

    while (isspace(c = InptPrsrGetC(FALSE, FALSE)));   /* Skip white blanks. */

    if (c == '"') {		  /* Its a string token - read up to next ". */
	i = 0;
	while ((IPGlblCharData[i] = InptPrsrGetC(TRUE, FALSE)) != '"') {
	    if (IPGlblCharData[i] == '\\') /* Its escape char. for next one: */
		IPGlblCharData[i] = InptPrsrGetC(TRUE, FALSE);
	    if (IPGlblCharData[i] < ' ' || i > INPUT_LINE_LEN - 2) {
		RetVal = IP_TKN_ERROR;
		IPGlblCharData[i] = 0;
		IPGlblParseError = IR_ERR_STR_TOO_LONG;
		break;
	    }
	    i++;
	}
	if (RetVal != IP_TKN_ERROR) {
	    IPGlblCharData[i] = 0;
	    RetVal = IP_TKN_STRING;
	}
    }
    else if (isalpha(c)) {		  /* Is it a variable/function name? */
	if (islower(c))
	    IPGlblCharData[i = 0] = toupper(c);
	else
	    IPGlblCharData[i = 0] = c;

	while (isalpha(c = InptPrsrGetC(FALSE, FALSE)) || isdigit(c) || c == '_')
	    if (islower(c))
		IPGlblCharData[++i] = toupper(c);
	    else
		IPGlblCharData[++i] = c;
	IPGlblCharData[++i] = 0;
	InptPrsrUnGetC(c);

	if ((int) strlen(IPGlblCharData) >= IRIT_LINE_LEN_VLONG) {
	    RetVal = IP_TKN_ERROR;
	    IPGlblParseError = IR_ERR_NAME_TOO_LONG;
	}
	else {
	    RetVal = GetVarFuncToken(IPGlblCharData, Data);
	}
    }
    else if (isdigit(c) || (c == '.')) {	      /* Is it numeric data? */
	IPGlblCharData[i=0] = c;

	while (isdigit(c = InptPrsrGetC(FALSE, FALSE)) || (c == '.') ||
					(c == 'e') || (c == 'E') || (c == 'e'))
	    IPGlblCharData[++i] = c;
	/* Handle the special case of negative exponent ("111.111E-22"). */
	if (c == '-' && (IPGlblCharData[i] == 'e' ||
			 IPGlblCharData[i] == 'E')) {
	    IPGlblCharData[++i] = c;
	    while (isdigit(c = InptPrsrGetC(FALSE, FALSE)) || (c == '.'))
		IPGlblCharData[++i] = c;
	}
	IPGlblCharData[++i] = 0;

	InptPrsrUnGetC(c);

#	ifdef IRIT_DOUBLE
	    sscanf(IPGlblCharData, "%lf", Data);
#	else
	    sscanf(IPGlblCharData, "%f", Data);
#	endif /* IRIT_DOUBLE */

        RetVal = IP_TKN_NUMBER;
    }
    else
	switch (c) {
	    case '+':
		RetVal = IP_TKN_PLUS;
		break;
	    case '-':
		switch (InptPrsrLastToken) {
		    case 0:	      /* If first token (no last token yet). */
		    case IP_TKN_PLUS:
		    case IP_TKN_MINUS:
		    case IP_TKN_MULT:
		    case IP_TKN_DIV:
		    case IP_TKN_POWER:
		    case IP_TKN_COMMA:
		    case IP_TKN_EQUAL:
		    case IP_TKN_CMP_EQUAL:
		    case IP_TKN_CMP_NOTEQUAL:
		    case IP_TKN_CMP_LSEQUAL:
		    case IP_TKN_CMP_GTEQUAL:
		    case IP_TKN_CMP_LESS:
		    case IP_TKN_CMP_GREAT:
		    case IP_TKN_BOOL_AND:
		    case IP_TKN_BOOL_OR:
		    case IP_TKN_BOOL_NOT:
		    case IP_TKN_COLON:
		    case IP_TKN_UNARMINUS:
		    case IP_TKN_OPENPARA:
			RetVal = IP_TKN_UNARMINUS;
			break;
		    default:
                        RetVal = IP_TKN_MINUS;
	    		break;
		}
		break;
	    case '*':
		RetVal = IP_TKN_MULT;
		break;
	    case '/':
		RetVal = IP_TKN_DIV;
		break;
	    case '^':
		RetVal = IP_TKN_POWER;
		break;
	    case '(':
		RetVal = IP_TKN_OPENPARA;
		break;
	    case ')':
		RetVal = IP_TKN_CLOSPARA;
		break;
	    case '=':
		switch (c = InptPrsrGetC(FALSE, FALSE)) {
		    case '=':
		        RetVal = IP_TKN_CMP_EQUAL;
			break;
		    default:
			InptPrsrUnGetC(c);
			RetVal = IP_TKN_EQUAL;
			break;
		}
		break;
	    case '<':
		switch (c = InptPrsrGetC(FALSE, FALSE)) {
		    case '=':
		        RetVal = IP_TKN_CMP_LSEQUAL;
			break;
		    default:
			InptPrsrUnGetC(c);
			RetVal = IP_TKN_CMP_LESS;
			break;
		}
		break;
	    case '>':
		switch (c = InptPrsrGetC(FALSE, FALSE)) {
		    case '=':
		        RetVal = IP_TKN_CMP_GTEQUAL;
			break;
		    default:
			InptPrsrUnGetC(c);
			RetVal = IP_TKN_CMP_GREAT;
			break;
		}
		break;
	    case '&':
		if ((c = InptPrsrGetC(FALSE, FALSE)) == '&') {
		    RetVal = IP_TKN_BOOL_AND;
		    break;
		}
		else {
		    RetVal = IP_TKN_ERROR;
		    IPGlblCharData[0] = '&';
		    IPGlblCharData[1] = c;
		    IPGlblCharData[2] = 0;
		    IPGlblParseError = IR_ERR_UNDEF_TOKEN;
		    break;
		}
	    case '|':
		if ((c = InptPrsrGetC(FALSE, FALSE)) == '|') {
		    RetVal = IP_TKN_BOOL_OR;
		    break;
		}
		else {
		    RetVal = IP_TKN_ERROR;
		    IPGlblCharData[0] = '|';
		    IPGlblCharData[1] = c;
		    IPGlblCharData[2] = 0;
		    IPGlblParseError = IR_ERR_UNDEF_TOKEN;
		    break;
		}
	    case ',':
		RetVal = IP_TKN_COMMA;
		break;
	    case ':':
		RetVal = IP_TKN_COLON;
		break;
	    case ';':
		RetVal = IP_TKN_END;
		break;	       /* End of expression! */
	    case '!':
		if ((c = InptPrsrGetC(FALSE, FALSE)) == '=') {
		    RetVal = IP_TKN_CMP_NOTEQUAL;
		}
		else {
		    InptPrsrUnGetC(c);
		    RetVal = IP_TKN_BOOL_NOT;
		}
		break;
	    default:
		RetVal = IP_TKN_ERROR;
		IPGlblCharData[0] = c;
		IPGlblCharData[1] = 0;
		IPGlblParseError = IR_ERR_UNDEF_TOKEN;
		break;
    }

    InptPrsrLastToken = RetVal;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test alpha Token for match with one of the defined functions    *
* and returns that Token function if found one.				     *
*   Otherwise it is assumed to be a user defined function or a variable.     *
*                                                                            *
* PARAMETERS:                                                                *
*   Token:     Token to search, in string function.                          *
*   Data:      Real numbers will be saved herein.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       Token number                                                  *
*****************************************************************************/
static int GetVarFuncToken(char *Token, IrtRType *Data)
{
    int i, IritInitFile;
    char c, c2;
    UserDefinedFuncDefType *UserFunc;

    if (strcmp("COMMENT", Token) == 0) {
	/* Get first nonspace char after the COMMENT key word: */
	while (isspace(c = InptPrsrGetC(FALSE, FALSE)));
	/* And read the input until this char appear again (end of comment): */
	IritInitFile = strnicmp(InptPrsrQueryCurrentFile(),
				"iritinit", 8) == 0;
	while (c != (c2 = InptPrsrGetC(FALSE, !IritInitFile))) {
	    if (!IritInitFile)
               printf("%c", c2);
	}
	if (!IritInitFile)
	    printf("\n");

	return GetToken(Data);		       /* Return next token instead. */
    }

    for (UserFunc = UserDefinedFuncList;
	 UserFunc != NULL;
	 UserFunc = UserFunc -> Pnext)
	if (strcmp(UserFunc -> FuncName, Token) == 0) {
	    while (isspace(c = InptPrsrGetC(FALSE, FALSE)));  /* Skip white blanks. */
	    InptPrsrUnGetC(c);
	    if (c == '(') {
		IPGlblUserFunc = UserFunc;
		return IP_USERINSTDEF;
	    }
	    else
		break;
	}
    for (i = 0; i < NumFuncTableSize; i++)        /* Is it Numeric function? */
	if (strcmp(NumFuncTable[i].FuncName, Token) == 0)
	    return NumFuncTable[i].FuncToken;
    for (i = 0; i < ObjFuncTableSize; i++)	   /* Is it Object function? */
	if (strcmp(ObjFuncTable[i].FuncName, Token) == 0)
	    return ObjFuncTable[i].FuncToken;
    for (i = 0; i < GenFuncTableSize; i++)	  /* Is it General function? */
	if (strcmp(GenFuncTable[i].FuncName, Token) == 0)
	    return GenFuncTable[i].FuncToken;

    if (strcmp("FUNCTION", Token) == 0)
	return IP_USERFUNCDEF;
    if (strcmp("PROCEDURE", Token) == 0)
	return IP_USERPROCDEF;
	
    for (i = 0; i < ConstantTableSize; i++)/* Replace constant by its value. */
	if (strcmp(ConstantTable[i].FuncName, Token) == 0) {
	    sprintf(Token, "%g", ConstantTable[i].Value);
	    *Data = ConstantTable[i].Value;
	    return IP_TKN_NUMBER;
	}

    return IP_TKN_PARAMETER;/* If not a func - it is assumed to be variable. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return parsing error if happen one, zero return value otherwise.M
*                                                                            *
* PARAMETERS:                                                                M
*   Message:        To place a reference to the error message here.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   InptPrsrEvalErrType:    The error number.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   InptPrsrParseError                                                       M
*****************************************************************************/
InptPrsrEvalErrType InptPrsrParseError(char **Message)
{
    InptPrsrEvalErrType Temp;

    *Message = IPGlblCharData;
    Temp = IPGlblParseError;
    IPGlblParseError = IPE_NO_ERR;
    return Temp;
}
