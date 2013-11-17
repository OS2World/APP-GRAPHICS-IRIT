/*****************************************************************************
*  Header file of the generic tools of interactive curve editing.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, Mar. 1998.  *
*****************************************************************************/

#ifndef EDIT_CRVS_H
#define EDIT_CRVS_H

#include "grap_loc.h"
#include "cagd_lib.h"

#define IG_CRV_EDIT_LST_SQR_DEF_PERCENT 20

#define IG_CRV_RATIONAL_CIRC_LENGTH	IGGlblNormalSize

typedef enum {
    IG_CRV_EDIT_EVENT_STATE,
    IG_CRV_EDIT_EVENT_CURVE_NAME,
    IG_CRV_EDIT_EVENT_ORDER,
    IG_CRV_EDIT_EVENT_END_COND,
    IG_CRV_EDIT_EVENT_RATIONAL,
    IG_CRV_EDIT_EVENT_LSTSQR_PERCENT,
    IG_CRV_EDIT_EVENT_CURVE_TYPE,
    IG_CRV_EDIT_EVENT_PARAM_TYPE,
    IG_CRV_EDIT_EVENT_REGION,
    IG_CRV_EDIT_EVENT_REFINE,
    IG_CRV_EDIT_EVENT_REFINE_ONE,
    IG_CRV_EDIT_EVENT_REFINE_ALL,
    IG_CRV_EDIT_EVENT_REFINE_REGION,
    IG_CRV_EDIT_EVENT_DRAISE,
    IG_CRV_EDIT_EVENT_SUBDIV,
    IG_CRV_EDIT_EVENT_SUBDIV1,
    IG_CRV_EDIT_EVENT_SUBDIV2,
    IG_CRV_EDIT_EVENT_SUBDIV_C0_CONT,
    IG_CRV_EDIT_EVENT_SUBDIV_C1_CONT,
    IG_CRV_EDIT_EVENT_MR_SLIDE,
    IG_CRV_EDIT_EVENT_MOVE_CTLPTS,
    IG_CRV_EDIT_EVENT_DELETE_CTLPTS,
    IG_CRV_EDIT_EVENT_MODIFY_CURVE,
    IG_CRV_EDIT_EVENT_MERGE_CURVES,
    IG_CRV_EDIT_EVENT_SAVE_CURVE,
    IG_CRV_EDIT_EVENT_SUBMIT_CURVE,
    IG_CRV_EDIT_EVENT_DRAW_MESH,
    IG_CRV_EDIT_EVENT_DRAW_ORIG,
    IG_CRV_EDIT_EVENT_REVERSE,
    IG_CRV_EDIT_EVENT_EVALUATE,
    IG_CRV_EDIT_EVENT_PRIMITIVES,
    IG_CRV_EDIT_EVENT_UNDO,
    IG_CRV_EDIT_EVENT_REDO,
    IG_CRV_EDIT_EVENT_CONSTRAINTS,
    IG_CRV_EDIT_EVENT_CLEAR,
    IG_CRV_EDIT_EVENT_DISMISS,
    IG_CRV_EDIT_EVENT_NONE
} IGCrvEditEventType;

typedef enum {
    IG_CRV_EDIT_STATE_PRIMITIVES,
    IG_CRV_EDIT_STATE_ATTACH_OLD,
    IG_CRV_EDIT_STATE_CLONE_OLD,
    IG_CRV_EDIT_STATE_START_CTLPT,
    IG_CRV_EDIT_STATE_START_SKETCH,
    IG_CRV_EDIT_STATE_START_SKETCH_ON_SRF,
    IG_CRV_EDIT_STATE_EDIT,
    IG_CRV_EDIT_STATE_DETACH
} IGCrvEditStateType;

typedef enum {
    IG_CRV_EDIT_BUTTONUP,
    IG_CRV_EDIT_BUTTONDOWN,
    IG_CRV_EDIT_MOTION
} IGCrvEditMotionType;

typedef struct CEditMultiResOneKvStruct {
    CagdRType *Kv;
    CagdRType **InnerProds;
    int KvLen;
} CEditMultiResOneKvStruct;

typedef struct CEditMultiResKvsStruct {
    CEditMultiResOneKvStruct *Kvs;
    int Order;
    int NumKvs;
} CEditMultiResKvsStruct;

typedef struct IGCrvEditParamStruct {
    int Order;
    CagdGeomType Type;
    CagdEndConditionType EndCond;
    CagdParametrizationType ParamType;

    CagdBType Rational;

    IGCrvEditStateType CrvState;

    int LstSqrPercent;		        /* Least squares percentage fitting. */

    int SupportConstraints;             /* Constraints related global state. */
    int AbortIfCnstFailed;
    int CnstXSymmetry;
    int CnstYSymmetry;
    int CnstCSymmetry;
    int CnstArea;
    IrtRType CnstMaxAllowedCoef;
} IGCrvEditParamStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER char
    *IGCrvEditStateEntries[],
    *IGCrvEditSubdivEntries[],
    *IGCrvEditRefineEntries[],
    *IGCrvEditEndCondEntries[],
    *IGCrvEditParamEntries[],
    *IGCrvEditEvalEntityEntries[],
    *IGCrvEditPrimitivesEntries[];
