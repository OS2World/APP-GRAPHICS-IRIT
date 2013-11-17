/*****************************************************************************
* Program to create illustrations of wireframe drawings.		     *
*   Usually the output of this program is piped to irit2ps, although it      *
* creates a regular IRIT data files with polylines.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, June 1993   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "program.h"
#include "misc_lib.h"
#include "grap_lib.h"
#include "allocate.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"

#define HEAT_CRV_NUM_PTS	9

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "Illustrt		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "Illustrt		" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char 
    *CtrlStr = "Illustrt I%-#UIso[:#VIso[:#WIso]]!s f%-PolyOpti|SampPerCrv!d!F s%- M%- P%- p%- O%- l%-MaxLnLen!F a%- t%-TrimInter!F o%-OutName!s Z%-InterSameZ!F m%- T%-AnimTime!F b%- z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *StrNumOfIsolines = "10:10:10",
    *GlblOutFileName = "illustrt.itd";

IRIT_STATIC_DATA int
    GlblBinaryOutput = FALSE,
    GlblTalkative = FALSE,
    GlblSortOutput = FALSE,
    GlblDrawSurfaceMesh = FALSE,
    GlblDrawSurface = TRUE,
    GlblNumOfIsolines[3] = { IG_DEFAULT_NUM_OF_ISOLINES,
			     IG_DEFAULT_NUM_OF_ISOLINES,
			     IG_DEFAULT_NUM_OF_ISOLINES };

IRIT_STATIC_DATA SymbCrvApproxMethodType
    GlblPolylineOptiApprox = SYMB_CRV_APPROX_UNIFORM;

IRIT_STATIC_DATA IrtRType
    GlblMaxLineLen = DEFAULT_MAX_LINE_LEN,
    GlblCrv2PllnsSamples = IG_DEFAULT_SAMPLES_PER_CURVE;

IRIT_STATIC_DATA IPObjectStruct
    *GlblShadedObjects = NULL;

IRIT_STATIC_DATA IrtHmgnMatType CrntViewMat;		/* This is the current view! */

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
  { "NumOfIsolines",  "-I", (VoidPtr) &StrNumOfIsolines,     IC_STRING_TYPE },
  { "PolyOpti",       "-f", (VoidPtr) &GlblPolylineOptiApprox,IC_INTEGER_TYPE },
  { "SortOutput",     "-s", (VoidPtr) &GlblSortOutput,	     IC_BOOLEAN_TYPE },
  { "OpenPolyData",   "-O", (VoidPtr) &GlblOpenPolyData,     IC_BOOLEAN_TYPE },
  { "DrawSurfaceMesh","-M", (VoidPtr) &GlblDrawSurfaceMesh,  IC_BOOLEAN_TYPE },
  { "DrawSurface",    "-P", (VoidPtr) &GlblDrawSurface,	     IC_BOOLEAN_TYPE },
  { "VertexPoints",   "-p", (VoidPtr) &GlblVertexPoints,     IC_BOOLEAN_TYPE },
  { "AngularDist",    "-a", (VoidPtr) &GlblAngularDistance,  IC_BOOLEAN_TYPE },
  { "MoreVerbose",    "-m", (VoidPtr) &GlblTalkative,	     IC_BOOLEAN_TYPE },
  { "BinaryOutput",   "-b", (VoidPtr) &GlblBinaryOutput,     IC_BOOLEAN_TYPE },
  { "SamplesPerCurve","-f", (VoidPtr) &GlblCrv2PllnsSamples, IC_REAL_TYPE },
  { "TrimInter",      "-t", (VoidPtr) &GlblTrimIntersect,    IC_REAL_TYPE },
  { "InterSameZ",     "-Z", (VoidPtr) &GlblInterSameZ,	     IC_REAL_TYPE }
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

IRIT_GLOBAL_DATA int
    GlblAngularDistance = TRUE,
    GlblOpenPolyData = FALSE,
    GlblVertexPoints = FALSE,
    GlblSplitLongLines = FALSE;

IRIT_GLOBAL_DATA IrtRType
    GlblInterSameZ = INTER_SAME_Z,
    GlblTrimIntersect = DEFAULT_TRIM_INTERSECT;

static void GetRelResolutions(IPObjectStruct *PObj,
			      IrtRType *SamplesPerCurve,
			      int NumOfIsolines[3]);
static void DumpData(const char *FileName, IPObjectStruct *PObjects);
static IPObjectStruct *ProcessSpeedWave(IPObjectStruct *PObj,
					const char *SpeedWaveAttrs);
static IPPolygonStruct *GenSpeedWave(IrtRType Coords[3],
				     IrtRType GenRand,
				     IrtRType Dir[3],
				     IrtRType Len,
				     IrtRType Dist,
				     IrtRType LenRand,
				     IrtRType DistRand);
