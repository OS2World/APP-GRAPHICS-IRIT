/*****************************************************************************
*  Header file of the generic tools of constraints during curve editing.     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 0.1, June 1999.  *
*****************************************************************************/

#ifndef CNST_CRVS_H
#define CNST_CRVS_H

#include "grap_loc.h"
#include "cagd_lib.h"

#define IG_CRV_EDIT_LST_SQR_DEF_PERCENT 20

typedef enum {
    IG_CRV_CNST_EVENT_ADD,
    IG_CRV_CNST_EVENT_DELETE,
    IG_CRV_CNST_EVENT_DISMISS,
    IG_CRV_CNST_EVENT_SATISFY_ALL,
    IG_CRV_CNST_EVENT_DO_CONSTRAINTS,
    IG_CRV_CNST_EVENT_UNDULATE,
    IG_CRV_CNST_EVENT_X_SYMMETRY,
    IG_CRV_CNST_EVENT_Y_SYMMETRY,
    IG_CRV_CNST_EVENT_Z_SYMMETRY,
    IG_CRV_CNST_EVENT_C_SYMMETRY,
    IG_CRV_CNST_EVENT_AREA,
    IG_CRV_CNST_EVENT_NONE
} IGCrvConstraintEventType;

typedef enum {
    IG_CRV_CNST_POSITION,
    IG_CRV_CNST_TANGENT,
    IG_CRV_CNST_CTL_PT,
    IG_CRV_CNST_X_SYMMETRY,
    IG_CRV_CNST_Y_SYMMETRY,
    IG_CRV_CNST_Z_SYMMETRY,
    IG_CRV_CNST_C_SYMMETRY,
    IG_CRV_CNST_AREA,
    IG_CRV_CNST_NONE
} IGCrvConstraintType;

typedef struct IGCrvConstraintStruct {
    struct IGCrvConstraintStruct *Pnext;
    IGCrvConstraintType Type;
    CagdRType Param;
} IGCrvConstraintStruct;

typedef struct CCnstSolutionWeightsStruct {
    CagdRType *W[3];		             /* Coefficients of X, Y, and Z. */
    int IndexFirst, DataSize; /* Index of first non zero and length of data. */
} CCnstSolutionWeightsStruct;

IRIT_GLOBAL_DATA_HEADER IGCrvConstraintStruct *CrvCnstList;
IRIT_GLOBAL_DATA_HEADER IGCrvConstraintEventType IGCCnstOperation;
IRIT_GLOBAL_DATA_HEADER IGCrvConstraintType IGCCnstType;
IRIT_GLOBAL_DATA_HEADER char *IGCrvCnstAddEntries[];
IRIT_GLOBAL_DATA_HEADER IrtRType IGCrvCnstMaxAllowedCoef;

/* Call back functions that must be supplied by all drivers. */
void IGCrvCnstParamUpdateWidget(void);

/* Functions in cnstcrvs.c that are generic to all drivers. */
void CCnstAddPositionalConstraint(CagdRType Param);
void CCnstAddTangentialConstraint(CagdRType Param);
void CCnstAddControlPtConstraint(int Index);
void CCnstInsertConstraint(IGCrvConstraintStruct *Cnst);
void CCnstDeleteConstraint(int Index);
void CCnstHandleMouse(int x, int y, int Event);
void CCnstHandleNonMouseEvents(IGCrvConstraintEventType Event, int MenuIndex);
void CCnstRedrawConstraints(void);
CCnstSolutionWeightsStruct *CCnstSolveConstraints(IrtVecType TransDir,
						  CagdCrvStruct *ParamCrv,
						  CEditMultiResOneKvStruct
						      *KvsInfo,
						  CagdRType t,
						  int CtlPtIndex,
						  IGCrvConstraintType
						      AdditionalConst,
						  CagdBType *SatisfyAll);
char *CCnstConstraints2Str(void);
void CCnstStr2Constraints(const char *Str);
void CCnstDisableConstraints(void);
void CCnstDetachConstraints(void);

#endif /* CNST_CRVS_H */
