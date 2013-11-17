/*****************************************************************************
*  Header file of the generic tools of interactive surface editing.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, May 1999.   *
*****************************************************************************/

#ifndef EDIT_SRFS_H
#define EDIT_SRFS_H

#include "grap_loc.h"
#include "cagd_lib.h"
#include "editcrvs.h"

typedef enum {
    IG_SRF_EDIT_EVENT_STATE,
    IG_SRF_EDIT_EVENT_SRF_NAME,
    IG_SRF_EDIT_EVENT_U_ORDER,
    IG_SRF_EDIT_EVENT_V_ORDER,
    IG_SRF_EDIT_EVENT_U_END_COND,
    IG_SRF_EDIT_EVENT_V_END_COND,
    IG_SRF_EDIT_EVENT_RATIONAL,
    IG_SRF_EDIT_EVENT_SRF_TYPE,
    IG_SRF_EDIT_EVENT_U_PARAM_TYPE,
    IG_SRF_EDIT_EVENT_V_PARAM_TYPE,
    IG_SRF_EDIT_EVENT_REGION,
    IG_SRF_EDIT_EVENT_REFINE,
    IG_SRF_EDIT_EVENT_REFINE_ONE,
    IG_SRF_EDIT_EVENT_REFINE_ALL,
    IG_SRF_EDIT_EVENT_REFINE_REGION,
    IG_SRF_EDIT_EVENT_U_DRAISE,
    IG_SRF_EDIT_EVENT_V_DRAISE,
    IG_SRF_EDIT_EVENT_SUBDIV,
    IG_SRF_EDIT_EVENT_SUBDIV1,
    IG_SRF_EDIT_EVENT_SUBDIV2,
    IG_SRF_EDIT_EVENT_SUBDIV_C0_CONT,
    IG_SRF_EDIT_EVENT_SUBDIV_C1_CONT,
    IG_SRF_EDIT_EVENT_UMR_SLIDE,
    IG_SRF_EDIT_EVENT_VMR_SLIDE,
    IG_SRF_EDIT_EVENT_MOVE_CTLPTS,
    IG_SRF_EDIT_EVENT_MODIFY_SRF,
    IG_SRF_EDIT_EVENT_MERGE_SRFS,
    IG_SRF_EDIT_EVENT_MODIFY_NORMAL_DIR,
    IG_SRF_EDIT_EVENT_SAVE_SRF,
    IG_SRF_EDIT_EVENT_SUBMIT_SRF,
    IG_SRF_EDIT_EVENT_DRAW_MESH,
    IG_SRF_EDIT_EVENT_DRAW_ORIG,
    IG_SRF_EDIT_EVENT_REVERSE,
    IG_SRF_EDIT_EVENT_TRIM,
    IG_SRF_EDIT_EVENT_TRIM1,
    IG_SRF_EDIT_EVENT_TRIM2,
    IG_SRF_EDIT_EVENT_EVALUATE,
    IG_SRF_EDIT_EVENT_PRIMITIVES,
    IG_SRF_EDIT_EVENT_UNDO,
    IG_SRF_EDIT_EVENT_REDO,
    IG_SRF_EDIT_EVENT_CLEAR,
    IG_SRF_EDIT_EVENT_DISMISS,
    IG_SRF_EDIT_EVENT_NONE
} IGSrfEditEventType;

typedef enum {
    IG_SRF_EDIT_STATE_PRIMITIVES,
    IG_SRF_EDIT_STATE_ATTACH_OLD,
    IG_SRF_EDIT_STATE_CLONE_OLD,
    IG_SRF_EDIT_STATE_EDIT,
    IG_SRF_EDIT_STATE_DETACH
} IGSrfEditStateType;

typedef enum {
    IG_SRF_EDIT_BUTTONUP,
    IG_SRF_EDIT_BUTTONDOWN,
    IG_SRF_EDIT_MOTION
} IGSrfEditMotionType;

#define SRF_EDIT_GET_SRF_OBJ(Obj)	(IP_IS_SRF_OBJ(Obj) \
					         ? (Obj) -> U.Srfs \
						 : (Obj) -> U.TrimSrfs -> Srf)
#define SRF_EDIT_SET_SRF_OBJ(Obj, NSrf) if (IP_IS_SRF_OBJ(Obj)) \
					     (Obj) -> U.Srfs = NSrf; \
					else (Obj) -> U.TrimSrfs -> Srf = NSrf;

typedef struct SEditMultiResKvsStruct {
    CEditMultiResOneKvStruct *UKvs, *VKvs;
    int NumUKvs, NumVKvs;
} SEditMultiResKvsStruct;

