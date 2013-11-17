/*****************************************************************************
* Render's Victor Vasarely's ZEBRA style pictures of given geometry.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 0.1, June 2000.   *
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

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *VersionStr = "IZebra		Version 11,	Gershon Elber,\n\
	 (C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *VersionStr = "IZebra		" IRIT_VERSION ",	Gershon Elber,	"
	__DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *CtrlStr = "IZebra o%-OutName!s m%- O%-ImgOper!d F%-PolyOpti|FineNess!d!F u%- I%-NumIters!d Z%-ZbufSize!d B%-CbcBspSize!d D%-DataSrf!s A%-StripeAngle!F b%- s%-Stripes!d S%-ZScale!F d%-ZInitDepth!F z%- DFiles!*s";

IRIT_STATIC_DATA char
    *GlblSrfDataFileName = NULL,
    *GlblOutFileName = "izebra.itd";

IRIT_STATIC_DATA int
    GlblBinaryOutput = FALSE,
    GlblZBufferSize = 500,
    GlblCubicBsplineSize = 100;

IRIT_STATIC_DATA IrtRType
    GlblZScaling = 0.1,
    GlblZInitDepth = 0.0,
    GlblStripesAngle = 60.0;

IRIT_GLOBAL_DATA int
    GlblTalkative = FALSE,
    GlblMaxNumIterations = 10,
    GlblSamplesPerCurve = IG_DEFAULT_SAMPLES_PER_CURVE,
    GlblPolyOptimalMethod = 0;
IRIT_GLOBAL_DATA IrtRType
    GlblPolyFineNess = IG_DEFAULT_POLYGON_FINENESS;

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
  { "ZBufSize",       "-Z", (VoidPtr) &GlblZBufferSize,     IC_INTEGER_TYPE },
  { "CbcBspSize",     "-B", (VoidPtr) &GlblCubicBsplineSize,IC_INTEGER_TYPE },
  { "PolyOpti",       "-F", (VoidPtr) &GlblPolyOptimalMethod,IC_INTEGER_TYPE },
  { "NumIters",       "-I", (VoidPtr) &GlblMaxNumIterations,IC_INTEGER_TYPE },
  { "MoreVerbose",    "-m", (VoidPtr) &GlblTalkative,	    IC_BOOLEAN_TYPE },
  { "BinaryOutput",   "-b", (VoidPtr) &GlblBinaryOutput,    IC_BOOLEAN_TYPE },
  { "StripesAngle",   "-A", (VoidPtr) &GlblStripesAngle,    IC_REAL_TYPE },
  { "ZScale",	      "-S", (VoidPtr) &GlblZScaling,	    IC_REAL_TYPE },
  { "ZInitDepth",     "-d", (VoidPtr) &GlblZInitDepth,	    IC_REAL_TYPE },
  { "FineNess",       "-F", (VoidPtr) &GlblPolyFineNess,    IC_REAL_TYPE }
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

static void EtchZBufferIntoSurface(CagdSrfStruct *Srf);
static void UpdateSrfWhileInjective(CagdSrfStruct *Srf,
				    int x,
				    int y,
				    IrtVecType V,
				    IrtRType IterNumFrac);
void ShaderExit(int ExitCode);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of Izebra - Read command line and do what is needed...	     M
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
        ImageOperatorFlag = FALSE,
	SrfDataFileNameFlag = FALSE,
        ZBufferSizeFlag = FALSE,
	CubicBsplineSizeFlag = FALSE,
	StripesAngleFlag = FALSE,
	MaxNumIterationsFlag = FALSE,
	ForceUnitMat = FALSE,
	OutputStripesFlag = FALSE,
	SrfFineNessFlag = FALSE,
	ZInitDepthFlag = FALSE,
	ZScalingFlag = FALSE,
	OutFileFlag = FALSE,
	VerFlag = FALSE,
	ImageOperator = 0,
	NumOfStripes = 100,
	NumFiles = 0;
    char
	**FileNames = NULL;
    IPObjectStruct *PObjects, *PObj;
    IrtHmgnMatType CrntViewMat;
    CagdSrfStruct
	*Srf = NULL;
    FILE *f;
    IritConfig("izebra", SetUp, NUM_SET_UP);/* Read config. file if exists.*/

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    if ((Error = GAGetArgs(argc, argv, CtrlStr,
			   &OutFileFlag, &GlblOutFileName, &GlblTalkative,
			   &ImageOperatorFlag, &ImageOperator,
			   &SrfFineNessFlag, &GlblPolyOptimalMethod,
			   &GlblPolyFineNess, &ForceUnitMat,
			   &MaxNumIterationsFlag, &GlblMaxNumIterations,
			   &ZBufferSizeFlag, &GlblZBufferSize,
			   &CubicBsplineSizeFlag, &GlblCubicBsplineSize,
			   &SrfDataFileNameFlag, &GlblSrfDataFileName,
			   &StripesAngleFlag, &GlblStripesAngle,
			   &GlblBinaryOutput, &OutputStripesFlag,
			   &NumOfStripes, &ZScalingFlag, &GlblZScaling,
			   &ZInitDepthFlag, &GlblZInitDepth,
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

    /* Get the data files: */
    IPSetPolyListCirc(TRUE);
    if ((PObjects = IPGetDataFiles((const char **) FileNames,
				   NumFiles, TRUE, FALSE)) == NULL)
	ShaderExit(0);

    if (ForceUnitMat) {
	MatGenUnitMat(CrntViewMat);
    }
    else {
	if (IPWasPrspMat)
	    MatMultTwo4by4(CrntViewMat, IPViewMat, IPPrspMat);
	else
	    IRIT_GEN_COPY(CrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    }
    PObj = GMTransformObjectList(PObjects, CrntViewMat);
    IPFreeObjectList(PObjects);
    PObjects = PObj;

    /* Open output file, if necessary. */
    if (OutFileFlag) {
	if ((f = fopen(GlblOutFileName, "w")) == NULL) {
	    IRIT_WARNING_MSG_PRINTF("Failed to open \"%s\".\n", GlblOutFileName);
	    ShaderExit(2);
	}
    }
    else
	f = stdout;

    /* Prepare the Z buffer! */
    if (GlblTalkative)
        IRIT_INFO_MSG("Scan converting surfaces/polygons into ZBuffer\n");
    ScanConvertPolySrfs(PObjects, GlblZBufferSize,
			(ZInitDepthFlag ? GlblZInitDepth : 0.0),
			ImageOperator);
    IPFreeObjectList(PObjects);
    if (GlblTalkative) {
	int x, y;
	IrtRType z,
	    ZInit = ZInitDepthFlag ? GlblZInitDepth : 0.0,
	    ZMin = IRIT_INFNTY,
	    ZMax = -IRIT_INFNTY;

	for (x = 0; x < GlblZBufferSize; x++) {
	    for (y = 0; y < GlblZBufferSize; y++) {
		z = GetPointDepth(x, y);
		if (!IRIT_APX_EQ(z, ZInit)) {
		    if (z > ZMax)
			ZMax = z;
		    if (z < ZMin)
			ZMin = z;
		}
	    }
	}
	IRIT_INFO_MSG_PRINTF("ZBuffer detected (ZInit = %g) ZMin = %g, ZMax = %g\n",
		ZInit, ZMin, ZMax);
    }

    /* Constructs a uniform cubic Bspline above the Z buffer. */
    if (SrfDataFileNameFlag) {
	IPObjectStruct
	    *PSrf = IPGetDataFiles((const char **) &GlblSrfDataFileName,
				   1, TRUE, FALSE);

	if (PSrf == NULL || !IP_IS_SRF_OBJ(PSrf)) {
	    IRIT_WARNING_MSG_PRINTF("Failed to read a surface from \"%s\"\n",
		    GlblSrfDataFileName);
	    ShaderExit(0);
	}
	else {
	    Srf = CagdSrfCopy(PSrf -> U.Srfs);
	    IPFreeObject(PSrf);
	}
    }
    else {
	int x, y,
	    GlblCubicBsplineSize2 = (GlblCubicBsplineSize >> 1);
	CagdRType *XPts, *YPts;
	IrtHmgnMatType Mat;
	CagdSrfStruct *TSrf;

	Srf = BspSrfNew(GlblCubicBsplineSize, GlblCubicBsplineSize,
			4, 4, CAGD_PT_E2_TYPE);

	XPts = Srf -> Points[1];
	YPts = Srf -> Points[2];

	/* Initialize the mesh's grid. */
	for (x = 0; x < GlblCubicBsplineSize; x++) {
	    for (y = 0; y < GlblCubicBsplineSize; y++) {
		int Idx = CAGD_MESH_UV(Srf, x, y);

		XPts[Idx] = (x - GlblCubicBsplineSize2) /
					((CagdRType) GlblCubicBsplineSize2);
		YPts[Idx] = (y - GlblCubicBsplineSize2) /
					((CagdRType) GlblCubicBsplineSize2);
	    }
	}

	MatGenMatRotZ1(IRIT_DEG2RAD(GlblStripesAngle), Mat);
	TSrf = CagdSrfMatTransform(Srf, Mat);
	CAGD_PROPAGATE_ATTR(TSrf, Srf);
	CagdSrfFree(Srf);
	Srf = TSrf;
    }
	
    BspKnotUniformFloat(Srf -> ULength, Srf -> UOrder, Srf -> UKnotVector);
    BspKnotUniformFloat(Srf -> VLength, Srf -> VOrder, Srf -> VKnotVector);

    /* Update the surface according to the heights of the Z buffer. */
    EtchZBufferIntoSurface(Srf);

    /* Dump data out. */
    if (OutputStripesFlag) { /* Dump isoparametric curves. */
	int i = 0;
	CagdRType UMin, UMax, VMin, VMax, v, dv;

	PObj = IPGenListObject("IZebra", NULL, NULL);

	NumOfStripes = IRIT_MAX(NumOfStripes, 2);

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	dv = (VMax - VMin) / (NumOfStripes - 1);
	for (v = VMin; v <= VMax; v += dv) {
	    CagdCrvStruct
		*Crv = CagdCrvFromSrf(Srf, v, CAGD_CONST_V_DIR);

	    IPListObjectInsert(PObj, i++, IPGenCRVObject(Crv));
	}
	IPListObjectInsert(PObj, i++, NULL);
	CagdSrfFree(Srf);
    }
    else { /* Dump the surface. */
	PObj = IPGenSrfObject("IZebra", Srf, NULL);
    }

    IPPutObjectToFile(f, PObj, GlblBinaryOutput);
    IPFreeObjectList(PObj);
    fclose(f);

    ShaderExit(0);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update the control points of the surface to reflect the heights in the   *
* Z buffer...                                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        To update its XY warping to follow the Z buffer heights.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EtchZBufferIntoSurface(CagdSrfStruct *Srf)
{
    int x, y, i,
	GlblZBufferSize2 = (GlblZBufferSize >> 1);
    CagdRType *XPtsOrig, *YPtsOrig,
	*XPts = Srf -> Points[1],
	*YPts = Srf -> Points[2];
    CagdVType ShiftDir;
    IrtHmgnMatType Mat;
    CagdSrfStruct *SrfOrig;

    MatGenMatRotZ1(IRIT_DEG2RAD(GlblStripesAngle), Mat);

    ShiftDir[0] = 0.0;
    ShiftDir[1] = 1.0;
    ShiftDir[2] = 0.0;
    MatMultVecby4by4(ShiftDir, ShiftDir, Mat);

    SrfOrig = CagdSrfCopy(Srf);
    XPtsOrig = SrfOrig -> Points[1],
    YPtsOrig = SrfOrig -> Points[2];

    if (GlblTalkative)
        IRIT_INFO_MSG("Processing:\n");

    /* Iterate over all control points and move the control points following */
    /* ZBuffer heights while Srf is Injective until we converge (or not...). */
    for (i = 0; i < GlblMaxNumIterations; i++) {
	int Updated = FALSE;
	IrtRType
	    TotalError = 0.0;

	for (x = 0; x < Srf -> ULength; x++) {
	    for (y = 0; y < Srf -> VLength; y++) {
		int Idx = CAGD_MESH_UV(Srf, x, y),
		    ZBufX = (int) ((XPts[Idx] + 1.0) * GlblZBufferSize2),
		    ZBufY = (int) ((YPts[Idx] + 1.0) * GlblZBufferSize2);
		IrtRType Error,
		    Height = GetPointDepth(ZBufX, ZBufY) / GlblZBufferSize;
		IrtVecType DiffVec;

		DiffVec[0] = XPts[Idx] - XPtsOrig[Idx];
		DiffVec[1] = YPts[Idx] - YPtsOrig[Idx];
		DiffVec[2] = 0.0;

		Error = -IRIT_DOT_PROD(DiffVec, ShiftDir) - Height * GlblZScaling;

		TotalError += Error;
		if (!IRIT_APX_EQ(Error, 0.0)) {
		    IrtVecType V;

		    Updated = TRUE;

		    if (GlblTalkative && IRIT_FABS(Error) > 1.0)
			IRIT_INFO_MSG_PRINTF(
				"Shift requested might be too large (%f)\n",
				Error);
		    IRIT_VEC_COPY(V, ShiftDir);
		    IRIT_VEC_SCALE(V, Error);
		    UpdateSrfWhileInjective(Srf, x, y, V,
				    i / ((IrtRType) GlblMaxNumIterations));
		}
	    }
	}
	if (GlblTalkative)
	    IRIT_INFO_MSG_PRINTF("\tInteration %3d, Error = %g        \r",
		    i, TotalError);

	if (!Updated)
	    break;
    }

    CagdSrfFree(SrfOrig);

    if (GlblTalkative)
        IRIT_INFO_MSG("\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Updates the XY coefficients of surface Srf at indices (x, y).		     *
*   A test is conducted to make sure the surface stays injective and the     *
* largest translation is conducted upto V that preserves injectivity.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    Surface to                                                       *
*   x, y:   Indices of control point to update.                              *
*   V:      Translation vector desired.                                      *
*   IterNumFrac: Iteration number frunction.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateSrfWhileInjective(CagdSrfStruct *Srf,
				    int x,
				    int y,
				    IrtVecType V,
				    IrtRType IterNumFrac)
{
    int i, j, Injective,
	Idx = CAGD_MESH_UV(Srf, x, y);
    CagdRType
	*XPts = Srf -> Points[1],
	*YPts = Srf -> Points[2],
	OrigX = XPts[Idx],
	OrigY = YPts[Idx];

    do {
	Injective = TRUE;

	XPts[Idx] = OrigX + V[0];
	YPts[Idx] = OrigY + V[1];

	/* Test all 16 patches that this control points participates in. */
	for (i = x - 3; Injective && i <= x; i++) {
	    for (j = y - 3; Injective && j <= y; j++) {
		if (i >= 0 && i < Srf -> ULength - 3 &&
		    j >= 0 && j < Srf -> VLength - 3) {
		    int ii, jj, l;
		    CagdRType X44[4][4], Y44[4][4];

		    for (ii = 0; ii < 4; ii++) {
		        for (jj = 0; jj < 4; jj++) {
			    l = CAGD_MESH_UV(Srf, i + ii, j + jj);

			    X44[ii][jj] = XPts[l];
			    Y44[ii][jj] = YPts[l];
			}
		    }
		    if (!SymbCubicBspInjective(X44, Y44))
			Injective = FALSE;
		}
	    }
	}

	V[0] *= 0.5;
	V[1] *= 0.5;
    }
    while (!Injective && (IRIT_FABS(V[0]) + IRIT_FABS(V[1]) > IRIT_EPS));

    /* March a lot at first and then ease the step size upto a half. */
    XPts[Idx] = OrigX + V[0] * 2.0 * (1.0 - IterNumFrac);
    YPts[Idx] = OrigY + V[1] * 2.0 * (1.0 - IterNumFrac);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Izebra Exit routine.							     M
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
