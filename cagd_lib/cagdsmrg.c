/******************************************************************************
* CagdCMrg.c - Surface/Surface merging routine.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep 92.					      *
******************************************************************************/

#include "cagd_loc.h"

static void InterpolateLinearSeg(CagdRType *V1,
				 CagdRType *V2,
				 int Len,
				 int Step);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two surfaces in the requested direction Dir.			     M
*  If SameEdge, it is assumed last edge of Srf1 is identical to first edge   M
* of Srf2 and one row is dropped from new mesh. Otherwise a ruled surface is M
* fit between the two edges.                                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf1:                To connect to Srf2's starting boundary at its end. M
*   CSrf2:                To connect to Srf1's end boundary at its start.    M
*   Dir:                  Direction the merge should take place. Either U    M
*                         or V.						     M
*   SameEdge:             If the two surfaces share a common edge.           M
*   InterpolateDiscont:   If TRUE, linearly interpolate discontinuity.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:     The merged surface.                                 M
*                                                                            M
* KEYWORDS:                                                                  M
*   CagdMergeSrfSrf, merge                                                   M
*****************************************************************************/
CagdSrfStruct *CagdMergeSrfSrf(const CagdSrfStruct *CSrf1,
			       const CagdSrfStruct *CSrf2,
			       CagdSrfDirType Dir,
			       CagdBType SameEdge,
			       int InterpolateDiscont)
{
    CagdBType IsNotRational;
    int i, j, UOrder, VOrder, ULen1, VLen1, ULen2, VLen2, MaxCoord, Length;
    CagdRType **Points1, **Points2, **Points;
    CagdPointType SrfPType;
    CagdSrfStruct *Srf, *Srf1, *Srf2;

    if (CAGD_IS_PERIODIC_SRF(CSrf1) || CAGD_IS_PERIODIC_SRF(CSrf2)) {
	Srf1 = CagdCnvrtPeriodic2FloatSrf(CSrf1);
	Srf2 = CagdCnvrtPeriodic2FloatSrf(CSrf2);
    }
    else {
	/* To make surfaces compatible: */
	Srf1 = CagdSrfCopy(CSrf1);
	Srf2 = CagdSrfCopy(CSrf2);
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	case CAGD_CONST_V_DIR:
	    if (!CagdMakeSrfsCompatible(&Srf1, &Srf2, TRUE, TRUE,
					Dir == CAGD_CONST_V_DIR,
					Dir == CAGD_CONST_U_DIR))
		CAGD_FATAL_ERROR(CAGD_ERR_SRF_FAIL_CMPT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    /* Verify surface geometric types. */
    switch (Srf1 -> GType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = CagdCnvrtBzr2BspSrf(Srf1);
	    CagdSrfFree(Srf1);
	    Srf1 = Srf;
	    Srf = CagdCnvrtBzr2BspSrf(Srf2);
	    CagdSrfFree(Srf2);
	    Srf2 = Srf;
	    break;
	case CAGD_SBSPLINE_TYPE:
	    break;
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    break;
    }

    UOrder = Srf1 -> UOrder;
    VOrder = Srf1 -> VOrder;
    ULen1 = Srf1 -> ULength;
    VLen1 = Srf1 -> VLength;
    ULen2 = Srf2 -> ULength;
    VLen2 = Srf2 -> VLength;
    Points1 = Srf1 -> Points;
    Points2 = Srf2 -> Points;
    IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf1);
    MaxCoord = CAGD_NUM_OF_PT_COORD(Srf1 -> PType),
    SrfPType = CAGD_MAKE_PT_TYPE(!IsNotRational, MaxCoord);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Length = SameEdge ? ULen1 + ULen2 - 1
			      : InterpolateDiscont ? ULen1 + ULen2 + UOrder - 2
						   : ULen1 + ULen2;

	    Srf = BspSrfNew(Length, VLen1, UOrder, VOrder, SrfPType);

	    /* Update knot vectors. We assume open end condition here... */
	    CAGD_GEN_COPY(Srf -> UKnotVector, Srf1 -> UKnotVector,
			  (ULen1 + UOrder - 1) * sizeof(CagdRType));
	    CAGD_GEN_COPY(Srf -> VKnotVector, Srf1 -> VKnotVector,
			  (VLen1 + VOrder) * sizeof(CagdRType));

	    if (SameEdge) {
		/* Copy kv of second surface immediately after. */
		CAGD_GEN_COPY(&Srf -> UKnotVector[ULen1 + UOrder - 1],
			      &Srf2 -> UKnotVector[UOrder],
			      ULen2 * sizeof(CagdRType));
		BspKnotAffineTrans(&Srf -> UKnotVector[ULen1 + UOrder - 1],
				   ULen2,
				   Srf -> UKnotVector[ULen1 + UOrder - 2] -
				   Srf2 -> UKnotVector[0],
				   1.0);
	    }
	    else if (InterpolateDiscont) {
		/* Copy kv of second surface order after. */
		CAGD_GEN_COPY(&Srf -> UKnotVector[ULen1 + UOrder - 1],
			      &Srf2 -> UKnotVector[1],
			      (ULen2 + UOrder - 1) * sizeof(CagdRType));
		BspKnotAffineTrans(&Srf -> UKnotVector[ULen1 + UOrder - 1],
				   ULen2 + UOrder - 1,
				   Srf -> UKnotVector[ULen1 + UOrder - 2] -
				   Srf -> UKnotVector[ULen1 + UOrder - 1] + 1.0,
				   1.0);
	    }
	    else {
		CAGD_GEN_COPY(&Srf -> UKnotVector[ULen1 + UOrder - 1],
			      &Srf2 -> UKnotVector[UOrder - 1],
			      (ULen2 + 1) * sizeof(CagdRType));
		BspKnotAffineTrans(&Srf -> UKnotVector[ULen1 + UOrder - 1],
				   ULen2 + 1,
				   Srf1 -> UKnotVector[ULen1 + UOrder - 1] -
				   Srf -> UKnotVector[ULen1 + UOrder - 1],
				   1.0);
	    }

	    /* Make sure the connection is non decreasingly monotone. */
	    BspKnotMakeRobustKV(&Srf -> UKnotVector[ULen1 + UOrder - 2], UOrder);

	    Points = Srf -> Points;

	    for (i = 0; i < VLen1; i++) {
		for (j = IsNotRational; j <= MaxCoord; j++) {
		    CAGD_GEN_COPY(&Points[j][CAGD_MESH_UV(Srf, 0, i)],
				  &Points1[j][CAGD_MESH_UV(Srf1, 0, i)],
				  ULen1 * sizeof(CagdRType));
		    if (SameEdge) {
			/* Copy row of second surface immediately after. */
			CAGD_GEN_COPY(
			    &Points[j][CAGD_MESH_UV(Srf, ULen1 - 1, i)],
			    &Points2[j][CAGD_MESH_UV(Srf2, 0, i)],
			    ULen2 * sizeof(CagdRType));
		    }
		    else if (InterpolateDiscont) {
			/* Copy row of 2nd srf order after and lin. interp. */
			CAGD_GEN_COPY(
			    &Points[j][CAGD_MESH_UV(Srf, ULen1 + UOrder - 2, i)],
			    &Points2[j][CAGD_MESH_UV(Srf2, 0, i)],
			    ULen2 * sizeof(CagdRType));
			InterpolateLinearSeg(
			    &Points[j][CAGD_MESH_UV(Srf, ULen1 - 1, i)],
			    &Points[j][CAGD_MESH_UV(Srf, ULen1 + UOrder - 2, i)],
			    UOrder, 1);
		    }
		    else {
			/* Copy row of 2nd srf order after and lin. interp. */
			CAGD_GEN_COPY(
			    &Points[j][CAGD_MESH_UV(Srf, ULen1, i)],
			    &Points2[j][CAGD_MESH_UV(Srf2, 0, i)],
			    ULen2 * sizeof(CagdRType));
		    }
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    Length = SameEdge ? VLen1 + VLen2 - 1
			      : InterpolateDiscont ? VLen1 + VLen2 + VOrder - 2
						   : VLen1 + VLen2;

	    Srf = BspSrfNew(ULen1, Length, UOrder, VOrder, SrfPType);

	    /* Update knot vectors. We assume open end condition here... */
	    CAGD_GEN_COPY(Srf -> UKnotVector, Srf1 -> UKnotVector,
			  (ULen1 + UOrder) * sizeof(CagdRType));
	    CAGD_GEN_COPY(Srf -> VKnotVector, Srf1 -> VKnotVector,
			  (VLen1 + VOrder - 1) * sizeof(CagdRType));

	    if (SameEdge) {
		/* Copy kv of second surface immediately after. */
		CAGD_GEN_COPY(&Srf -> VKnotVector[VLen1 + VOrder - 1],
			      &Srf2 -> VKnotVector[VOrder],
			      VLen2 * sizeof(CagdRType));
		BspKnotAffineTrans(&Srf -> VKnotVector[VLen1 + VOrder - 1],
				   VLen2,
				   Srf -> VKnotVector[VLen1 + VOrder - 2] -
				   Srf2 -> VKnotVector[0],
				   1.0);
	    }
	    else if (InterpolateDiscont) {
		/* Copy kv of second surface order after. */
		CAGD_GEN_COPY(&Srf -> VKnotVector[VLen1 + VOrder - 1],
			      &Srf2 -> VKnotVector[1],
			      (VLen2 + VOrder - 1) * sizeof(CagdRType));
		BspKnotAffineTrans(&Srf -> VKnotVector[VLen1 + VOrder - 1],
				   VLen2 + VOrder - 1,
				   Srf -> VKnotVector[VLen1 + VOrder - 2] -
				   Srf -> VKnotVector[VLen1 + VOrder - 1] + 1.0,
				   1.0);
	    }
            else {
		CAGD_GEN_COPY(&Srf -> VKnotVector[VLen1 + VOrder - 1],
			      &Srf2 -> VKnotVector[VOrder - 1],
			      (VLen2 + 1) * sizeof(CagdRType));
		BspKnotAffineTrans(&Srf -> VKnotVector[VLen1 + VOrder - 1],
				   VLen2 + 1,
				   Srf1 -> VKnotVector[VLen1 + VOrder - 1] -
				   Srf -> VKnotVector[VLen1 + VOrder - 1],
				   1.0);
	    }

	    /* Make sure the connection is non decreasingly monotone. */
	    BspKnotMakeRobustKV(&Srf -> VKnotVector[VLen1 + VOrder - 2], VOrder);

	    Points = Srf -> Points;

	    for (i = 0; i < ULen1; i++) {
		for (j = IsNotRational; j <= MaxCoord; j++) {
		    CAGD_GEN_COPY_STEP(&Points[j][CAGD_MESH_UV(Srf, i, 0)],
				       &Points1[j][CAGD_MESH_UV(Srf1, i, 0)],
				       VLen1, ULen1, ULen1, CagdRType);
		    if (SameEdge) {
			/* Copy col of second surface immediately after. */
			CAGD_GEN_COPY_STEP(
			    &Points[j][CAGD_MESH_UV(Srf, i, VLen1 - 1)],
			    &Points2[j][CAGD_MESH_UV(Srf2, i, 0)],
			    VLen2, ULen1, ULen1, CagdRType);
		    }
		    else if (InterpolateDiscont) {
			/* Copy col of 2nd srf order after and lin. interp. */
			CAGD_GEN_COPY_STEP(
			    &Points[j][CAGD_MESH_UV(Srf, i, VLen1 + VOrder - 2)],
			    &Points2[j][CAGD_MESH_UV(Srf2, i, 0)],
			    VLen2, ULen1, ULen1, CagdRType);
			InterpolateLinearSeg(
			    &Points[j][CAGD_MESH_UV(Srf, i, VLen1 - 1)],
			    &Points[j][CAGD_MESH_UV(Srf, i, VLen1 + VOrder - 2)],
			    VOrder, ULen1);
		    }
		    else {
			/* Copy col of 2nd srf order after and lin. interp. */
			CAGD_GEN_COPY_STEP(
			    &Points[j][CAGD_MESH_UV(Srf, i, VLen1)],
			    &Points2[j][CAGD_MESH_UV(Srf2, i, 0)],
			    VLen2, ULen1, ULen1, CagdRType);
		    }
		}
	    }
	    break;
	default:
	    Srf = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges a list of surfaces by connecting the end of one surface to the      M
* begining of the next. See also CagdMergeSrfSrf.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:              To connect into one surface.                       M
*   Dir:                  Direction the merge should take place. Either U    M
*                         or V.						     M
*   SameEdge:             If the two surfaces sharea common edge.            M
*   InterpolateDiscont:   If TRUE, linearly interpolate discontinuity.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:     The merged surface.                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMergeSrfList, merge                                                  M
*****************************************************************************/
CagdSrfStruct *CagdMergeSrfList(const CagdSrfStruct *SrfList,
				CagdSrfDirType Dir,
				CagdBType SameEdge,
				int InterpolateDiscont)
{
    if (SrfList != NULL && SrfList -> Pnext != NULL) {
	CagdSrfStruct
	    *MergedSrf = CagdSrfCopy(SrfList);

	for (SrfList = SrfList -> Pnext;
	     SrfList != NULL;
	     SrfList = SrfList -> Pnext) {
	    CagdSrfStruct
		*TmpSrf = CagdMergeSrfSrf(MergedSrf, SrfList, Dir, SameEdge,
					  InterpolateDiscont);

	    CagdSrfFree(MergedSrf);
	    MergedSrf = TmpSrf;
	}
	return MergedSrf;
    }
    else
	return SrfList ? CagdSrfCopy(SrfList) : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Linearly interpolates between V1 and V2 values Len times (Len includes V1  *
* and V2) and step the array using Step.				     *
*                                                                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   V1:       First coefficient to linearly interpolate.                     *
*   V2:       Seoncd coefficient to linearly interpolate.                    *
*   Len:      Of coefficients to linearly interpolate from V1 to V2.         *
*   Step:     To take while interpolating.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InterpolateLinearSeg(CagdRType *V1,
				 CagdRType *V2,
				 int Len,
				 int Step)
{
    int i;
    CagdRType
	*V = V1 + Step;

    if (Len-- <= 2)
	return;				     /* No middle points to interp. */

    for (i = 1; i < Len; i++) {
	*V = (i * (*V2) + (Len - i) * (*V1)) / Len;

	V += Step;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an avergate of edge lengths of edges of the control mesh of the   M
* given surface in the U direction and in the V direction.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To computer average mesh edge length in the U and the V       M
*	       directions.		                                     M
*   AvgULen, AvgVLen:  Average length of edges of the control mesh of Srf in M
*	       the U and V mesh directions.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The ratio of AvgULen / AvgVLen.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvArcLenPoly, CagdLimitCrvArcLen	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfAvgArgLenMesh, arc length                                         M
*****************************************************************************/
CagdRType CagdSrfAvgArgLenMesh(const CagdSrfStruct *Srf,
			       CagdRType *AvgULen,
			       CagdRType *AvgVLen)
{
    int i, j, NumUEdges, NumVEdges;
    CagdSrfStruct
	*SrfE3 = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);
    CagdRType
	**Points = SrfE3 -> Points;    

    *AvgULen = *AvgVLen = 0.0;
    NumUEdges = NumVEdges = 0;

    for (i = 0; i < SrfE3 -> ULength; i++) {
        for (j = 0; j < SrfE3 -> VLength; j++) {
	    int DuIdx, DvIdx,
		Idx = CAGD_MESH_UV(Srf, i, j);

	    if (i > 0) {
	        DuIdx = CAGD_MESH_UV(Srf, i - 1, j);
	        *AvgULen += sqrt(IRIT_SQR(Points[1][Idx] - Points[1][DuIdx]) +
				 IRIT_SQR(Points[2][Idx] - Points[2][DuIdx]) +
				 IRIT_SQR(Points[3][Idx] - Points[3][DuIdx]));
		NumUEdges++;
	    }

	    if (j > 0) {
	        DvIdx = CAGD_MESH_UV(Srf, i, j - 1);
	        *AvgVLen += sqrt(IRIT_SQR(Points[1][Idx] - Points[1][DvIdx]) +
				 IRIT_SQR(Points[2][Idx] - Points[2][DvIdx]) +
				 IRIT_SQR(Points[3][Idx] - Points[3][DvIdx]));
		NumVEdges++;
	    }
	}
    }

    if (NumUEdges > 0)
        *AvgULen /= NumUEdges;
    else
	*AvgULen = 1.0;

    if (NumVEdges > 0)
        *AvgVLen /= NumVEdges;
    else
	*AvgVLen = 1.0;

    CagdSrfFree(SrfE3);

    return *AvgVLen == 0.0 ? IRIT_INFNTY : *AvgULen / *AvgVLen;
}
