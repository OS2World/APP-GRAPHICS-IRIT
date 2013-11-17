/*****************************************************************************
*  A windows NT interface for srf edit param. (based on motif interface).    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 0.1, June 1999.   *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"
#include "editsrfs.h"

IRIT_STATIC_DATA HWND SrfEditForm, UMRSrfEditScaleForm, VMRSrfEditScaleForm;
IRIT_STATIC_DATA HMENU SrfEditStatePopupMenuForm, SrfEditSubdivPopupMenuForm,
    SrfEditRefinePopupMenuForm, SrfEditEndCondPopupMenuForm,
    SrfEditParamPopupMenuForm, SrfEditEvalEntityPopupMenuForm,
    SrfEditPrimitivesPopupMenuForm, SrfEditReversePopupMenuForm,
    SrfEditTrimSrfPopupMenuForm;

static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData);
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID CallData);

static BOOL CALLBACK SrfEditParamFormDlgProc(HWND hDlg,
					     UINT uMsg,
					     WPARAM wParam,
					     LPARAM lParam);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main Srf Edit Param window			   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateSrfEditParam                                                       M
*****************************************************************************/
void CreateSrfEditParam(HWND IGTopLevel)
{
    int i;

    /* SrfEditParam is child of IGTopLevel. */
    SrfEditForm = CreateDialog(GetWindowInstance(IGTopLevel),
			       MAKEINTRESOURCE(IDSE_SRF_EDIT_PARAM_FORM),
			       IGTopLevel,
			       SrfEditParamFormDlgProc);

    SrfEditStatePopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditStateEntries[i] != NULL; i++)
	AppendMenu(SrfEditStatePopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditStateEntries[i]);

    SrfEditSubdivPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditSubdivEntries[i] != NULL; i++)
	AppendMenu(SrfEditSubdivPopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditSubdivEntries[i]);

    SrfEditTrimSrfPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditTrimSrfEntries[i] != NULL; i++)
	AppendMenu(SrfEditTrimSrfPopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditTrimSrfEntries[i]);

    SrfEditRefinePopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditRefineEntries[i] != NULL; i++)
	AppendMenu(SrfEditRefinePopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditRefineEntries[i]);

    SrfEditEndCondPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditEndCondEntries[i] != NULL; i++)
	AppendMenu(SrfEditEndCondPopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditEndCondEntries[i]);

    SrfEditParamPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditParamEntries[i] != NULL; i++)
	AppendMenu(SrfEditParamPopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditParamEntries[i]);

    SrfEditPrimitivesPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditPrimitivesEntries[i] != NULL; i++)
	AppendMenu(SrfEditPrimitivesPopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditPrimitivesEntries[i]);

    SrfEditReversePopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditReverseSrfEntries[i] != NULL; i++)
	AppendMenu(SrfEditReversePopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditReverseSrfEntries[i]);

    SrfEditEvalEntityPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGSrfEditEvalEntityEntries[i] != NULL; i++)
	AppendMenu(SrfEditEvalEntityPopupMenuForm, MF_STRING, i + 1,
		   IGSrfEditEvalEntityEntries[i]);

    UMRSrfEditScaleForm = GetDlgItem(SrfEditForm, IDSE_MRU_TRACKBAR_SCALE);
    SendMessage(UMRSrfEditScaleForm, SBM_SETRANGE, 0, 100);
    VMRSrfEditScaleForm = GetDlgItem(SrfEditForm, IDSE_MRV_TRACKBAR_SCALE);
    SendMessage(VMRSrfEditScaleForm, SBM_SETRANGE, 0, 100);

    IGSrfEditParamUpdateWidget();
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
    SetDlgItemText(SrfEditForm, IDSE_MESSAGE, Msg);
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
    LONG
	Value = SendMessage(Scale, SBM_GETPOS, 0, 0);

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
	case SB_TOP:
	    Value = 0;
	    break;
	case SB_BOTTOM:
	    Value = 100;
	    break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	    Value = HIWORD(wParam);
	    break;
    }

    SendMessage(Scale, SBM_SETPOS, (short) Value, TRUE);

    if (Scale == UMRSrfEditScaleForm) {		  /* Multiresolution slider. */
	Value = IRIT_BOUND(Value, 0, 100);
	IGSrfEditMRULevel = Value / 100.0;
	IGRedrawViewWindow();            /* Update the UMR region displayed. */
    }
    else if (Scale == VMRSrfEditScaleForm) {	  /* Multiresolution slider. */
	Value = IRIT_BOUND(Value, 0, 100);
	IGSrfEditMRVLevel = Value / 100.0;
	IGRedrawViewWindow();            /* Update the VMR region displayed. */
    }
    else
	return;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of all buttons. Invokes the proper action.            *
