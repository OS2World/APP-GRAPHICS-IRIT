/*****************************************************************************
* Grap_lib.h - local header file for the GRAP library.			     *
* This header is also the interface header to the world.		     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Jan. 1992   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Global definitions of	Graphics interface.			             *
*****************************************************************************/

#ifndef	GRAP_LIB_H	/* Define only once */
#define	GRAP_LIB_H

#include "irit_sm.h"
#include "misc_lib.h"
#include "miscattr.h"
#include "iritprsr.h"
#include "geom_lib.h"

#define IG_IRIT_BLACK		0
#define IG_IRIT_BLUE		1
#define IG_IRIT_GREEN		2
#define IG_IRIT_CYAN		3
#define IG_IRIT_RED		4
#define IG_IRIT_MAGENTA		5
#define IG_IRIT_BROWN		6
#define IG_IRIT_LIGHTGREY	7
#define IG_IRIT_DARKGRAY	8
#define IG_IRIT_LIGHTBLUE	9
#define IG_IRIT_LIGHTGREEN	10
#define IG_IRIT_LIGHTCYAN	11
#define IG_IRIT_LIGHTRED	12
#define IG_IRIT_LIGHTMAGENTA	13
#define IG_IRIT_YELLOW		14
#define IG_IRIT_WHITE		15

#define IG_MAX_COLOR		IP_ATTR_IRIT_COLOR_TABLE_SIZE
#define IG_DEFAULT_COLOR	1      /* For objects with no color defined. */

#define IG_POLYGON_Z_TRANS 0.005      /* Z trans of polygons over polylines. */

/* The current point cross length is divided by scalar to form real length:  */
#define IG_POINT_DEFAULT_LENGTH	20
#define IG_POINT_SCALER_LENGTH	1000

#define IG_SKETCHING_NONE		0
#define IG_SKETCHING_ISO_PARAM		1
#define IG_SKETCHING_CURVATURE		2
#define IG_SKETCHING_ISOCLINES		3
#define IG_SKETCHING_ORTHOCLINES	4

#define IG_SKETCH_TYPE_NONE		0
#define IG_SKETCH_TYPE_SILHOUETTE	1
#define IG_SKETCH_TYPE_SHADING		2
#define IG_SKETCH_TYPE_IMPORTANCE	3

#define IG_DEFAULT_NUM_OF_ISOLINES	10
#define IG_DEFAULT_SAMPLES_PER_CURVE	64
#define IG_DEFAULT_PLLN_OPTI_FINENESS	0.005
#define IG_DEFAULT_POLYGON_FINENESS	20
#define IG_DEFAULT_IRIT_MAT		"irit.imd"

#define IG_VIEW_PERSPECTIVE		1		      /* View modes. */
#define IG_VIEW_ORTHOGRAPHIC		2
#define IG_DEFAULT_PERSPECTIVE_Z	-5.0	   /* Default Z focal point. */

#define IG_HIGHLIGHT1_OBJ_TAG	0x20
#define IG_HIGHLIGHT2_OBJ_TAG	0x40
#define IG_IS_HIGHLIGHT1_OBJ(Obj)   ((Obj) -> Tags & IG_HIGHLIGHT1_OBJ_TAG)
#define IG_SET_HIGHLIGHT1_OBJ(Obj)  ((Obj) -> Tags |= IG_HIGHLIGHT1_OBJ_TAG)
#define IG_RST_HIGHLIGHT1_OBJ(Obj)  ((Obj) -> Tags &= ~IG_HIGHLIGHT1_OBJ_TAG)
#define IG_IS_HIGHLIGHT2_OBJ(Obj)   ((Obj) -> Tags & IG_HIGHLIGHT2_OBJ_TAG)
#define IG_SET_HIGHLIGHT2_OBJ(Obj)  ((Obj) -> Tags |= IG_HIGHLIGHT2_OBJ_TAG)
#define IG_RST_HIGHLIGHT2_OBJ(Obj)  ((Obj) -> Tags &= ~IG_HIGHLIGHT2_OBJ_TAG)

