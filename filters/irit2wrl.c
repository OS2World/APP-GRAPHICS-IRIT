/*****************************************************************************
* Filter to convert IRIT data files to VRML  format.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Made by:  Gershon Elber 				Ver 1.0, Oct 1998    *
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

#define DUMMY_RESOLUTION	10

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "irit2wrl		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "irit2wrl	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT	", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2wrl l%- 4%- u%- F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampTol!d!F o%-OutName!s T%- z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *OutFileName = "iritvrml.wrl";

IRIT_STATIC_DATA IrtHmgnMatType
    GlblCrntViewMat;

static void ProcessObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void Irit2wrlExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2wrl - Read command line and do what is needed...	     M
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
    int Handler, Error,
	SrfFineNessFlag = FALSE,
	CrvFineNessFlag = FALSE,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
        ForceUnitMatFlat = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;
    IPObjectStruct *PObjects;
    FILE *VrmlFile;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IPFFCState.ComputeUV = TRUE;

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat,
			   &ForceUnitMatFlat,
			   &SrfFineNessFlag, &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess,
			   &CrvFineNessFlag, &IPFFCState.CrvApproxMethod,
			   &IPFFCState.CrvApproxTolSamples,
			   &OutFileFlag, &OutFileName,
			   &IPFFCState.Talkative,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2wrlExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2wrlExit(0);
    }

    if (IPFFCState.LinearOnePolyFlag) {
	CagdSetLinear2Poly(CAGD_ONE_POLY_PER_COLIN);
    }
    else
        CagdSetLinear2Poly(CAGD_REG_POLY_PER_LIN);

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2wrlExit(1);
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    IPSetVrmlExternalMode(FALSE);
    IPSetPolyListCirc(TRUE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2wrlExit(1);
    PObjects = IPResolveInstances(PObjects);

    /* If more than one object in input stream - chain into one list object. */
    if (PObjects != NULL && PObjects -> Pnext != NULL)
	PObjects = IPObjLnkListToListObject(PObjects);

    if (ForceUnitMatFlat) {
	MatGenUnitMat(GlblCrntViewMat);
    }
    else {
	if (IPWasPrspMat)
	    MatMultTwo4by4(GlblCrntViewMat, IPViewMat, IPPrspMat);
	else
	    IRIT_GEN_COPY(GlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    }

    /* Convert freeform to polygon/lines. */
    IPTraverseObjListHierarchy(PObjects, GlblCrntViewMat, ProcessObject);

    /* Dump the data as a VRML data (Resolution is irreleveant as only       */
    /* polygons are expected at this time anyway.			     */
    if (OutFileFlag) {
        if ((VrmlFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open VRML file \"%s\" for writing\n",
		                    OutFileName);
	    Irit2wrlExit(1);
	}
    }
    else
        VrmlFile = stdout;
	  
    Handler = IPOpenStreamFromVrml(VrmlFile, FALSE, FALSE, FALSE);

    IPPutVrmlViewPoint(Handler, &GlblCrntViewMat, 0);

    IPPutObjectToHandler(Handler, PObjects);

    IPCloseStream(Handler, TRUE);

    Irit2wrlExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy.  Thisfunction is employed for processing   *
* all freeform shapes into polygons and polylines.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    if (IP_IS_FFGEOM_OBJ(PObj)) {
        IPConvertFreeForm(PObj, &IPFFCState);	       /* Convert in place. */

	if (PObj -> Pnext) {
	    /* A memory leak as we do not free the old PObj and new root... */
	    *PObj = *IPObjLnkListToListObject(PObj);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* irit2wrl exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2wrlExit(int ExitCode)
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
