/*****************************************************************************
*   Magellan/SpaceMouse interface support.                                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Oleg Ilushin				Ver 0.1, Dec 2002.   *
*****************************************************************************/

#include "magellan.h"
#include "grap_loc.h"

IRIT_STATIC_DATA IrtRType ScalingFactors[] = 
		        { 0.0001, 0.0001, -0.0001, 0.0005, 0.0005, -0.0005 };

static void SpaceMouseHandleMotionEvent(MagellanSpaceMouseDataStruct *pData,
					MSG *pMsg);
static void SpaceMouseHandleButtonEvent(MagellanSpaceMouseDataStruct *pData,
					MSG *pMsg);
static void SpaceMouseInitControls(MagellanSpaceMouseDataStruct *pData);
static void SpaceMouseFilterDominantDir(IrtRType *motionData);
static void SpaceMouseHelp(void);

/*****************************************************************************
* DESCRIPTION:								     M
* Attemp to connect to the SpaceMouse device. Should be always called first. M
*									     *
* PARAMETERS:								     M
*   pData:       SpaceMouse data to be initialized.			     M
*   hWndFrame:   Handle to the application window.                           M
*									     *
* RETURN VALUE:								     M
*   int:         TRUE if pData was initialized successfully,		     M
*                FALSE otherwise.					     M
* 									     *
* SEE ALSO:     							     M
*   MagellanCloseSpaceMouseDevice, MagellanSpaceMouseHandleEvent	     M
* 									     *
* KEYWORDS:								     M
*   MagellanInitSpaceMouseDevice					     M
*****************************************************************************/
int MagellanInitSpaceMouseDevice(MagellanSpaceMouseDataStruct *pData,
				 HWND hWndFrame)
{
    int Status = TRUE;

#ifdef IRIT_HAVE_MAGELLAN_LIB
    SiInitialize();
    SiOpenWinInit(&(pData -> oData), hWndFrame);

    if ((pData -> hdl = SiOpen("wntgdrvs", SI_ANY_DEVICE, SI_NO_MASK,
			       SI_EVENT, &(pData -> oData))) == SI_NO_HANDLE) {
        Status = FALSE;
        SiTerminate();
    }
    else
        SpaceMouseInitControls(pData);
#endif /* IRIT_HAVE_MAGELLAN_LIB */

    return Status;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Terminate connection to the SpaceMouse device.			     M
*									     *
* PARAMETERS:								     M
*   pData:    SpaceMouse data.						     M
*									     *
* RETURN VALUE:								     M
*   void								     M
* 									     *
* SEE ALSO:     							     M
*   MagellanInitSpaceMouseDevice,  MagellanSpaceMouseHandleEvent	     M
* 									     *
* KEYWORDS:								     M
*   MagellanCloseSpaceMouseDevice					     M
*****************************************************************************/
void MagellanCloseSpaceMouseDevice(MagellanSpaceMouseDataStruct *pData)
{
#ifdef IRIT_HAVE_MAGELLAN_LIB
    SiClose(pData -> hdl);
    SiTerminate();
#endif /* IRIT_HAVE_MAGELLAN_LIB */
}

/*****************************************************************************
* DESCRIPTION:								     M
* Process SpaceMouse event (Motion/Button).				     M
*									     *
* PARAMETERS:								     M
*   pData:    SpaceMouse data.						     M
*   pMsg:     Message to process.					     M
*   									     *
* RETURN VALUE:								     M
*   int:  Status report.						     M
* 									     *
* SEE ALSO:     							     M
*   MagellanInitSpaceMouseDevice, MagellanCloseSpaceMouseDevice 	     M
* 									     *
* KEYWORDS:								     M
*   MagellanSpaceMouseHandleEvent					     M
*****************************************************************************/
int MagellanSpaceMouseHandleEvent(MagellanSpaceMouseDataStruct *pData,
				  MSG *pMsg)
{
    int Status = FALSE;

#ifdef IRIT_HAVE_MAGELLAN_LIB
    SiGetEventWinInit(&(pData -> eData), pMsg -> message,
		      pMsg -> wParam, pMsg -> lParam);

    /* The event was a spaceMouse event */
    if (SiGetEvent(pData -> hdl, 0, &(pData -> eData),
		   &(pData -> siEvent)) == SI_IS_EVENT) {
        Status = TRUE;
	
	if (pData -> siEvent.type == SI_MOTION_EVENT)
	    SpaceMouseHandleMotionEvent(pData, pMsg);
	else if (pData -> siEvent.type == SI_BUTTON_EVENT)
	    SpaceMouseHandleButtonEvent(pData, pMsg);
    }
#endif /* IRIT_HAVE_MAGELLAN_LIB */

    return Status;
}

/*****************************************************************************
* DESCRIPTION:								     *
* Process SpaceMouse Motion event.					     *
*  									     *
* PARAMETERS:								     *
*   pData:    SpaceMouse data.						     *
*   pMsg :    Message to process.					     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void SpaceMouseHandleMotionEvent(MagellanSpaceMouseDataStruct *pData,
					MSG *pMsg)
{
#ifdef IRIT_HAVE_MAGELLAN_LIB
    int i,
    	NeedRedraw = FALSE;
    IrtRType MotionData[6];
    
    for (i = 0; i < 6; i++)
	MotionData[i] = 0.0;
    
    if (pData -> bAllowTranslation) {
	MotionData[SI_TX] = (IrtRType) pData -> siEvent.u.spwData.mData[SI_TX];
	MotionData[SI_TY] = (IrtRType) pData -> siEvent.u.spwData.mData[SI_TY];
	MotionData[SI_TZ] = (IrtRType) pData -> siEvent.u.spwData.mData[SI_TZ];
    }
    if (pData -> bAllowRotation) {
	MotionData[SI_RX] = (IrtRType) pData -> siEvent.u.spwData.mData[SI_RX];
	MotionData[SI_RY] = (IrtRType) pData -> siEvent.u.spwData.mData[SI_RY];
	MotionData[SI_RZ] = (IrtRType) pData -> siEvent.u.spwData.mData[SI_RZ];
    }
    if (pData -> bDominant)
	SpaceMouseFilterDominantDir(MotionData);
    
    for (i = 0; i < 6; i++) {
	MotionData[i] *= ScalingFactors[i] * pData -> Sensitivity;
	NeedRedraw = NeedRedraw || MotionData[i];
    }
    
    if (!NeedRedraw)
	return;
    
    if (MotionData[SI_TX])
	IGProcessEvent(IG_EVENT_TRANSLATE_X, &MotionData[SI_TX]);
    if (MotionData[SI_TY])
	IGProcessEvent(IG_EVENT_TRANSLATE_Y, &MotionData[SI_TY]);
    if (MotionData[SI_TZ])
	IGProcessEvent(IG_EVENT_TRANSLATE_Z, &MotionData[SI_TZ]);
    if (MotionData[SI_RX])
	IGProcessEvent(IG_EVENT_ROTATE_X, &MotionData[SI_RX]);
    if (MotionData[SI_RY])
	IGProcessEvent(IG_EVENT_ROTATE_Y, &MotionData[SI_RY]);
    if (MotionData[SI_RZ])
	IGProcessEvent(IG_EVENT_ROTATE_Z, &MotionData[SI_RZ]);
    
    IGRedrawViewWindow();

#endif /* IRIT_HAVE_MAGELLAN_LIB */
}

