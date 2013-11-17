/*****************************************************************************
* Filter to convert IRIT data files to Geom View OFF format.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, May 1998    *
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

#define SAME_VRTX_DEF_EPS	1e-4

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2off		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Off	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2off l%- 4%- n%- F%-PolyOpti|FineNess!d!F E%-VrtxEps!F o%-OutName!s m%- z%- DFiles!*s";

IRIT_STATIC_DATA IrtRType
    GlblSameVrtxEps = SAME_VRTX_DEF_EPS;
IRIT_STATIC_DATA int
    GlblDumpVrtxNormal = FALSE;

static void DumpDataForOff(IPObjectStruct *PObjects, char *OutFileName);

static void Irit2OffExit(int ExitCode);

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
	OutFileFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL,
        *OutFileName = NULL;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat,
			   &GlblDumpVrtxNormal,
			   &SrfFineNessFlag, &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &EpsFlag, &GlblSameVrtxEps,
			   &OutFileFlag, &OutFileName, &IPFFCState.Talkative,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {

	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2OffExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2OffExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2OffExit(1);
    }

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("%s triangles per flat will be created.\n",
		             IPFFCState.FourPerFlat ? "Four" : "Two");

    /* Get the data files: */
    IPSetFlattenObjects(TRUE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2OffExit(1);
    PObjects = IPResolveInstances(PObjects);

    if (IPWasPrspMat)
	MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    IPTraverseObjListHierarchy(PObjects, CrntViewMat, IPMapObjectInPlace);

    DumpDataForOff(PObjects, OutFileName);

    Irit2OffExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for OFF to OutFileName.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForOff(IPObjectStruct *PObjects, char *OutFileName)
{
    int i,
        VIndex = 0,
	TotalVertices = 0,
	TotalPolys = 0;
    IrtRType RGBColor[3];
    IrtVecType *Vertices, *Normals;
    IPVertexStruct *PVertex;
    IPPolygonStruct *PPoly;
    IPObjectStruct *PObj;
    FILE
	*f = NULL;

    RGBColor[0] = RGBColor[1] = RGBColor[2] = 1.0;     /* Default to white. */

    if (OutFileName != NULL) {
	if ((f = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    Irit2OffExit(2);
	}
    }
    else
	f = stdout;

    /* Process freeform geometry into polys. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	if (IP_IS_FFGEOM_OBJ(PObj))
	    PObjects = IPConvertFreeForm(PObj, &IPFFCState);/*Cnvrt inplace.*/
    }

    /* Count how many polygons/vertices we have in this data set and print. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	if (!IP_IS_POLY_OBJ(PObj)) {
	    IRIT_WARNING_MSG_PRINTF("Non polygonal object \"%s\" ignored.\n",
		                    IP_GET_OBJ_NAME(PObj));
	    continue;
	}

	for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
	    TotalPolys++;

	    TotalVertices += IPVrtxListLen(PPoly -> PVertex);
	}
    }

    if (TotalVertices == 0) {
	IRIT_WARNING_MSG("Zero vertices were found.  No OFF file will be created.\n");
	exit(0);
    }
    if (TotalPolys == 0) {
	IRIT_WARNING_MSG("Zero polygons were found.  No OFF file will be created.\n");
	exit(0);
    }

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("%d vertices in %d polygons will be processed...\n",
		             TotalVertices, TotalPolys);

    /* Allocate a data structure to hold all vertices and pack similar ones, */
    /* by comparing current vertex with all previous ones in O(n^2)...	     */
    Vertices = IritMalloc(sizeof(IrtVecType) * TotalVertices);
    Normals = IritMalloc(sizeof(IrtVecType) * TotalVertices);
    VIndex = 0;
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	int Red, Green, Blue;

	if (!IP_IS_POLY_OBJ(PObj))
	    continue;

	RGBColor[0] = RGBColor[1] = RGBColor[2] = 1.0;  /* Default to white. */
	if (AttrGetObjectRGBColor2(PObj, NULL, &Red, &Green, &Blue)) {
	    RGBColor[0] = Red / 255.0;
	    RGBColor[1] = Green / 255.0;
	    RGBColor[2] = Blue / 255.0;
	}

	for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
	    for (PVertex = PPoly -> PVertex;
		 PVertex != NULL;
		 PVertex = PVertex -> Pnext) {
		for (i = 0; i < VIndex; i++) {
		    if (IRIT_PT_APX_EQ_EPS(PVertex -> Coord, Vertices[i],
				      GlblSameVrtxEps)) {
			break;
		    }
		}
		if (i == VIndex) {
		    /* It is a new vertex - add it to data structure. */
		    IRIT_PT_COPY(Vertices[i], PVertex -> Coord);
		    IRIT_VEC_COPY(Normals[i], PVertex -> Normal);
		    VIndex++;
		}

		/* Now it must be in the new data structure in index VIndex. */
		AttrSetIntAttrib(&PVertex -> Attr, "_VertexIndex", i);
	    }
	}
    }

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("\t%d vertices in %d polygons will be output\n",
		             VIndex, TotalPolys);

    fprintf(f, "OFF\n# Automatically created from IRIT solid modeller data using Irit2Off\n");

    fprintf(f, "%d %d 0\n\n", VIndex, TotalPolys);

    /* Dump the vertices, one per line. */
    for (i = 0; i < VIndex; i++) {
        if (GlblDumpVrtxNormal) {
	    fprintf(f, "%f %f %f   %f %f %f\n",
		    Vertices[i][0], Vertices[i][1], Vertices[i][2],
		    Normals[i][0], Normals[i][1], Normals[i][2]);
	}
	else
	    fprintf(f, "%f %f %f\n",
		    Vertices[i][0], Vertices[i][1], Vertices[i][2]);
    }

    IRIT_INFO_MSG("\n");

    /* Count how many polygons/vertices we have in this data set and print. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	if (!IP_IS_POLY_OBJ(PObj))
	    continue;

	for (PPoly = PObj -> U.Pl; PPoly != NULL; PPoly = PPoly -> Pnext) {
	    int NumVrtcs = IPVrtxListLen(PPoly -> PVertex);

	    fprintf(f, "%d\t", NumVrtcs);

	    for (PVertex = PPoly -> PVertex;
		 PVertex != NULL;
		 PVertex = PVertex -> Pnext) {
		fprintf(f, " %d", AttrGetIntAttrib(PVertex -> Attr,
						   "_VertexIndex"));
	    }
	    fprintf(f, "\t%f %f %f\n", RGBColor[0], RGBColor[1], RGBColor[2]);
	}
    }

    IritFree(Vertices);
    IritFree(Normals);
    fclose(f);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Off exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2OffExit(int ExitCode)
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
