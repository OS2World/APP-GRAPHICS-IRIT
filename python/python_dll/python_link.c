/*****************************************************************************
*   Auxiliart python supprt for main module of "Irit"			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Usage:								     *
*   Irit [-t] [-g] [-i] [-z] [file[.irt]]				     *
*									     *
* Written by:  Gershon Elber				Ver 3.0, Apr. 1990   *
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
#include "inptprsl.h"
#include "objects.h"
#include "freeform.h"
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

#ifdef DJGCC
#include "intr_lib.h"
#include "intr_gr.h"
#endif /* DJGCC */
#ifdef __UNIX__
#if defined (__FreeBSD__)
#include <term.h>
IRIT_STATIC_DATA struct termios GlblOrigTermio;
#else
#include <termio.h>
IRIT_STATIC_DATA struct termio GlblOrigTermio;
#endif /* !__FreeBSD__ */
#endif /* __UNIX__ */

#include "python_link.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA char
    *VersionStr = "Irit		Version 10.0, Gershon Elber,\n\
	(C) Copyright 1989-2009 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA char
    *VersionStr = "Irit	        " IRIT_VERSION
	",	Gershon Elber,	" __DATE__ ",   " __TIME__ "\n"
	IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA char
     *CtrlStr = "Irit [-t] [-g] [-q] [-i] [-z] [file[.irt]]";

IRIT_STATIC_DATA char
    *GlblPrgmHeader = "                Irit - the not only polygonal solid modeller";

IRIT_STATIC_DATA char
    *GlblCopyRight  =
#ifdef OS2GCC
     IRIT_COPYRIGHT ", OS2 " IRIT_VERSION ",   " __DATE__;
#elif AMIGA
     IRIT_COPYRIGHT ", Amiga " IRIT_VERSION ",   " __DATE__;
#elif __WINNT__
     IRIT_COPYRIGHT ", Windows " IRIT_VERSION ", " __DATE__;
#elif __MACOSX__
     IRIT_COPYRIGHT ", Mac " IRIT_VERSION ", " __DATE__;
#elif __FreeBSD__
     IRIT_COPYRIGHT ", FreeBSD " IRIT_VERSION ", " __DATE__;
#elif defined(sun) || defined(SUN4)
     IRIT_COPYRIGHT ", SUN4 " IRIT_VERSION ", " __DATE__;
#elif sgi
     IRIT_COPYRIGHT ", SGI " IRIT_VERSION ", " __DATE__;
#elif LINUX386
     IRIT_COPYRIGHT ", Linux " IRIT_VERSION ", " __DATE__;
#elif __CYGWIN__
     IRIT_COPYRIGHT ", Cygwin " IRIT_VERSION ", " __DATE__;
#elif __UNIX__
     IRIT_COPYRIGHT ", Unix " IRIT_VERSION ", " __DATE__;
#elif NO_CONCAT_STR
     "(C) Copyright 1989-2009 Gershon Elber,  Unix  Version 10";
#else
     IRIT_COPYRIGHT ", Unknown " IRIT_VERSION ", " __DATE__;
#endif

IRIT_STATIC_DATA char
    *GlblAuthorName = "Written by Gershon Elber, python port made with the support of Hutchinson/Total";

IRIT_STATIC_DATA int
    GlblKeepClientsCursorEvents = FALSE,
    GlblQuietMode = FALSE;

IRIT_STATIC_DATA char GlblCrntWorkingDir[IRIT_LINE_LEN];/* Init CWD to recover on exit.*/

IRIT_STATIC_DATA IPObjectStruct
    *GlblLastClientCursorEvent = NULL;        /* Cursor events from clients. */

IRIT_GLOBAL_DATA IPObjectStruct
    *GlblObjList = NULL;		   /* All objects defined on system. */

IRIT_GLOBAL_DATA jmp_buf GlblLongJumpBuffer;		  /* Used in error recovery. */

IRIT_GLOBAL_DATA int
#ifdef DJGCC					   /* Defaults for intr_lib. */
    GlblWindowFrameWidth = 8,
    GlblViewFrameColor   = INTR_COLOR_RED,
    GlblViewBackColor    = INTR_COLOR_BLACK,
    GlblTransFrameColor  = INTR_COLOR_GREEN,
    GlblTransBackColor   = INTR_COLOR_BLACK,
    GlblStatusFrameColor = INTR_COLOR_MAGENTA,
    GlblStatusBackColor  = INTR_COLOR_BLACK,
    GlblInputFrameColor  = INTR_COLOR_YELLOW,
    GlblInputBackColor   = INTR_COLOR_BLACK,
    GlblDrawHeader       = FALSE,
    GlblSmoothTextScroll = TRUE,
    GlblIntrSaveMethod   = INTR_SAVE_DISK,
    GlblMouseSensitivity = 10,       /* Sensitivity control of mouse device. */
    GlblJoystickExists   = FALSE,
