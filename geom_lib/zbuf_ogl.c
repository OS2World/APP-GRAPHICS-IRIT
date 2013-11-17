/*****************************************************************************
*   An interface to Zbuffer manipulation using Open GL			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, September 1997 *
*****************************************************************************/

#ifdef __OPENGL__
#   if defined(__WINNT__) || defined(__WINCE__)
#       include <windows.h>
#	ifdef __WINNT__
#	    include <process.h>
#	endif /* __WINNT__ */
#   else
#       include <Xm/Xm.h>
#       include <Xm/Form.h>
#       include <Xm/DialogS.h>
#       ifdef __MESA__
#           include <GL/glx.h>
#       else
#           include <GL/GLwMDrawA.h>
#       endif /* __MESA__ */
#   endif /* __WINNT__ || __WINCE__ */

#   include <GL/glu.h>
#   include <GL/gl.h>
#endif /* __OPENGL__ */

#include "irit_sm.h"
#include "misc_lib.h"
#include "grap_lib.h"
#include "geom_loc.h"

#define TEST_OGL_ERROR  { int i; \
			  if ((i = glGetError()) != GL_NO_ERROR) \
			      IRIT_WARNING_MSG_PRINTF("OGL Error: %d\n", i); }

#ifdef __OPENGL__

IRIT_STATIC_DATA int
    GlblZBufWidth = 1000,
    GlblZBufHeight = 1000;
IRIT_STATIC_DATA IrtRType
    GlblMinZ = -5000.0,
    GlblMaxZ =  5000.0;

#ifdef __WINNT__

#define ZBUF_OGL_CLASS		"zbuf_olg"
#define ZBUF_OGL_TITLE		IRIT_EXP_STR("ZBuffer using Open GL")

IRIT_STATIC_DATA int
    GlblZBufInit = FALSE;
IRIT_STATIC_DATA HPALETTE ghpalOld,
    GlblPalette = (HPALETTE) 0;
IRIT_STATIC_DATA HDC
    GlblhDC = (HDC) 0;
IRIT_STATIC_DATA HGLRC
    GlblhRC = (HGLRC) 0;

static void HandleWNTEvents(void *Data);
static LONG APIENTRY ZbufWndProc(HWND hWnd,
				 UINT wMsg,
				 WPARAM wParam,
				 LONG lParam);
static unsigned char ComponentFromIndex(int i, UINT NBits, UINT Shift);
static void CreateRGBPalette(HDC hDC);
static BOOL bSetupPixelFormat(HDC hDC);

