/*****************************************************************************
* Definitions for the display devices' programs:                             *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#ifndef GRAP_DRVS_LOC_H
#define GRAP_DRVS_LOC_H

#include "irit_sm.h"
#include "misc_lib.h"
#include "miscattr.h"
#include "iritprsr.h"
#include "geom_lib.h"
#include "grap_lib.h"

#define IG_LINE_SOLID		1
#define IG_LINE_DOTTED		2
#define IG_LINE_DASHED		3
#define IG_LINE_DOTTED_DASHED	4

/* The current NormalLength is divided by scalar to form real normal length: */
#define IG_NORMAL_DEFAULT_LENGTH	100
#define IG_NORMAL_SCALER_LENGTH		1000

#define IG_DEFAULT_IRIT_PICK_SAVE	"iritpick"

#define IG_IS_DRAG_EVENT(Event)	(Event == IG_EVENT_PERS_ORTHO_Z || \
				 Event == IG_EVENT_ROTATE || \
				 Event == IG_EVENT_ROTATE_X || \
				 Event == IG_EVENT_ROTATE_Y || \
				 Event == IG_EVENT_ROTATE_Z || \
				 Event == IG_EVENT_TRANSLATE || \
				 Event == IG_EVENT_TRANSLATE_X || \
				 Event == IG_EVENT_TRANSLATE_Y || \
				 Event == IG_EVENT_TRANSLATE_Z || \
				 Event == IG_EVENT_SCALE || \
				 Event == IG_EVENT_NEAR_CLIP || \
				 Event == IG_EVENT_FAR_CLIP)

#define IG_PICK_POLY       (1 << IP_OBJ_POLY)
#define IG_PICK_NUMERIC    (1 << IP_OBJ_NUMERIC)
#define IG_PICK_POINT      (1 << IP_OBJ_POINT)
#define IG_PICK_VECTOR     (1 << IP_OBJ_VECTOR)
#define IG_PICK_PLANE      (1 << IP_OBJ_PLANE)
#define IG_PICK_MATRIX     (1 << IP_OBJ_MATRIX)
#define IG_PICK_CURVE      (1 << IP_OBJ_CURVE)
#define IG_PICK_SURFACE    (1 << IP_OBJ_SURFACE)
#define IG_PICK_STRING     (1 << IP_OBJ_STRING)
#define IG_PICK_LIST_OBJ   (1 << IP_OBJ_LIST_OBJ)
#define IG_PICK_CTLPT      (1 << IP_OBJ_CTLPT)
#define IG_PICK_TRIMSRF    (1 << IP_OBJ_TRIMSRF)
#define IG_PICK_TRIVAR     (1 << IP_OBJ_TRIVAR)
#define IG_PICK_INSTANCE   (1 << IP_OBJ_INSTANCE)
#define IG_PICK_TRISRF     (1 << IP_OBJ_TRISRF)
#define IG_PICK_MODEL      (1 << IP_OBJ_MODEL)
#define IG_PICK_MULTIVAR   (1 << IP_OBJ_MULTIVAR)
#define IG_PICK_ANY    (IG_PICK_POLY | IG_PICK_NUMERIC | IG_PICK_POINT | \
		        IG_PICK_VECTOR | IG_PICK_PLANE | IG_PICK_MATRIX | \
		        IG_PICK_CURVE | IG_PICK_SURFACE | IG_PICK_STRING | \
		        IG_PICK_LIST_OBJ | IG_PICK_CTLPT | IG_PICK_TRIMSRF | \
		        IG_PICK_TRIVAR | IG_PICK_INSTANCE | IG_PICK_TRISRF | \
		        IG_PICK_MODEL | IG_PICK_MULTIVAR)
#define IG_PICK_OBJ(IGObjType)  ((IGObjType) & IGGlblPickObjTypes)
#define IG_PICK_SET_TYPE(Type, Set) if (Set) \
				        IGGlblPickObjTypes |= Type; \
				    else \
				        IGGlblPickObjTypes &= ~(Type)

typedef enum {
    IG_WIDGET_ENVIRONMENT = 1,
    IG_WIDGET_ANIMATION   = 2,
    IG_WIDGET_CURVES      = 4,
    IG_WIDGET_SURFACES    = 8,
    IG_WIDGET_SHADING     = 16,
    IG_WIDGET_PICK_OBJS   = 32,
    IG_WIDGET_OBJS_TRANS  = 64
} IGWidgetType;

typedef enum {               /* Mouse event reports on pick cursor requests. */
    IG_PICK_REP_NONE,
    IG_PICK_REP_MOTION,
    IG_PICK_REP_BTN1DOWN,
    IG_PICK_REP_BTN2DOWN,
    IG_PICK_REP_BTN3DOWN,
    IG_PICK_REP_BTN_UP
} IGPickReportType;

