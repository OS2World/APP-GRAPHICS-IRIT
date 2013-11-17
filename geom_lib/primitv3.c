/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.1, Oct. 2006   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to generate the geometric controller primitives.		     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "cagd_lib.h"
#include "allocate.h"
#include "attribut.h"
#include "geom_loc.h"

#define GM_PRIM_TRANS_CNTRL_CYL_RES	8
#define GM_PRIM_TRANS_CNTRL_SPHERE_RES	16
#define GM_PRIM_TRANS_CNTRL_MIN_REL_DIM 0.1
#define GM_PRIM_TRANS_CNTRL_SPHERE_RAD	(MaxDim / 20.0)
#define GM_PRIM_TRANS_CNTRL_CUBE_DIM	(MaxDim / 13.0)
#define GM_PRIM_TRANS_CNTRL_CYL_REL_RAD	(MaxDim / 50.0)
#define GM_PRIM_TRANS_CNTRL_MIN_CYL_LEN	0.25

#define GM_PRIM_TRANS_CNTRL_CUBE2D_DIM	(MaxDim / 25.0)
#define GM_PRIM_TRANS_CNTRL_MIN2D_REL_DIM 0.15

#define GM_PRIM_TRANS_CNTRL_FRAME_COLOR		155,  55, 155
#define GM_PRIM_TRANS_CNTRL_TRANS1_COLOR	200, 255, 250
#define GM_PRIM_TRANS_CNTRL_TRANS2_COLOR	255, 200, 200
#define GM_PRIM_TRANS_CNTRL_ROT_COLOR		255, 250, 150
#define GM_PRIM_TRANS_CNTRL_SCALE_COLOR		150, 255, 150

#define GM_PRIM_UPDATE_AXIS_VAL_V(Pl, Axs, Val) { \
    IPVertexStruct *_V = Pl -> PVertex; \
    do { \
        _V -> Coord[Axs] = Val; \
	_V = _V -> Pnext; \
    } \
    while (_V != NULL && _V != Pl -> PVertex); \
}
#define GM_PRIM_UPDATE_AXIS_VAL_P(Pls, Axs, Val) { \
    IPPolygonStruct *_Pl; \
    for (_Pl = Pls; _Pl != NULL; _Pl = _Pl -> Pnext) \
        GM_PRIM_UPDATE_AXIS_VAL_V(_Pl, Axs, Val) \
}

#define GM_PRIM_ADD_AXIS_VAL_V(Pl, Axs, Val) { \
    IPVertexStruct *_V = Pl -> PVertex; \
    do { \
        _V -> Coord[Axs] += Val; \
	_V = _V -> Pnext; \
    } \
    while (_V != NULL && _V != Pl -> PVertex); \
}
#define GM_PRIM_ADD_AXIS_VAL_P(Pls, Axs, Val) { \
    IPPolygonStruct *_Pl; \
    for (_Pl = Pls; _Pl != NULL; _Pl = _Pl -> Pnext) \
        GM_PRIM_ADD_AXIS_VAL_V(_Pl, Axs, Val) \
}

#define GM_PRIM_SCALE_VAL_V(Pl, X, Y, Z) { \
    IPVertexStruct *_V = Pl -> PVertex; \
    do { \
        _V -> Coord[0] *= X; \
        _V -> Coord[1] *= Y; \
        _V -> Coord[2] *= Z; \
	_V = _V -> Pnext; \
    } \
    while (_V != NULL && _V != Pl -> PVertex); \
}
#define GM_PRIM_SCALE_VAL_P(Pls, X, Y, Z) { \
    IPPolygonStruct *_Pl; \
    for (_Pl = Pls; _Pl != NULL; _Pl = _Pl -> Pnext) \
        GM_PRIM_SCALE_VAL_V(_Pl, X, Y, Z) \
}

static IPObjectStruct *GMMakeTransControlCylinder(IrtRType *Crnr1,
						  IrtRType *Crnr2,
						  IrtRType MaxDim,
						  char Axis);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a 2D bounding box with transformation handles to control the  M
