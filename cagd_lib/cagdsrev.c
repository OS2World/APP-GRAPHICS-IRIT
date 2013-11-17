/******************************************************************************
* CagdSRev.c - Surface of revolution out of a given profile.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 91.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define MAX_SREV_ANGLE_ERR	(IRIT_UEPS / 4.0)
#define MAX_SREV_ANGLE_ITER	100

IRIT_STATIC_DATA CagdRType 
    CircRationalKnotVector[12] = { 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4 },
    CircPolynomialKnotVector[17] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
							3, 3, 3, 4, 4, 4, 4 };

IRIT_STATIC_DATA CagdRType
    PolyApproxRotAngles[] = {
	0,
	33.523898, /* arcsin(4 (sqrt(2) - 1) / 3) */
	90 - 33.523898,
	90
    };

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface of revolution around the Z axis of the given profile  M
* curve. Resulting surface will be a Bspline surface, while input may be     M
* either a Bspline or a Bezier curve.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:        To create surface of revolution around Z with.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Surface of revolution.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSurfaceRev2, CagdSurfaceRevAxis, CagdSurfaceRev2Axis,		     M
*   CagdSurfaceRevPolynomialApprox					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSurfaceRev, surface of revolution, surface constructors              M
*****************************************************************************/
CagdSrfStruct *CagdSurfaceRev(const CagdCrvStruct *CCrv)
{
    int i, j, i9,
	Len = CCrv -> Length;
    CagdPointType
	PType = CCrv -> PType;
    CagdRType **SrfPoints,
	sin45 = sin(M_PI / 4.0);
    CagdRType * const *CrvPoints;
    CagdCrvStruct *Crv;
    CagdSrfStruct
	*Srf = BspPeriodicSrfNew(9, Len, 3, CCrv -> Order,
				 FALSE, CCrv -> Periodic, CAGD_PT_P3_TYPE);

    /* Make sure the curve resides in R^3. */
    if ((CAGD_NUM_OF_PT_COORD(PType) < 3)) {
        PType = CAGD_IS_RATIONAL_PT(PType) ? CAGD_PT_P3_TYPE
					   : CAGD_PT_E3_TYPE;
	Crv = CagdCoerceCrvTo(CCrv, PType, FALSE);
    }
    else
        Crv = CagdCrvCopy(CCrv);

    CrvPoints = Crv -> Points;

    IRIT_GEN_COPY(Srf -> UKnotVector, CircRationalKnotVector,
		  sizeof(CagdRType) * 12);

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    BspKnotUniformOpen(Len, Crv -> Order, Srf -> VKnotVector);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    CAGD_GEN_COPY(Srf -> VKnotVector, Crv -> KnotVector,
			  sizeof(CagdRType) * (CAGD_CRV_PT_LST_LEN(Crv) +
					                       Crv -> Order));
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    SrfPoints = Srf -> Points;

    /* For each control point in the original curve - generate 9 points that */
    /* form a circle perpendicular to the Z axis.			     */
    for (i = i9 = 0; i < Len; i++, i9 += 9) {
	SrfPoints[W][i9] = 1.0;
	switch (PType) {
	    case CAGD_PT_P3_TYPE:
		SrfPoints[W][i9] = CrvPoints[W][i];
	    case CAGD_PT_E3_TYPE:
		SrfPoints[X][i9] = CrvPoints[X][i];
		SrfPoints[Y][i9] = CrvPoints[Y][i];
		SrfPoints[Z][i9] = CrvPoints[Z][i];
		break;
	    default:
		CAGD_FATAL_ERROR(CAGD_ERR_UNSUPPORT_PT);
		break;
	}

	/* Last point is exactly the same as first one in circle - copy it.  */
	for (j = W; j <= Z; j++)
	    SrfPoints[j][i9 + 8] = SrfPoints[j][i9];

	/* The Z components are identical in all circle, while the XY        */
	/* components are rotated 45 degrees at a time:			     */
	for (j = 1; j < 8; j++) {
	    SrfPoints[W][i9 + j] = SrfPoints[W][i9];
	    SrfPoints[X][i9 + j] = SrfPoints[X][i9 + j - 1] * sin45 -
				   SrfPoints[Y][i9 + j - 1] * sin45;
	    SrfPoints[Y][i9 + j] = SrfPoints[X][i9 + j - 1] * sin45 +
				   SrfPoints[Y][i9 + j - 1] * sin45;
	    SrfPoints[Z][i9 + j] = SrfPoints[Z][i9];
	}

	/* And finally we need to compensate for the W's on every second pt. */
	for (j = 1; j < 8; j += 2) {
	    SrfPoints[W][i9 + j] *= sin45;
	    SrfPoints[Z][i9 + j] *= sin45;
	}
    }

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_SRF_OF_REV);

    CagdCrvFree(Crv);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface of revolution around vector Axis of the given profile M
