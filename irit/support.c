/*****************************************************************************
*   Support data for "Irit" - the 3d (not only polygonal) solid modeller.    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 3.0, Feb. 2007   *
*****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "program.h"
#include "misc_lib.h"
#include "ctrl-brk.h"
#include "dosintr.h"
#include "inptprsg.h"
#include "iritprsr.h"
#include "objects.h"
#include "geom_lib.h"
#include "grap_lib.h"
#include "bool_lib.h"
#include "trim_lib.h"
#include "triv_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */
#ifdef __WINNT__
#include <direct.h>
#endif /* __WINNT__ */

IRIT_GLOBAL_DATA IPObjectStruct
    *GlblLastIritClientCursorEvent = NULL;    /* Cursor events from clients. */

IRIT_GLOBAL_DATA FILE
    *GlblLogFile = NULL;

IRIT_GLOBAL_DATA int
    GlblKeepClientsCursorEvents = FALSE,
    GlblQuietMode = FALSE,
    GlblCurrentDisplay = -1,       /* Handlers to streams to display device. */
    GlblFlatLoadMode = FALSE,
    GlblHandleDependencies = 0,
    GlblStdinInteractive = FALSE,
    GlblRunScriptAndQuit = FALSE,
    GlblBspProdMethod	 = 0,
    GlblDoGraphics       = TRUE,/* Control if running in graphics/text mode. */
    GlblGUIMode          = FALSE,		     /* Running under a GUI. */
    GlblFatalError       = FALSE, /* True if disaster in system - must quit! */
    GlblPrintLogFile     = FALSE,    /* If TRUE everything goes to log file. */
    GlblPointLenAux      = IG_POINT_DEFAULT_LENGTH;

IRIT_GLOBAL_DATA IrtRType
    GlblPointLen = 0.02;		       /* Scaler for point if drawn. */

IRIT_GLOBAL_DATA IrtVecType
    GlblBoolFreeformTols = { 0.01, 1e-8, 0.01 }; /* Subdiv/Numer/Trace tols. */

IRIT_GLOBAL_DATA jmp_buf IritGlblLongJumpBuffer;  /* Used in error recovery. */

IRIT_GLOBAL_DATA char
    *GlblHelpFileName = "irit.hlp",
    *GlblLineEditControl = " 8,  4,  2,  6,  1,  5, 16, 14, 11,  9, 10",
    *GlblStartFileName = "",
    *GlblLogFileName = "",
#ifdef IRIT_DOUBLE
    *GlblFloatFormat = "%-16.14lg";
#else
    *GlblFloatFormat = "%-10.8g";
#endif /* IRIT_DOUBLE */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Exec a line by injecting it into the input stream.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Line:     To execute for processing in input stream.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
 *                                                                            *
