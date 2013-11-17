/*****************************************************************************
*   General routines common to graphics driver.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "ip_cnvrt.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "editsrfs.h"
#include "editmanp.h"

#ifdef IRIT_HAVE_OGL_CG_LIB
#include "opngl_cg.h"
#endif /* IRIT_HAVE_OGL_CG_LIB */

#ifdef OS2GCC
#define INCL_DOSPROCESS
#include <os2.h>
#endif /* OS2GCC */

#ifdef __WINNT__
#include <windows.h>
#endif /* __WINNT__ */

#define SPHERE_CONE_DENSITY	300
#define COPLANARITY_EPS		IRIT_UEPS

typedef struct StateNameNumType {
    IGGlblStateType Num;
    char *Name;
} StateNameNumType;

IRIT_STATIC_DATA StateNameNumType StateNameNum[] =
{
    { IG_STATE_MOUSE_SENSITIVE,		"MouseSense" },
    { IG_STATE_SCR_OBJ_TGL,		"ScrnObjct" },
    { IG_STATE_PERS_ORTHO_TGL,		"PerpsOrtho" },
    { IG_STATE_DEPTH_CUE,		"DepthCue" },
    { IG_STATE_CACHE_GEOM,		"CacheGeom" },
    { IG_STATE_DRAW_STYLE,		"DrawStyle" },
    { IG_STATE_SHADING_MODEL,		"ShadingMdl" },
    { IG_STATE_BACK_FACE_CULL,		"BFaceCull" },
    { IG_STATE_DOUBLE_BUFFER,		"DblBuffer" },
    { IG_STATE_ANTI_ALIASING,		"AntiAlias" },
    { IG_STATE_DRAW_INTERNAL,		"DrawIntrnl" },
    { IG_STATE_DRAW_VNORMAL,		"DrawVNrml" },
    { IG_STATE_DRAW_PNORMAL,		"DrawPNrml" },
    { IG_STATE_DRAW_POLYGONS,		"DrawPlgns" },
    { IG_STATE_DRAW_SRF_MESH,		"DSrfMesh" },
    { IG_STATE_DRAW_SRF_WIRE,		"DSrfWire" },
    { IG_STATE_DRAW_SRF_BNDRY,		"DSrfBndry" },
    { IG_STATE_DRAW_SRF_SILH,		"DSrfSilh" },
    { IG_STATE_DRAW_SRF_POLY,		"DSrfPoly" },
    { IG_STATE_DRAW_SRF_SKTCH,		"DSrfSktch" },
    { IG_STATE_FOUR_PER_FLAT,		"4PerFlat" },
    { IG_STATE_NUM_ISOLINES,		"NumIsos" },
    { IG_STATE_POLY_APPROX,		"PolyAprx" },
    { IG_STATE_SAMP_PER_CRV_APPROX,	"PllnAprx" },
    { IG_STATE_LENGTH_VECTORS,		"LenVecs" },
    { IG_STATE_WIDTH_LINES,		"WidthLines" },
    { IG_STATE_WIDTH_POINTS,		"WidthPts" },
    { IG_STATE_VIEW_FRONT,		"Front" },
    { IG_STATE_VIEW_SIDE,		"Side" },
    { IG_STATE_VIEW_TOP,		"Top" },
    { IG_STATE_VIEW_ISOMETRY,		"Isometry" },
    { IG_STATE_VIEW_4,			"4Views" },
    { IG_STATE_CLEAR_VIEW,		"Clear" },

    { IG_STATE_RES_ADAP_ISO,		"ResAdapIso" },
    { IG_STATE_RES_RULED_SRF,		"ResRldSrf" },
    { IG_STATE_RULED_SRF_APPROX,	"RuledSrfApx" },
    { IG_STATE_ADAP_ISO_DIR,		"AdapIsoDir" },

    { IG_STATE_LOWRES_RATIO,		"LowResRatio" },

    { IG_STATE_NONE,			NULL }
};

IRIT_STATIC_DATA int
    GlblDelayedClear = FALSE,
    GlblPickedPolyIsPolyline = FALSE,
    GlblPickedPolyIsPointList = FALSE,
    GlblProcessCommandMesssages = TRUE,
    GlblThisPickObjTypes = IG_PICK_ANY;
IRIT_STATIC_DATA IrtRType
    GlblPickScale = 1.0,
    GlblMinFoundDepth = -IRIT_INFNTY;
IRIT_STATIC_DATA char
    *GlblBackGroundStr = "0, 0, 0",
    *GlblExecAnimEachStep = "",
    *GlblFirstLoadedFile = "",
    *GlblHighlight1Str = "255, 0, 255",
    *GlblHighlight2Str = "255, 200, 100",
    *GlblLightSrc1PosStr = "1.0,  2.0,  5.0, 0.0",
    *GlblLightSrc2PosStr = "1.0, -1.0, -3.0, 0.0";
IRIT_STATIC_DATA const char
    *GlblObjectSearchName = "";
IRIT_STATIC_DATA IPPolygonStruct
    *GlblPickedPoly = NULL;

IRIT_STATIC_DATA IPObjectStruct
    *GlblObjectFoundByName = NULL;

IRIT_GLOBAL_DATA int
    IGGlbl3DGlassesImgIndx = -1,
    IGGlblLightOneSide = TRUE,
    IGGlbl4Views = FALSE,
    IGGlblActiveXMode = FALSE,
    IGGlblBackGroundImage = FALSE,
    IGGlblClientHandleNumber = -1,
    IGGlblDebugEchoInputFlag = FALSE,
    IGGlblDebugObjectsFlag = FALSE,
    IGGlblForceUnitMat = FALSE,
    IGGlblInitWidgetDisplay = 0,
    IGGlblIOHandle = -1,
    IGGlblLastWasSolidRendering = FALSE,
    IGGlblPickObjTypes = IG_PICK_ANY,
    IGGlblSavePickedPoly = FALSE,
    IGGlblStandAlone = TRUE;
IRIT_GLOBAL_DATA char
    *IGGlblExecAnimation = "",
    *IGGlblImageFileName = "",
    *IGGlblPolyPickFileName = "",
    *IGGlblTransPrefPos = "455, 640, 520, 965",
    *IGGlblViewPrefPos =  "  1, 450, 520, 965";
IRIT_GLOBAL_DATA IrtRType
    IGGlblMinFoundDist = IRIT_INFNTY;
IRIT_GLOBAL_DATA IrtPtType
    IGGlblPickPosE3,
    IGGlblPickPos;
IRIT_GLOBAL_DATA IGGlasses3DType
    IGGlbl3DGlassesMode;
IRIT_GLOBAL_DATA IPObjectStruct
    *IGGlblNewDisplayObjects = NULL,
    *IGGlblPickedObj = NULL,
    *IGGlblPickedPolyObj = NULL;

