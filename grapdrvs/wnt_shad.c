/*****************************************************************************
*  A windows NT interface for shading param. (based on the motif interface). *
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
#include <commctrl.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"

IRIT_STATIC_DATA int 
    ShadeActiveLgtSrc = 0;
IRIT_STATIC_DATA HWND ShadeParamForm, ShadeParamScale1, ShadeParamScale2,
    ShadeParamScale3, ShadeParamScale4, ShadeParamScale5, ShadeParamScale6,
    ShadeParamScale7, ShadeParamScale8, ShadeParamScale9, ShadeParamLightNum;
IRIT_STATIC_DATA IGShadeParamStruct IGShadeParamReset;
IRIT_STATIC_DATA IGSketchParamStruct IGSketchParamReset;

static void ShadeParamResetCb(void);
static void ShadeParamDismissCb(void);
static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData);
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID *cbs);

static BOOL CALLBACK ShadeParamFormDlgProc(HWND hDlg,
					   UINT uMsg,
					   WPARAM wParam,
					   LPARAM lParam);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main ShadeParam window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateShadeParam                                                         M
*****************************************************************************/
void CreateShadeParam(HWND IGTopLevel)
{
    char NewLabel[30];

    IGShadeParamReset = IGShadeParam;	                /* Save init value. */
    IGSketchParamReset = IGSketchParam;

    /* ShadeParam is child of IGTopLevel. */
    ShadeParamForm = CreateDialog(GetWindowInstance(IGTopLevel),
				  MAKEINTRESOURCE(IDS_SHADE_PARAM_FORM),
				  IGTopLevel,
				  ShadeParamFormDlgProc);

    ShadeParamScale1 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR1_SCALE);
    SendMessage(ShadeParamScale1, SBM_SETRANGE, -100, 100);
    SendMessage(ShadeParamScale1, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Ambient %2.3f", IGShadeParam.LightAmbient[0]);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE1, NewLabel);

    ShadeParamScale2 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR2_SCALE);
    SendMessage(ShadeParamScale2, SBM_SETRANGE, -100, 100);
    SendMessage(ShadeParamScale2, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Diffuse %2.3f", IGShadeParam.LightDiffuse[0]);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE2, NewLabel);

    ShadeParamScale3 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR3_SCALE);
    SendMessage(ShadeParamScale3, SBM_SETRANGE, -100, 100);
    SendMessage(ShadeParamScale3, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Specular %2.3f", IGShadeParam.LightSpecular[0]);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE3, NewLabel);

    ShadeParamScale4 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR4_SCALE);
    SendMessage(ShadeParamScale4, SBM_SETRANGE, -100, 100);
    SendMessage(ShadeParamScale4, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Shininess %2.3f", IGShadeParam.Shininess);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE4, NewLabel);

    ShadeParamScale5 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR5_SCALE);
    SendMessage(ShadeParamScale5, SBM_SETRANGE, -100, 100);
    SendMessage(ShadeParamScale5, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Emission %2.3f", IGShadeParam.LightEmissive[0]);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE5, NewLabel);

    ShadeParamScale6 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR6_SCALE);
    SendMessage(ShadeParamScale6, SBM_SETRANGE, 0, 100);
    SendMessage(ShadeParamScale6, SBM_SETPOS,
		(short) (100 * IGSketchParam.SilPower), TRUE);
    sprintf(NewLabel, "Sketch Sil %2.3f", IGSketchParam.SilPower);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE12, NewLabel);

    ShadeParamScale7 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR7_SCALE);
    SendMessage(ShadeParamScale7, SBM_SETRANGE, 0, 100);
    SendMessage(ShadeParamScale7, SBM_SETPOS,
		(short) (100 * IGSketchParam.ShadePower), TRUE);
    sprintf(NewLabel, "Sketch Shd %2.3f", IGSketchParam.ShadePower);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE13, NewLabel);

    ShadeParamScale8 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR8_SCALE);
    SendMessage(ShadeParamScale8, SBM_SETRANGE, -1000, 1000);
    SendMessage(ShadeParamScale8, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Importance %2.3f", IGSketchParam.SketchImpDecay);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE14, NewLabel);

    ShadeParamScale9 = GetDlgItem(ShadeParamForm, IDS_TRACKBAR9_SCALE);
    SendMessage(ShadeParamScale9, SBM_SETRANGE, 0, 9000);
    SendMessage(ShadeParamScale9, SBM_SETPOS, 9000, TRUE);
    sprintf(NewLabel, "Frontal Support %2.3f",
	    IGSketchParam.SketchImpFrntSprt);
    SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE15, NewLabel);

    ShadeParamLightNum = GetDlgItem(ShadeParamForm, IDS_LIGHT_NUM);
    CreateUpDownControl(WS_CHILD | WS_VISIBLE | WS_BORDER |
			UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
			0, 0, 0, 0,
			ShadeParamForm,
			IDS_LIGHT_NUM,
			GetWindowInstance(IGTopLevel),
			ShadeParamLightNum, 
			0, 7, 0);

    ShadeUpdateRadioButtons();
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
	OldValue = 0;
    char NewLabel[30];
    int DiffValue,
	IsSlider = FALSE;
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
	case SB_LEFT:
	    Value = -100;
	    break;
	case SB_RIGHT:
	    Value = 100;
	    break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	    Value = (short) HIWORD(wParam);
	    DiffValue = Value - OldValue;
	    IsSlider = TRUE;
	    OldValue = Value;
	    break;
	case SB_ENDSCROLL:
	    OldValue = 0;
	    return;
    }

    if (Scale == ShadeParamScale1) {				  /* Ambient */
	if (IsSlider)
	    Value = DiffValue;
	IGShadeParam.LightAmbient[0] =
	    IGShadeParam.LightAmbient[1] =
		IGShadeParam.LightAmbient[2] = (float)
		    (IGShadeParam.LightAmbient[0] *
		     exp(IGGlblChangeFactor * Value / 100.0));
	if (IGShadeParam.LightAmbient[0] < 0.001)
	    IGShadeParam.LightAmbient[0] =
		IGShadeParam.LightAmbient[1] =
		    IGShadeParam.LightAmbient[2] = (float) 0.001;

	SendMessage(Scale, SBM_SETPOS, 0, TRUE);

	sprintf(NewLabel, "Ambient %2.3f", IGShadeParam.LightAmbient[0]);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE1, NewLabel);
    }
    else if (Scale == ShadeParamScale2) {			  /* Diffuse */
	if (IsSlider)
	    Value = DiffValue;
	IGShadeParam.LightDiffuse[0] =
	    IGShadeParam.LightDiffuse[1] =
		IGShadeParam.LightDiffuse[2] = (float)
		    (IGShadeParam.LightDiffuse[0] *
		     exp(IGGlblChangeFactor * Value / 100.0));
	if (IGShadeParam.LightDiffuse[0] < 0.001)
	    IGShadeParam.LightDiffuse[0] =
		IGShadeParam.LightDiffuse[1] =
		    IGShadeParam.LightDiffuse[2] = (float) 0.001;

	SendMessage(Scale, SBM_SETPOS, 0, TRUE);

	sprintf(NewLabel, "Diffuse %2.3f", IGShadeParam.LightDiffuse[0]);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE2, NewLabel);
    }
    else if (Scale == ShadeParamScale3) {			 /* Specular */
	if (IsSlider)
	    Value = DiffValue;
	IGShadeParam.LightSpecular[0] =
	    IGShadeParam.LightSpecular[1] =
		IGShadeParam.LightSpecular[2] = (float)
		    (IGShadeParam.LightSpecular[0] *
		     exp(IGGlblChangeFactor * Value / 100.0));

	if (IGShadeParam.LightSpecular[0] < 0.001)
	    IGShadeParam.LightSpecular[0] =
		IGShadeParam.LightSpecular[1] =
		    IGShadeParam.LightSpecular[2] = (float) 0.001;

	SendMessage(Scale, SBM_SETPOS, 0, TRUE);

	sprintf(NewLabel, "Specular %2.3f", IGShadeParam.LightSpecular[0]);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE3, NewLabel);
    }
    else if (Scale == ShadeParamScale4) {			/* Shininess */
	if (IsSlider)
	    Value = DiffValue;
	IGShadeParam.Shininess = (float)
	    (IGShadeParam.Shininess * exp(IGGlblChangeFactor * Value / 100.0));

	if (IGShadeParam.Shininess < 0.001)
	    IGShadeParam.Shininess = (float) 0.001;

	SendMessage(Scale, SBM_SETPOS, 0, TRUE);

	sprintf(NewLabel, "Shininess %2.3f", IGShadeParam.Shininess);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE4, NewLabel);
    }
    else if (Scale == ShadeParamScale5) {			 /* Emissive */
	if (IsSlider)
	    Value = DiffValue;
	IGShadeParam.LightEmissive[0] =
	    IGShadeParam.LightEmissive[1] =
		IGShadeParam.LightEmissive[2] = (float)
		    (IGShadeParam.LightEmissive[0] *
		     exp(IGGlblChangeFactor * Value / 100.0));

	if (IGShadeParam.LightEmissive[0] < 0.001)
	    IGShadeParam.LightEmissive[0] =
		IGShadeParam.LightEmissive[1] =
		    IGShadeParam.LightEmissive[2] = (float) 0.001;

	SendMessage(Scale, SBM_SETPOS, 0, TRUE);

	sprintf(NewLabel, "Emissision %2.3f", IGShadeParam.LightEmissive[0]);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE5, NewLabel);
    }
    else if (Scale == ShadeParamScale6) {			 /* SilPower */
	IGSketchParam.SilPower = (float) (Value / 100.0);

	SendMessage(ShadeParamScale6, SBM_SETPOS,
		    (short) (100 * IGSketchParam.SilPower), TRUE);

	sprintf(NewLabel, "Sketch Silhouette %2.3f", IGSketchParam.SilPower);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE12, NewLabel);
    }
    else if (Scale == ShadeParamScale7) {		       /* ShadePower */
	IGSketchParam.ShadePower = (float) (Value / 100.0);

	SendMessage(ShadeParamScale7, SBM_SETPOS,
		    (short) (100 * IGSketchParam.ShadePower), TRUE);

	sprintf(NewLabel, "Sketch Shading %2.3f", IGSketchParam.ShadePower);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE13, NewLabel);
    }
    else if (Scale == ShadeParamScale8) {		       /* ImpDecay */
        if (IsSlider)
	    Value = DiffValue;
	
	IGSketchParam.SketchImpDecay *= pow(10, Value / 1000.0);

        SendMessage(ShadeParamScale8, SBM_SETPOS, 0, TRUE);

	sprintf(NewLabel, "Sketch Importance %2.3f",
		IGSketchParam.SketchImpDecay);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE14, NewLabel);

        /* Force a reevaluation of the sketches. */
	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 FALSE, FALSE, TRUE, FALSE);
    }
    else if (Scale == ShadeParamScale9) {		       /* FrntSprt */
	IGSketchParam.SketchImpFrntSprt = (float) (Value * 0.01);

	SendMessage(ShadeParamScale9, SBM_SETPOS,
		    (short) (100 * IGSketchParam.SketchImpFrntSprt), TRUE);

	sprintf(NewLabel, "Frontal Support %3.2f",
		IGSketchParam.SketchImpFrntSprt);
	SetDlgItemText(ShadeParamForm, ID_STATIC_SCALETITLE15, NewLabel);

        /* Force a reevaluation of the sketches. */
	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 FALSE, FALSE, TRUE, FALSE);
    }
    else
	return;

    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid mode drawing. */
    InvalidateRect(IGhWndView, NULL, FALSE);        /* And request a redraw. */
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
    char Str[IRIT_LINE_LEN];

    switch (Name) {
	case IDCANCEL:
	case IDS_DISMISS:
	    ShadeParamDismissCb();
	    break;
	case IDS_RESET:
	    ShadeParamResetCb();
	    break;
	case IDS_SKETCH_SIL_ISOCRVS:
	    IGSketchParam.SketchSilType = IG_SKETCHING_ISO_PARAM;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SIL_CRVTR:
	    IGSketchParam.SketchSilType = IG_SKETCHING_CURVATURE;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SIL_ISOCLINES:
	    IGSketchParam.SketchSilType = IG_SKETCHING_ISOCLINES;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SIL_ORTHOCLINES:
	    IGSketchParam.SketchSilType = IG_SKETCHING_ORTHOCLINES;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SHD_ISOCRVS:
	    IGSketchParam.SketchShdType = IG_SKETCHING_ISO_PARAM;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SHD_CRVTR:
	    IGSketchParam.SketchShdType = IG_SKETCHING_CURVATURE;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SHD_ISOCLINES:
	    IGSketchParam.SketchShdType = IG_SKETCHING_ISOCLINES;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SHD_ORTHOCLINES:
	    IGSketchParam.SketchShdType = IG_SKETCHING_ORTHOCLINES;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_IMP_ISOCRVS:
	    IGSketchParam.SketchImpType = IG_SKETCHING_ISO_PARAM;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_IMP_CRVTR:
	    IGSketchParam.SketchImpType = IG_SKETCHING_CURVATURE;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_IMP_ISOCLINES:
	    IGSketchParam.SketchImpType = IG_SKETCHING_ISOCLINES;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_IMP_ORTHOCLINES:
	    IGSketchParam.SketchImpType = IG_SKETCHING_ORTHOCLINES;
	    IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					     FALSE, FALSE, TRUE, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_SHD_INV:
	    IGSketchParam.SketchInvShd = !IGSketchParam.SketchInvShd;
	    CheckDlgButton(ShadeParamForm, IDS_SKETCH_SHD_INV,
		IGSketchParam.SketchInvShd ? BST_CHECKED : BST_UNCHECKED);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_SKETCH_IMPORTANCE:
	    IGSketchParam.SketchImp = !IGSketchParam.SketchImp;
	    CheckDlgButton(ShadeParamForm, IDS_SKETCH_IMPORTANCE,
		IGSketchParam.SketchImp ? BST_CHECKED : BST_UNCHECKED);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDS_STYLE_NONE:
	    IGGlblShadingModel = IG_SHADING_NONE;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDS_STYLE_BACKGROUND:
	    IGGlblShadingModel = IG_SHADING_BACKGROUND;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDS_STYLE_FLAT:
	    IGGlblShadingModel = IG_SHADING_FLAT;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDS_STYLE_GOURAUD:
	    IGGlblShadingModel = IG_SHADING_GOURAUD;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDS_STYLE_PHONG:
	    IGGlblShadingModel = IG_SHADING_PHONG;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDS_LIGHT_NUM:
	    ShadeActiveLgtSrc = GetDlgItemInt(ShadeParamForm, IDS_LIGHT_NUM,
					      NULL, TRUE);
	    sprintf(Str, "%2.2f",
		    IGShadeParam.LightPos[ShadeActiveLgtSrc][0]);
	    SetDlgItemText(ShadeParamForm, IDS_LIGHT_X, Str);
	    sprintf(Str, "%2.2f",
		    IGShadeParam.LightPos[ShadeActiveLgtSrc][1]);
	    SetDlgItemText(ShadeParamForm, IDS_LIGHT_Y, Str);
	    sprintf(Str, "%2.2f",
		    IGShadeParam.LightPos[ShadeActiveLgtSrc][2]);
	    SetDlgItemText(ShadeParamForm, IDS_LIGHT_Z, Str);
	    sprintf(Str, "%2.2f",
		    IGShadeParam.LightPos[ShadeActiveLgtSrc][3]);
	    SetDlgItemText(ShadeParamForm, IDS_LIGHT_W, Str);
	    break;
	case IDS_LIGHT_X:
	case IDS_LIGHT_Y:
	case IDS_LIGHT_Z:
	case IDS_LIGHT_W:
	    if (GetDlgItemText(ShadeParamForm, Name, Str,
			       IRIT_LINE_LEN - 1) != 0) {
	        float f;

		if (sscanf(Str, "%f", &f) == 1) {
		    IRIT_STATIC_DATA IrtVecType
			WhiteColor = { 1.0, 1.0, 1.0 };

		    IGShadeParam.LightPos[ShadeActiveLgtSrc]
					 [Name - IDS_LIGHT_X] = f;
		    
		    IGSetLightSource(IGShadeParam.LightPos[ShadeActiveLgtSrc],
				     WhiteColor, ShadeActiveLgtSrc);

		    InvalidateRect(IGhWndView, NULL, FALSE);
		}
		else {
		    sprintf(Str, "%2.2f",
			    IGShadeParam.LightPos[ShadeActiveLgtSrc]
						 [Name - IDS_LIGHT_X]);
		    SetDlgItemText(ShadeParamForm, Name, Str);
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
*   ShadeParamCB	                                                     M
*****************************************************************************/
void ShadeParamCB(void)
{  
    RECT Wr, Dr, Cr;      

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(ShadeParamForm, &Cr);			   /* Get form size. */
    SetWindowPos(ShadeParamForm, 
		 HWND_TOP,
		 min(Wr.right + 3, Dr.right - Cr.right - 3),
		 Wr.top,
		 0, 
		 0,
		 SWP_NOSIZE | SWP_SHOWWINDOW);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for reset button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ShadeParamResetCb(void) 
{
    IGShadeParam = IGShadeParamReset;	              /* Restore init value. */
    IGSketchParam = IGSketchParamReset;

    SendMessage(ShadeParamScale1, SBM_SETPOS, 0, TRUE);
    SendMessage(ShadeParamScale2, SBM_SETPOS, 0, TRUE);
    SendMessage(ShadeParamScale3, SBM_SETPOS, 0, TRUE);
    SendMessage(ShadeParamScale4, SBM_SETPOS, 0, TRUE);
    SendMessage(ShadeParamScale5, SBM_SETPOS, 0, TRUE);
    SendMessage(ShadeParamScale6, SBM_SETPOS,
		       (short) (100 * IGSketchParam.SilPower), TRUE);
    SendMessage(ShadeParamScale7, SBM_SETPOS,
		       (short) (100 * IGSketchParam.ShadePower), TRUE);
    SendMessage(ShadeParamScale8, SBM_SETPOS, 0, TRUE);
    SendMessage(ShadeParamScale9, SBM_SETPOS,
		       (short) (100 * IGSketchParam.SketchImpFrntSprt), TRUE);

    IGGlblLastWasSolidRendering = FALSE;/* Force init of solid mode drawing. */
    InvalidateRect(IGhWndView, NULL, FALSE);        /* And request a redraw. */
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
static void ShadeParamDismissCb(void) 
{
    ShowWindow(ShadeParamForm, SW_HIDE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates the radio buttons in the shading pop up following global state.  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ShadeUpdateRadioButtons                                                  M
*****************************************************************************/
void ShadeUpdateRadioButtons(void)
{
    switch (IGGlblShadingModel) {
        case IG_SHADING_NONE:
	    CheckRadioButton(ShadeParamForm,
			     IDS_STYLE_NONE, IDS_STYLE_PHONG,
			     IDS_STYLE_NONE);
	    break;
	case IG_SHADING_BACKGROUND:
	    CheckRadioButton(ShadeParamForm,
			     IDS_STYLE_NONE, IDS_STYLE_PHONG,
			     IDS_STYLE_BACKGROUND);
	    break;
	case IG_SHADING_FLAT:
	    CheckRadioButton(ShadeParamForm,
			     IDS_STYLE_NONE, IDS_STYLE_PHONG,
			     IDS_STYLE_FLAT);
	    break;
	case IG_SHADING_GOURAUD:
	    CheckRadioButton(ShadeParamForm,
			     IDS_STYLE_NONE, IDS_STYLE_PHONG,
			     IDS_STYLE_GOURAUD);
	    break;
	case IG_SHADING_PHONG:
	    CheckRadioButton(ShadeParamForm,
			     IDS_STYLE_NONE, IDS_STYLE_PHONG,
			     IDS_STYLE_PHONG);
	    break;
    }

    switch (IGSketchParam.SketchSilType) {
	case IG_SKETCHING_ISO_PARAM:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SIL_ISOCRVS,
			     IDS_SKETCH_SIL_ORTHOCLINES,
			     IDS_SKETCH_SIL_ISOCRVS);
	    break;
	case IG_SKETCHING_CURVATURE:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SIL_ISOCRVS,
			     IDS_SKETCH_SIL_ORTHOCLINES,
			     IDS_SKETCH_SIL_CRVTR);
	    break;
        case IG_SKETCHING_ISOCLINES:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SIL_ISOCRVS,
			     IDS_SKETCH_SIL_ORTHOCLINES,
			     IDS_SKETCH_SIL_ISOCLINES);
	    break;
        case IG_SKETCHING_ORTHOCLINES:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SIL_ISOCRVS,
			     IDS_SKETCH_SIL_ORTHOCLINES,
			     IDS_SKETCH_SIL_ORTHOCLINES);
	    break;
    }

    switch (IGSketchParam.SketchShdType) {
	case IG_SKETCHING_ISO_PARAM:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SHD_ISOCRVS,
			     IDS_SKETCH_SHD_ORTHOCLINES,
			     IDS_SKETCH_SHD_ISOCRVS);
	    break;
	case IG_SKETCHING_CURVATURE:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SHD_ISOCRVS,
			     IDS_SKETCH_SHD_ORTHOCLINES,
			     IDS_SKETCH_SHD_CRVTR);
	    break;
        case IG_SKETCHING_ISOCLINES:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SHD_ISOCRVS,
			     IDS_SKETCH_SHD_ORTHOCLINES,
			     IDS_SKETCH_SHD_ISOCLINES);
	    break;
        case IG_SKETCHING_ORTHOCLINES:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_SHD_ISOCRVS,
			     IDS_SKETCH_SHD_ORTHOCLINES,
			     IDS_SKETCH_SHD_ORTHOCLINES);
	    break;
    }

    CheckDlgButton(ShadeParamForm, IDS_SKETCH_SHD_INV,
		   IGSketchParam.SketchInvShd ? BST_CHECKED : BST_UNCHECKED);

    switch (IGSketchParam.SketchImpType) {
	case IG_SKETCHING_ISO_PARAM:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_IMP_ISOCRVS,
			     IDS_SKETCH_IMP_ORTHOCLINES,
			     IDS_SKETCH_IMP_ISOCRVS);
	    break;
	case IG_SKETCHING_CURVATURE:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_IMP_ISOCRVS,
			     IDS_SKETCH_IMP_ORTHOCLINES,
			     IDS_SKETCH_IMP_CRVTR);
	    break;
        case IG_SKETCHING_ISOCLINES:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_IMP_ISOCRVS,
			     IDS_SKETCH_IMP_ORTHOCLINES,
			     IDS_SKETCH_IMP_ISOCLINES);
	    break;
        case IG_SKETCHING_ORTHOCLINES:
	    CheckRadioButton(ShadeParamForm,
			     IDS_SKETCH_IMP_ISOCRVS,
			     IDS_SKETCH_IMP_ORTHOCLINES,
			     IDS_SKETCH_IMP_ORTHOCLINES);
	    break;
    }

    CheckDlgButton(ShadeParamForm, IDS_SKETCH_IMPORTANCE,
		   IGSketchParam.SketchImp ? BST_CHECKED : BST_UNCHECKED);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements emulation of Motif ShadeParam dialog.                         *
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
static BOOL CALLBACK ShadeParamFormDlgProc(HWND hDlg,
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
	    if (hWnd != ShadeParamScale1 &&
		hWnd != ShadeParamScale2 &&
		hWnd != ShadeParamScale3 &&
		hWnd != ShadeParamScale4 &&
		hWnd != ShadeParamScale5 &&
		hWnd != ShadeParamScale6 &&
		hWnd != ShadeParamScale7 &&
		hWnd != ShadeParamScale8 &&
		hWnd != ShadeParamScale9) {
	        clientId = (PVOID) LOWORD(wParam);        /* ID of the item. */
		ButtonsCallBack(hWnd, clientId, NULL);
	    }
	    return TRUE;
	case WM_HSCROLL:
            hWnd = (HWND) lParam;                 /* Handle of control HWND. */
	    if (hWnd == ShadeParamScale1 ||
	        hWnd == ShadeParamScale2 ||
	        hWnd == ShadeParamScale3 ||
	        hWnd == ShadeParamScale4 ||
	        hWnd == ShadeParamScale5 ||
	        hWnd == ShadeParamScale6 ||
	        hWnd == ShadeParamScale7 ||
	        hWnd == ShadeParamScale8 ||
	        hWnd == ShadeParamScale9) {
		ScaleCB(hWnd, wParam, NULL);    
	    }
	    return TRUE;
	default:
	    return FALSE;
    }
}