typedef enum {               /* Mouse event reports on pick cursor requests. */
    IG_PICK_ENTITY_NONE,
    IG_PICK_ENTITY_DONE,
    IG_PICK_ENTITY_OBJECT,
    IG_PICK_ENTITY_OBJ_NAME,
    IG_PICK_ENTITY_CURSOR
} IGPickEntityType;

typedef enum {
    IG_GLASSES_3D_NONE,
    IG_GLASSES_3D_CHROMADEPTH,
    IG_GLASSES_3D_RED_BLUE,
    IG_GLASSES_3D_RED_GREEN
} IGGlasses3DType;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER int
    IGGlblBackGroundImage,
    IGGlbl4Views,
    IGGlblForceUnitMat,
    IGGlblStandAlone,
    IGGlblDebugObjectsFlag,
    IGGlblDebugEchoInputFlag,
    IGGlblDelayedClear,
    IGGlblLastWasSolidRendering,
    IGGlblContinuousMotion,
    IGGlblClientHandleNumber,
    IGGlblPickObjTypes,
    IGGlblSavePickedPoly,
    IGGlblNumPolys,
    IGGlblIOHandle,
    IGGlblInitWidgetDisplay,
    IGGlblActiveXMode,
    IGGlbl3DGlassesImgIndx,
    IGGlblLightOneSide;

IRIT_GLOBAL_DATA_HEADER char
    *IGGlblImageFileName,
    *IGGlblPolyPickFileName,
    *IGGlblExecAnimation,
    *IGGlblTransPrefPos,
    *IGGlblViewPrefPos,
    **IGGlblFileNames;

IRIT_GLOBAL_DATA_HEADER IrtRType
    IGGlblOpacity;

IRIT_GLOBAL_DATA_HEADER IrtPtType
    IGGlblPickPos,
    IGGlblPickPosE3;

IRIT_GLOBAL_DATA_HEADER IGGlasses3DType
    IGGlbl3DGlassesMode;

IRIT_GLOBAL_DATA_HEADER IPObjectStruct
    *IGGlblNewDisplayObjects,
    *IGGlblPickedObj,
    *IGGlblPickedPolyObj;

#ifdef __WINNT__
void IGRedirectIOToConsole(void);
#endif /* __WINNT__ */

/* Gen_grap.c routines - generic graphic's driver routines. */
void IGConfigureGlobals(char *PrgmName, int argc, char **argv);
void IGConfigReset(void);
void IGConfirmConvexPolys(IPObjectStruct *PObj, int Depth);
void IGProcessCommandMessages(int ProcessCommandMessages);
int IGReadObjectsFromSocket(int ViewMode, IPObjectStruct **DisplayList);
void IGAddReplaceObjDisplayList(IPObjectStruct **DisplayList,
				IPObjectStruct *NewObj,
				char *ObjName);
int IGHandleObjectsFromSocket(int ViewMode,
			      IPObjectStruct *PObjs,
			      IPObjectStruct **DisplayList);
void IGSubmitCurrentMat(int ViewMode);
void IGHighlightSavePickedPoly(IPPolygonStruct *Pl,
			       int IsPolyline,
			       int IsPointList);
void IGHandleContinuousMotion(void);
int IGProcessEvent(IGGraphicEventType Event, IrtRType *ChangeFactor);
int IGStateHandler(int State, int StateStatus, int Refresh);
int IGDeleteOneObject(IPObjectStruct *PObj);
void IGTraverseObjListHierarchy(IPObjectStruct *PObjList,
				IrtHmgnMatType CrntViewMat,
				IPApplyObjFuncType ApplyFunc);
void IGPredefinedAnimation(void);
IPObjectStruct *IGFindObjectByName(const char *Name);
void IGDrawPolySilhBndry(IPObjectStruct *PObj);
void IGDrawPolygonSketches(IPObjectStruct *PObj);
IPObjectStruct *IGGenPolygonSketches(IPObjectStruct *PObj, IrtRType FineNess);
void IGDrawPolySilhOpt(IPObjectStruct *PObj);
int IGInitSrfTexture(IPObjectStruct *PObj);
void IGUpdateViewConsideringScale(IrtHmgnMatType Mat);
void IGInitializeSubViewMat(void);

/* Functions that should be defined in most graphics driver. */
int IGHandleState(int State, int StateStatus, int Refresh);
void IGHandleInternalEvents(void);
void IGUpdateWindowPixelsFromBG(void);
void IGLoadImageToDisplay(IrtBType *Image, int x, int y);
void IGSaveDisplayAsImage(char *ImageFileName);
void IGPopupCrvEditor(IPObjectStruct *CrvObj);
void IGPopupSrfEditor(IPObjectStruct *SrfObj);
void IGPopupObjEditor(IPObjectStruct *PObj, int CloneIt);
void IGCreateStateMenu(void);
void IGViewObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);
void IGSaveCurrentMatInFile(int ViewMode);
void IGIritError(char *Msg);
int IGIritYesNoQuestion(char *Msg);
int IGSetDisplay4Views(int Display4Views);