IRIT_STATIC_DATA IritConfigStruct SetUp[] =
{
  { "TransPrefPos",   "-g", (VoidPtr) &IGGlblTransPrefPos,	IC_STRING_TYPE },
  { "ViewPrefPos",    "-G", (VoidPtr) &IGGlblViewPrefPos,	IC_STRING_TYPE },
  { "BackGround",     "-b", (VoidPtr) &GlblBackGroundStr,	IC_STRING_TYPE },
  { "Highlight1",     "",   (VoidPtr) &GlblHighlight1Str,	IC_STRING_TYPE },
  { "Highlight2",     "",   (VoidPtr) &GlblHighlight2Str,	IC_STRING_TYPE },
  { "LightSrcPos",    "-S", (VoidPtr) &GlblLightSrc1PosStr,	IC_STRING_TYPE },
  { "LightSrcPos2",   "",   (VoidPtr) &GlblLightSrc2PosStr,	IC_STRING_TYPE },
  { "ExecAnimCmd",    "-x", (VoidPtr) &GlblExecAnimEachStep,	IC_STRING_TYPE },
  { "ExecAnimation",  "-X", (VoidPtr) &IGGlblExecAnimation,	IC_STRING_TYPE },
  { "BackGrndImage",  "-K", (VoidPtr) &IGGlblBackGroundImage,	IC_BOOLEAN_TYPE },
  { "Internal",	      "-i", (VoidPtr) &IGGlblDrawInternal,	IC_BOOLEAN_TYPE },
  { "DrawVNormal",    "-n", (VoidPtr) &IGGlblDrawVNormal,	IC_BOOLEAN_TYPE },
  { "DrawPNormal",    "-N", (VoidPtr) &IGGlblDrawPNormal,	IC_BOOLEAN_TYPE },
  { "MoreVerbose",    "-m", (VoidPtr) &IGGlblMore,		IC_BOOLEAN_TYPE },
  { "UnitMatrix",     "-u", (VoidPtr) &IGGlblForceUnitMat,	IC_BOOLEAN_TYPE },
  { "DrawStyle",      "-r", (VoidPtr) &IGGlblDrawStyle,		IC_BOOLEAN_TYPE },
  { "BFaceCull",      "-B", (VoidPtr) &IGGlblBackFaceCull,	IC_BOOLEAN_TYPE },
  { "DoubleBuffer",   "-2", (VoidPtr) &IGGlblDoDoubleBuffer,	IC_BOOLEAN_TYPE },
  { "DebugObjects",   "-d", (VoidPtr) &IGGlblDebugObjectsFlag,	IC_BOOLEAN_TYPE },
  { "DebugEchoInput", "-D", (VoidPtr) &IGGlblDebugEchoInputFlag,IC_BOOLEAN_TYPE },
  { "DepthCue",	      "-c", (VoidPtr) &IGGlblDepthCue,		IC_BOOLEAN_TYPE },
  { "CacheGeom",      "-C", (VoidPtr) &IGGlblCacheGeom,		IC_BOOLEAN_TYPE },
  { "FourPerFlat",    "-4", (VoidPtr) &IGGlblFourPerFlat,	IC_BOOLEAN_TYPE },
  { "AntiAlias",      "-a", (VoidPtr) &IGGlblAntiAliasing,	IC_BOOLEAN_TYPE },
  { "DrawSurfaceMesh","-M", (VoidPtr) &IGGlblDrawSurfaceMesh,	IC_BOOLEAN_TYPE },
  { "DrawSurfacePoly","-P", (VoidPtr) &IGGlblDrawSurfacePoly,	IC_BOOLEAN_TYPE },
  { "DrawSurfaceWire","-W", (VoidPtr) &IGGlblDrawSurfaceWire,	IC_BOOLEAN_TYPE },
  { "DrawSurfaceSktc","-W", (VoidPtr) &IGGlblDrawSurfaceSketch,	IC_BOOLEAN_TYPE },
  { "StandAlone",     "-s", (VoidPtr) &IGGlblStandAlone,	IC_BOOLEAN_TYPE },
  { "PolyStrip",      "-R", (VoidPtr) &IGGlblPolygonStrips,	IC_BOOLEAN_TYPE },
  { "ContMotion",     "-T", (VoidPtr) &IGGlblContinuousMotion,	IC_BOOLEAN_TYPE },
  { "NrmlOrientation","-o", (VoidPtr) &IGGlblFlipNormalOrient,	IC_BOOLEAN_TYPE },
  { "QuickLoad",      "-q", (VoidPtr) &IGGlblQuickLoad,		IC_BOOLEAN_TYPE },
  { "NumOfIsolines",  "-I", (VoidPtr) &IGGlblNumOfIsolines,	IC_INTEGER_TYPE },
  { "LineWidth",      "-l", (VoidPtr) &IGGlblLineWidth,		IC_INTEGER_TYPE },
  { "AdapIsoDir",     "",   (VoidPtr) &IGGlblAdapIsoDir,	IC_INTEGER_TYPE },
  { "PolygonOpti",    "-F", (VoidPtr) &IGGlblPolygonOptiApprox,	IC_INTEGER_TYPE },
  { "PolylineOpti",   "-f", (VoidPtr) &IGGlblPolylineOptiApprox,IC_INTEGER_TYPE },
  { "ShadingModel",   "-A", (VoidPtr) &IGGlblShadingModel,	IC_INTEGER_TYPE },
  { "TransMode",      "",   (VoidPtr) &IGGlblTransformMode,	IC_INTEGER_TYPE },
  { "ViewMode",	      "",   (VoidPtr) &IGGlblViewMode,		IC_INTEGER_TYPE },
  { "SketchSilStyle", "-k", (VoidPtr) &IGSketchParam.SketchSilType,IC_INTEGER_TYPE },
  { "SketchShdStyle", "-k", (VoidPtr) &IGSketchParam.SketchShdType,IC_INTEGER_TYPE },
  { "SketchImpStyle", "-k", (VoidPtr) &IGSketchParam.SketchImpType,IC_INTEGER_TYPE },
  { "SketchShdInv",   "-k", (VoidPtr) &IGSketchParam.SketchInvShd,IC_INTEGER_TYPE },
  { "SketchImp",      "-k", (VoidPtr) &IGSketchParam.SketchImp, IC_INTEGER_TYPE },
  { "PickObjType",    "-O", (VoidPtr) &IGGlblPickObjTypes,	IC_INTEGER_TYPE },
  { "InitWidget",     "-w", (VoidPtr) &IGGlblInitWidgetDisplay, IC_INTEGER_TYPE },
  { "ZClipMin",	      "-Z", (VoidPtr) &IGGlblZMinClip,		IC_REAL_TYPE },
  { "ZClipMax",	      "-Z", (VoidPtr) &IGGlblZMaxClip,		IC_REAL_TYPE },
  { "PlgnFineNess",   "-F", (VoidPtr) &IGGlblPlgnFineness,	IC_REAL_TYPE },
  { "PllnFineNess",   "-f", (VoidPtr) &IGGlblPllnFineness,	IC_REAL_TYPE },
  { "PointSize",      "-p", (VoidPtr) &IGGlblPointSize,		IC_REAL_TYPE },
  { "RelLowRes",      "-E", (VoidPtr) &IGGlblRelLowresFineNess,	IC_REAL_TYPE },
  { "PickEventDist",  "-e", (VoidPtr) &IGGlblMinPickDist,	IC_REAL_TYPE },
  { "NormalSize",     "-L", (VoidPtr) &IGGlblNormalSize,	IC_REAL_TYPE },
  { "SketchSilPwr",   "-k", (VoidPtr) &IGSketchParam.SilPower,  IC_REAL_TYPE },
  { "SketchShdPwr",   "-k", (VoidPtr) &IGSketchParam.ShadePower,IC_REAL_TYPE },
  { "SketchImpDecay", "-k", (VoidPtr) &IGSketchParam.SketchImpDecay,IC_REAL_TYPE }
};
#define NUM_SET_UP	(sizeof(SetUp) / sizeof(IritConfigStruct))

#ifdef NO_CONCAT_STR
IRIT_STATIC_DATA const char
    *GenVersionStr = "	 Version 11,	Gershon Elber,\n\
	(C) Copyright 1989-2012 Gershon Elber, Non commercial use only.";
#else
IRIT_STATIC_DATA const char
    *GenVersionStr = "	" IRIT_VERSION ",	Gershon Elber,	"
    __DATE__ ",   " __TIME__ "\n" IRIT_COPYRIGHT ", Non commercial use only.";
#endif /* NO_CONCAT_STR */

IRIT_STATIC_DATA const char
    *GenCtrlStr = " s%- u%- n%- N%- i%- c%- C%- m%- a%- q%- g%-\"x1,x2,y1,y2\"!s G%-\"x1,x2,y1,y2\"!s I%-#IsoLines!d F%-PlgnOpti|PlgnFineNess!d!F R%- f%-PllnOpti|PllnFineNess!d!F E%-RelLowRes!F p%-Point|Size!F l%-Line|Width!d r%- A%-Shader!d B%- 2%- d%- D%- L%-Normal|Size!F 4%- k%-SketchSilTyp|SilPwr|ShdTyp|ShdPwr|InvShd|ImpTyp|Imp!d!F!d!F!d!d!F K%- b%-\"R,B,G|(background)\"!s S%-\"x,y,z,w{,a,d,s}\"!s 1%- e%-PickDist!F O%-PickObjType!d Z%-ZMin|ZMax!F!F M%- W%-WireSetup!d P%- o%- x%-ExecAnimCmd!s X%-Min,Max,Dt,R{,flags}!s w%-InitWidget!d T%- z%- DFiles!*s";

