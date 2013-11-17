/******************************************************************************
* Offset.c - computes offset approximation to curves and surfaces.            *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March. 93.					      *
******************************************************************************/

#include "irit_sm.h"
#include "symb_loc.h"
#include "user_lib.h"
#include "geom_lib.h"
#include "extra_fn.h"
#include "iritprsr.h"
#include "allocate.h"

#define MAX_OFFSET_IMPROVE_ITERS	20
#define NORMAL_PERTURB			1e-4
#define OFFSET_TRIM_EPS			1.25
#define CONTOUR_EPS   5.6*.38841e-8 /* Level above zero to actually contour. */
#define CNTR_DIAGONAL_EPS		1e-3

static int GetMatchedTrimParameter(const CagdCrvStruct *Crv,
				   CagdPtStruct *Pts,
				   CagdPtStruct **Pt1,
				   int *Idx1,
				   CagdPtStruct **Pt2,
				   int *Idx2);
static CagdPtStruct *ExtractValidDomainFromContours(IPPolygonStruct *Cntrs);
static void InsertInvalidDomainIntoList(CagdPtStruct **PtList,
					CagdRType TMin,
					CagdRType TMax);
static void InsertInvalidDomainIntoListAux(CagdPtStruct *Pt,
					   CagdRType TMin,
					   CagdRType TMax);
static int OffsetGlblTrimUpdateInter(const CagdCrvStruct *Crv,
				     const CagdCrvStruct *DCrv,
				     CagdRType *OrigT1,
				     CagdRType *OrigT2,
				     CagdRType Eps);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and an offset amount OffsetDist, returns an approximation to M
* the offset curve by offseting the control polygon in the normal direction. M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         To approximate its offset curve with distance OffsetDist.  M
*   OffsetDist:   Amount of offset. Negative denotes other offset direction. M
*   BezInterp:    If TRUE, control points are interpolated when the curve is M
*		  reduced to a Bezier form. Otherwise, control points are    M
*		  translated OffsetDist amount only, under estimating the    M
*		  Offset. 						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the offset curve.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,		     M
*   SymbCrvAdapOffset, SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset,       M
*   SymbCrvMatchingOffset, SymbCrvVarOffset				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvOffset, offset                                                    M
*****************************************************************************/
CagdCrvStruct *SymbCrvOffset(const CagdCrvStruct *CCrv,
			     CagdRType OffsetDist,
			     CagdBType BezInterp)
{
    CagdBType
	IsBezierCurve = FALSE,
	IsRational = CAGD_IS_RATIONAL_CRV(CCrv);
    int i, j,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CCrv -> PType),
	Order = CCrv -> Order,
	Length = CCrv -> Length;
    CagdBType
	HasNewKV = FALSE;
    CagdVecStruct *N;
    CagdCrvStruct *OffsetCrv, *Crv;
    CagdRType *Nodes, *NodePtr, **Points,
	*KV = NULL;

    switch (CCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCrvCopy(CCrv);
	    HasNewKV = TRUE;
	    KV = BspKnotUniformOpen(Length, Order, NULL);
	    IsBezierCurve = TRUE;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCnvrtBsp2OpenCrv(CCrv);
	    KV = Crv -> KnotVector;
	    IsBezierCurve = Crv -> Length == Crv -> Order;
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    return NULL;
    }

    OffsetCrv = CagdCrvCopy(Crv);
    Points = OffsetCrv -> Points;
    Nodes = BspKnotNodes(KV, Length + Order, Order);
    NodePtr = Nodes;

    /* Interpolate the computed control points instead of under-estimating */
    /* this offset.						           */
    if (BezInterp && IsBezierCurve) {
	CagdCrvStruct *TCrv;

	if (IsRational) {
	    TCrv = CagdCoerceCrvTo(OffsetCrv, CAGD_MAKE_PT_TYPE(FALSE,
				   CAGD_NUM_OF_PT_COORD(OffsetCrv -> PType)),
				   FALSE);

	    CagdCrvFree(OffsetCrv);
	    OffsetCrv = TCrv;
	    Points = OffsetCrv -> Points;
	}

	for (j = 0; j < Length; j++, NodePtr++) {
	    CagdRType
		*R = CagdCrvEval(Crv, *NodePtr);

	    if (MaxCoord == 3 && Order > 2) {
	        if ((N = CagdCrvNormal(Crv, *NodePtr, TRUE)) == NULL &&
		    (N = CagdCrvNormal(Crv,
				       NodePtr == Nodes ?
				           *NodePtr + NORMAL_PERTURB :
				           *NodePtr - NORMAL_PERTURB,
				       TRUE)) == NULL) {
		    CagdCrvFree(Crv);
		    CagdCrvFree(OffsetCrv);
		    SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		    return NULL;
		}
	    }
	    else {
	        if ((N = CagdCrvNormalXY(Crv, *NodePtr, TRUE)) == NULL &&
		    (N = CagdCrvNormalXY(Crv,
					 NodePtr == Nodes ?
				             *NodePtr + NORMAL_PERTURB :
				             *NodePtr - NORMAL_PERTURB,
					 TRUE)) == NULL) {
		    CagdCrvFree(Crv);
		    CagdCrvFree(OffsetCrv);
		    SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		    return NULL;
		}
	    }

	    for (i = 1; i <= MaxCoord; i++)
		Points[i][j] = R[i] / (IsRational ? R[0] : 1.0) +
			       N -> Vec[i - 1] * OffsetDist;
	}

	TCrv = CagdCrvCopy(OffsetCrv);
	for (i = 1; i <= MaxCoord; i++)
	    BzrCrvInterp(TCrv -> Points[i], OffsetCrv -> Points[i], Length);

	CagdCrvFree(OffsetCrv);
	OffsetCrv = TCrv;
    }
    else {
	for (j = 0; j < Length; j++, NodePtr++) {
	    if (MaxCoord == 3 && Order > 2) {
	        if ((N = CagdCrvNormal(Crv, *NodePtr, TRUE)) == NULL &&
		    (N = CagdCrvNormal(Crv,
				       NodePtr == Nodes ?
				           *NodePtr + NORMAL_PERTURB :
				           *NodePtr - NORMAL_PERTURB,
				       TRUE)) == NULL) {
		    CagdCrvFree(Crv);
		    CagdCrvFree(OffsetCrv);
		    SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		    return NULL;
		}
	    }
	    else {
	        if ((N = CagdCrvNormalXY(Crv, *NodePtr, TRUE)) == NULL &&
		    (N = CagdCrvNormalXY(Crv,
					 NodePtr == Nodes ?
				             *NodePtr + NORMAL_PERTURB :
				             *NodePtr - NORMAL_PERTURB,
					 TRUE)) == NULL) {
		    CagdCrvFree(Crv);
		    CagdCrvFree(OffsetCrv);
		    SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		    return NULL;
		}
	    }

	    for (i = 1; i <= MaxCoord; i++)
		Points[i][j] +=
		    N -> Vec[i - 1] * OffsetDist *
			(IsRational ? Points[SYMB_W][j] : 1.0);
	}
    }

    if (HasNewKV)
	IritFree(KV);
    IritFree(Nodes);
    CagdCrvFree(Crv);

    return OffsetCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and an offset amount function Var OffsetDist, returns an     M
