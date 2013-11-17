/*****************************************************************************
*   An SGI 4D driver using Open GL.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DialogS.h>
#include <X11/cursorfont.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "grap_loc.h"
#include "geom_lib.h"
#include "xmtdrvs.h"

IRIT_STATIC_DATA GLXContext GlxContext;
IRIT_STATIC_DATA Colormap ColorMap;
IRIT_STATIC_DATA float ColorMaps[] = { /* Snitched from the aux OpenGL lib. */
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

static XVisualInfo *FindVisual(int DoDoubleBuffer,
			       int DoRGB,
			       int DoDepth,
			       int DoStencil,
			       int DoAlpha,
			       int DoAccum);
static void SetRGBMap(XVisualInfo *Vi, int Size, float *rgb);
static int WaitForMapNotify(Display *d, XEvent *e, char *arg);
static void RedrawOneViewWindow(int ClearAll);

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
	List[i++] = (GLenum) GLX_DOUBLEBUFFER;
    }

    if (DoRGB) {
	List[i++] = (GLenum) GLX_RGBA;
	List[i++] = (GLenum) GLX_RED_SIZE;
	List[i++] = (GLenum) 1;
	List[i++] = (GLenum) GLX_GREEN_SIZE;
	List[i++] = (GLenum) 1;
	List[i++] = (GLenum) GLX_BLUE_SIZE;
	List[i++] = (GLenum) 1;
	if (DoAlpha) {
	    List[i++] = (GLenum) GLX_ALPHA_SIZE;
	    List[i++] = (GLenum) 1;
	}
	if (DoAccum) {
	    List[i++] = (GLenum) GLX_ACCUM_RED_SIZE;
	    List[i++] = (GLenum) 1;
	    List[i++] = (GLenum) GLX_ACCUM_GREEN_SIZE;
	    List[i++] = (GLenum) 1;
	    List[i++] = (GLenum) GLX_ACCUM_BLUE_SIZE;
	    List[i++] = (GLenum) 1;
	    List[i++] = (GLenum) GLX_ACCUM_ALPHA_SIZE;
	    List[i++] = (GLenum) 1;
	}
    } else {
	List[i++] = (GLenum) GLX_BUFFER_SIZE;
	List[i++] = (GLenum) 1;
    }

    if (DoDepth) {
	List[i++] = (GLenum) GLX_DEPTH_SIZE;
	List[i++] = (GLenum) 1;
    }

    if (DoStencil) {
	List[i++] = (GLenum) GLX_STENCIL_SIZE;
	List[i++] = (GLenum) 1;
    }

    List[i] = (int) None;

    return glXChooseVisual(IGXDisplay, IGXScreen, (int *) List);
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
		XStoreColor(IGXDisplay, ColorMap, &c);
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
		XStoreColor(IGXDisplay, ColorMap, &c);
	    }
	    break;
    }

    XSync(IGXDisplay, 0);
}

static int WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
    if (e -> type == MapNotify && e -> xmap.window == IGViewWndw) {
	return GL_TRUE;
    }
    return GL_FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets up a view window.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:   Command line,                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SetViewWindow                                                            M