static void IGHandleStateCommand(char *Str);
static void HandleAnimate(char *Params);
static int ReplyWithObject(IPObjectStruct *DisplayList, char *ObjName);
static int IGDeleteOneObjectAux(IPObjectStruct *PObj, IPObjectStruct *PLst);
static void UpdateAllActiveWidgets(IPObjectStruct *PObj, int Added);
static void UnhighObjects(void);
static void UnhighObjectsAux(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void IGFindObjectByNameAux(IPObjectStruct *PObj, IrtHmgnMatType Mat);
static void PickObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets up configuration of global variables.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PrgmName:    Name of program invoking this module.                       M
*   argc, argv:  Comand line.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGConfigureGlobals                                                       M
*****************************************************************************/
void IGConfigureGlobals(char *PrgmName, int argc, char **argv)
{
    int Error,
	NumFiles = 0,
	DrawSrfWireSetup = 0,
	DrawSrfWireFlag = FALSE,
	TransPosFlag = FALSE,
	ViewPosFlag = FALSE,
	IsoLinesFlag = FALSE,
	SrfFinenessFlag = FALSE,
	CrvFinenessFlag = FALSE,
	LineWidthFlag = FALSE,
	NormalSizeFlag = FALSE,
	BackGroundFlag = FALSE,
	LightSrcPosFlag = FALSE,
	MinPickDistFlag = FALSE,
	PickObjTypeFlag = FALSE,
	ZClipPlaneFlag = FALSE,
	PointSizeFlag = FALSE,
	ShadingModelFlag = FALSE,
	SketchStyleFlag = FALSE,
	ExecAnimEachStepFlag = FALSE,
	ExecAnimationFlag = FALSE,
	LowResFineNessFlag = FALSE,
	InitWidgetFlag = FALSE,
	AntiAliasingFlag = FALSE,
	DrawSolidFlag = FALSE,
	VerFlag = FALSE;
    char Line[IRIT_LINE_LEN_VLONG],
	**FileNames;
    float Ambient, Diffuse, Specular;
    IPObjectStruct *PObj;

#ifdef __WINCE__
    IritConfig(argv[0], SetUp, NUM_SET_UP);  /* Read config. file from path. */
#else
    IritConfig(PrgmName, SetUp, NUM_SET_UP); /* Read config. file if exists. */
#endif /* __WINCE__ */

    strcpy(Line, PrgmName);
    strcat(Line, GenCtrlStr);

    if ((Error = GAGetArgs(argc, argv, Line, &IGGlblStandAlone,
			   &IGGlblForceUnitMat, &IGGlblDrawVNormal,
			   &IGGlblDrawPNormal, &IGGlblDrawInternal,
			   &IGGlblDepthCue, &IGGlblCacheGeom, &IGGlblMore,
			   &IGGlblAntiAliasing, &IGGlblQuickLoad,
			   &TransPosFlag, &IGGlblTransPrefPos,
			   &ViewPosFlag, &IGGlblViewPrefPos,
			   &IsoLinesFlag, &IGGlblNumOfIsolines,
			   &SrfFinenessFlag, &IGGlblPolygonOptiApprox,
			   &IGGlblPlgnFineness, &IGGlblPolygonStrips,
			   &CrvFinenessFlag, &IGGlblPolylineOptiApprox,
			   &IGGlblPllnFineness, &LowResFineNessFlag,
			   &IGGlblRelLowresFineNess, &PointSizeFlag,
			   &IGGlblPointSize, &LineWidthFlag,
			   &IGGlblLineWidth, &DrawSolidFlag,
			   &ShadingModelFlag, &IGGlblShadingModel,
			   &IGGlblBackFaceCull, &IGGlblDoDoubleBuffer,
			   &IGGlblDebugObjectsFlag,
			   &IGGlblDebugEchoInputFlag, &NormalSizeFlag,
			   &IGGlblNormalSize, &IGGlblFourPerFlat,
			   &SketchStyleFlag, &IGSketchParam.SketchSilType,
			   &IGSketchParam.SilPower,
			   &IGSketchParam.SketchShdType, 
			   &IGSketchParam.ShadePower,
			   &IGSketchParam.SketchInvShd,
			   &IGSketchParam.SketchImpType,
			   &IGSketchParam.SketchImpDecay,
			   &IGGlblBackGroundImage,
			   &BackGroundFlag, &GlblBackGroundStr,
			   &LightSrcPosFlag, &GlblLightSrc1PosStr,
			   &IGGlblLightOneSide,
			   &MinPickDistFlag, &IGGlblMinPickDist,
			   &PickObjTypeFlag, &IGGlblPickObjTypes,
			   &ZClipPlaneFlag, &IGGlblZMinClip, &IGGlblZMaxClip,
			   &IGGlblDrawSurfaceMesh, &DrawSrfWireFlag,
			   &DrawSrfWireSetup, &IGGlblDrawSurfacePoly,
			   &IGGlblFlipNormalOrient,
			   &ExecAnimEachStepFlag, &GlblExecAnimEachStep,
			   &ExecAnimationFlag, &IGGlblExecAnimation,
			   &InitWidgetFlag, &IGGlblInitWidgetDisplay,
			   &IGGlblContinuousMotion,
			   &VerFlag, &NumFiles, &FileNames)) != 0) {
        GAStringErrMsg(Error, Line);
	sprintf(&Line[strlen(Line)], "\n");
	GAStringHowTo(GenCtrlStr, &Line[strlen(Line)]);
	IGIritError(Line);

	if (!IGGlblActiveXMode)
	    exit(1);
    }

#ifdef __WINNT__
    if (IGGlblMore)
	IGRedirectIOToConsole();
#endif /* __WINNT__ */

    if (IGGlblDrawStyle == 0)
	IGGlblDrawStyle = IG_STATE_DRAW_STYLE_WIREFRAME;
    else
        IGGlblDrawStyle = IG_STATE_DRAW_STYLE_SOLID;
    if (DrawSolidFlag)
	IGGlblDrawStyle = IG_STATE_DRAW_STYLE_SOLID;

    if (IGGlblAntiAliasing == 0)
	IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_OFF;
    else
        IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_ON;
    if (AntiAliasingFlag)
	IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_ON;

    if (sscanf(GlblBackGroundStr, "%d %d %d",
	       &IGGlblBackGroundColor[0],
	       &IGGlblBackGroundColor[1],
	       &IGGlblBackGroundColor[2]) != 3)
        sscanf(GlblBackGroundStr, "%d,%d,%d",
	       &IGGlblBackGroundColor[0],
	       &IGGlblBackGroundColor[1],
	       &IGGlblBackGroundColor[2]);

    if (sscanf(GlblHighlight1Str, "%d %d %d",
	       &IGGlblHighlight1Color[0],
	       &IGGlblHighlight1Color[1],
	       &IGGlblHighlight1Color[2]) != 3)
	sscanf(GlblHighlight1Str, "%d,%d,%d",
	       &IGGlblHighlight1Color[0],
	       &IGGlblHighlight1Color[1],
	       &IGGlblHighlight1Color[2]);

    if (sscanf(GlblHighlight2Str, "%d %d %d",
	       &IGGlblHighlight2Color[0],
	       &IGGlblHighlight2Color[1],
	       &IGGlblHighlight2Color[2]) != 3)
	sscanf(GlblHighlight2Str, "%d,%d,%d",
	       &IGGlblHighlight2Color[0],
	       &IGGlblHighlight2Color[1],
	       &IGGlblHighlight2Color[2]);

    if (sscanf(GlblLightSrc1PosStr, "%f %f %f %f %f %f %f",
	       &IGShadeParam.LightPos[0][0],
	       &IGShadeParam.LightPos[0][1],
	       &IGShadeParam.LightPos[0][2],
	       &IGShadeParam.LightPos[0][3],
	       &Ambient, &Diffuse, &Specular) == 7 ||
	sscanf(GlblLightSrc1PosStr, "%f,%f,%f,%f,%f,%f,%f",
	       &IGShadeParam.LightPos[0][0],
	       &IGShadeParam.LightPos[0][1],
	       &IGShadeParam.LightPos[0][2],
	       &IGShadeParam.LightPos[0][3],
	       &Ambient, &Diffuse, &Specular) == 7) {
	/* Need to update intensities as well. */
        IGShadeParam.LightAmbient[0] =
	    IGShadeParam.LightAmbient[1] =
		IGShadeParam.LightAmbient[2] = Ambient;
	IGShadeParam.LightDiffuse[0] =
		IGShadeParam.LightDiffuse[1] =
		    IGShadeParam.LightDiffuse[2] = Diffuse;
	IGShadeParam.LightSpecular[0] =
		IGShadeParam.LightSpecular[1] =
		    IGShadeParam.LightSpecular[2] = Specular;
    }
    else {
        if (sscanf(GlblLightSrc1PosStr, "%f %f %f %f",
		   &IGShadeParam.LightPos[0][0],
		   &IGShadeParam.LightPos[0][1],
		   &IGShadeParam.LightPos[0][2],
		   &IGShadeParam.LightPos[0][3]) != 4)
	  sscanf(GlblLightSrc1PosStr, "%f,%f,%f,%f",
		 &IGShadeParam.LightPos[0][0],
		 &IGShadeParam.LightPos[0][1],
		 &IGShadeParam.LightPos[0][2],
		 &IGShadeParam.LightPos[0][3]);
    }

    if (sscanf(GlblLightSrc2PosStr, "%f %f %f %f",
	       &IGShadeParam.LightPos[1][0],
	       &IGShadeParam.LightPos[1][1],
	       &IGShadeParam.LightPos[1][2],
	       &IGShadeParam.LightPos[1][3]) != 4)
        sscanf(GlblLightSrc2PosStr, "%f,%f,%f,%f",
	       &IGShadeParam.LightPos[1][0],
	       &IGShadeParam.LightPos[1][1],
	       &IGShadeParam.LightPos[1][2],
	       &IGShadeParam.LightPos[1][3]);

    if (VerFlag) {
	sprintf(Line, "%s%s\n\n", PrgmName, GenVersionStr);
	GAStringHowTo(GenCtrlStr, &Line[strlen(Line)]);
	IGIritError(Line);

	IritConfigPrint(SetUp, NUM_SET_UP);

	if (!IGGlblActiveXMode)
	    exit(0);
    }

    if (ShadingModelFlag &&
	(IGGlblShadingModel < IG_SHADING_NONE ||
	 IGGlblShadingModel > IG_SHADING_PHONG)) {
	sprintf(Line,
		"Shading Model between 0 (None) and 3 (Phong)\n\n%s%s\n\n",
		PrgmName, GenVersionStr);
	GAStringHowTo(GenCtrlStr, &Line[strlen(Line)]);
	IGIritError(Line);

	if (!IGGlblActiveXMode)
	    exit(0);
    }

    if (DrawSrfWireFlag) {
        IGGlblDrawSurfaceWire = DrawSrfWireSetup & 0x01;
        IGGlblDrawSurfaceBndry = DrawSrfWireSetup & 0x02;
        IGGlblDrawSurfaceSilh = DrawSrfWireSetup & 0x04;
        IGGlblDrawSurfaceSketch = DrawSrfWireSetup & 0x08;
        IGGlblDrawSurfaceRflctLns = DrawSrfWireSetup & 0x10;
    }

    IPSetFlattenObjects(FALSE);
    if (!IGGlblStandAlone) {
        if ((IGGlblIOHandle = IPSocClntInit()) < 0) {
	    sprintf(Line, "Failed to establish server connection\n");
	    IGIritError(Line);
	    if (!IGGlblActiveXMode)
	        exit(1);
	}

	if (IGGlblDebugEchoInputFlag)
	    IPSocEchoInput(IGGlblIOHandle, TRUE);
    }

    if (IGGlblAdapIsoDir == 1)
	IGGlblAdapIsoDir = CAGD_CONST_U_DIR;
    else if (IGGlblAdapIsoDir == 2)
	IGGlblAdapIsoDir = CAGD_CONST_V_DIR;

    /* Get all the geometry in. */
    if (NumFiles > 0) {
	GlblFirstLoadedFile = NumFiles > 0 ? IritStrdup(FileNames[0]) : "";

	if ((IGGlblDisplayList = IGLoadGeometry((const char **) FileNames,
						NumFiles)) == NULL) {
	    IGIritError("Failed to load the data file specified");
	    if (!IGGlblActiveXMode)
	        exit(1);
	}
    }

    /* If we have a direct curve/surface edit request - honor it. */
    for (PObj = IGGlblDisplayList; PObj != NULL; PObj = PObj -> Pnext) {
        if (IP_IS_CRV_OBJ(PObj) &&
	    AttrGetObjectIntAttrib(PObj, "EditCrvDirectly") == 1)
	IGCrvEditPreloadEditCurveObj = PObj;
	if (IP_IS_SRF_OBJ(PObj) &&
	    AttrGetObjectIntAttrib(PObj, "EditSrfDirectly") == 1)
	IGSrfEditPreloadEditSurfaceObj = PObj;
    }

    Cagd2PolyClipPolysAtPoles(TRUE);

    GMSphConeSetConeDensity(SPHERE_CONE_DENSITY);

    IRIT_PT_NORMALIZE_FLOAT(IGShadeParam.LightPos[0]);
    IRIT_PT_NORMALIZE_FLOAT(IGShadeParam.LightPos[1]);

    /* Make sure that if we ask to draw solid, depth cueing is disabled. */
    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
	IGGlblDepthCue = FALSE;

    if (!IPWasViewMat && !IGGlblQuickLoad) {
	if (IGGlblDisplayList != NULL) {
	    int BBoxPosWeights;
	    IrtRType s;

	    /* Precompute proper bounding box to begin transformation with. */
	    GMBBBboxStruct *BBox;
	    IrtVecType Center, Scaling;
	    IrtHmgnMatType Mat1, Mat2;


	    /* Ignore zero weights while deriving the bbox. */
	    BBoxPosWeights = CagdIgnoreNonPosWeightBBox(TRUE);
	    GMBBSetGlblBBObjList(IGGlblDisplayList);
	    BBox = GMBBComputeBboxObjectList(IGGlblDisplayList);
	    CagdIgnoreNonPosWeightBBox(BBoxPosWeights);

	    IRIT_PT_ADD(Center, BBox -> Max, BBox -> Min);
	    IRIT_PT_SCALE(Center, 0.5);
	    MatGenMatTrans(-Center[0], -Center[1], -Center[2], Mat1);

	    IRIT_PT_SUB(Scaling, BBox -> Max, BBox -> Min);
	    s = IRIT_MAX(Scaling[0], IRIT_MAX(Scaling[1], Scaling[2]));
	    if (s < IRIT_EPS)
	      s = IRIT_EPS;
	    MatGenMatUnifScale(1.0 / s, Mat2);

	    MatMultTwo4by4(Mat1, Mat1, Mat2);
	    MatMultTwo4by4(IPViewMat, Mat1, IPViewMat);
	}
    }

    if (IGGlblForceUnitMat) {
	MatGenUnitMat(IPViewMat);
	IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
    }

    IRIT_GEN_COPY(IGGlblPushViewMat, IPViewMat, sizeof(IrtHmgnMatType));
    IRIT_GEN_COPY(IGGlblPushPrspMat, IPPrspMat, sizeof(IrtHmgnMatType));

    switch (IGGlblViewMode) {        /* Update the current view. */
        case IG_VIEW_ORTHOGRAPHIC:
	    IRIT_GEN_COPY(IGGlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IGGlblCrntViewMat, IPViewMat, IPPrspMat);
	    break;
    }

    if (!IGGlblQuickLoad) {
	GMBBSetGlblBBObjList(IGGlblDisplayList);
	for (PObj = IGGlblDisplayList; PObj != NULL; PObj = PObj -> Pnext)
	    IGUpdateObjectBBox(PObj);
    }

    GMAnimResetAnimStruct(&IGAnimation);
    GMAnimFindAnimationTime(&IGAnimation, IGGlblDisplayList);
    if (!IRT_STR_NULL_ZERO_LEN(GlblExecAnimEachStep))
	IGAnimation.ExecEachStep = GlblExecAnimEachStep;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reset all static state variables of the display device into starting     M
* values.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGConfigReset                                                            M
*****************************************************************************/
void IGConfigReset(void)
{
    UpdateAllActiveWidgets(NULL, FALSE);

    /* Static variables in this module. */
    GlblDelayedClear = FALSE;
    GlblPickedPolyIsPolyline = FALSE;
    GlblPickedPolyIsPointList = FALSE;
    GlblProcessCommandMesssages = TRUE;
    GlblThisPickObjTypes = IG_PICK_ANY;
    GlblPickScale = 1.0;
    GlblMinFoundDepth = -IRIT_INFNTY;
    GlblBackGroundStr = "0, 0, 0";
    GlblExecAnimEachStep = "";
    GlblFirstLoadedFile = "";
    GlblHighlight1Str = "255, 0, 255";
    GlblHighlight2Str = "255, 100, 200";
    GlblLightSrc1PosStr = "1.0,  2.0,  5.0, 0.0";
    GlblLightSrc2PosStr = "1.0, -1.0, -3.0, 0.0";
    GlblObjectSearchName = "";
    GlblPickedPoly = NULL;
    GlblObjectFoundByName = NULL;

    /* Global variables in this module. */
    IGGlblQuickLoad = FALSE;
    IGGlbl3DGlassesImgIndx = -1;
    IGGlbl4Views = FALSE;
    IGGlblLightOneSide = TRUE;
    IGGlblActiveXMode = FALSE;
    IGGlblAnimation = FALSE;
    IGGlblAntiAliasing = 0;
    IGGlblBackGroundColor[0] =
        IGGlblBackGroundColor[1] =
            IGGlblBackGroundColor[2] = 0;
    IGGlblBackGroundImage = FALSE;
    IGGlblClientHandleNumber = -1;
    IGGlblContinuousMotion = FALSE;
    IGGlblCountFramePerSec = 0;
    IGGlblDebugEchoInputFlag = FALSE;
    IGGlblDebugObjectsFlag = FALSE;
    IGGlblDoDoubleBuffer = TRUE;
    IGGlblForceUnitMat = FALSE;
    IGGlblInitWidgetDisplay = 0;
    IGGlblIOHandle = -1;
    IGGlblLastWasSolidRendering = FALSE;
    IGGlblPickObjTypes = IG_PICK_ANY;
    IGGlblSavePickedPoly = FALSE;
    IGGlblShadingModel = IG_SHADING_PHONG;
    IGGlblStandAlone = TRUE;
    IGGlblTransformMode = IG_TRANS_SCREEN;
    IGGlblExecAnimation = "";
    IGGlblImageFileName = "";
    IGGlblPolyPickFileName = "";
    IGGlblTransPrefPos = "455, 640, 520, 965";
    IGGlblViewPrefPos =  "  1, 450, 520, 965";
    IGGlblChangeFactor = 1.0;
    IGGlblEyeDistance = 0.03;
    IGGlblFramePerSec = 0.0;
    IGGlblMinFoundDist = IRIT_INFNTY;
    IGGlblZMinClip = -2.0;
    IGGlblZMaxClip = 2.0;
    IGGlblDisplayList = NULL;
    IGGlblNewDisplayObjects = NULL;
    IGGlblPickedObj = NULL;
    IGGlblPickedPolyObj = NULL;

    /* Global variables in graph_gen.c module; grap_lib. */
    IGGlblAbortKeyPressed = FALSE;
    IGGlblAdapIsoDir = CAGD_CONST_U_DIR;
    IGGlblBackFaceCull = FALSE;
    IGGlblCacheGeom = TRUE;
    IGGlblCountNumPolys = FALSE;
    IGGlblDepthCue = TRUE;
    IGGlblDrawInternal = FALSE;
    IGGlblDrawPNormal = FALSE;
    IGGlblDrawPolygons = TRUE;
    IGGlblDrawSurfaceMesh = FALSE;
    IGGlblDrawSurfacePoly = FALSE;
    IGGlblDrawSurfaceWire = TRUE;
    IGGlblDrawSurfaceBndry = FALSE;
    IGGlblDrawSurfaceSilh = FALSE;
    IGGlblDrawSurfaceSketch = FALSE;
    IGGlblDrawSurfaceRflctLns = FALSE;
    IGGlblDrawStyle = 0;
    IGGlblDrawVNormal = FALSE;
    IGGlblFlipNormalOrient = FALSE;
    IGGlblPolygonStrips = FALSE;
    IGGlblFourPerFlat = TRUE;
    IGGlblHighlight1Color[0] = 255;
    IGGlblHighlight1Color[1] = 0;
    IGGlblHighlight1Color[2] = 255;
    IGGlblHighlight2Color[0] = 255;
    IGGlblHighlight2Color[1] = 0;
    IGGlblHighlight2Color[2] = 0;
    IGGlblIntensityHighState = TRUE,
    IGGlblLastLowResDraw = FALSE;
    IGGlblLineWidth = 1;
    IGGlblManipulationActive = FALSE;
    IGGlblMore = FALSE;
    IGGlblNumOfIsolines = IG_DEFAULT_NUM_OF_ISOLINES;
    IGGlblNumPolys = 0;
    IGGlblPolygonOptiApprox = TRUE;
    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
    IGGlblMinPickDist = 0.2;
    IGGlblNormalSize = 0.2;
    IGGlblPointSize = 0.2;
    IGGlblPlgnFineness = 10.0;
    IGGlblPllnFineness = IG_DEFAULT_PLLN_OPTI_FINENESS;
    IGGlblPointSize = 0.02;
    IGGlblRelLowresFineNess = 0.3;

    if (IGGlblPickedPolyObj != NULL) {
	IPFreeObject(IGGlblPickedPolyObj);
	IGGlblPickedPolyObj = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Are we suppose to handle command messages of the socket? A command message M
* will be a string objct sent over the socket with object name "COMMAND_".   M
*                                                                            *
* PARAMETERS:                                                                M
*   ProcessCommandMessages:   Sets the command message handling option.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGProcessCommandMessages                                                 M
*****************************************************************************/
void IGProcessCommandMessages(int ProcessCommandMessages)
{
    GlblProcessCommandMesssages = ProcessCommandMessages;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads an object from communication socket.				     M
*   Returns TRUE if screen needs to be redrawn in the case that the	     M
* DisplayList was modified.						     M
*   This function DOES NOT BLOCK if no input is unavailable.		     M
*   Handles commands via a string object with name "COMMAND_", if required   M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:      Either perspective or orthographics.                      M
*   DisplayList:   Global object display list. Will be updated in place.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           TRUE, if display needs to be refreshed.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGReadObjectsFromSocket                                                  M
*****************************************************************************/
int IGReadObjectsFromSocket(int ViewMode, IPObjectStruct **DisplayList)
{
    IPObjectStruct *PObjs;

    if ((PObjs = IPSocReadOneObject(IGGlblIOHandle)) != NULL) {
#if defined(OS2GCC)
	/* We are working here in a separated thread - keep it for a later */
	/* use by the main thread - when we do not use the display list.   */
        while (IGGlblNewDisplayObjects != NULL)
	    IritSleep(10);
	IritSleep(10);
	IGGlblNewDisplayObjects = PObjs;
	return FALSE;
#else
        return IGHandleObjectsFromSocket(ViewMode, PObjs, DisplayList);
#endif /* OS2GCC */
    }

    return FALSE;
}
    
/*****************************************************************************
* DESCRIPTION:                                                               M
* Reads an object from communication socket.				     M
*   Returns TRUE if screen needs to be redrawn in the case that the	     M
* DisplayList was modified.						     M
*   This function DOES NOT BLOCK if no input is unavailable.		     M
*   Handles commands via a string object with name "COMMAND_", if required   M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:      Either perspective or orthographics.                      M
*   PObjs:	   To insert into the display list.			     M
*   DisplayList:   Global object display list. Will be updated in place.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:           TRUE, if display needs to be refreshed.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleObjectsFromSocket                                                M
*****************************************************************************/
int IGHandleObjectsFromSocket(int ViewMode,
			      IPObjectStruct *PObjs,
			      IPObjectStruct **DisplayList)
{
    int Redraw = FALSE;
    char Line[IRIT_LINE_LEN_LONG];

    if (PObjs != NULL) {
	IPObjectStruct *PObj, *PTmp;

	Redraw = TRUE;

	if (GlblDelayedClear) {
	    GlblDelayedClear = FALSE;

	    IGStateHandler(IG_STATE_CLEAR_VIEW, IG_STATE_ON, TRUE);
	}

	while (PObjs != NULL) {
	    PObj = PObjs;
	    PObjs = PObjs -> Pnext;
	    PObj -> Pnext = NULL;

	    PObj -> Count = 0;     /* Can be much higher coming from socket. */

	    if (IGGlblDebugObjectsFlag)
		IPPutObjectToFile(stderr, PObj, FALSE);

	    if (GlblProcessCommandMesssages) {
		switch (PObj -> ObjType) {
		    case IP_OBJ_STRING:
		        if (stricmp(IP_GET_OBJ_NAME(PObj), "COMMAND_") == 0) {
			    char
				*Str = PObj -> U.Str;

			    if (strnicmp(Str, "ANIMATE", 7) == 0) {
			        HandleAnimate(&Str[7]);
				Redraw = TRUE;
			    }
			    else if (stricmp(Str, "BEEP") == 0) {
				IGIritBeep();
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "CLEAR") == 0) {
			        IGStateHandler(IG_STATE_CLEAR_VIEW,
					       IG_STATE_ON, TRUE);
			    }
			    else if (stricmp(Str, "DCLEAR") == 0) {
				GlblDelayedClear = TRUE;
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "DISCONNECT") == 0) {
				IPCloseStream(IGGlblIOHandle, TRUE);
				IGGlblStandAlone = TRUE;
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "EXIT") == 0) {
				IPCloseStream(IGGlblIOHandle, TRUE);
				if (!IGGlblActiveXMode)
				    exit(0);
			    }
			    else if (strnicmp(Str, "EDITCRV", 7) == 0) {
				if (strlen(&Str[7]) < 2)
				    PTmp = NULL;
				else
				    PTmp = IGFindObjectByName(&Str[8]);

				IGPopupCrvEditor(PTmp);
			    }
			    else if (strnicmp(Str, "EDITSRF", 7) == 0) {
				if (strlen(&Str[7]) < 2)
				    PTmp = NULL;
				else
				    PTmp = IGFindObjectByName(&Str[8]);

				IGPopupSrfEditor(PTmp);
			    }
			    else if (strnicmp(Str, "EDITOBJ", 7) == 0) {
				if (strlen(&Str[7]) < 2)
				    PTmp = NULL;
				else
				    PTmp = IGFindObjectByName(&Str[8]);

				IGPopupObjEditor(PTmp, FALSE);
			    }
			    else if (strnicmp(Str, "CLONEOBJ", 8) == 0) {
				if (strlen(&Str[8]) < 2)
				    PTmp = NULL;
				else
				    PTmp = IGFindObjectByName(&Str[9]);

				IGPopupObjEditor(PTmp, TRUE);
			    }
			    else if (strnicmp(Str, "GETOBJ", 6) == 0) {
				if (strlen(&Str[6]) < 2 ||
				    !ReplyWithObject(*DisplayList, &Str[7])) {
				    sprintf(Line,
					    "No such object \"%s\"\n",
					    &Str[6]);
				    IGIritError(Line);
				}
				Redraw = FALSE;
			    }
			    else if (strnicmp(Str, "HANDLENUM", 9) == 0) {
				if (strlen(&Str[9]) < 2 ||
				    sscanf(&Str[10], "%d",
					   &IGGlblClientHandleNumber) != 1) {
				    sprintf(Line,
					    "Expected client handle number\n");
				    IGIritError(Line);
				}
				Redraw = FALSE;
			    }
			    else if (strnicmp(Str, "HIGHLIGHT1", 10) == 0) {
				if (strlen(&Str[10]) > 1 &&
				    (PTmp = IGFindObjectByName(&Str[11])) != NULL)
				    IG_SET_HIGHLIGHT1_OBJ(PTmp);
			    }
			    else if (strnicmp(Str, "HIGHLIGHT2", 10) == 0) {
				if (strlen(&Str[10]) > 1 &&
				    (PTmp = IGFindObjectByName(&Str[11])) != NULL)
				    IG_SET_HIGHLIGHT2_OBJ(PTmp);
			    }
			    else if (strnicmp(Str, "IMGSAVE", 7) == 0) {
			        if (strlen(&Str[7]) > 1)
				    IGSaveDisplayAsImage(&Str[8]);
				Redraw = FALSE;
			    }
			    else if (strnicmp(Str, "MSAVE", 5) == 0) {
			        if (strlen(&Str[5]) > 1)
				    IGSaveCurrentMat(ViewMode, &Str[6]);
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "PICKCRSR") == 0) {
			        IGHandlePickObject(IG_PICK_ENTITY_CURSOR);
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "PICKDONE") == 0) {
			        IGHandlePickObject(IG_PICK_ENTITY_DONE);
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "PICKNAME") == 0) {
			        IGHandlePickObject(IG_PICK_ENTITY_OBJ_NAME);
				Redraw = FALSE;
			    }
			    else if (stricmp(Str, "PICKOBJ") == 0) {
			        IGHandlePickObject(IG_PICK_ENTITY_OBJECT);
				Redraw = FALSE;
			    }
			    else if (strnicmp(Str, "REMOVE", 6) == 0) {
			        if (strlen(&Str[6]) > 1)
				    IGAddReplaceObjDisplayList(DisplayList,
							       NULL,
							       &Str[7]);
				Redraw = TRUE;
			    }
			    else if (strnicmp(Str, "STATE", 5) == 0) {
			        IGHandleStateCommand(Str);
			    }
			    else if (stricmp(Str, "UNHIGHLIGHT") == 0) {
				UnhighObjects();
			    }
			}
			IPFreeObject(PObj);
			break;
		    case IP_OBJ_MATRIX:
			/* The parser will place "VIEW_MAT" and "PRSP_MAT"   */
			/* in IPViewMat and IPPrspMat, respectively,         */
			/* so we need do nothing here.		             */
			IPFreeObject(PObj);
			break;
		    default:
			IGAddReplaceObjDisplayList(DisplayList, PObj, NULL);
			break;
		}
	    }
	    else {
		IGAddReplaceObjDisplayList(DisplayList, PObj, NULL);
	    }
	}

	return Redraw;
    }
    else
	return Redraw;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process a command that is a view state command.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:   Command as "STATE StateName [Param]", where param can be one of   *
