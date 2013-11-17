/******************************************************************************
* Bsp-Gen.c - Bspline generic routines.					      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

#define CAGD_MESH_CONT_LENRATIO_EPS	0.05
#define CAGD_MESH_CONT_ANGULAR_EPS	0.99

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new Bspline surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:      Number of control points in the U direction.               M
*   VLength:      Number of control points in the V direction.               M
*   UOrder:       The order of the surface in the U direction.               M
*   VOrder:       The order of the surface in the V direction.               M
*   PType:        Type of control points (E2, P3, etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An uninitialized freeform Bspline surface.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfNew, BspPeriodicSrfNew, CagdSrfNew, CagdPeriodicSrfNew, TrimSrfNew M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfNew, allocation                                                    M
*****************************************************************************/
CagdSrfStruct *BspSrfNew(int ULength,
			 int VLength,
			 int UOrder,
			 int VOrder,
			 CagdPointType PType)
{
    CagdSrfStruct *Srf;

    if (ULength < UOrder || VLength < VOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }

    Srf = CagdSrfNew(CAGD_SBSPLINE_TYPE, PType, ULength, VLength);

    Srf -> UKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
							   (UOrder + ULength));
    Srf -> VKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
							   (VOrder + VLength));

    Srf -> UOrder = UOrder;
    Srf -> VOrder = VOrder;

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new, possibly periodic, Bspline      M
* surface.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   ULength:      Number of control points in the U direction.               M
*   VLength:      Number of control points in the V direction.               M
*   UOrder:       The order of the surface in the U direction.               M
*   VOrder:       The order of the surface in the V direction.               M
*   UPeriodic:    Is this surface periodic in the U direction?               M
*   VPeriodic:    Is this surface periodic in the V direction?               M
*   PType:        Type of control points (E2, P3, etc.).                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An uninitialized freeform Bspline surface. If both    M
*                      UPeriodic and VPeriodic are FALSE, this function is   M
*                      identical to BspSrfNew.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfNew, BzrSrfNew, CagdSrfNew, CagdPeriodicSrfNew, TrimSrfNew         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspPeriodicSrfNew, allocation                                            M
*****************************************************************************/
CagdSrfStruct *BspPeriodicSrfNew(int ULength,
				 int VLength,
				 int UOrder,
				 int VOrder,
				 CagdBType UPeriodic,
				 CagdBType VPeriodic,
				 CagdPointType PType)
{
    CagdSrfStruct *Srf;

    if (ULength < UOrder || VLength < VOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }

    Srf = CagdPeriodicSrfNew(CAGD_SBSPLINE_TYPE, PType, ULength, VLength,
			     UPeriodic, VPeriodic);

    Srf -> UKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
			   (UOrder + ULength + (UPeriodic ? UOrder - 1 : 0)));
    Srf -> VKnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
			   (VOrder + VLength + (VPeriodic ? VOrder - 1 : 0)));

    Srf -> UOrder = UOrder;
    Srf -> VOrder = VOrder;

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new Bspline curve.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points                                     M
*   Order:      The order of the curve                                       M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An uninitialized freeform Bspline curve.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvNew, BspPeriodicCrvNew, CagdCrvNew, CagdPeriodicCrvNew, TrimCrvNew M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvNew, allocation                                                    M
*****************************************************************************/
CagdCrvStruct *BspCrvNew(int Length, int Order, CagdPointType PType)
{
    CagdCrvStruct *Crv;

    if (Length < Order) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }

    Crv = CagdCrvNew(CAGD_CBSPLINE_TYPE, PType, Length);

    Crv -> KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
							     (Order + Length));

    Crv -> Order = Order;

    if (Crv -> Order == 2)
	CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_LINEAR);
    else if (Crv -> Order == 1)
	CAGD_SET_GEOM_TYPE(Crv, CAGD_GEOM_CONST);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates the memory required for a new, possibly periodic, Bspline      M