*                                                                            *
* PARAMETERS:                                                                *
*   w:            Of button to handle.                                       *
*   ClientData:   To get the Name of button.                                 *
*   CallData:     To get the strings in the text widgets.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID CallData)
{
    int i;
    char Str[IRIT_LINE_LEN],	*p;
    DWORD 
	Name = (DWORD) ClientData;
    POINT Point;

    switch (Name) {
	case IDSE_CLEAR:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_CLEAR, 0);
	    break;
	case IDSE_DISMISS:
	case IDCANCEL:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_STATE,
				      IG_SRF_EDIT_STATE_DETACH);
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_DISMISS, 0);
	    ShowWindow(SrfEditForm, SW_HIDE);
	    break;
	case IDSE_DRAW_MESH:
	    IGSrfEditDrawMesh = !IGSrfEditDrawMesh;
	    CheckDlgButton(SrfEditForm, IDSE_DRAW_MESH,
			   IGSrfEditDrawMesh ? BST_CHECKED : BST_UNCHECKED);
	    IGRedrawViewWindow();
	    break;
	case IDSE_DRAW_ORIG:
	    IGSrfDrawOriginal = !IGSrfDrawOriginal;
	    CheckDlgButton(SrfEditForm, IDSE_DRAW_ORIG,
			   IGSrfDrawOriginal ? BST_CHECKED : BST_UNCHECKED);
	    if (IGSrfEditCurrentObj == NULL)
		break;
	    IGActiveFreePolyIsoAttribute(IGSrfEditCurrentObj,
					 TRUE, TRUE, TRUE, TRUE);
	    if (IGSrfDrawOriginal)
		SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj, IGSrfOriginalSurface)
	    else
		SRF_EDIT_SET_SRF_OBJ(IGSrfEditCurrentObj, NULL);
	    IGRedrawViewWindow();
	    break;
	case IDSE_MODIFY_NORMAL_DIR:
	    IGSrfEditNormalDir = !IGSrfEditNormalDir;
	    CheckDlgButton(SrfEditForm, IDSE_MODIFY_NORMAL_DIR,
			   IGSrfEditNormalDir ? BST_CHECKED : BST_UNCHECKED);
	    break;
	case IDSE_STATE:
	    Point.x = 100;
	    Point.y = 0;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(SrfEditStatePopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;
	    switch (--i) {
		case IG_SRF_EDIT_STATE_PRIMITIVES:
	            if ((i = TrackPopupMenu(SrfEditPrimitivesPopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		        break;
		    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_PRIMITIVES,
					      --i);
	            break;
		default:
		    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_STATE, i);
		    SetDlgItemText(SrfEditForm, IDSE_STATE,
			   IGSrfEditStateEntries[IGSrfEditParam.SrfState]);
		    break;
	    }
	    break;
	case IDSE_SUBDIV:
	    Point.x = 50;
	    Point.y = 175;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(SrfEditSubdivPopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SUBDIV, --i);
	    break;
	case IDSE_REFINE:
	    Point.x = 125;
	    Point.y = 175;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(SrfEditRefinePopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REFINE, --i);
	    break;
	case IDSE_REGION:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REGION, 0);
	    break;
	case IDSE_U_DRAISE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_DRAISE, 0);
	    break;
	case IDSE_V_DRAISE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_DRAISE, 0);
	    break;
	case IDSE_U_ORDER:
	    if ((i = GetDlgItemText(SrfEditForm, IDSE_U_ORDER,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i < 999) {
		    IGSrfEditParam.UOrder = i;
		    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_ORDER, i);
		}
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%2d", IGSrfEditParam.UOrder);
		    SetDlgItemText(SrfEditForm, IDSE_U_ORDER, Line);
		}
	    }
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_ORDER, 0);
	    break;
	case IDSE_V_ORDER:
	    if ((i = GetDlgItemText(SrfEditForm, IDSE_V_ORDER,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i < 999) {
		    IGSrfEditParam.VOrder = i;
		    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_ORDER, i);
		}
		else {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%2d", IGSrfEditParam.VOrder);
		    SetDlgItemText(SrfEditForm, IDSE_V_ORDER, Line);
		}
	    }
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_ORDER, 0);
	    break;
	case IDSE_NAME:
	    if ((i = GetDlgItemText(SrfEditForm, IDSE_NAME,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (IGSrfEditCurrentObj != NULL) {
		    IP_SET_OBJ_NAME2(IGSrfEditCurrentObj, Str);
		}
	    }
	    break;
	case IDSE_U_END_COND:
	    Point.x = 150;
	    Point.y = 100;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(SrfEditEndCondPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;
	    
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_END_COND, --i);

	    switch (IGSrfEditParam.UEndCond) {
		case CAGD_END_COND_OPEN:
		    p = "U Open EC";
		    break;
		case CAGD_END_COND_FLOAT:
		    p = "U Float EC";
		    break;
		case CAGD_END_COND_PERIODIC:
		    p = "U Periodic EC";
		    break;
		default:
		    p = "Error";
		    break;
	    };
	    SetDlgItemText(SrfEditForm, IDSE_U_END_COND, p);
	    break;
	case IDSE_V_END_COND:
	    Point.x = 200;
	    Point.y = 100;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(SrfEditEndCondPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;
	    
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_END_COND, --i);

	    switch (IGSrfEditParam.VEndCond) {
		case CAGD_END_COND_OPEN:
		    p = "V Open EC";
		    break;
		case CAGD_END_COND_FLOAT:
		    p = "V Float EC";
		    break;
		case CAGD_END_COND_PERIODIC:
		    p = "V Periodic EC";
		    break;
		default:
		    p = "Error";
		    break;
	    };
	    SetDlgItemText(SrfEditForm, IDSE_V_END_COND, p);
	    break;
	case IDSE_RATIONAL:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_RATIONAL, 0);
	    SetDlgItemText(SrfEditForm, IDSE_RATIONAL,
			   IGSrfEditParam.Rational ? "Rational"
			                           : "Polynomial");
	    break;
	case IDSE_SRF_TYPE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SRF_TYPE, 0);
	    SetDlgItemText(SrfEditForm, IDSE_SRF_TYPE,
			   IGSrfEditParam.Type == CAGD_SBEZIER_TYPE ? "Bezier"
								  : "Bspline");
	    break;
	case IDSE_U_PARAM_TYPE:
	    Point.x = 150;
	    Point.y = 125;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(SrfEditParamPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_U_PARAM_TYPE, --i);

	    switch (IGSrfEditParam.UParamType) {
	        case CAGD_UNIFORM_PARAM:
		    p = "U Uniform KV";
		    break;
	        case CAGD_CENTRIPETAL_PARAM:
		    p = "U Centripetal KV";
		    break;
	        case CAGD_CHORD_LEN_PARAM:
		    p = "U Chord Len KV";
		    break;
		default:
		    p = "Error";
	    }
	    SetDlgItemText(SrfEditForm, IDSE_U_PARAM_TYPE, p);
	    break;
	case IDSE_V_PARAM_TYPE:
	    Point.x = 200;
	    Point.y = 125;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(SrfEditParamPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_V_PARAM_TYPE, --i);

	    switch (IGSrfEditParam.VParamType) {
	        case CAGD_UNIFORM_PARAM:
		    p = "V Uniform KV";
		    break;
	        case CAGD_CENTRIPETAL_PARAM:
		    p = "V Centripetal KV";
		    break;
	        case CAGD_CHORD_LEN_PARAM:
		    p = "V Chord Len KV";
		    break;
		default:
		    p = "Error";
	    }
	    SetDlgItemText(SrfEditForm, IDSE_V_PARAM_TYPE, p);
	    break;
	case IDSE_MOVE_CTLPTS:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_MOVE_CTLPTS, 0);
	    break;
	case IDSE_MODIFY_SRF:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_MODIFY_SRF, 0);
	    break;
	case IDSE_SMERGE:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_MERGE_SRFS, 0);
	    break;
	case IDSE_REVERSE_SRF:
	    Point.x = 50;
	    Point.y = 220;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(SrfEditReversePopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REVERSE, --i);
	    break;
	case IDSE_TRIM_SRF:
	    Point.x = 125;
	    Point.y = 230;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(SrfEditTrimSrfPopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_TRIM, --i);
	    break;
	case IDSE_EVALUATE_SRF:
	    Point.x = 200;
	    Point.y = 200;
	    ClientToScreen(SrfEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(SrfEditEvalEntityPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, SrfEditForm, NULL)) == 0)
		break;

	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_EVALUATE, --i);
	    break;
	case IDSE_SAVE_SRF:
	    if (IGSrfEditCurrentObj != NULL) {
		OPENFILENAME ofn;
		char FileName[IRIT_LINE_LEN_LONG];

		strcpy(FileName, "*.itd");
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = SrfEditForm;
		ofn.lpstrFile = FileName;
		ofn.lpstrFileTitle = FileName;
		ofn.nMaxFile = IRIT_LINE_LEN_LONG;
		ofn.lpstrTitle = "Save Surface as IRIT dat file";
		ofn.lpstrFilter = "*.itd";
		ofn.Flags = OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn) && !IRT_STR_ZERO_LEN(FileName)) {
		    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SAVE_SRF,
					      (int) FileName);
		}
		else
		    IGCrvEditPlaceMessage("Failed to pick surface's name");
	    }
	    else
		IGSrfEditPlaceMessage("No surface under editing to save");
	    break;
	case IDSE_SUBMIT_SRF:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_SUBMIT_SRF, 0);
	    break;
	case IDSE_MRU_TRACKBAR_SCALE:
	    break;
	case IDSE_MRV_TRACKBAR_SCALE:
	    break;
	case IDSE_UNDO:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_UNDO, 0);
	    break;
	case IDSE_REDO:
	    SEditHandleNonMouseEvents(IG_SRF_EDIT_EVENT_REDO, 0);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Getting input values from the user.					     M
