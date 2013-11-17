/*****************************************************************************
* Filter to convert IRIT data files to POVRAY ray tracer's format.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Dec 1996    *
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
#define DEFAULT_POLYLINE_WIDTH		0.05

#define CONE_SIZE	5e-4

#define MIN_RATIO	0.01

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Pov		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Irit2Pov	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "irit2pov l%- 4%- C%- F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampTol!d!F o%-OutName!s g%- p%-Zmin|Zmax!F!F P%- M%- T%- b%-\"R,B,G|(background)\"!s t%-AnimTime!F I%-#UIso[:#VIso[:#WIso]]!s s%-ObjSeq#!d i%-PovIncludes!s z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *GlblBackGroundStr = "0,0,0",
    *GlblOutFileName = "Irit2Pov";

IRIT_STATIC_DATA FILE
    *OutputFilePov = NULL,
    *OutputFileGeom = NULL;

IRIT_STATIC_DATA int
    GlblBackGroundColor[3] = { 0, 0, 0 },
    GlblHasLightSources = FALSE,
    GlblDefaultBezierSteps = 4,
    GlblTotalPolysSrfs = 0,
    GlblDumpPolylines = FALSE,
    GlblDumpOnlyGeomFile = FALSE,
    GlblObjectSeqNum = 1;

IRIT_STATIC_DATA IrtRType
    GlblZBBox[2] = { IRIT_INFNTY, -IRIT_INFNTY },
    GlblPolylineDepthCue[3] = { -IRIT_INFNTY, IRIT_INFNTY };

static void DumpDataForPovRay(IPObjectStruct *PObjects, char *IncludeFileNames);
static void DumpLightSrcObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static int DumpOneObject(FILE *FPov, FILE *FGeom, IPObjectStruct *PObject);
static int DumpOneSrf(FILE *FGeom,
		      CagdSrfStruct *PSrf,
		      int SrfIndex,
		      IPObjectStruct *PObj,
		      int ObjNum,
		      char *Name);
static int DumpOnePoly(FILE *FGeom,
		       IPPolygonStruct *PPoly,
		       IPObjectStruct *PObj,
		       IrtRType *TextureImageScale,
		       int TextureImageFlip,
		       int ObjNum,
		       char *Name,
		       IrtRType Width);
static int DumpOnePolygon(FILE *f,
			  IPPolygonStruct *PPolygon,
			  IrtRType *TextureImageScale,
			  int TextureImageFlip);
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
static void Irit2PovExit(int ExitCode);

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
    int i,
	Error,
	HasTime = FALSE,
	SrfFineNessFlag = FALSE,
	CrvFineNessFlag = FALSE,
	VerFlag = FALSE,
	ObjSeqFlag = FALSE,
	OutFileFlag = FALSE,
	BackGroundFlag = FALSE,
	IncludeFlag = FALSE,
	NumOfIsolinesFlag = FALSE,
	UseCubicBzrPatches = FALSE,
	NumFiles = 0;
    char Line[IRIT_LINE_LEN_LONG], *p,
	*StrNumOfIsolines = NULL,
	*IncludeFileNames = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &IPFFCState.LinearOnePolyFlag,
			   &IPFFCState.FourPerFlat, &UseCubicBzrPatches,
			   &SrfFineNessFlag, &IPFFCState.OptimalPolygons,
			   &IPFFCState.FineNess, &CrvFineNessFlag,
			   &IPFFCState.CrvApproxMethod,
			   &IPFFCState.CrvApproxTolSamples, &OutFileFlag,
			   &GlblOutFileName, &GlblDumpOnlyGeomFile,
			   &GlblDumpPolylines,
			   &GlblPolylineDepthCue[0],
			   &GlblPolylineDepthCue[1],
			   &IPFFCState.DumpObjsAsPolylines,
			   &IPFFCState.DrawFFMesh, &IPFFCState.Talkative,
			   &BackGroundFlag, &GlblBackGroundStr,
			   &HasTime, &CurrentTime,
			   &NumOfIsolinesFlag, &StrNumOfIsolines,
			   &ObjSeqFlag, &GlblObjectSeqNum,
			   &IncludeFlag, &IncludeFileNames, &VerFlag,
			   &NumFiles, &FileNames)) != 0) {

	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	Irit2PovExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	Irit2PovExit(0);
    }

    if (sscanf(GlblBackGroundStr, "%d %d %d",
	       &GlblBackGroundColor[0],
	       &GlblBackGroundColor[1],
	       &GlblBackGroundColor[2]) != 3)
        sscanf(GlblBackGroundStr, "%d,%d,%d",
	       &GlblBackGroundColor[0],
	       &GlblBackGroundColor[1],
	       &GlblBackGroundColor[2]);

    if (GlblDumpPolylines ||
	IPFFCState.DrawFFMesh ||
	&IPFFCState.DumpObjsAsPolylines) {
	if (IRIT_APX_EQ(GlblPolylineDepthCue[0], GlblPolylineDepthCue[1])) {
	    IRIT_WARNING_MSG("Cannot compute depth cue if Zmin == Zmax\n");
	    Irit2PovExit(1);
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
		    Irit2PovExit(1);
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

    if (!IncludeFlag)
	IncludeFileNames = NULL;

    IRIT_INFO_MSG_PRINTF("%s triangles per flat will be created.\n",
	                 IPFFCState.FourPerFlat ? "Four" : "Two");

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit.\n");
	GAPrintHowTo(CtrlStr);
	Irit2PovExit(1);
    }

    if (!OutFileFlag) {		/* Pick the first input name as output name. */
	strcpy(Line, FileNames[0]);
	if ((p = strstr(Line, ".itd")) != NULL ||   /* Remove old file type. */
	    (p = strstr(Line, ".imd")) != NULL ||
	    (p = strstr(Line, ".ibd")) != NULL)
	    *p = 0;
	GlblOutFileName = IritStrdup(Line);
    }

    /* Dumps bicubic Bezier patches, and sets their default step size. */
    if (UseCubicBzrPatches)
	IPFFCState.SrfsToBicubicBzr = TRUE;
    for (i = 1, GlblDefaultBezierSteps = 0;
	 i < IPFFCState.FineNess;
	 i += i, GlblDefaultBezierSteps++);

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	Irit2PovExit(1);
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

    DumpDataForPovRay(PObjects, IncludeFileNames);

    if (IPFFCState.DumpObjsAsPolylines && GlblZBBox[0] < GlblZBBox[1])
	IRIT_INFO_MSG_PRINTF("Z depth cueing of polylines spans [%f : %f]\n",
		             GlblZBBox[0], GlblZBBox[1]);

    Irit2PovExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data for Ray Shade to stdout.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To dump into file.                                           *
