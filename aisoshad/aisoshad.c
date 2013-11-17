/*****************************************************************************
* Adaptive Iso Shader program to render freeform models.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "attribut.h"
#include "allocate.h"
#include "grap_lib.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "program.h"

typedef enum {		           /* Type of distance function computation. */
    AISO_DIST_REGULAR = 0,
    AISO_DIST_PHONG,
    AISO_DIST_PHONG_2L,
    AISO_DIST_PHONG_SPEC,
    AISO_DIST_PHONG_2L_SPEC,
    AISO_DIST_ZNORMAL,
    AISO_DIST_POINT_E3
} AIsoDistCompType;

#define NORMAL_ANGLE_SCALE	1e5
#define TRUNCATED_DOMAIN	1e-3
#define MIN_DOMAIN_VALID	1e-2
#define MAX_LEVEL_VALID		100

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "AIsoShad		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "AIsoShad		" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "AIsoShad o%-OutName!s m%- i%- F%-PolyOpti|FineNess!d!F f%-PolyOpti|SampTol!d!d r%-RndrMdl!d c%-CosPwr!F s%-SdrPwr!F l%-Lx|Ly|Lz!F!F!F R%-Random!F d%-AdapDir!d t%-SrfZTrans!F M%-MinSubdiv!d D%-AdapDist!F w%-AdapIsoWidth!F S%-WidthScale!F W%- u%- Z%-ZbufSize!d b%- z%- DFiles!*s";

IRIT_STATIC_DATA char
    *GlblLightSourceStr = "",
    *GlblOutFileName = "aisoshad.itd";

IRIT_STATIC_DATA AIsoDistCompType
    GlblDistRndrModel = AISO_DIST_PHONG;

IRIT_STATIC_DATA int
    GlblBinaryOutput = FALSE,
    GlblViewSpace = TRUE,
    GlblZBufferSize = 500,
    GlblApplyVisibTest = FALSE,
    GlblSymbInterp = FALSE,
    GlblAdapIsoColor = IG_IRIT_WHITE,
    GlblAdapIsoDir[2] = { CAGD_CONST_U_DIR, CAGD_NO_DIR },
    GlblMinSubdivLevel = 1,
    GlblVariableWidth = FALSE;

IRIT_STATIC_DATA IrtRType
    GlblWidthScale = 1.0,
    GlblCosinePower = 1.0,
    GlblShaderPower = 2.0,
    GlblLightSource[3] = { 1.0, 0.0, 0.0 },
    GlblSrfTranslate[3] = { 0.0, 0.0, 0.0 },
    GlblRandomDist = 1.0,
    GlblAdapDistance = 1.0,
    GlblAdapIsoWidth = 0.001;

IRIT_STATIC_DATA IrtHmgnMatType InvCrntViewMat;

IRIT_GLOBAL_DATA SymbCrvApproxMethodType
    GlblCrvApproxMethod = SYMB_CRV_APPROX_UNIFORM;
IRIT_GLOBAL_DATA int
    GlblSamplesPerCurve = IG_DEFAULT_SAMPLES_PER_CURVE,
    GlblTalkative = FALSE,
    GlblPolyOptimalMethod = 0;