#define IG_ADD_ORIENT_NRML(P, N) \
				if (IGGlblFlipNormalOrient) { \
				    P[0] = P[0] - N[0] * IGGlblNormalSize; \
				    P[1] = P[1] - N[1] * IGGlblNormalSize; \
				    P[2] = P[2] - N[2] * IGGlblNormalSize; \
				} \
				else { \
				    P[0] = P[0] + N[0] * IGGlblNormalSize; \
				    P[1] = P[1] + N[1] * IGGlblNormalSize; \
				    P[2] = P[2] + N[2] * IGGlblNormalSize; \
				}

#define IG_MAX_LIGHT_SOURCES 10          /* Maximal number of light sources. */

typedef int (*IGDrawUpdateFuncType)(void);
typedef void (*IGDrawObjectFuncType)(IPObjectStruct *PObj);

typedef enum {   /* Note that some device drivers depends on this order. */
    IG_STATE_NONE,
    IG_STATE_OOPS,
    IG_STATE_MOUSE_SENSITIVE,
    IG_STATE_SCR_OBJ_TGL,
    IG_STATE_CONT_MOTION,
    IG_STATE_PERS_ORTHO_TGL,
    IG_STATE_DEPTH_CUE,
    IG_STATE_3D_GLASSES,
    IG_STATE_CACHE_GEOM,
    IG_STATE_DRAW_STYLE,
    IG_STATE_SHADING_MODEL,
    IG_STATE_BACK_FACE_CULL,
    IG_STATE_DOUBLE_BUFFER,
    IG_STATE_ANTI_ALIASING,
    IG_STATE_DRAW_INTERNAL,
    IG_STATE_DRAW_VNORMAL,
    IG_STATE_DRAW_PNORMAL,
    IG_STATE_DRAW_POLYGONS,
    IG_STATE_DRAW_SRF_MESH,
    IG_STATE_DRAW_SRF_WIRE,
    IG_STATE_DRAW_SRF_BNDRY,
    IG_STATE_DRAW_SRF_SILH,
    IG_STATE_DRAW_SRF_POLY,
    IG_STATE_DRAW_SRF_SKTCH,
    IG_STATE_DRAW_SRF_RFLCT_LNS,
    IG_STATE_FOUR_PER_FLAT,
    IG_STATE_NUM_ISOLINES,
    IG_STATE_POLYGON_APPROX,
    IG_STATE_SAMP_PER_CRV_APPROX,
    IG_STATE_LENGTH_VECTORS,
    IG_STATE_WIDTH_LINES,
    IG_STATE_WIDTH_POINTS,
    IG_STATE_POLYGON_OPTI,
    IG_STATE_POLYLINE_OPTI,
    IG_STATE_VIEW_FRONT,
    IG_STATE_VIEW_SIDE,
    IG_STATE_VIEW_TOP,
    IG_STATE_VIEW_ISOMETRY,
    IG_STATE_VIEW_4,
    IG_STATE_CLEAR_VIEW,
    IG_STATE_ANIMATION,

    IG_STATE_NUM_POLY_COUNT,
    IG_STATE_NRML_ORIENT,
    IG_STATE_LIGHT_ONE_SIDE,
    IG_STATE_POLY_APPROX,

    IG_STATE_RES_ADAP_ISO,
    IG_STATE_RES_RULED_SRF,
    IG_STATE_RULED_SRF_APPROX,
    IG_STATE_ADAP_ISO_DIR,

    IG_STATE_LOWRES_RATIO,

    IG_STATE_SKETCH_ISO_PARAM,
    IG_STATE_SKETCH_CURVATURE,
    IG_STATE_SKETCH_SILHOUETTE,

    IG_STATE_SKETCH_SIL_POWER,
    IG_STATE_SKETCH_SHADING_POWER,
    IG_STATE_SKETCH_IMP_DECAY,
    IG_STATE_SKETCH_IMP_FRNT_SPRT,
    IG_STATE_SKETCH_IMPORTANCE,

    IG_STATE_SHADE_PARAM,
    IG_STATE_SHADE_AMBIENT,
    IG_STATE_SHADE_DIFFUSE,
    IG_STATE_SHADE_SPECULAR,
    IG_STATE_SHADE_SHININESS,
    IG_STATE_SHADE_EMISSION,

    IG_STATE_SHADE_LGT_SRC_IDX,
    IG_STATE_SHADE_LGT_SRC_X,
    IG_STATE_SHADE_LGT_SRC_Y,
    IG_STATE_SHADE_LGT_SRC_Z,
    IG_STATE_SHADE_LGT_SRC_W,

    IG_STATE_SHADE_RESET,
    IG_STATE_SHADE_DISMISS,

    IG_STATE_DRAW_STYLE_SOLID,
    IG_STATE_DRAW_STYLE_WIREFRAME,
    IG_STATE_DRAW_STYLE_POINTS,

    IG_STATE_ANTI_ALIAS_OFF,
    IG_STATE_ANTI_ALIAS_ON,
    IG_STATE_ANTI_ALIAS_BLEND,

    IG_STATE_CRV_EDIT,

    IG_STATE_SAVE_IMAGE,

    IG_STATE_FRAME_PER_SEC
} IGGlblStateType;