* way an object is transformed in the plane.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox:             Dimensions of constructed box (only XY).               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list object with the proper sub-objects named as   M
*		    R for (Z) rotations, {X,Y} for translations.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenTransformController, PrimGenTransformController2DCrvs             M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenTransformController2D                                             M
*****************************************************************************/
IPObjectStruct *PrimGenTransformController2D(const GMBBBboxStruct *BBox)
{
    int i, j, l,
	OldRes = PrimSetResolution(GM_PRIM_TRANS_CNTRL_CYL_RES),
	OldPrimType = PrimSetGeneratePrimType(GM_GEN_PRIM_POLYS);
    char Name[IRIT_LINE_LEN], Name2[IRIT_LINE_LEN];
    IrtRType R, R2,
	MinDim = IRIT_INFNTY,
	MaxDim = 0.0;
    IrtVecType Dir, Vec, Crnrs[4], Dims, Cntr;
    GMBBBboxStruct B;
    IPPolygonStruct *Pl;
    IPObjectStruct *PObj, *PTmp, *PTmp2, *PObjTrans, *PObjRot,
	*PObjRet = IPGenLISTObject(NULL);

    /* Make sure the bbox is not degenerate in any way. */
    for (i = 0, MaxDim = 0.0; i < 2; i++) {
        B.Min[i] = IRIT_MIN(BBox -> Min[i], BBox -> Max[i]);
        B.Max[i] = IRIT_MAX(BBox -> Min[i], BBox -> Max[i]);
	Dims[i] = B.Max[i] - B.Min[i];
	MaxDim = IRIT_MAX(MaxDim, Dims[i]);
    }
    for (i = 0; i < 2; i++) {
	if (Dims[i] < MaxDim * GM_PRIM_TRANS_CNTRL_MIN_REL_DIM) {
	    B.Min[i] = B.Max[i] = (B.Min[i] + B.Max[i]) * 0.5;
	    B.Min[i] -= MaxDim * GM_PRIM_TRANS_CNTRL_MIN_REL_DIM * 0.5;
	    B.Max[i] += MaxDim * GM_PRIM_TRANS_CNTRL_MIN_REL_DIM * 0.5;
	    Dims[i] = B.Max[i] - B.Min[i];
	}
	MinDim = IRIT_MIN(MinDim, Dims[i]);
	Cntr[i] = (B.Min[i] + B.Max[i]) * 0.5;
    }
    Cntr[2] = 0.0;

    /* Create the four corners of the box. */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
	    int Idx = i + j * 2;

	    Crnrs[Idx][0] = i == 0 ? B.Min[0] : B.Max[0];
	    Crnrs[Idx][1] = j == 0 ? B.Min[1] : B.Max[1];
	    Crnrs[Idx][2] = 0.0;
	}
    }

    PObjTrans = IPGenLISTObject(NULL);

    for (i = 0; i < 3; i++)
        Vec[i] = Cntr[i] - GM_PRIM_TRANS_CNTRL_CUBE_DIM * 0.5;
    PObj = PrimGenBOXObject(Vec, GM_PRIM_TRANS_CNTRL_CUBE_DIM,
			         GM_PRIM_TRANS_CNTRL_CUBE_DIM,
				 GM_PRIM_TRANS_CNTRL_CUBE_DIM);

    for (l = 0; l < 2; l++) {
        for (i = 0; i < 2; i++) {
	    sprintf(Name, "%c", 'X' + i);

	    /* Create the face-central boxs - translate in one axis. */
	    PTmp = IPCopyObject(NULL, PObj, FALSE);
	    IP_SET_OBJ_NAME2(PTmp, Name);
	    R = (l == 0 ? B.Min[i] : B.Max[i]) - Cntr[i];
	    GM_PRIM_ADD_AXIS_VAL_P(PTmp -> U.Pl, i, R);
	    AttrSetObjectRGBColor(PTmp, GM_PRIM_TRANS_CNTRL_TRANS2_COLOR);
	    IPListObjectAppend(PObjTrans, PTmp);

	    for (j = i + 1; j < 2; j++) {
		/* Use the one axis translation boxes and transform them */
		/* once more to create translate-in-some-plane boxes.    */
		sprintf(Name2, "%c%c", 'X' + i, 'X' + j);

		PTmp2 = IPCopyObject(NULL, PTmp, FALSE);
		R2 = B.Min[j] - Cntr[j];
		GM_PRIM_ADD_AXIS_VAL_P(PTmp2 -> U.Pl, j, R2);
		IP_SET_OBJ_NAME2(PTmp2, Name2);
		AttrSetObjectRGBColor(PTmp2, GM_PRIM_TRANS_CNTRL_TRANS1_COLOR);
		IPListObjectAppend(PObjTrans, PTmp2);

		PTmp2 = IPCopyObject(NULL, PTmp, FALSE);
		R2 = B.Max[j] - Cntr[j];
		GM_PRIM_ADD_AXIS_VAL_P(PTmp2 -> U.Pl, j, R2);
		IP_SET_OBJ_NAME2(PTmp2, Name2);
		AttrSetObjectRGBColor(PTmp2, GM_PRIM_TRANS_CNTRL_TRANS1_COLOR);
		IPListObjectAppend(PObjTrans, PTmp2);
	    }
	}
    }

    Pl = PrimGenPolyline4Vrtx(Crnrs[0], Crnrs[1], Crnrs[3], Crnrs[2], NULL);
    PTmp = IPGenPOLYLINEObject(Pl);
    IP_SET_OBJ_NAME2(PTmp, "RotFrame");
    AttrSetObjectRGBColor(PTmp, GM_PRIM_TRANS_CNTRL_TRANS2_COLOR);
    IPListObjectAppend(PObjTrans, PTmp);

    IPFreeObject(PObj);
    IPListObjectAppend(PObjRet, PObjTrans);

    /* Do the Min/Max cylinder frame. */
    PrimSetResolution(GM_PRIM_TRANS_CNTRL_CYL_RES);

    PObjRot = IPGenLISTObject(NULL);

    for (i = 0; i < 4; i++) {
        IRIT_STATIC_DATA int
	    Idx[4] = { 0, 1, 3, 2 };

        IRIT_VEC_SUB(Dir, Crnrs[Idx[(i + 1) & 0x03]], Crnrs[Idx[i]]);

	PTmp = PrimGenCYLINObject(Crnrs[Idx[i]], Dir,
				  GM_PRIM_TRANS_CNTRL_CYL_REL_RAD, 0);
	IP_SET_OBJ_NAME2(PTmp, "R");
	IPListObjectAppend(PObjRot, PTmp);
    }
    IPListObjectAppend(PObjRet, PObjRot);

    PrimSetResolution(OldRes);
    PrimSetGeneratePrimType(OldPrimType);

    IPForEachPoly(PObjRet, (void (*) (IPPolygonStruct *)) IPUpdatePolyPlane);

    return PObjRet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a 2D bounding box with transformation handles to control the  M
