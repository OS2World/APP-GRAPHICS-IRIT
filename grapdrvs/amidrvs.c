/*****************************************************************************
*   An Amiga driver.                                                         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Kriton Kyrimis                                                *
*        and   Gershon Elber                        Ver 0.2, February 1995.  *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <cybergraphics/cybergraphics.h>

#undef SIGN

#include "irit_sm.h"
#include "misc_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"

#ifdef __SASC
#include <dos/dos.h>
#endif

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/cybergraphics.h>

/* Window coordinates for transformation window gadgetry */
#define L 24
#define R 162
#define S_O_TOP 11
#define S_O_BOT 25
#define O_P_TOP 32
#define O_P_BOT 46
#define O_PZ_TOP O_P_BOT
#define O_PZ_BOT 60
#define RX_TOP 77
#define RX_BOT 91
#define RY_TOP RX_BOT
#define RY_BOT 104
#define RZ_TOP RY_BOT
#define RZ_BOT 118
#define TX_TOP 136
#define TX_BOT 149
#define TY_TOP TX_BOT
#define TY_BOT 163
#define TZ_TOP TY_BOT
#define TZ_BOT 177
#define SCALE_TOP 194
#define SCALE_BOT 208
#define DQ_TOP 219
#define DQ_BOT 233
#define ANIM_TOP 243
#define ANIM_BOT 257
#define SAVE_TOP 267
#define SAVE_BOT 281
#define PUSH_TOP 291
#define PUSH_BOT 305
#define POP_TOP 315
#define POP_BOT 329
#define QUIT_TOP 341
#define QUIT_BOT 355

#define TWIDTH 188
#define THEIGHT 365

#define MAP_X_COORD(x) (int)(((IrtRType)(ViewWidth - 1) * (x + 1.0)) / 2.0)
#define MAP_Y_COORD(y) (int)(((IrtRType)(ViewHeight - 1) * (1.0 - y)) / 2.0)

IRIT_GLOBAL_DATA struct GfxBase *GfxBase = NULL;
IRIT_GLOBAL_DATA struct IntuitionBase *IntuitionBase = NULL;
IRIT_GLOBAL_DATA struct Library *LayersBase = NULL;
IRIT_GLOBAL_DATA struct Library *CyberGfxBase = NULL;

IRIT_STATIC_DATA struct Screen *screen = NULL;
IRIT_STATIC_DATA struct Window *TransformWindow = NULL;
IRIT_STATIC_DATA struct RastPort *trp;
IRIT_STATIC_DATA struct Window *ViewWindow[2] = {NULL, NULL};
IRIT_STATIC_DATA struct RastPort *vrp;

IRIT_STATIC_DATA int CurrentWindow = 0;
IRIT_STATIC_DATA int Animating = FALSE;

IRIT_STATIC_DATA int ViewWidth, ViewHeight,
    CurrentXPosition = 0,
    CurrentYPosition = 0,
    CurrentColor = IG_IRIT_WHITE;

IRIT_STATIC_DATA struct ColorSpec colors[] =
{
    { 0,  0,  0,  0},	/* 0. IG_IRIT_BLACK */
    { 1,  0,  0, 10},	/* 1. IG_IRIT_BLUE */
    { 2,  0, 10,  0},	/* 2. IG_IRIT_GREEN */
    { 3,  0, 10, 10},	/* 3. IG_IRIT_CYAN */
    { 4, 10,  0,  0},	/* 4. IG_IRIT_RED */
    { 5, 10,  0, 10},	/* 5. IG_IRIT_MAGENTA */
    { 6, 10, 10,  0},	/* 6. IG_IRIT_BROWN */
    { 7, 10, 10, 10},	/* 7. IG_IRIT_LIGHTGREY */
    { 8,  5,  5,  5},	/* 8. IG_IRIT_DARKGRAY */
    { 9,  5,  5, 15},	/* 9. IG_IRIT_LIGHTBLUE */
    {10,  5, 15,  5},	/* 10. IG_IRIT_LIGHTGREEN */
    {11,  5, 15, 15},	/* 11. IG_IRIT_LIGHTCYAN */
    {12, 15,  5,  5},	/* 12. IG_IRIT_LIGHTRED */
    {13, 15,  5, 15},	/* 13. IG_IRIT_LIGHTMAGENTA */
    {14, 15, 15,  5},	/* 14. IG_IRIT_YELLOW */
    {15, 15, 15, 15},	/* 15. IG_IRIT_WHITE */
    {-1,  0,  0,  0}	/* end */
};

