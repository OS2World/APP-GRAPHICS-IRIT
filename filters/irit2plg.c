/*****************************************************************************
* Filter to convert IRIT data files to REND386 (IBMPC only) Plg format.	     *
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
#include "grap_lib.h"
#include "ip_cnvrt.h"
#include "misc_lib.h"

#define GLOBAL_SCALE	200.0  /* Scale obj space -1..1 to -200..200 pixels. */

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Plg		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Plg	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2plg l%- 4%- F%-PolyOpti|FineNess!d!F T%- z%- DFiles!*s";

IRIT_STATIC_DATA int
    GlblDumpVertices = FALSE,
    GlblTotalVertices = 0,
    GlblTotalPolys = 0;

IRIT_STATIC_DATA int PlgColorTable[16] = {
	/* IRIT     PLG   */
    15, /* BLOCK -> WHITE */
    11, /* BLUE */
    7,  /* GREEN */
    8,  /* CYAN */
    1,  /* RED */
    13, /* MAGENTA */
    2,  /* BROWN */
    14, /* LIGHTGRAY */
    14, /* DARKGRAY */
    10, /* LIGHT BLUE */
    9,  /* LIGHT GREEN */
    8,  /* LIGHT CYAN */
    1,  /* LIGHT RED */
    13, /* LIGHT MAGENTA */
    6,  /* YELLOW */
    15, /* WHITE */    
};

IRIT_STATIC_DATA FILE
    *GlblOutFile;

static void FF2PolyInPlace(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void DumpDataForPlg(IPObjectStruct *PObjects, IrtHmgnMatType Mat);
static void DumpOneObject(FILE *f,
			  IPObjectStruct *PObject,
			  int DumpVertices,
			  int *IncVertices);
static void DumpOnePolygon(FILE *f,
			   IPPolygonStruct *PPolygon,
			   int Color,
			   int DumpVertices,
			   int *IncVertices,
			   int IsPolygon);
static void Irit2PlgExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2plg - Read command line and do what is needed...	     M
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
	FineNessFlag = FALSE,
	VerFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat, UnitMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &FineNessFlag,
			   &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &IPFFCState.Talkative,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2PlgExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2PlgExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2PlgExit(1);
    }

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("%s triangles per flat will be created.\n",
		             IPFFCState.FourPerFlat ? "Four" : "Two");

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2PlgExit(1);
    PObjects = IPResolveInstances(PObjects);

    if (IPWasPrspMat)
        MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    IPTraverseObjListHierarchy(PObjects, CrntViewMat, IPMapObjectInPlace);
    IPTraverseObjListHierarchy(PObjects, CrntViewMat, FF2PolyInPlace);

    MatGenUnitMat(UnitMat);
    GlblOutFile = stdout;      /* Change it if you want it into "real" file. */
    GlblDumpVertices = TRUE;
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpDataForPlg);

    if (!IP_VALID_OBJ_NAME(PObjects))
        fprintf(GlblOutFile, "IRIT %d %d\n",
		GlblTotalVertices, GlblTotalPolys);
    else
        fprintf(GlblOutFile, "%s_IRIT %d %d\n",
		IP_GET_OBJ_NAME(PObjects), GlblTotalVertices, GlblTotalPolys);

    GlblDumpVertices = FALSE;
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpDataForPlg);

    fclose(GlblOutFile);
    Irit2PlgExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Maps the object according to the given matrix, in place.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to map, in place, according to Mat.                    *