/*****************************************************************************
* DESCRIPTION:								     *
* Process SpaceMouse button event.					     *
*									     *
* PARAMETERS:								     *
*   pData:    SpaceMouse data.						     *
*   pMsg :    Message to process.					     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void SpaceMouseHandleButtonEvent(MagellanSpaceMouseDataStruct *pData,
					MSG *pMsg)
{
#ifdef IRIT_HAVE_MAGELLAN_LIB
    IRIT_STATIC_DATA char
	*BeepString = "CcC";
    int Button;
    
    if ((Button = SiButtonPressed(&(pData -> siEvent))) != SI_NO_BUTTON) {
	switch(Button) {
	    case 1: 		   /* Toggle enable translation mode on/off. */
		pData -> bAllowTranslation = !pData -> bAllowTranslation;
		SiBeep(pData -> hdl, BeepString);
		
		/* We don't want both trans and rotate off at the same time! */
		if (pData -> bAllowRotation == FALSE &&
		    pData -> bAllowRotation == FALSE)
		    pData -> bAllowRotation = TRUE;
		break;
	    
	    case 2:		      /* Toggle enable rotation mode on/off. */
		pData -> bAllowRotation = !pData -> bAllowRotation;
		SiBeep(pData -> hdl, BeepString);
		
		/* We don't want both trans and rotate off at the same time! */
		if (pData -> bAllowRotation == FALSE &&
		    pData -> bAllowRotation == FALSE)
		    pData -> bAllowTranslation = TRUE;
		break;
	    
	    case 3: 		   /* Toggle dominant direction mode on/off. */
		pData -> bDominant = !pData -> bDominant;
		SiBeep(pData -> hdl, BeepString);
		break;
	    
	    case 4: 			      /* Zero out SpaceNouse motion. */
		SiRezero(pData -> hdl);
		SiBeep(pData -> hdl, BeepString);
		break;
	    
	    case 5: 			     /* Increase motion sensitivity. */
		pData -> Sensitivity *= MAGELLAN_SENSITIVITY_FRACTION;
		SiBeep(pData -> hdl, BeepString);
		break;
		
	    case 6: 			     /* Decrease motion sensitivity. */
		pData -> Sensitivity /= MAGELLAN_SENSITIVITY_FRACTION;
		SiBeep(pData -> hdl, BeepString);
		break;
	    
	    case 7: 	/* Set SpaceMouse motion controls to their defaults. */
		SpaceMouseInitControls(pData);
		SiBeep(pData -> hdl, BeepString);
		break;
	    
	    case 8: 				/* Display Help message box. */
		SpaceMouseHelp();
		break;
	    
	    default:
		break;
	}
    }
