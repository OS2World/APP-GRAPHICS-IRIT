/*****************************************************************************
* Filter to convert IGES data files to IRIT .irt files.	        	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber	 			 Ver 1.0, May 1998   *
*****************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"

IRIT_STATIC_DATA const char
#ifdef NO_CONCAT_STR
    *VersionStr =
	"IGS2Irit		Version 11,	     Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
    *VersionStr = "igs2irit	" IRIT_VERSION ",   Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR*/

IRIT_STATIC_DATA const char
    *CtrlStr = "IGS2Irit m%- M%- c%- a%- s%- o%-OutName!s b%- z%- IGSFile!*s";

static IPObjectStruct *IgesGetSrfsOnly(IPObjectStruct *IritObjs);
static void Iges2IritExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of igs2irit - Read command line and do what is needed...	     M
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
	DumpAll = FALSE,
	DumpOnlySrfs = FALSE,
	MoreFlag = 1,
	WarningFlag = FALSE,
	InformativeFlag = FALSE,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
	ClipTrimmedSrf = FALSE,
	BinaryOutput = FALSE,
	NumFiles = 0;
    char
	*OutFileName = NULL,
	**FileNames = NULL;
    FILE *DATFile;
    IPObjectStruct *IritObjs;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &WarningFlag, &InformativeFlag,
			   &ClipTrimmedSrf, &DumpAll, &DumpOnlySrfs,
			   &OutFileFlag, &OutFileName, &BinaryOutput,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Iges2IritExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Iges2IritExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG_PRINTF("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Iges2IritExit(1);
    }
    else if (NumFiles > 1) {
	IRIT_WARNING_MSG_PRINTF("Cannot handle more than one IGES file at a time, exit.\n");
	GAPrintHowTo(CtrlStr);
	Iges2IritExit(1);
    }

    if (InformativeFlag)
	MoreFlag = 3;
    else if (WarningFlag)
	MoreFlag = 2;

    if (OutFileName != NULL) {
	if ((DATFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    Iges2IritExit(2);
	}
    }
    else
	DATFile = stdout;

    if ((IritObjs = IPIgesLoadFile(FileNames[0], ClipTrimmedSrf,
				   DumpAll, FALSE, MoreFlag)) != NULL) {
	if (!DumpOnlySrfs) {
	    IPPutObjectToFile(DATFile, IritObjs, BinaryOutput);
	}
	else {
	    IPObjectStruct
		*PSrfObjs = IgesGetSrfsOnly(IritObjs);

	    IPPutObjectToFile(DATFile, PSrfObjs, BinaryOutput);
	    IPFreeObjectList(PSrfObjs);
	}
    }

    IPFreeObjectList(IritObjs);

    fclose(DATFile);

    Iges2IritExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetchs only (trimmed) surfaces out of the given data.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   IritObjs:   Given data.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Only (trimmed) surfaces in the given data.            *
*****************************************************************************/
static IPObjectStruct *IgesGetSrfsOnly(IPObjectStruct *IritObjs)
{
    int i = 0,
        j = 0;
    IPObjectStruct *PTmp, *PObj,
        *PSrfObjs = IPGenListObject(IritObjs -> ObjName, NULL, NULL);

    while ((PObj = IPListObjectGet(IritObjs, j++)) != NULL) {
        if (IP_IS_SRF_OBJ(PObj) || IP_IS_TRIMSRF_OBJ(PObj)) {
	    PTmp = IPCopyObject(NULL, PObj, TRUE);
	    PTmp -> Pnext = NULL;

	    IPListObjectInsert(PSrfObjs, i++, PTmp);
	}
	else if (IP_IS_OLST_OBJ(PObj)) {
	    PTmp = IgesGetSrfsOnly(PObj);
	    IPListObjectInsert(PSrfObjs, i++, PTmp);
	}
    }
    IPListObjectInsert(PSrfObjs, i, NULL);

    return PSrfObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Igs2Irit exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Iges2IritExit(int ExitCode)
{
    exit(ExitCode);
}
