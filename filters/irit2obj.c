/*****************************************************************************
* Filter to convert IRIT .irt files to Wavefront's OBJ data files. 	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Nadav Shragai			Ver 1.0, October 2009        *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA char
    *VersionStr = "obj2irit		Version 10a,		Gershon Elber,\n\
	 (C) Copyright 1989-2010 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA char
    *VersionStr = "irit2obj	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

/* w - With warnings or not.                                                 */
/* u - Use unique vertices for each polygon (The default is to use as much as*/
/*     possible the same vertices for different polygons).                   */
/* i - Input file. If missing, use the standard input.                       */
/* o - Output file. Required.                                                */
/* z - Print version.                                                        */
IRIT_STATIC_DATA char
    *CtrlStr = "irit2obj w%- i%-InName!s o!-OutName!s z%- u%-";

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2obj - Read command line and do what is needed...	     M
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
    IPObjectStruct *PObj;
    char *InName, *OutName;
    int Error, 
        VerFlag = FALSE,
        WarningMsgs = FALSE,
        InNameFlag = FALSE,
        OutNameFlag = FALSE,
        UniqueVertices = FALSE;

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &WarningMsgs,
			   &InNameFlag, &InName, &OutNameFlag, &OutName,
			   &VerFlag, &UniqueVertices)) != FALSE) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	return 1;
    }

    if (VerFlag) {
        IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	return 0;
    }

    if (!InNameFlag) {
        char *Extension = "itd";
        FILE *Stdin = stdin;

        PObj = IPGetDataFromFilehandles(&Stdin, 1,&Extension, WarningMsgs, 
					WarningMsgs);
    }
    else
        PObj = IPGetDataFiles((const char **) &InName, 1,
			      WarningMsgs, WarningMsgs);

    if (!OutNameFlag)
        OutName = NULL;
    return !IPOBJSaveFile(PObj, OutName, WarningMsgs, UniqueVertices);
}
