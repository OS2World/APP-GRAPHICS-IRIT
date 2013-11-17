/*****************************************************************************
*  Generic tools of object transformations.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, July 1999.  *
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "allocate.h"
#include "grap_loc.h"
#include "editmanp.h"

#define IG_OBJ_MANIP_START_NUM_OBJS		10

IRIT_STATIC_DATA int
    IGObjManipOriginalObjsLen = 0;
IRIT_STATIC_DATA IrtHmgnMatType
    IGAccumTransform;
IRIT_GLOBAL_DATA char
    IGObjManipMatName[IRIT_LINE_LEN_LONG],
    *IGObjManipStateEntries[] = {
	"Attach to One Object",
	"Attach to Few Objects",
	"Attach to All Objects",
	"Clone Old Object",
	"Manipulate/Edit Object",
	"Detach Current Object",
	NULL
    };
IRIT_GLOBAL_DATA int
    IGObjManipNumActiveObjs = 0,
    IGObjManipScreenSpace = TRUE,
    IGObjManipGrabMouse = FALSE,
    IGObjManipSnap = FALSE;
IRIT_GLOBAL_DATA IrtRType
    IGObjManipSnapDegrees = 5.0,
    IGObjManipSnapDistance = 0.1;
IRIT_GLOBAL_DATA IGObjManipStateType
    IGObjManipState = IG_OBJ_MANIP_STATE_DETACH;
IRIT_GLOBAL_DATA IPObjectStruct
    **IGObjManipOriginalObjs = NULL,
    **IGObjManipCurrentObjs = NULL;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attach to an object for further editing.	                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Object to attach to.	                                     M
*   CloneIt:  If TRUE, make a copy of given object fist and edit the clone.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipAttachOldDirectly                                              M
*****************************************************************************/
void IGObjManipAttachOldDirectly(IPObjectStruct *PObj, int CloneIt)
{
    if (PObj == NULL) {
        IGObjManipPlaceMessage("Invalid name or not found");
	return;
    }

    IGObjManipActivateObj(PObj, CloneIt, TRUE);
    IGObjManipState = IG_OBJ_MANIP_STATE_EDIT;
    IGObjManipGrabMouse = TRUE;
    IGObjManipPlaceMessage("Waiting for object editing");
    IGObjManipParamUpdateWidget();
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle mouse events while the surface editing grabs the mouse.           M
*                                                                            *
* PARAMETERS:                                                                M
*   x, y:    Coordinates of the mouse event.                                 M
*   Event:   Type of event (mouse move, button up etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipHandleMouse                                                    M
*****************************************************************************/
void IGObjManipHandleMouse(int x, int y, int Event)
{
    IRIT_STATIC_DATA short StartMouseX, StartMouseY,
	MouseRotate = FALSE,
	MouseTranslate = FALSE;
    IRIT_STATIC_DATA IPObjectStruct
	**IGObjManipStartObjs = NULL;
    int i;
    IrtRType ChangeFactor[4];
    IPObjectStruct *PObj;

    switch (IGObjManipState) {
	case IG_OBJ_MANIP_STATE_ATTACH_ONE:
	    if (Event != IG_OBJ_MANIP_BTN1DOWN)
	        break;
	    if ((PObj = IGHandlePickEvent(x, y, IG_PICK_ANY)) != NULL &&
		IGObjManipActivateObj(PObj, FALSE, TRUE)) {
		IGReleasePickedObject();
		IGObjManipState = IG_OBJ_MANIP_STATE_EDIT;
		IGObjManipGrabMouse = TRUE;
	    }
	    else {
	        IGObjManipPlaceMessage("Attached to no object");
		IGObjManipGrabMouse = FALSE;
	    }

	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_STATE_ATTACH_FEW:
	    if (Event != IG_OBJ_MANIP_BTN1DOWN)
	        break;
	    if ((PObj = IGHandlePickEvent(x, y, IG_PICK_ANY)) != NULL &&
		IGObjManipActivateObj(PObj, FALSE, TRUE)) {
		IGReleasePickedObject();
		IGObjManipGrabMouse = TRUE;
	    }
	    else {
		IGObjManipState = IG_OBJ_MANIP_STATE_EDIT;
		IGObjManipGrabMouse = FALSE;
	    }

	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_STATE_ATTACH_ALL:
	    break;
	case IG_OBJ_MANIP_STATE_CLONE_OLD:
	    if (Event != IG_OBJ_MANIP_BTN1DOWN)
	        break;
	    if ((PObj = IGHandlePickEvent(x, y, IG_PICK_ANY)) != NULL &&
		IGObjManipActivateObj(PObj, TRUE, TRUE)) {
		IGReleasePickedObject();
		IGObjManipState = IG_OBJ_MANIP_STATE_EDIT;
		IGObjManipGrabMouse = TRUE;
	    }
	    else {
	        IGObjManipPlaceMessage("Cloned no object");
		IGObjManipGrabMouse = FALSE;
	    }

	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_STATE_EDIT:
	    switch (Event) {
		case IG_OBJ_MANIP_BTN1DOWN:
		case IG_OBJ_MANIP_BTN3DOWN:
		    StartMouseX = x;
		    StartMouseY = y;

		    IGObjManipStartObjs = IGObjManipCopyStartObjs();

		    if (Event == IG_OBJ_MANIP_BTN1DOWN)
		        MouseRotate = TRUE;
		    else
		        MouseTranslate = TRUE;
		    IGGlblManipulationActive = TRUE;
		    break;
		case IG_OBJ_MANIP_MOTION:
		    if (!IGGlblManipulationActive)
		        break;
		    ChangeFactor[0] = (x - StartMouseX) * IGGlblChangeFactor;
		    ChangeFactor[1] = (StartMouseY - y) * IGGlblChangeFactor;

		    if (IGObjManipNumActiveObjs > 0) {
			for (i = 0; i < IGObjManipNumActiveObjs; i++) {
			    if (MouseRotate)
				IGObjManipObjTrans(IGObjManipCurrentObjs[i],
						   IGObjManipStartObjs[i],
						   IG_EVENT_ROTATE,
						   ChangeFactor);
			    else if (MouseTranslate)
				IGObjManipObjTrans(IGObjManipCurrentObjs[i],
						   IGObjManipStartObjs[i],
						   IG_EVENT_TRANSLATE,
						   ChangeFactor);
			}
			
			IGRedrawViewWindow();
		    }
		    break;
		case IG_OBJ_MANIP_BTN_UP:
		    IGObjManipFreeStartObjs(IGObjManipStartObjs);
		    IGObjManipStartObjs = NULL;
		    MouseTranslate = MouseRotate = FALSE;
		    IGObjManipObjTrans(NULL, NULL, IG_EVENT_ACCUM_MATRIX,
				       ChangeFactor);
		    IGGlblManipulationActive = FALSE;
		    IGRedrawViewWindow();
		    break;
	    }
	    break;
        default:
	    assert(0);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Create a copy of the current objects for a new mouse (down-motion-up)    M
* sequence.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct **:  A copy of currently active objects as array of       M
*		(IPObjectStruct *) of length IGObjManipNumActiveObjs.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGObjManipFreeStartObjs                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipCopyStartObjs                                                  M
*****************************************************************************/
IPObjectStruct **IGObjManipCopyStartObjs(void)
{
    int i;
    IPObjectStruct
        **IGObjManipStartObjs = (IPObjectStruct **)
	    IritMalloc(sizeof(IPObjectStruct *)
		       * (1 + IGObjManipNumActiveObjs));

    for (i = 0; i < IGObjManipNumActiveObjs; i++)
        IGObjManipStartObjs[i] =
	    IPCopyObject(NULL, IGObjManipCurrentObjs[i], TRUE);
    IGObjManipStartObjs[i] = NULL;

    return IGObjManipStartObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the copy of the current objects in a new mouse (down-motion-up)     M
* sequence.                                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   IGObjManipStartObjs:  Array of objects to free.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGObjManipCopyStartObjs                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipFreeStartObjs                                                  M
*****************************************************************************/
void IGObjManipFreeStartObjs(IPObjectStruct **IGObjManipStartObjs)
{
    int i;

    if (IGObjManipStartObjs == NULL)
	return;

    for (i = 0; i < IGObjManipNumActiveObjs; i++) {
        if (IGObjManipStartObjs[i] == NULL)
	    break;
        IPFreeObject(IGObjManipStartObjs[i]);
    }

    IritFree(IGObjManipStartObjs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds all objects in given list object to the active list.                M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     The object to add in.                                          M
*   Clone:    If TRUE, clone the object instead of use in place.             M
*   Highlight:  If TRUE, objects are also tagged as highlighted.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if all successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGObjManipActivateObj	                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipActivateListObj                                                M
*****************************************************************************/
int IGObjManipActivateListObj(IPObjectStruct *PObj,
			      int Clone,
			      int Highlight)
{
    int i = 0,
	RetVal = TRUE;
    IPObjectStruct *PTmp;

    if (IP_IS_OLST_OBJ(PObj)) {
	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL)
	    RetVal &= IGObjManipActivateListObj(PTmp, Clone, Highlight);

	return RetVal;
    }
    else
        return IGObjManipActivateObj(PObj, Clone, Highlight);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Adds another object to the active list.			             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     The object to add in.                                          M
*   Clone:    If TRUE, clone the object instead of use in place.             M
*   Highlight:  If TRUE, object is are also tagged as highlighted.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if successful, FALSE otherwise.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGObjManipActivateListObj                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipActivateObj	                                             M
*****************************************************************************/
int IGObjManipActivateObj(IPObjectStruct *PObj,
			  int Clone,
			  int Highlight)
{
    int i;

    for (i = 0; i < IGObjManipNumActiveObjs; i++)
	if (IGObjManipCurrentObjs[i] == PObj)
	    return FALSE; /* This object is already in the list - ignore it. */

    IGActiveFreePolyIsoAttribute(PObj, TRUE, TRUE, TRUE, TRUE);

    /* Make sure our vectors to hold the object's points are large enough. */
    if (IGObjManipCurrentObjs == NULL) {
	IGObjManipOriginalObjsLen = IG_OBJ_MANIP_START_NUM_OBJS;
	i = sizeof(IPObjectStruct *) * IGObjManipOriginalObjsLen;

	IGObjManipCurrentObjs = (IPObjectStruct **) IritMalloc(i);
	IGObjManipOriginalObjs = (IPObjectStruct **) IritMalloc(i);
    }
    if (IGObjManipOriginalObjsLen < IGObjManipNumActiveObjs + 1) {
	IGObjManipOriginalObjsLen <<= 1;
	i = sizeof(IPObjectStruct *) * IGObjManipOriginalObjsLen;

	IGObjManipCurrentObjs =
	    (IPObjectStruct **) IritRealloc(IGObjManipCurrentObjs, i >> 1, i);
	
	IGObjManipOriginalObjs =
	    (IPObjectStruct **) IritRealloc(IGObjManipOriginalObjs, i >> 1, i);
    }

    if (Clone) {
	IG_RST_HIGHLIGHT1_OBJ(PObj);
	IGObjManipCurrentObjs[IGObjManipNumActiveObjs] =
						IPCopyObject(NULL, PObj, TRUE);
	IP_CAT_OBJ_NAME(IGObjManipCurrentObjs[IGObjManipNumActiveObjs],
			"Clone");
	IRIT_LIST_PUSH(IGObjManipCurrentObjs[IGObjManipNumActiveObjs],
		  IGGlblDisplayList);
    }
    else {
	IGObjManipCurrentObjs[IGObjManipNumActiveObjs] = PObj;
    }

    /* Reset transformation accumulation and save matrix name. */
    MatGenUnitMat(IGAccumTransform);
    strcpy(IGObjManipMatName,
	   IP_GET_OBJ_NAME(IGObjManipCurrentObjs[IGObjManipNumActiveObjs]));
    strcat(IGObjManipMatName, "Mat");

    /* Make sure we mark all manipulated active objects. */
    if (Highlight) {
	IG_RST_HIGHLIGHT1_OBJ(IGObjManipCurrentObjs[IGObjManipNumActiveObjs]);
	IG_SET_HIGHLIGHT2_OBJ(IGObjManipCurrentObjs[IGObjManipNumActiveObjs]);
    }

    /* Save an original copy. */
    IGObjManipOriginalObjs[IGObjManipNumActiveObjs] =
	IPCopyObject(NULL, IGObjManipCurrentObjs[IGObjManipNumActiveObjs],
		     TRUE);

    IGObjManipNumActiveObjs++;        /* One more object in the active list. */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle non mouse events while the surface editing.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Event:     Type of event (clear, order change, etc.).                    M
*   MenuIndex: Menu index of sub-pop-up menu.			             M
*   Data:      Data to process, if any.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   Returned info, if requested.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipHandleNonMouseEvents                                           M
*****************************************************************************/
VoidPtr IGObjManipHandleNonMouseEvents(IGObjManipEventType Event,
				       int MenuIndex,
				       VoidPtr Data)
{
    static int Clrs[3];
    int i, *c,
	RefreshWin = TRUE;
    IPObjectStruct *PObj;

    switch (Event) {
	case IG_OBJ_MANIP_EVENT_STATE:
	    switch (MenuIndex) {
		case IG_OBJ_MANIP_STATE_ATTACH_ALL:
	            for (PObj = IGGlblDisplayList;
			 PObj != NULL;
			 PObj = PObj -> Pnext)
		        IGObjManipActivateListObj(PObj, FALSE, TRUE);

		    IGObjManipState = IG_OBJ_MANIP_STATE_EDIT;
		    IGObjManipParamUpdateWidget();
		    IGObjManipPlaceMessage("All objects selected");
		    IGRedrawViewWindow();
	            break;
		case IG_OBJ_MANIP_STATE_ATTACH_FEW:
		case IG_OBJ_MANIP_STATE_ATTACH_ONE:
		case IG_OBJ_MANIP_STATE_CLONE_OLD:
		    IGObjManipState = (IGObjManipStateType) MenuIndex;
		    IGObjManipPlaceMessage("Select an object");
		    IGObjManipGrabMouse = TRUE;
		    break;
		case IG_OBJ_MANIP_STATE_EDIT:
		    IGObjManipState = IG_OBJ_MANIP_STATE_EDIT;
		    IGObjManipParamUpdateWidget();
		    break;
		case IG_OBJ_MANIP_STATE_DETACH:
		    if (IGObjManipNumActiveObjs == 0)
			break;

		    IGObjManipDetachObj();
		    IGRedrawViewWindow();
		    IGObjManipState = (IGObjManipStateType) MenuIndex;
		    IGObjManipPlaceMessage("Detached from object(s)");
		    IGObjManipGrabMouse = FALSE;
		    break;
		default:
		    IGObjManipGrabMouse = FALSE;
		    break;
	    }
	    RefreshWin = FALSE;
	    break;
	case IG_OBJ_MANIP_EVENT_OBJECT_SCREEN:
	    IGObjManipScreenSpace = !IGObjManipScreenSpace;
	    IGObjManipParamUpdateWidget();
	    RefreshWin = FALSE;
	    break;
	case IG_OBJ_MANIP_EVENT_SAVE:
	    if (IGObjManipNumActiveObjs == 0) {
	        IGObjManipPlaceMessage("No object(s) to save");
		break;
	    }
	    else {
		char Line[IRIT_LINE_LEN_LONG],
		    *FileName = (char *) Data;

		if (!IRT_STR_NULL_ZERO_LEN(FileName)) {
		    int Handler;

		    if ((Handler = IPOpenDataFile(FileName, FALSE,
						  FALSE)) >= 0) {
		        for (i = 0; i < IGObjManipNumActiveObjs; i++)
			    IPPutObjectToHandler(Handler,
						 IGObjManipCurrentObjs[i]);
			IPCloseStream(Handler, TRUE);

			sprintf(Line, "Saved under \"%s\"", FileName);
			IGObjManipPlaceMessage(Line);	
		    }
		    else {
		        sprintf(Line, "Failed to open file \"%s\"", FileName);
		        IGIritError(Line);
		    }
		}
		else
		    IGObjManipPlaceMessage("Failed to pick file name");
	    }
	    RefreshWin = FALSE;
	    break;
	case IG_OBJ_MANIP_EVENT_GET_COLOR:
	    if (IGObjManipNumActiveObjs == 0) {
	        IGObjManipPlaceMessage("No object(s) to color");
		return NULL;
	    }
	    
	    if (!AttrGetObjectRGBColor(IGObjManipCurrentObjs[0],
				       &Clrs[0], &Clrs[1], &Clrs[2]))
	        Clrs[0] = Clrs[1] = Clrs[2] = 0;

	    return Clrs;
	case IG_OBJ_MANIP_EVENT_SET_COLOR:
	    if (IGObjManipNumActiveObjs == 0) {
	        IGObjManipPlaceMessage("No object(s) to color");
		break;
	    }

	    c = (int *) Data;
	    for (i = 0; i < IGObjManipNumActiveObjs; i++) {
	        AttrSetObjectRGBColor(IGObjManipCurrentObjs[i],
				      c[0], c[1], c[2]);
		IG_RST_HIGHLIGHT2_OBJ(IGObjManipCurrentObjs[i]);
	    }

	    IGObjManipNumActiveObjs = 0;
		
	    IGObjManipGrabMouse = FALSE;
	    IGObjManipState = IG_OBJ_MANIP_STATE_DETACH;
	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_EVENT_REVERSE:
	    if (IGObjManipNumActiveObjs == 0) {
	        IGObjManipPlaceMessage("No object(s) to reverse");
		break;
	    }
	    for (i = 0; i < IGObjManipNumActiveObjs; i++) {
	        IPObjectStruct
		    *PObj = IGObjManipCurrentObjs[i],
		    *PRev = IPReverseObject(PObj);

		IGObjManipCurrentObjs[i] = NULL;
	        IGDeleteOneObject(PObj);
		IPFreeObject(IGObjManipOriginalObjs[i]);
		IGAddReplaceObjDisplayList(&IGGlblDisplayList, PRev, NULL);
	    }
	    IGObjManipNumActiveObjs = 0;
		
	    IGObjManipGrabMouse = FALSE;
	    IGObjManipState = IG_OBJ_MANIP_STATE_DETACH;
	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_EVENT_DELETE:
	    if (IGObjManipNumActiveObjs == 0) {
	        IGObjManipPlaceMessage("No object(s) to delete");
		break;
	    }
	    for (i = 0; i < IGObjManipNumActiveObjs; i++) {
	        IPObjectStruct
		    *PObj = IGObjManipCurrentObjs[i];

		IGObjManipCurrentObjs[i] = NULL;
	        IGDeleteOneObject(PObj);
		IPFreeObject(IGObjManipOriginalObjs[i]);
	    }
	    IGObjManipNumActiveObjs = 0;
		
	    IGObjManipGrabMouse = FALSE;
	    IGObjManipState = IG_OBJ_MANIP_STATE_DETACH;
	    IGObjManipParamUpdateWidget();
	    break;
	case IG_OBJ_MANIP_EVENT_RESET:
	    if (IGObjManipNumActiveObjs == 0) {
	        IGObjManipPlaceMessage("No object(s) to reset");
		break;
	    }
	    for (i = 0; i < IGObjManipNumActiveObjs; i++) {
		IGObjManipOriginalObjs[i] -> Pnext =
		    IGObjManipCurrentObjs[i] -> Pnext;
		IPCopyObject(IGObjManipCurrentObjs[i],
			     IGObjManipOriginalObjs[i], TRUE);
	    }
	    MatGenUnitMat(IGAccumTransform);
	    break;
	case IG_OBJ_MANIP_EVENT_SUBMIT_MAT:
	    if (IGGlblStandAlone) {
		IGObjManipPlaceMessage("No submissions in stand alone mode");
		break;
	    }
	    PObj = IPGenMatObject("_SubmitMat_", IGAccumTransform, NULL);
	    AttrSetObjectStrAttrib(PObj, "ObjName", IGObjManipMatName);
	    IPSocWriteOneObject(IGGlblIOHandle, PObj);
	    IPFreeObject(PObj);
	    IGObjManipPlaceMessage("Transformation matrix submitted");
	    break;
	case IG_OBJ_MANIP_EVENT_DISMISS:
	    IGObjManipDetachObj();
	    break;
        default:
	    assert(0);
    }

    if (RefreshWin) {
        IGGlblPickedObj = NULL;
	if (IGGlblPickedPolyObj != NULL) {
	    IPFreeObject(IGGlblPickedPolyObj);
	    IGGlblPickedPolyObj = NULL;
	}
	IGRedrawViewWindow();
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Processes the given event. Returns TRUE if redraw of view window is needed.M
* The given PObj object is transformed in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:	    Object to transform in place.                            M
*   PStart:	    Object in start position (before this mouse seq.).       M
*   Event:          Event to process.                                        M
*   ChangeFactor:   A continuous scale between -1 and 1 to quantify the      M
*                   change to apply according to the event type, as a pair   M
*		    for composed operation contains both X and Y information.M
*		    The second pair (third and fourth numbers in this array) M
*		    provides the accumulated change in this operation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:            TRUE if refresh is needed.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipObjTrans                                                       M
*****************************************************************************/
int IGObjManipObjTrans(IPObjectStruct *PObj,
		       IPObjectStruct *PStart,
		       IGGraphicEventType Event,
		       IrtRType *ChangeFactor)
{
    static IrtHmgnMatType Mat;
    char Line[IRIT_LINE_LEN];
    int UpdateView = TRUE;
    IrtRType
	Amount = 0.0,
	Amount2 = 0.0;
    IrtVecType V;
    IrtHmgnMatType TMat;
    IPObjectStruct *PTmp;
    GMBBBboxStruct *BBox;

    if (Event != IG_EVENT_ACCUM_MATRIX)
	MatGenUnitMat(Mat);

    /* Compute the transformation amount and send a message to the user. */
    switch (Event) {
	case IG_EVENT_ROTATE:		    /* Its rotation in both X and Y. */
	    Amount = ChangeFactor[0] * IG_MAX_ROTATE_ANGLE / 150.0;
	    Amount2 = ChangeFactor[1] * IG_MAX_ROTATE_ANGLE / 150.0;
	    break;
        case IG_EVENT_ROTATE_X:		   /* Its rotation along the X axis. */
	case IG_EVENT_ROTATE_Y:		   /* Its rotation along the Y axis. */
	case IG_EVENT_ROTATE_Z:		   /* Its rotation along the Z axis. */
	    Amount = ChangeFactor[0] * IG_MAX_ROTATE_ANGLE;
	    break;
	case IG_EVENT_TRANSLATE:	 /* Its translation in both X and Y. */
	    Amount = ChangeFactor[0] * IG_MAX_TRANSLATE_FACTOR / 300.0;
	    Amount2 = ChangeFactor[1] * IG_MAX_TRANSLATE_FACTOR / 300.0;
	    break;
	case IG_EVENT_TRANSLATE_X:	/* Its translation along the X axis. */
	case IG_EVENT_TRANSLATE_Y:	/* Its translation along the Y axis. */
	case IG_EVENT_TRANSLATE_Z:	/* Its translation along the Z axis. */
	    Amount = ChangeFactor[0] * IG_MAX_TRANSLATE_FACTOR;
	    break;
	case IG_EVENT_SCALE:		      /* Its scaling along all axes. */
	    if (ChangeFactor[0] > 0.0)		      /* Make it around 1... */
	        Amount = ChangeFactor[0] * IG_MAX_SCALE_FACTOR + 1.0;
	    else
	        Amount = 1.0 / (-ChangeFactor[0] * IG_MAX_SCALE_FACTOR + 1.0);
	    break;
	default:
	    break;
    }

    if (IGObjManipSnap) {
        switch (Event) {
	    case IG_EVENT_ROTATE:
		if (IGObjManipSnapDegrees != 0.0) {
		    Amount2 = ((int) (Amount2 / IGObjManipSnapDegrees))
						      * IGObjManipSnapDegrees;
		}
	    case IG_EVENT_ROTATE_X:
	    case IG_EVENT_ROTATE_Y:
	    case IG_EVENT_ROTATE_Z:
		if (IGObjManipSnapDegrees != 0.0) {
		    Amount = ((int) (Amount / IGObjManipSnapDegrees))
						      * IGObjManipSnapDegrees;
		}
		break;
	    case IG_EVENT_TRANSLATE:
		if (IGObjManipSnapDistance != 0.0) {
		    Amount2 = ((int) (Amount2 / IGObjManipSnapDistance))
						     * IGObjManipSnapDistance;
		}
	    case IG_EVENT_TRANSLATE_X:
	    case IG_EVENT_TRANSLATE_Y:
	    case IG_EVENT_TRANSLATE_Z:
		if (IGObjManipSnapDistance != 0.0) {
		    Amount = ((int) (Amount / IGObjManipSnapDistance))
						     * IGObjManipSnapDistance;
		}
		break;
            default:
	        assert(0);
	}
    }

    switch (Event) {
	case IG_EVENT_ROTATE:		    /* Its rotation in both X and Y. */
	    sprintf(Line, "Rotated (%.4f, %.4f) degrees", Amount, Amount2);
	    break;
        case IG_EVENT_ROTATE_X:		   /* Its rotation along the X axis. */
	case IG_EVENT_ROTATE_Y:		   /* Its rotation along the Y axis. */
	case IG_EVENT_ROTATE_Z:		   /* Its rotation along the Z axis. */
	    sprintf(Line, "Rotated %.4f degrees", Amount);
	    break;
	case IG_EVENT_TRANSLATE:	 /* Its translation in both X and Y. */
	    V[0] = Amount;
	    V[1] = Amount2;
	    V[2] = 0.0;
	    MatMultVecby4by4(V, V, IGGlblInvCrntViewMat);
	    
	    sprintf(Line, "Trans by (%.4f, %.4f, %.4f)", V[0], V[1], V[2]);
	    break;
	case IG_EVENT_TRANSLATE_X:	/* Its translation along the X axis. */
	    V[0] = Amount;
	    V[1] = 0.0;
	    V[2] = 0.0;
	    
	    MatMultVecby4by4(V, V, IGGlblInvCrntViewMat);
	    
	    sprintf(Line, "Trans by (%.4f, %.4f, %.4f)", V[0], V[1], V[2]);
	    break;
	case IG_EVENT_TRANSLATE_Y:	/* Its translation along the Y axis. */
	    V[0] = 0.0;
	    V[1] = Amount;
	    V[2] = 0.0;
	    
	    MatMultVecby4by4(V, V, IGGlblInvCrntViewMat);
	    
	    sprintf(Line, "Trans by (%.4f, %.4f, %.4f)", V[0], V[1], V[2]);
	    break;
	case IG_EVENT_TRANSLATE_Z:	/* Its translation along the Z axis. */
	    V[0] = 0.0;
	    V[1] = 0.0;
	    V[2] = Amount;
	    
	    MatMultVecby4by4(V, V, IGGlblInvCrntViewMat);
	    
	    sprintf(Line, "Trans by (%.4f, %.4f, %.4f)", V[0], V[1], V[2]);
	    break;
	case IG_EVENT_SCALE:		      /* Its scaling along all axes. */
	    sprintf(Line, "Scaled by %.4f", Amount);
	    break;
	default:
	    Line[0] = 0;
	    break;
    }
    if (!IRT_STR_ZERO_LEN(Line))
	IGObjManipPlaceMessage(Line);

    switch (Event) {
	case IG_EVENT_ROTATE:		    /* Its rotation in both X and Y. */
	    /* Doing it seperatly for X and Y is not the right thing, but it */
	    /* does work for us, in interactive use.			     */
	    MatGenMatRotY1(IRIT_DEG2RAD(Amount), TMat);
	    MatGenMatRotX1(IRIT_DEG2RAD(-Amount2), Mat);
	    MatMultTwo4by4(Mat, TMat, Mat);
	    break;
	case IG_EVENT_ROTATE_X:		   /* Its rotation along the X axis. */
	    MatGenMatRotX1(IRIT_DEG2RAD(Amount), Mat);
	    break;
	case IG_EVENT_ROTATE_Y:		   /* Its rotation along the Y axis. */
	    MatGenMatRotY1(IRIT_DEG2RAD(Amount), Mat);
	    break;
	case IG_EVENT_ROTATE_Z:		   /* Its rotation along the Z axis. */
	    MatGenMatRotZ1(IRIT_DEG2RAD(Amount), Mat);
	    break;
	case IG_EVENT_TRANSLATE:	 /* Its translation in both X and Y. */
	    MatGenMatTrans(Amount, Amount2, 0.0, Mat);
	    break;
	case IG_EVENT_TRANSLATE_X:	/* Its translation along the X axis. */
	    MatGenMatTrans(Amount, 0.0, 0.0, Mat);
	    break;
	case IG_EVENT_TRANSLATE_Y:	/* Its translation along the Y axis. */
	    MatGenMatTrans(0.0, Amount, 0.0, Mat);
	    break;
	case IG_EVENT_TRANSLATE_Z:	/* Its translation along the Z axis. */
	    MatGenMatTrans(0.0, 0.0, Amount, Mat);
	    break;
	case IG_EVENT_SCALE:		      /* Its scaling along all axes. */
	    MatGenMatUnifScale(Amount, Mat);
	    break;
	default:
	    UpdateView = FALSE;
	    break;
    }

    /* If the transformation is to be taken in screen space, we must apply   */
    /* A transformation that takes the object to screen space and then back  */
    /* to its object space.  Prepare that screen space transform, if needed. */
    if (Event != IG_EVENT_ACCUM_MATRIX && IGObjManipScreenSpace) {
        MatMultTwo4by4(Mat, IGGlblCrntViewMat, Mat);
	MatMultTwo4by4(Mat, Mat, IGGlblInvCrntViewMat);
    }

    /* Update object (and accumulated matrix) to its new position, in place. */
    if (Event == IG_EVENT_ACCUM_MATRIX) {
        MatMultTwo4by4(IGAccumTransform, IGAccumTransform, Mat);
	MatGenUnitMat(Mat);
    }
    else {
        PTmp = GMTransformObject(PStart, Mat);

	PTmp -> Pnext = PObj -> Pnext;

	IP_ATTR_FREE_ATTRS(PTmp -> Attr);
	PTmp -> Attr = PObj -> Attr;
	PObj -> Attr = NULL;

	IPCopyObject(PObj, PTmp, TRUE);
	IPFreeObject(PTmp);

	IGActiveFreePolyIsoAttribute(PObj, TRUE, TRUE, TRUE, TRUE);
	BBox = GMBBComputeBboxObject(PObj);
	IRIT_PT_COPY(PObj -> BBox[0], BBox -> Min);
	IRIT_PT_COPY(PObj -> BBox[1], BBox -> Max);
    }

    return UpdateView;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Detach the currently transformed object.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGObjManipDetachObj                                                      M
*****************************************************************************/
void IGObjManipDetachObj(void)
{
    if (IGObjManipNumActiveObjs > 0) {
	int i;

	for (i = 0; i < IGObjManipNumActiveObjs; i++) {
	    IG_RST_HIGHLIGHT2_OBJ(IGObjManipCurrentObjs[i]);
	    IPFreeObject(IGObjManipOriginalObjs[i]);
	}
    }
    else
        IGObjManipPlaceMessage("No Object to detach");

    IGObjManipNumActiveObjs = 0;

    IGObjManipGrabMouse = FALSE;
    IGObjManipState = IG_OBJ_MANIP_STATE_DETACH;
    IGObjManipParamUpdateWidget();
}
