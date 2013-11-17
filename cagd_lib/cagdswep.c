/******************************************************************************
* CagdSwep.c - Sweep srf operator out of given cross section and an axis.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

#define MAX_AXIS_REFINE_LEVEL		50
#define CAGD_SWEEP_MRF_STEPS		10
#define CAGD_NRML_SAMPLES_DENSITY	10

static CagdBType ComputeNormalOrientation(CagdCrvStruct *Axis,
					  CagdVType FrameVec,
					  CagdCrvStruct *FrameCrv,
					  CagdRType CrntT,
					  CagdVecStruct *Tangent,
					  CagdVecStruct *Normal,
					  CagdBType FirstTime);
static CagdRType CosineHalfAngle(CagdRType **Points, int Index);
static void TransformCrossSection(CagdRType **SPoints,
				  int Index,
				  CagdCrvStruct *CrossSection,
				  CagdRType *Position,
				  CagdRType Scale,
				  CagdRType NormalScale,
				  CagdVecStruct *Tangent,
				  CagdVecStruct *Normal);
static void GenTransformMatrix(CagdMType Mat,
			       CagdRType *Trans,
			       CagdVecStruct *Normal,
			       CagdVecStruct *Tangent,
			       CagdRType Scale,
			       CagdRType NormalScale);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a sweep surface using the following curves:                     M
* 1. CrossSection - defines the basic cross section of the sweep. Must be    M
*    in the XY plane.  Can be several curves to be blended along the Axis.   M
* 2. Axis - a 3D curve the CrossSection will be swept along such that the    M
*    Axis normal aligns with the Y axis of the cross section. If Axis is     M
*    linear (i.e. no normal), the normal is picked randomly or to fit the    M
*    non linear part of the Axis (if any).				     M
* 3. Scale - a scaling curve for the sweep, If NULL a scale of Scale is      M
*    used.								     M
* 4. Frame - a curve or a vector that specifies the orientation of the sweep M
*    by specifying the axes curve's binormal. If Frame is a vector, it is a  M
*    constant binormal. If Frame is a curve (FrameIsCrv = TRUE), it is       M
*    assumed to be a vector field binormal. If NULL, it is computed from     M
*    the Axis curve's pseudo Frenet frame, that minimizes rotation.	     M
*                                                                            M
*   This operation is only an approximation. See CagdSweepAxisRefine for a   M
* tool to refine the Axis curve and improve accuracy.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   CrossSection:  Of the constructed sweep surface.  If more than one curve M
*		   is given as a linked list of curves, the cross sections   M
*		   are modified as we progresses along the sweep, blending   M
*		   between the cross sections so that last cross section     M
*		   is used in the last parameter value of the Axis.          M
*   Axis:          Of the constructed sweep surface.                         M
*   ScalingCrv:    Optional scale or profiel curve.                          M
*   Scale:         if no Scaling Crv, Scale is used to apply a fixed scale   M
*                  on the CrossSection curve.                                M
*   Frame:         An optional vector or a curve to specified the binormal   M
*                  orientation. Otherwise Frame must be NULL.		     M
*   FrameIsCrv:    If TRUE Frame is a curve, if FALSE a vector (if Frame is  M
*                  not NULL).					             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Constructed sweep surface.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSweepSrf, sweep, surface constructors                                M
*****************************************************************************/
CagdSrfStruct *CagdSweepSrf(CagdCrvStruct *CrossSection,
			    CagdCrvStruct *Axis,
			    const CagdCrvStruct *ScalingCrv,
			    CagdRType Scale,
			    const VoidPtr Frame,
			    CagdBType FrameIsCrv)
{
    CagdSrfStruct
	*Srf = NULL;
    CagdPointType
	SrfPType = CAGD_PT_E3_TYPE;
    CagdGeomType
	SrfGType = CAGD_SBSPLINE_TYPE;
    int i, j, k,
	NumCrossSecs = 1,
	ULength = CrossSection -> Length,
	VLength = Axis -> Length,
	UOrder = CrossSection -> Order,
	VOrder = Axis -> Order;
    CagdRType **Points, AxisTMin, AxisTMax,
	**AxisE3Points,
	*AxisKV = NULL,
	*AxisNodes = NULL,
	*AxisNodePtr = NULL,
	*AxisWeights = NULL,
	*FrameVec = FrameIsCrv ? NULL : (CagdRType *) Frame;
    CagdVecStruct Normal, Tangent;
    CagdCrvStruct *AxisE3, *CpScalingCrv,
        **MultiCrossSecs = &CrossSection,
	*FrameCrv = FrameIsCrv ? (CagdCrvStruct *) Frame : NULL;

    if (CrossSection -> Pnext != NULL) {
	/* We have a set of curves as cross sections.  Make them all        */
        /* compatible - same type, order, and knot sequence, and put into   */
	/* one array for ease of access.			            */
	NumCrossSecs = CagdListLength(CrossSection);

	MultiCrossSecs = (CagdCrvStruct **)
	    IritMalloc(sizeof(CagdCrvStruct *) * NumCrossSecs);

	for (i = 0; i < NumCrossSecs; i++) {
	    MultiCrossSecs[i] = CagdCrvCopy(CrossSection);
	    CrossSection = CrossSection -> Pnext;
	}

	for (i = 0; i < NumCrossSecs - 1; i++) {
	    for (j = i + 1; j < NumCrossSecs; j++) {
	        if (!CagdMakeCrvsCompatible(&MultiCrossSecs[i],
					    &MultiCrossSecs[j],
					    TRUE, TRUE))
		    CAGD_FATAL_ERROR(CAGD_ERR_CRVS_INCOMPATIBLE);
	    }
	}

	/* Update the length and order of the cross sections. */
	CrossSection = MultiCrossSecs[0];
	ULength = CrossSection -> Length;
	UOrder = CrossSection -> Order;
    }

    switch (Axis -> GType) {
	case CAGD_CBEZIER_TYPE:
	    SrfGType = CAGD_SBEZIER_TYPE;
	    AxisKV = BspKnotUniformOpen(VLength, VOrder, NULL);
	    AxisNodes = AxisNodePtr = BspKnotNodes(AxisKV,
						   VLength + VOrder,
						   VOrder);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    SrfGType = CAGD_SBSPLINE_TYPE;
	    AxisKV = Axis -> KnotVector;
	    AxisNodePtr = AxisNodes = BspKnotNodes(Axis -> KnotVector,
						   VLength + VOrder,
						   VOrder);
	    if (Axis -> Periodic) {
		/* Nodes will give us very skewed samples. Take middle of */
		/* every interior span as samples for axis positioning.   */
		for (i = 0; i < VLength; i++)
		    AxisNodes[i] = (AxisKV[i + VOrder - 1] +
				    AxisKV[i + VOrder]) * 0.5;
	    }
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }
    if (CAGD_IS_RATIONAL_CRV(Axis))
	AxisWeights = Axis -> Points[W];
    CagdCrvDomain(Axis, &AxisTMin, &AxisTMax);

    switch (CrossSection -> GType) {
	case CAGD_CBEZIER_TYPE:
	    break;
	case CAGD_CBSPLINE_TYPE:
	    SrfGType = CAGD_SBSPLINE_TYPE;
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }

    if (ScalingCrv) {
	switch (ScalingCrv -> GType) {
	    case CAGD_CBEZIER_TYPE:
		CpScalingCrv = CagdCnvrtBzr2BspCrv(ScalingCrv);
	        break;
	    case CAGD_CBSPLINE_TYPE:
		if (CAGD_IS_PERIODIC_CRV(ScalingCrv))
		    CpScalingCrv = CagdCnvrtPeriodic2FloatCrv(ScalingCrv);
		else
		    CpScalingCrv = CagdCrvCopy(ScalingCrv);
		break;
	    case CAGD_CPOWER_TYPE:
		CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
		return NULL;
	    default:
		CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
		return NULL;
	}

    	/* Affine trans. CpScalingCrv KV to match Axis. */
	BspKnotAffineTrans2(CpScalingCrv -> KnotVector,
			    CpScalingCrv -> Order + CpScalingCrv -> Length,
			    AxisTMin, AxisTMax);
    }
    else
        CpScalingCrv = NULL;

    if (FrameCrv) {
	switch (FrameCrv -> GType) {
	    case CAGD_CBEZIER_TYPE:
		FrameCrv = CagdCnvrtBzr2BspCrv(FrameCrv);
	        break;
	    case CAGD_CBSPLINE_TYPE:
		if (CAGD_IS_PERIODIC_CRV(FrameCrv))
		    FrameCrv = CagdCnvrtPeriodic2FloatCrv(FrameCrv);
		else
		    FrameCrv = CagdCrvCopy(FrameCrv);
		break;
	    case CAGD_CPOWER_TYPE:
		CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
		break;
	    default:
		CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
		break;
	}

    	/* Affine trans. FrameCrv KV to match Axis. */
	BspKnotAffineTrans2(FrameCrv -> KnotVector,
			    FrameCrv -> Order + FrameCrv -> Length,
			    AxisTMin, AxisTMax);
    }

    if (CAGD_IS_RATIONAL_CRV(Axis) || CAGD_IS_RATIONAL_CRV(CrossSection))
	SrfPType = CAGD_PT_P3_TYPE;

    switch (SrfGType) {
	case CAGD_SBEZIER_TYPE:
	    Srf = BzrSrfNew(ULength, VLength, SrfPType);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    Srf = BspPeriodicSrfNew(ULength, VLength, UOrder, VOrder,
				    CrossSection -> Periodic, Axis -> Periodic,
				    SrfPType);
	    if (CrossSection -> GType == CAGD_CBSPLINE_TYPE)
		CAGD_GEN_COPY(Srf -> UKnotVector, CrossSection -> KnotVector,
			      sizeof(CagdRType) *
				 (CAGD_CRV_PT_LST_LEN(CrossSection) + UOrder));
	    else
		BspKnotUniformOpen(ULength, UOrder, Srf -> UKnotVector);
	    if (Axis -> GType == CAGD_CBSPLINE_TYPE)
		CAGD_GEN_COPY(Srf -> VKnotVector, Axis -> KnotVector,
			      sizeof(CagdRType) *
			          (CAGD_CRV_PT_LST_LEN(Axis) + VOrder));
	    else
		BspKnotUniformOpen(VLength, VOrder, Srf -> VKnotVector);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    break;
    }
    Points = Srf -> Points;

    /* Prepare an E3 version of the Axis. */
    AxisE3 = CagdCoerceCrvTo(Axis, CAGD_PT_E3_TYPE, FALSE);
    AxisE3Points = AxisE3 -> Points;

    /* For each ctl points of the axis, transform the cross section          */
    /* according to ctl point position, tangent to axis at the point and in  */
    /* such a way to minimize Normal change.				     */
    for (i = 0; i < VLength; i++, AxisNodePtr++) {
	int m;
	CagdRType PosE3[3], ScaleE2[2], NormalScale,
	    *Scaling = CpScalingCrv ? CagdCrvEval(CpScalingCrv, *AxisNodePtr)
				    : NULL;

	Tangent = *CagdCrvTangent(Axis, *AxisNodePtr, TRUE);

	if (Scaling)
	    CagdCoerceToE2(ScaleE2, &Scaling, -1, CpScalingCrv -> PType);
	else
	    ScaleE2[1] = Scale;

	/* Compute a normal direction scale which is a result of angular     */
	/* change in the direction of the control polygon of the Axis curve. */
	if (i == 0 || i == VLength - 1)
	    NormalScale = 1.0;
	else
	    NormalScale = CosineHalfAngle(AxisE3Points, i);

	/* If Normal is fully specified, get it now. */
	ComputeNormalOrientation(Axis, FrameVec, FrameCrv, *AxisNodePtr,
				 &Tangent, &Normal, i == 0);

	if (Axis -> Periodic) {
	    CagdRType
		*R = CagdCrvEval(Axis, *AxisNodePtr);

	    CagdCoerceToE3(PosE3, &R, -1, Axis -> PType);
	}
	else
	    CagdCoerceToE3(PosE3, Axis -> Points, i, Axis -> PType);

	/* Compute index into multi cross-sections' array. 0 if only one. */
	m = (int) ((NumCrossSecs - 1.0) * i / (VLength - 1.0) + 0.5);
	TransformCrossSection(Points, i * ULength, MultiCrossSecs[m],
			      PosE3, ScaleE2[1], NormalScale,
			      &Tangent, &Normal);
    }

    /* Do fixups if axis is a rational curve (note surface is P3). */
    if (AxisWeights) {
	if (CAGD_IS_RATIONAL_CRV(CrossSection)) {
	    /* Need only scale by the Axis curve weights: */
	    for (j = 0, k = 0; j < VLength; j++)
		for (i = 0; i < ULength; i++, k++) {
		    Points[X][k] *= AxisWeights[j];
		    Points[Y][k] *= AxisWeights[j];
		    Points[Z][k] *= AxisWeights[j];
		    Points[W][k] *= AxisWeights[j];
		}
	}
	else {
	    /* Weights do not exists at the moment - need to copy them. */
	    for (j = 0, k = 0; j < VLength; j++)
		for (i = 0; i < ULength; i++, k++) {
		    Points[X][k] *= AxisWeights[j];
		    Points[Y][k] *= AxisWeights[j];
		    Points[Z][k] *= AxisWeights[j];
		    Points[W][k] = AxisWeights[j];
		}
	}
    }

    if (NumCrossSecs > 1) {
	/* We have a set of curves as cross sections - free them all. */
	for (i = 0; i < NumCrossSecs; i++)
	    CagdCrvFree(MultiCrossSecs[i]);
	IritFree(MultiCrossSecs);
    }

    if (Axis -> GType == CAGD_CBEZIER_TYPE)
        IritFree(AxisKV);

    if (CpScalingCrv)
        CagdCrvFree(CpScalingCrv);

    IritFree(AxisNodes);

    CagdCrvFree(AxisE3);
    if (FrameCrv)
	CagdCrvFree(FrameCrv);

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_SWEEP_SRF);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Estimates a normal for the orientation frame at the given parameter t.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Axis:	Axis curve of sweep.                                         *
*   FrameVec:   Binormal constant of orienation frame, if not NULL.          *
*   FrameCrv:   Binormak vector field of orienation frame, if not NULL.      *
*   CrntT:      Parameter value where to evaluate.			     *
*   Tangent:    of Axis curve at parameter value t.			     *
*   Normal:     An estimated normal for the orientation frame.		     *
*   FirstTime:  TRUE if first time to compute the normal.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if computed, FALSE otherwise.			     *
*****************************************************************************/
static CagdBType ComputeNormalOrientation(CagdCrvStruct *Axis,
					  CagdVType FrameVec,
					  CagdCrvStruct *FrameCrv,
					  CagdRType CrntT,
					  CagdVecStruct *Tangent,
					  CagdVecStruct *Normal,
					  CagdBType FirstTime)
{
    IRIT_STATIC_DATA CagdRType
	PrevT = 0.0;

    /* If Normal is fully specified, get it now. */
    if (FrameVec != NULL) {
	IRIT_CROSS_PROD(Normal -> Vec, FrameVec, Tangent -> Vec);
    }
    else if (FrameCrv != NULL) {
	CagdRType Binormal[3],
	    *FrameCrvVal = CagdCrvEval(FrameCrv, CrntT);

	CagdCoerceToE3(Binormal, &FrameCrvVal, -1, FrameCrv -> PType);

	IRIT_CROSS_PROD(Normal -> Vec, Binormal, Tangent -> Vec);
    }
    else if (FirstTime) {
	CagdVecStruct *Vec;

	/* Compute the normal to axis curve and use it. If Axis curve has no */
	/* normal (i.e. it is a linear segment), arbitrary normal is picked. */
	Vec = CagdCrvNormal(Axis, CrntT, FALSE);
	if (Vec != NULL &&
	    IRIT_PT_SQR_LENGTH(Vec -> Vec) >=
					    IRIT_SQR(IRIT_PT_NORMALIZE_ZERO)) {
	    CAGD_COPY_VECTOR(*Normal, *Vec);
	}
	else {
	    CagdRType TMin, TMax;
	    CagdVType V;

	    /* We might hit an inflection point.  Try to move a little. */
	    CagdCrvDomain(Axis, &TMin, &TMax);
	    if (CrntT + IRIT_EPS < TMax &&
		(Vec = CagdCrvNormal(Axis, CrntT + IRIT_EPS, FALSE)) != NULL &&
		IRIT_PT_SQR_LENGTH(Vec -> Vec) >=
					    IRIT_SQR(IRIT_PT_NORMALIZE_ZERO)) {
		CAGD_COPY_VECTOR(*Normal, *Vec);
	    }
	    else if (CrntT - IRIT_EPS >= TMin &&
		     (Vec = CagdCrvNormal(Axis,
					  CrntT - IRIT_EPS, FALSE)) != NULL &&
		     IRIT_PT_SQR_LENGTH(Vec -> Vec) >=
					    IRIT_SQR(IRIT_PT_NORMALIZE_ZERO)) {
		CAGD_COPY_VECTOR(*Normal, *Vec);
	    }
	    else {
		/* Failed again - find a vector normal to the tangent. */
		IRIT_VEC_RESET(V);
		if (IRIT_FABS(Tangent -> Vec[2]) <=
						IRIT_FABS(Tangent -> Vec[0]) &&
		    IRIT_FABS(Tangent -> Vec[2]) <=
						IRIT_FABS(Tangent -> Vec[1]))
		    V[2] = 1.0;
		else if (IRIT_FABS(Tangent -> Vec[1]) <= 
						IRIT_FABS(Tangent -> Vec[0]) &&
			 IRIT_FABS(Tangent -> Vec[1]) <=
						IRIT_FABS(Tangent -> Vec[2]))
		    V[1] = 1.0;
		else
		    V[0] = 1.0;

		IRIT_CROSS_PROD(Normal -> Vec, V, Tangent -> Vec);
	    }
	}
	IRIT_VEC_SCALE(Normal -> Vec, -1.0);
    }
    else {
        int i;
	CagdRType R, t,
	    Dt = (CrntT - PrevT) / CAGD_SWEEP_MRF_STEPS;

	for (i = 0, t = PrevT + Dt; i < CAGD_SWEEP_MRF_STEPS; i++, t += Dt) {
	    CagdVecStruct Tangent2;

	    if (IRIT_APX_EQ(t, CrntT))
		t = CrntT;    /* Avoid numerical error at destination point. */

	    Tangent2 = *CagdCrvTangent(Axis, t, TRUE);
	    if (IRIT_FABS(R = IRIT_DOT_PROD(Normal -> Vec,
				  Tangent2.Vec)) > 1.0 - IRIT_UEPS) {
	        /* An exact (or more) turn of 90 degrees would fail here! */
		CAGD_FATAL_ERROR(CAGD_ERR_SWEEP_AXIS_TOO_COMPLEX);
 	    }
	    CAGD_MULT_VECTOR(Tangent2, R);
	    CAGD_SUB_VECTOR(*Normal, Tangent2);
	    CAGD_NORMALIZE_VECTOR_MSG_ZERO(*Normal);
	}

	PrevT = CrntT;
	return FALSE;
    }

    CAGD_NORMALIZE_VECTOR_MSG_ZERO(*Normal);

    PrevT = CrntT;
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimates an orientation frame (tangent and normal) for the given curve  M
* at the given parameter t.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:	Curve to evaluate orientation frame for.                     M
*   CrntT:      Parameter value where to evaluate.			     M
*   Tangent:    Of curve at parameter value t.				     M
*   Normal:     Of curve at parameter value t.				     M
*   FirstTime:  TRUE if first time to compute a fraem for this curve.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if computed, FALSE if error/failed.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvOrientationFrame, orientation frame, sweep, surface constructors  M
*****************************************************************************/
CagdBType CagdCrvOrientationFrame(CagdCrvStruct *Crv,
				  CagdRType CrntT,
				  CagdVecStruct *Tangent,
				  CagdVecStruct *Normal,
				  CagdBType FirstTime)
{
    IRIT_STATIC_DATA int
        NrmlVecSize = 0;
    IRIT_STATIC_DATA CagdVecStruct N, *Nrml, *Tan,
        *Nrmls = NULL;
    int i;
    CagdRType t, dt, TMin, TMax;
    CagdVType V;

    CagdCrvDomain(Crv, &TMin, &TMax);

    if (FirstTime) {
        /* Compute a dense vector of normals and correct all normals that    */
        /* are flipped due to inflection points.			     */
        NrmlVecSize = Crv -> Length * CAGD_NRML_SAMPLES_DENSITY;
        if (Nrmls != NULL)
	    IritFree(Nrmls);
	Nrmls = IritMalloc(sizeof(CagdVecStruct) * NrmlVecSize);

	dt = (TMax - TMin) / (NrmlVecSize - 1) - IRIT_UEPS;

	/* Compute normals at all sampling locations. */
	for (i = 0, t = TMin; i < NrmlVecSize; i++, t += dt) {
	    if ((Nrml = CagdCrvNormal(Crv, t, FALSE)) == NULL ||
		IRIT_VEC_SQR_LENGTH(Nrml -> Vec) < IRIT_UEPS) {
	        if (i == 0) {
		    if ((Tan = CagdCrvTangent(Crv, t, FALSE)) == NULL)
		        return FALSE;
		    else {
		        /* Find the smallest coefficient and define a vector */
		        /* in that direction and then a normal that is       */
		        /* orthogonal to both.				     */
		        int j,
			    MinIdx = -1;
			CagdRType
			    MinVal = IRIT_INFNTY;

			for (j = 0; j < 3; j++) {
			    if (MinVal > IRIT_FABS(Tan -> Vec[j])) {
			        MinIdx = j;
				MinVal = IRIT_FABS(Tan -> Vec[j]);
			    }
			}
			assert(MinIdx >= 0);
			IRIT_VEC_RESET(V);
			V[MinIdx] = 1.0;
			IRIT_CROSS_PROD(Nrmls[i].Vec, Tan -> Vec, V);
		    }
		}
		else {
		    Nrmls[i] = Nrmls[i - 1];
		}
	    }
	    else {
	         Nrmls[i] = *Nrml;
	    }	    
	}

	/* Properly orient the normals. */
	for (i = 1; i < NrmlVecSize; i++) {
	    if (IRIT_DOT_PROD(Nrmls[i].Vec, Nrmls[i - 1].Vec) < 0.0) {
	        IRIT_VEC_SCALE(Nrmls[i].Vec, -1.0);
	    }
	}
    }

    i = (int) (0.5 + (NrmlVecSize - 1) * (CrntT - TMin) / (TMax - TMin));
    assert(i >= 0 && i < NrmlVecSize);
    Tan = CagdCrvTangent(Crv, CrntT, FALSE);
    if ((Nrml = CagdCrvNormal(Crv, CrntT, FALSE)) == NULL ||
	IRIT_VEC_SQR_LENGTH(Nrml -> Vec) < IRIT_UEPS) {
        /* Use a close normal from the vector of normals. */
	IRIT_CROSS_PROD(V, Tan -> Vec, Nrmls[i].Vec);
	IRIT_CROSS_PROD(N.Vec, Tan -> Vec, V);
	Nrml = &N;
    }

    if (IRIT_DOT_PROD(Nrml -> Vec, Nrmls[i].Vec) < 0.0) {
        IRIT_VEC_SCALE(Nrml -> Vec, -1.0);
    }

    *Normal = *Nrml;
    *Tangent = *Tan;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the cosine of half the angle between the two vectors between    *
* the three consecutive control points Index-1, Index, and Index+1.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Points:    Points to consider.                                           *
*   Index:     at indices Index-1, Index, and Index+1.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Cosine of half the angle between the given three points     *
*****************************************************************************/
static CagdRType CosineHalfAngle(CagdRType **Points, int Index)
{
    int i, j;
    CagdRType R;
    CagdVType V[2];

    for (i = Index - 1; i <= Index; i++)
        for (j = 0; j < 3; j++)
	    V[i - (Index - 1)][j] = Points[j + 1][i + 1] - Points[j + 1][i];

    /* If the points are too close - ignore! */
    if (IRIT_PT_SQR_LENGTH(V[0]) < IRIT_UEPS || IRIT_PT_SQR_LENGTH(V[1]) < IRIT_UEPS)
	return 1.0;

    IRIT_PT_NORMALIZE(V[0]);
    IRIT_PT_NORMALIZE(V[1]);

    R = IRIT_DOT_PROD(V[0], V[1]);
    if (IRIT_APX_EQ(R, 1.0))
	return 1.0;

    return cos(acos(R) * 0.5);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Transforms the CrossSection Points, to Position such that Tangent is       *
* perpendicular to the cross section (which is assumed to be on the XY       *
* plane). The +Y axis of the cross section is aligned with Normal direction  *
* to minimize twist along the sweep and been updated to new normal.	     *
*   Transformed cross section is place into srf Points, SPoints starting     *
* from index SIndex.							     *
*   All agrument vectors are assumed to be normalized to a unit length.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   SPoints:        Final destination of the transformed points.             *
*   SIndex:         Index in SPOints where to start and place new points.    *
*   CrossSection:   To transform and place in SPoints.                       *
*   Position:       Translation factor.                                      *
*   Scale:          Scale factor.                                            *
*   NormalScale:    Scale factor in the normal vector direction.             *
*   Tangent:        Tangent direction to prescribe orientaion.               *
*   Normal:         Normal direction to prescribe orientaion.                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TransformCrossSection(CagdRType **SPoints,
				  int SIndex,
				  CagdCrvStruct *CrossSection,
				  CagdRType *Position,
				  CagdRType Scale,
				  CagdRType NormalScale,
				  CagdVecStruct *Tangent,
				  CagdVecStruct *Normal)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_PT(CrossSection -> PType);
    CagdMType Mat;
    CagdCrvStruct *CSCopy;
    int i, j, MaxCoord,
	Len = CrossSection -> Length;
    CagdRType **CSPoints;

    GenTransformMatrix(Mat, Position, Normal, Tangent, Scale, NormalScale);
    CSCopy = CagdCrvMatTransform(CrossSection, Mat);
    CSPoints = CSCopy -> Points;

    /* Max coord. may be modified by CagdCrvMatTransform to be 3D if was 2D! */
    MaxCoord = CAGD_NUM_OF_PT_COORD(CSCopy -> PType);

    for (i = 0; i < Len; i++)
	for (j = IsNotRational; j <= MaxCoord; j++)
	    SPoints[j][SIndex + i] = CSPoints[j][i];

    CagdCrvFree(CSCopy);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to prepar a transformation martix to do the following (in this   *
* order): scale by Scale, rotate such that X axis is in Normal dir           *
* and Y is collinear with the BiNormal and then translate by Trans.	     *
*    Algorithm: given the Trans vector, it forms the 4th line of Mat. Dir is *
* used to form the second line (the first 3 lines set the rotation), and     *
* finally Scale is used to scale first 3 lines/columns to the needed scale:  *
*                |  Nx  Ny  Nz  0 |   A transformation which takes the coord *
*                |  Bx  By  Bz  0 |  system into T, N & B as required and    *
* [X  Y  Z  1] * |  Tx  Ty  Tz  0 |  then translate it to C. T, N, B are     *
*                |  Cx  Cy  Cz  1 |  scaled by Scale.			     *
* T is exactly Tangent (unit vec). N is set to be Normal and B their cross   *
* product.								     *
*   All agrument vectors are assumed to be normalized to a unit length.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:          To place the newly computed transformation.                *
*   Trans:        Translation factor.                                        *
*   Tangent:      Tangent direction to prescribe orientaion.                 *
*   Normal:       Normal direction to prescribe orientaion.                  *
*   Scale:        Scale factor.                                              *
*   NormalScale:  Scale factor in the normal vector direction.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GenTransformMatrix(CagdMType Mat,
			       CagdRType *Trans,
			       CagdVecStruct *Normal,
			       CagdVecStruct *Tangent,
			       CagdRType Scale,
			       CagdRType NormalScale)
{
    int i;
    CagdVecStruct B;

    IRIT_CROSS_PROD(B.Vec, Tangent -> Vec, Normal -> Vec);

    for (i = 0; i < 3; i++) {
	Mat[0][i] = Normal -> Vec[i] * Scale / NormalScale;
	Mat[1][i] = B.Vec[i] * Scale;
	Mat[2][i] = Tangent -> Vec[i] * Scale;
	Mat[3][i] = Trans[i];
	Mat[i][3] = 0.0;
    }
    Mat[3][3] = 1.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to refine the axis curve, according to the scaling curve to      M
* better approximate the requested sweep operation.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Axis:         Axis to be used in future sweep operation with the         M
*                 associated ScalingCrv.				     M
*   ScalingCrv:   If sweep is to have one, NULL otherwise.		     M
*   RefLevel:     Some refinement control. Keep it low like 2 or 3.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Refined Axis curve.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSweepAxisRefine, sweep, refinement                                   M
*****************************************************************************/
CagdCrvStruct *CagdSweepAxisRefine(const CagdCrvStruct *Axis,
				   const CagdCrvStruct *ScalingCrv,
				   int RefLevel)
{
  CagdCrvStruct *NewAxis, *RetAxis;

    if (RefLevel < 1 || RefLevel > MAX_AXIS_REFINE_LEVEL)
	return CagdCrvCopy(Axis);

    if (CAGD_IS_BEZIER_CRV(Axis))
	NewAxis = CagdCnvrtBzr2BspCrv(Axis);
    else
	NewAxis = CagdCrvCopy(Axis);

    if (ScalingCrv != NULL) {
        CagdCrvStruct *CpScalingCrv;

	if (CAGD_IS_BEZIER_CRV(ScalingCrv))
	    ScalingCrv = CpScalingCrv = CagdCnvrtBzr2BspCrv(ScalingCrv);
	else
	    CpScalingCrv = NULL;

	if (CAGD_IS_BSPLINE_CRV(ScalingCrv) &&
	    CAGD_IS_PERIODIC_CRV(ScalingCrv)) {
	    CagdCrvStruct
		*TCrv = CagdCnvrtPeriodic2FloatCrv(ScalingCrv);

	    if (CpScalingCrv != NULL)
		CagdCrvFree(CpScalingCrv);
	    ScalingCrv = CpScalingCrv = TCrv;
	}

	if (CAGD_IS_BSPLINE_CRV(ScalingCrv)) {
	    int i, j,
		SOrder = ScalingCrv -> Order,
		SLength = ScalingCrv -> Length;
	    CagdRType AxisTMin, AxisTMax,
		*KV = BspKnotCopy(NULL, ScalingCrv -> KnotVector,
				  SLength + SOrder),
		*KVRef = (CagdRType *) IritMalloc(sizeof(CagdRType) * RefLevel *
						      (1 + SLength - SOrder));

	    CagdCrvDomain(Axis, &AxisTMin, &AxisTMax);
	    BspKnotAffineTrans2(KV, SLength + SOrder, AxisTMin, AxisTMax);

	    for (i = SOrder - 1, j = 0; i < SLength; i++) {
		int k;
		CagdRType
		    T1 = KV[i],
		    T2 = KV[i+1];

		for (k = 0; k < RefLevel; k++)
		    KVRef[j++] = (T1 * (RefLevel - k) + T2 * k) / RefLevel;
	    }
	    IritFree(KV);

	    if (j > 1) {
		/* Skip the first knot which is on the domain's boundary. */
		RetAxis = CagdCrvRefineAtParams(NewAxis, FALSE,
						&KVRef[1], j - 1);
	    }
	    else
		RetAxis = CagdCrvCopy(Axis);

	    IritFree(KVRef);
	}
	else
	    RetAxis = CagdCrvCopy(Axis);

	if (CpScalingCrv != NULL)
	    CagdCrvFree(CpScalingCrv);
    }
    else {
	int i, j,
	    AOrder = NewAxis -> Order,
	    ALength = CAGD_CRV_PT_LST_LEN(NewAxis);
	CagdRType
	    *AKV = NewAxis -> KnotVector,
	    *KVRef = (CagdRType *) IritMalloc(sizeof(CagdRType) * RefLevel *
						      (1 + ALength - AOrder));

	for (i = AOrder - 1, j = 0; i < ALength; i++) {
	    int k;
	    CagdRType
	        T1 = AKV[i],
	        T2 = AKV[i+1];

	    for (k = 0; k < RefLevel; k++)
		KVRef[j++] = (T1 * (RefLevel - k) + T2 * k) / RefLevel;
	}

	if (j > 1) {
	    /* Skip the first knot which is on the domain's boundary. */
	    RetAxis = CagdCrvRefineAtParams(NewAxis, FALSE, &KVRef[1], j - 1);
	}
	else
	    RetAxis = CagdCrvCopy(Axis);

	IritFree(KVRef);
    }

    CagdCrvFree(NewAxis);

    return RetAxis;
}
