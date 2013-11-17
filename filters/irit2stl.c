/*****************************************************************************
* Filter to convert IRIT data files to STL layered manufacturing format.     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0, April 2000   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Stl		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char 
    *VersionStr = "Irit2Stl	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "Irit2Stl l%- 4%- r%- F%-PolyOpti|FineNess!d!F E%-VrtxEps!F s%- S%- o%-OutName!s m%- u%- z%- DFiles!*s";

static void Irit2StlExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2scn - Read command line and do what is needed...	     M
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
	SrfFineNessFlag = FALSE,
	EpsFlag = FALSE,
	VerFlag = FALSE,
        RegularTriang = TRUE,
	MultiObjSplit = 0,
	SplitObjFlag = FALSE,
	SplitObjFileFlag = FALSE,
	OutFileFlag = FALSE,
	ForceUnitMat = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL,
        *OutFileName = NULL;
    IrtRType SameVrtxEps;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &RegularTriang,
			   &SrfFineNessFlag, &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &EpsFlag, &SameVrtxEps,
			   &SplitObjFlag, &SplitObjFileFlag,
			   &OutFileFlag, &OutFileName, &IPFFCState.Talkative,
			   &ForceUnitMat, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2StlExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2StlExit(0);
    }

    if (SplitObjFlag && SplitObjFileFlag) {
	IRIT_WARNING_MSG("Only one of '-s' and '-S' at a time please, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2StlExit(1);
    }
    if (SplitObjFlag)
        MultiObjSplit = 1;
    else if (SplitObjFileFlag)
        MultiObjSplit = 2;
	
    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2StlExit(1);
    }

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("%s triangles per flat will be created.\n",
		             IPFFCState.FourPerFlat ? "Four" : "Two");

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    IPSetPolyListCirc(TRUE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2StlExit(1);
    PObjects = IPResolveInstances(PObjects);

    if ((!IPWasPrspMat && !IPWasViewMat) || ForceUnitMat) {
	MatGenUnitMat(CrntViewMat);
    }
    else {
        if (IPWasPrspMat)
	    MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
	else
	    IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    }

    if (EpsFlag)
	IPSTLSaveSetVrtxEps(SameVrtxEps);

    IPSTLSaveFile(PObjects, CrntViewMat, RegularTriang, MultiObjSplit,
		  OutFileName, TRUE);

    Irit2StlExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Stl exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2StlExit(int ExitCode)
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
