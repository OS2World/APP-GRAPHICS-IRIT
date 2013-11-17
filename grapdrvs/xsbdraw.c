/*****************************************************************************
*   An HP Starbase low level drawing routines.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1994.  *
*****************************************************************************/

#include <starbase.c.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XHPlib.h>
#include <X11/cursorfont.h>
#include <X11/Xresource.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "irit_sm.h"
#include "attribut.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"
#include "x11drvs.h"
#include "xsbdrvs.h"

#define MAX_NUM_OF_VERTICES	1000
#define PRGM_TITLE		"xsbmdrvs"

STATIC_DATA int
    GlblFilDes = -1;

STATIC_DATA short Colors[IG_MAX_COLOR + 1][3] =
{
    { 0,   0,   0   },  /* 0. BLACK */
    { 0,   0,   170 },  /* 1. BLUE */
    { 0,   170, 0   },  /* 2. GREEN */
    { 0,   170, 170 },  /* 3. CYAN */
    { 170, 0,   0   },  /* 4. RED */
    { 170, 0,   170 },  /* 5. MAGENTA */
    { 170, 170, 0   },  /* 6. BROWN */
    { 170, 170, 170 },  /* 7. LIGHTGREY */
    { 85,  85,  85  },  /* 8. DARKGRAY */
    { 85,  85,  255 },  /* 9. LIGHTBLUE */
    { 85,  255, 85  },  /* 10. LIGHTGREEN */
    { 85,  255, 255 },  /* 11. LIGHTCYAN */
    { 255, 85,  85  },  /* 12. LIGHTRED */
    { 255, 85,  255 },  /* 13. LIGHTMAGENTA */
    { 255, 255, 85  },  /* 14. YELLOW */
    { 255, 255, 255 }   /* 15. WHITE */
};