* approximation to the offset curve by offseting the control polygon in the  M
* normal direction.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:           To approximate its variable offset curve.                M
*   VarOffsetDist:  Scalar function prescribing the amount of offset.        M
*		    Must posses a parametric domain similar to Crv.	     M
*   BezInterp:      If TRUE, control points are interpolated when the curve  M
*		    is reduced to a Bezier form. Otherwise, control points   M
*		    are translated OffsetDist amount only, under estimating  M
*		    the Offset. 					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the varying offset amount curve.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,		     M
*   SymbCrvAdapOffset, SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset,       M
*   SymbCrvMatchingOffset, SymbCrvOffset				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvVarOffset, offset                                                 M
*****************************************************************************/
CagdCrvStruct *SymbCrvVarOffset(const CagdCrvStruct *CCrv,
				const CagdCrvStruct *VarOffsetDist,
				CagdBType BezInterp)
{
    CagdBType
	IsBezierCurve = FALSE,
	IsRational = CAGD_IS_RATIONAL_CRV(CCrv);
    int i, j,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CCrv -> PType),
	Order = CCrv -> Order,
	Length = CCrv -> Length;
    CagdBType
	HasNewKV = FALSE;
    CagdVecStruct *N;
    CagdCrvStruct *OffsetCrv, *Crv;
    CagdRType *Nodes, *NodePtr, **Points,
	*KV = NULL;

    switch (CCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCrvCopy(CCrv);
	    HasNewKV = TRUE;
	    KV = BspKnotUniformOpen(Length, Order, NULL);
	    IsBezierCurve = TRUE;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCnvrtBsp2OpenCrv(CCrv);
	    KV = Crv -> KnotVector;
	    IsBezierCurve = Crv -> Length == Crv -> Order;
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    return NULL;
    }

    OffsetCrv = CagdCrvCopy(Crv);
    Points = OffsetCrv -> Points;
    Nodes = BspKnotNodes(KV, Length + Order, Order);
    NodePtr = Nodes;

    /* Interpolate the computed control points instead of under-estimating */
    /* this offset.						           */
    if (BezInterp && IsBezierCurve) {
	CagdCrvStruct *TCrv;

	if (IsRational) {
	    TCrv = CagdCoerceCrvTo(OffsetCrv, CAGD_MAKE_PT_TYPE(FALSE,
				   CAGD_NUM_OF_PT_COORD(OffsetCrv -> PType)),
				   FALSE);

	    CagdCrvFree(OffsetCrv);
	    OffsetCrv = TCrv;
	    Points = OffsetCrv -> Points;
	}

	for (j = 0; j < Length; j++, NodePtr++) {
	    CagdRType *R, OffDist;

	    R = CagdCrvEval(VarOffsetDist, *NodePtr);
	    OffDist = R[1];
	    R = CagdCrvEval(Crv, *NodePtr);

	    if (MaxCoord == 3 && Order > 2) {
	        if ((N = CagdCrvNormal(Crv, *NodePtr, TRUE)) == NULL &&
		    (N = CagdCrvNormal(Crv,
				       NodePtr == Nodes ?
				           *NodePtr + NORMAL_PERTURB :
				           *NodePtr - NORMAL_PERTURB,
				       TRUE)) == NULL) {
		    CagdCrvFree(Crv);
		    CagdCrvFree(OffsetCrv);
		    SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		    return NULL;
		}
	    }
	    else {
	        if ((N = CagdCrvNormalXY(Crv, *NodePtr, TRUE)) == NULL &&
		    (N = CagdCrvNormalXY(Crv,
					 NodePtr == Nodes ?
				             *NodePtr + NORMAL_PERTURB :
				             *NodePtr - NORMAL_PERTURB,
					 TRUE)) == NULL) {
		    CagdCrvFree(Crv);
		    CagdCrvFree(OffsetCrv);
		    SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		    return NULL;
		}
	    }

	    for (i = 1; i <= MaxCoord; i++)
		Points[i][j] = R[i] / (IsRational ? R[0] : 1.0) +
			       N -> Vec[i - 1] * OffDist;
	}

	TCrv = CagdCrvCopy(OffsetCrv);
	for (i = 1; i <= MaxCoord; i++)
	    BzrCrvInterp(TCrv -> Points[i], OffsetCrv -> Points[i], Length);

	CagdCrvFree(OffsetCrv);
	OffsetCrv = TCrv;
    }
    else {
	for (j = 0; j < Length; j++, NodePtr++) {
	    CagdRType *R, OffDist;

	    R = CagdCrvEval(VarOffsetDist, *NodePtr);
	    OffDist = R[1];

	    if ((N = CagdCrvNormalXY(Crv, *NodePtr, TRUE)) == NULL &&
		(N = CagdCrvNormalXY(Crv,
				     NodePtr == Nodes ?
				         *NodePtr + NORMAL_PERTURB :
				         *NodePtr - NORMAL_PERTURB,
				     TRUE)) == NULL) {
		CagdCrvFree(Crv);
		CagdCrvFree(OffsetCrv);
		SYMB_FATAL_ERROR(SYMB_ERR_CANNOT_COMP_NORMAL);
		return NULL;
	    }

	    for (i = 1; i <= MaxCoord; i++)
		Points[i][j] +=
		    N -> Vec[i - 1] * OffDist *
			(IsRational ? Points[SYMB_W][j] : 1.0);
	}
    }

    if (HasNewKV)
	IritFree(KV);
    IritFree(Nodes);
    CagdCrvFree(Crv);

    return OffsetCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and an offset amount OffsetDist, returns an approximation to M
* the offset curve by offseting the control polygon in the normal direction. M
*   If resulting offset is not satisfying the required tolerance the curve   M
* is subdivided and the algorithm recurses on both parts.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:         To approximate its offset curve with distance OffsetDist.  M
*   OffsetDist:   Amount of offset. Negative denotes other offset direction. M
*   Tolerance:    Accuracy control.                                          M
*   BezInterp:    If TRUE, control points are interpolated when the curve is M
*		  reduced to a Bezier form. Otherwise, control points are    M
*		  translated OffsetDist amount only, under estimating the    M
*		  Offset. 						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the offset curve, to within       M
*                      Tolerance.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbSrfOffset, SymbSrfSubdivOffset, SymbCrvAdapOffset,    M
*   SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset, SymbCrvMatchingOffset    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvSubdivOffset, offset                                              M
*****************************************************************************/
CagdCrvStruct *SymbCrvSubdivOffset(const CagdCrvStruct *CCrv,
				   CagdRType OffsetDist,
				   CagdRType Tolerance,
				   CagdBType BezInterp)
{
    CagdCrvStruct *Crv, *OffCrv, *Dist, *DistSqr;
    CagdRType *R, MinVal, MaxVal, TMin, TMax;

    switch (CCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCnvrtBzr2BspCrv(CCrv);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCnvrtBsp2OpenCrv(CCrv);
	    break;
	case CAGD_CPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    return NULL;
    }

    OffCrv = SymbCrvOffset(Crv, OffsetDist, BezInterp);
    Dist = SymbCrvSub(Crv, OffCrv);
    DistSqr = SymbCrvDotProd(Dist, Dist);

    CagdCrvFree(Dist);

    R = SymbExtremumCntPtVals(DistSqr -> Points, DistSqr -> Length, TRUE);
    MinVal = R[1] < 0.0 ? 0.0 : sqrt(R[1]);
    R = SymbExtremumCntPtVals(DistSqr -> Points, DistSqr -> Length, FALSE);
    MaxVal = R[1] < 0.0 ? 0.0 : sqrt(R[1]);

    CagdCrvFree(DistSqr);

    CagdCrvDomain(Crv, &TMin, &TMax);
    if ((IRIT_FABS(MinVal - IRIT_FABS(OffsetDist)) > Tolerance ||
	 IRIT_FABS(MaxVal - IRIT_FABS(OffsetDist)) > Tolerance) &&
	TMax - TMin > NORMAL_PERTURB * 10.0) {
	CagdCrvStruct *Crv1, *Crv2, *OffCrv1, *OffCrv2;

	if (TMax - TMin > NORMAL_PERTURB * 10.0) {
	    CagdCrvFree(OffCrv);

	    Crv1 = CagdCrvSubdivAtParam(Crv, (TMin + TMax) * 0.5);
	    Crv2 = Crv1 -> Pnext;
	    Crv1 -> Pnext = NULL;
	    OffCrv1 = SymbCrvSubdivOffset(Crv1, OffsetDist, Tolerance, BezInterp);
	    OffCrv2 = SymbCrvSubdivOffset(Crv2, OffsetDist, Tolerance, BezInterp);
	    CagdCrvFree(Crv1);
	    CagdCrvFree(Crv2);

	    OffCrv = CagdMergeCrvCrv(OffCrv1, OffCrv2, TRUE);
	    CagdCrvFree(OffCrv1);
	    CagdCrvFree(OffCrv2);
	}
    }

    CagdCrvFree(Crv);

    return OffCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and an offset amount OffsetDist, returns an approximation  M
* to the offset surface by offseting the control mesh in the normal	     M
* direction.                                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:        To approximate its offset surface with distance OffsetDist. M
*   OffsetDist:  Amount of offset. Negative denotes other offset direction.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An approximation to the offset surface.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfSubdivOffset,		     M
*   SymbCrvAdapOffset, SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset,       M
*   SymbCrvMatchingOffset						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfOffset, offset                                                    M
*****************************************************************************/
CagdSrfStruct *SymbSrfOffset(const CagdSrfStruct *CSrf, CagdRType OffsetDist)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(CSrf);
    int	i, Row, Col, MaxCoord,
	UOrder = CSrf -> UOrder,
	VOrder = CSrf -> VOrder,
	ULength = CSrf -> ULength,
	VLength = CSrf -> VLength;
    CagdBType
	HasNewKV = FALSE;
    CagdVecStruct *N;
    CagdSrfStruct *TSrf, *OffsetSrf, *Srf;
    CagdRType *UNodes, *VNodes, *UNodePtr, *VNodePtr, **Points,
	*UKV = NULL,
	*VKV = NULL;

    switch (CSrf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = CagdSrfCopy(CSrf);
	    HasNewKV = TRUE;
	    UKV = BspKnotUniformOpen(ULength, UOrder, NULL);
	    VKV = BspKnotUniformOpen(VLength, VOrder, NULL);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    Srf = CagdCnvrtBsp2OpenSrf(CSrf);
	    UKV = Srf -> UKnotVector;
	    VKV = Srf -> VKnotVector;
	    break;
	case CAGD_SPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);
	    return NULL;
    }

    OffsetSrf = CagdSrfCopy(Srf);

    switch (OffsetSrf -> PType) {
        case CAGD_PT_E2_TYPE:
	    TSrf = CagdCoerceSrfTo(OffsetSrf, CAGD_PT_E3_TYPE, FALSE);
	    CagdSrfFree(OffsetSrf);
	    OffsetSrf = TSrf;
	    break;
        case CAGD_PT_P2_TYPE:
	    TSrf = CagdCoerceSrfTo(OffsetSrf, CAGD_PT_P3_TYPE, FALSE);
	    CagdSrfFree(OffsetSrf);
	    OffsetSrf = TSrf;
	    break;
	case CAGD_PT_E3_TYPE:
	case CAGD_PT_P3_TYPE:
	    break;
        default:
	    SYMB_FATAL_ERROR(SYMB_ERR_WRONG_PT_TYPE);
	    return NULL;
    }
    Points = OffsetSrf -> Points;
    MaxCoord = IRIT_MAX(CAGD_NUM_OF_PT_COORD(OffsetSrf -> PType), 3);

    UNodes = BspKnotNodes(UKV, ULength + UOrder, UOrder);
    VNodes = BspKnotNodes(VKV, VLength + VOrder, VOrder);

    if (IsNotRational)
	for (Row = 0, VNodePtr = VNodes; Row < VLength; Row++, VNodePtr++)
	    for (Col = 0, UNodePtr = UNodes; Col < ULength; Col++, UNodePtr++) {
	    	N = CagdSrfNormal(Srf, *UNodePtr, *VNodePtr, TRUE);
	    	for (i = 1; i <= MaxCoord; i++)
		    Points[i][CAGD_MESH_UV(OffsetSrf, Col, Row)] +=
			N -> Vec[i - 1] * OffsetDist;
	    }
    else
	for (Row = 0, VNodePtr = VNodes; Row < VLength; Row++, VNodePtr++)
	    for (Col = 0, UNodePtr = UNodes; Col < ULength; Col++, UNodePtr++) {
	    	N = CagdSrfNormal(Srf, *UNodePtr, *VNodePtr, TRUE);
	    	for (i = 1; i <= MaxCoord; i++)
	    	    Points[i][CAGD_MESH_UV(OffsetSrf, Col, Row)] +=
			N -> Vec[i - 1] * OffsetDist *
			    Points[SYMB_W][CAGD_MESH_UV(OffsetSrf, Col, Row)];
	    }

    if (HasNewKV) {
	IritFree(UKV);
	IritFree(VKV);
    }

    IritFree(UNodes);
    IritFree(VNodes);

    CagdSrfFree(Srf);

    return OffsetSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and an offset amount OffsetDist, returns an approximation  M
