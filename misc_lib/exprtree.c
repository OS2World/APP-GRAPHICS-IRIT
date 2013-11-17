/*****************************************************************************
*   Module to convert infix expression given as a string into a binary tree. *
* Evaluate trees, and make symbolic derivation of such trees.                *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	June 1988    *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "irit_sm.h"
#include "misc_loc.h"

#define IRIT_ASCII_TAB       9

#define IRIT_E2T_MAX_PARSER_STACK   100

IRIT_STATIC_DATA int
    E2TParsingError,                         /* Globals used by the parser. */
    E2TGlblLastTkn,
    E2TGlblDerivError;                      /* Globals used by derivations. */
IRIT_STATIC_DATA IrtRType
    E2TGlobalParam[IRIT_E2T_PARAM_NUM_PARAM];  /* Parameters are save here. */

static IritE2TExprNodeStruct *E2TMallocNode();
static void E2TFreeNode(IritE2TExprNodeStruct *Ptr);
static IrtRType IritE2Expt2TreeDefaultFetchParamValue(const char *SData);
static void E2TMakeUpper(char s[]);
static IritE2TExprNodeStruct *E2TOperatorPrecedence(const char s[], int *i);
static int E2TTestPreceeding(int Token1, int Token2);
static int E2TGetToken(const char s[], int *i, IrtRType *RData, char **SData);
static void E2TLocalPrintTree(const IritE2TExprNodeStruct *Root,
			      int Level,
			      char *Str);
static IritE2TExprNodeStruct *E2TLclDerivTree(const IritE2TExprNodeStruct *Root,
					      int Prm);
static IritE2TExprNodeStruct *E2TGen1u2Tree(int Sign1,
					    int Sign2,
					    IrtRType Exponent,
					    IritE2TExprNodeStruct *Expr);
static IritE2TExprNodeStruct *E2TOptimize(IritE2TExprNodeStruct *Root, 
					  int *Flag);

