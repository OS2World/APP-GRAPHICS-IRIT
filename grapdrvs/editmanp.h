/*****************************************************************************
*  Header file of the generic tools of object transformations.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, July 1999.   *
*****************************************************************************/

#ifndef OBJ_MANIP_H
#define OBJ_MANIP_H

#include "grap_loc.h"

typedef enum {
    IG_OBJ_MANIP_EVENT_STATE,
    IG_OBJ_MANIP_EVENT_OBJECT_SCREEN,
    IG_OBJ_MANIP_EVENT_SNAP,
    IG_OBJ_MANIP_EVENT_SNAP_DEGREES,
    IG_OBJ_MANIP_EVENT_SNAP_DISTANCE,
    IG_OBJ_MANIP_EVENT_GET_COLOR,
    IG_OBJ_MANIP_EVENT_SET_COLOR,
    IG_OBJ_MANIP_EVENT_REVERSE,
    IG_OBJ_MANIP_EVENT_DELETE,
    IG_OBJ_MANIP_EVENT_SAVE,
    IG_OBJ_MANIP_EVENT_RESET,
    IG_OBJ_MANIP_EVENT_SUBMIT_MAT,
    IG_OBJ_MANIP_EVENT_DISMISS,
    IG_OBJ_MANIP_EVENT_NAME_MATRIX,
    IG_OBJ_MANIP_EVENT_NONE
} IGObjManipEventType;

typedef enum {
    IG_OBJ_MANIP_STATE_ATTACH_ONE,
    IG_OBJ_MANIP_STATE_ATTACH_FEW,
    IG_OBJ_MANIP_STATE_ATTACH_ALL,
    IG_OBJ_MANIP_STATE_CLONE_OLD,
    IG_OBJ_MANIP_STATE_EDIT,
    IG_OBJ_MANIP_STATE_DETACH
} IGObjManipStateType;

typedef enum {
    IG_OBJ_MANIP_NONE,
    IG_OBJ_MANIP_MOTION,
    IG_OBJ_MANIP_BTN1DOWN,
    IG_OBJ_MANIP_BTN2DOWN,
    IG_OBJ_MANIP_BTN3DOWN,
    IG_OBJ_MANIP_BTN_UP
} IGObjManipMotionType;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER char
    IGObjManipMatName[],
    *IGObjManipStateEntries[];
IRIT_GLOBAL_DATA_HEADER int
    IGObjManipNumActiveObjs,
    IGObjManipScreenSpace,
    IGObjManipGrabMouse,
    IGObjManipSnap;
IRIT_GLOBAL_DATA_HEADER IrtRType
    IGObjManipSnapDegrees,
    IGObjManipSnapDistance;
IRIT_GLOBAL_DATA_HEADER IGObjManipStateType
    IGObjManipState;
IRIT_GLOBAL_DATA_HEADER IPObjectStruct
    **IGObjManipOriginalObjs,
    **IGObjManipCurrentObjs;

/* Call back functions that must be supplied by all drivers. */
void IGObjManipParamUpdateWidget(void);
void IGObjManipPlaceMessage(char *Msg);

/* Functions in editmanp.c that are generic to all drivers. */
IPObjectStruct **IGObjManipCopyStartObjs(void);
void IGObjManipFreeStartObjs(IPObjectStruct **IGObjManipStartObjs);
void IGObjManipAttachOldDirectly(IPObjectStruct *PObj, int CloneIt);
void IGObjManipHandleMouse(int x, int y, int Event);
VoidPtr IGObjManipHandleNonMouseEvents(IGObjManipEventType Event,
				       int MenuIndex,
				       VoidPtr Data);
int IGObjManipObjTrans(IPObjectStruct *PObj,
		       IPObjectStruct *PStart,
		       IGGraphicEventType Event,
		       IrtRType *ChangeFactor);
int IGObjManipActivateListObj(IPObjectStruct *PObj,
			      int Clone,
			      int Highlight);
int IGObjManipActivateObj(IPObjectStruct *PObj,
			  int Clone,
			  int Highlight);
void IGObjManipDetachObj(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* OBJ_MANIP_H */