/*****************************************************************************
* DESCRIPTION:                                                               *
* A secondary thread that waits for input from socket.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Data:      Not used.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void HandleWNTEvents(void *Data)
{
    MSG msg;

    msg.wParam = 1;
    while (TRUE) {
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
	IritSleep(1);			            /* Do not use all CPU. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraw the viewing window.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   hWnd:            Handle on window to draw to.                            *
*   wMsg:            Event to handle.                                        *
*   wParam, lParam:  Some parameters of event.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   LONG APIENTRY:      Window's condition.                                  *
*****************************************************************************/
static LONG APIENTRY ZbufWndProc(HWND hWnd,
				 UINT wMsg,
				 WPARAM wParam,
				 LONG lParam)
{
    switch (wMsg) {
	case WM_CREATE:
	    {
		GlblhDC = GetDC(hWnd);
		bSetupPixelFormat(GlblhDC);

		GlblhRC = wglCreateContext(GlblhDC);
		wglMakeCurrent(GlblhDC, GlblhRC);

		glDisable(GL_FOG);

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);

		glDisable(GL_COLOR_MATERIAL);

		glDisable(GL_LIGHTING);

		glDisable(GL_BLEND);

		glDrawBuffer(GL_FRONT);
		glReadBuffer(GL_FRONT);

		glFrontFace(GL_CW);
		glDisable(GL_CULL_FACE);

		glClearColor(0.0, 0.0, 0.0, 0.0);

		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, GlblZBufWidth, GlblZBufHeight);

		GMZBufferOGLClear();

		GlblZBufInit = TRUE;
		
		return TRUE;
	    }
	    break;

	case WM_PAINT:
	case WM_QUERYNEWPALETTE:
	case WM_PALETTECHANGED:
	    break;

	case WM_DESTROY:
	    PostQuitMessage(0);
	    return 0;

	default:
	    return (LONG) DefWindowProc(hWnd, wMsg, wParam, lParam);
	    break;
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* The following three ugly routines (ComponentFromIndex, CreateRGBPalette    *
* and bSetupPixelFormat) were copied verbatim from the gengl example in      *
* /mstools/samples/opengl/demos/gengl. All they do is to figure out the      *
* color capability of the hardware and create a reasonablepallete for 8 bit  *
* per pixel devices to emulate true 24 bit colors. This should have been a   *
* service provided by the OS.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   i, NBits, Shift:   Dont ask me. I am not responsible for this mess	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   unsigned:          Same!                                                 *
*****************************************************************************/
static unsigned char ComponentFromIndex(int i, UINT NBits, UINT Shift)
{
    IRIT_STATIC_DATA unsigned char
	ThreeTo8[8] = {
	    0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
	},
	TwoTo8[4] = {
	    0, 0x55, 0xaa, 0xff
	},
	OneTo8[2] = {
	    0, 255
	};
    unsigned char
	Val = (unsigned char) (i >> Shift);

    switch (NBits) {
	case 1:
	    Val &= 0x1;
	    return OneTo8[Val];
	case 2:
	    Val &= 0x3;
	    return TwoTo8[Val];
	case 3:
	    Val &= 0x7;
	    return ThreeTo8[Val];
	default:
	    return 0;
    }
}

static void CreateRGBPalette(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *pPal;
    int n, i;
    IRIT_STATIC_DATA int defaultOverride[13] = {
	0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
    };
    IRIT_STATIC_DATA PALETTEENTRY defaultPalEntry[20] = {
	{ 0,   0,   0,    0 },
	{ 0x80,0,   0,    0 },
	{ 0,   0x80,0,    0 },
	{ 0x80,0x80,0,    0 },
	{ 0,   0,   0x80, 0 },
	{ 0x80,0,   0x80, 0 },
	{ 0,   0x80,0x80, 0 },
	{ 0xC0,0xC0,0xC0, 0 },

	{ 192, 220, 192,  0 },
	{ 166, 202, 240,  0 },
	{ 255, 251, 240,  0 },
	{ 160, 160, 164,  0 },

	{ 0x80,0x80,0x80, 0 },
	{ 0xFF,0,   0,    0 },
	{ 0,   0xFF,0,    0 },
	{ 0xFF,0xFF,0,    0 },
	{ 0,   0,   0xFF, 0 },
	{ 0xFF,0,   0xFF, 0 },
	{ 0,   0xFF,0xFF, 0 },
	{ 0xFF,0xFF,0xFF, 0 }
    };

    n = GetPixelFormat(hDC);
    DescribePixelFormat(hDC, n, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
        n = 1 << pfd.cColorBits;
        pPal = (PLOGPALETTE) LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) +
                n * sizeof(PALETTEENTRY));
        pPal -> palVersion = 0x300;
        pPal -> palNumEntries = n;
        for (i=0; i<n; i++) {
            pPal -> palPalEntry[i].peRed =
                    ComponentFromIndex(i, pfd.cRedBits, pfd.cRedShift);
            pPal -> palPalEntry[i].peGreen =
                    ComponentFromIndex(i, pfd.cGreenBits, pfd.cGreenShift);
            pPal -> palPalEntry[i].peBlue =
                    ComponentFromIndex(i, pfd.cBlueBits, pfd.cBlueShift);
            pPal -> palPalEntry[i].peFlags = 0;
        }

        /* Fix up the palette to include the default GDI palette. */
        if ((pfd.cColorBits == 8)                           &&
            (pfd.cRedBits   == 3) && (pfd.cRedShift   == 0) &&
            (pfd.cGreenBits == 3) && (pfd.cGreenShift == 3) &&
            (pfd.cBlueBits  == 2) && (pfd.cBlueShift  == 6)) {
            for (i = 1 ; i <= 12 ; i++)
                pPal -> palPalEntry[defaultOverride[i]] = defaultPalEntry[i];
        }

        GlblPalette = CreatePalette(pPal);
        LocalFree(pPal);

        ghpalOld = SelectPalette(hDC, GlblPalette, FALSE);
        n = RealizePalette(hDC);
    }
}