* curve. Resulting surface will be a Bspline surface, while input may be     M
* either a Bspline or a Bezier curve.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To create surface of revolution around Axis.                 M
*   Axis:       Of rotation of Crv.  This axis is always through the origin. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Surface of revolution.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSurfaceRev, CagdSurfaceRev2, CagdSurfaceRev2Axis,		     M
*   CagdSurfaceRevPolynomialApprox					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSurfaceRevAxis, surface of revolution, surface constructors          M
*****************************************************************************/
CagdSrfStruct *CagdSurfaceRevAxis(const CagdCrvStruct *Crv, CagdVType Axis)
{
    IrtVecType UnitAxis;
    IrtHmgnMatType Mat, InvMat;
    CagdCrvStruct *TCrv;
    CagdSrfStruct *Srf, *TSrf;

    IRIT_VEC_COPY(UnitAxis, Axis);
    IRIT_VEC_NORMALIZE(UnitAxis);

    GMGenMatrixZ2Dir(Mat, UnitAxis);
    MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

    TCrv = CagdCrvMatTransform(Crv, InvMat);  /* Bring crv to Z axis of rot. */
    Srf = CagdSurfaceRev(TCrv);		/* Create the surface of revolution. */
    CagdCrvFree(TCrv);

    TSrf = CagdSrfMatTransform(Srf, Mat); /* Bring srf of rev. back to Axis. */
    CagdSrfFree(Srf);
    Srf = TSrf;

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_SRF_OF_REV);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface of revolution around the Z axis of the given profile  M
* curve from StartAngle to EndAngle. Resulting surface will be a Bspline     M
* surface, while input may be either a Bspline or a Bezier curve.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To create surface of revolution around Z with.               M
*   StartAngle: Starting Angle to consider rotating Crv from, in degrees.    M
*   EndAngle:   Terminating Angle to consider rotating Crv from, in degrees. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Surface of revolution.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSurfaceRev, CagdSurfaceRevAxis, CagdSurfaceRev2Axis,		     M
*   CagdSurfaceRevPolynomialApprox					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSurfaceRev2, surface of revolution, surface constructors             M
*****************************************************************************/
CagdSrfStruct *CagdSurfaceRev2(const CagdCrvStruct *Crv,
			       CagdRType StartAngle,
			       CagdRType EndAngle)
{
    int j = 0;
    CagdRType StartParam, EndParam, *R, TMin, TMax, TMid, A;
    CagdCrvStruct
	*Circ = BspCrvCreateUnitCircle();
    CagdSrfStruct *TSrf, *Srf;
    CagdMType ZRotMat;

    /* Find parameter values for starting and ending angles on a circle. */
    if (StartAngle > EndAngle)
	IRIT_SWAP(IrtRType, StartAngle, EndAngle)

    StartAngle = IRIT_DEG2RAD(StartAngle);
    MatGenMatRotZ1(StartAngle, ZRotMat);

    EndAngle = IRIT_DEG2RAD(EndAngle) - StartAngle;

    CagdCrvDomain(Circ, &TMin, &TMax);
    StartParam = TMin;
    do {
	/* Compute angle of mid of domain, between zero and 2Pi. */
	TMid = (TMin + TMax) * 0.5;
	R = CagdCrvEval(Circ, TMid);
	A = atan2(R[2], R[1]);
	if (A < 0.0)
	    A += M_PI_MUL_2;

	if (A > EndAngle)
	    TMax = TMid;
	else
	    TMin = TMid;
    }
    while (TMax - TMin > MAX_SREV_ANGLE_ERR && j++ < MAX_SREV_ANGLE_ITER);
    EndParam = (TMin + TMax) * 0.5;

    CagdCrvFree(Circ);

    TSrf = CagdSurfaceRev(Crv);
    Srf = CagdSrfRegionFromSrf(TSrf, StartParam, EndParam, CAGD_CONST_U_DIR);
    CagdSrfFree(TSrf);

    /* Rotate so it starts at StartAngle. */
    TSrf = CagdSrfMatTransform(Srf, ZRotMat);
    CagdSrfFree(Srf);
    Srf = TSrf;

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_SRF_OF_REV);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface of revolution around vector Axis of the given profile M
* curve from StartAngle to EndAngle. Resulting surface will be a Bspline     M
* surface, while input may be either a Bspline or a Bezier curve.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To create surface of revolution around Axis.                 M
*   StartAngle: Starting Angle to consider rotating Crv from, in degrees.    M
*   EndAngle:   Terminating Angle to consider rotating Crv from, in degrees. M
*   Axis:       Of rotation of Crv.  This axis is always through the origin. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Surface of revolution.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSurfaceRev, CagdSurfaceRev2, CagdSurfaceRev2Axis,		     M
*   CagdSurfaceRevPolynomialApprox					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSurfaceRev2Axis, surface of revolution, surface constructors         M
*****************************************************************************/
CagdSrfStruct *CagdSurfaceRev2Axis(const CagdCrvStruct *Crv,
				   CagdRType StartAngle,
				   CagdRType EndAngle,
				   const CagdVType Axis)
{
    IrtVecType UnitAxis;
    IrtHmgnMatType Mat, InvMat;
    CagdCrvStruct *TCrv;
    CagdSrfStruct *Srf, *TSrf;

    IRIT_VEC_COPY(UnitAxis, Axis);
    IRIT_VEC_NORMALIZE(UnitAxis);

    GMGenMatrixZ2Dir(Mat, UnitAxis);
    MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

    TCrv = CagdCrvMatTransform(Crv, InvMat);  /* Bring crv to Z axis of rot. */
    Srf = CagdSurfaceRev2(TCrv, StartAngle, EndAngle); /* Create srf of rev. */
    CagdCrvFree(TCrv);

    TSrf = CagdSrfMatTransform(Srf, Mat); /* Bring srf of rev. back to Axis. */
    CagdSrfFree(Srf);
    Srf = TSrf;

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_SRF_OF_REV);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface of revolution around the Z axis of the given profile  M
* curve. Resulting surface will be a Bspline surface, while input may be     M
* either a Bspline or a Bezier curve.                                        M
*   Resulting surface will be a polynomial Bspline surface, approximating a  M
* surface of revolution using a polynomial circle approx.                    M
* (See Faux & Pratt "Computational Geometry for Design and Manufacturing").  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To approximate a surface of revolution around Z with.        M
*		Crv is assumed planar in a plane holding the Z axis.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Surface of revolution approximation.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSurfaceRev, CagdSurfaceRev2, CagdSurfaceRevAxis, CagdSurfaceRev2Axis M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSurfaceRevPolynomialApprox, surface of revolution, surface           M
*   constructors						             M
*****************************************************************************/
CagdSrfStruct *CagdSurfaceRevPolynomialApprox(const CagdCrvStruct *Crv)
{
    int i, j, i13,
	Len = Crv -> Length;
    CagdPointType
	PType = Crv -> PType;
    CagdRType **SrfPoints;
    CagdRType * const *CrvPoints;
    CagdCrvStruct *CpCrv;
    CagdSrfStruct
	*Srf = BspPeriodicSrfNew(13, Len, 4, Crv -> Order,
				 FALSE, Crv -> Periodic, CAGD_PT_E3_TYPE);

    if (CAGD_IS_RATIONAL_CRV(Crv)) {
	CAGD_FATAL_ERROR(CAGD_ERR_POLYNOMIAL_EXPECTED);
	return NULL;
    }

    /* Make sure the curve resides in R^3. */
    if (CAGD_NUM_OF_PT_COORD(PType) < 3)
	Crv = CpCrv = CagdCoerceCrvTo(Crv, PType = CAGD_PT_E3_TYPE, FALSE);
    else
        CpCrv = NULL;

    CrvPoints = Crv -> Points;

    IRIT_GEN_COPY(Srf -> UKnotVector, CircPolynomialKnotVector,
		  sizeof(CagdRType) * 17);

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    BspKnotUniformOpen(Len, Crv -> Order, Srf -> VKnotVector);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    CAGD_GEN_COPY(Srf -> VKnotVector, Crv -> KnotVector,
			  sizeof(CagdRType) * (CAGD_CRV_PT_LST_LEN(Crv) +
								Crv -> Order));
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    SrfPoints = Srf -> Points;

    /* For each control point in original curve - generate 13 points that    */
    /* Form a circle approximation) perpendicular to the Z axis.	     */
    for (i = i13 = 0; i < Len; i++, i13 += 13) {
	int Quad;

	switch (PType) {
	    case CAGD_PT_E3_TYPE:
		SrfPoints[X][i13] = sqrt(IRIT_SQR(CrvPoints[X][i]) +
					 IRIT_SQR(CrvPoints[Y][i]));
		SrfPoints[Y][i13] = 0.0;
		SrfPoints[Z][i13] = CrvPoints[Z][i];
		break;
	    default:
		CAGD_FATAL_ERROR(CAGD_ERR_UNSUPPORT_PT);
		break;
	}

	/* Last point is exactly the same as first one in circle - copy it.  */
	for (j = X; j <= Z; j++)
	    SrfPoints[j][i13 + 12] = SrfPoints[j][i13];

	/* The Z components are identical in all circle, while the XY        */
	/* components are functions of PolyApproxRotAngles:		     */
	for (Quad = 0, j = 1; j < 12; j++) {
	    CagdRType Angle, CosAngle, SinAngle;

	    if (j % 3 == 0)
		Quad++;
	    Angle = Quad * 90 + PolyApproxRotAngles[j % 3];
	    CosAngle = cos(IRIT_DEG2RAD(Angle));
	    SinAngle = sin(IRIT_DEG2RAD(Angle));

	    if (IRIT_FABS(CosAngle) > IRIT_FABS(SinAngle))
		CosAngle /= IRIT_FABS(CosAngle);
	    else
		SinAngle /= IRIT_FABS(SinAngle);
	    SrfPoints[X][i13 + j] = SrfPoints[X][i13] * CosAngle;
	    SrfPoints[Y][i13 + j] = SrfPoints[X][i13] * SinAngle;
	    SrfPoints[Z][i13 + j] = SrfPoints[Z][i13];
	}
    }

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_SRF_OF_REV);

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return Srf;
}
