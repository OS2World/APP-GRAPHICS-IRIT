/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to handle the high level Boolean operations. The other modules    *
* should only call this module to perform Boolean operations. All the        *
* operations are none-destructives, meaning the given data is not modified.  *
*   Note all the polygons of the two given objects must be convex, and the   *
* returned object will also have only convex polygons!			     *
*****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "irit_sm.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_loc.h"
#include "geom_lib.h"

IRIT_STATIC_DATA jmp_buf LclLongJumpBuffer;  /* Used in fatal Boolean error. */
IRIT_STATIC_DATA int FatalErrorType;	     /* Type of fatal Boolean error. */
IRIT_STATIC_DATA BoolOperType BooleanOperation;	/* One of BOOL_OPER_OR, etc. */
IRIT_STATIC_DATA IrtRType
    BoolPerturbAmount = 1e-6;

IRIT_GLOBAL_DATA BoolFatalErrorFuncType
    _BoolActiveFatalErrorFunc = BoolDfltFatalError;

IRIT_GLOBAL_DATA IrtRType
    _BoolGlobalScale = 1.0;

IRIT_GLOBAL_DATA int
    BoolHandleCoplanarPoly = FALSE,     /* Whether to handle coplanar polys. */
    BoolFoundCoplanarPoly = FALSE,   /* If coplanar polygons actually found. */
    BoolOutputInterCurve = FALSE,	/* Kind of output from Boolean oper. */
    BoolParamSurfaceUVVals = FALSE;  /* Do surface Boolean should return UV. */

static void BoolPropObjectID(IPObjectStruct *PObj);
static int BooleanInputCoplanar2D(IPObjectStruct *PObj1,
				  IPObjectStruct *PObj2);
static IPObjectStruct *BooleanCombineThreeObjs(IPObjectStruct *PObj1,
					       IPObjectStruct *PObj2,
					       IPObjectStruct *PObj3);
static IPObjectStruct *BooleanCoplanar(IPObjectStruct *PObj1,
				       IPObjectStruct *PObj2,
				       BoolOperType BoolOper);
static IPObjectStruct *VerifyBooleanInput(IPObjectStruct *PObj1,
					  IPObjectStruct *PObj2,
					  BoolOperType Oper);

static void BooleanFPE(int Type);

