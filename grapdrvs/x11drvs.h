/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Jan. 1992   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Global definitions of	x11drvs interface.			             *
*****************************************************************************/

#ifndef	X11DRVS_H	/* Define only once */
#define	X11DRVS_H

#define RESOURCE_NAME		"irit"

#define DEFAULT_VIEW_WIDTH	400
#define DEFAULT_VIEW_HEIGHT	400

IRIT_GLOBAL_DATA_HEADER int
    IGViewHasSize,
    IGViewHasPos,
    IGViewPosX,
    IGViewPosY,
    IGXScreen;
IRIT_GLOBAL_DATA_HEADER unsigned int
    IGMaxColors,
    IGViewBackGroundPixel,
    IGViewTextPixel,
    IGViewBorderPixel,
    IGViewBorderWidth,
    IGViewWidth,
    IGViewHeight;
IRIT_GLOBAL_DATA_HEADER Display
    *IGXDisplay;
IRIT_GLOBAL_DATA_HEADER Window
    IGXRoot,
    IGViewWndw;
IRIT_GLOBAL_DATA_HEADER Colormap
    IGXColorMap;
IRIT_GLOBAL_DATA_HEADER GC
    IGXViewGraphContext;
IRIT_GLOBAL_DATA_HEADER XColor
    IGBlackColor,
    *IGViewCursorColor,
    IGXViewColorsHigh[IG_MAX_COLOR + 1],
    IGXViewColorsLow[IG_MAX_COLOR + 1];

void SetViewWindow(int argc, char **argv);

#endif /* X11DRVS_H */