*   Mat:       Transformation matrix.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FF2PolyInPlace(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    if (IP_IS_FFGEOM_OBJ(PObj))
        IPConvertFreeForm(PObj, &IPFFCState);	       /* Convert in place. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for REND386 to stdout.                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForPlg(IPObjectStruct *PObjects, IrtHmgnMatType Mat)
{
    int IncVertices;
    IPVertexStruct *PVertex;
    IPPolygonStruct *PPoly;
    IPObjectStruct *PObj;

    if (GlblDumpVertices) {
	/* Count how many polygons/vertices we have in data set and print. */
	for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	    if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLYGON_OBJ(PObj))
	        continue;

	    for (PPoly = PObj -> U.Pl;
		 PPoly != NULL;
		 PPoly = PPoly -> Pnext) {
	        GlblTotalPolys++;

		for (PVertex = PPoly -> PVertex;
		     PVertex != NULL;
		     PVertex = PVertex -> Pnext) {
		    GlblTotalVertices++;
		}
	    }
	}

	for (IncVertices = 0, PObj = PObjects;
	     PObj != NULL;
	     PObj = PObj -> Pnext)
	    DumpOneObject(GlblOutFile, PObj, TRUE, &IncVertices);
    }
    else {
        for (IncVertices = 0, PObj = PObjects;
	     PObj != NULL;
	     PObj = PObj -> Pnext)
	    DumpOneObject(GlblOutFile, PObj, FALSE, &IncVertices);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to file f.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PObject:      Object to dump to file f.                                  *
*   DumpVertices: Do we dump vertices now?				     *
*   IncVertices:  Count number of vertices.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneObject(FILE *f,
			  IPObjectStruct *PObject,
			  int DumpVertices,
			  int *IncVertices)
{
    int Color;
    IPPolygonStruct *PList;

    if (!IP_IS_POLY_OBJ(PObject))
	return;

    PList = PObject -> U.Pl;

    if ((Color = AttrGetObjectColor(PObject)) != IP_ATTR_NO_COLOR)
        Color = PlgColorTable[Color];         /* Translates to Plg colors. */
    else
        Color = IG_DEFAULT_COLOR;

    while (PList) {
	DumpOnePolygon(f, PList, Color, DumpVertices, IncVertices,
						IP_IS_POLYGON_OBJ(PObject));
	PList =	PList -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump polygon to.    		                     *
*   PPolygon:     Polygon to dump to file f.                                 *
*   Color:	  Attribute of polygon.					     *
*   DumpVertices: Do we dump vertices now?				     *
*   IncVertices:  Count number of vertices.				     *
*   IsPolygon:    Is it a polygon or a polyline?                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void DumpOnePolygon(FILE *f,
			   IPPolygonStruct *PPolygon,
			   int Color,
			   int DumpVertices,
			   int *IncVertices,
			   int IsPolygon)
{
    int CountVertices;
    IrtRType *Pts;
    IPVertexStruct *V,
	*VList = PPolygon -> PVertex;

    if (VList == NULL)
	return;

    if (DumpVertices && !GMIsConvexPolygon2(PPolygon)) {
	IRIT_STATIC_DATA int
	    Printed = FALSE;

	if (!Printed) {
	    IRIT_WARNING_MSG(
		    "\nWARNING: Non convex polygon(s) might be in data (see CONVEX in IRIT),\n\t\t\t\toutput can be wrong as the result!\n");
	    Printed = TRUE;
	}
    }

    if (IsPolygon) {
	if (DumpVertices) {
	    for (V = VList; V != NULL; V = V -> Pnext) {
		Pts = V -> Coord;
		fprintf(f, "%4d %4d %4d\n",
			(int) (Pts[0] * GLOBAL_SCALE),
			(int) (Pts[1] * GLOBAL_SCALE),
			(int) (Pts[2] * GLOBAL_SCALE));
		(*IncVertices)++;
	    }
	}
	else {
	    for (CountVertices = 0, V = VList; V != NULL; V = V -> Pnext)
	        CountVertices++;
	    fprintf(f, "0x%02xff %d", Color | 0x0010, CountVertices);
	    while (CountVertices-- > 0)
	        fprintf(f, " %d", (*IncVertices)++);
	    fprintf(f, "\n");
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Plg exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2PlgExit(int ExitCode)
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