#endif /* IRIT_HAVE_MAGELLAN_LIB */
}

/*****************************************************************************
* DESCRIPTION:								     *
* Initialize SpaceMouse motion controls.				     *
*									     *
* PARAMETERS:								     *
*   pData:    SpaceMouse data.						     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void SpaceMouseInitControls(MagellanSpaceMouseDataStruct *pData)
{
#ifdef IRIT_HAVE_MAGELLAN_LIB
    pData -> bAllowRotation    = TRUE;
    pData -> bAllowTranslation = TRUE;
    pData -> bDominant         = FALSE;
    pData -> Sensitivity       = 1.0;
#endif /* IRIT_HAVE_MAGELLAN_LIB */
}

/*****************************************************************************
* DESCRIPTION:								     *
* Filter out dominant motion direction (translation/rotation).		     * 
*									     *
* PARAMETERS:								     *
*   MotionData:    Array of size 6 wich dominant value will be filtered out. *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void SpaceMouseFilterDominantDir(IrtRType *MotionData)
{
#ifdef IRIT_HAVE_MAGELLAN_LIB
    int i,
    	iDom = 0;
    IrtRType
	DomDir = MotionData[0];
    
    for (i = 1; i < 6; i++) {
	if (IRIT_FABS(DomDir) < IRIT_FABS(MotionData[i])) {
	   MotionData[iDom] = 0.0;
	   DomDir = MotionData[i];
	   iDom = i;
	}
	else
	    MotionData[i] = 0;
    }
#endif /* IRIT_HAVE_MAGELLAN_LIB */
}

/*****************************************************************************
* DESCRIPTION:								     *
* Displays Help Message Box.						     *
*									     *
* PARAMETERS:								     *
*   None								     *
*									     *
* RETURN VALUE:								     *
*   void								     *
*****************************************************************************/
static void SpaceMouseHelp(void)
{
    IRIT_STATIC_DATA char
	*Title  = "Special Button Functions";
    IRIT_STATIC_DATA char
	*hlpStr = " Button 1 = Toggle Translation (on)/off\n"
		  " Button 2 = Toggle Rotation (on)/off\n"
		  " Button 3 = Toggle Dominant Direction Filtering on/(off)\n"
		  " Button 4 = Rezero SpaceMouse\n"
		  " Button 5 = Increase SpaceMouse Sensitivity\n"
		  " Button 6 = Decrease SpaceMouse Sensitivity\n"
		  " Button 7 = Reset Defaults\n"
		  " Button 8 = Display Help";

    MessageBox(NULL, hlpStr, Title, MB_ICONINFORMATION | MB_OK);
}