static void SetColorIndex(int c);
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
	move3d(GlblFilDes, Ends[i][0], Ends[i][1], Ends[i][2]);
	draw3d(GlblFilDes, Ends[i+1][0], Ends[i+1][1], Ends[i+1][2]);
    }

    if (IP_IS_VEC_OBJ(PObj)) {
	move3d(GlblFilDes, 0.0, 0.0, 0.0);
	draw3d(GlblFilDes, Pt[0], Pt[1], Pt[2]);
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

    if (Pl -> PAux == NULL) {
	/* Initialize this object's star base data. */
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    float *r, *r2;
	    int i = 0,
		Len = IPVrtxListLen(Pl -> PVertex);

	    r2 = r = (float *) IritMalloc(1 + sizeof(float) * Len * 6);

	    *((int *) r2) = Len;
	    r2++;

	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		*r2++ = V -> Coord[0];
		*r2++ = V -> Coord[1];
		*r2++ = V -> Coord[2];
		*r2++ = V -> Normal[0];
		*r2++ = V -> Normal[1];
		*r2++ = V -> Normal[2];
	    }

	    Pl -> PAux = r;
	}

	Pl = PObj -> U.Pl;
    }

    if (IP_IS_POLYLINE_OBJ(PObj)) {
	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    float
		*r = (float *) Pl -> PAux;

	    polyline3d(GlblFilDes, &r[1], *((int *) r), FALSE);
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
		    move3d(GlblFilDes,
			   Ends[i][0], Ends[i][1], Ends[i][2]);
		    draw3d(GlblFilDes,
			   Ends[i+1][0], Ends[i+1][1], Ends[i+1][2]);
		}
	    }
	}
    }
    else if (IP_IS_POLYGON_OBJ(PObj)) {
	int i, j,
	    PolysFromSrf = AttrGetObjectIntAttrib(PObj, "_srf_polys") == TRUE,
	    NumOfVertices;
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

	for (; Pl != NULL; Pl = Pl -> Pnext) {
	    if (IGGlblBackFaceCull) {
	        IrtRType P[3];

		if (IGGlblCountNumPolys)
		    IGGlblNumPolys++;

		MatMultVecby4by4(P, Pl -> Plane, IPViewMat);
		if ((P[2] > 0.0) ^ IGGlblFlipNormalOrient)
		    continue;
	    }

	    if (IGGlblDrawPNormal) {
		NumOfVertices = 0;
		VEC_RESET(PNormal);
	    }

	    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
		float
		    *r = (float *) Pl -> PAux;

		switch (IGGlblShadingModel) {
		    case IG_SHADING_NONE:
		    case IG_SHADING_BACKGROUND:
		    case IG_SHADING_FLAT:
			polygon_with_data3d(GlblFilDes, &r[1], *((int *) r),
					    3, NULL, NULL);
			break;
		    case IG_SHADING_GOURAUD:
		    case IG_SHADING_PHONG:
			polygon3d(GlblFilDes, &r[1], *((int *) r), FALSE);
			break;
		}
	    }
	    else {
		int NextMove = TRUE;
		IrtRType
		    *R = Pl -> PVertex -> Coord;

		move3d(GlblFilDes, R[0], R[1], R[2]);

		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    R = V -> Coord;
		    if (NextMove)
		        move3d(GlblFilDes, R[0], R[1], R[2]);
		    else
		        draw3d(GlblFilDes, R[0], R[1], R[2]);

		    NextMove = IP_IS_INTERNAL_VRTX(V) && !IGGlblDrawInternal;

		    if (IGGlblDrawPNormal) {
			for (j = 0; j < 3; j++)
			    PNormal[j] += V -> Coord[j];
			NumOfVertices++;
		    }
		}
		if (!NextMove) {
		    R = Pl -> PVertex -> Coord;
		    draw3d(GlblFilDes, R[0], R[1], R[2]);
		}
	    }

	    if (IGGlblDrawPNormal && IP_HAS_PLANE_POLY(Pl)) {
		for (i = 0; i < 3; i++)
		    PNormal[i] /= NumOfVertices;
		move3d(GlblFilDes, PNormal[0], PNormal[1], PNormal[2]);
		if (IGGlblFlipNormalOrient)
		    VEC_SUB(PNormal, PNormal,
			    IGGlblNormalSize * Pl -> Plane);
		else
		    VEC_ADD(PNormal, PNormal,
			    IGGlblNormalSize * Pl -> Plane);
		draw3d(GlblFilDes, PNormal[0], PNormal[1], PNormal[2]);
	    }

	    if (IGGlblDrawVNormal) {
		for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		    if (IP_HAS_NORMAL_VRTX(V)) {
			if (IGGlblFlipNormalOrient)
			    VEC_SUB(VNormal, V -> Coord,
				    IGGlblNormalSize * V -> Normal);
			else
			    VEC_ADD(VNormal, V -> Coord,
				    IGGlblNormalSize * V -> Normal);
			move3d(GlblFilDes,
			       V -> Coord[0], V -> Coord[1], V -> Coord[2]);
			draw3d(GlblFilDes, VNormal[0], VNormal[1], VNormal[2]);
		    }
		}
	    }
	}
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
    /* This is another "feature" of starbase. The line_width routine only */
    /* works for draw2d and not for draw3d, so this has no affect.	  */
    line_endpoint(GlblFilDes, ROUNDED);
    line_width(GlblFilDes, Width / 100.0, MC_UNITS);
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

    Color[0] = Colors[color][0];
    Color[1] = Colors[color][1];
    Color[2] = Colors[color][2];

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
    line_color(GlblFilDes,
	       Color[0] / 255.0, Color[1] / 255.0, Color[2] / 255.0);
    fill_color(GlblFilDes,
	       Color[0] / 255.0, Color[1] / 255.0, Color[2] / 255.0);
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
    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID)
	clear_control(GlblFilDes, CLEAR_DISPLAY_SURFACE | CLEAR_ZBUFFER);
    else
	clear_control(GlblFilDes, CLEAR_DISPLAY_SURFACE);
    clear_view_surface(GlblFilDes);

    hidden_surface(GlblFilDes, FALSE, FALSE);
    if (IGGlblDoDoubleBuffer)
	double_buffer(GlblFilDes, TRUE | INIT , 24);
    else
	double_buffer(GlblFilDes, FALSE , 24);
    vdc_extent(GlblFilDes, -1.0, -1.0, IGGlblZMinClip,
		            1.0,  1.0, IGGlblZMaxClip);
    clip_depth(GlblFilDes, IGGlblZMaxClip, IGGlblZMinClip);
    depth_indicator(GlblFilDes, TRUE, TRUE);

    shade_mode(GlblFilDes, INIT | CMAP_FULL,
	       IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID);
    hidden_surface(GlblFilDes, IGGlblDrawStyle  == IG_STATE_DRAW_STYLE_SOLID,
		   IGGlblBackFaceCull);
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
    char *SBDevice;
    float CrntView[4][4];
    int i, j, transparentOverlays, numOverlayVisuals, numVisuals,
	numImageVisuals, imageDepthToUse, mustFreeImageColormap;
    OverlayInfo	*pOverlayVisuals;
    XVisualInfo	*pVisuals, **pImageVisuals, *pPseudoColorVisualInfo,
	*pGrayScaleVisualInfo;
    Visual	*pImageVisualToUse;
    Colormap	imageColormap;

    if (!IGViewHasSize) {
	IGViewWidth = DEFAULT_VIEW_WIDTH;
	IGViewHeight = DEFAULT_VIEW_HEIGHT;
    }
    if (!IGViewHasPos) {
	IGViewPosX = 0;
	IGViewPosY = 0;
    }

    /* Get all of the visual information about the screen: */
    if (GetXVisualInfo(IGXDisplay, IGXScreen, &transparentOverlays,
		       &numVisuals, &pVisuals,
		       &numOverlayVisuals, &pOverlayVisuals,
		       &numImageVisuals, &pImageVisuals))
    {
	fprintf(stderr, "Could not get visual info.\n");
	exit(1);
    }

    /* Find an appropriate image planes visual: */
    if (FindImagePlanesVisual(IGXDisplay, IGXScreen, numImageVisuals,
			      pImageVisuals, 1, 8, 1, &pImageVisualToUse,
			      &imageDepthToUse))
    {
	fprintf(stderr, "Could not find a good image planes visual.\n");
	exit(1);
    }

    if (CreateImagePlanesWindow(IGXDisplay, IGXScreen,
				RootWindow(IGXDisplay, IGXScreen),
				IGViewPosX, IGViewPosY,
				IGViewWidth, IGViewHeight,
				imageDepthToUse, pImageVisualToUse,
				argc, argv, PRGM_TITLE, PRGM_TITLE,
				&IGViewWndw, &imageColormap,
				&mustFreeImageColormap))
    {
	fprintf(stderr, "Could not create the image planes window.\n");
	exit(1);
    }

    /* Map the windows: */
    XMapWindow(IGXDisplay, IGViewWndw);
    XSync(IGXDisplay, FALSE);
    XInstallColormap(IGXDisplay, imageColormap);
    XSync(IGXDisplay, FALSE);

    XSelectInput(IGXDisplay, IGViewWndw,
		 ExposureMask | ResizeRedirectMask | ButtonPressMask);
    XFlush(IGXDisplay);

    SBDevice = make_X11_gopen_string(IGXDisplay, IGViewWndw);
    GlblFilDes = gopen(SBDevice, OUTDEV, "", INIT | THREE_D | MODEL_XFORM);

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    CrntView[i][j] = i == j;
    concat_transformation3d(GlblFilDes, CrntView, PRE, PUSH);

    light_ambient(GlblFilDes,
		  IGShadeParam.LightAmbient[0],
		  IGShadeParam.LightAmbient[1],
		  IGShadeParam.LightAmbient[2]);
    light_source(GlblFilDes, 1, DIRECTIONAL, 0.7, 0.7, 0.7,
		 IGShadeParam.LightPos[0][0],
		 IGShadeParam.LightPos[0][1],
		 IGShadeParam.LightPos[0][2]);
    light_source(GlblFilDes, 2, DIRECTIONAL, 0.7, 0.7, 0.7,
		 IGShadeParam.LightPos[1][0],
		 IGShadeParam.LightPos[1][1],
		 IGShadeParam.LightPos[1][2]);
    light_switch(GlblFilDes, 1 + 2 + 4);

    vdc_extent(GlblFilDes, -1.0, -1.0, IGGlblZMinClip,
		            1.0,  1.0, IGGlblZMaxClip);
    clip_depth(GlblFilDes, IGGlblZMaxClip, IGGlblZMinClip);
    depth_indicator(GlblFilDes, TRUE, TRUE);

    background_color(GlblFilDes, IGGlblBackGroundColor[0] / 255.0,
		     	         IGGlblBackGroundColor[1] / 255.0,
				 IGGlblBackGroundColor[2] / 255.0);

    interior_style(GlblFilDes, INT_SOLID, FALSE);

    shade_mode(GlblFilDes, INIT | CMAP_FULL,
	       IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID);

    vertex_format(GlblFilDes, 3, 3, 0, FALSE,
		  COUNTER_CLOCKWISE | UNIT_NORMALS);

    ClearViewArea();
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
    STATIC_DATA int
	CurrentDisplay = 0;
    char Line[LINE_LEN_LONG];
    IPObjectStruct *PObj;

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

    if (IGGlblCountNumPolys)
	IGGlblNumPolys = 0;
    if (IGGlblCountFramePerSec)
        IGUpdateFPS(TRUE);

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
	XStoreName(IGXDisplay, IGViewWndw, IGGenerateWindowHeaderString(Line));
	fprintf(stderr, "\t%s%s           \r", RESOURCE_NAME, Line);
    }

    if (IGGlblDoDoubleBuffer)
	dbuffer_switch(GlblFilDes, !CurrentDisplay);
    else
        flush_buffer(GlblFilDes);

    CurrentDisplay = !CurrentDisplay;
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
    float CrntView[4][4];
    IrtHmgnMatType TMat;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    CrntView[i][j] = j == 2 ? -Mat[i][j] : Mat[i][j];

    replace_matrix3d(GlblFilDes, CrntView);

    HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    HMGN_MAT_COPY(IGGlblCrntViewMat, Mat);

    IGDrawObject(PObj);

    HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);
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
	MaxDim = MAX(IGViewWidth, IGViewHeight);

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
	MaxDim = MAX(IGViewWidth, IGViewHeight);

    IGHandleGenericCursorEvent((ScreenX * 2.0 - IGViewWidth) / MaxDim,
			       (IGViewHeight - ScreenY * 2.0) / MaxDim,
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
    IrtRType t,
	MaxDim = MAX(IGViewWidth, IGViewHeight);

    IGGenericScreenToObject((ScreenX * 2.0 - IGViewWidth) / MaxDim,
			    (IGViewHeight - ScreenY * 2.0) / MaxDim,
			    Pt, Dir);

    /* Find the intersection of the ray with the XY plane (Z == 0). */
    if (FABS(Dir[2]) < IRIT_UEPS)
	t = -Pt[2] / IRIT_UEPS;
    else
	t = -Pt[2] / Dir[2];

    for (i = 0; i < 3; i++)
	Pt[i] += Dir[i] * t;
}
