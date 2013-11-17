/*****************************************************************************
* Filter to convert STL data files to IRIT .irt files.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 2002   *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "geom_lib.h"
#include "misc_lib.h"

#ifdef __WINNT__
#include <io.h>
#include <fcntl.h>
#endif /* __WINNT__ */

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char 
    *VersionStr = "stl2irit		Version 11,		Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "stl2irit	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "stl2irit b%- w%- n%- o%-OutName!s z%- STLFile!*s";

static void Stl2IritExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of stl2irit - Read command line and do what is needed...	     M
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
    int Error, NumFiles,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
        BinarySTL = FALSE,
        EndianSwap = FALSE,
        NormalFlip = FALSE;
    char 
	*OutFileName = NULL,
	**FileNames = NULL;
    FILE *DATFile;
    IPObjectStruct *PObj;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &BinarySTL, &EndianSwap, &NormalFlip,
			   &OutFileFlag, &OutFileName, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Stl2IritExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Stl2IritExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Stl2IritExit(1);
    }
    else if (NumFiles > 1) {
	IRIT_WARNING_MSG("Cannot handle more than one STL file at a time, exit.\n");
	GAPrintHowTo(CtrlStr);
	Stl2IritExit(1);
    }

    PObj = IPSTLLoadFile(FileNames[0], BinarySTL, EndianSwap, NormalFlip,
			 TRUE);

    if (OutFileName != NULL) {
	if ((DATFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    Stl2IritExit(2);
	}
    }
    else
        DATFile = stdout;
    IPPutObjectToFile(DATFile, PObj, FALSE);
    fclose(DATFile);
    IPFreeObject(PObj);

    Stl2IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Stl2Irit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Stl2IritExit(int ExitCode)
{
    exit(ExitCode);
}
