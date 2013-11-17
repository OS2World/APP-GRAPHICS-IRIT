/*****************************************************************************
*   Program to compute hidden curve removal of freeform surfaces.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan. 2000   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "misc_lib.h"
#include "mdl_lib.h"
#include "trim_lib.h"
#include "ip_cnvrt.h"

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "IHidden	Version 11,	Gershon Elber,\n\
	(C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "IHidden	" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",  " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char *CtrlStr =
    "ihidden q%- H%- M%- I%-#UIso[:#VIso[:#WIso]]!s d%- s%-Stage!d b%- o%-OutName!s t%-Tolerance!F Z%-ZBufSz!d T%-AnimTime!F z%- DFiles!*s";

IRIT_STATIC_DATA const char
    *GlblStrNumOfIsolines = "10:10:10";

IRIT_STATIC_DATA const IrtVecType
    GlblZDir = { 0, 0, 1 };

/* The following are setable variables (via configuration file ihidden.cfg). */
IRIT_GLOBAL_DATA int
    GlblQuiet = FALSE,
    GlblBinaryOutput = FALSE,
    GlblOutputInvisible = FALSE,
    GlblMonotoneCrvs = FALSE,
    GlblDisplayDiscontCurves = TRUE,
    GlblNumOfIsolines[3] = { IG_DEFAULT_NUM_OF_ISOLINES,
			     IG_DEFAULT_NUM_OF_ISOLINES,
			     IG_DEFAULT_NUM_OF_ISOLINES },
    GlblStopStage = IHID_STAGE_RAY_SRF_INTER,
    GlblScrnRSIFineness = IHID_DEF_SCRN_RSI_FINENESS;

IRIT_GLOBAL_DATA IrtRType
    GlblIHidTolerance = IHID_DEF_IHID_TOLERANCE;

IRIT_GLOBAL_DATA IrtHmgnMatType GlblViewMat;		  /* Current view of object. */

IRIT_STATIC_DATA int
    GlblSegUniqueID = 1;

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
    { "Quiet",	      "-q", (VoidPtr) &GlblQuiet,	    IC_BOOLEAN_TYPE },
    { "DumpHidden",   "-H", (VoidPtr) &GlblOutputInvisible, IC_BOOLEAN_TYPE },
    { "NumOfIsolines","-I", (VoidPtr) &GlblStrNumOfIsolines, IC_STRING_TYPE },
    { "DiscontDisplay","-d",(VoidPtr) &GlblDisplayDiscontCurves,IC_BOOLEAN_TYPE },
    { "BinaryOutput", "-b", (VoidPtr) &GlblBinaryOutput,    IC_BOOLEAN_TYPE },
    { "MonotoneCrvs", "-M", (VoidPtr) &GlblMonotoneCrvs,    IC_BOOLEAN_TYPE },
    { "Tolerance",    "-t", (VoidPtr) &GlblIHidTolerance,   IC_REAL_TYPE },
    { "ZFineNess",    "-Z", (VoidPtr) &GlblScrnRSIFineness, IC_INTEGER_TYPE }
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