static BOOL bSetupPixelFormat(HDC hDC)
{
    IRIT_STATIC_DATA PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	/* Size of this pfd. */
	1,				/* Version number. */
	PFD_DRAW_TO_WINDOW |		/* Support window. */
	  PFD_SUPPORT_OPENGL |		/* Support OpenGL. */
	  PFD_DOUBLEBUFFER,		/* Double buffered. */
	PFD_TYPE_RGBA,			/* RGBA type. */
	24,				/* 24-bit color depth. */
	0, 0, 0, 0, 0, 0,		/* Color bits ignored. */
	0,				/* No alpha buffer. */
	0,				/* Shift bit ignored. */
	0,				/* No accumulation buffer. */
	0, 0, 0, 0, 			/* Accum bits ignored. */
	32,				/* 32-bit z-buffer. */
	0,				/* No stencil buffer. */
	0,				/* No auxiliary buffer. */
	PFD_MAIN_PLANE,			/* Main layer. */
	0,				/* Reserved. */
	0, 0, 0				/* Layer masks ignored. */
    };
    int PixelFormat, i;

    if ((PixelFormat = ChoosePixelFormat(hDC, &pfd)) == 0) {
	i = GetLastError();
        MessageBox(NULL, IRIT_EXP_STR("ChoosePixelFormat failed"),
		   "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hDC, PixelFormat, &pfd) == FALSE) {
	i = GetLastError();
        MessageBox(NULL, IRIT_EXP_STR("SetPixelFormat failed"),
		   "Error", MB_OK);
        return FALSE;
    }

    CreateRGBPalette(hDC);

    return TRUE;
}
#else /*  __WINNT__ */
IRIT_STATIC_DATA GLXContext GlxContext;

IRIT_STATIC_DATA Colormap ColorMap;
IRIT_STATIC_DATA float ColorMaps[] = {      /* Snitched from the aux OpenGL lib. */
    0.000000, 1.000000, 0.000000, 1.000000, 0.000000, 1.000000, 
    0.000000, 1.000000, 0.333333, 0.776471, 0.443137, 0.556863, 
    0.443137, 0.556863, 0.219608, 0.666667, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.039216, 0.078431, 0.117647, 0.156863, 
    0.200000, 0.239216, 0.278431, 0.317647, 0.356863, 0.400000, 
    0.439216, 0.478431, 0.517647, 0.556863, 0.600000, 0.639216, 
    0.678431, 0.717647, 0.756863, 0.800000, 0.839216, 0.878431, 
    0.917647, 0.956863, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 0.000000, 0.000000, 
    1.000000, 1.000000, 0.000000, 0.000000, 1.000000, 1.000000, 
    0.333333, 0.443137, 0.776471, 0.556863, 0.443137, 0.219608, 
    0.556863, 0.666667, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.039216, 0.078431, 0.117647, 0.156863, 0.200000, 0.239216, 
    0.278431, 0.317647, 0.356863, 0.400000, 0.439216, 0.478431, 
    0.517647, 0.556863, 0.600000, 0.639216, 0.678431, 0.717647, 
    0.756863, 0.800000, 0.839216, 0.878431, 0.917647, 0.956863, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.141176, 0.282353, 0.427451, 
    0.568627, 0.713726, 0.854902, 1.000000, 0.000000, 0.141176, 
    0.282353, 0.427451, 0.568627, 0.713726, 0.854902, 1.000000, 
    0.000000, 0.141176, 0.282353, 0.427451, 0.568627, 0.713726, 
    0.854902, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 0.333333, 0.443137, 
    0.443137, 0.219608, 0.776471, 0.556863, 0.556863, 0.666667, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.666667, 0.333333, 
    0.666667, 0.333333, 0.666667, 0.333333, 0.039216, 0.078431, 
    0.117647, 0.156863, 0.200000, 0.239216, 0.278431, 0.317647, 
    0.356863, 0.400000, 0.439216, 0.478431, 0.517647, 0.556863, 
    0.600000, 0.639216, 0.678431, 0.717647, 0.756863, 0.800000, 
    0.839216, 0.878431, 0.917647, 0.956863, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
    0.000000, 0.000000, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 0.247059, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 0.498039, 
    0.498039, 0.498039, 0.498039, 0.498039, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 0.749020, 
    0.749020, 0.749020, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
};

