/******************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		      *
*									      *
* Written by:  Gershon Elber				 Ver 0.2, Mar. 1990   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Procedures to handle the dos interface - print/change the current dir. etc. *
******************************************************************************/

#ifdef __WINNT__
#include <direct.h>
#endif /* __WINNT__ */

#ifndef __WINCE__
#include <sys/types.h>
#include <errno.h>
#endif /* __WINCE__ */

#if defined(LINUX386)
#include <unistd.h>
#endif /* LINUX386 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "program.h"
#include "dosintr.h"
#include "ctrl-brk.h"
  
#ifdef AMIGA
#undef IRIT_DOUBLE
#include <proto/dos.h>
#endif /* AMIGA */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to change current directory					     M
*                                                                            *
* PARAMETERS:                                                                M
*   s:        New directory to change to.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DosChangeDir                                                             M
*****************************************************************************/
void DosChangeDir(const char *s)
{
    chdir(s);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Procedure to return current time from last time, time was reset.	     M
* Time is reset if the given parameter is non zero. Time is returned in      M
* seconds.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ResetTime:   To we want to resent the clock?                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   double:      Time since last reset/beginning of program.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   DosGetTime                                                               M
*****************************************************************************/
double DosGetTime(double ResetTime)
{
    return IritCPUTime(!IRIT_APX_EQ(ResetTime, 0.0));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Executes a system command.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Str:       The system's command to execute.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DosSystem                                                                M
*****************************************************************************/
void DosSystem(const char *Str)
{
#ifdef AMIGA
    if (SystemTagList(Str, NULL))
#else
    if (system(Str))
#endif /* AMIGA */
	IRIT_WNDW_PUT_STR("Undefined error in attempt to run command");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to milli-seconds sleep.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MiliSeconds:   How long do we want to sleep, in milliseconds?            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MilisecondSleep                                                          M
*****************************************************************************/
void MilisecondSleep(IrtRType *MiliSeconds)
{
    IritSleep(IRIT_REAL_PTR_TO_INT(MiliSeconds));
}