IRIT_GLOBAL_DATA_HEADER int
    IGCrvEditActive,
    IGCrvAbortIfCnstFailed,
    IGCrvSupportConstraints,
    IGCrvDrawOriginal,
    IGCrvEditDrawMesh,
    IGCrvEvalEntity,
    IGCrvEditGrabMouse;
IRIT_GLOBAL_DATA_HEADER IrtRType
    IGCrvEditMRLevel;
IRIT_GLOBAL_DATA_HEADER IGCrvEditParamStruct
    IGCrvEditParam;
IRIT_GLOBAL_DATA_HEADER CagdCrvStruct
    *IGCrvOriginalCurve,
    *IGCrvEditCurrentCrv,
    *IGCrvEditCurrentUVCrv;
IRIT_GLOBAL_DATA_HEADER CagdSrfStruct
    *IGCrvEditSktchSrf;
IRIT_GLOBAL_DATA_HEADER IPObjectStruct
    *IGCrvEditPreloadEditCurveObj,
    *IGCrvEditCurrentObj;
IRIT_GLOBAL_DATA_HEADER IGCrvEditEventType
    IGCEditOperation;

/* Call back functions that must be supplied by all drivers. */
void IGCrvEditParamUpdateWidget(void);
void IGCrvEditPlaceMessage(char *Msg);

/* Functions in editcrvs.c that are generic to all drivers. */
int CEditPushState(CagdCrvStruct *Crv,
		   CagdCrvStruct *UVCrv,
		   IGCrvEditParamStruct *CrvEditParam);
CagdCrvStruct *CEditUndoState(IGCrvEditParamStruct *CrvEditParam,
			      CagdCrvStruct **UVCrv);
CagdCrvStruct *CEditRedoState(IGCrvEditParamStruct *CrvEditParam,
			      CagdCrvStruct **UVCrv);
void CEditFreeStateStack(void);
int CEditAddPoint(CagdCrvStruct **Crv, int Index, CagdPType NewPos);
int CEditDelPoint(CagdCrvStruct **Crv, int Index);
int CEditMovePoint(CagdCrvStruct *Crv,
		   int Index,
		   int ModifyWeight,
		   IrtPtType MousePt,
		   IrtVecType MouseDir);
int CEditModifyCurve(CagdCrvStruct *Crv,
		     IrtRType CurveParam,
		     IrtPtType MouseStartPt,
		     IrtVecType MouseStartDir,
		     IrtPtType MousePt,
		     IrtVecType MouseDir);
void CEditUpdateKnotVector(CagdCrvStruct *Crv,
			   CagdEndConditionType EndCond,
			   CagdParametrizationType Param);
void CEditSketchCrvReset(void);
void CEditSketchAddPoint(CagdPType Pt, CagdRType *UV);
CagdCrvStruct *CEditSketchGetCurve(void);
CagdCrvStruct *CEditSketchGetUVCurve(void);
IPPolygonStruct *CEditSketchGetStrokedPoly(void);
int CEditRefineCrv(CagdCrvStruct **Crv, CagdRType *t, int n);
int CEditSubdivCrv(CagdCrvStruct **Crv, CagdRType t, int Continuity);
int CEditMergeCrvs(CagdCrvStruct **Crv1, CagdCrvStruct *Crv2);
int CEditDomainFromCrv(CagdCrvStruct **Crv, CagdRType t1, CagdRType t2);
CagdRType CEditFindClosestParameter(CagdCrvStruct *Crv,
				    CagdPType MousePt,
				    CagdPType MouseDir);
int CEditFindClosestControlPoint(CagdCrvStruct *Crv,
				 CagdPType MousePt,
				 CagdPType MouseDir,
				 int *FoundRational);
void CEditRedrawCrv(void);
CEditMultiResKvsStruct *CEditMultiResPrepKVs(CagdRType *OrigKV,
					     int OrigKVLen,
					     int Order,
					     int Discont,
					     int Periodic,
					     int InnerProd);
CagdCrvStruct *CEditMultiResModify(CagdCrvStruct *Crv,
				   int Order,
				   CEditMultiResKvsStruct *MultiKvs,
				   IrtRType MRLevel,
				   CagdRType t,
				   IrtVecType TransDir);
void CEditAttachOldDirectly(IPObjectStruct *CrvObj);
void CEditHandleMouse(int x, int y, int Event);
void CEditConstrainedReevalAll(void);
void CEditHandleNonMouseEvents(IGCrvEditEventType Event, int MenuIndex);
void CEditDetachCurve(void);
IPObjectStruct *CEditGetUnitCircle(void);
IPObjectStruct *CEditGetUnitDiamond(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* EDIT_CRVS_H */