IRIT_GLOBAL_DATA IrtRType
    GlblPolyFineNess = IG_DEFAULT_POLYGON_FINENESS;

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
  { "AdapDir",        "-d", (VoidPtr) &GlblAdapIsoDir[0],   IC_INTEGER_TYPE },
  { "AdapIsoColor",   "",   (VoidPtr) &GlblAdapIsoColor,    IC_INTEGER_TYPE },
  { "RenderModel",    "-r", (VoidPtr) &GlblDistRndrModel,   IC_INTEGER_TYPE },
  { "MinSubdiv",      "-M", (VoidPtr) &GlblMinSubdivLevel,  IC_INTEGER_TYPE },
  { "ZBufSize",       "-Z", (VoidPtr) &GlblZBufferSize,     IC_INTEGER_TYPE },
  { "SampPerCurve",   "-f", (VoidPtr) &GlblSamplesPerCurve, IC_INTEGER_TYPE },
  { "PolyOpti",       "-F", (VoidPtr) &GlblPolyOptimalMethod,IC_INTEGER_TYPE },
  { "VisibTest",      "-Z", (VoidPtr) &GlblApplyVisibTest,  IC_BOOLEAN_TYPE },
  { "SymbInterp",     "-i", (VoidPtr) &GlblSymbInterp,	    IC_BOOLEAN_TYPE },
  { "MoreVerbose",    "-m", (VoidPtr) &GlblTalkative,	    IC_BOOLEAN_TYPE },
  { "VariableWidth",  "-W", (VoidPtr) &GlblVariableWidth,   IC_BOOLEAN_TYPE },
  { "ViewSpace",      "-u", (VoidPtr) &GlblViewSpace,       IC_BOOLEAN_TYPE },
  { "BinaryOutput",   "-b", (VoidPtr) &GlblBinaryOutput,    IC_BOOLEAN_TYPE },
  { "LightSrcDir",    "-l", (VoidPtr) &GlblLightSourceStr,  IC_STRING_TYPE },
  { "FineNess",       "-F", (VoidPtr) &GlblPolyFineNess,    IC_REAL_TYPE },
  { "AdapIsoWidth",   "-w", (VoidPtr) &GlblAdapIsoWidth,    IC_REAL_TYPE },
  { "WidthScale",     "-S", (VoidPtr) &GlblWidthScale,      IC_REAL_TYPE },
  { "CosinePower",    "-c", (VoidPtr) &GlblCosinePower,     IC_REAL_TYPE },
  { "ShaderPower",    "-s", (VoidPtr) &GlblShaderPower,     IC_REAL_TYPE },
  { "RandomDist",     "-R", (VoidPtr) &GlblRandomDist,      IC_REAL_TYPE },
  { "SrfZTrans",      "-t", (VoidPtr) &GlblSrfTranslate[2], IC_REAL_TYPE },
  { "AdapDistance",   "-D", (VoidPtr) &GlblAdapDistance,    IC_REAL_TYPE },
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

static CagdCrvStruct *AdapIsoDistSqr(int Level,
				     CagdCrvStruct *Crv1,
				     CagdCrvStruct *NCrv1,
				     CagdCrvStruct *Crv2,
				     CagdCrvStruct *NCrv2);