* way an object is transformed in the plane.  This transform is formed out   M
* of curves only.				                             M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox:             Dimensions of constructed box (only XY).               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list object with the proper sub-objects named as   M
*		    R for (Z) rotations, {X,Y} for translations              M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenTransformController, PrimGenTransformController2D                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenTransformController2DCrvs                                         M
*****************************************************************************/
IPObjectStruct *PrimGenTransformController2DCrvs(const GMBBBboxStruct *BBox)
{
    IRIT_STATIC_DATA CagdPtStruct
	Origin = { NULL, NULL, { 0.0, 0.0, 0.0 } };
    int i, j;
    IrtRType h,
	MinDim = IRIT_INFNTY,
	MaxDim = 0.0;
    IrtVecType Vec, Crnrs[4], Dims, Cntr;
    GMBBBboxStruct B;
    CagdCrvStruct *SquareCrv, *CircleCrv;
    IPObjectStruct *PObj, *PTmp, *PObjTrans;

    /* Make sure the bbox is not degenerate in any way. */
    for (i = 0, MaxDim = 0.0; i < 2; i++) {
        B.Min[i] = IRIT_MIN(BBox -> Min[i], BBox -> Max[i]);
        B.Max[i] = IRIT_MAX(BBox -> Min[i], BBox -> Max[i]);
	Dims[i] = B.Max[i] - B.Min[i];
	MaxDim = IRIT_MAX(MaxDim, Dims[i]);
    }
    for (i = 0; i < 2; i++) {
	if (Dims[i] < MaxDim * GM_PRIM_TRANS_CNTRL_MIN2D_REL_DIM) {
	    B.Min[i] = B.Max[i] = (B.Min[i] + B.Max[i]) * 0.5;
	    B.Min[i] -= MaxDim * GM_PRIM_TRANS_CNTRL_MIN2D_REL_DIM * 0.5;
	    B.Max[i] += MaxDim * GM_PRIM_TRANS_CNTRL_MIN2D_REL_DIM * 0.5;
	    Dims[i] = B.Max[i] - B.Min[i];
	}
	MinDim = IRIT_MIN(MinDim, Dims[i]);
	Cntr[i] = (B.Min[i] + B.Max[i]) * 0.5;
    }
    Cntr[2] = 0.0;

    /* Create the four corners of the box. */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
	    int Idx = i + j * 2;

	    Crnrs[Idx][0] = i == 0 ? B.Min[0] : B.Max[0];
	    Crnrs[Idx][1] = j == 0 ? B.Min[1] : B.Max[1];
	    Crnrs[Idx][2] = 0.0;
	}
    }

    /* Make the main frame. */
    PObj = IPGenCrvObject("R",
			  CagdPrimRectangleCrv(Crnrs[0][0], Crnrs[0][1],
					       Crnrs[3][0], Crnrs[3][1], 0.0),
			  NULL);

    PObjTrans = IPGenLISTObject(PObj);

    h = GM_PRIM_TRANS_CNTRL_CUBE2D_DIM;
    SquareCrv = CagdPrimRectangleCrv(-h, -h, h, h, 0.0);
    CircleCrv = BspCrvCreatePCircle(&Origin, h);

    /* Create the corner circles. */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 2; i++) {
	    int Idx = i + j * 2;

	    PTmp = IPGenCRVObject(CagdCrvCopy(CircleCrv));
	    CagdCrvTransform(PTmp -> U.Crvs, Crnrs[Idx], 1.0);
	    IP_SET_OBJ_NAME2(PTmp, "XY");
	    AttrSetObjectRGBColor(PTmp, GM_PRIM_TRANS_CNTRL_TRANS1_COLOR);
	    IPListObjectAppend(PObjTrans, PTmp);
	}
    }

    /* Create the edge squares. */
    Vec[2] = 0.0;
    for (i = 0; i < 2; i++) {
	PTmp = IPGenCrvObject("X", CagdCrvCopy(SquareCrv), NULL);
	Vec[0] = i == 0 ? B.Min[0] : B.Max[0];
	Vec[1] = Cntr[1];
	CagdCrvTransform(PTmp -> U.Crvs, Vec, 1.0);
	AttrSetObjectRGBColor(PTmp, GM_PRIM_TRANS_CNTRL_TRANS2_COLOR);
	IPListObjectAppend(PObjTrans, PTmp);

	PTmp = IPGenCrvObject("Y", CagdCrvCopy(SquareCrv), NULL);
	Vec[0] = Cntr[0];
	Vec[1] = i == 0 ? B.Min[1] : B.Max[1];
	CagdCrvTransform(PTmp -> U.Crvs, Vec, 1.0);
	AttrSetObjectRGBColor(PTmp, GM_PRIM_TRANS_CNTRL_TRANS2_COLOR);
	IPListObjectAppend(PObjTrans, PTmp);
    }

    CagdCrvFree(SquareCrv);
    CagdCrvFree(CircleCrv);

    return PObjTrans;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Create a cylinder rod of the transformation controller.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Crnr1, Crnr2:  The two end points of the cylinder's axis.                *