#ifdef DEBUG
static void PrintVrtxList(IPVertexStruct *V);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verify input for Booleans. Ret. NULL if OK, otherwise an obj to return.  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1:     First object for to be performed Boolean.                     *
*   PObj2:     Second object for to be performed Boolean.                    *
*   Oper:      Type of operation (and, or, etc.).                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct:      NULL if Boolean operation can/should be performed.  *
*                        Otherwise, an object to return as Boolean result.   *
*****************************************************************************/
static IPObjectStruct *VerifyBooleanInput(IPObjectStruct *PObj1,
					  IPObjectStruct *PObj2,
					  BoolOperType Oper)
{
    IPObjectStruct *PObj;
    IPPolygonStruct *Pl;
    GMBBBboxStruct *BBox;
    IrtRType d1, d2;

    BooleanOperation = Oper;

    if (!IP_IS_POLY_OBJ(PObj1) || (PObj2 != NULL && !IP_IS_POLY_OBJ(PObj2)))
	BOOL_FATAL_ERROR(BOOL_ERR_NO_POLY_OBJ);

    if (PObj1 -> U.Pl == NULL || (PObj2 != NULL && PObj2 -> U.Pl == NULL))
	BOOL_FATAL_ERROR(BOOL_ERR_EMPTY_POLY_OBJ);

    signal(SIGFPE, BooleanFPE);		/* Will trap floating point errors. */

    BoolPropObjectID(PObj1);
    if (PObj2 != NULL) 
        BoolPropObjectID(PObj2);

#ifdef DEBUG
    /* If a real Boolean operation, verify plane equations defined. */
    if (PObj2 != NULL && !IP_IS_POLYLINE_OBJ(PObj2)) {
        for (Pl = PObj1 -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
            if (!IP_HAS_PLANE_POLY(Pl))
	        assert(0);
	for (Pl = PObj2 -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
	    if (!IP_HAS_PLANE_POLY(Pl))
	        assert(0);
    }
#endif /* DEBUG */

    /* Estimate size for relative comparisons. */
    _BoolGlobalScale = 1.0;
    BBox = GMBBComputeBboxObject(PObj1);
    d1 = IRIT_MAX(BBox -> Max[0] - BBox -> Min[0],
	          IRIT_MAX(BBox -> Max[1] - BBox -> Min[1],
			   BBox -> Max[2] - BBox -> Min[2]));
    if (PObj2 != NULL) {
        BBox = GMBBComputeBboxObject(PObj2);
	d2 = IRIT_MAX(BBox -> Max[0] - BBox -> Min[0],
		 IRIT_MAX(BBox -> Max[1] - BBox -> Min[1],
			  BBox -> Max[2] - BBox -> Min[2]));
    }
    else
        d2 = 1.0;
    /* Intersections, if any, are bounded by the minimum of these two sizes. */
    if (d1 > 10.0 && d2 > 10.0)
        _BoolGlobalScale = IRIT_MIN(d1, d2);
    else if (d1 < 0.1 && d2 < 0.1)
        _BoolGlobalScale = IRIT_MIN(d1, d2);

    switch (Oper) {
	case BOOL_OPER_OR:
	case BOOL_OPER_AND:
	    if (IP_IS_POLYLINE_OBJ(PObj1) && IP_IS_POLYLINE_OBJ(PObj2)) {
		if (Oper == BOOL_OPER_OR) {
		    /* Merge the polylines into one large list. */
		    if (PObj1 -> U.Pl == NULL)
		        PObj = IPCopyObject(NULL, PObj2, FALSE);
		    else {
			PObj = IPCopyObject(NULL, PObj1, FALSE);
			Pl = IPGetLastPoly(PObj -> U.Pl);
			Pl -> Pnext = IPCopyPolygonList(PObj2 -> U.Pl);
		    }
		    return PObj;
		}
		else if (Oper == BOOL_OPER_AND) {
		    int i;
		    IPPolygonStruct *Pl1, *Pl2;
		    Bool2DInterStruct *Pt,
		        *PtsList = NULL;
		    IPObjectStruct
			*PObjList = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

		    for (Pl1 = PObj1 -> U.Pl;
			 Pl1 != NULL;
			 Pl1 = Pl1 -> Pnext) {
			for (Pl2 = PObj2 -> U.Pl;
			     Pl2 != NULL;
			     Pl2 = Pl2 -> Pnext) {
			    Bool2DInterStruct
			        *PtsOneInterList =
				    Boolean2DComputeInters(Pl1, Pl2,
							   FALSE, FALSE);

			    if (PtsOneInterList != NULL) {
				Bool2DInterStruct *PtLast;

				for (PtLast = PtsOneInterList;
				     PtLast -> Pnext != NULL;
				     PtLast = PtLast -> Pnext);
				PtLast -> Pnext = PtsList;
				PtsList = PtsOneInterList;
			    }
			}
		    }

		    /* Compute the intersection points of the polylines. */

		    for (Pt = PtsList, i = 0; Pt != NULL; ) {
			Bool2DInterStruct
			    *PtNext = Pt -> Pnext;

			IPListObjectInsert(PObjList, i++,
					   IPGenPTObject(&Pt -> InterPt[0],
							 &Pt -> InterPt[1],
							 &Pt -> InterPt[2]));
			IritFree(Pt);
			Pt = PtNext;
		    }
		    IPListObjectInsert(PObjList, i, NULL);
		    return PObjList;
		}
	    }
	case BOOL_OPER_SUB:
	case BOOL_OPER_CUT:
	case BOOL_OPER_ICUT:
	case BOOL_OPER_MERGE:
	case BOOL_OPER_NEG:
	case BOOL_OPER_SELF:
	case BOOL_OPER_CONTOUR:
    	    if (IP_IS_POLYLINE_OBJ(PObj1) ||
		(PObj2 != NULL && IP_IS_POLYLINE_OBJ(PObj2))) {
    	    	IRIT_WARNING_MSG(
		    "Boolean: illegal operation on mixed polygon/line geometric object(s).");
		PObj = IPGenPolyObject("", NULL, NULL);
		return PObj;
    	    }

	    if (Oper != BOOL_OPER_NEG && Oper != BOOL_OPER_MERGE) {
	    	GMConvexPolyObject(PObj1); /* Make sure all polys are convex.*/
		if (PObj2 != NULL)
		    GMConvexPolyObject(PObj2);
	    }

	    return NULL;
	default:
    	    BOOL_FATAL_ERROR(BOOL_ERR_NO_BOOL_OP_SUPPORT);
    	    return NULL;			     /* Make warning silent. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Propagate object ID to polygons if has one.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:   Object to propagate obj IDs to its polygons.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolPropObjectID(IPObjectStruct *PObj)
{
    int ID;

    if (PObj == NULL)
        return;

    ID = AttrGetIntAttrib(PObj -> Attr, "ID");

    if (!IP_ATTR_IS_BAD_INT(ID)) {
        IPPolygonStruct
	   *Pl = PObj -> U.Pl;

	while (Pl) {
	    AttrSetIntAttrib(&Pl -> Attr, "ID", ID);

	    Pl = Pl -> Pnext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examines the input to the Booleans for the case of a coplanar input, so  *
* a 2D Boolean should be applied instead.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1, PObj2: The two objects that serve as input to the Booleans.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if indeed input is planar 2D and two objects are coplanar.    *
*****************************************************************************/
static int BooleanInputCoplanar2D(IPObjectStruct *PObj1,
				  IPObjectStruct *PObj2)
{
    IPPolygonStruct *Pl1, *Pl2;

    for (Pl1 = PObj1 -> U.Pl, Pl2 = PObj2 -> U.Pl;
	 Pl1 != NULL;
	 Pl1 = Pl1 -> Pnext)
        if (!IRIT_PLANE_APX_EQ_EPS(Pl1 -> Plane, Pl2 -> Plane, IRIT_EPS))
	    return FALSE;

    for (Pl1 = PObj1 -> U.Pl, Pl2 = PObj2 -> U.Pl;
	 Pl2 != NULL;
	 Pl2 = Pl2 -> Pnext)
        if (!IRIT_PLANE_APX_EQ_EPS(Pl1 -> Plane, Pl2 -> Plane, IRIT_EPS))
	    return FALSE;

    /* All polygons are coplanar. */
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean OR between two objects.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object to perform the Boolean operation on.              M
*   PObj2:    Second object to perform the Boolean operation on.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG, Boolean2D, M
*   BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly, BoolSetPolySortAxis, M
*   BoolSetParamSurfaceUVVals, BooleanSELF, BooleanICUT,		     M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanOR, Booleans                                                      M
*****************************************************************************/
IPObjectStruct *BooleanOR(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;

    BoolFoundCoplanarPoly = FALSE;

    if (PObj1 == PObj2)
	return BooleanSELF(PObj1);
    else if ((PObj = VerifyBooleanInput(PObj1, PObj2, BOOL_OPER_OR)) != NULL)
	return PObj;
    else {
        IrtRType
	    GMBasicEps = GMBasicSetEps(BOOL_GM_BASIC_EPS);

	if (setjmp(LclLongJumpBuffer) == 0) { /* Its the setjmp itself call! */
	    signal(SIGFPE, BooleanFPE);  /* Will trap floating point errors. */
	    if (BoolOutputInterCurve)
		PObj = BooleanLow1Out2(PObj1, PObj2);/* Ret intersection crv.*/
	    else if (BooleanInputCoplanar2D(PObj1, PObj2)) {
	        PObj = IPGenPOLYObject(Boolean2D(PObj1 -> U.Pl,
						 PObj2 -> U.Pl, BOOL_OPER_OR));
	    }
	    else {
		IPObjectStruct
		    *PObj12 = BooleanLow1Out2(PObj1, PObj2),
		    *PObj21 = BooleanLow1Out2(PObj2, PObj1);

		/* Note that BooleanLow1Out2 must be invoked before calling  */
		/* BooleanCoplanar as BooleanCoplanar depends on the former. */
		PObj = BooleanCombineThreeObjs(PObj12, PObj21,
					       BooleanCoplanar(PObj1, PObj2,
							       BOOL_OPER_OR));
	    }
	}
	else {
	    /* We gain control from fatal error long jump - usually we should*/
	    /* return empty object, but if error is no intersection between  */
	    /* the two objects, we assume they have no common volume and     */
	    /* return a new object consists of the concat. of all polygons!  */
	    if (FatalErrorType != FTL_BOOL_NO_INTER) {
		PObj = IPGenPolyObject("", NULL, NULL); /* Return empty obj. */
	    }
	    else {
	        IrtHmgnMatType Mat;
		IPObjectStruct *PObj2Perturbed;

		if (PObj1 -> U.Pl == NULL)
		    PObj = IPCopyObject(NULL, PObj2, FALSE);
		else if (BoolPerturbAmount > 0.0 &&
			 AttrGetObjectIntAttrib(PObj2, "_Perturbed") != TRUE) {

		    BoolClnAdjacencies(PObj1);
		    BoolClnAdjacencies(PObj2);

		    /* Perturb the second object and retry. */
		    MatGenMatTrans(BoolPerturbAmount,
				   BoolPerturbAmount,
				   BoolPerturbAmount, Mat);
		    PObj2Perturbed = GMTransformObject(PObj2, Mat);
		    AttrSetObjectIntAttrib(PObj2Perturbed, "_Perturbed", TRUE);
		    PObj = BooleanOR(PObj1, PObj2Perturbed);
		    IPFreeObject(PObj2Perturbed);
		}
		else {
		    PObj = IPCopyObject(NULL, PObj1, FALSE);/* Cp Obj1 polys.*/

		    if (AttrGetObjectIntAttrib(PObj2, "_Perturbed") != TRUE) {
		        /* Perturb PObj2 back so we merge original geometry. */
		        MatGenMatTrans(-BoolPerturbAmount,
				       -BoolPerturbAmount,
				       -BoolPerturbAmount, Mat);
			PObj2Perturbed = GMTransformObject(PObj2, Mat);
			IPGetLastPoly(PObj -> U.Pl) -> Pnext =
						       PObj2Perturbed -> U.Pl;
			PObj2Perturbed -> U.Pl = NULL;
			IPFreeObject(PObj2Perturbed);
		    }
		    else
		        IPGetLastPoly(PObj -> U.Pl) -> Pnext =
		               IPCopyPolygonList(PObj2 -> U.Pl); /* Obj2 pl. */
		}
	    }
	}

	GMBasicSetEps(GMBasicEps);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean AND between two objects.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object to perform the Boolean operation on.              M
*   PObj2:    Second object to perform the Boolean operation on.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG,             M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanSELF, BooleanICUT,M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanAND, Booleans                                                     M
*****************************************************************************/
IPObjectStruct *BooleanAND(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;

    BoolFoundCoplanarPoly = FALSE;

    if (PObj1 == PObj2)
	return BooleanSELF(PObj1);
    else if ((PObj = VerifyBooleanInput(PObj1, PObj2, BOOL_OPER_AND)) != NULL)
	return PObj;
    else {
        IrtRType
	    GMBasicEps = GMBasicSetEps(BOOL_GM_BASIC_EPS);

	if (setjmp(LclLongJumpBuffer) == 0) { /* Its the setjmp itself call! */
	    signal(SIGFPE, BooleanFPE);  /* Will trap floating point errors. */
	    if (BoolOutputInterCurve)
		PObj = BooleanLow1In2(PObj1, PObj2);/* Ret intersection crv. */
	    else if (BooleanInputCoplanar2D(PObj1, PObj2)) {
	        PObj = IPGenPOLYObject(Boolean2D(PObj1 -> U.Pl,
						 PObj2 -> U.Pl, BOOL_OPER_AND));
	    }
	   else {
		IPObjectStruct
		    *PObj12 = BooleanLow1In2(PObj1, PObj2),
		    *PObj21 = BooleanLow1In2(PObj2, PObj1);

		/* Note that BooleanLow1Out2 must be invoked before calling  */
		/* BooleanCoplanar as BooleanCoplanar depends on the former. */
		PObj = BooleanCombineThreeObjs(PObj12, PObj21,
					       BooleanCoplanar(PObj1, PObj2,
							       BOOL_OPER_AND));
	    }
	}
	else {/* We gain control from fatal error long jump - try to pertub  */
	      /* PObj2 and retry once or else return an empty obj.	     */
	    if (FatalErrorType == FTL_BOOL_NO_INTER &&
		BoolPerturbAmount > 0.0 &&
	 	AttrGetObjectIntAttrib(PObj2, "_Perturbed") != TRUE) {
	        IrtHmgnMatType Mat;
		IPObjectStruct *PObj2Perturbed;

		BoolClnAdjacencies(PObj1);
		BoolClnAdjacencies(PObj2);

		/* Perturb the second object and retry. */
		MatGenMatTrans(BoolPerturbAmount,
			       BoolPerturbAmount,
			       BoolPerturbAmount, Mat);
		PObj2Perturbed = GMTransformObject(PObj2, Mat);
		AttrSetObjectIntAttrib(PObj2Perturbed, "_Perturbed", TRUE);
		PObj = BooleanAND(PObj1, PObj2Perturbed);
		IPFreeObject(PObj2Perturbed);
	    }
	    else
	        PObj = IPGenPolyObject("", NULL, NULL);
	}

	GMBasicSetEps(GMBasicEps);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean SUBtracion between two objects.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object to perform the Boolean operation on.              M
*   PObj2:    Second object to perform the Boolean operation on.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanCUT, BooleanMERGE, BooleanNEG,             M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanSELF, BooleanICUT,M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanSUB, Booleans                                                     M
*****************************************************************************/
IPObjectStruct *BooleanSUB(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;

    BoolFoundCoplanarPoly = FALSE;

    if (PObj1 == PObj2)
	return BooleanSELF(PObj1);
    else if ((PObj = VerifyBooleanInput(PObj1, PObj2, BOOL_OPER_SUB)) != NULL)
	return PObj;
    else {
        IrtRType
	    GMBasicEps = GMBasicSetEps(BOOL_GM_BASIC_EPS);

	if (setjmp(LclLongJumpBuffer) == 0) { /* Its the setjmp itself call! */
	    signal(SIGFPE, BooleanFPE);	 /* Will trap floating point errors. */
	    /* The 1 in 2 must be reversed (the inside/outside orientation): */
	    if (BoolOutputInterCurve) {
		PObj = BooleanLow1In2(PObj2, PObj1);/* Ret intersection crv. */
	    }
	    else if (BooleanInputCoplanar2D(PObj1, PObj2)) {
	        PObj = IPGenPOLYObject(Boolean2D(PObj1 -> U.Pl,
						 PObj2 -> U.Pl,
						 BOOL_OPER_SUB));
	    }
	    else {
		IPObjectStruct
		    *PObj21 = BooleanLow1In2(PObj2, PObj1),
		    *PObj21Rev = BooleanNEG(PObj21),
		    *PObj12 = BooleanLow1Out2(PObj1, PObj2);

		IPFreeObject(PObj21);

		/* Note that BooleanLow1Out2 must be invoked before calling  */
		/* BooleanCoplanar as BooleanCoplanar depends on the former. */
		PObj = BooleanCombineThreeObjs(PObj12, PObj21Rev,
					       BooleanCoplanar(PObj1, PObj2,
							       BOOL_OPER_SUB));
	    }
	}
	else {/* We gain control from fatal error long jump - try to pertub  */
	      /* PObj2 and retry once or else return an empty obj.           */
	    if (FatalErrorType == FTL_BOOL_NO_INTER &&
		BoolPerturbAmount > 0.0 &&
	 	AttrGetObjectIntAttrib(PObj2, "_Perturbed") != TRUE) {
	        IrtHmgnMatType Mat;
		IPObjectStruct *PObj2Perturbed;

		BoolClnAdjacencies(PObj1);
		BoolClnAdjacencies(PObj2);

		/* Perturb the second object and retry. */
		MatGenMatTrans(BoolPerturbAmount,
			       BoolPerturbAmount,
			       BoolPerturbAmount, Mat);
		PObj2Perturbed = GMTransformObject(PObj2, Mat);
		AttrSetObjectIntAttrib(PObj2Perturbed, "_Perturbed", TRUE);
		PObj = BooleanSUB(PObj1, PObj2Perturbed);
		IPFreeObject(PObj2Perturbed);
	    }
	    else
	        PObj = IPGenPolyObject("", NULL, NULL);
	}

	GMBasicSetEps(GMBasicEps);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean CUT between two objects.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object to perform the Boolean operation on.              M
*   PObj2:    Second object to perform the Boolean operation on.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanMERGE, BooleanNEG,             M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanSELF, BooleanICUT,M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanCUT, Booleans                                                     M
*****************************************************************************/
IPObjectStruct *BooleanCUT(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;

    BoolFoundCoplanarPoly = FALSE;

    if (PObj1 == PObj2)
	return BooleanSELF(PObj1);
    else if ((PObj = VerifyBooleanInput(PObj1, PObj2, BOOL_OPER_CUT)) != NULL)
	return PObj;
    else {
        IrtRType
	    GMBasicEps = GMBasicSetEps(BOOL_GM_BASIC_EPS);

	if (setjmp(LclLongJumpBuffer) == 0) { /* Its the setjmp itself call! */
	    signal(SIGFPE, BooleanFPE);	 /* Will trap floating point errors. */
	    /* The 1 in 2 must be reversed (the inside/outside orientation): */
	    if (BoolOutputInterCurve) {
		PObj = BooleanLow1In2(PObj2, PObj1);/* Ret intersection crv. */
	    }
	    else {
		PObj = BooleanLow1Out2(PObj1, PObj2);
	    }
	}
	else {/* We gain control from fatal error long jump - try to pertub  */
	      /* PObj2 and retry once or else return an empty obj.           */
	    if (FatalErrorType == FTL_BOOL_NO_INTER &&
		BoolPerturbAmount > 0.0 &&
	 	AttrGetObjectIntAttrib(PObj2, "_Perturbed") != TRUE) {
	        IrtHmgnMatType Mat;
		IPObjectStruct *PObj2Perturbed;

		BoolClnAdjacencies(PObj1);
		BoolClnAdjacencies(PObj2);

		/* Perturb the second object and retry. */
		MatGenMatTrans(BoolPerturbAmount,
			       BoolPerturbAmount,
			       BoolPerturbAmount, Mat);
		PObj2Perturbed = GMTransformObject(PObj2, Mat);
		AttrSetObjectIntAttrib(PObj2Perturbed, "_Perturbed", TRUE);
		PObj = BooleanCUT(PObj1, PObj2Perturbed);
		IPFreeObject(PObj2Perturbed);
	    }
	    else
	        PObj = IPGenPolyObject("", NULL, NULL);
	}

	GMBasicSetEps(GMBasicEps);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean Inside CUT between two objects.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object to perform the Boolean operation on.              M
*   PObj2:    Second object to perform the Boolean operation on.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanMERGE, BooleanNEG,             M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanSELF, BooleanCUT, M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanICUT, Booleans                                                    M
*****************************************************************************/
IPObjectStruct *BooleanICUT(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;

    BoolFoundCoplanarPoly = FALSE;

    if (PObj1 == PObj2)
	return BooleanSELF(PObj1);
    else if ((PObj = VerifyBooleanInput(PObj1, PObj2, BOOL_OPER_ICUT)) != NULL)
	return PObj;
    else {
        IrtRType
	    GMBasicEps = GMBasicSetEps(BOOL_GM_BASIC_EPS);

	if (setjmp(LclLongJumpBuffer) == 0) { /* Its the setjmp itself call! */
	    signal(SIGFPE, BooleanFPE);	 /* Will trap floating point errors. */
	    /* The 1 in 2 must be reversed (the inside/outside orientation): */
	    if (BoolOutputInterCurve) {
		PObj = BooleanLow1In2(PObj2, PObj1);/* Ret intersection crv. */
	    }
	    else {
		PObj = BooleanLow1In2(PObj1, PObj2);
	    }
	}
	else {/* We gain control from fatal error long jump - try to pertub  */
	      /* PObj2 and retry once or else return an empty obj.           */
	    if (FatalErrorType == FTL_BOOL_NO_INTER &&
		BoolPerturbAmount > 0.0 &&
	 	AttrGetObjectIntAttrib(PObj2, "_Perturbed") != TRUE) {
	        IrtHmgnMatType Mat;
		IPObjectStruct *PObj2Perturbed;

		BoolClnAdjacencies(PObj1);
		BoolClnAdjacencies(PObj2);

		/* Perturb the second object and retry. */
		MatGenMatTrans(BoolPerturbAmount,
			       BoolPerturbAmount,
			       BoolPerturbAmount, Mat);
		PObj2Perturbed = GMTransformObject(PObj2, Mat);
		AttrSetObjectIntAttrib(PObj2Perturbed, "_Perturbed", TRUE);
		PObj = BooleanICUT(PObj1, PObj2Perturbed);
		IPFreeObject(PObj2Perturbed);
	    }
	    else
	        PObj = IPGenPolyObject("", NULL, NULL);
	}

	GMBasicSetEps(GMBasicEps);
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a contouring of the given poly object with the given plane.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to perform the contouring operation on.		     M
*   Plane:   Plane to use in the contouring.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the contouring operation.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanMultiCONTOUR, BooleanOR, BooleanAND, BooleanSUB, BooleanMERGE,    M
*   BooleanNEG, Boolean2D, BoolSetOutputInterCurve,			     M
*   BoolSetHandleCoplanarPoly, BoolSetPolySortAxis,			     M
*   BoolSetParamSurfaceUVVals, BooleanSELF, BooleanCUT, BoolSetPerturbAmount M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanCONTOUR, Booleans                                                 M
*****************************************************************************/
IPObjectStruct *BooleanCONTOUR(IPObjectStruct *PObj, IrtPlnType Plane)
{
    int OldRes = PrimSetResolution(4),
	OldInterCrvs = BoolSetOutputInterCurve(TRUE);
    IrtRType R;
    IrtVecType V;
    IrtPtType Center;
    IrtPlnType Pln;
    GMBBBboxStruct *BBox;
    IPObjectStruct *PPln, *PRes;

    IRIT_PLANE_COPY(Pln, Plane);
    if ((R = IRIT_PT_LENGTH(Pln)) > 0.0) {
        IRIT_PLANE_SCALE(Pln, 1.0 / R);
    }
    BBox = GMBBComputeBboxObject(PObj);

    /* Move the center of the disk to be reconstructed to be on Pln. */
    IRIT_PT_BLEND(Center, BBox -> Min, BBox -> Max, 0.5);
    R = IRIT_DOT_PROD(Pln, Center) + Pln[3];
    IRIT_PT_SCALE2(V, Pln, -R);
    IRIT_PT_ADD(Center, Center, V);
    /* Make sure newly computed center location is on the plane. */
    assert(IRIT_FABS(IRIT_DOT_PROD(Pln, Center) + Pln[3]) < IRIT_EPS);

    R = BBox -> Max[0] - BBox -> Min[0] +
        BBox -> Max[1] - BBox -> Min[1] +
        BBox -> Max[2] - BBox -> Min[2];
    PPln = PrimGenPOLYDISKObject(Pln, Center, R);

    PRes = BooleanAND(PObj, PPln);

    IPFreeObject(PPln);
    PrimSetResolution(OldRes);
    BoolSetOutputInterCurve(OldInterCrvs);

    return PRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs multiple contouring of the given poly object with parallel      M
* planes.  Planes must be X/Y/Z constant.				     M
*   This function is called 3 times:					     M
* + With Init=TRUE, to initialize the process.  Returns NULL.		     M
* + With Done=TRUE, to terminate the process.  Returns NULL.		     M
* + Otherwise performs contouring at the requested level and returns the     M
*   computed contour (can be empty polyline object).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Object to perform the multi-contouring operation on.	     M
*               Relevant only when Init = TRUE.				     M
*   CntrLevel:  Level to contour at. Relevant only if Init = Done = FALSE.   M
*		Contouring requests must come in increasing level order.     M
*   Axis:       0, 1, 2 for X/Y/Z constant planes to use.		     M
*   Init:       TRUE to initialize a multi-contouring process.		     M
*   Done:       TRUE to terminate a multi-contouring process.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the contouring operation.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanCONTOUR, BooleanOR, BooleanAND, BooleanSUB, BooleanMERGE,         M
*   BooleanNEG, Boolean2D, BoolSetOutputInterCurve,			     M
*   BoolSetHandleCoplanarPoly, BoolSetPolySortAxis,			     M
*   BoolSetParamSurfaceUVVals, BooleanSELF, BooleanCUT, BoolSetPerturbAmount M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanMultiCONTOUR, Booleans                                            M
*****************************************************************************/
IPObjectStruct *BooleanMultiCONTOUR(IPObjectStruct *PObj,
				    IrtRType CntrLevel,
				    int Axis,
				    int Init,
				    int Done)
{
    IRIT_STATIC_DATA IrtRType PlnRad;
    IRIT_STATIC_DATA IrtPtType Center;
    IRIT_STATIC_DATA IrtPlnType Pln;
    IRIT_STATIC_DATA IPObjectStruct
        *SortedPolys = NULL,
        *ActivePObj = NULL;
    IPObjectStruct *PPln, *PRes;

    if (Init) {
        int OldAxis = BoolSetPolySortAxis(Axis);
        GMBBBboxStruct *BBox;

        /* Prepare the plane. */
        IRIT_PLANE_RESET(Pln);
	Pln[Axis] = -1.0;

	/* Sort all polygons according to their minimal bbox in dir. Axis.  */
	if (SortedPolys != NULL)
	    IPFreeObject(SortedPolys);
	SortedPolys = IPCopyObject(NULL, PObj, FALSE);
        BooleanPrepObject(SortedPolys);
	BoolSetPolySortAxis(OldAxis);

	if (ActivePObj != NULL)
	    IPFreeObject(ActivePObj);
	ActivePObj = IPGenPOLYObject(NULL);	 /* Make empty active list. */

	/* Prepare the contouring plane as a disk polygon:  Move the        */
	/* center of the disk to be on the Plane.  See also in contouring.  */
	BBox = GMBBComputeBboxObject(PObj);
	IRIT_PT_BLEND(Center, BBox -> Min, BBox -> Max, 0.5);
	PlnRad = BBox -> Max[0] - BBox -> Min[0] +
	         BBox -> Max[1] - BBox -> Min[1] +
                 BBox -> Max[2] - BBox -> Min[2];
	return NULL;
    }
    else if (Done) {
        IPFreeObject(SortedPolys);
        IPFreeObject(ActivePObj);
	SortedPolys = ActivePObj = NULL;
	return NULL;
    }
    else {
        int OldRes = PrimSetResolution(4),
	    OldInterCrvs = BoolSetOutputInterCurve(TRUE);
	IPPolygonStruct *Pl, *PlTmp;

	/* Update the active object that contains all polygons that         */
	/* potentially intersect level CntrLevel.  First, purge for the     */
	/* active list all polygons that drop below our current level.      */
	Pl = ActivePObj -> U.Pl;
	ActivePObj -> U.Pl = NULL;
	while (Pl != NULL) {
	    IRIT_LIST_POP(PlTmp, Pl);

	    if (PlTmp -> BBox[1][Axis] < CntrLevel) {
	        /* Purge this polygon that is below our current level. */
	        IPFreePolygon(PlTmp);
	    }
	    else {
	        /* Keep it. */
	        IRIT_LIST_PUSH(PlTmp, ActivePObj -> U.Pl);
	    }
	}
	/* Now add to the active list all polygons in the sorted list that  */
	/* their minimum is now below our current level.		    */
	if ((Pl = SortedPolys -> U.Pl) != NULL) {
	    while (Pl -> Pnext != NULL &&
		   Pl -> Pnext -> BBox[0][Axis] <= CntrLevel)
	        Pl = Pl -> Pnext;
	    if (Pl -> Pnext != NULL) {
	        PlTmp = Pl -> Pnext;
		Pl -> Pnext = NULL;
	    }
	    else
	        PlTmp = NULL;

	    ActivePObj -> U.Pl = IPAppendPolyLists(ActivePObj -> U.Pl,
						   SortedPolys -> U.Pl);
	    SortedPolys -> U.Pl = PlTmp;
	}

        /* Build the contouring plane as a disk polygon:  Move the          */
	/* center of the disk to be on the Plane.			    */
        Pln[3] = Center[Axis] = CntrLevel;
	PPln = PrimGenPOLYDISKObject(Pln, Center, PlnRad);

	if (ActivePObj -> U.Pl == NULL)
	    PRes = IPGenPOLYObject(NULL);
	else
	    PRes = BooleanAND(ActivePObj, PPln);

	IPFreeObject(PPln);
	PrimSetResolution(OldRes);
	BoolSetOutputInterCurve(OldInterCrvs);

	return PRes;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean MERGE between two objects.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    First object to perform the Boolean operation on.              M
*   PObj2:    Second object to perform the Boolean operation on.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanNEG,               M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanSELF, BooleanICUT,M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanMERGE, Booleans                                                   M
*****************************************************************************/
IPObjectStruct *BooleanMERGE(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct *PObj;
    IPPolygonStruct *Pl;

    if (PObj1 == PObj2)
	return IPCopyObject(NULL, PObj1, FALSE);
    else if ((PObj = VerifyBooleanInput(PObj1, PObj2,
					BOOL_OPER_MERGE)) != NULL)
	return PObj;
    else {
	if (PObj1 -> U.Pl == NULL)
    	    PObj = IPCopyObject(NULL, PObj2, FALSE);
    	else {
    	    PObj = IPCopyObject(NULL, PObj1, FALSE);     /* Copy Obj1 polys. */
	    Pl = PObj -> U.Pl;
	    while (Pl -> Pnext) Pl = Pl -> Pnext;
	    Pl -> Pnext = IPCopyPolygonList(PObj2 -> U.Pl);   /* Obj2 polys. */
	}
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean NEG between two objects.				     M
*   Negation is simply reversing the direction of the plane equation of each M
* polygon - the simplest Boolean operation...				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to negate.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE,             M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanSELF, BooleanICUT,M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanNEG, Booleans                                                     M
*****************************************************************************/
IPObjectStruct *BooleanNEG(IPObjectStruct *PObj)
{
    int i;
    IPObjectStruct *PTemp;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    if ((PTemp = VerifyBooleanInput(PObj, NULL, BOOL_OPER_NEG)) != NULL)
	return PTemp;
    else {
	PTemp = IPCopyObject(NULL, PObj, FALSE);/* Make fresh cpy of object. */

	/* Scans all polygons and reverse plane equation and their vetrex    */
	/* list (cross prod. of consecutive edges must be in normal dir.).   */
	Pl = PTemp -> U.Pl;
	while (Pl != NULL) {
	    for (i = 0; i < 4; i++)
	        Pl -> Plane[i] = (-Pl -> Plane[i]);
	    IP_RST_CONVEX_POLY(Pl);

	    /* Invert vertices normals as well. */
	    V = Pl -> PVertex;
	    do {
		IRIT_PT_SCALE(V -> Normal, -1.0);
		V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);

	    /* And reverse the order of the vertices. */
	    IPReverseVrtxList(Pl);

	    Pl = Pl -> Pnext;
	}
    }

    return PTemp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Performs a Boolean Self intersection operation.                          M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to perform the self intersecting Boolean operation on.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The result of the Boolean operation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanMERGE, BooleanNEG,             M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetParamSurfaceUVVals, BooleanCUT, BooleanICUT, M
*   BooleanCONTOUR, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanSELF, Booleans                                                    M
*****************************************************************************/
IPObjectStruct *BooleanSELF(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjTmp;

    BoolFoundCoplanarPoly = FALSE;

    if ((PObjTmp = VerifyBooleanInput(PObj, NULL, BOOL_OPER_SELF)) != NULL)
	return PObjTmp;
    else {
	if (setjmp(LclLongJumpBuffer) == 0) { /* Its the setjmp itself call! */
	    signal(SIGFPE, BooleanFPE);	 /* Will trap floating point errors. */
	    /* The 1 in 2 must be reversed (the inside/outside orientation): */
	    if (BoolOutputInterCurve) {
		PObj = BooleanLowSelfInOut(PObj, TRUE);  /* Ret. inter. crv. */
	    }
	    else {
		IRIT_WARNING_MSG("Self intersection is supported for intersection curves only.\n");
		PObj = IPGenPolyObject("", NULL, NULL);
	    }
	}
	else {/* We gain control from fatal error long jump - ret empty obj. */
	    PObj = IPGenPolyObject("", NULL, NULL);
	}
    }

    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Combining three geometric objects, by simply concat. their polygon.      *
* lists. Any object may be NULL in which only the other two are merged.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1, PObj2, PObj3: The three objects to concatenate.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *: Concatenated object.                                   *
*****************************************************************************/
static IPObjectStruct *BooleanCombineThreeObjs(IPObjectStruct *PObj1,
					       IPObjectStruct *PObj2,
					       IPObjectStruct *PObj3)
{
    IPPolygonStruct *Pl;

    if (PObj1 != NULL) {
        GMCleanUpPolygonList(&PObj1 -> U.Pl, BOOL_IRIT_REL_EPS);
	if (PObj1 -> U.Pl == NULL) {
	    IPFreeObject(PObj1);
	    PObj1 = NULL;
	}
    }
    if (PObj2 != NULL) {
	GMCleanUpPolygonList(&PObj2 -> U.Pl, BOOL_IRIT_REL_EPS);
	if (PObj2 -> U.Pl == NULL) {
	    IPFreeObject(PObj2);
	    PObj2 = NULL;
	}
    }
    if (PObj3 != NULL) {
	GMCleanUpPolygonList(&PObj3 -> U.Pl, BOOL_IRIT_REL_EPS);
	if (PObj3 -> U.Pl == NULL) {
	    IPFreeObject(PObj3);
	    PObj3 = NULL;
	}
    }

    if (PObj1 == NULL && PObj2 == NULL && PObj3 != NULL) {
        IRIT_WARNING_MSG("Only coplanar intersections detected - result might be wrong.");
    }

    if (PObj2 == NULL) {
	PObj2 = PObj3;
	PObj3 = NULL;
    }
    if (PObj1 == NULL) {
	PObj1 = PObj2;
	PObj2 = PObj3;
	PObj3 = NULL;
    }

    if (PObj2 != NULL) {
	/* Concat the polygons of PObj2 after the polygons of PObj1. */
	Pl = PObj1 -> U.Pl;
	while (Pl -> Pnext != NULL)
	    Pl = Pl -> Pnext;
	Pl -> Pnext = PObj2 -> U.Pl;  /* Concat. the polygons into one list. */
	PObj2 -> U.Pl = NULL;	    /* And release the second object header. */
	IPFreeObject(PObj2);

        if (PObj3 != NULL) {
	    /* Concat the polygons of PObj3 after the polygons of PObj1/2. */
	    while (Pl -> Pnext != NULL)
		Pl = Pl -> Pnext;
	    Pl -> Pnext = PObj3 -> U.Pl;  /* Concat. polygons into one list. */
	    PObj3 -> U.Pl = NULL;   /* And release the second object header. */
	    IPFreeObject(PObj3);
        }
    }

    return PObj1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   If required search all coplanar polygons in PObj1/2 and invoke the       *
* necessary two dimenstional Boolean.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj1, PObj2: Two objects to search for coplanar polygons.               *
*   BoolOper:     Type of Boolean operation to be performed between PObj1/2. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   The resulting Boolean operation.                     *
*****************************************************************************/
static IPObjectStruct *BooleanCoplanar(IPObjectStruct *PObj1,
				       IPObjectStruct *PObj2,
				       BoolOperType BoolOper)
{
    IrtHmgnMatType RotMat;
    IPObjectStruct *PObj;
    IPPolygonStruct *Pl, *PlTmp, *Pl1, *Pl2, *Pl1XY, *Pl2XY, *Pl1XYR, *Pl2XYR,
	*PlOut = NULL;

    if (!BoolHandleCoplanarPoly || !BoolFoundCoplanarPoly)
	return NULL;

    for (Pl1 = PObj1 -> U.Pl; Pl1 != NULL; Pl1 = Pl1 -> Pnext) {
	int i;
	IrtRType Plane1not[4],
	    *Plane1 = Pl1 -> Plane;

	for (i = 0; i < 4; i++)
	    Plane1not[i] = -Plane1[i];

        for (Pl2 = PObj2 -> U.Pl; Pl2 != NULL; Pl2 = Pl2 -> Pnext) {
	    IrtRType
		*Plane2 = Pl2 -> Plane;
	    int Shared = IRIT_BOOL_APX_EQ(Plane1[0], Plane2[0]) &&
		         IRIT_BOOL_APX_EQ(Plane1[1], Plane2[1]) &&
		         IRIT_BOOL_APX_EQ(Plane1[2], Plane2[2]) &&
		         IRIT_BOOL_APX_EQ(Plane1[3], Plane2[3]),
		AntiShared = IRIT_BOOL_APX_EQ(Plane1not[0], Plane2[0]) &&
			     IRIT_BOOL_APX_EQ(Plane1not[1], Plane2[1]) &&
			     IRIT_BOOL_APX_EQ(Plane1not[2], Plane2[2]) &&
			     IRIT_BOOL_APX_EQ(Plane1not[3], Plane2[3]);

	    if (!Shared && !AntiShared) {
		/* The two polygons are not coplanar, do not intersect. */
		continue;
	    }

	    if (Shared) {
		switch (BoolOper) {
		    case BOOL_OPER_AND:
		    case BOOL_OPER_OR:
		    case BOOL_OPER_SUB:
			GMGenRotateMatrix(RotMat, Pl1 -> Plane);

			Pl1XY = BooleanComputeRotatedPolys(Pl1, TRUE, RotMat);
			Pl2XY = BooleanComputeRotatedPolys(Pl2, TRUE, RotMat);

			MatTranspMatrix(RotMat, RotMat); /* Compute inverse. */

			if ((Pl = Boolean2D(Pl1XY, Pl2XY, BoolOper)) != NULL) {
			    Pl = BooleanComputeRotatedPolys(Pl, FALSE, RotMat);
			    for (PlTmp = Pl;
				 PlTmp -> Pnext != NULL;
				 PlTmp = PlTmp -> Pnext);
			    PlTmp -> Pnext = PlOut;
			    PlOut = Pl;
			}

			IPFreePolygonList(Pl1XY);
			IPFreePolygonList(Pl2XY);
			break;

		    default:
		        BOOL_FATAL_ERROR(BOOL_ERR_NO_BOOL_OP_SUPPORT);
		        return NULL;
		}
	    }
	    else if (AntiShared) {
		switch (BoolOper) {
		    case BOOL_OPER_AND:
		    case BOOL_OPER_SUB:
			IRIT_WARNING_MSG("Antishared coplanar polygons are ignored.");
			break;

		    case BOOL_OPER_OR:
			GMGenRotateMatrix(RotMat, Pl1 -> Plane);

			Pl1XY = BooleanComputeRotatedPolys(Pl1, TRUE, RotMat);
			Pl1XYR = IPCopyPolygonList(Pl1XY);
			IPReverseVrtxList(Pl1XYR);
			Pl2XY = BooleanComputeRotatedPolys(Pl2, TRUE, RotMat);
			Pl2XYR = IPCopyPolygonList(Pl2XY);
			IPReverseVrtxList(Pl2XYR);

			MatTranspMatrix(RotMat, RotMat); /* Compute inverse. */

			if ((Pl = Boolean2D(Pl1XY, Pl2XYR,
					    BOOL_OPER_SUB)) != NULL) {
			    Pl = BooleanComputeRotatedPolys(Pl, FALSE, RotMat);
			    for (PlTmp = Pl;
				 PlTmp -> Pnext != NULL;
				 PlTmp = PlTmp -> Pnext);
			    PlTmp -> Pnext = PlOut;
			    PlOut = Pl;
			}
			if ((Pl = Boolean2D(Pl2XY, Pl1XYR,
					    BOOL_OPER_SUB)) != NULL) {
			    Pl = BooleanComputeRotatedPolys(Pl, FALSE, RotMat);
			    for (PlTmp = Pl;
				 PlTmp -> Pnext != NULL;
				 PlTmp = PlTmp -> Pnext);
			    PlTmp -> Pnext = PlOut;
			    PlOut = Pl;
			}

			IPFreePolygonList(Pl1XY);
			IPFreePolygonList(Pl2XY);
			IPFreePolygonList(Pl1XYR);
			IPFreePolygonList(Pl2XYR);
			break;

		    default:
		        BOOL_FATAL_ERROR(BOOL_ERR_NO_BOOL_OP_SUPPORT);
		        return NULL;
		}
	    }
	}
    }

    PObj = IPAllocObject("", IP_OBJ_POLY, NULL);
    PObj -> U.Pl = PlOut;
    return PObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to optionally copy (if CpolyOnePl) a single polygon and rotate   M
* according to the rotation matrix provided. If, however, CopyOnePl is False M
* all polygons in list are converted.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:             Polygon(s) to transform.  Assumed to be convex.          M
*   CopyOnePl:      Should we copy?  Also if FALSE Pl might be non convex!   M
*   RotMat:         Transformation matrix.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: Transformed polygon(s).                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BooleanComputeRotatedPolys, Booleans                                     M
*****************************************************************************/
IPPolygonStruct *BooleanComputeRotatedPolys(IPPolygonStruct *Pl,
					    int CopyOnePl,
					    IrtHmgnMatType RotMat)
{
    IPVertexStruct *V, *VHead;
    IPPolygonStruct *PlNext, *PlTemp,
	*PlOut = NULL;

    while (Pl != NULL) {
	PlNext = Pl -> Pnext;

	if (CopyOnePl) {
	    PlTemp = IPAllocPolygon(Pl -> Tags,
				    IPCopyVertexList(Pl -> PVertex), NULL);
	    IRIT_PLANE_COPY(PlTemp ->Plane, Pl -> Plane);
	    Pl = PlTemp;
	}

	V = VHead = Pl -> PVertex;
	do {				    /* Transform the polygon itself. */
	    MatMultPtby4by4(V -> Coord, V -> Coord, RotMat);

	    MatMultVecby4by4(V -> Normal, V -> Normal, RotMat);
	    IRIT_PT_NORMALIZE(V -> Normal);

	    V = V -> Pnext;
	}
	while (V != NULL && V != VHead);

	MatMultVecby4by4(Pl -> Plane, Pl -> Plane, RotMat);
	IRIT_PT_NORMALIZE(Pl -> Plane);
	Pl -> Plane[3] = -IRIT_DOT_PROD(Pl -> Plane, Pl -> PVertex -> Coord);

	Pl -> Pnext = PlOut;
	PlOut = Pl;

	if (CopyOnePl)
	    break;
	else
	    Pl = PlNext;
    }

    return PlOut;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine that is called from the floating point package in case of fatal  *
* floating point error. Print error message, and quit the Boolean module.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Type:       of exception.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BooleanFPE(int Type)
{
    char Line[IRIT_LINE_LEN];

    sprintf(Line, IRIT_EXP_STR("Floating point error %d."), Type);
    IRIT_WARNING_MSG(Line);

    FatalErrorType = FTL_BOOL_FPE;

    longjmp(LclLongJumpBuffer, 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if intersection curves or full Boolean operation is to be       M
* performed.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   OutputInterCurve:  If TRUE only intersection curves are computed, If     M
*                      false, full blown Boolean is applied.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	Old value.                                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG, M
*   Boolean2D, BoolSetHandleCoplanarPoly, BoolSetPolySortAxis,		     M
*   BoolSetParamSurfaceUVVals, BoolSetPerturbAmount			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSetOutputInterCurve, Booleans                                        M
*****************************************************************************/
int BoolSetOutputInterCurve(int OutputInterCurve)
{
    int Old = BoolOutputInterCurve;

    BoolOutputInterCurve = OutputInterCurve;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls the perturbation amount, if any, of the second object to        M
* improve the success likelihood.  Perturbations are applied once the        M
* Booleans return with an empty intersection.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PerturbAmount:  Perturbation amount of objects before reattempting       M
*                   Booleans that ends up empty.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:	Old value.                                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG, M
*   Boolean2D, BoolSetHandleCoplanarPoly, BoolSetPolySortAxis,		     M
*   BoolSetParamSurfaceUVVals, BoolSetOutputInterCurve			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSetPerturbAmount, Booleans                                           M
*****************************************************************************/
IrtRType BoolSetPerturbAmount(IrtRType PerturbAmount)
{
    IrtRType Old = BoolPerturbAmount;

    BoolPerturbAmount = PerturbAmount;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if coplanar polygons should be handled or not.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   HandleCoplanarPoly:  If TRUE, coplanar polygons are handled.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Old value.                                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG, M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetPolySortAxis,		     M
*   BoolSetParamSurfaceUVVals, BoolSetPerturbAmount			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSetHandleCoplanarPoly, Booleans                                      M
*****************************************************************************/
int BoolSetHandleCoplanarPoly(int HandleCoplanarPoly)
{
    int Old = BoolHandleCoplanarPoly;

    BoolHandleCoplanarPoly = HandleCoplanarPoly;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls if UV paprameter values of original surface should be returned. M
*                                                                            *
* PARAMETERS:                                                                M
*   HandleBoolParamSrfUVVals:  If TRUE, UV values are to be returned.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Old value.                                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BooleanOR, BooleanAND, BooleanSUB, BooleanCUT, BooleanMERGE, BooleanNEG, M
*   Boolean2D, BoolSetOutputInterCurve, BoolSetHandleCoplanarPoly,	     M
*   BoolSetPolySortAxis, BoolSetPerturbAmount				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSetParamSurfaceUVVals, Booleans                                      M
*****************************************************************************/
int BoolSetParamSurfaceUVVals(int HandleBoolParamSrfUVVals)
{
    int Old = BoolParamSurfaceUVVals;

    BoolParamSurfaceUVVals = HandleBoolParamSrfUVVals;

    return Old;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the fatal error trapping routine of the boolean library.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrFunc:      New error trapping function to use.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   BoolFatalErrorFuncType:  Old error trapping function.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolDfltFatalError                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolSetFatalErrorFunc, error handling                                    M
*****************************************************************************/
BoolFatalErrorFuncType BoolSetFatalErrorFunc(BoolFatalErrorFuncType ErrFunc)
{
    BoolFatalErrorFuncType
	OldErrFunc = _BoolActiveFatalErrorFunc;

    _BoolActiveFatalErrorFunc = ErrFunc;
    return OldErrFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Defalu fatal error trapping rotuine of the Boolean library.              M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrID:      Error type that was raised.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolSetFatalErrorFunc                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolDfltFatalError, error handling                                       M
*****************************************************************************/
void BoolDfltFatalError(BoolFatalErrorType ErrID)
{
    const char
	*ErrorMsg = BoolDescribeError(ErrID);

    IRIT_FATAL_ERROR_PRINTF("BOOL_LIB: %s\n", ErrorMsg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine that is called by the bool-low module in fatal error cases.      *
* Will print error message and long jump using LclLongJumpBuffer.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   ErrorType:   Type of error.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void FatalBooleanError(int ErrorType)
{
    char s[IRIT_LINE_LEN_LONG];

    FatalErrorType = ErrorType;

    switch (ErrorType) {
	case FTL_BOOL_NO_INTER:
	    if (BoolPerturbAmount > 0)
		break;

	    /* If operation is union (OR), then if no intersection we assume */
	    /* the objects have no common volume and simply combine them!    */
	    if (!BoolOutputInterCurve && BooleanOperation != BOOL_OPER_OR) {
		IRIT_WARNING_MSG("Boolean: Objects do not intersect - Empty object result");
	    }
	    break;
	default:
	    sprintf(s, IRIT_EXP_STR("Boolean: Undefined Fatal Error (%d !?)"),
		    ErrorType);
	    IRIT_WARNING_MSG(s);
	    break;
    }

    longjmp(LclLongJumpBuffer, 1);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints the content of the given vertex list, to standard output.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:      Vertex list to print.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintVrtxList(IPVertexStruct *V)
{
    IPVertexStruct
	*VHead = V;

    do {
	IRIT_INFO_MSG_PRINTF("[NORMAL %8lf %8lf %8lf] %12lf %12lf %12lf",
	       V -> Normal[0], V -> Normal[1], V -> Normal[2],
	       V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	if (IP_IS_INTERNAL_VRTX(V))
	    IRIT_INFO_MSG(" (Internal)\n");
	else
	    IRIT_INFO_MSG("\n");
	V = V -> Pnext;
    }
    while (V!= NULL && V != VHead);
}

#endif /* DEBUG */
