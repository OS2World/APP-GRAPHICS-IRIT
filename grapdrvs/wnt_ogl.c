/*****************************************************************************
*   A Windows NT driver - Open GL graphics calls.			     *
*   The Pallete code was taken from the gengl open gl example in	     *
* /mstools/samples/opengl/demos/gengl. Ugly staff to create default pallete! *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1994.  *
*****************************************************************************/

#include <windows.h>
#include <gl/gl.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"
#include "editcrvs.h"
#include "wntdrvs.h"

/* Disable warnings for double <-> float conversions. */
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)

#define IG_MAX_OGL_WIN	4

IRIT_STATIC_DATA HPALETTE ghpalOld,
    ghPalette = (HPALETTE) 0;
IRIT_STATIC_DATA HDC hViewDC, hSubViewDC[IG_MAX_OGL_WIN];
IRIT_STATIC_DATA HGLRC hViewRC, hSubViewRC[IG_MAX_OGL_WIN];

static BOOL bSetupPixelFormat(HDC hDC);

static BOOL bSetupPixelFormat(HDC hDC)
{
    IRIT_STATIC_DATA PIXELFORMATDESCRIPTOR Pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	/* Size of this pfd. */
	1,				/* Version number. */
	PFD_DRAW_TO_WINDOW |		/* Support window. */
	  PFD_SUPPORT_OPENGL |		/* Support OpenGL. */
	  PFD_DOUBLEBUFFER,		/* Double buffered. */
	PFD_TYPE_RGBA,			/* RGBA type. */
	24,				/* 24-bit color depth. */
	8, 0, 8, 0, 8, 0,		/* Color/shift bits. */
	8,				/* Alpha buffer. */
	0,				/* Alpha shift bit ignored. */
	1,				/* Accumulation buffer. */
	0, 0, 0, 0, 			/* Accum bits. */
	32,				/* 32-bit z-buffer. */
	0,				/* No stencil buffer. */
	0,				/* No auxiliary buffer. */
	PFD_MAIN_PLANE,			/* Main layer. */
	0,				/* Reserved. */
	0, 0, 0				/* Layer masks ignored. */
    };
    int PixelFormat = ChoosePixelFormat(hDC, &Pfd);

    if (PixelFormat == 0 || !SetPixelFormat(hDC, PixelFormat, &Pfd)) {
        char Line[IRIT_LINE_LEN_LONG];

        sprintf(Line, "Set/ChoosePixelFormat failed, Error %d, Format %d",
		GetLastError(), PixelFormat);
        MessageBox(NULL, Line, "Error", MB_OK);
        return FALSE;
    }

    return TRUE;
}

