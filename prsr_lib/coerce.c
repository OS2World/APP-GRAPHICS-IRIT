/******************************************************************************
* coerce.c - coerce an object into a different type.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, January 1993.				      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "prsr_loc.h"
#include "bool_lib.h"
#include "cagd_lib.h"
#include "trim_lib.h"
#include "mdl_lib.h"
#include "allocate.h"

#define CAGD_PT_E_TYPES  case CAGD_PT_E1_TYPE: \
			 case CAGD_PT_E2_TYPE: \
			 case CAGD_PT_E3_TYPE: \
			 case CAGD_PT_E4_TYPE: \
			 case CAGD_PT_E5_TYPE: \
			 case CAGD_PT_E6_TYPE: \
			 case CAGD_PT_E7_TYPE: \
			 case CAGD_PT_E8_TYPE: \
			 case CAGD_PT_E9_TYPE: \
			 case CAGD_PT_E10_TYPE: \
			 case CAGD_PT_E11_TYPE: \
			 case CAGD_PT_E12_TYPE: \
			 case CAGD_PT_E13_TYPE: \
			 case CAGD_PT_E14_TYPE: \
			 case CAGD_PT_E15_TYPE: \
			 case CAGD_PT_E16_TYPE: \
			 case CAGD_PT_E17_TYPE: \
			 case CAGD_PT_E18_TYPE:

#define CAGD_PT_P_TYPES  case CAGD_PT_P1_TYPE: \
			 case CAGD_PT_P2_TYPE: \
			 case CAGD_PT_P3_TYPE: \
			 case CAGD_PT_P4_TYPE: \
			 case CAGD_PT_P5_TYPE: \
			 case CAGD_PT_P6_TYPE: \
			 case CAGD_PT_P7_TYPE: \
			 case CAGD_PT_P8_TYPE: \
			 case CAGD_PT_P9_TYPE: \
			 case CAGD_PT_P10_TYPE: \
			 case CAGD_PT_P11_TYPE: \
			 case CAGD_PT_P12_TYPE: \
			 case CAGD_PT_P13_TYPE: \
			 case CAGD_PT_P14_TYPE: \
			 case CAGD_PT_P15_TYPE: \
			 case CAGD_PT_P16_TYPE: \
			 case CAGD_PT_P17_TYPE: \
			 case CAGD_PT_P18_TYPE:

#define CAGD_PT_TYPES    \
			 CAGD_PT_E_TYPES \
			 CAGD_PT_P_TYPES

static IPObjectStruct *CurveReverse(const IPObjectStruct *CrvObj);
static IPObjectStruct *SurfaceReverse(const IPObjectStruct *SrfObj);
static IPObjectStruct *ModelReverse(const IPObjectStruct *SrfObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Given a set of points, returns the list's common denominator that spans   M
* the space of all the points, taking into account type Type.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:  List of points.                                              M
*   Type:       Point type that we must span its space as well.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPointType:   Point type that spans the space of point type Type as   M
*                    well as all points in PtObjList.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCoerceCommonSpace, coercion                                            M
*****************************************************************************/
CagdPointType IPCoerceCommonSpace(IPObjectStruct *PtObjList,
				  CagdPointType Type)
{
    int i,
	Dim = CAGD_NUM_OF_PT_COORD(Type),
	Proj = CAGD_IS_RATIONAL_PT(Type);
    IPObjectStruct *PtObj;

    if (!IP_IS_OLST_OBJ(PtObjList)) {
	IP_FATAL_ERROR(IP_ERR_LIST_OBJ_EXPECTED);
	return CAGD_PT_NONE;
    }

    for (i = 0; (PtObj = IPListObjectGet(PtObjList, i++)) != NULL; ) {
	if (IP_IS_CTLPT_OBJ(PtObj)) {
	    Dim = IRIT_MAX(Dim, CAGD_NUM_OF_PT_COORD(PtObj -> U.CtlPt.PtType));
	    Proj |= CAGD_IS_RATIONAL_PT(PtObj -> U.CtlPt.PtType);
	}
	else if (IP_IS_POINT_OBJ(PtObj) || IP_IS_VEC_OBJ(PtObj)) {
	    Dim = IRIT_MAX(Dim, 3);
	}
	else {
	    IP_FATAL_ERROR(IP_ERR_PT_OBJ_EXPECTED);
	    return CAGD_PT_NONE;
	}
    }

    return CAGD_MAKE_PT_TYPE(Proj, Dim);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a list of objects to Type.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:   Coerce points/vectors/control points in this list to Type.  M
*   Type:        A minimum space type to coerce to in PtObjList.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPointType:  The coercion type actually took place with in PtObjList. M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCoercePtsListTo, coercion                                              M
*****************************************************************************/
CagdPointType IPCoercePtsListTo(IPObjectStruct *PtObjList, CagdPointType Type)
{
    int i;
    IPObjectStruct *TmpObj, *PtObj;
    CagdPointType
	PtType = IPCoerceCommonSpace(PtObjList, Type);

    if (PtType != CAGD_PT_NONE) {
	for (i = 0; (PtObj = IPListObjectGet(PtObjList, i++)) != NULL; ) {
	    if (IP_IS_CTLPT_OBJ(PtObj)) {
		TmpObj = IPCoerceObjectTo(PtObj, PtType);
		PtObj -> U.CtlPt = TmpObj -> U.CtlPt;
		IPFreeObject(TmpObj);
	    }
	    else if (IP_IS_POINT_OBJ(PtObj) || IP_IS_VEC_OBJ(PtObj)) {
		TmpObj = IPCoerceObjectTo(PtObj, PtType);
		IPReallocNewTypeObject(PtObj, IP_OBJ_CTLPT);
		PtObj -> U.CtlPt = TmpObj -> U.CtlPt;
		IPFreeObject(TmpObj);
	    }
	}
    }

    return PtType;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces an object to a new object.					     M
*   Points, vectors, control points and planes can always be coerced between M
* themselves using this routine by specifying the new object type desired    M
* such as IP_OBJ_PLANE or control point type like CAGD_PT_E4_TYPE. 	     M
*   Control points of curves and surfaces may be coerced to a new type by    M
* prescribing the needed point type as NewType, such as CAGD_PT_P2_TYPE.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to coerce.                                             M
*   NewType:   New type which can be object type like IP_OBJ_VECTOR or point M
*              type like E2.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Newly coerced object.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCoerceObjectTo, coercion                                               M
*****************************************************************************/
IPObjectStruct *IPCoerceObjectTo(const IPObjectStruct *PObj, int NewType)
{
    int i;
    IrtRType Pt[CAGD_MAX_PT_SIZE];
    const IrtRType *R;
    IPObjectStruct
	*NewObj = NULL;

    switch (PObj -> ObjType) {
	case IP_OBJ_POINT:
	case IP_OBJ_VECTOR:
	case IP_OBJ_PLANE:
	case IP_OBJ_CTLPT:
	    if (PObj -> ObjType == NewType)
	        return IPCopyObject(NULL, PObj, FALSE);

	    Pt[0] = 1.0;
	    for (i = 1; i < CAGD_MAX_PT_SIZE; i++)
	        Pt[i] = 0.0;
	    switch (PObj -> ObjType) {
		case IP_OBJ_POINT:
		    IRIT_PT_COPY(&Pt[1], PObj -> U.Pt);
		    break;
		case IP_OBJ_VECTOR:
		    IRIT_PT_COPY(&Pt[1], PObj -> U.Vec);
		    break;
		case IP_OBJ_PLANE:
		    IRIT_PLANE_COPY(&Pt[1], PObj -> U.Plane);
		    break;
		case IP_OBJ_CTLPT:
		    R = PObj -> U.CtlPt.Coords;
		    CagdCoercePointTo(&Pt[1], CAGD_MAX_E_POINT,
				      (IrtRType * const *) &R,
				      -1, PObj -> U.CtlPt.PtType);
		    break;
		default:
		    break;
	    }

	    switch (NewType) {
		case IP_OBJ_POINT:
		    NewObj = IPGenPTObject(&Pt[1], &Pt[2], &Pt[3]);
		    break;
		case IP_OBJ_VECTOR:
		    NewObj = IPGenVECObject(&Pt[1], &Pt[2], &Pt[3]);
		    break;
		case IP_OBJ_PLANE:
		    NewObj = IPGenPLANEObject(&Pt[1], &Pt[2], &Pt[3], &Pt[4]);
		    break;
		case IP_OBJ_CTLPT:
		    NewObj = IPGenCTLPTObject(CAGD_PT_E3_TYPE, Pt);
		    break;
		CAGD_PT_P_TYPES
		    if (PObj -> ObjType == IP_OBJ_CTLPT)
		        CagdCoercePointTo(Pt, CAGD_MAX_P_POINT,
					  (IrtRType * const *) &R,
					  -1, PObj -> U.CtlPt.PtType);
		CAGD_PT_E_TYPES
		    NewObj = IPGenCTLPTObject((CagdPointType) NewType, Pt);
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
		default:
		    break;		    
	    }
	    break;
	case IP_OBJ_CURVE:
	    switch (NewType) {
		CAGD_PT_TYPES
		    NewObj = IPGenCRVObject(
				   CagdCoerceCrvsTo(PObj -> U.Crvs,
						    (CagdPointType) NewType,
						    TRUE));
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
		default:
		    break;
	    }
	    break;
	case IP_OBJ_SURFACE:
	    switch (NewType) {
		CAGD_PT_TYPES
		    NewObj = IPGenSRFObject(
				  CagdCoerceSrfsTo(PObj -> U.Srfs,
						   (CagdPointType) NewType,
						   TRUE));
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
	        case IP_OBJ_MODEL:
		    NewObj = IPGenMODELObject(MdlCnvrtSrf2Mdl(PObj -> U.Srfs));
		    break;
		default:
		    break;
	    }
	    break;
	case IP_OBJ_TRIVAR:
	    switch (NewType) {
		CAGD_PT_TYPES
		    NewObj = IPGenTRIVARObject(
				   TrivCoerceTVsTo(PObj -> U.Trivars,
						   (CagdPointType) NewType));
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
		default:
		    break;
	    }
	    break;
	case IP_OBJ_TRIMSRF:
	    switch (NewType) {
		CAGD_PT_TYPES
		    NewObj = IPGenTRIMSRFObject(TrimSrfNew(
			CagdCoerceSrfTo(PObj -> U.TrimSrfs -> Srf,
					(CagdPointType) NewType, FALSE),
			TrimCrvCopyList(PObj -> U.TrimSrfs -> TrimCrvList),
			TRUE));
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
	        case IP_OBJ_MODEL:
		    NewObj = IPGenMODELObject(
				 MdlCnvrtTrimmedSrf2Mdl(PObj -> U.TrimSrfs));
		    break;
		default:
		    break;
	    }
	    break;
	case IP_OBJ_TRISRF:
	    switch (NewType) {
		CAGD_PT_TYPES
		    NewObj = IPGenTRISRFObject(
				  TrngCoerceTriSrfsTo(PObj -> U.TriSrfs,
						   (CagdPointType) NewType));
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
		default:
		    break;
	    }
	    break;
	case IP_OBJ_MULTIVAR:
	    switch (NewType) {
		CAGD_PT_TYPES
		    NewObj = IPGenMULTIVARObject(
				   MvarCoerceMVsTo(PObj -> U.MultiVars,
						   (MvarPointType) NewType));
		    break;
	        case CAGD_PT_MAX_SIZE_TYPE:		    
		    /* Should not happend - might happen if CagdPointType  */
		    /* is modified in cagd_lib.h.			   */
		    assert(0);
		default:
		    break;
	    }
	    break;
	default:
	    break;
    }

    return NewObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to reverse a curve.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvObj:     Curve to reverse its parametrization.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    Reversed curve.                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   CurveReverse                                                             *
*****************************************************************************/
static IPObjectStruct *CurveReverse(const IPObjectStruct *CrvObj)
{
    const CagdCrvStruct *Crv;
    CagdCrvStruct *RCrv,
	*RevCrvs = NULL;

    for (Crv = CrvObj -> U.Crvs; Crv != NULL; Crv = Crv -> Pnext) {
        RCrv = CagdCrvReverse(Crv);
	IRIT_LIST_PUSH(RCrv, RevCrvs);
    }

    if (RevCrvs == NULL)
	return NULL;

    return IPGenCRVObject(CagdListReverse(RevCrvs));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to reverse a surface by reversing the U direction.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   SrfObj:     Surface to reverse.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    Reversed surface. The normal of the reversed        *
*                        surface is flipped with respected to the original   *
*                        surface, SrfObj, by 180 degrees.		     *
*****************************************************************************/
static IPObjectStruct *SurfaceReverse(const IPObjectStruct *SrfObj)
{
    IPObjectStruct
	*RetVal = NULL;

    if (IP_IS_SRF_OBJ(SrfObj)) {
        const CagdSrfStruct *Srf;
	CagdSrfStruct *RSrf,
	    *RevSrfs = NULL;

	for (Srf = SrfObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	    RSrf = CagdSrfReverse(Srf);
	    IRIT_LIST_PUSH(RSrf, RevSrfs);
	}

	if (RevSrfs != NULL)
	    RetVal = IPGenSRFObject(CagdListReverse(RevSrfs));
    }
    else if (IP_IS_TRIMSRF_OBJ(SrfObj)) {
        const TrimSrfStruct *Srf;
        TrimSrfStruct*RSrf,
	    *RevSrfs = NULL;

	for (Srf = SrfObj -> U.TrimSrfs; Srf != NULL; Srf = Srf -> Pnext) {
	    RSrf = TrimSrfReverse(Srf);
	    IRIT_LIST_PUSH(RSrf, RevSrfs);
	}

	if (RevSrfs != NULL)
	    RetVal = IPGenTRIMSRFObject(CagdListReverse(RevSrfs));
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to reverse a model.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   MdlObj:     Model to reverse inside out.	                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    Reversed model.                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   CurveReverse                                                             *
*****************************************************************************/
static IPObjectStruct *ModelReverse(const IPObjectStruct *MdlObj)
{
    return IPGenMODELObject(MdlModelNegate(MdlObj -> U.Mdls));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a similar hierarchy to given one but with reversed semantics.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Input object to reverse. 		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:     Reversed object.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPReverseObject                                                          M
*****************************************************************************/
IPObjectStruct *IPReverseObject(IPObjectStruct *PObj)
{
    IPObjectStruct *RetVal;

    switch (PObj ->ObjType) {
        case IP_OBJ_NUMERIC:
	    RetVal = IPGenNUMValObject(-PObj -> U.R);
	    break;
        case IP_OBJ_POINT:
	    RetVal = IPCopyObject(NULL, PObj, FALSE);
	    IRIT_PT_SCALE(RetVal -> U.Pt, -1);
	    break;
        case IP_OBJ_VECTOR:
	    RetVal = IPCopyObject(NULL, PObj, FALSE);
	    IRIT_VEC_SCALE(RetVal -> U.Vec, -1);
	    break;
        case IP_OBJ_CTLPT:
	    {
	        int i;
		CagdCtlPtStruct *CtlPt;

		RetVal = IPCopyObject(NULL, PObj, FALSE);
		CtlPt = &RetVal -> U.CtlPt;
		for (i = 1; i <= CAGD_NUM_OF_PT_COORD(CtlPt -> PtType); i++)
		    CtlPt -> Coords[i] = -CtlPt -> Coords[i];
	    }
	    break;
        case IP_OBJ_PLANE:
	    RetVal = IPCopyObject(NULL, PObj, FALSE);
	    IRIT_VEC_SCALE(RetVal -> U.Plane, -1);
	    break;
        case IP_OBJ_STRING:
	    {
	        int i, j;
		char *Str;

		RetVal = IPCopyObject(NULL, PObj, FALSE);
		Str = RetVal -> U.Str;
		for (i = 0, j = (int) strlen(Str) - 1; i < j; i++, j--)
		    IRIT_SWAP(char, Str[i], Str[j]);
	    }
	    break;
        case IP_OBJ_MATRIX:
	    {
	        IrtRType
		    R = -1.0;

		RetVal = IPAllocObject("", IP_OBJ_MATRIX, NULL);
		MatScale4by4(*RetVal -> U.Mat, *PObj -> U.Mat, &R);
	    }
	    break;
        case IP_OBJ_POLY:
	    if (IP_IS_POLYGON_OBJ(PObj)) {
	        RetVal = BooleanNEG(PObj);
	    }
	    else {
	        IPPolygonStruct *Pl;

		RetVal = IPCopyObject(NULL, PObj, FALSE);
		for (Pl = RetVal -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
		    Pl -> PVertex = IPReverseVrtxList2(Pl -> PVertex);
	    }
	    break;
         case IP_OBJ_CURVE:
	     RetVal = CurveReverse(PObj);
	    break;
         case IP_OBJ_SURFACE:
         case IP_OBJ_TRIMSRF:
	     RetVal = SurfaceReverse(PObj);
	     break;
         case IP_OBJ_MODEL:
	     RetVal = ModelReverse(PObj);
	     break;
         case IP_OBJ_LIST_OBJ:
	     {
	         int i = 0;
		 IPObjectStruct *PTmp;

		 RetVal = IPGenListObject(IP_GET_OBJ_NAME(PObj), NULL, NULL);
		 while ((PTmp = IPListObjectGet(PObj, i)) != NULL)
		     IPListObjectInsert(RetVal, i++, IPReverseObject(PTmp));
		 IPListObjectInsert(RetVal, i, NULL);
	     }
	     break;
         default:
	     RetVal = IPCopyObject(NULL, PObj, FALSE);
	     break;
    }

    IPCopyObjectAuxInfo(RetVal, PObj);
    IP_SET_OBJ_NAME2(RetVal, IP_GET_OBJ_NAME(PObj));

    return RetVal;
}