typedef enum {
    IG_EVENT_ZERO = 0,
    IG_EVENT_RESET,
    IG_EVENT_NONE = 2000,
    IG_EVENT_DISCONNECT,
    IG_EVENT_QUIT,
    IG_EVENT_SCR_OBJ_TGL,
    IG_EVENT_CONT_MOTION,
    IG_EVENT_NRML_ORIENT,
    IG_EVENT_PERS_ORTHO_TGL,
    IG_EVENT_PERS_ORTHO_Z,
    IG_EVENT_ROTATE,
    IG_EVENT_ROTATE_X,
    IG_EVENT_ROTATE_Y,
    IG_EVENT_ROTATE_Z,
    IG_EVENT_TRANSLATE,
    IG_EVENT_TRANSLATE_X,
    IG_EVENT_TRANSLATE_Y,
    IG_EVENT_TRANSLATE_Z,
    IG_EVENT_SCALE,
    IG_EVENT_NEAR_CLIP,
    IG_EVENT_FAR_CLIP,
    IG_EVENT_DEPTH_CUE,
    IG_EVENT_3D_GLASSES,
    IG_EVENT_DBL_BUFFER,
    IG_EVENT_ACCUM_MATRIX,
    IG_EVENT_SAVE_MATRIX,
    IG_EVENT_SUBMIT_MATRIX,
    IG_EVENT_PUSH_MATRIX,
    IG_EVENT_POP_MATRIX,
    IG_EVENT_ANIMATION,
    IG_EVENT_SHADE_PARAM,
    IG_EVENT_CRV_EDIT,
    IG_EVENT_SRF_EDIT,
    IG_EVENT_PICK_OBJS,
    IG_EVENT_OBJ_MANIP,
    IG_EVENT_STATE
} IGGraphicEventType;

#define IG_MAX_ROTATE_ANGLE	45.0 /* Max. rates used by interactive mode. */
#define IG_MAX_TRANSLATE_FACTOR	2.0
#define IG_MAX_SCALE_FACTOR	2.0
#define IG_MAX_CLIP_FACTOR	0.5

#define IG_TRANS_SCREEN	1     /* Screen, Object coords. transformation mode. */
#define IG_TRANS_OBJECT	2

#define IG_STATE_OFF		-32760
#define IG_STATE_ON		-32761
#define IG_STATE_DEC		IG_STATE_OFF
#define IG_STATE_INC		IG_STATE_ON
#define IG_STATE_TGL		-32762

#define IG_SHADING_NONE		0
#define IG_SHADING_BACKGROUND	1
#define IG_SHADING_FLAT		2
#define IG_SHADING_GOURAUD	3
#define IG_SHADING_PHONG	4

typedef float IGLightType[4];

typedef struct IGSketchParamStruct {
    IrtRType ShadePower;
    IrtRType SilPower;
    int SketchSilType;
    int SketchShdType;
    int SketchImpType;
    int SketchImp;
    int SketchInvShd;
    IrtRType SketchImpDecay;
    IrtRType SketchImpFrntSprt;
} IGSketchParamStruct;

typedef struct IGShadeParamStruct {
    int NumOfLightSrcs;
    IGLightType LightPos[IG_MAX_LIGHT_SOURCES];
    IGLightType LightAmbient;
    IGLightType LightDiffuse;
    IGLightType LightSpecular;
    IGLightType LightEmissive;
    float Shininess;
} IGShadeParamStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER IPObjectStruct
    *IGGlblDisplayList;

