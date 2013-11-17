/*****************************************************************************
*   An X11 drawing routines.						     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "grap_loc.h"
#include "attribut.h"
#include "allocate.h"
#include "x11drvs.h"

#define X11_MAP_X_COORD(x) (((int) ((x + 1.0) * IGViewWidth)) / 2)
#define X11_MAP_Y_COORD(y) (((int) ((1.0 - y) * IGViewWidth)) / 2 + \
			    (IGViewHeight - IGViewWidth) / 2)

IRIT_STATIC_DATA int
    CurrentXPosition = 0,
    CurrentYPosition = 0;
IRIT_STATIC_DATA XGCValues CrntColorHighIntensity, CrntColorLowIntensity;

static void SetColorIndex(int color);

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
    long ValueMask;
    XSizeHints Hints;
    XSetWindowAttributes SetWinAttr;

    SetWinAttr.background_pixel = IGViewBackGroundPixel;
    SetWinAttr.border_pixel = IGViewBorderPixel;
    ValueMask = CWBackPixel | CWBorderPixel;

    Hints.flags = PMinSize | PMaxSize;
    Hints.x = Hints.y = 1;
    Hints.min_width = 50;
    Hints.max_width = 2000;
    Hints.min_height = 50;
    Hints.max_height = 2000;
    if (IGViewHasSize) {
	Hints.flags |= PSize;
	if (IGViewWidth < Hints.min_width)
	    IGViewWidth = Hints.min_width;
	if (IGViewWidth > Hints.max_width)
	    IGViewWidth = Hints.max_width;
	if (IGViewHeight < Hints.min_height)
	    IGViewHeight = Hints.min_height;
	if (IGViewHeight > Hints.max_height)
	    IGViewHeight = Hints.max_height;
	Hints.width = IGViewWidth;
	Hints.height = IGViewHeight;
    }
    else {
	Hints.flags |= PSize;
	Hints.width = IGViewWidth = DEFAULT_VIEW_WIDTH;
	Hints.height = IGViewHeight = DEFAULT_VIEW_HEIGHT;
    }
    if (IGViewHasPos) {
	Hints.flags |= USPosition;
	Hints.x = IGViewPosX;
	Hints.y = IGViewPosY;
    }

    IGViewWndw = XCreateWindow(IGXDisplay, IGXRoot,
			       IGViewPosX, IGViewPosY,
			       IGViewWidth, IGViewHeight,
			       1, 0, CopyFromParent, CopyFromParent,
			       ValueMask, &SetWinAttr);

    XSetStandardProperties(IGXDisplay, IGViewWndw,
			   RESOURCE_NAME, RESOURCE_NAME, None,
			   argv, argc,
			   &Hints);

    XSelectInput(IGXDisplay, IGViewWndw,
		 ExposureMask | KeyPressMask | ButtonPressMask |
		 ButtonReleaseMask | ButtonMotionMask);
    
    XMapWindow(IGXDisplay, IGViewWndw);
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
    char Line[IRIT_LINE_LEN_LONG];

    if (IGGlblCountNumPolys)
	IGGlblNumPolys = 0;
    if (IGGlblCountFramePerSec)
        IGUpdateFPS(TRUE);

    IGPredefinedAnimation();

    XClearWindow(IGXDisplay, IGViewWndw);

    switch (IGGlblViewMode) {		 /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    IRIT_GEN_COPY(IGGlblCrntViewMat, IPViewMat,
			  sizeof(IrtHmgnMatType));
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
	XStoreName(IGXDisplay, IGViewWndw, IGGenerateWindowHeaderString(Line));
	fprintf(stderr, "\t%s%s           \r", RESOURCE_NAME, Line);
    }

    XFlush(IGXDisplay);
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

    if (IGGlblAbortKeyPressed)
        return;

    IRIT_HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    IRIT_HMGN_MAT_COPY(IGGlblCrntViewMat, Mat);

    IGDrawObject(PObj);

    IRIT_HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);
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
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Low level 2D drawing routine. Coordinates are normalized to -1 to 1 by     M
* this time.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y:    Coordinates of 2D location to plot at.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPlotTo2D                                                               M
*****************************************************************************/
void IGPlotTo2D(IrtRType X, IrtRType Y)
{
    XDrawPoint(IGXDisplay, IGViewWndw, IGXViewGraphContext,
	       X11_MAP_X_COORD(X), X11_MAP_Y_COORD(Y));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Low level 2D drawing routine. Coordinates are normalized to -1 to 1 by     M
* this time.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y:    Coordinates of 2D location to move to.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGMoveTo2D                                                               M
*****************************************************************************/
void IGMoveTo2D(IrtRType X, IrtRType Y)
{
    CurrentXPosition = X11_MAP_X_COORD(X);
    CurrentYPosition = X11_MAP_Y_COORD(Y);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Low level 2D drawing routine. Coordinates are normalized to -1 to 1 by     M
* this time.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   X, Y:    Coordinates of 2D location to draw to.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGLineTo2D                                                               M
*****************************************************************************/
void IGLineTo2D(IrtRType X, IrtRType Y)
{
    int NewX, NewY;

    XDrawLine(IGXDisplay, IGViewWndw, IGXViewGraphContext,
	      CurrentXPosition,
	      CurrentYPosition,
	      NewX = X11_MAP_X_COORD(X),
	      NewY = X11_MAP_Y_COORD(Y));

    CurrentXPosition = NewX;
    CurrentYPosition = NewY;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the intensity of a color (high or low).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   High:     TRUE for high, FALSE for low.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetColorIntensity                                                      M
*****************************************************************************/
void IGSetColorIntensity(int High)
{
    XChangeGC(IGXDisplay, IGXViewGraphContext, GCForeground,
	      High ? &CrntColorHighIntensity : &CrntColorLowIntensity);
    IGGlblIntensityHighState = High;
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
    XGCValues values;

    values.line_width = Width;
    XChangeGC(IGXDisplay, IGXViewGraphContext, GCLineWidth, &values);
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
    if (color >= IGMaxColors)
	color = IG_IRIT_WHITE;

    CrntColorHighIntensity.foreground = IGXViewColorsHigh[color].pixel;
    CrntColorLowIntensity.foreground = IGXViewColorsLow[color].pixel;
    XChangeGC(IGXDisplay, IGXViewGraphContext, GCForeground,
	      &CrntColorHighIntensity);
    IGGlblIntensityHighState = TRUE;
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
    XColor XClr;

    XClr.red   = (Color[0] << 8);
    XClr.green = (Color[1] << 8);
    XClr.blue  = (Color[2] << 8);

    /* If fails to allocate the color - take WHITE instead. */
    if (!XAllocColor(IGXDisplay, IGXColorMap, &XClr)) {
	fprintf(stderr,
		"x11drvs: Failed to allocate color, selected WHITE instead\n");
	XClr.pixel = WhitePixel(IGXDisplay, IGXScreen);
    }
    CrntColorHighIntensity.foreground = XClr.pixel;

    XClr.red   = (Color[0] << 7);
    XClr.green = (Color[1] << 7);
    XClr.blue  = (Color[2] << 7);

    /* If fails to allocate the color - take WHITE instead. */
    if (!XAllocColor(IGXDisplay, IGXColorMap, &XClr)) {
	fprintf(stderr,
		"x11drvs: Failed to allocate color, selected WHITE instead\n");
	XClr.pixel = WhitePixel(IGXDisplay, IGXScreen);
    }
    CrntColorLowIntensity.foreground = XClr.pixel;

    XChangeGC(IGXDisplay, IGXViewGraphContext, GCForeground,
	      &CrntColorHighIntensity);
    IGGlblIntensityHighState = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle pick events.                                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   ScreenX, ScreenY: Screen coordinates of pick event.                      M
*   PickType:	      Types of object to pick or IG_PICK_ANY for any object. M
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
IPObjectStruct *IGHandlePickEvent(int ScreenX, int ScreenY, int PickType)
{
    IrtRType
	MaxDim = IRIT_MAX(IGViewWidth, IGViewHeight);

    return IGHandleGenericPickEvent((ScreenX * 2.0 - IGViewWidth) / MaxDim,
				    (IGViewHeight - ScreenY * 2.0) / MaxDim,
				    PickType);
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
