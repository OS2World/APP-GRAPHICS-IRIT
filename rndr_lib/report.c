/*****************************************************************************
* Error reporting facility.                                                  *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  David Shafrir & Alex Reicher       Ver 0.3, Sep. 2003         *
*****************************************************************************/

#include "rndr_loc.h"
#include <stdarg.h>

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports a warning message.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Fmt:       IN, message format, like the "printf" format.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IRndrReportWarning, report warning                                      M
*****************************************************************************/
void _IRndrReportWarning(const char *Fmt, ...)
{
    va_list vl;
    char Line[IRIT_LINE_LEN_VLONG];

    IRIT_WARNING_MSG("rndr_lib: ");
    va_start(vl, Fmt);
    vsprintf(Line, Fmt, vl);
    va_end(vl);
    IRIT_WARNING_MSG(Line);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports an error message.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Fmt:       IN, message format, like the "printf" format.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IRndrReportError, report error                                          M
*****************************************************************************/
void _IRndrReportError(const char *Fmt, ...)
{
    va_list vl;
    char Line[IRIT_LINE_LEN_VLONG];

    IRIT_WARNING_MSG("rndr_lib: ");
    va_start(vl, Fmt);
    vsprintf(Line, Fmt, vl);
    va_end(vl);
    IRIT_WARNING_MSG(Line);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports a fatal error and halts.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Fmt:       IN, message format, like the "printf" format.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _IRndrReportFatal, report fatal halt                                     M
*****************************************************************************/
void _IRndrReportFatal(const char *Fmt, ...)
{
    va_list vl;
    char Line[IRIT_LINE_LEN_VLONG];

    IRIT_WARNING_MSG("rndr_lib: ");
    va_start(vl, Fmt);
    vsprintf(Line, Fmt, vl);
    va_end(vl);
    IRIT_WARNING_MSG(Line);
    exit(-1);
}