IRIT_STATIC_DATA IritE2TExprNodeParamFuncType
    E2GlblFetchParamValueFunc = IritE2Expt2TreeDefaultFetchParamValue;

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to allocate an expression node.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   None		                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IritE2TExprNodeStruct *:  Allocated node.                                *
*****************************************************************************/
static IritE2TExprNodeStruct *E2TMallocNode()
{
    IritE2TExprNodeStruct
        *Ptr = (IritE2TExprNodeStruct *) IritMalloc(sizeof(IritE2TExprNodeStruct));

    Ptr -> SData = NULL;

    return Ptr;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to deallocate an expression node.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Ptr:  Node to free.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void E2TFreeNode(IritE2TExprNodeStruct *Ptr)
{
    if (Ptr -> SData != NULL)
        IritFree(Ptr -> SData);

    IritFree(Ptr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the function to use to fetch a parameter value, given its name,     M
* during evaluation.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   FetchParamValueFunc:  Function pointer to use to fetch a value of a      M
*                         parameter, given its name.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritE2TExprNodeParamFuncType:  Old function pointer.	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2Expt2TreeSetFetchParamValueFunc                                    M
*****************************************************************************/
IritE2TExprNodeParamFuncType IritE2Expt2TreeSetFetchParamValueFunc(
			    IritE2TExprNodeParamFuncType FetchParamValueFunc)
{
    IritE2TExprNodeParamFuncType
        OldFunc = E2GlblFetchParamValueFunc;

    E2GlblFetchParamValueFunc = FetchParamValueFunc;
    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Default function to fetch the value of a parameter, during evaluation.   *
*                                                                            *
* PARAMETERS:                                                                *
*   char: N.S.F.I.                                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:                                                                *
*****************************************************************************/
static IrtRType IritE2Expt2TreeDefaultFetchParamValue(const char *SData)
{
    assert(SData != NULL && SData[0] >= 'A' && SData[0] <= 'Z');

    return E2TGlobalParam[SData[0] - 'A'];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to convert lower case chars into upper one in the given string.  *
*                                                                            *
* PARAMETERS:                                                                *
*   s:  String to convert to upper case.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void E2TMakeUpper(char s[])
{
    int i = 0;

    while (s[i] != 0) {
        if (islower(s[i]))
	    s[i] = toupper(s[i]);
	i++;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert the expression in string S into a binary tree.        M
* Algorithm: Using operator precedence with the following grammer:           M
* EXPR    ::= EXPR    |  EXPR + EXPR    |  EXPR - EXPR                       M
* EXPR    ::= EXPR    |  EXPR * EXPR    |  EXPR / EXPR                       M
* EXPR    ::= EXPR    |  EXPR ^ EXPR                                         M
* EXPR    ::= NUMBER  |  -EXPR          |  (EXPR)        |  FUNCTION         M
* FUCTION ::= SIN(EXPR)    | COS(EXPR)    | TAN(EXPR)                        M
*             ARCSIN(EXPR) | ARCCOS(EXPR) | ARCTAN(EXPR)                     M
*             SQRT(EXPR)   | SQR(EXPR)    | ABS(EXPR)                        M
*             LN(EXPR)     | LOG(EXPR)    | EXP(EXPR)                        M
*                                                                            M
* And left associativity for +, -, *, /, ^.                                  M
* Precedence of operators is as usual:                                       M
*     <Highest> {unar minus}   {^}   {*,/}   {+,-} <Lowest>                  M
*                                                                            M
* Returns NULL if an error was found, and error is in E2TParsingError        M
*                                                                            *
* PARAMETERS:                                                                M
*   s:   String expression to parse.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritE2TExprNodeStruct *:    Built binary tree.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TExpr2Tree                                                         M
*****************************************************************************/
IritE2TExprNodeStruct *IritE2TExpr2Tree(const char s[])
{
    IritE2TExprNodeStruct *Root;
    int i;
    char
        *s2 = IritStrdup(s);

    E2TMakeUpper(s2);
    E2TGlblLastTkn = 0;       /* Used to hold last Token read from stream. */
    E2TParsingError = 0;                           /* No errors so far ... */
    i = 0;
    Root = E2TOperatorPrecedence(s2, &i);    

    if (E2TParsingError)
        return NULL;					        /* Error ! */
    else
        return Root;

    IritFree(s2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to parse a string expression, s, using operator precedence.      *
* Few Notes:                                                                 *
* 1. Parse the string s with the help of E2TGetToken routine. i holds        *
*    current position in string s.                                           *
* 2. All Tokens must be in the range of 0..999 as we use the numbers above   *
*    it (adding 1000) to deactivate them in the handle searching (i.e. when  *
*    they were reduced to sub.-expression).                                  *
* 3. Returns NULL pointer in case of an error (see gIritE2TExpr2Tree.h for   *
*    errors.								     *
* 4. See "Compilers - principles, techniques and tools" by Aho, Sethi &      *
*    Ullman, pages 207-210.                                                  *
*                                                                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   s:   String to parse.                                                    *
*   i:   Location in s.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IritE2TExprNodeStruct *:    Built binary tree.                           *
*****************************************************************************/
static IritE2TExprNodeStruct *E2TOperatorPrecedence(const char s[], int *i)
{
    char *SData = NULL;
    int Token, LowHandle, Temp1, Temp2,
        StackPointer = 0;
    IrtRType RData;
    IritE2TExprNodeStruct *Stack[IRIT_E2T_MAX_PARSER_STACK];

    /* Push the start symbol on stack (Node pointer points on tos): */
    Stack[StackPointer] = E2TMallocNode();
    Stack[StackPointer] -> NodeKind = IRIT_E2T_TOKENSTART;

    /* Get a look ahead Token to start with. */
    Stack[StackPointer] -> Right = Stack[StackPointer] -> Left = NULL;

    Token = E2TGetToken(s, i, &RData, &SData);
    do {
	if (E2TParsingError)
	    return NULL;

        Temp1 = StackPointer;   /* Find top active Token (less than 1000). */
        while (Stack[Temp1] -> NodeKind >= 1000)
	    Temp1--;

        /* Now test to see if the new Token completes an handle: */
        if (E2TTestPreceeding(Stack[Temp1] -> NodeKind, Token) > 0) {
            switch (Token) {
		case IRIT_E2T_CLOSPARA:
                    if (Stack[Temp1] -> NodeKind == IRIT_E2T_OPENPARA) {
                        if (StackPointer-Temp1 != 1) {
                            E2TParsingError = IRIT_E2T_PARAMATCH_ERROR;
			    return NULL;
			}
                        switch (Stack[Temp1 - 1] -> NodeKind) {
			    /* If it is of the form Func(Expr) then reduce  */
			    /* it directly to that function, else (default) */
			    /* reduce  to sub-expression.                   */
			    case IRIT_E2T_ABS:
			    case IRIT_E2T_ARCCOS:
			    case IRIT_E2T_ARCSIN:
			    case IRIT_E2T_ARCTAN:
			    case IRIT_E2T_COS:
			    case IRIT_E2T_EXP:
			    case IRIT_E2T_LN:
			    case IRIT_E2T_LOG:
			    case IRIT_E2T_SIN:
			    case IRIT_E2T_SQR:
			    case IRIT_E2T_SQRT:
			    case IRIT_E2T_TAN:
                                IritFree(Stack[Temp1]); /* Free open paran. */
				Stack[StackPointer] -> NodeKind -= 1000;
				Stack[Temp1 - 1] -> NodeKind += 1000;
			        Stack[Temp1 - 1] -> Right = Stack[StackPointer];
				StackPointer -= 2;
				break;
			    default:
                                IritFree(Stack[Temp1]); /* Free open paran. */
                                Stack[Temp1] = Stack[StackPointer--];
				break;
			}
			/* Get another Token. */
                        Token = E2TGetToken(s, i, &RData, &SData);
                        continue;
		    }
                    else if (Stack[Temp1] -> NodeKind == IRIT_E2T_TOKENSTART) {
			/* No match for this one! */
                        E2TParsingError = IRIT_E2T_PARAMATCH_ERROR;
			return NULL;
		    }
		    break;
                case IRIT_E2T_TOKENEND:
                    if (Stack[Temp1] -> NodeKind == IRIT_E2T_TOKENSTART) {
                        if (StackPointer != 1) {
                            E2TParsingError = IRIT_E2T_SYNTAX_ERROR;
			    return NULL;
			}
			if (Stack[0] != NULL)
			    IritFree(Stack[0]);
			Stack[1] -> NodeKind -= 1000;
			return Stack[1];
		    }
		}

            Temp2 = Temp1 - 1;           /* Find the lower bound of handle. */
            while (Stack[Temp2] -> NodeKind >= 1000)
	        Temp2--;
            LowHandle = Temp2 + 1;
            if (LowHandle < 1) {                 /* No low bound was found. */
                E2TParsingError = IRIT_E2T_SYNTAX_ERROR;
	        return NULL;/* We ignore Data till now. */
            }
	    switch (StackPointer - LowHandle + 1) {
		case 1:   /* Its a scalar one - mark it as used (add 1000). */
		    switch (Stack[StackPointer] -> NodeKind) {
			case IRIT_E2T_NUMBER:
			case IRIT_E2T_PARAMETER:
		            Stack[StackPointer] -> NodeKind += 1000;
			    break;
			default:
			    E2TParsingError = IRIT_E2T_PARAM_EXPECT_ERROR;
			    return NULL;
		    }
		    break;
		case 2:    /* Its a nmonadic operator - create the subtree. */
		    switch (Stack[StackPointer - 1] -> NodeKind) {
		        case IRIT_E2T_UNARMINUS:
		            Stack[StackPointer - 1] -> Right =
                                                        Stack[StackPointer];
		            Stack[StackPointer] -> NodeKind -= 1000;
		            Stack[StackPointer - 1] -> NodeKind += 1000;
		            StackPointer --;
		            break;
		        case IRIT_E2T_OPENPARA:
			    E2TParsingError = IRIT_E2T_PARAMATCH_ERROR;
			    return NULL;
		        default:
			    E2TParsingError = IRIT_E2T_ONE_OPERAND_ERROR;
			    return NULL;
		    }
		    break;
		case 3:      /* Its a diadic operator - create the subtree. */
		    switch (Stack[StackPointer - 1] -> NodeKind) {
		        case IRIT_E2T_PLUS:
		        case IRIT_E2T_MINUS:
		        case IRIT_E2T_MULT:
		        case IRIT_E2T_DIV:
		        case IRIT_E2T_POWER:
		            Stack[StackPointer - 1] -> Right =
                                  Stack[StackPointer];
                            Stack[StackPointer - 1] -> Left =
                                  Stack[StackPointer - 2];
		            Stack[StackPointer - 2] -> NodeKind -= 1000;
		            Stack[StackPointer] -> NodeKind -= 1000;
		            Stack[StackPointer - 1] -> NodeKind += 1000;
		            Stack[StackPointer - 2] = Stack[StackPointer - 1];
		            StackPointer -= 2;
                            break;
                        default:
			    E2TParsingError = IRIT_E2T_TWO_OPERAND_ERROR;
			    return NULL;
		    }
		    break;
	    }
        }
        else {          /* Push that Token on Stack - it is not handle yet. */
	    Stack[++StackPointer] = E2TMallocNode();
            if (StackPointer == IRIT_E2T_MAX_PARSER_STACK - 1) {
                E2TParsingError = IRIT_E2T_STACK_OV_ERROR;
		return NULL;			/* We ignore data till now. */
	    }
            Stack[StackPointer] -> NodeKind = Token;
	    /* We might need that... */
            Stack[StackPointer] -> SData = SData;
	    SData = NULL;
            Stack[StackPointer] -> RData = RData;
	    Stack[StackPointer] -> Right = 
	        Stack[StackPointer] -> Left = (IritE2TExprNodeStruct *) NULL;
	    /* And get new Token from stream. */
            Token = E2TGetToken(s, i, &RData, &SData);
	}
    }
    while (TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to test precedence of two Tokens. returns 0, <0 or >0 according  *
* to comparison results.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Token1, Token2:   The two tokens to compare precedence.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     0, <0 or >0 according to comparison result.                     *
*****************************************************************************/
static int E2TTestPreceeding(int Token1, int Token2)
{
    int Preced1, Preced2;

    if ((Token1 >= 1000) || (Token2 >= 1000))
        return 0; /* Ignore sub-expr. */

    switch (Token1) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN: 
	    Preced1 = 100;
	    break;
	case IRIT_E2T_NUMBER:
	case IRIT_E2T_PARAMETER: 
	    Preced1 = 120;
	    break;
	case IRIT_E2T_PLUS:

	case IRIT_E2T_MINUS:
	    Preced1 = 20;
	    break;
	case IRIT_E2T_MULT:
	case IRIT_E2T_DIV:
	    Preced1 = 40;
	    break;
	case IRIT_E2T_POWER:
	    Preced1 = 60;
	    break;
	case IRIT_E2T_UNARMINUS:
	    Preced1 = 65;
	    break;
	case IRIT_E2T_OPENPARA:
	    Preced1 = 5;
	    break;
	case IRIT_E2T_CLOSPARA:
	     Preced1 = 120;
	     break;
	case IRIT_E2T_TOKENSTART:
	case IRIT_E2T_TOKENEND:
	    Preced1 =  5;
	    break;
        default:
	    Preced1 =  -1;
	    assert(0);
	    break;
    }

    switch (Token2) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	    Preced2 = 90;
	    break;
	case IRIT_E2T_NUMBER:
	case IRIT_E2T_PARAMETER:
	    Preced2 =110;
	    break;
	case IRIT_E2T_PLUS:
	case IRIT_E2T_MINUS:
	    Preced2 = 10;
	    break;
	case IRIT_E2T_MULT:
	case IRIT_E2T_DIV: 
	    Preced2 = 30;
	    break;
	case IRIT_E2T_POWER:
	    Preced2 = 50;
	    break;
	case IRIT_E2T_UNARMINUS:
	    Preced2 = 70;
	    break;
	case IRIT_E2T_OPENPARA:
	    Preced2 = 110;
	    break;
	case IRIT_E2T_CLOSPARA:
	    Preced2 = 0;
	    break;
	case IRIT_E2T_TOKENSTART:
	case IRIT_E2T_TOKENEND:
	    Preced2 = 0;
	    break;
        default:
	    Preced2 =  -1;
	    assert(0);
	    break;
    }

    return Preced1 - Preced2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to get the next Token out of the expression.                     *
* Gets the expression in S, and current position in i.                       *
* Returns the next Token found, set data to the returned value (if any),     *
* and update i to one char ofter the new Token found.                        *
*   Note that in minus Sign case, it is determined whether it is monadic or  *
* diadic minus by the last Token - if the last Token was operator or '('     *
* it is monadic minus.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   s:      The fetched token as a string.                                   *
*   i:      The next char.                                                   *
*   RData:  If numeric, save here.                                           *
*   SData:  IF String, save here.	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   Fetched token.                                                    *
*****************************************************************************/
static int E2TGetToken(const char s[], int *i, IrtRType *RData, char **SData)
{
    int j;
    char StrTkn[IRIT_LINE_LEN_LONG];
	
    while ((s[*i] == ' ') || (s[*i] == IRIT_ASCII_TAB))
        (*i)++;
    if (*i >= (int) strlen(s))
        return IRIT_E2T_TOKENEND;	   /* No more Tokens around here... */

    /* Check is next Token is one char - if so its a parameter. */
    if (islower(s[*i]) || isupper(s[*i])) {
        j=0;
	while (islower(s[*i]) || isupper(s[*i]) || isdigit(s[*i]))
	    StrTkn[j++] = s[(*i)++];
        StrTkn[j] = 0;
        *SData = IritStrdup(StrTkn);
        E2TGlblLastTkn = IRIT_E2T_PARAMETER;
        return IRIT_E2T_PARAMETER;
    }
        
    if (isdigit(s[*i]) || (s[*i] == '.')) {
        j=0;
        while (isdigit(s[*i]) || (s[*i] == '.'))
	    StrTkn[j++] = s[(*i)++];
        StrTkn[j] = 0;
        sscanf(StrTkn, "%lf", RData);
        E2TGlblLastTkn = IRIT_E2T_NUMBER;
        return IRIT_E2T_NUMBER;
    }

    switch (s[(*i)++]) {
        case 0:
	    E2TGlblLastTkn = 0;
	    return 0;
	case '+':
	    E2TGlblLastTkn = IRIT_E2T_PLUS;
	    return IRIT_E2T_PLUS;
	case '-':
	    switch (E2TGlblLastTkn) {
	        case 0:  /* If first Token (no last Token yet) */
	        case IRIT_E2T_PLUS:
	        case IRIT_E2T_MINUS:
	        case IRIT_E2T_MULT:
	        case IRIT_E2T_DIV:
	        case IRIT_E2T_POWER:
	        case IRIT_E2T_UNARMINUS:
	        case IRIT_E2T_OPENPARA:
		    E2TGlblLastTkn = IRIT_E2T_UNARMINUS;
		    return IRIT_E2T_UNARMINUS;
		default:
                    E2TGlblLastTkn = IRIT_E2T_MINUS;
		    return IRIT_E2T_MINUS;
	    }
	case '*':
	    E2TGlblLastTkn = IRIT_E2T_MULT;
	    return IRIT_E2T_MULT;
	case '/':
	    E2TGlblLastTkn = IRIT_E2T_DIV;
	    return IRIT_E2T_DIV;
	case '^':
	    E2TGlblLastTkn = IRIT_E2T_POWER;
	    return IRIT_E2T_POWER;
	case '(':
	    E2TGlblLastTkn = IRIT_E2T_OPENPARA;
	    return IRIT_E2T_OPENPARA;
	case ')':
	    E2TGlblLastTkn = IRIT_E2T_CLOSPARA;
	    return IRIT_E2T_CLOSPARA;
	case 'A':
	    if ((s[*i] == 'R') && (s[*i+1] == 'C')) {
	        (*i) += 2;
		switch (E2TGetToken(s, i, RData, SData)) {
		    case IRIT_E2T_SIN:
		        E2TGlblLastTkn = IRIT_E2T_ARCSIN;
			return IRIT_E2T_ARCSIN;
		    case IRIT_E2T_COS:
		        E2TGlblLastTkn = IRIT_E2T_ARCCOS;
			return IRIT_E2T_ARCCOS;
		    case IRIT_E2T_TAN:
		        E2TGlblLastTkn = IRIT_E2T_ARCTAN;
			return IRIT_E2T_ARCTAN;
		    default:
		        E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
			return IRIT_E2T_TOKENERROR;
		}
	    }
	    else if ((s[*i] == 'B') && (s[*i+1] == 'S')) {
	        (*i) += 2;
		E2TGlblLastTkn = IRIT_E2T_ABS;
		return IRIT_E2T_ABS;
	    }
	    else {
	        E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
		return IRIT_E2T_TOKENERROR;
	    }
	case 'C':
	    if ((s[*i] == 'O') && (s[*i+1] == 'S')) {
	        (*i) += 2;
		E2TGlblLastTkn = IRIT_E2T_COS;
		return IRIT_E2T_COS;
	    }
	    else {
	        E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
		return IRIT_E2T_TOKENERROR;
	    }
	case 'E':
	    if ((s[*i] == 'X') && (s[*i+1] == 'P')) {
	        (*i) += 2;
		E2TGlblLastTkn = IRIT_E2T_EXP;
		return IRIT_E2T_EXP;
	    }
	    else {
	        E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
		return IRIT_E2T_TOKENERROR;
	    }
	case 'L':
	    if ((s[*i] == 'O') && (s[*i+1] == 'G')) {
	        (*i)+=2;
		E2TGlblLastTkn = IRIT_E2T_LOG;
		return IRIT_E2T_LOG;
	    }
	    else if (s[*i] == 'N') {
	        (*i)++;
		E2TGlblLastTkn = IRIT_E2T_LN;
		return IRIT_E2T_LN;
	    }
	    else {
	        E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
		return IRIT_E2T_TOKENERROR;
	    }
	case 'S':
	    if ((s[*i] == 'I') && (s[*i+1] == 'N')) {
	        (*i) += 2;
		E2TGlblLastTkn = IRIT_E2T_SIN;
		return IRIT_E2T_SIN;
	    }
	    else if ((s[*i] == 'Q') && (s[*i+1] == 'R')) {
	        (*i) += 2;
		if (s[*i] == 'T') {
		    (*i)++;
		    E2TGlblLastTkn = IRIT_E2T_SQRT;
		    return IRIT_E2T_SQRT;
		}
		else {
		    E2TGlblLastTkn = IRIT_E2T_SQR;
		    return IRIT_E2T_SQR;
		}
	    }
	    else {
	        E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
		return IRIT_E2T_TOKENERROR;
	    }
	case 'T':
	    if ((s[*i] == 'A')&&(s[*i+1] == 'N')) {
	        (*i) += 2;
		E2TGlblLastTkn = IRIT_E2T_TAN;
		return IRIT_E2T_TAN;
	    }
	    else
	        return IRIT_E2T_TOKENERROR;
	default:
	    E2TParsingError = IRIT_E2T_UNDEF_TOKEN_ERROR;
	    return IRIT_E2T_TOKENERROR;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to print a content of Root (using inorder traversal):            M
* If *str = NULL print on stdout, else on given string str.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:   Tree to print.                                                   M
*   Str:    Destination.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TPrintTree                                                         M
*****************************************************************************/
void IritE2TPrintTree(const IritE2TExprNodeStruct *Root, char *Str)
{
    char LocalStr[IRIT_LINE_LEN_VLONG];

    strcpy(LocalStr, "");                          /* Make the String empty */

    if (Str == NULL) {
        E2TLocalPrintTree(Root, 0, LocalStr);         /* Copy to local Str. */
	printf(LocalStr);                                   /* and print... */
    }
    else {
        strcpy(Str, "");			   /* Make the string empty */
	E2TLocalPrintTree(Root, 0, Str);/* No print to stdout - copy to Str.*/
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to print a content of ROOT (using inorder traversal):            *
* Level holds: 0 for lowest level +/-, 1 for *,/, 2 for ^ operations.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Root:    Tree to print.                                                  *
*   Level:   Of recursion.                                                   *
*   Str:     Where output goes to.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void E2TLocalPrintTree(const IritE2TExprNodeStruct *Root,
			      int Level,
			      char *Str)
{
    int CloseFlag = FALSE;

    if (!Root)
        return;

    switch (Root -> NodeKind) {
        case IRIT_E2T_ABS:
	    strcat(Str, "abs(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_ARCCOS:
	    strcat(Str, "arccos(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_ARCSIN:
	    strcat(Str, "arcsin(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_ARCTAN:
	    strcat(Str, "arctan(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_COS:   
	    strcat(Str, "cos(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_EXP:   
	    strcat(Str, "exp(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_LN:   
	    strcat(Str, "ln(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_LOG:   
	    strcat(Str, "log(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_SIN:   
	    strcat(Str, "sin(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_SQR:   
	    strcat(Str, "sqr(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_SQRT:  
	    strcat(Str, "sqrt(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_TAN:   
	    strcat(Str, "tan(");
	    Level = 0;        /* Paranthesis are opened */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
	case IRIT_E2T_DIV:
	    if (Level > 1) {
	        strcat(Str, "(");
		CloseFlag = TRUE; /* must close paranthesis */
	    }
	    Level = 1;        /* Div Level */
	    E2TLocalPrintTree(Root -> Left, Level, Str);
	    strcat(Str, "/");
	    break;
	case IRIT_E2T_MINUS:
	    if (Level > 0) {
	        strcat(Str, "(");
		CloseFlag = TRUE; /* must close paranthesis */
	    }
	    Level = 0;        /* Minus Level */
	    E2TLocalPrintTree(Root -> Left, Level, Str);
	    strcat(Str, "-");
	    break;
	case IRIT_E2T_MULT:
	    if (Level > 1) {
	        strcat(Str, "(");
		CloseFlag = TRUE; /* must close paranthesis */
	    }
	    Level = 1;        /* Mul Level */
	    E2TLocalPrintTree(Root -> Left, Level, Str);
	    strcat(Str, "*");
	    break;
	case IRIT_E2T_PLUS:
	    if (Level > 0) {
	        strcat(Str, "(");
		CloseFlag = TRUE; /* must close paranthesis */
	    }
	    Level = 0;        /* Plus Level */
	    E2TLocalPrintTree(Root -> Left, Level, Str);
	    strcat(Str, "+");
	    break;
	case IRIT_E2T_POWER:
	    Level = 2;        /* Power Level */
	    E2TLocalPrintTree(Root -> Left, Level, Str);
	    strcat(Str, "^");
	    break;
	case IRIT_E2T_UNARMINUS:
	    strcat(Str, "(-");
	    Level = 0;        /* Unarminus Level ! */
	    CloseFlag = TRUE; /* must close paranthesis */
	    break;
        case IRIT_E2T_NUMBER:
	    sprintf(&Str[strlen(Str)], "%lg", Root -> RData);
	    break;
	case IRIT_E2T_PARAMETER:
	    sprintf(&Str[strlen(Str)], "%s", Root -> SData);
	    break;
    }

    E2TLocalPrintTree(Root -> Right, Level, Str);
    if (CloseFlag)
        strcat(Str, ")");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a new copy of a given tree.                  	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:   Tree to duplicate.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritE2TExprNodeStruct *:   Duplicated tree.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TCopyTree                                                          M
*****************************************************************************/
IritE2TExprNodeStruct *IritE2TCopyTree(const IritE2TExprNodeStruct *Root)
{
    IritE2TExprNodeStruct *Node;

    if (!Root)
        return NULL;

    Node = E2TMallocNode();
    switch (Root -> NodeKind) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	    Node -> NodeKind = Root -> NodeKind;
	    Node -> Right = IritE2TCopyTree(Root -> Right);
	    Node -> Left = NULL;
	    return Node;
	case IRIT_E2T_DIV:
	case IRIT_E2T_MINUS:
	case IRIT_E2T_MULT:
	case IRIT_E2T_NUMBER:
	case IRIT_E2T_PARAMETER:
	case IRIT_E2T_PLUS:
	case IRIT_E2T_POWER:
	case IRIT_E2T_UNARMINUS:
	    Node -> NodeKind = Root -> NodeKind;
	    if ((Root -> NodeKind == IRIT_E2T_PARAMETER) ||
		(Root -> NodeKind == IRIT_E2T_NUMBER)) {
	        Node -> RData = Root -> RData;
	        Node -> SData =
		    Root -> SData != NULL ? IritStrdup(Root -> SData) : NULL;
	    }
	    Node -> Right = IritE2TCopyTree(Root -> Right);
	    Node -> Left  = IritE2TCopyTree(Root -> Left );
	    return Node;
        default:
	    assert(0);
	    return NULL;		               /* Never get here... */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to evaluate a value of a given tree Root and set parameters.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:  Tree to evaluate.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Evaluated result.                                            M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TEvalTree                                                          M
*****************************************************************************/
IrtRType IritE2TEvalTree(const IritE2TExprNodeStruct *Root)
{
    IrtRType Temp;

    switch (Root -> NodeKind) {
	case IRIT_E2T_ABS:
	    Temp = IritE2TEvalTree(Root -> Right);
	    return Temp > 0 ? Temp: -Temp;
	case IRIT_E2T_ARCSIN:
	    return asin(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_ARCCOS:
	    return acos(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_ARCTAN:
	    return atan(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_COS:
	    return cos(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_EXP:
	    return exp(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_LN:
	    return log(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_LOG:
	    return log10(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_SIN:
	    return sin(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_SQR:
	    Temp = IritE2TEvalTree(Root -> Right);
	    return IRIT_SQR(Temp);
	case IRIT_E2T_SQRT:
	    return sqrt(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_TAN:
	    return tan(IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_DIV:
	    return IritE2TEvalTree(Root -> Left) /
	           IritE2TEvalTree(Root -> Right);
	case IRIT_E2T_MINUS:
	    return IritE2TEvalTree(Root -> Left) -
	           IritE2TEvalTree(Root -> Right);
	case IRIT_E2T_MULT:
	    return IritE2TEvalTree(Root -> Left) *
	           IritE2TEvalTree(Root -> Right);
	case IRIT_E2T_PLUS:
	    return IritE2TEvalTree(Root -> Left) +
	           IritE2TEvalTree(Root -> Right);
	case IRIT_E2T_POWER:
	    return pow(IritE2TEvalTree(Root -> Left),
		       IritE2TEvalTree(Root -> Right));
	case IRIT_E2T_UNARMINUS:
	    return -IritE2TEvalTree(Root -> Right);
	case IRIT_E2T_NUMBER:
	    return Root -> RData;
	case IRIT_E2T_PARAMETER:
	    assert(Root -> SData != NULL);
	    return
	        Root -> SData != NULL ? E2GlblFetchParamValueFunc(Root -> SData)
				      : 0.0;
    }
    return 0; /* Never get here (only make lint quite...) */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees a given expression tree.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:   Tree to free.                                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TFreeTree                                                          M
*****************************************************************************/
void IritE2TFreeTree(IritE2TExprNodeStruct *Root)
{
    if (!Root)
        return;

    switch (Root -> NodeKind) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	    IritE2TFreeTree(Root -> Right);
	    break;
	case IRIT_E2T_DIV:
	case IRIT_E2T_MINUS:
	case IRIT_E2T_MULT:
	case IRIT_E2T_NUMBER:
	case IRIT_E2T_PARAMETER:
	case IRIT_E2T_PLUS:
	case IRIT_E2T_POWER:
	case IRIT_E2T_UNARMINUS:
	    IritE2TFreeTree(Root -> Right);
	    IritE2TFreeTree(Root -> Left);
	    break;
        default:
	    assert(0);
	    break;			               /* Never get here... */
    }

    E2TFreeNode(Root);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to generate the tree represent the derivative of tree Root.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:   To derive.                                                       M
*   Param:  The parameter to differentiate according to.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritE2TExprNodeStruct *:   Derived tree.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TDerivTree                                                         M
*****************************************************************************/
IritE2TExprNodeStruct *IritE2TDerivTree(const IritE2TExprNodeStruct *Root,
					int Param)
{
    int i,
        Flag = TRUE;
    IritE2TExprNodeStruct *Node,
	*newNode = NULL;

    Node = E2TLclDerivTree(Root, Param);
    if (!E2TGlblDerivError) {
        while (Flag) {
            Flag = FALSE;
            newNode = E2TOptimize(Node, &Flag);
            E2TFreeNode(Node); /* Release old tree area. */
            Node = newNode;
        }
        for (i = 0; i < 10; i++) {
	    /* Do more loops - might E2TOptimize by shift. */
            Flag = FALSE;
            newNode = E2TOptimize(Node, &Flag);
            E2TFreeNode(Node); /* Release old tree area. */
            Node = newNode;
        }
    }
    return newNode;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to generate non optimal derivative of the tree Root.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Root:   To derive.                                                       *
*   Prm:    The parameter to differentiate according to.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IritE2TExprNodeStruct *:   Derived tree.                                 *
*****************************************************************************/
static IritE2TExprNodeStruct *E2TLclDerivTree(const IritE2TExprNodeStruct *Root,
					      int Prm)
{
    IritE2TExprNodeStruct *Node1, *Node2, *Node3, *Node4, *NodeMul;

    NodeMul = E2TMallocNode();
    NodeMul -> NodeKind = IRIT_E2T_MULT;

    switch (Root -> NodeKind) {
	case IRIT_E2T_ABS:
	    E2TGlblDerivError = IRIT_E2T_DERIV_NO_ABS_DERIV_ERROR;
	    return NULL; /* No derivative ! */
	case IRIT_E2T_ARCSIN:
	    NodeMul -> Left = E2TGen1u2Tree(IRIT_E2T_PLUS, IRIT_E2T_MINUS,
					    -0.5,
					    IritE2TCopyTree(Root -> Right));
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_ARCCOS:
	    NodeMul -> Left = E2TGen1u2Tree(IRIT_E2T_MINUS, IRIT_E2T_MINUS,
					    -0.5,
					    IritE2TCopyTree(Root -> Right));
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_ARCTAN:
	    NodeMul -> Left = E2TGen1u2Tree(IRIT_E2T_PLUS, IRIT_E2T_PLUS,
					    -1.0,
					    IritE2TCopyTree(Root -> Right));
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_COS:
	    Node1 = E2TMallocNode();
	    Node2 = E2TMallocNode();
	    Node1 -> NodeKind = IRIT_E2T_UNARMINUS;
	    Node2 -> NodeKind = IRIT_E2T_SIN;
	    Node2 -> Right = IritE2TCopyTree(Root -> Right);
	    Node1 -> Left = Node2 -> Left = NULL;
	    Node1 -> Right = Node2;
	    NodeMul -> Left = Node1;
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_EXP:
	    Node1 = E2TMallocNode();
	    Node1 -> NodeKind = IRIT_E2T_EXP;
	    Node1 -> Left = NULL;
	    Node1 -> Right = IritE2TCopyTree(Root -> Right);
	    NodeMul -> Left = Node1;
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_LN:
	    NodeMul -> NodeKind = IRIT_E2T_DIV; /* Not nice, but work ! */
	    NodeMul -> Right = IritE2TCopyTree(Root -> Right);
	    NodeMul -> Left = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_LOG:
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_DIV;
	    Node1 -> Right = IritE2TCopyTree(Root -> Right);
	    Node1 -> Left = E2TLclDerivTree(Root -> Right, Prm);
	    Node2 -> NodeKind = IRIT_E2T_NUMBER;;
	    Node2 -> RData = log10(exp(1.0));
	    NodeMul -> Left = Node1;
	    NodeMul -> Right = Node2;
	    return NodeMul;
	case IRIT_E2T_SIN:
	    Node1 = E2TMallocNode();
	    Node1 -> NodeKind = IRIT_E2T_COS;
	    Node1 -> Right = IritE2TCopyTree(Root -> Right);
	    Node1 -> Left = NULL;
	    NodeMul -> Left = Node1;
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_SQR:
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_NUMBER;
	    Node1 -> Right = Node1 -> Left = NULL;
	    Node1 -> RData = 2.0;
	    Node2 -> NodeKind = IRIT_E2T_MULT;
	    Node2 -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    Node2 -> Left  = IritE2TCopyTree(Root -> Right);
	    NodeMul -> Left = Node1;
	    NodeMul -> Right = Node2;
	    return NodeMul;
	case IRIT_E2T_SQRT:
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node3 = E2TMallocNode();   
	    Node4 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_NUMBER;
	    Node1 -> Right = Node1 -> Left = NULL;
	    Node1 -> RData = -0.5;
	    Node2 -> NodeKind = IRIT_E2T_POWER;
	    Node2 -> Right = Node1;
	    Node2 -> Left  = IritE2TCopyTree(Root -> Right);
	    Node3 -> NodeKind = IRIT_E2T_NUMBER;
	    Node3 -> Right = Node3 -> Left = NULL;
	    Node3 -> RData = 0.5;
	    Node4 -> NodeKind = IRIT_E2T_MULT;
	    Node4 -> Right = Node2;
	    Node4 -> Left  = Node3;
	    NodeMul -> Left = Node4;
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_TAN:
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_COS;
	    Node1 -> Left = NULL;
	    Node1 -> Right = IritE2TCopyTree(Root -> Right);
	    Node2 -> NodeKind = IRIT_E2T_SQR;
	    Node2 -> Left = NULL;
	    Node2 -> Right = Node1;
	    NodeMul -> NodeKind = IRIT_E2T_DIV;     /* Not noce, but work! */
	    NodeMul -> Right = Node2;
	    NodeMul -> Left = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_DIV:
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node3 = E2TMallocNode();   
	    Node4 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_MULT;
	    Node1 -> Left  = IritE2TCopyTree(Root -> Left);
	    Node1 -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    Node2 -> NodeKind = IRIT_E2T_MULT;
	    Node2 -> Left  = E2TLclDerivTree(Root -> Left, Prm);
	    Node2 -> Right = IritE2TCopyTree(Root -> Right);
	    Node3 -> NodeKind = IRIT_E2T_MINUS;
	    Node3 -> Right = Node1;
	    Node3 -> Left = Node2;
	    Node4 -> NodeKind = IRIT_E2T_SQR;
	    Node4 -> Right = IritE2TCopyTree(Root -> Right);
	    Node4 -> Left  = NULL;
	    NodeMul -> NodeKind = IRIT_E2T_DIV;     /* Not nice, but work! */
	    NodeMul -> Left = Node3;
	    NodeMul -> Right = Node4;
	    return NodeMul;
	case IRIT_E2T_MINUS:
	    NodeMul -> NodeKind = IRIT_E2T_MINUS;   /* Not nice, but work! */
	    NodeMul -> Left = E2TLclDerivTree(Root -> Left, Prm);
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_MULT:
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_MULT;
	    Node1 -> Left  = IritE2TCopyTree(Root -> Left);
	    Node1 -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    Node2 -> NodeKind = IRIT_E2T_MULT;
	    Node2 -> Left  = E2TLclDerivTree(Root -> Left, Prm);
	    Node2 -> Right = IritE2TCopyTree(Root -> Right);
	    NodeMul -> NodeKind = IRIT_E2T_PLUS;    /* Not nice, but work! */
	    NodeMul -> Left = Node1;
	    NodeMul -> Right = Node2;
	    return NodeMul;
	case IRIT_E2T_PLUS:
	    NodeMul -> NodeKind = IRIT_E2T_PLUS;    /* Not nice, but work! */
	    NodeMul -> Left = E2TLclDerivTree(Root -> Left, Prm);
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    return NodeMul;
	case IRIT_E2T_POWER:
	    if (Root -> Right -> NodeKind != IRIT_E2T_NUMBER) {
	        E2TGlblDerivError = IRIT_E2T_DERIV_NONE_CONST_EXP_ERROR;
		return NULL;
	    }
	    Node1 = E2TMallocNode();   
	    Node2 = E2TMallocNode();   
	    Node3 = E2TMallocNode();   
	    Node4 = E2TMallocNode();   
	    Node1 -> NodeKind = IRIT_E2T_NUMBER;
	    Node1 -> Left  = Node1 -> Right = NULL;
	    Node1 -> RData = Root -> Right -> RData - 1;
	    Node2 -> NodeKind = IRIT_E2T_POWER;
	    Node2 -> Left  = IritE2TCopyTree(Root -> Left);
	    Node2 -> Right = Node1;
	    Node3 -> NodeKind = IRIT_E2T_NUMBER;
	    Node3 -> Left  = Node3 -> Right = NULL;
	    Node3 -> RData = Root -> Right -> RData;
	    Node4 -> NodeKind = IRIT_E2T_MULT;
	    Node4 -> Right = Node2;
	    Node4 -> Left  = Node3;
	    NodeMul -> Left = Node4;
	    NodeMul -> Right = E2TLclDerivTree(Root -> Left, Prm);
	    return NodeMul;
	case IRIT_E2T_UNARMINUS:
	    NodeMul -> NodeKind = IRIT_E2T_UNARMINUS;/* Not nice, but work! */
	    NodeMul -> Right = E2TLclDerivTree(Root -> Right, Prm);
	    NodeMul -> Left  = NULL;
	    return NodeMul;
	case IRIT_E2T_NUMBER:
	    NodeMul -> NodeKind = IRIT_E2T_NUMBER;   /* Not nice, but work! */
	    NodeMul -> Left = NodeMul -> Right = NULL;
	    NodeMul -> RData = 0.0;
	    return NodeMul;
	case IRIT_E2T_PARAMETER:
	    NodeMul -> NodeKind = IRIT_E2T_NUMBER;   /* Not nice, but work! */
	    NodeMul -> Left = NodeMul -> Right = NULL;
	    if ((int) (Root -> RData) == Prm) 
	        NodeMul -> RData = 1.0;
	    else
	        NodeMul -> RData = 0.0;
	    return NodeMul;
        default:
	    assert(0);
	    return NULL;		               /* Never get here... */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Routine to generate a tree to the expression:                             *
*      Sign1(1 Sign2 SQR(Expr)) ^ Exponent                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Sign1, Sign2:    One of IRIT_E2T_PLUS/MINUS.                             *
*   Exponent:        Desired power.                                          *
*   Expr:            Sub extression to process.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   IritE2TExprNodeStruct *:   The constructed tree.                         *
*****************************************************************************/
static IritE2TExprNodeStruct *E2TGen1u2Tree(int Sign1, 
					    int Sign2, 
					    IrtRType Exponent, 
					    IritE2TExprNodeStruct *Expr)
{
    IritE2TExprNodeStruct *Node1, *Node2, *Node3, *Node4, *Node5, *Node6;
    
    Node1 = E2TMallocNode();   
    Node2 = E2TMallocNode();   
    Node3 = E2TMallocNode();   
    Node4 = E2TMallocNode();   
    Node5 = E2TMallocNode();   
    Node1 -> NodeKind = IRIT_E2T_NUMBER;
    Node1 -> Left  = Node1 -> Right = NULL;
    Node1 -> RData = 1.0;
    Node2 -> NodeKind = IRIT_E2T_SQR;
    Node2 -> Right  = IritE2TCopyTree(Expr);
    Node2 -> Left = NULL;
    Node3 -> NodeKind = Sign2;
    Node3 -> Left = Node1;
    Node3 -> Right = Node2;
    Node4 -> NodeKind = IRIT_E2T_NUMBER;
    Node4 -> RData = Exponent;
    Node4 -> Right = Node4 -> Left = NULL;
    Node5 -> NodeKind = IRIT_E2T_POWER;
    Node5 -> Right = Node4;
    Node5 -> Left = Node3;
    if (Sign1 == IRIT_E2T_PLUS)
        return Node5;
    else { /* Must be IRIT_E2T_MINUS. */
        Node6 = E2TMallocNode();   
        Node6 -> NodeKind = IRIT_E2T_UNARMINUS;
        Node6 -> Left = NULL;
        Node6 -> Right = Node5;
        return Node6;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Routine to optimize the binary tree:                                      *
* Note: the old tree is NOT modified. Flag returns TRUE if optimized.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Root:   To optimize.                                                     *
*   Flag:   TRUE if modified, FASLE otherwise.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IritE2TExprNodeStruct *:  Optimized tree.                                *
*****************************************************************************/
static IritE2TExprNodeStruct *E2TOptimize(IritE2TExprNodeStruct *Root,
					  int *Flag)
{
    IritE2TExprNodeStruct *Node, *Node2;

    if (!Root)
        return NULL;

    if ((Root -> NodeKind != IRIT_E2T_NUMBER) &&
        (!IritE2TParamInTree(Root, NULL))) {
        /* Expression is constant. */
        *Flag = TRUE;
        Node = E2TMallocNode();
        Node -> NodeKind = IRIT_E2T_NUMBER;
        Node -> RData = IritE2TEvalTree(Root);
        Node -> Right = Node -> Left = NULL;
        return Node;
    }
    /* Shift in Mult or Plus: A+(B+C) - ->   C+(A+B). */
    if ((Root -> NodeKind == IRIT_E2T_PLUS) ||
	(Root -> NodeKind == IRIT_E2T_MULT))
        if ((Root -> NodeKind == Root -> Right -> NodeKind) &&
	    (Root -> NodeKind == IRIT_E2T_PLUS ||
	     Root -> NodeKind == IRIT_E2T_MULT)) {
	    Node = Root -> Left;
	    Root -> Left = Root -> Right -> Right;
	    Root -> Right -> Right = Root -> Right -> Left;
	    Root -> Right -> Left = Node;
	}

    /* Shift in Mult or Plus: (A+B)+C - ->   (C+A)+B. */
    if (Root -> NodeKind == IRIT_E2T_PLUS ||
	Root -> NodeKind == IRIT_E2T_MULT)
        if ((Root -> NodeKind == Root -> Left -> NodeKind) &&
	    (Root -> NodeKind == IRIT_E2T_PLUS ||
	     Root -> NodeKind == IRIT_E2T_MULT)) {
	    Node = Root -> Right;
	    Root -> Right = Root -> Left -> Left;
	    Root -> Left -> Left = Root -> Left -> Right;
	    Root -> Left -> Right = Node;
	}

    switch (Root -> NodeKind) {
	case IRIT_E2T_DIV:
	    if ((Root -> Right -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Right -> RData == 1.0)) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Left, Flag);        /* Div by 1. */
	    }
	    if ((Root -> Left  -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Left  -> RData == 0.0)) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Left, Flag);/* Div 0 - return 0. */
	    }
	    if (IritE2TCmpTree(Root -> Left, Root -> Right)) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_NUMBER;
		Node -> RData = 1.0;
		Node -> Right = Node -> Left = NULL;
		return Node;                                   /* f/f == 1. */
	    }
	    break;
	case IRIT_E2T_MINUS:
	    if ((Root -> Right -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Right -> RData == 0.0)) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Left, Flag);           /* Sub 0. */
	    }
	    if (IritE2TCmpTree(Root -> Left, Root -> Right)) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_NUMBER;
		Node -> RData = 0.0;
		Node -> Right = Node -> Left = NULL;
		return Node; /* f-f == 0.0 */
	    }
	    if (Root -> Right -> NodeKind == IRIT_E2T_UNARMINUS) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_PLUS;
		Node -> Left = E2TOptimize(Root -> Left, Flag);
		Node -> Right = E2TOptimize(Root -> Right -> Right, Flag);
		Node2 = E2TOptimize(Node, Flag);     /* a-(-b) - ->  a + b. */
		IritE2TFreeTree(Node);
		return Node2;
	    }
	    break;
	case IRIT_E2T_MULT:
	    if ((Root -> Right -> NodeKind == IRIT_E2T_NUMBER) &&
		((Root -> Right -> RData == 1.0) ||
		 (Root -> Right -> RData == 0.0))) {
	        *Flag = TRUE;
		if (Root -> Right -> RData == 1.0)
		    return E2TOptimize(Root -> Left , Flag);   /* Mul by 1. */
		else
		    return E2TOptimize(Root -> Right, Flag);   /* Mul by 0. */
	    }
	    if ((Root -> Left  -> NodeKind == IRIT_E2T_NUMBER) &&
		((Root -> Left  -> RData == 1.0) ||
		 (Root -> Left  -> RData == 0.0))) {
	        *Flag = TRUE;
		if (Root -> Left -> RData == 1.0)
		    return E2TOptimize(Root -> Right, Flag);   /* Mul by 1. */
		else
		    return E2TOptimize(Root -> Left , Flag);   /* Mul by 0. */
	    }
	    if (IritE2TCmpTree(Root -> Left, Root -> Right)) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_SQR;
		Node -> Right = E2TOptimize(Root -> Right, Flag);
		Node -> Left = NULL;
		return Node;                                  /* f*f = f^2. */
	    }
	    break;
	case IRIT_E2T_PLUS:
	    if ((Root -> Right -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Right -> RData == 0.0)) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Left, Flag);           /* Add 0. */
	    }
	    if ((Root -> Left  -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Left  -> RData == 0.0)) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Right, Flag);          /* Add 0. */
	    }
	    if (IritE2TCmpTree(Root -> Left, Root -> Right)) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_MULT;
		Node -> Left = E2TOptimize(Root -> Right, Flag);
		Node -> Right = E2TMallocNode();
		Node -> Right -> NodeKind = IRIT_E2T_NUMBER;
		Node -> Right -> RData = 2.0;
		Node -> Right -> Left = Node -> Right -> Right = NULL;
		return Node; /* f+f = f*2 */
	    }
	    if (Root -> Right  -> NodeKind == IRIT_E2T_UNARMINUS) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_MINUS;
		Node -> Left = E2TOptimize(Root -> Left, Flag);
		Node -> Right = E2TOptimize(Root -> Right -> Right, Flag);
		Node2 = E2TOptimize(Node, Flag);       /* a+(-b) - ->  a-b. */
		IritE2TFreeTree(Node);
		return Node2;
	    }
	    if (Root -> Left  -> NodeKind == IRIT_E2T_UNARMINUS) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_MINUS;
		Node -> Left = E2TOptimize(Root -> Right, Flag);
		Node -> Right = E2TOptimize(Root -> Left -> Right, Flag);
		Node2 = E2TOptimize(Node, Flag);       /* (-a)+b - ->  b-a. */
		IritE2TFreeTree(Node);
		return Node2;
	    }
	    break;
	case IRIT_E2T_POWER:
	    if ((Root -> Right -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Right -> RData == 0.0)) {
	        *Flag = TRUE;
		Node = E2TMallocNode();
		Node -> NodeKind = IRIT_E2T_NUMBER;
		Node -> RData = 1.0;
		Node -> Right = Node -> Left = NULL;
		return Node;				       /* f^0 == 1. */
	    }
	    if ((Root -> Right -> NodeKind == IRIT_E2T_NUMBER) &&
		(Root -> Right -> RData == 1.0)) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Left, Flag);         /* f^1 = f. */
	    }
	    break;
	case IRIT_E2T_UNARMINUS:
	    if (Root -> Right -> NodeKind == IRIT_E2T_UNARMINUS) {
	        *Flag = TRUE;
		return E2TOptimize(Root -> Right -> Right, Flag); /* --a=a. */
	    }
	    break;
        default:
	    break;
    }

    /* If we are here - no optimization took place: */
    Node = E2TMallocNode();
    Node -> NodeKind = Root -> NodeKind;
    if ((Root -> NodeKind == IRIT_E2T_PARAMETER) ||
        (Root -> NodeKind == IRIT_E2T_NUMBER)) {
        Node -> RData = Root -> RData;
        Node -> SData = Root -> SData != NULL ? IritStrdup(Root -> SData)
					      : NULL;
    }
    Node -> Right = E2TOptimize(Root -> Right, Flag);
    Node -> Left  = E2TOptimize(Root -> Left , Flag);
    return Node;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compere two trees - for equality:                             M
* The trees are compered to be symbolically equal i.e. A*B == B*A !          M
*                                                                            *
* PARAMETERS:                                                                M
*   Root1, Root2:   The two trees to compare.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if equal, FALSE otherwise.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TCmpTree                                                           M
*****************************************************************************/
int IritE2TCmpTree(const IritE2TExprNodeStruct *Root1,
		   const IritE2TExprNodeStruct *Root2)
{
    if (Root1 -> NodeKind != Root2 -> NodeKind)
        return FALSE;

    switch (Root1 -> NodeKind) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	case IRIT_E2T_UNARMINUS:
	    return IritE2TCmpTree(Root1 -> Right, Root2 -> Right);

	case IRIT_E2T_MULT:      /* Note that A*B = B*A ! */
	case IRIT_E2T_PLUS:
	    return ((IritE2TCmpTree(Root1 -> Right, Root2 -> Right) &&
		     IritE2TCmpTree(Root1 -> Left , Root2 -> Left )) ||
		    (IritE2TCmpTree(Root1 -> Right, Root2 -> Left ) &&
		     IritE2TCmpTree(Root1 -> Left , Root2 -> Right)));

	case IRIT_E2T_DIV:
	case IRIT_E2T_MINUS:
	case IRIT_E2T_POWER:
	    return (IritE2TCmpTree(Root1 -> Right, Root2 -> Right) &&
		    IritE2TCmpTree(Root1 -> Left , Root2 -> Left ));

	case IRIT_E2T_NUMBER:
	    return Root1 -> RData == Root2 -> RData;

	case IRIT_E2T_PARAMETER:
	    assert(Root1 -> SData != NULL && Root2 -> SData != NULL);
	    return strcmp(Root1 -> SData, Root2 -> SData) == 0;

        default:
	    assert(0);
	    return FALSE;		               /* Never get here... */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to test if the parameter is in the tree:                         M
* If ParamName is NULL then any parameter return TRUE.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:        Tree to examine for an existence of a parameter.            M
*   ParamName:   Name of parameter to seek, or NULL to seek if any param.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if parameter exists, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TParamInTree                                                       M
*****************************************************************************/
int IritE2TParamInTree(const IritE2TExprNodeStruct *Root,
		       const char *ParamName)
{
    if (!Root)
        return FALSE;

    switch (Root -> NodeKind) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	case IRIT_E2T_UNARMINUS:
	    return IritE2TParamInTree(Root -> Right, ParamName);

	case IRIT_E2T_DIV:
	case IRIT_E2T_MINUS:
	case IRIT_E2T_MULT:
	case IRIT_E2T_PLUS:
	case IRIT_E2T_POWER:
	    return IritE2TParamInTree(Root -> Right, ParamName) ||
	           IritE2TParamInTree(Root -> Left, ParamName);

	case IRIT_E2T_NUMBER:
	    return FALSE;

	case IRIT_E2T_PARAMETER:
	    assert(Root -> SData != NULL);
	    if (ParamName != NULL)
	        return strcmp(Root -> SData, ParamName) == 0;
	    else
	        return TRUE;

	default:
	    assert(0);
	    return FALSE;		              /* Never get here ... */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to free a tree - release all memory allocated by it.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Root:   Tree to release.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   E2TFreeNodetree                                                          M
*****************************************************************************/
void E2TFreeNodetree(IritE2TExprNodeStruct *Root)
{
    if (!Root)
        return;

    switch (Root -> NodeKind) {
	case IRIT_E2T_ABS:
	case IRIT_E2T_ARCSIN:
	case IRIT_E2T_ARCCOS:
	case IRIT_E2T_ARCTAN:
	case IRIT_E2T_COS:
	case IRIT_E2T_EXP:
	case IRIT_E2T_LN:
	case IRIT_E2T_LOG:
	case IRIT_E2T_SIN:
	case IRIT_E2T_SQR:
	case IRIT_E2T_SQRT:
	case IRIT_E2T_TAN:
	case IRIT_E2T_UNARMINUS:
	    E2TFreeNodetree(Root -> Right);
	    E2TFreeNode(Root);
	    break;

	case IRIT_E2T_DIV:
	case IRIT_E2T_MINUS:
	case IRIT_E2T_MULT:
	case IRIT_E2T_PLUS:
	case IRIT_E2T_POWER:
	    E2TFreeNodetree(Root -> Right);
	    E2TFreeNodetree(Root -> Left);
	    E2TFreeNode(Root);
	    break;

	case IRIT_E2T_NUMBER:
	case IRIT_E2T_PARAMETER:
	    E2TFreeNode(Root);
	    break;

	default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to set the value of a Parameter before evaluating an expression. M
*                                                                            *
* PARAMETERS:                                                                M
*   Value:   New value to assign to a parameter.                             M
*   Index:   The index of the parameter.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TSetParamValue                                                     M
*****************************************************************************/
void IritE2TSetParamValue(IrtRType Value, int Index)
{
    if (Index >= 0 && Index < IRIT_E2T_PARAM_NUM_PARAM)
        E2TGlobalParam[Index] = Value;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get a parsing error is was one or 0 in none.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Error or 0 in none.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TParseError                                                        M
*****************************************************************************/
int IritE2TParseError(void)
{
    int Temp;

    Temp = E2TParsingError;
    E2TParsingError = 0;
    return Temp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get a derivative error is was one or 0 in none.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   Error or 0 in none.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritE2TDerivError                                                        M
*****************************************************************************/
int IritE2TDerivError(void)
{
    int Temp;

    Temp = E2TGlblDerivError;
    E2TGlblDerivError = 0;
    return Temp;
}

