/*****************************************************************************
*   An SGI 4D driver using GL (experimental adaptive isocurve renderer.).    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <gl/gl.h>
#include <gl/device.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"

/* Interactive menu setup structure: */
#define INTERACT_NUM_OF_STRINGS		4
#define INTERACT_NUM_OF_SUB_WNDWS	17
#define IN_VIEW_WINDOW(x, y)	((x) >= ViewWinLeft && \
				 (y) >= ViewWinLow && \
				 (x) <= ViewWinLeft + ViewWinWidth && \
				 (y) <= ViewWinLow + ViewWinHeight)
#define IN_TRANS_WINDOW(x, y)   ((x) >= TransWinLeft && \
				 (y) >= TransWinLow && \
				 (x) <= TransWinLeft + TransWinWidth && \
				 (y) <= TransWinLow + TransWinHeight)

/* Handle the positional as well as possible RGB attribute of a vertex. */
#define HANDLE_VERTEX(V) { char *Str; \
			   int Color[3]; \
			   if ((Str = AttrGetStrAttrib((V) -> Attr, \
						       "RGB")) != NULL && \
			       sscanf(Str, "%d,%d,%d", \
				      &Color[0], &Color[1], &Color[2])) { \
			       c3i((long *) Color); \
			   } \
			   v3d((V) -> Coord); \
			 }


typedef struct InteractString {
    IrtRType X, Y;
    int Color;
    char *Str;
} InteractString;
typedef struct InteractSubWindow {
    IrtRType X, Y;					   /* Center points. */
    int Color;
    IGGraphicEventType Event;
    int TextInside; /* If TRUE, Str will be in window, otherwise left to it. */
    char *Str;
} InteractSubWindow;
typedef struct InteractWindowStruct {	 /* The interactive menu structures. */
    /* Rotate, Translate, Scale strings: */
    InteractString Strings[INTERACT_NUM_OF_STRINGS];
    InteractSubWindow SubWindows[INTERACT_NUM_OF_SUB_WNDWS];
} InteractWindowStruct;

#define INTERACT_SUB_WINDOW_WIDTH  0.8		 /* Relative to window size. */
#define INTERACT_SUB_WINDOW_HEIGHT 0.04

STATIC_DATA int
    ReturnPickedObject = FALSE,
    ReturnPickedObjectName = TRUE,
    PickCrsrGrabMouse = FALSE,
    GlblStateMenu = 0,
    GlblRuledSrfApprox = FALSE;

STATIC_DATA long
    TransWinID = 0,
    TransWinWidth = 100,
    TransWinWidth2 = 50,
    TransWinHeight = 100,
    TransWinLow = 0,
    TransWinLeft = 0,
    ViewWinID = 0,
    ViewWinWidth = 100,
    ViewWinHeight = 100,
    ViewWinLow = 0,
    ViewWinLeft = 0;

STATIC_DATA IrtRType
    GlblAdapIsoEps = 0.05,
    GlblRuledSrfEps = 0.02;

/* Interactive mode menu set up structure is define below: */
STATIC_DATA InteractWindowStruct InteractMenu = {
    { { 0.5, 0.81, IG_IRIT_RED,		"Rotate" },
      { 0.5, 0.65, IG_IRIT_GREEN,	"Translate" },
      { 0.5, 0.49, IG_IRIT_CYAN,	"Scale" },
      { 0.5, 0.41, IG_IRIT_LIGHTGREEN,	"Clip Plane" },
    },
    { { 0.5, 0.93, IG_IRIT_YELLOW, IG_EVENT_SCR_OBJ_TGL,	TRUE,  "Screen Coords." },
      { 0.5, 0.87, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_TGL,     TRUE,  "Perspective" },
      { 0.5, 0.83, IG_IRIT_BLUE,   IG_EVENT_PERS_ORTHO_Z,	FALSE, "Z" },
      { 0.5, 0.75, IG_IRIT_RED,    IG_EVENT_ROTATE_X,		FALSE, "X" }, /* Rot */
      { 0.5, 0.71, IG_IRIT_RED,    IG_EVENT_ROTATE_Y,		FALSE, "Y" },
      { 0.5, 0.67, IG_IRIT_RED,    IG_EVENT_ROTATE_Z,		FALSE, "Z" },
      { 0.5, 0.59, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_X,	FALSE, "X" }, /* Trans */
      { 0.5, 0.55, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Y,	FALSE, "Y" },
      { 0.5, 0.51, IG_IRIT_GREEN,  IG_EVENT_TRANSLATE_Z,	FALSE, "Z" },
      { 0.5, 0.43, IG_IRIT_CYAN,   IG_EVENT_SCALE,		FALSE, "" },  /* Scale */
      { 0.5, 0.35, IG_IRIT_LIGHTGREEN, IG_EVENT_NEAR_CLIP,	FALSE,  "" },
      { 0.5, 0.31, IG_IRIT_LIGHTGREEN, IG_EVENT_FAR_CLIP,	FALSE,  "" },

      { 0.5, 0.23, IG_IRIT_YELLOW, IG_EVENT_SAVE_MATRIX,	TRUE,  "Save Matrix" },
      { 0.5, 0.19, IG_IRIT_YELLOW, IG_EVENT_SUBMIT_MATRIX,	TRUE,  "Submit Matrix" },
      { 0.5, 0.13, IG_IRIT_YELLOW, IG_EVENT_PUSH_MATRIX,	TRUE,  "Push Matrix" },
      { 0.5, 0.09, IG_IRIT_YELLOW, IG_EVENT_POP_MATRIX,		TRUE,  "Pop Matrix" },
      { 0.5, 0.03, IG_IRIT_WHITE,  IG_EVENT_QUIT,		TRUE,  "Quit" },
    }
};

static void IGDrawPolyNormal(IPObjectStruct *PObj, int HasNormals);
static IPPolygonStruct *IritSurface2AdapIso(CagdSrfStruct *Srf,
					    CagdSrfDirType Dir,
					    IrtRType Eps);
static void SetColorIndex(int c);
static void ClearViewArea(void);
static void SetTransformWindow(void);
static void RedrawTransformWindow(void);
static void SetViewWindow(void);
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor,
					  int BlockForEvent);
static void DrawText(char *Str, long PosX, long PosY);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of xgladap - Adaptive isocurve freeform surface rendering      M
*			   graphics driver of IRIT.             	     M
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
    IrtRType ChangeFactor[2];
    IGGraphicEventType Event;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IGConfigureGlobals("x11drvs", argc, argv);

    SetViewWindow();
    IGRedrawViewWindow();
    SetTransformWindow();
    RedrawTransformWindow();

    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    qdevice(RIGHTMOUSE);
    qdevice(LEFTSHIFTKEY);
    qdevice(RIGHTSHIFTKEY);

    /* The default drawing window is the view window. */
    winset(ViewWinID);

    setbell(1);						   /* Make it short. */

    IGCreateStateMenu();

    while ((Event = GetGraphicEvent(ChangeFactor, TRUE)) != IG_EVENT_QUIT) {
        ChangeFactor[0] *= IGGlblChangeFactor;
	ChangeFactor[1] *= IGGlblChangeFactor;

	if (IGProcessEvent(Event, ChangeFactor))
	    IGRedrawViewWindow();
    }

    if (IGGlblIOHandle >= 0)
        IPCloseStream(IGGlblIOHandle, TRUE);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the curve editor, if not already up and hook CrvObj to it.         M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to edit.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupCrvEditor                                                         M