void IGSetTranspObj(IrtRType Transparency);
void IGSetLinePattern(int Pattern);
void IGSetWidthObj(int Width);
void IGSetColorObj(IPObjectStruct *PObj);
void IGSetColorRGB(int Color[3]);
void IGSetColorIntensity(int High);
int IGSetTexture(IPObjectStruct *PObj);
void IGSetLightSource(IGLightType LightPos,
		      IrtVecType LightColor,
		      int LightIndex);
void IGDrawPolylineNormal(IPObjectStruct *PObj);

/* The primitive drawing routines. */
void IGPlotTo3D(IrtRType *Pt);
void IGMoveTo3D(IrtRType *Pt);
void IGLineTo3D(IrtRType *Pt);
void IGDrawObject(IPObjectStruct *PObj);
void IGDrawPtVec(IPObjectStruct *PObj);
void IGDrawString(IPObjectStruct *PObj);
void IGDrawPoly(IPObjectStruct *PObj);
void IGDrawCurve(IPObjectStruct *PObj);
void IGDrawSurface(IPObjectStruct *PObj);
void IGDrawTrimSrf(IPObjectStruct *PObj);
void IGDrawTrivar(IPObjectStruct *PObj);
void IGDrawTriangSrf(IPObjectStruct *PObj);
void IGDrawModel(IPObjectStruct *PObj);
int IGRedrawViewWinInitRedGreen(int ImgIndx);
void IGRedrawViewWindowOGL(int ClearAll);

void IGIritBeep(void);

/* Default point/line drawing routines that might be required for no such    */
/* routines are available by the drawing device/library.  This function are  */
/* required if and only if we have very basic graphics support.		     */
void IGPlotTo2D(IrtRType X, IrtRType Y);
void IGMoveTo2D(IrtRType X, IrtRType Y);
void IGLineTo2D(IrtRType X, IrtRType Y);

/* The picking routine. */
void IGReleasePickedObject(void);
IPObjectStruct *IGHandleGenericPickEvent(IrtRType ObjectX,
					 IrtRType ObjectY,
					 int PickTypes);
void IGHandleGenericCursorEvent(IrtRType ObjectX,
				IrtRType ObjectY,
				IGPickReportType PickReport);
void IGHandlePickObject(IGPickEntityType PickEntity);
char *IGGenerateWindowHeaderString(char *Str);
char *IGStrResGenericPickEvent(IPObjectStruct *PObj);
IPObjectStruct *IGHandlePickEvent(int ScreenX, int ScreenY, int PickTypes);
void IGHandleCursorEvent(int ScreenX,
			 int ScreenY,
			 IGPickReportType PickReport);
void IGGenericScreenToObject(IrtRType ScreenX,
			     IrtRType ScreenY,
			     IrtPtType Pt,
			     IrtVecType Dir);
void IGScreenToObject(int ScreenX, int ScreenY, IrtPtType Pt, IrtVecType Dir);

/* Sketching related routines for freeforms surfaces and polygonal meshes. */
IPObjectStruct *IGSketchGenSrfSketches(CagdSrfStruct *Srf,
				       IrtRType FineNess,
				       IPObjectStruct *PObj,
				       int Importance);
void IGSketchDrawSurface(IPObjectStruct *PObj);

IPObjectStruct *IGSketchGenPolySketches(IPObjectStruct *PlObj,
					IrtRType FineNess,
					int Importance);
IPObjectStruct *IGSketchGenPolyImportanceSketches(IPObjectStruct *PObj,
					    IGSketchParamStruct *SketchParams,
					    IrtRType FineNess);
void IGSketchDrawPolygons(IPObjectStruct *PObjSketches);

/* View dependent polygonization - using output sensitive silhouettes. */
IPObjectStruct *IGVGGenDataStruct(IPObjectStruct *PObj);
IPObjectStruct *IGVGSelect(IPObjectStruct *PObj);

/* Vertex shader hardware rendering routines. */
int IGDrawDTexture(IPObjectStruct *PObj);
void IGFreeDTexture(IPObjectStruct *PObj);

typedef int (*IGVertexHandleExtraFuncType)(IPVertexStruct *V);
IGVertexHandleExtraFuncType IGSetHandleVertexProcessingFunc(
				    IGVertexHandleExtraFuncType NewVertexFunc);
int IGFfdDraw(IPObjectStruct *PObj);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GRAP_DRVS_LOC_H */
