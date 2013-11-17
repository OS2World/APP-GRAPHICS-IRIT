/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Feb. 1995   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Global definitions of	xsbmdrvs interface.			             *
*****************************************************************************/

#ifndef	XSBDRVS_H	/* Define only once */
#define	XSBDRVS_H

/* These macros are the values of the "transparentType" above: */
#ifndef None
#define None 0
#endif
#ifndef TransparentPixel
#define TransparentPixel	1
#endif


/* These macros define how flexible a program is when it requests a window's
 * creation with either the CreateImagePlanesWindow() or
 * CreateOverlayPlanesWindow():
 */
#ifndef NOT_FLEXIBLE
#define NOT_FLEXIBLE		0
#define FLEXIBLE		1
#endif

/* These macros define the values of the "sbCmapHint" parameter of the
 * CreateImagePlanesWindow():
 */
#ifndef SB_CMAP_TYPE_NORMAL
#define SB_CMAP_TYPE_NORMAL	1
#endif

#ifndef SB_CMAP_TYPE_MONOTONIC
#define SB_CMAP_TYPE_MONOTONIC	2
#endif

#ifndef SB_CMAP_TYPE_FULL
#define SB_CMAP_TYPE_FULL	4
#endif

typedef struct
{
    VisualID	visualID;
    int		transparentType;
    int		value;
    int		layer;
} OverlayVisualPropertyRec;

typedef struct
{
    XVisualInfo	*pOverlayVisualInfo;
    int		transparentType;
    int		value;
    int		layer;
} OverlayInfo;

int GetXVisualInfo(Display *display,
		   int screen,
		   int *transparentOverlays,
		   int *numVisuals,
		   XVisualInfo **pVisuals,
		   int *numOverlayVisuals,
		   OverlayInfo **pOverlayVisuals,
		   int *numImageVisuals,
		   XVisualInfo ***pImageVisuals);
int FindImagePlanesVisual(Display *display,
			  int screen,
			  int numImageVisuals,
			  XVisualInfo **pImageVisuals,
			  int sbCmapHint,
			  int depthHint,
			  int depthFlexibility,
			  Visual **pImageVisualToUse,
			  int *depthObtained);
int CreateImagePlanesWindow(Display *display,
			    int screen,
			    Window parentWindow,
			    int windowX,
			    int windowY,
			    int windowWidth,
			    int windowHeight,
			    int windowDepth,
			    Visual *pImageVisualToUse,
			    int argc,
			    char **argv,
			    char *windowName,
			    char *iconName,
			    Window *imageWindow,
			    Colormap *imageColormap,
			    int *mustFreeImageColormap);

#endif /* XSBDRVS_H */
