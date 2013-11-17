/*****************************************************************************
*   Main module of "Irit" - the 3d (not only polygonal) solid modeller.      *
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
#include "iritprsr.h"
#include "inptprsg.h"
#include "inptprsl.h"
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

#ifdef __UNIX__
#include <unistd.h>
#if defined (__FreeBSD__) || defined(__MACOSX__)
#include <term.h>
IRIT_STATIC_DATA struct termios GlblOrigTermio;
#else
#include <termio.h>
IRIT_STATIC_DATA struct termio GlblOrigTermio;
#endif /* __FreeBSD__  || __MACOSX__ */
#endif /* __UNIX__ */

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit		Version 11, Gershon Elber,\n\
	(C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit	        " IRIT_VERSION
	",	Gershon Elber,	" __DATE__ ",   " __TIME__ "\n"
	IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
     *CtrlStr = "Irit [-t] [-s] [-g] [-q] [-i] [-z] [file[.irt]]";

IRIT_STATIC_DATA const char
    *GlblPrgmHeader = "                Irit - the not only polygonal solid modeller";

IRIT_STATIC_DATA const char
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
     "(C) Copyright 1989-2012 Gershon Elber,  Unix  Version 10";
#else
     IRIT_COPYRIGHT ", Unknown " IRIT_VERSION ", " __DATE__;
#endif

IRIT_STATIC_DATA const char
    *GlblAuthorName = "                         Written by Gershon Elber";

IRIT_STATIC_DATA char GlblCrntWorkingDir[IRIT_LINE_LEN];/*To recover on exit.*/

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

static void Interact(void);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of IRIT - Read command line and do what is needed...	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Exit reason code.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    const char *FullPathStartFileName;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

#ifdef DEBUG
    InputTestFuncTablesValidity();

    /* Verify the parser's tables sizes. */
    assert(IP_REAL_VAL_LAST < IP_NUM_FUNC_END &&
	   IP_OBJ_VAL_LAST < IP_OBJ_FUNC_END &&
	   IP_GEN_VAL_LAST < IP_GEN_FUNC_END);
#endif /* DEBUG */

    IritCPUTime(TRUE);					 /* Reset the clock. */

    IritConfig("irit", SetUp, NUM_SET_UP);   /* Read config. file if exists. */

    while (argc >= 2) {
    	if (strncmp(argv[1], "-z", 2) == 0) {
	    IRIT_INFO_MSG_PRINTF("\n%s\n\nUsage: %s\n",
		                 VersionStr, CtrlStr);
	    IritConfigPrint(SetUp, NUM_SET_UP);
	    exit(0);
    	}
	else if (strncmp(argv[1], "-i", 2) == 0) {
	    GlblStdinInteractive = TRUE;
	}
	else if (strncmp(argv[1], "-s", 2) == 0) {
	    GlblRunScriptAndQuit = TRUE;
	}
	else if (strncmp(argv[1], "-t", 2) == 0) {
	    GlblDoGraphics = FALSE;
	}
	else if (strncmp(argv[1], "-g", 2) == 0) {
	    GlblGUIMode = TRUE;
	}
	else if (strncmp(argv[1], "-q", 2) == 0) {
	    GlblQuietMode = TRUE;
	}
#	ifdef DEBUG_IRIT_MALLOC
	    else if (strncmp(argv[1], "-m", 2) == 0) {
		assert(argc >= 5);
	        IritInitTestDynMemory2(atoi(argv[2]),
				       atoi(argv[3]),
				       atoi(argv[4]));
		argv += 3;
		argc -= 3;
	    }
#	endif /* DEBUG_IRIT_MALLOC */
	else {
	    break;
	}
    	argv++;
	argc--;
    }

    getcwd(GlblCrntWorkingDir, IRIT_LINE_LEN - 1);

    SetUpCtrlBrk();	     /* Set up control break trap routine (int 1bh). */
    signal(SIGFPE, IritDefaultFPEHandler);    /* Trap floating point errors. */

    BspMultComputationMethod(GlblBspProdMethod);
    IPSetFloatFormat(GlblFloatFormat);
    IPSetFlattenObjects(FALSE);
    IPSetPolyListCirc(TRUE);

    /* Print some copyright messages: */
    if (GlblQuietMode) {
	InptPrsrSetEchoSource(FALSE);
    }
    else {
	IRIT_WNDW_PUT_STR(GlblPrgmHeader);
	IRIT_WNDW_PUT_STR(GlblCopyRight);
	IRIT_WNDW_PUT_STR(GlblAuthorName);
    }

    SetUpPredefObjects();		  /* Prepare the predefined objects. */

    BoolSetHandleCoplanarPoly(TRUE);		/* Handle coplanar polygons. */
    BoolSetFatalErrorFunc(IritDefaultBoolFatalError);

    /* Execute the file specified in the command line if was one: */
    if (argc == 2)
	InptPrsrFileInclude(argv[1]);

    /* Execute the start up file first by inserting it to the include stack. */
    if (!IRT_STR_ZERO_LEN(GlblStartFileName) &&
	(FullPathStartFileName = searchpath(GlblStartFileName)) != NULL)
	InptPrsrFileInclude(FullPathStartFileName);

#   ifdef __UNIX__
    {
#   if defined (__FreeBSD__) || defined(__MACOSX__)
	struct termios Termio;
#   else
	struct termio Termio;
#   endif /* __FreeBSD__ || __MACOSX__ */

	/* We read stdin on a char by char basis with a 0.1 second timer so  */
	/* we could simultaneously handle other requests (for example, from  */
	/* display devices).						     */
#   if defined (__FreeBSD__) || defined(__MACOSX__)
	tcgetattr(0, &GlblOrigTermio);
	tcgetattr(0, &Termio);
#   else
	ioctl(0, TCGETA, &GlblOrigTermio);
	ioctl(0, TCGETA, &Termio);
#   endif /* __FreeBSD__ || __MACOSX__ */
	Termio.c_lflag &= ~ICANON;	     /* Clear the canonical editing. */
	Termio.c_cc[VEOF] = 0;    /* MIN = 0, no minimal length to wait for. */
	Termio.c_cc[VEOL] = 1;    /* TIME - 1 tenth of a second as time out. */
#       ifdef VSUSP
	    Termio.c_cc[VSUSP] = 0;		   /* Disable ^Z suspension. */
#       endif /* VSUSP */
#   if defined (__FreeBSD__) || defined(__MACOSX__)
	tcsetattr(0, TCSANOW, &Termio);
#   else
	ioctl(0, TCSETA, &Termio);
#   endif /* __FreeBSD__ || __MACOSX__ */
    }
#   endif /* __UNIX__ */

    IPSocSrvrInit();            /* Initialize the listen socket for clients. */

    Interact();				      /* Go and do some real work... */

    IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Interact - the main read/eval/print routine. This routine reads data from  *
* standart input and execute it "forever" (using Input Parser).		     *
*   Note exit from this program is controled by input parser itself.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Interact(void)
{
    /* setjmp return 0 on first install time. Will return 1 if error is      */
    /* recoverable, 2 if cannt continue - must quit the program now.	     */
    switch (setjmp(IritGlblLongJumpBuffer)) {     /* Used in error recovery. */
	case 1:
	    IritDBValidateVariables();
	case 0:
	    while (TRUE) {
		if (!InptPrsrInputParser(NULL))	 /* Print the error message. */
		    IritPrintInptPrsrError();
		IritDBValidateVariables();
	    }
	case 2:
	    IRIT_WNDW_PUT_STR("Press return to die...");
	    getchar();
	    break;
	case 3:
	    /* Optional - trapping stdin request. */
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit warning errors.		     M
*   This function just prints the given error message.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message to print.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritInformationMsgPrintf, IritFatalError, IritWarningMsg                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritInformationMsg, information messages                                 M
*****************************************************************************/
void IritInformationMsg(const char *Msg)
{
    if (Msg != NULL) {
	IRIT_WNDW_FPRINTF2("Irit Info: %s", Msg);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit warning errors, printf style.      M
*                                                                            *
* PARAMETERS:                                                                M
*   va_alist:   Do "man stdarg".                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritInformationMsg, IritFatalErrorPrintf, IritWarningMsgPrintf           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritInformationMsgPrint, information messages                            M
*****************************************************************************/
#ifdef USE_VARARGS
void IritInformationMsgPrintf(const char *va_alist, ...)
{
    char *Format, *p;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void IritInformationMsgPrintf(const char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    IRIT_WNDW_FPRINTF2("%s", p);

    va_end(ArgPtr);
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
* SEE ALSO:                                                                  M
*   IritWarningMsgPrintf, IritFatalError, IritInformationMsg                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritWarningMsg, error trap                                               M
*****************************************************************************/
void IritWarningMsg(const char *Msg)
{
    if (Msg != NULL) {
	IRIT_WNDW_FPRINTF2("Irit Warning: %s", Msg);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit warning errors, printf style.      M
*                                                                            *
* PARAMETERS:                                                                M
*   va_alist:   Do "man stdarg".                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritWarningMsg, IritFatalErrorPrintf, IritInformationMsgPrintf           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritWarningMsgPrintf, warning trap, error trap                           M
*****************************************************************************/
#ifdef USE_VARARGS
void IritWarningMsgPrintf(const char *va_alist, ...)
{
    char *Format, *p;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void IritWarningMsgPrintf(const char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    IRIT_WNDW_FPRINTF2("%s", p);

    va_end(ArgPtr);
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
*                                                                            *
* SEE ALSO:                                                                  M
*   IritFatalErrorPrintf, IritWarningMsg, IritInformationMsg                 M
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for IRIT programs for irit warning errors, printf style.      M
*                                                                            *
* PARAMETERS:                                                                M
*   va_alist:   Do "man stdarg".                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritFatalError, IritWarningMsgPrintf, IritInformationMsgPrintf           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritFatalErrorPrintf, error trap                                         M
*****************************************************************************/
#ifdef USE_VARARGS
void IritFatalErrorPrintf(const char *va_alist, ...)
{
    char *Format, *p;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void IritFatalErrorPrintf(const char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    IRIT_WNDW_FPRINTF2("%s", p);

    va_end(ArgPtr);

    exit(-1);
}

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
#   if defined (__FreeBSD__) || defined(__MACOSX__)
	tcsetattr(0, TCSANOW, &GlblOrigTermio);
#   else
	ioctl(0, TCSETA, &GlblOrigTermio);
#   endif /* __FreeBSD__ || __MACOSX__ */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Misc_lib errors right here. Call back function of misc_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in misc_lib library.                              M
*   ErrDesc:  Possibly, an additional description on error.                  M
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

    IRIT_WNDW_FPRINTF3("MISC_LIB: %s%s", ErrorMsg, ErrDesc);

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Trap Mdl_lib errors right here. Call back function of mdl_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MdlFatalError, error handling                                            M
*****************************************************************************/
void MdlFatalError(MdlFatalErrorType ErrID)
{
    const char
	*ErrorMsg = MdlDescribeError(ErrID);

    IRIT_WNDW_FPRINTF2("MDL_LIB: %s", ErrorMsg);

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
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

    IRIT_INFO_MSG_PRINTF("IritIdle: %d\r", i++);
#endif /* DUMP_COUNTER */

    if (!IPSocSrvrListen() && MiliSeconds > 0)
        IritSleep(MiliSeconds);
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
*   Call back function of the server listening to clients.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Handler:    Client handler from which an event has been received.        M
*   PObj:       NULL if a new client has connected, object received from     M
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

    InptPrsrFlushToEndOfExpr(TRUE);

#   ifdef DEBUG
    IritAllFatalErrorTrap();
#   endif /* DEBUG */

    longjmp(IritGlblLongJumpBuffer, 1);	   /* Go back to main loop directly. */
}

#endif /* !IRIT_QUIET_STRINGS */