static void ObjectProcessing(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static CagdCrvStruct *MakeActiveCrvsMonotone(CagdCrvStruct *CrvGeom);
static CagdCrvStruct *ExtractCurveGeometry(IPObjectStruct *PObjects);
static CagdCrvStruct *ExtractSilCrvsFromSrf(const CagdSrfStruct *Srf);
static CagdCrvStruct *ExtractCrvsFromSrf(const IPObjectStruct *POrigObj,
					 const CagdSrfStruct *Srf);
static CagdCrvStruct *ExtractCrvsFromTrimSrf(const IPObjectStruct *POrigObj,
					     const TrimSrfStruct *TSrf);
static CagdCrvStruct *EvalCurvesIntoEuclidean(CagdCrvStruct *UVCrvs,
					      const CagdSrfStruct *Srf,
					      const IPObjectStruct *POrigObj,
					      IHidCrvType CType);
static CagdRType *DeriveIsoParamVals(CagdRType Min,
				     CagdRType Max,
				     int NumC1Disconts,
				     CagdRType *C1Disconts,
				     int *NumIso);
static CagdCrvStruct *GenIsoCrvSegs(const CagdSrfStruct *Srf,
				    CagdRType t,
				    CagdSrfDirType Dir,
				    const IPObjectStruct *POrigObj,
				    IHidCrvType CType,
				    TrimIsoInterStruct *TrimIsoInters,
				    TrimIsoInterStruct *IsoInters);
static void UpdateAttributesAndDumpOut(CagdCrvStruct *Crvs, int OutStream);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of ihidden - Read command line and do what is needed...	     M
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
    int Error, OutStream,
	NumOfIsolinesFlag = FALSE,
	VerFlag = FALSE,
	StopStageFlag = FALSE,
	OutFileFlag = FALSE,
	IFineNess = FALSE,
	ZFineNess = FALSE,
        TimeFlag = FALSE,
        NumFiles = FALSE;
    char
	*OutFileName = NULL,
	**FileNames = NULL;
    IrtRType CurrentTime;
    FILE *OutFile;
    CagdCrvStruct *CrvGeom;
    IPObjectStruct *PObjects, *PObjs;
    IrtHmgnMatType Mat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IritConfig("ihidden", SetUp, NUM_SET_UP); /* Read config. file if exists.*/

    if ((Error = GAGetArgs(argc, argv, CtrlStr, &GlblQuiet,
			   &GlblOutputInvisible, &GlblMonotoneCrvs,
			   &NumOfIsolinesFlag, &GlblStrNumOfIsolines,
			   &GlblDisplayDiscontCurves,
			   &StopStageFlag, &GlblStopStage, &GlblBinaryOutput,
			   &OutFileFlag, &OutFileName,
			   &IFineNess, &GlblIHidTolerance,
			   &ZFineNess, &GlblScrnRSIFineness,
			   &TimeFlag, &CurrentTime, &VerFlag, &NumFiles,
			   &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	IHiddenExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	IritConfigPrint(SetUp, NUM_SET_UP);
	IHiddenExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names were given, exit\n");
	GAPrintHowTo(CtrlStr);
	IHiddenExit(1);
    }

    if (NumOfIsolinesFlag && GlblStrNumOfIsolines != NULL) {
	if (sscanf(GlblStrNumOfIsolines, "%d:%d:%d",
		   &GlblNumOfIsolines[0],
		   &GlblNumOfIsolines[1],
		   &GlblNumOfIsolines[2]) != 3) {
	    if (sscanf(GlblStrNumOfIsolines, "%d:%d",
		       &GlblNumOfIsolines[0],
		       &GlblNumOfIsolines[1]) != 2) {
		if (sscanf(GlblStrNumOfIsolines, "%d",
			   &GlblNumOfIsolines[1]) != 1) {
		    IRIT_WARNING_MSG(
			    "Number(s) of isolines (-I) cannot be parsed.\n");
		    GAPrintHowTo(CtrlStr);
		    IHiddenExit(1);
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
    if ((PObjects = IPGetDataFiles((const char **) FileNames, NumFiles,
				   TRUE, !GlblQuiet)) == NULL)
	IHiddenExit(1);
    PObjects = IPResolveInstances(PObjects);

    MatGenUnitMat(Mat);
    IPTraverseObjListHierarchy(PObjects, Mat, ObjectProcessing);

    /* Do we have animation time flag? */
    if (TimeFlag)
        GMAnimEvalAnimationList(CurrentTime, PObjects);
    else
        GMAnimEvalAnimationList(GM_ANIM_NO_DEFAULT_TIME, PObjects);

    IPFlattenInvisibleObjects(FALSE);
    PObjects = IPFlattenForrest(PObjects);

    BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

    /* Update the global viewing matrix and transform all objects: */
    if (IPWasPrspMat)
	MatMultTwo4by4(GlblViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(GlblViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    PObjs = GMTransformObjectList(PObjects, GlblViewMat);
    IPFreeObjectList(PObjects);
    PObjects = PObjs;

    if (OutFileFlag) {
	if ((OutFile = fopen(OutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", OutFileName);
	    IHiddenExit(2);
	}
    }
    else
	OutFile = stdout;
    OutStream = IPOpenStreamFromFile(OutFile, FALSE, GlblBinaryOutput,
				     FALSE, FALSE);

    /* Stage 1 - extract desired curves: */
    if (!GlblQuiet)
        IRIT_INFO_MSG("Stage 1 - Extracting curves:  \b");
    CrvGeom = ExtractCurveGeometry(PObjects);

    if (!GlblQuiet)
        IRIT_INFO_MSG_PRINTF(" %d curves\n", CagdListLength(CrvGeom));

    if (GlblMonotoneCrvs)
	CrvGeom = MakeActiveCrvsMonotone(CrvGeom);

    if (GlblStopStage == IHID_STAGE_CRV_EXTRACT)
        UpdateAttributesAndDumpOut(CrvGeom, OutStream);
    else {
        CagdCrvStruct *CrvGeom2;

	if (!GlblQuiet)
	    IRIT_INFO_MSG("Stage 2 - Intersecting curves:  \b");
	CrvGeom2 = CrvCrvIntersections(CrvGeom, GlblIHidTolerance);

	if (GlblStopStage == IHID_STAGE_CRV_CRV_INTER)
	    UpdateAttributesAndDumpOut(CrvGeom2, OutStream);
	else {   /* IHID_STAGE_RAY_SRF_INTER */
	    if (!GlblQuiet)
	        IRIT_INFO_MSG("\nStage 3 - Visibility examination:  \b");
	    RaySrfIntersections(PObjects, CrvGeom2);

	    UpdateAttributesAndDumpOut(CrvGeom2, OutStream);
	}
    }

    IPCloseStream(OutStream, TRUE);

    IHiddenExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Preprocess the geometry.  Operation done are:                            *
*   1.  All models are converted in place to trimmed surfaces.               *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Objects to process.                                              *
*   Mat:    Transformation matrix.  Ignored.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ObjectProcessing(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    /* Convert, in place, models to lists of trimmed surfaces. */
    if (IP_IS_MODEL_OBJ(PObj)) {
        MdlModelStruct
	    *Mdl = PObj -> U.Mdls;
        MdlTrimSegStruct *Seg,
	    *MdlSegs = Mdl -> TrimSegList;
	TrimSrfStruct *TrimSrfs;
	IPObjectStruct *TSrfsObjs;

	/* Mark all trimming segments with unique IDs. */
	for (Seg = MdlSegs;
	     Seg != NULL;
	     Seg = Seg -> Pnext, GlblSegUniqueID++) {
	    if (Seg -> UVCrvFirst != NULL)
	        AttrSetIntAttrib(&Seg -> UVCrvFirst -> Attr,
				 "MdlID", GlblSegUniqueID);
	    if (Seg -> UVCrvSecond != NULL)
	        AttrSetIntAttrib(&Seg -> UVCrvSecond -> Attr,
				 "MdlID", GlblSegUniqueID);
	    if (Seg -> EucCrv != NULL)
	        AttrSetIntAttrib(&Seg -> EucCrv -> Attr,
				 "MdlID", GlblSegUniqueID);
	}

        TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(Mdl);
	TSrfsObjs = IPLnkListToListObject(TrimSrfs, IP_OBJ_TRIMSRF);

	/* Update the original object with the computed trimmed surfaces. */
	IPFreeObjectGeomData(PObj);
	PObj -> ObjType = IP_OBJ_LIST_OBJ;
	PObj -> U.Lst = TSrfsObjs -> U.Lst;

	/* And free the original list object. */
	TSrfsObjs -> ObjType = IP_OBJ_NUMERIC;
	IPFreeObject(TSrfsObjs);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Make sure all active curve are monotone and cannot self intersect.       *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvGeom:    Curve geometry extracted from the surfaces in the scene.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Same curve geometry but with monotone curve geometry. *
*****************************************************************************/
static CagdCrvStruct *MakeActiveCrvsMonotone(CagdCrvStruct *CrvGeom)
{
    CagdCrvStruct *Crv, *DCrv,
	*RetGeom = NULL;

    while (CrvGeom) {
	int CType;

        IRIT_LIST_POP(Crv, CrvGeom);
	CType = AttrGetIntAttrib(Crv -> Attr, "ctype");

	if (CType == IHID_CURVE_BOUNDARY ||
	    CType == IHID_CURVE_SILHOUETTE) {
	    if (Crv -> Order <= 2) {
		int i, SplitCrv;
	        CagdRType Prev, This, Next, TMin, TMax, t;

		do {
		    CagdRType
		        *WPts = Crv -> Points[0],
		        *XPts = Crv -> Points[1];

		    SplitCrv = FALSE;

		    CagdCrvDomain(Crv, &TMin, &TMax);

		    Prev = WPts != NULL ? XPts[0] / WPts[0] : XPts[0];
		    This = WPts != NULL ? XPts[1] / WPts[1] : XPts[1];
		    for (i = 2; i < Crv -> Length; i++) {
		        Next = WPts != NULL ? XPts[i] / WPts[i] : XPts[i];

			if ((This - Prev) * (This - Next) > 0) {
			    CagdCrvStruct *Crv1, *Crv2;

			    t = TMin +
				(TMax - TMin) * i / (Crv -> Length - 1.0);

			    if (CCIUpdateSubdivCrvs(Crv, t, &Crv1, &Crv2)) {
			        IRIT_LIST_PUSH(Crv1, RetGeom);
				CagdCrvFree(Crv);
				Crv = Crv2;
				SplitCrv = TRUE;
				break;
			    }
			}
			Prev = This;
			This = Next;
		    }
		}
		while (SplitCrv);
	    }
	    else {
	        const SymbNormalConeStruct
		    *TangCone = SymbTangentConeForCrv(Crv, TRUE);

		if (TangCone == NULL || TangCone -> ConeAngle >= M_PI * 0.5) {
		    CagdPtStruct *Pts, *Pt;

		    /* Crv's angular span >= 90 degrees, split at X'(t) = 0. */
		    DCrv = CagdCrvDerive(Crv);
		    Pts = SymbCrvZeroSet(DCrv, 1, GlblIHidTolerance, TRUE);
		    CagdCrvFree(DCrv);
		    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
		        CagdCrvStruct *Crv1, *Crv2;

			if (CCIUpdateSubdivCrvs(Crv, Pt -> Pt[0],
						&Crv1, &Crv2)) {
			    IRIT_LIST_PUSH(Crv1, RetGeom);
			    CagdCrvFree(Crv);
			    Crv = Crv2;
			}		  
		    }
		    CagdPtFreeList(Pts);
		}
	    }
	}

	IRIT_LIST_PUSH(Crv, RetGeom);
    }

    return RetGeom;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extract the different geometric curves requested from the surface        *
* geometry.                                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjects:   To extract curve geometry from.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Curve geometry tagged using "ctype" attribute.        *
*****************************************************************************/
static CagdCrvStruct *ExtractCurveGeometry(IPObjectStruct *PObjects)
{
    int i;
    IPObjectStruct *PObj;
    CagdCrvStruct *Crv,
	*CrvE3Geom = NULL,
	*CrvGeom = NULL;
    CagdSrfStruct **Srfs;

    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
        switch (PObj -> ObjType) {
	    case IP_OBJ_CURVE:
		Crv = CagdCrvCopy(PObj -> U.Crvs);
		IHID_MARK_CTYPE(Crv, PObj, IHID_CURVE_INDEPNDNT);
		break;
	    case IP_OBJ_SURFACE:
		CrvGeom = CagdListAppend(ExtractCrvsFromSrf(PObj,
							    PObj -> U.Srfs),
					 CrvGeom);
		break;
	    case IP_OBJ_TRIMSRF:
		CrvGeom =
		    CagdListAppend(ExtractCrvsFromTrimSrf(PObj,
							  PObj -> U.TrimSrfs),
				   CrvGeom);
		break;
	    case IP_OBJ_TRIVAR:
		Srfs = TrivBndrySrfsFromTV(PObj -> U.Trivars);

		for (i = 0; i < 6; i++) {
		    CrvGeom = CagdListAppend(ExtractCrvsFromSrf(PObj,
								Srfs[i]),
					     CrvGeom);
		    CagdSrfFree(Srfs[i]);
		}
		break;
	    case IP_OBJ_TRISRF:
		break;
	    case IP_OBJ_MODEL:
	        /* Models should be converted to trimmed srfs at this time. */
	        assert(0);
		break;
	    default:
		break;
	}
    }

    /* Coerce to E3 curves that one could coerce to E3 (similar weights). */
    while (CrvGeom != NULL) {
	IRIT_LIST_POP(Crv, CrvGeom);

	if (CAGD_IS_RATIONAL_CRV(Crv) &&
	    CagdAllWeightsSame(Crv -> Points, Crv -> Length)) {
	    CagdCrvStruct
	        *TCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E3_TYPE, FALSE);

	    /* Transfer the attributes. */
	    TCrv -> Attr = Crv -> Attr;
	    Crv -> Attr = NULL;

	    CagdCrvFree(Crv);
	    IRIT_LIST_PUSH(TCrv, CrvE3Geom);
	}
	else {
	    IRIT_LIST_PUSH(Crv, CrvE3Geom);
	}
    }

    return CrvE3Geom;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extract the silhouette curves from the given surface geometry.           *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:       To extract silhouette curve geometry from.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Extracted silhouette curve geometry.                  *
*****************************************************************************/
static CagdCrvStruct *ExtractSilCrvsFromSrf(const CagdSrfStruct *Srf)
{
    IPObjectStruct
        *PObjSils = MvarSrfSilhouette(Srf, GlblZDir,
				      GlblIHidTolerance * 10.0,
				      GlblIHidTolerance * 10.0,
				      GlblIHidTolerance * 0.01, FALSE);
    IPPolygonStruct *Pl, *PSils;
    CagdCrvStruct *Crv1,
        *SilGeom = NULL;

    if (PObjSils == NULL)
	return NULL;

    PSils = PObjSils -> U.Pl;
    PObjSils -> U.Pl = NULL;
    IPFreeObject(PObjSils);

    GMCleanUpPolylineList(&PSils, IRIT_EPS);      /* Clean the silhouettes. */

    for (Pl = PSils; Pl != NULL; Pl = Pl -> Pnext) {
        Crv1 = IPPolyline2Curve(Pl, 2);
        IRIT_LIST_PUSH(Crv1, SilGeom);
    }

    IPFreePolygonList(PSils);

    return SilGeom;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extract the curves requested from the given surface geometry.            *
*   The curves are intersected here against the silhouette curves of the     *
* surface due to the instability of conducting these intersection in E3.     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Reference to original object.                                 *
*   Srf:       To extract curve geometry from.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Extracted curve geometry.                             *
*****************************************************************************/
static CagdCrvStruct *ExtractCrvsFromSrf(const IPObjectStruct *POrigObj,
					 const CagdSrfStruct *Srf)
{
    int i, j, l, NumIsos[2], NumC1Disconts;
    CagdRType UMin, UMax, VMin, VMax, *C1Disconts, *IsoParams,
	RelNumIsos = AttrGetObjectRealAttrib(POrigObj, "num_of_isolines");
    CagdCrvStruct *Crvs,
	*SilGeom = NULL,
	*CrvGeom = NULL;
    CagdSrfStruct
        *NewSrf = NULL;
    TrimIsoInterStruct **IsoInters, **DIsoInters;

    /* Make sure surface is open ended Bspline surface. */
    if (CAGD_IS_BSPLINE_SRF(Srf) && !BspSrfHasOpenEC(Srf)) {
	Srf = NewSrf = BspSrfOpenEnd(Srf);
    }
    if (CAGD_IS_BEZIER_SRF(Srf)) {
        CagdSrfStruct
	    *TSrf = CagdCnvrtBzr2BspSrf(Srf);

	if (NewSrf != NULL)
	    CagdSrfFree(NewSrf);
	Srf = NewSrf = TSrf;
    }

    BspSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Extract silhouette curves. */
    SilGeom = ExtractSilCrvsFromSrf(Srf);

    /* Extract isoparametric curves, including boundary.  We compute the    */
    /* intersections with the silhouette curves, in the parametric domain   */
    /* which is far more stable and robust.				    */
    if (!IP_ATTR_IS_BAD_REAL(RelNumIsos)) {
        NumIsos[0] = (int) (RelNumIsos * GlblNumOfIsolines[0]);
	NumIsos[1] = (int) (RelNumIsos * GlblNumOfIsolines[1]);
    }
    else {
        NumIsos[0] = GlblNumOfIsolines[0];
	NumIsos[1] = GlblNumOfIsolines[1];
    }
    /* Allow no isos or at least two isos. */
    NumIsos[0] = IRIT_MAX(NumIsos[0], 0);
    NumIsos[1] = IRIT_MAX(NumIsos[1], 0);
    if (NumIsos[0] == 1)
	NumIsos[0] = 2;
    if (NumIsos[1] == 1)
	NumIsos[1] = 2;

    /* Do the U/V constant boundary/discont/interior isoparametric curves. */
    for (l = 0; l < 2; l++) {	   /* l == 0 for U, l == 1 for V direction. */
	int Length, Order;
	CagdSrfDirType Dir;
	CagdRType *KV, Min, Max;

        switch (l) {
	    default:
	    case 0:
		KV = Srf -> UKnotVector;
		Length = Srf -> ULength;
		Order = Srf -> UOrder;
		Dir = CAGD_CONST_U_DIR;
		Min = UMin;
		Max = UMax;
	        break;
	    case 1:
		KV = Srf -> VKnotVector;
		Length = Srf -> VLength;
		Order = Srf -> VOrder;
		Dir = CAGD_CONST_V_DIR;
		Min = VMin;
		Max = VMax;
	        break;
	}
	if (!CagdIsClosedSrf(Srf, Dir) && NumIsos[l] == 0)
	    NumIsos[l] = 2; /* For the boundary. */

	/* Extract discontinuity curves. */
	if (GlblDisplayDiscontCurves) {
	    C1Disconts = BspKnotAllC1Discont(KV, Order, Length,
					     &NumC1Disconts);
	    if (NumC1Disconts > 0) {
	        DIsoInters = TrimIntersectCrvsIsoVals(SilGeom, Dir,
						      C1Disconts,
						      NumC1Disconts);
		for (i = 0; i < NumC1Disconts; i++) {
		    Crvs = GenIsoCrvSegs(Srf, C1Disconts[i], Dir, POrigObj,
					 IHID_CURVE_DISCONT, NULL,
					 DIsoInters[i]);
		    CrvGeom = CagdListAppend(Crvs, CrvGeom);
		}
		IritFree(DIsoInters);
	    }
	}
	else {
	    NumC1Disconts = 0;
	    C1Disconts = NULL;
	}

	IsoParams = DeriveIsoParamVals(Min, Max, NumC1Disconts, C1Disconts,
				       &NumIsos[l]);

	/* Find all U intersections of isocurves with silhouettes. */
	if (NumIsos[l] > 0)
	    IsoInters = TrimIntersectCrvsIsoVals(SilGeom, Dir, IsoParams,
						 NumIsos[l]);
	else {
	    IsoParams = NULL;
	    IsoInters = NULL;
	}

	/* Do the boundary curves. */
	if (!CagdIsClosedSrf(Srf, Dir)) {
	    /* This is an open srf in this dir. - make two boundaries.  */
	    Crvs = GenIsoCrvSegs(Srf, Min, Dir, POrigObj,
				 IHID_CURVE_BOUNDARY, NULL, IsoInters[0]);
	    CrvGeom = CagdListAppend(Crvs, CrvGeom);

	    Crvs = GenIsoCrvSegs(Srf, Max, Dir, POrigObj,
				 IHID_CURVE_BOUNDARY, NULL,
				 IsoInters[NumIsos[l] - 1]);
	    CrvGeom = CagdListAppend(Crvs, CrvGeom);
	}

	/* Do the interior isoparametric curves. */
	for (i = 1; i < NumIsos[l] - 1; i++) {
	    /* Lets see if we have a discontinuity here. */
	    for (j = 0; j < NumC1Disconts; j++)
	        if (IRIT_APX_EQ(C1Disconts[j], IsoParams[i]))
		    break;
	    if (j < NumC1Disconts)
	        continue;		 /* Indeed this is a discontinuity. */

	    Crvs = GenIsoCrvSegs(Srf, IsoParams[i], Dir, POrigObj,
				 IHID_CURVE_ISOPARAM, NULL, IsoInters[i]);
	    CrvGeom = CagdListAppend(Crvs, CrvGeom);
	}

	if (IsoParams != NULL)
	    IritFree(IsoParams);
	if (IsoInters != NULL)
	    IritFree(IsoInters);
	if (C1Disconts != NULL)
	    IritFree(C1Disconts);
    }

    /* Evaluate the silhouette curve into E3 and append to global list. */
    SilGeom = EvalCurvesIntoEuclidean(SilGeom, Srf, POrigObj,
				      IHID_CURVE_SILHOUETTE);
    CrvGeom = CagdListAppend(SilGeom, CrvGeom);

    if (NewSrf != NULL)
        CagdSrfFree(NewSrf);

    return CrvGeom;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extract the curves requested from the given trimmed surface geometry.    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Reference to original object.                                 *
*   TSrf:       To extract curve geometry from.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Extracted curve geometry.                             *
*****************************************************************************/
static CagdCrvStruct *ExtractCrvsFromTrimSrf(const IPObjectStruct *POrigObj,
					     const TrimSrfStruct *TSrf)
{    
    int i, j, l, NumIsos[2], NumC1Disconts;
    CagdRType UMin, UMax, VMin, VMax, *C1Disconts, *IsoParams,
	RelNumIsos = AttrGetObjectRealAttrib(POrigObj, "num_of_isolines");
    CagdCrvStruct *Crv1, *Crvs, *TCrvs, *TrimCrvs,
	*SilGeom = NULL,
	*CrvGeom = NULL;
    CagdSrfStruct
	*NewSrf = NULL,
        *Srf = TSrf -> Srf;
    TrimIsoInterStruct **IsoInters, **TIsoInters, **DIsoInters;

    /* Make sure surface is open ended Bspline surface. */
    if (CAGD_IS_BSPLINE_SRF(Srf) && !BspSrfHasOpenEC(Srf)) {
	Srf = NewSrf = BspSrfOpenEnd(Srf);
    }
    if (CAGD_IS_BEZIER_SRF(Srf)) {
        CagdSrfStruct
	    *TmpSrf = CagdCnvrtBzr2BspSrf(Srf);

	if (NewSrf != NULL)
	    CagdSrfFree(NewSrf);
	Srf = NewSrf = TmpSrf;
    }

    BspSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Extract silhouette curves. */
    SilGeom = ExtractSilCrvsFromSrf(Srf);

    /* Extract isoparametric curves, including boundary.  We compute the    */
    /* intersections with the silhouette curves, in the parametric domain   */
    /* which is far more stable and robust.				    */
    if (!IP_ATTR_IS_BAD_REAL(RelNumIsos)) {
        NumIsos[0] = (int) (RelNumIsos * GlblNumOfIsolines[0]);
	NumIsos[1] = (int) (RelNumIsos * GlblNumOfIsolines[1]);
    }
    else {
        NumIsos[0] = GlblNumOfIsolines[0];
	NumIsos[1] = GlblNumOfIsolines[1];
    }
    /* Allow no isos or at least two isos. */
    NumIsos[0] = IRIT_MAX(NumIsos[0], 0);
    NumIsos[1] = IRIT_MAX(NumIsos[1], 0);
    if (NumIsos[0] == 1)
	NumIsos[0] = 2;
    if (NumIsos[1] == 1)
	NumIsos[1] = 2;

    /* Do the U/V constant boundary/discont/interior isoparametric curves.  */
    for (l = 0; l < 2; l++) {	   /* l == 0 for U, l == 1 for V direction. */
	int Length, Order;
	CagdSrfDirType Dir;
	CagdRType *KV, Min, Max;

        switch (l) {
	    default:
	    case 0:
		KV = Srf -> UKnotVector;
		Length = Srf -> ULength;
		Order = Srf -> UOrder;
		Dir = CAGD_CONST_U_DIR;
		Min = UMin;
		Max = UMax;
	        break;
	    case 1:
		KV = Srf -> VKnotVector;
		Length = Srf -> VLength;
		Order = Srf -> VOrder;
		Dir = CAGD_CONST_V_DIR;
		Min = VMin;
		Max = VMax;
	        break;
	}

	/* Extract discontinuity curves. */
	if (GlblDisplayDiscontCurves) {
	    C1Disconts = BspKnotAllC1Discont(KV, Order, Length,
					     &NumC1Disconts);
	    if (NumC1Disconts > 0) {
	        TIsoInters = TrimIntersectTrimCrvIsoVals(TSrf, Dir, C1Disconts,
							 NumC1Disconts, TRUE);
		DIsoInters = TrimIntersectCrvsIsoVals(SilGeom, Dir,
						      C1Disconts,
						      NumC1Disconts);
		for (i = 0; i < NumC1Disconts; i++) {
		    Crvs = GenIsoCrvSegs(Srf, C1Disconts[i], Dir, POrigObj,
					 IHID_CURVE_DISCONT, TIsoInters[i],
					 DIsoInters[i]);
		    CrvGeom = CagdListAppend(Crvs, CrvGeom);
		}
		IritFree(DIsoInters);
		IritFree(TIsoInters);
	    }
	}
	else {
	    NumC1Disconts = 0;
	    C1Disconts = NULL;
	}

	IsoParams = DeriveIsoParamVals(Min, Max, NumC1Disconts, C1Disconts,
				       &NumIsos[l]);

	/* Find all U intersections of isocurves with silhouettes. */
	if (IsoParams != NULL) {
	    IsoInters = TrimIntersectCrvsIsoVals(SilGeom, Dir, IsoParams,
					         NumIsos[l]);
	    TIsoInters = TrimIntersectTrimCrvIsoVals(TSrf, Dir, IsoParams,
						     NumIsos[l], TRUE);
	}
	else {
	    IsoInters = NULL;
	    TIsoInters = NULL;
	}

	/* Do the interior isoparametric curves. */
	for (i = 1; i < NumIsos[l] - 1; i++) {
	    /* Lets see if we have a discontinuity here. */
	    for (j = 0; j < NumC1Disconts; j++)
	        if (IRIT_APX_EQ(C1Disconts[j], IsoParams[i]))
		    break;
	    if (j < NumC1Disconts)
	        continue;		 /* Indeed this is a discontinuity. */

	    Crvs = GenIsoCrvSegs(Srf, IsoParams[i], Dir, POrigObj,
				 IHID_CURVE_ISOPARAM, TIsoInters[i],
				 IsoInters[i]);
	    CrvGeom = CagdListAppend(Crvs, CrvGeom);
	}

	if (IsoParams != NULL)
	    IritFree(IsoParams);
	if (IsoInters != NULL)
	    IritFree(IsoInters);
	if (TIsoInters != NULL)
	    IritFree(TIsoInters);
	if (C1Disconts != NULL)
	    IritFree(C1Disconts);
    }

    /* Do the boundary/trimming curves and intersect against the sils. */
    
    /* Trim silhouette curves against the trimming curves. */
    TCrvs = NULL;
    while (SilGeom) {
	IRIT_LIST_POP(Crv1, SilGeom);

	Crvs = TrimCrvAgainstTrimCrvs(Crv1, TSrf, GlblIHidTolerance);
	TCrvs = CagdListAppend(Crvs, TCrvs);
    } 
    SilGeom = TCrvs;

    /* Extract and split boundary trimming curves against silhouette curves. */
    TrimSetTrimCrvLinearApprox(IHID_TRIM_EDGES_TOLERANCE,
			       SYMB_CRV_APPROX_UNIFORM);
    TrimCrvs = TrimGetTrimmingCurves(TSrf, TRUE, TRUE);
    TCrvs = TrimCrvs;
    TrimCrvs = NULL;
    while (TCrvs) {
        CagdPtStruct *Inters, *Inter;
	CagdCrvStruct *TCrv;

	IRIT_LIST_POP(TCrv, TCrvs);

	if (SilGeom != NULL &&
	    (Inters = CagdCrvCrvInter(TCrv, SilGeom,
				      GlblIHidTolerance)) != NULL) {
	    while (Inters) {
	        CagdRType TMin, TMax;
		CagdCrvStruct *TCrv1, *TCrv2;

		IRIT_LIST_POP(Inter, Inters);

		CagdCrvDomain(TCrv, &TMin, &TMax);
		if (Inter -> Pt[0] > TMin &&
		    Inter -> Pt[0] < TMax &&
		    !IRIT_APX_EQ_EPS(Inter -> Pt[0], TMin,
				     GlblIHidTolerance) &&
		    !IRIT_APX_EQ_EPS(Inter -> Pt[0], TMax,
				     GlblIHidTolerance)) {
		    TCrv1 = CagdCrvSubdivAtParam(TCrv, Inter -> Pt[0]);
		    TCrv2 = TCrv1 -> Pnext;
		    TCrv1 -> Pnext = NULL;

		    IRIT_LIST_PUSH(TCrv1, TrimCrvs);
		    CagdCrvFree(TCrv);
		    TCrv = TCrv2;			      
		}
		IritFree(Inter);
	    }
	    IRIT_LIST_PUSH(TCrv, TrimCrvs);
	}
	else
	    IRIT_LIST_PUSH(TCrv, TrimCrvs);
    }

    /* Evaluate boundary trimming curve into E3 and append to global list. */
    while (TrimCrvs != NULL) {
        CagdRType TMin, TMax;
        CagdCrvStruct *Crv;

	IRIT_LIST_POP(Crv, TrimCrvs);
	CagdCrvDomain(Crv, &TMin, &TMax);

	Crv1 = TrimEvalTrimCrvToEuclid(TSrf, Crv);
	BspKnotAffineTransOrder2(Crv1 -> KnotVector, Crv1 -> Order,
				 Crv1 -> Order + Crv1 -> Length,
				 TMin, TMax);
	CAGD_PROPAGATE_ATTR(Crv1, Crv);
	AttrSetPtrAttrib(&Crv1 -> Attr, "_Uv", Crv);
	IHID_MARK_CTYPE(Crv1, (IPObjectStruct *) POrigObj,
			IHID_CURVE_BOUNDARY);
	IRIT_LIST_PUSH(Crv1, CrvGeom);
    }

    /* Evaluate the silhouette curve into E3 and append to global list. */
    if (SilGeom != NULL) {
        SilGeom = EvalCurvesIntoEuclidean(SilGeom, Srf, POrigObj,
					  IHID_CURVE_SILHOUETTE);
	CrvGeom = CagdListAppend(SilGeom, CrvGeom);
    }

    if (NewSrf != NULL)
        CagdSrfFree(NewSrf);

    return CrvGeom;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluate the given piecewise linear curves into euclidean space.         *
*                                                                            *
* PARAMETERS:                                                                *
*   UVCrvs:    UV curves to evaluate.                                        *
*   Srf:       Surface to evaluate curves from.                              *
*   POrigObj:  Original object Srf was originated from.                      *
*   CType:     Type of iso curves we extract.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   Euclidean curves on Srf, from UVCrvs.                 *
*****************************************************************************/
static CagdCrvStruct *EvalCurvesIntoEuclidean(CagdCrvStruct *UVCrvs,
					      const CagdSrfStruct *Srf,
					      const IPObjectStruct *POrigObj,
					      IHidCrvType CType)
{
    CagdCrvStruct
	*E3Crvs = NULL;

    while (UVCrvs != NULL) {
        CagdCrvStruct
	    *CrvUV = UVCrvs,
	    *CrvE3 = CagdCoerceCrvTo(CrvUV, CAGD_PT_E3_TYPE, FALSE);
	int i,
	    Len = CrvE3 -> Length;
	CagdRType *R,
	    **Points = CrvE3 -> Points;
	CagdPType PtE3;

	UVCrvs = UVCrvs -> Pnext;
	CrvUV -> Pnext = NULL;
	for (i = 0; i < Len; i++) {
	    R = CagdSrfEval(Srf, Points[1][i], Points[2][i]);
	    CagdCoerceToE3(PtE3, &R, -1, Srf -> PType);
	    Points[1][i] = PtE3[0];
	    Points[2][i] = PtE3[1];
	    Points[3][i] = PtE3[2];
	}

	AttrSetPtrAttrib(&CrvE3 -> Attr, "_Uv", CrvUV);
	IHID_MARK_CTYPE(CrvE3, (IPObjectStruct *) POrigObj, CType);
	IRIT_LIST_PUSH(CrvE3, E3Crvs);
    }

    return E3Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Derive the locations to sample isoparametric curves based upon the       *
* discontinuities in the surface and the number of requested isocurves.      *
*                                                                            *
* PARAMETERS:                                                                *
*   Min, Max:      Boundary of parametric domain.                            *
*   NumC1Disconts: Number of discontinuities in the domain.                  *
*   C1Disconts:    Array with the parameter values of the discontinuities.   *
*   NumIso:        Number of isocurves requested, to be updated with the     *
*		   actual number prescribed.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   Derived parameter values.                                 *
*****************************************************************************/
static CagdRType *DeriveIsoParamVals(CagdRType Min,
				     CagdRType Max,
				     int NumC1Disconts,
				     CagdRType *C1Disconts,
				     int *NumIso)
{
    int n, j,
	i = 0;
    CagdRType d, dt, *IsoParams,
	Dt = Max - Min - 2 * IHID_BNDRY_EPS,
	t = Min + IHID_BNDRY_EPS;

    if (*NumIso == 0)
	return NULL;

    IsoParams = (CagdRType *) IritMalloc(sizeof(CagdRType) * *NumIso
					 * (NumC1Disconts + 2));

    IsoParams[i++] = t;

    for (j = 0; j < NumC1Disconts; j++) {
        dt = C1Disconts[j] - t;
        if ((n = (int) (0.5 + *NumIso * dt / Dt)) > 1) {
	    d = dt / n;
	    while (n--) {
		t += d;
	        IsoParams[i++] = t;
	    }
	}
	else if (j == 0) {
	    /* Force at least the boundary. */
	    IsoParams[i++] = t;
	}
	t = C1Disconts[j];
    }

    dt = Max - t - IHID_BNDRY_EPS;
    if ((n = (int) (0.5 + (*NumIso - 1) * dt / Dt)) > 1) {
        d = dt / n;
	while (n--) {
	    t += d;
	    IsoParams[i++] = t;
	}
    }
    else {
        /* Force at least the boundary. */
        IsoParams[i++] = Max - IHID_BNDRY_EPS;
    }

    *NumIso = i;

    return IsoParams;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Extract the sub curve segments of the isocurve of surface Srf in         *
* direction Dir at parameter t.  The sub curves are defined via the list of  *
* parameter values specified in IsoInter.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:       Surface to extract the iso curve segments from.               *
*   t:         Parameter value of the iso curve segments.                    *
*   Dir:       Direction (U or V) of the iso curve segments.                 *
*   POrigObj:  Original object Srf was originated from.                      *
*   CType:     Type of iso curves we extract.                                *
*   TrimIsoInters: List of points where the iso curves intersects the        *
*	       trimming curves, if any.  If NULL, no trimming curves.        *
*   IsoInters: List of points where the iso curve must be broken into segs.  *
*	       This list is freed on the fly here.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  List of iso curves segments.                           *
*****************************************************************************/
static CagdCrvStruct *GenIsoCrvSegs(const CagdSrfStruct *Srf,
				    CagdRType t,
				    CagdSrfDirType Dir,
				    const IPObjectStruct *POrigObj,
				    IHidCrvType CType,
				    TrimIsoInterStruct *TrimIsoInters,
				    TrimIsoInterStruct *IsoInters)
{
    CagdRType UMin, UMax, VMin, VMax, TMin, TMax, **Points;
    CagdCrvStruct *UVCrv, *Crvs, *NewCrvs,
	*Crv = BspSrfCrvFromSrf(Srf, t, Dir);

    if (TrimIsoInters != NULL) {
	/* Trim the given curve Crv to according to the trimming curves. */
        Crvs = TrimCrvTrimParamList(Crv, TrimIsoInters);

	if (Crvs == NULL)
	    return NULL;		     /* Completely trimmed away. */
	else
	    Crv = Crvs;
    }

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    UVCrv = BspCrvNew(2, 2, CAGD_PT_E2_TYPE);
    BspKnotUniformOpen(2, 2, UVCrv -> KnotVector);
    Points = UVCrv -> Points;
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    BspKnotAffineTransOrder2(UVCrv -> KnotVector, 2, 4, VMin, VMax);
	    Points[1][0] = t;
	    Points[1][1] = t;
	    Points[2][0] = VMin;
	    Points[2][1] = VMax;
	    break;
	case CAGD_CONST_V_DIR:
	    BspKnotAffineTransOrder2(UVCrv -> KnotVector, 2, 4, UMin, UMax);
	    Points[1][0] = UMin;
	    Points[1][1] = UMax;
	    Points[2][0] = t;
	    Points[2][1] = t;
	    break;
        default:
	    assert(0);
    }

    Crvs = Crv;
    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        IHID_MARK_CTYPE(Crv, (IPObjectStruct *) POrigObj, CType);
    }
    while (IsoInters != NULL) {
        CagdCrvStruct *Crv1, *Crv2;
	TrimIsoInterStruct
	    *InterNext = IsoInters -> Pnext;

	NewCrvs = NULL;
	while (Crvs != NULL) {
	    IRIT_LIST_POP(Crv, Crvs);
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    if (IsoInters -> Param > TMin &&
		IsoInters -> Param < TMax &&
		!IRIT_APX_EQ_EPS(IsoInters -> Param, TMin,
				 GlblIHidTolerance) &&
		!IRIT_APX_EQ_EPS(IsoInters -> Param, TMax,
				 GlblIHidTolerance)) {
	        /* Found curve with this param. as its domain - split it. */
	        Crv1 = CagdCrvSubdivAtParam(Crv, IsoInters -> Param);
		
		CagdCrvFree(Crv);
		Crv2 = Crv1 -> Pnext;
		Crv1 -> Pnext = NULL;

		IHID_MARK_CTYPE(Crv1, (IPObjectStruct *) POrigObj, CType);
		IHID_MARK_CTYPE(Crv2, (IPObjectStruct *) POrigObj, CType);

		IRIT_LIST_PUSH(Crv1, NewCrvs);
		IRIT_LIST_PUSH(Crv2, NewCrvs);
		NewCrvs = CagdListAppend(Crvs, NewCrvs);
		break;
	    }
	    else {
	        IRIT_LIST_PUSH(Crv, NewCrvs);
	    }
	}
	Crvs = NewCrvs;

	IritFree(IsoInters);
	IsoInters = InterNext;
    }

    /* Extract identical curves in parameter space for each segment. */
    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	CagdCrvStruct *UVCrv1;

        CagdCrvDomain(Crv, &TMin, &TMax);

	UVCrv1 = CagdCrvRegionFromCrv(UVCrv, TMin, TMax);
	AttrSetPtrAttrib(&Crv -> Attr, "_Uv", UVCrv1);
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Dumps the provided curve geometry to the currently open file.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Crvs:       Curve geometry to dump to file.                              *
*   OutStream:  Open stream to dump geometry to.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateAttributesAndDumpOut(CagdCrvStruct *Crvs, int OutStream)
{
    CagdCrvStruct *Crv;

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	int Hidden = AttrGetIntAttrib(Crv -> Attr, "hidden") == TRUE;
        IPObjectStruct
	    *OrigObj = (IPObjectStruct *) AttrGetPtrAttrib(Crv -> Attr,
							   "_OrigObj"),
	    *CrvObj = IPGenCRVObject(CagdCrvCopy(Crv));

	CrvObj -> U.Crvs -> Pnext = NULL;
	CrvObj -> Attr = Crv -> Attr;
	Crv -> Attr = NULL;

	if (OrigObj == NULL) {
	    IRIT_WARNING_MSG(
		    "Curve geometry with no original parent detected\n");
	}
	else {
	    IPAttributeStruct
		*Attr = CrvObj -> Attr;

	    /* Append attributes of original object to curve. */
	    if (Attr != NULL) {
		for ( ; Attr -> Pnext != NULL; Attr = Attr -> Pnext);
		Attr -> Pnext = IP_ATTR_COPY_ATTRS(OrigObj -> Attr);
	    }
	    else
	        CrvObj -> Attr = IP_ATTR_COPY_ATTRS(OrigObj -> Attr);
	}

#	ifdef IHID_DUMP_RANDOM_COLOR
	{
	    IRIT_STATIC_DATA int
		Index = 1;
	    int CType = AttrGetIntAttrib(CrvObj -> U.Crvs -> Attr, "ctype");
	    char
		*SType = "Crv";

	    switch (CType) {
	        case IHID_CURVE_BOUNDARY:
		    SType = "Boundary";
	            break;
	        case IHID_CURVE_SILHOUETTE:
		    SType = "Silhouette";
	            break;
	        case IHID_CURVE_DISCONT:
		    SType = "Discont";
	            break;
		case IHID_CURVE_ISOPARAM:
		    SType = "Isoparam";
		    break;

	    }
	    AttrSetObjectRGBColor(CrvObj,
				  64 + (int) IritRandom(0, 191),
				  64 + (int) IritRandom(0, 191),
				  64 + (int) IritRandom(0, 191));
	    sprintf(CrvObj -> Name, "%s%d", SType, Index++);
	}
#	endif /* IHID_DUMP_RANDOM_COLOR */

	if (Hidden) {
	    if (GlblOutputInvisible) {
	        AttrSetObjectWidth(CrvObj, IHID_DEF_HIDDEN_WIDTH);

		IPPutObjectToHandler(OutStream, CrvObj);
	    }
	}
	else
	    IPPutObjectToHandler(OutStream, CrvObj);

	CrvObj -> U.Crvs = NULL;
	IPFreeObject(CrvObj);
    }

    CagdCrvFreeList(Crvs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* IHidden Exit routine.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   ExitCode:    To notify O.S. with result of program.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IHiddenExit                                                              M
*****************************************************************************/
void IHiddenExit(int ExitCode)
{
    if (!GlblQuiet)
	IRIT_INFO_MSG("\n");

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