*   IncludeFileNames:  Comma separated pov include files, if any.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpDataForPovRay(IPObjectStruct *PObjects, char *IncludeFileNames)
{
    IRIT_STATIC_DATA char *Header1[] = {
	"//",
	"// This file was automatically created from data of the IRIT solid modeller",
	"// using Irit2Pov - IRIT to PovRay filter.",
	"//",
	"//            (c) Copyright 1989-2012 Gershon Elber, Non commercial use only.",
	"//",
	"//",
	"//",
	"//",
	"",
	NULL
    };
    IRIT_STATIC_DATA char *Header2[] = {
	"#version 3.5;",
	"global_settings { assumed_gamma 2.2 }",
	"",
	"camera {",
	"    orthographic",
	"    location <0, 0, 10>",
	"    right -x*2.66",
	"    up y*2",
	"    look_at <0, 0, 0>",
	"}",
	"",
	NULL
    };
    IRIT_STATIC_DATA char *Header3[] = {
	"//Created by Chris Colefax, 1 June 1999;",
	"//  Updated 12 September 1999: renamed macros, improved memory use,",
	"//     added full mesh {} support."
	""
	"// COMMON TRANSFORMATION MACRO (REQUIRED FOR BOTH MAPPING METHODS)",
	" #macro tri_matrix() #local NX = P2-P1; #local NY = P3-P1; #local NZ = vcross(NX, NY); matrix <NX.x, NX.y, NX.z, NY.x, NY.y, NY.z, NZ.x, NZ.y, NZ.z, P1.x, P1.y, P1.z> #end",
	"",
	"// COLOURED VERTEX MACROS",
	" #ifndef (triangle_base_texture) #declare triangle_base_texture = texture {} #end",
	" #macro get_cv_texture () texture {triangle_base_texture pigment {",
	"      #if (C1.red=C2.red & C1.red=C3.red & C1.green=C2.green & C1.green=C3.green & C1.blue=C2.blue & C1.blue=C3.blue & C1.filter=C2.filter & C1.filter=C3.filter & C1.transmit=C2.transmit & C1.transmit=C3.transmit) C1 #else",
	"      average pigment_map {[1 gradient x color_map {[0 rgb 0] [1 C2*3]}] [1 gradient y color_map {[0 rgb 0] [1 C3*3]}] [1 gradient z color_map {[0 rgb 0] [1 C1*3]}]}",
	"      matrix <1.01, 0, 1, 0, 1.01, 1, 0, 0, 1, -.002, -.002, -1> tri_matrix() #end }} #end",
	" #macro cv_triangle (P1, C1, P2, C2, P3, C3) #local T = get_cv_texture () triangle {P1, P2, P3 texture {T}} #end",
	" #macro cv_smooth_triangle (P1, N1, C1, P2, N2, C2, P3, N3, C3) #local T = get_cv_texture () smooth_triangle {P1, N1, P2, N2, P3, N3 texture {T}} #end",
	"",
	NULL
    };
    IRIT_STATIC_DATA char *DefLgtSrc[] = {
	"light_source { <1, 2, 10> color red 1 green 1 blue 1 }",
	"",
	NULL
    };
    int i;
    char Line[IRIT_LINE_LEN_LONG];
    IrtHmgnMatType UnitMat;

    sprintf(Line, "%s.pov", GlblOutFileName);
    if (!GlblDumpOnlyGeomFile) {
	if ((OutputFilePov = fopen(Line, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
	    Irit2PovExit(2);
	}
    }
    else
	OutputFilePov = NULL;

    sprintf(Line, "%s.geom", GlblOutFileName);
    if ((OutputFileGeom = fopen(Line, "w")) == NULL) {
#	if defined(OS2GCC) || defined(__WINNT__)
	    sprintf(Line, "%s.geo", GlblOutFileName);
	    if ((OutputFileGeom = fopen(Line, "w")) == NULL)
#	endif /* OS2GCC || __WINNT__ */
	    {
		IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", Line);
		Irit2PovExit(2);
	    }
    }

    if (OutputFilePov != NULL)
	for (i = 0; Header1[i] != NULL; i++)
	    fprintf(OutputFilePov, "%s\n", Header1[i]);

    for (i = 0; Header1[i] != NULL; i++)
	fprintf(OutputFileGeom, "%s\n", Header1[i]);
    for (i = 0; Header3[i] != NULL; i++)
	fprintf(OutputFileGeom, "%s\n", Header3[i]);

    if (OutputFilePov != NULL) {
	for (i = 0; Header2[i] != NULL; i++)
	    fprintf(OutputFilePov, "%s\n", Header2[i]);

	fprintf(OutputFilePov, "background { rgb < %f, %f, %f > }\n\n",
		GlblBackGroundColor[0] / 255.0,
		GlblBackGroundColor[1] / 255.0,
		GlblBackGroundColor[2] / 255.0);

	if (IncludeFileNames != NULL) {         /* Dumps '#include' command. */
	    char *p, Buf[IRIT_LINE_LEN_LONG];

	    strcpy(Buf, IncludeFileNames);
	    p = strtok(Buf, ",");
	    fprintf(OutputFilePov, "\n");
	    do {
		fprintf(OutputFilePov, "#include \"%s\"\n", p);
	    }
	    while ((p = strtok(NULL, ",")) != NULL);
	    fprintf(OutputFilePov, "\n");
	}
    }

    /* Look for light sources, if any, and dump them. */
    if (OutputFilePov != NULL) {
        GlblHasLightSources = FALSE;
	MatGenUnitMat(UnitMat);
	IPTraverseObjListHierarchy(PObjects, UnitMat, DumpLightSrcObject);
	if (!GlblHasLightSources) {
	    for (i = 0; DefLgtSrc[i] != NULL; i++)
	        fprintf(OutputFilePov, "%s\n", DefLgtSrc[i]);
	}
	fprintf(OutputFilePov, "\n");
    }

    /* Dump the geometry. */
    MatGenUnitMat(UnitMat);
    IPTraverseObjListHierarchy(PObjects, UnitMat, DumpOneTraversedObject);

    if (OutputFilePov != NULL) {
	fprintf(OutputFilePov, "\n#include \"%s\"\n", Line);
	fclose(OutputFilePov);
    }

    fclose(OutputFileGeom);

    IRIT_INFO_MSG_PRINTF("\nTotal number of polygons and surface - %d\n",
	                 GlblTotalPolysSrfs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy. This function will only dump light sources.*
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpLightSrcObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    if (IP_IS_POINT_OBJ(PObj) &&
	!IP_ATTR_IS_BAD_INT(AttrGetObjectIntAttrib(PObj, "LIGHT_SOURCE"))) {
        int r, g, b;

        /* We found a light source prescription. */
        GlblHasLightSources = TRUE;

	if (!AttrGetObjectRGBColor(PObj, &r, &g, &b))
	    r = g = b = 255;

	fprintf(OutputFilePov,
		"light_source { < %.4g, %.4g, %.4g > color red %.4g green %.4g blue %.4g }\n",
		PObj -> U.Pt[0], PObj -> U.Pt[1], PObj -> U.Pt[2],
		r / 256.0, g / 256.0, b/ 256.0);
    }
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

    if (IP_IS_FFGEOM_OBJ(PObj)) {
	if (IP_IS_SRF_OBJ(PObj)) {
	    CagdSrfStruct
		*Srf = PObj -> U.Srfs;

	    if (CAGD_IS_BSPLINE_SRF(Srf)) {
		/* Normalize domain to control texture mapping replication. */
		BspKnotAffineTransOrder2(Srf -> UKnotVector, Srf -> UOrder,
				     Srf -> UOrder + Srf -> ULength, 0.0, 1.0);
		BspKnotAffineTransOrder2(Srf -> VKnotVector, Srf -> VOrder,
				     Srf -> VOrder + Srf -> VLength, 0.0, 1.0);
	    }
	}

        IPFFCState.ComputeUV = 
	                  AttrGetObjectStrAttrib(PObj, "ptexture") != NULL;
        PObjs = IPConvertFreeForm(PObj, &IPFFCState);  /* Convert in place. */
    }
    else
	PObjs = PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext)
	GlblTotalPolysSrfs += DumpOneObject(OutputFilePov,
					    OutputFileGeom, PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one object PObject to files.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   FPov:         Ray file to dump object to.                                *
*   FGeom:        Geometry file to dump object to.                           *
*   PObject:      Object to dump to file f.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int DumpOneObject(FILE *FPov, FILE *FGeom, IPObjectStruct *PObject)
{
    IRIT_STATIC_DATA char
        *FinishStrAttribs[] = {
	    "ambient",
	    "diffuse",
	    "brilliance",
	    "phong",
	    "phong_size",
	    "specular",
	    "roughness",
	    "metallic",
	    "reflection",
	    "crand",
	    "conserve_energy",
	    "irid",
	    NULL
	},
        *InteriorStrAttribs[] = {
	    "ior",
	    "caustics",
	    "dispersion",
	    "dispersion_samples",
	    "fade_distance",
	    "fade_power",
	    "fade_color",
	    NULL
	};
    IRIT_STATIC_DATA int
        ObjNum = 0;
    int i, RGBIColor[3], 
	TextureImageFlip = 0,
	HasAmbient = FALSE,
	HasDiffuse = FALSE,
	HasSpecular = FALSE,
        PolyCount = 0,
        SrfCount = 0,
	HasTransp = FALSE,
	HasColor = FALSE;
    char Name[IRIT_LINE_LEN_LONG], TextureFName[IRIT_LINE_LEN_LONG];
    const char *p,
	*PTextureAttr = AttrGetObjectStrAttrib(PObject, "PTexture"),
	*TextureAttr = AttrGetObjectStrAttrib(PObject, "Texture"),
	*PigmentAttr = AttrGetObjectStrAttrib(PObject, "Pigment"),
	*FinishAttr = AttrGetObjectStrAttrib(PObject, "Finish"),
	*InteriorAttr = AttrGetObjectStrAttrib(PObject, "Interior"),
	*HaloAttr = AttrGetObjectStrAttrib(PObject, "Halo"),
	*NormalAttr = AttrGetObjectStrAttrib(PObject, "Normal");
    IrtRType RGBColor[3], TranspVal, TextureImageScale[3],
	Width = DEFAULT_POLYLINE_WIDTH;

    if (!IP_IS_SRF_OBJ(PObject) &&
	(!IP_IS_POLY_OBJ(PObject) ||
	 IP_IS_POINTLIST_OBJ(PObject) ||
	 PObject -> U.Pl == NULL))
	return 0;

    ObjNum++;

    p = IP_GET_OBJ_NAME(PObject);
    if (p == NULL || p[0] == 0 || strcmp(p, "_") == 0)
	sprintf(Name, "ObjSeq%d", GlblObjectSeqNum);
    else
	strcpy(Name, p[0] == '_' ? &p[1] : p);

    Width = AttrGetObjectRealAttrib(PObject, "Width");
    if (IP_ATTR_IS_BAD_REAL(Width))
        Width = DEFAULT_POLYLINE_WIDTH;

    /* Validate the "ptexture" attribute if has one. */
    TextureImageScale[0] = TextureImageScale[1] = TextureImageScale[2] = 1.0;
    if (PTextureAttr != NULL &&
	!IrtImgParsePTextureString(PTextureAttr, TextureFName,
				   TextureImageScale, &TextureImageFlip))
        PTextureAttr = NULL;

    if (IP_IS_SRF_OBJ(PObject)) {
	CagdSrfStruct *SList;

	/* Dumps bicubic Bezier patches. */
	for (SList = PObject -> U.Srfs; SList != NULL; SList = SList -> Pnext) {
	    SrfCount += DumpOneSrf(FGeom, SList, SrfCount, PObject,
				   ObjNum, Name);
	}

	if (IPFFCState.Talkative)
	    IRIT_INFO_MSG_PRINTF("Converting \"%s\" - %d bicubic srfs.\n",
		                 Name, SrfCount);

	fprintf(FGeom, "union { // %s\n", Name);
	for (i = 0; i < SrfCount; i++) {
	    fprintf(FGeom, "    object{ Pov%s%dn%d }\n", Name, ObjNum, i);
	}
    }
    else if (IP_IS_POLY_OBJ(PObject)) {
	IPPolygonStruct *PList;

	if (PTextureAttr != NULL) {  /* Make sure we have UV coordinates set. */
	    GMGenUVValsForPolys(PObject,
				TextureImageScale[0],
				TextureImageScale[1],
				TextureImageScale[2],
				TextureImageScale[2] != IRIT_INFNTY);
	}

	if (IP_IS_POLYGON_OBJ(PObject))
	    fprintf(FGeom, "mesh { // %s\n", Name);
	else
	    fprintf(FGeom, "union { // %s\n", Name);

	/* Dumps polygons. */
	for (PList = PObject -> U.Pl; PList != NULL; PList = PList -> Pnext) {
	    PolyCount += DumpOnePoly(FGeom, PList, PObject,
				     TextureImageScale, TextureImageFlip,
				     ObjNum, Name, Width);
	}

	if (IPFFCState.Talkative)
	    IRIT_INFO_MSG_PRINTF("Converting \"%s\" - %d triangles.\n",
		                 Name, PolyCount);
    }

    if (AttrGetObjectRGBColor2(PObject, NULL,
			       &RGBIColor[0], &RGBIColor[1], &RGBIColor[2])) {
	HasColor = TRUE;
	for (i = 0; i < 3; i++)
	    RGBColor[i] = RGBIColor[i] / 255.0;
    }

    TranspVal = AttrGetObjectRealAttrib(PObject, "transp");
    HasTransp = !IP_ATTR_IS_BAD_REAL(TranspVal);

    HasAmbient = AttrGetObjectStrAttrib(PObject, "ambient") != NULL;
    HasDiffuse = AttrGetObjectStrAttrib(PObject, "diffuse") != NULL;
    HasSpecular = AttrGetObjectStrAttrib(PObject, "specular") != NULL;

    /* Take care of the "texture" modifier. */
    fprintf(FGeom, "    texture { Pov%s%dTexture }\n", Name, ObjNum);
    if (FPov != NULL) {
	fprintf(FPov, "#declare Pov%s%dTexture = texture {\n",
		Name, ObjNum);
	if (TextureAttr == NULL) {
	    /* Dumps out the "pigment" section. */
	    if (PigmentAttr != NULL) {
		fprintf(FPov, "    pigment { %s }\n", PigmentAttr);
	    }
	    else if (PTextureAttr != NULL) {
	        char *TextureFType;

		if ((TextureFType = strchr(TextureFName, '.')) != NULL)
		    TextureFType++;
		else
		    TextureFType = "gif";		    /* Default type. */

	        fprintf(FPov, "    uv_mapping pigment {\n\timage_map { %s \"%s\" }\n\tscale <1, 1, 1>\n    }\n",
			TextureFType, TextureFName);
	    }
	    else if (HasColor) {
		if (HasTransp)
		    fprintf(FPov, "    pigment { rgbf <%f, %f, %f, %f> }\n",
			    RGBColor[0], RGBColor[1], RGBColor[2], TranspVal);
		else
		    fprintf(FPov, "    pigment { rgb <%f, %f, %f> }\n",
			    RGBColor[0], RGBColor[1], RGBColor[2]);
	    }
	    else
		fprintf(FPov, "    pigment { rgb < 1, 1, 1> }\n");

	    /* Dumps out the "finish" section. */
	    if (FinishAttr != NULL) {
		fprintf(FPov, "    finish { %s }\n", FinishAttr);
	    }
	    else {
		fprintf(FPov, "    finish {\n");
		if (!HasAmbient)
		    fprintf(FPov, "\tambient 0\n");
		if (!HasDiffuse)
		    fprintf(FPov, "\tdiffuse 1\n");
		if (!HasSpecular)
	            fprintf(FPov, "\tspecular 1\n");
		for (i = 0; FinishStrAttribs[i] != NULL; i++) {
		    if ((p = AttrGetObjectStrAttrib(PObject,
						FinishStrAttribs[i])) != NULL)
			fprintf(FPov, "\t%s %s\n", FinishStrAttribs[i], p);
		}
		fprintf(FPov, "    }\n");
	    }

	    /* Dumps out the "normal" section. */
	    if (NormalAttr != NULL) {
		fprintf(FPov, "    normal { %s }\n", NormalAttr);
	    }

	    /* Dumps out the "halo" section. */
	    if (HaloAttr != NULL) {
		fprintf(FPov, "    halo { %s }\n", HaloAttr);
	    }
 	}
	else {
	    fprintf(FPov, "    %s\n", TextureAttr);
	}
	fprintf(FPov, "} // %s\n", Name);
    }

    /* Take care of the (optional) "interior" modifier. */
    for (i = 0; InteriorStrAttribs[i] != NULL; i++)
	if ((p = AttrGetObjectStrAttrib(PObject,
					InteriorStrAttribs[i])) != NULL)
	    break;
    if (p != NULL) { /* Dump "interior" modifier */
        fprintf(FGeom, "    interior { Pov%s%dInterior }\n", Name, ObjNum);
	if (FPov != NULL) {
	    if (InteriorAttr != NULL) {
	        fprintf(FPov, "#declare Pov%s%dInterior = interior { %s }\n",
			Name, ObjNum, InteriorAttr);
	    }
	    else {
	        fprintf(FPov, "#declare Pov%s%dInterior = interior {\n",
			Name, ObjNum);
		for (i = 0; InteriorStrAttribs[i] != NULL; i++) {
		    if ((p = AttrGetObjectStrAttrib(PObject,
						    InteriorStrAttribs[i])) != NULL)
		        fprintf(FPov, "    %s %s\n", InteriorStrAttribs[i], p);
		}
		fprintf(FPov, "}\n");
	    }
	}
    }

    fprintf(FGeom, "} // %s\n\n", Name);
    if (FPov != NULL)
	fprintf(FPov, "\n");
    GlblObjectSeqNum++;

    return PolyCount + SrfCount;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one surface (a cubic Bezier patch).                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   FGeom:        Geometry file to dump poly to.                             *
*   PSrf:         Surface to dump to file FGeom.                             *
*   SrfIndex:     Index of surface in object.				     *
*   PObj:         Object PSrf cames from to identify a polygon from polyline.*
*   ObjNum:	  Number of object. 	                                     *
*   Name:         Name of object.	                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of Srfs.                                            *
*****************************************************************************/
static int DumpOneSrf(FILE *FGeom,
		      CagdSrfStruct *PSrf,
		      int SrfIndex,
		      IPObjectStruct *PObj,
		      int ObjNum,
		      char *Name)
{
    int i;
    CagdRType **Points;
    CagdSrfStruct
	*Srf = CagdCoerceSrfTo(PSrf, CAGD_PT_E3_TYPE, FALSE);
    CagdRType
	Steps = AttrGetObjectRealAttrib(PObj, "Steps"),
	USteps = AttrGetObjectRealAttrib(PObj, "USteps"),
	VSteps = AttrGetObjectRealAttrib(PObj, "VSteps");

    if (IP_ATTR_IS_BAD_REAL(Steps))
	Steps = GlblDefaultBezierSteps;
    if (IP_ATTR_IS_BAD_REAL(USteps))
        USteps = Steps;
    if (IP_ATTR_IS_BAD_REAL(VSteps))
        VSteps = Steps;

    fprintf(FGeom,
	    "#declare Pov%s%dn%d = bicubic_patch {\n\ttype 1\n\tflatness 0\n\tu_steps %f\n\tv_steps %f\n",
	    Name, ObjNum, SrfIndex, USteps, VSteps);

    Points = Srf -> Points;
    for (i = 0; i < Srf -> UOrder * Srf -> VOrder; i++) {
	fprintf(FGeom, "\t<%7.5f, %7.5f, %7.5f>\n",
		Points[1][i], Points[2][i], Points[3][i]);
    }
    fprintf(FGeom, "}\n");

    return 1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps one polygon/line.                                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   FGeom:             Geometry file to dump poly to.                        *
*   PPoly:             Poly to dump to file FGeom.                           *
*   PObj:              Object PPoly cames from.				     *
*   TextureImageScale: Replication scales for the parametric texture.	     *
*   TextureImageFlip:  Flips the U and V coordinates if TRUE.		     *
*   ObjNum:	       Number of object. 	                             *
*   Name:              Name of object.	                                     *
*   Width:	       Of polylines dumped as skinny cones/spheres.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of Polys.                                           *
*****************************************************************************/
static int DumpOnePoly(FILE *FGeom,
		       IPPolygonStruct *PPoly,
		       IPObjectStruct *PObj,
		       IrtRType *TextureImageScale,
		       int TextureImageFlip,
		       int ObjNum,
		       char *Name,
		       IrtRType Width)
{
    if (IP_IS_POLYGON_OBJ(PObj)) {
	if (IPFFCState.DumpObjsAsPolylines)
	    return DumpOnePolyline(FGeom, PPoly, ObjNum, Name, Width, FALSE);
	else
	    return DumpOnePolygon(FGeom, PPoly, 
				  TextureImageScale, TextureImageFlip);
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
*   FGeom:             Geometry file to dump object to.                      *
*   PPolygon:          Polygon to dump to file FGeom.                        *
*   TextureImageScale: Replication scales for the parametric texture.	     *
*   TextureImageFlip:  Flips the U and V coordinates if TRUE.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          Number of triangles.                                       *
*****************************************************************************/
static int DumpOnePolygon(FILE *FGeom,
			  IPPolygonStruct *PPolygon,
			  IrtRType *TextureImageScale,
			  int TextureImageFlip)
{
    int i,
	TriCount = 0;
    float *UV[3];
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

	UV[0] = AttrGetUVAttrib(VFirst -> Attr, "uvvals");
	UV[1] = AttrGetUVAttrib(V1 -> Attr, "uvvals");
	UV[2] = AttrGetUVAttrib(V2 -> Attr, "uvvals");

	/* Test for two type of degeneracies. Make sure that no two  */
	/* points in the triangle are the same and that they are     */
	/* not collinear.					     */
	if (!IRIT_PT_APX_EQ(Pts[0], Pts[1]) &&
	    !IRIT_PT_APX_EQ(Pts[0], Pts[2]) &&
	    !IRIT_PT_APX_EQ(Pts[1], Pts[2])) {
	    IPVertexStruct
		*Vertices[3];

	    Vertices[0] = VFirst;
	    Vertices[1] = V1;
	    Vertices[2] = V2;

	    IRIT_PT_SUB(Vec1, Pts[0], Pts[1]);
	    IRIT_PT_SUB(Vec2, Pts[1], Pts[2]);
	    IRIT_PT_NORMALIZE(Vec1);
	    IRIT_PT_NORMALIZE(Vec2);
	    IRIT_CROSS_PROD(Normal, Vec1, Vec2);
	    
	    if (IRIT_PT_LENGTH(Normal) > SIZE_IRIT_EPS) {
		int Red, Green, Blue,
		    HasVrtxRGB = FALSE;

		IRIT_PT_NORMALIZE(Normal);

		for (i = 0; i < 3; i++)
		    Nrmls[i] = Vertices[i] -> Normal;
		
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

		HasVrtxRGB = AttrGetRGBColor(Vertices[0] -> Attr,
					     &Red, &Green, &Blue) &&
			     AttrGetRGBColor(Vertices[1] -> Attr,
					     &Red, &Green, &Blue) &&
			     AttrGetRGBColor(Vertices[2] -> Attr,
					     &Red, &Green, &Blue);

		fprintf(FGeom, HasVrtxRGB ? "    cv_smooth_triangle (\n"
					  : "    smooth_triangle {\n");
		for (i = 0; i < 3; i++) {
		    fprintf(FGeom,
			    "\t<%10.7f, %10.7f, %10.7f>, <%9.6f, %9.6f, %9.6f>",
			    Pts[i][0],
			    Pts[i][1],
			    Pts[i][2],
			    Nrmls[i][0],
			    Nrmls[i][1],
			    Nrmls[i][2]);
		    if (AttrGetRGBColor(Vertices[i] -> Attr,
					&Red, &Green, &Blue)) {
			    fprintf(FGeom,
				    " rgb <%6.3f, %6.3f, %6.3f>",
				    Red / 255.0,
				    Green / 255.0,
				    Blue / 255.0);
		    }

		    fprintf(FGeom, "%s", i == 2 ? "\n" : ",\n");
		}

		if (UV[0] != NULL && UV[1] != NULL && UV[2] != NULL) {
		    fprintf(FGeom, "\tuv_vectors");
		    for (i = 0; i < 3; i++)
		        fprintf(FGeom, " <%8.5f, %8.5f>",
			      UV[i][TextureImageFlip] * TextureImageScale[0],
			      UV[i][!TextureImageFlip] * TextureImageScale[1]);
		    fprintf(FGeom, "\n");
		}

		fprintf(FGeom, HasVrtxRGB ? "    )\n" : "    }\n");
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
			"    sphere {\n\t<%7.5f, %7.5f, %7.5f>, %7.5f\n\ttexture { Pov%s%dTexture }\n    }\n",
			Pts[0][0],
			Pts[0][1],
			Pts[0][2],
			Width * IRIT_MAX(Pts[0][2] -
				    GlblPolylineDepthCue[0],
					  MIN_RATIO) / GlblPolylineDepthCue[2],
			Name, ObjNum);
	    fprintf(FGeom,
		    "    sphere {\n\t<%7.5f, %7.5f, %7.5f>, %7.5f\n\ttexture { Pov%s%dTexture }\n    }\n",
		    Pts[1][0],
		    Pts[1][1],
		    Pts[1][2],
		    Width * IRIT_MAX(Pts[1][2] -
				GlblPolylineDepthCue[0],
					  MIN_RATIO) / GlblPolylineDepthCue[2],
		    Name, ObjNum);
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
			"    sphere {\n\t<%7.5f, %7.5f, %7.5f>, %7.5f\n\ttexture { Pov%s%dTexture }\n    }\n",
			Pts[0][0],
			Pts[0][1],
			Pts[0][2],
			Width * IRIT_MAX(Pts[0][2] -
				    GlblPolylineDepthCue[0],
					  MIN_RATIO) / GlblPolylineDepthCue[2],
			Name, ObjNum);
	    fprintf(FGeom,
		    "    sphere {\n\t<%7.5f, %7.5f, %7.5f>, %7.5f\n\ttexture { Pov%s%dTexture }\n    }\n",
		    Pts[1][0],
		    Pts[1][1],
		    Pts[1][2],
		    Width * IRIT_MAX(Pts[1][2] -
				GlblPolylineDepthCue[0],
					  MIN_RATIO) / GlblPolylineDepthCue[2],
		    Name, ObjNum);
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

    fprintf(FGeom,
	    "    cone {\n\t<%7.5f, %7.5f, %7.5f> %7.5f,\n\t<%7.5f, %7.5f, %7.5f> %7.5f\n\ttexture { Pov%s%dTexture }\n    }\n",
	    X1, Y1, Z1, R1, X2, Y2, Z2, R2, Name, ObjNum);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Irit2Pov exit routine.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   ExitCode:                                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Irit2PovExit(int ExitCode)
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