*									     *
* PARAMETERS:								     M
*   None                                 				     M
*									     *
* RETURN VALUE:								     M
*   void 								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfEditCB		                                                     M
*****************************************************************************/
void SrfEditCB(void)
{  
    RECT Wr, Dr, Cr;

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(SrfEditForm, &Cr);		           /* Get form size. */
    SetWindowPos(SrfEditForm, 
		 HWND_TOP,
		 min(Wr.right + 3, Dr.right - Cr.right - 3),
		 Wr.top,
		 0, 
		 0,
		 SWP_NOSIZE | SWP_SHOWWINDOW);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the MR scale widget.			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSrfEditParamUpdateMRScale                                              M
*****************************************************************************/
void IGSrfEditParamUpdateMRScale(void) 
{
    SendMessage(UMRSrfEditScaleForm, SBM_SETPOS,
		(short) (IGSrfEditMRULevel * 100.0), TRUE);
    SendMessage(VMRSrfEditScaleForm, SBM_SETPOS,
		(short) (IGSrfEditMRVLevel * 100.0), TRUE);
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
    char Line[IRIT_LINE_LEN], *p;

    if (IGSrfEditCurrentObj != NULL)
	SetDlgItemText(SrfEditForm, IDSE_NAME,
		       IP_GET_OBJ_NAME(IGSrfEditCurrentObj));
    else
	SetDlgItemText(SrfEditForm, IDSE_NAME, "");

    CheckDlgButton(SrfEditForm, IDSE_DRAW_MESH,
		   IGSrfEditDrawMesh ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(SrfEditForm, IDSE_DRAW_ORIG,
		   IGSrfDrawOriginal ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(SrfEditForm, IDSE_MODIFY_NORMAL_DIR,
		   IGSrfEditNormalDir ? BST_CHECKED : BST_UNCHECKED);

    sprintf(Line, "%2d", IGSrfEditParam.UOrder);
    SetDlgItemText(SrfEditForm, IDSE_U_ORDER, Line);
    sprintf(Line, "%2d", IGSrfEditParam.VOrder);
    SetDlgItemText(SrfEditForm, IDSE_V_ORDER, Line);

    SetDlgItemText(SrfEditForm, IDSE_RATIONAL,
		   IGSrfEditParam.Rational ? "Rational" : "Polynomial");

    SetDlgItemText(SrfEditForm, IDSE_SRF_TYPE,
		   IGSrfEditParam.Type == CAGD_SBEZIER_TYPE ? "Bezier"
							    : "Bspline");

    SendMessage(UMRSrfEditScaleForm, SBM_SETPOS,
		(short) (IGSrfEditMRULevel * 100.0), TRUE);
    SendMessage(VMRSrfEditScaleForm, SBM_SETPOS,
		(short) (IGSrfEditMRVLevel * 100.0), TRUE);

    SetDlgItemText(SrfEditForm, IDSE_STATE,
		   IGSrfEditStateEntries[IGSrfEditParam.SrfState]);

    switch (IGSrfEditParam.UParamType) {
        case CAGD_UNIFORM_PARAM:
	    p = "U Uniform KV";
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    p = "U Centripetal KV";
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    p = "U Chord Len KV";
	    break;
	default:
	    p = NULL;
    }
    if (p != NULL)
	SetDlgItemText(SrfEditForm, IDSE_U_PARAM_TYPE, p);
    switch (IGSrfEditParam.VParamType) {
        case CAGD_UNIFORM_PARAM:
	    p = "V Uniform KV";
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    p = "V Centripetal KV";
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    p = "V Chord Len KV";
	    break;
	default:
	    p = NULL;
    }
    if (p != NULL)
	SetDlgItemText(SrfEditForm, IDSE_V_PARAM_TYPE, p);
    
    switch (IGSrfEditParam.UEndCond) {
	case CAGD_END_COND_OPEN:
	    p = "U Open EC";
	    break;
	case CAGD_END_COND_FLOAT:
	    p = "U Float EC";
	    break;
	case CAGD_END_COND_PERIODIC:
	    p = "U Periodic EC";
	    break;
	default:
	    p = NULL;
    };
    if (p != NULL)
	SetDlgItemText(SrfEditForm, IDSE_U_END_COND, p);

    switch (IGSrfEditParam.VEndCond) {
	case CAGD_END_COND_OPEN:
	    p = "V Open EC";
	    break;
	case CAGD_END_COND_FLOAT:
	    p = "V Float EC";
	    break;
	case CAGD_END_COND_PERIODIC:
	    p = "V Periodic EC";
	    break;
	default:
	    p = NULL;
    };
    if (p != NULL)
	SetDlgItemText(SrfEditForm, IDSE_V_END_COND, p);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Win32 dialog callback function of the srf edit dialog.                 *
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
static BOOL CALLBACK SrfEditParamFormDlgProc(HWND hDlg,
					     UINT uMsg,
					     WPARAM wParam,
					     LPARAM lParam)
{
    HWND hWnd;
    PVOID clientId;
        
    switch (uMsg) {
	case WM_INITDIALOG:
            return TRUE;
	case WM_DESTROY:
	    return FALSE;
	case WM_COMMAND:
            hWnd = (HWND) lParam;                 /* Handle of control HWND. */
	    if (((HWND) lParam) != UMRSrfEditScaleForm &&
		((HWND) lParam) != VMRSrfEditScaleForm) {
	        clientId = (PVOID) LOWORD(wParam);        /* ID of the item. */
		ButtonsCallBack(hWnd, clientId, NULL);
	    }
	    return TRUE;
	case WM_HSCROLL:
            hWnd = (HWND) lParam;                 /* Handle of control HWND. */
	    if (((HWND) lParam) == UMRSrfEditScaleForm ||
		((HWND) lParam) == VMRSrfEditScaleForm) {
		ScaleCB(hWnd, wParam, NULL);    
	    }
	    return TRUE;
	default:
	    return FALSE;
    }
}