IRIT_STATIC_DATA Display *XDisplay;
IRIT_STATIC_DATA int XScreen;
IRIT_STATIC_DATA Window ViewWndw;

static XVisualInfo *FindVisual(int DoDoubleBuffer,
			       int DoRGB,
			       int DoDepth,
			       int DoStencil,
			       int DoAlpha,
			       int DoAccum);
static void SetRGBMap(XVisualInfo *Vi, int Size, float *rgb);
static int WaitForMapNotify(Display *d, XEvent *e, char *arg);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The following routines were snitched from the aux OpenGL library.        *
* Try anonymous ftp to sgigate.sgi.com and cd to pub/opengl for opengl.tar.Z.*
*****************************************************************************/
static XVisualInfo *FindVisual(int DoDoubleBuffer,
			       int DoRGB,
			       int DoDepth,
			       int DoStencil,
			       int DoAlpha,
			       int DoAccum)
{
    GLenum List[20];
    int i = 0;

    if (DoDoubleBuffer) {
	List[i++] = GLX_DOUBLEBUFFER;
    }

    if (DoRGB) {
	List[i++] = GLX_RGBA;
	List[i++] = GLX_RED_SIZE;
	List[i++] = 1;
	List[i++] = GLX_GREEN_SIZE;
	List[i++] = 1;
	List[i++] = GLX_BLUE_SIZE;
	List[i++] = 1;
	if (DoAlpha) {
	    List[i++] = GLX_ALPHA_SIZE;
	    List[i++] = 1;
	}
	if (DoAccum) {
	    List[i++] = GLX_ACCUM_RED_SIZE;
	    List[i++] = 1;
	    List[i++] = GLX_ACCUM_GREEN_SIZE;
	    List[i++] = 1;
	    List[i++] = GLX_ACCUM_BLUE_SIZE;
	    List[i++] = 1;
	    List[i++] = GLX_ACCUM_ALPHA_SIZE;
	    List[i++] = 1;
	}
    } else {
	List[i++] = GLX_BUFFER_SIZE;
	List[i++] = 1;
    }

    if (DoDepth) {
	List[i++] = GLX_DEPTH_SIZE;
	List[i++] = 1;
    }

    if (DoStencil) {
	List[i++] = GLX_STENCIL_SIZE;
	List[i++] = 1;
    }

    List[i] = (int) None;

    return glXChooseVisual(XDisplay, XScreen, (int *) List);
}

