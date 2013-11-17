/*****************************************************************************
*  A windows NT interface for environment.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, Mar. 1998.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"

IRIT_STATIC_DATA HWND EnvParamForm, EnvNLenScale, EnvLWidthScale,
    EnvSensitiveScale, EnvLowresRatScale, EnvIsolinesScale,
    EnvPllnFineScale, EnvPlgnFineScale;

static void EnvParamDismissCb(void);
static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData);
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID *cbs);
static BOOL CALLBACK EnvParamFormDlgProc(HWND hDlg,
					 UINT uMsg,
					 WPARAM wParam,
					 LPARAM lParam);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main EnvParam window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateEnvironment                                                        M
*****************************************************************************/
void CreateEnvironment(HWND IGTopLevel)
{
    /* EnvParam is child of IGTopLevel. */
    EnvParamForm = CreateDialog(GetWindowInstance(IGTopLevel),
				MAKEINTRESOURCE(IDEN_ENV_PARAM_FORM),
				IGTopLevel,
				EnvParamFormDlgProc);

    EnvParamUpdateCb();
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
    int Value,
	NewValue = 0,
	DeltaValue = 0;

    switch (LOWORD(wParam)) {
	case SB_LINERIGHT:
	    DeltaValue++;
	    break;
	case SB_LINELEFT:
	    DeltaValue--;
	    break;
	case SB_PAGERIGHT:
	    DeltaValue = 10;
	    break;
	case SB_PAGELEFT:
	    DeltaValue = -10;
	    break;
	case SB_RIGHT:
	    DeltaValue = 100;
	    break;
	case SB_LEFT:
	    DeltaValue = -100;
	    break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	    NewValue = (short) HIWORD(wParam);
	    DeltaValue = NewValue - OldValue;
	    break;
	case SB_ENDSCROLL:
	    OldValue = 0;
	    return;
    }

    if (Scale == EnvNLenScale) {
	IGGlblNormalSize *= (float) exp(IGGlblChangeFactor *
					DeltaValue / 100.0);
	IGGlblPointSize *= (float) exp(IGGlblChangeFactor *
				       DeltaValue / 100.0);
	sprintf(NewLabel, "Normal Len %5.4f", IGGlblNormalSize);
	SetDlgItemText(EnvParamForm, IDEN_NORMAL_LEN_TITLE, NewLabel);

	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 FALSE, FALSE, TRUE, FALSE);
	IGRedrawViewWindow();
    }
    else if (Scale == EnvLWidthScale) {
	Value = (int) (IGGlblLineWidth * 
			   exp(IGGlblChangeFactor * DeltaValue / 100.0));
	IGGlblLineWidth = Value != IGGlblLineWidth ? Value
						   : Value + IRIT_SIGN(DeltaValue);
        if (IGGlblLineWidth < 1)
	    IGGlblLineWidth = 1;
	sprintf(NewLabel, "Lines Width %d", IGGlblLineWidth);
	SetDlgItemText(EnvParamForm, IDEN_LINE_WIDTH_TITLE, NewLabel);
	IGRedrawViewWindow();
    }
    else if (Scale == EnvSensitiveScale) {
	IGGlblChangeFactor *= exp(DeltaValue / 100.0);
	sprintf(NewLabel, "Sensitivity %2.2f", IGGlblChangeFactor);
	SetDlgItemText(EnvParamForm, IDEN_SENSITIVITY_TITLE, NewLabel);
    }
    else if (Scale == EnvLowresRatScale) {
	IGGlblRelLowresFineNess *= exp(IGGlblChangeFactor * DeltaValue / 30.0);
	IGGlblRelLowresFineNess = IRIT_BOUND(IGGlblRelLowresFineNess, 0.0, 1.0);
	sprintf(NewLabel, "Lowres Ratio %6.4f", IGGlblRelLowresFineNess);
	SetDlgItemText(EnvParamForm, IDEN_LOWRES_RATIO_TITLE, NewLabel);

	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 TRUE, TRUE, TRUE, FALSE);
	IGRedrawViewWindow();
    }
    else if (Scale == EnvIsolinesScale) {
	Value = (int) (IGGlblNumOfIsolines * 
			   exp(IGGlblChangeFactor * DeltaValue / 100.0));
	IGGlblNumOfIsolines = Value != IGGlblNumOfIsolines ? Value :
						      Value + IRIT_SIGN(DeltaValue);
	if (IGGlblNumOfIsolines < 0)
	    IGGlblNumOfIsolines = 0;
	sprintf(NewLabel, "Isolines %d", IGGlblNumOfIsolines);
	SetDlgItemText(EnvParamForm, IDEN_ISOLINES_TITLE, NewLabel);

	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 FALSE, TRUE, FALSE, FALSE);
	IGRedrawViewWindow();
    }
    else if (Scale == EnvPllnFineScale) {
	if (IGGlblPolylineOptiApprox == 0) {
	    Value = (int) (IGGlblPllnFineness *
		           exp(IGGlblChangeFactor * DeltaValue / 100.0));
	    IGGlblPllnFineness = Value != IGGlblPllnFineness ? Value
						    : Value + IRIT_SIGN(DeltaValue);
	    if (IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_UNIFORM &&
		IGGlblPllnFineness < 2)
	        IGGlblPllnFineness = 2;
	}
	else {
	    IGGlblPllnFineness /= exp(IGGlblChangeFactor * DeltaValue / 100.0);
	}

	sprintf(NewLabel, "Polyline FineNess %.5g", IGGlblPllnFineness);
	SetDlgItemText(EnvParamForm, IDEN_POLYLINE_FINENESS_TITLE, NewLabel);

	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 FALSE, TRUE, TRUE, FALSE);
	IGRedrawViewWindow();
    }
    else if (Scale == EnvPlgnFineScale) {
	if (IGGlblPolygonOptiApprox == 0) {
	    Value = (int) (IGGlblPlgnFineness *
			   exp(IGGlblChangeFactor * DeltaValue / 100.0));
	    IGGlblPlgnFineness = Value != IGGlblPlgnFineness ? Value
						    : Value + IRIT_SIGN(DeltaValue);
	    if (IGGlblPlgnFineness < 2.0)
	        IGGlblPlgnFineness = 2.0;
	}
	else {
	    IGGlblPlgnFineness /= exp(IGGlblChangeFactor * DeltaValue / 100.0);
	}

	sprintf(NewLabel, "Polygon FineNess %.5g", IGGlblPlgnFineness);
	SetDlgItemText(EnvParamForm, IDEN_POLYGON_FINENESS_TITLE, NewLabel);

	IGActiveListFreePolyIsoAttribute(IGGlblDisplayList,
					 TRUE, FALSE, FALSE, FALSE);
	IGRedrawViewWindow();
    }

    OldValue = NewValue;

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

    switch (Name) {
	case IDCANCEL:
	    EnvParamDismissCb();
	    break;
	case IDEN_BUTTON_SCREEN_OBJ:
	    IGGlblTransformMode = IGGlblTransformMode == IG_TRANS_OBJECT ? 
					    IG_TRANS_SCREEN : IG_TRANS_OBJECT;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_SCREEN_OBJ,
			   IGGlblTransformMode == IG_TRANS_OBJECT ?
					      "Objt Trans" : "Scr Trans");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    break;
	case IDEN_BUTTON_CONT_MOTION:
	    IGGlblContinuousMotion = !IGGlblContinuousMotion;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_CONT_MOTION,
			   IGGlblContinuousMotion ?
					      "Cont Motion" : "Rglr Motion");
	    break;
	case IDEN_BUTTON_NRML_ORIENT:
	    IGGlblFlipNormalOrient = !IGGlblFlipNormalOrient;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_NRML_ORIENT,
			   IGGlblFlipNormalOrient ?
				      "Rvrsd Nrmls" : "Rglr Nrmls");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_LIGHT_ONE_SIDE:
	    IGGlblLightOneSide = !IGGlblLightOneSide;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_LIGHT_ONE_SIDE,
			   IGGlblLightOneSide ?
				      "Lgt1Side" : "Lgt2Sides");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_PERSP_ORTHO:
	    IGGlblViewMode = IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
				   IG_VIEW_ORTHOGRAPHIC : IG_VIEW_PERSPECTIVE;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_PERSP_ORTHO,
			   IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
					     "Perspective" : "Orthographic");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    break;
	case IDEN_BUTTON_Z_BUFFER:
	    IGIritError("Z Buffer button was pressed");
	    break;
	case IDEN_BUTTON_DEPTH_CUE:
	    IGGlblDepthCue = !IGGlblDepthCue;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DEPTH_CUE,
			   IGGlblDepthCue ? "Depth Cue On" : "No Depth Cue");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    InvalidateRect(IGhWndTrans, NULL, TRUE);
	    break;
	case IDEN_BUTTON_CULL_BFACE:
	    IGGlblBackFaceCull = !IGGlblBackFaceCull;
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_CULL_BFACE,
			   IGGlblBackFaceCull ? "Cull BFace"
			   		      : "No Cull BFace");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_INT_EDGES:
	    IGStateHandler(IG_STATE_DRAW_INTERNAL, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_INT_EDGES,
			   IGGlblDrawInternal ? "Drw Intrl Edgs"
					      : "No Intrl Edgs");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DOUBLE_BUF:
	    IGStateHandler(IG_STATE_DOUBLE_BUFFER, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DOUBLE_BUF,
			   IGGlblDoDoubleBuffer ? "Dbl Buffer"
			   			: "Sngl Buffer");
	    break;
	case IDEN_BUTTON_DRAW_VNRMLS:
	    IGStateHandler(IG_STATE_DRAW_VNORMAL, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_VNRMLS,
			   IGGlblDrawVNormal ? "Draw VNrmls"
			   		     : "No VNrmls");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_PNRMLS:
	    IGStateHandler(IG_STATE_DRAW_PNORMAL, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_PNRMLS,
			   IGGlblDrawPNormal ? "Draw PNrmls"
			   		     : "No PNrmls");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_SMESH:
	    IGStateHandler(IG_STATE_DRAW_SRF_MESH, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SMESH,
			   IGGlblDrawSurfaceMesh ? "Draw Srf Mesh"
			   			 : "No Srf Mesh");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_SPOLY:
	    IGStateHandler(IG_STATE_DRAW_SRF_POLY, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SPOLY,
			   IGGlblDrawSurfacePoly ? "Draw Srf Polys"
			   			 : "No Srf Polys");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_BNDRY:
	    IGStateHandler(IG_STATE_DRAW_SRF_BNDRY, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_BNDRY,
			   IGGlblDrawSurfaceBndry ? "Draw Srf Bndry"
			   			  : "No Srf Bndry");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_SILH:
	    IGStateHandler(IG_STATE_DRAW_SRF_SILH, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SILH,
			   IGGlblDrawSurfaceSilh ? "Draw Srf Silhs"
			   			 : "No Srf Silhs");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_RFLCT_LNS:
	    IGStateHandler(IG_STATE_DRAW_SRF_RFLCT_LNS, IG_STATE_TGL,
				  FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_RFLCT_LNS,
			   IGGlblDrawSurfaceRflctLns ? "Draw Rflct Lns"
			   			     : "No Rflct Lns");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_POLYGONS:
	    IGStateHandler(IG_STATE_DRAW_POLYGONS, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_POLYGONS,
			   IGGlblDrawPolygons ? "Drw Plygns"
			   		      : "No Plygns");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_SISO:
	    IGStateHandler(IG_STATE_DRAW_SRF_WIRE, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SISO,
			   IGGlblDrawSurfaceWire ? "Draw Srf Isos"
			   			 : "No Srf Isos");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_SSKTCH:
	    IGStateHandler(IG_STATE_DRAW_SRF_SKTCH, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SSKTCH,
			   IGGlblDrawSurfaceSketch ? "Draw Srf Sktch"
						   : "No Srf Sktch");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_DRAW_4_FLAT:
	    IGStateHandler(IG_STATE_FOUR_PER_FLAT, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_4_FLAT,
			   IGGlblFourPerFlat ? "4 Per Flat" : "2 Per Flat");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_NUM_POLY_COUNT:
	    IGStateHandler(IG_STATE_NUM_POLY_COUNT, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_NUM_POLY_COUNT,
			   IGGlblCountNumPolys ? "#Poly Cnt" : "No #Pl Cnt");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_FRAME_PER_SEC:
	    IGStateHandler(IG_STATE_FRAME_PER_SEC, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_FRAME_PER_SEC,
			   IGGlblCountFramePerSec ? "FPS" : "No FPS");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_POLYGON_OPTI:
	    IGStateHandler(IG_STATE_POLYGON_OPTI, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_POLYGON_OPTI,
			   IGGlblPolygonOptiApprox ? "Optimal" : "Uniform");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;
	case IDEN_BUTTON_POLYLINE_OPTI:
	    IGStateHandler(IG_STATE_POLYLINE_OPTI, IG_STATE_TGL, FALSE);
	    SetDlgItemText(EnvParamForm, IDEN_BUTTON_POLYLINE_OPTI,
			   IGGlblPolylineOptiApprox !=
			      SYMB_CRV_APPROX_UNIFORM ? "Optimal" : "Uniform");
	    InvalidateRect(IGhWndView, NULL, FALSE);/* And request a redraw. */
	    break;

	case IDEN_SHADE_STYLE_NONE:
	    IGGlblShadingModel = IG_SHADING_NONE;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDEN_SHADE_STYLE_BACKGROUND:
	    IGGlblShadingModel = IG_SHADING_BACKGROUND;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDEN_SHADE_STYLE_FLAT:
	    IGGlblShadingModel = IG_SHADING_FLAT;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDEN_SHADE_STYLE_GOURAUD:
	    IGGlblShadingModel = IG_SHADING_GOURAUD;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDEN_SHADE_STYLE_PHONG:
	    IGGlblShadingModel = IG_SHADING_PHONG;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;

        case IDEN_ANTI_ALIAS_OFF:
	    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_OFF;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
        case IDEN_ANTI_ALIAS_ON:
	    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_ON;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
        case IDEN_ANTI_ALIAS_BLEND:
	    IGGlblAntiAliasing = IG_STATE_ANTI_ALIAS_BLEND;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;

        case IDEN_DRAW_STYLE_SOLID:
	    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_SOLID;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
        case IDEN_DRAW_STYLE_WIREFRAME:
	    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_WIREFRAME;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
        case IDEN_DRAW_STYLE_POINTS:
	    IGGlblDrawStyle = IG_STATE_DRAW_STYLE_POINTS;
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    IGCreateStateMenu();
	    break;
	case IDEN_BUTTON_CLEAR_VIEW:
	    IGStateHandler(IG_STATE_CLEAR_VIEW, IG_STATE_TGL, FALSE);
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;
	case IDEN_BUTTON_DISMISS:
	    EnvParamDismissCb();
	    break;
	case IDEN_BUTTON_DRAW_3D_GLASSES:
	    switch (IGGlbl3DGlassesMode) {
		case IG_GLASSES_3D_NONE:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_CHROMADEPTH;
		    IGGlbl3DGlassesImgIndx = -1;
		    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_3D_GLASSES,
				   "ChromaDep");
	            break;
		case IG_GLASSES_3D_CHROMADEPTH:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_RED_BLUE;
	            SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_3D_GLASSES,
				   "Red Blue");
	            break;
		case IG_GLASSES_3D_RED_BLUE:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_RED_GREEN;
	            SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_3D_GLASSES,
				   "Red Green");
	            break;
		case IG_GLASSES_3D_RED_GREEN:
		default:
		    IGGlbl3DGlassesMode = IG_GLASSES_3D_NONE;
		    IGGlbl3DGlassesImgIndx = -1;
	            SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_3D_GLASSES,
				   "No 3D Glss");
	            break;
	    }
	    InvalidateRect(IGhWndView, NULL, FALSE);
	    break;		
    }
    IGCreateStateMenu();
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
*   EnvironmentCB	                                                     M
*****************************************************************************/
void EnvironmentCB(void)
{  
    RECT Wr, Dr, Cr;      

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(EnvParamForm, &Cr);			   /* Get form size. */
    SetWindowPos(EnvParamForm, 
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
*   EnvParamUpdateCb                                                         M
*****************************************************************************/
void EnvParamUpdateCb(void) 
{
    char NewLabel[30];

    /* Update all the scroll bars. */
    EnvNLenScale = GetDlgItem(EnvParamForm, IDEN_NORMAL_LEN_TRACKBAR);
    SendMessage(EnvNLenScale, SBM_SETRANGE, -100, 100);
    SendMessage(EnvNLenScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Normal Len %5.4f", IGGlblNormalSize);
    SetDlgItemText(EnvParamForm, IDEN_NORMAL_LEN_TITLE, NewLabel);

    EnvLWidthScale = GetDlgItem(EnvParamForm, IDEN_LINE_WIDTH_TRACKBAR);
    SendMessage(EnvLWidthScale, SBM_SETRANGE, -20, 20);
    SendMessage(EnvLWidthScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Lines Width %d", IGGlblLineWidth);
    SetDlgItemText(EnvParamForm, IDEN_LINE_WIDTH_TITLE, NewLabel);

    EnvSensitiveScale = GetDlgItem(EnvParamForm, IDEN_SENSITIVITY_TRACKBAR);
    SendMessage(EnvSensitiveScale, SBM_SETRANGE, -100, 100);
    SendMessage(EnvSensitiveScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Sensitivity %2.2f", IGGlblChangeFactor);
    SetDlgItemText(EnvParamForm, IDEN_SENSITIVITY_TITLE, NewLabel);

    EnvLowresRatScale = GetDlgItem(EnvParamForm, IDEN_LOWRES_RATIO_TRACKBAR);
    SendMessage(EnvLowresRatScale, SBM_SETRANGE, -100, 100);
    SendMessage(EnvLowresRatScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Lowres Ratio %6.4f", IGGlblRelLowresFineNess);
    SetDlgItemText(EnvParamForm, IDEN_LOWRES_RATIO_TITLE, NewLabel);

    EnvIsolinesScale = GetDlgItem(EnvParamForm, IDEN_ISOLINES_TRACKBAR);
    SendMessage(EnvIsolinesScale, SBM_SETRANGE, -100, 100);
    SendMessage(EnvIsolinesScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Isolines %3d", IGGlblNumOfIsolines);
    SetDlgItemText(EnvParamForm, IDEN_ISOLINES_TITLE, NewLabel);

    EnvPllnFineScale = GetDlgItem(EnvParamForm, IDEN_POLYLINE_FINENESS_TRACKBAR);
    SendMessage(EnvPllnFineScale, SBM_SETRANGE, -100, 100);
    SendMessage(EnvPllnFineScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Polyline FineNess %.5g", IGGlblPllnFineness);
    SetDlgItemText(EnvParamForm, IDEN_POLYLINE_FINENESS_TITLE, NewLabel);

    EnvPlgnFineScale = GetDlgItem(EnvParamForm, IDEN_POLYGON_FINENESS_TRACKBAR);
    SendMessage(EnvPlgnFineScale, SBM_SETRANGE, -100, 100);
    SendMessage(EnvPlgnFineScale, SBM_SETPOS, 0, TRUE);
    sprintf(NewLabel, "Polygon FineNess %.5g", IGGlblPlgnFineness);
    SetDlgItemText(EnvParamForm, IDEN_POLYGON_FINENESS_TITLE, NewLabel);

    /* Update all the buttons. */
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_SCREEN_OBJ,
		   IGGlblTransformMode == IG_TRANS_OBJECT ?
					      "Objt Trans" : "Scrn Trans");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_CONT_MOTION,
		   IGGlblContinuousMotion ? "Cont Motion" : "Rglr Motion");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_PERSP_ORTHO,
		   IGGlblViewMode == IG_VIEW_PERSPECTIVE ?
					     "Perspective" : "Orthographic");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DOUBLE_BUF,
		   IGGlblDoDoubleBuffer ? "Dbl Buffer" : "Sng Buffer");

    /* Shading style widget is between here. */

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_VNRMLS,
		   IGGlblDrawVNormal ? "Draw VNrmls" : "No VNrmls");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_PNRMLS,
		   IGGlblDrawPNormal ? "Draw PNrmls" : "No PNrmls");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_NRML_ORIENT,
		   IGGlblFlipNormalOrient ? "Rvrsd Nrmls" : "Rglr Nrmls");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_LIGHT_ONE_SIDE,
		   IGGlblLightOneSide ? "Lgt1Side" : "Lgt2Sides");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DEPTH_CUE,
		   IGGlblDepthCue ? "Depth Cue" : "No DpthCue");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_CULL_BFACE,
		   IGGlblBackFaceCull ? "Cull BFace" : "No CullBF");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_INT_EDGES,
		   IGGlblDrawInternal ? "Drw Intrl Edgs" : "No Intrl Edgs");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_POLYGONS,
		   IGGlblDrawPolygons ? "Drw Plygns" : "No Plygns");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SSKTCH,
		   IGGlblDrawSurfaceSketch ? "Draw Srf Sktch" : "No Srf Sktch");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SMESH,
		   IGGlblDrawSurfaceMesh ? "Draw Srf Mesh" : "No Srf Mesh");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SPOLY,
		   IGGlblDrawSurfacePoly ? "Draw Srf Polys" : "No Srf Polys");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SISO,
		   IGGlblDrawSurfaceWire ? "Draw Srf Isos" : "No Srf Isos");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_BNDRY,
		   IGGlblDrawSurfaceBndry ? "Draw Srf Bndry" : "No Srf Bndry");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_SILH,
		   IGGlblDrawSurfaceSilh ? "Draw Srf Silhs" : "No Srf Silhs");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_RFLCT_LNS,
		   IGGlblDrawSurfaceRflctLns ? "Draw Rflct Lns" : "No Rflct Lns");
    SetDlgItemText(EnvParamForm, IDEN_BUTTON_DRAW_4_FLAT,
		   IGGlblFourPerFlat ? "4 Per Flat" : "2 Per Flat");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_NUM_POLY_COUNT,
		   IGGlblCountNumPolys ? "#Poly Cnt" : "No #Pl Cnt");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_FRAME_PER_SEC,
		   IGGlblCountFramePerSec ? "FPS" : "No FPS");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_POLYGON_OPTI,
		   IGGlblPolygonOptiApprox ? "Optimal" : "Uniform");

    SetDlgItemText(EnvParamForm, IDEN_BUTTON_POLYLINE_OPTI,
		   IGGlblPolylineOptiApprox !=
			      SYMB_CRV_APPROX_UNIFORM ? "Optimal" : "Uniform");

    /* Update the radio bottons. */
    switch (IGGlblShadingModel) {
        case IG_SHADING_NONE:
	    CheckRadioButton(EnvParamForm,
			     IDEN_SHADE_STYLE_NONE, IDEN_SHADE_STYLE_PHONG,
			     IDEN_SHADE_STYLE_NONE);
	    break;
	case IG_SHADING_BACKGROUND:
	    CheckRadioButton(EnvParamForm,
			     IDEN_SHADE_STYLE_NONE, IDEN_SHADE_STYLE_PHONG,
			     IDEN_SHADE_STYLE_BACKGROUND);
	    break;
	case IG_SHADING_FLAT:
	    CheckRadioButton(EnvParamForm,
			     IDEN_SHADE_STYLE_NONE, IDEN_SHADE_STYLE_PHONG,
			     IDEN_SHADE_STYLE_FLAT);
	    break;
	case IG_SHADING_GOURAUD:
	    CheckRadioButton(EnvParamForm,
			     IDEN_SHADE_STYLE_NONE, IDEN_SHADE_STYLE_PHONG,
			     IDEN_SHADE_STYLE_GOURAUD);
	    break;
	case IG_SHADING_PHONG:
	    CheckRadioButton(EnvParamForm,
			     IDEN_SHADE_STYLE_NONE, IDEN_SHADE_STYLE_PHONG,
			     IDEN_SHADE_STYLE_PHONG);
	    break;
    }

    switch (IGGlblDrawStyle) {
	case IG_STATE_DRAW_STYLE_WIREFRAME:
	    CheckRadioButton(EnvParamForm,
			     IDEN_DRAW_STYLE_WIREFRAME, IDEN_DRAW_STYLE_POINTS,
			     IDEN_DRAW_STYLE_WIREFRAME);
	    break;
	case IG_STATE_DRAW_STYLE_SOLID:
	    CheckRadioButton(EnvParamForm,
			     IDEN_DRAW_STYLE_WIREFRAME, IDEN_DRAW_STYLE_POINTS,
			     IDEN_DRAW_STYLE_SOLID);
	    break;
	case IG_STATE_DRAW_STYLE_POINTS:
	    CheckRadioButton(EnvParamForm,
			     IDEN_DRAW_STYLE_WIREFRAME, IDEN_DRAW_STYLE_POINTS,
			     IDEN_DRAW_STYLE_POINTS);
	    break;
    }

    switch (IGGlblAntiAliasing) {
	case IG_STATE_ANTI_ALIAS_OFF:
	    CheckRadioButton(EnvParamForm,
			     IDEN_ANTI_ALIAS_OFF, IDEN_ANTI_ALIAS_BLEND,
			     IDEN_ANTI_ALIAS_OFF);
	    break;
	case IG_STATE_ANTI_ALIAS_ON:
	    CheckRadioButton(EnvParamForm,
			     IDEN_ANTI_ALIAS_OFF, IDEN_ANTI_ALIAS_BLEND,
			     IDEN_ANTI_ALIAS_ON);
	    break;
	case IG_STATE_ANTI_ALIAS_BLEND:
	    CheckRadioButton(EnvParamForm,
			     IDEN_ANTI_ALIAS_OFF, IDEN_ANTI_ALIAS_BLEND,
			     IDEN_ANTI_ALIAS_BLEND);
	    break;
    }
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
static void EnvParamDismissCb(void) 
{
    ShowWindow(EnvParamForm, SW_HIDE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements emulation of Motif EnvParam dialog.                         *
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
static BOOL CALLBACK EnvParamFormDlgProc(HWND hDlg,
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
	    if (hWnd != EnvNLenScale &&
		hWnd != EnvLWidthScale &&
		hWnd != EnvSensitiveScale &&
		hWnd != EnvLowresRatScale &&
		hWnd != EnvIsolinesScale &&
		hWnd != EnvPllnFineScale &&
		hWnd != EnvPlgnFineScale) {
	        clientId = (PVOID) LOWORD(wParam);        /* ID of the item. */
		ButtonsCallBack(hWnd, clientId, NULL);
	    }
	    return TRUE;
	case WM_HSCROLL:
            hWnd = (HWND) lParam;                 /* Handle of control HWND. */
	    if (hWnd == EnvNLenScale ||
	        hWnd == EnvLWidthScale ||
	        hWnd == EnvSensitiveScale ||
	        hWnd == EnvLowresRatScale ||
	        hWnd == EnvIsolinesScale ||
	        hWnd == EnvPllnFineScale ||
	        hWnd == EnvPlgnFineScale) {
		ScaleCB(hWnd, wParam, NULL);    
	    }
	    return TRUE;
	default:
	    return FALSE;
    }
}
