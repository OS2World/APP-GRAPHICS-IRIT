/*****************************************************************************
* Points (distribution) Shader program to render freeform models.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 1.0, September 1996 *
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

#ifdef IRIT_HAVE_URT_RLE
#define NO_DECLARE_MALLOC /* For rle.h */
#include <rle.h>
#include <rle_raw.h>
#endif /* IRIT_HAVE_URT_RLE */

typedef struct PixelStruct {
    unsigned char Gray;
} PixelStruct;

typedef struct ImageStruct {
    int xSize, ySize;                  
    PixelStruct *data;
    struct ImageStruct *DxImage;                     
    struct ImageStruct *DyImage;                     
} ImageStruct;

typedef enum {		           /* Type of distance function computation. */
    NO_SHADER = 0,
    PTS_DIST_REGULAR,
    PTS_DIST_PHONG,
    PTS_DIST_PHONG_2L,
    PTS_DIST_PHONG_SPEC,
    PTS_DIST_PHONG_2L_SPEC,
    PTS_DIST_ZNORMAL,
    PTS_DIST_POINT_E3
} PtsDistCompType;

#define SRF_AREA_FINENESS	20
#define PTS_DENSITY		1000
#define SRF_MARCH_STEP		0.05
#define WOOD_LENGTH_DECAY_POWER 4
#define WOOD_MIN_LENGTH		0.01
#define MAX_X_DIFF		0.02

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "LineShad		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "LineShad		" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "LineShad o%-OutName!s m%- F%-PolyOpti|FineNess!d!F R%-RelStepSize!F f%-PolyOpti|SampTol!d!d r%-RndrMdl!d c%-CosPwr!F s%-SdrPwr!F i%-Intensity!F l%-Lx|Ly|Lz!F!F!F v%-Vx|Vy|Vz!F!F!F w%-Width!F d%-Density!F t%-SrfZTrans!F S%-WidthScale!F T%-Texture!s Z%-ZbufSize!d b%- z%- DFiles!*s";

IRIT_STATIC_DATA char
    *GlblSrfTexture = "silhouette",
    *GlblLightSourceStr = "",
    *GlblViewDirStr = "",
    *GlblOutFileName = "LineShad.itd";
IRIT_STATIC_DATA PtsDistCompType
    GlblDistRndrModel = NO_SHADER;
IRIT_STATIC_DATA int
    GlblZBufferSize = 500,
    GlblApplyVisibTest = FALSE,
    GlblBinaryOutput = FALSE;
IRIT_STATIC_DATA IrtRType
    GlblRelStepSize = 1.0,
    GlblWidthScale = 1.0,
    GlblMaxWidth = 0.01,
    GlblCosinePower = 1.0,
    GlblShaderPower = 2.0,
    GlblIntensity = 1.0,
    GlblLightSource[3] = { 0.5, 0.1, 2.0 },
    GlblViewDir[3] = { 0.0, 0.0, 1.0 },
    GlblSrfTranslate[3] = { 0.0, 0.0, 0.01 },
    GlblPtsDensity = 1.0;
IRIT_STATIC_DATA FILE
    *GlblOutFile = NULL;

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
  { "RenderModel",    "-r", (VoidPtr) &GlblDistRndrModel,   IC_INTEGER_TYPE },
  { "SampPerCurve",   "-f", (VoidPtr) &GlblSamplesPerCurve, IC_INTEGER_TYPE },
  { "PolyOpti",       "-F", (VoidPtr) &GlblPolyOptimalMethod,IC_INTEGER_TYPE },
  { "ZBufSize",       "-Z", (VoidPtr) &GlblZBufferSize,     IC_INTEGER_TYPE },
  { "MoreVerbose",    "-m", (VoidPtr) &GlblTalkative,	    IC_BOOLEAN_TYPE },
  { "BinaryOutput",   "-b", (VoidPtr) &GlblBinaryOutput,    IC_BOOLEAN_TYPE },
  { "VisibTest",      "-Z", (VoidPtr) &GlblApplyVisibTest,  IC_BOOLEAN_TYPE },
  { "Texture",        "-T", (VoidPtr) &GlblSrfTexture,	    IC_STRING_TYPE },
  { "LightSrcDir",    "-l", (VoidPtr) &GlblLightSourceStr,  IC_STRING_TYPE },
  { "ViewDir",	      "-v", (VoidPtr) &GlblViewDirStr,      IC_STRING_TYPE },
  { "FineNess",       "-F", (VoidPtr) &GlblPolyFineNess,    IC_REAL_TYPE },
  { "RelStepSize",    "-R", (VoidPtr) &GlblRelStepSize,     IC_REAL_TYPE },
  { "Density",        "-d", (VoidPtr) &GlblPtsDensity,      IC_REAL_TYPE },
  { "Intensity",      "-i", (VoidPtr) &GlblIntensity,       IC_REAL_TYPE },
  { "MaxWidth",       "-w", (VoidPtr) &GlblMaxWidth,        IC_REAL_TYPE },
  { "CosinePower",    "-c", (VoidPtr) &GlblCosinePower,     IC_REAL_TYPE },
  { "WidthScale",     "-S", (VoidPtr) &GlblWidthScale,      IC_REAL_TYPE },
  { "ShaderPower",    "-s", (VoidPtr) &GlblShaderPower,     IC_REAL_TYPE },
  { "SrfZTrans",      "-t", (VoidPtr) &GlblSrfTranslate[2], IC_REAL_TYPE }
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

static void ProcessUVPts(CagdSrfStruct *Srf,
			 IPObjectStruct *PObj,
			 CagdUVType *UVPts,
			 int n,
			 const char *SrfTexture,
			 CagdSrfStruct *SrfTextureSrf,
			 ImageStruct *ImageTexture);
static CagdCrvStruct *MarchOnSurface(CagdVType Dir,
				     CagdSrfStruct *Srf,
				     CagdSrfStruct *NSrf,
				     CagdSrfStruct *DuSrf,
				     CagdSrfStruct *DvSrf,
				     CagdUVType UVOrig,
				     CagdRType Length,
				     CagdRType *BreakNormal,
				     CagdSrfStruct *SrfTextureSrf,
				     ImageStruct *ImageTexture,
				     CagdBType ClosedInU,
				     CagdBType ClosedInV);
static IrtRType LineShader(CagdSrfStruct *Srf,
			   CagdSrfStruct *NSrf,
			   CagdUVType UV);
static IrtRType GetSrfArea(CagdSrfStruct *Srf);
static ImageStruct *RLELoadImage(const char *File);

