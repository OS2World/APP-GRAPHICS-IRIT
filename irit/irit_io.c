/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller - IO.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Module to handle the windows used by the solid modeller.		     *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "program.h"
#include "iritprsr.h"
#ifdef __WINNT__
#include <process.h>
#endif /* __WINNT__ */

#define LE_NUM_OF_PREV_LINES 50

typedef enum {
    ASCII_NUL,
    ASCII_CTL_A,
    ASCII_CTL_B,
    ASCII_CTL_C,
    ASCII_CTL_D,
    ASCII_CTL_E,
    ASCII_CTL_F,
    ASCII_CTL_G,
    ASCII_CTL_H,
    ASCII_CTL_I,
    ASCII_CTL_J,
    ASCII_LF = ASCII_CTL_J,
    ASCII_CTL_K,
    ASCII_CTL_L,
    ASCII_CTL_M,
    ASCII_CR = ASCII_CTL_M,
    ASCII_CTL_N,
    ASCII_CTL_O,
    ASCII_CTL_P,
    ASCII_CTL_Q,
    ASCII_CTL_R,
    ASCII_CTL_S,
    ASCII_CTL_T,
    ASCII_CTL_U,
    ASCII_CTL_V,
    ASCII_CTL_W,
    ASCII_CTL_X,
    ASCII_CTL_Y,
    ASCII_CTL_Z,
    ASCII_DEL = 127
} AsciiControlType;

typedef enum {
    LE_ERASE,
    LE_DELETE,
    LE_BACKWARD,
    LE_FORWARD,
    LE_BEGIN_LINE,
    LE_END_LINE,
    LE_PREV_LINE,
    LE_NEXT_LINE,
    LE_KILL_END,
    LE_OVERWRITE_INSERT,
    LE_NEW_LINE,
    LE_NUM_OF_CONTROLS
} LineEditControlType;

IRIT_STATIC_DATA char LECtrl[LE_NUM_OF_CONTROLS] =
{
    ASCII_CTL_H,  /* Erase */
    ASCII_CTL_D,  /* Delete */
    ASCII_CTL_B,  /* Backward */
    ASCII_CTL_F,  /* Forward */
    ASCII_CTL_A,  /* BeginLine */
    ASCII_CTL_E,  /* EndLine */
    ASCII_CTL_P,  /* PrevLine */
    ASCII_CTL_N,  /* NextLine */
    ASCII_CTL_K,  /* Kill End */
    ASCII_CTL_I,  /* OverwriteInsert */
    ASCII_LF      /* NewLine */
};

IRIT_STATIC_DATA int
    GlblAutoExecDisplay = -1;

static IPObjectStruct *IritPickCrsrClientEvent(IrtRType *RWaitTime);

#ifdef __WINNT__

IRIT_STATIC_DATA char
    *GlblNextStdLine = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               *
