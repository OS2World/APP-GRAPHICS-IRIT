/*****************************************************************************
*   An SGI GL low level drawing routines.				     *
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
#include "attribut.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "xgldrvs.h"
#include "grap_loc.h"
#include "allocate.h"

/* Handle the positional as well as possible RGB attribute of a vertex. */
#define HANDLE_VERTEX(V) { char *Str; \
			   int Color[3]; \
			   if ((Str = AttrGetStrAttrib((V) -> Attr, \
						       "RGB")) != NULL && \
			       sscanf(Str, "%d,%d,%d", \
				      &Color[0], &Color[1], &Color[2]) == 3) {\
			       c3i((long *) Color); \
			   } \
			   v3d((V) -> Coord); \
			 }

IRIT_GLOBAL_DATA long
    ViewWinID = 0,
    ViewWinWidth = 100,
    ViewWinWidth2 = 50,
    ViewWinHeight = 100,
    ViewWinHeight2 = 50,
    ViewWinLow = 0,
    ViewWinLeft = 0;

static void SetColorIndex(int c);
static void SetParamViewWindow(void);
static void ClearViewArea(void);

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
    IrtPtType Ends[6], Zero;
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
	Zero[0] = Zero[1] = Zero[2] = 0.0;
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
    IPVertexStruct *V;
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblCountNumPolys)
		IGGlblNumPolys++;

	    bgnline();
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
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

	if (IGGlblShadingModel == IG_SHADING_BACKGROUND) {
	    IGSetColorRGB(IGGlblBackGroundColor);
	}

	if (IGGlblShadingModel == IG_SHADING_NONE ||
	    IGGlblShadingModel == IG_SHADING_BACKGROUND) {
	    lmbind(LMODEL, 0);
	}
	else {
	    lmbind(LMODEL, 1);
	}

	mmode(MPROJECTION);
	translate(0.0, 0.0, -IG_POLYGON_Z_TRANS);

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

		    switch (IGGlblShadingModel) {
			case IG_SHADING_NONE:
			    break;
			case IG_SHADING_FLAT:
			    if (IGGlblFlipNormalOrient) {
			        n[0] = Pl -> Plane[0];
				n[1] = Pl -> Plane[1];
				n[2] = Pl -> Plane[2];
			    }
			    else {
			        n[0] = -Pl -> Plane[0];
				n[1] = -Pl -> Plane[1];
				n[2] = -Pl -> Plane[2];
			    }
			    n3f(n);
			    break;
			case IG_SHADING_GOURAUD:
			case IG_SHADING_PHONG:
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
			    break;
		    }

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
			v3d(V -> Coord);
			v3d(VNormal);
			endline();
		    }
		}
	    }
	}

	translate(0.0, 0.0, IG_POLYGON_Z_TRANS);
	mmode(MVIEWING);
    }
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
    /* No support for antialiased wide lines!? */
    if (IGGlblAntiAliasing != IG_STATE_ANTI_ALIAS_OFF)
	linewidth(1);
    else
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
	    AMBIENT,  0.25, 0.25, 0.25,
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
	    lmbind(LIGHT2, 2);

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
* Sets up the parameters of the view window.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   None		                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetParamViewWindow(void)
{
    STATIC_DATA int
        OldDoDoubleBuffer = -1,
	OldDepthCue = -1,
	OldAntiAliasing = -1;
    int Modified = FALSE;

    winset(ViewWinID);

    if (OldDoDoubleBuffer != IGGlblDoDoubleBuffer) {
	if (IGGlblDoDoubleBuffer)
	    doublebuffer();
	else
	    singlebuffer();
	gconfig();

	Modified = TRUE;
	OldDoDoubleBuffer = IGGlblDoDoubleBuffer;
    }

    if (OldDepthCue != IGGlblDepthCue) {
	if (IGGlblDepthCue) {
#	    ifndef _AIX
	        glcompat(GLC_ZRANGEMAP, 1);
#	    endif /* _AIX */
	    lRGBrange(0, 0, 0, 255, 255, 255, 0x0, 0x7fffff);
	    depthcue(IGGlblDepthCue);
	}
	else
	    depthcue(FALSE);

	Modified = TRUE;
	OldDepthCue = IGGlblDepthCue;
    }

    if (OldAntiAliasing != IGGlblAntiAliasing) {
	if (IGGlblAntiAliasing == IG_STATE_ANTI_ALIAS_ON) {
#	    ifndef _AIX
	        linesmooth(SML_ON | SML_SMOOTHER);
	        if (getgdesc(GD_PNTSMOOTH_RGB) == 0 ||
		    getgdesc(GD_BITS_NORM_DBL_CMODE) < 8) {
		}
		else {
		    subpixel(TRUE);
		    pntsmooth(SMP_ON);
		}
#	    endif /* _AIX */
	}
	else
	    linesmooth(SML_OFF);

	Modified = TRUE;
	OldAntiAliasing = IGGlblAntiAliasing;
    }

    /* This is wierd. without the sleep the gl get mixed up between the two  */
    /* windows. If you have any idea why, let me know...		     */
    if (Modified)
	IritSleep(100);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets up a view window.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:   Command line.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetViewWindow                                                            M
*****************************************************************************/
void SetViewWindow(int argc, char **argv)
{
    long PrefPos[4];
    STATIC_DATA int
	UpdateLightPos = FALSE;
    STATIC_DATA float
	Light1[] = {
	    POSITION, 1.0, 2.0, 10.0, 0.0,
	    AMBIENT, 0.25, 0.25, 0.25,
	    LMNULL
	},
	Light2[] = {
	    POSITION, -5.0, -1.0, -10.0, 0.0,
	    AMBIENT, 0.25, 0.25, 0.25,
	    LMNULL
	};

    if (!UpdateLightPos) {
	int i;

	for (i = 0; i < 4; i++) {
	    Light1[i + 1] = IGShadeParam.LightPos[0][i];
	    Light2[i + 1] = IGShadeParam.LightPos[1][i];
	}

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
    wintitle("xGLdrvs");
    ViewWinID = winget();

    RGBmode();
    SetParamViewWindow();

    getorigin(&ViewWinLeft, &ViewWinLow);
    getsize(&ViewWinWidth, &ViewWinHeight);
    ViewWinWidth2 = ViewWinWidth / 2;
    ViewWinHeight2 = ViewWinHeight / 2;

    if (ViewWinWidth > ViewWinHeight) {
	int i = (ViewWinWidth - ViewWinHeight) / 2;

	viewport((Screencoord) 0,
		 (Screencoord) ViewWinWidth,
		 (Screencoord) -i, 
		 (Screencoord) (ViewWinWidth - i));
    }
    else {
	int i = (ViewWinHeight - ViewWinWidth) / 2;

	viewport((Screencoord) -i,
		 (Screencoord) (ViewWinHeight - i),
		 (Screencoord) 0,
		 (Screencoord) ViewWinHeight);
    }

    concave(FALSE);

    /* Define necessary staff for Lighting. */
    lmdef(DEFMATERIAL, 1, 0, NULL);
    lmdef(DEFLMODEL, 1, 0, NULL);
    lmdef(DEFLIGHT, 1, sizeof(Light1) / sizeof(float), Light1);
    lmdef(DEFLIGHT, 2, sizeof(Light2) / sizeof(float), Light2);

    IGRedrawViewWindow();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Redraws the view window.						     M
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

    SetParamViewWindow();

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
    IrtHmgnMatType TMat;
    Matrix GLViewMat;
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    GLViewMat[i][j] = Mat[i][j];

    /* Push this matrix as well in the hierarcy and update IGGlblCrntViewMat */
    /* to current accumulated viewing transformation.			     */
    pushmatrix();
    multmatrix(GLViewMat);
    getmatrix(GLViewMat);

    HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    IGGlblCrntViewMat[i][j] = GLViewMat[i][j];

    IGDrawObject(PObj);

    HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);

    popmatrix();
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
* Handles redraw of view window event.                                       M
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
    long dev;
    short data;

    if (qtest()) {
	if ((dev = qread(&data)) == REDRAW && data == ViewWinID) {
	    getorigin(&ViewWinLeft, &ViewWinLow);
	    getsize(&ViewWinWidth, &ViewWinHeight);
	    ViewWinWidth2 = ViewWinWidth / 2;
	    ViewWinHeight2 = ViewWinHeight / 2;

	    if (ViewWinWidth > ViewWinHeight) {
		int i = (ViewWinWidth - ViewWinHeight) / 2;

		viewport((Screencoord) 0,
			 (Screencoord) ViewWinWidth,
			 (Screencoord) -i,
			 (Screencoord) (ViewWinWidth - i));
	    }
	    else {
		int i = (ViewWinHeight - ViewWinWidth) / 2;

		viewport((Screencoord) -i,
			 (Screencoord) (ViewWinHeight - i),
			 (Screencoord) 0,
			 (Screencoord) ViewWinHeight);
	    }

	    ortho2(-0.5, ViewWinWidth - 0.5, -0.5, ViewWinHeight - 0.5);
	    IGRedrawViewWindow();
	}
	else {
	    /* push it back onto the queue. */
	    qenter((Device) dev, data);
	}
    }
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