*****************************************************************************/
void SetViewWindow(int argc, char **argv)
{
    int i;
    XSetWindowAttributes Wa;
    XSizeHints Sh;
    XEvent e;
    unsigned long Mask;
    XVisualInfo *Vi;
    GLfloat FogColor[4];
    IRIT_STATIC_DATA IrtVecType
	WhiteColor = { 1.0, 1.0, 1.0 };

    Vi = FindVisual(TRUE, TRUE, TRUE, FALSE, FALSE, FALSE);
    if (Vi == NULL &&
	(Vi = FindVisual(FALSE, TRUE, FALSE, FALSE, FALSE, FALSE)) != NULL)
	fprintf(stderr, "No double buffer/Z buffer\n");
    if (Vi == NULL &&
	(Vi = FindVisual(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)) != NULL)
	fprintf(stderr, "No double buffer/Z buffer/True Color\n");
    if (Vi == NULL)
	fprintf(stderr,
		"Failed to allocate visual - chances are we are to die.\n");

    GlxContext = glXCreateContext(IGXDisplay, Vi, None, GL_TRUE);
    if (!GlxContext) {
	fprintf(stderr, "Can't create a graphics context!\n");
	return;
    }

    if (Vi -> class == DirectColor || Vi -> class == TrueColor) {
	ColorMap = XCreateColormap(IGXDisplay,
				   RootWindow(IGXDisplay, IGXScreen),
				   Vi -> visual, AllocNone);
	Wa.colormap = ColorMap;
	SetRGBMap(Vi, 256, ColorMaps);
	Wa.background_pixel = 0xFFFFFFFF;
	Wa.border_pixel = 0;
    }
    else {
	if (Vi -> class != StaticColor && Vi -> class != StaticGray) {
	    ColorMap = XCreateColormap(IGXDisplay,
				       RootWindow(IGXDisplay, IGXScreen),
				       Vi -> visual, AllocAll);
	} else {
	    ColorMap = XCreateColormap(IGXDisplay,
				       RootWindow(IGXDisplay, IGXScreen),
				       Vi -> visual, AllocNone);
	}
	Wa.colormap = ColorMap;
	SetRGBMap(Vi, 256, ColorMaps);
	Wa.background_pixel = 7;
	Wa.border_pixel = 0;
     }

    Wa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask |
		    ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
    Mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

    IGViewWndw = XCreateWindow(IGXDisplay, RootWindow(IGXDisplay, IGXScreen),
			       0, 0, IGViewWidth, IGViewHeight, 0,
			       Vi -> depth, InputOutput, Vi -> visual,
			       Mask, &Wa);

    if (IGViewHasPos) {
	Sh.flags = USPosition;
	Sh.x = IGViewPosX;
	Sh.y = IGViewPosY;
    }

    XSetStandardProperties(IGXDisplay, IGViewWndw,
			   RESOURCE_NAME, RESOURCE_NAME, None, 0, 0, &Sh);

    XMapWindow(IGXDisplay, IGViewWndw);
    XIfEvent(IGXDisplay, &e, WaitForMapNotify, 0);

    XSetWMColormapWindows(IGXDisplay, IGViewWndw, &IGViewWndw, 1);
    if (!glXMakeCurrent(IGXDisplay, IGViewWndw, GlxContext)) {
	fprintf(stderr, "Failed to make current context\n");
    }
    XFlush(IGXDisplay);

    /* Set up global Open GL defaults. */
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

    for (i = 0; i < IGShadeParam.NumOfLightSrcs; i++)
	IGSetLightSource(IGShadeParam.LightPos[i],
			 WhiteColor, i );
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Redraw one view window.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   ClearAll: TRUE to clear both the Z buffer and RGB, FALSE only Z buffer.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawOneViewWindow(int ClearAll)
{
    int i;
    char Line[IRIT_LINE_LEN_LONG];

    if (IGGlblCountNumPolys)
	IGGlblNumPolys = 0;
    if (IGGlblCountFramePerSec)
        IGUpdateFPS(TRUE);

    if (IGViewWidth > IGViewHeight) {
	i = ((IGViewWidth - IGViewHeight) >> 1);

	glViewport(0, -i, IGViewWidth, IGViewWidth);
    }
    else {
	i = ((IGViewHeight - IGViewWidth) >> 1);

	glViewport(-i, 0, IGViewHeight, IGViewHeight);
    }

    IGRedrawViewWindowOGL(ClearAll);

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
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redraw the view window.                                                  M
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
    IGPredefinedAnimation();

    if (IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_BLUE ||
	IGGlbl3DGlassesMode == IG_GLASSES_3D_RED_GREEN) {
        for (IGGlbl3DGlassesImgIndx = 0;
	     IGGlbl3DGlassesImgIndx < 2;
	     IGGlbl3DGlassesImgIndx++)
	    RedrawOneViewWindow(IGGlbl3DGlassesImgIndx == 0);
    }
    else {
        RedrawOneViewWindow(TRUE);
    }

    if (IGGlblDoDoubleBuffer)
	glXSwapBuffers(IGXDisplay, IGViewWndw);
}
