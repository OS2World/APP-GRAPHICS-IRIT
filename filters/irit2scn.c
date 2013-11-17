/*****************************************************************************
* Filter to convert IRIT data files to SCN format.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Made by:  Gershon Elber 						     *
* Modified by:  Antonio Costa				Ver 1.0, Apr 1992    *
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

#define DIST_IRIT_EPS	2e-4
#define SIZE_IRIT_EPS	1e-5

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Scn		Version 11,	Antonio Costa,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Scn	" IRIT_VERSION ",	Antonio Costa,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT	", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2scn l%- 4%- F%-PolyOpti|FineNess!d!F o%-OutName!s g%- T%-  t%-AnimTime!F z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *OutFileName = "irit2scn";

IRIT_STATIC_DATA FILE
    *OutputFileGeom = NULL,
    *OutputFileScn = NULL;

IRIT_STATIC_DATA int
    GlblTotalPolys = 0,
    GlblDumpOnlyGeometry = FALSE;

static void DumpDataForSCN(IPObjectStruct *PObjects);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static int DumpOneObject(FILE *FScn, FILE *FGeom, IPObjectStruct *PObject);
static int DumpOnePolygon(FILE *f,
			  IPPolygonStruct *PPolygon,
			  int IsPolygon);
static void Irit2ScnExit(int ExitCode);

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
	HasTime = FALSE,
	FineNessFlag = FALSE,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
	NumFiles = 0;
    char Line[IRIT_LINE_LEN_LONG], *p,
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &FineNessFlag,
			   &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &OutFileFlag,
			   &OutFileName, &GlblDumpOnlyGeometry,
			   &IPFFCState.Talkative, &HasTime, &CurrentTime,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2ScnExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2ScnExit(0);
    }

    if (IPFFCState.LinearOnePolyFlag) {
	CagdSetLinear2Poly(CAGD_ONE_POLY_PER_COLIN);
    }
    else
        CagdSetLinear2Poly(CAGD_REG_POLY_PER_LIN);

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2ScnExit(1);
    }

    if (!OutFileFlag) {		/* Pick the first input name as output name. */
	strcpy(Line, FileNames[0]);
	if ((p = strstr(Line, ".itd")) != NULL)	    /* Remove old file type. */
	    *p = 0;
	OutFileName = IritStrdup(Line);
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2ScnExit(1);
    PObjects = IPResolveInstances(PObjects);
    if (HasTime)
	GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    if (IPWasPrspMat)
	MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    IPTraverseObjListHierarchy(PObjects, CrntViewMat, IPMapObjectInPlace);

    DumpDataForSCN(PObjects);

    Irit2ScnExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for SCN to stdout.         	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForSCN(IPObjectStruct *PObjects)
{
    IRIT_STATIC_DATA char *Header1[] = {
	"%",
	"% This file was automatically created from IRIT solid modeller data",
	"% using Irit2Scn - IRIT to SCN filter.",
	"%",
	"% (c) Copyright 1992 Antonio Costa, Non commercial use only.",
	"%",
	"",
	NULL
    };
    IRIT_STATIC_DATA char *Header2[] = {
	"",
	"eye  0 0 10",
	"look 0 0 0",
	"up   0 1 0",
	"fov 6",
	"",
	"light point 10 30 10",
	"",
	NULL
    };
    int i;
    char Line[128];
    IrtHmgnMatType UnitMat;

    sprintf(Line, "%s.scn", OutFileName);
    if (!GlblDumpOnlyGeometry) {
	if ((OutputFileScn = fopen(Line, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
	    Irit2ScnExit(2);
	}
    }
    else
	OutputFileScn = NULL;

    sprintf(Line, "%s.geom", OutFileName);
    if ((OutputFileGeom = fopen(Line, "w")) == NULL) {
#	if defined(OS2GCC) || defined(__WINNT__)
	    sprintf(Line, "%s.geo", OutFileName);
	    if ((OutputFileGeom = fopen(Line, "w")) == NULL)
#	endif /* OS2GCC || __WINNT__ */
	    {
		IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
		Irit2ScnExit(2);
	    }
    }

    if (OutputFileScn != NULL)
	for (i = 0; Header1[i] != NULL; i++)
	    fprintf(OutputFileScn, "%s\n", Header1[i]);
    for (i = 0; Header1[i] != NULL; i++)
	fprintf(OutputFileGeom, "%s\n", Header1[i]);

    MatGenUnitMat(UnitMat);
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpOneTraversedObject);

    if (OutputFileScn != NULL) {
	fprintf(OutputFileScn, "#include \"%s\"\n", Line);
	for (i = 0; Header2[i] != NULL; i++)
	    fprintf(OutputFileScn, "%s\n", Header2[i]);
	fclose(OutputFileScn);
    }

    fclose(OutputFileGeom);

    IRIT_INFO_MSG_PRINTF("\nTotal number of polygons - %d\n", GlblTotalPolys);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IPObjectStruct *PObjs;

    if (IP_IS_FFGEOM_OBJ(PObj))
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext)
	GlblTotalPolys += DumpOneObject(OutputFileScn, OutputFileGeom, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to files.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FScn:         SCN file to dump object to.                                *
*   FGeom:        Geometry file to dump object to.                           *
*   PObject:      Object to dump to file f.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int DumpOneObject(FILE *FScn, FILE *FGeom, IPObjectStruct *PObject)
{
    IRIT_STATIC_DATA int
	ObjectSeqNum = 1;
    int i, RGBIColor[3],
        PolyCount = 0,
	HasColor = FALSE,
	HasSrfProp = FALSE;
    const char *p;
    char Name[IRIT_LINE_LEN], SrfPropString[IRIT_LINE_LEN_LONG];
    IrtRType RGBColor[3];
    IPPolygonStruct *PList;

    if (!IP_IS_POLY_OBJ(PObject) ||
	!IP_IS_POLYGON_OBJ(PObject) ||
	PObject -> U.Pl == NULL)
	return 0;

    PList = PObject -> U.Pl;

    if (!IP_VALID_OBJ_NAME(PObject))
	sprintf(Name, "Object_%d", ObjectSeqNum);
    else
	strcpy(Name, IP_GET_OBJ_NAME(PObject));

    fprintf(FGeom, "begin %% %s\n", Name);

    if (AttrGetObjectRGBColor2(PObject, NULL,
			       &RGBIColor[0], &RGBIColor[1], &RGBIColor[2])) {
	HasColor = TRUE;
	for (i = 0; i < 3; i++)
	    RGBColor[i] = RGBIColor[i];
    }

    if ((p = AttrGetObjectStrAttrib(PObject, "SCNsurface")) != NULL) {
	HasSrfProp = TRUE;
	strcpy(SrfPropString, p);
    }

    if (HasColor || HasSrfProp) {
	if (FScn != NULL) {
	    fprintf(FScn, "#define Surface_%s \\\n", Name);
	    if (HasColor && !HasSrfProp) {
		for (i = 0; i < 3; i++)
		    RGBColor[i] /= 255.0;

		fprintf(FScn, "\tsurface matte %g %g %g\n",
			RGBColor[0],
			RGBColor[1],
			RGBColor[2]);
	    }
	    else if (!HasColor && HasSrfProp) {
		fprintf(FScn, "\t%s\n", SrfPropString);
	    }
	    else {
		for (i = 0; i < 3; i++)
		    RGBColor[i] /= 255.0;

		fprintf(FScn, "\t%s\n", SrfPropString);
		fprintf(FScn, "\t%% color %g %g %g\n",
			RGBColor[0],
			RGBColor[1],
			RGBColor[2]);
	    }
	}

	fprintf(FGeom, "Surface_%s\n", Name);
    }

    if ((p = AttrGetObjectStrAttrib(PObject, "SCNrefraction")) != NULL) {
	if (FScn != NULL)
	    fprintf(FScn, "#define Refraction_%s %s\n", Name, p);
    }

    if ((p = AttrGetObjectStrAttrib(PObject, "SCNtexture")) != NULL) {
	if (FScn != NULL) {
	    fprintf(FScn, "#define Texture_%s %s\n", Name, p);
	    fprintf(FGeom, "Texture_%s\n", Name);
	}
    }

    while (PList) {
	PolyCount += DumpOnePolygon(FGeom, PList, IP_IS_POLYGON_OBJ(PObject));
	PList =	PList -> Pnext;
    }
    fprintf(FGeom, "end\n");

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("Converting \"%s\" - %d triangles.\n",
		             Name, PolyCount);

    fprintf(FGeom, "\n");
    if (FScn != NULL)
	fprintf(FScn, "\n");

    ObjectSeqNum++;

    return PolyCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FGeom:        Geometry file to dump object to.                           *
*   PPolygon:     Polygon to dump to file f.                                 *
*   IsPolygon:    Is it a polygon or a polyline?                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of triangles.                                       *
*****************************************************************************/
static int DumpOnePolygon(FILE *FGeom,
			  IPPolygonStruct *PPolygon,
			  int IsPolygon)
{
    int i,
	TriCount = 0;
    IrtRType *Nrmls[3], *Pts[3], Normal[3], Vec1[3], Vec2[3];
    IPVertexStruct *VFirst, *V1, *V2,
	*VList = PPolygon -> PVertex;

    if (VList == NULL)
	return 0;

    if (!GMIsConvexPolygon2(PPolygon)) {
	IRIT_STATIC_DATA int
	    Printed = FALSE;

	if (!Printed) {
	    IRIT_WARNING_MSG(
		    "\nWARNING: Non convex polygon(s) might be in data (see CONVEX in IRIT),\n\t\t\t\toutput can be wrong as the result!\n");
	    Printed = TRUE;
	}
    }

    if (IsPolygon) {
	VFirst = VList;
	V1 = VFirst -> Pnext;
	V2 = V1 -> Pnext;

	while (V2 != NULL) {
	    Pts[0] = VFirst -> Coord;
	    Pts[1] = V1 -> Coord;
	    Pts[2] = V2 -> Coord;

	    /* Test for two type of degeneracies. Make sure that no two  */
	    /* points in the triangle are the same and that they are     */
	    /* not collinear.						 */
	    if (!IRIT_PT_APX_EQ(Pts[0], Pts[1]) &&
		!IRIT_PT_APX_EQ(Pts[0], Pts[2]) &&
		!IRIT_PT_APX_EQ(Pts[1], Pts[2])) {

		IRIT_PT_SUB(Vec1, Pts[0], Pts[1]);
		IRIT_PT_SUB(Vec2, Pts[1], Pts[2]);
		IRIT_PT_NORMALIZE(Vec1);
		IRIT_PT_NORMALIZE(Vec2);
		IRIT_CROSS_PROD(Normal, Vec1, Vec2);

		if (IRIT_PT_LENGTH(Normal) > SIZE_IRIT_EPS) {
		    IRIT_PT_NORMALIZE(Normal);

		    Nrmls[0] = VFirst -> Normal;
		    Nrmls[1] = V1 -> Normal;
		    Nrmls[2] = V2 -> Normal;

		    if (IRIT_DOT_PROD(Normal, Nrmls[0]) < -SIZE_IRIT_EPS ||
			IRIT_DOT_PROD(Normal, Nrmls[1]) < -SIZE_IRIT_EPS ||
			IRIT_DOT_PROD(Normal, Nrmls[2]) < -SIZE_IRIT_EPS) {
			IRIT_SWAP(IrtRType *, Pts[1], Pts[2]);
			IRIT_SWAP(IrtRType *, Nrmls[1], Nrmls[2]);
			IRIT_PT_SCALE(Normal, -1.0);
		    }

		    /* Make sure all normals are set properly: */
		    if (IRIT_DOT_PROD(Nrmls[0],
				 Nrmls[0]) < SIZE_IRIT_EPS)
		        IRIT_PT_COPY(Nrmls[0], Normal);
		    if (IRIT_DOT_PROD(Nrmls[1],
				 Nrmls[1]) < SIZE_IRIT_EPS)
		        IRIT_PT_COPY(Nrmls[1], Normal);
		    if (IRIT_DOT_PROD(Nrmls[2],
				 Nrmls[2]) < SIZE_IRIT_EPS)
		        IRIT_PT_COPY(Nrmls[2], Normal);

		    TriCount++;

		    for (i = 0; i < 3; i++)
		      fprintf(FGeom,
			      "%s %10.7f %10.7f %10.7f  %9.6f %9.6f %9.6f\n",
			      i == 0 ? "triangle normal\n\t" : "\t",
				  Pts[i][0],
				  Pts[i][1],
				  Pts[i][2],
				  Nrmls[i][0],
				  Nrmls[i][1],
			          Nrmls[i][2]);
		}
	    }

	    V1 = V2;
	    V2 = V2 -> Pnext;
	}
    }

    return TriCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Scn exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2ScnExit(int ExitCode)
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