static void RedrawOneViewWindow(HWND hWnd, int ClearAll);

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
    char NewTitle[IRIT_LINE_LEN_LONG];

    NewTitle[0] = 0;

    if (IGGlblCountNumPolys)
	IGGlblNumPolys = 0;
    if (IGGlblCountFramePerSec)
        IGUpdateFPS(TRUE);

    if (IGGlbl4Views) {
        int i;

	ClearBase4Views(IGhWndView); 
	for (i = 0; i < IG_NUM_OF_SUB_VIEWS; i++) {
	    RedrawSubViewWindow(IGhWndSubView[i], WM_PAINT, 0);

	    if (i == 0 && IGGlblCountNumPolys)
		sprintf(NewTitle, ", Rendered %d polygons", IGGlblNumPolys);
	}
    }
    else {
        if (IGGlblActiveXMode)
	    InvalidateRect(IGhWndView, NULL, FALSE);
	else
	    RedrawViewWindow(IGhWndView, WM_PAINT, 0);

	if (IGGlblCountNumPolys)
	    sprintf(NewTitle, ", Rendered %d polygons", IGGlblNumPolys);
    }

    if (IGGlblCountFramePerSec) {
        IGUpdateFPS(FALSE);
        sprintf(&NewTitle[strlen(NewTitle)],
		", FPS: %.1f", IGGlblFramePerSec);
    }

    if (IGGlblCountNumPolys || IGGlblCountFramePerSec)
        SetWindowText(IGhTopLevel, IGGenerateWindowHeaderString(NewTitle));
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
    IRIT_STATIC_DATA IrtVecType
        WhiteColor = { 1.0, 1.0, 1.0 };
    int i;
    GLfloat FogColor[4];

    if (ViewNum == -1) { /* Main view window. */
        hViewDC = GetDC(hWnd);
	bSetupPixelFormat(hViewDC);

	hViewRC = wglCreateContext(hViewDC);
	wglMakeCurrent(hViewDC, hViewRC);
    }
    else {
        hSubViewDC[ViewNum] = GetDC(hWnd);
	bSetupPixelFormat(hSubViewDC[ViewNum]);
  
	hSubViewRC[ViewNum] = wglCreateContext(hSubViewDC[ViewNum]);
	wglMakeCurrent(hSubViewDC[ViewNum], hSubViewRC[ViewNum]);
    }

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glFrontFace(GL_CCW);

    for (i = 0; i < 3; i++)
	FogColor[i] = (float) IGGlblBackGroundColor[i] / 255.0;
    FogColor[3] = (float) 1.0;
    glFogfv(GL_FOG_COLOR, FogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);

    glClearColor(IGGlblBackGroundColor[0] / 255.0,
		 IGGlblBackGroundColor[1] / 255.0,
		 IGGlblBackGroundColor[2] / 255.0,
		 1.0);
    glClearAccum(0.0, 0.0, 0.0, 0.0);

    for (i = 0; i < IGShadeParam.NumOfLightSrcs; i++)
        IGSetLightSource(IGShadeParam.LightPos[i], WhiteColor, i);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redraw the (sub) viewing window.                                         M
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
    int Result,
	ViewNum = GetViewNum(hWnd);
    IrtHmgnMatType TempMat;

    IRIT_GEN_COPY(TempMat, IPViewMat, sizeof(IrtHmgnMatType));
    UpdateSubViewMatrix(ViewNum);
    wglMakeCurrent(hSubViewDC[ViewNum], hSubViewRC[ViewNum] );

    Result = RedrawViewWindow(hWnd, wMsg, wParam);

    wglMakeCurrent(hViewDC, hViewRC);
    IRIT_GEN_COPY(IPViewMat, TempMat, sizeof(IrtHmgnMatType));  

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initialized Open GL calls for view window.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   hWnd:     Handle to window to draw to.                                   *
*   ClearAll: TRUE to clear both the Z buffer and RGB, FALSE only Z buffer.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawOneViewWindow(HWND hWnd, int ClearAll)
{
    int i;
    RECT rect;

    GetClientRect(hWnd, &rect);

    if (rect.right > rect.bottom) {
        i = (rect.right - rect.bottom) / 2;

	glViewport(0, -i, rect.right, rect.right);
    }
    else {
        i = (rect.bottom - rect.right) / 2;

	glViewport(-i, 0, rect.bottom, rect.bottom);
    }

    IGRedrawViewWindowOGL(ClearAll);
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
    RECT rect;
    int i;

    GetClientRect(hWnd, &rect);

    switch (wMsg) {
	case WM_CREATE:
	    CreateSubViewWindow(hWnd, -1);
	    glPushAttrib(GL_ALL_ATTRIB_BITS);
	    break;
	case WM_DESTROY:
	    glPopAttrib();
	    if (hViewRC) {
		wglMakeCurrent(hViewDC, NULL);
	        wglDeleteContext(hViewRC);
		hViewRC = 0;
	    }
	    for (i = 0; i < IG_MAX_OGL_WIN; i++) {
	        if (hSubViewRC[i]) {
		    wglMakeCurrent(hSubViewDC[i], NULL);
		    wglDeleteContext(hSubViewRC[i]);
		    hSubViewRC[i] = 0;
		}
	    }

	    if (hViewDC) {
	        ReleaseDC(hWnd, hViewDC);
		hViewDC = 0;
	    }
	    for (i = 0; i < IG_MAX_OGL_WIN; i++) {
	        if (hSubViewDC[i]) {
		    ReleaseDC(hWnd, hSubViewDC[i]);
		    hSubViewDC[i] = 0;
		}
	    }
	    break;
	case WM_SIZE:
	    if (IGGlblBackGroundImage && rect.bottom != 0 && rect.right != 0) {
	        if (IGGlblWinBGImage != NULL)
		    IritFree(IGGlblWinBGImage);

	        IGGlblWinBGImage = IGCaptureWindowPixels(hWnd,
						&IGGlblWinBGImageX,
						&IGGlblWinBGImageY, TRUE);
	    }
	    break;
	case WM_PAINT:
	    if (rect.bottom == 0 || rect.right == 0)
	        break;

	    /* Reset the raster position. */
	    {
		RECT rect;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glOrtho(0.0, 1.0, 0.0, 1.0, IGGlblZMinClip, IGGlblZMaxClip);
		GetClientRect(hWnd, &rect);
		glViewport(0, 0, rect.bottom, rect.right);

		glRasterPos2i(0, 0);
	    }

	    if (IGCurrenthDC = BeginPaint(hWnd, &ps)) {
		if (IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_BLUE ||
		    IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_GREEN) {
		    for (IGGlbl3DGlassesImgIndx = 0;
			 IGGlbl3DGlassesImgIndx < 2;
			 IGGlbl3DGlassesImgIndx++)
		        RedrawOneViewWindow(hWnd,
					    IGGlbl3DGlassesImgIndx == 0);
		}
		else {
		    RedrawOneViewWindow(hWnd, TRUE);
		}

		if (IGGlblDoDoubleBuffer) {
		    HDC hDC2 = wglGetCurrentDC();

		    SwapBuffers(hDC2);
		}

		EndPaint(hWnd, &ps);
		IGCurrenthDC = 0;
	    }
	    break;
	case WM_QUERYNEWPALETTE:
	    {
		HDC     hDC;

		if (ghPalette) {
		    hDC = GetDC(hWnd);

		    ghpalOld = SelectPalette(hDC, ghPalette, FALSE);
		    RealizePalette(hDC);

		    InvalidateRect(hWnd, NULL, TRUE);
		    UpdateWindow(hWnd);

		    if (ghpalOld)
			SelectPalette(hDC, ghpalOld, FALSE);

		    ReleaseDC(hWnd, hDC);

		    return TRUE;
		}

		return FALSE;
	    }
	case WM_PALETTECHANGED:
	    {
		HDC	 hDC;

		if (ghPalette) {
		    
		    if (wParam != (WPARAM) hWnd)
		    {
			hDC = GetDC(hWnd);

			ghpalOld = SelectPalette(hDC, ghPalette, FALSE);
			RealizePalette(hDC);

			UpdateColors(hDC);

			if (ghpalOld)
			    SelectPalette(hDC, ghpalOld, FALSE);

			ReleaseDC(hWnd, hDC);
		    }
		}
		break;
	    }
    }

    return FALSE;
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
        RECT Rect;

	GetClientRect(hWnd, &Rect);
	glViewport(0, 0, Rect.right, Rect.right);
	glDrawBuffer(GL_FRONT);

	glClearColor(IGGlbl4ViewSeperationColor[0] / 255.0,
		     IGGlbl4ViewSeperationColor[1] / 255.0,
		     IGGlbl4ViewSeperationColor[2] / 255.0,
		     1.0);
	  
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();

	EndPaint(hWnd, &ps);
	IGCurrenthDC = 0;
    }
}