*   MaxDim:        The maxima dimension of the controller.		     *
*   Axis:	   'X' or 'Y' or 'Z'.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    Constructed cylinder.                               *
*****************************************************************************/
static IPObjectStruct *GMMakeTransControlCylinder(IrtRType *Crnr1,
						  IrtRType *Crnr2,
						  IrtRType MaxDim,
						  char Axis)
{
    char Name[IRIT_LINE_LEN];
    int Caps;
    IrtVecType Dir, DDir;
    IrtPtType PCrnr1, PCrnr2;
    IrtRType
        R = MaxDim * GM_PRIM_TRANS_CNTRL_MIN_CYL_LEN;
    IPObjectStruct *PCyl;

    IRIT_VEC_SUB(Dir, Crnr1, Crnr2);

    if (IRIT_VEC_SQR_LENGTH(Dir) < IRIT_SQR(R)) {
        IRIT_PT_COPY(PCrnr1, Crnr1);
	IRIT_PT_COPY(PCrnr2, Crnr2);

	/* Expand the cylinders beyond the frame. */
	R /= 4.0 * IRIT_VEC_LENGTH(Dir);
	IRIT_VEC_SCALE2(DDir, Dir, R);
	IRIT_PT_SUB(PCrnr2, PCrnr2, DDir);

	IRIT_VEC_SCALE(DDir, 2.0);
	IRIT_PT_ADD(Dir, Dir, DDir);

	Crnr1 = PCrnr1;
	Crnr2 = PCrnr2;

	Caps = 3;
    }
    else
        Caps = 0;

    PCyl = PrimGenCYLINObject(Crnr2, Dir,
			      GM_PRIM_TRANS_CNTRL_CYL_REL_RAD, Caps);
    sprintf(Name, "Rot_%c", Axis);
    IP_SET_OBJ_NAME2(PCyl, Name);

    return PCyl;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a bounding box with transformation handles to control the     M
* way an object is transformed.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox:             Dimensions of constructed box.                         M
*   HasRotation:      Should we add rotational handles?                      M
*   HasTranslation:   Should we add translation handles?                     M
*   HasUniformScale:  Should we add uniform scale handles?                   M
*   BoxOpacity:       Opacity of the bbox itself, between zero and one.  If  M
*		      one (fully transparent), no bbox geometry is created.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A list object with the proper sub-objects named as   M
*		    R{X,Y,Z} for rotations, T{X,Y,Z} for translations, and   M
*		    SXYZ for scaling.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenTransformController2D, PrimGenTransformController2DCrvs           M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenTransformController                                               M
*****************************************************************************/
IPObjectStruct *PrimGenTransformController(const GMBBBboxStruct *BBox, 
					   int HasRotation,
					   int HasTranslation,
					   int HasUniformScale,
					   IrtRType BoxOpacity)
{
    int i, j, k, l,
	OldRes = PrimSetResolution(GM_PRIM_TRANS_CNTRL_CYL_RES),
	OldPrimType = PrimSetGeneratePrimType(GM_GEN_PRIM_POLYS);
    char Name[IRIT_LINE_LEN], Name2[IRIT_LINE_LEN];
    IrtRType R, R2,
	MinDim = IRIT_INFNTY,
	MaxDim = 0.0;
    IrtVecType Vec, Crnrs[8], Dims, Cntr;
    GMBBBboxStruct B;
    IPPolygonStruct *Pl;
    IPObjectStruct *PObj, *PTmp, *PTmp2,
	*PObjRet = IPGenLISTObject(NULL);

    /* Make sure the bbox is not degenerate in any way. */
    for (i = 0, MaxDim = 0.0; i < 3; i++) {
        B.Min[i] = IRIT_MIN(BBox -> Min[i], BBox -> Max[i]);
        B.Max[i] = IRIT_MAX(BBox -> Min[i], BBox -> Max[i]);
	Dims[i] = B.Max[i] - B.Min[i];
	MaxDim = IRIT_MAX(MaxDim, Dims[i]);
    }
    for (i = 0; i < 3; i++) {
	if (Dims[i] < MaxDim * GM_PRIM_TRANS_CNTRL_MIN_REL_DIM) {
	    B.Min[i] = B.Max[i] = (B.Min[i] + B.Max[i]) * 0.5;
	    B.Min[i] -= MaxDim * GM_PRIM_TRANS_CNTRL_MIN_REL_DIM * 0.5;
	    B.Max[i] += MaxDim * GM_PRIM_TRANS_CNTRL_MIN_REL_DIM * 0.5;
	    Dims[i] = B.Max[i] - B.Min[i];
	}
	MinDim = IRIT_MIN(MinDim, Dims[i]);
	Cntr[i] = (B.Min[i] + B.Max[i]) * 0.5;
    }

    /* Create the eight corners of the box. */
    for (k = 0; k < 2; k++) {
	for (j = 0; j < 2; j++) {
	    for (i = 0; i < 2; i++) {
	        int Idx = i + j * 2 + k * 4;

		Crnrs[Idx][0] = i == 0 ? B.Min[0] : B.Max[0];
		Crnrs[Idx][1] = j == 0 ? B.Min[1] : B.Max[1];
		Crnrs[Idx][2] = k == 0 ? B.Min[2] : B.Max[2];
	    }
	}
    }

    if (HasTranslation) {
        IPObjectStruct
	    *PObjTrans = IPGenListObject("Translations", NULL, NULL);

	for (i = 0; i < 3; i++)
	    Vec[i] = Cntr[i] - GM_PRIM_TRANS_CNTRL_CUBE_DIM * 0.5;
        PObj = PrimGenBOXObject(Vec, GM_PRIM_TRANS_CNTRL_CUBE_DIM,
				     GM_PRIM_TRANS_CNTRL_CUBE_DIM,
				     GM_PRIM_TRANS_CNTRL_CUBE_DIM);
	IP_SET_OBJ_NAME2(PObj, "BboxFrame");

	for (l = 0; l < 2; l++) {
	    for (i = 0; i < 3; i++) {
		/* Create the face-central boxs - translate in one axis. */
		PTmp = IPCopyObject(NULL, PObj, FALSE);
		R = (l == 0 ? B.Min[i] : B.Max[i]) - Cntr[i];
		GM_PRIM_ADD_AXIS_VAL_P(PTmp -> U.Pl, i, R);

		for (j = i + 1; j < 3; j++) {
		    /* Use the one axis translation boxes and transform them */
		    /* once more to create translate-in-some-plane boxes.    */
		    sprintf(Name2, "TransScale_%c%c", 'X' + i, 'X' + j);

		    PTmp2 = IPCopyObject(NULL, PTmp, FALSE);
		    R2 = B.Min[j] - Cntr[j];
		    GM_PRIM_ADD_AXIS_VAL_P(PTmp2 -> U.Pl, j, R2);
		    IP_SET_OBJ_NAME2(PTmp2, Name2);
		    AttrSetObjectRGBColor(PTmp2,
					  GM_PRIM_TRANS_CNTRL_TRANS1_COLOR);
		    IPListObjectAppend(PObjTrans, PTmp2);

		    PTmp2 = IPCopyObject(NULL, PTmp, FALSE);
		    R2 = B.Max[j] - Cntr[j];
		    GM_PRIM_ADD_AXIS_VAL_P(PTmp2 -> U.Pl, j, R2);
		    IP_SET_OBJ_NAME2(PTmp2, Name2);
		    AttrSetObjectRGBColor(PTmp2,
					  GM_PRIM_TRANS_CNTRL_TRANS1_COLOR);
		    IPListObjectAppend(PObjTrans, PTmp2);
		}
		IPFreeObject(PTmp);

		for (j = 0; j < 3; j++)
		    Vec[j] = (i == j ? (l == 0 ? B.Min[j] : B.Max[j])
				     : Cntr[j]) -
			GM_PRIM_TRANS_CNTRL_CUBE_DIM * (i != j ? 0.4 : 0.8);
		PTmp = PrimGenBOXObject(Vec,
			GM_PRIM_TRANS_CNTRL_CUBE_DIM * (i != 0 ? 0.8 : 1.6),
			GM_PRIM_TRANS_CNTRL_CUBE_DIM * (i != 1 ? 0.8 : 1.6),
			GM_PRIM_TRANS_CNTRL_CUBE_DIM * (i != 2 ? 0.8 : 1.6));
	        sprintf(Name, "TransScale_%c", 'X' + i);
		IP_SET_OBJ_NAME2(PTmp, Name);
		AttrSetObjectRGBColor(PTmp, GM_PRIM_TRANS_CNTRL_TRANS2_COLOR);
		IPListObjectAppend(PObjTrans, PTmp);
	    }
	}
	IPFreeObject(PObj);

	IPListObjectAppend(PObjRet, PObjTrans);
    }

    if (HasRotation) {
        IPObjectStruct
	    *PObjRot = IPGenListObject("Rotations", NULL, NULL);

	PrimSetResolution(GM_PRIM_TRANS_CNTRL_CYL_RES);

	/* Do the Min/Max Z planes. */
	for (i = 0; i < 4; i++) {
	    IRIT_STATIC_DATA int
	        Idx[4] = { 0, 1, 3, 2 };

	    PTmp = GMMakeTransControlCylinder(Crnrs[Idx[(i + 1) & 0x03]],
					      Crnrs[Idx[i]], MaxDim,
					      i & 0x01 ? 'Y' : 'X');
	    IPListObjectAppend(PObjRot, PTmp);

	    PTmp2 = IPCopyObject(NULL, PTmp, TRUE);
	    GM_PRIM_ADD_AXIS_VAL_P(PTmp2 -> U.Pl, 2, Dims[2]);
	    IPListObjectAppend(PObjRot, PTmp2);
	}

	/* Do the vertical bars. */
	for (i = 0; i < 4; i++) {
	    PTmp = GMMakeTransControlCylinder(Crnrs[i + 4], Crnrs[i],
					      MaxDim, 'Z');

	    IPListObjectAppend(PObjRot, PTmp);
	}

	AttrSetObjectRGBColor(PObjRot, GM_PRIM_TRANS_CNTRL_ROT_COLOR);
	IPListObjectAppend(PObjRet, PObjRot);
    }

    if (HasUniformScale) {
        IPObjectStruct
	    *PObjScale = IPGenListObject("Scales", NULL, NULL);

	PrimSetResolution(GM_PRIM_TRANS_CNTRL_SPHERE_RES);

        for (i = 0; i < 8; i++) {
	    PTmp = PrimGenSPHEREObject(Crnrs[i],
				       GM_PRIM_TRANS_CNTRL_SPHERE_RAD);
	    IP_SET_OBJ_NAME2(PTmp, "Scale_XYZ");
	    IPListObjectAppend(PObjScale, PTmp);
	}

	AttrSetObjectRGBColor(PObjScale, GM_PRIM_TRANS_CNTRL_SCALE_COLOR);
	IPListObjectAppend(PObjRet, PObjScale);
    }

    /* Create the box. */
    if (BoxOpacity == 1.0) {
        if (!HasRotation) {
	    /* Make just wireframe. */
	    PObj = PrimGenBOXWIREObject(Crnrs[0], Dims[0], Dims[1], Dims[2]);
	    IP_SET_OBJ_NAME2(PObj, "BboxFrame");
	}
	else
	    PObj = NULL;
    }
    else {
        /* Make a polygonal object and set transparency values. */
        PObj = PrimGenBOXObject(B.Min, Dims[0], Dims[1], Dims[2]);
	IP_SET_OBJ_NAME2(PObj, "BboxFrame");
	AttrSetObjectRealAttrib(PObj, "transp", 1.0 - BoxOpacity);
    }
    if (PObj != NULL) {
        AttrSetObjectRGBColor(PObj, GM_PRIM_TRANS_CNTRL_FRAME_COLOR);
        IPListObjectAppend(PObjRet, PObj);
    }

    /* Create the middle wireframe. */
    Pl = PrimGenPolyline4Vrtx(Crnrs[0], Crnrs[1], Crnrs[3], Crnrs[2], NULL);
    GM_PRIM_UPDATE_AXIS_VAL_V(Pl, 2, Cntr[2]);
    Pl = PrimGenPolyline4Vrtx(Crnrs[0], Crnrs[1], Crnrs[5], Crnrs[4], Pl);
    GM_PRIM_UPDATE_AXIS_VAL_V(Pl, 1, Cntr[1]);
    Pl = PrimGenPolyline4Vrtx(Crnrs[0], Crnrs[2], Crnrs[6], Crnrs[4], Pl);
    GM_PRIM_UPDATE_AXIS_VAL_V(Pl, 0, Cntr[0]);
    PObj = IPGenPolylineObject("BboxMidFrame", Pl, NULL);
    AttrSetObjectRGBColor(PObj, GM_PRIM_TRANS_CNTRL_FRAME_COLOR);

    IPListObjectAppend(PObjRet, PObj);

    PrimSetResolution(OldRes);
    PrimSetGeneratePrimType(OldPrimType);

    IPForEachPoly(PObjRet, (void (*) (IPPolygonStruct *)) IPUpdatePolyPlane);

    AttrPropagateAttr(PObjRet, NULL);

    return PObjRet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to create a polyline out of a list of 4 vertices V1/2/3/4.	     M
*   No test is made to make sure the 4 points are co-planar...		     M
*   The points are placed in order.                      		     M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2, V3, V4:    Four vertices of the constructed polyline.  V1 is     M
*		       duplicated as fifth last point as well.               M
*   Pnext:             Next is chain of polyline, in linked list.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: The constructed polygon.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   PrimGenPolyline4Vrtx, PrimGenPolygon3Vrtx, GMGenPolyline2Vrtx            M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrimGenPolyline4Vrtx                                                     M
*****************************************************************************/
IPPolygonStruct *PrimGenPolyline4Vrtx(const IrtVecType V1,
				      const IrtVecType V2,
				      const IrtVecType V3,
				      const IrtVecType V4,
				      IPPolygonStruct *Pnext)
{
    IPPolygonStruct *PPoly;
    IPVertexStruct *V;

    PPoly = IPAllocPolygon(0, V = IPAllocVertex2(NULL), Pnext);
    IRIT_PT_COPY(V -> Coord, V1);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V2);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V3);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V4);

    V -> Pnext = IPAllocVertex2(NULL); V = V -> Pnext;
    IRIT_PT_COPY(V -> Coord, V1);

    return PPoly;
}