typedef struct IGSrfEditParamStruct {
    int UOrder, VOrder;
    CagdGeomType Type;
    CagdEndConditionType UEndCond, VEndCond;
    CagdParametrizationType UParamType, VParamType;

    CagdBType Rational;

    IGSrfEditStateType SrfState;
} IGSrfEditParamStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER char
    *IGSrfEditStateEntries[],
    *IGSrfEditSubdivEntries[],
    *IGSrfEditTrimSrfEntries[],
    *IGSrfEditRefineEntries[],
    *IGSrfEditEndCondEntries[],
    *IGSrfEditParamEntries[],
    *IGSrfEditEvalEntityEntries[],
    *IGSrfEditPrimitivesEntries[],
    *IGSrfEditReverseSrfEntries[];
IRIT_GLOBAL_DATA_HEADER int
    IGSrfEditActive,
    IGSrfDrawOriginal,
    IGSrfEditDrawMesh,
    IGSrfEvalEntity,
    IGSrfEditNormalDir,
    IGSrfEditGrabMouse;
IRIT_GLOBAL_DATA_HEADER IrtRType
    IGSrfEditMRULevel,
    IGSrfEditMRVLevel;
IRIT_GLOBAL_DATA_HEADER IGSrfEditParamStruct
    IGSrfEditParam;
IRIT_GLOBAL_DATA_HEADER CagdSrfStruct
    *IGSrfOriginalSurface,
    *IGSrfEditCurrentSrf;
IRIT_GLOBAL_DATA_HEADER IPObjectStruct
    *IGSrfEditPreloadEditSurfaceObj,
    *IGSrfEditCurrentObj;
IRIT_GLOBAL_DATA_HEADER IGSrfEditEventType
    IGSEditOperation;

/* Call back functions that must be supplied by all drivers. */
void IGSrfEditParamUpdateWidget(void);
void IGSrfEditPlaceMessage(char *Msg);

/* Functions in editsrfs.c that are generic to all drivers. */
int SEditPushState(CagdSrfStruct *Srf,
		   TrimSrfStruct *TSrfSkel,
		   IGSrfEditParamStruct *SrfEditParam);
CagdSrfStruct *SEditUndoState(IGSrfEditParamStruct *SrfEditParam,
			      TrimSrfStruct **TSrfSkel);
CagdSrfStruct *SEditRedoState(IGSrfEditParamStruct *SrfEditParam,
			      TrimSrfStruct **TSrfSkel);
void SEditFreeStateStack(void);
void SEditUpdateKnotVector(CagdSrfStruct *Srf,
			   CagdEndConditionType EndCond,
			   CagdSrfDirType Dir,
			   CagdParametrizationType Param);
int SEditRefineSrf(CagdSrfStruct **Srf,
		   CagdRType *t,
		   int n,
		   CagdSrfDirType Dir);
int SEditSubdivSrf(CagdSrfStruct **Srf,
		   CagdRType t,
		   int Continuity,
		   CagdSrfDirType Dir);
int SEditMergeSrfs(CagdSrfStruct **Srf1, CagdSrfStruct *Srf2);
int SEditTrimSrf(IPObjectStruct *SrfObj,
		 CagdCrvStruct *UVCrv,
		 CagdBType First);
int SEditDomainFromSrf(CagdSrfStruct **Srf,
		       CagdRType t1,
		       CagdRType t2,
		       CagdSrfDirType Dir);
CagdRType *SEditFindClosestParameter(CagdSrfStruct *Srf,
				     CagdPType MousePt,
				     CagdVType MouseDir,
				     CagdSrfDirType *Dir);
int SEditFindClosestControlPoint(CagdSrfStruct *Srf,
				 CagdPType MousePt,
				 CagdPType MouseDir,
				 int *FoundRational);
void SEditRedrawSrf(void);
SEditMultiResKvsStruct *SEditMultiResPrepKVs(CagdSrfStruct *Srf,
					     int Discont,
					     int UPeriodic,
					     int VPeriodic);
CagdSrfStruct *SEditMultiResModify(CagdSrfStruct *Srf,
				   SEditMultiResKvsStruct *SMultiKvs,
				   
				   IrtRType UMRLevel,
				   IrtRType VMRLevel,
				   CagdRType u,
				   CagdRType v,
				   IrtVecType TransDir);
void SEditAttachOldDirectly(IPObjectStruct *SrfObj);
void SEditHandleMouse(int x, int y, int Event);
void SEditHandleNonMouseEvents(IGSrfEditEventType Event, int MenuIndex);
void SEditDetachSurface(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* EDIT_SRFS_H */
