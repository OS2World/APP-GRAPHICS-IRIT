/*****************************************************************************
*   An X11 driver using only libx11.a.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xresource.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

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
#include "x11drvs.h"

#define X11_FONT_NAME		"8x13"

#define DEFAULT_TRANS_WIDTH	200
#define DEFAULT_TRANS_HEIGHT	500

/* If your X does not recognizes XrmGetDatabase, uncomment the following */
/* and try again:							 */
/* #define XrmGetDatabase(XDisplay) ((XDisplay)->db)			 */

IRIT_STATIC_DATA GC XTransGraphContext;
IRIT_STATIC_DATA XFontStruct *XLoadedFont;
IRIT_STATIC_DATA XColor
    *TransCursorColor = NULL;
IRIT_STATIC_DATA unsigned long
    TransBackGroundPixel,
    TransBorderPixel,
    TransTextPixel,
    TransSubWinBackPixel,
    TransSubWinBorderPixel;
IRIT_STATIC_DATA int
    ReturnPickedObject = FALSE,
    ReturnPickedObjectName = TRUE,
    PickCrsrGrabMouse = FALSE,
    XFontYOffsetToCenter = 0,
    TransHasSize = FALSE,
    TransHasPos = FALSE,
    TransPosX = 0,
    TransPosY = 0;
IRIT_STATIC_DATA unsigned int
    TransWidth = DEFAULT_TRANS_WIDTH,
    TransHeight = DEFAULT_TRANS_HEIGHT;

/* X Colors to be used for viewed object (see also grap_lib/drvs.h): */
IRIT_STATIC_DATA int XViewColorDefs[IG_MAX_COLOR + 1][3] =
{
    {     0,     0,     0 },  /* 0. IG_IRIT_BLACK */
    {     0,     0, 43350 },  /* 1. IG_IRIT_BLUE */
    {     0, 43350,     0 },  /* 2. IG_IRIT_GREEN */
    {     0, 43350, 43350 },  /* 3. IG_IRIT_CYAN */
    { 43350,     0,     0 },  /* 4. IG_IRIT_RED */
    { 43350,     0, 43350 },  /* 5. IG_IRIT_MAGENTA */
    { 43350, 43350,     0 },  /* 6. IG_IRIT_BROWN */
    { 43350, 43350, 43350 },  /* 7. IG_IRIT_LIGHTGREY */
    { 21675, 21675, 21675 },  /* 8. IG_IRIT_DARKGRAY */
    { 21675, 21675, 65535 },  /* 9. IG_IRIT_LIGHTBLUE */
    { 21675, 65535, 21675 },  /* 10. IG_IRIT_LIGHTGREEN */
    { 21675, 65535, 65535 },  /* 11. IG_IRIT_LIGHTCYAN */
    { 65535, 21675, 21675 },  /* 12. IG_IRIT_LIGHTRED */
    { 65535, 21675, 65535 },  /* 13. IG_IRIT_LIGHTMAGENTA */
    { 65535, 65535, 21675 },  /* 14. IG_IRIT_YELLOW */
    { 65535, 65535, 65535 }   /* 15. IG_IRIT_WHITE */
};

/* X transformation window staff goes here: */
IRIT_STATIC_DATA Window TransformWndw;
IRIT_STATIC_DATA Window ObjScrTglWndw;
IRIT_STATIC_DATA Window PersOrthoTglWndw, PersOrthoZWndw;
IRIT_STATIC_DATA Window RotateXWndw, RotateYWndw, RotateZWndw;
IRIT_STATIC_DATA Window TranslateXWndw, TranslateYWndw, TranslateZWndw;
IRIT_STATIC_DATA Window ScaleWndw;
IRIT_STATIC_DATA Window DepthCueWndw;
IRIT_STATIC_DATA Window AnimationWndw;
IRIT_STATIC_DATA Window SaveMatrixWndw;
IRIT_STATIC_DATA Window SubmitMatrixWndw;
IRIT_STATIC_DATA Window PushMatrixWndw;
IRIT_STATIC_DATA Window PopMatrixWndw;
IRIT_STATIC_DATA Window QuitWndw;

/* Viewing state variables: */
IRIT_STATIC_DATA int
    SubWindowWidthState2 = 1,
    SubWindowHeightState2 = 1;

/* X global specific staff goes here: */
IRIT_GLOBAL_DATA XColor IGXViewColorsHigh[IG_MAX_COLOR + 1];
IRIT_GLOBAL_DATA XColor IGXViewColorsLow[IG_MAX_COLOR + 1];

IRIT_GLOBAL_DATA int
    IGViewHasSize = FALSE,
    IGViewHasPos = FALSE,
    IGViewPosX = 0,
    IGViewPosY = 0,
    IGXScreen;
IRIT_GLOBAL_DATA unsigned int
    IGMaxColors = IG_MAX_COLOR,
    IGViewBackGroundPixel,
    IGViewBorderPixel,
    IGViewTextPixel,
    IGViewBorderWidth = 1,
    IGViewWidth = DEFAULT_VIEW_WIDTH,
    IGViewHeight = DEFAULT_VIEW_HEIGHT;
IRIT_GLOBAL_DATA Colormap IGXColorMap;
IRIT_GLOBAL_DATA XColor IGBlackColor,
    *IGViewCursorColor = NULL;
IRIT_GLOBAL_DATA Display *IGXDisplay;
IRIT_GLOBAL_DATA Window IGXRoot, IGViewWndw;
IRIT_GLOBAL_DATA GC IGXViewGraphContext;

static char *ReadOneXDefault(char *Entry);
static void ReadXDefaults(void);
static void SetTransformWindow(int argc, char **argv);
static void RedrawTransformWindow(void);
static Window SetTransformSubWindow(int SubTransPosX,
				    int SubTransPosY,
				    unsigned int SubTransWidth,
				    unsigned int SubTransHeight);