#endif /* DJGCC */
    GlblInterpProd	 = TRUE;    

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
  { "DoGraphics",	"-t", (VoidPtr) &GlblDoGraphics,     IC_BOOLEAN_TYPE },
  { "BspProdMethod",	"",   (VoidPtr) &GlblBspProdMethod,  IC_INTEGER_TYPE },
  { "PointLength",	"",   (VoidPtr) &GlblPointLenAux,    IC_INTEGER_TYPE },
  { "LineEdit",         "",   (VoidPtr) &GlblLineEditControl,IC_STRING_TYPE },
  { "StartFile",	"",   (VoidPtr) &GlblStartFileName,  IC_STRING_TYPE },
  { "LogFile",		"",   (VoidPtr) &GlblLogFileName,    IC_STRING_TYPE },
  { "FloatFrmt",	"",   (VoidPtr) &GlblFloatFormat,    IC_STRING_TYPE },
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

static void ConvertData2Irt(IPObjectStruct *PObj);
static void IritBoolFatalError(BoolFatalErrorType ErrID);
static void PrintInptPrsrError(void);

#ifdef DEBUG
static void AllFatalErrorTrap(void);
#endif /* DEBUG */

extern IRIT_GLOBAL_DATA InptPrsrEvalErrType IPGlblEvalError;
extern IRIT_GLOBAL_DATA char IPGlblCharData[];

