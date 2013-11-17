/*****************************************************************************
*   Generic Open GL drawing functions.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "geom_lib.h"
#include "grap_loc.h"

#ifdef __UNIX__
#   include <Xm/Xm.h>
#   include <Xm/Form.h>
#   include <Xm/DialogS.h>
#   include <X11/cursorfont.h>
#   include <GL/glx.h>
#   include <GL/glu.h>
#   include <GL/gl.h>
#   include "xmtdrvs.h"
#endif /* __UNIX__ */

#if defined (__WINNT__)
#   include <windows.h>
#   include <gl/gl.h>
#   include <gl/glu.h>
#   ifdef IRIT_HAVE_OGL_CG_LIB
#       include <gl/glext.h>
#   endif /* IRIT_HAVE_OGL_CG_LIB */
#   include "wntdrvs.h"
#elif defined (__WINCE__)
#   include <windows.h>
#   include <GLES/gl.h>
#   include "oglesemu.h"
#   include "wntdrvs.h"
#endif /* __WINNT__ || __WINCE__ */

#ifdef IRIT_DOUBLE
#   define OGL_GL_NORMAL	glNormal3d
#   define OGL_GL_VRTX_V	glVertex3dv
#else
#   define OGL_GL_NORMAL	glNormal3f
#   define OGL_GL_VRTX_V	glVertex3fv
#endif /* IRIT_FLOAT */

#define OGL_HANDLE_NORMAL(Normal) { \
			if (IGGlblFlipNormalOrient) \
			    OGL_GL_NORMAL(-Normal[0], \
					  -Normal[1], \
					  -Normal[2]); \
			else \
			    OGL_GL_NORMAL(Normal[0], \
					  Normal[1], \
					  Normal[2]); }

#ifdef DEBUG
#define IRIT_TEST_OGL_ERROR() { \
    int i; \
    if ((i = glGetError()) != GL_NO_ERROR) \
        IRIT_WARNING_MSG_PRINTF("OGL Error: %s, file %s, line %d\n", \
		                gluErrorString(i), __FILE__, __LINE__); }
#else
#define IRIT_TEST_OGL_ERROR()
#endif /* DEBUG */

IRIT_GLOBAL_DATA IrtRType
    IGGlblOpacity = 1.0;

IRIT_STATIC_DATA int
    GlblTextureActive = FALSE;
IRIT_STATIC_DATA IGVertexHandleExtraFuncType
    GlblVertexHandleExtraFunc = NULL;

