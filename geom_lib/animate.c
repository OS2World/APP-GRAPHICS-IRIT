/*****************************************************************************
*   Animation module - machine independent part. 			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*							Ver 0.1, Feb. 1995.  *
*****************************************************************************/

#ifdef USE_VARARGS
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* USE_VARARGS */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "attribut.h"
#include "allocate.h"
#include "grap_lib.h"
#include "geom_loc.h"

IRIT_STATIC_DATA int
    GlblAnimMatHierarchy = TRUE,
    GlblFileSaveHandler = 0,
    GlblEvalAnimInternalNodes = FALSE;

static CagdRType *EvalCurveObject(IPObjectStruct *CrvObj, IrtRType t);
static void ExecuteAnimation(GMAnimationStruct *Anim, IPObjectStruct *PObjs);
static void ExecuteAnimation2(GMAnimationStruct *Anim, IPObjectStruct *PObjs);
static void ExecuteAnimationAux(GMAnimationStruct *Anim, 
				IPObjectStruct *PObjs,
				IrtHmgnMatType ObjsMat,
				int WasAnim);
static int ExecuteAnimationAux2(GMAnimationStruct *Anim, 
				IPObjectStruct *PObjs,
				IrtHmgnMatType ObjMat);
static void ExecuteAnimationSetMat(IPObjectStruct *PObj,
				   IrtHmgnMatType ObjMat);
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat);

