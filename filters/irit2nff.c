/*****************************************************************************
* Filter to convert IRIT data files to NFF format.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan 1992    *
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

#define DEFAULT_KD      " 1.0"
#define DEFAULT_KS      " 0.0"
#define DEFAULT_SHINE   " 0.0"
#define DEFAULT_TRANS   " 0.0"
#define DEFAULT_INDEX   " 0.0"
#define DEFAULT_RGB_COLOR	" 1.0 1.0 1.0"

#define DIST_IRIT_EPS	2e-4
#define SIZE_IRIT_EPS	1e-5

#define STRCAT2(Str1, Str2, Str3) strcat(strcat(Str1, Str2), Str3)

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Nff		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Nff	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2nff l%- 4%- c%- F%-PolyOpti|FineNess!d!F o%-OutName!s T%-  t%-AnimTime!F g%- z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *OutFileName = "irit2nff";

IRIT_STATIC_DATA FILE
    *OutputFileGeom = NULL,
    *OutputFileNff = NULL;

IRIT_STATIC_DATA int
    GlblTotalPolys = 0,
    GlblCPPSupport = FALSE,
    GlblDumpOnlyGeometry = FALSE;

static void DumpDataForNFF(IPObjectStruct *PObjects);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static int DumpOneObject(FILE *FRay, FILE *FGeom, IPObjectStruct *PObject);
static int DumpOnePolygon(FILE *f, IPPolygonStruct *PPolygon, int IsPolygon);
static void Irit2NffExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of irit2nff - Read command line and do what is needed...	     M
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

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &GlblCPPSupport,
			   &FineNessFlag, &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess,
			   &OutFileFlag, &OutFileName, &IPFFCState.Talkative,
			   &HasTime, &CurrentTime, &GlblDumpOnlyGeometry,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2NffExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2NffExit(0);
    }

    if (IPFFCState.LinearOnePolyFlag) {
	CagdSetLinear2Poly(CAGD_ONE_POLY_PER_COLIN);
    }
    else
        CagdSetLinear2Poly(CAGD_REG_POLY_PER_LIN);

    if (GlblDumpOnlyGeometry && !GlblCPPSupport) {
	IRIT_WARNING_MSG("Flag -g makes sense only with -c.\n");
	Irit2NffExit(1);
    }
    
    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2NffExit(1);
    }

    if (!OutFileFlag) {		/* Pick the first input name as output name. */
	strcpy(Line, FileNames[0]);
	if ((p = strstr(Line, ".itd")) != NULL)     /* Remove old file type. */
	    *p = 0;
	OutFileName = IritStrdup(Line);
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2NffExit(1);
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

    DumpDataForNFF(PObjects);

    Irit2NffExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for NFF to stdout.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForNFF(IPObjectStruct *PObjects)
{
    IRIT_STATIC_DATA char *Header1[] = {
	"#",
	"# This file was automatically created from IRIT solid modeller data",
	"# using Irit2Nff - IRIT to NFF filter.",
	"#",
	"#            (c) Copyright 1989-2012 Gershon Elber, Non commercial use only.",
	"#",
	NULL
    };
    IRIT_STATIC_DATA char *Header2[] = {
	"",
	"v",
	"from   0  0 10",
	"at     0  0  0",
	"up     0  1  0",
	"angle  12",
	"hither 0",
	"resolution 512 512",
	"",
	"l      1 1 1",
	"",
	NULL
    };
    int i;
    char Line[128];
    IrtHmgnMatType UnitMat;

    sprintf(Line, "%s.nff", OutFileName);
    if (!GlblDumpOnlyGeometry) {
        if ((OutputFileNff = fopen(Line, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
	    Irit2NffExit(2);
        }
    }
    else
        OutputFileNff = NULL;

    if (GlblCPPSupport) {   /* Separated files for geometry/surface quality. */
	sprintf(Line, "%s.geom", OutFileName);
	if ((OutputFileGeom = fopen(Line, "w")) == NULL) {
#	    if defined(OS2GCC) || defined(__WINNT__)
	        sprintf(Line, "%s.geo", OutFileName);
	        if ((OutputFileGeom = fopen(Line, "w")) == NULL)
#	    endif /* OS2GCC || __WINNT__ */
		{
		    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
		    Irit2NffExit(2);
		}
	}
    }
    else
    	OutputFileGeom = NULL;

    if (OutputFileNff != NULL) {
        if (GlblCPPSupport)
            fprintf(OutputFileNff, "/*\n");
        for (i = 0; Header1[i] != NULL; i++)
	    fprintf(OutputFileNff, "%s\n", Header1[i]);
        if (GlblCPPSupport)
            fprintf(OutputFileNff, "*/\n");
        fprintf(OutputFileNff, "\n");

        for (i = 0; Header2[i] != NULL; i++)
	    fprintf(OutputFileNff, "%s\n", Header2[i]);
    }
    
    if (GlblCPPSupport) {
        fprintf(OutputFileGeom, "/*\n");
	for (i = 0; Header1[i] != NULL; i++)
	    fprintf(OutputFileGeom, "%s\n", Header1[i]);
        fprintf(OutputFileGeom, "*/\n\n");
    }

    MatGenUnitMat(UnitMat);
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpOneTraversedObject);

    if (GlblCPPSupport && OutputFileNff != NULL)
	fprintf(OutputFileNff, "#include \"%s\"\n", Line);

    if (OutputFileNff != NULL)
	fclose(OutputFileNff);
    if (OutputFileGeom != NULL)
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
	GlblTotalPolys += DumpOneObject(OutputFileNff, OutputFileGeom, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to files.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FNff:         NFF file to dump object to.                                *
*   FGeom:        Geometry file to dump object to.                           *
*   PObject:      Object to dump tofile f.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int DumpOneObject(FILE *FNff, FILE *FGeom, IPObjectStruct *PObject)
{
    IRIT_STATIC_DATA int
	ObjectSeqNum = 1;
    int i, Red, Green, Blue,
        PolyCount = 0,
	HasColor = FALSE;
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
	sprintf(Name, "ObjSeq%d", ObjectSeqNum);
    else
	strcpy(Name, IP_GET_OBJ_NAME(PObject));

    SrfPropString[0] = 0;
    if ((p = AttrGetObjectStrAttrib(PObject, "kd")) != NULL)
	STRCAT2(SrfPropString, " ", p);
    else
	strcat(SrfPropString, DEFAULT_KD);
    if ((p = AttrGetObjectStrAttrib(PObject, "ks")) != NULL)
	STRCAT2(SrfPropString, " ", p);
    else
	strcat(SrfPropString, DEFAULT_KS);
    if ((p = AttrGetObjectStrAttrib(PObject, "shine")) != NULL)
	STRCAT2(SrfPropString, " ", p);
    else
	strcat(SrfPropString, DEFAULT_SHINE);
    if ((p = AttrGetObjectStrAttrib(PObject, "trans")) != NULL)
	STRCAT2(SrfPropString, " ", p);
    else
	strcat(SrfPropString, DEFAULT_TRANS);
    if ((p = AttrGetObjectStrAttrib(PObject, "index")) != NULL)
	STRCAT2(SrfPropString, " ", p);
    else
	strcat(SrfPropString, DEFAULT_INDEX);

    if (AttrGetObjectRGBColor2(PObject, NULL, &Red, &Green, &Blue)) {
	HasColor = TRUE;
	RGBColor[0] = Red;
	RGBColor[1] = Green;
	RGBColor[2] = Blue;
    }

    if (FNff != NULL)
        fprintf(FNff, GlblCPPSupport ? "/*\n#\n# %s\n#\n*/\n" : "#\n# %s\n#\n",
	        Name);
    if (GlblCPPSupport) {
	if (FNff != NULL)
	    fprintf(FNff, "#define %s_SRF_PROP ", Name);
	fprintf(FGeom, "/*\n#\n# %s\n#\n*/\n", Name);
	fprintf(FGeom, "f %s_SRF_PROP\n", Name);
    }
    else {
	if (FNff != NULL)
	    fprintf(FNff, "f ");
    }
    if (HasColor) {
	for (i = 0; i < 3; i++)
	    RGBColor[i] /= 255.0;
	if (FNff != NULL)
	    fprintf(FNff, "%7.4f %7.4f %7.4f",
		    RGBColor[0], RGBColor[1], RGBColor[2]);
    }
    else {
	if (FNff != NULL)
	    fprintf(FNff, "%s", DEFAULT_RGB_COLOR);
    }
    if (FNff != NULL)
        fprintf(FNff, " %s\n\n", SrfPropString);

    while (PList) {
	PolyCount += DumpOnePolygon(GlblCPPSupport ? FGeom : FNff,
				    PList, IP_IS_POLYGON_OBJ(PObject));
	PList =	PList -> Pnext;
    }

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("Converting \"%s\" - %d triangles.\n",
		             Name, PolyCount);

    if (FNff != NULL)
	fprintf(FNff, "\n\n");
    if (GlblCPPSupport)
	fprintf(FGeom, "\n\n");

    ObjectSeqNum++;

    return PolyCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Fl:           File to dump polygon to.  		                     *
*   PPolygon:     Polygon to dump to file f.                                 *
*   IsPolygon:    Is it a polygon or a polyline?                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of triangles.                                       *
*****************************************************************************/
static int DumpOnePolygon(FILE *Fl, IPPolygonStruct *PPolygon, int IsPolygon)
{
    int i,
	TriCount = 0;
    IrtRType *Nrmls[3], *Pts[3], Normal[3], Vec1[3], Vec2[3];
    IPVertexStruct *VFirst, *V1, *V2,
	*VList = PPolygon -> PVertex;

    if (Fl == NULL || VList == NULL)
	return 0;

    if (!GMIsConvexPolygon2(PPolygon)) {
	IRIT_STATIC_DATA int Printed = FALSE;

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
	    /* not collinear.					         */
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
		    if (IRIT_DOT_PROD(Nrmls[0], Nrmls[0]) < SIZE_IRIT_EPS)
		        IRIT_PT_COPY(Nrmls[0], Normal);
		    if (IRIT_DOT_PROD(Nrmls[1], Nrmls[1]) < SIZE_IRIT_EPS)
		        IRIT_PT_COPY(Nrmls[1], Normal);
		    if (IRIT_DOT_PROD(Nrmls[2], Nrmls[2]) < SIZE_IRIT_EPS)
		        IRIT_PT_COPY(Nrmls[2], Normal);

		    TriCount++;

		    fprintf(Fl, "pp 3\n");
		    for (i = 0; i < 3; i++)
		      fprintf(Fl,
			      "  %10.7f %10.7f %10.7f  %9.6f %9.6f %9.6f\n",
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
* Irit2Nff exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2NffExit(int ExitCode)
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