static void SetRGBMap(XVisualInfo *Vi, int Size, float *rgb)
{
    XColor c;
    int rShift, gShift, bShift, Max, i;

    switch (Vi -> class) {
	case DirectColor:
	    Max = (Size > Vi -> colormap_size) ? Vi -> colormap_size : Size;
	    for (i = 0; i < Max; i++) {
		rShift = ffs((unsigned int) Vi -> red_mask) - 1;
		gShift = ffs((unsigned int) Vi -> green_mask) - 1;
		bShift = ffs((unsigned int) Vi -> blue_mask) - 1;
		c.pixel = ((i << rShift) & Vi -> red_mask) |
		          ((i << gShift) & Vi -> green_mask) |
		          ((i << bShift) & Vi -> blue_mask);
		c.red =   (unsigned short)(rgb[i] * 65535.0 + 0.5);
		c.green = (unsigned short)(rgb[Size+i] * 65535.0 + 0.5);
		c.blue =  (unsigned short)(rgb[Size*2+i] * 65535.0 + 0.5);
		c.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(XDisplay, ColorMap, &c);
	    }
	    break;
	case GrayScale:
	case PseudoColor:
	    Max = (Size > Vi -> colormap_size) ? Vi -> colormap_size : Size;
	    for (i = 0; i < Max; i++) {
		c.pixel = i;
		c.red =   (unsigned short) (rgb[i] * 65535.0 + 0.5);
		c.green = (unsigned short) (rgb[Size + i] * 65535.0 + 0.5);
		c.blue =  (unsigned short) (rgb[Size*2+i] * 65535.0 + 0.5);
		c.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(XDisplay, ColorMap, &c);
	    }
	    break;
    }

    XSync(XDisplay, 0);
}

static int WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
    if (e -> type == MapNotify && e -> xmap.window == ViewWndw) {
	return GL_TRUE;
    }
    return GL_FALSE;
}

#endif /*  __WINNT__ */
#endif /* __OPENGL__ */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets up the Zbuffer implementation.  This one is employing Open GL.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Width, Height: Width and Height of the Z buffer.                         M
*   ZMin, ZMax:    Z domain that the Z buffer will have to support.          M
*   OffScreen:     Z buffer should be hidden (TRUE) or displayed (FALSE).    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IritIntPtrSizeType:	An I.D. of the constructed Z buffer.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLInit                                                         M
*****************************************************************************/
IritIntPtrSizeType GMZBufferOGLInit(int Width,
				    int Height,
				    IrtRType ZMin,
				    IrtRType ZMax,
				    int OffScreen)
{
#ifdef __OPENGL__
#ifdef __WINNT__
    HWND hWndZBuf;
    WNDCLASS wndClass;
    int Registered = TRUE;

    GlblZBufWidth = Width;
    GlblZBufHeight = Height;
    GlblMaxZ = IRIT_MAX(IRIT_FABS(ZMin), IRIT_FABS(ZMax));
    GlblMinZ = -GlblMaxZ;

    /* Set up common defaults. */
    wndClass.style         = CS_SAVEBITS;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hCursor       = NULL;
    wndClass.hInstance     = NULL;
    wndClass.lpfnWndProc   = (WNDPROC) ZbufWndProc;
    wndClass.hIcon         = NULL;
    wndClass.lpszMenuName  = NULL;
    wndClass.hbrBackground = GetStockObject(BLACK_BRUSH);
    wndClass.lpszClassName = ZBUF_OGL_CLASS;
    if (!RegisterClass(&wndClass))
	return 0;

    hWndZBuf = CreateWindow(ZBUF_OGL_CLASS, ZBUF_OGL_TITLE,
			    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			    0, 0, Width, Height,
			    NULL, NULL, NULL, NULL);

    if (hWndZBuf) {
        ShowWindow(hWndZBuf, SW_SHOW);
        UpdateWindow(hWndZBuf);

	/* Set up event handler of this window as a secondary thread.      */
	_beginthread(HandleWNTEvents, 0, NULL);

	/* And wait for the secondary thread to initialize the Z buffer. */
	while (!GlblZBufInit)
	    IritSleep(1);
    }
    else {
	LPVOID Msg;
	
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		      FORMAT_MESSAGE_FROM_SYSTEM,
		      NULL, GetLastError(),
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      (LPTSTR) &Msg, 0, NULL);
	IRIT_WARNING_MSG_PRINTF("GMZBufferOGLInit error: %s", Msg);
	LocalFree(Msg);
    }
    return (IritIntPtrSizeType) hWndZBuf;
