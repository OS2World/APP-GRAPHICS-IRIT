/*****************************************************************************
*   A windows NT interface for animation (based on the motif interface).     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Michael Plavnik			       Ver 0.1, Sept. 1995.  *
* Written by:  Iris Steinvarts			       Ver 0.1, March 1995.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "wntdrvs.h"

#define BUTTONSNUM		20
#define ANIM_SCALE_CONST	100000.0

typedef enum {
    MOVIE_MIN,
    MOVIE_MAX,
    MOVIE_DT,
    MOVIE_FAST_DT
}
MovieScaleType;

typedef struct PromptDialogStruct {
    char *Title;
    char *SelectionLabel;
    void (*OkCB)(HWND, PVOID, PVOID);
    PVOID ClientData;
} PromptDialogStruct;

IRIT_STATIC_DATA double
    MovieFastDt = 0.05;
IRIT_STATIC_DATA int
    MovieSingleStep = FALSE;

IRIT_STATIC_DATA HWND 
    AnimationForm, 
    AnimationScale, 
    AnimationButtons[BUTTONSNUM];

static void MovieRewindCb(void);
static void MovieSaveGeomCb(HWND Parent);
static void MovieSaveImageCb(HWND Parent);
static void MovieDismissCb(void);
static void MoviePlayBackCb(void);
static void MovieStopCb(void);
static void MoviePlayCb(void);
static void MovieFastForwardCb(void);
static void MovieRestartCb(void);
static void MovieFastDtCb(void);
static void MovieDtCb(void);
static void MovieScaleCb(float ScaleValue);

static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData);
static void InitButtons(void);
static HWND CreateButton(int idButton, int idBitmap);
static void ButtonsCallBack(HWND hWnd,
			    PVOID ClientData,
			    PVOID *cbs);
static void IntervalDtCB(HWND Parent);
static void SingleStepCB(HWND Parent);
static void FastIntervalDtCB(HWND Parent);
static void MinTimeCB(HWND Parent);
static void MaxTimeCB(HWND Parent);
static void ReadValue(HWND HWND,
		      PVOID ClientData,
		      PVOID CallData);
static void NewValue(HWND HWND,
		     PVOID ClientData,
		     PVOID CallData);
static int IsNumber(char *Buf, double *Value);
static void DoSingleStepAnimation(GMAnimationStruct *Anim,
				  IPObjectStruct *PObj);
static BOOL CALLBACK PromptDlgProc(HWND hDlg,
				   UINT uMsg,
				   WPARAM wParam,
				   LPARAM lParam);
static LRESULT CALLBACK ImageButtonWndProc(HWND hWnd,
					   UINT msg,
					   WPARAM wParam,
					   LPARAM lParam);
static BOOL CALLBACK AnimationFormDlgProc(HWND hDlg,
					  UINT uMsg,
					  WPARAM wParam,
					  LPARAM lParam);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main animation window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell HWND (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateAnimation	                                                     M
*****************************************************************************/
void CreateAnimation(HWND IGTopLevel)
{
    GMAnimResetAnimStruct(&IGAnimation);
    GMAnimFindAnimationTime(&IGAnimation, IGGlblDisplayList);
    IGAnimation.TextInterface = FALSE;

    /* Animation is child of IGTopLevel and gets WM_DESTROY message from it */
    AnimationForm = CreateDialog(GetWindowInstance(IGTopLevel),
                                 MAKEINTRESOURCE(IDD_ANIMATION_FORM),
                                 IGTopLevel,
                                 AnimationFormDlgProc);

    AnimationScale = GetDlgItem(AnimationForm, IDC_TRACKBAR_SCALE);
    SendMessage(AnimationScale, SBM_SETRANGE,
		(int) (ANIM_SCALE_CONST * IGAnimation.StartT),
		(int) (ANIM_SCALE_CONST * IGAnimation.FinalT));
    SendMessage(AnimationScale, SBM_SETPOS,
		(int) (ANIM_SCALE_CONST * IGAnimation.StartT), TRUE);

    CheckDlgButton(AnimationForm, IDC_TWO_WAYS,
		   IGAnimation.TwoWaysAnimation ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(AnimationForm, IDC_REPEAT,
		   IGAnimation.NumOfRepeat > 1 ? BST_CHECKED : BST_UNCHECKED);
                                 
    InitButtons();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initialize the buttons.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InitButtons(void)
{
    unsigned int
	n = 0;

    AnimationButtons[n++] = CreateButton(IDC_BUTTON_REWIND, IDB_BITMAP_REWIND);
    AnimationButtons[n++] = CreateButton(IDC_BUTTON_PLAYBACKWARD,
					 IDB_BITMAP_PLAYBACKWARD);
    AnimationButtons[n++] = CreateButton(IDC_BUTTON_STOP, IDB_BITMAP_STOP);
    AnimationButtons[n++] = CreateButton(IDC_BUTTON_PLAY, IDB_BITMAP_PLAY);
    AnimationButtons[n++] = CreateButton(IDC_BUTTON_FORWARD,
					 IDB_BITMAP_FORWARD);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create one button, using a button id and image as parameters.            *
*                                                                            *
* PARAMETERS:                                                                *
*   idButton: Button id.                                                     *
*   idBitmap: Button bitmap id.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   HWND:                                                                    *
*****************************************************************************/
static HWND CreateButton(int idButton, int idBitmap)
{
    HINSTANCE
        hInst = GetWindowInstance(AnimationForm);
    HWND
	Button = GetDlgItem(AnimationForm, idButton);    
    HBITMAP
	Bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(idBitmap));

    SubclassWindow(Button, ImageButtonWndProc);
    SetWindowLong(Button, GWL_USERDATA, (long) Bitmap);
    return Button;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of scale.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Scale:       The HWND to handle.                                         *
*   wParam:      Slider info,                                                *
*   CallData:    Holds the scale's value.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScaleCB(HWND Scale, WPARAM wParam, PVOID CallData)
{
    LONG Dummy,
	Value = SendMessage(Scale, SBM_GETPOS, 0, 0);
    SCROLLINFO LPSi;

    switch (LOWORD(wParam)) {
	case SB_LINERIGHT:
	    Value += (LONG) (ANIM_SCALE_CONST / 10000);
	    break;
	case SB_LINELEFT:
	    Value -= (LONG) (ANIM_SCALE_CONST / 10000);
	    break;
	case SB_PAGERIGHT:
	    Value += (LONG) (ANIM_SCALE_CONST / 100);
	    break;
	case SB_PAGELEFT:
	    Value -= (LONG) (ANIM_SCALE_CONST / 100);
	    break;
	case SB_TOP:
	    SendMessage(AnimationScale, SBM_GETRANGE,
			(WPARAM) &Value, (LPARAM) &Dummy);
	    break;
	case SB_BOTTOM:
	    SendMessage(AnimationScale, SBM_GETRANGE,
			(WPARAM) &Dummy, (LPARAM) &Value);
	    break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
	    LPSi.cbSize = sizeof(SCROLLINFO);
	    LPSi.fMask = SIF_RANGE | SIF_TRACKPOS;
	    GetScrollInfo(Scale, SB_CTL, &LPSi);
	    Value = LPSi.nTrackPos;
	    break;
    }

    SendMessage(Scale, SBM_SETPOS, (int) Value, TRUE);
    MovieScaleCb((float) (Value / ANIM_SCALE_CONST));
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
	    MovieDismissCb();
	    break;
	case IDC_BUTTON_INTERVALDT:
	    IntervalDtCB(hWnd);
	    break;
	case IDC_BUTTON_FASTINTERVALDT:
	    FastIntervalDtCB(hWnd);
	    break;
	case IDC_BUTTON_SINGLESTEP:
	    SingleStepCB(hWnd);
	    break;
	case IDC_BUTTON_MINTIME:
	    MinTimeCB(hWnd);
	    break;
	case IDC_BUTTON_MAXTIME:
	    MaxTimeCB(hWnd);
	    break;
	case IDC_BUTTON_PLAYBACKWARD:
	    MoviePlayBackCb();
	    break;
	case IDC_BUTTON_REWIND:
	    MovieRewindCb();
	    break;
	case IDC_BUTTON_SAVE_GEOM:
	    MovieSaveGeomCb(hWnd);
	    break;
	case IDC_BUTTON_SAVE_IMAGE:
	    MovieSaveImageCb(hWnd);
	    break;
	case IDC_BUTTON_STOP:
	    MovieStopCb();
	    break;
	case IDC_BUTTON_PLAY:
	    MoviePlayCb();
	    break;
	case IDC_BUTTON_FORWARD:
	    MovieFastForwardCb();
	    break;
	case IDC_BUTTON_RESTART:
	    MovieRestartCb();
	    break;
	case IDC_TWO_WAYS:
	    IGAnimation.TwoWaysAnimation = !IGAnimation.TwoWaysAnimation;
	    CheckDlgButton(AnimationForm, IDC_TWO_WAYS,
		IGAnimation.TwoWaysAnimation ? BST_CHECKED : BST_UNCHECKED);
	    break;
	case IDC_REPEAT:
	    if (IGAnimation.NumOfRepeat == 1)
		IGAnimation.NumOfRepeat = 32767;
	    else
		IGAnimation.NumOfRepeat = 1;
	    CheckDlgButton(AnimationForm, IDC_REPEAT,
		IGAnimation.NumOfRepeat > 1 ? BST_CHECKED : BST_UNCHECKED);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the interval button.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:        HWND.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IntervalDtCB(HWND Parent)
{
    char Buf[BUFSIZ];

    sprintf(Buf, "Current Dt value is %0.4f\nEnter New Interval Dt Value:",
	    IGAnimation.Dt); 
    PromptDialogBox(Parent, "New Dt", Buf, ReadValue, (PVOID) MOVIE_DT);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the fast interval button.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:        HWND.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FastIntervalDtCB(HWND Parent)
{
    char Buf[BUFSIZ];
    sprintf(Buf, "Current Fast Dt value is %0.4f\nEnter New Fast Interval Dt Value:",
	    MovieFastDt); 
    PromptDialogBox(Parent, "New Fast Dt", Buf, ReadValue,
		    (PVOID) MOVIE_FAST_DT);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for single step button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   SingleStepCB                                                             *
*****************************************************************************/
static void SingleStepCB(HWND Parent)
{
    if (MovieSingleStep = !MovieSingleStep) {
	SetWindowText(Parent, "SStp");
    }
    else {
	SetWindowText(Parent, "Cont");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the minimum time button.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:        HWND.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MinTimeCB(HWND Parent)
{
    int ScaleValue, Dummy;
    char Buf[BUFSIZ];

    SendMessage(AnimationScale, SBM_GETRANGE,
		(WPARAM) &ScaleValue, (LPARAM) &Dummy);
    sprintf(Buf, "Minimum time is %0.4f\nEnter new minimum time value:",
	    ScaleValue / ANIM_SCALE_CONST); 
    PromptDialogBox(Parent, "Update minimum time", Buf, NewValue,
		    (PVOID) MOVIE_MIN);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the maximum time button.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:        HWND.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MaxTimeCB(HWND Parent)
{
    int ScaleValue, Dummy;
    char Buf[BUFSIZ];

    SendMessage(AnimationScale, SBM_GETRANGE,
		(WPARAM) &Dummy, (LPARAM) &ScaleValue);
    sprintf(Buf, "Maximum time is %0.4f\nEnter new maximum time value:",
	    ScaleValue / ANIM_SCALE_CONST); 
    PromptDialogBox(Parent, "Update maximum time",  Buf, NewValue,
		    (PVOID) MOVIE_MAX);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets one real value from user.                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   HWND:         Not used.                                                  *
*   ClientData:   Is this for fast or regular motion!?                       *
*   CallData:     To get a handle on string.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReadValue(HWND widget, PVOID ClientData, PVOID CallData)
{
    char Buf[BUFSIZ];
    double Value;

    if (!GetDlgItemText((HWND) CallData, IDC_EDIT_SELECTION, Buf, BUFSIZ-1)) {
        IGIritError("Read Value Error: Cannot convert compound String");
        return;
    }

    if (IsNumber(Buf, &Value)) {
	if (((int) ClientData) == MOVIE_FAST_DT) {
	    MovieFastDt = Value;
	    MovieFastDtCb();
	}
	else {
	    IGAnimation.Dt = Value;
	    MovieDtCb();
	}
    }
    else
        DisplayErrorMsg();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets one real value from user.                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   HWND:         Not used.                                                  *
*   ClientData:   Is this for fast or regular motion!?                       *
*   CallData:     To get a handle on string.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void NewValue(HWND widget, PVOID ClientData, PVOID CallData)
{
    int ScaleValue, Dummy;
    double Value;
    char Buf[BUFSIZ];

    if (!GetDlgItemText((HWND) CallData, IDC_EDIT_SELECTION, Buf, BUFSIZ - 1)) {
        IGIritError("Read Value Error: Cannot convert compound String");
        return;
    }

    if (!IsNumber(Buf, &Value)) {
        DisplayErrorMsg();
    }
    else {
        char StrTime[IRIT_LINE_LEN];

	Value = ANIM_SCALE_CONST * Value;
	if (Value != (int) Value) {
	    DisplayWrnMsg();
	    Value = (int) Value;
	}

	if (((int) ClientData) == MOVIE_MIN) {
	    SendMessage(AnimationScale, SBM_GETRANGE,
			(WPARAM) &Dummy, (LPARAM) &ScaleValue);
	    if ((double) ScaleValue <= Value) 
	        DisplayErrValue("Max");
	    else {
		IGAnimation.StartT = Value / ANIM_SCALE_CONST;
		if (IGAnimation.RunTime < IGAnimation.StartT)
		    IGAnimation.RunTime = IGAnimation.StartT;
		SendMessage(AnimationScale, SBM_SETRANGE,
			    (int) Value, ScaleValue);
	    }
	}
	else {
	    SendMessage(AnimationScale, SBM_GETRANGE,
			(WPARAM) &ScaleValue, (LPARAM) &Dummy);
	    if ((double) ScaleValue >= Value)
	        DisplayErrValue("Min");
	    else {
		IGAnimation.FinalT = Value / ANIM_SCALE_CONST;
		if (IGAnimation.RunTime > IGAnimation.FinalT)
		    IGAnimation.RunTime = IGAnimation.FinalT;
		SendMessage(AnimationScale, SBM_SETRANGE, ScaleValue,
			    (int) Value);
	    }
	}

	SendMessage(AnimationScale, SBM_SETPOS,
		    (int) (IGAnimation.RunTime * ANIM_SCALE_CONST), TRUE);

	sprintf(StrTime, "Time = %.5f", IGAnimation.RunTime);
	SetDlgItemText(AnimationForm, ID_STATIC_SCALETITLE1, StrTime);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Error messages for input of numbers.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DisplayErrorMsg                                                          M
*****************************************************************************/
void DisplayErrorMsg(void)
{
    MessageBox(IGhTopLevel, "Value is not a number.", "Error",
	       MB_OK | MB_ICONSTOP);    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Warning messages for input of numbers.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DisplayWrnMsg                                                            M
*****************************************************************************/
void DisplayWrnMsg(void)
{
    MessageBox(IGhTopLevel, 
               "Only 4 digits after decimal point. Value will be rounded.", 
               "Warning", 
               MB_OK | MB_ICONEXCLAMATION);    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Error messages for input of numbers.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Msg:   To print.                                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   DisplayErrValue                                                          M
*****************************************************************************/
void DisplayErrValue(char *Msg)
{
    Msg = (strcmp(Msg, "Max") == 0) 
                    ? "Maximum time <= new given minimum"
                    : "Minimum time >= new given maximum";
    MessageBox(IGhTopLevel, Msg, "Error", MB_OK | MB_ICONSTOP);    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verifies and converts a string into a numeric value.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Buf:    To convert to a number.                                          *
*   Value:  Numeric result.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE is is a number, FALSE otherwise.                             *
*****************************************************************************/
static int IsNumber(char *Buf, double *Value)
{
    return sscanf(Buf, "%lf", Value);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation? Senses the event queue of X.              M
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
    MSG Event;

    UpdateWindow(IGhWndView);
    while (PeekMessage(&Event, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE) ||
	   PeekMessage(&Event, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
	TranslateMessage(&Event);
	DispatchMessage(&Event);    
    }

    return Anim -> StopAnim;
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
*   AnimationCB		                                                     M
*****************************************************************************/
void AnimationCB(void)
{  
    RECT Wr, Dr, Cr;      

    GetWindowRect(GetDesktopWindow(), &Dr); 
    GetWindowRect(IGhTopLevel, &Wr);    
    GetClientRect(AnimationForm, &Cr);  /* Get form size */
    SetWindowPos(AnimationForm, 
		 HWND_TOP,
		 min(Wr.right + 3, Dr.right - Cr.right - 3),
		 Wr.top,
		 0, 
		 0,
		 SWP_NOSIZE | SWP_SHOWWINDOW);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Executes a single animation step according to Anim info an PObj geom.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Anim:    Prescription of the animation step to perform.                  *
*   PObj:    Current geometry to display.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DoSingleStepAnimation(GMAnimationStruct *Anim,
				  IPObjectStruct *PObj)
{
    char StrTime[IRIT_LINE_LEN];
    IGAnimation.StopAnim = FALSE;

    IGGlblAnimation = TRUE;

    GMAnimDoSingleStep(Anim, PObj);

    IGGlblAnimation = FALSE;

    if (IGAnimation.RunTime > IGAnimation.FinalT)
	IGAnimation.RunTime = IGAnimation.FinalT;
    if (IGAnimation.RunTime < IGAnimation.StartT)
	IGAnimation.RunTime = IGAnimation.StartT;

    SendMessage(AnimationScale, SBM_SETPOS,
		(int) (IGAnimation.RunTime * ANIM_SCALE_CONST), TRUE);

    sprintf(StrTime, "Time = %.5f", IGAnimation.RunTime);
    SetDlgItemText(AnimationForm, ID_STATIC_SCALETITLE1, StrTime);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for play back button.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MoviePlayBackCb(void)
{
    char StrTime[IRIT_LINE_LEN];

    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime -= IGAnimation.Dt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

        for ( ;
	     IGAnimation.RunTime >= IGAnimation.StartT + IRIT_EPS &&
	     !IGAnimation.StopAnim;
	     IGAnimation.RunTime -= IGAnimation.Dt) {
    	    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	}
    }

    IGGlblAnimation = FALSE;

    sprintf(StrTime, "Time = %.5f", IGAnimation.RunTime);
    SetDlgItemText(AnimationForm, ID_STATIC_SCALETITLE1, StrTime);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for rewind button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieRewindCb(void)
{
    char StrTime[IRIT_LINE_LEN];

    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime -= MovieFastDt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

        for ( ;
	     IGAnimation.RunTime >= IGAnimation.StartT - IRIT_EPS &&
	     !IGAnimation.StopAnim;
	     IGAnimation.RunTime -= MovieFastDt) {
    	    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	}
    }

    sprintf(StrTime, "Time = %.5f", IGAnimation.RunTime);
    SetDlgItemText(AnimationForm, ID_STATIC_SCALETITLE1, StrTime);

    IGGlblAnimation = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for save button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:        HWND.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieSaveGeomCb(HWND Parent)
{
    if (IGAnimation.SaveAnimationGeom = !IGAnimation.SaveAnimationGeom) {
	SetWindowText(Parent, "Sv Geom");
    }
    else {
	SetWindowText(Parent, "No Geom");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for save button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:        HWND.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieSaveImageCb(HWND Parent)
{
    if (!IGAnimation.SaveAnimationImage) {
	static char FileName[IRIT_LINE_LEN_LONG];
        OPENFILENAME ofn;
	char TmpFileName[IRIT_LINE_LEN_LONG], *Fn, *Ft;

	strcpy(TmpFileName, "");
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = Parent;
	ofn.lpstrFile = TmpFileName;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFile = IRIT_LINE_LEN_LONG;
	ofn.lpstrTitle = "Save Animation Images";
	ofn.lpstrFilter = "";
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn)) {
	    SetWindowText(Parent, "Sv Image");
	    IGAnimation.SaveAnimationImage = TRUE;

	    Fn = TmpFileName;
	    if ((Ft = strrchr(TmpFileName, '.')) != NULL) {
	        *Ft = 0;
		Ft++;
	    }
	    else
	        Ft = "ppm";

	    /* Format the file names as "Fname%04d.Ftype". */ 
	    sprintf(FileName, "%s%s.%s", Fn, "%04d", Ft);
	    IGGlblImageFileName = FileName;
	}
    }
    else {
	SetWindowText(Parent, "No Image");
	IGAnimation.SaveAnimationImage = FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for stop button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieStopCb(void) 
{
    IGAnimation.StopAnim = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for play button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MoviePlayCb(void) 
{
    char StrTime[IRIT_LINE_LEN];

    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime += IGAnimation.Dt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

	do {
	    for ( ;
		 IGAnimation.RunTime <= IGAnimation.FinalT + IRIT_EPS &&
		 !IGAnimation.StopAnim;
		 IGAnimation.RunTime += IGAnimation.Dt) {
	        DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	    }

	    if (IGAnimation.TwoWaysAnimation) {
	        for ( ;
		     IGAnimation.RunTime >= IGAnimation.StartT - IRIT_EPS &&
		     !IGAnimation.StopAnim;
		     IGAnimation.RunTime -= IGAnimation.Dt) {
		    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
		}
	    }

	    if (IGAnimation.NumOfRepeat > 1) {
	        IGAnimation.NumOfRepeat--;
		IGAnimation.RunTime = IGAnimation.StartT;
	    }
	}
	while (IGAnimation.NumOfRepeat > 1 && !IGAnimation.StopAnim);
    }

    IGGlblAnimation = FALSE;

    sprintf(StrTime, "Time = %.5f", IGAnimation.RunTime);
    SetDlgItemText(AnimationForm, ID_STATIC_SCALETITLE1, StrTime);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for fast forward button. 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieFastForwardCb(void) 
{
    char StrTime[IRIT_LINE_LEN];

    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime += MovieFastDt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

        for ( ;
	     IGAnimation.RunTime <= IGAnimation.FinalT + IRIT_EPS &&
	     !IGAnimation.StopAnim;
	     IGAnimation.RunTime += MovieFastDt) {
    	    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	}
    }

    IGGlblAnimation = FALSE;

    sprintf(StrTime, "Time = %.5f", IGAnimation.RunTime);
    SetDlgItemText(AnimationForm, ID_STATIC_SCALETITLE1, StrTime);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for restart button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieRestartCb(void) 
{
    IGAnimation.RunTime = IGAnimation.StartT;
    MoviePlayCb();
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
static void MovieDismissCb(void) 
{
    ShowWindow(AnimationForm, SW_HIDE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for set fast Dt button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieFastDtCb(void)
{
    IGAnimation.Dt = MovieFastDt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for set Dt button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieDtCb(void)
{
    IGAnimation.Dt = IGAnimation.Dt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for set scale animation value. 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   ScaleValue:     Value of scale to update animation with.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieScaleCb(float ScaleValue)
{
    IGAnimation.RunTime = ScaleValue;

    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements emulation of Motif prompt dialog.                             *
*   Is Win32 dialog callback function.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   hDlg:   Handle of the dialog that calls the function.                    **
*   uMsg:   Message sent by the dialog.                                      *
*   wParam: First parameter.                                                 *
*   lParam: Second parameter.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   BOOL: returns true when message is processed completely.                 *
*****************************************************************************/
static BOOL CALLBACK PromptDlgProc(HWND hDlg,
				   UINT uMsg,
				   WPARAM wParam,
				   LPARAM lParam)
{
    PromptDialogStruct *p;

    switch (uMsg) {
	case WM_INITDIALOG:
            if (lParam) {
	        p = (PromptDialogStruct *) lParam;
		if (p -> Title)
		    SetWindowText(hDlg, p -> Title);
		if (p -> SelectionLabel)
		    SetDlgItemText(hDlg, IDC_STATIC_SELECTIONLABEL,
				   p -> SelectionLabel);
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		SetFocus(GetDlgItem(hDlg, IDC_EDIT_SELECTION));
	    }
	    return FALSE;
	case WM_COMMAND:
	    switch (LOWORD(wParam)) {
	        case IDOK:
		    p = (PromptDialogStruct *)
				        GetWindowLong(hDlg, GWL_USERDATA); 
		    (*p -> OkCB)((HWND) lParam, p -> ClientData, hDlg);
		case IDCANCEL:
		    EndDialog(hDlg, LOWORD(wParam));
		    break;
	    }
	    return TRUE;
	default:
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Runs emulation of Motif prompt dialog.                                   M
*   Is Win32 dialog callback function.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Parent:     Handle to the parent window.                                 M
*   Title:      Title of the prompt dialog.                                  M
*   SelectionLabel: Label that marks edit field.                             M
*   OkCB:       Function pointer to OK button handler.                       M
*   ClientData: Pointer to the data passed to OkCB.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Returns TRUE when message is processed completely.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   PromptDialogBox                                                          M
*****************************************************************************/
int PromptDialogBox(HWND Parent,
		    char *Title,
		    char *SelectionLabel,
		    void (*OkCB)(HWND, PVOID, PVOID),
		    PVOID ClientData)
{
    PromptDialogStruct Args;

    Args.Title = Title;
    Args.SelectionLabel = SelectionLabel;
    Args.OkCB = OkCB;
    Args.ClientData = ClientData;
    return DialogBoxParam(NULL, 
                          MAKEINTRESOURCE(IDD_PROMPT), 
                          Parent, 
                          PromptDlgProc, 
                          (LPARAM) &Args);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements emulation of Motif animation dialog.                          *
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
static BOOL CALLBACK AnimationFormDlgProc(HWND hDlg,
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
	    if (hWnd != AnimationScale) {
	        clientId = (PVOID) LOWORD(wParam);        /* ID of the item. */
		ButtonsCallBack(hWnd, clientId, NULL);
	    }
	    return TRUE;
	case WM_HSCROLL:
            hWnd = (HWND) lParam;                 /* Handle of control HWND. */
	    if (hWnd == AnimationScale) {
		ScaleCB(hWnd, wParam, NULL);    
	    }
	    return TRUE;
	default:
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements Motif button with image on it. (We could not use OCX).        *
*   That button is a subclassed version of standard button.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   hWnd:   Handle of the button.                                            *
*   msg:    Message recieved.                                                *
*   wParam: 1st parameter.                                                   *
*   lParam: 2nd parameter.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   LRESULT: true when function completely processes the message.            *
*****************************************************************************/
static LRESULT CALLBACK ImageButtonWndProc(HWND hWnd,
					   UINT msg,
					   WPARAM wParam,
					   LPARAM lParam)
{
    HDC hdcWin, hdcMem;
    HGDIOBJ hOld;
    HBITMAP hBitmap;
    BITMAP bmpData;
    RECT rect;
    LRESULT Result;
    WNDPROC
	WndProc = (WNDPROC) GetClassLong(hWnd, GCL_WNDPROC);
    unsigned
	Offset = 0;

    switch (msg) {
	case WM_DESTROY:
	    DeleteBitmap(GetWindowLong(hWnd, GWL_USERDATA));
	    return CallWindowProc(WndProc, hWnd, msg, wParam, lParam);
	case BM_SETSTATE:
            if (wParam) 
                Offset = 1;
	case WM_PAINT:
            Result = CallWindowProc(WndProc, hWnd, msg, wParam, lParam);
	    hdcWin = GetDC(hWnd);
	    hdcMem = CreateCompatibleDC(hdcWin); 
	    hBitmap = (HBITMAP) GetWindowLong(hWnd, GWL_USERDATA);
	    hOld = SelectBitmap(hdcMem, hBitmap);
	    GetClientRect(hWnd, &rect);
	    GetObject(hBitmap, sizeof(bmpData), &bmpData);
	    rect.left = max(0, (rect.right - bmpData.bmWidth) / 2) + Offset;
	    rect.top = max(0, (rect.bottom - bmpData.bmHeight) / 2) + Offset;
	    BitBlt(hdcWin, rect.left, rect.top, bmpData.bmWidth,
		   bmpData.bmHeight, hdcMem, 0, 0, SRCAND);
	    SelectObject(hdcMem, hOld);
	    DeleteDC(hdcMem);
	    ReleaseDC(hWnd, hdcWin);
	    return Result;       
    };
    return CallWindowProc(WndProc, hWnd, msg, wParam, lParam);
}
