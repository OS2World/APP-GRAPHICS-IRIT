/*****************************************************************************
* Default information printing handler for irit, printf style.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.2, April 1993  *
*****************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "misc_loc.h"
#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */

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
*   IritInformationMsgPrintf, information messages                           M
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

    IritInformationMsg(p);

    va_end(ArgPtr);
}
