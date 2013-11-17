/*****************************************************************************
*   A windows NT interface for linear transformations on objects.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, July 1999.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"
#include "editmanp.h"

#define OBJ_MANIP_NUM_OF_SCALES	7

IRIT_STATIC_DATA IPObjectStruct
    **IGObjManipStartObjs = NULL;

IRIT_STATIC_DATA HWND ObjManipScales[OBJ_MANIP_NUM_OF_SCALES], ObjManipParamForm;
IRIT_STATIC_DATA HMENU ObjManipStatePopupMenuForm;

static BOOL CALLBACK ObjManipFormDlgProc(HWND hDlg,
					 UINT uMsg,
					 WPARAM wParam,
					 LPARAM lParam);
static void ButtonsCallBack(HWND hWnd, PVOID ClientData, PVOID *cbs);
static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData);

/*****************************************************************************
* DESCRIPTION:								     M
*   Creates the main LinTransParam window			  	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateObjectManip                                                        M
*****************************************************************************/
void CreateObjectManip(HWND IGTopLevel)
{
    IRIT_STATIC_DATA int ScalesIds[OBJ_MANIP_NUM_OF_SCALES] = {
	IDOT_ROT_X_TRACKBAR,
	IDOT_ROT_Y_TRACKBAR,
	IDOT_ROT_Z_TRACKBAR,
	IDOT_TRANS_X_TRACKBAR,
	IDOT_TRANS_Y_TRACKBAR,
	IDOT_TRANS_Z_TRACKBAR,
	IDOT_SCALE_TRACKBAR,
    };
    int i;

    /* ObjManipParam is child of IGTopLevel. */
    ObjManipParamForm = CreateDialog(GetWindowInstance(IGTopLevel),
				     MAKEINTRESOURCE(IDOT_OBJ_MANIP_FORM),
				     IGTopLevel,
				     ObjManipFormDlgProc);

    for (i = 0; i < OBJ_MANIP_NUM_OF_SCALES; i++) {
	HWND Scale = GetDlgItem(ObjManipParamForm, ScalesIds[i]);

	SendMessage(Scale, SBM_SETRANGE, -1000, 1000);
	SendMessage(Scale, SBM_SETPOS, 0, TRUE);

	ObjManipScales[i] = Scale;
    }

    ObjManipStatePopupMenuForm = CreatePopupMenu();
    for (i = 0; IGObjManipStateEntries[i] != NULL; i++)
	AppendMenu(ObjManipStatePopupMenuForm, MF_STRING, i + 1,
		   IGObjManipStateEntries[i]);

    IGObjManipParamUpdateWidget();
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
    char Line[IRIT_LINE_LEN];

    SetDlgItemText(ObjManipParamForm, IDOT_NAME, IGObjManipMatName);
    SetDlgItemText(ObjManipParamForm, IDOT_SCREEN_SPACE,
		   IGObjManipScreenSpace ? "Screen Space" : "Object Space");

    if (IGObjManipNumActiveObjs == 1) {
	sprintf(Line, "Manip. \"%s\"",
		IP_GET_OBJ_NAME(IGObjManipCurrentObjs[0]));
	IGObjManipPlaceMessage(Line);
    }
    else {
	sprintf(Line, "Manip. Several/All Objs");
	IGObjManipPlaceMessage(Line);
    }

    sprintf(Line, "%g", IGObjManipSnapDegrees);
    SetDlgItemText(ObjManipParamForm, IDOT_SNAP_DEGREES, Line);
    sprintf(Line, "%g", IGObjManipSnapDistance);
    SetDlgItemText(ObjManipParamForm, IDOT_SNAP_DISTANCE, Line);

    CheckDlgButton(ObjManipParamForm, IDOT_SNAP,
		   IGObjManipSnap ? BST_CHECKED : BST_UNCHECKED);

    SetDlgItemText(ObjManipParamForm, IDOT_STATE,
		   IGObjManipStateEntries[IGObjManipState]);
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
    SetDlgItemText(ObjManipParamForm, IDOT_MESSAGE, Msg);
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Pop up the object transform widget.					     M
*									     *
* PARAMETERS:								     M
*   None                                 				     M
*									     *
* RETURN VALUE:								     M
*   void 								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ObjManipCB   	                                                     M
*****************************************************************************/
void ObjManipCB(void)
{  
    RECT Wr, Dr, Cr;      

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(ObjManipParamForm, &Cr);		   /* Get form size. */
    SetWindowPos(ObjManipParamForm, 
		 HWND_TOP,
		 min(Wr.right + 3, Dr.right - Cr.right - 3),
		 Wr.top,
		 0, 
		 0,
		 SWP_NOSIZE | SWP_SHOWWINDOW);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements emulation of Motif ObjManipParam dialog.                      *
*   Is Win32 dialog callback function.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   hDlg:   Handle of the dialog that calls the function.                    *
*   uMsg:   Message sent by the dialog.                                      *
*   wParam: First parameter.                                                 *
*   lParam: Second parameter.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   BOOL: returns true when message is processed completely.                 *
*****************************************************************************/
static BOOL CALLBACK ObjManipFormDlgProc(HWND hDlg,
					 UINT uMsg,
					 WPARAM wParam,
					 LPARAM lParam)
{
    int i;
    PVOID clientId;
        
    switch (uMsg) {
	case WM_INITDIALOG:
            return TRUE;
	case WM_DESTROY:
	    return FALSE;
	case WM_COMMAND:
	    for (i = 0; i < OBJ_MANIP_NUM_OF_SCALES; i++)
	        if ((int) lParam == (int) ObjManipScales[i])
		    return TRUE;

	    clientId = (PVOID) LOWORD(wParam);    	  /* ID of the item. */
	    ButtonsCallBack((HWND) lParam, clientId, NULL);

	    return TRUE;
	case WM_HSCROLL:
	    for (i = 0; i < OBJ_MANIP_NUM_OF_SCALES; i++)
	        if ((int) lParam == (int) ObjManipScales[i])
		    ScaleCB((HWND) lParam, wParam, NULL);    

	    return TRUE;
	default:
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of all buttons. Invokes the proper action.            *
*                                                                            *
* PARAMETERS:                                                                *
*   w:            Of button to handle.                                       *
*   ClientData:   To get the Name of button.                                 *
*   cbs:          Not used.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ButtonsCallBack(HWND hWnd, PVOID ClientData, PVOID *cbs)
{
    char Str[IRIT_LINE_LEN_LONG];
    int i;
    double d;
    DWORD 
	Name = (DWORD) ClientData;
    POINT Point;

    switch (Name) {
	case IDOT_STATE:
	    Point.x = 100;
	    Point.y = 0;
	    ClientToScreen(ObjManipParamForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(ObjManipStatePopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, ObjManipParamForm, NULL)) == 0)
		break;

	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_STATE,
					   --i, NULL);
	    SetDlgItemText(ObjManipParamForm, IDOT_STATE,
			   IGObjManipStateEntries[IGObjManipState]);
	    break;
	case IDCANCEL:
	case IDOT_DISMISS:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_DISMISS,
					   0, NULL);
	    ShowWindow(ObjManipParamForm, SW_HIDE);
	    break;
	case IDOT_SUBMIT:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_SUBMIT_MAT,
					   0, NULL);
	    break;
	case IDOT_SCREEN_SPACE:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_OBJECT_SCREEN,
					   0, NULL);
	    break;
	case IDOT_NAME:
	    if ((i = GetDlgItemText(ObjManipParamForm, IDOT_NAME,
				    Str, IRIT_LINE_LEN_LONG - 1)) != 0)
	        strncpy(IGObjManipMatName, Str, IRIT_LINE_LEN_LONG - 2);
	    break;
	case IDOT_SNAP:
	    IGObjManipSnap = !IGObjManipSnap;
	    IGObjManipParamUpdateWidget();
	    break;
	case IDOT_SNAP_DEGREES:
	    if ((i = GetDlgItemText(ObjManipParamForm, IDOT_SNAP_DEGREES,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%lf", &d) == 1 && d >= 0 && d < 360)
		    IGObjManipSnapDegrees = d;
		else {
		    sprintf(Str, "%g", IGObjManipSnapDegrees);
		    SetDlgItemText(ObjManipParamForm, IDOT_SNAP_DEGREES, Str);
		}
	    }
	    break;
	case IDOT_SNAP_DISTANCE:
	    if ((i = GetDlgItemText(ObjManipParamForm, IDOT_SNAP_DISTANCE,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%lf", &d) == 1 && d >= 0 && d < 360)
		    IGObjManipSnapDistance = d;
		else {
		    sprintf(Str, "%g", IGObjManipSnapDistance);
		    SetDlgItemText(ObjManipParamForm, IDOT_SNAP_DISTANCE, Str);
		}
	    }
	    break;
	case IDOT_SAVE:
	    {
		OPENFILENAME ofn;
		char FileName[IRIT_LINE_LEN_LONG];

		strcpy(FileName, "*.itd");
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = ObjManipParamForm;
		ofn.lpstrFile = FileName;
		ofn.lpstrFileTitle = FileName;
		ofn.nMaxFile = IRIT_LINE_LEN_LONG;
		ofn.lpstrTitle = "Save objects as IRIT dat file";
		ofn.lpstrFilter = "*.itd";
		ofn.Flags = OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn) && !IRT_STR_ZERO_LEN(FileName)) {
		    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_SAVE,
						   0, FileName);
		}
		else
		    IGObjManipPlaceMessage("Failed to pick file name");
	    }
	    break;
	case IDOT_DELETE:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_DELETE, 0, NULL);
	    break;
	case IDOT_COLOR:
	    {
	        int *Clrs, c[3];
	        CHOOSECOLOR cc;
		static COLORREF CustColors[16];
		

		memset(&cc, 0, sizeof(CHOOSECOLOR));
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = ObjManipParamForm;
		Clrs = (int *)
		   IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_GET_COLOR,
						  0, NULL);

		if (Clrs != NULL) {
		    cc.rgbResult = RGB(Clrs[0], Clrs[1], Clrs[2]);
		    cc.lpCustColors = CustColors;
		    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

		    if (ChooseColor(&cc) != FALSE) {
		        c[0] = GetRValue(cc.rgbResult);
			c[1] = GetGValue(cc.rgbResult);
			c[2] = GetBValue(cc.rgbResult);
			IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_SET_COLOR,
						       0, c);
		    }
		}
	    }
	    break;
	case IDOT_REVERSE:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_REVERSE,
					   0, NULL);
	    break;
	case IDOT_RESET:
	    IGObjManipHandleNonMouseEvents(IG_OBJ_MANIP_EVENT_RESET, 0, NULL);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of scale.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Scale:       The HWND to handle.                                         *
