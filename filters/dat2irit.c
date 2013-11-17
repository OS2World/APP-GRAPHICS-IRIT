/*****************************************************************************
* Filter to convert IRIT data files back to IRIT .irt files.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Sep 1991    *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "triv_lib.h"
#include "trim_lib.h"
#include "trng_lib.h"
#include "mvar_lib.h"
#include "mdl_lib.h"
#include "misc_lib.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Dat2Irit		Version 11,		Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Dat2Irit	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "dat2irit d%-FloatFormat!s z%- DFiles!*s";

static void PrintLine(const char *Line);
static void Dat2IritExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of dat2irit - Read command line and do what is needed...	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    int Error,
	VerFlag = FALSE,
	FormatFlag = FALSE,
	NumFiles = 0;
    char
	*FloatFormat = "%16.14lg",
	**FileNames = NULL;
    IPObjectStruct *PObjects;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &FormatFlag, &FloatFormat, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Dat2IritExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Dat2IritExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Dat2IritExit(1);
    }

    IRIT_INFO_MSG_PRINTF("Using Float Format of \"%s\"\n", FloatFormat);
    IPSetFloatFormat(FloatFormat);

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Dat2IritExit(0);

    IPCnvSetPrintFunc(PrintLine);
    IPCnvDataToIrit(PObjects);

    Dat2IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one line to stdout.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Line:   String to dump.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintLine(const char *Line)
{
    fputs(Line, stdout);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dat2Irit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:   error/exit code.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Dat2IritExit(int ExitCode)
{
    exit(ExitCode);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Dummy function to link at debugging time.                               *
*                                                                            *
* PARAMETERS:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*****************************************************************************/
void DummyLinkCagdDebug(void)
{
    IPDbg();
}

#endif /* DEBUG */