static IrtRType RandomPointPerturb(IrtRType RandomFactor);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of AIsoShad - Read command line and do what is needed...	     M
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
	DistCompFlag = FALSE,
	CosinePowerFlag = FALSE,
	ShaderPowerFlag = FALSE,
	AdapDirFlag = FALSE,
	SrfFineNessFlag = FALSE,
	CrvOptiPolylineFlag = FALSE,
	SrfZTransFlag = FALSE,
	AdapDistanceFlag = FALSE,
	AdapIsoWidthFlag = FALSE,
	WidthScaleFlag = FALSE,
        LightSrcFlag = FALSE,
        MinSubdivFlag = FALSE,
	RandomFlag = FALSE,
	OutFileFlag = FALSE,
	VerFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;
    FILE *f;
    IPObjectStruct *PObjects, *PObj;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IritConfig("aisoshad", SetUp, NUM_SET_UP);/* Read config. file if exists.*/
    if (!IRT_STR_NULL_ZERO_LEN(GlblLightSourceStr)) {
#ifdef IRIT_DOUBLE
	if (sscanf(GlblLightSourceStr, "%lf,%lf,%lf",
#else
	if (sscanf(GlblLightSourceStr, "%f,%f,%f",
#endif /* IRIT_DOUBLE */
		   &GlblLightSource[0],
		   &GlblLightSource[1],
		   &GlblLightSource[2]) != 3) {
	    IRIT_WARNING_MSG(
		    "Fail to parse LightSrcDir in configuration file.\n");
	    ShaderExit(-1);
	}
    }

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &OutFileFlag, &GlblOutFileName, &GlblTalkative,
			   &GlblSymbInterp, &SrfFineNessFlag,
			   &GlblPolyOptimalMethod,
			   &GlblPolyFineNess, &CrvOptiPolylineFlag,
			   &GlblCrvApproxMethod, &GlblSamplesPerCurve,
			   &DistCompFlag, &GlblDistRndrModel,
			   &CosinePowerFlag, &GlblCosinePower,
			   &ShaderPowerFlag, &GlblShaderPower,
			   &LightSrcFlag, &GlblLightSource[0],
			   &GlblLightSource[1], &GlblLightSource[2],
			   &RandomFlag, &GlblRandomDist,
			   &AdapDirFlag, &GlblAdapIsoDir[0],
			   &SrfZTransFlag, &GlblSrfTranslate[2],
			   &MinSubdivFlag, &GlblMinSubdivLevel,
			   &AdapDistanceFlag, &GlblAdapDistance,
			   &AdapIsoWidthFlag, &GlblAdapIsoWidth,
			   &WidthScaleFlag, &GlblWidthScale,
			   &GlblVariableWidth, &GlblViewSpace,
			   &GlblApplyVisibTest, &GlblZBufferSize,
			   &GlblBinaryOutput,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
	GAPrintErrMsg(Error);
	GAPrintHowTo(CtrlStr);
	ShaderExit(1);
    }

    if (VerFlag) {
	IRIT_INFO_MSG_PRINTF("\n%s\n\n", VersionStr);
	GAPrintHowTo(CtrlStr);
	IritConfigPrint(SetUp, NUM_SET_UP);
	ShaderExit(0);
    }

    if (!NumFiles) {
	IRIT_WARNING_MSG("No data file names where given, exit.\n");
	GAPrintHowTo(CtrlStr);
	ShaderExit(1);
    }

    switch (GlblAdapIsoDir[0]) {
        case 0:
	    GlblAdapIsoDir[0] = CAGD_CONST_U_DIR;
	    break;
	case 1:
	    GlblAdapIsoDir[0] = CAGD_CONST_V_DIR;
	    break;
	case 2:
	default:
	    GlblAdapIsoDir[0] = CAGD_CONST_U_DIR;
	    GlblAdapIsoDir[1] = CAGD_CONST_V_DIR;
	    break;
    }

    BspMultComputationMethod(GlblSymbInterp);
    SymbSetAdapIsoExtractMinLevel(GlblMinSubdivLevel);

    if (GlblDistRndrModel != AISO_DIST_POINT_E3)
	IRIT_PT_NORMALIZE(GlblLightSource);

    if (IRIT_FABS(GlblRandomDist) < 1) {
	IRIT_WARNING_MSG("Randomization [-R] should not be less than one\n");
	ShaderExit(0);
    }

    /* Get the data files: */
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	ShaderExit(0);

    if (IPWasPrspMat)
	MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
    else
	IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));

    PObj = GMTransformObjectList(PObjects, CrntViewMat);
    IPFreeObjectList(PObjects);
    PObjects = PObj;

    if (GlblViewSpace) {
	MatGenUnitMat(InvCrntViewMat);
    }
    else {
	if (!MatInverseMatrix(CrntViewMat, InvCrntViewMat)) {
	    IRIT_WARNING_MSG( "Failed to compute the inverse transformation\n");
	    exit(1);
	}
    }

    /* Open output file, if necessary. */
    if (OutFileFlag) {
	if ((f = fopen(GlblOutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF( "Failed to open \"%s\".\n", GlblOutFileName);
	    ShaderExit(2);
	}
    }
    else
	f = stdout;

    /* If we are going to remove the hidden portion - prepare the Z buffer! */
    if (GlblApplyVisibTest) {
        if (GlblTalkative)
	    IRIT_INFO_MSG("Scan converting surfaces/polygons into ZBuffer\n");
        ScanConvertPolySrfs(PObjects, GlblZBufferSize, -IRIT_INFNTY, 0);
    }

    /* Traverse all the objects and generate the coverage for them. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	if (GlblTalkative)
	    IRIT_INFO_MSG_PRINTF("Processing object \"%s\"\n",
				 IP_GET_OBJ_NAME(PObj));

	if (IP_IS_SRF_OBJ(PObj)) {
	    CagdSrfStruct *Srf;
	    int SrfAdapIsoDir[2], Color, AdapIsoColor, AdapIsoRGB[3], HasRGB,
		SrfAdapIsoMinSubdiv = AttrGetObjectIntAttrib(PObj,
							  "AdapIsoMinSubdiv");
	    IrtRType
		RelativeAdapIsoDist = AttrGetObjectRealAttrib(PObj,
							      "AdapIsoDist");

	    if (IP_ATTR_IS_BAD_REAL(RelativeAdapIsoDist))
		RelativeAdapIsoDist = 1.0;

	    SrfAdapIsoDir[0] = AttrGetObjectIntAttrib(PObj, "AdapIsoDir");
	    SrfAdapIsoDir[1] = CAGD_NO_DIR;
	    switch (SrfAdapIsoDir[0]) {
		case 0:
		    SrfAdapIsoDir[0] = CAGD_CONST_U_DIR;
		    break;
		case 1:
		    SrfAdapIsoDir[0] = CAGD_CONST_V_DIR;
		    break;
		case 2:
		    SrfAdapIsoDir[0] = CAGD_CONST_U_DIR;
		    SrfAdapIsoDir[1] = CAGD_CONST_V_DIR;
		    break;
		default:
		    SrfAdapIsoDir[0] = GlblAdapIsoDir[0];
		    SrfAdapIsoDir[1] = GlblAdapIsoDir[1];
		    break;
	    }

	    if (!IP_ATTR_IS_BAD_INT(SrfAdapIsoMinSubdiv))
	        SymbSetAdapIsoExtractMinLevel(SrfAdapIsoMinSubdiv);

	    if ((HasRGB = AttrGetObjectRGBColor(PObj,
						&AdapIsoRGB[0],
						&AdapIsoRGB[1],
						&AdapIsoRGB[2])) != FALSE) {
	        AdapIsoColor = GlblAdapIsoColor;
	    }
	    else if ((Color = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR)
	        AdapIsoColor = Color;
	    else
	        AdapIsoColor = GlblAdapIsoColor;

	    /* Set the color of the surfaces to black. */
	    AttrSetObjectColor(PObj, IG_IRIT_BLACK);
	    AttrSetObjectRGBColor(PObj, 0, 0, 0);

	    for (Srf = PObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		IrtRType UMin, UMax, VMin, VMax;
		IPObjectStruct *PAdapIsoObj;
		CagdSrfStruct *TSrf, *TSrf1,
		    *NSrf = SymbSrfNormalSrf(Srf);
		CagdCrvStruct *Crv, *AdapIso;

		/* Remove a little from all four boundaries to prevent from */
		/* dealing with degenerated cases so common on boundary.    */
		CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
		TSrf1 = CagdSrfRegionFromSrf(Srf,
					     UMin + TRUNCATED_DOMAIN,
					     UMax - TRUNCATED_DOMAIN,
					     CAGD_CONST_U_DIR);
		TSrf = CagdSrfRegionFromSrf(TSrf1,
					    VMin + TRUNCATED_DOMAIN,
					    VMax - TRUNCATED_DOMAIN,
					    CAGD_CONST_V_DIR);
					     
		CagdSrfFree(TSrf1);

		CagdSrfTransform(TSrf, GlblSrfTranslate, 1.0);
		AdapIso = SymbAdapIsoExtract(TSrf, NSrf, AdapIsoDistSqr,
					     (CagdSrfDirType) SrfAdapIsoDir[0],
					     GlblAdapDistance *
					         RelativeAdapIsoDist,
					     FALSE, FALSE);
		if (SrfAdapIsoDir[1] != CAGD_NO_DIR) {
		    CagdCrvStruct
		        *AdapIso2 = SymbAdapIsoExtract(TSrf, NSrf,
						       AdapIsoDistSqr,
						       (CagdSrfDirType)
							   SrfAdapIsoDir[1],
						       GlblAdapDistance *
						          RelativeAdapIsoDist,
						       FALSE, FALSE);

		    AdapIso = CagdListAppend(AdapIso, AdapIso2);
		}

		CagdSrfFree(NSrf);
		CagdSrfFree(TSrf);

		/* Remove the normal curves from the list. */
		for (Crv = AdapIso; Crv != NULL; Crv = Crv -> Pnext) {
		    if (Crv -> Pnext != NULL) {
		        CagdCrvStruct
			    *NextCrv = Crv -> Pnext -> Pnext;

			Crv -> Pnext -> Pnext = NULL;
			CagdCrvFree(Crv -> Pnext);
			Crv -> Pnext = NextCrv;
		    }
		}

		if (GlblApplyVisibTest) {
		    IPObjectStruct *PObjTmp;

		    PObjTmp = TestCurveVisibility(AdapIso, GlblWidthScale);
		    PAdapIsoObj = GMTransformObjectList(PObjTmp,
							InvCrntViewMat);
		    IPFreeObjectList(PObjTmp);

		    for (PObjTmp = PAdapIsoObj;
			 PObjTmp != NULL;
			 PObjTmp = PObjTmp -> Pnext) {
		        if (HasRGB)
			    AttrSetObjectRGBColor(PObjTmp, AdapIsoRGB[0],
					                   AdapIsoRGB[1],
						           AdapIsoRGB[2]);
			else
			    AttrSetObjectColor(PObjTmp, AdapIsoColor);
			IPPutObjectToFile(f, PObjTmp, GlblBinaryOutput);

		        if (HasRGB)
			    AttrSetObjectRGBColor(PAdapIsoObj, AdapIsoRGB[0],
					                       AdapIsoRGB[1],
						               AdapIsoRGB[2]);
			else
			    AttrSetObjectColor(PAdapIsoObj, AdapIsoColor);

			if (GlblVariableWidth) {
			    /* Save every curve with its var. width function.*/
			}
			else {
			    AttrSetObjectRealAttrib(PAdapIsoObj, "width",
						    GlblAdapIsoWidth);
			}
			IPPutObjectToFile(f, PAdapIsoObj,
					  GlblBinaryOutput);
		    }
		    IPFreeObjectList(PAdapIsoObj);
		}
		else {
		    CagdCrvStruct *TCrv;

		    if (GlblVariableWidth) {
		        /* Save every curve with its variable width function.*/
		        for (Crv = AdapIso; Crv != NULL; ) {
			    CagdCrvStruct
			        *NextCrv = Crv -> Pnext;

			    Crv -> Pnext = NULL;
			    TCrv = CagdCrvMatTransform(Crv, InvCrntViewMat);
			    CAGD_PROPAGATE_ATTR(TCrv, Crv);
			    CagdCrvFree(Crv);
			    Crv = TCrv;

			    PAdapIsoObj = IPGenCRVObject(Crv);
			    PAdapIsoObj -> Attr = Crv -> Attr;
			    Crv -> Attr = NULL;

			    if (HasRGB)
			        AttrSetObjectRGBColor(PAdapIsoObj,
						      AdapIsoRGB[0],
						      AdapIsoRGB[1],
						      AdapIsoRGB[2]);
			    else
			        AttrSetObjectColor(PAdapIsoObj, AdapIsoColor);

			    IPPutObjectToFile(f, PAdapIsoObj,
					      GlblBinaryOutput);
			    IPFreeObject(PAdapIsoObj);

			    Crv = NextCrv;
			}
		    }
		    else {
		        CagdCrvStruct
			    *TransAdapIso = NULL;

		        for (Crv = AdapIso; Crv != NULL; Crv = Crv -> Pnext) {
			    TCrv = CagdCrvMatTransform(Crv, InvCrntViewMat);
			    CAGD_PROPAGATE_ATTR(TCrv, Crv);
			    IRIT_LIST_PUSH(TCrv, TransAdapIso);
			}
			CagdCrvFreeList(AdapIso);
			AdapIso = CagdListReverse(TransAdapIso);

			PAdapIsoObj = IPGenCRVObject(AdapIso);
		        if (HasRGB)
			    AttrSetObjectRGBColor(PAdapIsoObj, AdapIsoRGB[0],
					                       AdapIsoRGB[1],
						               AdapIsoRGB[2]);
			else
			    AttrSetObjectColor(PAdapIsoObj, AdapIsoColor);
			AttrSetObjectRealAttrib(PAdapIsoObj, "width",
						GlblAdapIsoWidth);

			IPPutObjectToFile(f, PAdapIsoObj, GlblBinaryOutput);
			IPFreeObject(PAdapIsoObj);
		    }
		}
	    }

	    SymbSetAdapIsoExtractMinLevel(GlblMinSubdivLevel);
	}
	else if (IP_IS_CRV_OBJ(PObj)) {
	    if (GlblApplyVisibTest) {
		IPObjectStruct *PTmp,
		    *PObjs = TestCurveVisibility(PObj -> U.Crvs,
						 GlblWidthScale);

		for (PTmp = PObjs; PTmp != NULL; PTmp = PTmp -> Pnext) {
		    PTmp -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

		    IPPutObjectToFile(f, PTmp, GlblBinaryOutput);
		}

		IPFreeObjectList(PObjs);
	    }
	    else
	        IPPutObjectToFile(f, PObj, GlblBinaryOutput);
	}
    }

    ShaderExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a random number between 1/RandomFactor to RandomFactor.         *
