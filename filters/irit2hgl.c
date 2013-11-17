/*****************************************************************************
* Filter to convert IRIT data files to an HPGL file.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Nov 1995    *
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

#define MAX_HPGL_COORD	3000

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Hgl		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char *VersionStr = "Irit2Hgl	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2Hgl t%-XTrans|YTrans!F!F I%-#UIso[:#VIso[:#WIso]]!s f%-PolyOpti|SampTol!d!F F%-PolyOpti|FineNess!d!F M%- G%- T%- a%-AnimTime!F i%- o%-OutName!s z%- DFiles!*s";

IRIT_STATIC_DATA short Colors[IG_MAX_COLOR + 1] = /* Pen indices. */
{
    1,  /* 0. BLACK */
    5,  /* 1. BLUE */
    3,  /* 2. GREEN */
    7,  /* 3. CYAN */
    2,  /* 4. RED */
    6,  /* 5. MAGENTA */
    4,  /* 6. BROWN */
    1,  /* 7. LIGHTGREY */
    1,  /* 8. DARKGRAY */
    5,  /* 9. LIGHTBLUE */
    3,  /* 10. LIGHTGREEN */
    7,  /* 11. LIGHTCYAN */
    2,  /* 12. LIGHTRED */
    6,  /* 13. LIGHTMAGENTA */
    4,  /* 14. YELLOW */
    1   /* 15. WHITE */
};

IRIT_STATIC_DATA const char
    *OutFileName = "irit2Hgl.out";

IRIT_STATIC_DATA FILE
    *OutputFile = NULL;

IRIT_STATIC_DATA IrtRType
    GlblXTranslate = 0.0,
    GlblYTranslate = 0.0;