IRIT_STATIC_DATA struct TextAttr font = { "topaz.font", 8, 0, 0 };

IRIT_STATIC_DATA UWORD pens[] =
{
    IG_IRIT_BLACK,		/* DETAILPEN */
    IG_IRIT_DARKGRAY,		/* BLOCKPEN */
    IG_IRIT_LIGHTGREY,		/* TEXTPEN */
    IG_IRIT_LIGHTGREY,		/* SHINEPEN */
    IG_IRIT_DARKGRAY,		/* SHADOWPEN */
    IG_IRIT_LIGHTGREY,		/* FILLPEN */
    IG_IRIT_LIGHTMAGENTA,	/* FILLTEXTPEN */
    IG_IRIT_BLACK,		/* BACKGROUNDPEN */
    IG_IRIT_WHITE,		/* HIGHLIGHTTEXTPEN */
    (UWORD) ~0			/* done */
};
IRIT_STATIC_DATA int SocInputSocketChannel;

static void Cleanup(void);
static void DrawTransformWindow(void);
static void Box(int, int, int, int);
static void PotBox(int, int, int, int);
static void CenterText(char *, int);
static void SideText(char *, int);
static int GetColorRGB(int *);
static int Hit(int, int, int, int);
static IGGraphicEventType GetGraphicEvent(IrtRType *);
static void singlebuffer(void);
static void doublebuffer(void);
static void swapbuffers(void);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of amidrvs - Amiga graphics driver of IRIT.             	     M
*                                                                            *
* PARAMETERS:                                                                M
*   argc, argv:  Command line.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:         Exit code.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
int main(int argc, char *argv[])
{
    IrtRType ChangeFactor[2];
    IGGraphicEventType Event;
    ULONG ModeID;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    atexit(Cleanup);   /* Absolutely, positively *must* clean up before exit */

    IGConfigureGlobals("amidrvs", argc, argv);

    if ((IntuitionBase = (struct IntuitionBase *)
	 OpenLibrary ("intuition.library", 37L)) == NULL){
	fprintf(stderr, "This program requires AmigaDOS 2.04 or higher\n");
	exit(RETURN_FAIL);
    }
    if ((GfxBase =
	 (struct GfxBase *) OpenLibrary ("graphics.library", 0L)) == NULL){
	fprintf(stderr, "Can't open graphics.library\n");
	exit(RETURN_FAIL);
    }
    if ((LayersBase = OpenLibrary ("layers.library", 0L)) == NULL){
	fprintf(stderr, "Can't open layers.library\n");
	exit(RETURN_FAIL);
    }
    CyberGfxBase = OpenLibrary ("cybergraphics.library", 0L);

    if (CyberGfxBase != NULL) {
      screen = LockPubScreen(NULL);
      if (screen) {
#if 0
        ModeID = BestCModeIDTags(CYBRBIDTG_NominalWidth, screen->Width,
				 CYBRBIDTG_NominalHeight, screen->Height,
				 TAG_END);
#else
	UWORD ModesAllowed[] = {PIXFMT_LUT8, 0xffff};
	ModeID = CModeRequestTags(
	  NULL,
	  CYBRMREQ_WinTitle, (STRPTR)"amidrvs: select screen mode",
	  CYBRMREQ_MinWidth, 640,
	  CYBRMREQ_MinHeight, 480,
	  CYBRMREQ_MinDepth, 4,
	  CYBRMREQ_CModelArray, ModesAllowed,
	  TAG_DONE);
#endif
	UnlockPubScreen(NULL, screen);
	if (ModeID == 0 || ModeID == 0xffff) {
	  CloseLibrary(CyberGfxBase);
	  CyberGfxBase = NULL;
	  ModeID = HIRESLACE_KEY;
	}
      }
    }else{
      ModeID = HIRESLACE_KEY;
    }

    screen = OpenScreenTags(NULL,
			    SA_Depth,	  4,
			    SA_DisplayID, ModeID,
			    SA_Quiet,	  TRUE,
			    SA_Title,	  (ULONG)"Irit",
			    SA_Font,	  (ULONG)&font,
			    SA_Pens,	  (ULONG)&pens,
			    SA_Colors,	  (ULONG)&colors,
			    TAG_END);
    if (!screen) {
	fprintf(stderr, "Can't open screen\n");
	exit(RETURN_FAIL);
    }

    TransformWindow = OpenWindowTags(NULL,
  				WA_CustomScreen,(ULONG)screen,
  				WA_Left,	screen->Width - TWIDTH,
  				WA_Top,		(screen->Height - THEIGHT) / 2,
				WA_Width,	TWIDTH,
				WA_Height,	THEIGHT,
				WA_IDCMP,	IDCMP_MOUSEBUTTONS,
				WA_RMBTrap,	TRUE,
  				TAG_END);
    if (!TransformWindow) {
	fprintf(stderr, "Can't open transform window\n");
	exit(RETURN_FAIL);
    }
    trp = TransformWindow -> RPort;

    DrawTransformWindow();

    ViewWidth = screen -> Width - TWIDTH;
    ViewHeight = screen -> Height;

    ViewWindow[0] = OpenWindowTags(NULL,
				   WA_CustomScreen, (ULONG)screen,
				   WA_Borderless,   TRUE,
				   WA_Width,	    ViewWidth,
				   WA_Height,	    ViewHeight,
				   WA_IDCMP,	    0,
				   WA_RMBTrap,	    TRUE,
				   TAG_END);

    if (!ViewWindow[0]) {
	fprintf(stderr, "Can't open view window\n");
	exit(RETURN_FAIL);
    }
    if (IGGlblDoDoubleBuffer) {
        doublebuffer();
    }else{
        singlebuffer();
    }
    IGRedrawViewWindow();

    while ((Event = GetGraphicEvent(ChangeFactor)) != IG_EVENT_QUIT) {
        ChangeFactor[0] *= IGGlblChangeFactor;
	ChangeFactor[1] *= IGGlblChangeFactor;

	if (IGProcessEvent(Event, ChangeFactor)) {
	    IGRedrawViewWindow();
	}
    }

    if (IGGlblIOHandle >= 0)
        IPCloseStream(IGGlblIOHandle, TRUE);

    exit(RETURN_OK);

    return 0;
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
    DisplayBeep(screen);
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
    CurrentXPosition = MAP_X_COORD(X);
    CurrentYPosition = MAP_Y_COORD(Y);
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
    int NewX, NewY, i;

    NewX = MAP_X_COORD(X);
    NewY = MAP_Y_COORD(Y);
    Move(vrp, CurrentXPosition, CurrentYPosition);
    Draw(vrp, NewX, NewY);
    if (IGGlblLineWidth > 1) {
	for (i=1; i<IGGlblLineWidth; i++) {
	    Move(vrp, CurrentXPosition + i, CurrentYPosition);
	    Draw(vrp, NewX + i, NewY);
	    Move(vrp, CurrentXPosition, CurrentYPosition - i);
	    Draw(vrp, NewX, NewY - i);
	}
	Move(vrp, NewX, NewY);
	
	RectFill(vrp, NewX, NewY - IGGlblLineWidth + 1,
		      NewX + IGGlblLineWidth - 1, NewY);
    }
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
  if (High) {
      CurrentColor |= 8;
  } else {
      CurrentColor &= ~8;
  }
  SetAPen(vrp, CurrentColor);
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
        CurrentColor = GetColorRGB(Color);
    }
    else if ((c = AttrGetObjectColor(PObj)) != IP_ATTR_NO_COLOR) {
        CurrentColor = c;
    }
    else {
        /* Use white as default color: */
        CurrentColor = IG_IRIT_WHITE;
    }
    SetAPen(vrp, c | 8);
    IGGlblIntensityHighState = TRUE;
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
* Finds the closest color to a given RGB combination. 		             *
*                                                                            *
* PARAMETERS:                                                                *
*   Color:    An RGB array of three integer values between 0 and 255.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      Index to best color find.                                      *
*****************************************************************************/
static int GetColorRGB(int *Color)
{
    int r, g, b;
    int dist, mindist = MAXINT;
    int bestcolor = IG_IRIT_WHITE;
    int i;

    r = Color[0] >> 4;
    g = Color[1] >> 4;
    b = Color[2] >> 4;

    for (i = 8; i <= 15; i++) {	    /* Only consider high intensity colors */
	dist = colors[i].Red * colors[i].Red - r * r
	     + colors[i].Green * colors[i].Green - g * g
	     + colors[i].Blue * colors[i].Blue - b * b;
	if (dist < 0)
	    dist = -dist;
	if (dist < mindist) {
	    mindist = dist;
	    bestcolor = i;
	}
    }
    return bestcolor;
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
	    UpdateView = IGStateHandler(State, Statetatus, Refresh);
	    CenterText((IGGlblDepthCue ? "  Depth Cue  "
				       : "No Depth Cue"), DQ_BOT);
	    break;
	case IG_STATE_DOUBLE_BUFFER:
	    UpdateView = IGDStateHandler(State, StateStatus, Refresh);
	    if (IGGlblDoDoubleBuffer) {
	        doublebuffer();
	    }else{
	        singlebuffer();
	    }
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
* Handles input events                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   ChangeFactor:        A continuous numeric value between -1 and 1. This   *
*			 value will be used to set amount of event such as   *
*			 rotation or translation.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IGGraphicEventType:  Type of new event.                                  *
*****************************************************************************/
static IGGraphicEventType GetGraphicEvent(IrtRType *ChangeFactor)
{
    ULONG class;
    USHORT code;
    SHORT x, y;
    struct IntuiMessage *message;
    STATIC_DATA int LastX;

    ChangeFactor[0] = ChangeFactor[1] = 0.0;

    while(TRUE) {
	if (Animating) {
	    CenterText(" Animate ", ANIM_BOT);
	    Animating = FALSE;
	}
	/* Maybe we have something in communication socket. */
	if (!IGGlblStandAlone &&
	    IGReadObjectsFromSocket(IGGlblViewMode, &IGGlblDisplayList)) {
	    IGRedrawViewWindow();
	}

	if (message = (struct IntuiMessage *)
				    GetMsg(TransformWindow -> UserPort)) {
	    class = message -> Class;
	    code = message -> Code;
	    x = message -> MouseX;
	    y = message -> MouseY;
	    ReplyMsg((struct Message *) message);
	    if (class == IDCMP_MOUSEBUTTONS && code == SELECTDOWN) {
		LastX = x;
	        ModifyIDCMP(TransformWindow,
			    IDCMP_MOUSEBUTTONS | IDCMP_INTUITICKS);
	        *ChangeFactor = (IrtRType)((x - L - 1) - ((R - L - 1) / 2))
	    		      / (IrtRType)((R - L - 1) / 2);
		if (Hit(x, y, S_O_TOP, S_O_BOT)) {
		    IGGlblTransformMode =
			IGGlblTransformMode == IG_TRANS_OBJECT ?
					       IG_TRANS_SCREEN :
					       IG_TRANS_OBJECT;
		    CenterText((IGGlblTransformMode == IG_TRANS_OBJECT ?
			       "Object": "Screen"), S_O_BOT);
		    return IG_EVENT_SCR_OBJ_TGL;
		}
		if (Hit(x, y, O_P_TOP, O_P_BOT)) {
		    IGGlblViewMode =
			IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
					  IG_VIEW_ORTHOGRAPHIC :
					  IG_VIEW_PERSPECTIVE;
		    CenterText((IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
			       " Perspective " : "Orthographic"), O_P_BOT);
		    return IG_EVENT_PERS_ORTHO_TGL;
		}
		if (Hit(x, y, O_PZ_TOP, O_PZ_BOT)) {
		    return IG_EVENT_PERS_ORTHO_Z;
		}
		if (Hit(x, y, RX_TOP, RX_BOT)) {
		    return IG_EVENT_ROTATE_X;
		}
		if (Hit(x, y, RY_TOP, RY_BOT)) {
		    return IG_EVENT_ROTATE_Y;
		}
		if (Hit(x, y, RZ_TOP, RZ_BOT)) {
		    return IG_EVENT_ROTATE_Z;
		}
		if (Hit(x, y, TX_TOP, TX_BOT)) {
		    return IG_EVENT_TRANSLATE_X;
		}
		if (Hit(x, y, TY_TOP, TY_BOT)) {
		    return IG_EVENT_TRANSLATE_Y;
		}
		if (Hit(x, y, TZ_TOP, TZ_BOT)) {
		    return IG_EVENT_TRANSLATE_Z;
		}
		if (Hit(x, y, SCALE_TOP, SCALE_BOT)) {
		    return IG_EVENT_SCALE;
		}
		if (Hit(x, y, DQ_TOP, DQ_BOT)) {
		    IGGlblDepthCue = !IGGlblDepthCue;
		    CenterText((IGGlblDepthCue ?
			       "  Depth Cue  ": "No Depth Cue"), DQ_BOT);
		    return IG_EVENT_DEPTH_CUE;
		}
		if (Hit(x, y, ANIM_TOP, ANIM_BOT)) {
		    return IG_EVENT_ANIMATION;
		}
		if (Hit(x, y, SAVE_TOP, SAVE_BOT)) {
		    return IG_EVENT_SAVE_MATRIX;
		}
		if (Hit(x, y, PUSH_TOP, PUSH_BOT)) {
		    return IG_EVENT_PUSH_MATRIX;
		}
		if (Hit(x, y, POP_TOP, POP_BOT)) {
		    return IG_EVENT_POP_MATRIX;
		}
		if (Hit(x, y, QUIT_TOP, QUIT_BOT)) {
		    return IG_EVENT_QUIT;
		}
	    }
	    if (class == IDCMP_MOUSEBUTTONS && code == SELECTUP) {
	        ModifyIDCMP(TransformWindow, IDCMP_MOUSEBUTTONS);
	    }
	    if (class == IDCMP_INTUITICKS) {
		/* We may get events of movement in Y which are ignored. */
		if (x - LastX == 0) {
		    continue;
		}
		*ChangeFactor = (IrtRType) (x - LastX) /
				(IrtRType) ((R - L - 1) / 2);
		LastX = x;

		if (Hit(x, y, O_PZ_TOP, O_PZ_BOT)) {
		    return IG_EVENT_PERS_ORTHO_Z;
		}
		if (Hit(x, y, RX_TOP, RX_BOT)) {
		    return IG_EVENT_ROTATE_X;
		}
		if (Hit(x, y, RY_TOP, RY_BOT)) {
		    return IG_EVENT_ROTATE_Y;
		}
		if (Hit(x, y, RY_TOP, RY_BOT)) {
		    return IG_EVENT_ROTATE_Y;
		}
		if (Hit(x, y, RZ_TOP, RZ_BOT)) {
		    return IG_EVENT_ROTATE_Z;
		}
		if (Hit(x, y, TX_TOP, TX_BOT)) {
		    return IG_EVENT_TRANSLATE_X;
		}
		if (Hit(x, y, TY_TOP, TY_BOT)) {
		    return IG_EVENT_TRANSLATE_Y;
		}
		if (Hit(x, y, TZ_TOP, TZ_BOT)) {
		    return IG_EVENT_TRANSLATE_Z;
		}
		if (Hit(x, y, SCALE_TOP, SCALE_BOT)) {
		    return IG_EVENT_SCALE;
		}
	    }
	}
    }
    return IG_EVENT_NONE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Determines if point (x,y) is inside rectangle (L,y1) (R,y2).               *
*                                                                            *
* PARAMETERS:                                                                *
*   x, y:     Point to consider for inclusion.                               *
*   y1, y2:   Y dimensions of bounding box.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if inside, FALSE otherwise.                               *
*****************************************************************************/
static int Hit(int x, int y, int y1, int y2)
{
    return (x > L && x < R && y > y1 && y < y2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Deallocate resources                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Cleanup(void)
{
    if (ViewWindow[0]) {
	CloseWindow(ViewWindow[0]);
	ViewWindow[0] = NULL;
    }
    if (ViewWindow[1]) {
	CloseWindow(ViewWindow[1]);
	ViewWindow[1] = NULL;
    }
    if (TransformWindow) {
	CloseWindow(TransformWindow);
	TransformWindow = NULL;
    }
    if (screen) {
	CloseScreen(screen);
	screen = NULL;
    }
    if (IntuitionBase){
	CloseLibrary((struct Library *)IntuitionBase);
	IntuitionBase = NULL;
    }
    if (GfxBase){
	CloseLibrary((struct Library *)GfxBase);
	GfxBase = NULL;
    }
    if (LayersBase){
	CloseLibrary(LayersBase);
	LayersBase = NULL;
    }
    if (CyberGfxBase){
	CloseLibrary(CyberGfxBase);
	CyberGfxBase = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw a box in the transform window given two opposite corners              *
*                                                                            *
* PARAMETERS:                                                                *
*   x1, y1, x2, y2:   Dimension of box.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void Box(int x1, int y1, int x2, int y2)
{
    Move(trp, x1, y1);
    Draw(trp, x2, y1);
    Draw(trp, x2, y2);
    Draw(trp, x1, y2);
    Draw(trp, x1, y1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Same as above, but with a vertical line in the middle                      *
*                                                                            *
* PARAMETERS:                                                                *
*   x1, y1, x2, y2:   Dimension of box.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PotBox(int x1, int y1, int x2, int y2)
{
    Box(x1, y1, x2, y2);
    Move(trp, (x1 + x2) / 2, y1);
    Draw(trp, (x1 + x2) / 2, y2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw the transform window                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawTransformWindow(void)
{
    SetAPen(trp, IG_IRIT_LIGHTGREY);
    SetDrMd(trp, JAM2);

    Box(L, S_O_TOP, R, S_O_BOT);			   /* Screen/Object */
    CenterText((IGGlblTransformMode == IG_TRANS_OBJECT ?
		"Object": "Screen"), S_O_BOT);

    Box(L, O_P_TOP, R, O_P_BOT);		/* Orthographic/Perspective */
    CenterText((IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
		" Perspective " : "Orthographic"), O_P_BOT);
    PotBox(L, O_PZ_TOP, R, O_PZ_BOT);
    SideText("Z", O_PZ_BOT);

    PotBox(L, RX_TOP, R, RX_BOT);				  /* Rotate */
    PotBox(L, RY_TOP, R, RY_BOT);
    PotBox(L, RZ_TOP, R, RZ_BOT);
    CenterText("Rotate", RX_TOP);
    SideText("X", RX_BOT);
    SideText("Y", RY_BOT);
    SideText("Z", RZ_BOT);

    PotBox(L, TX_TOP, R, TX_BOT);			       /* Translate */
    PotBox(L, TY_TOP, R, TY_BOT);
    PotBox(L, TZ_TOP, R, TZ_BOT);
    CenterText("Translate", TX_TOP);
    SideText("X", TX_BOT);
    SideText("Y", TY_BOT);
    SideText("Z", TZ_BOT);

    PotBox(L, SCALE_TOP, R, SCALE_BOT);				  /* Scale */
    CenterText("Scale", SCALE_TOP);

    Box(L, DQ_TOP, R, DQ_BOT);			 /* Depth Cue/No Cepth Que */
    CenterText((IGGlblDepthCue ? "  Depth Cue  ": "No Depth Cue"), DQ_BOT);

    Box(L, ANIM_TOP, R, ANIM_BOT);			    /* Animate */
    CenterText("Animate", ANIM_BOT);

    Box(L, SAVE_TOP, R, SAVE_BOT);			    /* Save Matrix */
    CenterText("Save Matrix", SAVE_BOT);

    Box(L, PUSH_TOP, R, PUSH_BOT);			    /* Push Matrix */
    CenterText("Push Matrix", PUSH_BOT);

    Box(L, POP_TOP, R, POP_BOT);			     /* Pop Matrix */
    CenterText("Pop Matrix", POP_BOT);

    Box(L, QUIT_TOP, R, QUIT_BOT);				   /* Quit */
    CenterText("Quit", QUIT_BOT);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw some centered text in the transform window                            *
*                                                                            *
* PARAMETERS:                                                                *
*   txt:        Text to draw.                                                *
*   y:          Y location of text.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CenterText(char *txt, int y)
{
    int len;

    Move(trp, L, y - 5);
    len = TextLength(trp, txt, strlen(txt));
    Move(trp, L + 1 + (R - L - 1 - len) / 2, y - 5);
    Text(trp, txt, strlen(txt));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Draw some text next to a transform in the button window                    *
*                                                                            *
* PARAMETERS:                                                                *
*   txt:        Text to draw.                                                *
*   y:          Y location of text.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SideText(char *txt, int y)
{
    Move(trp, L - 10, y - 4);
    Text(trp, txt, strlen(txt));
}

STATIC_DATA void
singlebuffer(void)
{
    if (ViewWindow[!CurrentWindow]) {
	CloseWindow(ViewWindow[!CurrentWindow]);
	ViewWindow[!CurrentWindow] = NULL;
    }
    vrp = ViewWindow[CurrentWindow]->RPort;
}

STATIC_DATA void
doublebuffer(void)
{
    if (!ViewWindow[!CurrentWindow]) {
	ViewWindow[!CurrentWindow] =
	    OpenWindowTags(NULL, WA_CustomScreen, (ULONG)screen,
				 WA_Borderless,   TRUE,
				 WA_Width,	  ViewWidth,
				 WA_Height,	  ViewHeight,
				 WA_IDCMP,	  0,
				 WA_RMBTrap,	  TRUE,
				 TAG_END);
	if (!ViewWindow[!CurrentWindow]) {
	    fprintf(stderr, "Can't open second view window\n");
	    exit(RETURN_FAIL);
	}

	MoveWindowInFrontOf(ViewWindow[!CurrentWindow],
	 		    ViewWindow[CurrentWindow]);
    }
    vrp = ViewWindow[CurrentWindow]->RPort;
}

STATIC_DATA void
swapbuffers(void)
{
/* Intuition may take up to 1/10th sec to rearrange the windows if we use
   MoveWindowInFrontOf, causing severe flicker when swapping windows
   containing simple scenes. We avoid this by manipulating the layers
   ourselves.
 */
    MoveLayerInFrontOf(ViewWindow[CurrentWindow]->WLayer,
		       ViewWindow[!CurrentWindow]->WLayer);
    CurrentWindow = !CurrentWindow;
    vrp = ViewWindow[CurrentWindow]->RPort;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation. Checks if the user hit the Animate button *
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we need to abort, FALSE otherwise.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AnimCheckInterrupt                                                       M
*****************************************************************************/
int AnimCheckInterrupt(AnimationStruct *Anim)
{
    ULONG class;
    USHORT code;
    SHORT x, y;
    struct IntuiMessage *message;

    if (!Animating) {
	CenterText("Stop Anim", ANIM_BOT);
        Animating = TRUE;
    }
    if (message = (struct IntuiMessage *)
				GetMsg(TransformWindow -> UserPort)) {
	class = message -> Class;
	code = message -> Code;
	x = message -> MouseX;
	y = message -> MouseY;
	ReplyMsg((struct Message *) message);
	if (class == IDCMP_MOUSEBUTTONS && code == SELECTDOWN) {
	    if (Hit(x, y, ANIM_TOP, ANIM_BOT)) {
		Anim -> StopAnim = TRUE;
		CenterText(" Animate ", ANIM_BOT);
		Animating = FALSE;
		fprintf(stderr, "\nAnimation was interrupted by the user.\n");
		return TRUE;
	    }
	}
    }
    return FALSE;
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
    IPObjectStruct *PObj;
    IrtHmgnMatType IritView;

    IGPredefinedAnimation();

    Move(vrp, 0L, 0L);
    ClearScreen(vrp);

    switch (IGGlblViewMode) {		 /* Update the current view. */
	case IG_VIEW_ORTHOGRAPHIC:
	    GEN_COPY(IritView, IPViewMat, sizeof(IrtHmgnMatType));
	    break;
	case IG_VIEW_PERSPECTIVE:
	    MatMultTwo4by4(IritView, IPViewMat, IPPrspMat);
	    break;
    }

    IGTraverseObjListHierarchy(IGGlblDisplayList, IritView, IGViewObject);

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

    if (IGGlblAbortKeyPressed)
        return;

    HMGN_MAT_COPY(TMat, IGGlblCrntViewMat);
    HMGN_MAT_COPY(IGGlblCrntViewMat, Mat);

    IGDrawObject(PObj);

    HMGN_MAT_COPY(IGGlblCrntViewMat, TMat);
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
    fprintf(stderr, "Pick event attempted at %d %d\n", ScreenX, ScreenY);
    return NULL;
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
    fprintf(stderr, "Cursor event attempted at %d %d (Event %d)\n",
	    ScreenX, ScreenY, PickReport);
    return NULL;
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
    fprintf(stderr, "IGScreenToObject attempted at %d %d\n",
	    ScreenX, ScreenY);

    Pt[0] = 0.0;
    Pt[1] = 0.0;
    Pt[2] = 0.0;

    Dir[0] = Dir[1] = 0.0;
    Dir[1] = 1.0;
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
*   Msg:       Title message.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if yes was selected, FALSE otherwise.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritYesNoQuestion                                                      M
*****************************************************************************/
int IGIritYesNoQuestion(char *Msg)
{
    fprintf(stderr,
	    "Yes/No message box not implemented:\n%s\nreturning yes...", Msg);

    return TRUE;
}