*   wParam:      Slider Info.                                                *
*   CallData:    Holds the scale's value.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData)
{
    IRIT_STATIC_DATA int
	EventTable[OBJ_MANIP_NUM_OF_SCALES] = {
	    IG_EVENT_ROTATE_X,
	    IG_EVENT_ROTATE_Y,
	    IG_EVENT_ROTATE_Z,
	    IG_EVENT_TRANSLATE_X,
	    IG_EVENT_TRANSLATE_Y,
	    IG_EVENT_TRANSLATE_Z,
	    IG_EVENT_SCALE
	};
    int i,
        SliderActive = FALSE;
    LONG
	Value = SendMessage(Scale, SBM_GETPOS, 0, 0);
    IrtRType ChangeFactor[2];

    switch (LOWORD(wParam)) {
	case SB_LINERIGHT:
	    Value++;
	    break;
	case SB_LINELEFT:
	    Value--;
	    break;
	case SB_PAGERIGHT:
	    Value += 10;
	    break;
	case SB_PAGELEFT:
	    Value -= 10;
	    break;
	case SB_LEFT:
	    Value = -100;
	    break;
	case SB_RIGHT:
	    Value = 100;
	    break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	    Value = (short) HIWORD(wParam);
	    SliderActive = TRUE;
	    break;
	case SB_ENDSCROLL:
	    IGObjManipObjTrans(NULL, NULL, IG_EVENT_ACCUM_MATRIX,
			       ChangeFactor);
            IGObjManipFreeStartObjs(IGObjManipStartObjs);
            IGObjManipStartObjs = NULL;
	    return;
    }

    if (SliderActive) {
	if (IGObjManipStartObjs == NULL)
	    IGObjManipStartObjs = IGObjManipCopyStartObjs();
    }
    else {
	IGObjManipFreeStartObjs(IGObjManipStartObjs);
	IGObjManipStartObjs = NULL;
    }

    for (i = 0; i < OBJ_MANIP_NUM_OF_SCALES; i++)
	if (Scale == ObjManipScales[i])
	    break;

    ChangeFactor[0] = IGGlblChangeFactor * Value / 1000.0;
    ChangeFactor[1] = 0.0;
    if (IGObjManipNumActiveObjs > 0) {
	int j;

	for (j = 0; j < IGObjManipNumActiveObjs; j++)
	    IGObjManipObjTrans(IGObjManipCurrentObjs[j],
			       IGObjManipStartObjs == NULL ?
			           IGObjManipCurrentObjs[j] :
			           IGObjManipStartObjs[j],
			       EventTable[i], ChangeFactor);
	IGRedrawViewWindow();
    }
    else
	IGObjManipPlaceMessage("Select Object First");

    SendMessage(Scale, SBM_SETPOS, 0, TRUE);
}