static void GetChromaModeColor(int *Color, IrtRType Pt);
static void HandleVertex(IPVertexStruct *V);
static void IGDrawPolyStrip(IPPolygonStruct *Pl);
static void SetColorIndex(int c);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set a function callback for vertex handling.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   NewVertexFunc:   New Vertex handling function to set.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IGVertexHandleExtraFuncType:  The old callback function pointer.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetHandleVertexProcessingFunc                                          M
*****************************************************************************/
IGVertexHandleExtraFuncType IGSetHandleVertexProcessingFunc(
				   IGVertexHandleExtraFuncType NewVertexFunc)
{
    IGVertexHandleExtraFuncType
	OldFunc = GlblVertexHandleExtraFunc;

    GlblVertexHandleExtraFunc = NewVertexFunc;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets the color of a vertex in Chroma depth color mode.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Color:  to set.                                                          *
*   Pt:     Z Position to set a color for.                                   *
*   Min:    Minimum depth.                                                   *
*   Max:    Maximum depth.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetChromaModeColor(int *Color, IrtRType Pt)
{
    IrtRType
	Part = (IGGlblZMaxClip - IGGlblZMinClip) / 4.75; 

    Pt = Pt - IGGlblZMinClip; 
    Color[0] = Color[1] = Color[2] = 0;

    /* We need to add a little black to prevent flickering so the Max       */
    /* value of a color is 225 and not 255.			            */
    if (Pt < 0)			                  /* When scene radius < 1. */
        Color[2] = 225;  
    else if (Pt <= Part) {			        /* Increasing blue. */
        Color[2] = 20 + (int) (205 * (Pt / Part));
	if (Color[2] < 20)
	    Color[2] = 20;				   /* For clipping. */
    }
    else if (Pt <= 2.1 * Part) {	 /* Full blue and increasing green. */
        Color[1] = (int) (225 * ((Pt - 1 * Part) / Part));
	Color[2] = 225;
    }
    else if (Pt <= 2.6 * Part) {         /* Full green and decreasing blue. */
        Color[1] = 225;
	Color[2] = 225 - (int) (225 * ((Pt - 2.1 * Part) / Part));
    }
    else if (Pt <= 3.1 * Part) {	  /* Full green and increasing red. */
        Color[0] = (int) (225 * ((Pt - 2.6 * Part) / Part));
	Color[1] = 225;
    }
    else if (Pt <= 4.5 * Part) {	  /* Full red and decreasing green. */
        Color[0] = 225;
	Color[1] = 225 - (int) (225 * ((Pt - 3.1 * Part) / Part)); 
    }
    else						       /* Full red. */
        Color[0] = 225;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize a dual color drawing mode.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   ImgIndx:  Index of image in dual screen (Red/Green etc.) drawing modes.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if needs to pop a pushed matrix.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedrawViewWinInitRedGreen                                              M
*****************************************************************************/
int IGRedrawViewWinInitRedGreen(int ImgIndx)
{
#ifndef __WINCE__
    IrtRType
	RBDistance = -2.0,
	Eye = 0.03;
    
    if (IGGlblViewMode == IG_VIEW_PERSPECTIVE)
        Eye = IGGlblEyeDistance;

    if (ImgIndx == 0 &&
	(IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_BLUE ||
	 IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_GREEN)) {/*Draw red image.*/
        glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE); 
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	gluLookAt(-Eye, 0, 0, 0, 0, (GLdouble) RBDistance, 0, 1, 0);
	return TRUE;
    }
    else if (ImgIndx == 1) {		      /* Draw the blue/green image. */
        if (IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_BLUE)
	    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_TRUE);
	else if (IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_GREEN)
	    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	gluLookAt(Eye, 0, 0, 0, 0, (GLdouble) RBDistance, 0, 1, 0);
	return TRUE;
    }
    else {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
        return FALSE;
    }
#else
    return FALSE;
#endif /* __WINCE__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Conduct the Open GL calls for redraw the view window.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   ClearAll: TRUE to clear both the Z buffer and RGB, FALSE only Z buffer.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedrawViewWindowOGL                                                    M
*****************************************************************************/
void IGRedrawViewWindowOGL(int ClearAll)
{
    IRIT_TEST_OGL_ERROR();

#ifndef __WINCE__
    glDrawBuffer(IGGlblDoDoubleBuffer ? GL_BACK : GL_FRONT);
#endif /* __WINCE__ */
    glClearColor((float) (IGGlblBackGroundColor[0] / 255.0),
		 (float) (IGGlblBackGroundColor[1] / 255.0),
		 (float) (IGGlblBackGroundColor[2] / 255.0),
		 1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, IGGlblZMinClip, IGGlblZMaxClip);

    glEnable(GL_NORMALIZE);   /* We do scaling extensively here. */

    if ((IGGlblAntiAliasing == IG_STATE_ANTI_ALIAS_ON &&
	 IGGlblDrawStyle == IG_STATE_DRAW_STYLE_WIREFRAME) ||
	(IGGlblAntiAliasing == IG_STATE_ANTI_ALIAS_BLEND &&
	 IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
    }
    else {
        glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_BLEND);
    }

    if (IGGlblBackFaceCull)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    glCullFace(GL_FRONT);          /* In irit we have reversed orientation. */

    if (IGGlblDepthCue)
        glEnable(GL_FOG);
    else
        glDisable(GL_FOG);

    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
        glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
    }

    switch (IGGlblViewMode) {                   /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    IRIT_GEN_COPY(IGGlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IGGlblCrntViewMat, IPViewMat, IPPrspMat);
	    break;
    }
    MatInverseMatrix(IGGlblCrntViewMat, IGGlblInvCrntViewMat);

    IGSetLightSource(NULL, NULL, -1);             /* Refresh light sources. */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (!IGGlblManipulationActive &&
	IGGlblDoDoubleBuffer &&
	IGGlblAntiAliasing != IG_STATE_ANTI_ALIAS_OFF &&
	IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
        static double
	    Jitter4[4][2] = {
		{  -0.57, -0.53 },
		{   0.57,  0.53 },
		{  -0.53,  0.57 },
		{   0.53, -0.57 }
	    };
	GLint ViewPort[4];
	int j;

	glGetIntegerv(GL_VIEWPORT, ViewPort);
#ifndef __WINCE__
	glClear(GL_ACCUM_BUFFER_BIT);
#endif /* __WINCE__ */

	for (j = 0; j < 4; j++) {
	    IrtHmgnMatType JitteredCrntViewMat, Mat;

	    if (j == 0) {   /* Clear screen/set background & clear ZBuffer. */
	        if (IGGlblBackGroundImage) {
		    if (ClearAll)
		        IGUpdateWindowPixelsFromBG();
		    glClear(GL_DEPTH_BUFFER_BIT);
		}
		else if (ClearAll)
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		else
		    glClear(GL_DEPTH_BUFFER_BIT);

		if (IGGlbl3DGlassesImgIndx >= 0)
		    IGRedrawViewWinInitRedGreen(IGGlbl3DGlassesImgIndx);
	    }

	    MatGenMatTrans(Jitter4[j][0] / ViewPort[2],
			   Jitter4[j][1] / ViewPort[3],
			   0.0, Mat);
	    MatMultTwo4by4(JitteredCrntViewMat, IGGlblCrntViewMat, Mat);

	    if (IGGlblPickedPolyObj != NULL)
		IGViewObject(IGGlblPickedPolyObj, IGGlblCrntViewMat);
	    IGTraverseObjListHierarchy(IGGlblDisplayList,
				       JitteredCrntViewMat,
				       IGViewObject);
	    if (IGGlblPickedPolyObj != NULL)/* Draw twice in case no Z test.*/
		IGViewObject(IGGlblPickedPolyObj, IGGlblCrntViewMat);
#ifndef __WINCE__
	    glAccum(GL_ACCUM, 0.25);
#endif /* __WINCE__ */
	}
#ifndef __WINCE__
	glAccum(GL_RETURN, 1);
#endif /* __WINCE__ */
    }
    else {
        if (IGGlblBackGroundImage) {
	    if (ClearAll)
	        IGUpdateWindowPixelsFromBG();
	    glClear(GL_DEPTH_BUFFER_BIT);
	}
	else if (ClearAll)
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
	    glClear(GL_DEPTH_BUFFER_BIT);

	if (IGGlbl3DGlassesImgIndx >= 0)
	    IGRedrawViewWinInitRedGreen(IGGlbl3DGlassesImgIndx);

	if (IGGlblPickedPolyObj != NULL)
	    IGViewObject(IGGlblPickedPolyObj, IGGlblCrntViewMat);
	IGTraverseObjListHierarchy(IGGlblDisplayList,
				   IGGlblCrntViewMat, IGViewObject);
	if (IGGlblPickedPolyObj != NULL)   /* Draw twice in case no Z test. */
	    IGViewObject(IGGlblPickedPolyObj, IGGlblCrntViewMat);
    }

    if (IGGlbl3DGlassesImgIndx >= 0) {
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    }

    glFlush();

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handle the positional as well as possible RGB attribute of a vertex.     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:      Vertex to process.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void HandleVertex(IPVertexStruct *V) 
{ 
    const char *Str;
    int Color[3];
    float *Uv;
    IrtPtType Pt;

    /* If we have a special vertex handler - proces with it. */
    if (GlblVertexHandleExtraFunc != NULL && GlblVertexHandleExtraFunc(V))
        return;

    switch (IGGlbl3DGlassesMode) {
	case IG_GLASSES_3D_NONE:
	default:
	    if ((Str = AttrGetStrAttrib(V -> Attr, "RGB")) != NULL &&
		sscanf(Str, "%d,%d,%d",
		       &Color[0], &Color[1], &Color[2]) == 3) {
	        glColor3ub((GLubyte) Color[0],
			   (GLubyte) Color[1],
			   (GLubyte) Color[2]);
	    }
            break;
	case IG_GLASSES_3D_CHROMADEPTH:
	    /* Get the vertex after transformations. */
	    MatMultPtby4by4(Pt, V -> Coord, IGGlblCrntViewMat);
	    GetChromaModeColor(Color, Pt[2]);
	    glColor3ub((GLubyte) Color[0],
		       (GLubyte) Color[1],
		       (GLubyte) Color[2]);
	    break;
	case IG_GLASSES_3D_RED_BLUE:
	    if (IGGlbl3DGlassesImgIndx == 0)
	        glColor3ub(255, 0, 0);
	    else
	        glColor3ub(0, 0, 255);
            break;
	case IG_GLASSES_3D_RED_GREEN:
            if (IGGlbl3DGlassesImgIndx == 0)
	        glColor3ub(255, 0, 0);
	    else
	        glColor3ub(0, 255, 0);
            break;
    }

    if (GlblTextureActive &&
	(Uv = AttrGetUVAttrib(V -> Attr, "uvvals")) != NULL)
        glTexCoord2f((GLfloat) Uv[0], (GLfloat) Uv[1]);

    OGL_GL_VRTX_V(V -> Coord);
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
    int i;
    IrtRType
	*Pt = PObj -> U.Pt;

    glDisable(GL_LIGHTING);

    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS) {
        float FPt[3];

	FPt[0] = (float) Pt[0];
	FPt[1] = (float) Pt[1];
	FPt[2] = (float) Pt[2];

        glVertexPointer(3, GL_FLOAT, 0, FPt);	

	glEnableClientState(GL_POINTS);
	glDrawArrays(GL_LINES, 0, 1);
	glDisableClientState(GL_POINTS);
    }
    else {
        static float
	    Zero[3] = { 0.0, 0.0, 0.0 };
        float Ends[2][3];

	for (i = 0; i < 3; i++) {
	    Ends[0][0] = Ends[1][0] = (float) Pt[0];
	    Ends[0][1] = Ends[1][1] = (float) Pt[1];
	    Ends[0][2] = Ends[1][2] = (float) Pt[2];

	    Ends[0][i] -= (float) IGGlblPointSize;
	    Ends[1][i] += (float) IGGlblPointSize;

	    glVertexPointer(3, GL_FLOAT, 0, Ends);	

	    glEnableClientState(GL_VERTEX_ARRAY);
	    glDrawArrays(GL_LINES, 0, 2);
	    glDisableClientState(GL_VERTEX_ARRAY);
	}

	if (IP_IS_VEC_OBJ(PObj)) {
	    Ends[0][0] = (float) Pt[0];
	    Ends[0][1] = (float) Pt[1];
	    Ends[0][2] = (float) Pt[2];
	    IRIT_GEN_COPY(Ends[1], Zero, sizeof(float) * 3);

	    glVertexPointer(3, GL_FLOAT, 0, Ends);	

	    glEnableClientState(GL_VERTEX_ARRAY);
	    glDrawArrays(GL_LINES, 0, 2);
	    glDisableClientState(GL_VERTEX_ARRAY);
	}
    }

    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
	glEnable(GL_LIGHTING);
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
    const char *Str;
    int Color[3];
    IPVertexStruct *V;
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    if (Pl == NULL)
        return;

    if (IP_IS_POLYLINE_OBJ(PObj)) {
        glDisable(GL_LIGHTING);
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if ((Str = AttrGetStrAttrib(Pl -> Attr, "RGB")) != NULL &&
		sscanf(Str, "%d,%d,%d",
		       &Color[0], &Color[1], &Color[2]) == 3) {
	        glColor3ub((GLubyte) Color[0],
			   (GLubyte) Color[1],
			   (GLubyte) Color[2]);
	    }

	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS)
	        glBegin(GL_POINTS);
	    else
	        glBegin(GL_LINE_STRIP);
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	        HandleVertex(V);
	    }
	    glEnd();
	}
	if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
	    glEnable(GL_LIGHTING);
    }
    else if (IP_IS_POINTLIST_OBJ(PObj)) {
        glDisable(GL_LIGHTING);
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if ((Str = AttrGetStrAttrib(Pl -> Attr, "RGB")) != NULL &&
		sscanf(Str, "%d,%d,%d",
		       &Color[0], &Color[1], &Color[2]) == 3) {
	        glColor3ub((GLubyte) Color[0],
			   (GLubyte) Color[1],
			   (GLubyte) Color[2]);
	    }

	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS) {
	        glBegin(GL_POINTS);
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    HandleVertex(V);
		}
		glEnd();
	    }
	    else {
	        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    int i;
		    IrtPtType Ends[6];
		    IrtRType
		        *Pt = V -> Coord;

		    for (i = 0; i < 6; i++)
		        IRIT_PT_COPY(Ends[i], Pt);

		    Ends[0][0] -= IGGlblPointSize;
		    Ends[1][0] += IGGlblPointSize;
		    Ends[2][1] -= IGGlblPointSize;
		    Ends[3][1] += IGGlblPointSize;
		    Ends[4][2] -= IGGlblPointSize;
		    Ends[5][2] += IGGlblPointSize;

		    for (i = 0; i < 6; i += 2) {
		        glBegin(GL_LINE_STRIP);
			OGL_GL_VRTX_V(Ends[i]);
			OGL_GL_VRTX_V(Ends[i + 1]);
			glEnd();
		    }
		}
	    }
	}
	if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
	    glEnable(GL_LIGHTING);
    }
    else if (IP_IS_POLYGON_OBJ(PObj) || IP_SET_POLYSTRIP_OBJ(PObj)) {
        int AllowPolyRGB = FALSE,
	    PolysFromSrf = AttrGetObjectIntAttrib(PObj, "_srf_polys") == TRUE;
	IrtPtType PNormal, VNormal;

	if (!((PolysFromSrf && !IGGlblDrawSurfacePoly) ||
	      (!PolysFromSrf && !IGGlblDrawPolygons))) {
	    if (IGGlblShadingModel == IG_SHADING_BACKGROUND) {
		glColor3d(IGGlblBackGroundColor[0] / 255.0,
			  IGGlblBackGroundColor[1] / 255.0,
			  IGGlblBackGroundColor[2] / 255.0);
		glDisable(GL_LIGHTING);
	    }
	    else if (IGGlblShadingModel == IG_SHADING_NONE) {
		glDisable(GL_LIGHTING);
	    }
	    else
	        AllowPolyRGB = TRUE;

	    glMatrixMode(GL_PROJECTION);
	    glPushMatrix();
	    glTranslated(0.0, 0.0, -IG_POLYGON_Z_TRANS);
	
	    for (; Pl != NULL; Pl = Pl -> Pnext) {
		int PlDrawn = TRUE;

		if (AllowPolyRGB &&
		    (Str = AttrGetStrAttrib(Pl -> Attr, "RGB")) != NULL &&
		    sscanf(Str, "%d,%d,%d",
			   &Color[0], &Color[1], &Color[2]) == 3) {
		    glColor3ub((GLubyte) Color[0],
			       (GLubyte) Color[1],
			       (GLubyte) Color[2]);
		}

		if (IGGlblCountNumPolys)
		    IGGlblNumPolys++;

		if (IP_IS_STRIP_POLY(Pl)) {
		    IGDrawPolyStrip(Pl);
		    continue;
		}

		if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
		    glBegin(GL_POLYGON);
		    switch (IGGlblShadingModel) {
		        case IG_SHADING_FLAT:
			    OGL_HANDLE_NORMAL(Pl -> Plane);
			case IG_SHADING_BACKGROUND:
			case IG_SHADING_NONE:
			    for (V = Pl -> PVertex;
				 V != NULL;
				 V = V -> Pnext) {
				HandleVertex(V);
			    }
			    break;
			case IG_SHADING_GOURAUD:
			case IG_SHADING_PHONG:
			    for (V = Pl -> PVertex;
				 V != NULL;
				 V = V -> Pnext) {
				OGL_HANDLE_NORMAL(V -> Normal);
				HandleVertex(V);
			    }
		    }
		    glEnd();
		}
		else if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_WIREFRAME) {
		    if (IGGlblBackFaceCull) {
			IrtRType P[3];

			MatMultVecby4by4(P, Pl -> Plane, IPViewMat);
			if ((P[2] > 0.0) ^ IGGlblFlipNormalOrient)
			    PlDrawn = FALSE;
		    }

		    if (PlDrawn) {
			glBegin(GL_LINE_STRIP);
			for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
			    HandleVertex(V);

			    if (IP_IS_INTERNAL_VRTX(V) && !IGGlblDrawInternal) {
				glEnd();
				glBegin(GL_LINE_STRIP);
			    }
			}
		    }
		    HandleVertex(Pl -> PVertex);
		    glEnd();
		}
		else if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_POINTS) {
		    glBegin(GL_POINTS);
		    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		        HandleVertex(V);
		    }
		    glEnd();
		}

		if (PlDrawn && IGGlblDrawPNormal && IP_HAS_PLANE_POLY(Pl)) {
		    IrtRType R;
		    int NumOfVertices = 1;

		    V = Pl -> PVertex;
		    IRIT_PT_COPY(PNormal, V -> Coord);

		    for (V = V -> Pnext; V != NULL; V = V -> Pnext) {
			IRIT_PT_ADD(PNormal, PNormal, V -> Coord);
			NumOfVertices++;
		    }

		    R = 1.0 / NumOfVertices;
		    IRIT_PT_SCALE(PNormal, R);
		    glBegin(GL_LINES);
		    OGL_GL_VRTX_V(PNormal);
		    IG_ADD_ORIENT_NRML(PNormal, Pl -> Plane);
		    OGL_GL_VRTX_V(PNormal);
		    glEnd();
		}

		if (PlDrawn && IGGlblDrawVNormal) {
		    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
			if (IP_HAS_NORMAL_VRTX(V)) {
			    IRIT_VEC_COPY(VNormal, V -> Coord);
			    IG_ADD_ORIENT_NRML(VNormal, V -> Normal);
			    glBegin(GL_LINES);
			    OGL_GL_VRTX_V(V -> Coord);
			    OGL_GL_VRTX_V(VNormal);
			    glEnd();
			}
		    }
		}
	    }

	    glPopMatrix();
	    glMatrixMode(GL_MODELVIEW);

	    glDisable(GL_LIGHTING);
	    IGSetColorObj(PObj);
	}

	if (IGGlblDrawSurfaceSketch && !PolysFromSrf)
	    IGDrawPolygonSketches(PObj);

	if (IGGlblDrawSurfaceSilh || IGGlblDrawSurfaceBndry)
	    IGDrawPolySilhBndry(PObj);

	if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
	    glEnable(GL_LIGHTING);
    }

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw a strip of polygons using current modes and transformations.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:     A poly strip to draw.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawPolyStrip(IPPolygonStruct *Pl)
{
    int i, j;
    IrtPtType VNormal;
    IPVertexStruct *Prev1V, *Prev2V,
	*V = Pl -> PVertex;

    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
	glBegin(GL_TRIANGLE_STRIP);

	switch (IGGlblShadingModel) {                   /* Do first vertex. */
	    case IG_SHADING_NONE:
	    case IG_SHADING_BACKGROUND:
	        break;
	    case IG_SHADING_FLAT:
		OGL_HANDLE_NORMAL(Pl -> Plane);
		break;
	    case IG_SHADING_GOURAUD:
	    case IG_SHADING_PHONG:
		OGL_HANDLE_NORMAL(V -> Normal);
		break;
	}
	HandleVertex(V);

	V = V -> Pnext;
	switch (IGGlblShadingModel) {                  /* Do second vertex. */
	    case IG_SHADING_NONE:
	    case IG_SHADING_BACKGROUND:
	    case IG_SHADING_FLAT:
		break;
	    case IG_SHADING_GOURAUD:
	    case IG_SHADING_PHONG:
		OGL_HANDLE_NORMAL(V -> Normal);
		break;
	}
	HandleVertex(V);

	V = V -> Pnext;
	do {
	    switch (IGGlblShadingModel) {                  /* Do the strip. */
		case IG_SHADING_NONE:
	        case IG_SHADING_BACKGROUND:
		case IG_SHADING_FLAT:
		    break;
		case IG_SHADING_GOURAUD:
		case IG_SHADING_PHONG:
		    OGL_HANDLE_NORMAL(V -> Normal);
		    break;
	    }
	    HandleVertex(V);

	    V = V -> Pnext;
	}
	while (V != NULL);

	glEnd();				    /* Of GL_TRIANGLE_STRIP */

	if (IGGlblDrawVNormal) {
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		if (IP_HAS_NORMAL_VRTX(V)) {
		    for (j = 0; j < 3; j++)
			VNormal[j] = V -> Coord[j] +
				     V -> Normal[j] * IGGlblNormalSize;
		    glBegin(GL_LINE_STRIP);
		    OGL_GL_VRTX_V(V -> Coord);
		    OGL_GL_VRTX_V(VNormal);
		    glEnd();
		}
	    }
	}
    }
    else if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_WIREFRAME) {
	/* Line drawing. */
	glBegin(GL_LINES);
	Prev1V = Pl -> PVertex;
	HandleVertex(Prev1V);
	Prev2V = Prev1V -> Pnext;
	HandleVertex(Prev2V);
	V = Prev2V -> Pnext;
	glEnd();

	for ( ; V != NULL; V = V -> Pnext) {
	    glBegin(GL_LINES);
	    HandleVertex(V);
	    HandleVertex(Prev1V);
	    glEnd();

	    glBegin(GL_LINES);
	    HandleVertex(V);
	    HandleVertex(Prev2V);
	    glEnd();

	    Prev1V = Prev2V;
	    Prev2V = V;
	}

	if (IGGlblDrawVNormal) {
	    IrtVecType VNormal;

	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		if (IP_HAS_NORMAL_VRTX(V)) {
		    for (j = 0; j < 3; j++)
			VNormal[j] = V -> Coord[j] +
				     V -> Normal[j] * IGGlblNormalSize;
		    glBegin(GL_LINE_STRIP);
		    OGL_GL_VRTX_V(V -> Coord);
		    OGL_GL_VRTX_V(VNormal);
		    glEnd();
		}
	    }
	}
    }
    else {
        glBegin(GL_POINTS);
	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    HandleVertex(V);
	}
	glEnd();
    }

    if (IGGlblDrawPNormal) {
	int j;

	Prev1V = Pl -> PVertex;
	Prev2V = Prev1V -> Pnext;
	for (V = Prev2V -> Pnext, j = 0; V != NULL; V = V -> Pnext, j++) {
	    IrtVecType Center, V1, V2, Nrml;

	    IRIT_PT_COPY(Center, V -> Coord);
	    IRIT_PT_ADD(Center, Center, Prev1V -> Coord);
	    IRIT_PT_ADD(Center, Center, Prev2V -> Coord);
	    IRIT_PT_SCALE(Center, 1.0 / 3.0);

	    IRIT_PT_SUB(V1, Prev1V -> Coord, Prev2V -> Coord);
	    IRIT_PT_SUB(V2, Prev2V -> Coord, V -> Coord);
	    IRIT_CROSS_PROD(Nrml, V1, V2);
	    IRIT_PT_NORMALIZE(Nrml);

	    /* Make sure we have the orientation right - cannot get it      */
	    /* right from the polygonal strip itself! 		            */
	    if (IP_HAS_NORMAL_VRTX(V)) {
		if (IRIT_DOT_PROD(Nrml, V -> Normal) < 0)
		    IRIT_PT_SCALE(Nrml, -1.0);
	    }
	    else if (j & 0x01) {
		/* Maybe flipped, but at least be consistent thoughout. */
		IRIT_PT_SCALE(Nrml, -1.0);
	    }

	    glBegin(GL_LINES);
	    OGL_GL_VRTX_V(Center);
	    for (i = 0; i < 3; i++)
		Center[i] += Nrml[i] * IGGlblNormalSize;
	    OGL_GL_VRTX_V(Center);
	    glEnd();

	    Prev1V = Prev2V;
	    Prev2V = V;
	}
    }

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Poly object using current modes and transformations.	     M
*   This function allows polylines to have normals so we can actually draw   M
* the adaptive isolines to render a surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         A polygon or a polyline to draw.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawPolylineNormal                                                     M
*****************************************************************************/
void IGDrawPolylineNormal(IPObjectStruct *PObj)
{
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    IPVertexStruct *V;

	    glBegin(GL_LINE_STRIP);
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		OGL_HANDLE_NORMAL(V -> Normal);
		OGL_GL_VRTX_V(V -> Coord);
	    }
	    glEnd();
	}
    }

    IRIT_TEST_OGL_ERROR();
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

    IRIT_TEST_OGL_ERROR();
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
    glLineWidth((GLfloat) Width);

    IRIT_TEST_OGL_ERROR();
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
    IRIT_TEST_OGL_ERROR();
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

    if (Transparency > 0.0) {
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	IGGlblOpacity = 1.0 - IRIT_BOUND(Transparency, 0.0, 1.0);
    }
    else {
        IGGlblOpacity = 1.0;

	if ((IGGlblAntiAliasing == IG_STATE_ANTI_ALIAS_ON &&
	     IGGlblDrawStyle == IG_STATE_DRAW_STYLE_WIREFRAME) ||
	    (IGGlblAntiAliasing == IG_STATE_ANTI_ALIAS_BLEND &&
	     IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)) {
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	    glEnable(GL_LINE_SMOOTH);
	    glEnable(GL_POINT_SMOOTH);
	    glEnable(GL_BLEND);
	}
	else {
	    glBlendFunc(GL_ONE, GL_ZERO);
	    glDisable(GL_LINE_SMOOTH);
	    glDisable(GL_POINT_SMOOTH);
	    glDisable(GL_BLEND);
	}

	glDepthMask(GL_TRUE);
    }

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Prepares the texture mapping function of an object.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to apply texture to.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if successful, FALSE otherwise.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetTexture                                                             M
*****************************************************************************/
int IGSetTexture(IPObjectStruct *PObj)
{
    int TexID;

#ifdef IRIT_HAVE_OGL_CG_LIB
    PFNGLACTIVETEXTUREARBPROC
        glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
	    wglGetProcAddress("glActiveTextureARB");
#endif /* IRIT_HAVE_OGL_CG_LIB */

    if (PObj == NULL) {
        glDisable(GL_TEXTURE_2D);
	GlblTextureActive = FALSE;
	return FALSE;
    }

    if ((TexID = AttrGetObjectIntAttrib(PObj, "_ImageTexID")) > 0) {
	glEnable(GL_TEXTURE_2D);
	GlblTextureActive = TRUE;
#ifdef IRIT_HAVE_OGL_CG_LIB
/*	glActiveTextureARB(GL_TEXTURE0_ARB); */
#endif /* IRIT_HAVE_OGL_CG_LIB */
	glBindTexture(GL_TEXTURE_2D, TexID);    
      	return TRUE;
    }
    else {
	IrtBType *Pwr2Image,
	    *Image = AttrGetObjectPtrAttrib(PObj, "_ImageTexture");
	int PwrX, PwrY,
	    Width = AttrGetObjectIntAttrib(PObj, "_ImageWidth"),
	    Height = AttrGetObjectIntAttrib(PObj, "_ImageHeight"),
	    Alpha = AttrGetObjectIntAttrib(PObj, "_ImageAlpha");

	if (Image == NULL)
	    return FALSE;

	/* Find the smallest power of two larger/equal to both axes. */
	for (PwrX = 1; PwrX <= Width; PwrX <<= 1);
	if (PwrX != Width)
	    PwrX >>= 1;
	for (PwrY = 1; PwrY <= Height; PwrY <<= 1);
	if (PwrY != Height)
	    PwrY >>= 1;

	while (PwrX > GL_MAX_TEXTURE_SIZE || PwrY > GL_MAX_TEXTURE_SIZE) {
	    PwrX >>= 1;
	    PwrY >>= 1;
	}

        /* Padding of 4 due to a bug in gluScaleImage six lines below. */
        Pwr2Image = (IrtBType *) IritMalloc(sizeof(IrtBType) * (Alpha ? 4 : 3)
					    * PwrX * PwrY + 1000);

        /* Create a power of two image size out of the input image. */
#if defined(__WINCE__)
#pragma message("Warning: gluScaleImage is not supported by this compiler and is bypassed.")
        {
#else
        if ((Width == PwrX && Height == PwrY) ||
	    gluScaleImage(Alpha ? GL_RGBA : GL_RGB, Width, Height,
			  GL_UNSIGNED_BYTE, Image, PwrX, PwrY,
			  GL_UNSIGNED_BYTE, Pwr2Image) != 0) {
#endif /* __WINCE__ */
	    /* Do the best you can with the input image. */
  	    PwrX = Width;
	    PwrY = Height;
	    IritFree(Pwr2Image);
	    Pwr2Image = (IrtBType *) Image;
        }

	/* Get a unique texture ID. */
	glEnable(GL_TEXTURE_2D);
	GlblTextureActive = TRUE;
	glGenTextures(1, (GLuint *) &TexID);

	glBindTexture(GL_TEXTURE_2D, TexID);    
	AttrSetObjectIntAttrib(PObj, "_ImageTexID", TexID);

	glTexImage2D(GL_TEXTURE_2D, 0, Alpha ? GL_RGBA : GL_RGB,
		     PwrX, PwrY, 0, Alpha ? GL_RGBA : GL_RGB,
		     GL_UNSIGNED_BYTE, Pwr2Image);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (((IrtBType *) Image) != Pwr2Image)
	    IritFree(Pwr2Image);

      	return TRUE;
    }

    IRIT_TEST_OGL_ERROR();
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

    IRIT_TEST_OGL_ERROR();
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
    if (IGGlblDrawStyle) {
	int i;
	GLfloat MatAmbient[4], MatDiffuse[4], MatSpecular[4], MatEmissive[4];

 	for (i = 0; i < 3; i++) {
	    MatAmbient[i] = (GLfloat) (IGShadeParam.LightAmbient[i]
 							   * Color[i] / 255.0);
	    MatDiffuse[i] = (GLfloat) (IGShadeParam.LightDiffuse[i]
							   * Color[i] / 255.0);
	    MatSpecular[i] = (GLfloat) IGShadeParam.LightSpecular[i],
	    MatEmissive[i] = (GLfloat) IGShadeParam.LightEmissive[i];
	}
	MatAmbient[3] = MatDiffuse[3] = MatSpecular[3] = MatEmissive[3] = 1.0;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MatAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MatDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, MatSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, MatEmissive);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &IGShadeParam.Shininess);
    }

    if (IGGlblOpacity < 1.0)
        glColor4f((GLfloat) (Color[0] / 255.0),
		  (GLfloat) (Color[1] / 255.0),
		  (GLfloat) (Color[2] / 255.0),
		  (GLfloat) IGGlblOpacity);
    else
        glColor3f((GLfloat) (Color[0] / 255.0),
		  (GLfloat) (Color[1] / 255.0),
		  (GLfloat) (Color[2] / 255.0));

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the light source index Index to a new position/location.         M
*                                                                            *
* PARAMETERS:                                                                M
*   LightPos:       New location of light source. (0, 0, 0) disables         M
*		    light source.  If NULL, refresh all light sources color. M
*   LightColor:     Color of light source.				     M
*   LightIndex:     Index of light source in Open GL. -1 will allocate the   M
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
    IRIT_STATIC_DATA unsigned char
	LightSourceActive[IG_MAX_LIGHT_SOURCES] = { 0 };
    IRIT_STATIC_DATA IrtVecType
        LightSourceClr[IG_MAX_LIGHT_SOURCES] = { { 0.0, 0.0, 0.0 } };
    IRIT_STATIC_DATA IGLightType
        LightSourcePos[IG_MAX_LIGHT_SOURCES] = { { 0.0, 0.0, 0.0, 0.0 } };
    int l, i;

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, (float) !IGGlblLightOneSide);

    if (LightPos != NULL) {
	if (LightIndex == -1) {
	    for (i = 0; i < IG_MAX_LIGHT_SOURCES; i++)
		if (!LightSourceActive[i])
		    break;
	    if (i < IG_MAX_LIGHT_SOURCES)
		LightIndex = i;
	}
	if (LightIndex >= IG_MAX_LIGHT_SOURCES || LightIndex < 0)
	    return;

	l = GL_LIGHT0 + LightIndex;
	     
	if (IRIT_APX_EQ(LightPos[0], 0.0) &&
	    IRIT_APX_EQ(LightPos[1], 0.0) &&
	    IRIT_APX_EQ(LightPos[2], 0.0)) {
	    LightSourceActive[LightIndex] = FALSE;
	    glDisable(l);
	}
	else {
	    glLightfv(l, GL_POSITION, LightPos);

	    IRIT_VEC_COPY(LightSourceClr[LightIndex], LightColor);
	    IRIT_GEN_COPY(LightSourcePos[LightIndex], LightPos,
			  sizeof(IGLightType));
	    LightSourceActive[LightIndex] = TRUE;
	    glEnable(l);
	}
    }

    /* Update the colors of the light sources. */
    for (i = 0; i < IG_MAX_LIGHT_SOURCES; i++) {
	if (LightSourceActive[i]) {
	    int j;
	    GLfloat Clr[4];

	    Clr[3] = 0.0;
	    l = GL_LIGHT0 + i;

	    glLightfv(l, GL_POSITION, LightSourcePos[i]);

	    for (j = 0; j < 3; j++)
		Clr[j] = (GLfloat) (LightSourceClr[i][j] *
				    IGShadeParam.LightAmbient[j]);
	    glLightfv(l, GL_AMBIENT, Clr);

	    for (j = 0; j < 3; j++)
		Clr[j] = (GLfloat) (LightSourceClr[i][j] *
				    IGShadeParam.LightDiffuse[j]);
	    glLightfv(l, GL_DIFFUSE, Clr);

	    for (j = 0; j < 3; j++)
		Clr[j] = (GLfloat) (LightSourceClr[i][j] *
				    IGShadeParam.LightSpecular[j]);
	    glLightfv(l, GL_SPECULAR, Clr);
	}
    }

    IRIT_TEST_OGL_ERROR();
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
    IrtHmgnMatType TMat;
    GLfloat GLViewMat[16];
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    GLViewMat[i * 4 + j] = (float) Mat[i][j];

    /* Push this matrix as well in hierarcy and update IGGlblCrntViewMat    */
    /* to current accumulated viewing transformation.			    */
    glPushMatrix();
    glMultMatrixf(GLViewMat);

    glGetFloatv(GL_MODELVIEW_MATRIX, GLViewMat); 

    IRIT_HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    IGGlblCrntViewMat[i][j] = GLViewMat[i * 4 + j];

    IGDrawObject(PObj);

    IRIT_HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);

    glPopMatrix();

    IRIT_TEST_OGL_ERROR();
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
    IrtRType
	MaxDim = IRIT_MAX(IGViewWidth, IGViewHeight);

    IRIT_TEST_OGL_ERROR();

    return IGHandleGenericPickEvent((ScreenX * 2.0 - IGViewWidth) / MaxDim,
				    (IGViewHeight - ScreenY * 2.0) / MaxDim,
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
    IrtRType
	MaxDim = IRIT_MAX(IGViewWidth, IGViewHeight);

    IGHandleGenericCursorEvent((ScreenX * 2.0 - IGViewWidth) / MaxDim,
			       (IGViewHeight - ScreenY * 2.0) / MaxDim,
			       PickReport);

    IRIT_TEST_OGL_ERROR();
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
    IrtRType t,
	MaxDim = IRIT_MAX(IGViewWidth, IGViewHeight);

    IGGenericScreenToObject((ScreenX * 2.0 - IGViewWidth) / MaxDim,
			    (IGViewHeight - ScreenY * 2.0) / MaxDim,
			    Pt, Dir);

    /* Find the intersection of the ray with the XY plane (Z == 0). */
    if (IRIT_FABS(Dir[2]) < IRIT_UEPS)
	t = -Pt[2] / IRIT_UEPS;
    else
	t = -Pt[2] / Dir[2];

    for (i = 0; i < 3; i++)
	Pt[i] += Dir[i] * t;

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* To handle internal events. Should not block.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleInternalEvents                                                   M
*****************************************************************************/
void IGHandleInternalEvents(void)
{
    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Saves one iteration of the animation sequence as an image.		     M
*									     *
* PARAMETERS:								     M
*   Anim:	Animation structure.					     M
*   PObjs:	Objects to render.					     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*									     *
* KEYWORDS:								     M
*   GMAnimSaveIterationsAsImages, animation				     M
*****************************************************************************/
void GMAnimSaveIterationsAsImages(GMAnimationStruct *Anim,
				  IPObjectStruct *PObjs)
{
    static int
	ImageIndex = 1;
    static IrtImgImageType 
	ImageType = IRIT_IMAGE_PPM3_TYPE;
    static char
	*ImageFileName = NULL;
    int i, Width, Height;
    char FileName[IRIT_LINE_LEN_LONG], *Pixels, *Alpha, *AlphaPixels,
	*p, *a, *ap;

    if (IGGlblImageFileName != NULL) {
        /* We have a new prescription of image name - reset the stage. */
        ImageFileName = IGGlblImageFileName;
	IGGlblImageFileName = NULL;
	ImageIndex = 1;

	if (ImageFileName != NULL &&
	    ((p = strrchr(ImageFileName, '.')) != NULL))
	    ImageType = IrtImgWriteSetType(p + 1);

	if (ImageType == IRIT_IMAGE_UNKNOWN_TYPE) {
	    char Line[IRIT_LINE_LEN];

	    sprintf(Line, "Image type \"%s\" unknown - no save of images.",
		    p + 1);
	    IGIritError(Line);

	    ImageFileName = NULL;                    /* Disable image save. */
	}
    }

    if (ImageFileName == NULL)
	return; /* Something is wrong here - no file name to save to... */

    sprintf(FileName, ImageFileName, ImageIndex++);

    /* Get the image from Open GL. */
    Width = (IGViewWidth + 3) & 0xffffffc;     /* 4-Align needed in OGL. */
    Height = IGViewHeight;
    AlphaPixels = ap = (char *) IritMalloc((Width + 1) * (Height + 1) * 4);
    Alpha = a = (char *) IritMalloc((Width + 1) * (Height + 1) * 1);
    Pixels = p = (char *) IritMalloc((Width + 1) * (Height + 1) * 3);
    glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE,
		 AlphaPixels);

    for (i = 0; i < Width * Height; i++) {
        *p++ = *ap++;
        *p++ = *ap++;
        *p++ = *ap++;
        *a++ = *ap++;
    }

    if (IrtImgWriteOpenFile(NULL, FileName, TRUE, Width, Height)) {
        for (i = 0; i < Height; i++)
	    IrtImgWritePutLine((IrtBType *) &Alpha[Width * i],
			       (IrtImgPixelStruct *) &Pixels[Width * 3 * i]);
	IrtImgWriteCloseFile();
    }
    else {
        char Line[IRIT_LINE_LEN];

	sprintf(Line, "Failed to open image file \"%s\"", FileName);
	IGIritError(Line);
    }

    IritFree(Pixels);
    IritFree(Alpha);
    IritFree(AlphaPixels);

    IRIT_TEST_OGL_ERROR();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the pixels in the view window, from the saved background image.   M
*                                                                            *
* PARAMETERS:                                                                M
*   None			                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGUpdateWindowPixels, IGCaptureWindowPixels                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGUpdateWindowPixelsFromBG                                               M
*****************************************************************************/
void IGUpdateWindowPixelsFromBG(void)
{
#if defined(__WINCE__) || defined(__WINNT__)
    if (IGGlblWinBGImage != NULL)
        IGLoadImageToDisplay(IGGlblWinBGImage,
			     IGGlblWinBGImageX, IGGlblWinBGImageY);

    IRIT_TEST_OGL_ERROR();
#endif /* __WINCE__|| __WINNT__ */
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Loads the given image into the current display.			     M
*									     *
* PARAMETERS:								     M
*   Image:  Image to load into current display.				     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGSaveDisplayAsImage                                                     M
*									     *
* KEYWORDS:								     M
*   IGLoadImageToDisplay						     M
*****************************************************************************/
void IGLoadImageToDisplay(IrtBType *Image, int x, int y)
{
#ifndef __WINCE__
    glDrawPixels(x, y, GL_RGB, GL_UNSIGNED_BYTE, Image);

    IRIT_TEST_OGL_ERROR();
#endif /* !__WINCE__ */
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Saves current display as an image.					     M
*									     *
* PARAMETERS:								     M
*   ImageFileName:  File name where to save the current display as an image. M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGLoadImageToDisplay                                                     M
*									     *
* KEYWORDS:								     M
*   IGSaveDisplayAsImage						     M
*****************************************************************************/
void IGSaveDisplayAsImage(char *ImageFileName)
{
    int i, Width, Height;
    char *Pixels, *Alpha, *AlphaPixels, *p, *a, *ap;

    if (ImageFileName != NULL &&
	((p = strrchr(ImageFileName, '.')) != NULL)) {
	IrtImgImageType
	    ImageType = IrtImgWriteSetType(p + 1);

	if (ImageType == IRIT_IMAGE_UNKNOWN_TYPE) {
	    char Line[IRIT_LINE_LEN];

	    sprintf(Line, "Image type \"%s\" unknown - no save of image.",
		    p + 1);
	    IGIritError(Line);

	    return;
	}
    }

    /* Get the image from Open GL. */
    Width = (IGViewWidth + 3) & 0xffffffc;     /* 4-Align needed in OGL. */
    Height = IGViewHeight;
    AlphaPixels = ap = (char *) IritMalloc((Width + 1) * (Height + 1) * 4);
    Alpha = a = (char *) IritMalloc((Width + 1) * (Height + 1) * 1);
    Pixels = p = (char *) IritMalloc((Width + 1) * (Height + 1) * 3);
    glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE,
		 AlphaPixels);

    for (i = 0; i < Width * Height; i++) {
        *p++ = *ap++;
        *p++ = *ap++;
        *p++ = *ap++;
        *a++ = *ap++;
    }

    if (IrtImgWriteOpenFile(NULL, ImageFileName, TRUE, Width - 1, Height - 1)) {
        for (i = 0; i < Height; i++)
	    IrtImgWritePutLine((IrtBType *) &Alpha[Width * i],
			       (IrtImgPixelStruct *) &Pixels[Width * 3 * i]);
	IrtImgWriteCloseFile();
    }
    else {
        char Line[IRIT_LINE_LEN];

	sprintf(Line, "Failed to open image file \"%s\"", ImageFileName);
	IGIritError(Line);
    }

    IritFree(Pixels);
    IritFree(Alpha);
    IritFree(AlphaPixels);

    IRIT_TEST_OGL_ERROR();
}