IRIT_GLOBAL_DATA_HEADER int
    IGGlblBackGroundColor[3],
    IGGlblAbortKeyPressed,
    IGGlblAdapIsoDir,
    IGGlblBackFaceCull,
    IGGlblCacheGeom,
    IGGlblCountNumPolys,
    IGGlblDepthCue,
    IGGlblDrawInternal,
    IGGlblDrawPNormal,
    IGGlblDrawPolygons,
    IGGlblDrawSurfaceMesh,
    IGGlblDrawSurfacePoly,
    IGGlblDrawSurfaceWire,
    IGGlblDrawSurfaceBndry,
    IGGlblDrawSurfaceSilh,
    IGGlblDrawSurfaceDiscont,
    IGGlblDrawSurfaceContours,
    IGGlblDrawSurfaceIsophotes,
    IGGlblDrawSurfaceSketch,
    IGGlblDrawSurfaceRflctLns,
    IGGlblDrawStyle,
    IGGlblDrawVNormal,
    IGGlblFlipNormalOrient,
    IGGlblPolygonStrips,
    IGGlblFourPerFlat,
    IGGlblHighlight1Color[3],
    IGGlblHighlight2Color[3],
    IGGlblIntensityHighState,
    IGGlblLastLowResDraw,
    IGGlblLineWidth,
    IGGlblManipulationActive,
    IGGlblMore,
    IGGlblNumOfIsolines,
    IGGlblNumPolys,
    IGGlblPolygonOptiApprox,
    IGGlblQuickLoad,
    IGGlblContinuousMotion,
    IGGlblViewMode,
    IGGlblAntiAliasing,
    IGGlblShadingModel,
    IGGlblDoDoubleBuffer,
    IGGlblAnimation,
    IGGlblTransformMode,
    IGGlblCountFramePerSec,
    IGGlblAlwaysGenUV,
    IGGlblNumFiles;
IRIT_GLOBAL_DATA_HEADER IrtRType
    IGGlblNormalSize,
    IGGlblPointSize,
    IGGlblMinPickDist,
    IGGlblPlgnFineness,
    IGGlblPllnFineness,
    IGGlblRelLowresFineNess,
    IGGlblFramePerSec,
    IGGlblChangeFactor,
    IGGlblZMinClip,
    IGGlblZMaxClip,
    IGGlblEyeDistance;
IRIT_GLOBAL_DATA_HEADER SymbCrvApproxMethodType
    IGGlblPolylineOptiApprox;
IRIT_GLOBAL_DATA_HEADER IrtHmgnMatType
    IGGlblCrntViewMat,
    IGGlblInvCrntViewMat,
    IGGlblLastProcessMat,
    IGGlblPushViewMat,
    IGGlblPushPrspMat,
    IGGlblIsometryViewMat;

IRIT_GLOBAL_DATA_HEADER GMAnimationStruct IGAnimation;
IRIT_GLOBAL_DATA_HEADER IGShadeParamStruct IGShadeParam;
IRIT_GLOBAL_DATA_HEADER IGSketchParamStruct IGSketchParam;

IRIT_GLOBAL_DATA_HEADER IGDrawObjectFuncType IGDrawPolyFuncPtr;

/* Functions that should be supplied by the user of this lib. */
void IGRedrawViewWindow(void);

/*** Function from this point in are fully implemented in grap_lib: ***/

/* Grap_lib general functions. */
void IGConfirmConvexPolys(IPObjectStruct *PObj, int Depth);
IPObjectStruct *IGLoadGeometry(const char **FileNames, int NumFiles);
void IGSaveCurrentMat(int ViewMode, char *Name);
void IGActiveListFreePolyIsoAttribute(IPObjectStruct *PObjs,
				      int FreePolygons,
				      int FreeIsolines,
				      int FreeSketches,
				      int FreeCtlMesh);
void IGActiveListFreeNamedAttribute(IPObjectStruct *PObjs, char *Name);
void IGActiveFreePolyIsoAttribute(IPObjectStruct *PObj,
				  int FreePolygons,
				  int FreeIsolines,
				  int FreeSketches,
				  int FreeCtlMesh);