*                                                                            *
* PARAMETERS:                                                                *
*   RandomFactor:   Domain of random numbers.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:       A random number.                                         *
*****************************************************************************/
static IrtRType RandomPointPerturb(IrtRType RandomFactor)
{
    CagdRType
	R = IritRandom(-RandomFactor, RandomFactor),
	RandomFactor1 = RandomFactor + 1.0;

    return 1.0 + (R > 0.0 ? R : RandomFactor1 / (RandomFactor1 + R));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a distance square function between Crv1 and Crv2 as a scalar    *
* field. The normal vector fields (not unit size) of the two curves are also *
* provided as NCrv1, NCrv2.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Level:     Maximum depth of recursion of adaptive iso algorithm.         *
*   Crv1:      First curve to compute distance field from it to Crv2.        *
*   NCrv1:     Normal vector field of Crv1.                                  *
*   Crv2:      Second curve to compute distance field from it to Crv1.       *
*   NCrv2:     Normal vector field of Crv2.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  A scalar distance function field.                      *
*****************************************************************************/
static CagdCrvStruct *AdapIsoDistSqr(int Level,
				     CagdCrvStruct *Crv1,
				     CagdCrvStruct *NCrv1,
				     CagdCrvStruct *Crv2,
				     CagdCrvStruct *NCrv2)
{
    int i;
    CagdRType TMin, TMax, *Points1, *Points2;
    CagdCrvStruct
	*DistCrv2Aux1 = NULL,
	*DistCrv2Aux2 = NULL,
        *DiffCrv = SymbCrvSub(Crv1, Crv2),
        *DiffCrv2D = CagdCoerceCrvTo(DiffCrv, CAGD_PT_E2_TYPE, FALSE),
        *DistCrv2 = SymbCrvDotProd(DiffCrv2D, DiffCrv);

    CagdCrvFree(DiffCrv);
    CagdCrvFree(DiffCrv2D);

    CagdCrvDomain(Crv1, &TMin, &TMax);

    if (GlblVariableWidth) {
	if (DistCrv2 -> PType != CAGD_PT_E1_TYPE)
	    DistCrv2Aux1 = CagdCoerceCrvTo(DistCrv2, CAGD_PT_E1_TYPE, FALSE);
	else
	    DistCrv2Aux1 = CagdCrvCopy(DistCrv2);
	DistCrv2Aux2 = CagdCrvCopy(DistCrv2Aux1);

	Points1 = DistCrv2Aux1 -> Points[1];
	Points2 = DistCrv2Aux2 -> Points[1];
    }
    else
	Points1 = Points2 = NULL;

    if (TMax - TMin > MIN_DOMAIN_VALID && Level < MAX_LEVEL_VALID) {
	CagdRType TMin1, TMax1, TMin2, TMax2,
	    *Points = DistCrv2 -> Points[1],
	    *Nodes = CagdCrvNodes(DistCrv2);

	CagdCrvDomain(NCrv1, &TMin1, &TMax1);
	CagdCrvDomain(NCrv2, &TMin2, &TMax2);

	switch (GlblDistRndrModel) {
	    case AISO_DIST_PHONG:
	    case AISO_DIST_PHONG_2L:
	    case AISO_DIST_PHONG_SPEC:
	    case AISO_DIST_PHONG_2L_SPEC:
	        for (i = 0; i < DistCrv2 -> Length; i++) {
		    CagdVType N1, N2, RefDir;
		    CagdRType *R, Z, Z1, Z2,
		        t1 = IRIT_BOUND(Nodes[i], TMin1, TMax1),
		        t2 = IRIT_BOUND(Nodes[i], TMin2, TMax2);

		    R = CagdCrvEval(NCrv1, t1);
		    CagdCoerceToE3(N1, &R, -1, NCrv1 -> PType);
		    R = CagdCrvEval(NCrv2, t2);
		    CagdCoerceToE3(N2, &R, -1, NCrv2 -> PType);

		    IRIT_PT_NORMALIZE(N1);
		    IRIT_PT_NORMALIZE(N2);

		    if (GlblVariableWidth) {
			if (GlblDistRndrModel == AISO_DIST_PHONG_2L ||
			    GlblDistRndrModel == AISO_DIST_PHONG_2L_SPEC) {
			    Z1 = IRIT_FABS(IRIT_DOT_PROD(N1, GlblLightSource));
			    Z2 = IRIT_FABS(IRIT_DOT_PROD(N2, GlblLightSource));
			    Z = (Z1 + Z2) * 0.5;

			    if (GlblDistRndrModel == AISO_DIST_PHONG_2L_SPEC) {
				/* Add specular term, if have one. */
				if (IRIT_DOT_PROD(N1, GlblLightSource) > 0.0) {
				    IRIT_PT_COPY(RefDir, N1);
				    IRIT_PT_SCALE(RefDir, 2 * Z);
				    IRIT_PT_SUB(RefDir, RefDir, GlblLightSource);
				    Z1 += pow(IRIT_FABS(RefDir[2]),
					      GlblCosinePower);
				}
				else {
				    IRIT_PT_COPY(RefDir, N1);
				    IRIT_PT_SCALE(RefDir, -2 * Z);
				    IRIT_PT_SUB(RefDir, RefDir, GlblLightSource);
				    Z1 += pow(IRIT_FABS(RefDir[2]),
					      GlblCosinePower);
				}

				if (IRIT_DOT_PROD(N2, GlblLightSource) > 0.0) {
				    IRIT_PT_COPY(RefDir, N2);
				    IRIT_PT_SCALE(RefDir, 2 * Z);
				    IRIT_PT_SUB(RefDir, RefDir, GlblLightSource);
				    Z2 += pow(IRIT_FABS(RefDir[2]),
					      GlblCosinePower);
				}
				else {
				    IRIT_PT_COPY(RefDir, N2);
				    IRIT_PT_SCALE(RefDir, -2 * Z);
				    IRIT_PT_SUB(RefDir, RefDir, GlblLightSource);
				    Z2 += pow(IRIT_FABS(RefDir[2]),
					      GlblCosinePower);
				}
			    }
			}
			else { /* AISO_DIST_PHONG || AISO_DIST_PHONG_SPEC */
			    Z1 = IRIT_DOT_PROD(N1, GlblLightSource);
			    Z1 = (Z1 + 1.0) * 0.5;
			    Z2 = IRIT_DOT_PROD(N2, GlblLightSource);
			    Z2 = (Z2 + 1.0) * 0.5;
			    Z = (Z1 + Z2) * 0.5;

			    if (GlblDistRndrModel == AISO_DIST_PHONG_SPEC) {
				/* Add specular term, if have one. */
				if (IRIT_DOT_PROD(N1, GlblLightSource) > 0.0) {
				    IRIT_PT_COPY(RefDir, N1);
				    IRIT_PT_SCALE(RefDir, 2 * Z);
				    IRIT_PT_SUB(RefDir, RefDir, GlblLightSource);
				    Z1 += pow(IRIT_FABS(RefDir[2]),
					      GlblCosinePower);
				}

				if (IRIT_DOT_PROD(N2, GlblLightSource) > 0.0) {
				    IRIT_PT_COPY(RefDir, N2);
				    IRIT_PT_SCALE(RefDir, 2 * Z);
				    IRIT_PT_SUB(RefDir, RefDir, GlblLightSource);
				    Z2 += pow(IRIT_FABS(RefDir[2]),
					      GlblCosinePower);
				}
			    }
			}

			Z1 = pow(Z1, GlblShaderPower);
			Z2 = pow(Z2, GlblShaderPower);
	
			Points1[i] = Z1 + 1.0 / NORMAL_ANGLE_SCALE;
			Points2[i] = Z2 + 1.0 / NORMAL_ANGLE_SCALE;
		    }
		    else {
			if (GlblDistRndrModel == AISO_DIST_PHONG_2L ||
			    GlblDistRndrModel == AISO_DIST_PHONG_2L_SPEC) {
			    if (IRIT_DOT_PROD(N1, N2) > 0.0) {
				Z = (IRIT_DOT_PROD(N1, GlblLightSource) +
				     IRIT_DOT_PROD(N2, GlblLightSource)) * 0.5;
				Z = IRIT_FABS(Z);

				if (GlblDistRndrModel == AISO_DIST_PHONG_2L_SPEC) {
				    IRIT_PT_BLEND(RefDir, N1, N2, 0.5);
				    IRIT_PT_NORMALIZE(RefDir);
			
				    /* Add specular term, if have one. */
				    if (IRIT_DOT_PROD(RefDir,
						      GlblLightSource) > 0.0) {
					IRIT_PT_SCALE(RefDir, 2 * Z);
					IRIT_PT_SUB(RefDir, RefDir,
						    GlblLightSource);
					Z += pow(IRIT_FABS(RefDir[2]),
						 GlblCosinePower);
				    }
				    else {
					IRIT_PT_SCALE(RefDir, -2 * Z);
					IRIT_PT_SUB(RefDir, RefDir,
						    GlblLightSource);
					Z += pow(IRIT_FABS(RefDir[2]),
						 GlblCosinePower);
				    }
				}
			    }
			    else
			      Z = 1.0;
			}
			else { /* AISO_DIST_PHONG || AISO_DIST_PHONG_SPEC */
			    if (IRIT_DOT_PROD(N1, N2) > 0.0) {
				Z = (IRIT_DOT_PROD(N1, GlblLightSource) +
				     IRIT_DOT_PROD(N2, GlblLightSource)) * 0.5;
				Z = (Z + 1.0) * 0.5;

				if (GlblDistRndrModel == AISO_DIST_PHONG_SPEC) {
				    IRIT_PT_BLEND(RefDir, N1, N2, 0.5);
				    IRIT_PT_NORMALIZE(RefDir);
			
				    /* Add specular term, if have one. */
				    if (IRIT_DOT_PROD(RefDir,
						      GlblLightSource) > 0.0) {
					IRIT_PT_SCALE(RefDir, 2 * Z);
					IRIT_PT_SUB(RefDir, RefDir,
						    GlblLightSource);
					Z += pow(IRIT_FABS(RefDir[2]),
						 GlblCosinePower);
				    }
				}
			    }
			    else
			      Z = 1.0;
			}

			Z = pow(Z, GlblShaderPower);

			Points[i] *= Z + 1.0 / NORMAL_ANGLE_SCALE;
		    }
		}
	        break;
	    case AISO_DIST_ZNORMAL:
		for (i = 0; i < DistCrv2 -> Length; i++) {
		    CagdVType N1, N2;
		    CagdRType *R, Z, Z1, Z2,
		        t1 = IRIT_BOUND(Nodes[i], TMin1, TMax1),
		        t2 = IRIT_BOUND(Nodes[i], TMin2, TMax2);

		    R = CagdCrvEval(NCrv1, t1);
		    CagdCoerceToE3(N1, &R, -1, NCrv1 -> PType);
		    R = CagdCrvEval(NCrv2, t2);
		    CagdCoerceToE3(N2, &R, -1, NCrv2 -> PType);

		    IRIT_PT_NORMALIZE(N1);
		    IRIT_PT_NORMALIZE(N2);

		    if (GlblVariableWidth) {
			Z1 = 1.0 + GlblCosinePower - IRIT_FABS(N1[2]),
			Z2 = 1.0 + GlblCosinePower - IRIT_FABS(N2[2]);

			Z1 = pow(Z1, GlblShaderPower);
			Z2 = pow(Z2, GlblShaderPower);

			Points1[i] = Z1;
			Points2[i] = Z2;
		    }
		    else {
			Z = 1.0 + GlblCosinePower -
			          (IRIT_FABS(N1[2]) + IRIT_FABS(N2[2])) * 0.5;

			Z = pow(Z, GlblShaderPower);

			Points[i] *= Z;
		    }
		}
	        break;
	    case AISO_DIST_POINT_E3:
		for (i = 0; i < DistCrv2 -> Length; i++) {
		    CagdVType P1, P2;
		    CagdRType *R, Z, Z1, Z2,
		        t1 = IRIT_BOUND(Nodes[i], TMin1, TMax1),
		        t2 = IRIT_BOUND(Nodes[i], TMin2, TMax2);

		    R = CagdCrvEval(Crv1, t1);
		    CagdCoerceToE3(P1, &R, -1, NCrv1 -> PType);
		    R = CagdCrvEval(Crv2, t2);
		    CagdCoerceToE3(P2, &R, -1, NCrv2 -> PType);

		    if (GlblVariableWidth) {
			Z1 = IRIT_PT_PT_DIST(P1, GlblLightSource);
			Z2 = IRIT_PT_PT_DIST(P2, GlblLightSource);

			Z1 = pow(Z1, GlblShaderPower);
			Z2 = pow(Z2, GlblShaderPower);

			Points1[i] = 1 / (Z1 + GlblCosinePower);
			Points2[i] = 1 / (Z2 + GlblCosinePower);
		    }
		    else {
			IRIT_PT_BLEND(P1, P1, P2, 0.5);
			Z = IRIT_PT_PT_DIST(P1, GlblLightSource);

			Z = pow(Z, GlblShaderPower);

			Points[i] *= 1 / (Z + GlblCosinePower);
		    }
		}
	        break;
	    default:
		break;
	}

	IritFree(Nodes);

	if (GlblRandomDist > 0.0) {
	    Points = DistCrv2 -> Points[1];

	    for (i = 0; i < DistCrv2 -> Length; i++)
		Points[i] *= RandomPointPerturb(GlblRandomDist);

	    if (GlblVariableWidth) {
	        Points1 = DistCrv2Aux1 -> Points[1],
		Points2 = DistCrv2Aux2 -> Points[1];

		for (i = 0; i < DistCrv2 -> Length; i++) {
		    Points1[i] *= RandomPointPerturb(GlblRandomDist);
		    Points2[i] *= RandomPointPerturb(GlblRandomDist);
		}
	    }
	}
    }

    if (GlblVariableWidth) {
	AttrSetObjAttrib(&Crv1 -> Attr, "VarWidth",
			 IPGenCRVObject(DistCrv2Aux1), FALSE);
	AttrSetObjAttrib(&Crv2 -> Attr, "VarWidth",
			 IPGenCRVObject(DistCrv2Aux2), FALSE);
    }

    return DistCrv2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* AIsoShad Exit routine.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   ExitCode:    To notify O.S. with result of program.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ShaderExit                                                               M
*****************************************************************************/
void ShaderExit(int ExitCode)
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