#ifdef DEBUG_PRIM_TRANS_CNTRL

/*****************************************************************************
* AUXILIARY:								     *
* main routine to test PrimGenTransformController.		             *
*****************************************************************************/
void main(int argc, char **argv)
{
    GMBBBboxStruct
	B = { {-1, -1, -1 }, { 1, 1, 1 } };
    int i,
	Do2DControl = FALSE,
	Do2DCrvControl = FALSE,
        HasRotation = FALSE,
        HasTranslation = FALSE,
        HasUniformScale = FALSE;
    IPObjectStruct *PObj;
    IrtRType
	BoxOpacity = 1.0;

    while (argc > 1) {
	if (argv[1][0] == '-') {
	    switch (argv[1][1]) {
		case 'b':
		    sscanf(argv[2], "%lf", &B.Min[0]);
		    sscanf(argv[3], "%lf", &B.Min[1]);
		    sscanf(argv[4], "%lf", &B.Min[2]);
		    sscanf(argv[5], "%lf", &B.Max[0]);
		    sscanf(argv[6], "%lf", &B.Max[1]);
		    sscanf(argv[7], "%lf", &B.Max[2]);
		    fprintf(stderr, "Bbox read [%f %f %f] :: [%f %f %f]\n",
			    B.Min[0], B.Min[1], B.Min[2],
			    B.Max[0], B.Max[1], B.Max[2]);
		    argc -= 7;
		    argv += 7;
		    break;
		case 'c':
		    Do2DCrvControl = TRUE;
		    argc--;
		    argv++;
		    break;
	        case 'o':
		    sscanf(argv[2], "%lf", &BoxOpacity);
		    fprintf(stderr, "Has opacity = %f\n", BoxOpacity);
		    argc -= 2;
		    argv += 2;
		    break;
	        case 'r':
		    HasRotation = TRUE;
		    fprintf(stderr, "Has rotation\n");
		    argc--;
		    argv++;
		    break;
	        case 's':
		    HasUniformScale = TRUE;
		    fprintf(stderr, "Has uniform scale\n");
		    argc--;
		    argv++;
		    break;
	        case 't':
		    HasTranslation = TRUE;
		    fprintf(stderr, "Has translation\n");
		    argc--;
		    argv++;
		    break;
		case '2':
		    Do2DControl = TRUE;
		    argc--;
		    argv++;
		    break;
		default:
		    fprintf(stderr, "Unknown option %c\n", argv[1][1]);
		    IritSleep(1000);
		    exit(1);
	    }
	}
	else {
	    break;
	}
    }

    if (Do2DCrvControl) {
	PObj = PrimGenTransformController2DCrvs(&B);
	IPStdoutObject(PObj, FALSE);
    }
    else if (Do2DControl) {
	PObj = PrimGenTransformController2D(&B);
	IPStdoutObject(PObj, FALSE);
    }
    else {
        fprintf(stderr, "Starting allocating 100 controls...");
	for (i = 0; i < 100; i++) {
	    PObj = PrimGenTransformController(&B, HasRotation, HasTranslation,
					      HasUniformScale, BoxOpacity);
	    IPFreeObject(PObj);
	}
	fprintf(stderr, "Done.\n");

	PObj = PrimGenTransformController(&B, HasRotation, HasTranslation,
					  HasUniformScale, BoxOpacity);
	IPStdoutObject(PObj, FALSE);
    }

    IPFreeObject(PObj);
}
    
#endif /* DEBUG_PRIM_TRANS_CNTRL */
