/*****************************************************************************
* Filter to convert Wavefront's OBJ data files to IRIT .irt files.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Nadav Shragai				Ver 1.0, August 2009 *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "obj2irit		Version 11,		Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "obj2irit	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "obj2irit d%- t%- w%- s%- i%-InName!s o%-OutName!s z%-";

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of obj2irit - Read command line and do what is needed...	     M
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
    FILE *f;
    IPObjectStruct *PObj;
    char *InName = NULL, 
        *OutName = NULL;
    int Error,
        VerFlag = FALSE,
        WhiteDiffuseTextureFlag = FALSE,
        IgnoreFullTranspFlag = FALSE,
        ForceSmoothingFlag = FALSE,
        WarningFlag = FALSE,
        InNameFlag = FALSE,
        OutNameFlag = FALSE;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &WhiteDiffuseTextureFlag, 
			   &IgnoreFullTranspFlag, &WarningFlag, 
			   &ForceSmoothingFlag, &InNameFlag, &InName, 
			   &OutNameFlag, &OutName, &VerFlag)) != FALSE) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	return 1;
    }

    if ((OutNameFlag) && 
        (stricmp(OutName + strlen(OutName) - 4, ".itd") != 0)) {
        char *NewOutName = IritMalloc((strlen(OutName) + 5) * sizeof(char));

        sprintf(NewOutName, "%s.itd", OutName);
        OutName = NewOutName;
    }

    if (VerFlag) {
        IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	return 0;
    }

    if (!InNameFlag)
        InName = NULL;
    PObj = IPOBJLoadFile(InName, WarningFlag, WhiteDiffuseTextureFlag, 
        IgnoreFullTranspFlag, ForceSmoothingFlag);
    if (PObj != NULL) {
        if (!OutNameFlag)
            f = stdout;
        else
            f = fopen(OutName, "w");
        IPPutObjectToFile(f, PObj, FALSE);
        if (OutNameFlag)
            fclose(f);
    }
    else
        return 1;

    return 0;
}