* A secondary thread that waits for input from stdin.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:      Not used.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IritGetInputFromStdin(void *Data)
{
    IRIT_STATIC_DATA char Line[IRIT_LINE_LEN_VLONG];

    while (TRUE) {
	while (GlblNextStdLine != NULL)
	    IritSleep(5);
	fgets(Line, IRIT_LINE_LEN_VLONG - 1, stdin);

	GlblNextStdLine = Line;
    }
}
#endif /* __WINNT__ */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reads one line, terminated by a new line, from stdin.  Calls             M
* IritIdleFunction when no input is available.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Line:      Where to place the read line.                                 M
*   Prompt:    To print, calling for input from stdin.                       M
*   MaxLen:    Maximal length of line.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:    Points to Line.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritIdleFunction                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritGetLineStdin                                                         M
*****************************************************************************/
char *IritGetLineStdin(char *Line, const char *Prompt, int MaxLen)
{
    IRIT_STATIC_DATA char
        **PrevBuf = NULL;
    IRIT_STATIC_DATA int
	Overwrite = FALSE,
	FirstTime = TRUE;
    int i, c,
	PrevLen = 0,
	PrevPtr = -1,
	l = 0,						     /* Line length. */
        r = 0;						 /* Cursor position. */

    if (FirstTime) {
	int Ctrl[LE_NUM_OF_CONTROLS];

#	ifdef __WINNT__
	    _beginthread(IritGetInputFromStdin, 0, NULL);
#	endif /* __WINNT__ */

	if (sscanf(GlblLineEditControl,
		   "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		   &Ctrl[0], &Ctrl[1], &Ctrl[2], &Ctrl[3], &Ctrl[4], &Ctrl[5],
		   &Ctrl[6], &Ctrl[7], &Ctrl[8], &Ctrl[9], &Ctrl[10]) ==
							  LE_NUM_OF_CONTROLS) {
	    for (i = 0; i < LE_NUM_OF_CONTROLS; i++)
		LECtrl[i] = Ctrl[i];
	}
	else {
	    IRIT_INFO_MSG("Line Edit: Failed to parse configuration; use defaults\n");
	}
	FirstTime = FALSE;
    }

    if (!GlblStdinInteractive) {
	printf(Prompt);
	fflush(stdout);
    }

#   ifdef __WINNT__
	while (GlblNextStdLine == NULL) {
	    IritIdleFunction(1);	      /* See if anything else to do. */
	}

	if (GlblNextStdLine[0] == 4)	
	    IritExit(0);	   /* Eof typed on keyboard (usually CtrlD). */

        strncpy(Line, GlblNextStdLine, MaxLen - 1);
        GlblNextStdLine = NULL;
        return Line;
#   endif /* __WINNT__ */

    if (PrevBuf == NULL) {
	/* Allocate the buffer to hold previous lines. */
	PrevBuf = (char **) IritMalloc(sizeof(char *) * LE_NUM_OF_PREV_LINES);
	IRIT_ZAP_MEM(PrevBuf, sizeof(char *) * LE_NUM_OF_PREV_LINES);
    }

    while (TRUE) {
	if (GlblStdinInteractive) {
	    /* The way we print the line and the cursor are not the most     */
	    /* efficient, but simplicity and portability outways it!  */
	    Line[l] = 0;
	    printf(IRIT_EXP_STR("\r%s%s"), Prompt, Line);
	    for (i = l; i < PrevLen; i++)
		printf(IRIT_EXP_STR(" "));

	    /* Bring cursor to proper place, via a 2nd print to cursor. */
	    printf(IRIT_EXP_STR("\r%s"), Prompt);
	    for (i = 0; i < r; i++)
		printf(IRIT_EXP_STR("%c"), Line[i]);
	}

	c = fgetc(stdin);

	if (c == 4 && l == 0)	
	    IritExit(0);	   /* Eof typed on keyboard (usually CtrlD). */

	PrevLen = l;

	if (c == LECtrl[LE_ERASE]) {
	    if (r > 0) {
	        for (i = r; i < l; i++)
		    Line[i - 1] = Line[i];
		l--;
		r--;
	    }
	}
	else if (c == LECtrl[LE_DELETE]) {
	    if (r < l) {
	        for (i = r; i < l; i++)
		    Line[i] = Line[i + 1];
		l--;
	    }
	}
        else if (c == LECtrl[LE_BACKWARD]) {
	    if (r > 0)
	        r--;
	}
        else if (c == LECtrl[LE_FORWARD]) {
	    if (r < l)
	        r++;
	}
        else if (c == LECtrl[LE_BEGIN_LINE]) {
	    r = 0;
	}
        else if (c == LECtrl[LE_END_LINE]) {
	    r = l;
	}
        else if (c == LECtrl[LE_PREV_LINE]) {
	    if (PrevPtr < LE_NUM_OF_PREV_LINES && PrevBuf[PrevPtr + 1] != NULL) {
	        strcpy(Line, PrevBuf[++PrevPtr]);
		l = r = (int) strlen(Line) - 1;            /*  Minus the LF. */
	    }
	}
        else if (c == LECtrl[LE_NEXT_LINE]) {
	    if (PrevPtr > 0 && PrevBuf[PrevPtr - 1] != NULL) {
	        strcpy(Line, PrevBuf[--PrevPtr]);
		l = r = (int) strlen(Line) - 1;            /*  Minus the LF. */
	    }
	}
        else if (c == LECtrl[LE_KILL_END]) {
	    l = r;
	}
        else if (c == LECtrl[LE_OVERWRITE_INSERT]) {
	    Overwrite = !Overwrite;
	}
        else if (c == LECtrl[LE_NEW_LINE] || c == ASCII_CR || c == ASCII_LF) {
	    Line[l++] = c;
	    Line[l] = 0;				  /* Close the line. */

	    if (PrevBuf[LE_NUM_OF_PREV_LINES - 1] != NULL)/* Free last line. */
	        IritFree(PrevBuf[LE_NUM_OF_PREV_LINES - 1]);
	    for (i = LE_NUM_OF_PREV_LINES - 1; i > 0; i--)   /* Shift lines. */
		PrevBuf[i] = PrevBuf[i - 1];
	    PrevBuf[0] = IritStrdup(Line);            /* Keep this new line. */

	    return Line;
	}
	else {
	    if (isascii(c) && !iscntrl(c)) {
	        if (Overwrite) {
	            Line[r++] = c;
		    if (l < r)
		        l = r;
		}
		else {
		    for (i = l; i > r; i--)
		        Line[i] = Line[i - 1];
		    Line[r++] = c;
		    l++;
		}
	    }
	}

	if (l >= MaxLen - 2) {
	     l = MaxLen - 2;
	     if (r > l)
		 r = l;
	}

	IritIdleFunction(c == EOF ? 10 : 0);  /* See if anything else to do. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to handle the text on the Input Window. This window echoes all     M
* the input steam - the input parser input. Errors or information are also   M
* calling this function.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:        To print to stdout.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPutStr	                                                             M
*****************************************************************************/
void IritPutStr(const char *Msg)
{
    if (GlblPrintLogFile)
	fprintf(GlblLogFile, Msg[strlen(Msg) - 1] == '\n' ? "%s" : "%s\n",
		Msg);

    IritPrintOneString(Msg);
    if (Msg[strlen(Msg) - 1] != '\n')
	IritPrintOneString("\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same as IRIT_WNDW_PUT_STR but does not put a new line if have none.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:        To print to stdout.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritPutStr2                                                              M
*****************************************************************************/
void IritPutStr2(const char *Msg)
{
    if (GlblPrintLogFile)
	fprintf(GlblLogFile, "%s", Msg);

    IritPrintOneString(Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same as fprintf but with indentation.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   va_alist:   Do "man stdarg"                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritFprintf                                                              M
*****************************************************************************/
#ifdef USE_VARARGS
void IritFprintf(const char *va_alist, ...)
{
    char *Format, *p;
    int i;
    va_list ArgPtr;

    va_start(ArgPtr);
    Format = va_arg(ArgPtr, char *);
#else
void IritFprintf(const char *Format, ...)
{
    char *p;
    va_list ArgPtr;

    va_start(ArgPtr, Format);
#endif /* USE_VARARGS */

    IRIT_VSPRINTF(p, Format, ArgPtr);

    IRIT_WNDW_PUT_STR(p);

    va_end(ArgPtr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to handle reading one line from stdin into a object of type Type.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Type:      Type of object requested.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An object of type Type initialized from data from    M
*                       stdin.                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritInputStdinObject                                                     M
*****************************************************************************/
IPObjectStruct *IritInputStdinObject(IrtRType *Type)
{
    char Str[IRIT_LINE_LEN_LONG];
    IrtRType R, Pt[3];
    IPObjectStruct *PObj;
    IPObjStructType
	IType = (IPObjStructType) IRIT_REAL_PTR_TO_INT(Type);

    IritGetLineStdin(Str, "", IRIT_LINE_LEN_LONG - 1);
    if (Str[strlen(Str) - 1] < ' ') 
	Str[strlen(Str) - 1] = 0;				/* No CR/LF. */

#ifdef IRIT_DOUBLE
    switch (IType) {
	case IP_OBJ_NUMERIC:
	    if (sscanf(Str, "%lf", &R) == 1)
		return IPGenNUMObject(&R);
	    break;
	case IP_OBJ_POINT:
	    if (sscanf(Str, "%lf %lf %lf", &Pt[0], &Pt[1], &Pt[2]) == 3 ||
		sscanf(Str, "%lf,%lf,%lf", &Pt[0], &Pt[1], &Pt[2]) == 3)
		return IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]);
	    break;
	case IP_OBJ_VECTOR:
	    if (sscanf(Str, "%lf %lf %lf", &Pt[0], &Pt[1], &Pt[2]) == 3 ||
		sscanf(Str, "%lf,%lf,%lf", &Pt[0], &Pt[1], &Pt[2]) == 3)
		return IPGenVECObject(&Pt[0], &Pt[1], &Pt[2]);
	    break;
	case IP_OBJ_PLANE:
	    if (sscanf(Str, "%lf %lf %lf %lf",
		       &Pt[0], &Pt[1], &Pt[2], &Pt[3]) == 4 ||
		sscanf(Str, "%lf,%lf,%lf,%lf",
		       &Pt[0], &Pt[1], &Pt[2], &Pt[3]) == 4)
		return IPGenPLANEObject(&Pt[0], &Pt[1], &Pt[2], &Pt[3]);
	    break;
	default:
	    break;
    }
#else
    switch (IType) {
	case IP_OBJ_NUMERIC:
	    if (sscanf(Str, "%f", &R) == 1)
		return IPGenNUMObject(&R);
	    break;
	case IP_OBJ_POINT:
	    if (sscanf(Str, "%f %f %f", &Pt[0], &Pt[1], &Pt[2]) == 3 ||
		sscanf(Str, "%f,%f,%f", &Pt[0], &Pt[1], &Pt[2]) == 3)
		return IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]);
	    break;
	case IP_OBJ_VECTOR:
	    if (sscanf(Str, "%f %f %f", &Pt[0], &Pt[1], &Pt[2]) == 3 ||
		sscanf(Str, "%f,%f,%f", &Pt[0], &Pt[1], &Pt[2]) == 3)
		return IPGenVECObject(&Pt[0], &Pt[1], &Pt[2]);
	    break;
	case IP_OBJ_PLANE:
	    if (sscanf(Str, "%f %f %f %f",
		       &Pt[0], &Pt[1], &Pt[2], &Pt[3]) == 4 ||
		sscanf(Str, "%f,%f,%f,%f",
		       &Pt[0], &Pt[1], &Pt[2], &Pt[3]) == 4)
		return IPGenPLANEObject(&Pt[0], &Pt[1], &Pt[2], &Pt[3]);
	    break;
	default:
	    break;
    }
#endif /* IRIT_DOUBLE */

    PObj = IPGenSTRObject(Str);
    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the display device channel.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispViewSetDisplay                                                   M
*****************************************************************************/
void IritDispViewSetDisplay(void)
{
    char
	*Program = getenv(IRIT_EXP_STR("IRIT_DISPLAY"));

    if (!GlblDoGraphics)
	return;

    if (Program == NULL) {
        IRIT_WNDW_PUT_STR("No \"IRIT_DISPLAY\" environment - run display device manually");
    }
    else if ((GlblAutoExecDisplay = (int) IritClientExecute(Program)) < 0) {
	IRIT_WNDW_FPRINTF2("Failed to execute display device \"%s\"\n",
			   Program == NULL ? "?" : Program);
	exit(0);
    }

    GlblCurrentDisplay = IP_CLNT_BROADCAST_ALL_HANDLES;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the display device channel.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Ri:  Handle of display client.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispSetCrntDisplay                                                   M
*****************************************************************************/
void IritDispSetCrntDisplay(IrtRType *Ri)
{
    int i = IRIT_REAL_PTR_TO_INT(Ri);

    GlblCurrentDisplay = i;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Display one object on display device.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to display.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispViewObject                                                       M
*****************************************************************************/
void IritDispViewObject(IPObjectStruct *PObj)
{
    IrtRType R, R2;

    if (!GlblDoGraphics)
	return;

    if (GlblCurrentDisplay < 0)
	IritDispViewSetDisplay();
    if (GlblCurrentDisplay < 0)
	return;

    R = GlblCurrentDisplay;

    if (IP_IS_STR_OBJ(PObj)) {
	if (strcmp(IP_GET_OBJ_NAME(PObj), IRIT_EXP_STR("COMMAND_")) == 0 &&
	    (strcmp(PObj -> U.Str, IRIT_EXP_STR("EXIT")) == 0 ||
	     strcmp(PObj -> U.Str, IRIT_EXP_STR("DISCONNECT")) == 0)) {
	    R2 = strcmp(PObj -> U.Str, IRIT_EXP_STR("EXIT")) == 0;
	    IritClientClose(&R, &R2);
	    GlblCurrentDisplay = IP_CLNT_BROADCAST_ALL_HANDLES;
	}
	else
	    IritClientWrite(&R, PObj);
    }
    else
	IritClientWrite(&R, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Clears the display device's device.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispViewClearScreen                                                  M
*****************************************************************************/
void IritDispViewClearScreen(void)
{
    IPObjectStruct
	*PObj = IPGenStrObject(IRIT_EXP_STR("COMMAND_"),
			       IRIT_EXP_STR("CLEAR"), NULL);

    IritDispViewObject(PObj);

    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Asks the automatically executed display device (if active) to save its     M
* current transformation matrix in file FileName.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   Name of file to save current transformation.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispViewSaveMatrix                                                   M
*****************************************************************************/
void IritDispViewSaveMatrix(const char *FileName)
{
    assert(strlen(FileName) < IRIT_LINE_LEN_VLONG - 10);

    if (GlblAutoExecDisplay >= 0) {
        char Str[IRIT_LINE_LEN_VLONG];
        IrtRType
	    R = GlblAutoExecDisplay;
        IPObjectStruct *PObj;

	sprintf(Str, IRIT_EXP_STR("MSAVE %s"), FileName);

	PObj = IPGenStrObject(IRIT_EXP_STR("COMMAND_"), Str, NULL);

	IritClientWrite(&R, PObj);

	IPFreeObject(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Disconnect from current display device but dont ask it t quit.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispViewDisconnect                                                   M
*****************************************************************************/
void IritDispViewDisconnect(void)
{
    IPObjectStruct
	*PObj = IPGenStrObject(IRIT_EXP_STR("COMMAND_"),
			       IRIT_EXP_STR("DISCONNECT"), NULL);

    IritDispViewObject(PObj);

    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Disconnect from current display device and ask it to terminate.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritDispViewExit                                                         M
*****************************************************************************/
void IritDispViewExit(void)
{
    IPObjectStruct *PObj;

    /* Do we have any active displays? */
    if (GlblCurrentDisplay < 0)
        return;

    PObj = IPGenStrObject(IRIT_EXP_STR("COMMAND_"),
			  IRIT_EXP_STR("EXIT"), NULL);

    IritDispViewObject(PObj);

    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Executes the program named PrgmName as a subprocess and hook a           M
* bidirectional communication channel between this IRIT server and PrgmName. M
*                                                                            *
* PARAMETERS:                                                                M
*   PrgmName:   To execute as a subprocess.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:	Handle of the forked out client.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritClientExecute, ipc                                                   M
*****************************************************************************/
double IritClientExecute(const char *PrgmName)
{
    int Handle = IPSocExecAndConnect(PrgmName,
			     getenv(IRIT_EXP_STR("IRIT_BIN_IPC")) != NULL);

    if (Handle < 0)
	IRIT_NON_FATAL_ERROR("Failed to execute program, exec failed");

    return (double) Handle;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Close a connection to a client subprocess specified by Handler.          M
*                                                                            *
* PARAMETERS:                                                                M
*   RHandler:     Valid subprocess handler.                                  M
*   RKillClient:  If TRUE, a request for the client to die is sent.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritClientClose, ipc                                                     M
*****************************************************************************/
void IritClientClose(IrtRType *RHandler, IrtRType *RKillClient)
{
    int Handle = IRIT_REAL_PTR_TO_INT(RHandler),
	KillClient = IRIT_REAL_PTR_TO_INT(RKillClient);

    IPSocDisConnectAndKill(KillClient, Handle);
    
    if (GlblGUIMode)
	IRIT_WNDW_FPRINTF2("<<IPC>> Closed client - handler %d <<CPI>>\n",
			   Handle);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Read one object from subprocess's input channel specified by RHandler.   M
*                                                                            *
* PARAMETERS:                                                                M
*   RHandler:  Valid subprocess handler.                                     M
*   RBlock:    If positive, blocks at most that much time if no object.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   One read object. If timeout, a string object is      M
*			returned with content of "No data (timeout)".        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritClientRead, ipc                                                      M
*****************************************************************************/
IPObjectStruct *IritClientRead(IrtRType *RHandler, IrtRType *RBlock)
{
    int Handler = IRIT_REAL_PTR_TO_INT(RHandler),
	Block = IRIT_REAL_PTR_TO_INT(RBlock);
    IPObjectStruct
	*PObj = NULL;

    if (Handler == IP_CLNT_BROADCAST_ALL_HANDLES) {
	IRIT_NON_FATAL_ERROR("Cannot read from all clients; be specific");
    }
    else {
	do {
	    PObj = IPSocReadOneObject(Handler);

	    /* Sleep 10 miliseconds at a time, if no data. */
	    if (PObj == NULL) {
		Block -= 10;
		if (Block > 10)
		    IritSleep(10);
	    }
	    else
		Block = 0;
	}
	while (Block > 0);
    }

    if (PObj == NULL)
	PObj = IPGenSTRObject(IRIT_EXP_STR("No data (timeout)"));

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Read one cursor event from subprocess's/clients input channel specified  M
* by RHandler.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   RWaitTime:  Maximal time to wait for the cursor event, in miliseconds.   M
*		If zero, waits indefinitly.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   One read object. If timeout, a zero length list      M
*			object is returned.  Otherwise a list of a point and M
*			a vector defining the cursor line in 3-space is      M
*			returned.  An attribute "EventType" of the event     M
*			type is also placed on the objects.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritClientCursor, ipc                                                    M
*****************************************************************************/
IPObjectStruct *IritClientCursor(IrtRType *RWaitTime)
{
    return IritPickCrsrClientEvent(RWaitTime);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Writes one object to subprocess's output channel specified by Handler.   M
*                                                                            *
* PARAMETERS:                                                                M
*   RHandler:  Valid subprocess handler.                                     M
*   PObj:      Object to write.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritClientWrite, ipc                                                     M
*****************************************************************************/
void IritClientWrite(IrtRType *RHandler, IPObjectStruct *PObj)
{
    IPSocWriteOneObject(IRIT_REAL_PTR_TO_INT(RHandler), PObj);
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
static IPObjectStruct *IritPickCrsrClientEvent(IrtRType *RWaitTime)
{
    int OrigWaitTime = IRIT_REAL_PTR_TO_INT(RWaitTime),
	WaitTime = OrigWaitTime;
    IPObjectStruct *RetVal;

    while (WaitTime >= 0) {
        if (GlblLastIritClientCursorEvent != NULL) {
	    RetVal = GlblLastIritClientCursorEvent;
	    GlblLastIritClientCursorEvent = NULL;	
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