void IGActiveFreeNamedAttribute(IPObjectStruct *PObj, char *Name);
void IGUpdateObjectBBox(IPObjectStruct *PObj);
void IGUpdateViewConsideringScale(IrtHmgnMatType Mat);
IrtRType IGFindMinimalDist(IPObjectStruct *PObj,
			   IPPolygonStruct **MinPl,
			   IrtPtType MinPt,
			   int *MinPlIsPolyline,
			   IrtPtType LinePos,
			   IrtVecType LineDir,
			   IrtRType *HitDepth);
void IGDrawPolygonSketches(IPObjectStruct *PObj);
IPObjectStruct *IGGenPolygonSketches(IPObjectStruct *PObj, IrtRType FineNess);
void IGDrawPolySilhBndry(IPObjectStruct *PObj);
void IGDrawPolySilhouette(IPObjectStruct *PObj);
void IGDrawPolyBoundary(IPObjectStruct *PObj);
void IGDrawPolyDiscontinuities(IPObjectStruct *PObj);
int IGDrawPolyContoursSetup(IrtRType x, IrtRType y, IrtRType z, int n);
void IGDrawPolyContours(IPObjectStruct *PObj);
int IGDrawPolyIsophotesSetup(IrtRType x, IrtRType y, IrtRType z, int n);
void IGDrawPolyIsophotes(IPObjectStruct *PObj);
IPObjectStruct *IGGetObjIsoLines(IPObjectStruct *PObj);
IPObjectStruct *IGGetObjPolygons(IPObjectStruct *PObj);
int IGInitSrfTexture(IPObjectStruct *PObj);
int IGDefaultProcessEvent(IGGraphicEventType Event, IrtRType *ChangeFactor);
int IGDefaultStateHandler(int State, int StateStatus, int Refresh);
void IGUpdateFPS(int Start);

/* Generic drawing routines - essentially convert the geometry into a      */
/* displayable form and invoke the drawing routines recursively.	   */
void IGDrawCurve(IPObjectStruct *PObj);
void IGDrawCurveGenPolylines(IPObjectStruct *PObj);
void IGDrawModel(IPObjectStruct *PObj);
void IGDrawModelGenPolygons(IPObjectStruct *PObj);
void IGDrawString(IPObjectStruct *PObj);
void IGDrawSurface(IPObjectStruct *PObj);
void IGDrawSurfaceGenPolygons(IPObjectStruct *PObj);
void IGDrawSurfaceAIso(IPObjectStruct *PObj);
void IGDrawTriangSrf(IPObjectStruct *PObj);
void IGDrawTriangGenSrfPolygons(IPObjectStruct *PObj);
void IGDrawTrimSrf(IPObjectStruct *PObj);
void IGDrawTrimSrfGenPolygons(IPObjectStruct *PObj);
void IGDrawTrivar(IPObjectStruct *PObj);
void IGDrawTrivarGenSrfPolygons(IPObjectStruct *PObj);

/* Sketching related routines for freeforms surfaces and polygonal meshes. */
void IGSketchDrawSurface(IPObjectStruct *PObjSketches);
IPObjectStruct *IGSketchGenSrfSketches(CagdSrfStruct *Srf,
				       IrtRType FineNess,
				       IPObjectStruct *PObj,
				       int Importance);
void IGSketchDrawPolygons(IPObjectStruct *PObjSketches);
IPObjectStruct *IGSketchGenPolySketches(IPObjectStruct *PlObj,
					IrtRType FineNess,
					int Importance);
IPObjectStruct *IGSketchGenPolyImportanceSketches(IPObjectStruct *PObj,
					    IGSketchParamStruct *SketchParams,
					    IrtRType FineNess);

/* Allow control over possible pre/post processing of different style draw. */
IGDrawUpdateFuncType IGSetSrfPolysPreFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetSrfPolysPostFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetSrfWirePreFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetSrfWirePostFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetSketchPreFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetSketchPostFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetCtlMeshPreFunc(IGDrawUpdateFuncType Func);
IGDrawUpdateFuncType IGSetCtlMeshPostFunc(IGDrawUpdateFuncType Func);

/* Open GL CG hardware GPU support. */
int IGCGDrawDTexture(IPObjectStruct *PObj);
void IGCGFreeDTexture(IPObjectStruct *PObj);
int IGCGFfdDraw(IPObjectStruct *PObj);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GRAP_LIB_H */

