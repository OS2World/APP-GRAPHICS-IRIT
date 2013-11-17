/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Feb. 1995   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Global definitions of	xmtdrvs interface.			             *
*****************************************************************************/

#ifndef	XMTDRVS_H	/* Define only once */
#define	XMTDRVS_H

#define XTP			XtPointer
#define XTC			XtCallbackProc

#include "x11drvs.h"

IRIT_GLOBAL_DATA_HEADER Widget IGTopLevel, MainTransForm, ShadeParamForm,
	PickParamForm, CrvEditForm, SrfEditForm, ObjManipParamForm;

/* xmtdrvs.c */
Widget CreateSubForm(Widget Parent, int Pos);
Widget AddButton(Widget Parent,
		 char *Label,
		 int Pos,
		 XTC FuncPtr,
		 XTP FuncEvent);
void AddTwoButtons(Widget Parent,
		   int Pos,
		   char *Label1,
		   XTC FuncPtr1,
		   XTP FuncEvent1,
		   char *Label2,
		   XTC FuncPtr2,
		   XTP FuncEvent2,
		   Widget *Btn1,
		   Widget *Btn2);
void AddThreeButtons(Widget Parent,
		   int Pos,
		   char *Label1,
		   XTC FuncPtr1,
		   XTP FuncEvent1,
		   char *Label2,
		   XTC FuncPtr2,
		   XTP FuncEvent2,
		   char *Label3,
		   XTC FuncPtr3,
		   XTP FuncEvent3,
		   Widget *Btn1,
		   Widget *Btn2,
		   Widget *Btn3);
Widget AddRadioButton(Widget Parent,
		      char *Header,
		      int DefVal,
		      char **Labels,
		      int n,
		      int Pos,
		      XTC FuncPtr);
Widget AddCheckButton(Widget Parent,
		      char *Header,
		      int DefVal,
		      char **Labels,
		      int n,
		      int Pos,
		      XTC FuncPtr);
Widget AddLabel(Widget Parent, char *Label, int Pos);
Widget AddSlide(Widget Parent,
		int Pos,
		int Min,
		int Max,
		XTC ScaleFuncPtr,
		XTC DragFuncPtr,
		XTP FuncEvent);
Widget AddButtonSlide(Widget Parent,
		      int Pos,
		      char *BtnLabel,
		      XTC BtnFuncEvent,
		      XTP BtnFuncPtr,
		      int SlMin,
		      int SlMax,
		      XTC SlScaleFuncPtr,
		      XTC SlDragFuncPtr,
		      XTP SlFuncEvent);
void SetLabel(Widget  w, char *NewLabel);
void ViewWndwInputHandler(XEvent *Event);
void CreatePopupWindow(Widget w);
void CreateControlPanel(Widget TopLevel);

/* xmt_anim.c */
void CreateAnimation(Widget w);
void AnimationCB(void);
void ReplaceLabel(Widget w, char *NewLabel);

/* xmt_shad.c */
void CreateShadeParam(Widget IGTopLevel);

/* xmt_pick.c */
void CreatePickParam(Widget IGTopLevel);

/* xmt_crvs.c */
void CreateCrvEdit(Widget IGTopLevel);
void IGCrvEditParamUpdateMRScale(void);

/* xmt_srfs.c */
void CreateSrfEdit(Widget IGTopLevel);
void IGSrfEditParamUpdateMRScale(void);

/* xmt_manp.c */
void CreateObjManipParam(Widget IGTopLevel);

#endif /* XMTDRVS_H */
