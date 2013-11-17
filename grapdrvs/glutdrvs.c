/*****************************************************************************
* GLut interface functions.						     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 2006    *
*****************************************************************************/

#ifdef __WINNT__
#   include <io.h>
#   include <fcntl.h>
#   include <windows.h>
#endif /* __WINNT__ */

#ifdef __WINCE__
#   include <windows.h>
#   include <commctrl.h> 
#   include <commdlg.h>
#   include <GLES/gl.h>
#   include <GLES/glutes.h>
#   include "oglesemu.h"
#   define	MAX_WINCE_ARGVS	100
#else
#   include <gl/gl.h>
#   include <gl/glu.h>
#   include <gl/glut.h>
#endif /* __WINCE__ */

#include <stdio.h>
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
#include "editsrfs.h"
#include "editmanp.h"
#include "glutdrvs.h"

#define GLUT_DRVS_WINDOW_TITLE		    "GLUT IRIT"
#define GLUT_DRVS_SCALE_FACTOR_MODIFIER     0.01

void glutDrvsCallBackDisplay(void);
void glutDrvsCallBackReShape(int w, int h);
void glutDrvsCallBackMotion(int x, int y);
void glutDrvsCallBackMouse(int button, int state, int x, int y);
void glutDrvsCallBackKeyBoardSpecial(int key, int x, int y);
void glutDrvsCallBackKeyBoard(unsigned char key, int x, int y);
static void HandleGlutDrvsMenu(int Entry);
static void CreateGlutDrvsMenu(void);

typedef enum {
    GLUT_DRVS_FILE_LOAD,
    GLUT_DRVS_FILE_BG_COLOR,
    GLUT_DRVS_FILE_CLEAR_ALL,
    GLUT_DRVS_FILE_QUIT,

    GLUT_DRVS_TRANS_TRANS,
    GLUT_DRVS_TRANS_ROT,
    GLUT_DRVS_TRANS_SCL,
    GLUT_DRVS_TRANS_MORE_SENS,
    GLUT_DRVS_TRANS_LESS_SENS,

    GLUT_DRVS_STATE_MORE_ISOS,
    GLUT_DRVS_STATE_LESS_ISOS,
    GLUT_DRVS_STATE_FINE_APPROX,
    GLUT_DRVS_STATE_COARSE_APPROX,
    GLUT_DRVS_STATE_SHORTER_VECS,
    GLUT_DRVS_STATE_LONGER_VECS,
    GLUT_DRVS_STATE_THICKER_LINES,
    GLUT_DRVS_STATE_THINNER_LINES,
    GLUT_DRVS_STATE_THICKER_PTS,
    GLUT_DRVS_STATE_THINNER_PTS,

    GLUT_DRVS_TGGL_DRAW_STYLE,
    GLUT_DRVS_TGGL_SCRN_OBJ_TRANS,
    GLUT_DRVS_TGGL_CONT_MOTION,
    GLUT_DRVS_TGGL_PERSP_ORTHO,
    GLUT_DRVS_TGGL_DEPTH_CUE,
    GLUT_DRVS_TGGL_ANTI_ALIASING,
    GLUT_DRVS_TGGL_BACK_FACE_CULL,
    GLUT_DRVS_TGGL_SHADE_MODE,
    GLUT_DRVS_TGGL_INTERNAL_EDGES,
    GLUT_DRVS_TGGL_VRTX_NRMLS,
    GLUT_DRVS_TGGL_POLY_NRMLS,
    GLUT_DRVS_TGGL_RVRS_ORIENT,
    GLUT_DRVS_TGGL_DRAW_CTL_MESH,
    GLUT_DRVS_TGGL_DRAW_SRF_POLYS,
    GLUT_DRVS_TGGL_DRAW_SRF_BNDRY,
    GLUT_DRVS_TGGL_DRAW_SRF_SILS,
    GLUT_DRVS_TGGL_DRAW_SRF_ISOS,
    GLUT_DRVS_TGGL_DRAW_4_PER_FLAT,
    GLUT_DRVS_TGGL_NUM_POLY_COUNT,
    GLUT_DRVS_TGGL_FRAME_PER_SEC,

    GLUT_DRVS_VIEW_FRONT,
    GLUT_DRVS_VIEW_SIDE,
    GLUT_DRVS_VIEW_TOP,
    GLUT_DRVS_VIEW_ISOMETRY
} GlutDrvsEventType;

#if defined(__WINCE__) || defined(__WINNT__)
IRIT_GLOBAL_DATA void
    *IGGlblWinBGImage = NULL;