static void DumpDataForHpgl(const char *FileName, IPObjectStruct *PObjects);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void DumpOneObject(FILE *f, IPObjectStruct *PObject);
static void DumpOnePoly(FILE *f, IPPolygonStruct *PPolygon, int IsPolygon);
static int *MapPoint(IrtRType *Pt);
static void Irit2HglExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2Hgl - Read command line and do what is needed...	     M
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
	NumOfIsolinesFlag = FALSE,
	CrvOptimalPolyFlag = FALSE,
	SrfOptimalPolyFlag = FALSE,
	VerFlag = FALSE,
	OutFileFlag = FALSE,
	TranslateFlag = FALSE,
	NumFiles = 0;
    char
        *StrNumOfIsolines = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs (argc, argv, CtrlStr,
			    &TranslateFlag, &GlblXTranslate, &GlblYTranslate,
			    &NumOfIsolinesFlag, &StrNumOfIsolines,
			    &CrvOptimalPolyFlag, &IPFFCState.CrvApproxMethod,
			    &IPFFCState.CrvApproxTolSamples,
			    &SrfOptimalPolyFlag, &IPFFCState.OptimalPolygons,
			    &IPFFCState.FineNess,
			    &IPFFCState.DrawFFMesh, &IPFFCState.DrawFFGeom,
			    &IPFFCState.Talkative, &HasTime, &CurrentTime,
			    &IPFFCState.ShowInternal, &OutFileFlag, &OutFileName,
			    &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2HglExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2HglExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2HglExit(1);
    }

    IPFFCState.DumpObjsAsPolylines = !SrfOptimalPolyFlag;
    IPFFCState.ComputeNrml = FALSE;      /* No need for normals of vertices. */

    if (NumOfIsolinesFlag && StrNumOfIsolines != NULL) {
	if (sscanf(StrNumOfIsolines, "%d:%d:%d",
		   &IPFFCState.NumOfIsolines[0],
		   &IPFFCState.NumOfIsolines[1],
		   &IPFFCState.NumOfIsolines[2]) != 3) {
	    if (sscanf(StrNumOfIsolines, "%d:%d",
		       &IPFFCState.NumOfIsolines[0],
		       &IPFFCState.NumOfIsolines[1]) != 2) {
		if (sscanf(StrNumOfIsolines, "%d",
			   &IPFFCState.NumOfIsolines[1]) != 1) {
		    IRIT_WARNING_MSG(
			    "Number(s) of isolines (-I) cannot be parsed.\n");
		    GAPrintHowTo(CtrlStr);
		    Irit2HglExit(1);
		}
		else {
		    IPFFCState.NumOfIsolines[2] =
			IPFFCState.NumOfIsolines[1] =
			    IPFFCState.NumOfIsolines[0];
		}
	    }
	    else {
		IPFFCState.NumOfIsolines[2] = IPFFCState.NumOfIsolines[0];
	    }

	}
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2HglExit(1);
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

    DumpDataForHpgl(OutFileFlag ? OutFileName : NULL, PObjects);

    Irit2HglExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for Hpgl into FileName (stdout if NULL).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName:   Where output should go to.                                   *
*   PObjects:   To dump into file.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForHpgl(const char *FileName, IPObjectStruct *PObjects)
{
    IrtHmgnMatType UnitMat;

    if (FileName != NULL) {
	if ((OutputFile = fopen(FileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", FileName);
	    Irit2HglExit(2);
	}
    }
    else
	OutputFile = stdout;

    fprintf(OutputFile, "SP1;IP-%d,-%d,%d,%d;\nSC0,%d,0,%d;\n",	   /* Black. */
	    MAX_HPGL_COORD, MAX_HPGL_COORD, MAX_HPGL_COORD, MAX_HPGL_COORD,
	    MAX_HPGL_COORD, MAX_HPGL_COORD);

    MatGenUnitMat(UnitMat);
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpOneTraversedObject);

    fclose(OutputFile);
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
    int Color;
    IPObjectStruct *PObjs;

    if (IP_IS_FFGEOM_OBJ(PObj))
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
	if ((Color = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR)
	    fprintf(OutputFile, "SP%d;\n", Colors[Color]);
	else
	    fprintf(OutputFile, "SP1;\n");			   /* Black. */

        DumpOneObject(OutputFile, PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to file f.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PObject:      Object to dump to file f.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneObject(FILE *f, IPObjectStruct *PObject)
{
    IPPolygonStruct
	*PList = NULL;

    switch (PObject -> ObjType) {
	case IP_OBJ_POLY:
	    if (IP_IS_POLYGON_OBJ(PObject) ||
		IP_IS_POLYLINE_OBJ(PObject))
	        PList = PObject -> U.Pl;
	    break;
	case IP_OBJ_CURVE:
	case IP_OBJ_SURFACE:
	    IRIT_FATAL_ERROR("Curves and surfaces should have been converted to polylines.");
	    break;
	default:
	    break;
    }

    while (PList) {
	DumpOnePoly(f, PList, IP_IS_POLYGON_OBJ(PObject));
	PList =	PList -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon/line.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   f:            File to dump object to.                                    *
*   PPolygon:     Poly to dump to file f.                                    *
*   IsPolygon:    TRUE for polygon, FALSE otherwise.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOnePoly(FILE *f, IPPolygonStruct *PPolygon, int IsPolygon)
{
    int Length;
    IPVertexStruct
	*VList = PPolygon -> PVertex;

    Length = 0;
    do {
	int *MappedPt = MapPoint(VList -> Coord);

	if (!IPFFCState.ShowInternal && IP_IS_INTERNAL_VRTX(VList)) {
	    if (Length++ > 0) {
		fprintf(f, "PA%d,%d;PU;", MappedPt[0], MappedPt[1]);
		Length = 0;
	    }
	}
	else {
	    if (Length++ == 0)
		fprintf(f, "PA%d,%d;PD;", MappedPt[0], MappedPt[1]);
	    else
		fprintf(f, "PA%d,%d;", MappedPt[0], MappedPt[1]);
	}
	if (Length % 4 == 0)
	    fprintf(f, "\n");

	VList = VList -> Pnext;
    }
    while (VList != NULL && VList != PPolygon -> PVertex);
    if (Length > 0) {
	if (IsPolygon) {
	    int *MappedPt = MapPoint(PPolygon -> PVertex -> Coord);

	    fprintf(f, "PA%d,%d;PU;\n", MappedPt[0], MappedPt[1]);
	}
	else
	    fprintf(f, "PU;\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Maps the given E3 point.                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:        Point to map.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *:   Mapped point, in local static place (for up to 3 calls).   *
*****************************************************************************/
static int *MapPoint(IrtRType *Pt)
{
    IRIT_STATIC_DATA int IMappedPts[3][3],
	Count = 0;
    int *IMappedPt = IMappedPts[Count++],
	XTranslate = (int) (GlblXTranslate * MAX_HPGL_COORD),
	YTranslate = (int) (-GlblYTranslate * MAX_HPGL_COORD);

    if (Count >= 3)
	Count = 0;

    IMappedPt[0] = (int) (Pt[0] * MAX_HPGL_COORD + XTranslate);
    IMappedPt[1] = (int) (Pt[1] * MAX_HPGL_COORD + YTranslate);

    return IMappedPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Hgl exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2HglExit(int ExitCode)
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
