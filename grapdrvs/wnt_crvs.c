/*****************************************************************************
*  A windows NT interface for crv edit param. (based on motif interface).    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 0.1, Sept. 1997.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"
#include "editcrvs.h"
#include "cnstcrvs.h"

IRIT_STATIC_DATA int
    CEditDrawMesh = TRUE;
IRIT_STATIC_DATA HWND CrvEditForm, MRCrvEditScaleForm;
IRIT_STATIC_DATA HMENU CrvEditStatePopupMenuForm, CrvEditSubdivPopupMenuForm,
    CrvEditRefinePopupMenuForm, CrvEditEndCondPopupMenuForm,
    CrvEditParamPopupMenuForm, CrvEditEvalEntityPopupMenuForm,
    CrvEditPrimitivesPopupMenuForm;

IRIT_STATIC_DATA HWND CrvConstraintsForm;
IRIT_STATIC_DATA HMENU CrvCnstAddPopupMenuForm;

static void CrvEditScaleCB(HWND Scale, WPARAM wParam, PVOID CallData);
static void CrvEditButtonsCB(HWND hWnd,
			    PVOID ClientData,
			    PVOID CallData);
static void CrvCnstButtonsCB(HWND hWnd,
			    PVOID ClientData,
			    PVOID CallData);
static BOOL CALLBACK CrvEditParamFormDlgProc(HWND hDlg,
					     UINT uMsg,
					     WPARAM wParam,
					     LPARAM lParam);
static BOOL CALLBACK CrvCnstParamFormDlgProc(HWND hDlg,
					     UINT uMsg,
					     WPARAM wParam,
					     LPARAM lParam);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main Crv Edit Param window			   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateCrvEditParam                                                       M
*****************************************************************************/
void CreateCrvEditParam(HWND IGTopLevel)
{
    int i;

    /* CrvEditParam is child of IGTopLevel. */
    CrvEditForm = CreateDialog(GetWindowInstance(IGTopLevel),
			       MAKEINTRESOURCE(IDCE_CRV_EDIT_PARAM_FORM),
			       IGTopLevel,
			       CrvEditParamFormDlgProc);

    CrvEditStatePopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditStateEntries[i] != NULL; i++)
	AppendMenu(CrvEditStatePopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditStateEntries[i]);

    CrvEditSubdivPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditSubdivEntries[i] != NULL; i++)
	AppendMenu(CrvEditSubdivPopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditSubdivEntries[i]);

    CrvEditRefinePopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditRefineEntries[i] != NULL; i++)
	AppendMenu(CrvEditRefinePopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditRefineEntries[i]);

    CrvEditEndCondPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditEndCondEntries[i] != NULL; i++)
	AppendMenu(CrvEditEndCondPopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditEndCondEntries[i]);

    CrvEditParamPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditParamEntries[i] != NULL; i++)
	AppendMenu(CrvEditParamPopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditParamEntries[i]);

    CrvEditPrimitivesPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditPrimitivesEntries[i] != NULL; i++)
	AppendMenu(CrvEditPrimitivesPopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditPrimitivesEntries[i]);

    CrvEditEvalEntityPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvEditEvalEntityEntries[i] != NULL; i++)
	AppendMenu(CrvEditEvalEntityPopupMenuForm, MF_STRING, i + 1,
		   IGCrvEditEvalEntityEntries[i]);

    MRCrvEditScaleForm = GetDlgItem(CrvEditForm, IDCE_MR_TRACKBAR_SCALE);
    SendMessage(MRCrvEditScaleForm, SBM_SETRANGE, 0, 100);

        /* CrvEditParam is child of IGTopLevel. */
    CrvConstraintsForm = CreateDialog(GetWindowInstance(IGTopLevel),
			       MAKEINTRESOURCE(IDCC_CRV_CNST_PARAM_FORM),
			       IGTopLevel,
			       CrvCnstParamFormDlgProc);

    CrvCnstAddPopupMenuForm = CreatePopupMenu();
    for (i = 0; IGCrvCnstAddEntries[i] != NULL; i++)
	AppendMenu(CrvCnstAddPopupMenuForm, MF_STRING, i + 1,
		   IGCrvCnstAddEntries[i]);

    IGCrvEditParamUpdateWidget();
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
    SetDlgItemText(CrvEditForm, IDCE_MESSAGE, Msg);
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
static void CrvEditScaleCB(HWND Scale, WPARAM wParam, PVOID CallData)
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

    if (Scale == MRCrvEditScaleForm) {		  /* Multiresolution slider. */
	Value = IRIT_BOUND(Value, 0, 100);
	IGCrvEditMRLevel = Value / 100.0;
	IGRedrawViewWindow();             /* Update the MR region displayed. */
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
static void CrvEditButtonsCB(HWND hWnd,
			     PVOID ClientData,
			     PVOID CallData)
{
    int i;
    char Str[IRIT_LINE_LEN],	*p;
    DWORD 
	Name = (DWORD) ClientData;
    POINT Point;
    BOOL Rc;
    RECT Wr, Dr, Cr;

    switch (Name) {
	case IDCE_CLEAR:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_CLEAR, 0);
	    break;
	case IDCE_CONSTRAINTS:
	    GetWindowRect(GetDesktopWindow(), &Dr); 
	    GetWindowRect(CrvEditForm, &Wr);    
	    GetClientRect(CrvConstraintsForm, &Cr);        /* Get form size. */
	    if (!IGCrvEditParam.SupportConstraints)
		CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DO_CONSTRAINTS, 0);
	    Rc = SetWindowPos(CrvConstraintsForm, 
			      HWND_TOP,
			      Wr.left,
			      min(Wr.bottom + 3, Dr.bottom - Cr.bottom - 3),
			      0, 
			      0,
			      SWP_NOSIZE | SWP_SHOWWINDOW);
	    break;
	case IDCE_DISMISS:
	case IDCANCEL:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DISMISS, 0);
	    ShowWindow(CrvConstraintsForm, SW_HIDE);

	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_STATE,
				      IG_CRV_EDIT_STATE_DETACH);
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_DISMISS, 0);
	    ShowWindow(CrvEditForm, SW_HIDE);
	    break;
	case IDCE_DRAW_MESH:
	    IGCrvEditDrawMesh = !IGCrvEditDrawMesh;
	    CheckDlgButton(CrvEditForm, IDCE_DRAW_MESH,
			   IGCrvEditDrawMesh ? BST_CHECKED : BST_UNCHECKED);
	    IGRedrawViewWindow();
	    break;
	case IDCE_DRAW_ORIG:
	    IGCrvDrawOriginal = !IGCrvDrawOriginal;
	    CheckDlgButton(CrvEditForm, IDCE_DRAW_ORIG,
			   IGCrvDrawOriginal ? BST_CHECKED : BST_UNCHECKED);
	    if (IGCrvEditCurrentObj == NULL)
		break;
	    IGActiveFreePolyIsoAttribute(IGCrvEditCurrentObj,
					 FALSE, TRUE, FALSE, TRUE);
	    if (IGCrvDrawOriginal)
		IGCrvEditCurrentObj -> U.Crvs = IGCrvOriginalCurve;
	    else
		IGCrvEditCurrentObj -> U.Crvs = NULL;
	    IGRedrawViewWindow();
	    break;
	case IDCE_STATE:
	    Point.x = 150;
	    Point.y = 0;
	    ClientToScreen(CrvEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(CrvEditStatePopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		break;
	    switch (--i) {
		case IG_CRV_EDIT_STATE_PRIMITIVES:
	            if ((i = TrackPopupMenu(CrvEditPrimitivesPopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		        break;
		    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_PRIMITIVES,
					      --i);
	            break;
		default:
		    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_STATE, i);
		    SetDlgItemText(CrvEditForm, IDCE_STATE,
			     IGCrvEditStateEntries[IGCrvEditParam.CrvState]);
	    }
	    break;
	case IDCE_SUBDIV:
	    Point.x = 50;
	    Point.y = 100;
	    ClientToScreen(CrvEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(CrvEditSubdivPopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		break;

	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_SUBDIV, --i);
	    break;
	case IDCE_REFINE:
	    Point.x = 125;
	    Point.y = 125;
	    ClientToScreen(CrvEditForm, &Point);

	    /* Activate the pop up menu. */
	    if ((i = TrackPopupMenu(CrvEditRefinePopupMenuForm,
			     TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			     Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		break;

	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REFINE, --i);
	    break;
	case IDCE_REGION:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REGION, 0);
	    break;
	case IDCE_DRAISE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_DRAISE, 0);
	    break;
	case IDCE_CMERGE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_MERGE_CURVES, 0);
	    break;
	case IDCE_ORDER:
	    if ((i = GetDlgItemText(CrvEditForm, IDCE_ORDER,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i < 999) {
		    IGCrvEditParam.Order = i;
		    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_ORDER, i);
		}
		else {
		    sprintf(Str, "%2d", IGCrvEditParam.Order);
		    SetDlgItemText(CrvEditForm, IDCE_ORDER, Str);
		}
	    }
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_ORDER, 0);
	    break;
	case IDCE_NAME:
	    if ((i = GetDlgItemText(CrvEditForm, IDCE_NAME,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (IGCrvEditCurrentObj != NULL) {
		    IP_SET_OBJ_NAME2(IGCrvEditCurrentObj, Str);
		}
	    }
	    break;
	case IDCE_END_COND:
	    Point.x = 150;
	    Point.y = 50;
	    ClientToScreen(CrvEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(CrvEditEndCondPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		break;
	    
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_END_COND, --i);

	    switch (IGCrvEditParam.EndCond) {
		case CAGD_END_COND_OPEN:
		    p = "Open EC";
		    break;
		case CAGD_END_COND_FLOAT:
		    p = "Float EC";
		    break;
		case CAGD_END_COND_PERIODIC:
		    p = "Periodic EC";
		    break;
		default:
		    p = "Error";
		    break;
	    };
	    SetDlgItemText(CrvEditForm, IDCE_END_COND, p);
	    break;
	case IDCE_RATIONAL:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_RATIONAL, 0);
	    SetDlgItemText(CrvEditForm, IDCE_RATIONAL,
			   IGCrvEditParam.Rational ? "Rational"
			                           : "Polynomial");
	    break;
	case IDCE_CURVE_TYPE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_CURVE_TYPE, 0);
	    SetDlgItemText(CrvEditForm, IDCE_CURVE_TYPE,
			   IGCrvEditParam.Type == CAGD_CBEZIER_TYPE ? "Bezier"
								  : "Bspline");
	    break;
	case IDCE_LST_SQR_PERCENT:
	    if ((i = GetDlgItemText(CrvEditForm, IDCE_LST_SQR_PERCENT,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%d", &i) == 1 && i > 0 && i <= 100)
		    IGCrvEditParam.LstSqrPercent = i;
		else {
		    sprintf(Str, "%2d", IGCrvEditParam.LstSqrPercent);
		    SetDlgItemText(CrvEditForm, IDCE_LST_SQR_PERCENT, Str);
		}
	    }
	    break;
	case IDCE_PARAM_TYPE:
	    Point.x = 150;
	    Point.y = 100;
	    ClientToScreen(CrvEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(CrvEditParamPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		break;

	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_PARAM_TYPE, --i);

	    switch (IGCrvEditParam.ParamType) {
	        case CAGD_GENERAL_PARAM:
		    p = "General KV";
		    break;
	        case CAGD_UNIFORM_PARAM:
		    p = "Uniform KV";
		    break;
	        case CAGD_CENTRIPETAL_PARAM:
		    p = "Centripetal KV";
		    break;
	        case CAGD_CHORD_LEN_PARAM:
		    p = "Chord Len KV";
		    break;
		default:
		    p = "Error";
	    }
	    SetDlgItemText(CrvEditForm, IDCE_PARAM_TYPE, p);
	    break;
	case IDCE_MOVE_CTLPTS:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_MOVE_CTLPTS, 0);
	    break;
	case IDCE_DELETE_CTLPTS:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_DELETE_CTLPTS, 0);
	    break;
	case IDCE_MODIFY_CURVE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_MODIFY_CURVE, 0);
	    break;
	case IDCE_REVERSE_CURVE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REVERSE, 0);
	    break;
	case IDCE_EVALUATE_CURVE:
	    Point.x = 220;
	    Point.y = 200;
	    ClientToScreen(CrvEditForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(CrvEditEvalEntityPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, CrvEditForm, NULL)) == 0)
		break;

	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_EVALUATE, --i);
	    break;
	case IDCE_SAVE_CURVE:
	    if (IGCrvEditCurrentObj != NULL) {
		OPENFILENAME ofn;
		char FileName[IRIT_LINE_LEN_LONG];

		strcpy(FileName, "*.itd");
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = CrvEditForm;
		ofn.lpstrFile = FileName;
		ofn.lpstrFileTitle = FileName;
		ofn.nMaxFile = IRIT_LINE_LEN_LONG;
		ofn.lpstrTitle = "Save Curve as IRIT dat file";
		ofn.lpstrFilter = "*.itd";
		ofn.Flags = OFN_OVERWRITEPROMPT;

		if (GetSaveFileName(&ofn) && !IRT_STR_ZERO_LEN(FileName)) {
		    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_SAVE_CURVE,
					      (int) FileName);
		}
		else
		    IGCrvEditPlaceMessage("Failed to pick curve's name");
	    }
	    else
		IGCrvEditPlaceMessage("No curve under editing to save");
	    break;
	case IDCE_SUBMIT_CURVE:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_SUBMIT_CURVE, 0);
	    break;
	case IDCE_MR_TRACKBAR_SCALE:
	    break;
	case IDCE_UNDO:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_UNDO, 0);
	    break;
	case IDCE_REDO:
	    CEditHandleNonMouseEvents(IG_CRV_EDIT_EVENT_REDO, 0);
	    break;
    }
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
static void CrvCnstButtonsCB(HWND hWnd,
			     PVOID ClientData,
			     PVOID CallData)
{
    int i;
    char Str[IRIT_LINE_LEN];
    DWORD 
	Name = (DWORD) ClientData;
    POINT Point;

    switch (Name) {
	case IDCC_DISMISS:
	case IDCANCEL:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DISMISS, 0);
	    ShowWindow(CrvConstraintsForm, SW_HIDE);
	    break;
	case IDCC_DELETE_CNST:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DELETE, 0);
	    break;
	case IDCC_ADD_CNST:
	    Point.x = 50;
	    Point.y = 150;
	    ClientToScreen(CrvConstraintsForm, &Point);

	    /* Activate the pop up menu. Events goes to WM_COMMAND. */
	    if ((i = TrackPopupMenu(CrvCnstAddPopupMenuForm,
			   TPM_CENTERALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
			   Point.x, Point.y, 0, CrvConstraintsForm,
			   NULL)) == 0)
		break;
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_ADD, --i);
	    break;
        case IDCC_CONSTRAINTS:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_DO_CONSTRAINTS, 0);
	    break;
        case IDCC_SATISFY_ALL:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_SATISFY_ALL, 0);
	    break;
        case IDCC_X_SYMMETRY:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_X_SYMMETRY, 0);
	    break;
        case IDCC_Y_SYMMETRY:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_Y_SYMMETRY, 0);
	    break;
        case IDCC_C_SYMMETRY:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_C_SYMMETRY, 0);
	    break;
        case IDCC_AREA:
	    CCnstHandleNonMouseEvents(IG_CRV_CNST_EVENT_AREA, 0);
	    break;
        case IDCC_UNDULATE:
	    if ((i = GetDlgItemText(CrvConstraintsForm, IDCC_UNDULATE,
				    Str, IRIT_LINE_LEN - 1)) != 0) {
		if (sscanf(Str, "%d", &i) == 1 && i > 0) {
		    IGCrvEditParam.CnstMaxAllowedCoef = i;
		}
		else {
		    sprintf(Str, "%d",
			    (int) IGCrvEditParam.CnstMaxAllowedCoef);
		    SetDlgItemText(CrvEditForm, IDCC_UNDULATE, Str);
		}
	    }
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
*   CrvEditCB		                                                     M
*****************************************************************************/
void CrvEditCB(void)
{  
    RECT Wr, Dr, Cr;

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(CrvEditForm, &Cr);		           /* Get form size. */
    SetWindowPos(CrvEditForm, 
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
*   IGCrvEditParamUpdateMRScale                                              M
*****************************************************************************/
void IGCrvEditParamUpdateMRScale(void) 
{
    SendMessage(MRCrvEditScaleForm, SBM_SETPOS,
		(short) (IGCrvEditMRLevel * 100.0), TRUE);
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
    char Line[IRIT_LINE_LEN], *p;

    if (IGCrvEditCurrentObj != NULL)
	SetDlgItemText(CrvEditForm, IDCE_NAME,
		       IP_GET_OBJ_NAME(IGCrvEditCurrentObj));
    else
	SetDlgItemText(CrvEditForm, IDCE_NAME, "");

    CheckDlgButton(CrvEditForm, IDCE_DRAW_MESH,
		   IGCrvEditDrawMesh ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(CrvEditForm, IDCE_DRAW_ORIG,
		   IGCrvDrawOriginal ? BST_CHECKED : BST_UNCHECKED);

    sprintf(Line, "%3d", IGCrvEditParam.LstSqrPercent);
    SetDlgItemText(CrvEditForm, IDCE_LST_SQR_PERCENT, Line);
    sprintf(Line, "%2d", IGCrvEditParam.Order);
    SetDlgItemText(CrvEditForm, IDCE_ORDER, Line);

    SetDlgItemText(CrvEditForm, IDCE_RATIONAL,
		   IGCrvEditParam.Rational ? "Rational" : "Polynomial");

    SetDlgItemText(CrvEditForm, IDCE_CURVE_TYPE,
		   IGCrvEditParam.Type == CAGD_CBEZIER_TYPE ? "Bezier"
							    : "Bspline");

    SendMessage(MRCrvEditScaleForm, SBM_SETPOS,
		(short) (IGCrvEditMRLevel * 100.0), TRUE);

    SetDlgItemText(CrvEditForm, IDCE_STATE,
		   IGCrvEditStateEntries[IGCrvEditParam.CrvState]);

    switch (IGCrvEditParam.ParamType) {
	case CAGD_GENERAL_PARAM:
	    p = "General KV";
	    break;
        case CAGD_UNIFORM_PARAM:
	    p = "Uniform KV";
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    p = "Centripetal KV";
	    break;
	case CAGD_CHORD_LEN_PARAM:
	    p = "Chord Len KV";
	    break;
	default:
	    p = NULL;
    }
    if (p != NULL)
	SetDlgItemText(CrvEditForm, IDCE_PARAM_TYPE, p);
    
    switch (IGCrvEditParam.EndCond) {
	case CAGD_END_COND_OPEN:
	    p = "Open EC";
	    break;
	case CAGD_END_COND_FLOAT:
	    p = "Float EC";
	    break;
	case CAGD_END_COND_PERIODIC:
	    p = "Periodic EC";
	    break;
	default:
	    p = NULL;
    };
    if (p != NULL)
	SetDlgItemText(CrvEditForm, IDCE_END_COND, p);

    IGCrvCnstParamUpdateWidget();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the widget with the current curve constraints parameters.        M
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
    int Len = 0;
    char Line[IRIT_LINE_LEN_VLONG],
	*l = Line;
    IGCrvConstraintStruct
	*Cnst = CrvCnstList;

    Line[0] = 0;

    if (IGCrvEditParam.CnstXSymmetry) {
        sprintf(l, "X symmetry Constraint,                           ");
	Len += strlen(l);
	l = &Line[Len];
    }

    if (IGCrvEditParam.CnstYSymmetry) {
	sprintf(l, "Y symmetry Constraint,                           ");
	Len += strlen(l);
	l = &Line[Len];
    }

    if (IGCrvEditParam.CnstCSymmetry) {
        sprintf(l, "C symmetry Constraint,                           ");
	Len += strlen(l);
	l = &Line[Len];
    }

    if (IGCrvEditParam.CnstArea) {
	CagdRType t1, t2, *R;
	CagdCrvStruct *Crv, *TCrv;

	if (CAGD_IS_BSPLINE_CRV(IGCrvEditCurrentCrv)) {
	    if (CAGD_IS_PERIODIC_CRV(IGCrvEditCurrentCrv))
	        Crv = CagdCnvrtPeriodic2FloatCrv(IGCrvEditCurrentCrv);
	    else
	        Crv = CagdCrvCopy(IGCrvEditCurrentCrv);

	    if (!BspCrvHasOpenEC(Crv)) {
	        TCrv = CagdCnvrtFloat2OpenCrv(Crv);
		CagdCrvFree(Crv);
		Crv = TCrv;
	    }
	}
	else
	    Crv = CagdCrvCopy(IGCrvEditCurrentCrv);

	TCrv = SymbCrvEnclosedArea(Crv);
	CagdCrvFree(Crv);

	/* Let the user know the current area. */
	CagdCrvDomain(TCrv, &t1, &t2);
	R = CagdCrvEval(TCrv, t2);
	CagdCrvFree(TCrv);
	sprintf(l, "Area Constraint (Area = %.5f)%s            ",
		IRIT_FABS(R[1]),
		Cnst == NULL ? "" : ", ");

	Len += strlen(l);
	l = &Line[Len];
    }

    for ( ; Cnst != NULL; Cnst = Cnst -> Pnext) {
	switch (Cnst -> Type) {
	    case IG_CRV_CNST_TANGENT:
		sprintf(l, "Tangential Constraint at t = %.5f%s",
			Cnst -> Param,
			Cnst -> Pnext == NULL ? "" : ", ");
		break;
	    case IG_CRV_CNST_POSITION:
		sprintf(l, "Positional Constraint at t = %.5f%s",
			Cnst -> Param,
			Cnst -> Pnext == NULL ? "" : ", ");
		break;
	    case IG_CRV_CNST_CTL_PT:
		sprintf(l, "Cntrl-Point Constraint at Index = %.5f%s",
			Cnst -> Param,
			Cnst -> Pnext == NULL ? "" : ", ");
		break;
	    default:
		sprintf(l, "?????? Constraint at ? ");
		break;
	}
	Len += strlen(l);
	l = &Line[Len];
    }

    SetDlgItemText(CrvConstraintsForm, IDCC_CRV_CNST_TEXT, Line);

    sprintf(Line, "%d", (int) IGCrvEditParam.CnstMaxAllowedCoef);
    SetDlgItemText(CrvConstraintsForm, IDCC_UNDULATE, Line);

    CheckDlgButton(CrvConstraintsForm, IDCC_CONSTRAINTS,
	IGCrvEditParam.SupportConstraints ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(CrvConstraintsForm, IDCC_SATISFY_ALL,
	IGCrvEditParam.AbortIfCnstFailed ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(CrvConstraintsForm, IDCC_X_SYMMETRY,
	IGCrvEditParam.CnstXSymmetry ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(CrvConstraintsForm, IDCC_Y_SYMMETRY,
	IGCrvEditParam.CnstYSymmetry ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(CrvConstraintsForm, IDCC_C_SYMMETRY,
	IGCrvEditParam.CnstCSymmetry ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(CrvConstraintsForm, IDCC_AREA,
	IGCrvEditParam.CnstArea ? BST_CHECKED : BST_UNCHECKED);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Win32 dialog callback function of the crv edit dialog.                 *
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
static BOOL CALLBACK CrvEditParamFormDlgProc(HWND hDlg,
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
	    if (((HWND) lParam) != MRCrvEditScaleForm) {
	        clientId = (PVOID) LOWORD(wParam);        /* ID of the item. */
		CrvEditButtonsCB(hWnd, clientId, NULL);
	    }
	    return TRUE;
	case WM_HSCROLL:
            hWnd = (HWND) lParam;                 /* Handle of control HWND. */
	    if (((HWND) lParam) == MRCrvEditScaleForm) {
		CrvEditScaleCB(hWnd, wParam, NULL);    
	    }
	    return TRUE;
	default:
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Win32 dialog callback function of the crv constraints dialog.          *
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
static BOOL CALLBACK CrvCnstParamFormDlgProc(HWND hDlg,
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
	    clientId = (PVOID) LOWORD(wParam);            /* ID of the item. */
	    CrvCnstButtonsCB(hWnd, clientId, NULL);
	    return TRUE;
	case WM_HSCROLL:
	    return TRUE;
	default:
	    return FALSE;
    }
}
