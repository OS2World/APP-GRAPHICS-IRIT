/*****************************************************************************
*   A Windows NT driver - regular NT graphics calls.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <stdio.h>
#include <windows.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "wntdrvs.h"

IRIT_STATIC_DATA int 
    GlblLineWidth = 1;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Redraw the viewing window.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   None	                                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedrawViewWindow                                                       M
*****************************************************************************/
void IGRedrawViewWindow(void)
{
    InvalidateRect(IGhWndView, NULL, FALSE);
}
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears a base for the 4 views display mode.                              M
*   The choosen color will be the color of the seperator between the view    M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd:     Handle on window to draw to.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ClearBase4Views                                                          M
*****************************************************************************/
void ClearBase4Views(HWND hWnd)
{
    PAINTSTRUCT ps;

    if (IGCurrenthDC = BeginPaint(hWnd, &ps)) {
	RECT rect;

	GetClientRect(hWnd, &rect);
	FillRect(IGCurrenthDC, &rect, IG4ViewSeperationBrush);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Initializes a Sub view window.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd:    Handle on window to draw to.                                    M
*   ViewNum: The index of the SubView.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateSubViewWindow                                                      M
*****************************************************************************/
void CreateSubViewWindow(HWND hWnd, int ViewNum)
{
    /* Need not do anything. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redraw the (sub) viewing window.                                         M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd:   Handle to window to draw to.                                     M
*   wMsg:   Event to handle.                                                 M
*   wParam: Parameters of event.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Window's status.                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   RedrawSubViewWindow                                                      M
*****************************************************************************/
int RedrawSubViewWindow(HWND hWnd, UINT wMsg, WPARAM wParam)
{
    int Result;
    unsigned int 
	OrigViewWidth = IGViewWidth,
	OrigViewHeight = IGViewHeight,
	OrigViewWidth2 = IGViewWidth2,
	OrigViewHeight2 = IGViewHeight2;
    IrtHmgnMatType TempMat;

    IRIT_GEN_COPY(TempMat, IPViewMat, sizeof(IrtHmgnMatType));
    UpdateSubViewMatrix(GetViewNum(hWnd));
  
    /* Update the size of the screen. */
    IGViewWidth = IGSubViewWidth;
    IGViewHeight = IGSubViewHeight;
    IGViewWidth2 = IGSubViewWidth / 2;
    IGViewHeight2 = IGSubViewHeight / 2;
  
    Result = RedrawViewWindow(hWnd, wMsg, wParam);
  
    /* Restore original values. */
    IGViewWidth = OrigViewWidth;
    IGViewHeight = IGViewHeight;
    IGViewWidth2 = OrigViewWidth2;
    IGViewHeight2 = OrigViewHeight2;
    IRIT_GEN_COPY(IPViewMat, TempMat, sizeof(IrtHmgnMatType));  

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Redraw the viewing window.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   hWnd:     Handle on window to draw to.                                   M
*   wMsg:     Event to handle.                                               M
*   wParam:   Some parameters of event.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Window's condition.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   RedrawViewWindow                                                         M
*****************************************************************************/
int RedrawViewWindow(HWND hWnd, UINT wMsg, WPARAM wParam)
{
    PAINTSTRUCT ps;

    if (IGGlblBackGroundImage && IGGlblWinBGImage == NULL) {
        IGGlblWinBGImage = IGCaptureWindowPixels(hWnd,
						&IGGlblWinBGImageX,
						&IGGlblWinBGImageY, FALSE);
    }
	    
    if (wMsg != WM_PAINT)
	return 0;

    switch (IGGlblViewMode) {		 /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    IRIT_GEN_COPY(IGGlblCrntViewMat, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IGGlblCrntViewMat, IPViewMat, IPPrspMat);
	    break;
    }

    if (IGCurrenthDC = BeginPaint(hWnd, &ps)) {
	RECT rect;
	char NewTitle[IRIT_LINE_LEN_LONG];

	NewTitle[0] = 0;

	if (IGGlblBackGroundImage) {
	    IGUpdateWindowPixelsFromBG();
	}
	else {
	    GetClientRect(hWnd, &rect);
	    FillRect(IGCurrenthDC, &rect, IGBackGroundBrush);
	}

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

	if (IGGlblCountNumPolys)
	    IGGlblNumPolys = 0;
	if (IGGlblCountFramePerSec)
	    IGUpdateFPS(TRUE);

	IGTraverseObjListHierarchy(IGGlblDisplayList, IGGlblCrntViewMat,
				   IGViewObject);

	if (IGGlblCountNumPolys)
	    sprintf(NewTitle, ", Rendered %d polygons", IGGlblNumPolys);
	if (IGGlblCountFramePerSec) {
	    IGUpdateFPS(FALSE);
	    sprintf(&NewTitle[strlen(NewTitle)],
		    ", FPS: %.1f", IGGlblFramePerSec);
	}
	if (IGGlblCountNumPolys || IGGlblCountFramePerSec)
	    SetWindowText(IGhTopLevel, IGGenerateWindowHeaderString(NewTitle));

	if (IGCurrenthPen)
            DeleteObject(SelectObject(IGCurrenthDC, IGCurrenthPen));
	EndPaint(hWnd, &ps);
	IGCurrenthDC = 0;
	IGCurrenthPen = 0;
    }

    return 0;
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
   int x, y;

   MoveToEx(IGCurrenthDC, x = WNT_MAP_X_COORD(X), y = WNT_MAP_Y_COORD(Y),
	    NULL);
   LineTo(IGCurrenthDC, x + 1, y);
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
    MoveToEx(IGCurrenthDC, WNT_MAP_X_COORD(X), WNT_MAP_Y_COORD(Y), NULL);
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
    LineTo(IGCurrenthDC, WNT_MAP_X_COORD(X), WNT_MAP_Y_COORD(Y));
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
    if (!IGCurrenthDC)
	return;
    if (IGCurrenthPen)
        DeleteObject(SelectObject(IGCurrenthDC, IGCurrenthPen));
    IGCurrenthPen = 
	SelectObject(IGCurrenthDC, CreatePen(PS_SOLID, GlblLineWidth,
					     High ? IGCrntColorHighIntensity
					          : IGCrntColorLowIntensity));

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
	IGSetColorIndex(c);
    }
    else {
	/* Use white as default color: */
	IGSetColorIndex(IG_IRIT_WHITE);
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
    GlblLineWidth = Width;
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
*   Color:     Index of color to use. Must be between 0 and IG_MAX_COLOR.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void IGSetColorIndex(int Color)
{
    if (Color > IG_MAX_COLOR)
	Color = IG_IRIT_WHITE;

    IGCrntColorHighIntensity = IGColorsHighIntensity[Color];
    IGCrntColorLowIntensity = IGColorsLowIntensity[Color];

    if (!IGCurrenthDC)
	return;
    if (IGCurrenthPen)
        DeleteObject(SelectObject(IGCurrenthDC, IGCurrenthPen));
    IGCurrenthPen = SelectObject(IGCurrenthDC,
				 CreatePen(PS_SOLID, GlblLineWidth,
					   IGCrntColorHighIntensity));

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
    IGCrntColorHighIntensity = GetNearestColor(IGCurrenthDC,
					       RGB(Color[0],
						   Color[1],
						   Color[2]));
    IGCrntColorLowIntensity = GetNearestColor(IGCurrenthDC,
					      RGB(Color[0] / 2,
						  Color[1] / 2,
						  Color[2] / 2));


    if (!IGCurrenthDC)
	return;
    if (IGCurrenthPen)
        DeleteObject(SelectObject(IGCurrenthDC, IGCurrenthPen));

    IGCurrenthPen = SelectObject(IGCurrenthDC,
				 CreatePen(PS_SOLID, GlblLineWidth,
					   IGCrntColorHighIntensity));

    IGGlblIntensityHighState = TRUE;
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
    return IGHandleGenericPickEvent(
			    (ScreenX * 2.0 - IGViewWidth) / IGViewWidth,
			    (IGViewHeight - ScreenY * 2.0) / IGViewHeight,
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
    IGHandleGenericCursorEvent(
			    (ScreenX * 2.0 - IGViewWidth) / IGViewWidth,
			    (IGViewHeight - ScreenY * 2.0) / IGViewHeight,
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

    IGGenericScreenToObject((ScreenX * 2.0 - IGViewWidth) / IGViewWidth,
			    (IGViewHeight - ScreenY * 2.0) / IGViewHeight,
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
* DESCRIPTION:                                                               M
*   Update the pixels in the view window, from the saved backround image.    M
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
    if (IGGlblWinBGImage != NULL)
        IGUpdateWindowPixels(IGhWndView, *((HDC *) IGGlblWinBGImage),
			     IGGlblWinBGImageX, IGGlblWinBGImageY);
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

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draws a single object with DTexture attributes using current modes       M
* and transformations.		                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to draw.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if no go.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGCGFreeDTexture                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCGDrawDTexture                                                         M
*****************************************************************************/
int IGCGDrawDTexture(IPObjectStruct *PObj)
{
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Draws a single object with ffd_texture attributes using current modes    M
* and transformations.		                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to draw.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if no go.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGCGDrawDTexture                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCGFfdDraw                                                              M
*****************************************************************************/
int IGCGFfdDraw(IPObjectStruct *PObj)
{
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Recreate Open GL display list from a single object and add the handle    M 
* for the display list to the object's attribute.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Object to update.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGCGDrawDTexture                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCGFreeDTexture                                                         M
*****************************************************************************/
void IGCGFreeDTexture(IPObjectStruct *PObj)
{
}