#ifdef IRIT_HAVE_URT_RLE
static ImageStruct *DiffImage(ImageStruct *Image, int XDir);
#endif /* IRIT_HAVE_URT_RLE */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of LineShad - Read command line and do what is needed...	     M
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
	WidthScaleFlag = FALSE,
	ShaderPowerFlag = FALSE,
	CrvOptiPolylineFlag = FALSE,
	SrfFineNessFlag = FALSE,
	RelStepSizeFlag = FALSE,
	SrfZTransFlag = FALSE,
	TextureFlag = FALSE,
	MaxWidthFlag = FALSE,
	IntensityFlag = FALSE,
        LightSrcFlag = FALSE,
        ViewDirFlag = FALSE,
	PtsDensityFlag = FALSE,
	OutFileFlag = FALSE,
	VerFlag = FALSE,
	NumFiles = 0;
    char
	**FileNames = NULL;
    IPObjectStruct *PObjects, *PObj;
    IrtHmgnMatType CrntViewMat;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IritConfig("lineshad", SetUp, NUM_SET_UP);/* Read config. file if exists.*/
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
    if (!IRT_STR_NULL_ZERO_LEN(GlblViewDirStr)) {
#ifdef IRIT_DOUBLE
	if (sscanf(GlblViewDirStr, "%lf,%lf,%lf",
#else
	if (sscanf(GlblViewDirStr, "%f,%f,%f",
#endif /* IRIT_DOUBLE */
		   &GlblViewDir[0],
		   &GlblViewDir[1],
		   &GlblViewDir[2]) != 3) {
	    IRIT_WARNING_MSG(
		    "Fail to parse ViewDir in configuration file.\n");
	    ShaderExit(-1);
	}
    }

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &OutFileFlag, &GlblOutFileName, &GlblTalkative,
			   &SrfFineNessFlag, &GlblPolyOptimalMethod,
			   &GlblPolyFineNess, &RelStepSizeFlag,
			   &GlblRelStepSize, &CrvOptiPolylineFlag,
			   &GlblCrvApproxMethod, &GlblSamplesPerCurve,
			   &DistCompFlag, &GlblDistRndrModel,
			   &CosinePowerFlag, &GlblCosinePower,
			   &ShaderPowerFlag, &GlblShaderPower,
			   &IntensityFlag, &GlblIntensity,
			   &LightSrcFlag, &GlblLightSource[0],
			   &GlblLightSource[1], &GlblLightSource[2],
			   &ViewDirFlag, &GlblViewDir[0],
			   &GlblViewDir[1], &GlblViewDir[2],
			   &MaxWidthFlag, &GlblMaxWidth,
			   &PtsDensityFlag, &GlblPtsDensity,
			   &SrfZTransFlag, &GlblSrfTranslate[2],
			   &WidthScaleFlag, &GlblWidthScale,
			   &TextureFlag, &GlblSrfTexture,
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

    if (GlblDistRndrModel != PTS_DIST_POINT_E3)
	IRIT_PT_NORMALIZE(GlblLightSource);

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

    /* If we are going to remove the hidden portion - prepare the Z buffer! */
    if (GlblApplyVisibTest) {
        if (GlblTalkative)
	    IRIT_INFO_MSG(
		    "Scan converting surfaces/polygons into ZBuffer\n");
        ScanConvertPolySrfs(PObjects, GlblZBufferSize, -IRIT_INFNTY, 0);
    }

    /* Open output file, if necessary. */
    if (OutFileFlag) {
	if ((GlblOutFile = fopen(GlblOutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", GlblOutFileName);
	    ShaderExit(2);
	}
    }
    else
	GlblOutFile = stdout;

    /* Traverse all the objects and generate the coverage for them. */
    for (PObj = PObjects; PObj != NULL; PObj = PObj -> Pnext) {
	if (GlblTalkative)
	    IRIT_INFO_MSG_PRINTF("Processing object \"%s\":",
				 IP_GET_OBJ_NAME(PObj));

	if (IP_IS_SRF_OBJ(PObj)) {
	    IrtRType
		RelPtsDensity = AttrGetObjectRealAttrib(PObj, "PtsDensity");
	    const char
	        *SrfTexture = AttrGetObjectStrAttrib(PObj, "ltexture"),
	        *ImgTexture = AttrGetObjectStrAttrib(PObj, "itexture");
	    IPObjectStruct
		*SrfTextureObj = AttrGetObjectObjAttrib(PObj, "lTextSrf");
	    ImageStruct
		*ImageText = ImgTexture != NULL ? RLELoadImage(ImgTexture)
						: NULL;
	    CagdSrfStruct *Srf,
		*SrfTextureSrf = SrfTextureObj != NULL ?
						SrfTextureObj -> U.Srfs : NULL;

	    if (SrfTexture == NULL)
		SrfTexture = GlblSrfTexture;

	    if (IP_ATTR_IS_BAD_REAL(RelPtsDensity))
		RelPtsDensity = 1.0;

	    for (Srf = PObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		IrtRType
		    Area = GetSrfArea(Srf);
		int n = (int) (GlblPtsDensity * RelPtsDensity *
			       Area * PTS_DENSITY);
		CagdUVType
		    *UVPts = SymbUniformAprxPtOnSrfDistrib(Srf, FALSE,
							   IRIT_MAX(n, 2), NULL);

		if (GlblTalkative)
		    IRIT_INFO_MSG_PRINTF(" area ~= %f", Area);

		ProcessUVPts(Srf, PObj, UVPts, n, SrfTexture,
			     SrfTextureSrf, ImageText);

		IritFree(UVPts);
	    }
	}
	else if (IP_IS_CRV_OBJ(PObj)) {
	    if (GlblApplyVisibTest) {
		IPObjectStruct *PTmp,
		    *PObjs = TestCurveVisibility(PObj -> U.Crvs,
						 GlblWidthScale);

		for (PTmp = PObjs; PTmp != NULL; PTmp = PTmp -> Pnext) {
		    PTmp -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

		    IPPutObjectToFile(GlblOutFile, PTmp, GlblBinaryOutput);
		}

		IPFreeObjectList(PObjs);
	    }
	    else
	        IPPutObjectToFile(GlblOutFile, PObj, GlblBinaryOutput);
	}

	if (GlblTalkative)
	    IRIT_INFO_MSG_PRINTF("\n");
    }

    fclose(GlblOutFile);

    ShaderExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process the even distribution of points on the surface according to the  *
* current shader and texture mapping and dump the result out.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:           To draw using line art.                                   *
*   UVPts:         Even distribution of points on Srf, in Euclidean space.   *
*   n:             Number of UV points in UVPts.                             *
*   SrfTexture:    Texture mapping function, if any, NULL otherwise.         *
*   SrfTextureSrf: Texture surface to verify positive against, if not NULL.  *
*   ImageTexture:  Image holding the gray level texture map of this surface. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessUVPts(CagdSrfStruct *Srf,
			 IPObjectStruct *PObj,
			 CagdUVType *UVPts,
			 int n,
			 const char *SrfTexture,
			 CagdSrfStruct *SrfTextureSrf,
			 ImageStruct *ImageTexture)
{
    int i;
    CagdBType ClosedInU, ClosedInV;
    CagdCrvStruct *TCrv, *TCrv1, *TCrv2,
	*ShadingCrvs = NULL;
    CagdSrfStruct
	*DuSrf = NULL,
	*DvSrf = NULL,
	*NSrf = NULL;
    CagdRType UMin, VMin, UMax, VMax, Length, Val;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    ClosedInU = CagdIsClosedSrf(Srf, CAGD_CONST_U_DIR);
    ClosedInV = CagdIsClosedSrf(Srf, CAGD_CONST_V_DIR);

#ifdef SRFS_ALL_OPEN
    ClosedInU = ClosedInV = FALSE;
#endif /* SRFS_ALL_OPEN */

    if (SrfTexture == NULL) {
	for (i = 0; i < n; i++) {
	    CagdPType Pt;
	    CagdRType *R;

	    /* Make sure we did not fail to generate all n points. */
	    if (UVPts[i][0] == -IRIT_INFNTY || UVPts[i][1] == -IRIT_INFNTY)
		break;

	    R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

	    CagdCoerceToE3(Pt, &R, -1, Srf -> PType);
	    if (!GlblApplyVisibTest ||
		TestPointVisibility(Pt[0], Pt[1], Pt[2] + GlblSrfTranslate[2])) {
		IPObjectStruct
		    *PtObj = IPGenPTObject(&Pt[0], &Pt[1], &Pt[2]);

		IPPutObjectToFile(GlblOutFile, PtObj, GlblBinaryOutput);

		IPFreeObject(PtObj);
	    }
	}
    }
    else {
	if (strnicmp(SrfTexture, "isoparam", 8) == 0) {
	    const char
	        *AuxTexture = strlen(SrfTexture) > 9 ? &SrfTexture[9] : "2_";
	    int VaryWidth, UIso, VIso;

	    UIso = AuxTexture != NULL &&
	               (AuxTexture[0] == '0' || AuxTexture[0] == '2');
	    VIso = AuxTexture != NULL &&
	               (AuxTexture[0] == '1' || AuxTexture[0] == '2');
	    VaryWidth = AuxTexture != NULL &&
	               (AuxTexture[1] == 'w' || AuxTexture[1] == 'W');

	    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
	    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
	    NSrf = SymbSrfNormalSrf(Srf);

	    /* Compute no texture - just shading. */
	    for (i = 0; i < n; i++) {
		CagdPType Pt;
		CagdRType t1, t2,
		    Shade = LineShader(Srf, NSrf, UVPts[i]),
		    *R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

		/* Evaluate position. */
		CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		/* Extract the isocurve out of the surface. */
		if (UIso) {
		    CagdRType USpeed, ULength;
		    CagdVType V;
		    R = CagdSrfEval(DuSrf, UVPts[i][0], UVPts[i][1]);

		    CagdCoerceToE3(V, &R, -1, Srf -> PType);

		    USpeed = IRIT_PT_LENGTH(V);
		    ULength = GlblIntensity / USpeed;
		    if (!VaryWidth)
		        ULength *= Shade;
		    TCrv = CagdCrvFromSrf(Srf, UVPts[i][1],
					  CAGD_CONST_V_DIR);

		    if (ULength > UMax - UMin) {
			IRIT_LIST_PUSH(TCrv, ShadingCrvs);
		    }
		    else {
			t1 = IRIT_MAX(UMin, UVPts[i][0] - ULength * 0.5);
			t2 = IRIT_MIN(UMax, UVPts[i][0] + ULength * 0.5);
			TCrv1 = CagdCrvRegionFromCrv(TCrv, t1, t2);
			CagdCrvFree(TCrv);
			IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
		    }
		}
		if (VaryWidth)
		    AttrSetRealAttrib(&ShadingCrvs -> Attr, "width",
				      Shade * GlblMaxWidth);

		if (VIso) {
		    CagdRType VSpeed, VLength;
		    CagdVType V;
		    R = CagdSrfEval(DvSrf, UVPts[i][0], UVPts[i][1]);

		    CagdCoerceToE3(V, &R, -1, Srf -> PType);

		    VSpeed = IRIT_PT_LENGTH(V);
		    VLength = GlblIntensity / VSpeed;
		    if (!VaryWidth)
		        VLength *= Shade;
		    TCrv = CagdCrvFromSrf(Srf, UVPts[i][0],
					  CAGD_CONST_U_DIR);

		    if (VLength > VMax - VMin) {
			IRIT_LIST_PUSH(TCrv, ShadingCrvs);
		    }
		    else {
			t1 = IRIT_MAX(VMin, UVPts[i][1] - VLength * 0.5);
			t2 = IRIT_MIN(VMax, UVPts[i][1] + VLength * 0.5);
			TCrv1 = CagdCrvRegionFromCrv(TCrv, t1, t2);
			CagdCrvFree(TCrv);
			IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
		    }
		}
		if (VaryWidth)
		    AttrSetRealAttrib(&ShadingCrvs -> Attr, "width",
				      Shade * GlblMaxWidth);
	    }
	}
	else if (strnicmp(SrfTexture, "wood", 4) == 0) {
	    IrtVecType WoodDir, WoodDirInv;

	    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
	    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
	    NSrf = SymbSrfNormalSrf(Srf);

	    if (strlen(SrfTexture) > 5) {
#		ifdef IRIT_DOUBLE
		    if (sscanf(&SrfTexture[5], "%lf,%lf,%lf",
#		else
		    if (sscanf(&SrfTexture[5], "%f,%f,%f",
#		endif /* IRIT_DOUBLE */
			       &WoodDir[0], &WoodDir[1], &WoodDir[2]) != 3) {
			IRIT_WARNING_MSG_PRINTF(
				"Failed to extract direction of wood texture \"%s\"\n",
				SrfTexture);

			return;
		    }
		IRIT_PT_NORMALIZE(WoodDir);
	    }
	    else {
		WoodDir[0] = WoodDir[1] = 0.0;
		WoodDir[2] = 1.0;
	    }
	    IRIT_PT_COPY(WoodDirInv, WoodDir);
	    IRIT_PT_SCALE(WoodDirInv, -1.0);

	    /* Compute the wood texture. */
	    for (i = 0; i < n; i++) {
		CagdPType Pt;
		CagdVType Nrml;
		CagdRType
		    *R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

		/* Evaluate position. */
		CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		/* Evaluate normal. */
		R = CagdSrfEval(NSrf, UVPts[i][0], UVPts[i][1]);
		CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		IRIT_PT_NORMALIZE(Nrml);

		Length = GlblIntensity *
		    (WOOD_MIN_LENGTH +
		     (1 - IRIT_FABS(IRIT_DOT_PROD(Nrml, WoodDir))));

		/* We should favour short edges as they "cover" less area. */
		Val = IritRandom(0.0, 1.1);
		if (Length < IRIT_SQR(IRIT_SQR(Val))) {
		    TCrv1 = MarchOnSurface(WoodDir, Srf, NSrf, DuSrf, DvSrf,
					   UVPts[i], Length * 0.5, Nrml,
					   SrfTextureSrf, ImageTexture,
					   ClosedInU, ClosedInV);
		    TCrv2 = MarchOnSurface(WoodDirInv, Srf, NSrf, DuSrf, DvSrf,
					   UVPts[i], Length * 0.5, Nrml,
					   SrfTextureSrf, ImageTexture,
					   ClosedInU, ClosedInV);

		    /* Add width attribute. */
		    if (GlblDistRndrModel == NO_SHADER)
		        Val = IRIT_SQR(IRIT_SQR(IritRandom(0.0, 1.0)));
		    else
		        Val = LineShader(Srf, NSrf, UVPts[i]);

		    if (TCrv1 != NULL) {
			AttrSetRealAttrib(&TCrv1 -> Attr, "width",
					  GlblMaxWidth * Val);
		        IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
		    }
		    if (TCrv2 != NULL) {
			AttrSetRealAttrib(&TCrv2 -> Attr, "width",
					  GlblMaxWidth * Val);
		        IRIT_LIST_PUSH(TCrv2, ShadingCrvs);
		    }
		}
	    }
	}
	else if (strnicmp(SrfTexture, "vood", 4) == 0) {
	    IrtRType RotAngles[2];
	    CagdSrfStruct *TSrf;
	    IrtHmgnMatType Mat1, Mat2, Mat, InvMat;

	    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
	    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
	    NSrf = SymbSrfNormalSrf(Srf);

	    if (strlen(SrfTexture) > 5) {
#		ifdef IRIT_DOUBLE
		    if (sscanf(&SrfTexture[5], "%lf,%lf",
#		else
		    if (sscanf(&SrfTexture[5], "%f,%f",
#		endif /* IRIT_DOUBLE */
			       &RotAngles[0], &RotAngles[1]) != 2) {
			IRIT_WARNING_MSG_PRINTF(
				"Failed to extract rotations of Vood texture \"%s\"\n",
				SrfTexture);

			return;
		    }
	        MatGenMatRotY1(IRIT_DEG2RAD(RotAngles[0]), Mat1);
	        MatGenMatRotZ1(IRIT_DEG2RAD(RotAngles[1]), Mat2);
	        MatMultTwo4by4(Mat, Mat1, Mat2);
		MatInverseMatrix(Mat, InvMat);

		TSrf = CagdSrfMatTransform(Srf, Mat);
	    }
	    else {
                MatGenUnitMat(InvMat);
	        TSrf = CagdSrfCopy(Srf);
	    }

	    /* Compute the Vood texture. */
	    for (i = 0; i < n; i++) {
		CagdPType Pt;
		CagdVType Nrml;
		CagdRType
		    *R = CagdSrfEval(TSrf, UVPts[i][0], UVPts[i][1]);

		/* Evaluate position. */
		CagdCoerceToE3(Pt, &R, -1, TSrf -> PType);

		/* Evaluate normal. */
		R = CagdSrfEval(NSrf, UVPts[i][0], UVPts[i][1]);
		CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		IRIT_PT_NORMALIZE(Nrml);

		TCrv1 = MarchOnSurface(NULL, TSrf, NSrf,
				       DuSrf, DvSrf,
				       UVPts[i], GlblIntensity * 0.5, Nrml,
				       SrfTextureSrf, ImageTexture,
				       ClosedInU, ClosedInV);

		TCrv2 = MarchOnSurface(NULL, TSrf, NSrf,
				       DuSrf, DvSrf,
				       UVPts[i], GlblIntensity * 0.5, Nrml,
				       SrfTextureSrf, ImageTexture,
				       ClosedInU, ClosedInV);

		/* Add width attribute. */
		if (GlblDistRndrModel == NO_SHADER)
		    Val = IRIT_SQR(IRIT_SQR(IritRandom(0.0, 1.0)));
		else
		    Val = LineShader(TSrf, NSrf, UVPts[i]);

		if (TCrv1 != NULL) {
		    TCrv = CagdCrvMatTransform(TCrv1, InvMat);
		    CagdCrvFree(TCrv1);
		    AttrSetRealAttrib(&TCrv -> Attr, "width",
				      GlblMaxWidth * Val);
		    IRIT_LIST_PUSH(TCrv, ShadingCrvs);
		}

		if (TCrv2 != NULL) {
		    TCrv = CagdCrvMatTransform(TCrv2, InvMat);
		    CagdCrvFree(TCrv2);
		    AttrSetRealAttrib(&TCrv -> Attr, "width",
				      GlblMaxWidth * Val);
		    IRIT_LIST_PUSH(TCrv, ShadingCrvs);
		}
	    }

	    CagdSrfFree(TSrf);
	}
	else if (strnicmp(SrfTexture, "isomarch", 8) == 0) {
	    const char
	        *AuxTexture = strlen(SrfTexture) > 9 ? &SrfTexture[9] : "2_";
	    IrtVecType Dir, DirInv;

	    NSrf = SymbSrfNormalSrf(Srf);

	    if (AuxTexture[0] == '0' || AuxTexture[0] == '2') {
		IRIT_PT_RESET(Dir);
		IRIT_PT_RESET(DirInv);
		Dir[0] = 1.0; 
		DirInv[0] = -1.0; 
	    
		/* Compute the isomarch texture. */
		for (i = 0; i < n; i++) {
		    CagdPType Pt;
		    CagdVType Nrml;
		    CagdRType RandVal,
		        *R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

		    /* Evaluate position. */
		    CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		    /* Evaluate normal. */
		    R = CagdSrfEval(NSrf, UVPts[i][0], UVPts[i][1]);
		    CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		    IRIT_PT_NORMALIZE(Nrml);

		    Length = GlblIntensity;

		    /* We should favour short edges that "covers" less area. */
		    RandVal = IritRandom(0.0, 1.1);
		    if (Length < IRIT_SQR(IRIT_SQR(RandVal))) {
			TCrv1 = MarchOnSurface(Dir, Srf, NSrf, NULL, NULL,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);
			TCrv2 = MarchOnSurface(DirInv, Srf, NSrf, NULL, NULL,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);

			/* Add width attribute. */
			RandVal = IRIT_SQR(IRIT_SQR(IritRandom(0.0, 1.0)));

			if (TCrv1 != NULL) {
			    AttrSetRealAttrib(&TCrv1 -> Attr, "width",
					      GlblMaxWidth * RandVal);
			    IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
			}
			if (TCrv2 != NULL) {
			    AttrSetRealAttrib(&TCrv2 -> Attr, "width",
					      GlblMaxWidth * RandVal);
			    IRIT_LIST_PUSH(TCrv2, ShadingCrvs);
			}
		    }
		}
	    }
	    if (AuxTexture[0] == '1' || AuxTexture[0] == '2') {
		IRIT_PT_RESET(Dir);
		IRIT_PT_RESET(DirInv);
		Dir[1] = 1.0; 
		DirInv[1] = -1.0; 
	    
		/* Compute the isomatch texture. */
		for (i = 0; i < n; i++) {
		    CagdPType Pt;
		    CagdVType Nrml;
		    CagdRType RandVal,
		        *R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

		    /* Evaluate position. */
		    CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		    /* Evaluate normal. */
		    R = CagdSrfEval(NSrf, UVPts[i][0], UVPts[i][1]);
		    CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		    IRIT_PT_NORMALIZE(Nrml);

		    Length = GlblIntensity;

		    /* We should favour short edges that "covers" less area. */
		    RandVal = IritRandom(0.0, 1.1);
		    if (Length < IRIT_SQR(IRIT_SQR(RandVal))) {
			TCrv1 = MarchOnSurface(Dir, Srf, NSrf, NULL, NULL,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);
			TCrv2 = MarchOnSurface(DirInv, Srf, NSrf, NULL, NULL,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);

			/* Add width attribute. */
			RandVal = IRIT_SQR(IRIT_SQR(IritRandom(0.0, 1.0)));

			if (TCrv1 != NULL) {
			    AttrSetRealAttrib(&TCrv1 -> Attr, "width",
					      GlblMaxWidth * RandVal);
			    IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
			}
			if (TCrv2 != NULL) {
			    AttrSetRealAttrib(&TCrv2 -> Attr, "width",
					      GlblMaxWidth * RandVal);
			    IRIT_LIST_PUSH(TCrv2, ShadingCrvs);
			}
		    }
		}
	    }
	}
	else if (strnicmp(SrfTexture, "silhouette", 10) == 0) {
	    IRIT_STATIC_DATA IrtVecType MarchDir, MarchDirInv;
	    IrtVecType ViewDirInv;
	    int TangCrvs, NrmlCrvs;
	    const char
	        *AuxTexture = strlen(SrfTexture) > 11 ? &SrfTexture[11] : "tn";

	    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
	    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
	    NSrf = SymbSrfNormalSrf(Srf);

	    IRIT_PT_NORMALIZE(GlblViewDir);
	    IRIT_PT_COPY(ViewDirInv, GlblViewDir);
	    IRIT_PT_SCALE(ViewDirInv, -1);

	    TangCrvs = strchr(AuxTexture, 't') != NULL ||
	               strchr(AuxTexture, 'T') != NULL;
	    NrmlCrvs = strchr(AuxTexture, 'n') != NULL ||
	               strchr(AuxTexture, 'N') != NULL;

	    /* Compute the silheoutte texture. */
	    for (i = 0; i < n; i++) {
		CagdPType Pt;
		CagdVType Nrml;
		CagdRType RandVal,
		    *R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

		/* Evaluate position. */
		CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		/* Evaluate normal. */
		R = CagdSrfEval(NSrf, UVPts[i][0], UVPts[i][1]);
		CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		IRIT_PT_NORMALIZE(Nrml);

		Length = pow(1 - IRIT_FABS(IRIT_DOT_PROD(Nrml, GlblViewDir)),
			     GlblCosinePower);

		/* We should favour long edges near the silhouette. */
		RandVal = IritRandom(0.0, 1.0);
		if (Length > RandVal) {
		    Length *= GlblIntensity;

		    IRIT_CROSS_PROD(MarchDir, Nrml, GlblViewDir);
		    IRIT_PT_NORMALIZE(MarchDir);
		    IRIT_PT_COPY(MarchDirInv, MarchDir);
		    IRIT_PT_SCALE(MarchDirInv, -1.0);

		    if (TangCrvs) {
			TCrv1 = MarchOnSurface(MarchDir, Srf, NSrf,
					       DuSrf, DvSrf,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);
			TCrv2 = MarchOnSurface(MarchDirInv, Srf, NSrf,
					       DuSrf, DvSrf,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);

			if (TCrv1 != NULL)
			    IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
			if (TCrv2 != NULL)
			    IRIT_LIST_PUSH(TCrv2, ShadingCrvs);
		    }

		    if (NrmlCrvs) {
			TCrv1 = MarchOnSurface(GlblViewDir, Srf, NSrf,
					       DuSrf, DvSrf,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);
			TCrv2 = MarchOnSurface(ViewDirInv, Srf, NSrf,
					       DuSrf, DvSrf,
					       UVPts[i], Length * 0.5, Nrml,
					       SrfTextureSrf, ImageTexture,
					       ClosedInU, ClosedInV);
			
			if (TCrv1 != NULL)
			    IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
			if (TCrv2 != NULL)
			    IRIT_LIST_PUSH(TCrv2, ShadingCrvs);
		    }
		}
	    }
	}
	else if (strnicmp(SrfTexture, "itexture", 8) == 0) {
	    IrtVecType ViewDirInv;

	    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
	    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
	    NSrf = SymbSrfNormalSrf(Srf);

	    IRIT_PT_NORMALIZE(GlblViewDir);
	    IRIT_PT_COPY(ViewDirInv, GlblViewDir);
	    IRIT_PT_SCALE(ViewDirInv, -1);

	    for (i = 0; i < n; i++) {
		CagdPType Pt;
		CagdVType Nrml;
		CagdRType
		    *R = CagdSrfEval(Srf, UVPts[i][0], UVPts[i][1]);

		/* Evaluate position. */
		CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		/* Evaluate normal. */
		R = CagdSrfEval(NSrf, UVPts[i][0], UVPts[i][1]);
		CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		IRIT_PT_NORMALIZE(Nrml);

		Length = GlblIntensity;

		/* We should favour short edges as they "cover" less area. */
		TCrv1 = MarchOnSurface(GlblViewDir, Srf, NSrf, DuSrf, DvSrf,
				       UVPts[i], Length * 0.5, Nrml,
				       SrfTextureSrf, ImageTexture,
				       ClosedInU, ClosedInV);
		TCrv2 = MarchOnSurface(ViewDirInv, Srf, NSrf, DuSrf, DvSrf,
				       UVPts[i], Length * 0.5, Nrml,
				       SrfTextureSrf, ImageTexture,
				       ClosedInU, ClosedInV);

		if (TCrv1 != NULL) {
		    IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
		}
		if (TCrv2 != NULL) {
		    IRIT_LIST_PUSH(TCrv2, ShadingCrvs);
		}
	    }
	}
	else if (strnicmp(SrfTexture, "curvature", 9) == 0) {
	    const char
		*AuxTexture = strlen(SrfTexture) > 10 ? &SrfTexture[10] : "2";
	    int MinPrinCrv = AuxTexture[0] == '0' || AuxTexture[0] == '2',
		MaxPrinCrv = AuxTexture[0] == '1' || AuxTexture[0] == '2';

	    NSrf = SymbSrfNormalSrf(Srf);

	    SymbEvalSrfCurvPrep(Srf, TRUE);

	    for (i = 0; i < n; i++) {
		CagdRType K1, K2;
		CagdVType D1, D2, D;

		Length = GlblIntensity;

		if (GlblDistRndrModel == NO_SHADER)
		    Length *= IRIT_SQR(IRIT_SQR(IritRandom(0.0, 1.0)));
		else
		    Length *= LineShader(Srf, NSrf, UVPts[i]);

		SymbEvalSrfCurvature(Srf, UVPts[i][0], UVPts[i][1], TRUE,
				     &K1, &K2, D1, D2);

		if (IRIT_FABS(K1) < IRIT_FABS(K2)) {
		    IRIT_SWAP(IrtRType, K1, K2);
		    IRIT_SWAP(IrtRType, D1[0], D2[0]);
		    IRIT_SWAP(IrtRType, D1[1], D2[1]);
		    IRIT_SWAP(IrtRType, D1[2], D2[2]);
		}

		/* We should favour short edges as they "cover" less area. */
		if (MaxPrinCrv) {
		    IRIT_PT_COPY(D, D1);
		    if ((TCrv = MarchOnSurface(D, Srf, NULL, NULL, NULL,
					       UVPts[i], Length * 0.5, NULL,
					       NULL, NULL,
					       ClosedInU, ClosedInV)) != NULL)
		    IRIT_LIST_PUSH(TCrv, ShadingCrvs);

		    IRIT_PT_COPY(D, D1);
		    IRIT_PT_SCALE(D, -1);
		    if ((TCrv = MarchOnSurface(D, Srf, NULL, NULL, NULL,
					       UVPts[i], Length * 0.5, NULL,
					       NULL, NULL,
					       ClosedInU, ClosedInV)) != NULL)
		    IRIT_LIST_PUSH(TCrv, ShadingCrvs);
		}
		if (MinPrinCrv) {
		    IRIT_PT_COPY(D, D2);
		    if ((TCrv = MarchOnSurface(D, Srf, NULL, NULL, NULL,
					       UVPts[i], Length * 0.5, NULL,
					       NULL, NULL,
					       ClosedInU, ClosedInV)) != NULL)
		        IRIT_LIST_PUSH(TCrv, ShadingCrvs);

		    IRIT_PT_COPY(D, D2);
		    IRIT_PT_SCALE(D, -1);
		    if ((TCrv = MarchOnSurface(D, Srf, NULL, NULL, NULL,
					       UVPts[i], Length * 0.5, NULL,
					       NULL, NULL,
					       ClosedInU, ClosedInV)) != NULL)
		        IRIT_LIST_PUSH(TCrv, ShadingCrvs);
		}
	    }

	    SymbEvalSrfCurvPrep(Srf, FALSE);
	}
	else if (strnicmp(SrfTexture, "curvestroke", 11) == 0) {
	    const char
		*AuxTexture = strlen(SrfTexture) > 12 ? &SrfTexture[12] : "5";
	    IPObjectStruct
	        *PatternObj = AttrGetObjectObjAttrib(PObj, "CurveStroke");
	    CagdCrvStruct
		*PatternCrv = PatternObj != NULL && IP_IS_CRV_OBJ(PatternObj) ?
						PatternObj -> U.Crvs : NULL;
	    int i, j, PCLength, PCOrder,
		FineNess = atoi(AuxTexture),
		UVLen = 0;
	    CagdRType *R, *PCKV, T1,
		T2 = 0.0;
	    CagdUVType *UVPattern;

	    if (PatternCrv == NULL) {
		IRIT_WARNING_MSG("Expected a \"CurveStroke\" curve attribute as a stroke, surface ignored\n");
		return;
	    }
	    PCLength = PatternCrv -> Length;
	    PCOrder = PatternCrv -> Order;

	    NSrf = SymbSrfNormalSrf(Srf);

	    if (CAGD_IS_BEZIER_CRV(PatternCrv)) {
		PatternCrv = CagdCnvrtBzr2BspCrv(PatternCrv);
	    }
	    else {
		PatternCrv = CagdCrvCopy(PatternCrv);
	    }
	    PCKV = PatternCrv -> KnotVector;

	    if (FineNess < 1)
		FineNess = 1;
	    if (FineNess > 10)
		FineNess = 10;

	    /* Evaluate the pattern only once. */
	    UVPattern = (CagdUVType *) IritMalloc(FineNess
						        * (PCLength + PCOrder)
				                        * sizeof(CagdUVType));
	    for (i = PCOrder - 1; i < PCLength; ) {
		T1 = PCKV[i];
		T2 = PCKV[++i];

		while (IRIT_APX_EQ(T1, T2) && i < PCLength)
		    T2 = PCKV[++i];

		for (j = 0; j < FineNess; j++) {
		    CagdRType
			t = ((CagdRType) j) / FineNess;

		    t = t * T2 + (1 - t) * T1;
		    R = CagdCrvEval(PatternCrv, t);
		    CagdCoerceToE2((CagdRType *) &UVPattern[UVLen++], &R, -1,
				   PatternCrv -> PType);
		}
	    }
	    R = CagdCrvEval(PatternCrv, T2);
	    CagdCoerceToE2((CagdRType *) &UVPattern[UVLen++], &R, -1,
			   PatternCrv -> PType);
	    
	    /* Compute the pattern based texture. */
	    for (i = 0; i < n; i++) {
		CagdRType **Points;

		TCrv1 = BspCrvNew(UVLen, IRIT_MIN(UVLen, 2), CAGD_PT_E3_TYPE);
		BspKnotUniformOpen(UVLen, IRIT_MIN(UVLen, 2),
				   TCrv1 -> KnotVector);
		Points = TCrv1 -> Points;

		for (j = 0; j < UVLen; j++) {
		    CagdPType Pt;
		    CagdRType *R,
			U = UVPts[i][0] + UVPattern[j][0],
			V = UVPts[i][1] + UVPattern[j][1];


		    if (ClosedInU) {
			if (U < UMin)
			    U = UMax + (U - UMin);
			else if (U > UMax)
			    U = UMin + (U - UMax);
		    }
		    if (ClosedInV) {
			if (V < VMin)
			    V = VMax + (V - VMin);
			else if (V > VMax)
			    V = VMin + (V - VMax);
		    }
		    U = IRIT_BOUND(U, UMin, UMax);
		    V = IRIT_BOUND(V, VMin, VMax);

		    R = CagdSrfEval(Srf, U, V);
		    CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

		    Points[1][j] = Pt[0];
		    Points[2][j] = Pt[1];
		    Points[3][j] = Pt[2];
		}
 
		/* Add width attribute. */
		if (GlblDistRndrModel == NO_SHADER)
		    Val = IRIT_SQR(IRIT_SQR(IritRandom(0.0, 1.0)));
		else
		    Val = LineShader(Srf, NSrf, UVPts[i]);

		if (TCrv1 != NULL) {
		    AttrSetRealAttrib(&TCrv1 -> Attr, "width",
				      GlblMaxWidth * Val);
		    IRIT_LIST_PUSH(TCrv1, ShadingCrvs);
		}
	    }

	    CagdCrvFree(PatternCrv);
	}
	else {
	    IRIT_WARNING_MSG_PRINTF("Undefined texture \"%s\" ignored\n", SrfTexture);
	}
    }

    if (ShadingCrvs != NULL) {
	while (ShadingCrvs) {
	    CagdCrvStruct
		*Crv = ShadingCrvs;
	    IPObjectStruct *PObj;
	    IrtRType
		Width = AttrGetRealAttrib(Crv -> Attr, "width");

	    ShadingCrvs = ShadingCrvs -> Pnext;
	    Crv -> Pnext = NULL;

	    if (GlblApplyVisibTest) {
		IPObjectStruct
		    *PObjs = TestCurveVisibility(Crv, GlblWidthScale);

		for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
		    if (!IP_ATTR_IS_BAD_REAL(Width))
		        AttrSetObjectRealAttrib(PObj, "width", Width);

		    IPPutObjectToFile(GlblOutFile, PObj, GlblBinaryOutput);
		}

		IPFreeObjectList(PObjs);
		CagdCrvFree(Crv);
	    }
	    else {
		PObj = IPGenCrvObject("LineArt", Crv, NULL);

		if (IP_ATTR_IS_BAD_REAL(Width))
		    AttrSetObjectRealAttrib(PObj, "width", Width);

		IPPutObjectToFile(GlblOutFile, PObj, GlblBinaryOutput);
		IPFreeObject(PObj);
	    }
	}
    }

    if (NSrf != NULL)
        CagdSrfFree(NSrf);
    if (DuSrf != NULL)
        CagdSrfFree(DuSrf);
    if (DvSrf != NULL)
        CagdSrfFree(DvSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Marches and create a curve on the surface of given Length in principal   *
* direction Dir.                                                             *
*   If DuSrf = DvSrf = NULL, the direction is taken directly from Dir.	     *
*   If Dir = NULL, the direction is coerced to by the vertical direction.    *
*   If NSrf == NULL, direction is derived from surface principal directions. *
*                                                                            *
* PARAMETERS:                                                                *
*   Dir:         Direction to march on surface (projected to tangent plane). *
*   Srf:         Surface to march on.                                        *
*   NSrf:        Normal field of surface to march on.                        *
*   DuSrf:       Partial with respect to u.                                  *
*   DvSrf:       Partial with respect to v.                                  *
*   UVOrig:      Origin on surface.                                          *
*   Length:      Length of March.                                            *
*   BreakNormal: If not NULL, compare current normal with BreatNormal and    *
*		 break of too much of a difference.			     *
*   SrfTextureSrf:  A texture surface to verify positivity against, if not   *
*		 NULL.							     *
*   ImageTexture:  Image holding the gray level texture map of this surface. *
*   ClosedInU:	 TRUE if surface is closed in U direction, FALSE otherwise.  *
*   ClosedInV:	 TRUE if surface is closed in V direction, FALSE otherwise.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   A curve that approximates the march.                  *
*****************************************************************************/
static CagdCrvStruct *MarchOnSurface(CagdVType Dir,
				     CagdSrfStruct *Srf,
				     CagdSrfStruct *NSrf,
				     CagdSrfStruct *DuSrf,
				     CagdSrfStruct *DvSrf,
				     CagdUVType UVOrig,
				     CagdRType Length,
				     CagdRType *BreakNormal,
				     CagdSrfStruct *SrfTextureSrf,
				     ImageStruct *ImageTexture,
				     CagdBType ClosedInU,
				     CagdBType ClosedInV)
{
    int i, j, CrvLen;
    CagdPType Pt;
    CagdVType Nrml, Du, Dv, V, TangentDir;
    CagdRType Det, Wu, Wv, UMin, VMin, UMax, VMax, **Points, d, *R,
	SrfMarchStep = GlblRelStepSize *
			  (Dir == NULL ? SRF_MARCH_STEP / 10 : SRF_MARCH_STEP);
    CagdUVType UV;
    CagdPtStruct *CrvPt,
	*CrvPtList = NULL;
    CagdCrvStruct *Crv;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    IRIT_GEN_COPY(UV, UVOrig, 2 * sizeof(CagdRType));

    if (SrfMarchStep * 5 > Length)
        SrfMarchStep = Length / 5;

    do {
	if (SrfTextureSrf != NULL) {
	    R = CagdSrfEval(SrfTextureSrf, UV[0], UV[1]);
	    if (R[1] < 0.0)
		break;
	}

	R = CagdSrfEval(Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt, &R, -1, Srf -> PType);

	if (Dir == NULL &&
	    CrvPtList != NULL &&
	    IRIT_FABS(Pt[0] - CrvPtList -> Pt[0]) > MAX_X_DIFF)
	    break;

	CrvPt = CagdPtNew();
	IRIT_PT_COPY(CrvPt -> Pt, Pt);
	IRIT_LIST_PUSH(CrvPt, CrvPtList);

	if (NSrf == NULL) {
	    CagdRType K1, K2;
	    CagdVType D1, D2;

	    SymbEvalSrfCurvature(Srf, UV[0], UV[1], TRUE, &K1, &K2, D1, D2);
	    IRIT_PT_NORMALIZE(D1);
	    IRIT_PT_NORMALIZE(D2);
	    if (IRIT_FABS(K1 = IRIT_DOT_PROD(D1, Dir)) >
		IRIT_FABS(K2 = IRIT_DOT_PROD(D2, Dir))) {
		if (K1 < 0)
		    IRIT_PT_SCALE(D1, -1);
		IRIT_PT_COPY(Dir, D1);
		IRIT_PT_NORMALIZE(D1);
		Wu = D1[0] * SrfMarchStep;
		Wv = D1[1] * SrfMarchStep;
	    }
	    else {
		if (K2 < 0)
		    IRIT_PT_SCALE(D2, -1);
		IRIT_PT_COPY(Dir, D2);
		IRIT_PT_NORMALIZE(D2);
		Wu = D2[0] * SrfMarchStep;
		Wv = D2[1] * SrfMarchStep;
	    }
	}
	else if (ImageTexture != NULL) {
	    i = (int) ((ImageTexture -> xSize - 1) *
		       (UV[0] - UMin) / (UMax - UMin));
	    j = (int) ((ImageTexture -> ySize - 1) *
		       (UV[1] - VMin) / (VMax - VMin));

	    i = i + j * (ImageTexture -> xSize - 1);
	    Wu = -((CagdRType) ImageTexture -> DyImage -> data[i].Gray);
	    Wv =  ((CagdRType) ImageTexture -> DxImage -> data[i].Gray);
	    if ((d = sqrt(IRIT_SQR(Wu) + IRIT_SQR(Wv))) > 0.0) {
		Wu *= SrfMarchStep / d;
		Wv *= SrfMarchStep / d;
	    }
	    else {
	    }
	}
	else if (DuSrf != NULL && DvSrf != NULL) {
	    R = CagdSrfEval(DuSrf, UV[0], UV[1]);
	    CagdCoerceToE3(Du, &R, -1, DuSrf -> PType);

	    R = CagdSrfEval(DvSrf, UV[0], UV[1]);
	    CagdCoerceToE3(Dv, &R, -1, DvSrf -> PType);

	    R = CagdSrfEval(NSrf, UV[0], UV[1]);
	    CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
	    IRIT_PT_NORMALIZE(Nrml);

	    if (BreakNormal != NULL &&
		IRIT_DOT_PROD(Nrml, BreakNormal) < IritRandom(0.9, 1.0))
	        break;

	    if (Dir == NULL) {
		IRIT_STATIC_DATA IrtVecType
		    XAxis = { 1, 0, 0 };

		IRIT_CROSS_PROD(TangentDir, XAxis, Nrml);
	    }
	    else {
		d = IRIT_DOT_PROD(Nrml, Dir);

		for (i = 0; i < 3; i++)
		    TangentDir[i] = Dir[i] - Nrml[i] * d;
	    }

	    /* Now figure out weights of Du and Dv to produce a vector in   */
	    /* direction TangentDir, solving "Wu Du + Wv Dv = TangentDir".  */
	    if (IRIT_FABS(Nrml[0]) > IRIT_FABS(Nrml[1]) &&
		IRIT_FABS(Nrml[0]) > IRIT_FABS(Nrml[2])) {
		/* Solve in the YZ plane. */
		Det = Du[1] * Dv[2] - Du[2] * Dv[1];
		Wu = (TangentDir[1] * Dv[2] - TangentDir[2] * Dv[1]) / Det;
		Wv = (TangentDir[2] * Du[1] - TangentDir[1] * Du[2]) / Det;
	    }
	    else if (IRIT_FABS(Nrml[1]) > IRIT_FABS(Nrml[0]) &&
		     IRIT_FABS(Nrml[1]) > IRIT_FABS(Nrml[2])) {
		/* Solve in the XZ plane. */
		Det = Du[0] * Dv[2] - Du[2] * Dv[0];
		Wu = (TangentDir[0] * Dv[2] - TangentDir[2] * Dv[0]) / Det;
		Wv = (TangentDir[2] * Du[0] - TangentDir[0] * Du[2]) / Det;
	    }
	    else {
		/* Solve in the XY plane. */
		Det = Du[0] * Dv[1] - Du[1] * Dv[0];
		Wu = (TangentDir[0] * Dv[1] - TangentDir[1] * Dv[0]) / Det;
		Wv = (TangentDir[1] * Du[0] - TangentDir[0] * Du[1]) / Det;
	    }

	    for (i = 0; i < 3; i++)
	        V[i] = Du[i] * Wu + Dv[i] * Wv;

	    if (IRIT_PT_LENGTH(V) == 0.0)
	        break;

	    Wu *= SrfMarchStep / IRIT_PT_LENGTH(V);
	    Wv *= SrfMarchStep / IRIT_PT_LENGTH(V);
	}
	else {
	    Wu = Dir[0] * SrfMarchStep;
	    Wv = Dir[1] * SrfMarchStep;
	}

	UV[0] += Wu;
	UV[1] += Wv;

	/* Check for overflow (outside surface domain) and wrap around if   */
	/* the surface is indeed closed.  Otherwise, break this loop.       */
	if (UV[0] > UMax || UV[0] < UMin) {
	    if (ClosedInU) {
	        while (UV[0] > UMax || UV[0] < UMin) {
		    if (UV[0] > UMax)
		        UV[0] -= UMax - UMin;
		    else
		        UV[0] += UMax - UMin;
		}
	    }
	    else
	        break;
	}
	if (UV[1] > VMax || UV[1] < VMin) {
	    if (ClosedInV) {
	        while (UV[1] > VMax || UV[1] < VMin) {
		    if (UV[1] > VMax)
		        UV[1] -= VMax - VMin;
		    else
		        UV[1] += VMax - VMin;
		}
	    }
	    else
	        break;
	}

	Length -= SrfMarchStep;
    }
    while (Length >= 0.0);

    CrvLen = CagdListLength(CrvPtList);
    if (CrvLen >= 2) {
#define SHRINK_VERT_POLYLINES
#ifdef SHRINK_VERT_POLYLINES
	if (Dir == NULL) {
	    /* It is a vertical line - drop the intermediate points. */
	    CrvPt = CagdPtCopy(CagdListLast(CrvPtList));
	    CagdPtFreeList(CrvPtList -> Pnext);
	    CrvPtList -> Pnext = CrvPt;
	    CrvLen = 2;
	}
#endif /* SHRINK_VERT_POLYLINES */

	Crv = BspCrvNew(CrvLen, IRIT_MIN(CrvLen, 3), CAGD_PT_E3_TYPE);
	BspKnotUniformOpen(CrvLen, IRIT_MIN(CrvLen, 3), Crv -> KnotVector);
	Points = Crv -> Points;
	for (CrvPt = CrvPtList, i = 0;
	     i < CrvLen;
	     CrvPt = CrvPt -> Pnext, i++) {
	    Points[1][i] = CrvPt -> Pt[0];
	    Points[2][i] = CrvPt -> Pt[1];
	    Points[3][i] = CrvPt -> Pt[2];
	}
    }
    else
	Crv = NULL;

    CagdPtFreeList(CrvPtList);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a shading value for a given UV point on Srf.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:     The surface with work with.                                     *
*   NSrf:    The normal field of the surface with work with.                 *
*   UV:      The current location to treat.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:    Value between zero (black) and one (maximum light).         *
*****************************************************************************/
static IrtRType LineShader(CagdSrfStruct *Srf,
			   CagdSrfStruct *NSrf,
			   CagdUVType UV)
{
    CagdVType N;
    CagdPType P;
    CagdRType *R, Z;

    switch (GlblDistRndrModel) {
	case NO_SHADER:
	case PTS_DIST_REGULAR:
	    Z = 1.0;
	    break;
	case PTS_DIST_PHONG:
	case PTS_DIST_PHONG_2L:
	case PTS_DIST_PHONG_SPEC:
	case PTS_DIST_PHONG_2L_SPEC:
	    R = CagdSrfEval(NSrf, UV[0], UV[1]);
	    CagdCoerceToE3(N, &R, -1, NSrf -> PType);
	    IRIT_PT_NORMALIZE(N);

	    if (GlblDistRndrModel == PTS_DIST_PHONG_2L ||
		GlblDistRndrModel == PTS_DIST_PHONG_2L_SPEC) {
		Z = IRIT_FABS(IRIT_DOT_PROD(N, GlblLightSource));

		/* Add specular term, if have one. */
		if (GlblDistRndrModel == PTS_DIST_PHONG_2L_SPEC)
		    Z = pow(Z, GlblCosinePower);
	    }
	    else { /* PTS_DIST_PHONG || PTS_DIST_PHONG_SPEC */
		Z = (IRIT_DOT_PROD(N, GlblLightSource) + 1.0) * 0.5;

		/* Add specular term, if have one. */
		if (GlblDistRndrModel == PTS_DIST_PHONG_SPEC)
		    Z = pow(Z, GlblCosinePower);
	    }
	    break;
	case PTS_DIST_ZNORMAL:
	    R = CagdSrfEval(NSrf, UV[0], UV[1]);
	    CagdCoerceToE3(N, &R, -1, NSrf -> PType);
	    IRIT_PT_NORMALIZE(N);

	    Z = pow(0.95 * (1.0 - N[2]) + 0.05, GlblCosinePower);
	    break;
	case PTS_DIST_POINT_E3:
	    R = CagdSrfEval(Srf,UV[0], UV[1]);
	    CagdCoerceToE3(P, &R, -1, Srf -> PType);

	    Z = IRIT_PT_PT_DIST(P, GlblLightSource);
	    Z = pow(Z, GlblShaderPower);
	    break;
	default:
	    IRIT_WARNING_MSG_PRINTF("Undefined shader %d ignored\n",
		    GlblDistRndrModel);
	    Z = 1.0;
	    break;
    }

    return Z;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   kReturns a crude approximation of the surface's area.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:      To estimate its area.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:    Estimated area                                              *
*****************************************************************************/
static IrtRType GetSrfArea(CagdSrfStruct *Srf)
{
    IPPolygonStruct
	*Polys = IPSurface2Polygons(Srf, FALSE, SRF_AREA_FINENESS,
				    FALSE, FALSE, 0);
    IPObjectStruct
	*PObj = IPGenPOLYObject(Polys);
    IrtRType
	Area = GMPolyObjectArea(PObj);

    IPFreeObject(PObj);

    return Area;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Loads image file in RLE format.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   File:  Name of the image file.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   ImageStruct *:  Pointer to dynamicaly created image.                     *
*****************************************************************************/
static ImageStruct *RLELoadImage(const char *File)
{
#ifdef IRIT_HAVE_URT_RLE
    rle_hdr Header;
    rle_pixel **Rows;
    ImageStruct *PImage;
    PixelStruct *p;
    int Error, x, y;

    Header.rle_file = rle_open_f_noexit("RleLoadImage", (char *) File, "r");
    if (!Header.rle_file)
        return NULL;
    if (Error = rle_get_setup(&Header)) {
        rle_get_error(Error, "RleLoadImage", (char *) File);
        return NULL;
    }
    rle_row_alloc(&Header, &Rows);
    PImage = (ImageStruct *) IritMalloc(sizeof(ImageStruct));
    PImage -> xSize = Header.xmax - Header.xmin;
    PImage -> ySize = Header.ymax - Header.ymin;
    PImage -> data = p = (PixelStruct *) IritMalloc(sizeof(PixelStruct) *
				(PImage -> ySize + 1) * (PImage -> xSize + 1));
    for (y = 0; y <= PImage -> ySize; y++) {
        rle_getrow(&Header, Rows);
        for (x = 0; x <= PImage -> xSize; x++, p++) {
	    if (Header.ncolors == 1)
	        p -> Gray = Rows[RLE_RED][x] / 2;   /* Between zero and 127. */
	    else /* 3 colors */
	        p -> Gray = (Rows[RLE_RED][x] +
                             Rows[RLE_GREEN][x] +
                             Rows[RLE_BLUE][x]) / 6;/* Between zero and 127. */
        }
    }

    PImage -> DxImage = DiffImage(PImage, TRUE);
    PImage -> DyImage = DiffImage(PImage, FALSE);

    return PImage;
#else
    return NULL;
#endif /* IRIT_HAVE_URT_RLE */
}

#ifdef IRIT_HAVE_URT_RLE

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Differentiate an image.	                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Image:  Image to differentiate.                                          *
*   XDir:   Differentiate in X if TRUE, in Y otherwise.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   ImageStruct *:  Differentiated image.		                     *
*****************************************************************************/
static ImageStruct *DiffImage(ImageStruct *Image, int XDir)
{
    ImageStruct
	*DImage = (ImageStruct *) IritMalloc(sizeof(ImageStruct));
    int x, y;

    DImage -> data = (PixelStruct *) IritMalloc(sizeof(PixelStruct) *
				     Image -> ySize * Image -> xSize);
    DImage -> xSize = Image -> xSize - 1;
    DImage -> ySize = Image -> ySize - 1;

    for (y = 0; y <= DImage -> ySize; y++) {
	PixelStruct
	    *p  = DImage -> data + (DImage -> xSize + 1) * y,
	    *p1 = Image -> data + (Image -> xSize + 1) * y,
	    *p2 = Image -> data + (Image -> xSize + 1) * (y + 1);

        for (x = 0; x <= DImage -> xSize; x++) {
	    if (XDir) {
		p -> Gray = p1[1].Gray - p1 -> Gray;
		p++;
		p1++;
	    }
	    else {
		p -> Gray = p2 -> Gray - p1 -> Gray;
		p++;
		p1++;
		p2++;
	    }
        }
    }

    DImage -> DxImage = DImage -> DyImage = NULL;

    return DImage;
}

#endif /* IRIT_HAVE_URT_RLE */

/*****************************************************************************
* DESCRIPTION:                                                               M
* LineShad Exit routine.						     M
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
