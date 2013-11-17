/******************************************************************************
* GeoMat3d.c - Trans. Matrices.						      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 1990.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_loc.h"

/* #define DEBUG_GM_ENTRY_EXIT	Print information on entry/exit of routines. */

/* #define DEBUG_GM_TEST_3PTS2EQLTRL_MAPPING  Print the results of this fnc. */

#define MAP_POINTS_THRU_MAT(Mat, AccumMat) { \
    MatMultPtby4by4(Pt1, Pt1, Mat); \
    MatMultPtby4by4(Pt2, Pt2, Mat); \
    MatMultPtby4by4(Pt3, Pt3, Mat); \
    MatMultTwo4by4(AccumMat, AccumMat, Mat); }

IRIT_STATIC_DATA GMTransObjUpdateFuncType
    TransObjUpdateFunc = NULL;
IRIT_STATIC_DATA GMTransObjUpdateAnimCrvsFuncType
    TransObjAnimCrvUpdateFunc = NULL;

static IPObjectStruct *GMTransformObjectAux(const IPObjectStruct *PObj,
					    IrtHmgnMatType Mat,
					    int AnimUpdate);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate rotation object around the X axis in Degree degrees:   M
*                                                                            *
* PARAMETERS:                                                                M
*   Degree:     Amount of rotation, in degrees.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectRotX, rotation, transformations                            M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectRotX(const IrtRType *Degree)
{
    IrtHmgnMatType Mat;

    MatGenMatRotX1(IRIT_DEG2RAD(*Degree), Mat);  /* Generate the trans. mat. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate rotation object around the Y axis in Degree degrees:   M
*                                                                            *
* PARAMETERS:                                                                M
*   Degree:     Amount of rotation, in degrees.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectRotY, rotation, transformations                            M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectRotY(const IrtRType *Degree)
{
    IrtHmgnMatType Mat;

    MatGenMatRotY1(IRIT_DEG2RAD(*Degree), Mat); /* Generate the trans. mat. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate rotation object around the Z axis in Degree degrees:   M
*                                                                            *
* PARAMETERS:                                                                M
*   Degree:     Amount of rotation, in degrees.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectRotZ, rotation, transformations                            M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectRotZ(const IrtRType *Degree)
{
    IrtHmgnMatType Mat;

    MatGenMatRotZ1(IRIT_DEG2RAD(*Degree), Mat); /* Generate the trans. mat. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to generate rotation object to rotate Z to Dir:		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dir:        Vector to rotate Z axis to it.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir, GMGenMatrixZ2Dir2, GMGenMatObjectZ2Dir2                M
*   GMGenMatrixRotV2V, GMGenMatrixRotVec				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectZ2Dir, rotation, transformations                           M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectZ2Dir(const IrtVecType Dir)
{
    IrtHmgnMatType Mat;

    GMGenMatrixZ2Dir(Mat, Dir);			    /* Generate the matrix. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate rotation object around the vector Dir in Degree degs:  M
*                                                                            *
* PARAMETERS:                                                                M
*   Dir:        Vector to rotate Z axis to it.                               M
*   Dir2:       Vector to rotate X axis to it.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir2                                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir2, GMGenMatrixZ2Dir, GMGenMatObjectZ2Dir                 M
*   GMGenMatrixRotV2V, GMGenMatrixRotVec				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectZ2Dir2, rotation, transformations                          M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectZ2Dir2(const IrtVecType Dir,
				     const IrtVecType Dir2)
{
    IrtHmgnMatType Mat;

    GMGenMatrixZ2Dir2(Mat, Dir, Dir2);              /* Generate the matrix. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate rotation object around the vector Vec in Degree degs:  M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:        Vector to rotate along its axis.                             M
*   Degree:     Amount of rotation, in degrees.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir, GMGenMatrixZ2Dir2, GMGenMatObjectZ2Dir2                M
*   GMGenMatrixRotVec                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectRotVec, rotation, transformations                          M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectRotVec(const IrtVecType Vec,
				     const IrtRType *Degree)
{
    IrtHmgnMatType Mat;

    GMGenMatrixRotVec(Mat, Vec, *Degree);           /* Generate the matrix. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a translation object.                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:     Amount of translation, in X, Y, and Z.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGenMatTrans                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectTrans, translation, transformations                        M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectTrans(const IrtVecType Vec)
{
    IrtHmgnMatType Mat;

    /* Generate the transformation matrix */
    MatGenMatTrans(Vec[0], Vec[1], Vec[2], Mat);

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a scaling object.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:     Amount of scaling, in X, Y, and Z.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGenMatScale                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectScale, scaling, transformations                            M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectScale(const IrtVecType Vec)
{
    IrtHmgnMatType Mat;

    /* Generate the transformation matrix */
    MatGenMatScale(Vec[0], Vec[1], Vec[2], Mat);

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to extract the translational part of a matrix or dump it.          M
*                                                                            *
* PARAMETERS:                                                                M
*   MatObj:	     To operate on.                                          M
*   TransPortion:    TRUE to extract translational portion out of Mat, FALSE M
*		     to dump the translational portion from Mat.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A matrix object hold either the translational portion M
*		       of Mat or anything but the translational part.        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGetMatTransPortion, translation, transformations                       M
*****************************************************************************/
IPObjectStruct *GMGetMatTransPortion(const IPObjectStruct *MatObj,
				     int TransPortion)
{
    int i, j;
    IrtHmgnMatType Mat;

    IRIT_GEN_COPY(Mat, *MatObj -> U.Mat, sizeof(IrtHmgnMatType));

    if (TransPortion) {
	for (i = 0; i < 3; i++)
	    for (j = 0; j < 3; j++)
	        Mat[i][j] = i == j;
    }
    else {
	for (i = 0; i < 3; i++)
	    Mat[3][i] = 0.0;
    }

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to transform a list of polygons according to the prescribed      M
*  transformation matrix.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:       List of polygons to transform.                                M
*   Mat:       Transformation matrix.                                        M
*   IsPolygon: TRUE for polygons, for for polylines/points.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:    A list of transformed polygons.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMTransformObject                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTransformPolyList                                                      M
*****************************************************************************/
IPPolygonStruct *GMTransformPolyList(const IPPolygonStruct *Pls,
				     IrtHmgnMatType Mat,
				     int IsPolygon)
{
    IrtVecType Pin;
    IPPolygonStruct *Pl, *PlsCpy;
    IPVertexStruct *V, *VFirst;

    Pl = PlsCpy = IPCopyPolygonList(Pls);

    while (Pl != NULL) {
	if ((V = VFirst = Pl -> PVertex) == NULL) {
	    Pl = Pl -> Pnext;
	    continue;
	}

	IRIT_PT_ADD(Pin, V -> Coord, Pl -> Plane); /* Prepare point IN side. */
	MatMultPtby4by4(Pin, Pin, Mat);      /* Transformed relative to new. */

	do {
	    MatMultPtby4by4(V -> Coord, V -> Coord, Mat);     /* Update pos. */

	    if (IsPolygon) {
	        /* Update normal. */
	        MatMultVecby4by4(V -> Normal, V -> Normal, Mat);
		if (!IRIT_PT_EQ_ZERO(V -> Normal))
		    IRIT_PT_NORMALIZE(V -> Normal);
	    }

	    V = V -> Pnext;
	}
	while (V != VFirst && V != NULL);	       /* VList is circular! */

	if (IsPolygon)
	    IPUpdatePolyPlane2(Pl, Pin);	        /* Update plane eqn. */

	Pl = Pl -> Pnext;
    }

    return PlsCpy;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Set the update transform call back function to a new function.  This     M
* call back function is invoked with the original object, the transformed    M
* object and the transformation matrix, just before GMTransformObject is     M
  returned.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   UpdateFunc:   New call back function for GMTransformObject               M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMTransObjUpdateFuncType:     Old value of call back function.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMTransformObject, GMTransObjSetAnimCrvUpdateFunc                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTransObjSetUpdateFunc                                                  M
*****************************************************************************/
GMTransObjUpdateFuncType GMTransObjSetUpdateFunc(GMTransObjUpdateFuncType
						                    UpdateFunc)
{
    GMTransObjUpdateFuncType
	OldFunc = TransObjUpdateFunc;

    TransObjUpdateFunc = UpdateFunc;
    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the function to update the animation curves to work properly after  M
* the applied transformation Mat to the parent object whose PAnim are his.   M
*                                                                            *
* PARAMETERS:                                                                M
*   AnimUpdateFunc:  New animation crvs update function for obj transform.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   GMTransObjUpdateAnimCrvsFuncType:   Old function                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMTransformObject, GMTransObjSetUpdateFunc, GMTransObjUpdateAnimCrvs     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTransObjSetAnimCrvUpdateFunc                                           M
*****************************************************************************/
GMTransObjUpdateAnimCrvsFuncType GMTransObjSetAnimCrvUpdateFunc(
			      GMTransObjUpdateAnimCrvsFuncType AnimUpdateFunc)
{
    GMTransObjUpdateAnimCrvsFuncType
	OldFunc = TransObjAnimCrvUpdateFunc;

    TransObjAnimCrvUpdateFunc = AnimUpdateFunc;
    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to transform an object according to the transformation matrix.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Object to be transformed.                                     M
*   Mat:       Transformation matrix.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Transformed object.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMTransObjUpdateAnimCrvs, GMTransObjSetUpdateFunc, GMTransformPolyList   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTransformObject, transformations                                       M
*****************************************************************************/
IPObjectStruct *GMTransformObject(const IPObjectStruct *PObj,
				  IrtHmgnMatType Mat)
{
    return GMTransformObjectAux(PObj, Mat, TRUE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to transform an object according to the transformation matrix.     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:            Object to be transformed.                               *
*   Mat:             Transformation matrix.                                  *
*   AnimUpdate:      Do we need to update animation curves?                  *
*									     *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    Transformed object.                                 *
*                                                                            *
* SEE ALSO:                                                                  *
*   GMTransObjUpdateAnimCrvs, GMTransObjSetUpdateFunc                        *
*                                                                            *
* KEYWORDS:                                                                  *
*   GMTransformObject, transformations                                       *
*****************************************************************************/
static IPObjectStruct *GMTransformObjectAux(const IPObjectStruct *PObj,
					    IrtHmgnMatType Mat,
					    int AnimUpdate)
{
    int i;
    IPObjectStruct *NewPObj, *PTmp,
	*PAnim = AttrGetObjectObjAttrib(PObj, "animation");
    int AnimUpdateChildrens = AnimUpdate && (PAnim == NULL);

    if (IP_IS_POLY_OBJ(PObj)) {
        int IsPolygon = IP_IS_POLYGON_OBJ(PObj);

        NewPObj = IPGenPOLYObject(GMTransformPolyList(PObj -> U.Pl,
						      Mat, IsPolygon));

	if (IP_IS_POLYGON_OBJ(PObj))
	    IP_SET_POLYGON_OBJ(NewPObj);
	else if (IP_IS_POLYLINE_OBJ(PObj))
	    IP_SET_POLYLINE_OBJ(NewPObj);
	else if (IP_IS_POINTLIST_OBJ(PObj))
	    IP_SET_POINTLIST_OBJ(NewPObj);
    }
    else if (IP_IS_CRV_OBJ(PObj)) {
        CagdCrvStruct *Crv, *TCrv,
	    *TransCrvs = NULL;

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	for (Crv = NewPObj -> U.Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	    TCrv = CagdCrvMatTransform(Crv, Mat);
	    IRIT_LIST_PUSH(TCrv, TransCrvs);
	}
	CagdCrvFreeList(NewPObj -> U.Crvs);
	NewPObj -> U.Crvs = CagdListReverse(TransCrvs);
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
        CagdSrfStruct *Srf, *TSrf,
	    *TransSrfs = NULL;

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	for (Srf = NewPObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	    TSrf = CagdSrfMatTransform(Srf, Mat);
	    IRIT_LIST_PUSH(TSrf, TransSrfs);
	}
	CagdSrfFreeList(NewPObj -> U.Srfs);
	NewPObj -> U.Srfs = CagdListReverse(TransSrfs);
    }
    else if (IP_IS_TRIMSRF_OBJ(PObj)) {
	TrimSrfStruct *TrimSrf;

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	for (TrimSrf = NewPObj -> U.TrimSrfs;
	     TrimSrf != NULL;
	     TrimSrf = TrimSrf -> Pnext)
	    TrimSrfMatTransform(TrimSrf, Mat);
    }
    else if (IP_IS_MODEL_OBJ(PObj)) {
    	NewPObj = IPCopyObject(NULL, PObj, FALSE);
	MdlModelMatTransform(NewPObj -> U.Mdls, Mat);
    }
    else if (IP_IS_TRIVAR_OBJ(PObj)) {
	TrivTVStruct *Trivar;

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	for (Trivar = NewPObj -> U.Trivars;
	     Trivar != NULL;
	     Trivar = Trivar -> Pnext)
	    TrivTVMatTransform(Trivar, Mat);
    }
    else if (IP_IS_MVAR_OBJ(PObj)) {
	MvarMVStruct *Multivar;

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	for (Multivar = NewPObj -> U.MultiVars;
	     Multivar != NULL;
	     Multivar = Multivar -> Pnext)
	    MvarMVMatTransform(Multivar, Mat);
    }
    else if (IP_IS_TRISRF_OBJ(PObj)) {
	TrngTriangSrfStruct *TriSrf;

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	for (TriSrf = NewPObj -> U.TriSrfs;
	     TriSrf != NULL;
	     TriSrf = TriSrf -> Pnext)
	    TrngTriSrfMatTransform(TriSrf, Mat);
    }
    else if (IP_IS_POINT_OBJ(PObj)) {
    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	MatMultPtby4by4(NewPObj -> U.Pt, NewPObj -> U.Pt, Mat);
    }
    else if (IP_IS_CTLPT_OBJ(PObj)) {
	IPObjectStruct
	    *TmpObj = IPCoerceObjectTo(PObj, CAGD_PT_P3_TYPE);
	IrtRType R[4];

	IRIT_PT_COPY(R, &TmpObj -> U.CtlPt.Coords[1]);
	R[3] = TmpObj -> U.CtlPt.Coords[0];
	MatMultWVecby4by4(R, R, Mat);
	TmpObj -> U.CtlPt.Coords[0] = R[3];
	IRIT_PT_COPY(&TmpObj -> U.CtlPt.Coords[1], R);

    	NewPObj = IPCoerceObjectTo(TmpObj, PObj -> U.CtlPt.PtType);

	if (CAGD_NUM_OF_PT_COORD(PObj -> U.CtlPt.PtType) > 3) {
	    CAGD_GEN_COPY(&NewPObj -> U.CtlPt.Coords[4],
			  &PObj -> U.CtlPt.Coords[4],
			  (CAGD_NUM_OF_PT_COORD(PObj -> U.CtlPt.PtType) - 3)
			      			         * sizeof(CagdRType));
        }
	IPFreeObject(TmpObj);
    }
    else if (IP_IS_VEC_OBJ(PObj)) {
    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	MatMultPtby4by4(NewPObj -> U.Vec, NewPObj -> U.Vec, Mat);
    }
    else if (IP_IS_INSTNC_OBJ(PObj)) {
	IrtHmgnMatType InvMat;

	MatInverseMatrix(Mat, InvMat);

    	NewPObj = IPCopyObject(NULL, PObj, FALSE);

	/* Prepend the transformation and postpend the inverse... */
	MatMultTwo4by4(NewPObj -> U.Instance -> Mat,
		       NewPObj -> U.Instance -> Mat, Mat);
	MatMultTwo4by4(NewPObj -> U.Instance -> Mat,
		       InvMat, NewPObj -> U.Instance -> Mat);
    }
    else if (IP_IS_OLST_OBJ(PObj)) {
	IPObjectStruct *PObjTmp;

	NewPObj = IPAllocObject("", IP_OBJ_LIST_OBJ, NULL);

	/* Const-safe still, but requires coercing. */
    	for (i = 0; (PObjTmp = IPListObjectGet((IPObjectStruct *) PObj,
					       i)) != NULL; i++)
    	    IPListObjectInsert(NewPObj, i,
			       GMTransformObjectAux(PObjTmp, Mat,
						    AnimUpdateChildrens));
	IPListObjectInsert(NewPObj, i, NULL);
    }
    else {
        NewPObj = IPCopyObject(NULL, PObj, FALSE);
    }

    IP_SET_OBJ_NAME2(NewPObj, IP_GET_OBJ_NAME(PObj));

    if (NewPObj -> Attr != NULL)
        IP_ATTR_FREE_ATTRS(NewPObj -> Attr);
    NewPObj -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

    NewPObj -> Dpnds = IPOD_ATTR_COPY_DEPENDENCIES(PObj -> Dpnds);

    /* Need other special processing, once the object was transformed? */
    if (TransObjUpdateFunc != NULL) {
	TransObjUpdateFunc(PObj, NewPObj, Mat, AnimUpdate);
    }

    /* Update any animation curves that might be found in PObj. */
    if (PAnim != NULL) {
        if (AnimUpdate) {
	    int OldRefCount = IPSetCopyObjectReferenceCount(FALSE);

	    /* We create a complete copy of the animation curves (as oppose  */
	    /* to new references) as we are about to change them now.        */
	    PAnim = IPCopyObject(NULL, PAnim, TRUE);
	    IPSetCopyObjectReferenceCount(OldRefCount);

	    /* Update animation curves! */
	    PTmp = TransObjAnimCrvUpdateFunc == NULL ?
			GMTransObjUpdateAnimCrvs(PAnim, Mat) :
			TransObjAnimCrvUpdateFunc(PAnim, Mat);
	    IPFreeObject(PAnim);
	    PAnim = PTmp;

	    AttrSetObjectObjAttrib(NewPObj, "animation", PAnim, FALSE);
	}
	else {
	    AttrSetObjectObjAttrib(NewPObj, "animation",
				   IPCopyObject(NULL, PAnim, FALSE), FALSE);
	}
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Update the animation curves to work properly after the applied           M
* transformation Mat to the parent object whose PAnim are his.               M
*                                                                            *
* PARAMETERS:                                                                M
*   PAnim:    Animation curves to update following transformation matrix Mat.M
*   Mat:      The transformation matrix.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The updated animation curves' list.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMTransformObject                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTransObjUpdateAnimCrvs, transformations                                M
*****************************************************************************/
IPObjectStruct *GMTransObjUpdateAnimCrvs(IPObjectStruct *PAnim,
					 IrtHmgnMatType Mat)
{
    int Len, i, OldRefCount;
    IrtHmgnMatType InvMat;
    IPObjectStruct *PAnimFirst, *PAnimLast, *PTmp;

    MatInverseMatrix(Mat, InvMat);

    OldRefCount = IPSetCopyObjectReferenceCount(FALSE);
    if (IP_IS_OLST_OBJ(PAnim))
        PAnim = IPCopyObject(NULL, PAnim, TRUE);
    else
        PAnim = IPGenLISTObject(IPCopyObject(NULL, PAnim, TRUE));
    IPSetCopyObjectReferenceCount(OldRefCount);

    Len = IPListObjectLength(PAnim);

    PAnimFirst = IPListObjectGet(PAnim, 0);
    PAnimLast = IPListObjectGet(PAnim, Len - 1);

    if (IP_IS_MAT_OBJ(PAnimFirst) &&
	strcmp(IP_GET_OBJ_NAME(PAnimFirst), "_RVRSANIM") == 0) {
        /* Multiply InvMat with the first matrix. */
        MatMultTwo4by4(*PAnimFirst -> U.Mat, InvMat, *PAnimFirst -> U.Mat);
    }
    else {
        /* Shift list forward and insert a matrix at the beginning. */
        for (i = Len - 1; i >= 0; i--) {
	    IPListObjectInsert(PAnim, i + 1, PTmp = IPListObjectGet(PAnim, i));
	    PTmp -> Count--;        /* No really a new additional reference. */
	}
	IPListObjectInsert(PAnim, ++Len, NULL);

	/* And place the inverse matrix as first object. */
	IPListObjectInsert(PAnim, 0,
			   IPGenMatObject("_RVRSANIM", InvMat, NULL));
    }

    if (IP_IS_MAT_OBJ(PAnimLast) &&
	strcmp(IP_GET_OBJ_NAME(PAnimLast), "_FRWDANIM") == 0) {
        /* Multiply Mat with the last matrix. */
        MatMultTwo4by4(*PAnimLast -> U.Mat, *PAnimLast -> U.Mat, Mat);
    }
    else {
        /* Append the transformation matrix as the last element in list. */
        IPListObjectInsert(PAnim, Len,
			   IPGenMatObject("_FRWDANIM", Mat, NULL));

	IPListObjectInsert(PAnim, Len + 1, NULL);
    }

    return PAnim;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to transform an list of objects according to a transformation      M
* matrix.	                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Object list to transform.                                    M
*   Mat:        Transformation matrix.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Transformed object list.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMTransformObjectList, transformations                                   M
*****************************************************************************/
IPObjectStruct *GMTransformObjectList(const IPObjectStruct *PObj,
				      IrtHmgnMatType Mat)
{
    IPObjectStruct
	*PTailObj = NULL,
	*PTransObj = NULL;

    for ( ; PObj != NULL; PObj = PObj -> Pnext) {
	if (PTailObj == NULL)
	    PTailObj = PTransObj = GMTransformObject(PObj, Mat);
	else {
	    PTailObj -> Pnext = GMTransformObject(PObj, Mat);
	    PTailObj = PTailObj -> Pnext;
	}
    }

    return PTransObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to prepare a transformation martix to do the following (in this  M
* order): scale by Scale, rotate such that the Z axis is in direction Dir    M
* and then translate by Trans.						     M
*    Algorithm: given the Trans vector, it forms the 4th line of Mat. Dir is M
* used to form the second line (the first 3 lines set the rotation), and     M
* finally Scale is used to scale first 3 lines/columns to the needed scale:  M
*                |  Tx  Ty  Tz  0 |   A transformation which takes the coord V
*                |  Bx  By  Bz  0 |  system into T, N & B as required and    V
* [X  Y  Z  1] * |  Nx  Ny  Nz  0 |  then translate it to C. T, N, B are     V
*                |  Cx  Cy  Cz  1 |  scaled by Scale.			     V
* N is exactly Dir (unit vec) but we got freedom on T & B which must be on   M
* a plane perpendicular to N and perpendicular between them but thats all!   M
* T is therefore selected using this (heuristic ?) algorithm:		     M
* Let P be the axis of which the absolute N coefficient is the smallest.     M
* Let B be (N cross P) and T be (B cross N).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Trans:     Translation factor.                                           M
*   Dir:       Direction to take Z axis to.                                  M
*   Scale:     Scaling factor.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    GMGenTransMatrixZ2Dir, transformations, rotation                        M
*****************************************************************************/
void GMGenTransMatrixZ2Dir(IrtHmgnMatType Mat,
			   const IrtVecType Trans,
			   const IrtVecType Dir,
			   IrtRType Scale)
{
    int i, j;
    IrtRType R;
    IrtVecType DirN, T, B, P;
    IrtHmgnMatType TempMat;

    IRIT_PT_COPY(DirN, Dir);
    IRIT_PT_NORMALIZE(DirN);
    IRIT_PT_RESET(P);
    for (i = 1, j = 0, R = IRIT_FABS(DirN[0]); i < 3; i++)
	if (R > IRIT_FABS(DirN[i])) {
	    R = DirN[i];
	    j = i;
	}
    P[j] = 1.0;/* Now P is set to the axis with the biggest angle from DirN. */

    GMVecCrossProd(B, DirN, P);			      /* Calc the bi-normal. */
    IRIT_PT_NORMALIZE(B);
    GMVecCrossProd(T, B, DirN);				/* Calc the tangent. */

    MatGenUnitMat(Mat);
    for (i = 0; i < 3; i++) {
	Mat[0][i] = T[i];
	Mat[1][i] = B[i];
	Mat[2][i] = DirN[i];
    }
    MatGenMatUnifScale(Scale, TempMat);
    MatMultTwo4by4(Mat, TempMat, Mat);

    MatGenMatTrans(Trans[0], Trans[1], Trans[2], TempMat);
    MatMultTwo4by4(Mat, Mat, TempMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to generate rotation matrix to rotate X to Dir:		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Dir:       Direction to take X axis to.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatrixX2Dir, rotation, transformations                              M
*****************************************************************************/
void GMGenMatrixX2Dir(IrtHmgnMatType Mat, const IrtVecType Dir)
{
    IrtHmgnMatType Mat1, Mat2;

    MatGenMatRotY1(-M_PI * 0.5, Mat1);			   /* Rotate X to Z. */
    GMGenMatrixZ2Dir(Mat2, Dir);	    /* Generate the Z to Dir matrix. */
    MatMultTwo4by4(Mat, Mat1, Mat2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to generate rotation matrix to rotate Y to Dir:		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Dir:       Direction to take Y axis to.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatrixY2Dir, rotation, transformations                              M
*****************************************************************************/
void GMGenMatrixY2Dir(IrtHmgnMatType Mat, const IrtVecType Dir)
{
    IrtHmgnMatType Mat1, Mat2;

    MatGenMatRotX1(M_PI * 0.5, Mat1);			   /* Rotate Y to Z. */
    GMGenMatrixZ2Dir(Mat2, Dir);	    /* Generate the Z to Dir matrix. */
    MatMultTwo4by4(Mat, Mat1, Mat2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Same as GMGenTransMatrixZ2Dir but with no scaling and/or translation.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Dir:       Direction to take Z axis to.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    GMGenMatrixZ2Dir, transformations, rotation                             M
*****************************************************************************/
void GMGenMatrixZ2Dir(IrtHmgnMatType Mat, const IrtVecType Dir)
{
    IrtVecType Trans;

    IRIT_PT_RESET(Trans);
    GMGenTransMatrixZ2Dir(Mat, Trans, Dir, 1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to prepare a transformation martix to do the following (in this  M
* order): scale by Scale, rotate such that the Z axis is in direction Dir    M
* and X axis is direction Dir2 and then translate by Trans.		     M
*    Algorithm: given the Trans vector, it forms the 4th line of Mat. Dir is M
* used to form the second line (the first 3 lines set the rotation), and     M
* finally Scale is used to scale first 3 lines/columns to the needed scale:  M
*                |  Tx  Ty  Tz  0 |   A transformation which takes the coord V
*                |  Bx  By  Bz  0 |  system into T, N & B as required and    V
* [X  Y  Z  1] * |  Nx  Ny  Nz  0 |  then translate it to C. T, N, B are     V
*                |  Cx  Cy  Cz  1 |  scaled by Scale.			     V
* N is exactly Dir (unit vec) and T is exactly Dir2.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Trans:     Translation factor.                                           M
*   Dir:       Direction to take Z axis to.                                  M
*   Dir2:      Direction to take X axis to.                                  M
*   Scale:     Scaling factor.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    GMGenTransMatrixZ2Dir2, transformations, rotation                       M
*****************************************************************************/
void GMGenTransMatrixZ2Dir2(IrtHmgnMatType Mat,
			    const IrtVecType Trans,
			    const IrtVecType Dir,
			    const IrtVecType Dir2,
			    IrtRType Scale)
{
    int i;
    IrtVecType DirN, Dir2N, B;
    IrtHmgnMatType TempMat;

    IRIT_PT_COPY(DirN, Dir);
    IRIT_PT_NORMALIZE(DirN);
    IRIT_PT_COPY(Dir2N, Dir2);
    IRIT_PT_NORMALIZE(Dir2N);

    GMVecCrossProd(B, DirN, Dir2N);		      /* Calc the bi-normal. */

    MatGenUnitMat(Mat);
    for (i = 0; i < 3; i++) {
	Mat[0][i] = Dir2N[i];
	Mat[1][i] = B[i];
	Mat[2][i] = DirN[i];
    }
    MatGenMatUnifScale(Scale, TempMat);
    MatMultTwo4by4(Mat, TempMat, Mat);

    MatGenMatTrans(Trans[0], Trans[1], Trans[2], TempMat);
    MatMultTwo4by4(Mat, Mat, TempMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Same as GMGenTransMatrixZ2Dir2 but with no scaling and/or translation.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Dir:       Direction to take Z axis to.                                  M
*   Dir2:      Direction to take X axis to.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    GMGenMatrixZ2Dir2, transformations, rotation                            M
*****************************************************************************/
void GMGenMatrixZ2Dir2(IrtHmgnMatType Mat,
		       const IrtVecType Dir,
		       const IrtVecType Dir2)
{
    IrtVecType Trans;

    IRIT_PT_RESET(Trans);
    GMGenTransMatrixZ2Dir2(Mat, Trans, Dir, Dir2, 1.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to create a viewing transformation matrix, standing at Pos, and    M
* looking in direction Dir, with UpDir being the upper viewing direction     M
*   Creates a transformation matrix that takes Pos to DEFAULT_VIEW_POS       M
* and rotate Dir into the Z axis, and UpDir into the Y axis.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos:        The location of the viewer.                                  M
*   Dir:        The viewing direction.					     M
*   UpDir:      The direction of up.  Must be different than Dir.	     M
*   Mat:	Matrix to update.					     N
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	TRUE if succesful, FALSE otherwise.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMatFromPosDir, transformations					     M
*****************************************************************************/
int GMMatFromPosDir(const IrtPtType Pos,
		    const IrtVecType Dir,
		    const IrtVecType UpDir,
		    IrtHmgnMatType Mat)
{
    IrtVecType SideDir, NormDir;
    IrtHmgnMatType InvMat;

    IRIT_VEC_COPY(NormDir, Dir);
    if (IRIT_VEC_SQR_LENGTH(NormDir) < IRIT_UEPS)
        return FALSE;
    IRIT_VEC_NORMALIZE(NormDir);			 /* Should go to Z. */

    IRIT_CROSS_PROD(SideDir, NormDir, UpDir);
    if (IRIT_VEC_SQR_LENGTH(SideDir) < IRIT_UEPS)
        return FALSE;
    IRIT_VEC_NORMALIZE(SideDir);			 /* Should go to X. */

    GMGenTransMatrixZ2Dir2(InvMat, Pos, NormDir, SideDir, 1.0);

    MatInverseMatrix(InvMat, Mat);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a transformation matrix that rotates V1 to V2.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   V1, V2:    Vector to rotate from (V1) to (V2).                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir, GMGenMatrixZ2Dir2, GMGenMatObjectZ2Dir2                M
*   GMGenMatrixRotVec                                                        M
*                                                                            M
* KEYWORDS:                                                                  M
*   GMGenMatrixRotV2V, transformations, rotation                             M
*****************************************************************************/
void GMGenMatrixRotV2V(IrtHmgnMatType Mat,
		       const IrtVecType V1,
		       const IrtVecType V2)
{
    IrtHmgnMatType Mat1, Mat1i, Mat2;
    IrtVecType V, V1Copy, V2Copy;

    IRIT_VEC_COPY(V1Copy, V1);
    IRIT_VEC_COPY(V2Copy, V2);
    IRIT_VEC_SAFE_NORMALIZE(V1Copy);
    IRIT_VEC_SAFE_NORMALIZE(V2Copy);

    IRIT_CROSS_PROD(V, V1Copy, V2Copy);
    
    if (IRIT_PT_EQ_ZERO(V)) {
	if (IRIT_DOT_PROD(V1Copy, V2Copy) > 0.0)		   /* Same vectors. */
	    MatGenUnitMat(Mat);
	else
	    MatGenMatUnifScale(-1.0, Mat);	       /* Opposite vectors. */

	return;
    }

    IRIT_VEC_SAFE_NORMALIZE(V);

    GMGenMatrixZ2Dir(Mat1i, V);
    MatTranspMatrix(Mat1i, Mat1);		     /* Compute the inverse. */

    /* Rotate both vectors to the XY plane. */
    MatMultVecby4by4(V1Copy, V1Copy, Mat1);
    MatMultVecby4by4(V2Copy, V2Copy, Mat1);

    MatGenMatRotZ(IRIT_DOT_PROD_2D(V1Copy, V2Copy),
		  IRIT_CROSS_PROD_2D(V1Copy, V2Copy), Mat2);
    MatMultTwo4by4(Mat1, Mat1, Mat2);

    MatMultTwo4by4(Mat, Mat1, Mat1i);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a transformation matrix that rotates V1 to V2.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2:    To compute the rotation from (V1) to (V2).                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir, GMGenMatrixZ2Dir2, GMGenMatObjectZ2Dir2                M
*   GMGenMatrixRotVec, GMGenMatrixRotV2V                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenMatObjectV2V, rotation, transformations                             M
*****************************************************************************/
IPObjectStruct *GMGenMatObjectV2V(const IrtVecType V1, const IrtVecType V2)
{
    IrtHmgnMatType Mat;

    GMGenMatrixRotV2V(Mat, V1, V2);                 /* Generate the matrix. */

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the linear transform that maps the given planar triangle         M
* Pt1Pt2Pt3 to an equilateral triangle around the origin so that edge        M
* Pt1Pt2 is horizontal and remains of the same size.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1Orig, Pt2Orig, Pt3Orig:   The three vertices of the input triangle.   M
*   Mat:		         The computed transform.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrix3Pts2EqltrlTri                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GM3Pts2EqltrlTriMat                                                      M
*****************************************************************************/
int GM3Pts2EqltrlTriMat(const IrtPtType Pt1Orig,
			const IrtPtType Pt2Orig,
			const IrtPtType Pt3Orig,
			IrtHmgnMatType Mat)
{
    IrtRType R;
    IrtHmgnMatType TMat;
    IrtPtType Pt, Pt1, Pt2, Pt3;

    IRIT_PT_COPY(Pt1, Pt1Orig);
    IRIT_PT_COPY(Pt2, Pt2Orig);
    IRIT_PT_COPY(Pt3, Pt3Orig);

    MatGenUnitMat(Mat);

    /* Bring Pt1 to origin. */
    MatGenMatTrans(-Pt1[0], -Pt1[1], -Pt1[2], TMat);
    MAP_POINTS_THRU_MAT(TMat, Mat);

    /* Rotate to Pt2 to the X axis. */
    R = IRIT_PT2D_LENGTH(Pt2);

    MatGenMatRotZ1(-atan2(Pt2[1] / R, Pt2[0] / R), TMat);
    MAP_POINTS_THRU_MAT(TMat, Mat);

    /* Build a shear matrix that makes the X coordinate of Pt3 half of Pt2. */
    if (IRIT_FABS(Pt3[1]) < IRIT_UEPS)
        return FALSE;

    MatGenUnitMat(TMat);
    TMat[1][0] = (Pt2[0] * 0.5 - Pt3[0]) / Pt3[1];
    MAP_POINTS_THRU_MAT(TMat, Mat);

    /* Scale in Y so Pt3[1] is sin(60) of dist(Pt1, Pt2). */
    MatGenMatScale(1.0, IRIT_FABS(Pt1[0] - Pt2[0]) * sin(M_PI / 3) / Pt3[1],
		   1.0, TMat);
    MAP_POINTS_THRU_MAT(TMat, Mat);

    /* Move origin to centroid. */
    IRIT_PT_COPY(Pt, Pt1);
    IRIT_PT_ADD(Pt, Pt, Pt2);
    IRIT_PT_ADD(Pt, Pt, Pt3);
    IRIT_PT_SCALE(Pt, 1.0 / 3.0);
    MatGenMatTrans(-Pt[0], -Pt[1], -Pt[2], TMat);
    MAP_POINTS_THRU_MAT(TMat, Mat);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugGMTest3Pts2EqltrlMapping, FALSE) {
	    printf("Original Points:\n");
	    printf("	Pt1 = %10.7lg %10.7lg %10.7lg\n",
		   Pt1Orig[0], Pt1Orig[1], Pt1Orig[2]);
	    printf("	Pt2 = %10.7lg %10.7lg %10.7lg\n",
		   Pt2Orig[0], Pt2Orig[1], Pt2Orig[2]);
	    printf("	Pt3 = %10.7lg %10.7lg %10.7lg\n",
		   Pt3Orig[0], Pt3Orig[1], Pt3Orig[2]);
	    printf("Mapped 1 Points:\n");
	    printf("	Pt1 = %10.7lg %10.7lg %10.7lg\n",
		   Pt1[0], Pt1[1], Pt1[2]);
	    printf("	Pt2 = %10.7lg %10.7lg %10.7lg\n",
		   Pt2[0], Pt2[1], Pt2[2]);
	    printf("	Pt3 = %10.7lg %10.7lg %10.7lg\n",
		   Pt3[0], Pt3[1], Pt3[2]);
	    printf("Mapped 2 Points:\n");
	    MatMultPtby4by4(Pt, Pt1Orig, Mat);
	    printf("	Pt1 = %10.7lg %10.7lg %10.7lg\n", Pt[0], Pt[1], Pt[2]);
	    MatMultPtby4by4(Pt, Pt2Orig, Mat);
	    printf("	Pt2 = %10.7lg %10.7lg %10.7lg\n", Pt[0], Pt[1], Pt[2]);
	    MatMultPtby4by4(Pt, Pt3Orig, Mat);
	    printf("	Pt3 = %10.7lg %10.7lg %10.7lg\n", Pt[0], Pt[1], Pt[2]);
	}
    }
#   endif /* DEBUG */

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the linear transform that maps the given planar triangle         M
* Pt1Pt2Pt3 to an equilateral triangle around the origin so that edge        M
* Pt1Pt2 is horizontal and remains of the same size.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:    The three points to compute the mapping for.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A matrix object.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GM3Pts2EqltrlTriMat                                                      M
*                                                                            M
* KEYWORDS:                                                                  M
*   GMGenMatrix3Pts2EqltrlTri, transformations, shear                        M
*****************************************************************************/
IPObjectStruct *GMGenMatrix3Pts2EqltrlTri(const IrtPtType Pt1,
					  const IrtPtType Pt2,
					  const IrtPtType Pt3)
{
    IrtHmgnMatType Mat;

    if (!GM3Pts2EqltrlTriMat(Pt1, Pt2, Pt3, Mat))
	return NULL;

    return IPGenMATObject(Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a transformation matrix that rotates the object around Vec,    M
* Angle degrees.                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       To place the computed transformation.                         M
*   Vec:       Vector to rotate along its axis.                              M
*   Degrees:   Amount ofrotation, in degrees.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenMatrixZ2Dir, GMGenMatrixZ2Dir2, GMGenMatObjectZ2Dir2                M
*   GMGenMatrixRotV2V                                                        M
*                                                                            M
* KEYWORDS:                                                                  M
*   GMGenMatrixRotVec, transformations, rotation                             M
*****************************************************************************/
void GMGenMatrixRotVec(IrtHmgnMatType Mat,
		       const IrtVecType Vec,
		       IrtRType Degrees)
{
    IrtHmgnMatType Mat1, Mat1i;

    GMGenMatrixZ2Dir(Mat1, Vec);
    MatTranspMatrix(Mat1, Mat1i);		     /* Compute the inverse. */

    MatGenMatRotZ1(IRIT_DEG2RAD(Degrees), Mat);

    MatMultTwo4by4(Mat, Mat, Mat1);
    MatMultTwo4by4(Mat, Mat1i, Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a matrix that projects 3D objects to the Projection Plane     M
* ProjPlane, having the eye at EyePos.                                       M
*   This solution is derived by solving for the intersection point of the    M
* line through the eye and the projected point and the given plane.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   ProjPlane:   The plane to project the objects onto.                      M
*   EyePos:      The position of the eye.                                    M
*   Mat:         Matrix to update.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenReflectionMat                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenProjectionMat                                                       M
*****************************************************************************/
void GMGenProjectionMat(const IrtPlnType ProjPlane,
			const IrtRType EyePos[4],
			IrtHmgnMatType Mat)
{
    int i, j;
    IrtRType
	Dot = ProjPlane[0] * EyePos[0] +
	      ProjPlane[1] * EyePos[1] +
	      ProjPlane[2] * EyePos[2] +
	      ProjPlane[3] * EyePos[3];

    for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
	    Mat[i][j] = -EyePos[j] * ProjPlane[i];

    for (i = 0; i < 4; i++)
	Mat[i][i] = Dot + Mat[i][i];

    if (Mat[3][3] != 0.0 && Mat[3][3] != 1.0) {
	IrtRType
	    w = 1 / Mat[3][3];

	/* Scale the matrix by 1/Mat[3][3]. */
	for (i = 0; i < 4; i++)
	    for (j = 0; j < 4; j++)
		Mat[i][j] *= w;
        
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a matrix that reflects 3D objects based upon the prescribed   M
* reflection plane, ReflectPlane.	                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   ReflectPlane:   The plane to computed a reflection matrix for.           M
*   Mat:            Matrix to update.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMGenProjectionMat, GMGenMatrixZ2Dir                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenReflectionMat                                                       M
*****************************************************************************/
void GMGenReflectionMat(const IrtPlnType ReflectPlane, IrtHmgnMatType Mat)
{
    IrtRType t;
    IrtPtType Pt;
    IrtHmgnMatType MatTrans, MatRot, MatFrwrd, MatBckwrd, MatRflct;

    /* Find a point on plane. */
    t = -ReflectPlane[3] / (IRIT_SQR(ReflectPlane[0]) +
			    IRIT_SQR(ReflectPlane[1]) +
			    IRIT_SQR(ReflectPlane[2]));
    IRIT_PT_COPY(Pt, ReflectPlane);
    printf("t = %f\n", t);
    IRIT_PT_SCALE(Pt, t);

    /* Bring the plane Z = 0 to the prescribed plane. */
    GMGenMatrixZ2Dir(MatRot, ReflectPlane);
    MatGenMatTrans(Pt[0], Pt[1], Pt[2], MatTrans);
    MatMultTwo4by4(MatFrwrd, MatTrans, MatRot);

    /* The reflection matrix around the Z = 0 plane. */
    MatGenMatScale(1, 1, -1, MatRflct);

    /* The back mapping. */
    MatInverseMatrix(MatFrwrd, MatBckwrd);

    /* Combine them all. */
    MatMultTwo4by4(Mat, MatRflct, MatFrwrd);
    MatMultTwo4by4(Mat, MatBckwrd, Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if a given points is contained in the given convex polygon.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:   Point to test for inclusion in convex polygon Pl.  Pt is assumed   M
*	  to be on the plane holding polygon Pl.	                     M
*   Pl:   Convex polygon to test for inclusion of Pt.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if Pt is inside Polygon Pl, FALSE otherwise.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPointOnPolygonBndry			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointInsideCnvxPolygon                                                 M
*****************************************************************************/
int GMPointInsideCnvxPolygon(const IrtPtType Pt, const IPPolygonStruct *Pl)
{
    IPVertexStruct
        *VHead = Pl -> PVertex,
	*V = VHead;
    IrtVecType VCross, V1, V2;
    IrtRType
	Sign = 0.0;

    do {
        IPVertexStruct
	    *VNext = V -> Pnext ? V -> Pnext : VHead;

	IRIT_PT_SUB(V1, Pt, V -> Coord);
	IRIT_PT_SUB(V2, V -> Coord, VNext -> Coord);
	IRIT_CROSS_PROD(VCross, V2, V1);
	if (Sign == 0.0)
	    Sign = IRIT_DOT_PROD(VCross, Pl -> Plane);
	else if (Sign * IRIT_DOT_PROD(VCross, Pl -> Plane) < 0.0)
	    return FALSE;

	V = VNext;
    }
    while (V != VHead);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if a given points is on the boundary of the given polygon.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt:   Point to test for inclusion in the boundary of polygon Pl.  Pt is  M
*	  assumed to be on the plane holding polygon Pl.                     M
*   Pl:   Polygon to test for inclusion of Pt.				     M
*   Eps:  Maximal distance from the boundary to be considered on boundary.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if Pt is on the boundary of Polygon Pl, FALSE otherwise.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPointInsideCnvxPolygon			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPointOnPolygonBndry                                                    M
*****************************************************************************/
int GMPointOnPolygonBndry(const IrtPtType Pt,
			  const IPPolygonStruct *Pl,
			  IrtRType Eps)
{
    IPVertexStruct
        *VHead = Pl -> PVertex,
	*V = VHead;
    IrtPtType Ptemp;
    IrtVecType Vec, Vec2;

    do {
        IPVertexStruct
	    *VNext = V -> Pnext ? V -> Pnext : VHead;

	IRIT_PT_SUB(Vec, VNext -> Coord, V -> Coord);

	/* Find closest point on the line. */
	GMPointFromPointLine(Pt, V -> Coord, Vec, Ptemp);
	if (IRIT_PT_PT_DIST_SQR(Pt, Ptemp) < IRIT_SQR(Eps)) {
	    /* On the line - lets see if in the segment (V, VNext). */
	    IRIT_PT_SUB(Vec2, Pt, V -> Coord);
	    if (IRIT_DOT_PROD(Vec2, Vec) > -Eps &&
		IRIT_PT_PT_DIST_SQR(Pt, V -> Coord) <
		    Eps + IRIT_PT_PT_DIST_SQR(VNext -> Coord, V -> Coord))
	        return TRUE;
	}

	V = VNext;
    }
    while (V != VHead);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if the given ray intersects the given convex polygon.              M
*                                                                            *
* PARAMETERS:                                                                M
*   RayOrigin:   Starting point of ray.                                      M
*   RayDir:      Direction of ray.                                           M
*   Pl:          Convex polygon to test against ray for intersection.        M
*   InterPoint:  Resulting intersection point.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if succesful, FALSE otherwise.                           *
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPointFromLinePlane, GMPointInsideCnvxPolygon                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMRayCnvxPolygonInter                                                    M
*****************************************************************************/
int GMRayCnvxPolygonInter(const IrtPtType RayOrigin,
			  const IrtVecType RayDir,
			  const IPPolygonStruct *Pl,
			  IrtPtType InterPoint)
{
    IrtRType t;

    if (!GMPointFromLinePlane(RayOrigin, RayDir, Pl -> Plane, InterPoint, &t))
	return FALSE;

    return GMPointInsideCnvxPolygon(InterPoint, Pl);
}