IRIT_GLOBAL_DATA int
    IGGlblWinBGImageX = 0,
    IGGlblWinBGImageY = 0;
#endif /* __WINCE__|| __WINNT__ */

IRIT_GLOBAL_DATA unsigned int
    IGViewWidth = GLUT_DRVS_DEFAULT_VIEW_WIDTH,
    IGViewHeight = GLUT_DRVS_DEFAULT_VIEW_HEIGHT;
IRIT_GLOBAL_DATA int
    GlblMouseLastPos[2] = { 0, 0 },
    GlblMouseInitPos[2] = { 0, 0 },
    GlblMouseActiveButton = GLUT_DRVS_MOUSE_NO_BUTTON;
IRIT_STATIC_DATA int
    GlblGlutDrvsMouseOperation = IG_EVENT_ROTATE;
IRIT_STATIC_DATA char
    GlblFileName[IRIT_LINE_LEN_VLONG] = { 0 };

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
*   Display call back of glut                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackDisplay                                                  M
*****************************************************************************/
void glutDrvsCallBackDisplay(void)
{
    char NewTitle[IRIT_LINE_LEN_LONG];

    NewTitle[0] = 0;

    if (IGGlblCountNumPolys)
	IGGlblNumPolys = 0;
    if (IGGlblCountFramePerSec)
        IGUpdateFPS(TRUE);

    if (IGGlbl4Views) {
        /* Not supported. */
    }
    else {
        IGRedrawViewWindowOGL(TRUE);

	glutSwapBuffers();

	if (IGGlblCountNumPolys)
	    sprintf(NewTitle, ", Rendered %d polygons", IGGlblNumPolys);
    }

    if (IGGlblCountFramePerSec) {
        IGUpdateFPS(FALSE);
        sprintf(&NewTitle[strlen(NewTitle)],
		", FPS: %.1f", IGGlblFramePerSec);
    }

    if (IGGlblCountNumPolys || IGGlblCountFramePerSec)
        glutSetWindowTitle(IGGenerateWindowHeaderString(NewTitle));

#ifdef __WINCE__
    {
        static int
	    LoadFirst = FALSE;

	if (!LoadFirst) {
	    LoadFirst = TRUE;
	    HandleGlutDrvsMenu(GLUT_DRVS_FILE_LOAD); /* Load first file. */
	}
    }
#endif /* __WINCE__ */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Captures the pixels in the window pointed out by hWnd.  		     M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:   The dimensions of the window and the captured buffer.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void *:   A buffer of size (3 *x *y), allocated dynamically, that        M
*	      captures the pixels in the window.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGCaptureWindowPixels                                                    M
*****************************************************************************/
void *IGCaptureWindowPixels(int *x, int *y)
{
    char *Pixels;

    /* Get the image from Open GL. */
    *x = (*x + 3) & 0xffffffc;     /* 4-Align needed in OGL. */

    Pixels = (char *) IritMalloc((*x + 1) * (*y + 1) * 3);
    glReadPixels(0, 0, *x, *y, GL_RGB, GL_UNSIGNED_BYTE,
		 Pixels);

    return Pixels;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Window resize call back of glut                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   w, h:    New width and height of window.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackReShape                                                  M
*****************************************************************************/
void glutDrvsCallBackReShape(int w, int h)
{
    int i;

    IGViewWidth = w;
    IGViewHeight = h;

    if (IGGlblBackGroundImage && w != 0 && h != 0) {
        if (IGGlblWinBGImage != NULL)
	    IritFree(IGGlblWinBGImage);

	IGGlblWinBGImageX = w;
	IGGlblWinBGImageY = h;
	IGGlblWinBGImage = IGCaptureWindowPixels(&IGGlblWinBGImageX,
						 &IGGlblWinBGImageY);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, IGViewWidth, 0.0, IGViewHeight,
	    IRIT_MIN(-((int) IGViewWidth), -((int) IGViewHeight)) * 2,
	    IRIT_MAX( IGViewWidth,  IGViewHeight) * 2);

    if (IGViewWidth > IGViewHeight) {
        i = (IGViewWidth - IGViewHeight) / 2;

	glViewport(0, -i, IGViewWidth, IGViewWidth);
    }
    else {
        i = (IGViewHeight - IGViewWidth) / 2;

	glViewport(-i, 0, IGViewHeight, IGViewHeight);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Call back for mouse motion of glut.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:   Mouse position in window.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackMotion                                                   M
*****************************************************************************/
void glutDrvsCallBackMotion(int x, int y)
{
    IrtRType ChangeFactor[2];

    switch (GlblMouseActiveButton) {
	case GLUT_DRVS_MOUSE_LEFT_BUTTON:
	    ChangeFactor[0] = (x - GlblMouseLastPos[0]) * IGGlblChangeFactor;
	    ChangeFactor[1] = (GlblMouseLastPos[1] - y) * IGGlblChangeFactor;

	    if (GlblGlutDrvsMouseOperation == IG_EVENT_SCALE) {
	        ChangeFactor[0] *= GLUT_DRVS_SCALE_FACTOR_MODIFIER;
		ChangeFactor[1] *= GLUT_DRVS_SCALE_FACTOR_MODIFIER;
	    }
	    if (IGProcessEvent(GlblGlutDrvsMouseOperation, ChangeFactor))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_DRVS_MOUSE_NO_BUTTON:
	default:
	    break;
    }

    GlblMouseLastPos[0] = x;
    GlblMouseLastPos[1] = y;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Call back of mouse event of glut.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   button:    The pressed button.                                           M
*   state:     The status of the press.                                      M
*   x, y:      Location of mouse in window.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackMouse                                                    M
*****************************************************************************/
void glutDrvsCallBackMouse(int button, int state, int x, int y)
{
    GlblMouseLastPos[0] = GlblMouseInitPos[0] = x;
    GlblMouseLastPos[1] = GlblMouseInitPos[1] = y;

    switch (button) {
	case GLUT_LEFT_BUTTON:
	    switch (state) {
		case GLUT_DOWN:
		    GlblMouseActiveButton = GLUT_DRVS_MOUSE_LEFT_BUTTON;
		    break;
		case GLUT_UP:
		default:
		    GlblMouseActiveButton = GLUT_DRVS_MOUSE_NO_BUTTON;
		    break;
            }
            break;
        default:
            break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Keyboard call back function of glut.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   key:     Key stroked.                                                    M
*   x, y:    Location of mouse in window.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackKeyBoardSpecial                                          M
*****************************************************************************/
void glutDrvsCallBackKeyBoardSpecial(int key, int x, int y)
{
    IrtRType ChangeFactor[2];

    switch (key) {
        case GLUT_KEY_F1:
	    if (GlblGlutDrvsMouseOperation == IG_EVENT_ROTATE)
	        GlblGlutDrvsMouseOperation = IG_EVENT_SCALE;
	    else
	        GlblGlutDrvsMouseOperation = IG_EVENT_ROTATE;
	    break;
	case GLUT_KEY_F2:
	    if (IGHandleState(IG_STATE_DRAW_STYLE, IG_STATE_TGL, FALSE))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_KEY_F3:
	    if (IGHandleState(IG_STATE_PERS_ORTHO_TGL, IG_STATE_TGL, FALSE))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_KEY_F4:
	    if (IGHandleState(IG_STATE_VIEW_ISOMETRY, IG_STATE_TGL, FALSE))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_KEY_RIGHT:
	    ChangeFactor[0] = 10 * IGGlblChangeFactor;
	    ChangeFactor[1] = 0;
	    if (IGProcessEvent(IG_EVENT_TRANSLATE, ChangeFactor))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_KEY_LEFT:
	    ChangeFactor[0] = -10 * IGGlblChangeFactor;
	    ChangeFactor[1] = 0;
	    if (IGProcessEvent(IG_EVENT_TRANSLATE, ChangeFactor))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_KEY_DOWN:
	    ChangeFactor[0] = 0;
	    ChangeFactor[1] = -10 * IGGlblChangeFactor;
	    if (IGProcessEvent(IG_EVENT_TRANSLATE, ChangeFactor))
	        glutDrvsCallBackDisplay();
	    break;
	case GLUT_KEY_UP:
	    ChangeFactor[0] = 0;
	    ChangeFactor[1] = 10 * IGGlblChangeFactor;
	    if (IGProcessEvent(IG_EVENT_TRANSLATE, ChangeFactor))
	        glutDrvsCallBackDisplay();
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Keyboard call back function of glut.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   char:    Key stroked.                                                    M
*   x, y:    Location of mouse in window.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackKeyBoard                                                 M
*****************************************************************************/
void glutDrvsCallBackKeyBoard(unsigned char key, int x, int y)
{
    switch (key) {
	case 'q':
	case 27:
	    exit(0);
	    break;
#   ifdef __WINCE__
	case '\n':
	    /* Simulate a MIDDLE click to open the menu. */
	    glutSimulateButton(GLUT_MIDDLE_BUTTON, 10, 20);
	    break;
#   endif /* __WINCE__ */
	default:
	    break;
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
    return IGStateHandler(State, StateStatus, Refresh);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Handles the menu item selected.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Entry:   Menu item selected.                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void HandleGlutDrvsMenu(int Entry)
{
    int r = TRUE; /* Refresh. */

    switch (Entry) {
	case GLUT_DRVS_FILE_LOAD:
#if defined(__WINNT__) || defined(__WINCE__)
	    {
	        OPENFILENAME ofn;
#ifdef __WINCE__
		WCHAR FullFileName[IRIT_LINE_LEN_LONG],
		      FileName[IRIT_LINE_LEN_LONG];
#else
		char FullFileName[IRIT_LINE_LEN_VLONG],
		     FileName[IRIT_LINE_LEN_VLONG];
#endif /* __WINCE__ */

		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = NULL;
		ofn.nMaxFile = IRIT_LINE_LEN_LONG;
		ofn.nMaxFileTitle = IRIT_LINE_LEN_LONG;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
#ifdef __WINCE__
		FileName[0] = FullFileName[0] = 0;
		ofn.lpstrFile = FullFileName;
		ofn.lpstrFileTitle = FileName;
		ofn.lpstrTitle = _T("GlutDrvs Load Irit File");
		ofn.lpstrFilter = _T("itd\0*.itd\0ibd\0*.ibd\0All\0*.*\0\0");
		ofn.lpstrInitialDir = _T("\\");
#else
		FileName[0] = FullFileName[0] = 0;
		ofn.lpstrFile = FullFileName;
		ofn.lpstrFileTitle = FileName;
		ofn.lpstrTitle = "GlutDrvs Load Image";
		ofn.lpstrFilter = "itd\0*.itd\0ibd\0*.ibd\0all\0*.*\0\0";
		ofn.lpstrInitialDir = "\\";
#endif /* __WINCE__ */

		if (GetOpenFileName(&ofn)) {
		    IPObjectStruct *PObj;
		    char *p[2];
#ifdef __WINCE__
		    char Name[IRIT_LINE_LEN_LONG];

		    wcstombs(Name, FullFileName, IRIT_LINE_LEN_LONG);
		    p[0] = Name;
#else
		    p[0] = FullFileName;
#endif /* __WINCE__ */
		    if ((PObj = IPGetDataFiles((const char **) p,
					       1, TRUE, FALSE)) != NULL) {
		        IGAddReplaceObjDisplayList(&IGGlblDisplayList,
						   PObj, NULL);
			r = TRUE;
		    }
		}
	    }
#endif /* __WINNT__ || __WINCE__ */
	    break;
	case GLUT_DRVS_FILE_BG_COLOR:
#if defined(__WINNT__) || defined(__WINCE__)
	    {
	        CHOOSECOLOR cc;
		static COLORREF CustColors[16];

		memset(&cc, 0, sizeof(CHOOSECOLOR));
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = NULL;
		cc.rgbResult = RGB(IGGlblBackGroundColor[0],
				   IGGlblBackGroundColor[1],
				   IGGlblBackGroundColor[2]);
		cc.lpCustColors = CustColors;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;

		if ((r = ChooseColor(&cc)) != FALSE) {
		    IGGlblBackGroundColor[0] = GetRValue(cc.rgbResult);
		    IGGlblBackGroundColor[1] = GetGValue(cc.rgbResult);
		    IGGlblBackGroundColor[2] = GetBValue(cc.rgbResult);
		}
	    }   
#endif /* __WINNT__ || __WINCE__ */
	    break;
	case GLUT_DRVS_FILE_CLEAR_ALL:
	    r = IGHandleState(IG_STATE_CLEAR_VIEW, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_FILE_QUIT:
	    exit(0);
	    break;

	case GLUT_DRVS_TRANS_TRANS:
	    GlblGlutDrvsMouseOperation = IG_EVENT_TRANSLATE;
	    break;
	case GLUT_DRVS_TRANS_ROT:
	    GlblGlutDrvsMouseOperation = IG_EVENT_ROTATE;
	    break;
	case GLUT_DRVS_TRANS_SCL:
	    GlblGlutDrvsMouseOperation = IG_EVENT_SCALE;
	    break;
	case GLUT_DRVS_TRANS_MORE_SENS:
	    r = IGHandleState(IG_STATE_MOUSE_SENSITIVE, IG_STATE_INC, FALSE);
	    break;
	case GLUT_DRVS_TRANS_LESS_SENS:
	    r = IGHandleState(IG_STATE_MOUSE_SENSITIVE, IG_STATE_DEC, FALSE);
	    break;

	case GLUT_DRVS_STATE_MORE_ISOS:
	    r = IGHandleState(IG_STATE_NUM_ISOLINES, IG_STATE_INC, FALSE);
	    break;
	case GLUT_DRVS_STATE_LESS_ISOS:
	    r = IGHandleState(IG_STATE_NUM_ISOLINES, IG_STATE_DEC, FALSE);
	    break;
	case GLUT_DRVS_STATE_FINE_APPROX:
	    r = IGHandleState(IG_STATE_POLY_APPROX, IG_STATE_INC, FALSE) ||
	        IGHandleState(IG_STATE_POLYGON_APPROX, IG_STATE_INC, FALSE) ||
	        IGHandleState(IG_STATE_SAMP_PER_CRV_APPROX,
				      IG_STATE_INC, FALSE);
	    break;
	case GLUT_DRVS_STATE_COARSE_APPROX:
	    r = IGHandleState(IG_STATE_POLY_APPROX, IG_STATE_DEC, FALSE) ||
	        IGHandleState(IG_STATE_POLYGON_APPROX, IG_STATE_DEC, FALSE) ||
	        IGHandleState(IG_STATE_SAMP_PER_CRV_APPROX,
			      IG_STATE_DEC, FALSE);
	    break;
	case GLUT_DRVS_STATE_SHORTER_VECS:
	    r = IGHandleState(IG_STATE_LENGTH_VECTORS, IG_STATE_DEC, FALSE);
	    break;
	case GLUT_DRVS_STATE_LONGER_VECS:
	    r = IGHandleState(IG_STATE_LENGTH_VECTORS, IG_STATE_INC, FALSE);
	    break;
	case GLUT_DRVS_STATE_THICKER_LINES:
	    r = IGHandleState(IG_STATE_WIDTH_LINES, IG_STATE_INC, FALSE);
	    break;
	case GLUT_DRVS_STATE_THINNER_LINES:
	    r = IGHandleState(IG_STATE_WIDTH_LINES, IG_STATE_DEC, FALSE);
	    break;
	case GLUT_DRVS_STATE_THICKER_PTS:
	    r = IGHandleState(IG_STATE_WIDTH_POINTS, IG_STATE_INC, FALSE);
	    break;
	case GLUT_DRVS_STATE_THINNER_PTS:
	    r = IGHandleState(IG_STATE_WIDTH_POINTS, IG_STATE_DEC, FALSE);
	    break;

	case GLUT_DRVS_TGGL_DRAW_STYLE:
	    r = IGHandleState(IG_STATE_DRAW_STYLE, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_SCRN_OBJ_TRANS:
	    r = IGHandleState(IG_STATE_SCR_OBJ_TGL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_CONT_MOTION:
	    r = IGHandleState(IG_STATE_CONT_MOTION, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_PERSP_ORTHO:
	    r = IGHandleState(IG_STATE_PERS_ORTHO_TGL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DEPTH_CUE:
	    r = IGHandleState(IG_STATE_DEPTH_CUE, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_ANTI_ALIASING:
	    r = IGHandleState(IG_STATE_ANTI_ALIASING, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_BACK_FACE_CULL:
	    r = IGHandleState(IG_STATE_BACK_FACE_CULL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_SHADE_MODE:
	    r = IGHandleState(IG_STATE_SHADING_MODEL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_INTERNAL_EDGES:
	    r = IGHandleState(IG_STATE_DRAW_INTERNAL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_VRTX_NRMLS:
	    r = IGHandleState(IG_STATE_DRAW_VNORMAL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_POLY_NRMLS:
	    r = IGHandleState(IG_STATE_DRAW_PNORMAL, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_RVRS_ORIENT:
	    r = IGHandleState(IG_STATE_NRML_ORIENT, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DRAW_CTL_MESH:
	    r = IGHandleState(IG_STATE_DRAW_SRF_MESH, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DRAW_SRF_POLYS:
	    r = IGHandleState(IG_STATE_DRAW_SRF_POLY, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DRAW_SRF_BNDRY:
	    r = IGHandleState(IG_STATE_DRAW_SRF_BNDRY, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DRAW_SRF_SILS:
	    r = IGHandleState(IG_STATE_DRAW_SRF_SILH, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DRAW_SRF_ISOS:
	    r = IGHandleState(IG_STATE_DRAW_SRF_WIRE, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_DRAW_4_PER_FLAT:
	    r = IGHandleState(IG_STATE_FOUR_PER_FLAT, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_NUM_POLY_COUNT:
	    r = IGHandleState(IG_STATE_NUM_POLY_COUNT, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_TGGL_FRAME_PER_SEC:
	    r = IGHandleState(IG_STATE_FRAME_PER_SEC, IG_STATE_TGL, FALSE);
	    break;

	case GLUT_DRVS_VIEW_FRONT:
	    r = IGHandleState(IG_STATE_VIEW_FRONT, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_VIEW_SIDE:
	    r = IGHandleState(IG_STATE_VIEW_SIDE, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_VIEW_TOP:
	    r = IGHandleState(IG_STATE_VIEW_TOP, IG_STATE_TGL, FALSE);
	    break;
	case GLUT_DRVS_VIEW_ISOMETRY:
	    r = IGHandleState(IG_STATE_VIEW_ISOMETRY, IG_STATE_TGL, FALSE);
	    break;
    }

    /* Should we refresh? */
    if (r)
        glutDrvsCallBackDisplay();
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
* DESCRIPTION:                                                               *
*   Constructs the menu.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CreateGlutDrvsMenu(void)
{
    int MainMenu,
        SubMenuFile, SubMenuTrans, SubMenuState, SubMenuToggles, SubMenuViews;

    SubMenuFile = glutCreateMenu(HandleGlutDrvsMenu); 
    glutAddMenuEntry("Load",		GLUT_DRVS_FILE_LOAD);
    glutAddMenuEntry("BG color",	GLUT_DRVS_FILE_BG_COLOR);
    glutAddMenuEntry("Clear all",	GLUT_DRVS_FILE_CLEAR_ALL);
 
    SubMenuTrans = glutCreateMenu(HandleGlutDrvsMenu); 
    glutAddMenuEntry("Translate",	GLUT_DRVS_TRANS_TRANS);
    glutAddMenuEntry("Rotate",		GLUT_DRVS_TRANS_ROT);
    glutAddMenuEntry("Scale",		GLUT_DRVS_TRANS_SCL);
    glutAddMenuEntry("More sensitive",	GLUT_DRVS_TRANS_MORE_SENS);
    glutAddMenuEntry("Less sensitive",	GLUT_DRVS_TRANS_LESS_SENS);
 
    SubMenuState = glutCreateMenu(HandleGlutDrvsMenu); 
    glutAddMenuEntry("More isolines",	GLUT_DRVS_STATE_MORE_ISOS);
    glutAddMenuEntry("Less isolines",	GLUT_DRVS_STATE_LESS_ISOS);
    glutAddMenuEntry("Finer approx.",	GLUT_DRVS_STATE_FINE_APPROX);
    glutAddMenuEntry("Coarser approx.",	GLUT_DRVS_STATE_COARSE_APPROX);
    glutAddMenuEntry("Shorter vectors",	GLUT_DRVS_STATE_SHORTER_VECS);
    glutAddMenuEntry("Longer vectors",	GLUT_DRVS_STATE_LONGER_VECS);
    glutAddMenuEntry("Thicker lines",	GLUT_DRVS_STATE_THICKER_LINES);
    glutAddMenuEntry("Thinner lines",	GLUT_DRVS_STATE_THINNER_LINES);
    glutAddMenuEntry("Thicker points",	GLUT_DRVS_STATE_THICKER_PTS);
    glutAddMenuEntry("Thinner points",	GLUT_DRVS_STATE_THINNER_PTS);
 
    SubMenuToggles = glutCreateMenu(HandleGlutDrvsMenu); 
    glutAddMenuEntry("Draw Style",	GLUT_DRVS_TGGL_DRAW_STYLE);
    glutAddMenuEntry("Scrn/Obj Trans",	GLUT_DRVS_TGGL_SCRN_OBJ_TRANS);
    glutAddMenuEntry("Cont. Motion",	GLUT_DRVS_TGGL_CONT_MOTION);
    glutAddMenuEntry("Persp./Ortho.",	GLUT_DRVS_TGGL_PERSP_ORTHO);
    glutAddMenuEntry("Depth Cue",	GLUT_DRVS_TGGL_DEPTH_CUE);
    glutAddMenuEntry("Anti Aliasing",	GLUT_DRVS_TGGL_ANTI_ALIASING);
    glutAddMenuEntry("Back Face Cull",	GLUT_DRVS_TGGL_BACK_FACE_CULL);
    glutAddMenuEntry("Shading Modes",	GLUT_DRVS_TGGL_SHADE_MODE);
    glutAddMenuEntry("Internal Edges",	GLUT_DRVS_TGGL_INTERNAL_EDGES);
    glutAddMenuEntry("Vertex Normals",  GLUT_DRVS_TGGL_VRTX_NRMLS);
    glutAddMenuEntry("Poly Normals",	GLUT_DRVS_TGGL_POLY_NRMLS);
    glutAddMenuEntry("Reverse Orient.",	GLUT_DRVS_TGGL_RVRS_ORIENT);
    glutAddMenuEntry("Draw Ctl Mesh",	GLUT_DRVS_TGGL_DRAW_CTL_MESH);
    glutAddMenuEntry("Draw Srf Polys",	GLUT_DRVS_TGGL_DRAW_SRF_POLYS);
    glutAddMenuEntry("Draw Srf Bndry",	GLUT_DRVS_TGGL_DRAW_SRF_BNDRY);
    glutAddMenuEntry("Draw Srf Sils",	GLUT_DRVS_TGGL_DRAW_SRF_SILS);
    glutAddMenuEntry("Draw Srf Isos",	GLUT_DRVS_TGGL_DRAW_SRF_ISOS);
    glutAddMenuEntry("4 Tris per flat",	GLUT_DRVS_TGGL_DRAW_4_PER_FLAT);
    glutAddMenuEntry("# Polys Count",	GLUT_DRVS_TGGL_NUM_POLY_COUNT);
    glutAddMenuEntry("Frames Per Sec",	GLUT_DRVS_TGGL_FRAME_PER_SEC);

    SubMenuViews = glutCreateMenu(HandleGlutDrvsMenu); 
    glutAddMenuEntry("Front",		GLUT_DRVS_VIEW_FRONT);
    glutAddMenuEntry("Side",		GLUT_DRVS_VIEW_SIDE);
    glutAddMenuEntry("Top",		GLUT_DRVS_VIEW_TOP);
    glutAddMenuEntry("Isometry",	GLUT_DRVS_VIEW_ISOMETRY);

    MainMenu = glutCreateMenu(HandleGlutDrvsMenu); 
    glutAddSubMenu("File",	 	SubMenuFile);
    glutAddSubMenu("Trans",		SubMenuTrans);
    glutAddSubMenu("State",		SubMenuState);
    glutAddSubMenu("Toggles",		SubMenuToggles);
    glutAddSubMenu("Views",		SubMenuViews);
    glutAddMenuEntry("Quit",		GLUT_DRVS_FILE_QUIT);

    glutAttachMenu(GLUT_MIDDLE_BUTTON);
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
*   Should we stop this animation? Senses for events.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we needs to abort, FALSE otherwise.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimCheckInterrupt                                                     M
*****************************************************************************/
int GMAnimCheckInterrupt(GMAnimationStruct *Anim)
{
    return Anim -> StopAnim;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make error message box.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:       Error message.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGIritError                                                              M
*****************************************************************************/
void IGIritError(char *Msg)
{
#if __WINNT__
    MessageBox(NULL, Msg, "Error", MB_ICONSTOP | MB_OK);
#elif __WINCE__
    WCHAR WMsg[IRIT_LINE_LEN_LONG];

    mbstowcs(WMsg, Msg, IRIT_LINE_LEN_LONG);

    MessageBox(NULL, WMsg, _T("Error"), MB_ICONSTOP | MB_OK);
#else
    fprintf(stderr, "Error: %s\n", Msg);
#endif /* __WINNT__ || __WINCE__ */
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
#   ifdef  __WINNT__ 
        Beep(1000, 100);
#   endif /* __WINNT__ */
}

#ifdef __WINNT__

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Redirects stdin/out/err to a second console window.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGRedirectIOToConsole                                                    M
*****************************************************************************/
void IGRedirectIOToConsole(void)
{
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

    /* Allocate a console for this app. */
    AllocConsole();

    /* Set the screen buffer to be big enough to let us scroll text. */
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), 
			       &coninfo);
    coninfo.dwSize.Y = GLUT_DRVS_MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), 
			       coninfo.dwSize);

    /* Redirect unbuffered STDOUT to the console. */
    lStdHandle = (long) GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Redirect unbuffered STDIN to the console. */
    lStdHandle = (long) GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "r");
    *stdin = *fp;
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Redirect unbuffered STDERR to the console. */
    lStdHandle = (long) GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf(stderr, NULL, _IONBF, 0);
}

#endif /* __WINNT__ */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Idle function for glut.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   glutDrvsCallBackIdle                                                     M
*****************************************************************************/
void glutDrvsCallBackIdle(void)
{
    IritSleep(1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Main module of glutdrvs - GLUT Open GL graphics display of IRIT.    	     M
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
int main(int argc, char **argv)
{
    int i;

#ifdef DEBUG_IRIT_MALLOC
    IritInitTestDynMemory();
#endif /* DEBUG_IRIT_MALLOC */

    /* Must select a function to draw polys. */
    IGDrawPolyFuncPtr = IGDrawPoly;

#ifdef __WINCE__
    if (argc > 1) {    /* All command line parameters are given in argv[1]. */
        int j = 0,
	    i = 0;
	char *Wargv[MAX_WINCE_ARGVS],
	    *Wargs = IritStrdup(argv[1]); 

	Wargv[j++] = argv[0];
	Wargv[j++] = Wargs;
	do {
	    if (Wargs[i] == '"') {
	        i++;
		Wargv[j-1] = &Wargv[j-1][1]; /* Skip the '"'. */
		while (Wargs[i] != '"' && Wargs[i] != 0)
		    i++;
		if (Wargs[i] != '"')
		    Wargs[i++] = 0;
	    }
	    else
	        while (!isspace(Wargs[i])) {
		    i++;
		}
	    Wargs[i++] = 0;
	    while (isspace(Wargs[i]))
	        i++;
	    if (Wargs[i] > 0)
	        Wargv[j++] = &Wargs[i];
	    if (j > MAX_WINCE_ARGVS - 2)
	        break;
	}
	while (Wargs[i] > 0);

	argv = Wargv;
	argc = j;
    }
#endif /* __WINCE__ */

    IGConfigureGlobals("glutdrvs", argc, argv);

    /* Init the glut library and set it up. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(IGViewWidth, IGViewHeight);
    glutCreateWindow(IGGenerateWindowHeaderString(NULL));
    glClearColor((float) (IGGlblBackGroundColor[0] / 255.0),
		 (float) (IGGlblBackGroundColor[1] / 255.0),
		 (float) (IGGlblBackGroundColor[2] / 255.0),
		 1.0);

    for (i = 0; i < IGShadeParam.NumOfLightSrcs; i++) {
        IRIT_STATIC_DATA IrtVecType
	    WhiteColor = { 1.0, 1.0, 1.0 };

        IGSetLightSource(IGShadeParam.LightPos[i], WhiteColor, i);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, IGViewWidth, 0.0, IGViewHeight,
	    IRIT_MIN(-((int) IGViewWidth), -((int) IGViewHeight)) * 2,
	    IRIT_MAX( IGViewWidth,  IGViewHeight) * 2);

    glutDisplayFunc(glutDrvsCallBackDisplay);
    glutReshapeFunc(glutDrvsCallBackReShape);
    glutKeyboardFunc(glutDrvsCallBackKeyBoard);
    glutSpecialFunc(glutDrvsCallBackKeyBoardSpecial);
    glutMotionFunc(glutDrvsCallBackMotion);
    glutMouseFunc(glutDrvsCallBackMouse);
    glutIdleFunc(glutDrvsCallBackIdle);

    CreateGlutDrvsMenu();

    glutMainLoop();
}

/* All definitions below serve no real purpose - just enable the link. */

IRIT_GLOBAL_DATA int
    IGObjManipNumActiveObjs = 0,
    IGSrfEditActive = FALSE,
    IGCrvEditActive = FALSE;
IRIT_GLOBAL_DATA IPObjectStruct
    **IGObjManipCurrentObjs = NULL,
    *IGCrvEditCurrentObj = NULL,
    *IGCrvEditPreloadEditCurveObj = NULL,
    *IGSrfEditCurrentObj = NULL,
    *IGSrfEditPreloadEditSurfaceObj = NULL;

int IGSetDisplay4Views(int Display4Views) { return 0; }
void IGObjManipDetachObj(void) {}
void CEditDetachCurve(void) {}
void CEditRedrawCrv(void) {}
void IGRedrawViewWindow(void) {}
void IGHandlePickObject(IGPickEntityType PickEntity) {}
void SEditDetachSurface(void) {}
void SEditRedrawSrf(void) {}
int IGCGFfdDraw(IPObjectStruct *PObj) { return 0; }
int IGCGDrawDTexture(IPObjectStruct *PObj) { return 0; }
void IGCGFreeDTexture(IPObjectStruct *PObj) {}
void IGInitializeSubViewMat(void) {}

