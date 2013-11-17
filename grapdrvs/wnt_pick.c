/*****************************************************************************
*  A windows NT interface for setting pickable objects.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, Dec. 1998.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"

IRIT_STATIC_DATA int
    GlblPickObjTypes[] = {
	IDPK_PICK_OBJS_POLY,
	IDPK_PICK_OBJS_NUMERIC,
	IDPK_PICK_OBJS_POINT,
	IDPK_PICK_OBJS_VECTOR,
	IDPK_PICK_OBJS_PLANE,
	IDPK_PICK_OBJS_MATRIX,
	IDPK_PICK_OBJS_CURVE,
	IDPK_PICK_OBJS_SURFACE,
	IDPK_PICK_OBJS_STRING,
	IDPK_PICK_OBJS_LIST_OBJ,
	IDPK_PICK_OBJS_CTLPT,
	IDPK_PICK_OBJS_TRIMSRF,
	IDPK_PICK_OBJS_TRIVAR,
	IDPK_PICK_OBJS_INSTANCE,
	IDPK_PICK_OBJS_TRISRF,
	IDPK_PICK_OBJS_MODEL,
	IDPK_PICK_OBJS_MULTIVAR,
	0
    },
    GlblIGObjTypes[] = {
	IG_PICK_POLY,
	IG_PICK_NUMERIC,
	IG_PICK_POINT,
	IG_PICK_VECTOR,
	IG_PICK_PLANE,
	IG_PICK_MATRIX,
	IG_PICK_CURVE,
	IG_PICK_SURFACE,
	IG_PICK_STRING,
	IG_PICK_LIST_OBJ,
	IG_PICK_CTLPT,
	IG_PICK_TRIMSRF,
	IG_PICK_TRIVAR,
	IG_PICK_INSTANCE,
	IG_PICK_TRISRF,
	IG_PICK_MODEL,
	IG_PICK_MULTIVAR,
	0
    };

IRIT_STATIC_DATA HWND PickObjsForm;

static void PickObjsDismissCb(void);
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID *cbs);
static BOOL CALLBACK PickObjsFormDlgProc(HWND hDlg,
					 UINT uMsg,
					 WPARAM wParam,
					 LPARAM lParam);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main PickObjs window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreatePickObjs                                                           M
*****************************************************************************/
void CreatePickObjs(HWND IGTopLevel)
{
    /* PickObjs is child of IGTopLevel. */
    PickObjsForm = CreateDialog(GetWindowInstance(IGTopLevel),
				MAKEINTRESOURCE(IDPK_PICK_OBJS_FORM),
				IGTopLevel,
				PickObjsFormDlgProc);

    PickObjsUpdateCb();
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
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID *cbs)
{
    DWORD 
	Name = (DWORD) ClientData;
    UINT
	Val = IsDlgButtonChecked(PickObjsForm, Name);
    int i;

    switch (Name) {
	case IDPK_PICK_DISMISS:
	case IDCANCEL:
	    PickObjsDismissCb();
	    return;
    }

    /* Update the radio bottons. */
    for (i = 0; GlblPickObjTypes[i] != 0; i++) {
	if (GlblPickObjTypes[i] == (int) Name) {
	    /* Found the button! */
	    if (Val)
		IGGlblPickObjTypes |= GlblIGObjTypes[i];
	    else
		IGGlblPickObjTypes &= ~GlblIGObjTypes[i];
	    PickObjsUpdateCb();
	    return;
	}
    }

    if (IDPK_PICK_SAVE_POLY == (int) Name) {
        IGGlblSavePickedPoly = Val != 0;
	PickObjsUpdateCb();
	return;
    }
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Pop up the pick object selection widget.				     M
*									     *
* PARAMETERS:								     M
*   None                                 				     M
*									     *
* RETURN VALUE:								     M
*   void 								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PickObjsCB   	                                                     M
*****************************************************************************/
void PickObjsCB(void)
{  
    RECT Wr, Dr, Cr;      

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(PickObjsForm, &Cr);			   /* Get form size. */
    SetWindowPos(PickObjsForm, 
		 HWND_TOP,
		 min(Wr.right + 3, Dr.right - Cr.right - 3),
		 Wr.top,
		 0, 
		 0,
		 SWP_NOSIZE | SWP_SHOWWINDOW);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update all buttons/sliders with current state.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PickObjsUpdateCb                                                         M
*****************************************************************************/
void PickObjsUpdateCb(void) 
{
    static int
	LastSavePickedPoly = FALSE;
    int i;

    /* Update the radio bottons. */
    for (i = 0; GlblPickObjTypes[i] != 0; i++) { 
	CheckDlgButton(PickObjsForm, GlblPickObjTypes[i],
		IG_PICK_OBJ(GlblIGObjTypes[i]) ? BST_CHECKED : BST_UNCHECKED);
    }
    CheckDlgButton(PickObjsForm, IDPK_PICK_SAVE_POLY,
		   IGGlblSavePickedPoly ? BST_CHECKED : BST_UNCHECKED);

    if (!LastSavePickedPoly && IGGlblSavePickedPoly) {
	static char FileName[IRIT_LINE_LEN_LONG];
        OPENFILENAME ofn;
	char TmpFileName[IRIT_LINE_LEN_LONG], *Fn, *Ft;

	strcpy(TmpFileName, "");
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = TmpFileName;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFile = IRIT_LINE_LEN_LONG;
	ofn.lpstrTitle = "Save Picked Polygon";
	ofn.lpstrFilter = "";
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn)) {
	    Fn = TmpFileName;
	    if ((Ft = strrchr(TmpFileName, '.')) != NULL) {
	        *Ft = 0;
		Ft++;
	    }
	    else
	        Ft = IRIT_TEXT_DATA_FILE;

	    /* Format the file names as "Fname%04d.Ftype". */ 
	    sprintf(FileName, "%s%s.%s", Fn, "%04d", Ft);
	    IGGlblPolyPickFileName = FileName;
	}
    }
    LastSavePickedPoly = IGGlblSavePickedPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for dismiss button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PickObjsDismissCb(void) 
{
    ShowWindow(PickObjsForm, SW_HIDE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements emulation of Motif PickObjs dialog.                         *
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
static BOOL CALLBACK PickObjsFormDlgProc(HWND hDlg,
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
	    ButtonsCallBack(hWnd, clientId, NULL);
	    return TRUE;
	default:
	    return FALSE;
    }
}