/*****************************************************************************
* DESCRIPTION:								     M
*   Resets the slots of an animation structure.                              M
*									     *
* PARAMETERS:								     M
*   Anim:      The animation state to reset.				     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimResetAnimStruct, animation                                         M
*****************************************************************************/
void GMAnimResetAnimStruct(GMAnimationStruct *Anim)
{
    Anim -> StartT = 0.0;
    Anim -> FinalT = 1.0;
    Anim -> Dt = 0.01;
    Anim -> RunTime = 0.0;
    Anim -> TwoWaysAnimation = FALSE;
    Anim -> SaveAnimationGeom = FALSE;
    Anim -> SaveAnimationImage = FALSE;
    Anim -> BackToOrigin = FALSE;
    Anim -> NumOfRepeat = 1;
    Anim -> StopAnim = FALSE;
    Anim -> SingleStep = FALSE;
    Anim -> TextInterface = TRUE;
    Anim -> MiliSecSleep = 30;
    Anim -> ExecEachStep = NULL;
    strcpy(Anim -> BaseFileName, GM_ANIM_DEFAULT_FILE_NAME);
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Getting input parameters of animation from user using textual user       M
* interface.								     M
*									     *
* PARAMETERS:								     M
*   Anim:      The animation state to update.				     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimGetAnimInfoText, animation                                         M
*****************************************************************************/
void GMAnimGetAnimInfoText(GMAnimationStruct *Anim)
{
#ifndef IRIT_QUIET_STRINGS
    char Line[IRIT_LINE_LEN];

#ifdef IRIT_DOUBLE
#    define REAL_FRMT "%lf"
#else
#    define REAL_FRMT "%f"
#endif /* IRIT_DOUBLE */
    do {
	printf("Start time [%f] : ", Anim -> StartT);
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
    }
    while (Line[0] >= ' ' &&
	   sscanf(Line, REAL_FRMT, &Anim -> StartT) != 1);

    do {
	printf("Final time [%f] : ", Anim -> FinalT);
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
    }
    while (Line[0] >= ' ' &&
	   sscanf(Line, REAL_FRMT, &Anim -> FinalT) != 1);

    do {
	printf("Interval of time [%f] : ", Anim -> Dt);
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
    }
    while (Line[0] >= ' ' &&
	   sscanf(Line, REAL_FRMT, &Anim -> Dt) != 1);

    printf("\nSpecial Commands (y/n) [n] : ");
    fgets(Line, IRIT_LINE_LEN - 1, stdin);
    if (Line[0] != 'y' && Line[0] != 'Y') {
    	Anim -> TwoWaysAnimation = FALSE;
    	Anim -> SaveAnimationGeom = FALSE;
    	Anim -> SaveAnimationImage = FALSE;
	Anim -> BackToOrigin = FALSE;
    	Anim -> NumOfRepeat = 1;
    	Anim -> MiliSecSleep = 30;
    	return;
    }

    printf("Bounce Animation (y/n) [n] : ");
    fgets(Line, IRIT_LINE_LEN - 1, stdin);
    if (Line[0] == 'y' || Line[0] == 'Y') {
      	Anim -> TwoWaysAnimation = TRUE;
	Anim -> BackToOrigin = FALSE;
    }
    else {
      	Anim -> TwoWaysAnimation = FALSE;
	printf("Back to origin (y/n) [n] : ");
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
	if (Line[0] == 'y' || Line[0] == 'Y')
	    Anim -> BackToOrigin = TRUE;
	else
	    Anim -> BackToOrigin = FALSE;
    }

    do {
	printf("Number of Repeatitions [%d] : ", Anim -> NumOfRepeat);
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
    }
    while (Line[0] >= ' ' && sscanf(Line, "%d", &Anim -> NumOfRepeat) != 1);
    Anim -> NumOfRepeat = IRIT_MAX(Anim -> NumOfRepeat, 1);

    do {
	printf("Mili Seconds' Sleep [%d] : ", Anim -> MiliSecSleep);
	fgets(Line, IRIT_LINE_LEN - 1, stdin);
    }
    while (Line[0] >= ' ' && sscanf(Line, "%d", &Anim -> MiliSecSleep) != 1);
    Anim -> MiliSecSleep = IRIT_BOUND(Anim -> MiliSecSleep, 0, 10000);

    printf("Save iterations into data files (y/n) [n] : ");
    fgets(Line, IRIT_LINE_LEN - 1, stdin);
    if (Line[0] == 'y' || Line[0] == 'Y') {
	Anim -> SaveAnimationGeom = TRUE; 
	do {
	    printf("Base name of data files : ");
	    fgets(Line, IRIT_LINE_LEN - 1, stdin);
	}
	while (Line[0] >= ' ' &&
	       sscanf(Line, "%s", Anim -> BaseFileName) != 1 &&
	       IRT_STR_ZERO_LEN(Anim -> BaseFileName));
    } 
    else
	Anim -> SaveAnimationGeom = FALSE;
#endif /* !IRIT_QUIET_STRINGS */
}

/*****************************************************************************
* DESCRIPTION:							     	     *
*   Evaluate curve in specified parameter value and project it to Euclidean  *
* space, if necessary.							     *
*									     *
* PARAMETERS:								     *
*   CrvObj:	a pointer to the curve's object.			     *
*   t:		the parameter of time to calculate the value of the curve.   * 
*									     *
* RETURN VALUE:								     *
*   CagdRType *: The value(s) of the curve at t.			     *
*****************************************************************************/
static CagdRType *EvalCurveObject(IPObjectStruct *CrvObj, IrtRType t)
{
    int i;
    CagdRType
	*Pt = CagdCrvEval(CrvObj -> U.Crvs, t);

    switch (CrvObj -> U.Crvs -> PType)  {
        case CAGD_PT_E1_TYPE:
        case CAGD_PT_E3_TYPE:
             break;
        case CAGD_PT_P1_TYPE:
	     Pt[1] /= Pt[0];
	     break;
        case CAGD_PT_P3_TYPE:
             for (i = 1; i <= 3; i++)
	          Pt[i] /= Pt[0];
	     break;
         default:
	     break;
    }
    return Pt;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Scan the given geometry for possible animation attributes.		     M
*									     *
* PARAMETERS:								     M
*   PObjs:      Objects to scan for animation attributes.		     M
*									     *
* RETURN VALUE:								     M
*   int:	TRUE if there are animation attributes. FALSE otherwise.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimHasAnimationOne                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimHasAnimation, animation     		                             M
*****************************************************************************/
int GMAnimHasAnimation(const IPObjectStruct *PObjs)
{
    const IPObjectStruct *PObj;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
        if (GMAnimHasAnimationOne(PObj))
	    return TRUE;
    }

    return FALSE;   
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Scan the given geometry for possible animation attributes.		     M
*									     *
* PARAMETERS:								     M
*   PObj:      One object to scan for animation attributes.		     M
*									     *
* RETURN VALUE:								     M
*   int:	TRUE if there are animation attributes. FALSE otherwise.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimHasAnimation                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimHasAnimationOne, animation  		                             M
*****************************************************************************/
int GMAnimHasAnimationOne(const IPObjectStruct *PObj)
{
    if (AttrGetObjectObjAttrib(PObj, "animation") != NULL)
        return TRUE;

    if (IP_IS_OLST_OBJ(PObj)) {
        int i = 0;
	const IPObjectStruct *PTmp;

	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL) {
	    if (GMAnimHasAnimationOne(PTmp))
	        return TRUE;
	}
    }

    return FALSE;   
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Computes the time span for which the animation executes.		     M
*									     *
* PARAMETERS:								     M
*   Anim:	Animation structure to update.				     M
*   PObjs:      Objects to scan for animation attributes.		     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimFindAnimationTimeOne                                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimFindAnimationTime, animation                                       M
*****************************************************************************/
void GMAnimFindAnimationTime(GMAnimationStruct *Anim, 
			     const IPObjectStruct *PObjs)
{
    const IPObjectStruct *PObj;

    if (!GMAnimHasAnimation(PObjs))
	return;

    Anim -> RunTime = Anim -> StartT = IRIT_INFNTY;
    Anim -> FinalT = -IRIT_INFNTY;

    for (PObj = PObjs; PObj != NULL; PObj = PObj -> Pnext) {
        GMAnimationStruct Anim2;

	Anim2.RunTime = Anim2.StartT = IRIT_INFNTY;
	Anim2.FinalT = -IRIT_INFNTY;

	GMAnimFindAnimationTimeOne(&Anim2, PObj);

	if (Anim2.StartT < Anim -> StartT)
	    Anim -> RunTime = Anim -> StartT = Anim2.StartT;
	if (Anim2.FinalT > Anim -> FinalT)
	    Anim -> FinalT = Anim2.FinalT;
    }
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Computes the time span for which the animation executes.		     M
*									     *
* PARAMETERS:								     M
*   Anim:	Animation structure to update.				     M
*   PObj:       One object to scan for animation attributes.		     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimFindAnimationTime                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimFindAnimationTimeOne, animation                                    M
*****************************************************************************/
void GMAnimFindAnimationTimeOne(GMAnimationStruct *Anim,
				const IPObjectStruct *PObj)
{
    IPObjectStruct *ObjPtr, *AnimationP;
    IrtRType T1, T2,
	StartT = IRIT_INFNTY,
	FinalT = -IRIT_INFNTY;

    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
	IPObjectStruct *PTmp;

	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL) {
	    GMAnimationStruct Anim2;

	    Anim2.RunTime = Anim2.StartT = IRIT_INFNTY;
	    Anim2.FinalT = -IRIT_INFNTY;

	    GMAnimFindAnimationTimeOne(&Anim2, PTmp);

	    StartT = IRIT_MIN(StartT, Anim2.StartT);
	    FinalT = IRIT_MAX(FinalT, Anim2.FinalT);
	}
    }
    else if ((AnimationP = AttrGetObjectObjAttrib(PObj, "animation")) != NULL) {
	if (IP_IS_OLST_OBJ(AnimationP)) {
	    int i = 0;

	    while ((ObjPtr = IPListObjectGet(AnimationP, i++)) != NULL) {
		if (IP_IS_CRV_OBJ(ObjPtr)) {
		    CagdCrvDomain(ObjPtr -> U.Crvs, &T1, &T2);
		    StartT = IRIT_MIN(StartT, T1);
		    FinalT = IRIT_MAX(FinalT, T2);
		}
	    }
	}
	else if (IP_IS_CRV_OBJ(AnimationP)) {
	    CagdCrvDomain(AnimationP -> U.Crvs, &T1, &T2);
	    StartT = IRIT_MIN(StartT, T1);
	    FinalT = IRIT_MAX(FinalT, T2);
	}
    }

    Anim -> RunTime = Anim -> StartT = StartT;
    Anim -> FinalT = FinalT;
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Executes one time step of the animation. 				     *
*									     *
* PARAMETERS:								     *
*   Anim:	Animation structure.					     *
*   PObjs:	Objects to process.					     *
*									     *
* RETURN VALUE:								     *
*   void 								     *
*****************************************************************************/
static void ExecuteAnimation(GMAnimationStruct *Anim, IPObjectStruct *PObjs)
{
    IPObjectStruct *PObj;
    IrtHmgnMatType ObjsMat, ObjMat;

    if (Anim -> TextInterface) {
	printf("\b\b\b\b\b\b\b%7.3f", Anim -> RunTime);
	fflush(stdout);
    }
    MatGenUnitMat(ObjsMat);

    for (PObj = PObjs;
	 PObj != NULL && !Anim -> StopAnim;
	 PObj = PObj -> Pnext) {
	IRIT_GEN_COPY(ObjMat, ObjsMat, sizeof(IrtHmgnMatType));
	ExecuteAnimationAux(Anim, PObj, ObjMat, FALSE);
	if (GMAnimCheckInterrupt(Anim))
	    break;
    }

    IGRedrawViewWindow();
    IritSleep(Anim -> MiliSecSleep);
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Executes one time step of the animation. 				     *
*									     *
* PARAMETERS:								     *
*   Anim:	Animation structure.					     *
*   PObjs:	Objects to process.					     *
*									     *
* RETURN VALUE:								     *
*   void 								     *
*****************************************************************************/
static void ExecuteAnimation2(GMAnimationStruct *Anim, IPObjectStruct *PObjs)
{
    IPObjectStruct *PObj;
    IrtHmgnMatType ObjsMat, ObjMat;

    MatGenUnitMat(ObjsMat);
    
    for (PObj = PObjs ; PObj != NULL && !Anim -> StopAnim ; 
	 PObj = PObj -> Pnext) { 
	IRIT_GEN_COPY(ObjMat, ObjsMat, sizeof(IrtHmgnMatType));
	ExecuteAnimationAux(Anim, PObj, ObjMat, FALSE);
    }
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Auxiliary function of ExecuteAnimation. Traverses the hierarchical       *
* object structure of the data.						     *
*****************************************************************************/
static void ExecuteAnimationAux(GMAnimationStruct *Anim, 
				IPObjectStruct *PObj,
				IrtHmgnMatType ObjsMat,
				int WasAnim)
{
    int HasAnim = ExecuteAnimationAux2(Anim, PObj, ObjsMat) || WasAnim;
    
    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
	IPObjectStruct *PTmp;
	IrtHmgnMatType ObjMat;

	while ((PTmp = IPListObjectGet(PObj, i++)) != NULL) {
	    IRIT_GEN_COPY(ObjMat, ObjsMat, sizeof(IrtHmgnMatType));
	    ExecuteAnimationAux(Anim, PTmp, ObjMat, HasAnim);
	}
    }
    else {
        if (!GlblEvalAnimInternalNodes) {
	    if (!MatIsUnitMatrix(ObjsMat, IRIT_EPS) || HasAnim) {
	        ExecuteAnimationSetMat(PObj, ObjsMat);
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:								     *
*   Auxiliary function of ExecuteAnimation. Processes a single object.       *
*   Returns TRUE if found "animation".					     *
*****************************************************************************/
static int ExecuteAnimationAux2(GMAnimationStruct *Anim, 
				IPObjectStruct *PObj,
				IrtHmgnMatType ObjMat)
{
    IPObjectStruct *AnimationP;
    int WasAnim = FALSE;

    if ((AnimationP = AttrGetObjectObjAttrib(PObj, "animation")) != NULL) {
        IrtRType t,
	    Time = Anim -> RunTime;

	if ((t = AttrGetObjectRealAttrib(PObj, "animtime")) !=
							    IP_ATTR_BAD_REAL)
	    Time = t;

	if (Time != GM_ANIM_NO_DEFAULT_TIME) {
	    IrtRType
	        Vis = GMExecuteAnimationEvalMat(AnimationP, Time, ObjMat);

	    if (Vis >= 0.0)
	        AttrSetObjectRealAttrib(PObj, "_isvisible", Vis);
	}

	if (GlblEvalAnimInternalNodes) {
	    ExecuteAnimationSetMat(PObj, ObjMat);
	}

	WasAnim = TRUE;
    }
    
    return WasAnim;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Update PObj with a a new animation matrix.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:     Object to update its animation matrix.                         *
*   MatObj:   The matrix to update PObj with.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ExecuteAnimationSetMat(IPObjectStruct *PObj,
				   IrtHmgnMatType ObjMat)
{
    IPObjectStruct *OldMatObj;

    /* Copy over existing matrix if has one - faster and thread-safer. */
    if ((OldMatObj = AttrGetObjectObjAttrib(PObj, "_animation_mat")) != NULL) {
        IRIT_HMGN_MAT_COPY(OldMatObj -> U.Mat, ObjMat);
    }
    else {
        AttrSetObjectObjAttrib(PObj, "_animation_mat",
			       IPGenMatObject("transform",
					      ObjMat, NULL), FALSE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Auxiliary function of ExecuteAnimation. Processes a linked list of       M
* objects.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   AnimationP:    A (list of) animation curve(s) to evaluate at time t.     M
*   Time:	   Time of animation.					     M
*   ObjMat:        A matrix to chain the new animation matrices into.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:	   Positive if visible (between zero and one hints on	     M
*		   opacity), 0.0 otherwise, -1.0 if no visible curve found.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMExecuteAnimationEvalMat                                                M
*****************************************************************************/
IrtRType GMExecuteAnimationEvalMat(IPObjectStruct *AnimationP,
				   IrtRType Time,
				   IrtHmgnMatType ObjMat)
{
    IPObjectStruct *PObj;
    IrtHmgnMatType Mat;
    int NeedToMult,
	i = 0;
    IrtRType
	Visible = -1.0;

    if (!GlblAnimMatHierarchy)
        MatGenUnitMat(ObjMat);

    while (IP_IS_OLST_OBJ(AnimationP) ?
	   (PObj = IPListObjectGet(AnimationP, i++)) != NULL :
	   (PObj = (i++ == 0 ? AnimationP : NULL)) != NULL) {
	CagdRType *CurveResult, CurveRes, TMin, TMax, t;
	char
	    *Name = IP_GET_OBJ_NAME(PObj); 

	NeedToMult = TRUE;
	switch (PObj -> ObjType) {
	    case IP_OBJ_MATRIX:
	        IRIT_GEN_COPY(Mat, *PObj -> U.Mat, sizeof(IrtHmgnMatType)); 
		break;
	    case IP_OBJ_CURVE:
		CagdCrvDomain(PObj -> U.Crvs, &TMin, &TMax);

		MatGenUnitMat(Mat);

		t = Time;
		if (t < TMin)
		    t = TMin;
		else if (t > TMax)
		    t = TMax;
		    
		CurveResult = EvalCurveObject(PObj, t); 
		CurveRes = CurveResult[1];
		if (strnicmp(Name, "scl", 3) == 0) {
		    if (strnicmp(Name, "scl_x", 5) == 0)
		        MatGenMatScale(CurveRes, 1, 1, Mat);
		    else if (strnicmp(Name, "scl_y", 5) == 0)
		        MatGenMatScale(1, CurveRes, 1, Mat);
		    else if (strnicmp(Name, "scl_z", 5) == 0)
		        MatGenMatScale(1, 1, CurveRes, Mat);
		    else
		        MatGenMatUnifScale(CurveRes, Mat);
		}
		else if (strnicmp(Name, "rot", 3) == 0) {
		    if (strnicmp(Name, "rot_x", 5) == 0)
		        MatGenMatRotX1(-IRIT_DEG2RAD(CurveRes), Mat );
		    else if (strnicmp(Name, "rot_y", 5) == 0)
		        MatGenMatRotY1(-IRIT_DEG2RAD(CurveRes), Mat);
		    else if (strnicmp(Name, "rot_z", 5) == 0)
		        MatGenMatRotZ1(-IRIT_DEG2RAD(CurveRes), Mat);
		}
		else if (strnicmp(Name, "mov", 3) == 0) {
		    if (strnicmp(Name, "mov_xyz", 7) == 0)
		        MatGenMatTrans(CurveResult[1],
				       CurveResult[2],
				       CurveResult[3],
				       Mat);
		    else if (strnicmp(Name, "mov_x", 5) == 0)
		        MatGenMatTrans(CurveRes, 0, 0, Mat);
		    else if (strnicmp(Name, "mov_y", 5) == 0)
		        MatGenMatTrans(0, CurveRes, 0, Mat);
		    else if (strnicmp(Name, "mov_z", 5) == 0)
		        MatGenMatTrans(0, 0, CurveRes, Mat);
		}
		else if (strnicmp(Name, "quaternion", 10) == 0) {
		    GMQuatNormalize(&CurveResult[1]);
		    GMQuatToMat(&CurveResult[1], Mat);
		}
		else if (strnicmp(Name, "visible", 7) == 0) { 
		    NeedToMult = FALSE;
		    Visible = IRIT_BOUND(CurveRes, 0.0, 1.0);
		}
		else {
		    GEOM_FATAL_ERROR(GEOM_ERR_UNKNOWN_ANIM_CRVS);
		}
		break;
	    default:
		NeedToMult = FALSE;
		GEOM_FATAL_ERROR(GEOM_ERR_ANIM_MAT_OR_CRV);
		break;
	}

	if (NeedToMult)
	    MatMultTwo4by4(ObjMat, ObjMat, Mat);
    }

    return Visible;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Routine to run a sequence of objects through an animation according to   M
* animation attributes of matrices and curves that are attached to them.     M
* 									     *
* PARAMETERS:								     M
*   Anim:	Animation structure.					     M
*   PObjs:	Objects to render.					     M
*									     *
* RETURN VALUE:								     M
*   void.								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimEvalAnimation                                                      M
*									     *
* KEYWORDS:								     M
*   GMAnimDoAnimation, animation					     M
*****************************************************************************/
void GMAnimDoAnimation(GMAnimationStruct *Anim, IPObjectStruct *PObjs)
{ 
    int Loops;

    Anim -> StopAnim = FALSE;

    if (!GMAnimHasAnimation(PObjs)) {         /* Checking for any animation. */
        return;
    }

    if (Anim -> TextInterface) {
	printf(IRIT_EXP_STR("Animate from %f to %f step %f\n"),
	       Anim -> StartT, Anim -> FinalT, Anim -> Dt);
	printf(IRIT_EXP_STR("\nAnimation time:        "));
    }

    Anim -> _Count = 1;
    for (Loops = 1; Loops <= Anim -> NumOfRepeat; Loops++) {
        for (Anim -> RunTime = Anim -> StartT;
	     Anim -> RunTime <= Anim -> FinalT + IRIT_EPS &&
	     !Anim -> StopAnim;
	     Anim -> RunTime += Anim -> Dt) {
    	    ExecuteAnimation(Anim, PObjs);
	    if (Loops == 1) {
		if (Anim -> SaveAnimationGeom)
		    GMAnimSaveIterationsToFiles(Anim, PObjs);
		if (Anim -> SaveAnimationImage)
		    GMAnimSaveIterationsAsImages(Anim, PObjs);
		if (Anim -> ExecEachStep != NULL) {
		    char Line[IRIT_LINE_LEN];

		    sprintf(Line, "%s %d",
			    Anim -> ExecEachStep, Anim -> _Count++);
		    system(Line);
		}
	    }
	}

    	if (Anim -> TwoWaysAnimation) 
	    for (Anim -> RunTime = Anim -> FinalT;
		 Anim -> RunTime >= Anim -> StartT - IRIT_EPS &&
		 !Anim -> StopAnim;
		 Anim -> RunTime -= Anim -> Dt)
 	    	ExecuteAnimation(Anim, PObjs);
    }

    if (Anim -> BackToOrigin && !IRIT_APX_EQ(Anim -> RunTime, Anim -> StartT)) {
	Anim -> RunTime = Anim -> StartT;
	ExecuteAnimation(Anim, PObjs);
    }

    if (Anim -> TextInterface) {
	printf(IRIT_EXP_STR("\n\nAnimation is done.\n"));
	fflush(stdout);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls the way animation matrices are computed in a hierarchy of       M
* parts (object tree).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   AnimMatHierarchy:    TRUE for animation matrices computed accumulative,  M
*			 FALSE for each node computed individually.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old settings.                                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimEvalAnimation                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimSetAnimMatHierarchy                                                M
*****************************************************************************/
int GMAnimSetAnimMatHierarchy(int AnimMatHierarchy)
{
    int OldVal = GlblAnimMatHierarchy;

    GlblAnimMatHierarchy = AnimMatHierarchy;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allows animation transformations to be saved at internal nodes.          M
*                                                                            *
* PARAMETERS:                                                                M
*   AnimInternalNodes:    New setting for internal animation nodes.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old settings.                                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimEvalAnimation                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimSetAnimInternalNodes                                               M
*****************************************************************************/
int GMAnimSetAnimInternalNodes(int AnimInternalNodes)
{
    int OldVal = GlblEvalAnimInternalNodes;

    GlblEvalAnimInternalNodes = AnimInternalNodes;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the animation curves at the given time, setting the proper      M
* animation attributes ("_animation_mat" and "_isvisible"), in place.        M
*                                                                            *
* PARAMETERS:                                                                M
*   t:         Time to evaluate the animation at.                            M
*   PObj:      To evaluate their animation curves.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimDoAnimation, GMAnimEvalAnimationList, GMAnimSetAnimInternalNodes,  M
*   GMAnimEvalObjAtTime                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimEvalAnimation, animation                                           M
*****************************************************************************/
void GMAnimEvalAnimation(IrtRType t, IPObjectStruct *PObj)
{
    GMAnimationStruct Anim;

    GMAnimResetAnimStruct(&Anim);

    Anim.StartT = t;
    Anim.FinalT = t + IRIT_EPS;
    Anim.Dt = t + 1.0;
    Anim.RunTime = t;
    Anim.TextInterface = FALSE;

    ExecuteAnimation2(&Anim, PObj);
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Evaluate the animation curves at the given time, and creating the object M
* in the proper place in time.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   t:         Time to evaluate the animation at.                            M
*   PObj:      To evaluate their animation curves.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Input object positioned at time t.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimDoAnimation, GMAnimEvalAnimationList, GMAnimSetAnimInternalNodes   M
*   GMAnimEvalAnimation                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimEvalObjAtTime, animation					     M
*****************************************************************************/
IPObjectStruct *GMAnimEvalObjAtTime(IrtRType t, IPObjectStruct *PObj)
{
    int OldRefCopy = IPSetCopyObjectReferenceCount(FALSE);
    IrtHmgnMatType Mat;
    GMAnimationStruct Anim;
    IPObjectStruct *PObj2;

    PObj = IPCopyObject(NULL, PObj, FALSE);        /* So we could modify it. */
    IPSetCopyObjectReferenceCount(OldRefCopy);

    GMAnimResetAnimStruct(&Anim);

    Anim.StartT = t;
    Anim.FinalT = t + IRIT_EPS;
    Anim.Dt = t + 1.0;
    Anim.RunTime = t;
    Anim.TextInterface = FALSE;

    ExecuteAnimation2(&Anim, PObj);

    MatGenUnitMat(Mat);
    IPTraverseObjListHierarchy(PObj, Mat, IPMapObjectInPlace);

    /* Make the reference counts one instead of zero. */
    PObj2 = IPCopyObject(NULL, PObj, FALSE);
    IPListObjectInsert(PObj2, 0, NULL);
    IPFreeObject(PObj2);

    return PObj;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the animation curves at the given time, setting the proper      M
* animation attributes ("_animation_mat" and "_isvisible"), in place.        M
*                                                                            *
* PARAMETERS:                                                                M
*   t:         Time to evaluate the animation at.                            M
*   PObjList:  A list of objects to evaluate their animation curves.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimDoAnimation, GMAnimEvalAnimation, GMAnimSetAnimInternalNodes       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimEvalAnimationList, animation                                       M
*****************************************************************************/
void GMAnimEvalAnimationList(IrtRType t, IPObjectStruct *PObjList)
{
    IPObjectStruct *PObj;

    for (PObj = PObjList; PObj != NULL; PObj = PObj -> Pnext)
        GMAnimEvalAnimation(t, PObj);
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Routine to exectue a single step the animation, at current time.         M
* 									     *
* PARAMETERS:								     M
*   Anim:	Animation structure.					     M
*   PObjs:	Objects to render.					     M
*									     *
* RETURN VALUE:								     M
*   void.								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMAnimDoAnimation, GMAnimEvalAnimation, GMAnimSetAnimInternalNodes       M
*									     *
* KEYWORDS:								     M
*   GMAnimDoSingleStep, animation					     M
*****************************************************************************/
void GMAnimDoSingleStep(GMAnimationStruct *Anim, IPObjectStruct *PObjs)
{ 
    Anim -> StopAnim = FALSE;

    if (!GMAnimHasAnimation(PObjs)) {         /* Checking for any animation. */
        return;
    }

    ExecuteAnimation(Anim, PObjs);

    if (Anim -> SaveAnimationGeom)
        GMAnimSaveIterationsToFiles(Anim, PObjs);
    if (Anim -> SaveAnimationImage)
        GMAnimSaveIterationsAsImages(Anim, PObjs);
    if (Anim -> ExecEachStep != NULL) {
	char Line[IRIT_LINE_LEN];

	sprintf(Line, "%s %d",
		Anim -> ExecEachStep, Anim -> _Count++);
	system(Line);
    }
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Saves one iteration of the animation sequence as IRIT data (*.itd).      M
*   The objects that are saved are those that are visibled on the current    M
* time frame as set via current animation_mat attribute.	             M
*									     *
* PARAMETERS:								     M
*   Anim:	Animation structure.					     M
*   PObjs:	Objects to render.					     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*									     *
* KEYWORDS:								     M
*   GMAnimSaveIterationsToFiles, animation				     M
*****************************************************************************/
void GMAnimSaveIterationsToFiles(GMAnimationStruct *Anim,
				 IPObjectStruct *PObjs)
{
    IPObjectStruct *PObj;
    char FileName[IRIT_LINE_LEN];
    IrtHmgnMatType ViewMat;

    sprintf(FileName, "%s%03d.itd", Anim -> BaseFileName, Anim -> _Count++);
    GlblFileSaveHandler = IPOpenDataFile(FileName, FALSE, TRUE);

    MatGenUnitMat(ViewMat);
    IPTraverseObjListHierarchy(PObjs, ViewMat, DumpOneTraversedObject);

    PObj = IPGenMatObject("view_mat", IPViewMat, NULL);
    IPPutObjectToHandler(GlblFileSaveHandler, PObj);
    IPFreeObject(PObj);
    if (IGGlblViewMode == IG_VIEW_PERSPECTIVE) {
	PObj = IPGenMatObject("prsp_mat", IPPrspMat, NULL);
	IPPutObjectToHandler(GlblFileSaveHandler, PObj);
    	IPFreeObject(PObj);
    }

    IPCloseStream(GlblFileSaveHandler, TRUE); 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of IPTraverseObjListHierarchy. Called on every non    *
* list object found in hierarchy.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Non list object to handle.                                   *
*   Mat:        Transformation matrix to apply to this object.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DumpOneTraversedObject(IPObjectStruct *PObj, IrtHmgnMatType Mat)
{
    IPObjectStruct *CopyObj, *CopyTObj;

    CopyObj = IPCopyObject(NULL, PObj, TRUE);
    AttrFreeOneAttribute(&CopyObj -> Attr, "animation");
    CopyObj -> Pnext = NULL;
    CopyTObj = GMTransformObject(CopyObj, Mat);

    IPPutObjectToHandler(GlblFileSaveHandler, CopyTObj);

    IPFreeObject(CopyObj);
    IPFreeObject(CopyTObj);
}