static void RedrawTransformSubWindow(Window Win,
				     int SubTransPosX,
				     int SubTransPosY,
				     unsigned int SubTransWidth,
				     unsigned int SubTransHeight,
				     int DrawMiddleVertLine,
				     char *DrawString);
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor);
static void DrawText(Window Win,
		     char *Str,
		     int PosX,
		     int PosY,
		     unsigned long Color);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of x11drvs - X11 graphics driver of IRIT.             	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Return code.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char **argv)
{
    int i;
    XGCValues values;
    IrtRType ChangeFactor[2];
    IGGraphicEventType Event;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    IGConfigureGlobals("x11drvs", argc, argv);

    /* Must select a function to draw polys. */
    IGDrawPolyFuncPtr = IGDrawPoly;

    /* Lets see if we can get access to the X server before we even start: */
    if ((IGXDisplay = (Display *) XOpenDisplay(NULL)) == NULL) {
	fprintf(stderr, "x11drvs: Failed to access X server, aborted.\n");
        exit(-1);
    }
    if ((XLoadedFont = XLoadQueryFont(IGXDisplay, X11_FONT_NAME)) == NULL) {
	fprintf(stderr,
		"x11drvs: Failed to load required X font \"%s\", aborted.\n",
		X11_FONT_NAME);
	exit(-1);
    }
    XFontYOffsetToCenter = (XLoadedFont -> ascent - XLoadedFont -> descent + 1)
									/ 2;

    IGXScreen = DefaultScreen(IGXDisplay);
    IGXRoot = RootWindow(IGXDisplay, IGXScreen);
    IGXColorMap = DefaultColormap(IGXDisplay, IGXScreen);
    values.foreground = WhitePixel(IGXDisplay, IGXScreen);
    values.background = BlackPixel(IGXDisplay, IGXScreen);
    values.font = XLoadedFont -> fid;
    XTransGraphContext = XCreateGC(IGXDisplay, IGXRoot,
			      GCForeground | GCBackground | GCFont, &values);
    IGXViewGraphContext = XCreateGC(IGXDisplay, IGXRoot,
				    GCForeground | GCBackground, &values);
    
    if (XrmGetDatabase(IGXDisplay) == NULL)
	XGetDefault(IGXDisplay, "", "");
    ReadXDefaults();

    for (i = 0; i <= IG_MAX_COLOR; i++) {
	IGXViewColorsHigh[i].red   = XViewColorDefs[i][0];
	IGXViewColorsHigh[i].green = XViewColorDefs[i][1];
	IGXViewColorsHigh[i].blue  = XViewColorDefs[i][2];

	/* If fails to allocate the color - take WHITE instead. */
	if (!XAllocColor(IGXDisplay, IGXColorMap, &IGXViewColorsHigh[i]))
	    IGXViewColorsHigh[i].pixel = WhitePixel(IGXDisplay, IGXScreen);

	IGXViewColorsLow[i].red   = XViewColorDefs[i][0] / 2;
	IGXViewColorsLow[i].green = XViewColorDefs[i][1] / 2;
	IGXViewColorsLow[i].blue  = XViewColorDefs[i][2] / 2;

	/* If fails to allocate the color - take WHITE instead. */
	if (!XAllocColor(IGXDisplay, IGXColorMap, &IGXViewColorsLow[i]))
	    IGXViewColorsLow[i].pixel = WhitePixel(IGXDisplay, IGXScreen);
    }

    values.line_width = IGGlblLineWidth;
    XChangeGC(IGXDisplay, IGXViewGraphContext, GCLineWidth, &values);

    IGCreateStateMenu();

    SetTransformWindow(argc, argv);
    SetViewWindow(argc, argv);

    sleep(1); /* Some systems get confused if we draw immediately. */

    XFlush(IGXDisplay);

    while ((Event = GetGraphicEvent(ChangeFactor)) != IG_EVENT_QUIT) {
        ChangeFactor[0] *= IGGlblChangeFactor;
	ChangeFactor[1] *= IGGlblChangeFactor;

	if (IGProcessEvent(Event, ChangeFactor))
	    IGRedrawViewWindow();
    }

    if (IGGlblIOHandle >= 0)
        IPCloseStream(IGGlblIOHandle, TRUE);

    XFreeGC(IGXDisplay, IGXViewGraphContext);
    XFreeGC(IGXDisplay, XTransGraphContext);
    XUnloadFont(IGXDisplay, XLoadedFont -> fid);
    XCloseDisplay(IGXDisplay);

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the curve editor, if not already up and hook CrvObj to it.         M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:    Curve to edit.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupCrvEditor                                                         M
*****************************************************************************/
void IGPopupCrvEditor(IPObjectStruct *CrvObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the surface editor, if not already up and hook SrfObj to it.       M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:    Surface to edit.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupSrfEditor                                                         M
*****************************************************************************/
void IGPopupSrfEditor(IPObjectStruct *SrfObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Popup the object editor, if not already up and hook PObj to it.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to edit.                                                 M
*   CloneIt: If TRUE make a copy of given object fist and edit the clone.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGPopupObjEditor                                                         M
*****************************************************************************/
void IGPopupObjEditor(IPObjectStruct *PObj, int CloneIt)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Optionally construct a state pop up menu for the driver, if has one.       M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCreateStateMenu                                                        M
*****************************************************************************/
void IGCreateStateMenu(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function initilized the subview mat to front, side, top & Isometry  M
* views.                                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGInitializeSubViewMat                                                   M
*****************************************************************************/
void IGInitializeSubViewMat(void)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Function to enable/disable 4views mode.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Display4Views:  TRUE for 4 views mode.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if a change in views' style occured.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSetDisplay4Views                                                       M
*****************************************************************************/
int IGSetDisplay4Views(int Display4Views)
{
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Reads one default from X resource data base.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Entry:    String name of the defau;t to read.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:   Value f found, NULL otherwise.                                 *
*****************************************************************************/
static char *ReadOneXDefault(char *Entry)
{
    XrmString Type;
    XrmValue Result;
    char Line[IRIT_LINE_LEN_LONG];

    sprintf(Line, "%s.%s", RESOURCE_NAME, Entry);
    if (XrmGetResource(XrmGetDatabase(IGXDisplay), Line,
		       "Program.Name", &Type, &Result))
	return Result.addr;
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Reads Defaults from X data base.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReadXDefaults(void)
{
    int i;
    XColor Color;
    char *TransBackGroundColor = ReadOneXDefault("Trans.BackGround"),
         *TransBorderColor = ReadOneXDefault("Trans*BorderColor"),
         *TransTextColor = ReadOneXDefault("Trans.TextColor"),
         *TransSubWinBackColor = ReadOneXDefault("Trans.SubWin.BackGround"),
         *TransSubWinBorderColor = ReadOneXDefault("Trans.SubWin.BorderColor"),
         *TransGeometry = ReadOneXDefault("Trans.Geometry"),
         *TransCursorColorStr = ReadOneXDefault("Trans.CursorColor"),
         *ViewBackGroundColor = ReadOneXDefault("View.BackGround"),
         *ViewTextColor = ReadOneXDefault("View.TextColor"),
         *ViewBorderColor = ReadOneXDefault("View.BorderColor"),
         *ViewBorderWidthStr = ReadOneXDefault("View.BorderWidth"),
         *ViewGeometry = ReadOneXDefault("View.Geometry"),
         *ViewCursorColorStr = ReadOneXDefault("View.CursorColor"),
         *MaxColorsStr = ReadOneXDefault("MaxColors");

    if (XParseColor(IGXDisplay, IGXColorMap, "Black", &IGBlackColor))
	XAllocColor(IGXDisplay, IGXColorMap, &IGBlackColor);

    if (TransBackGroundColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransBackGroundColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	TransBackGroundPixel = Color.pixel;
    else
	TransBackGroundPixel = BlackPixel(IGXDisplay, IGXScreen);

    if (TransBorderColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransBorderColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	TransBorderPixel = Color.pixel;
    else
	TransBorderPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (TransTextColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransTextColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	TransTextPixel = Color.pixel;
    else
	TransTextPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (TransSubWinBackColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransSubWinBackColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	TransSubWinBackPixel = Color.pixel;
    else
	TransSubWinBackPixel = BlackPixel(IGXDisplay, IGXScreen);

    if (TransSubWinBorderColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransSubWinBorderColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	TransSubWinBorderPixel = Color.pixel;
    else
	TransSubWinBorderPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (IGGlblTransPrefPos &&
	sscanf(IGGlblTransPrefPos, "%d,%d,%d,%d",
	       &TransPosX, &TransWidth, &TransPosY, &TransHeight) == 4) {
	TransWidth -= TransPosX;
	TransHeight -= TransPosY;
	TransHasSize = TransHasPos = TRUE;
    }
    else if (IRT_STR_NULL_ZERO_LEN(IGGlblTransPrefPos) && TransGeometry) {
	i = XParseGeometry(TransGeometry, &TransPosX, &TransPosY,
		                          &TransWidth, &TransHeight);
	TransHasPos = i & XValue && i & YValue;
	TransHasSize =  i & WidthValue && i & HeightValue;
    }
    else
        TransHasSize = TransHasPos = FALSE;

    if (TransCursorColorStr != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, TransCursorColorStr, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color)) {
	TransCursorColor = (XColor *) IritMalloc(sizeof(XColor));
	*TransCursorColor = Color;
    }
    else
	TransCursorColor = NULL;

    if (ViewBackGroundColor &&
	XParseColor(IGXDisplay, IGXColorMap, ViewBackGroundColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	IGViewBackGroundPixel = Color.pixel;
    else
	IGViewBackGroundPixel = BlackPixel(IGXDisplay, IGXScreen);

    if (ViewBorderColor &&
	XParseColor(IGXDisplay, IGXColorMap, ViewBorderColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	IGViewBorderPixel = Color.pixel;
    else
	IGViewBorderPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (ViewTextColor != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, ViewTextColor, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color))
	IGViewTextPixel = Color.pixel;
    else
	IGViewTextPixel = WhitePixel(IGXDisplay, IGXScreen);

    if (ViewBorderWidthStr)
	IGViewBorderWidth = atoi(ViewBorderWidthStr);
    else
	IGViewBorderWidth = 1;

    if (IGGlblViewPrefPos &&
	sscanf(IGGlblViewPrefPos, "%d,%d,%d,%d",
	       &IGViewPosX, &IGViewWidth, &IGViewPosY, &IGViewHeight) == 4) {
	IGViewWidth -= IGViewPosX;
	IGViewHeight -= IGViewPosY;
	IGViewHasSize = IGViewHasPos = TRUE;
    }
    else if (IRT_STR_NULL_ZERO_LEN(IGGlblViewPrefPos) && ViewGeometry) {
	i = XParseGeometry(ViewGeometry, &IGViewPosX, &IGViewPosY,
		                         &IGViewWidth, &IGViewHeight);
	IGViewHasPos = i & XValue && i & YValue;
	IGViewHasSize = i & WidthValue && i & HeightValue;
    }
    else
	IGViewHasSize = IGViewHasPos = FALSE;

    if (ViewCursorColorStr != NULL &&
	XParseColor(IGXDisplay, IGXColorMap, ViewCursorColorStr, &Color) &&
	XAllocColor(IGXDisplay, IGXColorMap, &Color)) {
	IGViewCursorColor = (XColor *) IritMalloc(sizeof(XColor));
	*IGViewCursorColor = Color;
    }
    else
	IGViewCursorColor = NULL;

    if (MaxColorsStr)
	IGMaxColors = atoi(MaxColorsStr);
    else
	IGMaxColors = IG_MAX_COLOR;

}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets up and draw a transformation window.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   argc, argv:   Command line.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SetTransformWindow(int argc, char **argv)
{
    int SubTransPosX, SubTransPosY, SubTransWidth, SubTransHeight;
    long ValueMask;
    XSizeHints Hints;
    XSetWindowAttributes SetWinAttr;

    SetWinAttr.background_pixel = TransBackGroundPixel;
    SetWinAttr.border_pixel = TransBorderPixel;
    ValueMask = CWBackPixel | CWBorderPixel;

    Hints.flags = PMinSize | PMaxSize;
    Hints.x = Hints.y = 1;
    Hints.min_width = 100;
    Hints.max_width = 2000;
    Hints.min_height = 200;
    Hints.max_height = 2000;
    if (TransHasSize) {
	Hints.flags |= PSize;
	if (TransWidth < Hints.min_width)
	    TransWidth = Hints.min_width;
	if (TransWidth > Hints.max_width)
	    TransWidth = Hints.max_width;
	if (TransHeight < Hints.min_height)
	    TransHeight = Hints.min_height;
	if (TransHeight > Hints.max_height)
	    TransHeight = Hints.max_height;
	Hints.width = TransWidth;
	Hints.height = TransHeight;
    }
    else {
	Hints.flags |= PSize;
	Hints.width = TransWidth = DEFAULT_TRANS_WIDTH;
	Hints.height = TransHeight = DEFAULT_TRANS_HEIGHT;
    }
    if (TransHasPos) {
	Hints.flags |= USPosition;
	Hints.x = TransPosX;
	Hints.y = TransPosY;
    }

    TransformWndw = XCreateWindow(IGXDisplay, IGXRoot,
				  TransPosX, TransPosY,
				  TransWidth, TransHeight,
			          1, 0, CopyFromParent, CopyFromParent,
			          ValueMask, &SetWinAttr);

    XSetStandardProperties(IGXDisplay, TransformWndw,
			   RESOURCE_NAME, RESOURCE_NAME, None,
			   argv, argc,
			   &Hints);

    /* Now create the sub windows inside. Note we do not place them yet. */
    SubTransPosX = 0;
    SubTransPosY = TransHeight;
    SubTransWidth = TransWidth - SubTransPosX * 2;
    SubTransHeight = TransHeight / 25;

    /* OBJECT/SCREEN space toggle: */
    ObjScrTglWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					  SubTransWidth, SubTransHeight);

    /* PERSPECTIVE/ORTHOGRPHIC toggle: */
    PersOrthoTglWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					     SubTransWidth, SubTransHeight);
    PersOrthoZWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					     SubTransWidth, SubTransHeight);

    /* ROTATE: */
    RotateXWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					SubTransWidth, SubTransHeight);
    RotateYWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					SubTransWidth, SubTransHeight);
    RotateZWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					SubTransWidth, SubTransHeight);
    /* TRANSLATE: */
    TranslateXWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					   SubTransWidth, SubTransHeight);
    TranslateYWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					   SubTransWidth, SubTransHeight);
    TranslateZWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					   SubTransWidth, SubTransHeight);
    /* SCALE: */
    ScaleWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
				      SubTransWidth, SubTransHeight);

    /* DEPTH CUE: */
    DepthCueWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					 SubTransWidth, SubTransHeight);

    /* ANIMATION: */
    AnimationWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					  SubTransWidth, SubTransHeight);

    /* SAVE MATRIX: */
    SaveMatrixWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					   SubTransWidth, SubTransHeight);

    /* SUBMIT MATRIX: */
    SubmitMatrixWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					     SubTransWidth, SubTransHeight);

    /* PUSH MATRIX: */
    PushMatrixWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					   SubTransWidth, SubTransHeight);

    /* POP MATRIX: */
    PopMatrixWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
					  SubTransWidth, SubTransHeight);

    /* QUIT: */
    QuitWndw = SetTransformSubWindow(SubTransPosX, SubTransPosY,
				     SubTransWidth, SubTransHeight);

    XSelectInput(IGXDisplay, TransformWndw, ExposureMask);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraws the transformation window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawTransformWindow(void)
{
    int SubTransPosX, SubTransPosY, SubTransWidth, SubTransHeight;
    XWindowAttributes TransWindowAttr;

    XClearWindow(IGXDisplay, TransformWndw);

    /* Get the window attributes, and see if it is the same size or not. */
    XGetWindowAttributes(IGXDisplay, TransformWndw, &TransWindowAttr);
    if (TransWindowAttr.width != TransWidth ||
	TransWindowAttr.height != TransHeight) {
	TransWidth = TransWindowAttr.width;
	TransHeight = TransWindowAttr.height;
    }

    /* Now lets update the sub windows inside: */
    SubTransPosX = IRIT_MIN(TransWidth / 10, 20);
    SubTransPosY =  TransHeight / 30;
    SubTransWidth = TransWidth - SubTransPosX * 2;
    SubTransHeight = TransHeight / 30;

    /* OBJECT/SCREEN space toggle: */
    RedrawTransformSubWindow(ObjScrTglWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     IGGlblTransformMode == IG_TRANS_OBJECT ?
			     "Object" : "Screen");
    SubTransPosY += SubTransHeight * 2;

    /* PERSPECTIVE/ORTHOGRAPHIC toggle: */
    RedrawTransformSubWindow(PersOrthoTglWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     IGGlblViewMode == IG_VIEW_ORTHOGRAPHIC ?
			     "Orthographic" : "Perspective");
    SubTransPosY += SubTransHeight;
    RedrawTransformSubWindow(PersOrthoZWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Z", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);
    SubTransPosY += SubTransHeight;

    /* ROTATE: */
    DrawText(TransformWndw, "Rotate", TransWidth / 2, SubTransPosY,
	     TransTextPixel);
    SubTransPosY += SubTransHeight / 2;
    RedrawTransformSubWindow(RotateXWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "X", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);
    RedrawTransformSubWindow(RotateYWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Y", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);
    RedrawTransformSubWindow(RotateZWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Z", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);

    /* TRANSLATE: */
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Translate", TransWidth / 2, SubTransPosY,
	     TransTextPixel);
    SubTransPosY += SubTransHeight / 2;
    RedrawTransformSubWindow(TranslateXWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "X", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);
    RedrawTransformSubWindow(TranslateYWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Y", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);
    RedrawTransformSubWindow(TranslateZWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Z", SubTransPosX / 2,
	     SubTransPosY - SubTransHeight / 4, TransTextPixel);

    /* SCALE: */
    SubTransPosY += SubTransHeight;
    DrawText(TransformWndw, "Scale", TransWidth / 2, SubTransPosY,
	     TransTextPixel);
    SubTransPosY += SubTransHeight / 2;
    RedrawTransformSubWindow(ScaleWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, TRUE, NULL);

    /* DEPTH CUE: */
    SubTransPosY += SubTransHeight * 2;
    RedrawTransformSubWindow(DepthCueWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     IGGlblDepthCue ? "Depth Cue" : "No Depth Cue");

    /* ANIMATION: */
    SubTransPosY += SubTransHeight * 2;
    RedrawTransformSubWindow(AnimationWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     "Animation");

    /* SAVE MATRIX: */
    SubTransPosY += SubTransHeight * 2;
    RedrawTransformSubWindow(SaveMatrixWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     "Save Matrix");

    /* SUBMIT MATRIX: */
    SubTransPosY += SubTransHeight + SubTransHeight / 2;
    RedrawTransformSubWindow(SubmitMatrixWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     "Submit Matrix");

    /* PUSH MATRIX: */
    SubTransPosY += SubTransHeight + SubTransHeight / 2;
    RedrawTransformSubWindow(PushMatrixWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     "Push Matrix");

    /* POP MATRIX: */
    SubTransPosY += SubTransHeight + SubTransHeight / 2;
    RedrawTransformSubWindow(PopMatrixWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE,
			     "Pop Matrix");

    /* QUIT: */
    SubTransPosY += SubTransHeight * 3;
    RedrawTransformSubWindow(QuitWndw, SubTransPosX, SubTransPosY,
			     SubTransWidth, SubTransHeight, FALSE, "Quit" );

    /* Save half of the window width so we can refer to the zero point on X */
    /* axes, which is the vertical line in the middle of the window:	    */
    SubWindowWidthState2 = SubTransWidth / 2;
    SubWindowHeightState2 = SubTransHeight / 2;

    XFlush(IGXDisplay);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Sets up a transformation sub window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   SubTransPosX, SubTransPosY:      Location of the subwindows.	     *
*   SubTransWidth, SubTransHeight:   Dimensions of the subwindow.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   Window:      Handle on constructed window.                               *
*****************************************************************************/
static Window SetTransformSubWindow(int SubTransPosX,
				    int SubTransPosY,
				    unsigned int SubTransWidth,
				    unsigned int SubTransHeight)
{
    long ValueMask;
    XSetWindowAttributes SetWinAttr;
    Window Win;

    SetWinAttr.background_pixel = TransSubWinBackPixel;
    SetWinAttr.border_pixel = TransSubWinBorderPixel;
    SetWinAttr.bit_gravity = SetWinAttr.win_gravity = CenterGravity;
    ValueMask = CWBackPixel | CWBorderPixel | CWBitGravity | CWWinGravity;

    Win = XCreateWindow(IGXDisplay, TransformWndw,
			SubTransPosX, SubTransPosY,
			SubTransWidth, SubTransHeight,
			1, 0, CopyFromParent, CopyFromParent,
			ValueMask, &SetWinAttr);

    XSelectInput(IGXDisplay, Win,
		 ButtonPressMask | Button1MotionMask);

    XMapWindow(IGXDisplay, Win);

    return Win;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Redraws a transformation sub window.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Win:			     A handle on transformation window.      *
*   SubTransPosX, SubTransPosY:      Location of the subwindows.	     *
*   SubTransWidth, SubTransHeight:   Dimensions of the subwindow.	     *
*   DrawMiddleVertLine:              Do we need a vertical middle line?      *
*   DrawString:                      Any string to draw inside?              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RedrawTransformSubWindow(Window Win,
				     int SubTransPosX,
				     int SubTransPosY,
				     unsigned int SubTransWidth,
				     unsigned int SubTransHeight,
				     int DrawMiddleVertLine,
				     char *DrawString)
{
    XGCValues values;

    XMoveResizeWindow(IGXDisplay, Win, SubTransPosX, SubTransPosY,
		                               SubTransWidth, SubTransHeight);
    if (DrawMiddleVertLine) {
	values.foreground = TransSubWinBorderPixel;
	XChangeGC(IGXDisplay, XTransGraphContext, GCForeground, &values);

	XDrawLine(IGXDisplay, Win, XTransGraphContext,
		  SubTransWidth / 2, 0, SubTransWidth / 2, SubTransHeight);
    }
    if (DrawString != NULL) {
	DrawText(Win, DrawString, SubTransWidth / 2, SubTransHeight / 2,
		 TransTextPixel);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Handles input events                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   ChangeFactor:        A continuous numeric value between -1 and 1. This   *
*			 value will be used to set amount of event such as   *
*			 rotation or translation. In some events it can      *
*			 contain a vector value.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IGGraphicEventType:  Type of new event.                                  *
*****************************************************************************/
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor)
{
    IRIT_STATIC_DATA int LastX, LastY;
    IRIT_STATIC_DATA IGGraphicEventType LastEvent;
    XEvent Event, NextEvent;
    XWindowAttributes WinAttr;

    ChangeFactor[0] = ChangeFactor[1] = 0.0;

    XMapWindow(IGXDisplay, TransformWndw);

    while (TRUE) {
	/* Maybe we have something in communication socket. */
	if (!IGGlblStandAlone &&
	    IGReadObjectsFromSocket(IGGlblViewMode, &IGGlblDisplayList))
	    IGRedrawViewWindow();

	if (XPending(IGXDisplay)) {
	    XNextEvent(IGXDisplay, &Event);

	    switch (Event.type) {
		case Expose:
	            /* Get rid of all Expose events in the queue. */
	            while (XCheckWindowEvent(IGXDisplay, Event.xbutton.window,
					     ExposureMask, &Event));
		    if (Event.xbutton.window == TransformWndw)
			RedrawTransformWindow();
		    else if (Event.xbutton.window == IGViewWndw) {
			XGetWindowAttributes(IGXDisplay, IGViewWndw, &WinAttr);
			IGViewWidth = WinAttr.width;
			IGViewHeight = WinAttr.height;
			IGRedrawViewWindow();
		    }
		    break;
		case ButtonRelease:
		    if (Event.xbutton.window == IGViewWndw) {
			if (Event.xbutton.button == 1 &&
			    IGCrvEditGrabMouse) {
			    CEditHandleMouse(Event.xbutton.x,
					     Event.xbutton.y,
					     IG_CRV_EDIT_BUTTONUP);
			    return IG_EVENT_ZERO;
			}
			if (PickCrsrGrabMouse) {
			    IGHandleCursorEvent(Event.xbutton.x,
						Event.xbutton.y,
						IG_PICK_REP_BTN_UP);
			    break;
			}

			IGGlblManipulationActive = FALSE;
			IGRedrawViewWindow();
		    }
		    break;
		case ButtonPress:
		    LastX = Event.xbutton.x;
		    *ChangeFactor =
			((IrtRType) (LastX - SubWindowWidthState2)) /
			             SubWindowWidthState2;

		    if (Event.xbutton.window == IGViewWndw) {
			IGGlblManipulationActive = TRUE;
			LastY = Event.xbutton.y;
			ChangeFactor[0] = ChangeFactor[1] = 0.0;
			switch (Event.xbutton.button) {
			    case 1:
			        if (IGCrvEditGrabMouse) {
				    CEditHandleMouse(Event.xbutton.x,
						     Event.xbutton.y,
						     IG_CRV_EDIT_BUTTONDOWN);
				    LastX = Event.xbutton.x;
				    LastY = Event.xbutton.y;
				    break;
				}	
				if (PickCrsrGrabMouse) {
				    IGHandleCursorEvent(Event.xbutton.x,
							Event.xbutton.y,
							IG_PICK_REP_BTN1DOWN);
				    break;
				}

			        if (Event.xbutton.state & ShiftMask) {
				    IPObjectStruct
				        *PObj = IGHandlePickEvent(
							Event.xbutton.x,
						        Event.xbutton.y,
							IG_PICK_ANY);

				    if (ReturnPickedObject) {
					IPObjectStruct *PObjDump;

					if (PObj != NULL) {
					    if (ReturnPickedObjectName)
					        PObjDump =
						    IPGenStrObject("_PickName_",
							   IP_GET_OBJ_NAME(PObj),
							   NULL);
					    else
					        PObjDump = PObj;
					}
					else
					    PObjDump =
					        IPGenStrObject("_PickFail_",
							   "*** no object ***",
							   NULL);

					IPSocWriteOneObject(IGGlblIOHandle,
							    PObjDump);
					if (PObj != PObjDump)
					    IPFreeObject(PObjDump);
				    }
				    else {
					char NewTitle[IRIT_LINE_LEN_LONG];

					sprintf(NewTitle,
					      "%s :: Picked \"%s\"",
					      RESOURCE_NAME,
					      PObj == NULL ? "nothing"
							   : IP_GET_OBJ_NAME(PObj));
					XStoreName(IGXDisplay, IGViewWndw,
						   NewTitle);
		
				        printf("Pick event found \"%s\"     \r",
					       PObj == NULL ? "nothing"
							    : IP_GET_OBJ_NAME(PObj));
				    }
				    break;
				}
				else
				    return (LastEvent = IG_EVENT_ROTATE);
			    case 2:
				if (PickCrsrGrabMouse) {
				    IGHandleCursorEvent(Event.xbutton.x,
							Event.xbutton.y,
							IG_PICK_REP_BTN2DOWN);
				    break;
				}
				break;
			    case 3:
				if (PickCrsrGrabMouse) {
				    IGHandleCursorEvent(Event.xbutton.x,
							Event.xbutton.y,
							IG_PICK_REP_BTN3DOWN);
				    break;
				}
			        return (LastEvent = IG_EVENT_TRANSLATE);
			}
		    }
		    else if (Event.xbutton.window == ObjScrTglWndw) {
			XClearWindow(IGXDisplay, ObjScrTglWndw);
			IGGlblTransformMode =
			    IGGlblTransformMode == IG_TRANS_OBJECT ?
						   IG_TRANS_SCREEN :
						   IG_TRANS_OBJECT;
			DrawText(ObjScrTglWndw,
			    IGGlblTransformMode == IG_TRANS_OBJECT ? "Object" :
								     "Screen",
			    SubWindowWidthState2, SubWindowHeightState2,
			    TransTextPixel);
			return IG_EVENT_SCR_OBJ_TGL;
		    }
		    else if (Event.xbutton.window == PersOrthoTglWndw) {
			XClearWindow(IGXDisplay, PersOrthoTglWndw);
			IGGlblViewMode =
			    IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
					      IG_VIEW_ORTHOGRAPHIC :
					      IG_VIEW_PERSPECTIVE;
			DrawText(PersOrthoTglWndw,
				 IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
				     "Perspective" : "Orthographic",
				 SubWindowWidthState2, SubWindowHeightState2,
				 TransTextPixel);
			return IG_EVENT_PERS_ORTHO_TGL;
		    }
		    else if (Event.xbutton.window == PersOrthoZWndw) {
			return IG_EVENT_PERS_ORTHO_Z;
		    }
		    else if (Event.xbutton.window == RotateXWndw) {
			return IG_EVENT_ROTATE_X;
		    }
		    else if (Event.xbutton.window == RotateYWndw) {
			return IG_EVENT_ROTATE_Y;
		    }
		    else if (Event.xbutton.window == RotateZWndw) {
			return IG_EVENT_ROTATE_Z;
		    }
		    else if (Event.xbutton.window == TranslateXWndw) {
			return IG_EVENT_TRANSLATE_X;
		    }
		    else if (Event.xbutton.window == TranslateYWndw) {
			return IG_EVENT_TRANSLATE_Y;
		    }
		    else if (Event.xbutton.window == TranslateZWndw) {
			return IG_EVENT_TRANSLATE_Z;
		    }
		    else if (Event.xbutton.window == ScaleWndw) {
			return IG_EVENT_SCALE;
		    }
		    else if (Event.xbutton.window == DepthCueWndw) {
			XClearWindow(IGXDisplay, DepthCueWndw);
			IGGlblDepthCue = !IGGlblDepthCue;
			DrawText(DepthCueWndw,
				 IGGlblDepthCue ? "Depth Cue" : "No Depth Cue",
				 SubWindowWidthState2, SubWindowHeightState2,
				 TransTextPixel);
			return IG_EVENT_DEPTH_CUE;
		    }
		    else if (Event.xbutton.window == AnimationWndw) { 
			XClearWindow(IGXDisplay, AnimationWndw);
			DrawText(AnimationWndw, "Animation",
				 SubWindowWidthState2, SubWindowHeightState2,
				 TransTextPixel);
			return IG_EVENT_ANIMATION;
                    }
		    else if (Event.xbutton.window == SaveMatrixWndw) {
			return IG_EVENT_SAVE_MATRIX;
		    }
		    else if (Event.xbutton.window == SubmitMatrixWndw) {
			return IG_EVENT_SUBMIT_MATRIX;
		    }
		    else if (Event.xbutton.window == PushMatrixWndw) {
			return IG_EVENT_PUSH_MATRIX;
		    }
		    else if (Event.xbutton.window == PopMatrixWndw) {
			return IG_EVENT_POP_MATRIX;
		    }
		    else if (Event.xbutton.window == QuitWndw) {
			XFlush(IGXDisplay);
			return IG_EVENT_QUIT;
		    }
		    break;
		case MotionNotify:
		    /* Flushes all motion events in the queue. */
		    while (XPending(IGXDisplay)) {
			XPeekEvent(IGXDisplay, &NextEvent);
			if (NextEvent.type == MotionNotify)
			    XNextEvent(IGXDisplay, &Event);
			else
			    break;
		    }

		    if (Event.xbutton.window == IGViewWndw) {
			if (IGCrvEditGrabMouse &&
			    (Event.xbutton.x != LastX ||
			     Event.xbutton.y != LastY)) {
			    CEditHandleMouse(Event.xbutton.x,
					     Event.xbutton.y,
					     IG_CRV_EDIT_MOTION);
			    LastX = Event.xbutton.x;
			    LastY = Event.xbutton.y;
			    break;
			}
			if (PickCrsrGrabMouse) {
			    IGHandleCursorEvent(Event.xbutton.x,
						Event.xbutton.y,
						IG_PICK_REP_MOTION);
			    break;
			}

			if (Event.xbutton.x - LastX != 0 ||
			    Event.xbutton.y - LastY != 0) {
			    ChangeFactor[0] = (Event.xbutton.x - LastX);
			    ChangeFactor[1] = (LastY - Event.xbutton.y);
			    LastX = Event.xbutton.x;
			    LastY = Event.xbutton.y;
			    return LastEvent;
			}
			break;
		    }

		    /* We may get events of movement in Y which are ignored. */
		    if (Event.xbutton.x - LastX == 0)
			break;

		    *ChangeFactor = ((IrtRType) (Event.xbutton.x - LastX)) /
							SubWindowWidthState2;
		    LastX = Event.xbutton.x;

		    if (Event.xbutton.window == PersOrthoZWndw) {
			return IG_EVENT_PERS_ORTHO_Z;
		    }
		    else if (Event.xbutton.window == RotateXWndw) {
			return IG_EVENT_ROTATE_X;
		    }
		    else if (Event.xbutton.window == RotateYWndw) {
			return IG_EVENT_ROTATE_Y;
		    }
		    else if (Event.xbutton.window == RotateZWndw) {
			return IG_EVENT_ROTATE_Z;
		    }
		    else if (Event.xbutton.window == TranslateXWndw) {
			return IG_EVENT_TRANSLATE_X;
		    }
		    else if (Event.xbutton.window == TranslateYWndw) {
			return IG_EVENT_TRANSLATE_Y;
		    }
		    else if (Event.xbutton.window == TranslateZWndw) {
			return IG_EVENT_TRANSLATE_Z;
		    }
		    else if (Event.xbutton.window == ScaleWndw) {
			return IG_EVENT_SCALE;
		    }
		    break;
		default:
		    fprintf(stderr,
			    "x11drvs: undefined event type %d.\n", Event.type);
	    }
	}
	IritSleep(10);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Pick an object from the display.                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   PickEntity:  Type of entity to pick (object, cursor etc.).               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandlePickObject                                                       M
*****************************************************************************/
void IGHandlePickObject(IGPickEntityType PickEntity)
{
    Cursor XCursor;

    switch (PickEntity) {
	case IG_PICK_ENTITY_DONE:
	    /* Restore the cursor. */
	    XCursor = XCreateFontCursor(IGXDisplay, XC_top_left_arrow);
	    XDefineCursor(IGXDisplay, IGViewWndw, XCursor);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = FALSE;
	    break;
	case IG_PICK_ENTITY_OBJECT:
	case IG_PICK_ENTITY_OBJ_NAME:
	    /* Set our own object picking cursor: */
	    XCursor = XCreateFontCursor(IGXDisplay, XC_cross);
	    XDefineCursor(IGXDisplay, IGViewWndw, XCursor);

	    PickCrsrGrabMouse = FALSE;
	    ReturnPickedObject = TRUE;
	    ReturnPickedObjectName = PickEntity == IG_PICK_ENTITY_OBJ_NAME;
	    break;
	case IG_PICK_ENTITY_CURSOR:
	    /* Set our own cursor picking cursor: */
	    XCursor = XCreateFontCursor(IGXDisplay, XC_pencil);
	    XDefineCursor(IGXDisplay, IGViewWndw, XCursor);

	    ReturnPickedObject = FALSE;
	    PickCrsrGrabMouse = TRUE;
	    break;
        default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Handles the events of the pop up window.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   State:       Event to handle.                                            M
*   StateStatus: IG_STATE_OFF, IG_STATE_ON, IG_STATE_TGL for turning off,    M
*		 on or toggling current value. 				     M
*		 IG_STATE_DEC and IG_STATE_INC serves as dec./inc. factors.  M
*   Refresh:     Do we need to refresh the screen according to what we know  M
*		 on entry.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE, if we need to refresh the screen.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGHandleState                                                            M
*****************************************************************************/
int IGHandleState(int State, int StateStatus, int Refresh)
{
    int UpdateView = TRUE;

    switch (State) {
	case IG_STATE_DEPTH_CUE:
	    XClearWindow(IGXDisplay, DepthCueWndw);
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    DrawText(DepthCueWndw,
		     IGGlblDepthCue ? "Depth Cue" : "No Depth Cue",
		     SubWindowWidthState2, SubWindowHeightState2,
		     TransTextPixel);

	    break;
	default:
	    UpdateView = IGStateHandler(State, StateStatus, Refresh);
	    break;
    }

    IGCreateStateMenu();

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draws text centered at the given position.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Win:        A handle on window to draw on.                               *
*   Str:        String to draw.                                              *
*   PosX, PosY: Location to draw at.                                         *
*   Color:      Of drawn text.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawText(Window Win,
		     char *Str,
		     int PosX,
		     int PosY,
		     unsigned long Color)
{
    int Len = strlen(Str),
        Width = XTextWidth(XLoadedFont, Str, Len);
    XGCValues values;

    values.foreground = Color;
    XChangeGC(IGXDisplay, XTransGraphContext, GCForeground, &values);

    XDrawString(IGXDisplay, Win, XTransGraphContext, PosX - Width / 2,
		PosY + XFontYOffsetToCenter, Str, Len);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make some sound.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritBeep                                                               M
*****************************************************************************/
void IGIritBeep(void)
{
    XBell(IGXDisplay, 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation. Senses the event queue of X11.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we need to abort, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimCheckInterrupt                                                     M
*****************************************************************************/
int GMAnimCheckInterrupt(GMAnimationStruct *Anim)
{
    if (XPending(IGXDisplay)) {
	Anim -> StopAnim = TRUE;
	fprintf(stderr, "\nAnimation was interrupted by the user.\n");
	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve parameters.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvEditParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvEditPlaceMessage                                                    M
*****************************************************************************/
void IGCrvEditPlaceMessage(char *Msg)
{
    fprintf(stderr, "%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve parameters.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCrvCnstParamUpdateWidget                                               M
*****************************************************************************/
void IGCrvCnstParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current surface parameters.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSrfEditParamUpdateWidget                                               M
*****************************************************************************/
void IGSrfEditParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSrfEditPlaceMessage                                                    M
*****************************************************************************/
void IGSrfEditPlaceMessage(char *Msg)
{
    fprintf(stderr, "%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current object transformation state.         M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipParamUpdateWidget                                              M
*****************************************************************************/
void IGObjManipParamUpdateWidget(void) 
{
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Place a message for the user to guide him/her.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   The message to place                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipPlaceMessage                                                   M
*****************************************************************************/
void IGObjManipPlaceMessage(char *Msg)
{
    fprintf(stderr, "%s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Saves the current matrix in a selected file name.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   ViewMode:   Perspective or orthographics current view mode.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSaveCurrentMatInFile                                                   M
*****************************************************************************/
void IGSaveCurrentMatInFile(int ViewMode)
{
    IGSaveCurrentMat(ViewMode, IG_DEFAULT_IRIT_MAT);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make error message box in printf style.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritError                                                              M
*****************************************************************************/
void IGIritError(char *Msg)
{
    fprintf(stderr, "Error: %s\n", Msg);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make yes/no message box.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Title message.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if yes was selected, FALSE otherwise.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritYesNoQuestion                                                      M
*****************************************************************************/
int IGIritYesNoQuestion(char *Msg)
{
    char Line[IRIT_LINE_LEN];

    do {
        fprintf(stderr, "%s <Y/N>:", Msg);
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
    }
    while (Line[0] != 'y' && Line[0] != 'Y' &&
	   Line[0] != 'n' && Line[0] != 'N');
    
    return Line[0] == 'y' || Line[0] != 'Y';
}