* to the offset surface by offseting the control mesh in the normal	     M
* direction.                                                                 M
*   If resulting offset is not satisfying the required tolerance the surface M
* is subdivided and the algorithm recurses on both parts.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:        To approximate its offset surface with distance OffsetDist. M
*   OffsetDist:  Amount of offset. Negative denotes other offset direction.  M
*   Tolerance:    Accuracy control.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An approximation to the offset surface, to within     M
*                      Tolerance.				             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfOffset, SymbCrvAdapOffset,    M
*   SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset, SymbCrvMatchingOffset    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSubdivOffset, offset                                              M
*****************************************************************************/
CagdSrfStruct *SymbSrfSubdivOffset(const CagdSrfStruct *CSrf,
				   CagdRType OffsetDist,
				   CagdRType Tolerance)
{
    CagdSrfStruct *Srf, *OffSrf, *Dist, *DistSqr;
    CagdRType *R, MinVal, MaxVal;
    CagdSrfDirType Dir;

    switch (CSrf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = CagdCnvrtBzr2BspSrf(CSrf);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    Srf = CagdCnvrtBsp2OpenSrf(CSrf);
	    break;
	case CAGD_SPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);
	    return NULL;
    }

    OffSrf = SymbSrfOffset(Srf, OffsetDist);
    Dist = SymbSrfSub(Srf, OffSrf);
    DistSqr = SymbSrfDotProd(Dist, Dist);

    CagdSrfFree(Dist);

    R = SymbExtremumCntPtVals(DistSqr -> Points,
			      DistSqr -> ULength * DistSqr -> VLength, TRUE);
    MinVal = R[1] < 0.0 ? 0.0 : sqrt(R[1]);
    R = SymbExtremumCntPtVals(DistSqr -> Points,
			      DistSqr -> ULength * DistSqr -> VLength, FALSE);
    MaxVal = R[1] < 0.0 ? 0.0 : sqrt(R[1]);

    CagdSrfFree(DistSqr);

    if (IRIT_FABS(MinVal - IRIT_FABS(OffsetDist)) > Tolerance ||
	IRIT_FABS(MaxVal - IRIT_FABS(OffsetDist)) > Tolerance) {
	CagdSrfStruct *Srf1, *Srf2, *OffSrf1, *OffSrf2;
	CagdRType UMin, UMax, VMin, VMax, t;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	if (IRIT_MAX(UMax - UMin, VMax - VMin) > NORMAL_PERTURB * 10.0) {
	    CagdSrfFree(OffSrf);

	    if (UMax - UMin > VMax - VMin) {
		Dir = CAGD_CONST_U_DIR;
		t = (UMin + UMax) * 0.5;
	    }
	    else {
		Dir = CAGD_CONST_V_DIR;
		t = (VMin + VMax) * 0.5;
	    }
	    Srf1 = CagdSrfSubdivAtParam(Srf, t, Dir);
	    Srf2 = Srf1 -> Pnext;
	    Srf1 -> Pnext = NULL;
	    OffSrf1 = SymbSrfSubdivOffset(Srf1, OffsetDist, Tolerance);
	    OffSrf2 = SymbSrfSubdivOffset(Srf2, OffsetDist, Tolerance);
	    CagdSrfFree(Srf1);
	    CagdSrfFree(Srf2);

	    OffSrf = CagdMergeSrfSrf(OffSrf1, OffSrf2, Dir, TRUE, TRUE);
	    CagdSrfFree(OffSrf1);
	    CagdSrfFree(OffSrf2);
	}
    }

    CagdSrfFree(Srf);

    return OffSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two parallel surfaces (a surface and its offsets or two offsets of M
* some surface, etc.) builds the (upto) four ruled surfaces that fills in    M
* the gaps on the Umin/max, Vmin/max boundaries.  Note that if UMin == UMax  M
* (VMin == VMax) no new surfaces will be added.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  The two surfaces to fill in their gapsbetween their         M
*                boundaries by new (ruled) surfaces.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Upto four surfaces that fill in the gaps between the  M
*		       boundaries of Srf1 and Srf2.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfCloseParallelSrfs2Shell                                           M
*****************************************************************************/
CagdSrfStruct *SymbSrfCloseParallelSrfs2Shell(const CagdSrfStruct *Srf1,
					      const CagdSrfStruct *Srf2)
{
    int i;
    CagdRType UMin, UMax, VMin, VMax;
    CagdCrvStruct *CrvUMin[2], *CrvUMax[2], *CrvVMin[2], *CrvVMax[2];
    const CagdSrfStruct * Srfs[2];
    CagdSrfStruct *Srf,
        *BndrySrfs = NULL;

    Srfs[0] = Srf1;
    Srfs[1] = Srf2;

    for (i = 0; i < 2; i++) {
        CagdSrfDomain(Srfs[i], &UMin, &UMax, &VMin, &VMax);
	CrvUMin[i] = CagdCrvFromSrf(Srfs[i], UMin, CAGD_CONST_U_DIR);
	CrvUMax[i] = CagdCrvFromSrf(Srfs[i], UMax, CAGD_CONST_U_DIR);
	CrvVMin[i] = CagdCrvFromSrf(Srfs[i], VMin, CAGD_CONST_V_DIR);
	CrvVMax[i] = CagdCrvFromSrf(Srfs[i], VMax, CAGD_CONST_V_DIR);
    }

    /* Examine if closed surfaces in U. */
    if (CagdCrvsSame(CrvUMin[0], CrvUMax[0], IRIT_EPS) &&
	CagdCrvsSame(CrvUMin[1], CrvUMax[1], IRIT_EPS)) {
        CagdCrvFree(CrvUMin[0]);
        CagdCrvFree(CrvUMax[0]);
        CagdCrvFree(CrvUMin[1]);
        CagdCrvFree(CrvUMax[1]);
	CrvUMin[0] = CrvUMax[0] = CrvUMin[1] = CrvUMax[1] = NULL;
    }

    /* Examine if closed surfaces in V. */
    if (CagdCrvsSame(CrvVMin[0], CrvVMax[0], IRIT_EPS) &&
	CagdCrvsSame(CrvVMin[1], CrvVMax[1], IRIT_EPS)) {
        CagdCrvFree(CrvVMin[0]);
        CagdCrvFree(CrvVMax[0]);
        CagdCrvFree(CrvVMin[1]);
        CagdCrvFree(CrvVMax[1]);
	CrvVMin[0] = CrvVMax[0] = CrvVMin[1] = CrvVMax[1] = NULL;
    }

    /* Build the (upto) four boundaries. Note we set the curves' order for */
    /* proper surface orientation's creation.				   */
    if (CrvUMin[0] != NULL && CrvUMin[1] != NULL) {
        if (CagdCrvArcLenPoly(CrvUMin[0]) > IRIT_EPS ||
	    CagdCrvArcLenPoly(CrvUMin[1]) > IRIT_EPS) {	    
	    Srf = CagdRuledSrf(CrvUMin[0], CrvUMin[1], 2, 2);
	    IRIT_LIST_PUSH(Srf, BndrySrfs);
	}
        CagdCrvFree(CrvUMin[0]);
        CagdCrvFree(CrvUMin[1]);
    }

    if (CrvUMax[0] != NULL && CrvUMax[1] != NULL) {
        if (CagdCrvArcLenPoly(CrvUMax[0]) > IRIT_EPS ||
	    CagdCrvArcLenPoly(CrvUMax[1]) > IRIT_EPS) {	    
	    Srf = CagdRuledSrf(CrvUMax[1], CrvUMax[0], 2, 2);
	    IRIT_LIST_PUSH(Srf, BndrySrfs);
	}
        CagdCrvFree(CrvUMax[0]);
        CagdCrvFree(CrvUMax[1]);
    }

    if (CrvVMin[0] != NULL && CrvVMin[1] != NULL) {
        if (CagdCrvArcLenPoly(CrvVMin[0]) > IRIT_EPS ||
	    CagdCrvArcLenPoly(CrvVMin[1]) > IRIT_EPS) {	    
	    Srf = CagdRuledSrf(CrvVMin[1], CrvVMin[0], 2, 2);
	    IRIT_LIST_PUSH(Srf, BndrySrfs);
	}
        CagdCrvFree(CrvVMin[0]);
        CagdCrvFree(CrvVMin[1]);
    }

    if (CrvVMax[0] != NULL && CrvVMax[1] != NULL) {
        if (CagdCrvArcLenPoly(CrvVMax[0]) > IRIT_EPS ||
	    CagdCrvArcLenPoly(CrvVMax[1]) > IRIT_EPS) {	    
	    Srf = CagdRuledSrf(CrvVMax[0], CrvVMax[1], 2, 2);
	    IRIT_LIST_PUSH(Srf, BndrySrfs);
	}
        CagdCrvFree(CrvVMax[0]);
        CagdCrvFree(CrvVMax[1]);
    }

    return BndrySrfs;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and an offset amount OffsetDist, returns an approximation to M
* the offset curve by offseting the control polygon in the normal direction. M
*   This function computes an approximation to the offset using		     M
* OffsetAprxFunc, measure the error and use it to refine and decrease the    M
* error adaptively.							     M
*   Bezier curves are promoted to Bsplines curves.			     M
*   See also: Gershon Elber and Elaine Cohen, "Error Bounded Variable	     M
* Distance Offset Operator for Free Form Curves and Surfaces". International M
* Journal of Computational Geometry & Applications, Vol. 1, Num. 1, March    M
* 1991, pp 67-78.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv:      To approximate its offset curve with distance OffsetDist.  M
*   OffsetDist:   Amount of offset. Negative denotes other offset direction. M
*   OffsetError:  Tolerance control.                                         M
*   OffsetAprxFunc:  A function that can be used to approximate an offset    M
*                    of a curve. If NULL SymbCrvOffset function is selected. M
*   BezInterp:    If TRUE, control points are interpolated when the curve is M
*		  reduced to a Bezier form. Otherwise, control points are    M
*		  translated OffsetDist amount only, under estimating the    M
*		  Offset. 						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the offset curve, to within       M
*                      OffsetError.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,  M
*   SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset, SymbCrvMatchingOffset,   M
*   SymbCrvVarOffset, SymbCrvAdapVarOffset				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvAdapOffset, offset                                                M
*****************************************************************************/
CagdCrvStruct *SymbCrvAdapOffset(const CagdCrvStruct *OrigCrv,
				 CagdRType OffsetDist,
				 CagdRType OffsetError,
				 SymbOffCrvFuncType OffsetAprxFunc,
				 CagdBType BezInterp)
{
    int i;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(OrigCrv);
    CagdRType Min, Max, TMin, TMax,
	OffsetDistSqr = IRIT_SQR(OffsetDist);
    CagdCrvStruct *Crv,
	*OffsetCrv = NULL;

    switch (OrigCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCnvrtBzr2BspCrv(OrigCrv);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCnvrtBsp2OpenCrv(OrigCrv);
	    break;
	case CAGD_CPOWER_TYPE:
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    Crv = NULL;
	    break;
    }

    if (OffsetAprxFunc == NULL)
	OffsetAprxFunc = SymbCrvOffset;

    CagdCrvDomain(Crv, &TMin, &TMax);

    for (i = 0; i < MAX_OFFSET_IMPROVE_ITERS; i++) {
	CagdCrvStruct *DiffCrv, *DistSqrCrv;

	if (OffsetCrv != NULL)
	    CagdCrvFree(OffsetCrv);
	OffsetCrv = OffsetAprxFunc(Crv, OffsetDist, BezInterp);

	DiffCrv = SymbCrvSub(OffsetCrv, Crv);
	DistSqrCrv = SymbCrvDotProd(DiffCrv, DiffCrv);
	CagdCrvFree(DiffCrv);

	CagdCrvMinMax(DistSqrCrv, 1, &Min, &Max);

	if (OffsetDistSqr - Min < OffsetError &&
	    Max - OffsetDistSqr < OffsetError) {
	    /* Error is within bounds - returns this offset approximation. */
	    CagdCrvFree(DistSqrCrv);
	    break;
	}
	else {
	    /* Refine in regions where the error is too large. */
	    int j, k,
	        Length = DistSqrCrv -> Length,
	        Order = DistSqrCrv -> Order,
	        KVLen = Length + Order;
	    CagdRType
		*KV = DistSqrCrv -> KnotVector,
		*Nodes = BspKnotNodes(KV, KVLen, Order),
		*RefKV = (CagdRType *) IritMalloc(sizeof(CagdRType) * 2 * Length);

	    for (j = k = 0; j < Length; j++) {
		CagdRType
		    *Pt = CagdCrvEval(DistSqrCrv, Nodes[j]),
		    V = OffsetDistSqr - (IsRational ? Pt[1] / Pt[0] : Pt[1]);

		if (IRIT_FABS(V) > OffsetError) {
		    int Index = BspKnotLastIndexLE(KV, KVLen, Nodes[j]);

		    if (IRIT_APX_EQ(KV[Index], Nodes[j])) {
			if (j > 0)
			    RefKV[k++] = (Nodes[j] + Nodes[j - 1]) * 0.5;
			if (j < Length - 1)
			    RefKV[k++] = (Nodes[j] + Nodes[j + 1]) * 0.5;
		    }
		    else
			RefKV[k++] = Nodes[j];
		}
	    }
	    CagdCrvFree(DistSqrCrv);
	    IritFree(Nodes);

	    if (k == 0) {
		/* No refinement was found needed - return current curve. */
		IritFree(RefKV);
		break;
	    }
	    else {
		CagdCrvStruct
		    *CTmp = CagdCrvRefineAtParams(Crv, FALSE, RefKV, k);

		IritFree(RefKV);
		CagdCrvFree(Crv);
		Crv = CTmp;
	    }
	}
    }

    CagdCrvFree(Crv);

    return OffsetCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and a scalar offset function VarOffsetDist, returns an       M
* approximation to the variable offset curve by offseting the control        M
* polygon in the normal direction.					     M
*   This function computes an approximation to the offset using		     M
* VarOffsetAprxFunc, measure the error and use it to refine and decrease the M
* error adaptively.							     M
*   Bezier curves are promoted to Bsplines curves.			     M
*   See also: Gershon Elber and Elaine Cohen, "Error Bounded Variable	     M
* Distance Offset Operator for Free Form Curves and Surfaces". International M
* Journal of Computational Geometry & Applications, Vol. 1, Num. 1, March    M
* 1991, pp 67-78.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv:       To approximate its offset curve with distance OffsetDist. M
*   VarOffsetDist:  A scalar distance function of the variable offset.	     M
*		    Must posses a parametric domain similar to OrigCrv.	     M
*   OffsetError:    Tolerance control.                                       M
*   VarOffsetAprxFunc: A function that can be used to approximate variable   M
*                   offset of a curve. If NULL SymbCrvVarOffset function is  M
*		    selected.						     M
*   BezInterp:      If TRUE, control points are interpolated when the curve  M
*		    is reduced to a Bezier form. Otherwise, control points   M
*		    are translated OffsetDist amount only, under estimating  M
*		    the Offset. 					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the offset curve, to within       M
*                      OffsetError.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvVarOffset, SymbCrvSubdivOffset, SymbCrvAdapOffset, M
*   SymbSrfOffset, SymbSrfSubdivOffset, SymbCrvAdapOffsetTrim,               M
*   SymbCrvLeastSquarOffset, SymbCrvMatchingOffset			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvAdapVarOffset, offset                                             M
*****************************************************************************/
CagdCrvStruct *SymbCrvAdapVarOffset(const CagdCrvStruct *OrigCrv,
				    const CagdCrvStruct *VarOffsetDist,
				    CagdRType OffsetError,
				    SymbVarOffCrvFuncType VarOffsetAprxFunc,
				    CagdBType BezInterp)
{
    int i;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(OrigCrv);
    CagdRType TMin, TMax;
    CagdCrvStruct *Crv,
	*OffsetCrv = NULL,
	*OffsetDistSqr = SymbCrvMult(VarOffsetDist, VarOffsetDist);

    switch (OrigCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCnvrtBzr2BspCrv(OrigCrv);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCnvrtBsp2OpenCrv(OrigCrv);
	    break;
	case CAGD_CPOWER_TYPE:
	default:
	    CagdCrvFree(OffsetDistSqr);
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    Crv = NULL;
	    break;
    }

    if (VarOffsetAprxFunc == NULL)
	VarOffsetAprxFunc = SymbCrvVarOffset;

    CagdCrvDomain(Crv, &TMin, &TMax);

    for (i = 0; i < MAX_OFFSET_IMPROVE_ITERS; i++) {
	CagdRType Min, Max;
	CagdCrvStruct *DiffCrv, *DistSqrCrv;

	if (OffsetCrv != NULL)
	    CagdCrvFree(OffsetCrv);
	OffsetCrv = VarOffsetAprxFunc(Crv, VarOffsetDist, BezInterp);

	DiffCrv = SymbCrvSub(OffsetCrv, Crv);
	DistSqrCrv = SymbCrvDotProd(DiffCrv, DiffCrv);
	CagdCrvFree(DiffCrv);

	DiffCrv = SymbCrvSub(DistSqrCrv, OffsetDistSqr);
	CagdCrvFree(DistSqrCrv);

	CagdCrvMinMax(DiffCrv, 1, &Min, &Max);

	if (IRIT_FABS(Min) < OffsetError && IRIT_FABS(Max) < OffsetError) {
	    /* Error is within bounds - returns this offset approximation. */
	    CagdCrvFree(DiffCrv);
	    break;
	}
	else {
	    /* Refine in regions where the error is too large. */
	    int j, k,
	        Length = DiffCrv -> Length,
	        Order = DiffCrv -> Order,
	        KVLen = Length + Order;
	    CagdRType
		*KV = DiffCrv -> KnotVector,
		*Nodes = BspKnotNodes(KV, KVLen, Order),
		*RefKV = (CagdRType *) IritMalloc(sizeof(CagdRType) * 2 * Length);

	    for (j = k = 0; j < Length; j++) {
		CagdRType
		    *R = CagdCrvEval(DiffCrv, Nodes[j]),
		    ErrSqr = IRIT_FABS(IsRational ? R[1] / R[0] : R[1]);

		if (ErrSqr > IRIT_SQR(OffsetError)) {
		    int Index = BspKnotLastIndexLE(KV, KVLen, Nodes[j]);

		    if (IRIT_APX_EQ(KV[Index], Nodes[j])) {
			if (j > 0)
			    RefKV[k++] = (Nodes[j] + Nodes[j - 1]) * 0.5;
			if (j < Length - 1)
			    RefKV[k++] = (Nodes[j] + Nodes[j + 1]) * 0.5;
		    }
		    else
			RefKV[k++] = Nodes[j];
		}
	    }
	    CagdCrvFree(DiffCrv);
	    IritFree(Nodes);

	    if (k == 0) {
		/* No refinement was found needed - return current curve. */
		IritFree(RefKV);
		break;
	    }
	    else {
		CagdCrvStruct
		    *CTmp = CagdCrvRefineAtParams(Crv, FALSE, RefKV, k);

		IritFree(RefKV);
		CagdCrvFree(Crv);
		Crv = CTmp;
	    }
	}
    }

    CagdCrvFree(Crv);
    CagdCrvFree(OffsetDistSqr);

    return OffsetCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Same function as CagdCrvAdapOffset, but trims the self intersection loops. M
*   See also: Gershon Elber and Elaine Cohen, "Error Bounded Variable	     M
* Distance Offset Operator for Free Form Curves and Surfaces". International M
* Journal of Computational Geometry & Applications, Vol. 1, Num. 1, March    M
* 1991, pp 67-78.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   OrigCrv:      To approximate its offset curve with distance OffsetDist.  M
*   OffsetDist:   Amount of offset. Negative denotes other offset direction. M
*   OffsetError:  Tolerance control.                                         M
*   OffsetAprxFunc:  A function that can be used to approximate an offset    M
*                    of a curve. If NULL SymbCrvOffset function is selected. M
*		     Third parameter of SymbOffCrvFuncType is optional.	     M
*   BezInterp:    If TRUE, control points are interpolated when the curve is M
*		  reduced to a Bezier form. Otherwise, control points are    M
*		  translated OffsetDist amount only, under estimating the    M
*		  Offset. 						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the offset curve, to within       M
*                      OffsetError.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,  M
*   SymbCrvAdapOffset, SymbCrvLeastSquarOffset, SymbCrvMatchingOffset        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvAdapOffsetTrim, offset                                            M
*****************************************************************************/
CagdCrvStruct *SymbCrvAdapOffsetTrim(const CagdCrvStruct *OrigCrv,
				     CagdRType OffsetDist,
				     CagdRType OffsetError,
				     SymbOffCrvFuncType OffsetAprxFunc,
				     CagdBType BezInterp)
{
    CagdBType
        IsRational = CAGD_IS_RATIONAL_CRV(OrigCrv);
    CagdCrvStruct *CrvtrSqr, *CrvtrSign, *Crv, *Crvs, *LastCrv,
	*CrvOffsetList = NULL;
    CagdPtStruct *Pts, *PtsHead, *LastPts;
    CagdRType
        OffsetDistSqr1 = 1.0 / IRIT_SQR(OffsetDist);

    switch (OrigCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCnvrtBzr2BspCrv(OrigCrv);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCnvrtBsp2OpenCrv(OrigCrv);
	    break;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
	    Crv = NULL;
	    break;
    }

    if (OffsetDist == 0.0)
	return Crv;

    CrvtrSqr = SymbCrv2DCurvatureSqr(Crv);
    CrvtrSign = SymbCrv2DCurvatureSign(Crv);

    PtsHead = SymbCrvConstSet(CrvtrSqr, 1, IRIT_SQR(OffsetError),
			      OffsetDistSqr1 / IRIT_SQR(OFFSET_TRIM_EPS), TRUE);

    for (Pts = PtsHead, LastPts = NULL; Pts != NULL;) {
	CagdRType *CrvtrSignPt, CrvtrSignVal;

	CrvtrSignPt = CagdCrvEval(CrvtrSign, Pts -> Pt[0]);
	CrvtrSignVal = IsRational ? CrvtrSignPt[1] / CrvtrSignPt[0]
				  : CrvtrSignPt[1];

	if (CrvtrSignVal * OffsetDist > 0.0) {
	    /* Remove point from list. */
	    if (LastPts != NULL) {
		LastPts -> Pnext = Pts -> Pnext;
		CagdPtFree(Pts);
		Pts = LastPts -> Pnext;
	    }
	    else {
		PtsHead = PtsHead -> Pnext;
		CagdPtFree(Pts);
		Pts = PtsHead;
	    }
	}
	else {
	    LastPts = Pts;
	    Pts = Pts -> Pnext;
	}
    }

    for (Pts = PtsHead;
	 Pts != NULL || Crv != NULL;
	 Pts = (Pts != NULL ? Pts -> Pnext : NULL)) {
	CagdCrvStruct
	    *Crv1 = Pts ? CagdCrvSubdivAtParam(Crv, Pts -> Pt[0])
			: CagdCrvCopy(Crv),
	    *Crv2 = Pts ? Crv1 -> Pnext : NULL;
	CagdRType Min, Max, *CrvtrSqrPt, CrvtrSqrVal,
						*CrvtrSignPt, CrvtrSignVal;

	CagdCrvDomain(Crv1, &Min, &Max);

	CrvtrSqrPt = CagdCrvEval(CrvtrSqr, (Min + Max) * 0.5);
	CrvtrSqrVal = CrvtrSqrPt[1] / CrvtrSqrPt[0];
	CrvtrSignPt = CagdCrvEval(CrvtrSign, (Min + Max) * 0.5);
	CrvtrSignVal = IsRational ? CrvtrSignPt[1] / CrvtrSignPt[0]
				  : CrvtrSignPt[1];

	if (CrvtrSqrVal < OffsetDistSqr1 / IRIT_SQR(OFFSET_TRIM_EPS) ||
	    CrvtrSignVal * OffsetDist > 0.0) {
	    CagdCrvStruct
		*Crv1Off = SymbCrvAdapOffset(Crv1, OffsetDist, OffsetError,
					     OffsetAprxFunc, BezInterp);

	    IRIT_LIST_PUSH(Crv1Off, CrvOffsetList);
	}

	CagdCrvFree(Crv1);
	CagdCrvFree(Crv);
	Crv = Crv2;
    }

    Crvs = CagdListReverse(CrvOffsetList);
    CrvOffsetList = NULL;
    LastCrv = Crvs;
    Crvs = Crvs -> Pnext;
    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	CagdCrvStruct *CTmp, *CTmp2;
	CagdPtStruct
	    *IPts = CagdCrvCrvInter(LastCrv, Crv, OffsetError);

	if (IPts != NULL) {
	    if (IPts -> Pnext) {
		SYMB_FATAL_ERROR(SYMB_ERR_TOO_COMPLEX);
		return NULL;
	    }
	    else {
		CagdRType TMin, TMax;

		CagdCrvDomain(LastCrv, &TMin, &TMax);
		CTmp = CagdCrvRegionFromCrv(LastCrv, TMin, IPts -> Pt[0]);
		CagdCrvFree(LastCrv);
		LastCrv = CTmp;

		CagdCrvDomain(Crv, &TMin, &TMax);
		CTmp = CagdCrvRegionFromCrv(Crv, IPts -> Pt[1], TMax);

		CTmp2 = CagdMergeCrvCrv(LastCrv, CTmp, FALSE);
		CagdCrvFree(CTmp);
		CagdCrvFree(LastCrv);
		LastCrv = CTmp2;
	    }

	    CagdPtFreeList(IPts);
	}
	else {
	    /* Simply chain the pieces together. */
	    CTmp = CagdMergeCrvCrv(LastCrv, Crv, FALSE);
	    CagdCrvFree(LastCrv);
	    LastCrv = CTmp;
	}
    }

    CagdPtFreeList(PtsHead);
    CagdCrvFreeList(Crvs);
    CagdCrvFree(CrvtrSqr);
    CagdCrvFree(CrvtrSign);

    return LastCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and an offset amount OffsetDist, returns an approximation to M
* the offset curve by least square fitting a curve to samples taken on the   M
* offset curve.								     M
*   Resulting curve of order Order (degree of Crv if Order == 0) will have   M
* NumOfDOF control points that least sqaure fit NumOfSamples samples on the  M
* offset curve.								     M
*   Tolerance will be updated to hold an error distance measure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          To approximate its offset curve with distance OffsetDist.  M
*   OffsetDist:   Amount of offset. Negative denotes other offset direction. M
*   NumOfSamples: Number of samples to sample the offset curve at.           M
*   NumOfDOF:     Number of degrees of freedom on the newly computed offset  M
*		  approximation. This is thesame as the number of control    M
*		  points the new curve will have.			     M
*   Order:        Of the newly constructed offset approximation. If equal to M
*		  zero, the order of Crv will be used.			     M
*   Tolerance:    To return an error estimate in the L-infinity norm.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An approximation to the offset curve.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,  M
*   SymbCrvAdapOffset, SymbCrvAdapOffsetTrim, SymbCrvMatchingOffset	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvLeastSquarOffset, offset                                          M
*****************************************************************************/
CagdCrvStruct *SymbCrvLeastSquarOffset(const CagdCrvStruct *Crv,
				       CagdRType OffsetDist,
				       int NumOfSamples,
				       int NumOfDOF,
				       int Order,
				       CagdRType *Tolerance)
{
    int i;
    CagdRType TMin, TMax, t, dt;
    CagdVType Tangent;
    CagdCrvStruct *OffApprox,
	*TangentCrv = CagdCrvDerive(Crv);
    CagdPtStruct
	*Pt = NULL,
	*PtList = NULL;

    /* Create NumOfSamples samples on the offset curve. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    dt = (TMax - TMin) / (NumOfSamples - 1);
    for (i = 0, t = TMin; i < NumOfSamples; i++, t += dt) {
	CagdRType *R;

	if (t > TMax)			 /* Take care of round off errors. */
	    t = TMax;

	if (PtList == NULL)
	    PtList = Pt = CagdPtNew();
	else {
	    Pt -> Pnext = CagdPtNew();
	    Pt = Pt -> Pnext;
	}

	R = CagdCrvEval(Crv, t);
	CagdCoerceToE3(Pt -> Pt, &R, -1, Crv -> PType);

	R = CagdCrvEval(TangentCrv, t);
	CagdCoerceToE2(Tangent, &R, -1, TangentCrv -> PType);
	Tangent[2] = 0.0;
	IRIT_PT_NORMALIZE(Tangent);
	
	Pt -> Pt[0] += Tangent[1] * OffsetDist;
	Pt -> Pt[1] -= Tangent[0] * OffsetDist;
    }

    OffApprox = BspCrvInterpPts(PtList,
				Order == 0 ? Crv -> Order : Order,
				IRIT_MIN(NumOfDOF, NumOfSamples),
				CAGD_UNIFORM_PARAM,
				Crv -> Periodic);

    *Tolerance = BspCrvInterpPtsError(OffApprox, PtList, CAGD_UNIFORM_PARAM,
				      Crv -> Periodic);

    CagdPtFreeList(PtList);
    CagdCrvFree(TangentCrv);

    return OffApprox;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Trims regions in the offset curve OffCrv that are closer than TrimAmount M
* to original Crv.  TrimAmount should be a fraction smaller than the offset  M
* amount itself.  See all:                                                   M
* Gershon Elber.  ``Trimming Local and Global Self-intersections in Offset   M
* Curves using Distance Maps.''	The 10th IMA conference on the Mathematics   M
* of Surfaces, Leeds, UK, pp 213-222, September 2003, LLCS2768.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          Original curve.                                            M
*   OffCrv:       The offset curve approximation.                            M
*   Tolerance:    Accuracy of computation.				     M
*   TrimAmount:   The trimming distance.  A fraction smaller than the offset M
*		  amount.						     M
*   NumerTol:     If Positive, a numerical marching improvement step is      M
*		  applied with NumerTol tolerance to the derived             M
*		  intersection/clipped regions.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of curve segments that are valid, after the     M
*		      trimming process took place.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCrvTrimGlblOffsetSelfInter                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvTrimGlblOffsetSelfInter                                           M
*****************************************************************************/
CagdCrvStruct *SymbCrvTrimGlblOffsetSelfInter(const CagdCrvStruct *Crv,
					      const CagdCrvStruct *OffCrv,
					      CagdRType Tolerance,
					      CagdRType TrimAmount,
					      CagdRType NumerTol)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, 0.0 };            /* A scalar srf - only X. */
    CagdRType TMin, TMax;
    CagdPtStruct *Pt, *ValidDomains;
    CagdCrvStruct *TCrv,
	*DCrv = NULL,
	*CrvList = NULL;
    CagdSrfStruct
	*DistSqr = SymbSrfDistCrvCrv(OffCrv, Crv, 1);
    IPPolygonStruct *Cntrs;

    Plane[3] = -IRIT_SQR(CONTOUR_EPS + TrimAmount);
    Cntrs = UserCntrSrfWithPlane(DistSqr, Plane, Tolerance);

    CagdSrfFree(DistSqr);

    ValidDomains = ExtractValidDomainFromContours(Cntrs);
    IPFreePolygonList(Cntrs);

    CagdCrvDomain(Crv, &TMin, &TMax);

    if (NumerTol > 0.0) {
	int Idx1, Idx2;
	CagdPtStruct *Pt1, *Pt2;

	DCrv = CagdCrvDerive(OffCrv);

	/* Mark the end points if on the original boundary as unused. */
	if (IRIT_APX_EQ(ValidDomains -> Pt[0], TMin))
	    AttrSetIntAttrib(&ValidDomains -> Attr, "_MtchedStrt", TRUE);
	Pt1 = CagdListLast(ValidDomains);
	if (IRIT_APX_EQ(Pt1 -> Pt[0], TMax))
	    AttrSetIntAttrib(&Pt1 -> Attr, "_MtchedEnd", TRUE);

	/* Improve as much as we can and while we can. */
        while (GetMatchedTrimParameter(OffCrv, ValidDomains,
				       &Pt1, &Idx1, &Pt2, &Idx2)) {
    	    OffsetGlblTrimUpdateInter(OffCrv, DCrv, &Pt1 -> Pt[Idx1],
				      &Pt2 -> Pt[Idx2], NumerTol);
	}
    }

    for (Pt = ValidDomains; Pt != NULL; Pt = Pt -> Pnext) {
	TCrv = CagdCrvRegionFromCrv(OffCrv, Pt -> Pt[0], Pt -> Pt[1]);

	IRIT_LIST_PUSH(TCrv, CrvList);
    }

    CagdPtFreeList(ValidDomains);

    if (NumerTol > 0.0)
	CagdCrvFree(DCrv);

    return CrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Get the closest pair among all possible matches to try and improve it    *
* numerically.  Closest is measured in Euclidean space.                      *
*   The end points locations are ignored as possible macthes.                *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       The curve to look for closest point.                          *
*   Pts:       List of points to search for best match.			     *
*   Pt1:       First matched entry.					     *
*   Idx1:      Index in the first matched entry, either 0 or 1.		     *
*   Pt2:       Second matched entry.					     *
*   Idx2:      Index in the second matched entry, either 0 or 1.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE of found match, FALSE otherwise.                         *
*****************************************************************************/
static int GetMatchedTrimParameter(const CagdCrvStruct *Crv,
				   CagdPtStruct *Pts,
				   CagdPtStruct **Pt1,
				   int *Idx1,
				   CagdPtStruct **Pt2,
				   int *Idx2)
{
    int MinIndx1 = -1,
	MinIndx2 = -1;
    CagdPtStruct *PtFirst, *PtSecond,
	*MinPt1 = NULL,
	*MinPt2 = NULL;
    CagdRType DistSqr, *R,
	MinDistSqr = IRIT_INFNTY;
    CagdPType PtE2Tmp1, PtE2Tmp2;

    for (PtFirst = Pts; PtFirst != NULL; PtFirst = PtFirst -> Pnext) {
	for (PtSecond = PtFirst -> Pnext;
	     PtSecond != NULL;
	     PtSecond = PtSecond -> Pnext) {
	    if (AttrGetIntAttrib(PtFirst -> Attr, "_MtchedStrt") != TRUE &&
		AttrGetIntAttrib(PtSecond -> Attr, "_MtchedStrt") != TRUE) {
	        R = CagdCrvEval(Crv, PtFirst -> Pt[0]);
		CagdCoerceToE2(PtE2Tmp1, &R, -1, Crv -> PType);
		R = CagdCrvEval(Crv, PtSecond -> Pt[0]);
		CagdCoerceToE2(PtE2Tmp2, &R, -1, Crv -> PType);

		DistSqr = IRIT_PT2D_DIST_SQR(PtE2Tmp1, PtE2Tmp2);
		if (MinDistSqr > DistSqr) {
		    MinDistSqr = DistSqr;
		    MinIndx1 = 0;
		    MinIndx2 = 0;
		    MinPt1 = PtFirst;
		    MinPt2 = PtSecond;
		}
	    }
	    if (AttrGetIntAttrib(PtFirst -> Attr, "_MtchedStrt") != TRUE &&
		AttrGetIntAttrib(PtSecond -> Attr, "_MtchedEnd") != TRUE) {
	        R = CagdCrvEval(Crv, PtFirst -> Pt[0]);
		CagdCoerceToE2(PtE2Tmp1, &R, -1, Crv -> PType);
		R = CagdCrvEval(Crv, PtSecond -> Pt[1]);
		CagdCoerceToE2(PtE2Tmp2, &R, -1, Crv -> PType);

		DistSqr = IRIT_PT2D_DIST_SQR(PtE2Tmp1, PtE2Tmp2);
		if (MinDistSqr > DistSqr) {
		    MinDistSqr = DistSqr;
		    MinIndx1 = 0;
		    MinIndx2 = 1;
		    MinPt1 = PtFirst;
		    MinPt2 = PtSecond;
		}
	    }
	    if (AttrGetIntAttrib(PtFirst -> Attr, "_MtchedEnd") != TRUE &&
		AttrGetIntAttrib(PtSecond -> Attr, "_MtchedEnd") != TRUE) {
	        R = CagdCrvEval(Crv, PtFirst -> Pt[1]);
		CagdCoerceToE2(PtE2Tmp1, &R, -1, Crv -> PType);
		R = CagdCrvEval(Crv, PtSecond -> Pt[1]);
		CagdCoerceToE2(PtE2Tmp2, &R, -1, Crv -> PType);

		DistSqr = IRIT_PT2D_DIST_SQR(PtE2Tmp1, PtE2Tmp2);
		if (MinDistSqr > DistSqr) {
		    MinDistSqr = DistSqr;
		    MinIndx1 = 1;
		    MinIndx2 = 1;
		    MinPt1 = PtFirst;
		    MinPt2 = PtSecond;
		}
	    }
	    if (AttrGetIntAttrib(PtFirst -> Attr, "_MtchedEnd") != TRUE &&
		AttrGetIntAttrib(PtSecond -> Attr, "_MtchedStrt") != TRUE) {
	        R = CagdCrvEval(Crv, PtFirst -> Pt[1]);
		CagdCoerceToE2(PtE2Tmp1, &R, -1, Crv -> PType);
		R = CagdCrvEval(Crv, PtSecond -> Pt[0]);
		CagdCoerceToE2(PtE2Tmp2, &R, -1, Crv -> PType);

		DistSqr = IRIT_PT2D_DIST_SQR(PtE2Tmp1, PtE2Tmp2);
		if (MinDistSqr > DistSqr) {
		    MinDistSqr = DistSqr;
		    MinIndx1 = 1;
		    MinIndx2 = 0;
		    MinPt1 = PtFirst;
		    MinPt2 = PtSecond;
		}
	    }
        }
    }

    if (MinPt1 != NULL && MinPt2 != NULL) {
	if (MinIndx1 == 1)
	    AttrSetIntAttrib(&MinPt1 -> Attr, "_MtchedEnd", TRUE);
	else
	    AttrSetIntAttrib(&MinPt1 -> Attr, "_MtchedStrt", TRUE);

	if (MinIndx2 == 1)
	    AttrSetIntAttrib(&MinPt2 -> Attr, "_MtchedEnd", TRUE);
	else
	    AttrSetIntAttrib(&MinPt2 -> Attr, "_MtchedStrt", TRUE);

	*Idx1 = MinIndx1;
	*Idx2 = MinIndx2;
	*Pt1 = MinPt1;
	*Pt2 = MinPt2;

	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given the Cntrs that signal the invalid domain, derives this invalid     *
* domain as a list of 2D points.  Each returned points signals one remaining *
* valid domain.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Cntrs:    The contours that signal the invalid domain.  The contours     *
*	      contain the distance square in the first component (X) and the *
*	      U and V as the YZ point coordinates.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *:  Points that prescribe the remaining valid domains.      *
*****************************************************************************/
static CagdPtStruct *ExtractValidDomainFromContours(IPPolygonStruct *Cntrs)
{
    IPPolygonStruct *Cntr;
    CagdPtStruct
	*ValidDomain = CagdPtNew();

    ValidDomain -> Pt[0] = 0.0;
    ValidDomain -> Pt[1] = 1.0;

    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
	IPVertexStruct *V;
	CagdRType
	    TMin = IRIT_INFNTY,
	    TMax = -IRIT_INFNTY;

	/* Get the U domain for this contour. */
	for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
	    if (TMin > V -> Coord[1])
	        TMin = V -> Coord[1];
	    if (TMax < V -> Coord[1])
	        TMax = V -> Coord[1];
	}

	if (TMin < TMax) {
#	    ifdef DUMP_TRIM_CNTR_DOMAINS
	        IRIT_INFO_MSG_PRINTF("TMin = %f, TMax = %f\n", TMin, TMax);
#	    endif /* DUMP_TRIM_CNTR_DOMAINS */

	    /* Update this U invalid domain into the return valid list. */
	    InsertInvalidDomainIntoList(&ValidDomain, TMin, TMax);
	}
    }

    return ValidDomain;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Inserts one new invalid domain from TMin to TMax to the list of valid    *
* domains PtList.			                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:      Current list of valid domains, to update in place.          *
*   TMin, TMax:  New invalid domain to insert.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertInvalidDomainIntoList(CagdPtStruct **PtList,
					CagdRType TMin,
					CagdRType TMax)
{
    CagdPtStruct Pts, *Pt;

    if (TMax <= TMin)
	return;

    Pts.Pnext = *PtList;

    for (Pt = &Pts;
	 Pt -> Pnext && TMin > Pt -> Pnext -> Pt[1];
	 Pt = Pt -> Pnext);

    InsertInvalidDomainIntoListAux(Pt, TMin, TMax);

    *PtList = Pts.Pnext;
}

/*****************************************************************************
* AUXILIARY:   		                                                     *
*   Auxiliary function for  InsertInvalidDomainIntoList                      *
*   Pre-condition Pt -> Pt[1] < TMin <= Pt -> Pnext -> Pt[1]                 * 
*****************************************************************************/
static void InsertInvalidDomainIntoListAux(CagdPtStruct *Pt,
					   CagdRType TMin,
					   CagdRType TMax)
{
    CagdPtStruct *Pt1;

    if (!Pt -> Pnext || TMax < Pt -> Pnext -> Pt[0])
	return;

    if (Pt -> Pnext -> Pt[0] < TMin) {
	Pt1 = CagdPtNew();
	Pt1 -> Pt[0] = Pt -> Pnext -> Pt[0];
	Pt1 -> Pt[1] = TMin;
	Pt1 -> Pnext = Pt -> Pnext;
	Pt -> Pnext = Pt1;
	Pt = Pt -> Pnext;
    }

    if (TMax < Pt -> Pnext -> Pt[1])
	Pt -> Pnext -> Pt[0] = TMax;
    else {
	Pt1 = Pt -> Pnext;
	Pt -> Pnext = Pt -> Pnext -> Pnext;
	CagdPtFree(Pt1);
	InsertInvalidDomainIntoListAux(Pt, TMin, TMax);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Apply a numerical marching (Newton Raphson) step to improve the	     *
* intersection points.	Curves are assumed planar.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:     Original, complete, offset curve.                               *
*   DCrv:    Thederivative curve, for tangent evaluations.                   *
*   OrigT1:  Last parameter of first curve segment from Crv in intersection. *
*   OrigT2:  First parameter of next curve segment from Crv in intersection. *
*   Eps:     Tolerance of numerical improvement.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if we improved the intersection location, FALSE otherwise.    *
*****************************************************************************/
static int OffsetGlblTrimUpdateInter(const CagdCrvStruct *Crv,
				     const CagdCrvStruct *DCrv,
				     CagdRType *OrigT1,
				     CagdRType *OrigT2,
				     CagdRType Eps)
{
    CagdRType TMin, TMax, *R,
	EpsSqr = IRIT_SQR(Eps),
	StartDistSqr = IRIT_INFNTY,
	CrntDistSqr = IRIT_INFNTY,
	t1 = *OrigT1,
	t2 = *OrigT2;
    CagdPType Pt1, Pt2;
    CagdVType Tan1, Tan2;

    CagdCrvDomain(Crv, &TMin, &TMax);

    Pt1[2] = Pt2[2] = Tan1[2] = Tan2[2] = 0.0;

    while (TRUE) {
	CagdRType DistSqr, Inter1Param, Inter2Param;
	CagdPType Inter1, Inter2;

	R = CagdCrvEval(Crv, t1);
	CagdCoerceToE2(Pt1, &R, -1, Crv -> PType);

	R = CagdCrvEval(Crv, t2);
	CagdCoerceToE2(Pt2, &R, -1, Crv -> PType);

	DistSqr = IRIT_PT2D_DIST_SQR(Pt1, Pt2);
	if (StartDistSqr == IRIT_INFNTY)		      /* First time. */
	    StartDistSqr = DistSqr;

	if (DistSqr < EpsSqr) {
	    /* Done - found accurate intersection point. */
	    *OrigT1 = t1;
	    *OrigT2 = t2;
	    return TRUE;
	}
	else if (CrntDistSqr < DistSqr) {              /* Failed to improve. */
	    if (StartDistSqr > DistSqr) {
	        *OrigT1 = t1;  /* Return somewhat better intersection point. */
		*OrigT2 = t2;
		return TRUE;
	    }
	    else
	        return FALSE;
	}
	else {
	    CrntDistSqr = DistSqr;
	}

	R = CagdCrvEval(DCrv, t1);
	CagdCoerceToE2(Tan1, &R, -1, DCrv -> PType);

	R = CagdCrvEval(DCrv, t2);
	CagdCoerceToE2(Tan2, &R, -1, DCrv -> PType);

	/* Solve for the intersection point and update the parameter values. */
	if (!GM2PointsFromLineLine(Pt1, Tan1, Pt2, Tan2,
				   Inter1, &Inter1Param,
				   Inter2, &Inter2Param))
	    return FALSE;

	t1 += Inter1Param;
	t1 = IRIT_BOUND(t1, TMin, TMax);

	t2 += Inter2Param;
	t2 = IRIT_BOUND(t2, TMin, TMax);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reports if the given offset curve OffCrv to given curve Crv contains     M
* local self intersections.                                                  M
*   Solution is the zeros of <Crv', OffCrv'>.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     Original curve.                                                 M
*   OffCrv:  Offset (approximation) curve.  Assumed to share the same        M
*            parametrization with Crv. I.e. Crv(t0) is offset to OffCrv(t0). M
*   SIDmns:  If not NULL set to domains that are in the self intersections.  M
*              Each consecutive set of two points in this list defines one   M
*            such domain.					             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if has local self intersection, FALSE otherwise.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsOffsetLclSelfInters                                                M
*****************************************************************************/
int SymbIsOffsetLclSelfInters(CagdCrvStruct *Crv,
			      CagdCrvStruct *OffCrv,
			      CagdPtStruct **SIDmns)
{
    CagdCrvStruct
        *DCrv = CagdCrvDerive(Crv),
        *DOffCrv = CagdCrvDerive(OffCrv),
	*DProdCrv = SymbCrvDotProd(DOffCrv, DCrv);
    CagdPtStruct *Dmns;

    if (SIDmns != NULL)
        *SIDmns = NULL;

    Dmns = SymbCrvZeroSet(DProdCrv, 1, IRIT_EPS, TRUE);

    CagdCrvFree(DCrv);
    CagdCrvFree(DOffCrv);
    CagdCrvFree(DProdCrv);

    if (Dmns != NULL) {
        if (SIDmns == NULL) {
	    CagdPtFreeList(Dmns);
	}
	else
	    *SIDmns = Dmns;

	return TRUE;
    }
    else
        return FALSE;
}
