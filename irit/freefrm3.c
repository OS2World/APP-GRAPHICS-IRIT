/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to provide the required interfact for the cagd library for the    *
* free form surfaces and curves.					     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "allocate.h"
#include "attribut.h"
#include "objects.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "ip_cnvrt.h"
#include "freeform.h"

static int CAVerifyParamType(IPObjectStruct *Params,
			     int Index,
			     IPObjStructType ObjType,
			     void **ParamAddr);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Editing of a single control point of a curve.			     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjCrv:   Curve to edit one of its control points.                      M
*   CtlPt:     Ptoint to replace PObjCrv's Index point.                      M
*   Index:     Index of point to replace in PObjCrv's control polygon.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A new curve after the replacement.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   EditCrvControlPoint                                                      M
*****************************************************************************/
IPObjectStruct *EditCrvControlPoint(IPObjectStruct *PObjCrv,
				    IPObjectStruct *CtlPt,
				    IrtRType *Index)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct *Crv;

    if (!IP_IS_CTLPT_OBJ(CtlPt))
        return NULL;

    Crv = CagdEditSingleCrvPt(PObjCrv -> U.Crvs, &CtlPt -> U.CtlPt,
			      IRIT_REAL_PTR_TO_INT(Index), TRUE);

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Editing of a single control point of a surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSrf:        Surface to edit one of its control points.               M
*   CtlPt:          Ptoint to replace PObjSrf's U/VIndex point.              M
*   UIndex, VIndex: Indices of point to replace in PObjSrf's control mesh.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A new surface after the replacement.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   EditSrfControlPoint                                                      M
*****************************************************************************/
IPObjectStruct *EditSrfControlPoint(IPObjectStruct *PObjSrf,
				    IPObjectStruct *CtlPt,
				    IrtRType *UIndex,
				    IrtRType *VIndex)
{
    IPObjectStruct *SrfObj;
    CagdSrfStruct *Srf;

    if (!IP_IS_CTLPT_OBJ(CtlPt))
        return NULL;

    Srf = CagdEditSingleSrfPt(PObjSrf -> U.Srfs, &CtlPt -> U.CtlPt,
			      IRIT_REAL_PTR_TO_INT(UIndex),
			      IRIT_REAL_PTR_TO_INT(VIndex), TRUE);

    SrfObj = IPGenSRFObject(Srf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Editing of a single control point of a trivariate.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjTV:         Trivariate to edit one of its control points.            M
*   CtlPt:          Ctl point to replace PObjTV's U/V/WIndex point.          M
*   UIndex, VIndex, WIndex: Indices of point to replace in PObjTV's	     M
*		    control mesh.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A new trivariate after the replacement.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   EditTVControlPoint                                                       M
*****************************************************************************/
IPObjectStruct *EditTVControlPoint(IPObjectStruct *PObjTV,
				   IPObjectStruct *CtlPt,
				   IrtRType *UIndex,
				   IrtRType *VIndex,
				   IrtRType *WIndex)
{
    IPObjectStruct *TVObj;
    TrivTVStruct *TV;

    if (!IP_IS_CTLPT_OBJ(CtlPt))
        return NULL;

    TV = TrivEditSingleTVPt(PObjTV -> U.Trivars, &CtlPt -> U.CtlPt,
			    IRIT_REAL_PTR_TO_INT(UIndex),
			    IRIT_REAL_PTR_TO_INT(VIndex),
			    IRIT_REAL_PTR_TO_INT(WIndex), TRUE);

    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Raises the degree of the given curve.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjCrv:     Curve to raise its degree.                                  M
*   RNewOrder:   New degree of curve.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Curve identical to PObjCrv, but with RNewOrder Order. M
*                                                                            *
* KEYWORDS:                                                                  M
*   RaiseCurveObject                                                         M
*****************************************************************************/
IPObjectStruct *RaiseCurveObject(IPObjectStruct *PObjCrv, IrtRType *RNewOrder)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
	*Crv = PObjCrv -> U.Crvs;
    int OldOrder = Crv -> Order,
	NewOrder = IRIT_REAL_PTR_TO_INT(RNewOrder);

    if (NewOrder <= OldOrder) {
	IRIT_NON_FATAL_ERROR("Order to raise to less then current order");
	return NULL;
    }

    CrvObj = IPGenCRVObject(CagdCrvDegreeRaiseN(Crv, NewOrder));

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reduces the degree of the given curve.  Result is typically an	     M
* aapproximation.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjCrv:     Curve to reduce its degree.                                 M
*   RNewOrder:   New degree of curve.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Curve identical to PObjCrv, but with one degree less. M
*                                                                            *
* KEYWORDS:                                                                  M
*   ReduceCurveObject                                                        M
*****************************************************************************/
IPObjectStruct *ReduceCurveObject(IPObjectStruct *PObjCrv, IrtRType *RNewOrder)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct *TCrv,
	*Crv = PObjCrv -> U.Crvs;
    int OldOrder = Crv -> Order,
	NewOrder = IRIT_REAL_PTR_TO_INT(RNewOrder);

    if (NewOrder >= OldOrder) {
	IRIT_NON_FATAL_ERROR("Order to reduce to larger then current order");
	return NULL;
    }
    NewOrder = IRIT_MAX(NewOrder, 1);

    while (OldOrder-- > NewOrder) {
	TCrv = CagdCrvDegreeReduce(Crv);
	if (Crv != PObjCrv -> U.Crvs)
	    CagdCrvFree(Crv);
	Crv = TCrv;
    }

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Raises the degree of the given surface.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSrf:     Surface to raise its degree.                                M
*   RDir:        Direction to raise degre. Either U or V.                    M
*   RNewOrder:   New degree of surface.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Surface identical to PObjSrf, but with RNewOrder      M
*		       Order in direction RDir.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RaiseSurfaceObject                                                       M
*****************************************************************************/
IPObjectStruct *RaiseSurfaceObject(IPObjectStruct *PObjSrf,
				   IrtRType *RDir,
				   IrtRType *RNewOrder)
{
    IPObjectStruct *SrfObj;
    CagdSrfStruct
	*Srf = PObjSrf -> U.Srfs;
    CagdSrfDirType
	Dir = (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(RDir);
    int OldOrder = Dir == CAGD_CONST_V_DIR ? Srf -> VOrder : Srf -> UOrder,
	NewOrder = IRIT_REAL_PTR_TO_INT(RNewOrder);

    if (NewOrder <= OldOrder) {
	IRIT_WNDW_PUT_STR("Order to raise to less then current order");
	return NULL;
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Srf = CagdSrfBlossomDegreeRaiseN(Srf, NewOrder, Srf -> VOrder);
	    break;
        case CAGD_CONST_V_DIR:
	    Srf = CagdSrfBlossomDegreeRaiseN(Srf, Srf -> UOrder, NewOrder);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    return NULL;
    }

    SrfObj = IPGenSRFObject(Srf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Raises the degree of the given trivariate.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjTV:      Trivar to raise its degree.                                 M
*   RDir:        Direction to raise degree. Either U, V, or W.               M
*   RNewOrder:   New degree of trivariate.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Trivar identical to PObjTV, but with RNewOrder        M
*		       Order in direction RDir.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RaiseTrivarObject                                                        M
*****************************************************************************/
IPObjectStruct *RaiseTrivarObject(IPObjectStruct *PObjTV,
				  IrtRType *RDir,
				  IrtRType *RNewOrder)
{
    IPObjectStruct *TVObj;
    TrivTVStruct
	*TV = PObjTV -> U.Trivars;
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    int OldOrder,
	NewOrder = IRIT_REAL_PTR_TO_INT(RNewOrder);

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    OldOrder = TV -> UOrder;
	    break;
	case TRIV_CONST_V_DIR:
	    OldOrder = TV -> VOrder;
	    break;
	case TRIV_CONST_W_DIR:
	    OldOrder = TV -> WOrder;
	    break;
	default:
	    IRIT_WNDW_PUT_STR("Undefined parametric direction");
	    return NULL;
    }

    if (NewOrder <= OldOrder) {
	IRIT_WNDW_PUT_STR("Order to raise to less then current order");
	return NULL;
    }

    TV = TrivTVDegreeRaiseN(TV, Dir, NewOrder);

    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Raises the degree of the given multivariate.                               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjMV:      Multivar to raise its degree.                               M
*   RDir:        Direction to raise degree.                                  M
*   RNewOrder:   New degree of multivariate.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Multivar identical to PObjMV, but with RNewOrder      M
*		       Order in direction RDir.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RaiseMultivarObject                                                      M
*****************************************************************************/
IPObjectStruct *RaiseMultivarObject(IPObjectStruct *PObjMV,
				    IrtRType *RDir,
				    IrtRType *RNewOrder)
{
    IPObjectStruct *MVObj;
    MvarMVStruct *RMV,
	*MV = PObjMV -> U.MultiVars;
    MvarMVDirType
	Dir = IRIT_BOUND((MvarMVDirType) IRIT_REAL_PTR_TO_INT(RDir),
		    0, MV -> Dim - 1);
    int *Orders,
	OldOrder = MV -> Orders[Dir],
	NewOrder = IRIT_REAL_PTR_TO_INT(RNewOrder);

    if (NewOrder <= OldOrder) {
	IRIT_WNDW_PUT_STR("Order to raise to less then current order");
	return NULL;
    }

    Orders = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    IRIT_GEN_COPY(Orders, MV -> Orders, sizeof(int) * MV -> Dim);
    Orders[Dir] = NewOrder;

    RMV = MvarMVDegreeRaiseN(MV, Orders);

    IritFree(Orders);

    MVObj = IPGenMULTIVARObject(RMV);

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Make, in place, the following two curves or surfaces compatible.           M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2:  Two curve or two surfaces to make compatible in order,    M
*		   point type and continuity.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MakeFreeFormCompatible                                                   M
*****************************************************************************/
void MakeFreeFormCompatible(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    if (IP_IS_CRV_OBJ(PObj1) && IP_IS_CRV_OBJ(PObj2)) {
        if (!CagdMakeCrvsCompatible(&PObj1 -> U.Crvs,
				    &PObj2 -> U.Crvs, TRUE, TRUE))
	    IRIT_WNDW_PUT_STR("Failed in making curves compatible.");
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_SRF_OBJ(PObj2)) {
	if (!CagdMakeSrfsCompatible(&PObj1 -> U.Srfs,
				    &PObj2 -> U.Srfs,
				    TRUE, TRUE, TRUE, TRUE))
	    IRIT_WNDW_PUT_STR("Failed in making surfaces compatible.");
    }
    else if (IP_IS_TRIVAR_OBJ(PObj1) && IP_IS_TRIVAR_OBJ(PObj2)) {
	if (!TrivMakeTVsCompatible(&PObj1 -> U.Trivars,
				   &PObj2 -> U.Trivars,
				   TRUE, TRUE, TRUE, TRUE, TRUE, TRUE))
	    IRIT_WNDW_PUT_STR("Failed in making trivariate compatible.");
    }
    else {
	IRIT_WNDW_PUT_STR("Only two crvs/srfs/trivars expected.");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new curve, which is the blend of the given two compatible       M
* curves.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjCrv1, PObjCrv2:  Two curves to blend.                                M
*   Method:              Method of blending:                                 M
*			 0 - simple linear interpolation between PObjCrv?.   M
*			 1/3 - corner cutting morphing technique.	     M
*			       scale to same length all time.		     M
*			       If 3, also filters out tangency test.	     M
*                        2/4 - same as 1 but scale to same bbox all time.    M
*			       If 4, also filters out tangency test.	     M
*                        5 - multires morphing.				     M
*   Blend:               Parameter of blend. Usually between zero and one    M
*			 form methods 0 and 3, distance between adjacent     M
*			 morphed curves from method 1 and 2.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Single blended curve if Method = 0, List of curves  M
*			 forming the entire corner cutting morphing if       M
*			 Method = 1.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoCrvsMorphing                                                          M
*****************************************************************************/
IPObjectStruct *TwoCrvsMorphing(IPObjectStruct *PObjCrv1,
				IPObjectStruct *PObjCrv2,
				IrtRType *Method,
				IrtRType *Blend)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
        *Crv = NULL;

    switch (IRIT_REAL_PTR_TO_INT(Method)) {
	case 0:
	    Crv = SymbTwoCrvsMorphing(PObjCrv1 -> U.Crvs, PObjCrv2 -> U.Crvs,
								       *Blend);
	    break;
	case 1:
	    Crv = SymbTwoCrvsMorphingCornerCut(PObjCrv1 -> U.Crvs,
					       PObjCrv2 -> U.Crvs,
					       *Blend, FALSE, FALSE);
	    break;
	case 2:
	    Crv = SymbTwoCrvsMorphingCornerCut(PObjCrv1 -> U.Crvs,
					       PObjCrv2 -> U.Crvs,
					       *Blend, TRUE, FALSE);
	    break;
	case 3:
	    Crv = SymbTwoCrvsMorphingCornerCut(PObjCrv1 -> U.Crvs,
					       PObjCrv2 -> U.Crvs,
					       *Blend, FALSE, TRUE);
	    break;
	case 4:
	    Crv = SymbTwoCrvsMorphingCornerCut(PObjCrv1 -> U.Crvs,
					       PObjCrv2 -> U.Crvs,
					       *Blend, TRUE, TRUE);
	    break;
	case 5:
	    if (CAGD_IS_BEZIER_CRV(PObjCrv1 -> U.Crvs)) {
		CagdCrvStruct
		    *Crv1 = CagdCnvrtBzr2BspCrv(PObjCrv1 -> U.Crvs),
		    *Crv2 = CagdCnvrtBzr2BspCrv(PObjCrv2 -> U.Crvs);

		Crv = SymbTwoCrvsMorphingMultiRes(Crv1, Crv2, *Blend);
		CagdCrvFree(Crv1);
		CagdCrvFree(Crv2);
	    }
	    else
		Crv = SymbTwoCrvsMorphingMultiRes(PObjCrv1 -> U.Crvs,
						  PObjCrv2 -> U.Crvs,
						  *Blend);
	    break;
	default:
	    IRIT_WNDW_PUT_STR("Wrong curve morphing method.\n");
	    return NULL;
    }

    if (Crv == NULL) {
	IRIT_WNDW_PUT_STR("Curve are incompatible, use FFCOMPAT first.");
	return NULL;
    }

    if (Crv -> Pnext != NULL) {
	int i;
	IPObjectStruct *PObj;

	CrvObj = IPGenLISTObject(NULL);
	for (i = 0; Crv != NULL; Crv = Crv -> Pnext, i++)
	    IPListObjectInsert(CrvObj, i, IPGenCRVObject(Crv));
	IPListObjectInsert(CrvObj, i, NULL);
	for (i = 0; (PObj = IPListObjectGet(CrvObj, i)) != NULL; i++)
	    PObj -> U.Crvs -> Pnext = NULL;
    }
    else
	CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new surface, which is the blend of the given two compatible     M
* surfaces.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSrf1, PObjSrf2:  Two surfaces to blend.                              M
*   Blend:               Parameter of blend. Usually between zero and one.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Blended surface.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoSrfsMorphing                                                          M
*****************************************************************************/
IPObjectStruct *TwoSrfsMorphing(IPObjectStruct *PObjSrf1,
				IPObjectStruct *PObjSrf2,
				IrtRType *Blend)
{
    IPObjectStruct *SrfObj;
    CagdSrfStruct
	*Srf = SymbTwoSrfsMorphing(PObjSrf1 -> U.Srfs, PObjSrf2 -> U.Srfs,
								       *Blend);

    if (Srf == NULL) {
	IRIT_WNDW_PUT_STR("Surfaces are incompatible, use FFCOMPAT first.");
	return NULL;
    }
    SrfObj = IPGenSRFObject(Srf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new trivariate, which is the blend of the given two compatible  M
* trivariates.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjTV1, PObjTV2:  Two trivariates to blend.                             M
*   Blend:             Parameter of blend. Usually between zero and one.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Blended trivariate.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoTVsMorphing                                                           M
*****************************************************************************/
IPObjectStruct *TwoTVsMorphing(IPObjectStruct *PObjTV1,
			       IPObjectStruct *PObjTV2,
			       IrtRType *Blend)
{
    IPObjectStruct *TVObj;
    TrivTVStruct
	*TV = TrivTwoTVsMorphing(PObjTV1 -> U.Trivars, PObjTV2 -> U.Trivars,
								       *Blend);

    if (TV == NULL) {
	IRIT_WNDW_PUT_STR("Trivariate are incompatible, use FFCOMPAT first.");
	return NULL;
    }
    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Gregory freeform into a Bezier freeform.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Gregory geometry to convert to Bezier geometry.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but in Gregory basis.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CnvrtGregoryToBezier                                                     M
*****************************************************************************/
IPObjectStruct *CnvrtGregoryToBezier(IPObjectStruct *PObj)
{
    if (PObj -> ObjType == IP_OBJ_TRISRF) {
	TrngTriangSrfStruct
	    *TriSrf = TrngCnvrtGregory2BzrTriSrf(PObj -> U.TriSrfs);

	return IPGenTRISRFObject(TriSrf);
    }
    else {
        IRIT_FATAL_ERROR("Only triangular surface expected.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bezier freeform into a power freeform.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Bezier geometry to convert to power geometry.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but in power basis.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CnvrtBezierToPower                                                       M
*****************************************************************************/
IPObjectStruct *CnvrtBezierToPower(IPObjectStruct *PObj)
{
    if (PObj -> ObjType == IP_OBJ_SURFACE) {
	CagdSrfStruct
	    *Srf = CagdCnvrtBzr2PwrSrf(PObj -> U.Srfs);

	return IPGenSRFObject(Srf);
    }
    else if (PObj -> ObjType == IP_OBJ_CURVE) {
	CagdCrvStruct
	    *Crv = CagdCnvrtBzr2PwrCrv(PObj -> U.Crvs);

	return IPGenCRVObject(Crv);
    }
    else if (PObj -> ObjType == IP_OBJ_MULTIVAR) {
	MvarMVStruct
	    *MV = MvarCnvrtBzr2PwrMV(PObj -> U.MultiVars);

	return IPGenMULTIVARObject(MV);
    }
    else {
        IRIT_FATAL_ERROR("Only curve/surface/multivar expected.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a power freeform into a Bezier freeform.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Power geometry to convert to Bezier geometry.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but in Bezier form.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CnvrtPowerToBezier                                                       M
*****************************************************************************/
IPObjectStruct *CnvrtPowerToBezier(IPObjectStruct *PObj)
{
    if (PObj -> ObjType == IP_OBJ_SURFACE) {
	CagdSrfStruct
	    *Srf = CagdCnvrtPwr2BzrSrf(PObj -> U.Srfs);

	return IPGenSRFObject(Srf);
    }
    else if (PObj -> ObjType == IP_OBJ_CURVE) {
	CagdCrvStruct
	    *Crv = CagdCnvrtPwr2BzrCrv(PObj -> U.Crvs);

	return IPGenCRVObject(Crv);
    }
    else if (PObj -> ObjType == IP_OBJ_MULTIVAR) {
	MvarMVStruct
	    *MV = MvarCnvrtPwr2BzrMV(PObj -> U.MultiVars);

	return IPGenMULTIVARObject(MV);
    }
    else {
        IRIT_FATAL_ERROR("Only curve/surface/multivar expected.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bezier freeform into a Bspline freeform.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Bezier geometry to convert to Bspline geometry.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Same geometry as PObj but as Bspline.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CnvrtBezierToBspline                                                     M
*****************************************************************************/
IPObjectStruct *CnvrtBezierToBspline(IPObjectStruct *PObj)
{
    if (PObj -> ObjType == IP_OBJ_SURFACE) {
	CagdSrfStruct
	    *Srf = CagdCnvrtBzr2BspSrf(PObj -> U.Srfs);

	return IPGenSRFObject(Srf);
    }
    else if (PObj -> ObjType == IP_OBJ_CURVE) {
	CagdCrvStruct
	    *Crv = CagdCnvrtBzr2BspCrv(PObj -> U.Crvs);

	return IPGenCRVObject(Crv);
    }
    else if (PObj -> ObjType == IP_OBJ_MULTIVAR) {
	MvarMVStruct
	    *MV = MvarCnvrtBzr2BspMV(PObj -> U.MultiVars);

	return IPGenMULTIVARObject(MV);
    }
    else if (PObj -> ObjType == IP_OBJ_TRIVAR) {
	TrivTVStruct
	    *TV = TrivCnvrtBzr2BspTV(PObj -> U.Trivars);

	return IPGenTRIVARObject(TV);
    }
    else if (PObj -> ObjType == IP_OBJ_TRISRF) {
	TrngTriangSrfStruct
	    *TriSrf = TrngCnvrtBzr2BspTriSrf(PObj -> U.TriSrfs);

	return IPGenTRISRFObject(TriSrf);
    }
    else {
        IRIT_FATAL_ERROR("Only multivariate/trivariate/trisrf/surface/curve expected.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Convert a Bspline freeform into list of Bezier freeforms.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A Bspline geometry to convert to a Bezier geometry.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A Bezier geometry representing same geometry as PObj. M
*                                                                            *
* KEYWORDS:                                                                  M
*   CnvrtBsplineToBezier                                                     M
*****************************************************************************/
IPObjectStruct *CnvrtBsplineToBezier(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjList, *PTmp;

    if (PObj -> ObjType == IP_OBJ_SURFACE) {
	CagdSrfStruct *TSrf,
	    *Srf = CagdCnvrtBsp2BzrSrf(PObj -> U.Srfs);

	if (Srf -> Pnext == NULL) {
	    PTmp = IPGenSRFObject(Srf);
	    IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);
	    return PTmp;
	}
	else {
	    int i;

	    PObjList = IPGenLISTObject(NULL);
	    PObjList -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

	    for (i = 0; Srf != NULL; i++) {
	        IPListObjectInsert(PObjList, i, PTmp = IPGenSRFObject(Srf));
		PTmp -> Attr = IP_ATTR_COPY_ATTRS(Srf -> Attr);

		TSrf = Srf -> Pnext;
		Srf -> Pnext = NULL;
		Srf = TSrf;
	    }
	    IPListObjectInsert(PObjList, i, NULL);

	    return PObjList;
	}				   
    }
    else if (PObj -> ObjType == IP_OBJ_CURVE) {
	CagdCrvStruct *TCrv,
	    *Crv = CagdCnvrtBsp2BzrCrv(PObj -> U.Crvs);

	if (Crv -> Pnext == NULL) {
	    PTmp = IPGenCRVObject(Crv);
	    IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);
	    return PTmp;
	}
	else {
	    int i;

	    PObjList = IPGenLISTObject(NULL);

	    for (i = 0; Crv != NULL; i++) {
	        IPListObjectInsert(PObjList, i, PTmp = IPGenCRVObject(Crv));
		IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);

		TCrv = Crv -> Pnext;
		Crv -> Pnext = NULL;
		Crv = TCrv;
	    }
	    IPListObjectInsert(PObjList, i, NULL);

	    return PObjList;
	}				   
    }
    else if (PObj -> ObjType == IP_OBJ_MULTIVAR) {
	MvarMVStruct *TMV,
	    *MV = MvarCnvrtBsp2BzrMV(PObj -> U.MultiVars);

	if (MV -> Pnext == NULL) {
	    PTmp = IPGenMULTIVARObject(MV);
	    IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);
	    return PTmp;
	}
	else {
	    int i;

	    PObjList = IPGenLISTObject(NULL);

	    for (i = 0; MV != NULL; i++) {
	        IPListObjectInsert(PObjList, i,
				   PTmp = IPGenMULTIVARObject(MV));
		IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);

		TMV = MV -> Pnext;
		MV -> Pnext = NULL;
		MV = TMV;
	    }
	    IPListObjectInsert(PObjList, i, NULL);

	    return PObjList;
	}				   
    }
    else if (PObj -> ObjType == IP_OBJ_TRIVAR) {
	TrivTVStruct *TTV,
	    *TV = TrivCnvrtBsp2BzrTV(PObj -> U.Trivars);

	if (TV -> Pnext == NULL) {
	    PTmp = IPGenTRIVARObject(TV);
	    IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);
	    return PTmp;
	}
	else {
	    int i;

	    PObjList = IPGenLISTObject(NULL);

	    for (i = 0; TV != NULL; i++) {
	        IPListObjectInsert(PObjList, i, PTmp = IPGenTRIVARObject(TV));
		IP_ATTR_SAFECOPY_ATTRS(PTmp -> Attr, PObj -> Attr);

		TTV = TV -> Pnext;
		TV -> Pnext = NULL;
		TV = TTV;
	    }
	    IPListObjectInsert(PObjList, i, NULL);

	    return PObjList;
	}				   
    }
    else {
        IRIT_FATAL_ERROR("Only multivariate/trivar/surface/curve expected.");
        return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new curve/surface that is the result of the product of the      M
* given two curves or surfaces.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2: Two curves or surfaces to multiply.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Product result.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoFreeformsProduct                                                      M
*****************************************************************************/
IPObjectStruct *TwoFreeformsProduct(IPObjectStruct *PObj1,
				    IPObjectStruct *PObj2)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_MVAR_OBJ(PObj1) && IP_IS_MVAR_OBJ(PObj2)) {
	MvarMVStruct
	    *MV = MvarMVMult(PObj1 -> U.MultiVars, PObj2 -> U.MultiVars);

	Obj = MV ? IPGenMULTIVARObject(MV) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_SRF_OBJ(PObj2)) {
	CagdSrfStruct
	    *Srf = SymbSrfMult(PObj1 -> U.Srfs, PObj2 -> U.Srfs);

	Obj = Srf ? IPGenSRFObject(Srf) : NULL;
    }
    else if (IP_IS_CRV_OBJ(PObj1) && IP_IS_CRV_OBJ(PObj2)) {
	CagdCrvStruct
	    *Crv = SymbCrvMult(PObj1 -> U.Crvs, PObj2 -> U.Crvs);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_NON_FATAL_ERROR("Pair of curves or pair of surfaces expected.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new scalar curve/surface that is the result of the dot product  M
* of the given two curves or surfaces.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2: Two curves or surfaces to multiply.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Product result.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoFreeformsDotProduct                                                   M
*****************************************************************************/
IPObjectStruct *TwoFreeformsDotProduct(IPObjectStruct *PObj1,
				       IPObjectStruct *PObj2)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_VEC_OBJ(PObj1)) {
        IRIT_SWAP(IPObjectStruct *, PObj1, PObj2);
    }

    if (IP_IS_MVAR_OBJ(PObj1) && IP_IS_MVAR_OBJ(PObj2)) {
	MvarMVStruct
	    *MV = MvarMVDotProd(PObj1 -> U.MultiVars, PObj2 -> U.MultiVars);

	Obj = MV ? IPGenMULTIVARObject(MV) : NULL;
    }
    else if (IP_IS_MVAR_OBJ(PObj1) && IP_IS_VEC_OBJ(PObj2)) {
	MvarMVStruct
	    *MV = MvarMVVecDotProd(PObj1 -> U.MultiVars, PObj2 -> U.Vec);

	Obj = MV ? IPGenMULTIVARObject(MV) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_SRF_OBJ(PObj2)) {
	CagdSrfStruct
	    *Srf = SymbSrfDotProd(PObj1 -> U.Srfs, PObj2 -> U.Srfs);

	Obj = Srf ? IPGenSRFObject(Srf) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_VEC_OBJ(PObj2)) {
	CagdSrfStruct
	    *Srf = SymbSrfVecDotProd(PObj1 -> U.Srfs, PObj2 -> U.Vec);

	Obj = Srf ? IPGenSRFObject(Srf) : NULL;
    }
    else if (IP_IS_CRV_OBJ(PObj1) && IP_IS_CRV_OBJ(PObj2)) {
	CagdCrvStruct
	    *Crv = SymbCrvDotProd(PObj1 -> U.Crvs, PObj2 -> U.Crvs);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else if (IP_IS_CRV_OBJ(PObj1) && IP_IS_VEC_OBJ(PObj2)) {
	CagdCrvStruct
	    *Crv = SymbCrvVecDotProd(PObj1 -> U.Crvs, PObj2 -> U.Vec);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_NON_FATAL_ERROR("Pair of curves/surfaces/multivars (or one vector) expected.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new curve/surface that is the result of the cross product       M
* of the given two curves or surfaces.           	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2: Two curves or surfaces to multiply.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Product result.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoFreeformsCrossProduct                                                 M
*****************************************************************************/
IPObjectStruct *TwoFreeformsCrossProduct(IPObjectStruct *PObj1,
					 IPObjectStruct *PObj2)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_MVAR_OBJ(PObj1) && IP_IS_MVAR_OBJ(PObj2)) {
	MvarMVStruct
	    *MV = MvarMVCrossProd(PObj1 -> U.MultiVars, PObj2 -> U.MultiVars);

	Obj = MV ? IPGenMULTIVARObject(MV) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_SRF_OBJ(PObj2)) {
	CagdSrfStruct
	    *Srf = SymbSrfCrossProd(PObj1 -> U.Srfs, PObj2 -> U.Srfs);

	Obj = Srf ? IPGenSRFObject(Srf) : NULL;
    }
    else if (IP_IS_CRV_OBJ(PObj1) && IP_IS_CRV_OBJ(PObj2)) {
	CagdCrvStruct
	    *Crv = SymbCrvCrossProd(PObj1 -> U.Crvs, PObj2 -> U.Crvs);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_NON_FATAL_ERROR("Pair of curves or pair of surfaces expected.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the inner products of two Bspline basis functions.  The function  M
* space is defined every time PCrv is given (=is a curve object), with a     M
* possibility of overriding the orders with the other two parameters.	     M
*   if PCrv is not a curve object, the other two parameters are assumed to   M
* be the indices of the two basis functions to compute the inner prodcut for,M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:    The curve to define the function space with.                    M
*   RInt1, RInt2:  If PCrv is a curve the the orders of the products which   M
*		   is typically same as PCrv's order with RInt1 <= RInt2.    M
*		   Otherwise, indices of the two basis functions' product.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Inner prodict of basis function's result.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoBasisInnerProduct	                                             M
*****************************************************************************/
IPObjectStruct *TwoBasisInnerProduct(IPObjectStruct *PCrv,
				     IrtRType *RInt1,
				     IrtRType *RInt2)
{
    CagdRType R;

    if (IP_IS_CRV_OBJ(PCrv)) {
        SymbBspBasisInnerProdPrep(PCrv -> U.Crvs -> KnotVector,
				  PCrv -> U.Crvs -> Order +
				      PCrv -> U.Crvs -> Length,
				  IRIT_REAL_PTR_TO_INT(RInt1),
				  IRIT_REAL_PTR_TO_INT(RInt2));
	R = 0.0;
    }
    else {
	R = SymbBspBasisInnerProd(IRIT_REAL_PTR_TO_INT(RInt1),
				  IRIT_REAL_PTR_TO_INT(RInt2));
    }

    return IPGenNUMValObject(R);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new curve/surface that is the result of the sum of the given    M
* two curves or surfaces.              			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2: Two curves or surfaces to add up.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Summation result.                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoFreeformsSum                                                          M
*****************************************************************************/
IPObjectStruct *TwoFreeformsSum(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_MVAR_OBJ(PObj1) && IP_IS_MVAR_OBJ(PObj2)) {
	MvarMVStruct
	    *MV = MvarMVAdd(PObj1 -> U.MultiVars, PObj2 -> U.MultiVars);

	Obj = MV ? IPGenMULTIVARObject(MV) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_SRF_OBJ(PObj2)) {
	CagdSrfStruct
	    *Srf = SymbSrfAdd(PObj1 -> U.Srfs, PObj2 -> U.Srfs);

	Obj = Srf ? IPGenSRFObject(Srf) : NULL;
    }
    else if (IP_IS_CRV_OBJ(PObj1) && IP_IS_CRV_OBJ(PObj2)) {
	CagdCrvStruct
	    *Crv = SymbCrvAdd(PObj1 -> U.Crvs, PObj2 -> U.Crvs);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_NON_FATAL_ERROR("Pair of curves or pair of surfaces expected.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a new curve/surface that is the result of the difference of the   M
* given two curves or surfaces.              			             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2: Two curves or surfaces to subtract.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Subtraction result.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TwoFreeformsDiff                                                         M
*****************************************************************************/
IPObjectStruct *TwoFreeformsDiff(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_MVAR_OBJ(PObj1) && IP_IS_MVAR_OBJ(PObj2)) {
	MvarMVStruct
	    *MV = MvarMVSub(PObj1 -> U.MultiVars, PObj2 -> U.MultiVars);

	Obj = MV ? IPGenMULTIVARObject(MV) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1) && IP_IS_SRF_OBJ(PObj2)) {
	CagdSrfStruct
	    *Srf = SymbSrfSub(PObj1 -> U.Srfs, PObj2 -> U.Srfs);

	Obj = Srf ? IPGenSRFObject(Srf) : NULL;
    }
    else if (IP_IS_CRV_OBJ(PObj1) && IP_IS_CRV_OBJ(PObj2)) {
	CagdCrvStruct
	    *Crv = SymbCrvSub(PObj1 -> U.Crvs, PObj2 -> U.Crvs);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_NON_FATAL_ERROR("Pair of curves or pair of surfaces expected.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compute the zero points of a curve.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Curve to compute inflection points for.                       M
*   Eps:       Accuracy of computation.                                      M
*   Axis:      Of search for zeros.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The zero set of curve PObj in axis Axis.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvZeros		                                                     M
*****************************************************************************/
IPObjectStruct *CrvZeros(IPObjectStruct *PObj, IrtRType *Eps, IrtRType *Axis)
{
    int i;
    CagdPtStruct *ZPtsTmp,
	*ZPts = SymbCrvZeroSet(PObj -> U.Crvs, (int) *Axis, *Eps, FALSE);
    IPObjectStruct
	*PObjList = IPGenLISTObject(NULL);

    for (ZPtsTmp = ZPts, i = 0;
	 ZPtsTmp != NULL;
	 ZPtsTmp = ZPtsTmp -> Pnext, i++) {
	IPListObjectInsert(PObjList, i, IPGenNUMValObject(ZPtsTmp -> Pt[0]));
    }

    CagdPtFreeList(ZPts);

    IPListObjectInsert(PObjList, i, NULL);

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the extreme points of a curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Curve to compute extremum locations for.                       M
*   Eps:      Accuracy control.                                              M
*   Axis:     Of search for extremum points.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The extremum set of curve PObj in axis Axis.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvExtremes                                                              M
*****************************************************************************/
IPObjectStruct *CrvExtremes(IPObjectStruct *PObj,
			    IrtRType *Eps,
			    IrtRType *Axis)
{
    int i;
    CagdPtStruct *ExPtsTmp,
	*ExPts = SymbCrvExtremSet(PObj -> U.Crvs, (int) *Axis, *Eps, FALSE);
    IPObjectStruct
	*PObjList = IPGenLISTObject(NULL);

    for (ExPtsTmp = ExPts, i = 0;
	 ExPtsTmp != NULL;
	 ExPtsTmp = ExPtsTmp -> Pnext, i++) {
	IPListObjectInsert(PObjList, i, IPGenNUMValObject(ExPtsTmp -> Pt[0]));
    }

    CagdPtFreeList(ExPts);

    IPListObjectInsert(PObjList, i, NULL);

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the intersection points of two curves.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2:  Two curves to intersect.                                  M
*   Eps:           Accuracy control.                                         M
*   SelfInter:     Do we need to handle self intersection tests.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of intersection locations. If SelfInter is TRUE M
*                       assume PObj1 and PObj2 are the same and search for   M
*			self intersections.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CrvCrvInterArrangment, SymbSrfDistCrvCrv, CagdCrvCrvInter,               M
*   SymbCrvCrvInter	                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCrvInter                                                              M
*****************************************************************************/
IPObjectStruct *CrvCrvInter(IPObjectStruct *PObj1,
			    IPObjectStruct *PObj2,
			    IrtRType *Eps,
			    IrtRType *SelfInter)
{
    IPObjectStruct *NewPObj;

    if (*Eps <= 0.0) {
	CagdSrfStruct
	    *DistSrf = SymbSrfDistCrvCrv(PObj1 -> U.Crvs, PObj2 -> U.Crvs, 1);

	CagdSrfStruct *DistSrf3D;
	CagdRType UMin, UMax, VMin, VMax;

	CagdSrfDomain(DistSrf, &UMin, &UMax, &VMin, &VMax);
	DistSrf3D = SymbPrmtSclrSrfTo3D(DistSrf, UMin, UMax, VMin, VMax);
	CagdSrfFree(DistSrf);

	NewPObj = IPGenSRFObject(DistSrf3D);
    }
    else {
	int i;
	CagdPtStruct *IPtsTmp, *IPts;

	if (IRIT_APX_EQ(*SelfInter, 0.0))
	    IPts = CagdCrvCrvInter(PObj1 -> U.Crvs, PObj2 -> U.Crvs, *Eps);
	else
	    IPts = SymbCrvCrvInter(PObj1 -> U.Crvs, PObj2 -> U.Crvs, *Eps,
				   TRUE);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i, IPGenPTObject(&IPtsTmp -> Pt[0],
							 &IPtsTmp -> Pt[1],
							 &IPtsTmp -> Pt[2]));
	}

	CagdPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the intersections/lower envelops of arrangments of planar curves. M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       The curves to process.                                       M
*   Eps:        Accuracy control.                                            M
*   Operation:  Operation to perform on the arrangment:                      M
*		1. Compute all CCI locations and keep as "InterPts" attribs. M
8		2. Compute all CCI locations and split curves there.	     M
*		3. Compute the Y-minimum lower envelop of the arrangement.   M
8	        4. Compute the radial lower envelop of the arrangement       M
*		   around location PtObj.				     M
*   PtObj:      Center point, for radial lower envelop computation (Op. 4).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of curves representing the output set.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CrvCrvInter, CagdCrvCrvInterArrangment, SymbCrvsLowerEnvelop             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCrvInterArrangment                                                    M
*****************************************************************************/
IPObjectStruct *CrvCrvInterArrangment(IPObjectStruct *PObj,
				      IrtRType *Eps,
				      IrtRType *Operation,
				      IPObjectStruct *PtObj)
{
    int i = 0;
    IPObjectStruct *CObj, *CrvListObj;
    CagdCrvStruct *Crv, *OCrvs, *NextCrv,
	*Crvs = NULL;

    while ((CObj = IPListObjectGet(PObj, i++)) != NULL) {
	if (!IP_IS_CRV_OBJ(CObj)) {
	    IRIT_NON_FATAL_ERROR("Non curve object found in list");
	    return NULL;
	}

	Crv = CagdCrvCopy(CObj -> U.Crvs);
	Crvs = CagdListAppend(Crv, Crvs);
    }

    switch (IRIT_REAL_PTR_TO_INT(Operation)) {
	default:
	    IRIT_NON_FATAL_ERROR("CARRANGMNT: invalid operation requested, intersections are computed instead.");
	case 1:    /* Compute intersection parameters as attributes on crvs. */
	    OCrvs = CagdCrvCrvInterArrangment(Crvs, FALSE, *Eps);
	    break;
	case 2:       /* Compute intersection parameters and split the crvs. */
	    OCrvs = CagdCrvCrvInterArrangment(Crvs, TRUE, *Eps);
	    break;
	case 3:                   /* Compute Y-minimum linear lower envelop. */
	    OCrvs = SymbCrvsLowerEnvelop(Crvs, NULL, *Eps);
	    break;
	case 4:                /* Compute radial lower envelop around PtObj. */
	    if (IP_IS_POINT_OBJ(PtObj))
	        OCrvs = SymbCrvsLowerEnvelop(Crvs, PtObj -> U.Pt, *Eps);
	    else {
	        IRIT_NON_FATAL_ERROR("CARRANGMNT: for radial lower envelop, expects a point of origin.");
		return NULL;
	    }
	    break;
    }

    CagdCrvFreeList(Crvs);

    CrvListObj = IPGenLISTObject(NULL);
    for (Crv = OCrvs, i = 0; Crv != NULL; Crv = NextCrv, i++) {
	IPObjectStruct *PCrvObj, *InterPtListObj;
	CagdPtStruct *IPt, *IPts;

        NextCrv = Crv -> Pnext;
	Crv -> Pnext = NULL;
	IPListObjectInsert(CrvListObj, i, PCrvObj = IPGenCRVObject(Crv));

	/* If we computed intersection parameters, copy them as well. */
	if (IRIT_REAL_PTR_TO_INT(Operation) == 1 &&
	    (IPts = (CagdPtStruct *) AttrGetRefPtrAttrib(Crv -> Attr,
							"InterPts")) != NULL) {
	    int l = 0;
	    IPts = CagdPtsSortAxis(IPts, 1);

	    InterPtListObj = IPGenLISTObject(NULL);
	    for (IPt = IPts; IPt != NULL; IPt = IPt -> Pnext)
	        IPListObjectInsert(InterPtListObj, l++,
				   IPGenNUMValObject(IPt -> Pt[0]));
	    IPListObjectInsert(InterPtListObj, l, NULL);
		
	    CagdPtFreeList(IPts);
	    AttrFreeOneAttribute(&Crv -> Attr, "InterPts");
	    AttrSetObjectObjAttrib(PCrvObj, "InterPts", InterPtListObj, FALSE);
	}

    }
    IPListObjectInsert(CrvListObj, i, NULL);

    return CrvListObj;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Fetches one parameter out of params list object and assign into PtrAddr. *
*                                                                            *
* PARAMETERS:                                                                *
*   Params:     List of parameters needed for this specific CA operation.    *
*   Index:      Index into the Params array, to fetch.			     *
*   ObjType:    Type of expected object to verify.			     *
*   ParamAddr:  Where to assign the address of the fetched parameter.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful. FALSE otherwise.                          *
*****************************************************************************/
static int CAVerifyParamType(IPObjectStruct *Params,
			     int Index,
			     IPObjStructType ObjType,
			     void **ParamAddr)
{
    *ParamAddr = NULL;

    if (Index >= 0 && Index < IPListObjectLength(Params)) {
        IPObjectStruct
	    *PTmp = IPListObjectGet(Params, Index);

	switch (ObjType) {
	    case IP_OBJ_NUMERIC:
	        if (IP_IS_NUM_OBJ(PTmp))
		    *((IrtRType **) ParamAddr) = &PTmp -> U.R;
		else
		    IRIT_WNDW_FPRINTF2("Input parameter %d was expected as numeric", Index);
	        break;
	    case IP_OBJ_LIST_OBJ:
	        if (IP_IS_OLST_OBJ(PTmp))
		    *ParamAddr = PTmp;
		else
		    IRIT_WNDW_FPRINTF2("Input parameter %d was expected as a list object", Index);
	        break;
	    default:
	        IRIT_WNDW_FPRINTF2("Input parameter %d is of invalid type", Index);
	        break;
	}
    }

    return *ParamAddr != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Process a set of planar curves and eventually computes its arrangment.   M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrvs:        List of curves to handle.                                  M
*   CAOperation:  Type of curve arranement (CA) operation to apply.          M
*   Params:       List of parameters needed for this specific CA operation.  M
*                 See full description of parameters in UserCrvArngmnt.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of curves after CA processing.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvArngmnt                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCrvInterArrangment2                                                   M
*****************************************************************************/
IPObjectStruct *CrvCrvInterArrangment2(IPObjectStruct *PCrvs,
				       IrtRType *CAOperation,
				       IPObjectStruct *Params)
{
    int InputError = FALSE;
    UserCAOpType
        Operation = IRIT_REAL_PTR_TO_INT(CAOperation);
    void *VParams[5];
    IPObjectStruct
        *PRet = NULL;
    UserCrvArngmntStruct
        *NewCA = NULL,
        *CA = (UserCrvArngmntStruct *) AttrGetObjectRefPtrAttrib(PCrvs,
								 "CrvArng");

    if (CA != NULL) {
        assert(IP_IS_CRV_OBJ(PCrvs));
	CA -> CagdCrvs = PCrvs -> U.Crvs;
    }

    /* Process the params based on the desired operation: */

    switch (Operation) {
	case USER_CA_OPER_CREATE:
	    VParams[0] = PCrvs;
	    if (IPListObjectLength(Params) != 4 ||
		!CAVerifyParamType(Params, 0, IP_OBJ_NUMERIC, &VParams[1]) ||
		!CAVerifyParamType(Params, 1, IP_OBJ_NUMERIC, &VParams[2]) ||
		!CAVerifyParamType(Params, 2, IP_OBJ_NUMERIC, &VParams[3]) ||
		!CAVerifyParamType(Params, 3, IP_OBJ_NUMERIC, &VParams[4])) {
	        InputError = TRUE;
		break;
	    }

	    if ((NewCA = UserCrvArngmnt(Operation, NULL,
					(const void **) VParams)) == NULL) {
	        IRIT_WNDW_PUT_STR("Input is invalid or not plannar");
	    }
	    break;
	case USER_CA_OPER_COPY:
	    if (CA != NULL)
		NewCA = UserCrvArngmnt(Operation, CA, NULL);
	    break;
	case USER_CA_OPER_FILTER_DUP:
	    if (CA != NULL) {
	        if (IPListObjectLength(Params) != 2 ||
		    !CAVerifyParamType(Params, 0, IP_OBJ_NUMERIC, &VParams[0]) ||
		    !CAVerifyParamType(Params, 1, IP_OBJ_NUMERIC, &VParams[1])) {
		    InputError = TRUE;
		    break;
		}

		NewCA = UserCrvArngmnt(Operation, CA, (const void **) VParams);
	    }
	    break;
	case USER_CA_OPER_SPLIT_CRV:
	    if (CA != NULL) {
	        if (IPListObjectLength(Params) != 2 ||
		    !CAVerifyParamType(Params, 0, IP_OBJ_NUMERIC, &VParams[0]) ||
		    !CAVerifyParamType(Params, 1, IP_OBJ_NUMERIC, &VParams[1])) {
		    InputError = TRUE;
		    break;
		}

		NewCA = UserCrvArngmnt(Operation, CA, (const void **) VParams);
	    }
	    break;
	case USER_CA_OPER_BREAK_INTER:
	case USER_CA_OPER_BREAK_LIN:
	case USER_CA_OPER_FILTER_TAN:
	case USER_CA_OPER_UNION_CRV:
	    if (CA != NULL) {
	        if (IPListObjectLength(Params) != 1 ||
		    !CAVerifyParamType(Params, 0, IP_OBJ_NUMERIC, &VParams[0])) {
		    InputError = TRUE;
		    break;
		}

		NewCA = UserCrvArngmnt(Operation, CA, (const void **) VParams);
	    }
	    break;
	case USER_CA_OPER_BREAK_NEAR_PTS:
	    if (CA != NULL) {
	        if (IPListObjectLength(Params) != 2 ||
		    !CAVerifyParamType(Params, 0, IP_OBJ_LIST_OBJ, &VParams[0]) ||
		    !CAVerifyParamType(Params, 1, IP_OBJ_NUMERIC, &VParams[1])) {
		    InputError = TRUE;
		    break;
		}

		NewCA = UserCrvArngmnt(Operation, CA, (const void **) VParams);
	    }
	    break;
	case USER_CA_OPER_LSTSQR_CRV:
	case USER_CA_OPER_EVAL_CA:
	case USER_CA_OPER_REPORT:
	    if (CA != NULL) {
	        if (IPListObjectLength(Params) != 1 ||
		    !CAVerifyParamType(Params, 0, IP_OBJ_NUMERIC, &VParams[0])) {
		    InputError = TRUE;
		    break;
		}

		NewCA = UserCrvArngmnt(Operation, CA, (const void **) VParams);
	    }
	    break;
	case USER_CA_OPER_CLASSIFY:
	    if (CA != NULL)
		UserCrvArngmnt(Operation, CA, NULL);
	    break;
	case USER_CA_OPER_OUTPUT:
	    if (CA != NULL) {
	        if (IPListObjectLength(Params) != 3 ||
		    !CAVerifyParamType(Params, 0, IP_OBJ_NUMERIC, &VParams[0]) ||
		    !CAVerifyParamType(Params, 1, IP_OBJ_NUMERIC, &VParams[1]) ||
		    !CAVerifyParamType(Params, 2, IP_OBJ_NUMERIC, &VParams[2])) {
		    InputError = TRUE;
		    break;
		}

		NewCA = UserCrvArngmnt(Operation, CA, (const void **) VParams);
	    }
	    break;
	case USER_CA_OPER_FREE:
	    if (CA != NULL) {
		CA -> CagdCrvs = NULL;              /* Detach PCrvs from CA. */
		AttrFreeObjectAttribute(PCrvs, "CrvArng");

		UserCrvArngmnt(Operation, CA, (const void **) VParams);
		CA = NULL;
	    }
	    break;
	case USER_CA_OPER_NONE:
        default:
	    break;
    }

    if (CA != NULL) {
	CA -> CagdCrvs = NULL;
    }

    if (NewCA != NULL) {
        if (NewCA -> Error != NULL)
            IRIT_WNDW_PUT_STR(NewCA -> Error);

	if (Operation == USER_CA_OPER_OUTPUT) {
	    PRet = NewCA -> Output;
	    NewCA -> Output = NULL;
	    UserCrvArngmntFree(NewCA);
	}
	else {
	    PRet = IPGenCRVObject(NewCA -> CagdCrvs);
	    NewCA -> CagdCrvs = NULL;
	    AttrSetObjectRefPtrAttrib(PRet, "CrvArng", NewCA);
	}

	return PRet;
    }
    else if (InputError) {
        return IPGenSTRObject("CA error in curve arrangement input/computation");
    }
    else
        return IPGenSTRObject("CA op complete");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the intersection points of two surfaces.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2:  Two curves to intersect.                                  M
*   Euclidean:     TRUE for intersection location in Euclidean space, FALSE  M
*		   for intersection location in curves' parameter space.     M
*   Eps:           Accuracy control.                                         M
*   AlignSrfs:     Do we need to align the two surfaces, optimizing the      M
*		   computation time?					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of intersection locations. If SelfInter is TRUE M
*                       assume PObj1 and PObj2 are the same and search for   M
*			self intersections.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfSrfInter                                                              M
*****************************************************************************/
IPObjectStruct *SrfSrfInter(IPObjectStruct *PObj1,
			    IPObjectStruct *PObj2,
			    IrtRType *Euclidean,
			    IrtRType *Eps,
			    IrtRType *AlignSrfs)
{
    int i;
    CagdCrvStruct *Crv, *Crvs1, *Crvs2,
	*NextCrv = NULL;

    if (UserSrfSrfInter(PObj1 -> U.Srfs, PObj2 -> U.Srfs,
			IRIT_REAL_PTR_TO_INT(Euclidean), *Eps,
			IRIT_REAL_PTR_TO_INT(AlignSrfs), &Crvs1, &Crvs2)) {
	IPObjectStruct
	    *NewPCrvs1Obj = IPGenLISTObject(NULL),
	    *NewPCrvs2Obj = IPGenLISTObject(NULL),
	    *NewPObj = IPGenLISTObject(NULL);

	for (Crv = Crvs1, i = 0; Crv != NULL; Crv = NextCrv, i++) {
	    NextCrv = Crv -> Pnext;
	    Crv -> Pnext = NULL;
	    IPListObjectInsert(NewPCrvs1Obj, i, IPGenCRVObject(Crv));
	}
	IPListObjectInsert(NewPCrvs1Obj, i, NULL);

	for (Crv = Crvs2, i = 0; Crv != NULL; Crv = NextCrv, i++) {
	    NextCrv = Crv -> Pnext;
	    Crv -> Pnext = NULL;
	    IPListObjectInsert(NewPCrvs2Obj, i, IPGenCRVObject(Crv));
	}
	IPListObjectInsert(NewPCrvs2Obj, i, NULL);

	IPListObjectInsert(NewPObj, 0, NewPCrvs1Obj);
	IPListObjectInsert(NewPObj, 1, NewPCrvs2Obj);
	IPListObjectInsert(NewPObj, 2, NULL);

	return NewPObj;
    }
    else
	return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the intersections of different varieties.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PGeom:  	 A list of different varieties to.	                     M
*   Euclidean:   TRUE for intersection location in Euclidean space, FALSE    M
*		 for intersection location in parameter space.		     M
*   SubdivTol:   Subdivision tolerance of computation.			     M
*   NumerTol:    Numerical tolerance of computation.  See multivariate zeros M
*	         solver for more on the SudivbTol and NumerTol.		     M
*   UseExprTree: TRUE to use multivariates' expresion trees, FALSE to use    M
*		 regular multivariates.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of intersection locations. If SelfInter is TRUE M
*                       assume PObj1 and PObj2 are the same and search for   M
*			self intersections.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultiVarsInter                                                           M
*****************************************************************************/
IPObjectStruct *MultiVarsInter(IPObjectStruct *PGeom,
			       IrtRType *SubdivTol,
			       IrtRType *NumerTol,
			       IrtRType *UseExprTree)
{
    IPObjectStruct *NewPObj,
	*PGeom1 = IPListObjectGet(PGeom, 0),
	*PGeom2 = IPListObjectGet(PGeom, 1);
    MvarPtStruct *MVPts;

    if (PGeom1 != NULL && IP_IS_CRV_OBJ(PGeom1)) {
        /* Expects 2 curves in R^2. */
        if (PGeom2 != NULL && IP_IS_CRV_OBJ(PGeom2) &&
	    CAGD_NUM_OF_PT_COORD(PGeom1 -> U.Crvs -> PType) >= 2 &&
	    CAGD_NUM_OF_PT_COORD(PGeom2 -> U.Crvs -> PType) >= 2)
	    MVPts = MvarCrvCrvInter(PGeom1 -> U.Crvs, PGeom2 -> U.Crvs,
				    *SubdivTol, *NumerTol,
				    IRIT_REAL_PTR_TO_INT(UseExprTree));
	else {
	    IRIT_NON_FATAL_ERROR("MVInter: Incompatible geometry.");
	    return NULL;
	}
    }
    else if (PGeom1 != NULL && IP_IS_SRF_OBJ(PGeom1)) {
	IPObjectStruct
	    *PGeom3 = IPListObjectGet(PGeom, 2);

	if (PGeom3 == NULL) {
	    /* Expects 2 surfaces in R^3. */
	    if (PGeom2 != NULL &&
		IP_IS_SRF_OBJ(PGeom2) &&
		CAGD_NUM_OF_PT_COORD(PGeom1 -> U.Srfs -> PType) >= 3 &&
		CAGD_NUM_OF_PT_COORD(PGeom2 -> U.Srfs -> PType) >= 3)
	        MVPts = MvarSrfSrfInter(PGeom1 -> U.Srfs, PGeom2 -> U.Srfs,
				       *SubdivTol, *NumerTol,
				       IRIT_REAL_PTR_TO_INT(UseExprTree));
	    else {
	        IRIT_NON_FATAL_ERROR("MVInter: Incompatible geometry.");
		return NULL;
	    }
	}
	else {
	    /* Expects 3 surfaces in R^3. */
	    if (PGeom2 != NULL &&
		IP_IS_SRF_OBJ(PGeom2) &&
		PGeom3 != NULL && 
		IP_IS_SRF_OBJ(PGeom3) &&
		CAGD_NUM_OF_PT_COORD(PGeom1 -> U.Srfs -> PType) >= 3 &&
		CAGD_NUM_OF_PT_COORD(PGeom2 -> U.Srfs -> PType) >= 3 &&
		CAGD_NUM_OF_PT_COORD(PGeom3 -> U.Srfs -> PType) >= 3)
	        MVPts = MvarSrfSrfSrfInter(PGeom1 -> U.Srfs, PGeom2 -> U.Srfs,
				       PGeom3 -> U.Srfs, *SubdivTol, *NumerTol,
				       IRIT_REAL_PTR_TO_INT(UseExprTree));
	    else {
	        IRIT_NON_FATAL_ERROR("MVInter: Incompatible geometry.");
		return NULL;
	    }
	}
    }
    else {
        IRIT_NON_FATAL_ERROR("MVInter: Expecting curves and surfaces.");
	return NULL;
    }

    NewPObj = MvarCnvrtMVPtsToCtlPts(MVPts, -1);

    MvarPtFreeList(MVPts);

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the intersections of different varieties.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PGeom1, PGeom2:  The two objects to examine for possible contacts.	     M
*   MotionCrvs:  A list of motion curves.  Currently only "MOV_XYZ", or      M
*		 "SCL" are supported.					     M
*   SubdivTol:   Subdivision tolerance of computation.			     M
*   NumerTol:    Numerical tolerance of computation.  See multivariate zeros M
*	         solver for more on the SudivbTol and NumerTol.		     M
*   UseExprTree: TRUE to use multivariates' expresion trees, FALSE to use    M
*		 regular multivariates.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   List of intersection locations. If SelfInter is TRUE M
*                       assume PObj1 and PObj2 are the same and search for   M
*			self intersections.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MultiVarsContact                                                         M
*****************************************************************************/
IPObjectStruct *MultiVarsContact(IPObjectStruct *PGeom1,
				 IPObjectStruct *PGeom2,
				 IPObjectStruct *MotionCrvs,
				 IrtRType *SubdivTol,
				 IrtRType *NumerTol,
				 IrtRType *UseExprTree)
{
    MvarPtStruct *MVPts;

    if (IP_IS_SRF_OBJ(PGeom1) && IP_IS_SRF_OBJ(PGeom2)) {
        int i;
	IPObjectStruct *PObj,
	    *PTrans = NULL,
	    *PScale = NULL;

	for (i = 0; (PObj = IPListObjectGet(MotionCrvs, i++)) != NULL; ) {
	    if (PObj != NULL &&	IP_IS_CRV_OBJ(PObj)) {
	        if (stricmp(PObj -> ObjName, "MOV_XYZ") == 0)
		    PTrans = PObj;
		else if (stricmp(PObj -> ObjName, "scl") == 0)
		    PScale = PObj;
	    }
	}

	if (PTrans == NULL && PScale == NULL) {
	    IRIT_NON_FATAL_ERROR("No motion curves detected.");
	    return NULL;
	}

	MVPts = MvarSrfSrfContact(PGeom1 -> U.Srfs, PGeom2 -> U.Srfs,
				  PTrans == NULL ? NULL : PTrans -> U.Crvs,
				  PScale == NULL ? NULL : PScale -> U.Crvs,
				  *SubdivTol, *NumerTol,
				  IRIT_REAL_PTR_TO_INT(UseExprTree));
	
	PObj = MvarCnvrtMVPtsToCtlPts(MVPts, -1);

	MvarPtFreeList(MVPts);

	return PObj;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the closest/farthest/distance square field from a curve to a      M
* given point.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:     To consider its distace to Points.                             M
*   Point:    To consider its distance to PCrv.                              M
*   MinDist:  Do we need minimal distance (TRUE) or maximal distance (FALSE).M
*   Eps:      If Eps > 0, computes the requested extremum location.          M
*	      Otherwise, computes the distance square scalar field.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Either extremum location or distance square field.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvPointDist                                                             M
*****************************************************************************/
IPObjectStruct *CrvPointDist(IPObjectStruct *PCrv,
			     IrtPtType Point,
			     IrtRType *MinDist,
			     IrtRType *Eps)
{
    IPObjectStruct *NewPObj;

    if (*Eps > 0.0) {
	NewPObj = IPGenNUMValObject(SymbDistCrvPoint(PCrv -> U.Crvs,
						     Point,
						     IRIT_REAL_PTR_TO_INT(MinDist),
						     *Eps));
    }
    else {
	int i;
	CagdPtStruct *IPtsTmp,
	    *IPts = SymbLclDistCrvPoint(PCrv -> U.Crvs, Point, -*Eps);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i,
			       IPGenNUMValObject(IPtsTmp -> Pt[0]));
	}

	CagdPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the closest/farthest/distance square field from a curve to a      M
* given line.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PCrv:     To consider its distace to Line.                               M
*   Point, Vec: Defining the line to consider its distance to PCrv.          M
*   MinDist:  Do we need minimal distance (TRUE) or maximal distance (FALSE).M
*   Eps:      If Eps > 0, computes the requested extremum location.          M
*	      Otherwise, computes the distance square scalar field.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Either extremum location or distance square field.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvLineDist                                                              M
*****************************************************************************/
IPObjectStruct *CrvLineDist(IPObjectStruct *PCrv,
			    IrtPtType Point,
			    IrtVecType Vec,
			    IrtRType *MinDist,
			    IrtRType *Eps)
{
    IPObjectStruct *NewPObj;
    IrtLnType Line;

    Line[0] = Vec[1];
    Line[1] = -Vec[0];
    Line[2] = -(Line[0] * Point[0] + Line[1] * Point[1]);

    if (*Eps > 0.0) {
	NewPObj = IPGenNUMValObject(SymbDistCrvLine(PCrv -> U.Crvs,
						    Line,
						    IRIT_REAL_PTR_TO_INT(MinDist),
						    *Eps));
    }
    else {
	int i;
	CagdPtStruct *IPtsTmp,
	    *IPts = SymbLclDistCrvLine(PCrv -> U.Crvs, Line, -*Eps,
				       TRUE, TRUE);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i,
			       IPGenNUMValObject(IPtsTmp -> Pt[0]));
	}

	CagdPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the closest/farthest/distance square field from a surface to a    M
* given point.  							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:      To consider its distace to Points.                            M
*   Point:     To consider its distance to PSrf.                             M
*   MinDist:   Do we need minimal distance (TRUE) or maximal distance	     M
*	       (FALSE).							     M
*   SubdivTol: Subdivision tolerance of computation. If negative, returns    M
*	       all the extrema potential locations.			     M
*   NumerTol:  Numerical tolerance of computation.  See multivariate zeros   M
*	       solver for more on the SubTol and NumTol.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Either extremum location or all extrema pts.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfPointDist                                                             M
*****************************************************************************/
IPObjectStruct *SrfPointDist(IPObjectStruct *PSrf,
			     IrtPtType Point,
			     IrtRType *MinDist,
			     IrtRType *SubdivTol,
			     IrtRType *NumerTol)
{
    CagdRType
        ZeroVal = 0.0;
    IPObjectStruct
	*NewPObj = NULL;

    if (*SubdivTol > 0.0) {
        CagdRType
	    *UV = MvarDistSrfPoint(PSrf -> U.Srfs, Point,
				   IRIT_REAL_PTR_TO_INT(MinDist),
				   *SubdivTol, *NumerTol);
	if (UV != NULL)
	    NewPObj = IPGenPTObject(&UV[0], &UV[1], &ZeroVal);
	else
	    IRIT_NON_FATAL_ERROR("No extrema found.");
    }
    else {
	int i;
	MvarPtStruct *IPtsTmp,
	    *IPts = MvarLclDistSrfPoint(PSrf -> U.Srfs, Point,
					-*SubdivTol, *NumerTol);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i,
			       IPGenPTObject(&IPtsTmp -> Pt[0],
					     &IPtsTmp -> Pt[1],
					     &ZeroVal));
	}

	MvarPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the closest/farthest/distance square field from a curve to a      M
* given line.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:      To consider its distace to Line.                              M
*   LnPt, LnDir: Defining the line to consider its distance to PSrf.         M
*   MinDist:   Do we need minimal distance (TRUE) or maximal distance	     M
	       (FALSE).							     M
*   SubdivTol: Subdivision tolerance of computation. If negative, returns    M
*	       all the extrema potential locations.			     M
*   NumerTol:  Numerical tolerance of computation.  See multivariate zeros   M
*	       solver for more on the SubTol and NumTol.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Either extremum location or all extrema pts.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfLineDist                                                              M
*****************************************************************************/
IPObjectStruct *SrfLineDist(IPObjectStruct *PSrf,
			    IrtPtType LnPt,
			    IrtVecType LnDir,
			    IrtRType *MinDist,
			    IrtRType *SubdivTol,
			    IrtRType *NumerTol)
{
    CagdRType
	ZeroVal = 0.0;
    IPObjectStruct
	*NewPObj = NULL;

    if (*SubdivTol > 0.0) {
        CagdRType
	    *UV = MvarDistSrfLine(PSrf -> U.Srfs, LnPt, LnDir,
				  IRIT_REAL_PTR_TO_INT(MinDist),
				  *SubdivTol, *NumerTol);
	if (UV != NULL)
	    NewPObj = IPGenPTObject(&UV[0], &UV[1], &ZeroVal);
	else
	    IRIT_NON_FATAL_ERROR("No extrema found.");
    }
    else {
	int i;
	MvarPtStruct *IPtsTmp,
	    *IPts = MvarLclDistSrfLine(PSrf -> U.Srfs, LnPt, LnDir,
				       *SubdivTol, *NumerTol);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i,
			       IPGenPTObject(&IPtsTmp -> Pt[0],
					     &IPtsTmp -> Pt[1],
					     &ZeroVal));
	}

	MvarPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Composes a curve on a curve or a surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1:    A curve or a surface to compose.                               M
*   PObj2:    A curve in the parametric space of PObj1.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A composed curve of the form PObj1(Pobj2).           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvComposition                                                           M
*****************************************************************************/
IPObjectStruct *CrvComposition(IPObjectStruct *PObj1, IPObjectStruct *PObj2)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_CRV_OBJ(PObj1)) {
	CagdCrvStruct
	    *Crv = SymbComposePeriodicCrvCrv(PObj1 -> U.Crvs,
					     PObj2 -> U.Crvs,
					     IRIT_EPS);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else if (IP_IS_SRF_OBJ(PObj1)) {
	CagdCrvStruct
	    *Crv = SymbComposePeriodicSrfCrv(PObj1 -> U.Srfs,
					     PObj2 -> U.Crvs,
					     IRIT_EPS);

	Obj = Crv ? IPGenCRVObject(Crv) : NULL;
    }
    else {
	IRIT_FATAL_ERROR("Curve or surface as first parameter only.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Decomposes a curve in its two composition componenet, if possible.         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A curve to try and decompose.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A pair of curves (Crv1, Crv2) in a list object such  M
*		that Crv1( Crv2 ) equals the original inputcurve, or an      M
*		empty list if no decomposition was found.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvDecomposition                                                         M
*****************************************************************************/
IPObjectStruct *CrvDecomposition(IPObjectStruct *PObj)
{
    IPObjectStruct
	*Obj = NULL;

    if (IP_IS_CRV_OBJ(PObj)) {
	CagdCrvStruct *Crv,
	    *Crvs = SymbDecomposeCrvCrv(PObj -> U.Crvs);

	if (Crvs == NULL)
	    return IPGenLISTObject(NULL);
	else {
	    int i;
	    IPObjectStruct
		*RetVal = IPGenLISTObject(NULL);

	    for (Crv = Crvs, i = 0; Crv != NULL; Crv = Crv -> Pnext, i++)
	        IPListObjectInsert(RetVal, i, IPGenCRVObject(Crv));
	    IPListObjectInsert(RetVal, i, NULL);
	    for (i = 0; (PObj = IPListObjectGet(RetVal, i)) != NULL; i++)
	         PObj -> U.Crvs -> Pnext = NULL;

	    return RetVal;	  
	}
	
    }
    else {
	IRIT_FATAL_ERROR("Curve as parameter only.");
    }

    return Obj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a layout (prisa) of the given set of surfaces.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:             (Trimmed) surface to layout flat.                      M
*   SamplesPerCurve:  Number of sample for piecewise linear curve	     M
*		      approximation.					     M
*   Epsilon:          Accuracy control.                                      M
*   Dir:              Direction of subdivision. Either U or V.               M
*   Space:            Spacing between the laid out pieces.                   M
*   RDoCrossSecs:     Should we do cross sections!?			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A list of flat 2d surfaces approximating Srfs.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfsPrisa                                                                M
*****************************************************************************/
IPObjectStruct *SrfsPrisa(IPObjectStruct *Srfs,
			  CagdRType *SamplesPerCurve,
			  CagdRType *Epsilon,
			  CagdRType *Dir,
			  CagdVType Space,
			  CagdRType *RDoCrossSecs)
{
    int i,
	DoCrossSecs = IRIT_REAL_PTR_TO_INT(RDoCrossSecs);
    IPObjectStruct *PObjList, *PObj;
    
    /* Break the linear list into a list object. */
    PObjList = IPGenLISTObject(NULL);

    if (DoCrossSecs) {
	CagdCrvStruct *Crv, *CrossSecs;
	CagdSrfStruct *PrisaSrfs;

	if (IP_IS_SRF_OBJ(Srfs))
	    PrisaSrfs = SymbAllPrisaSrfs(Srfs -> U.Srfs,
					 IRIT_REAL_PTR_TO_INT(SamplesPerCurve),
					 -IRIT_FABS(*Epsilon),
					 (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
					 Space);
	else if (IP_IS_TRIMSRF_OBJ(Srfs))
	    PrisaSrfs = SymbAllPrisaSrfs(Srfs -> U.TrimSrfs -> Srf,
					 IRIT_REAL_PTR_TO_INT(SamplesPerCurve),
					 -IRIT_FABS(*Epsilon),
					 (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
					 Space);
	else {
	    IRIT_FATAL_ERROR("(Trimmed) surface as first parameter only.");
	    PrisaSrfs = NULL;
	}

	CrossSecs = SymbPrisaGetCrossSections(PrisaSrfs, CAGD_CONST_U_DIR,
					      Space);
	CagdSrfFreeList(PrisaSrfs);

	for (Crv = CrossSecs, i = 0; Crv != NULL; Crv = Crv -> Pnext, i++)
	    IPListObjectInsert(PObjList, i, IPGenCRVObject(Crv));
	IPListObjectInsert(PObjList, i, NULL);
	for (i = 0; (PObj = IPListObjectGet(PObjList, i)) != NULL; i++)
	    PObj -> U.Crvs -> Pnext = NULL;
    }
    else if (IP_IS_SRF_OBJ(Srfs)) {
	CagdSrfStruct *Srf,
	    *PrisaSrfs = SymbAllPrisaSrfs(Srfs -> U.Srfs,
					 IRIT_REAL_PTR_TO_INT(SamplesPerCurve),
					 *Epsilon,
					 (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
					 Space);

	for (Srf = PrisaSrfs, i = 0; Srf != NULL; Srf = Srf -> Pnext, i++)
	    IPListObjectInsert(PObjList, i, IPGenSRFObject(Srf));
	IPListObjectInsert(PObjList, i, NULL);
	for (i = 0; (PObj = IPListObjectGet(PObjList, i)) != NULL; i++)
	    PObj -> U.Srfs -> Pnext = NULL;
    }
    else if (IP_IS_TRIMSRF_OBJ(Srfs)) {
	TrimSrfStruct *TSrf,
	    *PrisaTSrfs = TrimAllPrisaSrfs(Srfs -> U.TrimSrfs,
					 IRIT_REAL_PTR_TO_INT(SamplesPerCurve),
					 *Epsilon,
					 (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
					 Space);

	for (TSrf = PrisaTSrfs, i = 0; TSrf != NULL; TSrf = TSrf -> Pnext, i++)
	    IPListObjectInsert(PObjList, i, IPGenTRIMSRFObject(TSrf));
	IPListObjectInsert(PObjList, i, NULL);
	for (i = 0; (PObj = IPListObjectGet(PObjList, i)) != NULL; i++)
	    PObj -> U.TrimSrfs -> Pnext = NULL;
    }
    else {
	IRIT_FATAL_ERROR("(Trimmed) surface as first parameter only.");
    }

    return PObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an adaptive isocurve coverage to the given surface.  If GenIsos M
* adapative isocurves coverage is returned.  If !GenIsos, tiling of          M
* rectangles based on the adaptive isocurves is computed instead.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To compute adaptive isocurve coverage for.                  M
*   GenIsos:     TRUE to generate iso curves, FALSE for rectangular tiling.  M
*   Dir:         Direction of adaptive isocurves. Either U or V.             M
*   Eps:         Coverage accuracy.                                          M
*   FullIsoOutputType:    If GenIso - do we want full isocurves or just      M
*                       trimmed ones.			                     M
*                         If !GenIso - desired output type: 0, 1, or 2 for   M
*                       rectangular polys in UV space, in Euclidean space,   M
*                       or surfaces patches. 				     M
*   SinglePathSmoothing:  If GenIso - do we want everything in one long      M
*                       curve? 						     M
*                         If !GenIso - number of smoothing iterations to     M
*                       apply or zero to disable.			     M
*   WeightPtSclWdt: A list object.  If not empty, must specify a list of     M
*                three objects, a weighting point, a scaling factor, and a   M
*		 width factor to make coverage more dense near the           M
*		 designated weighting point.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An adaptive isocurve coverage for Srf.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfAdapIsoCurves                                                         M
*****************************************************************************/
IPObjectStruct *SrfAdapIsoCurves(IPObjectStruct *Srf,
				 CagdRType *GenIsos,
				 CagdRType *Dir,
				 CagdRType *Eps,
				 CagdRType *FullIsoOutputType,
				 CagdRType *SinglePathSmoothing,
				 IPObjectStruct *WeightPtSclWdt)
{
    IPObjectStruct *NewPObj;

    if (WeightPtSclWdt != NULL) {
        if (IP_IS_OLST_OBJ(WeightPtSclWdt)) {
	    if (IPListObjectLength(WeightPtSclWdt) == 3) {
	        IPObjectStruct
		    *WeightPt = IPListObjectGet(WeightPtSclWdt, 0),
		    *WeightScl = IPListObjectGet(WeightPtSclWdt, 1),
		    *WeightWdt = IPListObjectGet(WeightPtSclWdt, 2);

		if (WeightPt != NULL && IP_IS_POINT_OBJ(WeightPt) &&
		    WeightScl != NULL && IP_IS_NUM_OBJ(WeightScl) &&
		    WeightWdt != NULL && IP_IS_NUM_OBJ(WeightWdt)) {
		    SymbAdapIsoSetWeightPt(WeightPt -> U.Pt, WeightScl -> U.R,
					   WeightWdt -> U.R);
		}
		else {
		    IRIT_NON_FATAL_ERROR("Expected a point, a scaling factor, and a width factor in the WeightPt list.");
		}
	    }
	    else if (IPListObjectLength(WeightPtSclWdt) != 0) {
	        IRIT_NON_FATAL_ERROR("Expected a point, a scaling factor, and a width factor in the WeightPt list.");
	    }
	}
	else {
	    IRIT_NON_FATAL_ERROR("Expected a list object for weight point info.");
	}
    }

    if (IRIT_APX_EQ(*GenIsos, 0.0)) {
        NewPObj = SymbAdapIsoExtractRectRgns(Srf -> U.Srfs,
				(CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
				*Eps, 
				IRIT_REAL_PTR_TO_INT(SinglePathSmoothing),
				IRIT_REAL_PTR_TO_INT(FullIsoOutputType));
    }
    else {
        CagdCrvStruct
	    *Crvs = SymbAdapIsoExtract(Srf -> U.Srfs, NULL, NULL,
			       (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
			       *Eps, IRIT_REAL_PTR_TO_INT(FullIsoOutputType),
			       IRIT_REAL_PTR_TO_INT(SinglePathSmoothing));

	NewPObj = IPGenCRVObject(Crvs);
    }

    SymbAdapIsoSetWeightPt(NULL, 1.0, 0.0);		       /* Disable. */

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns the parametric domain of the given curve or surface, trimmed       M
* surface or a trivariate.					             M
*                                                                            *
* PARAMETERS:                                                                M
*   FreeformObj:  To return its parametric domain.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of two (curve) or four (surface, trimmed       M
*                      surface) or six (trivariate) numbers representing     M
*		       the parametric domain of FreeformObj.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetFreefromParamDomain                                                   M
*****************************************************************************/
IPObjectStruct *GetFreefromParamDomain(IPObjectStruct *FreeformObj)
{
    int i, n;
    IPObjectStruct *NewPObj;
    CagdRType LocalDomain[6], *Domain;

    Domain = LocalDomain;

    if (IP_IS_CRV_OBJ(FreeformObj)) {
	CagdCrvDomain(FreeformObj -> U.Crvs, &Domain[0], &Domain[1]);
	n = 2;
    }
    else if (IP_IS_SRF_OBJ(FreeformObj)) {
	CagdSrfDomain(FreeformObj -> U.Srfs,
		      &Domain[0], &Domain[1], &Domain[2], &Domain[3]);
	n = 4;
    }
    else if (IP_IS_TRIMSRF_OBJ(FreeformObj)) {
	CagdSrfDomain(FreeformObj -> U.TrimSrfs -> Srf,
		      &Domain[0], &Domain[1], &Domain[2], &Domain[3]);
	n = 4;
    }
    else if (IP_IS_TRIVAR_OBJ(FreeformObj)) {
	TrivTVDomain(FreeformObj -> U.Trivars,
		     &Domain[0], &Domain[1],
		     &Domain[2], &Domain[3],
		     &Domain[4], &Domain[5]);
	n = 6;
    }
    else if (IP_IS_TRISRF_OBJ(FreeformObj)) {
	TrngTriSrfDomain(FreeformObj -> U.TriSrfs,
			 &Domain[0], &Domain[1],
			 &Domain[2], &Domain[3],
			 &Domain[4], &Domain[5]);
	n = 6;
    }
    else if (IP_IS_MVAR_OBJ(FreeformObj)) {
	n = FreeformObj -> U.MultiVars -> Dim * 2;
	Domain = (CagdRType *) IritMalloc(sizeof(CagdRType) * n);

	for (i = 0; i < FreeformObj -> U.MultiVars -> Dim; i++)
	    MvarMVDomain(FreeformObj -> U.MultiVars,
			 &Domain[i * 2 ], &Domain[i * 2 + 1], i);
    }
    else {
	IRIT_FATAL_ERROR("Only curve/(trimmed/triangular) surface/trivariate/multivariate as first param.");
	return NULL;
    }

    NewPObj = IPGenLISTObject(NULL);
    for (i = 0; i < n ; i++)
	IPListObjectInsert(NewPObj, i, IPGenNUMValObject(Domain[i]));
    IPListObjectInsert(NewPObj, i, NULL);

    if (LocalDomain != Domain)
        IritFree(Domain);

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a least square approximation of a line fit. to a set of points.   M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:   Points to least squares fit a line to.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of a point on the line and a unit vector in    M
*		the line direction.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   LineLeastSquarePtData                                                    M
*****************************************************************************/
IPObjectStruct *LineLeastSquarePtData(IPObjectStruct *PtObjList)
{
    int i;
    CagdRType Err;
    CagdVType LnDir;
    CagdPType LnPos;
    CagdPtStruct
	*Pt = NULL,
	*PtList = NULL;
    IPObjectStruct *PtObj, *LineList;

    if (!IP_IS_OLST_OBJ(PtObjList))
	IRIT_NON_FATAL_ERROR("LINTERP: Not object list object!");

    for (i = 0; (PtObj = IPListObjectGet(PtObjList, i)) != NULL; i++) {
	if (!IP_IS_CTLPT_OBJ(PtObj) &&
	    !IP_IS_POINT_OBJ(PtObj) &&
	    !IP_IS_VEC_OBJ(PtObj)) {
	    CagdPtFreeList(PtList);
	    IRIT_NON_FATAL_ERROR("LINTERP: Non point object found in list");
	    return NULL;
	}

	PtObj = IPCoerceObjectTo(PtObj, IP_OBJ_POINT);
	if (PtList == NULL)
	    PtList = Pt = CagdPtNew();
	else {
	    Pt -> Pnext = CagdPtNew();
	    Pt = Pt -> Pnext;
	}

	/* Copy the point: */
	IRIT_PT_COPY(Pt -> Pt, PtObj -> U.Pt);

	IPFreeObject(PtObj);
    }

    Err = CagdLineFitToPts(PtList, LnDir, LnPos);

    CagdPtFreeList(PtList);

    LineList = IPGenLISTObject(IPGenPTObject(&LnPos[0], &LnPos[1], &LnPos[2]));
    IPListObjectInsert(LineList, 1,
		       IPGenVECObject(&LnDir[0], &LnDir[1], &LnDir[2]));
    IPListObjectInsert(LineList, 2, IPGenNUMValObject(Err));
    IPListObjectInsert(LineList, 3, NULL);

    return LineList;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a least square approximation of a plane fit. to a set of points.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:   Points to least squares fit a plane to.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of a plane and the average error of the fit.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   PlaneLeastSquarePtData                                                   M
*****************************************************************************/
IPObjectStruct *PlaneLeastSquarePtData(IPObjectStruct *PtObjList)
{
    int i;
    CagdRType Err, CN;
    IrtVecType MVec;
    IrtPtType Cntr;
    IrtPlnType Pln;
    CagdPtStruct
	*Pt = NULL,
	*PtList = NULL;
    IPObjectStruct *PtObj, *PlnList;

    if (!IP_IS_OLST_OBJ(PtObjList))
	IRIT_NON_FATAL_ERROR("PINTERP: Not object list object!");

    for (i = 0; (PtObj = IPListObjectGet(PtObjList, i)) != NULL; i++) {
	if (!IP_IS_CTLPT_OBJ(PtObj) &&
	    !IP_IS_POINT_OBJ(PtObj) &&
	    !IP_IS_VEC_OBJ(PtObj)) {
	    CagdPtFreeList(PtList);
	    IRIT_NON_FATAL_ERROR("PINTERP: Non point object found in list");
	    return NULL;
	}

	PtObj = IPCoerceObjectTo(PtObj, IP_OBJ_POINT);
	if (PtList == NULL)
	    PtList = Pt = CagdPtNew();
	else {
	    Pt -> Pnext = CagdPtNew();
	    Pt = Pt -> Pnext;
	}

	/* Copy the point: */
	IRIT_PT_COPY(Pt -> Pt, PtObj -> U.Pt);

	IPFreeObject(PtObj);
    }

    Err = CagdPlaneFitToPts(PtList, Pln, MVec, Cntr, &CN);

    CagdPtFreeList(PtList);

    PlnList = IPGenLISTObject(IPGenPLANEObject(&Pln[0], &Pln[1],
					       &Pln[2], &Pln[3]));
    IPListObjectInsert(PlnList, 1, IPGenNUMValObject(Err));
    IPListObjectInsert(PlnList, 2, NULL);

    return PlnList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a least square approximation/Interpolation of 1d pt data set.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtObjList:   Points to least squares fit.                                M
*   ROrder:      Order of fitting curve.                                     M
*   RCrvSize:    Size of fitting curve.                                      M
*   Params:      Type of parametrization.  Can be either a numeric object    M
*		 that specifies the type of parametrization (uniform, chord  M
*		 length, etc.) or can provide two lists, one of parameters   M
8		 at which to interpolate the points, and a second with the   M
*		 knot vector itself.                                         M
*   RPeriodic:   Periodic curve.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A Bspline curve that least squares fits PtObjList.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvLeastSquarePtData                                                     M
*****************************************************************************/
IPObjectStruct *CrvLeastSquarePtData(IPObjectStruct *PtObjList,
				     CagdRType *ROrder,
				     CagdRType *RCrvSize,
				     IPObjectStruct *Params,
				     CagdRType *RPeriodic)
{
    int i,
	Order = IRIT_REAL_PTR_TO_INT(ROrder),
	CrvSize = IRIT_REAL_PTR_TO_INT(RCrvSize),
	Periodic = IRIT_REAL_PTR_TO_INT(RPeriodic);
    CagdCtlPtStruct
	*CtlPt = NULL,
	*CtlPtList = NULL;
    CagdCrvStruct
	*Crv = NULL;
    IPObjectStruct *PtObj, *LstObj;

    if (IP_IS_CRV_OBJ(PtObjList)) {
        CagdRType Err;
	CagdParametrizationType ParamType;

	if (IP_IS_NUM_OBJ(Params))
	    ParamType = (CagdParametrizationType) ((int) Params -> U.R);
	else
	    ParamType = CAGD_UNIFORM_PARAM;

	if ((Crv = BspCrvFitLstSqr(PtObjList -> U.Crvs, Order, CrvSize,
				   ParamType, FALSE, TRUE, &Err)) == NULL)
	    return NULL;

	LstObj = IPGenLISTObject(NULL);
	IPListObjectInsert(LstObj, 0, IPGenCRVObject(Crv));
	IPListObjectInsert(LstObj, 1, IPGenNUMValObject(Err));
	IPListObjectInsert(LstObj, 2, NULL);

	return LstObj;
    }
    else if (!IP_IS_OLST_OBJ(PtObjList))
	IRIT_NON_FATAL_ERROR("CINTERP: Not object list object!");

    for (i = 0; (PtObj = IPListObjectGet(PtObjList, i)) != NULL; i++) {
	int Deriv;
	IPObjectStruct *CtlPtObj;

	if (!IP_IS_CTLPT_OBJ(PtObj) &&
	    !IP_IS_POINT_OBJ(PtObj) &&
	    !IP_IS_VEC_OBJ(PtObj)) {
	    CagdCtlPtFreeList(CtlPtList);
	    IRIT_NON_FATAL_ERROR("Non point object found in list");
	    return NULL;
	}

	CtlPtObj = IPCoerceObjectTo(PtObj, IP_OBJ_CTLPT);
	if (CtlPtList == NULL)
	    CtlPtList = CtlPt = CagdCtlPtNew(CtlPtObj -> U.CtlPt.PtType);
	else {
	    CtlPt -> Pnext = CagdCtlPtNew(CtlPtObj -> U.CtlPt.PtType);
	    CtlPt = CtlPt -> Pnext;
	}

	/* Copy the point: */
	IRIT_GEN_COPY(CtlPt -> Coords, CtlPtObj -> U.CtlPt.Coords,
		      CAGD_MAX_PT_SIZE * sizeof(CagdRType));
	CtlPt -> PtType = CtlPtObj -> U.CtlPt.PtType;

	/* Mark it as a derivative constraint, if it is. */
	if ((Deriv = AttrGetObjectIntAttrib(PtObj,
					    "Derivative")) != IP_ATTR_BAD_INT)
	    AttrSetIntAttrib(&CtlPt -> Attr, "Derivative", Deriv);

	IPFreeObject(CtlPtObj);
    }

    if (CtlPtList == NULL) {
	IRIT_NON_FATAL_ERROR("Expecting a list of points to interpolate. Got none.");
	return NULL;
    }

    if (IP_IS_OLST_OBJ(Params)) {
	IPObjectStruct
	    *PtParams = IPListObjectGet(Params, 0),
	    *KVParams = IPListObjectGet(Params, 1);

	if (PtParams == NULL ||
	    KVParams == NULL ||
	    IPListObjectGet(Params, 2) != NULL) {
	    IRIT_NON_FATAL_ERROR("Two lists of parameters and knots are expected");
	}
	else if (CrvSize != 0 && CrvSize < Order) {
	    IRIT_NON_FATAL_ERROR("Number of points to interpolate does not match order/length of curve.");
	}
	else {
	    char *ErrStr;
	    int C1Size = CrvSize,
		C2Size = CrvSize;
	    CagdRType *KV, *Prms;

	    if ((Prms = GetKnotVector(PtParams, Order, &C1Size, &ErrStr,
				      FALSE)) == NULL ||
		(KV = GetKnotVector(KVParams, Order, &C2Size, &ErrStr,
				    FALSE)) == NULL)
		IRIT_NON_FATAL_ERROR(ErrStr);
	    else {
		if (CrvSize == 0)
		    CrvSize = C2Size - Order;
		else if (CrvSize != C2Size - Order) {
		    IRIT_NON_FATAL_ERROR("Curve size does not match knot vector length; KV length used instead.");
		    CrvSize = C2Size - Order;
		}

		if ((Crv = BspCrvInterpolate(CtlPtList, Prms, KV, CrvSize,
					     Order, Periodic)) == NULL)
		    IRIT_NON_FATAL_ERROR("Failed to interpolate the curve (singular!?).");

		IritFree(Prms);
		IritFree(KV);
	    }
	}
    }
    else if (IP_IS_NUM_OBJ(Params)) {
	CagdParametrizationType
	    ParamType = (CagdParametrizationType) ((int) Params -> U.R);

	if (CrvSize != 0 && CrvSize < Order) {
	    IRIT_NON_FATAL_ERROR("Number of points to interpolate does not match order/length of curve.");
	}
	else {
	    
	    if ((Crv = BspCrvInterpPts2(CtlPtList, Order, CrvSize,
					ParamType, Periodic)) == NULL)
		IRIT_NON_FATAL_ERROR("Failed to interpolate the curve (singular!?).");
	}
    }
    else {
	IRIT_NON_FATAL_ERROR("Parametrization should be either a keyword or lists");
    }

    CagdCtlPtFreeList(CtlPtList);

    return Crv ? IPGenCRVObject(Crv) : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a least square approximation/Interpolation of 2d pt data set.     M
*                                                                            *
* PARAMETERS:                                                                M
*   LstObjList:   List of point lists.                                       M
*   RUOrder:      Uorder of fitting surface.                                 M
*   RVOrder:      Vorder of fitting surface.                                 M
*   RUSize:       USize of fitting surface.                                  M
*   RVSize:       VSize of fitting surface.                                  M
*   RParamType:   Point type.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A Bspline surface that least square fits LstObjList. M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfLeastSquarePtData                                                     M
*****************************************************************************/
IPObjectStruct *SrfLeastSquarePtData(IPObjectStruct *LstObjList,
				     CagdRType *RUOrder,
				     CagdRType *RVOrder,
				     CagdRType *RUSize,
				     CagdRType *RVSize,
				     CagdRType *RParamType)
{
    int i, j, k,
	Grided = TRUE,
	NumCoords = 1,
	NumVertices = 0,
	NumLists = 0,
	NumVerticesFirst = 0,
	UOrder = IRIT_REAL_PTR_TO_INT(RUOrder),
	VOrder = IRIT_REAL_PTR_TO_INT(RVOrder),
	USize = IRIT_REAL_PTR_TO_INT(RUSize),
	VSize = IRIT_REAL_PTR_TO_INT(RVSize);
    CagdParametrizationType
	ParamType = (CagdParametrizationType) IRIT_REAL_PTR_TO_INT(RParamType);
    CagdSrfStruct
	*Srf = NULL;
    IPObjectStruct *LstObj, *PtObj;

    if (IP_IS_SRF_OBJ(LstObjList)) {
        CagdRType Err;

	if ((Srf = BspSrfFitLstSqr(LstObjList -> U.Srfs, UOrder, VOrder,
				   USize, VSize, ParamType, &Err)) == NULL)
	    return NULL;

	LstObj = IPGenLISTObject(NULL);
	IPListObjectInsert(LstObj, 0, IPGenSRFObject(Srf));
	IPListObjectInsert(LstObj, 1, IPGenNUMValObject(Err));
	IPListObjectInsert(LstObj, 2, NULL);

	return LstObj;
    }
    else if (!IP_IS_OLST_OBJ(LstObjList)) {
	IRIT_NON_FATAL_ERROR("SINTERP: Not object list object!");
        return NULL;
    }

    while ((LstObj = IPListObjectGet(LstObjList, NumLists)) != NULL) {
        if (IP_IS_OLST_OBJ(LstObj)) {
	    /* Grid type interpolation/approximation. */
	    Grided = TRUE;
	    NumVertices = -1;
	    while ((PtObj = IPListObjectGet(LstObj, ++NumVertices)) != NULL) {
	        if (!IP_IS_CTLPT_OBJ(PtObj) &&
		    !IP_IS_POINT_OBJ(PtObj) &&
		    !IP_IS_VEC_OBJ(PtObj)) {
		    return NULL;
		}
	    }
	}
	else {
	    /* Scattered type interpolation/approximation. */
	    Grided = FALSE;
	    if (!IP_IS_CTLPT_OBJ(LstObj) &&
		!IP_IS_POINT_OBJ(LstObj) &&
		!IP_IS_VEC_OBJ(LstObj)) {
	        return NULL;
	    }
	    if (IP_IS_CTLPT_OBJ(LstObj) &&
		NumCoords < CAGD_NUM_OF_PT_COORD(LstObj -> U.CtlPt.PtType))
	        NumCoords = CAGD_NUM_OF_PT_COORD(LstObj -> U.CtlPt.PtType);
	    else if (NumCoords < 3) /* A vector or a Point */
	        NumCoords = 3;
	}

	if (NumLists++ == 0)
	    NumVerticesFirst = NumVertices;
	else
	    if (NumVerticesFirst != NumVertices)
	        return NULL;
    }

    /* Coerce all points to a point type, in place. */
    if (Grided) {
        CagdPtStruct **PtListArray, *Pt;

	i = 0;
	while ((LstObj = IPListObjectGet(LstObjList, i++)) != NULL)
	    IPCoercePtsListTo(LstObj, CAGD_PT_E3_TYPE);

	PtListArray = (CagdPtStruct **)
	                  IritMalloc(sizeof(CagdPtStruct *) * (NumLists + 1));
	PtListArray[NumLists] = NULL;

	for (i = 0; (LstObj = IPListObjectGet(LstObjList, i)) != NULL; i++) {
	    Pt = PtListArray[i] = CagdPtNew();
	    for (j = 0; (PtObj = IPListObjectGet(LstObj, j)) != NULL; j++) {
	        for (k = 0; k < 3; k++)
		    Pt -> Pt[k] = PtObj -> U.CtlPt.Coords[k + 1];

		if (j < NumVertices - 1) {
		    Pt -> Pnext = CagdPtNew();
		    Pt = Pt -> Pnext;
		}
	    }
	}

	if (NumLists < 3 || NumVertices < 3 ||
	    NumLists < UOrder || NumVertices < VOrder ||
	    (USize != 0 && (USize < UOrder)) ||
	    (VSize != 0 && (VSize < VOrder))) {
	    IRIT_NON_FATAL_ERROR("Number of points to interpolate does not match order/length of curve.");
	}
	else {
	    if ((Srf = BspSrfInterpPts((const CagdPtStruct **) PtListArray,
				       UOrder, VOrder,
				       USize, VSize, ParamType)) == NULL)
	        IRIT_NON_FATAL_ERROR("Failed to interpolate the curve (singular!?).");
	}

	for (i = 0; i < NumLists; i++)
	    CagdPtFreeList(PtListArray[i]);
	IritFree(PtListArray);
    }
    else {
        CagdCtlPtStruct *CtlPt,
	    *CtlPtHead = NULL;

	IPCoercePtsListTo(LstObjList, CAGD_MAKE_PT_TYPE(FALSE, NumCoords));

	for (i = 0; (PtObj = IPListObjectGet(LstObjList, i)) != NULL; i++) {
	    CtlPt = CagdCtlPtCopy(&PtObj -> U.CtlPt);
	    IRIT_LIST_PUSH(CtlPt, CtlPtHead);
	}

	Srf = BspSrfInterpScatPts(CtlPtHead, UOrder, VOrder, USize, VSize,
				  NULL, NULL);

	CagdCtlPtFreeList(CtlPtHead);
    }

    return Srf ? IPGenSRFObject(Srf) : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to fetch the resolution parameter from the RESOLUTION object.     M
*                                                                            *
* PARAMETERS:                                                                M
*   None.	                                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:          The fetched resolution.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetResolutionReal                                                        M
*****************************************************************************/
IrtRType GetResolutionReal(void)
{
    IPObjectStruct
	*PObj = IritDBGetObjByName("RESOLUTION");

    if (PObj == NULL || !IP_IS_NUM_OBJ(PObj)) {
	IRIT_NON_FATAL_ERROR("No numeric object name RESOLUTION is defined");
	return DEFAULT_RESOLUTION;
    }
    else {
        return PObj -> U.R;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to fetch the resolution parameter from the RESOLUTION object.     M
*  If ClipToMin TRUE, the Resolution is clipped to not be below MIN_RES.     M
*                                                                            *
* PARAMETERS:                                                                M
*   ClipToMin:    Do we want a lower bound?                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          Current resolution level.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GetResolution                                                            M
*****************************************************************************/
int GetResolution(int ClipToMin)
{
    int Resolution = (int) GetResolutionReal();

    if (ClipToMin) {
        Resolution = IRIT_MAX(Resolution, PRIM_MIN_RESOLUTION);

	Resolution = (Resolution >> 1) * 2;        /* Make sure its even. */
    }

    return Resolution;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a multi-resolution decomposition for a given curve using least    M
* squares. Returned is a list of curves of the decomposition.                M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvObj:     Curve to multi-resolution decompose.                         M
*   Discont:    Do we want to preserve discontinuities?			     M
*	        If, however, Discont == -1, the B-Wavelet of the index       M
*		'*LeastSqr' is computed and returned for given crv's space.  M
*   LeastSqr:   Use Least squares for the projection (if TRUE) or B-wavelets M
*		(if FALSE). If, however, Discont == -1, specifies the index  M
*		of the knot to compute the B-Wavelet function for.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list object of all the curves of the decomposition. M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurveMultiResDecomp                                                      M
*****************************************************************************/
IPObjectStruct *CurveMultiResDecomp(IPObjectStruct *CrvObj,
				    IrtRType *Discont,
				    IrtRType *LeastSqr)
{
    int i;
    IPObjectStruct *CrvObjList;
    SymbMultiResCrvStruct *MResCrv;

    if (IRIT_APX_EQ(*Discont, -1.0)) {
        /* Compute the BWavelet at knot index '*LeastSqr'. */
        CagdCrvStruct
	    *Crv = CrvObj -> U.Crvs,
	    *BWCrv = SymbCrvMultiResBWavelet(Crv -> KnotVector,
					     Crv -> Order,
					     Crv -> Length + Crv -> Order,
					     IRIT_REAL_PTR_TO_INT(LeastSqr));

	if (BWCrv == NULL)
	    return NULL;

	return IPGenCRVObject(BWCrv);
    }

    if (!IRIT_APX_EQ(*LeastSqr, 0))
	MResCrv = SymbCrvMultiResDecomp(CrvObj -> U.Crvs,
					IRIT_REAL_PTR_TO_INT(Discont));
    else
	MResCrv = SymbCrvMultiResDecomp2(CrvObj -> U.Crvs,
					 IRIT_REAL_PTR_TO_INT(Discont), FALSE);

    if (MResCrv == NULL) {
        IRIT_NON_FATAL_ERROR("Multi resolution decomposition failed.");
        return NULL;
    }

    CrvObjList = IPGenLISTObject(NULL);

    for (i = 0; i < MResCrv -> Levels; i++)
	IPListObjectInsert(CrvObjList, i,
			   IPGenCRVObject(CagdCrvCopy(MResCrv -> HieCrv[i])));

    IPListObjectInsert(CrvObjList, i, NULL);

    SymbCrvMultiResFree(MResCrv);

    return CrvObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection of two ruled surfaces.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1C1, Srf1C2:    The two curves defining the first ruled surface.      M
*   Srf2C1, Srf2C2:    The two curves defining the second ruled surface.     M
*   Tolerance:         Accuracy of computation.                              M
*   RZeroSetFunc:      TRUE, for zero set function, FALSE for actual inter.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of three lists.  First list is of the          M
*		       Euclidean curves, second list is of first surface's   M
*		       parametric domain, and third list is of second        M
*		       surface's parametric domain			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RuledRuledInter                                                          M
*****************************************************************************/
IPObjectStruct *RuledRuledInter(IPObjectStruct *Srf1C1,
				IPObjectStruct *Srf1C2,
				IPObjectStruct *Srf2C1,
				IPObjectStruct *Srf2C2,
				IrtRType *Tolerance,
				IrtRType *RZeroSetFunc)
{
    int i,
	ZeroSetFunc = IRIT_REAL_PTR_TO_INT(RZeroSetFunc);
    IPObjectStruct *CrvObjList, *E3Objs, *PObjs1, *PObjs2;
    CagdCrvStruct *PCrvs1, *PCrvs2, *Crv, *E3Crvs;

    /* Should we compute only the zero set function? */
    if (ZeroSetFunc) {
	CagdSrfStruct
	    *ZeroSetSrf = SymbRuledRuledZeroSetFunc(Srf1C1 -> U.Crvs,
						    Srf1C2 -> U.Crvs,
						    Srf2C1 -> U.Crvs,
						    Srf2C2 -> U.Crvs);
	return IPGenSRFObject(ZeroSetSrf);
    }

    /* Do we have any intersection? */
    if ((E3Crvs = SymbRuledRuledIntersection(Srf1C1 -> U.Crvs,
					     Srf1C2 -> U.Crvs,
					     Srf2C1 -> U.Crvs,
					     Srf2C2 -> U.Crvs,
					     *Tolerance,
					     &PCrvs1,
					     &PCrvs2)) == NULL) {
	CrvObjList = IPGenLISTObject(NULL);
        return CrvObjList;
    }

    E3Objs = IPGenLISTObject(NULL);
    for (i = 0; E3Crvs != NULL; i++) {
	Crv = E3Crvs;
	IPListObjectInsert(E3Objs, i, IPGenCRVObject(E3Crvs));
	E3Crvs = E3Crvs -> Pnext;
	Crv -> Pnext = NULL;
    }
    IPListObjectInsert(E3Objs, i, NULL);

    PObjs1 = IPGenLISTObject(NULL);
    for (i = 0; PCrvs1 != NULL; i++) {
	Crv = PCrvs1;
	IPListObjectInsert(PObjs1, i, IPGenCRVObject(PCrvs1));
	PCrvs1 = PCrvs1 -> Pnext;
	Crv -> Pnext = NULL;
    }
    IPListObjectInsert(PObjs1, i, NULL);

    PObjs2 = IPGenLISTObject(NULL);
    for (i = 0; PCrvs2 != NULL; i++) {
	Crv = PCrvs2;
	IPListObjectInsert(PObjs2, i, IPGenCRVObject(PCrvs2));
	PCrvs2 = PCrvs2 -> Pnext;
	Crv -> Pnext = NULL;
    }
    IPListObjectInsert(PObjs2, i, NULL);

    CrvObjList = IPGenLISTObject(E3Objs);
    IPListObjectInsert(CrvObjList, 1, PObjs1);
    IPListObjectInsert(CrvObjList, 2, PObjs2);
    IPListObjectInsert(CrvObjList, 3, NULL);

    return CrvObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection of two ruled surfaces.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1C, Srf1r:      Axis/radius curves of first ring surface.	     M
*   Srf2C, Srf2r:      Axis/radius curves of second ring surface.	     M
*   Tolerance:         Accuracy of computation.                              M
*   RZeroSetFunc:      TRUE, for zero set function, FALSE for actual inter.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of three lists.  First list is of the          M
*		       Euclidean curves, second list is of first surface's   M
*		       parametric domain, and third list is of second        M
*		       surface's parametric domain			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   RingRingInter                                                            M
*****************************************************************************/
IPObjectStruct *RingRingInter(IPObjectStruct *Srf1C,
			      IPObjectStruct *Srf1r,
			      IPObjectStruct *Srf2C,
			      IPObjectStruct *Srf2r,
			      IrtRType *Tolerance,
			      IrtRType *RZeroSetFunc)
{
    int i,
	ZeroSetFunc = IRIT_REAL_PTR_TO_INT(RZeroSetFunc);
    IPObjectStruct *CrvObjList, *E3Objs, *PObjs1, *PObjs2;
    CagdCrvStruct *PCrvs1, *PCrvs2, *Crv, *E3Crvs;

    /* Should we compute only the zero set function? */
    if (ZeroSetFunc) {
	CagdSrfStruct
	    *ZeroSetSrf = SymbRingRingZeroSetFunc(Srf1C -> U.Crvs,
						  Srf1r -> U.Crvs,
						  Srf2C -> U.Crvs,
						  Srf2r -> U.Crvs);
	return IPGenSRFObject(ZeroSetSrf);
    }

    /* Do we have any intersection? */
    if ((E3Crvs = SymbRingRingIntersection(Srf1C -> U.Crvs,
					   Srf1r -> U.Crvs,
					   Srf2C -> U.Crvs,
					   Srf2r -> U.Crvs,
					   *Tolerance,
					   &PCrvs1,
					   &PCrvs2)) == NULL) {
	CrvObjList = IPGenLISTObject(NULL);
        return CrvObjList;
    }

    E3Objs = IPGenLISTObject(NULL);
    for (i = 0; E3Crvs != NULL; i++) {
	Crv = E3Crvs;
	IPListObjectInsert(E3Objs, i, IPGenCRVObject(E3Crvs));
	E3Crvs = E3Crvs -> Pnext;
	Crv -> Pnext = NULL;
    }
    IPListObjectInsert(E3Objs, i, NULL);

    PObjs1 = IPGenLISTObject(NULL);
    for (i = 0; PCrvs1 != NULL; i++) {
	Crv = PCrvs1;
	IPListObjectInsert(PObjs1, i, IPGenCRVObject(PCrvs1));
	PCrvs1 = PCrvs1 -> Pnext;
	Crv -> Pnext = NULL;
    }
    IPListObjectInsert(PObjs1, i, NULL);

    PObjs2 = IPGenLISTObject(NULL);
    for (i = 0; PCrvs2 != NULL; i++) {
	Crv = PCrvs2;
	IPListObjectInsert(PObjs2, i, IPGenCRVObject(PCrvs2));
	PCrvs2 = PCrvs2 -> Pnext;
	Crv -> Pnext = NULL;
    }
    IPListObjectInsert(PObjs2, i, NULL);

    CrvObjList = IPGenLISTObject(E3Objs);
    IPListObjectInsert(CrvObjList, 1, PObjs1);
    IPListObjectInsert(CrvObjList, 2, PObjs2);
    IPListObjectInsert(CrvObjList, 3, NULL);

    return CrvObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compares the given two curves. Each curve is converted, if necessary,    M
*   into a set of Bezier curves, and the comparison is done by applying      M
*   comparison algorithm for each Bezier curve.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Obj, Crv2Obj:   The two curves to be compared.                       M
*   Eps:                A threshold for numerical computations.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of 5 real values as follows:                   M
*                      + First denotes the curves' relation as:              V
*                        1. Same curves (1),				     V
*			 2. Overlapping curves (2), or			     V
*			 3. Distinct curves (3).			     V
*                      + The rest of the 4 values are			     V
*			 (Start1, End1, Start2, End2).			     V
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvsCompare                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CurvesCompare                                                            M
*****************************************************************************/
IPObjectStruct *CurvesCompare(IPObjectStruct *Crv1Obj,
			      IPObjectStruct *Crv2Obj,
			      IrtRType *Eps)
{
    int IRel;
    IrtRType StartOvrlpPrmCrv1, EndOvrlpPrmCrv1,
	     StartOvrlpPrmCrv2, EndOvrlpPrmCrv2;
    SymbCrvRelType
	Rel = SymbCrvsCompare(Crv1Obj -> U.Crvs, Crv2Obj -> U.Crvs, *Eps,
			      &StartOvrlpPrmCrv1, &EndOvrlpPrmCrv1,
			      &StartOvrlpPrmCrv2, &EndOvrlpPrmCrv2);
    IPObjectStruct *RetObj;

    switch (Rel) {
	case SYMB_CRVREL_SAME_CRVS:
	    IRel = 1;
	    break;
	case SYMB_CRVREL_OVERLAPPING_CRVS:
	    IRel = 2;
	    break;
	default:
	case SYMB_CRVREL_DISTINCT_CRVS:
	    IRel = 3;
	    break;
    }

    
    RetObj = IPGenLISTObject(IPGenNUMValObject(IRel));
    IPListObjectInsert(RetObj, 1, IPGenNUMValObject(StartOvrlpPrmCrv1));
    IPListObjectInsert(RetObj, 2, IPGenNUMValObject(EndOvrlpPrmCrv1));
    IPListObjectInsert(RetObj, 3, IPGenNUMValObject(StartOvrlpPrmCrv2));
    IPListObjectInsert(RetObj, 4, IPGenNUMValObject(EndOvrlpPrmCrv2));
    IPListObjectInsert(RetObj, 5, NULL);

    return RetObj;
}