static IPObjectStruct *ProcessHeatWave(IPObjectStruct *PObj,
				       const char *HeatWaveAttrs);
static CagdCrvStruct *GenHeatWave(IrtRType Coords[3],
				  IrtRType GenRand,
				  IrtRType Len,
				  IrtRType Dist,
				  IrtRType LenRand,
				  IrtRType DistRand);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of illustrt - Read command line and do what is needed...	     M
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
	CrvOptiPolylinesFlag = FALSE,
        TrimInterFlag = FALSE,
	OutFileFlag = FALSE,
	InterSameZFlag = FALSE,
	VerFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;
    IrtRType CurrentTime;
    IPObjectStruct *PObjects, *PObj,
	*NoProcessObjsBegin = NULL,
	*NoProcessObjsEnd = NULL,
	*PObjWaves = NULL;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IritConfig("illustrt", SetUp, NUM_SET_UP);/* Read config. file if exists.*/

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &NumOfIsolinesFlag, &StrNumOfIsolines,
			   &CrvOptiPolylinesFlag, &GlblPolylineOptiApprox,
			   &GlblCrv2PllnsSamples,
			   &GlblSortOutput, &GlblDrawSurfaceMesh,
			   &GlblDrawSurface, &GlblVertexPoints,
			   &GlblOpenPolyData,
			   &GlblSplitLongLines, &GlblMaxLineLen,
			   &GlblAngularDistance,
			   &TrimInterFlag, &GlblTrimIntersect,
			   &OutFileFlag, &GlblOutFileName,
			   &InterSameZFlag, &GlblInterSameZ, &GlblTalkative,
			   &HasTime, &CurrentTime, &GlblBinaryOutput,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	IllustrateExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	IritConfigPrint(SetUp, NUM_SET_UP);
	IllustrateExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names where given, exit.\n");
	GAPrintHowTo(CtrlStr);
	IllustrateExit(1);
    }

    if (NumOfIsolinesFlag && StrNumOfIsolines != NULL) {
	if (sscanf(StrNumOfIsolines, "%d:%d:%d",
		   &GlblNumOfIsolines[0],
		   &GlblNumOfIsolines[1],
		   &GlblNumOfIsolines[2]) != 3) {
	    if (sscanf(StrNumOfIsolines, "%d:%d",
		       &GlblNumOfIsolines[0],
		       &GlblNumOfIsolines[1]) != 2) {
		if (sscanf(StrNumOfIsolines, "%d",
			   &GlblNumOfIsolines[1]) != 1) {
		    IRIT_WARNING_MSG(
			    "Number(s) of isolines (-I) cannot be parsed.\n");
		    GAPrintHowTo(CtrlStr);
		    IllustrateExit(1);
		}
		else {
		    GlblNumOfIsolines[2] =
			GlblNumOfIsolines[1] =
			    GlblNumOfIsolines[0];
		}
	    }
	    else {
		GlblNumOfIsolines[2] = GlblNumOfIsolines[0];
	    }
	}
    }

    /* Get the data files: */
    IPSetFlattenObjects(FALSE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	IllustrateExit(0);
    PObjects = IPResolveInstances(PObjects);

    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext)
	IPPropagateObjectName(PObj, NULL);

    /* Do we have animation time flag? */
    if (HasTime)
        GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    /* Flatten the tree, convert to polygons/lines on the fly and accumulate */
    /* objects that has "IllustrtShadeBG" in them, and append at the end.    */
    GlblShadedObjects = NULL;

    IPFlattenInvisibleObjects(FALSE);
    PObjects = IPFlattenForrest(PObjects);

    /* Freefrom objects with "IllustrtShadeBG" were processed in the call    */
    /* back function IPProcessFreeForm into GlblShadedObjects.		     */
    /* Poly objects with "IllustrtShadeBG" must be processed explicitely.    */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	IrtRType
	    ShadeBG = AttrGetRealAttrib(PObj -> Attr, "IllustrtShadeBG");

	if (!IP_ATTR_IS_BAD_REAL(ShadeBG)) {
	    IPObjectStruct
		*PTmp = IPCopyObject(NULL, PObj, TRUE);

	    AttrSetIntAttrib(&PTmp -> Attr, "fill", TRUE);
	    AttrSetRealAttrib(&PTmp -> Attr, "gray",
			      IRIT_BOUND(ShadeBG, 0.0, 1.0));
	    AttrFreeOneAttribute(&PObj -> Attr, "IllustrtShadeBG");
	    PTmp -> Pnext = GlblShadedObjects;
	    GlblShadedObjects = PTmp;
	}
    }

    PObjects = IPAppendObjLists(PObjects, GlblShadedObjects);

    if (IPWasPrspMat)
	MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    /* Scan the objects for wave supported attributes. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	IPObjectStruct *PObjWave;
	const char *p;

	if ((p = AttrGetObjectStrAttrib(PObj, "speedwave")) != NULL &&
	    (PObjWave = ProcessSpeedWave(PObj, p)) != NULL) {
	    PObjWave -> Pnext = PObjWaves;
	    PObjWaves = PObjWave;
	}
	if ((p = AttrGetObjectStrAttrib(PObj, "heatwave")) != NULL &&
	    (PObjWave = ProcessHeatWave(PObj, p)) != NULL) {
	    PObjWave -> Pnext = PObjWaves;
	    PObjWaves = PObjWave;
	}
    }

    PObj = GMTransformObjectList(PObjects, CrntViewMat);
    IPFreeObjectList(PObjects);
    PObjects = PObj;

    if (PObjWaves != NULL) {
	PObj = GMTransformObjectList(PObjWaves, CrntViewMat);
	IPFreeObjectList(PObjWaves);
	PObjWaves = PObj;
    }

    for (PObj = PObjects, PObjects = NULL; PObj != NULL; ) {
        IrtRType ShadeBG;
        IPObjectStruct
	    *PObjNext = PObj -> Pnext;

	ShadeBG = AttrGetRealAttrib(PObj -> Attr, "IllustrtShadeBG");
	if (!IP_ATTR_IS_BAD_REAL(ShadeBG)) {
	    int Red, Green, Blue;

	    AttrSetIntAttrib(&PObj -> Attr, "fill", TRUE);
	    AttrSetRealAttrib(&PObj -> Attr, "gray",
			      IRIT_BOUND(ShadeBG, 0.0, 1.0));
	    if (AttrGetRGBColor2(PObj -> Attr, NULL, &Red, &Green, &Blue)) {
	        Red = (int) (Red * ShadeBG);
		Green = (int) (Green * ShadeBG);
		Blue = (int) (Blue * ShadeBG);
	        AttrSetRGBColor(&PObj -> Attr, IRIT_BOUND(Red, 0, 255),
					       IRIT_BOUND(Green, 0, 255),
					       IRIT_BOUND(Blue, 0, 255));
	    }
	    PObj -> Pnext = NoProcessObjsBegin;
	    NoProcessObjsBegin = PObj;
	}
	else {
	    if (AttrFindAttribute(PObj -> Attr, "IllustrtNoProcess") != NULL) {
	        if (AttrGetIntAttrib(PObj -> Attr, "IllustrtNoProcess") <= 0) {
		    PObj -> Pnext = NoProcessObjsBegin;
		    NoProcessObjsBegin = PObj;
		}
		else {
		    PObj -> Pnext = NoProcessObjsEnd;
		    NoProcessObjsEnd = PObj;
		}
	    }
	    else {
	        PObj -> Pnext = PObjects;
		PObjects = PObj;
	    }
	}

	PObj = PObjNext;
    }

    if (GlblVertexPoints) {
	/* Add vertices of each polyline as points into data set. */
	for (PObj = PObjects; PObj != NULL;) {
	    if (IP_IS_POLY_OBJ(PObj)) {
		IPObjectStruct
		    *PtList = IPCopyObject(NULL, PObj, TRUE);

		IP_SET_POINTLIST_OBJ(PtList);

		RemoveInternalVertices(PtList);

		PObj -> Pnext = PtList;
		PObj = PtList -> Pnext;
	    }
	    else
		PObj = PObj -> Pnext;
	}
    }

    ProcessIntersections(PObjects);
    if (GlblSplitLongLines)
	SplitLongLines(PObjects, GlblMaxLineLen);

    /* Append the objects created to represent waves. */
    PObjects = IPAppendObjLists(PObjects, PObjWaves);

    if (GlblSortOutput)
	SortOutput(&PObjects);

    /* Prep/Append the objects that are tagged as no process. */
    PObjects = IPAppendObjLists(NoProcessObjsBegin, PObjects);
    PObjects = IPAppendObjLists(PObjects, NoProcessObjsEnd);

    DumpData(OutFileFlag ? GlblOutFileName : NULL, PObjects);

    IllustrateExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get relative resolutions for this specific object, out of attributes.    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:               Object to examine its resolutions' attributes.       *
