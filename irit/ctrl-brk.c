/*****************************************************************************
* Module to trap ctrl-brk/hardware error and handle them gracefully.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.1, Mar. 1990   *
*****************************************************************************/

#include <stdio.h>
#include "program.h"
#include "inptprsg.h"
#include "ctrl-brk.h"

typedef void (* SignalFuncPtr)(int);

#ifdef __WINNT__
#include <windows.h>
#include <tchar.h>
static BOOL WINAPI TrapCtrlCWin32sRoutine(DWORD dwCtrlType);
#endif /* __WINNT__ */

static void TrapCtrlC(int Type);

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine TrapCtrlC gains control if Control C was typed (DOS level):	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Type:     Type of exception.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrapCtrlC(int Type)
{
    SetUpCtrlBrk();
    InptPrsrFlushToEndOfExpr(FALSE);
    printf("\n*** Break ***\n");

#ifndef __WINNT__
    fflush(stdout);

    longjmp(IritGlblLongJumpBuffer, 1);
#endif /* __WINNT__ */
}

#ifdef __WINNT__
/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to trap Ctrl-C on Windows NT systems.	 Unfortunately this must be  *
* syncronized and cannot really interrupt a really heavy duty computation.   *
*                                                                            *
* PARAMETERS:                                                                *
*   dwCtrlType: Type of event to handle.	                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   BOOL WINAPI:  TRUE if event was handled, FALSE otherwise.                *
*****************************************************************************/
static BOOL WINAPI TrapCtrlCWin32sRoutine(DWORD dwCtrlType)
{
    printf("\n*** Break WIN32 ***\n");
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    InptPrsrFlushToEndOfExpr(FALSE);
    return TRUE;
}
#endif /* __WINNT__ */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine SetUpCtrlBrk must be called once by main program, at the beginning.M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetUpCtrlBrk                                                             M
*****************************************************************************/
void SetUpCtrlBrk(void)
{
    if (getenv("IRIT_NO_SIGNALS") == NULL) {
#if defined(__UNIX__) || defined(OS2GCC)
	signal(SIGINT, (SignalFuncPtr) TrapCtrlC);
	signal(SIGQUIT, (SignalFuncPtr) TrapCtrlC);
	signal(SIGKILL, (SignalFuncPtr) IritExit0);
#endif /* __UNIX__ || OS2GCC */
#if defined(__WINNT__)
	if (!SetConsoleCtrlHandler(TrapCtrlCWin32sRoutine, TRUE))
	    IRIT_WARNING_MSG_PRINTF("Failed to setup Ctrl-C handler (error %ld)\n",
		                    GetLastError());
	signal(SIGBREAK, (SignalFuncPtr) TrapCtrlC);
	signal(SIGTERM, (SignalFuncPtr) TrapCtrlC);
	signal(SIGINT, (SignalFuncPtr) TrapCtrlC);
#endif /* __WINNT__ */
    }
}