#else /* __WINNT__ */
    char
	*Title = "zbuf_ogl";
    XSetWindowAttributes Wa;
    XSizeHints Sh;
    XEvent e;
    unsigned long Mask;
    XVisualInfo *Vi;

    GlblZBufWidth = Width;
    GlblZBufHeight = Height;
    GlblMaxZ = IRIT_MAX(IRIT_FABS(ZMin), IRIT_FABS(ZMax));
    GlblMinZ = -GlblMaxZ;

    /* Lets see if we can get access to the X server before we even start: */
    if ((XDisplay = (Display *) XOpenDisplay(NULL)) == NULL) {
        GEOM_FATAL_ERROR(GEOM_ERR_OGL_NO_X_SERVER);
    }
    XScreen = DefaultScreen(XDisplay);

    if (!glXQueryExtension(XDisplay, NULL, NULL)) {
        GEOM_FATAL_ERROR(GEOM_ERR_X_NO_OGL_SERVER);
	return 0;
    }

    Vi = FindVisual(FALSE, TRUE, TRUE, FALSE, FALSE, FALSE);
    if (Vi == NULL) {
        GEOM_FATAL_ERROR(GEOM_ERR_X_NO_VISUAL);
    }

    GlxContext = glXCreateContext(XDisplay, Vi, None,
				  OffScreen ? GL_FALSE : GL_TRUE);
    if (!GlxContext) {
        GEOM_FATAL_ERROR(GEOM_ERR_X_NO_GCONTEXT);
	return 0;
    }

    if (Vi -> class == DirectColor || Vi -> class == TrueColor) {
	ColorMap = XCreateColormap(XDisplay, RootWindow(XDisplay, XScreen),
				   Vi -> visual, AllocNone);
	Wa.colormap = ColorMap;
	SetRGBMap(Vi, 256, ColorMaps);
	Wa.background_pixel = 0xFFFFFFFF;
	Wa.border_pixel = 0;
    }
    else {
	if (Vi -> class != StaticColor && Vi -> class != StaticGray) {
	    ColorMap = XCreateColormap(XDisplay, RootWindow(XDisplay, XScreen),
				       Vi -> visual, AllocAll);
	} else {
	    ColorMap = XCreateColormap(XDisplay, RootWindow(XDisplay, XScreen),
				       Vi -> visual, AllocNone);
	}
	Wa.colormap = ColorMap;
	SetRGBMap(Vi, 256, ColorMaps);
	Wa.background_pixel = 7;
	Wa.border_pixel = 0;
    }

    if (OffScreen) {
        Pixmap PixMap;
	GLXPixmap GlxPixMap;

	PixMap = XCreatePixmap(XDisplay, RootWindow(XDisplay, XScreen),
			       Width, Height, Vi -> depth);
        GlxPixMap = glXCreateGLXPixmap(XDisplay, Vi, PixMap);

	if (!glXMakeCurrent(XDisplay, GlxPixMap, GlxContext)) {
	    GEOM_FATAL_ERROR(GEOM_ERR_X_NO_GCONTEXT);
	    return 0;
	}
    }
    else {
        Wa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask |
	                ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
	Mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

	ViewWndw = XCreateWindow(XDisplay, RootWindow(XDisplay, XScreen), 0, 0,
				 GlblZBufWidth, GlblZBufHeight, 0,
				 Vi -> depth, InputOutput, Vi -> visual,
				 Mask, &Wa);

	Sh.flags = USPosition; /* Set position at origin. */
	Sh.x = 1;
	Sh.y = 1;

	XSetStandardProperties(XDisplay, ViewWndw, Title, Title,
			       None, 0, 0, &Sh);

	XMapWindow(XDisplay, ViewWndw);
	XIfEvent(XDisplay, &e, WaitForMapNotify, 0);

	XSetWMColormapWindows(XDisplay, ViewWndw, &ViewWndw, 1);

	if (!glXMakeCurrent(XDisplay, ViewWndw, GlxContext)) {
	    GEOM_FATAL_ERROR(GEOM_ERR_X_NO_GCONTEXT);
	    return 0;
	}
    }

    XFlush(XDisplay);

    glDisable(GL_FOG);

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);

    glDisable(GL_COLOR_MATERIAL);

    glDisable(GL_LIGHTING);

    glDisable(GL_BLEND);

    glDrawBuffer(GL_FRONT);
    glReadBuffer(GL_FRONT);

    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, GlblZBufWidth, GlblZBufHeight);

    GMZBufferOGLClear();

    return (IritIntPtrSizeType) GlxContext;