*   SamplesPerCurve:    Polyline fineness attribute or 1.0 if none.          *
*			Based on "crv_resolution" attribue.		     *
*   NumOfIsolines:      New number of isolines to scale relatively.  Based   *
*			on the "num_of_isolines" attribute.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetRelResolutions(IPObjectStruct *PObj,
			      IrtRType *SamplesPerCurve,
			      int NumOfIsolines[3])
{
    int i;
    IrtRType RelativeNumIsos;

    *SamplesPerCurve = AttrGetObjectRealAttrib(PObj, "crv_resolution");
    if (IP_ATTR_IS_BAD_REAL(*SamplesPerCurve))
        *SamplesPerCurve = 1.0;

    /* Varify the resulting values. */
    if (GlblPolylineOptiApprox == SYMB_CRV_APPROX_UNIFORM) {
	*SamplesPerCurve *= GlblCrv2PllnsSamples;

	if (*SamplesPerCurve < 2) {
	    *SamplesPerCurve = 2;
	    IRIT_WARNING_MSG(
		    "SamplesPerCurve is less than 2, 2 picked instead.\n");
	}
    }
    else {
	*SamplesPerCurve = GlblCrv2PllnsSamples / *SamplesPerCurve;
    }

    RelativeNumIsos = AttrGetObjectRealAttrib(PObj, "num_of_isolines");
    if (IP_ATTR_IS_BAD_REAL(RelativeNumIsos)) {
        for (i = 0; i < 3; i++)
	    NumOfIsolines[i] = GlblNumOfIsolines[i];
    }
    else {
	for (i = 0; i < 3; i++)
	    NumOfIsolines[i] = (int) (GlblNumOfIsolines[i] * RelativeNumIsos);
    }
    /* Very the resulting values. */
    for (i = 0; i < 3; i++) {
        if (NumOfIsolines[i] < 2) {
	    NumOfIsolines[i] = 2;
	    IRIT_WARNING_MSG(
		    "NumOfIsolines is less than 2, 2 picked instead.\n");
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert all surfaces/curves into polylines as follows:	     M
*   Curve is converted to a single polyline with SamplesPerCurve samples.    M
*   Surface is converted into GlblNumOfIsolines curves in each axes, each    M
* handled as Curve above. The original curves and surfaces are then deleted. M
*   This function is a call back function of the irit parser.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeForms:  Crvs/Srfs/Trimmed Srfs/Trivariates/TriSrf read from a file   M
*               by the irit parser.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Processed freeform geometry. This function simply    M
*                       returns what it gots.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPProcessFreeForm, conversion                                            M
*****************************************************************************/
IPObjectStruct *IPProcessFreeForm(IPFreeFormStruct *FreeForms)
{
    int NumOfIsolines[3];
    IrtRType SamplesPerCurve, ShadeBG;
    CagdCrvStruct *Crv, *Crvs;
    CagdSrfStruct *Srf, *Srfs;
    TrimSrfStruct *TrimSrf, *TrimSrfs;
    TrivTVStruct *Trivar, *Trivars;
    TrngTriangSrfStruct *TriSrf, *TriSrfs;
    IPObjectStruct *PObj, *PTmp,
	*CrvObjs = FreeForms -> CrvObjs,
	*SrfObjs = FreeForms -> SrfObjs,
	*TrimSrfObjs = FreeForms -> TrimSrfObjs,
	*TrivarObjs = FreeForms -> TrivarObjs,
	*TriSrfObjs = FreeForms -> TriSrfObjs;
    IPPolygonStruct *PPolygon;

    /* Make sure requested format is something reasonable. */
    if (CrvObjs) {
	for (PObj = CrvObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    GetRelResolutions(PObj, &SamplesPerCurve, NumOfIsolines);

	    if (GlblTalkative)
		IRIT_INFO_MSG_PRINTF("Processing curve object \"%s\"\n",
			             IP_GET_OBJ_NAME(PObj));

	    Crvs = PObj -> U.Crvs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
		if (GlblDrawSurface) {
		    PPolygon = IPCurve2Polylines(Crv, SamplesPerCurve,
						 GlblPolylineOptiApprox);

		    PObj -> U.Pl = IPAppendPolyLists(PPolygon, PObj -> U.Pl);
		}
		if (GlblDrawSurfaceMesh) {
		    PPolygon = IPCurve2CtlPoly(Crv);
		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
	    }
	    CagdCrvFreeList(Crvs);
	}
    }

    if (SrfObjs) {
	for (PObj = SrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    GetRelResolutions(PObj, &SamplesPerCurve, NumOfIsolines);
	    ShadeBG = AttrGetRealAttrib(PObj -> Attr, "IllustrtShadeBG");

	    if (GlblTalkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
			             IP_GET_OBJ_NAME(PObj));
	    
	    Srfs = PObj -> U.Srfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		if (GlblDrawSurface) {
		    PPolygon = IPSurface2Polylines(Srf, NumOfIsolines,
						   SamplesPerCurve,
						   GlblPolylineOptiApprox);

		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
		if (GlblDrawSurfaceMesh) {
		    PPolygon = IPSurface2CtlMesh(Srf);
		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
	    }

	    if (!IP_ATTR_IS_BAD_REAL(ShadeBG)) {
	        PTmp = IPGenSRFObject(Srfs);
		PTmp -> Attr = AttrCopyAttributes(PObj -> Attr);
		AttrFreeOneAttribute(&PObj -> Attr, "IllustrtShadeBG");
		IRIT_LIST_PUSH(PTmp, GlblShadedObjects);
	    }
	    else
	        CagdSrfFreeList(Srfs);
	}
    }

    /* Converts models, if any, to freeform trimmed surfaces, in place. */
    if (IPProcessModel2TrimSrfs(FreeForms))
	TrimSrfObjs = FreeForms -> TrimSrfObjs;

    if (TrimSrfObjs) {
	for (PObj = TrimSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    GetRelResolutions(PObj, &SamplesPerCurve, NumOfIsolines);
	    ShadeBG = AttrGetRealAttrib(PObj -> Attr, "IllustrtShadeBG");

	    if (GlblTalkative)
		IRIT_INFO_MSG_PRINTF("Processing trim surface object \"%s\"\n",
			             IP_GET_OBJ_NAME(PObj));
	    
	    TrimSrfs = PObj -> U.TrimSrfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (TrimSrf = TrimSrfs;
		 TrimSrf != NULL;
		 TrimSrf = TrimSrf -> Pnext) {
		if (GlblDrawSurface) {
		    PPolygon = IPTrimSrf2Polylines(TrimSrf,
						   NumOfIsolines,
						   SamplesPerCurve,
						   GlblPolylineOptiApprox,
						   TRUE, TRUE);

		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
		if (GlblDrawSurfaceMesh) {
		    PPolygon = IPTrimSrf2CtlMesh(TrimSrf);
		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		  }
	    }

	    if (!IP_ATTR_IS_BAD_REAL(ShadeBG)) {
	        PTmp = IPGenTRIMSRFObject(TrimSrfs);
		PTmp -> Attr = AttrCopyAttributes(PObj -> Attr);
		AttrFreeOneAttribute(&PObj -> Attr, "IllustrtShadeBG");

		IRIT_LIST_PUSH(PTmp, GlblShadedObjects);
	    }
	    else
	        TrimSrfFreeList(TrimSrfs);
	}
    }

    if (TrivarObjs) {
	for (PObj = TrivarObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    GetRelResolutions(PObj, &SamplesPerCurve, NumOfIsolines);
	    ShadeBG = AttrGetRealAttrib(PObj -> Attr, "IllustrtShadeBG");

	    if (GlblTalkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
			             IP_GET_OBJ_NAME(PObj));
	    
	    Trivars = PObj -> U.Trivars;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (Trivar = Trivars; Trivar != NULL; Trivar = Trivar -> Pnext) {
		if (GlblDrawSurface) {
		    PPolygon = IPTrivar2Polylines(Trivar, NumOfIsolines,
						  SamplesPerCurve,
						  GlblPolylineOptiApprox);

		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
		if (GlblDrawSurfaceMesh) {
		    PPolygon = IPTrivar2CtlMesh(Trivar);
		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
	    }

	    if (!IP_ATTR_IS_BAD_REAL(ShadeBG)) {
	        PTmp = IPGenTRIVARObject(Trivars);
		PTmp -> Attr = AttrCopyAttributes(PObj -> Attr);
		AttrFreeOneAttribute(&PObj -> Attr, "IllustrtShadeBG");
		IRIT_LIST_PUSH(PTmp, GlblShadedObjects);
	    }
	    else
	        TrivTVFreeList(Trivars);
	}
    }

    if (TriSrfObjs) {
	for (PObj = TriSrfObjs; PObj != NULL; PObj = PObj -> Pnext) {
	    GetRelResolutions(PObj, &SamplesPerCurve, NumOfIsolines);
	    ShadeBG = AttrGetRealAttrib(PObj -> Attr, "IllustrtShadeBG");

	    if (GlblTalkative)
		IRIT_INFO_MSG_PRINTF("Processing surface object \"%s\"\n",
			             IP_GET_OBJ_NAME(PObj));

	    TriSrfs = PObj -> U.TriSrfs;
	    PObj -> U.Pl = NULL;
	    PObj -> ObjType = IP_OBJ_POLY;
	    IP_SET_POLYLINE_OBJ(PObj);
	    for (TriSrf = TriSrfs; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
		if (GlblDrawSurface) {
		    PPolygon = IPTriSrf2Polylines(TriSrf, NumOfIsolines,
						  SamplesPerCurve,
						  GlblPolylineOptiApprox);

		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
		if (GlblDrawSurfaceMesh) {
		    PPolygon = IPTriSrf2CtlMesh(TriSrf);
		    if (PPolygon != NULL)
		        PObj -> U.Pl = IPAppendPolyLists(PPolygon,
							 PObj -> U.Pl);
		}
	    }

	    if (!IP_ATTR_IS_BAD_REAL(ShadeBG)) {
	        PTmp = IPGenTRISRFObject(TriSrfs);
		PTmp -> Attr = AttrCopyAttributes(PObj -> Attr);
		AttrFreeOneAttribute(&PObj -> Attr, "IllustrtShadeBG");
		IRIT_LIST_PUSH(PTmp, GlblShadedObjects);
	    }
	    else
	        TrngTriSrfFreeList(TriSrfs);
	}
    }

    return IPConcatFreeForm(FreeForms);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Dumps the data out into FileName (stdout in NULL).			     *
*                                                                            *
* PARAMETERS:                                                                *
*   FileName:        Where output should go to.                              *
*   PObjects:        List of objects to dump out. 	                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpData(const char *FileName, IPObjectStruct *PObjects)
{
    FILE *f;

    if (FileName != NULL) {
	if ((f = fopen(FileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", FileName);
	    IllustrateExit(2);
	}
    }
    else
	f = stdout;

    if (!GlblBinaryOutput)
        fprintf(f, "\tIrit Solid Modeller Data File (ILLUSTRT), %s\n\n",
		IritRealTimeDate());

    while (PObjects) {
	/* Dump only polys with at least two vertices and points/vectors. */
	if (IP_IS_POLY_OBJ(PObjects)) {
	    if (IP_IS_POLYLINE_OBJ(PObjects))
		GMCleanUpPolylineList(&PObjects -> U.Pl, IRIT_EPS);
	    if (PObjects -> U.Pl != NULL)
		IPPutObjectToFile(f, PObjects, GlblBinaryOutput);
	}
	else if (IP_IS_POINT_OBJ(PObjects) ||
		 IP_IS_CTLPT_OBJ(PObjects) ||
		 IP_IS_VEC_OBJ(PObjects) ||
		 IP_IS_CRV_OBJ(PObjects) ||
		 IP_IS_SRF_OBJ(PObjects) ||
		 IP_IS_TRIMSRF_OBJ(PObjects) ||
		 IP_IS_TRIVAR_OBJ(PObjects) ||
		 IP_IS_TRISRF_OBJ(PObjects)) {
	    IPPutObjectToFile(f, PObjects, GlblBinaryOutput);
	}

	PObjects = PObjects -> Pnext;
    }

    fclose(f);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Creates an object representing speed wave from object PObj. SpeedWaveAttr  *
* Holds the attributes of the speed wave in the following format:	     *
* "GenRand,DirX,DirY,DirZ,Len,Dist,LenRand,DistRand,Width" with		     *
* See GenSpeedWave below.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:               Object for which a speed wave it to be generated.    *
*   SpeedWaveAttrs:     String describing a speed wave.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   Object representing a speed wave.		     *
*****************************************************************************/
static IPObjectStruct *ProcessSpeedWave(IPObjectStruct *PObj,
					const char *SpeedWaveAttrs)
{
    int i, NumPoints;
    char StrLineWidth[IRIT_LINE_LEN];
    IrtRType **Points, GenRand, Dir[3], Len, Dist, LenRand, DistRand, Width;
    CagdSrfStruct
	*Srf = NULL;
    CagdCrvStruct
	*Crv = NULL;
    IPObjectStruct *PObjTmp,  *PObjWave;
    IPPolygonStruct *P, *PWave;

#ifdef IRIT_DOUBLE
    if (sscanf(SpeedWaveAttrs, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
#else
    if (sscanf(SpeedWaveAttrs, "%f,%f,%f,%f,%f,%f,%f,%f,%f",
#endif /* IRIT_DOUBLE */
	       &GenRand, &Dir[0], &Dir[1], &Dir[2], &Len,
	       &Dist, &LenRand, &DistRand, &Width) != 9) {
	IRIT_WARNING_MSG("Wrong parameters of speed wave. Ignored.\n");
	return NULL;
    }

    PObjWave = IPAllocObject("SpeedWave", IP_OBJ_POLY, NULL);
    sprintf(StrLineWidth, "%f", Width);
    AttrSetObjectStrAttrib(PObjWave, "width", StrLineWidth);
    IP_SET_POLYLINE_OBJ(PObjWave);

    switch (PObj -> ObjType) {
	case IP_OBJ_LIST_OBJ:
	    for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++)
		ProcessSpeedWave(PObjTmp, SpeedWaveAttrs);
	    break;
	case IP_OBJ_POLY:
	    for (P = PObj -> U.Pl; P != NULL; P = P -> Pnext) {
		IPVertexStruct
		    *V = P -> PVertex;

		do {
		    if ((PWave = GenSpeedWave(V -> Coord, GenRand, Dir, Len,
					  Dist, LenRand, DistRand)) != NULL) {
			PWave -> Pnext = PObjWave -> U.Pl;
			PObjWave -> U.Pl = PWave;
		    }

		    V = V -> Pnext;
		}
		while (V != NULL && V != P -> PVertex);
	    }
	    break;
	case IP_OBJ_SURFACE:
	case IP_OBJ_CURVE:
	    if (PObj -> ObjType == IP_OBJ_SURFACE) {
		Srf = CagdCoerceSrfTo(PObj -> U.Srfs, CAGD_PT_E3_TYPE, FALSE);

		Points = Srf -> Points;
		NumPoints = Srf -> ULength * Srf -> VLength;
	    }
	    else {
		Crv = CagdCoerceCrvTo(PObj -> U.Crvs, CAGD_PT_E3_TYPE, FALSE);

		Points = Crv -> Points;
		NumPoints = Crv -> Length;
	    }

	    for (i = 0; i < NumPoints; i++) {
		int j;
		IrtRType Coords[3];

		for (j = 0; j < 3; j++)
		    Coords[j] = Points[j + 1][i];

		if ((PWave = GenSpeedWave(Coords, GenRand, Dir, Len,
					  Dist, LenRand, DistRand)) != NULL) {
		    PWave -> Pnext = PObjWave -> U.Pl;
		    PObjWave -> U.Pl = PWave;
		}
	    }

	    if (PObj -> ObjType == IP_OBJ_SURFACE) {
		CagdSrfFree(Srf);
	    }
	    else {
		CagdCrvFree(Crv);
	    }
	    break;
	default:
	    break;
    }

    if (PObjWave -> U.Pl != NULL)
	return PObjWave;
    else {
	IPFreeObject(PObjWave);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Randomly generates a single speed wave instance.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Coords:             Location for which the wave is to be generated.      *
*   GenRand:            Probability of generating the wave.		     *
*   Dir:                Direction of wave.				     *
*   Len, Dist:          Length of the wave and distance from object.	     *
*   LenRand, DistRand:  Pertubation amount Len and Dist.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  An object representing one speed wave object         *
*                       in probability GenRand, NULL otherwise.              *
*****************************************************************************/
static IPPolygonStruct *GenSpeedWave(IrtRType Coords[3],
				     IrtRType GenRand,
				     IrtRType Dir[3],
				     IrtRType Len,
				     IrtRType Dist,
				     IrtRType LenRand,
				     IrtRType DistRand)
{
    IrtVecType DirCopy;
    IPVertexStruct *V1, *V2;
    IPPolygonStruct *P;

    /* Test if probability is in favor. */
    if (IritRandom(0.0, 1.0) > GenRand)
	return NULL;

    V2 = IPAllocVertex2(NULL);
    V1 = IPAllocVertex2(V2);
    P = IPAllocPolygon(0, V1, NULL);

    IRIT_PT_COPY(DirCopy, Dir);
    IRIT_PT_NORMALIZE(DirCopy);
    Dist += DistRand * IritRandom(-1.0, 1.0);
    IRIT_PT_SCALE(DirCopy, Dist);
    IRIT_PT_COPY(V1 -> Coord, Coords);
    IRIT_PT_ADD(V1 -> Coord, V1 -> Coord, DirCopy);

    IRIT_PT_COPY(DirCopy, Dir);
    IRIT_PT_NORMALIZE(DirCopy);
    Len += LenRand * IritRandom(-1.0, 1.0);
    IRIT_PT_SCALE(DirCopy, Len);
    IRIT_PT_COPY(V2 -> Coord, V1 -> Coord);
    IRIT_PT_ADD(V2 -> Coord, V2 -> Coord, DirCopy);

    return P;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Creates an object representing heat wave from object PObj. HeatWaveAttr    *
* Holds the attributes of the heat wave in the following format:	     *
* "GenRand,Len,Dist,LenRand,DistRand,Width" with			     *
* See GenHeatWave below.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:               Object for which a heat wave it to be generated.     *
*   SpeedWaveAttrs:     String describing a heat wave.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   Object representing a heat wave.		     *
*****************************************************************************/
static IPObjectStruct *ProcessHeatWave(IPObjectStruct *PObj,
				       const char *HeatWaveAttrs)
{
    int i, NumPoints;
    IrtRType **Points, GenRand, Len, Dist, LenRand, DistRand, Width;
    CagdSrfStruct
	*Srf = NULL;
    CagdCrvStruct *PWave,
	*Crv = NULL;
    IPObjectStruct *PObjTmp, *PObjWave;
    IPPolygonStruct *P;

#ifdef IRIT_DOUBLE
    if (sscanf(HeatWaveAttrs, "%lf,%lf,%lf,%lf,%lf,%lf",
#else
    if (sscanf(HeatWaveAttrs, "%f,%f,%f,%f,%f,%f",
#endif /* IRIT_DOUBLE */
	       &GenRand, &Len, &Dist, &LenRand, &DistRand, &Width) != 6) {
	IRIT_WARNING_MSG("Wrong parameters of heat wave. Ignored.\n");
	return NULL;
    }

    PObjWave = IPAllocObject("HeatWave", IP_OBJ_CURVE, NULL);
    AttrSetObjectRealAttrib(PObjWave, "width", Width);

    switch (PObj -> ObjType) {
	case IP_OBJ_LIST_OBJ:
	    for (i = 0; (PObjTmp = IPListObjectGet(PObj, i)) != NULL; i++)
		ProcessHeatWave(PObjTmp, HeatWaveAttrs);
	    break;
	case IP_OBJ_POLY:
	    for (P = PObj -> U.Pl; P != NULL; P = P -> Pnext) {
		IPVertexStruct
		    *V = P -> PVertex;

		do {
		    if ((PWave = GenHeatWave(V -> Coord, GenRand, Len,
					  Dist, LenRand, DistRand)) != NULL) {
			PWave -> Pnext = PObjWave -> U.Crvs;
			PObjWave -> U.Crvs = PWave;
		    }

		    V = V -> Pnext;
		}
		while (V != NULL && V != P -> PVertex);
	    }
	    break;
	case IP_OBJ_SURFACE:
	case IP_OBJ_CURVE:
	    if (PObj -> ObjType == IP_OBJ_SURFACE) {
		Srf = CagdCoerceSrfTo(PObj -> U.Srfs, CAGD_PT_E3_TYPE, FALSE);

		Points = Srf -> Points;
		NumPoints = Srf -> ULength * Srf -> VLength;
	    }
	    else {
		Crv = CagdCoerceCrvTo(PObj -> U.Crvs, CAGD_PT_E3_TYPE, FALSE);

		Points = Crv -> Points;
		NumPoints = Crv -> Length;
	    }

	    for (i = 0; i < NumPoints; i++) {
		int j;
		IrtRType Coords[3];

		for (j = 0; j < 3; j++)
		    Coords[j] = Points[j + 1][i];

		if ((PWave = GenHeatWave(Coords, GenRand, Len,
					 Dist, LenRand, DistRand)) != NULL) {
		    PWave -> Pnext = PObjWave -> U.Crvs;
		    PObjWave -> U.Crvs = PWave;
		}
	    }

	    if (PObj -> ObjType == IP_OBJ_SURFACE) {
		CagdSrfFree(Srf);
	    }
	    else {
		CagdCrvFree(Crv);
	    }
	    break;
	default:
	    break;
    }

    if (PObjWave -> U.Pl != NULL)
	return PObjWave;
    else {
	IPFreeObject(PObjWave);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Randomly generates a single heat wave instance.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Coords:             Location for which the wave is to be generated.      *
*   GenRand:            Probability of generating the wave.		     *
*   Len, Dist:          Length of the wave and distance from object.	     *
*   LenRand, DistRand:  Pertubation amount Len and Dist.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:    An object representing one heat wave object          *
*                       in probability GenRand, NULL otherwise.              *
*****************************************************************************/
static CagdCrvStruct *GenHeatWave(IrtRType Coords[3],
				  IrtRType GenRand,
				  IrtRType Len,
				  IrtRType Dist,
				  IrtRType LenRand,
				  IrtRType DistRand)
{
    IRIT_STATIC_DATA IrtVecType
	HeatCtlPts[HEAT_CRV_NUM_PTS] =
	{
	    { 0.0, 0.0, 0.0 },
	    { 0.1, 0.0, 0.25 },
	    { 0.0, 0.1, 0.3 },
	    { 0.0, 0.0, 0.2 },
	    { 0.1, 0.0, 0.35 },
	    { 0.0, 0.0, 0.5 },
	    { 0.0, 0.0, 0.35 },
	    { 0.1, 0.1, 0.5 },
	    { 0.1, 0.0, 0.6 },
	};
    int i, j;
    CagdCrvStruct *Crv;

    /* Test if probability is in favor. */
    if (IritRandom(0.0, 1.0) > GenRand)
	return NULL;

    Len += LenRand * IritRandom(-1.0, 1.0);
    Dist += DistRand * IritRandom(-1.0, 1.0);

    Crv = BspCrvNew(HEAT_CRV_NUM_PTS, 3, CAGD_PT_E3_TYPE);
    BspKnotUniformOpen(HEAT_CRV_NUM_PTS, 3, Crv -> KnotVector);
    for (i = 0; i < HEAT_CRV_NUM_PTS; i++)
	for (j = 0; j < 3; j++)
	    Crv -> Points[j + 1][i] = Coords[j] + (j == 2 ? Dist : 0.0) + 
		HeatCtlPts[i][j] * Len;

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Illustrt Exit routine.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   ExitCode:    To notify O.S. with result of program.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IllustrateExit                                                           M
*****************************************************************************/
void IllustrateExit(int ExitCode)
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
