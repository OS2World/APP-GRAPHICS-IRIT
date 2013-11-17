/*****************************************************************************
* Filter to convert IRIT data files to ray shade format.		     *
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

#define DIST_IRIT_EPS	2e-4
#define SIZE_IRIT_EPS	1e-5

#define DEFAULT_POLYLINE_WIDTH  0.05

#define CONE_SIZE	5e-4

#define MIN_RATIO	0.01

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Ray		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Ray	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2ray l%- 4%- G%-GridSize%d F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampTol!d!F o%-OutName!s g%- p%-Zmin|Zmax!F!F P%- M%- T%- t%-AnimTime!F I%-#UIso[:#VIso[:#WIso]]!s s%-ObjSeq#!d z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *GlblOutFileName = "irit2ray";

IRIT_STATIC_DATA FILE
    *OutputFileRay = NULL,
    *OutputFileGeom = NULL;

IRIT_STATIC_DATA int
    GlblTotalPolys = 0,
    GlblDumpPolylines = FALSE,
    GlblDumpOnlyGeomFile = FALSE,
    GlblObjectSeqNum = 1;

IRIT_STATIC_DATA IrtRType
    GlblZBBox[2] = { IRIT_INFNTY, -IRIT_INFNTY },
    GlblPolylineDepthCue[3] = { -IRIT_INFNTY, IRIT_INFNTY };

static void DumpDataForRayShade(IPObjectStruct *PObjects);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static int DumpOneObject(FILE *FRay, FILE *FGeom, IPObjectStruct *PObject);
static int DumpOnePoly(FILE *FGeom,
		       IPPolygonStruct *PPoly,
		       IPObjectStruct *PObj,
		       int ObjNum,
		       char *Name,
		       IrtRType Width);
static int DumpOnePolygon(FILE *f, IPPolygonStruct *PPolygon, char *Name);
static int DumpOnePolyline(FILE *f,
			   IPPolygonStruct *PPolyline,
			   int ObjNum,
			   char *Name,
			   IrtRType Width,
			   int IsPolyline);
static void DumpCone(IrtRType R1,
		     IrtRType X1,
		     IrtRType Y1,
		     IrtRType Z1,
		     IrtRType R2,
		     IrtRType X2,
		     IrtRType Y2,
		     IrtRType Z2,
		     FILE *FGeom,
		     char *Name,
		     int ObjNum);
static void Irit2RayExit(int ExitCode);

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
	SrfFineNessFlag = FALSE,
	CrvFineNessFlag = FALSE,
	VerFlag = FALSE,
	ObjSeqFlag = FALSE,
	OutFileFlag = FALSE,
        BBoxGrid = TRUE,
        GridSize = 5,
	NumOfIsolinesFlag = FALSE,
	NumFiles = 0;
    char Line[IRIT_LINE_LEN_LONG], *p,
	*StrNumOfIsolines = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &BBoxGrid,
			   &GridSize, &SrfFineNessFlag,
			   &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &CrvFineNessFlag,
			   &IPFFCState.CrvApproxMethod,
			   &IPFFCState.CrvApproxTolSamples, &OutFileFlag,
			   &GlblOutFileName, &GlblDumpOnlyGeomFile,
			   &GlblDumpPolylines,
			   &GlblPolylineDepthCue[0],
			   &GlblPolylineDepthCue[1],
			   &IPFFCState.DumpObjsAsPolylines,
			   &IPFFCState.DrawFFMesh, &IPFFCState.Talkative,
			   &HasTime, &CurrentTime,
			   &NumOfIsolinesFlag, &StrNumOfIsolines,
			   &ObjSeqFlag, &GlblObjectSeqNum, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {

	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2RayExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2RayExit(0);
    }

    IPFFCState.BBoxGrid = BBoxGrid ? GridSize : 0;

    if (GlblDumpPolylines ||
	IPFFCState.DrawFFMesh ||
	&IPFFCState.DumpObjsAsPolylines) {
	if (IRIT_APX_EQ(GlblPolylineDepthCue[0], GlblPolylineDepthCue[1])) {
	    IRIT_WARNING_MSG("Cannot compute depth cue if Zmin == Zmax\n");
	    Irit2RayExit(1);
	}
	else {
	    /* Precompute DZ. */
	    GlblPolylineDepthCue[2] =
		GlblPolylineDepthCue[1] - GlblPolylineDepthCue[0]; 
	}
    }

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
		    Irit2RayExit(1);
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

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("%s triangles per flat will be created.\n",
		             IPFFCState.FourPerFlat ? "Four" : "Two");

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2RayExit(1);
    }

    if (!OutFileFlag) {		/* Pick the first input name as output name. */
	strcpy(Line, FileNames[0]);
	if ((p = strstr(Line, ".itd")) != NULL)	    /* Remove old file type. */
	    *p = 0;
	GlblOutFileName = IritStrdup(Line);
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2RayExit(1);
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

    DumpDataForRayShade(PObjects);

    if (IPFFCState.DumpObjsAsPolylines && GlblZBBox[0] < GlblZBBox[1])
	IRIT_INFO_MSG_PRINTF("Z depth cueing of polylines spans [%f : %f]\n",
		             GlblZBBox[0], GlblZBBox[1]);

    Irit2RayExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for Ray Shade to stdout.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForRayShade(IPObjectStruct *PObjects)
{
    IRIT_STATIC_DATA char *Header1[] = {
	"/*",
	" * This file was automatically created from IRIT solid modeller data",
	" * using Irit2ray - IRIT to RayShade filter.",
	" *",
	" *            (c) Copyright 1989-2012 Gershon Elber, Non commercial use only.",
	" */",
	"",
	NULL
    };
    IRIT_STATIC_DATA char *Header2[] = {
	"",
	"eyep   0  0 10",
	"lookp  0  0  0",
	"up     0  1  0",
	"fov 12",
	"",
	"light 1 1 1 point 10 30 10",
	"",
	NULL
    };
    int i;
    char Line[128];
    IrtHmgnMatType UnitMat;

    sprintf(Line, "%s.ray", GlblOutFileName);
    if (!GlblDumpOnlyGeomFile) {
	if ((OutputFileRay = fopen(Line, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
	    Irit2RayExit(2);
	}
    }
    else
	OutputFileRay = NULL;

    sprintf(Line, "%s.geom", GlblOutFileName);
    if ((OutputFileGeom = fopen(Line, "w")) == NULL) {
#	if defined(OS2GCC) || defined(__WINNT__)
	    sprintf(Line, "%s.geo", GlblOutFileName);
	    if ((OutputFileGeom = fopen(Line, "w")) == NULL)
#	endif /* OS2GCC || __WINNT__ */
	    {
		IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
		Irit2RayExit(2);
	    }
    }

    if (OutputFileRay != NULL)
	for (i = 0; Header1[i] != NULL; i++)
	    fprintf(OutputFileRay, "%s\n", Header1[i]);
    for (i = 0; Header1[i] != NULL; i++)
	fprintf(OutputFileGeom, "%s\n", Header1[i]);

    MatGenUnitMat(UnitMat);
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpOneTraversedObject);

    if (OutputFileRay != NULL) {
	fprintf(OutputFileRay, "#include \"%s\"\n", Line);
	for (i = 0; Header2[i] != NULL; i++)
	    fprintf(OutputFileRay, "%s\n", Header2[i]);
	fclose(OutputFileRay);
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
	GlblTotalPolys += DumpOneObject(OutputFileRay, OutputFileGeom, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to files.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FRay:         Ray file to dump object to.                                *
*   FGeom:        Geometry file to dump object to.                           *
*   PObject:      Object to dump to file f.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int DumpOneObject(FILE *FRay, FILE *FGeom, IPObjectStruct *PObject)
{
    IRIT_STATIC_DATA char
        *StrAttribs[] = {
	"specpow",
	"reflect",
	"transp",
	"body",
	"index",
	NULL
    };
    IRIT_STATIC_DATA int ObjNum = 0;
    int i, RGBIColor[3],
        PolyCount = 0,
	HasColor = FALSE,
	HasSrfProp = FALSE;
    const char *p;
    char Name[IRIT_LINE_LEN], SrfPropString[IRIT_LINE_LEN_LONG];
    IrtRType RGBColor[3],
	Width = DEFAULT_POLYLINE_WIDTH;
    IPPolygonStruct *PList;

    if (!IP_IS_POLY_OBJ(PObject) ||
	IP_IS_POINTLIST_OBJ(PObject) ||
	PObject -> U.Pl == NULL)
	return 0;

    ObjNum++;
    PList = PObject -> U.Pl;

    p = IP_GET_OBJ_NAME(PObject);
    if (p == NULL || p[0] == 0 || strcmp(IP_GET_OBJ_NAME(PObject), "_") == 0)
	sprintf(Name, "ObjSeq%d", GlblObjectSeqNum);
    else
	strcpy(Name, p[0] == '_' ? &p[1] : p);

    SrfPropString[0] = 0;
    for (i = 0; StrAttribs[i] != NULL; i++) {
	if ((p = AttrGetObjectStrAttrib(PObject, StrAttribs[i])) != NULL) {
	    strcat(SrfPropString, StrAttribs[i]);
	    strcat(SrfPropString, " ");
	    strcat(SrfPropString, p);
	    strcat(SrfPropString, " ");
	    HasSrfProp = TRUE;
	}
    }

    Width = AttrGetObjectRealAttrib(PObject, "Width");
    if (IP_ATTR_IS_BAD_REAL(Width))
        Width = DEFAULT_POLYLINE_WIDTH;

    if (IPFFCState.BBoxGrid) {
	const char
	    *GridStr = AttrGetObjectStrAttrib(PObject, "GridSize");

	if (GridStr != NULL)
	    fprintf(FGeom, "name RS%s%d grid %s\n", Name, ObjNum, GridStr);
	else
	    fprintf(FGeom, "name RS%s%d grid %d %d %d\n", Name, ObjNum,
		    IPFFCState.BBoxGrid, IPFFCState.BBoxGrid, IPFFCState.BBoxGrid);
    }
    else
	fprintf(FGeom, "name RS%s%d list\n", Name, ObjNum);

    while (PList) {
	PolyCount += DumpOnePoly(FGeom, PList, PObject, ObjNum, Name, Width);
	PList =	PList -> Pnext;
    }
    fprintf(FGeom, "end\n");

    if (IPFFCState.Talkative)
	IRIT_INFO_MSG_PRINTF("Converting \"%s\" - %d triangles.\n",
		             Name, PolyCount);

    if (AttrGetObjectRGBColor2(PObject, NULL,
			       &RGBIColor[0], &RGBIColor[1], &RGBIColor[2])) {
	HasColor = TRUE;
	for (i = 0; i < 3; i++)
	    RGBColor[i] = RGBIColor[i];
    }

    if (HasColor || HasSrfProp) {
	if (FRay != NULL) {
	    fprintf(FRay, "surface RS%sSrfProp%d\n", Name, ObjNum);
	    if (HasColor) {
		for (i = 0; i < 3; i++)
		    RGBColor[i] /= 255.0;

		fprintf(FRay, "\tambient  %7.4f %7.4f %7.4f\n",
			0.1 * RGBColor[0],
			0.1 * RGBColor[1],
			0.1 * RGBColor[2]);
		fprintf(FRay, "\tdiffuse  %7.4f %7.4f %7.4f\n",
			0.7 * RGBColor[0],
			0.7 * RGBColor[1],
			0.7 * RGBColor[2]);
		fprintf(FRay, "\tspecular %7.4f %7.4f %7.4f\n",
			0.8, 0.8, 0.8);
	    }
	    if (HasSrfProp)
		fprintf(FRay, "\t%s\n", SrfPropString);
	}

	fprintf(FGeom, "object RS%sSrfProp%d RS%s%d", Name, ObjNum, Name, ObjNum);
    }
    else
	fprintf(FGeom, "object RS%s%d", Name, ObjNum);

    if ((p = AttrGetObjectStrAttrib(PObject, "texture")) != NULL) {
	char Scale[IRIT_LINE_LEN],
	    *q = strchr(p, ',');

	/* Remove the scaling factor, if any. */
	Scale[0] = 0;
	if (q != NULL) {
	    IrtRType
		R;

	    *q = 0;
	    q++;

#ifdef IRIT_DOUBLE
	    if (sscanf(q, "%lf", &R))
#else
	    if (sscanf(q, "%f", &R))
#endif /* IRIT_DOUBLE */
		sprintf(Scale, " scale %f %f %f", R, R, R);
	}

	if (FRay != NULL)
	    fprintf(FRay, "#define RS%s%dTEXTURE %s%s\n",
		    Name, ObjNum, p, Scale);
	fprintf(FGeom, " texture RS%s%dTEXTURE", Name, ObjNum);
    }
    fprintf(FGeom, "\n\n");
    if (FRay != NULL)
	fprintf(FRay, "\n\n");

    GlblObjectSeqNum++;

    return PolyCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon/line.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   FGeom:        Geometry file to dump poly to.                             *
*   PPoly:        Poly to dump to file f.                                    *
*   PObj:         Object PPoly came from to idetify a polygon from polyline. *
*   ObjNum:	  Number of object. 	                                     *
*   Name:         Name of object.	                                     *
*   Width:	  Of polylinesdumped as skinny cones/spheres.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of Polys.                                           *
*****************************************************************************/
static int DumpOnePoly(FILE *FGeom,
		       IPPolygonStruct *PPoly,
		       IPObjectStruct *PObj,
		       int ObjNum,
		       char *Name,
		       IrtRType Width)
{
    if (IP_IS_POLYGON_OBJ(PObj)) {
	if (IPFFCState.DumpObjsAsPolylines)
	    return DumpOnePolyline(FGeom, PPoly, ObjNum, Name, Width, FALSE);
	else
	    return DumpOnePolygon(FGeom, PPoly, Name);
    }
    else if (IP_IS_POLYLINE_OBJ(PObj)) {
	return DumpOnePolyline(FGeom, PPoly, ObjNum, Name, Width, TRUE);
    }
    else
	return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon.                                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FGeom:        Geometry file to dump object to.                           *
*   PPolygon:     Polygon to dump to file f.                                 *
*   Name:         Name of object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of triangles.                                       *
*****************************************************************************/
static int DumpOnePolygon(FILE *FGeom, IPPolygonStruct *PPolygon, char *Name)
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

    VFirst = VList;
    V1 = VFirst -> Pnext;
    V2 = V1 -> Pnext;
    
    while (V2 != NULL) {
	Pts[0] = VFirst -> Coord;
	Pts[1] = V1 -> Coord;
	Pts[2] = V2 -> Coord;
	
	/* Test for two type of degeneracies. Make sure that no two  */
	/* points in the triangle are the same and that they are     */
	/* not collinear.					     */
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
		
		for (i = 0; i < 3; i++)
		    fprintf(FGeom,
			    "%s %10.7f %10.7f %10.7f  %9.6f %9.6f %9.6f\n",
			    i == 0 ? "    triangle" : "\t    ",
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

    return TriCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polyline.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   FGeomaaa:     Geometry file to dump polyline to.  		             *
*   PPolyline:    Polyline to dump to file f.                                *
*   ObjNum:	  Number of object. 	                                     *
*   Name:         Name of object.                                            *
*   Width:	  Of polyline.						     *
*   IsPolyline:   Is it a polyline?					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of polylines.                                       *
*****************************************************************************/
static int DumpOnePolyline(FILE *FGeom,
			   IPPolygonStruct *PPolyline,
			   int ObjNum,
			   char *Name,
			   IrtRType Width,
			   int IsPolyline)
{
    int DumpedSphere = FALSE;
    IrtRType *Pts[2];
    IPVertexStruct *V1, *V2,
	*VList = PPolyline -> PVertex;

    if (VList == NULL)
	return 0;

    V1 = VList;
    V2 = V1 -> Pnext;
    
    while (V2 != NULL) {
	Pts[0] = V1 -> Coord;
	Pts[1] = V2 -> Coord;

	if (GlblZBBox[1] < Pts[0][2])
	    GlblZBBox[1] = Pts[0][2];
	if (GlblZBBox[0] > Pts[0][2])
	    GlblZBBox[0] = Pts[0][2];

	if (!IRIT_PT_APX_EQ(Pts[0], Pts[1]) &&
	    (IsPolyline || !IP_IS_INTERNAL_VRTX(V1))) {
	    DumpCone(Width * IRIT_MAX(Pts[0][2] - GlblPolylineDepthCue[0],
				 MIN_RATIO) / GlblPolylineDepthCue[2],
		     Pts[0][0],
		     Pts[0][1],
		     Pts[0][2],
		     Width * IRIT_MAX(Pts[1][2] - GlblPolylineDepthCue[0],
				 MIN_RATIO) / GlblPolylineDepthCue[2],
		     Pts[1][0],
		     Pts[1][1],
		     Pts[1][2],
		     FGeom, Name, ObjNum);

	    if (!DumpedSphere)
		fprintf(FGeom,
			"sphere RS%sSrfProp%d %7.5f %7.5f %7.5f %7.5f\n",
			Name, ObjNum, Width * IRIT_MAX(Pts[0][2] -
					  GlblPolylineDepthCue[0],
					  MIN_RATIO) / GlblPolylineDepthCue[2],
			Pts[0][0],
			Pts[0][1],
			Pts[0][2]);
	    fprintf(FGeom,
		    "sphere RS%sSrfProp%d %7.5f %7.5f %7.5f %7.5f\n",
		    Name, ObjNum, Width * IRIT_MAX(Pts[1][2] -
				      GlblPolylineDepthCue[0],
				      MIN_RATIO) / GlblPolylineDepthCue[2],
		    Pts[1][0],
		    Pts[1][1],
		    Pts[1][2]);
	    DumpedSphere = TRUE;
	}
	else {
	    DumpedSphere = FALSE;
	}
	
	V1 = V2;
	V2 = V2 -> Pnext;
    }

    if (!IsPolyline && !IP_IS_INTERNAL_VRTX(V1)) {
	Pts[0] = V1 -> Coord;
	Pts[1] = VList -> Coord;

	if (!IRIT_PT_APX_EQ(Pts[0], Pts[1])) {
	    DumpCone(Width * IRIT_MAX(Pts[0][2] - GlblPolylineDepthCue[0],
				 MIN_RATIO) / GlblPolylineDepthCue[2],
		     Pts[0][0],
		     Pts[0][1],
		     Pts[0][2],
		     Width * IRIT_MAX(Pts[1][2] - GlblPolylineDepthCue[0],
				 MIN_RATIO) / GlblPolylineDepthCue[2],
		     Pts[1][0],
		     Pts[1][1],
		     Pts[1][2],
		     FGeom, Name, ObjNum);

	    if (!DumpedSphere)
		fprintf(FGeom,
			"sphere RS%sSrfProp%d %7.5f %7.5f %7.5f %7.5f\n",
			Name, ObjNum, Width * IRIT_MAX(Pts[0][2] -
					  GlblPolylineDepthCue[0],
					  MIN_RATIO) / GlblPolylineDepthCue[2],
			Pts[0][0],
			Pts[0][1],
			Pts[0][2]);
	    fprintf(FGeom,
		    "sphere RS%sSrfProp%d %7.5f %7.5f %7.5f %7.5f\n",
		    Name, ObjNum, Width * IRIT_MAX(Pts[1][2] -
				      GlblPolylineDepthCue[0],
				      MIN_RATIO) / GlblPolylineDepthCue[2],
		    Pts[1][0],
		    Pts[1][1],
		    Pts[1][2]);
	}
    }

    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps a truncated cone (can degenerate to a cylinder or nothing).          *
*                                                                            *
* PARAMETERS:                                                                *
*   R1:          Radius of first base of cone.                               *
*   X1, Y1, Z1:  Center of first base of cone.                               *
*   R2:          Radius of second base of cone.                              *
*   X2, Y2, Z2:  Center of second base of cone.                              *
*   FGeom:       Geometry file when this cone goes to.                       *
*   Name:        Name of object                                              *
*   ObjNum:	 Number of object. 	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpCone(IrtRType R1,
		     IrtRType X1,
		     IrtRType Y1,
		     IrtRType Z1,
		     IrtRType R2,
		     IrtRType X2,
		     IrtRType Y2,
		     IrtRType Z2,
		     FILE *FGeom,
		     char *Name,
		     int ObjNum)
{
    IrtRType
	Len = sqrt(IRIT_SQR(X1 - X2) + IRIT_SQR(Y1 - Y2) + IRIT_SQR(Z1 - Z2));

    if (Len < CONE_SIZE)
	return;

    if (IRIT_FABS(R1 - R2) < CONE_SIZE) {
	fprintf(FGeom,
		"cylinder RS%sSrfProp%d %7.5f  %7.5f %7.5f %7.5f  %7.5f %7.5f %7.5f\n",
		Name, ObjNum, R1, X1, Y1, Z1, X2, Y2, Z2);
    }
    else {
	fprintf(FGeom,
		"cone RS%sSrfProp%d %7.5f %7.5f %7.5f %7.5f  %7.5f %7.5f %7.5f %7.5f\n",
		Name, ObjNum, R1, X1, Y1, Z1, R2, X2, Y2, Z2);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Ray exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2RayExit(int ExitCode)
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