* KEYWORDS:                                                                  M
*   IritExecOneLine                                                          M
*****************************************************************************/
void IritExecOneLine(const char *Line)
{
    InptPrsrQueueInputLine(Line);
    if (strchr(Line, ';') == NULL)
        InptPrsrQueueInputLine(";");

    /* Evaluate this expression. */
    if (!InptPrsrInputParser(NULL))
        IritPrintInptPrsrError();		 /* Print the error message. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Modifies the state of the IRIT solid modeller.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:      Name of state variable.                                       M
*   Data:      New value of state variable.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Old value                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSetIritState                                                         M
*****************************************************************************/
IPObjectStruct *IritSetIritState(const char *Name, IPObjectStruct *Data)
{
    IPObjectStruct
	*OldVal = NULL;

    if (stricmp(Name, "BoolPerturb") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(BoolSetPerturbAmount(Data -> U.R));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    if (stricmp(Name, "BoolFreeform") == 0) {
        if (IP_IS_VEC_OBJ(Data)) {
	    OldVal = IPGenVECObject(&GlblBoolFreeformTols[0],
				    &GlblBoolFreeformTols[1],
				    &GlblBoolFreeformTols[2]);
	    IRIT_VEC_COPY(GlblBoolFreeformTols, Data -> U.Vec);
	    MdlBooleanSetTolerances(GlblBoolFreeformTols[0],
				    GlblBoolFreeformTols[1],
				    GlblBoolFreeformTols[2]);
	}
	else
	    IRIT_WNDW_PUT_STR("Vector state value expected");
    }
    else if (stricmp(Name, "CmpObjEps") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			  InptPrsrSetCmpObjEps(Data -> U.R));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "CnvxPl2Vrtcs") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(
			     GMConvexRaysToVertices(IRIT_REAL_TO_INT(Data -> U.R)));
	}
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "Coplanar") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			  BoolSetHandleCoplanarPoly(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "CursorKeep") == 0) {
        OldVal = IPGenNUMValObject(GlblKeepClientsCursorEvents);
        GlblKeepClientsCursorEvents = IRIT_REAL_TO_INT(Data -> U.R);
	if (GlblKeepClientsCursorEvents && GlblLastIritClientCursorEvent != NULL) {
	    IPFreeObject(GlblLastIritClientCursorEvent);
	    GlblLastIritClientCursorEvent = NULL;
	}
    }
    else if (stricmp(Name, "DebugFunc") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			     InptPrsrDebugFuncLevel(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "Dependency") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(GlblHandleDependencies);
	    GlblHandleDependencies = IRIT_REAL_TO_INT(Data -> U.R);
	}
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "DoGraphics") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(GlblDoGraphics);
	    GlblDoGraphics = Data -> U.R != 0.0;
	}
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "DumpLevel") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(SetDumpLevel(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "EchoSource") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
	      GlblQuietMode ? FALSE
                            : InptPrsrSetEchoSource(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "FlatLoad") == 0) {
        if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(GlblFlatLoadMode);
	    GlblFlatLoadMode = IRIT_REAL_TO_INT(Data -> U.R);
	}
    }
    else if (stricmp(Name, "FloatFrmt") == 0) {
	if (IP_IS_STR_OBJ(Data)) {
	    GlblFloatFormat = IritStrdup(Data -> U.Str);
	    OldVal = IPGenSTRObject(IPSetFloatFormat(GlblFloatFormat));
	}
	else
	    IRIT_WNDW_PUT_STR("String state value expected");
    }
    else if (stricmp(Name, "FastPolys") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(
			        CagdSrf2PolygonFast(IRIT_REAL_TO_INT(Data -> U.R)));
	}
	else
	    IRIT_WNDW_PUT_STR("String state value expected");
    }
    else if (stricmp(Name, "GMEpsilon") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			              GMBasicSetEps(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "InterCrv") == 0) {
        if (IP_IS_NUM_OBJ(Data)) {
	    MdlBoolSetOutputInterCrv(IRIT_REAL_TO_INT(Data -> U.R));
	    MdlBoolSetOutputInterCrvType(0);
	    OldVal = IPGenNUMValObject(
			    BoolSetOutputInterCurve(IRIT_REAL_TO_INT(Data -> U.R)));
	} 
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "InterUV") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			  BoolSetParamSurfaceUVVals(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "BspProdMethod") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			          BspMultComputationMethod(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "MvDmnReduce") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(MvarMVsZerosDomainReduction(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "MvGradPrecond") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(MvarMVsZerosGradPreconditioning(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "MvHPlnTst") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(MvarMVsZerosParallelHyperPlaneTest(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "MvNConeTst") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(MvarMVsZerosNormalConeTest(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "MvKantTst") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(MvarMVsZerosKantorovichTest(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "LoadFont") == 0) {
	if (IP_IS_STR_OBJ(Data))
	    OldVal = IPGenNUMValObject(GMLoadTextFont(Data -> U.Str));
	else
	    IRIT_WNDW_PUT_STR("String state value expected");
    }
    else if (stricmp(Name, "PolySort") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			        BoolSetPolySortAxis(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "PrimType") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(
		       PrimSetGeneratePrimType(IRIT_REAL_TO_INT(Data -> U.R)));
	}
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "PrimRatSrfs") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    OldVal = IPGenNUMValObject(
		    PrimSetSurfacePrimitiveRational(IRIT_REAL_TO_INT(Data -> U.R)));
	}
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "RandomInit") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    IritRandomInit(IRIT_REAL_TO_INT(Data -> U.R));
	OldVal = IPGenNUMValObject(Data -> U.R);
    }
    else if (stricmp(Name, "TrimCrvs") == 0) {
	if (IP_IS_NUM_OBJ(Data)) {
	    TrimSetEuclidComposedFromUV(Data -> U.R <= 0);
	    TrimSetTrimCrvLinearApprox((int) (IRIT_FABS(Data -> U.R)),
				       SYMB_CRV_APPROX_UNIFORM);
	}
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else if (stricmp(Name, "UVBoolean") == 0) {
	if (IP_IS_NUM_OBJ(Data))
	    OldVal = IPGenNUMValObject(
			  BoolSetParamSurfaceUVVals(IRIT_REAL_TO_INT(Data -> U.R)));
	else
	    IRIT_WNDW_PUT_STR("Numeric state value expected");
    }
    else {
        IRIT_WNDW_FPRINTF2("Error: undefined IritState \"%s\" ignored\n",
			   Name);
    }

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to query (and print) the errors found by InputParser.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPrintInptPrsrError                                                   M
*****************************************************************************/
void IritPrintInptPrsrError(void)
{
    InptPrsrEvalErrType ErrorNum;
    char *ErrorMsg, *p;
    char Line[INPUT_LINE_LEN + IRIT_LINE_LEN_LONG];

    if ((ErrorNum = InptPrsrParseError(&ErrorMsg)) != IPE_NO_ERR) {/*Prsr err*/
	sprintf(Line, "Parsing Error: ");
	p = &Line[strlen(Line)];
	switch (ErrorNum) {
	    case IR_ERR_WRONG_SYNTAX:
		sprintf(p, "Wrong syntax\n");
		break;
	    case IR_ERR_PARAM_EXPECT:
		sprintf(p, "Parameter Expected - %s\n", ErrorMsg);
		break;
	    case IR_ERR_ONE_OPERAND:
	    case IR_ERR_TWO_OPERAND:
		sprintf(p, "Wrong # of operands - %s\n", ErrorMsg);
		break;
	    case IR_ERR_STACK_OV:
		sprintf(p, "Internal Stack OverFlow at - %s\n", ErrorMsg);
		break;
	    case IR_ERR_PARAM_MATCH:
		sprintf(p, "Parenthesis mismatch - %s\n", ErrorMsg);
		break;
	    case IR_ERR_UNDEF_TOKEN:
		sprintf(p, "Undefined token - %s\n", ErrorMsg);
		break;
	    case IR_ERR_UNDEF_FUNC:
		sprintf(p, "Undefined function - %s\n", ErrorMsg);
		break;
	    case IR_ERR_PARAM_FUNC:
		sprintf(p, "Parameters expected in func %s\n", ErrorMsg);
		break;
	    case IR_ERR_NO_PARAM_FUNC:
		sprintf(p, "No Parameters expected in func %s\n", ErrorMsg);
		break;
	    case IR_ERR_STR_TOO_LONG:
		sprintf(p, "String too long - %s\n", ErrorMsg);
		break;
	    default:
		sprintf(p, "Undefined error %d", ErrorNum);
		break;
	}
	IRIT_WNDW_PUT_STR(Line);
	return;
    }

    if ((ErrorNum = InptPrsrEvalError(&ErrorMsg)) != IPE_NO_ERR) {/*Eval err.*/
	sprintf(Line, "Eval Error: ");
	p = &Line[strlen(Line)];
	switch (ErrorNum) {
	    case IE_ERR_FATAL_ERROR:
		sprintf(p, "Fatal error - %s\n", ErrorMsg);
		break;
	    case IE_ERR_DIV_BY_ZERO:
		sprintf(p, "Division by zero - %s\n", ErrorMsg);
		break;
	    case IE_ERR_NO_OBJ_METHOD:
		sprintf(p, "No such method for object - %s\n", ErrorMsg);
		break;
	    case IE_ERR_TYPE_MISMATCH:
		sprintf(p, "Parameter type mismatch - %s\n", ErrorMsg);
		break;
	    case IE_ERR_ASSIGN_LEFT_OP:
		sprintf(p, "Lval is not a parameter - %s\n", ErrorMsg);
		break;
	    case IE_ERR_MIXED_OBJ:
		sprintf(p, "Mixed types in expression - %s\n", ErrorMsg);
		break;
	    case IE_ERR_IP_OBJ_UNDEFINED:
		sprintf(p, "No such object defined - %s\n", ErrorMsg);
		break;
	    case IE_ERR_NO_ASSIGNMENT:
		sprintf(p, "Assignment was expected\n");
		break;
	    case IE_ERR_FP_ERROR:
		sprintf(p, "Floating Point Error - %s\n", ErrorMsg);
		break;
	    case IE_ERR_NUM_PRM_MISMATCH:
		sprintf(p, "Number of func. param. mismatch - %s\n", ErrorMsg);
		break;
	    case IE_ERR_MAT_POWER:
		sprintf(p, "Wrong range or not exists, operator - %s\n", ErrorMsg);
		break;
	    case IE_ERR_FREE_SIMPLE:
		sprintf(p, "Free only geometric objects - %s\n", ErrorMsg);
		break;
	    case IE_ERR_MODIF_ITER_VAR:
		sprintf(p, "Iteration var. type modified or freed - %s\n", ErrorMsg);
		break;
	    case IE_ERR_BOOLEAN_ERR:
		sprintf(p, "Geometric Boolean operation error - %s\n", ErrorMsg);
		break;
	    case IE_ERR_OUT_OF_RANGE:
		sprintf(p, "Out of range.\n");
		break;
	    case IE_ERR_DATA_PRSR_ERROR:
		sprintf(p, "%s", ErrorMsg);
		break;
	    case IE_ERR_USER_FUNC_NO_RETVAL:
		sprintf(p, "User defined function \"%s\" has no returned value\n",
			ErrorMsg);
		break;
	    case IE_ERR_INCOMPARABLE_TYPES:
		sprintf(p, "Incomparable object types found");
		break;
	    case IE_ERR_ONLYEQUALITY_TEST:
		sprintf(p, "Only equality or non equality test is valid for these objects");
		break;
	    case IE_ERR_IF_HAS_NO_COND:
		sprintf(p, "Condition of if clause is illegal");
		break;
	    case IE_ERR_IP_USERFUNC_DUP_VAR:
		sprintf(p, "Duplicated variable, %s", ErrorMsg);
		break;
	    case IE_ERR_IP_USERFUNC_TOO_MANY_PRMS:
		sprintf(p, "Too many parameters, %s", ErrorMsg);
		break;
	    case IE_ERR_UNDEF_INSTANCE:
		sprintf(p, "Undefined object reference in instance, %s",
			ErrorMsg);
		break;
	    default:
		sprintf(p, "Undefined error %d\n", ErrorNum);
		break;
	}
	IRIT_WNDW_PUT_STR(Line);
	return;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to handle reading one line from stdin into string Str, with        M
* maximum of length Length in the Input Window.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Str:        Where input is placed at.                                    M
*   Length:     Maximum length of input to allow.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritInputWindowGetStr                                                    M
*****************************************************************************/
void IritInputWindowGetStr(char *Str, int Length)
{
    int i;

    if (GlblRunScriptAndQuit)
        IritExit(0);

    IritGetLineStdin(Str, IRIT_PROMPT, Length - 1);

    i = (int) strlen(Str);
    if (i > 0 && Str[i - 1] < ' ') 
	Str[i - 1] = 0;					      /* No CR/LF. */

    puts(Str);
    fflush(stdout);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* IRIT Reset routine. Initialize again all object as in the starting state.  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritReset                                                                M
*****************************************************************************/
void IritReset(void)
{
    InptEvalFreeFunc(NULL);

    IritDBFreeAll();

    SetUpPredefObjects();		 /* Prepare the predefined objects. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints IRIT none fatal error message and go back to cursor mode.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   va_alist:    Format of error message to print out.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   IritNonFatalError                                                        M
*****************************************************************************/
#ifdef USE_VARARGS
void IritNonFatalError(const char *va_alist, ...)
{
    char *Format, *p;
    int i;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void IritNonFatalError(const char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    IRIT_WNDW_PUT_STR(p);

    va_end(ArgPtr);

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap error and go back to cursor mode.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   None		                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritQuietError                                                           M
*****************************************************************************/
void IritQuietError(void)
{
    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Bool_lib errors right here. Call back function of bool_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in bool_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDefaultBoolFatalError                                                M
*****************************************************************************/
void IritDefaultBoolFatalError(BoolFatalErrorType ErrID)
{
    const char
        *ErrorMsg = BoolDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("BOOL_LIB: %s", ErrorMsg);

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the given PObj to irit script (.irt) style and dump out.        M
* Mostly used by the GUI interface of irit.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:        To convert given PObj object to .irt style.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritConvertData2Irt                                                      M
*****************************************************************************/
void IritConvertData2Irt(IPObjectStruct *PObj)
{
    IPPrintFuncType
	OldPrintFunc = IPCnvSetPrintFunc(IritPutStr2);

    IPCnvDataToIrit(PObj);

    IPCnvSetPrintFunc(OldPrintFunc);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Called from the floating point package in case of fatal floating point     M
* error.								     M
*   Prints error message and long jumps to main loop.			     M
*   Default FPE handler - must be reset after redirected to other module.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Type:      Of floating point error.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDefaultFPEHandler                                                    M
*****************************************************************************/
void IritDefaultFPEHandler(int Type)
{
    IRIT_WNDW_FPRINTF2("Floating point error %d.", Type);

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to toggle the input window log file printing. If turned on, test   M
* is made if file has been opened and if not open it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   SetObj:    TRUE for log file active, FALSE inactive, or string name      M
*	       for a new logfile name.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritLogPrintSetup                                                        M
*****************************************************************************/
void IritLogPrintSetup(IPObjectStruct *SetObj)
{
    if (IP_IS_NUM_OBJ(SetObj)) {
	int Set = IRIT_REAL_TO_INT(SetObj -> U.R);

	if (Set) {
	    if (GlblLogFile == NULL) {	    /* Not open yet - open it now: */
		if ((GlblLogFile = fopen(GlblLogFileName, "w")) == NULL) {
		    IRIT_WNDW_FPRINTF2("Failed to open log file \"%s\"",
				       GlblLogFileName);
		    return;
		}
		GlblPrintLogFile = TRUE;
	    }
	}
	else {
	    GlblPrintLogFile = FALSE;
	    fflush(GlblLogFile);
	}
    }
    else if (IP_IS_STR_OBJ(SetObj)) {
	GlblPrintLogFile = FALSE;
	if (GlblLogFile != NULL) {
	    fclose(GlblLogFile);
	    GlblLogFile = NULL;
	}
	if (!IRT_STR_ZERO_LEN(GlblLogFileName))
	    IritFree(GlblLogFileName);
	GlblLogFileName = IritStrdup(SetObj -> U.Str);
    }
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Dummy function to link at debugging time.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   None	                                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritAllFatalErrorTrap                                                    M
*****************************************************************************/
void IritAllFatalErrorTrap(void)
{
    IRIT_WNDW_PUT_STR("Trapped fatal error from external library");
    fflush(stderr);
    fflush(stdout);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Dummy function to link at debugging time.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   None	                                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDummyLinkDebug                                                       M
*****************************************************************************/
void IritDummyLinkDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