*	   0 for IG_STATE_FALSE, 1 for IG_STATE_TRUE, -1 for IG_STATE_TGL.   *
*          Default for no param is IG_STATE_TGL.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGHandleStateCommand(char *Str)
{
    int i;

    if (strlen(&Str[5]) > 1) {
        Str = &Str[6];

	for (i = 0;
	     StateNameNum[i].Name != NULL;
	     i++) {
	    int Len = (int) strlen(StateNameNum[i].Name);

	    if (strnicmp(Str, StateNameNum[i].Name, Len) == 0) {
	        int Param;

	        /* Skip the detected StateName and search for parameters. */
	        Str = &Str[Len];
		if (sscanf(Str, "%d", &Param) != 1)
		    Param = -1;

		switch (Param) {
		    case -1:
		        Param = IG_STATE_TGL;
			break;
		    case 0:
		        Param = IG_STATE_OFF;
			break;
		    case 1:
		        Param = IG_STATE_ON;
			break;
		}

	        IGHandleState(StateNameNum[i].Num, Param, TRUE);
		break;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Honors an animate request from the server.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Params:    A strings contains Tmin Tmax Dt, in this order.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void HandleAnimate(char *Params)
{
    double TMin, TMax, Dt;

    if (sscanf(Params, "%lf %lf %lf", &TMin, &TMax, &Dt) == 3) {
	IGAnimation.StartT = TMin;
	IGAnimation.FinalT = TMax;
	IGAnimation.Dt = Dt;
	GMAnimDoAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	char Line[IRIT_LINE_LEN_LONG];

	sprintf(Line,
		"Animate param, expected \"Tmin Tmax Dt\", found \"%s\"\n",
		Params);
	IGIritError(Line);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Adds or replaces an object on the display list.			     M
*   If ObjName is not NULL (and NewObj is NULL), that object is removed from M
* the display list.							     M
*   Otherwise NewObj is added to the display list, possibly replacing an     M
* object on the display list with the same name.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   DisplayList:   Global display list to update.                            M
*   NewObj:        New object to add to the global display list, or NULL.    M
*   ObjName:       Name of object to remove from display list, or NULL.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGAddReplaceObjDisplayList                                               M
*****************************************************************************/
void IGAddReplaceObjDisplayList(IPObjectStruct **DisplayList,
				IPObjectStruct *NewObj,
				char *ObjName)
{
    int Remove = ObjName != NULL;
    char
	*NewObjName = NewObj != NULL ? IP_GET_OBJ_NAME(NewObj) : "none",
	*Name = ObjName != NULL ? ObjName : NewObjName;
    IPObjectStruct *PObj;

    if (NewObj != NULL) {
	IGConfirmConvexPolys(NewObj, 1);
	IGUpdateObjectBBox(NewObj);
    }

    /* If object has no name or display list is empty, add it. */
    if (!Remove &&
	NewObjName != NULL &&
	(NewObjName[0] == 0 ||
	 NewObjName[0] == '_' ||
	 stricmp(NewObjName, "none") == 0)) {
	NewObj -> Pnext = *DisplayList;
	*DisplayList = NewObj;
	UpdateAllActiveWidgets(NewObj, TRUE);
	return;
    }
    if (*DisplayList == NULL) {
	if (NewObj != NULL) {
	    *DisplayList = NewObj;
	    UpdateAllActiveWidgets(NewObj, TRUE);
	}
	return;
    }

    if (stricmp(Name, IP_GET_OBJ_NAME(*DisplayList)) == 0) {
	if (Remove) {
	    PObj = *DisplayList;
	    *DisplayList = (*DisplayList) -> Pnext;
	    IPFreeObject(PObj);
	}
	else {
	    NewObj -> Pnext = (*DisplayList) -> Pnext;
	    IPFreeObject(*DisplayList);
	    *DisplayList = NewObj;
	}
	UpdateAllActiveWidgets(*DisplayList, FALSE);
    }
    else {
	for (PObj = *DisplayList; PObj -> Pnext != NULL; PObj = PObj -> Pnext) {
	    if (stricmp(Name, IP_GET_OBJ_NAME(PObj -> Pnext)) == 0) {
		IPObjectStruct
		    *PObjTmp = PObj -> Pnext;

		if (Remove) {
		    PObj -> Pnext = PObjTmp -> Pnext;
		}
		else {
		    PObj -> Pnext = NewObj;
		    NewObj -> Pnext = PObjTmp -> Pnext;
		}

		IPFreeObject(PObjTmp);
		UpdateAllActiveWidgets(PObjTmp, FALSE);
		return;
	    }
	}

	/* Name was not found. */
	if (!Remove) {
	    NewObj -> Pnext = *DisplayList;
	    *DisplayList = NewObj;

	    UpdateAllActiveWidgets(NewObj, TRUE);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns into the output channel the object requested via ObjName.        *
*                                                                            *
* PARAMETERS:                                                                *
*   DisplayList:   List of all current objects.                              *
*   ObjName:       Name of object to search.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	   TRUE if succesful, FALSE otherwise.                       *
*                                                                            *
* KEYWORDS:                                                                  *
*   ReplayWithObject                                                         *
*****************************************************************************/
static int ReplyWithObject(IPObjectStruct *DisplayList, char *ObjName)
{
    IPObjectStruct *PObj;

    for (PObj = DisplayList; PObj != NULL; PObj = PObj -> Pnext) {
	if (stricmp(ObjName, IP_GET_OBJ_NAME(PObj)) == 0) {
	    IPSocWriteOneObject(IGGlblIOHandle, PObj);
	    return TRUE;
	}
    }

    /* Dumps this string object instead, so we will not block the server. */
    PObj = IPGenStrObject("_PickFail_", "*** no object ***", NULL);
    IPSocWriteOneObject(IGGlblIOHandle, PObj);
    IPFreeObject(PObj);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Saves the current viewing matrices.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:     Either perspective or orthographic.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGSaveCurrentMat		                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSubmitCurrentMat                                                       M
*****************************************************************************/
void IGSubmitCurrentMat(int ViewMode)
{
    IPObjectStruct *PObj;

    if (IGGlblStandAlone) {
        IGIritError("No submissions in stand alone mode");
	return;
    }

    PObj = IPGenMATObject(IPViewMat);
    IP_SET_OBJ_NAME2(PObj, "_SubmitMat_");
    AttrSetObjectStrAttrib(PObj, "ObjName", "view_mat_dd");
    IPSocWriteOneObject(IGGlblIOHandle, PObj);
    IPFreeObject(PObj);

    if (ViewMode == IG_VIEW_PERSPECTIVE) {
        PObj = IPGenMATObject(IPPrspMat);
	IP_SET_OBJ_NAME2(PObj, "_SubmitMat_");
	AttrSetObjectStrAttrib(PObj, "ObjName", "prsp_mat_dd");
	IPSocWriteOneObject(IGGlblIOHandle, PObj);
	IPFreeObject(PObj);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Saves the picked polygons.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:          Pick poly.                                                  M
*   IsPolyline:  TRUE if picked poly a polyline, FALSE if a polygon.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHighlightSavePickedPoly                                                M
*****************************************************************************/
void IGHighlightSavePickedPoly(IPPolygonStruct *Pl,
			       int IsPolyline,
			       int IsPointList)
{
    static int
	PickFileIndex = 1;
    static char
	*PickFileName = NULL;
    char Name[IRIT_LINE_LEN_LONG];
    FILE *f;

    if (Pl == NULL)
        return;

    if (IGGlblPickedPolyObj != NULL)
	IPFreeObject(IGGlblPickedPolyObj);
    if (IsPointList) {
        IGGlblPickedPolyObj = IPGenPOINTLISTObject(IPCopyPolygon(Pl));
    }
    else {
        IGGlblPickedPolyObj =
		    IsPolyline ? IPGenPOLYLINEObject(IPCopyPolygon(Pl))
      			       : IPGenPOLYObject(IPCopyPolygon(Pl));
    }

    IG_SET_HIGHLIGHT2_OBJ(IGGlblPickedPolyObj);

    if (!IGGlblSavePickedPoly)
        return;

    if (IGGlblPolyPickFileName != NULL) {
        /* We have a new prescription of file name - reset the stage. */
        if (PickFileName != NULL)
	    IritFree(PickFileName);
        PickFileName = IGGlblPolyPickFileName;
	IGGlblPolyPickFileName = NULL;
	PickFileIndex = 1;
    }

    if (IRT_STR_NULL_ZERO_LEN(PickFileName))
        return;

    sprintf(Name, PickFileName, PickFileIndex++);

#if defined(AMIGA) && !defined(__SASC)
    if (IRT_STR_ZERO_LEN(Name) || (f = fopen(Name, "w")) == NULL) {
#else
    if (IRT_STR_ZERO_LEN(Name) || (f = fopen(Name, "wt")) == NULL) {
#endif /* defined(AMIGA) && !defined(__SASC) */
	IGIritBeep();
	return;
    }

    IPPutObjectToFile(f, IGGlblPickedPolyObj, FALSE);

    fclose(f);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   handles continous motion mode where the object is continuously rotating  M
* on the screen.  Updates the current view matrix based on the last	     M
* transformation applied and redraw.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGProcessEvent                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleContinuousMotion                                                 M
*****************************************************************************/
void IGHandleContinuousMotion(void)
{
    switch (IGGlblTransformMode) {      /* Udpate the global viewing matrix. */
        case IG_TRANS_SCREEN:
            MatMultTwo4by4(IPViewMat, IPViewMat, IGGlblLastProcessMat);
	    break;
	case IG_TRANS_OBJECT:
	    MatMultTwo4by4(IPViewMat, IGGlblLastProcessMat, IPViewMat);
	    break;
    }

    IGRedrawViewWindow();

    IritSleep(10);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Delete one object PObj from the global display list.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object to delete.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if found and deleted, FALSE otherwise.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDeleteOneObject                                                        M
*****************************************************************************/
int IGDeleteOneObject(IPObjectStruct *PObj)
{
    IPObjectStruct
	*PTmp = IGGlblDisplayList,
	*PPrev = NULL;

    for ( ; PTmp != NULL; PPrev = PTmp, PTmp = PTmp -> Pnext) {
        if (PObj == PTmp) {
	    if (PPrev == NULL) {
	        /* It is the first object in the list. */
	        IGGlblDisplayList = IGGlblDisplayList -> Pnext;
	    }
	    else {
	        PPrev -> Pnext = PTmp -> Pnext;
	    }

	    UpdateAllActiveWidgets(PTmp, FALSE);
	    GMBBSetGlblBBObjList(IGGlblDisplayList);

	    IPFreeObject(PTmp);
	    return TRUE;
	}
	else if (IGDeleteOneObjectAux(PObj, PTmp)) {
	    if (IP_IS_OLST_OBJ(PTmp) && IPListObjectLength(PTmp) == 0)
	        IGDeleteOneObject(PTmp);
	    return TRUE;
	}
    }

    return FALSE;					      /* Not found. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function to delete object PObj from the global display list.   *
*****************************************************************************/
static int IGDeleteOneObjectAux(IPObjectStruct *PObj, IPObjectStruct *PLst)
{
    if (IP_IS_OLST_OBJ(PLst)) {
	IPObjectStruct *PTmp;
	int i = 0;

	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PLst, i)) != NULL) {
	    if (PTmp == PObj) {
	        IPListObjectDelete(PLst, i, TRUE);

		if (IPListObjectLength(PLst) == 0)
		    IGDeleteOneObject(PLst);

		UpdateAllActiveWidgets(PTmp, FALSE);

		return TRUE;
	    }
	    else if (IP_IS_OLST_OBJ(PTmp) && IGDeleteOneObjectAux(PObj, PTmp))
	        return TRUE;
	    i++;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Every time the global display list is modified async. (via an input      *
* stream message), we must deactivate all active widgets.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:  Object that is to be added/removed, or NULL for complete cleanup. *
*   Added: If TRUE object has been added, FALSE for removal.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateAllActiveWidgets(IPObjectStruct *PObj, int Added)
{
    if (PObj == NULL || (!Added && PObj == IGCrvEditCurrentObj))
	CEditDetachCurve();

    if (PObj == NULL || (!Added && PObj == IGSrfEditCurrentObj))
	SEditDetachSurface();

    if (IGObjManipNumActiveObjs > 0) {
	int i;

	if (PObj == NULL)
	    IGObjManipDetachObj();
	else {
	    for (i = 0; i < IGObjManipNumActiveObjs; i++) {
		if (PObj == IGObjManipCurrentObjs[i]) {
		    IGObjManipDetachObj();
		    break;
		}
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Traverses the object's hierarchy for display purposes.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjList:     To traverse and apply.                                     M
*   CrntViewMat:  Viewing matrix.                                            M
*   ApplyFunc:    To invoke on each and every leaf object.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPTraverseObjListHierarchy                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGTraverseObjListHierarchy                                               M
*****************************************************************************/
void IGTraverseObjListHierarchy(IPObjectStruct *PObjList,
				IrtHmgnMatType CrntViewMat,
				IPApplyObjFuncType ApplyFunc)
{
    IPTraverseObjListHierarchy(PObjList, CrntViewMat, ApplyFunc);

    if (IGCrvEditActive)
	CEditRedrawCrv();

    if (IGSrfEditActive)
	SEditRedrawSrf();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handles a predefined animation of the '-X' flag.  Will activate the      M
* animation only on the first time it is called..			     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPredefinedAnimation                                                    M
*****************************************************************************/
void IGPredefinedAnimation(void)
{
    if (!IRT_STR_NULL_ZERO_LEN(IGGlblExecAnimation)) {
	int i, j;
	float Min, Max, Dt, NumRep;
	char 
	    *Flags = NULL,
	    *Params = IGGlblExecAnimation;

	IGGlblExecAnimation = NULL;

	IGAnimation.SaveAnimationGeom =
	    IGAnimation.SaveAnimationImage =
	        IGAnimation.BackToOrigin =
		    IGAnimation.TwoWaysAnimation = FALSE;

	for (i = j = 0; i < (int) strlen(Params); i++) {
	    if (Params[i] == ',')
	        j++;
	    if (j == 4) {
	        /* We have optional flags. */
	        Params[i] = 0;
		Flags = &Params[i + 1];
		break;
	    }
	}

	if (sscanf(Params, "%f,%f,%f,%f",
		   &Min, &Max, &Dt, &NumRep) == 4) {
	    IGAnimation.StartT = Min;
	    IGAnimation.FinalT = Max;
	    IGAnimation.Dt = Dt;
	    IGAnimation.NumOfRepeat = (int) NumRep;
	    if (Flags != NULL) {
	        if (strchr(Flags, 's') != NULL || strchr(Flags, 'S') != NULL)
		    IGAnimation.SaveAnimationGeom = TRUE;
		if (strchr(Flags, 't') != NULL || strchr(Flags, 'T') != NULL)
		    IGAnimation.TwoWaysAnimation = TRUE;
		if (strchr(Flags, 'b') != NULL || strchr(Flags, 'B') != NULL)
		    IGAnimation.BackToOrigin = TRUE;
	    }

	    GMAnimDoAnimation(&IGAnimation, IGGlblDisplayList);

	    if (Flags != NULL &&
		(strchr(Flags, 'x') != NULL || strchr(Flags, 'X') != NULL) &&
		!IGGlblActiveXMode)
	        exit(0);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Search for an object named Name in the global display list and return    M
* it if found.                                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Name:      of object to look for,                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Found object, NULL of none.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGFindObjectByName                                                       M
*****************************************************************************/
IPObjectStruct *IGFindObjectByName(const char *Name)
{
    IrtHmgnMatType Mat;

    MatGenUnitMat(Mat);

    GlblObjectFoundByName = NULL;
    GlblObjectSearchName = Name;

    IPTraverseObjListHierarchy(IGGlblDisplayList, Mat, IGFindObjectByNameAux);

    return GlblObjectFoundByName;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of the IGFindObjectByName above.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to search its name.                                    *
*   Mat:       Viewing matrix of object.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGFindObjectByNameAux(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    if (stricmp(IP_GET_OBJ_NAME(PObj), GlblObjectSearchName) == 0)
	GlblObjectFoundByName = PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Unhighlight all objects currently displayed.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UnhighObjects(void)
{
    IrtHmgnMatType Mat;

    MatGenUnitMat(Mat);

    IPTraverseObjListHierarchy(IGGlblDisplayList, Mat, UnhighObjectsAux);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of the UnhighObjects above.		             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to unhighlight.                                        *
*   Mat:       Viewing matrix of object.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UnhighObjectsAux(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IG_RST_HIGHLIGHT1_OBJ(PObj);
    IG_RST_HIGHLIGHT2_OBJ(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Release the currently picked object if has one.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleGenericPickEvent                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGReleasePickedObject                                                    M
*****************************************************************************/
void IGReleasePickedObject(void)
{
    if (IGGlblPickedObj)
        IG_RST_HIGHLIGHT1_OBJ(IGGlblPickedObj);
    if (IGGlblPickedPolyObj) {
	IPFreeObject(IGGlblPickedPolyObj);
	IGGlblPickedPolyObj = NULL;
    }

    GlblThisPickObjTypes = IG_PICK_ANY;
    IGGlblPickedObj = NULL;
    GlblPickedPoly = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Processes the given event. Returns TRUE if redraw of view window is needed.M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:          Event to process.                                        M
*   ChangeFactor:   A continuous scale between -1 and 1 to quantify the      M
*                   change to apply according to the event type.	     M
*		    For composed operation contains both X and Y information.M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            TRUE if refresh is needed.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleContinuousMotion                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGProcessEvent                                                           M
*****************************************************************************/
int IGProcessEvent(IGGraphicEventType Event, IrtRType *ChangeFactor)
{
    int UpdateView = TRUE;

    switch (Event) {
	case IG_EVENT_SAVE_MATRIX:
	    IGSaveCurrentMatInFile(IGGlblViewMode);
	    UpdateView = FALSE;
	    break;
	case IG_EVENT_SUBMIT_MATRIX:
	    IGSubmitCurrentMat(IGGlblViewMode);
	    UpdateView = FALSE;
	    break;
	case IG_EVENT_STATE:
	    IGCreateStateMenu();
	    break;
	default:
	    UpdateView = IGDefaultProcessEvent(Event, ChangeFactor);
    }

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Handle the state events.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       State event type to handle.                                 M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*		 on or toggling current value. 				     M
*		 IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Not used.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if needs to refresh.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGStateHandler                                                           M
*****************************************************************************/
int IGStateHandler(int State, int StateStatus, int Refresh)
{
    int UpdateView = TRUE;
    IrtHmgnMatType Mat;
    char Line[IRIT_LINE_LEN_LONG];

    switch (State) {
	case IG_STATE_CACHE_GEOM:
	    sprintf(Line, "You cannot change the geometry caching now\n");
	    IGIritError(Line);
	    break;
	case IG_STATE_LIGHT_ONE_SIDE:
	    IGGlblLightOneSide = !IGGlblLightOneSide;
	    break;
	case IG_STATE_VIEW_FRONT:
	    IGSetDisplay4Views(FALSE);
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    MatGenMatRotZ1(IRIT_DEG2RAD(0.0), Mat);
	    IGUpdateViewConsideringScale(Mat);
	    break;
	case IG_STATE_VIEW_SIDE:
	    IGSetDisplay4Views(FALSE);
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    MatGenMatRotY1(IRIT_DEG2RAD(90.0), Mat);
	    IGUpdateViewConsideringScale(Mat);
	    break;
	case IG_STATE_VIEW_TOP:
	    IGSetDisplay4Views(FALSE);
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    MatGenMatRotX1(IRIT_DEG2RAD(90.0), Mat);
	    IGUpdateViewConsideringScale(Mat);
	    break;
	case IG_STATE_VIEW_ISOMETRY:
	    IGSetDisplay4Views(FALSE);
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    IGUpdateViewConsideringScale(IGGlblIsometryViewMat);
	    break;
	case IG_STATE_VIEW_4:
	    IGSetDisplay4Views(TRUE);
	    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
	    IGInitializeSubViewMat();
	    break;
	case IG_STATE_CLEAR_VIEW:
	    IPFreeObjectList(IGGlblDisplayList);
	    IGGlblDisplayList = NULL;
	    IGGlblPickedObj = NULL;
	    if (IGGlblPickedPolyObj != NULL) {
	        IPFreeObject(IGGlblPickedPolyObj);
		IGGlblPickedPolyObj = NULL;
	    }
	    UpdateAllActiveWidgets(NULL, FALSE);
	    IrtImgReadClrCache();             /* Free all used texture maps. */
	    break;
	default:
	    UpdateView = IGDefaultStateHandler(State, StateStatus, Refresh);
    }

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports to the server on a pick event of the cursor/mouse.               M
* The reported object is a list object of a point and a vector defining the  M
* cursor line in 3-space.  The event type is returned as an "EventType"      M
* attribute on the reported object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjectX, ObjectY:    Location of the cursor, in object space.            M
*   PickReport:          Type of event: motion, button down, etc.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleGenericPickEvent			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleGenericCursorEvent                                               M
*****************************************************************************/
void IGHandleGenericCursorEvent(IrtRType ObjectX,
				IrtRType ObjectY,
				IGPickReportType PickReport)
{
    IRIT_STATIC_DATA IrtRType
	LastX = IRIT_INFNTY,
	LastY = IRIT_INFNTY;
    IrtVecType V;
    IrtPtType Pos, Pt1, Pt2;
    IPObjectStruct *PObj;

#   ifdef __WINNT__
    {
        static int LastCursorEventTime = 0;
	if (GetTickCount() - LastCursorEventTime < 100)
	    return;		    /* No more than 10 events per second. */
	LastCursorEventTime = GetTickCount();
    }
#   endif /* __WINNT__ */

    if (PickReport == IG_PICK_REP_MOTION &&
	IRIT_APX_EQ(LastX, ObjectX) &&
	IRIT_APX_EQ(LastY, ObjectY))
        return;

    LastX = ObjectX;
    LastY = ObjectY;

    /* Converts picked location to a line defined via a point and vector. */
    Pos[0] = ObjectX;
    Pos[1] = ObjectY;
    Pos[2] = 0.0;
    MatMultPtby4by4(Pt1, Pos, IGGlblInvCrntViewMat);
    Pos[2] = 1.0;
    MatMultPtby4by4(Pt2, Pos, IGGlblInvCrntViewMat);
    IRIT_PT_SUB(V, Pt2, Pt1);

    /* Picked location is now defined in Pt1 (pt on line) and V (dir). */
    PObj = IPAllocObject("_PickCrsr_", IP_OBJ_LIST_OBJ, NULL);
    IPListObjectInsert(PObj, 0, IPGenPTObject(&Pt1[0], &Pt1[1], &Pt1[2]));
    IPListObjectInsert(PObj, 1, IPGenVECObject(&V[0], &V[1], &V[2]));
    IPListObjectInsert(PObj, 2, NULL);
    AttrSetObjectIntAttrib(PObj, "EventType", PickReport);
	
    IPSocWriteOneObject(IGGlblIOHandle, PObj);

    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle pick events.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   ObjectX, ObjectY: Objects XY coordinates into the screen of pick event.  M
*   PickTypes:	      Types of object to pick or IG_PICK_ANY for any object. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Picked object or NULL if none.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGPickReportCursor, IGReleasePickedObject                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleGenericPickEvent                                                 M
*****************************************************************************/
IPObjectStruct *IGHandleGenericPickEvent(IrtRType ObjectX,
					 IrtRType ObjectY,
					 int PickTypes)
{
    IGGlblPickPos[0] = ObjectX;
    IGGlblPickPos[1] = ObjectY;
    IGGlblPickPos[2] = 0.0;

    IGGlblMinFoundDist = IGGlblMinPickDist;
    GlblMinFoundDepth = -IRIT_INFNTY;
    if (IGGlblPickedObj)
        IG_RST_HIGHLIGHT1_OBJ(IGGlblPickedObj);
    if (IGGlblPickedPolyObj) {
	IPFreeObject(IGGlblPickedPolyObj);
	IGGlblPickedPolyObj = NULL;
    }

    GlblThisPickObjTypes = PickTypes;
    IGGlblPickedObj = NULL;
    GlblPickedPoly = NULL;
    GlblPickScale = MatScaleFactorMatrix(IGGlblCrntViewMat);
    IGTraverseObjListHierarchy(IGGlblDisplayList, IGGlblCrntViewMat,
			       PickObject);
    if (GlblPickedPoly != NULL)
        IGHighlightSavePickedPoly(GlblPickedPoly, GlblPickedPolyIsPolyline,
				  GlblPickedPolyIsPointList);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPickEvent, FALSE) {
	    char Line[IRIT_LINE_LEN_LONG];
	
	    sprintf(Line,
		    "Pick event attempted at %f %f, Found Object \"%s\" (Dist %f)\n",
		    ObjectX, ObjectY,
		    IGGlblPickedObj ? IP_GET_OBJ_NAME(IGGlblPickedObj) : "NULL",
		    IGGlblMinFoundDist);
	    IGIritError(Line);
	}
    }
#endif /* DEBUG */

    if (IGGlblPickedObj) {
        IG_SET_HIGHLIGHT1_OBJ(IGGlblPickedObj);
	IGRedrawViewWindow();
    }

    return IGGlblPickedObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the object types that can be picked.  The object types are or'ed    M
* together to form the mask of pickable objects.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PickObjTypes:    Object types that can be picked.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Old mask of possible pockable objects.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleGenericPickEvent                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetPickObjectType                                                      M
*****************************************************************************/
int IGSetPickObjectType(int PickObjTypes)
{
    int OldPickObjTypes = PickObjTypes;

    IGGlblPickObjTypes = PickObjTypes;

    return OldPickObjTypes;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of the IGHandleGenericPickEvent above.                *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      Object to scan for picking.                                   *
*   Mat:       Viewing matrix of object.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PickObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    int MinPlIsPolyline;
    IrtRType MinDist, HitDepth;
    IrtHmgnMatType InvMat;
    IrtVecType RayDir, Dir;
    IrtPtType Pt1, Pt2, MinPt;
    IPPolygonStruct *MinPl;

    /* Check if object is pickable as set by the "pickable object" widget. */
    if (((1 << PObj -> ObjType) & IGGlblPickObjTypes) == 0)
	return;

    /* Check if this object is pickable by this specific pick request. */
    if (((1 << PObj -> ObjType) & GlblThisPickObjTypes) == 0)
	return;

    if (!MatInverseMatrix(Mat, InvMat))
	return;

    IGGlblPickPos[2] = 0.0;
    MatMultPtby4by4(Pt1, IGGlblPickPos, InvMat);
    IGGlblPickPos[2] = 1.0;
    MatMultPtby4by4(Pt2, IGGlblPickPos, InvMat);
    IRIT_PT_SUB(RayDir, Pt2, Pt1);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDrawPickEvent, FALSE) {
	    IPVertexStruct
	        *V2 = IPAllocVertex2(NULL),
	        *V1 = IPAllocVertex2(V2);
	    IPPolygonStruct
	        *Pl = IPAllocPolygon(0, V1, NULL);
	    IPObjectStruct
	        *PObj = IPGenPOLYObject(Pl);

	    IRIT_PT_COPY(V1 -> Coord, Pt1);
	    IRIT_PT_COPY(V2 -> Coord, Pt2);

	    IP_SET_POLYLINE_OBJ(PObj);

	    IRIT_LIST_PUSH(PObj, IGGlblDisplayList);
	}
    }
#endif /* DEBUG */

    /* Take care of special type objects. */
    if (IP_IS_STR_OBJ(PObj) &&
        (PObj = AttrGetObjectObjAttrib(PObj, "_geometry")) == NULL)
	    return;

    /* Find out the closest object in the display list to the picked line. */
    MinDist = GlblPickScale * IGFindMinimalDist(PObj, &MinPl, MinPt,
						&MinPlIsPolyline,
						Pt1, RayDir, &HitDepth);
    IRIT_VEC_COPY(Dir, RayDir);
    IRIT_VEC_SCALE(Dir, HitDepth);
    IRIT_PT_ADD(Pt1, Pt1, Dir);
    MatMultPtby4by4(Pt2, Pt1, Mat);
    if (IGGlblMinFoundDist > MinDist ||
	(IRIT_APX_EQ(IGGlblMinFoundDist, MinDist) && GlblMinFoundDepth < Pt2[2])) {
	IGGlblPickedObj = PObj;
	GlblPickedPoly = MinPl;
	GlblPickedPolyIsPolyline = MinPlIsPolyline;
	GlblPickedPolyIsPointList = IP_IS_POINTLIST_OBJ(PObj);
	IGGlblMinFoundDist = MinDist;
	GlblMinFoundDepth = HitDepth;

	IRIT_PT_COPY(IGGlblPickPosE3, MinPt);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Synthesize the string to present as the title of the window.             M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   A string to embed in the header.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:  Created window header.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGStrResGenericPickEvent                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGGenerateWindowHeaderString                                             M
*****************************************************************************/
char *IGGenerateWindowHeaderString(char *Msg)
{
    static char Str[IRIT_LINE_LEN_LONG];

    if (Msg == NULL)
	Msg = "";

    if (GlblFirstLoadedFile != NULL)
	sprintf(Str, "Irit Display (%s)%s", GlblFirstLoadedFile, Msg);
    else
        sprintf(Str, "Irit Display%s", Msg);

    return Str;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a string that represents the result of the last pick operation.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Picked object.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   char *:   The string result.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleGenericPickEvent, IGGenerateWindowHeaderString                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGStrResGenericPickEvent                                                 M
*****************************************************************************/
char *IGStrResGenericPickEvent(IPObjectStruct *PObj)
{
    static char Str[IRIT_LINE_LEN_LONG];

    if (IRIT_PT_EQ_ZERO(IGGlblPickPosE3) || PObj == NULL)
        sprintf(Str, " :: Picked \"%s\"",
		PObj == NULL ? "nothing" : IP_GET_OBJ_NAME(PObj));
    else
        sprintf(Str, " :: Picked \"%s\" [%.4lg, %.4lg, %.4lg]",
		IP_GET_OBJ_NAME(PObj),
		IGGlblPickPosE3[0], IGGlblPickPosE3[1], IGGlblPickPosE3[2]);

    return Str;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts screen coordinates (from a mouse, for example) to object space. M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY:   Screen space coordinates.                            M
*   Pt:                 Object space coordinates - origin of ray.            M
*   Dir:                Object space coordinates - direction of ray.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGGenericScreenToObject                                                  M
*****************************************************************************/
void IGGenericScreenToObject(IrtRType ScreenX,
			     IrtRType ScreenY,
			     IrtPtType Pt,
			     IrtVecType Dir)
{
    IrtPtType Pt1;

    Pt[0] = ScreenX;
    Pt[1] = ScreenY;
    Pt[2] = 0.0;
    MatMultPtby4by4(Pt1, Pt, IGGlblInvCrntViewMat);
    Pt[2] = 1.0;
    MatMultPtby4by4(Pt, Pt, IGGlblInvCrntViewMat);
    IRIT_PT_SUB(Dir, Pt1, Pt);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Geom_lib errors right here. Call back function of geom_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in geom_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   GeomFatalError                                                           M
*****************************************************************************/
void GeomFatalError(GeomFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = GeomDescribeError(ErrID);

    sprintf(Line, "GEOM_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Misc_lib errors right here. Call back function of misc_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in misc_lib library.                              M
*   ErrDesc:  Possibly, an additional description on error.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   MiscFatalError                                                           M
*****************************************************************************/
void MiscFatalError(MiscFatalErrorType ErrID, char *ErrDesc)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = MiscDescribeError(ErrID);

    sprintf(Line, "MISC_LIB fatal error: %s%s", ErrorMsg, ErrDesc);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Cagd_lib errors right here. Call back function of cagd_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in cagd_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdFatalError                                                           M
*****************************************************************************/
void CagdFatalError(CagdFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = CagdDescribeError(ErrID);

    sprintf(Line, "CAGD_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Symb_lib errors right here. Call back function of symb_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in symb_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   SymbFatalError                                                           M
*****************************************************************************/
void SymbFatalError(SymbFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = SymbDescribeError(ErrID);

    sprintf(Line, "SYMB_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Trim_lib errors right here. Call back function of trim_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in trim_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   TrimFatalError                                                           M
*****************************************************************************/
void TrimFatalError(TrimFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = TrimDescribeError(ErrID);

    sprintf(Line, "TRIM_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Triv_lib errors right here. Call back function of triv_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in triv_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   TrivFatalError                                                           M
*****************************************************************************/
void TrivFatalError(TrivFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = TrivDescribeError(ErrID);

    sprintf(Line, "TRIV_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Trng_lib errors right here. Call back function of trng_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in trng_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   TrngFatalError                                                           M
*****************************************************************************/
void TrngFatalError(TrngFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = TrngDescribeError(ErrID);

    sprintf(Line, "TRNG_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps User_lib errors right here. Call back function of user_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in user_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   UserFatalError                                                           M
*****************************************************************************/
void UserFatalError(UserFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = UserDescribeError(ErrID);

    sprintf(Line, "USER_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Traps Mvar_lib errors right here. Call back function of trng_lib.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:    Error number in trng_lib library.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            M
* KEYWORDS:                                                                  M
*   MvarFatalError                                                           M
*****************************************************************************/
void MvarFatalError(MvarFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];
    const char *ErrorMsg = MvarDescribeError(ErrID);

    sprintf(Line, "MVAR_LIB fatal error: %s", ErrorMsg);
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Default trap for Irit parser errors.					     M
*   This function prints the provided error message and dies.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:      Error message.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPFatalError, error handling                                             M
*****************************************************************************/
void IPFatalError(IPFatalErrorType ErrID)
{
    char Line[IRIT_LINE_LEN_LONG];

    sprintf(Line, "IP fatal error: %s", IPDescribeError(ErrID));
    IGIritError(Line);

    if (!IGGlblActiveXMode)
        exit(1);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of IGPrintGlblDisplayTree                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PLst: N.S.F.I.                                                           *
*   Sp:   N.S.F.I.                                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGPrintGlblDisplayTreeAux(IPObjectStruct *PLst, char *Sp)
{
    char Space[1000];

    sprintf(Space, "%s    ", Sp);

    if (IP_IS_OLST_OBJ(PLst)) {
	IPObjectStruct *PTmp;
	int i = 0;

        printf("%s[\n", Sp);
	/* Search in its list. */
	while ((PTmp = IPListObjectGet(PLst, i++)) != NULL) {
	    printf("%s\"%s\"\n", Space, IP_GET_OBJ_NAME(PTmp));
	    IGPrintGlblDisplayTreeAux(PTmp, Space);
	}
        printf("%s]\n", Sp);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints the global display list                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void IGDbgPrintGlblDisplayTree(void)
{
    IPObjectStruct
        *PTmp = IGGlblDisplayList;

    for ( ; PTmp != NULL; PTmp = PTmp -> Pnext) {
	printf("\"%s\"\n", IP_GET_OBJ_NAME(PTmp));
	IGPrintGlblDisplayTreeAux(PTmp, "");
    }
}

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
static void IGDummyLinkDebug(void)
{
    IPDbg();
}
#endif /* DEBUG */