#endif /* __WINNT__ */
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
    return (IritIntPtrSizeType) 0;
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clears the Z buffer to initialization state.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   None	                                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLClear                                                        M
*****************************************************************************/
void GMZBufferOGLClear(void)
{
#ifdef __OPENGL__
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, GlblZBufWidth, 0.0, GlblZBufHeight, GlblMinZ, GlblMaxZ);

    glPopAttrib();
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the colors of all drawing operations to come.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Red, Green, Blue:      The color specifications, each between 0 and 255. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLSetColor                                                     M
*****************************************************************************/
void GMZBufferOGLSetColor(int Red, int Green, int Blue)
{
#ifdef __OPENGL__
    glColor3ub((GLubyte) Red, (GLubyte) Green, (GLubyte) Blue);
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Make active context.  Once this function called drawing commands may be  M
* issued.  Here, the drawing commands are in Open GL.  Necessary only if     M
* other Open GL applications are active simultaneously.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Id:  I.D. of the Zbuffer to activate.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLMakeActive                                                   M
*****************************************************************************/
void GMZBufferOGLMakeActive(IritIntPtrSizeType Id)
{
#ifdef __OPENGL__
#ifdef __WINNT__
    if (!wglMakeCurrent(GlblhDC, GlblhRC)) {
#else
    if (!glXMakeCurrent(XDisplay, ViewWndw, (GLXContext) Id)) {
#endif /* __WINNT__ */
        GEOM_FATAL_ERROR(GEOM_ERR_X_NO_GCONTEXT);
    }
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns depth at the given location in the Zbuffer.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:   The XY coordinates of the point to consider for visibility.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The depth found at that XY location.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferOGLQueryColor                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLQueryZ                                                       M
*****************************************************************************/
IrtRType GMZBufferOGLQueryZ(IrtRType x, IrtRType y)
{
#ifdef __OPENGL__
    IRIT_STATIC_DATA GLint
	IxPrev = -1,
	IyPrev = -1;
    IRIT_STATIC_DATA float
	PrevZ = (float) IRIT_INFNTY;
    GLint
	Ix = (GLint) (x + 0.5),
	Iy = (GLint) (y + 0.5);

    if (Ix == IxPrev && Iy == IyPrev)
	return PrevZ;
    else {
	float f;

	IxPrev = Ix;
	IyPrev = Iy;

	glReadPixels(Ix, Iy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &f);

	PrevZ = (float) (-(GlblMinZ + (GlblMaxZ - GlblMinZ) * f));
	return PrevZ;
    }
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
    return 0.0;
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the color at the given location in the Zbuffer.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:             The XY coordinates of the point to consider for color. M
*   Red, Green, Blue: The color specifications.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMZBufferOGLQueryZ                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLQueryColor                                                   M
*****************************************************************************/
void GMZBufferOGLQueryColor(IrtRType x,
			    IrtRType y,
			    int *Red,
			    int *Green,
			    int *Blue)
{
#ifdef __OPENGL__
    unsigned char Color[3];

    glReadPixels((GLint) (x + 0.5), (GLint) (y + 0.5), 1, 1,
		 GL_RGB, GL_UNSIGNED_BYTE, &Color);

    *Red = Color[0];
    *Green = Color[1];
    *Blue = Color[2];
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
#endif /* __OPENGL__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Make sure all drawing commands are flushed and we are in sync.           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMZBufferOGLFlush                                                        M
*****************************************************************************/
void GMZBufferOGLFlush(void)
{
#ifdef __OPENGL__
    glFlush();
#else
    GEOM_FATAL_ERROR(GEOM_ERR_NO_OGL_SUPPORT);
#endif /* __OPENGL__ */
}