*****************************************************************************/
void IGPopupCrvEditor(IPObjectStruct *CrvObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the surface editor, if not already up and hook SrfObj to it.       M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to edit.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupSrfEditor                                                         M
*****************************************************************************/
void IGPopupSrfEditor(IPObjectStruct *SrfObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the object editor, if not already up and hook PObj to it.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to edit.                                                 M
*   CloneIt: If TRUE make a copy of given object fist and edit the clone.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupObjEditor                                                         M
*****************************************************************************/
void IGPopupObjEditor(IPObjectStruct *PObj, int CloneIt)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function initilized the subview mat to front, side, top & Isometry  M
* views.                                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGInitializeSubViewMat                                                   M
*****************************************************************************/
void IGInitializeSubViewMat(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Function to enable/disable 4views mode.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Display4Views:  TRUE for 4 views mode.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if a change in views' style occured.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetDisplay4Views                                                       M
*****************************************************************************/
int IGSetDisplay4Views(int Display4Views)
{
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Optionally construct a state pop up menu for the driver, if has one.       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCreateStateMenu                                                        M
*****************************************************************************/
void IGCreateStateMenu(void)
{
    if (GlblStateMenu) 
        freepup(GlblStateMenu);

    GlblStateMenu = newpup();

    addtopup(GlblStateMenu, " Set Up %t", 0);
    addtopup(GlblStateMenu, "Rht-On, Sft-Rht-Off%l", 0);
    addtopup(GlblStateMenu, "Mouse Sensitive%l", 0);
    addtopup(GlblStateMenu, IGGlblTransformMode == IG_TRANS_SCREEN ?
				"Screen Trans." : "Object Trans", 0);
    addtopup(GlblStateMenu,
	     IGGlblContinuousMotion ? "Cont Motion" : "Regular Motion", 0);
    addtopup(GlblStateMenu, IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
				"Perspective" : "Orthographic", 0);
    addtopup(GlblStateMenu,
	     IGGlblDepthCue ? "Depth Cue" : "No Depth Cue", 0);
    addtopup(GlblStateMenu,
	     IGGlblCacheGeom ? "Cache Geom" : "No Geom Cache", 0);

    switch (IGGlblDrawStyle) {
	case IG_STATE_DRAW_STYLE_WIREFRAME:
	    addtopup(GlblStateMenu, "Draw Wireframe", 0);
	    break;
	case IG_STATE_DRAW_STYLE_SOLID:
	    addtopup(GlblStateMenu, "Draw Solid", 0);
	    break;
	case IG_STATE_DRAW_STYLE_POINTS:
	    addtopup(GlblStateMenu, "Draw Points", 0);
	    break;
    }

    switch (IGGlblShadingModel) {
	case IG_SHADING_NONE:
	default:
	    addtopup(GlblStateMenu, "No Shading", 0);
	    break;
	case IG_SHADING_FLAT:
	    addtopup(GlblStateMenu, "Flat Shading", 0);
	    break;
	case IG_SHADING_BACKGROUND:
	    addtopup(GlblStateMenu, "Background Shading", 0);
	    break;
	case IG_SHADING_GOURAUD:
	    addtopup(GlblStateMenu, "Gouraud Shading", 0);
	    break;
	case IG_SHADING_PHONG:
	    addtopup(GlblStateMenu, "Phong Shading", 0);
	    break;
    }
    addtopup(GlblStateMenu,
	     IGGlblBackFaceCull ? "Cull Back Face" : "No Cull Back Face", 0);
    addtopup(GlblStateMenu,
	     IGGlblDoDoubleBuffer ? "Double Buffer" : "Single Buffer", 0);

    switch (IGGlblAntiAliasing) {
	case IG_STATE_ANTI_ALIAS_OFF:
	    addtopup(GlblStateMenu, "No Anti Aliasing%l", 0);
	    break;
	case IG_STATE_ANTI_ALIAS_ON:
	    addtopup(GlblStateMenu, "Anti Aliasing%l", 0);
	    break;
	case IG_STATE_ANTI_ALIAS_BLEND:
	    addtopup(GlblStateMenu, "Blending%l", 0);
	    break;
    }

    /* We jump here all a whole bunch of indices. */

    addtopup(GlblStateMenu, "Num Isolines%l", 0);
    addtopup(GlblStateMenu, "Res Polygons%l", 0);
    addtopup(GlblStateMenu, "Res Polylines%l", 0);
    addtopup(GlblStateMenu, "Length Vectors%l", 0);
    addtopup(GlblStateMenu, "Width Lines%l", 0);
    addtopup(GlblStateMenu, "Width Points%l", 0);
    addtopup(GlblStateMenu, "Front View", 0);
    addtopup(GlblStateMenu, "Side View", 0);
    addtopup(GlblStateMenu, "Top View", 0);
    addtopup(GlblStateMenu, "Isometry View%l", 0);
    addtopup(GlblStateMenu, "Clear View Area%l", 0);
    addtopup(GlblStateMenu, "Animation%l", 0);

    addtopup(GlblStateMenu, "Res Adap. Iso.", 0);
    addtopup(GlblStateMenu, "Res Ruled Srf.", 0);
    addtopup(GlblStateMenu, 
	     GlblRuledSrfApprox ? "Direct Adap. Iso." : "Ruled Adap. Iso.", 0);
    addtopup(GlblStateMenu, "Adap. Iso. Dir.", 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Point/Vector object using current modes and transformations. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A point/vector object to draw.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPtVec                                                              M
*****************************************************************************/
void IGDrawPtVec(IPObjectStruct *PObj)
{
    STATIC_DATA IrtPtType
	Zero = { 0.0, 0.0, 0.0 };
    int i;
    IrtPtType Ends[6];
    IrtRType
	*Pt = PObj -> U.Pt;

    for (i = 0; i < 6; i++)
	PT_COPY(Ends[i], Pt);

    Ends[0][0] -= IGGlblPointSize;
    Ends[1][0] += IGGlblPointSize;
    Ends[2][1] -= IGGlblPointSize;
    Ends[3][1] += IGGlblPointSize;
    Ends[4][2] -= IGGlblPointSize;
    Ends[5][2] += IGGlblPointSize;

    for (i = 0; i < 6; i += 2) {
	bgnline();
	v3d(Ends[i]);
	v3d(Ends[i+1]);
	endline();
    }

    if (IP_IS_VEC_OBJ(PObj)) {
	bgnline();
	v3d(Pt);
	v3d(Zero);
	endline();
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Poly object using current modes and transformations.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A poly object to draw.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPoly                                                               M
*****************************************************************************/
void IGDrawPoly(IPObjectStruct *PObj)
{
    IGDrawPolyNormal(PObj, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw a single Poly object using current modes and transformations.	     *
*   This function allows polylines to have normals so we can actually draw   *
* the adaptive isolines to render a surface.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:         A polygon or a polyline to draw.                           *
*   HasNormals:   Do we have normal information included?                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawPolyNormal(IPObjectStruct *PObj, int HasNormals)
{
    IPVertexStruct *V;
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    bgnline();
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		if (HasNormals) {
		    float n[3];

		    if (IGGlblFlipNormalOrient) {
		        n[0] = V -> Normal[0];
			n[1] = V -> Normal[1];
			n[2] = V -> Normal[2];
		    }
		    else {
		        n[0] = -V -> Normal[0];
			n[1] = -V -> Normal[1];
			n[2] = -V -> Normal[2];
		    }
		    n3f(n);
		}
		HANDLE_VERTEX(V);
	    }
	    endline();
	}
    }
    else if (IP_IS_POINTLIST_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		int i;
		IrtPtType Ends[6];
		IrtRType
		    *Pt = V -> Coord;

		for (i = 0; i < 6; i++)
		    PT_COPY(Ends[i], Pt);

		Ends[0][0] -= IGGlblPointSize;
		Ends[1][0] += IGGlblPointSize;
		Ends[2][1] -= IGGlblPointSize;
		Ends[3][1] += IGGlblPointSize;
		Ends[4][2] -= IGGlblPointSize;
		Ends[5][2] += IGGlblPointSize;

		for (i = 0; i < 6; i += 2) {
		    bgnline();
		    v3d(Ends[i]);
		    v3d(Ends[i+1]);
		    endline();
		}
	    }
	}
    }
    else if (IP_IS_POLYGON_OBJ(PObj)) {
	int i, j,
	    PolysFromSrf = AttrGetObjectIntAttrib(PObj, "_srf_polys") == TRUE,
	    NumOfVertices = 0;
	IrtPtType PNormal, VNormal;

	if (IGGlblDrawSurfaceSketch)
	    IGDrawPolygonSketches(PObj);

	if (IGGlblDrawSurfaceSilh || IGGlblDrawSurfaceBndry)
	    IGDrawPolySilhBndry(PObj);

	if ((PolysFromSrf && !IGGlblDrawSurfacePoly) ||
	    (!PolysFromSrf && !IGGlblDrawPolygons))
	    return;

	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    if (IGGlblBackFaceCull) {
	        IrtRType P[3];

		MatMultVecby4by4(P, Pl -> Plane, IPViewMat);
		if ((P[2] > 0.0) ^ IGGlblFlipNormalOrient)
		    continue;
	    }

	    if (IGGlblDrawPNormal) {
		NumOfVertices = 0;
		VEC_RESET(PNormal);
	    }

	    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
		bgnpolygon();
		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    float n[3];

		    if (IGGlblFlipNormalOrient) {
		        n[0] = V -> Normal[0];
			n[1] = V -> Normal[1];
			n[2] = V -> Normal[2];
		    }
		    else {
		        n[0] = -V -> Normal[0];
			n[1] = -V -> Normal[1];
			n[2] = -V -> Normal[2];
		    }
		    n3f(n);
		    HANDLE_VERTEX(V);

		    if (IGGlblDrawPNormal) {
			for (j = 0; j < 3; j++)
			    PNormal[j] += V -> Coord[j];
			NumOfVertices++;
		    }
		}
		endpolygon();
	    }
	    else {
		bgnline();
		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    HANDLE_VERTEX(V);
		    if (IP_IS_INTERNAL_VRTX(V) && !IGGlblDrawInternal) {
			endline();
			bgnline();
		    }

		    if (IGGlblDrawPNormal) {
			for (j = 0; j < 3; j++)
			    PNormal[j] += V -> Coord[j];
			NumOfVertices++;
		    }
		}
		HANDLE_VERTEX(Pl -> PVertex);
		endline();
	    }

	    if (IGGlblDrawPNormal && IP_HAS_PLANE_POLY(Pl)) {
		bgnline();
		for (i = 0; i < 3; i++)
		    PNormal[i] /= NumOfVertices;
		v3d(PNormal);
		IG_ADD_ORIENT_NRML(PNormal, Pl -> Plane);
		v3d(PNormal);
		endline();
	    }

	    if (IGGlblDrawVNormal) {
		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    if (IP_HAS_NORMAL_VRTX(V)) {
		        VEC_COPY(VNormal, V -> Coord);
		        IG_ADD_ORIENT_NRML(VNormal, V -> Normal);
			bgnline();
			v3d(V ->Coord);
			v3d(VNormal);
			endline();
		    }
		}
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Surface object using current modes and transformations.	     M
*   Surface must be with either E3 or P3 point type and must be a NURB srf.  M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlmesh" M
* attributes of PObj. Adaptive isocurves are saved under "_adap_iso" and     M
* polygons under "_polygons.".						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A surface object to draw.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawSurface                                                            M
*****************************************************************************/
void IGDrawSurface(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines, *PObjCtlMesh, *PObjPolygons;
    IPPolygonStruct *PPolylines, *PCtlMesh, *PPolygons, *PPolyTemp;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_isoline")) == NULL &&
	IGGlblNumOfIsolines > 0) {
	CagdSrfStruct *Srf,
	    *Srfs = PObj -> U.Srfs;

	PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjPolylines);
	for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	    int NumOfIso[2];

	    NumOfIso[0] = -IGGlblNumOfIsolines;
	    NumOfIso[1] = -IGGlblNumOfIsolines;
	    PPolylines = IPSurface2Polylines(Srf, NumOfIso,
					     IGGlblPllnFineness,
					     IGGlblPolylineOptiApprox);

	    if (PPolylines != NULL) {
		for (PPolyTemp = PPolylines;
		     PPolyTemp -> Pnext;
		     PPolyTemp = PPolyTemp -> Pnext);
		PPolyTemp -> Pnext = PObjPolylines -> U.Pl;
		PObjPolylines -> U.Pl = PPolylines;
	    }
	}
	AttrSetObjectObjAttrib(PObj, "_isoline", PObjPolylines, FALSE);
    }

    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
	if (IGGlblDrawSurfacePoly) {
	    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_polygons"))
								    == NULL) {
		CagdSrfStruct *Srf,
		    *Srfs = PObj -> U.Srfs;

		PObjPolygons = IPAllocObject("", IP_OBJ_POLY, NULL);
		PObjPolygons -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
		IP_SET_POLYGON_OBJ(PObjPolygons);

		for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		    PPolygons = IPSurface2Polygons(Srf, IGGlblFourPerFlat,
						   IGGlblPlgnFineness, FALSE,
						   TRUE,
						     IGGlblPolygonOptiApprox);

		    if (PPolygons != NULL) {
			if (PPolygons) {
			    for (PPolyTemp = PPolygons;
				 PPolyTemp -> Pnext;
				 PPolyTemp = PPolyTemp -> Pnext);
			    PPolyTemp -> Pnext = PObjPolygons -> U.Pl;
			    PObjPolygons -> U.Pl = PPolygons;
			}
		    }
		}
		AttrSetObjectObjAttrib(PObj, "_polygons", PObjPolygons, FALSE);
	    }

	    IGDrawPolyNormal(PObjPolygons, TRUE);
	}
	else {
	    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_adap_iso"))
								    == NULL) {
		CagdSrfStruct *Srf,
		    *Srfs = PObj -> U.Srfs;

		PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
		PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
		IP_SET_POLYLINE_OBJ(PObjPolylines);

		for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		    PPolylines = IritSurface2AdapIso(Srf,
					     (CagdSrfDirType) IGGlblAdapIsoDir,
					     GlblAdapIsoEps);

		    if (PPolylines) {
			for (PPolyTemp = PPolylines;
			     PPolyTemp -> Pnext;
			     PPolyTemp = PPolyTemp -> Pnext);
			PPolyTemp -> Pnext = PObjPolylines -> U.Pl;
			PObjPolylines -> U.Pl = PPolylines;
		    }
		}
		AttrSetObjectObjAttrib(PObj, "_adap_iso", PObjPolylines, FALSE);
	    }

	    IGDrawPolyNormal(PObjPolylines, TRUE);
	}
    }
    else
        IGDrawPolyNormal(PObjPolylines, FALSE);

    if (IGGlblDrawSurfaceMesh) {
	if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    CagdSrfStruct *Srf,
		*Srfs = PObj -> U.Srfs;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		PCtlMesh = IPSurface2CtlMesh(Srf);

		for (PPolyTemp = PCtlMesh;
		     PPolyTemp -> Pnext;
		     PPolyTemp = PPolyTemp -> Pnext);
		PPolyTemp -> Pnext = PObjCtlMesh -> U.Pl;
		PObjCtlMesh -> U.Pl = PCtlMesh;
	    }
	    AttrSetObjectObjAttrib(PObj, "_ctlmesh", PObjCtlMesh, FALSE);
	}

	IGDrawPolyNormal(AttrGetObjectObjAttrib(PObj, "_ctlmesh"), FALSE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Creates an adaptive isocurve coverage to a given surface.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:                Surface to convert to adaptive isocurve's coverage.  *
*   Dir:                Direction of isocurves. Either U or V.               *
*   Eps:                Accuracy of coverage.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  A list of polylines approximating the coverage.      *
*****************************************************************************/
static IPPolygonStruct *IritSurface2AdapIso(CagdSrfStruct *Srf,
					    CagdSrfDirType Dir,
					    IrtRType Eps)
{
    CagdCrvStruct *Coverage, *OneCoverage, *Crv;
    IPPolygonStruct
	*IsoPoly = NULL;
    CagdSrfStruct *NSrf;

    if (Dir == CAGD_NO_DIR)
	Dir = Srf -> UOrder == 2 ? CAGD_CONST_U_DIR : CAGD_CONST_V_DIR;

    fprintf(stderr, "Generating adaptive isocurve coverage...\n");
    if (GlblRuledSrfApprox) {
	int IsBspline = TRUE;
	CagdSrfStruct *RuledSrfs, *NormalSrf, *TSrf;

	if (CAGD_IS_BEZIER_SRF(Srf)) {
	    IsBspline = FALSE;
	    Srf = CnvrtBezier2BsplineSrf(Srf);
	}
	RuledSrfs = SymbPiecewiseRuledSrfApprox(Srf, TRUE, GlblRuledSrfEps,
						Dir);

	Coverage = NULL;
	NormalSrf = SymbSrfNormalSrf(Srf);
	for (TSrf = RuledSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    CagdRType UMin, UMax, VMin, VMax;

	    CagdSrfDomain(TSrf, &UMin, &UMax, &VMin, &VMax);
	    if (Dir == CAGD_CONST_V_DIR)
		NSrf = CagdSrfRegionFromSrf(NormalSrf, UMin, UMax,
					    CAGD_CONST_U_DIR);
	    else
		NSrf = CagdSrfRegionFromSrf(NormalSrf, VMin, VMax,
					    CAGD_CONST_V_DIR);
	    OneCoverage = SymbAdapIsoExtract(TSrf, NSrf, NULL, Dir,
					     Eps, FALSE, FALSE);
	    CagdSrfFree(NSrf);

	    for (Crv = OneCoverage; Crv -> Pnext != NULL; Crv = Crv -> Pnext);
	    Crv -> Pnext = Coverage;
	    Coverage = OneCoverage;
	    fprintf(stderr, "Done with one ruled surface approximation.\n");
	}
	CagdSrfFreeList(RuledSrfs);
	CagdSrfFree(NormalSrf);
	if (!IsBspline)
	    CagdSrfFree(Srf);

	/* Scan the adaptive isoline list. Note that normal curve is paired */
	/* after Euclidean curve, so we can step two curves at a time.      */
	for (Crv = Coverage; Crv != NULL; Crv = Crv -> Pnext -> Pnext) {
	    CagdCrvStruct
		*NCrv = Crv -> Pnext;
	    IPPolygonStruct
		*Poly = IPCurve2Polylines(Crv, 1, IGGlblPolylineOptiApprox);

	    if (Poly != NULL) {
		IrtRType
		    *Nrml1 = Poly -> PVertex -> Normal,
		    *Nrml2 = Poly -> PVertex -> Pnext -> Normal;
		CagdCoerceToE3(Nrml1, NCrv -> Points, 0, NCrv -> PType);
		CagdCoerceToE3(Nrml2, NCrv -> Points, NCrv -> Length - 1,
			       NCrv -> PType);
		PT_NORMALIZE(Nrml1);
		PT_SCALE(Nrml1, -1.0);
		PT_NORMALIZE(Nrml2);
		PT_SCALE(Nrml2, -1.0);

		Poly -> Pnext = IsoPoly;
		IsoPoly = Poly;
	    }
	}
    }
    else {
	NSrf = SymbSrfNormalSrf(Srf);

	Coverage = SymbAdapIsoExtract(Srf, NSrf, NULL, Dir, Eps, FALSE, FALSE);
	    fprintf(stderr,
		    "Done with adaptive isocurve surface approximation.\n");

	CagdSrfFree(NSrf);

	/* Scan the adaptive isoline list. Note that normal curve is paired */
	/* after Euclidean curve, so we can step two curves at a time.      */
	for (Crv = Coverage; Crv != NULL; Crv = Crv -> Pnext -> Pnext) {
	    IPVertexStruct *VP, *VN;
	    IPPolygonStruct
		*Poly = IPCurve2Polylines(Crv, IGGlblPllnFineness,
					  IGGlblPolylineOptiApprox),
		*NPoly = IPCurve2Polylines(Crv -> Pnext,
					   IGGlblPllnFineness,
					   IGGlblPolylineOptiApprox);

	    if (Poly != NULL && NPoly != NULL) {
		for (VP = Poly -> PVertex, VN = NPoly -> PVertex;
		     VP != NULL;
		     VP = VP -> Pnext, VN = VN -> Pnext) {
		    PT_COPY(VP -> Normal, VN -> Coord);
		    PT_NORMALIZE(VP -> Normal);
		    PT_SCALE(VP -> Normal, -1.0);
		}

		IPFreePolygonList(NPoly);

		Poly -> Pnext = IsoPoly;
		IsoPoly = Poly;
	    }
	}
    }

    CagdCrvFreeList(Coverage);

    return IsoPoly;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the color of an object according to its color/rgb attributes.	     M
*   If object has an RGB attribute it will be used. Otherwise, if the object M
* has a COLOR attribute it will use. Otherwise, WHITE will be used.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      To set the drawing color to its color.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetColorObj                                                            M
*****************************************************************************/
void IGSetColorObj(IPObjectStruct *PObj)
{
    int c, Color[3];

    if (AttrGetObjectRGBColor(PObj, &Color[0], &Color[1], &Color[2])) {
	IGSetColorRGB(Color);
    }
    else if ((c = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR) {
	SetColorIndex(c);
    }
    else {
	/* Use white as default color: */
	SetColorIndex(IG_IRIT_WHITE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the light source index Index to a new position/location.         M
*                                                                            *
* PARAMETERS:                                                                M
*   LightPos:       New location of light source. (0, 0, 0) disables         M
*		    light source.  If NULL, refresh all light sources color. M
*   LightColor:     Color of light source.				     M
*   LightIndex:     Index of light source in Open GL. -1 will alocate the    M
*		    next available slot.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetLightSource                                                         M
*****************************************************************************/
void IGSetLightSource(IGLightType LightPos,
		      IrtVecType LightColor,
		      int LightIndex)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the line width to draw the given object, in pixels.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Width:    In pixels of lines to draw with.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetWidthObj                                                            M
*****************************************************************************/
void IGSetWidthObj(int Width)
{
    linewidth(Width);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the line pattern to draw the given object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pattern:    The line pattern to use.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetLinePattern                                                         M
*****************************************************************************/
void IGSetLinePattern(int Pattern)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the opacity level to draw the given object.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Transparency:    Level of transparency - 1 full, 0 opaque.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetTranspObj                                                           M
*****************************************************************************/
void IGSetTranspObj(IrtRType Transparency)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepares the texture mapping function of an object.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to apply texture to.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if succesful, FALSE otherwise.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetTexture                                                             M
*****************************************************************************/
int IGSetTexture(IPObjectStruct *PObj)
{
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets the color according to the given color index.		    	     *
*                                                                            *
* PARAMETERS:                                                                *
*   color:     Index of color to use. Must be between 0 and IG_MAX_COLOR.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetColorIndex(int color)
{
    int Color[3];

    if (color < 0 || color > IG_MAX_COLOR)
        color = IG_IRIT_WHITE;

    Color[0] = AttrIritColorTable[color][0];
    Color[1] = AttrIritColorTable[color][1];
    Color[2] = AttrIritColorTable[color][2];

    IGSetColorRGB(Color);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the color according to the given RGB values.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Color:      An RGB vector of integer values between 0 and 255.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetColorRGB                                                            M
*****************************************************************************/
void IGSetColorRGB(int Color[3])
{
    int i;
    STATIC_DATA float
	Ambient = 0.25,
	Diffuse = 0.75,
	Specular = 1.0;
    STATIC_DATA float
	Material[] = {
	    AMBIENT,  0.5, 0.5, 0.5,
	    DIFFUSE,  0.75, 0.75, 0.75,
	    SPECULAR, 1.00, 1.00, 1.00,
	    SHININESS, 50,
	    LMNULL
	};

    c3i((long *) Color);
    if (IGGlblDepthCue)
	lRGBrange(0, 0, 0, Color[0], Color[1], Color[2], 0x0, 0x7fffff);

    /* Prepare material structure in this color and select it. */
    for (i = 0; i < 3; i++) {
	Material[1 + i] = Ambient * Color[i] / 255.0;
	Material[5 + i] = Diffuse * Color[i] / 255.0;
	Material[9 + i] = Specular * Color[i] / 255.0;
    }
    lmdef(DEFMATERIAL, 1, sizeof(Material) / sizeof(float), Material);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets up and draw a transformation window.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   None								     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetTransformWindow(void)
{
    long PrefPos[4];

#ifndef _AIX
    foreground();
#endif

    if (sscanf(IGGlblTransPrefPos, "%ld,%ld,%ld,%ld",
	       &PrefPos[0], &PrefPos[1], &PrefPos[2], &PrefPos[3]) == 4)
	prefposition(PrefPos[0], PrefPos[1], PrefPos[2], PrefPos[3]);
    else if (sscanf(IGGlblTransPrefPos, "%ld,%ld",
		    &PrefPos[0], &PrefPos[1]) == 2)
	prefsize(PrefPos[0], PrefPos[1]);
    winopen("Poly3dTrans");
    winconstraints();
    wintitle("xGLdrvs");
    RGBmode();
    gconfig();
    getorigin(&TransWinLeft, &TransWinLow);
    getsize(&TransWinWidth, &TransWinHeight);
    TransWinWidth2 = TransWinWidth / 2;
    TransWinID = winget();

    IGSetColorRGB(IGGlblBackGroundColor);
    clear();

    /* This is wierd. without the sleep the gl get mixed up between the two  */
    /* windows. If you have any idea why, let me know...		     */
    sleep(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraws the transformation window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawTransformWindow(void)
{
    int i;
    long SubTransPosX, SubTransPosY, SubTransWidth, SubTransHeight;

    /* Make sure the menu is consistent with internatal data. */
    InteractMenu.SubWindows[0].Str =
	IGGlblTransformMode == IG_TRANS_OBJECT ? "Object Coords."
					     : "Screen Coords.";
    InteractMenu.SubWindows[1].Str =
	IGGlblViewMode == IG_VIEW_PERSPECTIVE ? "Perspective" : "Orthographic";

    winset(TransWinID);                /* Draw in the transformation window. */

    SubTransWidth = (int) (TransWinWidth * INTERACT_SUB_WINDOW_WIDTH);
    SubTransHeight = (int) (TransWinHeight * INTERACT_SUB_WINDOW_HEIGHT);
    SubTransPosX = (TransWinWidth - SubTransWidth) / 2;

    IGSetColorRGB(IGGlblBackGroundColor);
    clear();

    for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
	SetColorIndex(InteractMenu.SubWindows[i].Color);
	SubTransPosY = (int) (TransWinHeight * InteractMenu.SubWindows[i].Y);

	move2i(SubTransPosX, SubTransPosY);
	draw2i(SubTransPosX + SubTransWidth, SubTransPosY);
	draw2i(SubTransPosX + SubTransWidth, SubTransPosY + SubTransHeight);
	draw2i(SubTransPosX, SubTransPosY + SubTransHeight);
	draw2i(SubTransPosX, SubTransPosY);
	if (InteractMenu.SubWindows[i].TextInside) {
	    DrawText(InteractMenu.SubWindows[i].Str,
		     TransWinWidth / 2,
		     SubTransPosY + SubTransHeight / 2);
	}
	else {
	    DrawText(InteractMenu.SubWindows[i].Str,
		     (TransWinWidth - SubTransWidth) / 3,
		     SubTransPosY + SubTransHeight / 2);
	    move2i(SubTransPosX + SubTransWidth / 2, SubTransPosY);
	    draw2i(SubTransPosX + SubTransWidth / 2,
		   SubTransPosY + SubTransHeight);
	}
    }

    for (i = 0; i < INTERACT_NUM_OF_STRINGS; i++) {
	SetColorIndex(InteractMenu.Strings[i].Color);
	DrawText(InteractMenu.Strings[i].Str,
		 (int) (InteractMenu.Strings[i].X * TransWinWidth),
		 (int) (InteractMenu.Strings[i].Y * TransWinHeight));
    }

    winset(ViewWinID);             /* Go back to the default drawing window. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Clears the viewing area.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ClearViewArea(void)
{
    IGSetColorRGB(IGGlblBackGroundColor);
    clear();
    mmode(MVIEWING);
    shademodel(GOURAUD);

    if (APX_EQ(IGGlblZMinClip, IGGlblZMaxClip))
        IGGlblZMaxClip += IRIT_EPS;
    ortho(-1.0, 1.0, -1.0, 1.0, IGGlblZMinClip, IGGlblZMaxClip);

    if (winget() == ViewWinID) {
	/* activate zbuffer only if we are in solid drawing mode. */
	if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
	    /* Define necessary staff for Lighting. */
	    lmbind(MATERIAL, 1);
	    lmbind(LMODEL, 1);
	    lmbind(LIGHT1, 1);

	    zbuffer(TRUE);
	    zclear();
	}
	else {
	    zbuffer(FALSE);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets up a view window.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   argc, argv:   Command line,                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   SetViewWindow                                                            *
*****************************************************************************/
static void SetViewWindow(void)
{
    long PrefPos[4];
    STATIC_DATA int
	UpdateLightPos = FALSE;
    STATIC_DATA float Light1[] = {
	AMBIENT, 0.25, 0.25, 0.25,
	POSITION, 0.1, 0.5, 1.0, 0.0,
	LMNULL
    };

    if (!UpdateLightPos) {
	int i;

	for (i = 0; i < 4; i++)
	    Light1[i + 5] = IGShadeParam.LightPos[0][i];

	UpdateLightPos = TRUE;
    }

#ifndef _AIX
    foreground();
#endif

    if (sscanf(IGGlblViewPrefPos, "%ld,%ld,%ld,%ld",
	       &PrefPos[0], &PrefPos[1], &PrefPos[2], &PrefPos[3]) == 4)
	prefposition(PrefPos[0], PrefPos[1], PrefPos[2], PrefPos[3]);
    else if (sscanf(IGGlblViewPrefPos, "%ld,%ld",
		    &PrefPos[0], &PrefPos[1]) == 2)
	prefsize(PrefPos[0], PrefPos[1]);
    winopen("Poly3dView");
    winconstraints();
    wintitle("xGLdrvs");
    if (IGGlblDoDoubleBuffer)
	doublebuffer();
    else
        singlebuffer();
    RGBmode();
    if (IGGlblDepthCue) {
#	ifndef _AIX
	    glcompat(GLC_ZRANGEMAP, 1);
#	endif /* _AIX */
	lRGBrange(0, 0, 0, 255, 255, 255, 0x0, 0x7fffff);
	depthcue(IGGlblDepthCue);
    }
    else
        depthcue(FALSE);
    if (IGGlblAntiAliasing == IG_STATE_ANTI_ALIAS_ON) {
#	ifndef _AIX
	    if (getgdesc(GD_PNTSMOOTH_CMODE) == 0 ||
		getgdesc(GD_BITS_NORM_DBL_CMODE) < 8)
		IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_OFF;
	    else {
		subpixel(TRUE);
		pntsmooth(SMP_ON);
	    }
#	endif /* _AIX */
    }
    gconfig();
    getorigin(&ViewWinLeft, &ViewWinLow);
    getsize(&ViewWinWidth, &ViewWinHeight);
    
    ViewWinID = winget();

    concave(FALSE);

    /* Define necessary staff for Lighting. */
    lmdef(DEFMATERIAL, 1, 0, NULL);
    lmdef(DEFLMODEL, 1, 0, NULL);
    lmdef(DEFLIGHT, 1, sizeof(Light1) / sizeof(float), Light1);

    ClearViewArea();
    if (IGGlblDoDoubleBuffer)
	swapbuffers();

    /* This is wierd. without the sleep the gl get mixed up between the two  */
    /* windows. If you have any idea why, let me know...		     */
    sleep(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Redraw the view window.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedrawViewWindow                                                       M
*****************************************************************************/
void IGRedrawViewWindow(void)
{
    char Line[LINE_LEN_LONG];

    if (IGGlblCountNumPolys)
	IGGlblNumPolys = 0;
    if (IGGlblCountFramePerSec)
        IGUpdateFPS(TRUE);

    IGPredefinedAnimation();

    ClearViewArea();

    switch (IGGlblViewMode) {		 /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    GEN_COPY(IGGlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IGGlblCrntViewMat, IPViewMat, IPPrspMat);
	    break;
    }
    MatInverseMatrix(IGGlblCrntViewMat, IGGlblInvCrntViewMat);

    IGTraverseObjListHierarchy(IGGlblDisplayList, IGGlblCrntViewMat,
			       IGViewObject);

    Line[0] = 0;
    if (IGGlblCountNumPolys) {
	sprintf(Line, ", Rendered %d polygons", IGGlblNumPolys);
    }
    if (IGGlblCountFramePerSec) {
        IGUpdateFPS(FALSE);
        sprintf(&Line[strlen(Line)], ", FPS: %.1f", IGGlblFramePerSec);
    }
    if (!IRT_STR_ZERO_LEN(Line)) {
	fprintf(stderr, "\t%s%s           \r", RESOURCE_NAME, Line);
    }

    if (IGGlblDoDoubleBuffer)
	swapbuffers();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Call back function of the IGTraverseObjListHierarchy above.              M
*   Also capable of displaying an object, given the global viewing matrix.   M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to display.                                            M
*   Mat:       Viewing matrix of object.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGViewObject	                                                     M
*****************************************************************************/
void IGViewObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    int i, j;
    Matrix CrntView;
    IrtHmgnMatType TMat;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    CrntView[i][j] = Mat[i][j];
    loadmatrix(CrntView);

    HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    HMGN_MAT_COPY(IGGlblCrntViewMat, Mat);

    IGDrawObject(PObj);

    HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Handles input events                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   ChangeFactor:        A continuous numeric value between -1 and 1. This   *
*			 value will be used to set amount of event such as   *
*			 rotation or translation. In some events it can      *
*			 contain a vector value.			     *
*   BlockForEvent:	 If TRUE, blocks until event is recieved.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IGGraphicEventType:  Type of new event.                                  *
*****************************************************************************/
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor,
					  int BlockForEvent)
{
    STATIC_DATA IGGraphicEventType
	LastEvent = IG_EVENT_NONE;
    STATIC_DATA int
	ShiftPressed = FALSE;
    STATIC_DATA long
	LastY = -1,
	LastX = -1;
    int i, TransformRequest,
	RightButtonIsPressed = getbutton(RIGHTMOUSE) == 1,
        LeftButtonIsPressed = getbutton(LEFTMOUSE) == 1;
    IGGraphicEventType
	RetVal = IG_EVENT_NONE;
    short data;
    long y,
	x = -1;
    IrtRType XPos, YPos;

    ChangeFactor[0] = ChangeFactor[1] = 0.0;

    /* Allow continuous drag on following events only: */
    if (LeftButtonIsPressed && !IG_IS_DRAG_EVENT(LastEvent)) {
	while (getbutton(LEFTMOUSE) == 1);
	LeftButtonIsPressed = FALSE;
    }
    if (RightButtonIsPressed && !IG_IS_DRAG_EVENT(LastEvent)) {
	while (getbutton(RIGHTMOUSE) == 1);
	RightButtonIsPressed = FALSE;
    }

    if (LeftButtonIsPressed) {
	/* Allow leaving the window if still pressed, and use last event     */
	/* as the returned event. Note we wait until current position is     */
	/* different from last one to make sure we do something.             */
	switch (LastEvent) {
	    case IG_EVENT_PERS_ORTHO_Z:
	    case IG_EVENT_ROTATE_X:
	    case IG_EVENT_ROTATE_Y:
	    case IG_EVENT_ROTATE_Z:
	    case IG_EVENT_TRANSLATE_X:
	    case IG_EVENT_TRANSLATE_Y:
	    case IG_EVENT_TRANSLATE_Z:
	    case IG_EVENT_SCALE:
	    case IG_EVENT_NEAR_CLIP:
	    case IG_EVENT_FAR_CLIP:
	        while ((x = getvaluator(MOUSEX)) == LastX &&
		       getbutton(LEFTMOUSE) == 1);

	        if (x != LastX) {
		    *ChangeFactor = (((IrtRType) x) - LastX) / TransWinWidth2;
		    LastX = x;
		    return LastEvent;
		}
		else
		    LeftButtonIsPressed = FALSE;
		break;
	    case IG_EVENT_ROTATE:
	        while ((x = getvaluator(MOUSEX)) == LastX &&
		       (y = getvaluator(MOUSEY)) == LastY &&
		       getbutton(LEFTMOUSE) == 1);
		x = getvaluator(MOUSEX);
		y = getvaluator(MOUSEY);

	        if (x != LastX || y != LastY) {
		    ChangeFactor[0] = (x - LastX);
		    ChangeFactor[1] = (y - LastY);

		    LastX = x;
		    LastY = y;
		    return LastEvent;
		}
		else
		    LeftButtonIsPressed = FALSE;
		break;
	    default:
		break;
	}
    }

    if (RightButtonIsPressed) {
	/* Allow leaving the window if still pressed, and use last event     */
	/* as the returned event. Note we wait until current position is     */
	/* different from last one to make sure we do something.             */
	switch (LastEvent) {
	    case IG_EVENT_TRANSLATE:
	        while ((x = getvaluator(MOUSEX)) == LastX &&
		       (y = getvaluator(MOUSEY)) == LastY &&
		       getbutton(RIGHTMOUSE) == 1);
		x = getvaluator(MOUSEX);
		y = getvaluator(MOUSEY);

		if (x != LastX || y != LastY) {
		    ChangeFactor[0] = (x - LastX);
		    ChangeFactor[1] = (y - LastY);

		    LastX = x;
		    LastY = y;
		    return LastEvent;
		}
		else
		    RightButtonIsPressed = FALSE;
	    default:
		break;
	}
    }

    LastEvent = IG_EVENT_NONE;

    do {
	/* Wait for left button to be pressed in the Trans window. Note this */
	/* is the loop we are going to cycle in idle time.		     */
	for (TransformRequest = FALSE; !TransformRequest;) {
	    x = getvaluator(MOUSEX);
	    y = getvaluator(MOUSEY);

	    if (qtest()) {	              /* Any external event occured? */
		switch (qread(&data)) {
		    case RIGHTSHIFTKEY:
		    case LEFTSHIFTKEY:
			ShiftPressed = data != 0;
		        break;
		    case REDRAW:
			if (data == ViewWinID) {
			    getorigin(&ViewWinLeft, &ViewWinLow);
			    getsize(&ViewWinWidth, &ViewWinHeight);
			    reshapeviewport();
			    ortho2(-0.5, ViewWinWidth - 0.5,
				   -0.5, ViewWinHeight - 0.5);
			    IGRedrawViewWindow();
			}
			else if (data == TransWinID) {
			    winset(TransWinID);
			    getorigin(&TransWinLeft, &TransWinLow);
			    getsize(&TransWinWidth, &TransWinHeight);
			    reshapeviewport();
			    ortho2(-0.5, TransWinWidth - 0.5,
				   -0.5, TransWinHeight - 0.5);
			    TransWinWidth2 = TransWinWidth / 2;
			    RedrawTransformWindow();
			    winset(ViewWinID);
			}
			break;
		    case RIGHTMOUSE:
			if (data) {			/* Mouse press down. */
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN3DOWN);
				continue;
			    }

			    if (IN_TRANS_WINDOW(x, y)) {
			        int Idx = dopup(GlblStateMenu);

				/* We jumped over index 13 to 26. */
				if (Idx >= 13)
				    Idx += 13;

				if (IGHandleState(Idx,
						  ShiftPressed ? IG_STATE_DEC
							       : IG_STATE_INC,
						  FALSE)) {
				    *ChangeFactor = 0.0;
				    return IG_EVENT_STATE;
				}
			    }
			    else if (IN_VIEW_WINDOW(x, y)) {
				RetVal = IG_EVENT_TRANSLATE;
				TransformRequest = TRUE;
			    }				
			}
			else {
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN_UP);
				continue;
			    }
			}
			break;
		    case MIDDLEMOUSE:
			if (data) {			/* Mouse press down. */
			    IPObjectStruct *PObj;

			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN2DOWN);
				continue;
			    }

			    PObj = IGHandlePickEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
					    IG_PICK_ANY);

			    if (ReturnPickedObject) {
			        IPObjectStruct *PObjDump;

				if (PObj != NULL) {
				    if (ReturnPickedObjectName)
				        PObjDump = IPGenStrObject("_PickName_",
								  PObj -> Name,
								  NULL);
				    else
				        PObjDump = PObj;
				}
				else
				    PObjDump = IPGenStrObject("_PickFail_",
							   "*** no object ***",
							   NULL);

				IPSocWriteOneObject(IGGlblIOHandle, PObjDump);
				if (PObj != PObjDump)
				    IPFreeObject(PObjDump);
			    }
			    else 
			        printf("Pick event found \"%s\"          \r",
					PObj == NULL ? "nothing" : PObj -> Name);
			}
			else {
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN_UP);
				continue;
			    }
			}
			break;
		    case LEFTMOUSE:
			if (data) {			/* Mouse press down. */
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN1DOWN);
				continue;
			    }

			    TransformRequest = TRUE;

			    if (IN_VIEW_WINDOW(x, y)) {
				RetVal = IG_EVENT_ROTATE;
			    }				
			}
			else {
			    if (PickCrsrGrabMouse) {
			        IGHandleCursorEvent(
					    getvaluator(MOUSEX) - ViewWinLeft,
					    getvaluator(MOUSEY) - ViewWinLow,
				            IG_PICK_REP_BTN_UP);
				continue;
			    }
			}
			break;
		}
		continue;
	    }

	    /* Maybe we have something in communication socket. */
	    if (!IGGlblStandAlone &&
		IGReadObjectsFromSocket(IGGlblViewMode, &IGGlblDisplayList))
		IGRedrawViewWindow();

	    if (BlockForEvent) {
	        if (IGGlblContinuousMotion)
		    IGHandleContinuousMotion();
		else
		    IritSleep(10);     /* So we do not use all CPU on idle. */
	    }
	    else
		break;
	}

	if (!TransformRequest && !BlockForEvent)
	    return RetVal;

	LastX = x;
	LastY = y;

	if (IN_TRANS_WINDOW(x, y)) {
	    x -= TransWinLeft;
	    y -= TransWinLow;

	    XPos = ((IrtRType) x) / TransWinWidth;
	    YPos = ((IrtRType) y) / TransWinHeight;

	    /* Make sure we are in bound in the X direction. */
	    if (XPos < (1.0 - INTERACT_SUB_WINDOW_WIDTH) / 2.0 ||
		XPos > 1.0 - (1.0 - INTERACT_SUB_WINDOW_WIDTH) / 2.0)
	        continue;

	    /* Now search the sub window the event occured in. */
	    for (i = 0; i < INTERACT_NUM_OF_SUB_WNDWS; i++) {
		if (InteractMenu.SubWindows[i].Y <= YPos &&
		    InteractMenu.SubWindows[i].Y +
		        INTERACT_SUB_WINDOW_HEIGHT >= YPos) {
		    RetVal = InteractMenu.SubWindows[i].Event;
		    break;
		}
	    }
	    if (i == INTERACT_NUM_OF_SUB_WNDWS)
	        continue;

	    /* Take care of special cases in which window should be updated. */
	    switch (RetVal) {
		case IG_EVENT_SCR_OBJ_TGL:
		    IGGlblTransformMode =
		        IGGlblTransformMode == IG_TRANS_OBJECT ?
					       IG_TRANS_SCREEN :
					       IG_TRANS_OBJECT;
		    RedrawTransformWindow();
		    break;
		case IG_EVENT_PERS_ORTHO_TGL:
	            IGGlblViewMode = IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
						       IG_VIEW_ORTHOGRAPHIC :
						       IG_VIEW_PERSPECTIVE;
		    RedrawTransformWindow();
		    break;
		default:
		    break;
	    }

	    *ChangeFactor = (((IrtRType) x) - TransWinWidth2) / TransWinWidth2;
	}
    }
    while (RetVal == IG_EVENT_NONE);

    LastEvent = RetVal;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Handles the events of the pop up window.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       Event to handle.                                            M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*		 on or toggling current value. 				     M
*		 IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Do we need to refresh the screen according to what we know  M
*		 on entry.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if we need to refresh the screen.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleState                                                            M
*****************************************************************************/
int IGHandleState(int State, int StateStatus, int Refresh)
{
    int UpdateView = TRUE;

    switch (State) {
	case IG_STATE_SCR_OBJ_TGL:
	    IGStateHandler(State, StateStatus, Refresh);
	    RedrawTransformWindow();
	    UpdateView = FALSE;
	    break;
	case IG_STATE_PERS_ORTHO_TGL:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    RedrawTransformWindow();
	    break;
	case IG_STATE_DEPTH_CUE:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    if (IGGlblDepthCue) {
#	        ifndef _AIX
		    glcompat(GLC_ZRANGEMAP, 1);
#	        endif /* _AIX */
		lRGBrange(0, 0, 0, 255, 255, 255,
			  0x0, 0x7fffff);
		depthcue(IGGlblDepthCue);
	    }
	    else
		depthcue(FALSE);
	    break;
	case IG_STATE_DOUBLE_BUFFER:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    winset(ViewWinID);
	    if (IGGlblDoDoubleBuffer)
		doublebuffer();
	    else
		singlebuffer();
	    gconfig();
	    break;
	case IG_STATE_ANTI_ALIASING:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
#	    ifndef _AIX
		if (getgdesc(GD_PNTSMOOTH_CMODE) == 0 ||
		    getgdesc(GD_BITS_NORM_DBL_CMODE) < 8)
		    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_OFF;
		else
		    subpixel(1);
#	    endif /* _AIX */
	    break;
	case IG_STATE_POLY_APPROX:
	    if (StateStatus == IG_STATE_INC) {
	        IGGlblPllnFineness++;
		IGGlblPlgnFineness++;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        IGGlblPlgnFineness--;
		if (IGGlblPlgnFineness < 2)
		    IGGlblPlgnFineness = 2;
		IGGlblPllnFineness--;
		if (IGGlblPllnFineness < 2)
		    IGGlblPllnFineness = 2;
	    }
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     TRUE, TRUE, TRUE, FALSE);
	    IGActiveListFreeNamedAttribute(IGGlblDisplayList, "_adap_iso");
	    break;
	case IG_STATE_RES_ADAP_ISO:
	    if (StateStatus == IG_STATE_INC) {
	        GlblAdapIsoEps /= 2.0;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        GlblAdapIsoEps *= 2.0;
	    }
	    IGActiveListFreeNamedAttribute(IGGlblDisplayList, "_adap_iso");
	    break;
	case IG_STATE_RES_RULED_SRF:
	    if (StateStatus == IG_STATE_INC) {
	        GlblRuledSrfEps /= 2.0;
	    }
	    else if (StateStatus == IG_STATE_DEC) {
	        GlblRuledSrfEps *= 2.0;
	    }
	    IGActiveListFreeNamedAttribute(IGGlblDisplayList, "_adap_iso");
	    break;
	case IG_STATE_RULED_SRF_APPROX:
	    if (StateStatus == IG_STATE_TGL)
	        GlblRuledSrfApprox = !GlblRuledSrfApprox;
	    else 
		GlblRuledSrfApprox = StateStatus == IG_STATE_ON ? TRUE : FALSE;
	    IGActiveListFreeNamedAttribute(IGGlblDisplayList, "_adap_iso");
	    break;
	case IG_STATE_ADAP_ISO_DIR:
	    if (StateStatus == IG_STATE_TGL) {
	        if (IGGlblAdapIsoDir == CAGD_CONST_U_DIR)
		    IGGlblAdapIsoDir = CAGD_CONST_V_DIR;
		else
		    IGGlblAdapIsoDir = CAGD_CONST_U_DIR;
	    }
	    else if (StateStatus == IG_STATE_ON)
	        IGGlblAdapIsoDir = CAGD_CONST_V_DIR;
	    else if (StateStatus == IG_STATE_OFF)
	        IGGlblAdapIsoDir = CAGD_CONST_U_DIR;

	    IGActiveListFreeNamedAttribute(IGGlblDisplayList, "_adap_iso");
	    break;
	default:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
    }

    if (Refresh && UpdateView)
	IGRedrawViewWindow();

    IGCreateStateMenu();

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draws text centered at the given position.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:        String to draw.                                              *
*   PosX, PosY: Location to draw at.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawText(char *Str, long PosX, long PosY)
{
    long Width = strwidth(Str);

    cmov2s((Scoord) (PosX - Width / 2),
	   (Scoord) (PosY - (getheight() / 2 - getdescender())));
    charstr(Str);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make some sound.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritBeep                                                               M
*****************************************************************************/
void IGIritBeep(void)
{
    ringbell();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle pick events.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY: Screen coordinates of pick event.                      M
*   PickTypes:	      Types of object to pick or IG_PICK_ANY for any object. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Picked object or NULL if none.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandleCursorEvent                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandlePickEvent                                                        M
*****************************************************************************/
IPObjectStruct *IGHandlePickEvent(int ScreenX, int ScreenY, int PickTypes)
{
    return IGHandleGenericPickEvent((ScreenX * 2.0 - ViewWinWidth) /
								ViewWinWidth,
				    (ScreenY * 2.0 - ViewWinHeight) /
							        ViewWinHeight,
				    PickTypes);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports to the server on a pick event of the cursor/mouse.               M
* The reported object is a list object of a point and a vector defining the  M
* cursor line in 3-space.  The event type is returned as an "EventType"      M
* attribute on the reported object.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY:   Location of the cursor, in screen space coords.      M
*   PickReport:         Type of event: motion, button down, etc.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGHandlePickEvent                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleCursorEvent                                                      M
*****************************************************************************/
void IGHandleCursorEvent(int ScreenX, int ScreenY, IGPickReportType PickReport)
{
    IGHandleGenericCursorEvent((ScreenX * 2.0 - ViewWinWidth) / ViewWinWidth,
			       (ScreenY * 2.0 - ViewWinHeight) / ViewWinHeight,
			       PickReport);
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
*   IGScreenToObject                                                         M
*****************************************************************************/
void IGScreenToObject(int ScreenX, int ScreenY, IrtPtType Pt, IrtVecType Dir)
{
    int i;
    IrtRType t;

    IGGenericScreenToObject((ScreenX * 2.0 - ViewWinWidth) / ViewWinWidth,
			    (ScreenY * 2.0 - ViewWinHeight) / ViewWinHeight,
			    Pt, Dir);

    /* Find the intersection of the ray with the XY plane (Z == 0). */
    if (FABS(Dir[2]) < IRIT_UEPS)
	t = -Pt[2] / IRIT_UEPS;
    else
	t = -Pt[2] / Dir[2];

    for (i = 0; i < 3; i++)
	Pt[i] += Dir[i] * t;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pick an object from the display.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   PickEntity:  Type of entity to pick (object, cursor etc.).               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandlePickObject                                                       M
*****************************************************************************/
void IGHandlePickObject(IGPickEntityType PickEntity)
{
    STATIC_DATA int
	FirstTime = TRUE;

    if (FirstTime) {
	STATIC_DATA unsigned short Cross1Cursor[16] = {
	    0x0240, 0x0240, 0x0240, 0x0240,
	    0x0240, 0x0240, 0xffff, 0x0240,
	    0x0240, 0xffff, 0x0240, 0x0240,
	    0x0240, 0x0240, 0x0240, 0x0240,
	};
	STATIC_DATA unsigned short Cross2Cursor[16] = {
	    0x8001, 0x4002, 0x2004, 0x1008,
	    0x0810, 0x0420, 0x0240, 0x0180,
	    0x0180, 0x0240, 0x0420, 0x0810,
	    0x1008, 0x2004, 0x4002, 0x8001,
	};

	drawmode(CURSORDRAW);
	mapcolor(1, 255, 0, 0);
	defcursor(1, Cross1Cursor);
	curorigin(1, 8, 8);
	defcursor(2, Cross2Cursor);
	curorigin(2, 8, 8);
	drawmode(NORMALDRAW);
    }

    switch (PickEntity) {
	case IG_PICK_ENTITY_DONE:
	    /* Restore the cursor. */
            setcursor(0, 0, 0);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = FALSE;
	    break;
	case IG_PICK_ENTITY_OBJECT:
	case IG_PICK_ENTITY_OBJ_NAME:
	    /* Set our own object picking cursor: */
            setcursor(1, 0, 0);

	    PickCrsrGrabMouse = FALSE;
	    ReturnPickedObject = TRUE;
	    ReturnPickedObjectName = PickEntity == IG_PICK_ENTITY_OBJ_NAME;
	    break;
	case IG_PICK_ENTITY_CURSOR:
	    /* Set our own cursor picking cursor: */
            setcursor(2, 0, 0);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = TRUE;
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation. Senses the event queue of X11.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we need to abort, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimCheckInterrupt                                                     M
*****************************************************************************/
int GMAnimCheckInterrupt(GMAnimationStruct *Anim)
{
    if (qtest()) {
	short data;
	long
	    dev = qread(&data);

	if (dev == RIGHTMOUSE) {
	    fprintf(stderr, "\nAnimation was interrupted by the user.\n");

	    Anim -> StopAnim = TRUE;
	}
	else if (dev == LEFTMOUSE) {
	    IGGraphicEventType Event;
	    IrtRType ChangeFactor[2];

	    qenter((Device) dev, data);

	    winset(TransWinID);
	    Event = GetGraphicEvent(ChangeFactor, FALSE);
	    ChangeFactor[0] *= IGGlblChangeFactor;
	    ChangeFactor[1] *= IGGlblChangeFactor;
	    winset(ViewWinID);

	    if (Event != IG_EVENT_NONE) {
		if (IGProcessEvent(Event, ChangeFactor))
		    IGRedrawViewWindow();
	    }

	}
    }

    return Anim -> StopAnim;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve parameters.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvEditParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditPlaceMessage                                                    M
*****************************************************************************/
void IGCrvEditPlaceMessage(char *Msg)
{
    fprintf(stderr, "%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve parameters.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvCnstParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvCnstParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current surface parameters.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSrfEditParamUpdateWidget                                               M
*****************************************************************************/
void IGSrfEditParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSrfEditPlaceMessage                                                    M
*****************************************************************************/
void IGSrfEditPlaceMessage(char *Msg)
{
    fprintf(stderr, "%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current object transformation state.         M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipParamUpdateWidget                                              M
*****************************************************************************/
void IGObjManipParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipPlaceMessage                                                   M
*****************************************************************************/
void IGObjManipPlaceMessage(char *Msg)
{
    fprintf(stderr, "%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saves the current matrix in a selected file name.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:   Perspective or orthographics current view mode.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSaveCurrentMatInFile                                                   M
*****************************************************************************/
void IGSaveCurrentMatInFile(int ViewMode)
{
    IGSaveCurrentMat(ViewMode, IG_DEFAULT_IRIT_MAT);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make error message box in printf style.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritError                                                              M
*****************************************************************************/
void IGIritError(char *Msg)
{
    fprintf(stderr, "Error: %s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make yes/no message box.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Title message.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if yes was selected, FALSE otherwise.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritYesNoQuestion                                                      M
*****************************************************************************/
int IGIritYesNoQuestion(char *Msg)
{
    char Line[LINE_LEN];

    do {
        fprintf(stderr, "%s <Y/N>:", Msg);
	fgets(Line, LINE_LEN - 1, stdin);
    }
    while (Line[0] != 'y' && Line[0] != 'Y' &&
	   Line[0] != 'n' && Line[0] != 'N');
    
    return Line[0] == 'y' || Line[0] != 'Y';
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Saves one iteration of the animation sequence as an image.		     M
*									     *
* PARAMETERS:								     M
*   ImageFileName:  File name where to save the current display as an image. M
*									     *
* RETURN VALUE:								     M
*   void								     M
*									     *
* KEYWORDS:								     M
*   IGSaveDisplayAsImage						     M
*****************************************************************************/
void IGSaveDisplayAsImage(char *ImageFileName)
{
}
