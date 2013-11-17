/*****************************************************************************
* Filter to convert IRIT data files to IGES files.	        	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber	 			 Ver 1.0, May 1998   *
*****************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "geom_lib.h"

IRIT_STATIC_DATA const char
#ifdef NO_CONCAT_STR
    *VersionStr =
	"Irit2IGS		Version 11,	     Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
    *VersionStr = "irit2igs	" IRIT_VERSION ",   Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR*/

IRIT_STATIC_DATA const char
    *CtrlStr = "Irit2igs m%- o%-OutName!s t%-AnimTime!F E%- u%- z%- IritFile!*s";

static void Irit2IgesExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2igs - Read command line and do what is needed...	     M
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
	MoreFlag = 1,
        HasTime = FALSE,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
	ForceUnitMat = FALSE,
	DumpEucTrimCrvs = FALSE,
	NumFiles = 0;
    char
	*IgesFileName = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    IrtHmgnMatType CrntViewMat;
    IPObjectStruct *PObjects;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &MoreFlag,
			   &OutFileFlag, &IgesFileName,
			   &HasTime, &CurrentTime, &DumpEucTrimCrvs,
			   &ForceUnitMat, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2IgesExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2IgesExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2IgesExit(1);
    }

    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2IgesExit(1);
    PObjects = IPResolveInstances(PObjects);

    if (HasTime)
	GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    if (ForceUnitMat) {
	MatGenUnitMat(CrntViewMat);
    }
    else {
	if (IPWasPrspMat)
	    MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
	else
	    IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    }

    IPIgesSaveFileSetup(DumpEucTrimCrvs);

    IPIgesSaveFile(PObjects, CrntViewMat,
		   OutFileFlag ? IgesFileName : NULL, MoreFlag);

    Irit2IgesExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2igs exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2IgesExit(int ExitCode)
{
    exit(ExitCode);
}