/*****************************************************************************
* DESCRIPTION:                                                               M
* IRIT Exit routine. Error code of zero.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritExit0                                                                M
*****************************************************************************/
void IritExit0(void)
{
    IritExit(0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* IRIT Exit routine.                           				     M
*                                                                            *
* PARAMETERS:                                                                M
*   ExitCode:   Exit code.                                                   *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritExit                                                                 M
*****************************************************************************/
void IritExit(int ExitCode)
{
    IrtRType
	R = IP_CLNT_BROADCAST_ALL_HANDLES,
        R2 = TRUE;

    if (GlblDoGraphics) {
    	IritDispViewExit();
    }

    IritClientClose(&R, &R2);

    chdir(GlblCrntWorkingDir);	  /* Recover original directory before exit. */

    if (GlblPrintLogFile)
	fclose(GlblLogFile);		      /* Close log file if was open. */

#   ifdef __UNIX__    
#if defined (__FreeBSD__)
	tcsetattr(0, TCSANOW, &GlblOrigTermio);
#else
	ioctl(0, TCSETA, &GlblOrigTermio);
#endif /* !__FreeBSD__ */
#   endif /* __UNIX__ */

#   ifdef __WINNT__
	_exit(ExitCode);
#   else
	exit(ExitCode);
#   endif /* __WINNT__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Geom_lib errors right here. Call back function of geom_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in geom_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   GeomFatalError                                                           M
*****************************************************************************/
void GeomFatalError(GeomFatalErrorType ErrID)
{
    const char
        *ErrorMsg = GeomDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("GEOM_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Misc_lib errors right here. Call back function of misc_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in misc_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   MiscFatalError                                                           M
*****************************************************************************/
void MiscFatalError(MiscFatalErrorType ErrID, char *ErrDesc)
{    
    const char
        *ErrorMsg = MiscDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("MISC_LIB: %s%s", ErrorMsg, ErrDesc);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Traps Bool_lib errors right here. Call back function of bool_lib.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   ErrID:    Error number in bool_lib library.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IritBoolFatalError(BoolFatalErrorType ErrID)
{
    const char
        *ErrorMsg = BoolDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("BOOL_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Cagd_lib errors right here. Call back function of cagd_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in cagd_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdFatalError                                                           M
*****************************************************************************/
void CagdFatalError(CagdFatalErrorType ErrID)
{
    const char
        *ErrorMsg = CagdDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("CAGD_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Symb_lib errors right here. Call back function of symb_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in symb_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   SymbFatalError                                                           M
*****************************************************************************/
void SymbFatalError(SymbFatalErrorType ErrID)
{
    const char
	*ErrorMsg = SymbDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("SYMB_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Trim_lib errors right here. Call back function of trim_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in trim_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   TrimFatalError                                                           M
*****************************************************************************/
void TrimFatalError(TrimFatalErrorType ErrID)
{
    const char
	*ErrorMsg = TrimDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("TRIM_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Triv_lib errors right here. Call back function of triv_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in triv_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   TrivFatalError                                                           M
*****************************************************************************/
void TrivFatalError(TrivFatalErrorType ErrID)
{
    const char
	*ErrorMsg = TrivDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("TRIV_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Trng_lib errors right here. Call back function of trng_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in trng_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   TrngFatalError                                                           M
*****************************************************************************/
void TrngFatalError(TrngFatalErrorType ErrID)
{
    const char
	*ErrorMsg = TrngDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("TRNG_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps mvar_lib errors right here. Call back function of mvar_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in mvar_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   MvarFatalError                                                           M
*****************************************************************************/
void MvarFatalError(MvarFatalErrorType ErrID)
{
    const char
	*ErrorMsg = MvarDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("MVAR_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps user_lib errors right here. Call back function of user_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in user_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   UserFatalError                                                           M
*****************************************************************************/
void UserFatalError(UserFatalErrorType ErrID)
{
    const char
	*ErrorMsg = UserDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("USER_LIB: %s", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prints IRIT' warnings to stdout.                       		     M
*   This function just prints the given error message.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message to print.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritWarningError, error trap                                             M
*****************************************************************************/
void IritWarningError(char *Msg)
{
    if (Msg != NULL) {
	IRIT_WNDW_FPRINTF2("Irit Warning: %s", Msg);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints IRIT's fatal error message and go back to cursor mode.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorMsg:    Error message to print out.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   IritFatalError, error trap                                               M
*****************************************************************************/
void IritFatalError(const char *ErrorMsg)
{
    if (ErrorMsg != NULL) {
	IRIT_WNDW_PUT_STR("Fatal error occured, please report it:");
	IRIT_WNDW_PUT_STR(ErrorMsg);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Idle function that is invoked when no input is waiting from stdin/file.  M
* typically sleeps and/or checks display device's io.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MiliSeconds:  Period to sleep if nothing to do.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritIdleFunction                                                         M
*****************************************************************************/
void IritIdleFunction(int MiliSeconds)
{
#ifdef DUMP_COUNTER
    IRIT_STATIC_DATA int
	i = 0;

    fprintf(stderr, IRIT_EXP_STR("IritIdle: %d\r"), i++);
#endif /* DUMP_COUNTER */

    if (!IPSocSrvrListen() && MiliSeconds > 0)
        IritSleep(MiliSeconds);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Call back function of the server listening to clients.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:    Client handler from which an event has been recieved.        M
*   PObj:       NULL if a new client has connected, object recieved from     M
*		Client otherwise.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritIdleFunction                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSocHandleClientEvent                                                   M
*****************************************************************************/
void IPSocHandleClientEvent(int Handler, IPObjectStruct *PObj)
{
    const char *p;

#ifdef DEBUG
    int i;

    if (PObj != NULL) {
        p = IP_GET_OBJ_NAME(PObj);
	for (i = 0; i < (int) strlen(p); i++)
	    if (islower(p[i]))
	        IPFatalError(IP_ERR_LOCASE_OBJNAME);
    }
#endif /* DEBUG */

    if (PObj == NULL) {
	if (GlblGUIMode)
	    IRIT_WNDW_FPRINTF2("<<IPC>> New client - handler %d <<CPI>>\n",
			       Handler);

	/* If no client connected so far, make sure we validate this one. */
	if (GlblCurrentDisplay < 0)
	    GlblCurrentDisplay = IP_CLNT_BROADCAST_ALL_HANDLES;
    }
    else if (GlblKeepClientsCursorEvents &&
	     strcmp(IP_GET_OBJ_NAME(PObj), "_PICKCRSR_") == 0) {
        /* Keep this object as the last cursor event. */
        if (GlblLastIritClientCursorEvent == NULL)
	    GlblLastIritClientCursorEvent = PObj;
	else
	    IPFreeObject(PObj);
    }
    else if ((strcmp(IP_GET_OBJ_NAME(PObj), "_SUBMITOBJECT_") == 0 ||
	      strcmp(IP_GET_OBJ_NAME(PObj), "_SUBMITMAT_") == 0) &&
	     (p = AttrGetObjectStrAttrib(PObj, "ObjName")) != NULL) {
        IP_SET_OBJ_NAME2(PObj, p);
	IritStrUpper(PObj -> ObjName);

	if (GlblGUIMode) {
	    IRIT_WNDW_FPRINTF3("<<IPC>> Client %d Start Submission \"%s\" <<CPI>>\n",
			       Handler, p);

	    IritConvertData2Irt(PObj);
	    IRIT_WNDW_FPRINTF3("<<IPC>> Client %d End Submission \"%s\" <<CPI>>\n",
			       Handler, p);
	}
	else {
	    IRIT_WNDW_FPRINTF3("<<IPC>> Accepted submitted object \"%s\" from handler %d <<CPI>>\n",
			       p, Handler);
	}
	AttrFreeObjectAttribute(PObj, "ObjName");
	IritDBInsertObjectLast(PObj, TRUE);
    }
    else {
        if (strcmp(IP_GET_OBJ_NAME(PObj), "_PICKFAIL_") == 0) {
	    IRIT_WNDW_FPRINTF2("<<IPC>> Client %d failed to pick <<CPI>>\n",
			       Handler);
	}
	else if (strcmp(IP_GET_OBJ_NAME(PObj), "_PICKNAME_") == 0) {
	    IRIT_WNDW_FPRINTF3("<<IPC>> Client %d picked object name \"%s\" <<CPI>>\n",
			       Handler, PObj -> U.Str);
	}
	else {
	    /* It is complete geometry. */
	    if (GlblGUIMode) {
	        IRIT_WNDW_FPRINTF3("<<IPC>> Client %d Start Submission \"%s\" <<CPI>>\n",
				   Handler, IP_GET_OBJ_NAME(PObj));
		IritConvertData2Irt(PObj);
		IRIT_WNDW_FPRINTF3("<<IPC>> Client %d End Submission \"%s\" <<CPI>>\n",
				   Handler, IP_GET_OBJ_NAME(PObj));
	    }
	    else
	        IRIT_WNDW_FPRINTF3("<<IPC>> Client %d picked complete geometry of \"%s\" <<CPI>>\n",
				   Handler, IP_GET_OBJ_NAME(PObj));
	}
	IPFreeObject(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the given PObj to irit script (.irt) style and dump out.        *
* Mostly used by the GUI interface of irit.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        To convert given PObj object to .irt style.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ConvertData2Irt(IPObjectStruct *PObj)
{
    IPPrintFuncType
	OldPrintFunc = IPCnvSetPrintFunc(IritPutStr2);

    IPCnvDataToIrit(PObj);

    IPCnvSetPrintFunc(OldPrintFunc);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get next cursor event from one of the clients, and wait at most WaitTime M
* milliseconds.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   RWaitTime:  Maximal time to wait for the cursor event, in miliseconds.   M
*		If zero, waits indefinitly.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The picked cursor event of a list object with one     M
*		string "_PickFail_" object of nothing occured.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPickCrsrClientEvent                                                  M
*****************************************************************************/
IPObjectStruct *IritPickCrsrClientEvent(IrtRType *RWaitTime)
{
    int OrigWaitTime = IRIT_REAL_PTR_TO_INT(RWaitTime),
	WaitTime = OrigWaitTime;
    IPObjectStruct *RetVal;

    while (WaitTime >= 0) {
        if (GlblLastClientCursorEvent != NULL) {
	    RetVal = GlblLastClientCursorEvent;
	    GlblLastClientCursorEvent = NULL;	
	    return RetVal;
	}

	if (OrigWaitTime > 0)
	    WaitTime -= 10;

	IritIdleFunction(10);
    }

    /* Return empty list object instead. */
    RetVal = IPGenListObject("_PickFail_", NULL, NULL);

    return RetVal;
}

#ifndef IRIT_QUIET_STRINGS

/*****************************************************************************
* DESCRIPTION:                                                               M
* Prints IRIT parser's fatal error message and go back to cursor mode.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorMsg:    Error message to print out.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   IPFatalError                                                             M
*****************************************************************************/
void IPFatalError(IPFatalErrorType ErrID)
{
    IRIT_WNDW_FPRINTF2("PRSR_LIB: %s", IPDescribeError(ErrID));
}

#endif /* !IRIT_QUIET_STRINGS */

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
*   DefaultFPEHandler                                                        M
*****************************************************************************/
void DefaultFPEHandler(int Type)
{
    IRIT_WNDW_FPRINTF2("Floating point error %d.", Type);
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes the interpreter.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritInit                                                                 M
*****************************************************************************/
void IritInit()
{
    IritCPUTime(TRUE);					 /* Reset the clock. */

    IritConfig("irit", SetUp, NUM_SET_UP);   /* Read config. file if exists. */

    getcwd(GlblCrntWorkingDir, IRIT_LINE_LEN - 1);

    BspMultComputationMethod(GlblInterpProd);
    IPSetFloatFormat(GlblFloatFormat);
    IPSetFlattenObjects(FALSE);
    IPSetPolyListCirc(TRUE);

    IRIT_WNDW_PUT_STR(GlblPrgmHeader);
    IRIT_WNDW_PUT_STR(GlblCopyRight);
    IRIT_WNDW_PUT_STR(GlblAuthorName);
   
    SetUpPredefObjects();		  /* Prepare the predefined objects. */

    BoolSetHandleCoplanarPoly(TRUE);		/* Handle coplanar polygons. */

    IPSocSrvrInit();            /* Initialize the listen socket for clients. */
}

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
*   ExecOneLine                                                              M
*****************************************************************************/
void ExecOneLine(char *Line)
{
    InptPrsrQueueInputLine(Line);
    if (strchr(Line, ';') == NULL)
        InptPrsrQueueInputLine(";");

    /* Evaluate this expression. */
    if (!InptPrsrInputParser(NULL))
        PrintInptPrsrError();			 /* Print the error message. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to query (and print) the errors found by InputParser.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintInptPrsrError(void)
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
*   Create a new IRIT control point object, given its data.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   PtType:        new type of point					     M
*   NumOfParams:   Size of Params vector.				     M
*   Params:        Coefficients of new control point.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     The new control point.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateCtlPt                                                              M
*****************************************************************************/
IPObjectStruct *CreateCtlPt(int PtType, int NumOfParams, IrtRType Params[])
{
    int i, NumPts, 
	CoordCount = 0;

    IPObjectStruct *PObj;
    
    PObj = IPAllocObject("", IP_OBJ_CTLPT, NULL);

    for (i = 0; i < NumOfParams; i++) {	
	if (i == 0) {	
            PObj -> U.CtlPt.PtType = PtType;	        

	    switch (PtType) {
		case CAGD_PT_E1_TYPE:
		case CAGD_PT_E2_TYPE:
		case CAGD_PT_E3_TYPE:
		case CAGD_PT_E4_TYPE:
		case CAGD_PT_E5_TYPE:
		case CAGD_PT_E6_TYPE:
		case CAGD_PT_E7_TYPE:
		case CAGD_PT_E8_TYPE:
		case CAGD_PT_E9_TYPE:
		    NumPts = CAGD_NUM_OF_PT_COORD(PtType);
		    CoordCount = 1;
		    break;
		case CAGD_PT_P1_TYPE:
		case CAGD_PT_P2_TYPE:
		case CAGD_PT_P3_TYPE:
		case CAGD_PT_P4_TYPE:
		case CAGD_PT_P5_TYPE:
		case CAGD_PT_P6_TYPE:
		case CAGD_PT_P7_TYPE:
		case CAGD_PT_P8_TYPE:
		case CAGD_PT_P9_TYPE:
		    NumPts = CAGD_NUM_OF_PT_COORD(PtType) + 1;
		    CoordCount = 0;
		    break;
		default:
		    IPGlblEvalError = IE_ERR_TYPE_MISMATCH;
		    strcpy(IPGlblCharData,
			   IRIT_EXP_STR("E{1-9} or P{1-9} point type expected"));
		    IPFreeObject(PObj);
		    return NULL;
	    }
	    if (NumOfParams - 1!= NumPts) {
		IPGlblEvalError = IE_ERR_NUM_PRM_MISMATCH;
		sprintf(IPGlblCharData,
			IRIT_EXP_STR("%d expected in function \"ctlpt\""),
			NumPts);
		IPFreeObject(PObj);
		return NULL;
	    }
	}
        else
	    PObj -> U.CtlPt.Coords[CoordCount++] = Params[i];
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Function to query and print the list of available IRIT functions.        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritQueryFunctions                                                       M
*****************************************************************************/
void IritQueryFunctions(void)
{
    int i, l;
    UserDefinedFuncDefType *UserFunc;

    for (i = 0; i < NumFuncTableSize; i++) {
	NumFuncTableType
	    *NumEntry = &NumFuncTable[i];

	printf("%sDEFFUNC 00002 %-10s %3d [", GUI_PROMPT,
	    NumEntry -> FuncName, NumEntry -> NumOfParam);
	if (NumEntry -> NumOfParam != IP_ANY_PARAM_NUM) {
	    for (l = 0; l < NumEntry -> NumOfParam; l++)
		printf("%s%05x",
		l == 0 ? "" : " ", NumEntry -> ParamObjType[l]);
	}
	printf("]\n");
    }

    for (i = 0; i < ObjFuncTableSize; i++) {
	ObjFuncTableType
	    *ObjEntry = &ObjFuncTable[i];

	printf("%sDEFFUNC %05x %-10s %3d [", GUI_PROMPT, ObjEntry -> RetType,
	    ObjEntry -> FuncName, ObjEntry -> NumOfParam);
	if (ObjEntry -> NumOfParam != IP_ANY_PARAM_NUM) {
	    for (l = 0; l < ObjEntry -> NumOfParam; l++)
		printf("%s%05x",
		l == 0 ? "" : " ", ObjEntry -> ParamObjType[l]);
	}
	printf("]\n");
    }

    for (i = 0; i < GenFuncTableSize; i++) {
	GenFuncTableType
	    *GenEntry = &GenFuncTable[i];

	printf("%sDEFFUNC 00000 %-10s %3d [", GUI_PROMPT,
	    GenEntry -> FuncName, GenEntry -> NumOfParam);
	if (GenEntry -> NumOfParam != IP_ANY_PARAM_NUM) {
	    for (l = 0; l < GenEntry -> NumOfParam; l++)
		printf("%s%05x",
		l == 0 ? "" : " ", GenEntry -> ParamObjType[l]);
	}
	printf("]\n");
    }

    for (UserFunc = UserDefinedFuncList;
	UserFunc != NULL;
	UserFunc = UserFunc -> Pnext) {
	    int n = IPObjListLen(UserFunc -> Params);

	    printf("%sDEFUFUNC %05x %-10s %3d [", GUI_PROMPT,
		UserFunc -> IsFunction ? ANY_EXPR : 0,
		UserFunc -> FuncName, n);
	    for (l = 0; l < n; l++)
		printf("%s%05x", l == 0 ? "" : " ", ANY_EXPR);
	    printf("]\n");
    }

    for (i = 0; i < ConstantTableSize; i++) {
	ConstantTableType
	    *GenEntry = &ConstantTable[i];

	printf("%sDEFCONST %-10s %-17.15f\n", GUI_PROMPT,
	    GenEntry -> FuncName, GenEntry -> Value);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Function that is called to place one line out.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Line:  To print out.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPrintOneString                                                       M
*****************************************************************************/
void IritPrintOneString(const char *Line)
{
    fputs(Line, stdout);
    fflush(stdout);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the IRIT global resolution value                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:    The resolution.	                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetResolution                                                      M
*****************************************************************************/
double IritPyGetResolution(void)
{
    IPObjectStruct
	*PObj = IritDBGetObjByName("RESOLUTION");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name RESOLUTION is defined");
	return DEFAULT_RESOLUTION;
    }
    else
        return (double) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the IRIT global resolution.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Res: New resolution to set.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:    The resolution.	                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetResolution                                                      M
*****************************************************************************/
double IritPySetResolution(double Res)
{
    IPObjectStruct
	*PObj = IritDBGetObjByName("RESOLUTION");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name RESOLUTION is defined");
	return DEFAULT_RESOLUTION;
    }
    else
        return (double) (PObj -> U.R = Res);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the IRIT global viewing matrix.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The sought matrix.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetViewMatrix                                                      M
*****************************************************************************/
IPObjectStruct *IritPyGetViewMatrix(void)
{
    int WasViewMat;

    return IPGenMATObject(*IPGetViewMat(&WasViewMat));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the IRIT global viewing matrix.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   NewMat:  New matrix to update                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The sought matrix.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetViewMatrix                                                      M
*****************************************************************************/
IPObjectStruct *IritPySetViewMatrix(IPObjectStruct *NewMat)
{
    int WasViewMat;

    IRIT_HMGN_MAT_COPY(IPGetViewMat(&WasViewMat), NewMat -> U.Mat);

    return IPGenMATObject(*NewMat -> U.Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the IRIT global PRSP matrix.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The sought matrix.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetPrspMatrix                                                      M
*****************************************************************************/
IPObjectStruct *IritPyGetPrspMatrix(void)
{
    int WasPrspMat;

    return IPGenMATObject(*IPGetPrspMat(&WasPrspMat));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the IRIT global PRSP matrix.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   NewMat:  New matrix to update                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The sought matrix.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetPrspMatrix                                                      M
*****************************************************************************/
IPObjectStruct *IritPySetPrspMatrix(IPObjectStruct *NewMat)
{
    int WasPrspMat;

    IRIT_HMGN_MAT_COPY(IPGetPrspMat(&WasPrspMat), NewMat -> U.Mat);

    return IPGenMATObject(*NewMat -> U.Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Presenting the object in Stdout .                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  Pointer to the object                                             M
*                                                                            M
* RETURN VALUE:                                                              M
*   None                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyStdoutObject                                                       M
*****************************************************************************/
void IritPyStdoutObject(const IPObjectStruct *PObj)
{
    printf("Object at %08x:\n", PObj);
    IPStdoutObject(PObj, 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Presenting the object in Stderr .                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  Pointer to the object                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   None                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyStderrObject                                                       M
*****************************************************************************/
void IritPyStderrObject(const IPObjectStruct *PObj)
{
    fprintf(stderr, "Object at %08x:\n", PObj);
    IPStderrObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generate string object.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   string: the string                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct*                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGenStrObject                                                       M
*****************************************************************************/
IPObjectStruct* IritPyGenStrObject( char* string )
{
    return( IPGenSTRObject(string) );
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generate Real object.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   num: the real number                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct*                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGenRealObject                                                      M
*****************************************************************************/
IPObjectStruct* IritPyGenRealObject( double num )
{
    return( IPGenNUMValObject(num) );
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generate Int object.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   num: the integer number                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct*                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGenIntObject                                                       M
*****************************************************************************/
IPObjectStruct* IritPyGenIntObject( int num )
{
    return( IPGenNUMValObject(num) );
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetch string from string object.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   obj: the string object                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   char*                                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyFetchStrObject                                                  M
*****************************************************************************/
char* IritPyFetchStrObject( IPObjectStruct* obj )
{
    return( obj -> U.Str );
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetch real (double) from real object.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   obj: the real object                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   double                                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyFetchRealObject                                                    M
*****************************************************************************/
double IritPyFetchRealObject( IPObjectStruct* obj )
{
    return( obj -> U.R );
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetch int from int object.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   obj: the int object                                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyFetchIntObject                                                     M
*****************************************************************************/
int IritPyFetchIntObject( IPObjectStruct* obj )
{
    return( (int)(obj -> U.R) );
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the DRAWCTLPT global variable                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    The DRAWCTLPT.                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetDrawCtlpt                                                       M
*****************************************************************************/
int IritPyGetDrawCtlpt(void)
{
     const IPObjectStruct
        *PObj = IritDBGetObjByName("DRAWCTLPT");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named DRAWCTLPT is defined");
	return -1;
    }
    else
        return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the DRAWCTLPT global variable                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   int: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetDrawCtlpt                                                       M
*****************************************************************************/
int IritPySetDrawCtlpt(int val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("DRAWCTLPT");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named DRAWCTLPT is defined");
	return -1;
    }
    else
	return (int) (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the FLAT4PLY global variable                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetFlat4Ply                                                        M
*****************************************************************************/
int IritPyGetFlat4Ply(void)
{
     const IPObjectStruct
        *PObj = IritDBGetObjByName("FLAT4PLY");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named FLAT4PLY is defined");
	return -1;
    }
    else
	return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the FLAT4PLY global variable                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   int: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetFlat4Ply                                                        M
*****************************************************************************/
int IritPySetFlat4Ply(int val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("FLAT4PLY");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named FLAT4PLY is defined");
	return -1;
    }
    else
	return (int) (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the POLYAPPROXOPT global variable                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetPolyApproxOpt                                                   M
*****************************************************************************/
int IritPyGetPolyApproxOpt(void)
{
     const IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_OPT");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_OPT is defined");
	return -1;
    }
    else
	return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the POLYAPPROXOPT global variable                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   int: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetPolyApproxOpt                                                   M
*****************************************************************************/
int IritPySetPolyApproxOpt(int val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_OPT");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_OPT is defined");
	return -1;
    }
    else
	return (int) (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the POLYAPPROXUV global variable                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetPolyApproxUV                                                    M
*****************************************************************************/
int IritPyGetPolyApproxUV(void)
{
     const IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_UV");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_UV is defined");
	return -1;
    }
    else
	return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the POLYAPPROXUV global variable                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   int: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetPolyApproxUV                                                    M
*****************************************************************************/
int IritPySetPolyApproxUV(int val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_UV");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_UV is defined");
	return -1;
    }
    else
	return (int) (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the POLYAPPROXTRI global variable                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetPolyApproxTri                                                   M
*****************************************************************************/
int IritPyGetPolyApproxTri(void)
{
     const IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_TRI");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_TRI is defined");
	return -1;
    }
    else
	return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the POLYAPPROXTRI global variable                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   int: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetPolyApproxTri                                                   M
*****************************************************************************/
int IritPySetPolyApproxTri(int val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_TRI");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_TRI is defined");
	return -1;
    }
    else
	return (int) (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the POLYAPPROXTOL global variable                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   double                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetPolyApproxTol                                                   M
*****************************************************************************/
double IritPyGetPolyApproxTol(void)
{
    const IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_TOL");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_TOL is defined");
	return -1.0;
    }
    else
	return PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the POLYAPPROXTOL global variable                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   double: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   double                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetPolyApproxTol                                                   M
*****************************************************************************/
double IritPySetPolyApproxTol(double val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_APPROX_TOL");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_APPROX_TOL is defined");
	return -1.0;
    }
    else
	return (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the POLYMERGECOPLANAR global variable                               M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetPolyMergeCoplanar                                               M
*****************************************************************************/
int IritPyGetPolyMergeCoplanar(void)
{
    const IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_MERGE_COPLANAR");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_MERGE_COPLANAR is defined");
	return -1;
    }
    else
	return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the POLYMERGECOPLANAR global variable                               M
*                                                                            *
* PARAMETERS:                                                                M
*   int: the new value                                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetPolyMergeCoplanar                                               M
*****************************************************************************/
int IritPySetPolyMergeCoplanar(int val)
{
    IPObjectStruct
        *PObj = IritDBGetObjByName("POLY_MERGE_COPLANAR");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named POLY_MERGE_COPLANAR is defined");
	return -1;
    }
    else
	return (int) (PObj -> U.R = val);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the MACHINE global variable                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int                                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetMachine                                                         M
*****************************************************************************/
int IritPyGetMachine(void)
{
    const IPObjectStruct
        *PObj = IritDBGetObjByName("MACHINE");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object named MACHINE is defined");
	return -1;
    }
    else
	return (int) PObj -> U.R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the IRIT global Axes                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    The sought axes.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetAxes                                                            M
*****************************************************************************/
IPObjectStruct *IritPyGetAxes(void)
{
    const IPObjectStruct
        *PObj = IritDBGetObjByName("AXES");

    if (PObj == NULL || !IP_IS_POLY_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No poly object named AXES is defined");
	return NULL;
    }
    else
        return IPCopyObject(NULL, PObj, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the USRFNLIST global variable                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct*                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetUsrFnList                                                       M
*****************************************************************************/
IPObjectStruct *IritPyGetUsrFnList(void)
{
    const IPObjectStruct
        *PObj = IritDBGetObjByName("USR_FN_LIST");

    if (PObj == NULL) {
	IRIT_NON_FATAL_ERROR("No object named USR_FN_LIST is defined");
	return NULL;
    }
    else
        return IPCopyObject(NULL, PObj, FALSE);;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   sets the name of an IRIT object.  Use with caution!                      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Object to change its name.                                       M
*   Name:   New name to use.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetObjectName                                                      M
*****************************************************************************/
void IritPySetObjectName(IPObjectStruct *PObj, const char *Name)
{
    assert(PObj != NULL);

    IP_SET_OBJ_NAME2(PObj, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a NULL IPObjectStruct.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A NULL pointer.                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetObjectName                                                      M
*****************************************************************************/
IPObjectStruct *IritPyGenNullObject()
{
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a NULL IPObjectStruct.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  The object to return its type.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if NULL object, FALSE otherwise.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPySetObjectName                                                      M
*****************************************************************************/
int IritPyIsNullObject(IPObjectStruct *PObj)
{
    return PObj == NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the type of given object PObj.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:  The object to return its type.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  The type.                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyThisObject                                                         M
*****************************************************************************/
int IritPyThisObject(IPObjectStruct *PObj)
{
    if (PObj == NULL)
        return IP_OBJ_UNDEF;
    else
        return PObj -> ObjType;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets the mesh size of the given freeform in specified dir.               M
*                                                                            *
* PARAMETERS:                                                                M
*   FFObj:      Freefrom object to get its mesh size.			     M
*   Dir:        Direction need of mesh size (ROw/COl).			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    The size.	     		                           	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPyGetMeshSize                                                        M
*****************************************************************************/
int IritPyGetMeshSize(IPObjectStruct *FFObj, int Dir)
{
    IrtRType
	R = Dir;

    return (int) GetMeshSize(FFObj, &R);
}