* curve.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:     Number of control points                                     M
*   Order:      The order of the curve                                       M
*   Periodic:   Is this curve periodic?                                      M
*   PType:      Type of control points (E2, P3, etc.).                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An uninitialized freeform Bspline curve. If Periodic  M
*                      is FALSE, this function is identical to BspCrvNew.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvNew, BspCrvNew, CagdCrvNew, CagdPeriodicCrvNew, TrimCrvNew	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspPeriodicCrvNew, allocation                                            M
*****************************************************************************/
CagdCrvStruct *BspPeriodicCrvNew(int Length,
				 int Order,
				 CagdBType Periodic,
				 CagdPointType PType)
{
    CagdCrvStruct *Crv;

    if (Length < Order) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }

    Crv = CagdPeriodicCrvNew(CAGD_CBSPLINE_TYPE, PType, Length, Periodic);

    Crv -> KnotVector = (CagdRType *) IritMalloc(sizeof(CagdRType) *
				(Order + Length + (Periodic ? Order - 1 : 0)));

    Crv -> Order = Order;

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the parametric domain of a Bspline curve.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To get its parametric domain.                                 M
*   TMin:      Where to put the minimal domain's boundary.                   M
*   TMax:      Where to put the maximal domain's boundary.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvDomain                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvDomain, domain, parametric domain                                  M
*****************************************************************************/
void BspCrvDomain(const CagdCrvStruct *Crv, CagdRType *TMin, CagdRType *TMax)
{
    int k = Crv -> Order,
	Len = CAGD_CRV_PT_LST_LEN(Crv);

    *TMin = Crv -> KnotVector[k - 1];
    *TMax = Crv -> KnotVector[Len];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the parametric domain of a Bspline surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To get its parametric domain.                                 M
*   UMin:      Where to put the minimal U domain's boundary.                 M
*   UMax:      Where to put the maximal U domain's boundary.                 M
*   VMin:      Where to put the minimal V domain's boundary.                 M
*   VMax:      Where to put the maximal V domain's boundary.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDomain, TrimSrfDomain                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfDomain, domain, parametric domain                                  M
*****************************************************************************/
void BspSrfDomain(const CagdSrfStruct *Srf,
		  CagdRType *UMin,
		  CagdRType *UMax,
		  CagdRType *VMin,
		  CagdRType *VMax)
{
    int UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	ULen = CAGD_SRF_UPT_LST_LEN(Srf),
	VLen = CAGD_SRF_VPT_LST_LEN(Srf);

    *UMin = Srf -> UKnotVector[UOrder - 1];
    *UMax = Srf -> UKnotVector[ULen];
    *VMin = Srf -> VKnotVector[VOrder - 1];
    *VMax = Srf -> VKnotVector[VLen];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a curve with open end conditions, similar to given curve.	     M
*   Open end curve is computed by extracting a subregion from Crv that is    M
* the entire curve's parametric domain, by inserting multiple knots at the   M
* domain's boundary.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     To convert to a new curve with open end conditions.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: Same curve as Crv but with open end conditions.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfOpenEnd                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvOpenEnd, open end conditions                                       M
*****************************************************************************/
CagdCrvStruct *BspCrvOpenEnd(const CagdCrvStruct *Crv)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *OpenCrv, *TCrv;

    CagdCrvDomain(Crv, &TMin, &TMax);

    if (CAGD_IS_PERIODIC_CRV(Crv)) {
        TCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
	OpenCrv = CagdCrvRegionFromCrv(TCrv, TMin, TMax);
	CagdCrvFree(TCrv);
    }
    else
        OpenCrv = CagdCrvRegionFromCrv(Crv, TMin, TMax);

    CAGD_PROPAGATE_ATTR(OpenCrv, Crv);

    return OpenCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns a surface with open end conditions, similar to given surface.    M
*   Open end surface is computed by extracting a subregion from Srf that is  M
* the entire surface's parametric domain, by inserting multiple knots at the M
* domain's boundary.                                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To convert to a new surface with open end conditions.  Input    M
*	     can also be periodic.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: Same surface as Srf but with open end conditions.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvOpenEnd                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfOpenEnd, open end conditions                                       M
*****************************************************************************/
CagdSrfStruct *BspSrfOpenEnd(const CagdSrfStruct *Srf)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *TSrf, *TSrf2, *OpenSrf;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    if (CAGD_IS_PERIODIC_SRF(Srf)) {
        TSrf2 = CagdCnvrtPeriodic2FloatSrf(Srf);
	TSrf = CagdSrfRegionFromSrf(TSrf2, UMin, UMax, CAGD_CONST_U_DIR);
	CagdSrfFree(TSrf2);
    }
    else
        TSrf = CagdSrfRegionFromSrf(Srf, UMin, UMax, CAGD_CONST_U_DIR);

    OpenSrf = CagdSrfRegionFromSrf(TSrf, VMin, VMax, CAGD_CONST_V_DIR);

    CagdSrfFree(TSrf);

    CAGD_PROPAGATE_ATTR(OpenSrf, Srf);

    return OpenSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scans the given knot vector of the given curve for a potential C0        M
* discontinuity.							     M
*   Looks for multiplicities in the knot sequence and then examine the mesh  M
* if indeed the mesh is discontinuous at that location.			     M
*   Assumes knot vectors has open end condition.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To examine its potential discontinuity.			     M
*   t:         Where to put the parameter value (knot) that can be C0        M
*              discontinuous.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a C0 discontinuity, FALSE otherwise.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvKnotC1Discont, BspCrvKnotC2Discont, BspKnotC0Discont,		     M
*   BspCrvMeshC1Continuous, BspCrvKnotC1Discont			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvKnotC0Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdBType BspCrvKnotC0Discont(const CagdCrvStruct *Crv, CagdRType *t)
{
    if (CAGD_IS_BEZIER_CRV(Crv) || CAGD_IS_POWER_CRV(Crv))
	return FALSE;
    else if (CAGD_IS_BSPLINE_CRV(Crv))
	return BspKnotC0Discont(Crv -> KnotVector, Crv -> Order, Crv -> Length, t);
    else
	return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scans the given knot vector of the given curve for a potential C1        M
* discontinuity.							     M
*   Looks for multiplicities in the knot sequence and then examine the mesh  M
* if indeed the mesh is discontinuous at that location.			     M
*   Assumes knot vectors has open end condition.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To examine its potential discontinuity.			     M
*   t:         Where to put the parameter value (knot) that can be C1        M
*              discontinuous.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a C1 discontinuity, FALSE otherwise.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvKnotC0Discont, BspCrvKnotC2Discont, BspKnotC1Discont,              M
*   BspCrvMeshC1Continuous					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvKnotC1Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdBType BspCrvKnotC1Discont(const CagdCrvStruct *Crv, CagdRType *t)
{
    if (CAGD_IS_BEZIER_CRV(Crv) || CAGD_IS_POWER_CRV(Crv))
	return FALSE;
    else if (CAGD_IS_BSPLINE_CRV(Crv))
	return BspKnotC1Discont(Crv -> KnotVector, Crv -> Order, Crv -> Length, t);
    else
	return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scans the given knot vector of the given curve for a potential C2        M
* discontinuity.							     M
*   Looks for multiplicities in the knot sequence and then examine the mesh  M
* if indeed the mesh is discontinuous at that location.			     M
*   Assumes knot vectors has open end condition.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To examine its potential discontinuity.			     M
*   t:         Where to put the parameter value (knot) that can be C1        M
*              discontinuous.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a C2 discontinuity, FALSE otherwise.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvKnotC0Discont, BspCrvKnotC1Discont, BspKnotC1Discont,              M
*   BspCrvMeshC1Continuous					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvKnotC2Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdBType BspCrvKnotC2Discont(const CagdCrvStruct *Crv, CagdRType *t)
{
    if (CAGD_IS_BEZIER_CRV(Crv) || CAGD_IS_POWER_CRV(Crv))
	return FALSE;
    else if (CAGD_IS_BSPLINE_CRV(Crv))
        return BspKnotC2Discont(Crv -> KnotVector, Crv -> Order, Crv -> Length, t);
    else
	return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examine the control polygon of the given curve in index Idx for a real   M
* C1 discontinuity in the mesh.				                     M
*   This index will typically be for a knot multiplicity potential discont.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To examine its potential discontinuity.		             M
*   Idx:       Index where to examine the discontinuity.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if continuous there, FALSE otherwise.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotC1Discont                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvMeshC1Continuous                                                   M
*****************************************************************************/
CagdBType BspCrvMeshC1Continuous(const CagdCrvStruct *Crv, int Idx)
{
    CagdRType
        * const *Points = Crv -> Points;
    CagdPType Pt0, Pt1, Pt2;
    CagdVType V1, V2;
    CagdPointType
	PType = Crv -> PType;

    CagdCoerceToE3(Pt0, Points, Idx - 1, PType);
    CagdCoerceToE3(Pt1, Points, Idx,     PType);
    CagdCoerceToE3(Pt2, Points, Idx + 1, PType);

    IRIT_PT_SUB(V1, Pt0, Pt1);
    IRIT_PT_SUB(V2, Pt1, Pt2);

    IRIT_VEC_NORMALIZE(V1);
    IRIT_VEC_NORMALIZE(V2);

    return IRIT_DOT_PROD(V1, V2) > CAGD_MESH_CONT_ANGULAR_EPS;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector of the given surface for a potential C0        M
* discontinuity.							     M
*   Looks for multiplicities in the knot sequence and then examine the mesh  M
* if indeed the mesh is discontinuous at that location.			     M
*   Assumes knot vectors has open end condition.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To examine its potential discontinuity across Dir.            M
*   Dir:       Direction to examine the discontinuity across.                M
*   t:         Where to put the parameter value (knot) that can be C0        M
*              discontinuous.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a C0 discontinuity, FALSE otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfKnotC1Discont, BspKnotC1Discont, BspSrfMeshC1Continuous            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfKnotC0Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdBType BspSrfKnotC0Discont(const CagdSrfStruct *Srf,
			      CagdSrfDirType Dir,
			      CagdRType *t)
{
    int Order = Dir == CAGD_CONST_U_DIR ? Srf -> UOrder : Srf -> VOrder,
	Length = Dir == CAGD_CONST_U_DIR ? Srf -> ULength : Srf -> VLength;
    CagdRType
 	*KV = Dir == CAGD_CONST_U_DIR ? Srf -> UKnotVector
				      : Srf -> VKnotVector;

    return BspKnotC0Discont(KV, Order, Length, t);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Scans the given knot vector of the given surface for a potential C1        M
* discontinuity.							     M
*   Looks for multiplicities in the knot sequence and then examine the mesh  M
* if indeed the mesh is discontinuous at that location.			     M
*   Assumes knot vectors has open end condition.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To examine its potential discontinuity across Dir.            M
*   Dir:       Direction to examine the discontinuity across.                M
*   t:         Where to put the parameter value (knot) that can be C1        M
*              discontinuous.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:     TRUE if found a C1 discontinuity, FALSE otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfKnotC0Discont, BspKnotC1Discont, BspSrfMeshC1Continuous            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfKnotC1Discont, knot vectors, continuity, discontinuity             M
*****************************************************************************/
CagdBType BspSrfKnotC1Discont(const CagdSrfStruct *Srf,
			      CagdSrfDirType Dir,
			      CagdRType *t)
{
    int Order = Dir == CAGD_CONST_U_DIR ? Srf -> UOrder : Srf -> VOrder,
	Length = Dir == CAGD_CONST_U_DIR ? Srf -> ULength : Srf -> VLength;
    CagdRType
 	*KV = Dir == CAGD_CONST_U_DIR ? Srf -> UKnotVector
				      : Srf -> VKnotVector;

    return BspKnotC1Discont(KV, Order, Length, t);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examine the mesh of the given surface across direction Dir in index      M
* of mesh Index for a real discontinuity in the mesh.                        M
*   This index will typically be for a knot multiplicity potential discont.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To examine its potential discontinuity across Dir.            M
*   Dir:       Direction to examine the discontiinuty across.                M
*   Idx:       Index where to examine the discontinuity.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if continuous there, FALSE otherwise.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotC1Discont, BspSrfIsC1DiscontAt                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfMeshC1Continuous                                                   M
*****************************************************************************/
CagdBType BspSrfMeshC1Continuous(const CagdSrfStruct *Srf,
				 CagdSrfDirType Dir,
				 int Idx)
{
    int i,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength;
    CagdRType Len1, Len2, t,
	LenRatio = IRIT_INFNTY;
    CagdRType
	* const *Pts = Srf -> Points;
    CagdPType Pt0, Pt1, Pt2;
    CagdVType V1, V2;
    CagdPointType
	PType = Srf -> PType;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
            for (i = 0; i < VLength; i++) {
	        CagdCoerceToE3(Pt0, Pts, CAGD_MESH_UV(Srf, Idx - 1, i), PType);
	        CagdCoerceToE3(Pt1, Pts, CAGD_MESH_UV(Srf, Idx, i),     PType);
	        CagdCoerceToE3(Pt2, Pts, CAGD_MESH_UV(Srf, Idx + 1, i), PType);

		IRIT_PT_SUB(V1, Pt0, Pt1);
		IRIT_PT_SUB(V2, Pt1, Pt2);

		Len1 = IRIT_PT_LENGTH(V1);
		Len2 = IRIT_PT_LENGTH(V2);

		if (Len1 < IRIT_EPS && Len2 < IRIT_EPS)
		    continue;

		if (LenRatio == IRIT_INFNTY && Len1 != 0 && Len2 != 0)
		    LenRatio = Len1 / Len2;
		else {
		    t = Len2 == 0 ? (Len1 == 0 ? LenRatio : IRIT_INFNTY)
				  : Len1 / Len2;
		    if (!IRIT_APX_EQ_EPS(LenRatio, t, CAGD_MESH_CONT_LENRATIO_EPS))
		        return FALSE;
		}

		if (Len1 > 0 && Len2 > 0) {
		    Len1 = 1.0 / Len1;
		    Len2 = 1.0 / Len2;
		    IRIT_PT_SCALE(V1, Len1);
		    IRIT_PT_SCALE(V2, Len2);

		    if (IRIT_DOT_PROD(V1, V2) < CAGD_MESH_CONT_ANGULAR_EPS)
		        return FALSE;
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
            for (i = 0; i < ULength; i++) {
	        CagdCoerceToE3(Pt0, Pts, CAGD_MESH_UV(Srf, i, Idx - 1), PType);
	        CagdCoerceToE3(Pt1, Pts, CAGD_MESH_UV(Srf, i, Idx),     PType);
	        CagdCoerceToE3(Pt2, Pts, CAGD_MESH_UV(Srf, i, Idx + 1), PType);

		IRIT_PT_SUB(V1, Pt0, Pt1);
		IRIT_PT_SUB(V2, Pt1, Pt2);

		Len1 = IRIT_PT_LENGTH(V1);
		Len2 = IRIT_PT_LENGTH(V2);

		if (Len1 < IRIT_EPS && Len2 < IRIT_EPS)
		    continue;

		if (LenRatio == IRIT_INFNTY && Len1 != 0 && Len2 != 0)
		    LenRatio = Len1 / Len2;
		else {
		    t = Len2 == 0 ? (Len1 == 0 ? LenRatio : IRIT_INFNTY)
				  : Len1 / Len2;
		    if (!IRIT_APX_EQ_EPS(LenRatio, t, CAGD_MESH_CONT_LENRATIO_EPS))
		        return FALSE;
		}

		if (Len1 > 0 && Len2 > 0) {
		    Len1 = 1.0 / Len1;
		    Len2 = 1.0 / Len2;
		    IRIT_PT_SCALE(V1, Len1);
		    IRIT_PT_SCALE(V2, Len2);

		    if (IRIT_DOT_PROD(V1, V2) < CAGD_MESH_CONT_ANGULAR_EPS)
		        return FALSE;
		}
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return TRUE;
}
