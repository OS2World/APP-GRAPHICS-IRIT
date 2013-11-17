/******************************************************************************
* Cagd_aux.c - auxiliary routine to interface to different free from types.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

#define VEC_FIELD_TRIES		10
#define VEC_FIELD_START_STEP	1e-6

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the parametric domain of a curve.	     		             M
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
*   BspCrvDomain                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDomain, domain, parametric domain                                 M
*****************************************************************************/
void CagdCrvDomain(const CagdCrvStruct *Crv, CagdRType *TMin, CagdRType *TMax)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CPOWER_TYPE:
	    *TMin = 0.0;
	    *TMax = 1.0;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    BspCrvDomain(Crv, TMin, TMax);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Affinely reset the parametric domain of a curve, in place.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To reset its parametric domain.                               M
*   TMin:      Minimal domain's new boundary.		                     M
*   TMax:      Maximal domain's new boundary.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Modified curve, in place.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvDomain, BspKnotAffineTrans2, CagdSrfSetDomain                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvSetDomain, domain, parametric domain                              M
*****************************************************************************/
CagdCrvStruct *CagdCrvSetDomain(CagdCrvStruct *Crv,
				CagdRType TMin,
				CagdRType TMax)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    if (IRIT_APX_EQ(TMin, 0.0) && IRIT_APX_EQ(TMax, 1.0)) {
		return Crv;
	    }
	    else {
		/* Convert to a B-spline curve. */
	        Crv -> Order = Crv -> Length;
		Crv -> KnotVector = BspKnotUniformOpen(Crv -> Length,
						       Crv -> Order, NULL);
		Crv -> GType = CAGD_CBSPLINE_TYPE;
	    }
	case CAGD_CBSPLINE_TYPE:
	    BspKnotAffineTrans2(Crv -> KnotVector,
				Crv -> Order + Crv -> Length, TMin, TMax);
	    break;
	case CAGD_CPOWER_TYPE:
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve and parameter value t, evaluate the curve at t.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To evaluate at the given parametric location t.                M
*   t:        The parameter value at which the curve Crv is to be evaluated. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of curve Crv's point type. If for example the curve's      M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvEvalAtParam, BspCrvEvalAtParam, BzrCrvEvalVecAtParam,              M
*   BspCrvEvalVecAtParam, BspCrvEvalCoxDeBoor, CagdCrvEvalToPolyline         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvEval, evaluation                                                  M
*****************************************************************************/
CagdRType *CagdCrvEval(const CagdCrvStruct *Crv, CagdRType t)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvEvalAtParam(Crv, t);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvEvalAtParam(Crv, t);
	case CAGD_CPOWER_TYPE:
	    return PwrCrvEvalAtParam(Crv, t);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the parametric domain of a surface.				     M
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
*   BspSrfDomain                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfDomain, domain, parametric domain                                 M
*****************************************************************************/
void CagdSrfDomain(const CagdSrfStruct *Srf,
		   CagdRType *UMin,
		   CagdRType *UMax,
		   CagdRType *VMin,
		   CagdRType *VMax)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	case CAGD_SPOWER_TYPE:
	    *UMin = 0.0;
	    *UMax = 1.0;
	    *VMin = 0.0;
	    *VMax = 1.0;
	    break;
	case CAGD_SBSPLINE_TYPE:
	    BspSrfDomain(Srf, UMin, UMax, VMin, VMax);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Affinely reset the parametric domain of a surface, in place.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To reset its parametric domain.                               M
*   UMin:      Minimal domain's new U boundary.		                     M
*   UMax:      Maximal domain's new U boundary.		                     M
*   VMin:      Minimal domain's new V boundary.		                     M
*   VMax:      Maximal domain's new V boundary.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Modified surface, in place.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfDomain, BspKnotAffineTrans2, CagdCrvSetDomain                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfSetDomain, domain, parametric domain                              M
*****************************************************************************/
CagdSrfStruct *CagdSrfSetDomain(CagdSrfStruct *Srf,
				CagdRType UMin,
				CagdRType UMax,
				CagdRType VMin,
				CagdRType VMax)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    if (IRIT_APX_EQ_EPS(UMin, 0.0, IRIT_UEPS) &&
		IRIT_APX_EQ_EPS(UMax, 1.0, IRIT_UEPS) &&
		IRIT_APX_EQ_EPS(VMin, 0.0, IRIT_UEPS) &&
		IRIT_APX_EQ_EPS(VMax, 1.0, IRIT_UEPS)) {
		return Srf;
	    }
	    else {
		/* Convert to a B-spline surface. */
	        Srf -> UOrder = Srf -> ULength;
	        Srf -> VOrder = Srf -> VLength;
		Srf -> UKnotVector = BspKnotUniformOpen(Srf -> ULength,
							Srf -> UOrder, NULL);
		Srf -> VKnotVector = BspKnotUniformOpen(Srf -> VLength,
							Srf -> VOrder, NULL);
		Srf -> GType = CAGD_SBSPLINE_TYPE;
	    }
	case CAGD_SBSPLINE_TYPE:
	    BspKnotAffineTrans2(Srf -> UKnotVector,
				Srf -> UOrder + Srf -> ULength, UMin, UMax);
	    BspKnotAffineTrans2(Srf -> VKnotVector,
				Srf -> VOrder + Srf -> VLength, VMin, VMax);
	    break;
	case CAGD_SPOWER_TYPE:
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    break;
    }

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and parameter values u, v, evaluate the surface at (u, v). M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To evaluate at the given parametric location (u, v).            M
*   u, v:    The parameter values at which the curve Crv is to be evaluated. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of surface Srf's point type. If for example the surface's  M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvEval, BspSrfEvalAtParam, BzrSrfEvalAtParam,                       M
*   BspSrfEvalAtParam2, TrimSrfEval				             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfEval, evaluation                                                  M
*****************************************************************************/
CagdRType *CagdSrfEval(const CagdSrfStruct *Srf, CagdRType u, CagdRType v)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfEvalAtParam(Srf, u, v);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfEvalAtParam(Srf, u, v);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single freeform surface to set of triangles	     M
* approximating it. FineNess is a fineness control on result and the larger  M
t is more triangles may result. A value of 10 is a good start value.	     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To approximate into triangles.                         M
*   FineNess:         Control on accuracy, the higher the finer.             M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   FourPerFlat:      If TRUE, four triangles are created per flat surface.  M
*                     If FALSE, only 2 triangles are created.                M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrf2Polygons, BspSrf2Polygons, CagdCrv2Polyline, CagdSrf2Polylines,   M
*   CagdSrf2PolygonStrip, CagdSrf2PolygonsN				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2Polygons, evaluation, polygonal approximation                    M
*****************************************************************************/
CagdPolygonStruct *CagdSrf2Polygons(const CagdSrfStruct *Srf,
				    int FineNess,
				    CagdBType ComputeNormals,
				    CagdBType FourPerFlat,
				    CagdBType ComputeUV)
{
    CagdPolygonStruct *Pls;
    CagdSrfStruct *TSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrf2Polygons(Srf, FineNess, ComputeNormals,
				   FourPerFlat, ComputeUV);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrf2Polygons(Srf, FineNess, ComputeNormals,
				   FourPerFlat, ComputeUV);
	case CAGD_SPOWER_TYPE:
	    TSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    Pls = BzrSrf2Polygons(TSrf, FineNess, ComputeNormals,
				  FourPerFlat, ComputeUV);
	    CagdSrfFree(TSrf);
	    return Pls;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single freeform surface to set of triangles	     M
* approximating it using a uniform fixed resolution of Nu x Nv.		     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To approximate into triangles.                         M
*   Nu, Nv:           The number of uniform samples in U and V of surface.   M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   FourPerFlat:      If TRUE, four triangles are created per flat surface.  M
*                     If FALSE, only 2 triangles are created.                M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrf2Polygons, BspSrf2Polygons, CagdCrv2Polyline, CagdSrf2Polylines,   M
*   CagdSrf2PolygonStrip, CagdSrf2Polygons				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolygonsN, evaluation, polygonal approximation                   M
*****************************************************************************/
CagdPolygonStruct *CagdSrf2PolygonsN(const CagdSrfStruct *Srf,
				     int Nu,
				     int Nv,
				     CagdBType ComputeNormals,
				     CagdBType FourPerFlat,
				     CagdBType ComputeUV)
{
    CagdPolygonStruct *Pls;
    CagdSrfStruct *TSrf;

    if (Nu < 2 || Nv < 2) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SIZE);
	return FALSE;
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrf2PolygonsN(Srf, Nu, Nv, ComputeNormals,
				   FourPerFlat, ComputeUV);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrf2PolygonsN(Srf, Nu, Nv, ComputeNormals,
				   FourPerFlat, ComputeUV);
	case CAGD_SPOWER_TYPE:
	    TSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    Pls = BzrSrf2PolygonsN(TSrf, Nu, Nv, ComputeNormals,
				   FourPerFlat, ComputeUV);
	    CagdSrfFree(TSrf);
	    return Pls;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single Bspline surface to NumOfIsolines polylines   M
* in each parametric direction with SamplesPerCurve in each isoparametric    M
* curve.                                                                     M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. Attempt is made to extract isolines along C1           M
* discontinuities first.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:                 Srf to extract isoparametric curves from.           M
*   NumOfIsocurves:      To extarct from Srf in each (U or V) direction.     M
*   SamplesPerCurve:     Fineness control on piecewise linear curve          M
*                        approximation.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *: List of polygons representing a piecewise linear   M
*                         approximation of the extracted isoparamteric       M
*                         curves or NULL is case of an error.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrSrf2Polylines, IritSurface2Polylines,                M
*   IritTrimSrf2Polylines, SymbSrf2Polylines, TrimSrf2Polylines              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2Polylines, polylines, isoparametric curves                       M
*****************************************************************************/
CagdPolylineStruct *CagdSrf2Polylines(const CagdSrfStruct *Srf,
				      int NumOfIsocurves[2],
				      int SamplesPerCurve)
{
    CagdPolylineStruct *Pls;
    CagdSrfStruct *TSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrf2Polylines(Srf, NumOfIsocurves, SamplesPerCurve);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrf2Polylines(Srf, NumOfIsocurves, SamplesPerCurve);
	case CAGD_SPOWER_TYPE:
	    TSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    Pls = BzrSrf2Polylines(TSrf, NumOfIsocurves, SamplesPerCurve);
	    CagdSrfFree(TSrf);
	    return Pls;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to extract from a surface NumOfIsoline isocurve list	     M
* in each param. direction.						     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of CagdCrvStruct.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             To extract isoparametric curves from.                   M
*   NumOfIsocurves:  In each (U or V) direction.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of extracted isoparametric curves. These curves   M
*                     inherit the order and continuity of the original Srf.  M
*                     NULL is returned in case of an error.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2PCurves, BzrSrf2Curves, SymbSrf2Curves	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2Curves, curves, isoparametric curves                             M
*****************************************************************************/
CagdCrvStruct *CagdSrf2Curves(const CagdSrfStruct *Srf, int NumOfIsocurves[2])
{
    CagdCrvStruct *Crvs;
    CagdSrfStruct *TSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrf2Curves(Srf, NumOfIsocurves);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrf2Curves(Srf, NumOfIsocurves);
	case CAGD_SPOWER_TYPE:
	    TSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    Crvs = BzrSrf2Curves(TSrf, NumOfIsocurves);
	    CagdSrfFree(TSrf);
	    return Crvs;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates a vector field surface to a unit size vector. If fails, moves a  M
* tad until success. Useful for normal field evaluations.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:            Where resulting unit length vector is to be saved.       M
*   VecFieldSrf:    A surface representing a vector field.                   M
*   U, V:           Parameter locations.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdEvaluateSurfaceVecField, normal, vector field                        M
*****************************************************************************/
void CagdEvaluateSurfaceVecField(CagdVType Vec,
				 CagdSrfStruct *VecFieldSrf,
				 CagdRType U,
				 CagdRType V)
{
    CagdRType
	*R = CagdSrfEval(VecFieldSrf, U, V);

    CagdCoerceToE3(Vec, &R, -1, VecFieldSrf -> PType);

    if (IRIT_PT_SQR_LENGTH(Vec) < IRIT_SQR(IRIT_UEPS)) {
	int i = 0;
	CagdRType UMin, UMax, VMin, VMax, UMid, VMid,
	    Step = VEC_FIELD_START_STEP;

	CagdSrfDomain(VecFieldSrf, &UMin, &UMax, &VMin, &VMax);
	UMid = (UMin + UMax) * 0.5;
	VMid = (VMin + VMax) * 0.5;
	while (IRIT_PT_SQR_LENGTH(Vec) < IRIT_SQR(IRIT_UEPS) &&
	       i++ < VEC_FIELD_TRIES) {
	    U += U < UMid ? Step : -Step;
	    V += V < VMid ? Step : -Step;
	    Step *= 2.0;

	    R = CagdSrfEval(VecFieldSrf, U, V);
	    CagdCoerceToE3(Vec, &R, -1, VecFieldSrf -> PType);
	}
    }

    IRIT_PT_NORMALIZE(Vec);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve, computes its derivative curve (Hodograph).                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute its Hodograph curve.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Resulting hodograph.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, BspCrvDerive, BzrCrvDeriveRational, BspCrvDeriveRational   M
*   CagdCrvDeriveScalar, CagdCrvScalarCrvSlopeBounds			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDerive, derivatives, Hodograph                                    M
*****************************************************************************/
CagdCrvStruct *CagdCrvDerive(const CagdCrvStruct *Crv)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvDerive(Crv);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvDerive(Crv);
	case CAGD_CPOWER_TYPE:
	    return PwrCrvDerive(Crv);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve, computes the derivative of all is scalar components.  For a M
* Euclidean curve this is the same as CagdCrvDerive but for a rational curve M
* the returned curves is not the vector field but simply the derivatives of  M
* all the curve's coefficients, including the weights.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute derivatives of all its components.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Resulting derivative.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvDerive, BspCrvDerive, BzrCrvDeriveRational, BspCrvDeriveRational   M
*   CagdCrvDerive, BzrCrvDeriveScalar, BspCrvDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDeriveScalar, derivatives, Hodograph                              M
*****************************************************************************/
CagdCrvStruct *CagdCrvDeriveScalar(const CagdCrvStruct *Crv)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvDeriveScalar(Crv);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvDeriveScalar(Crv);
	case CAGD_CPOWER_TYPE:
	    return PwrCrvDeriveScalar(Crv);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute slopes' bounds to a scalar curve.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      Scalar curve to estimate its extreme slopes.                   M
*   MinSlope: Minimal slope detected.                                        M
*   MaxSlope: Maximal slope detected.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvDerive							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvScalarCrvSlopeBounds                                              M
*****************************************************************************/
void CagdCrvScalarCrvSlopeBounds(const CagdCrvStruct *Crv,
				 CagdRType *MinSlope,
				 CagdRType *MaxSlope)
{
    CagdRType MinS, MaxS, Slope, dt1,
	*Pts = Crv -> Points[1];
    int i,
	Order = Crv -> Order,
	k = IRIT_MAX(Order - 1, 1);
	
    if (CAGD_IS_BEZIER_CRV(Crv)) {
        dt1 = 1.0 / k;
        MinS = MaxS = (Pts[1] - Pts[0]) * dt1;
        for (i = 1; i < Order; i++, Pts++) {
	    Slope = (Pts[1] - Pts[0]) * dt1;
	    if (MinS > Slope)
	        MinS = Slope;
	    if (MaxS < Slope)
	        MaxS = Slope;
	}
    }
    else {
        int Len = Crv -> Length;
	CagdRType *KV, Node, NextNode,
	    *KnotVector = Crv -> KnotVector;

	/* Compute the first average value. */
	for (Node = 0.0, i = 1; i <= k; i++)
	    Node += KnotVector[i];

	MinS = IRIT_INFNTY;
	MaxS = -IRIT_INFNTY;
	for (i = 1, KV = &KnotVector[1]; i < Len; i++, KV++, Pts++) {
	    NextNode = Node + KV[k] -KV[0];
	    if ((dt1 = NextNode - Node) != 0.0)
	        dt1 = k / dt1;
	    else
	        dt1 = IRIT_INFNTY;

	    Slope = (Pts[1] - Pts[0]) * dt1;
	    if (MinS > Slope)
	        MinS = Slope;
	    if (MaxS < Slope)
	        MaxS = Slope;

	    Node = NextNode;
	}
    }

    *MinSlope = MinS;
    *MaxSlope = MaxS;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve, compute its integral curve.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute its integral curve.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Resulting integral curve.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvIntegrate, BspCrvIntegrate                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvIntegrate, integrals                                              M
*****************************************************************************/
CagdCrvStruct *CagdCrvIntegrate(const CagdCrvStruct *Crv)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvIntegrate(Crv);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvIntegrate(Crv);
	case CAGD_CPOWER_TYPE:
	    return PwrCrvIntegrate(Crv);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve assumed to be planar, computes a normal field for the curve  M
* by rotating the tangent field 90 degrees.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute a normal field for.  This normal field is well	     M
*             defined at inflection points and is not flipped there.         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Resulting normal field.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvDerive, CagdCrvDeriveScalar					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrv2DNormalField, derivatives, normal field                          M
*****************************************************************************/
CagdCrvStruct *CagdCrv2DNormalField(const CagdCrvStruct *Crv)
{
    int i;
    CagdCrvStruct
	*DCrv = CagdCrvDerive(Crv);
    CagdRType
	*PtsX = DCrv -> Points[1],
	*PtsY = DCrv -> Points[2];

    if (PtsX == NULL || PtsY == NULL) {
	CAGD_FATAL_ERROR(CAGD_ERR_ONLY_2D_OR_3D);
	return NULL;
    }

    /* Rotate 90 degrees by (x, y) -> (-y, x). */
    for (i = 0; i < DCrv -> Length; i++, PtsX++, PtsY++) {
        IRIT_SWAP(CagdRType, *PtsX, *PtsY);
	*PtsY = -*PtsY;
    }

    return DCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve, compute its moebius transformation.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To compute its moebius transformation.                         M
*   c:        The scaling coefficient - c^n is the ratio between the first   M
*	      and last weight of the curve.  				     M
*	      If c == 0, the first and last weights are made equal.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Resulting curve after the moebius transformation.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvMoebiusTransform, BspCrvMoebiusTransform                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvMoebiusTransform, moebius transformation                          M
*****************************************************************************/
CagdCrvStruct *CagdCrvMoebiusTransform(const CagdCrvStruct *Crv, CagdRType c)
{
    CagdCrvStruct *TCrv, *BzrCrv;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvMoebiusTransform(Crv, c);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvMoebiusTransform(Crv, c);
	case CAGD_CPOWER_TYPE:
	    BzrCrv = CagdCnvrtPwr2BzrCrv(Crv);
	    TCrv = BzrCrvMoebiusTransform(BzrCrv, c);
	    CagdCrvFree(BzrCrv);
	    BzrCrv = CagdCnvrtBzr2PwrCrv(TCrv);
	    CagdCrvFree(TCrv);
	    return BzrCrv;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface, computes its partial derivative in the prescibed          M
* direction Dir.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute its derivative surface in direction Dir.            M
*   Dir:      Direction of differentiation. Either U or V.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Resulting partial derivative surface.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfDerive, BspSrfDerive, BzrSrfDeriveRational, BspSrfDeriveRational   M
*   CagdSrfDeriveScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfDerive, derivatives, partial derivatives                          M
*****************************************************************************/
CagdSrfStruct *CagdSrfDerive(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf, *BzrSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfDerive(Srf, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfDerive(Srf, Dir);
	case CAGD_SPOWER_TYPE:
	    BzrSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    TSrf = BzrSrfDerive(BzrSrf, Dir);
	    CagdSrfFree(BzrSrf);
	    BzrSrf = CagdCnvrtBzr2PwrSrf(TSrf);
	    CagdSrfFree(TSrf);
	    return BzrSrf;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface, computes its partial derivative in the prescibed          M
* direction Dir of all is scalar components.				     M
* For a Euclidean surface this is the same as CagdSrfDerive but for a        M
* rational surface the returned surfaces is not the vector field but simply  M
* the derivatives of all the surface's coefficients, including the weights.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute derivatives of all its components.                  M
*   Dir:      Direction of differentiation. Either U or V.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Resulting derivative.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfDerive, BspSrfDerive, BzrSrfDeriveRational, BspSrfDeriveRational   M
*   CagdSrfDerive, BzrSrfDeriveScalar, BspSrfDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfDeriveScalar, derivatives, Hodograph                              M
*****************************************************************************/
CagdSrfStruct *CagdSrfDeriveScalar(const CagdSrfStruct *Srf,
				   CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf, *BzrSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfDeriveScalar(Srf, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfDeriveScalar(Srf, Dir);
	case CAGD_SPOWER_TYPE:
	    BzrSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    TSrf = BzrSrfDeriveScalar(BzrSrf, Dir);
	    CagdSrfFree(BzrSrf);
	    BzrSrf = CagdCnvrtBzr2PwrSrf(TSrf);
	    CagdSrfFree(TSrf);
	    return BzrSrf;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface, compute its integral surface.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To compute its integral surface.                               M
*   Dir:      Direction of integration. Either U or V.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Resulting integral surface.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfIntegrate, BspSrfIntegrate                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfIntegrate, integrals                                              M
*****************************************************************************/
CagdSrfStruct *CagdSrfIntegrate(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf, *BzrSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfIntegrate(Srf, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfIntegrate(Srf, Dir);
	case CAGD_SPOWER_TYPE:
	    BzrSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    TSrf = BzrSrfIntegrate(BzrSrf, Dir);
	    CagdSrfFree(BzrSrf);
	    BzrSrf = CagdCnvrtBzr2PwrSrf(TSrf);
	    CagdSrfFree(TSrf);
	    return BzrSrf;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface, compute its moebius transformation.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to apply the Moebius transformation to.              M
*   c:          The scaling coefficient - c^n is the ratio between the first M
*	        and last weight of the surface, along each row or column.    M
*		If c == 0, the first and last weights are made equal, in the M
*		first row/column.				             M
*   Dir:        Direction to apply the Moebius transformation, row or col.   M
*		If Dir == CAGD_BOTH_DIR, the transformation is applied to    M
*		both the row and column directions, in this order.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Resulting surface after the moebius transformation.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfMoebiusTransform, BspSrfMoebiusTransform                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfMoebiusTransform, moebius transformation                          M
*****************************************************************************/
CagdSrfStruct *CagdSrfMoebiusTransform(const CagdSrfStruct *Srf,
				       CagdRType c,
				       CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf, *BzrSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfMoebiusTransform(Srf, c, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfMoebiusTransform(Srf, c, Dir);
	case CAGD_SPOWER_TYPE:
	    BzrSrf = CagdCnvrtPwr2BzrSrf(Srf);
	    TSrf = BzrSrfMoebiusTransform(BzrSrf, c, Dir);
	    CagdSrfFree(BzrSrf);
	    BzrSrf = CagdCnvrtBzr2PwrSrf(TSrf);
	    CagdSrfFree(TSrf);
	    return BzrSrf;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve - subdivides it into two curves at the given parameter       M
* value t.								     M
*    Returns pointer to first curve in a list of two subdivided curves.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To subdivide at the prescibed parameter value t.               M
*   t:        The parameter to subdivide the curve Crv at.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of the two curves resulting from the process    M
*                     of subdivision.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvSubdivAtParams                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvSubdivAtParam, subdivision                                        M
*****************************************************************************/
CagdCrvStruct *CagdCrvSubdivAtParam(const CagdCrvStruct *Crv, CagdRType t)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvSubdivAtParam(Crv, t);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvSubdivAtParam(Crv, t);
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve - subdivides it into curves at all the given parameter       M
* values Pts.  Pts can hold the parameters in any order in the Idx point     M
* coordinate.								     M
*    Returns pointer to first curve in a list of subdivided curves.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:  Curve to split at all parameter values as prescribed by Pts.      M
*	   Bezier curves are promoted to Bspline curves in this function.    M
*   Pts:   Ordered list of parameter values (first coordinate of point) to   M
*	   split curve Crv at.						     M
*   Idx:   Index of parameter in Pts points: 0 for X, 1 for Y, etc.          M
*   Eps:   parameter closer than Eps and/or closer to boundary than Eps are  M
8	   ignored.							     M
*   Proximity:  A 3 bits marker to return if the first (last) parameter was  M
*          too close to the boundary and/or two middle parameters were too   M
*          close and one of them was ignored as follows  		     M
*          0x01 - first parameter to split at was too close to the boundary. V
*          0x02 - last parameter to split at was too close to the boundary.  V
*          0x04 - a middle parameter was too close to another parameter.     V
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    List of splitted curves, in order.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvSubdivAtParam, CagdCrvSubdivAtParams                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvSubdivAtParams2                                                   M
*****************************************************************************/
CagdCrvStruct *CagdCrvSubdivAtParams2(const CagdCrvStruct *CCrv,
				      const CagdPtStruct *Pts,
				      int Idx,
				      CagdRType Eps,
				      int *Proximity)
{
    CagdPtStruct *SortedPts;
    CagdCrvStruct *SubdivCrvs;

    assert(Idx >= 0 && Idx < 3);

    CagdInsertInterPointInit();

    for ( ; Pts != NULL; Pts = Pts -> Pnext)
        CagdInsertInterPoints(Pts -> Pt[Idx], IRIT_INFNTY, Eps);
    SortedPts = CagdInsertInterPointInit();

    SubdivCrvs = CagdCrvSubdivAtParams(CCrv, SortedPts, Eps, Proximity);

    CagdPtFreeList(SortedPts);

    return SubdivCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve - subdivides it into curves at all the given parameter       M
* values Pts.  Pts is assumed to hold the parameters in order in the first   M
* point coordinate.							     M
*    Returns pointer to first curve in a list of subdivided curves.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:  Curve to split at all parameter values as prescribed by Pts.      M
*	   Bezier curves are promoted to Bspline curves in this function.    M
*   Pts:   Ordered list of parameter values (first coordinate of point) to   M
*	   split curve Crv at.						     M
*   Eps:   parameter closer than Eps to boundary or other parameters are     M
*	   ignored.							     M
*   Proximity:  A 3 bits marker to return if the first (last) parameter was  M
*          too close to the boundary and/or two middle parameters were too   M
*          close and one of them was ignored as follows 		     M
*          0x01 - first parameter to split at was too close to the boundary. V
*          0x02 - last parameter to split at was too close to the boundary.  V
*          0x04 - a middle parameter was too close to another parameter.     V
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    List of splitted curves, in order.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvSubdivAtParam, CagdCrvSubdivAtParams2                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvSubdivAtParams                                                    M
*****************************************************************************/
CagdCrvStruct *CagdCrvSubdivAtParams(const CagdCrvStruct *CCrv,
				     const CagdPtStruct *Pts,
				     CagdRType Eps,
				     int *Proximity)
{
    CagdRType t, TMin, TMax;
    CagdCrvStruct *Crvs, *Crv,
	*RetList = NULL;
    
    *Proximity = 0;

    CagdCrvDomain(CCrv, &TMin, &TMax);

    switch (CCrv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Crv = CagdCnvrtBzr2BspCrv(CCrv);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Crv = CagdCrvCopy(CCrv);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    t = TMin;
    while (Pts != NULL) {
        if (Pts -> Pt[0] >= TMax ||
	    (Eps != 0.0 && IRIT_APX_EQ_EPS(Pts -> Pt[0], TMax, Eps))) {
	    *Proximity |= 0x02;
	    break;
	}

	/* Verify monotonicity of parameter values. */
	if (Pts -> Pt[0] > t &&
	    (Eps == 0.0 || !IRIT_APX_EQ_EPS(Pts -> Pt[0], t, Eps))) {
	    Crvs = CagdCrvSubdivAtParam(Crv, Pts -> Pt[0]);

	    CagdCrvFree(Crv);

	    Crv = Crvs -> Pnext;
	    IRIT_LIST_PUSH(Crvs, RetList);

	    t = Pts -> Pt[0];
	}
	else if (IRIT_APX_EQ_EPS(Pts -> Pt[0], t, Eps)) {
	    *Proximity |= 
	             (IRIT_APX_EQ_EPS(Pts -> Pt[0], TMin, Eps) ? 0x01 : 0x04);
	}

	Pts = Pts -> Pnext;
    }
    IRIT_LIST_PUSH(Crv, RetList);

    return CagdListReverse(RetList);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve - extracts a sub-region within the domain specified by t1    M
* and t2.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To extract a sub-region from.                                 M
*   t1, t2:    Parametric domain boundaries of sub-region.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Sub-region extracted from Crv from t1 to t2.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvRegionFromCrv, regions, subdivision                               M
*****************************************************************************/
CagdCrvStruct *CagdCrvRegionFromCrv(const CagdCrvStruct *Crv,
				    CagdRType t1,
				    CagdRType t2)
{
    CagdRType TMin, TMax;
    CagdCrvStruct *Crvs, *CpCrv;
    CagdBType
	BezCrv = FALSE,
	OpenEnd = TRUE;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    BezCrv = TRUE;
	    break;
	case CAGD_CBSPLINE_TYPE:
	    OpenEnd = BspCrvHasOpenEC(Crv);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    CagdCrvDomain(Crv, &TMin, &TMax);
    if (!CAGD_IS_BEZIER_CRV(Crv)) {           /* No limits on Bezier curves! */
        CAGD_DOMAIN_T_VERIFY(t1, TMin, TMax);
	CAGD_DOMAIN_T_VERIFY(t2, TMin, TMax);
    }

    if (IRIT_APX_EQ_EPS(t1, TMin, CAGD_EPS_ROUND_KNOT) &&
	IRIT_APX_EQ_EPS(t2, TMax, CAGD_EPS_ROUND_KNOT) &&
	OpenEnd) {
	return CagdCrvCopy(Crv);
    }

    if (t1 > t2)
	IRIT_SWAP(CagdRType, t1, t2);

    if (!OpenEnd && CAGD_IS_PERIODIC_CRV(Crv))
	Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
        CpCrv = NULL;

    if (!IRIT_APX_EQ_EPS(t1, TMin, CAGD_EPS_ROUND_KNOT) || !OpenEnd) {
    	Crvs = CagdCrvSubdivAtParam(Crv, t1);
	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	Crv = CpCrv = Crvs -> Pnext;
	Crvs -> Pnext = NULL;
	CagdCrvFree(Crvs);			   /* Free the first region. */
    }

    if (IRIT_APX_EQ_EPS(t2, TMax, CAGD_EPS_ROUND_KNOT) && OpenEnd)
	return CpCrv != NULL ? CpCrv : CagdCrvCopy(Crv);
    else {
	if (BezCrv)
	    t2 = (t2 - t1) / (TMax - t1);

	Crvs = CagdCrvSubdivAtParam(Crv, t2);

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);

    	CagdCrvFree(Crvs -> Pnext);		  /* Free the second region. */
    	Crvs -> Pnext = NULL;
    	return Crvs;				/* Returns the first region. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve - refines it at the given n knots as defined by vector t.    M
* If Replace is TRUE, the values in t replaces current knot vector.	     M
* Returns pointer to refined surface (Note a Bezier curve will be converted  M
* into a Bspline curve).                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To refine.                                                    M
*   Replace:   If TRUE, t holds knots in exactly the same length as the      M
*              length of the knot vector of Crv and t simply replaces the    M
*              knot vector.                                                  M
*   t:         Vector of knots with length of n.                             M
*   n:         Length of vector t.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A refined curve of Crv after insertion of all the      M
*                     knots as specified by vector t of length n.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvRefineAtParams, refinement, subdivision                           M
*****************************************************************************/
CagdCrvStruct *CagdCrvRefineAtParams(const CagdCrvStruct *Crv,
				     CagdBType Replace,
				     CagdRType *t,
				     int n)
{
    CagdCrvStruct *BspCrv, *TCrv;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
    	    BspCrv = CagdCnvrtBzr2BspCrv(Crv);
	    TCrv = BspCrvKnotInsertNDiff(BspCrv, Replace, t, n);
	    CagdCrvFree(BspCrv);
	    return TCrv;
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvKnotInsertNDiff(Crv, Replace, t, n);
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve that is the reversed curve of Crv by reversing the     M
* control polygon and the knot vector of Crv is a Bspline curve.             M
* See also BspKnotReverse.                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be reversed.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Reversed curve of Crv.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvReverseUV				                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvReverse, reverse                                                  M
*****************************************************************************/
CagdCrvStruct *CagdCrvReverse(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i, Len, Col,
	Length = Crv -> Length,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct
        *ReversedCrv = CagdCrvCopy(Crv);
    CagdRType *KV,
	**Points = ReversedCrv -> Points;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	case CAGD_CBSPLINE_TYPE:
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    /* Reverse the Ctl Polygon: */
    Len = (Length >> 1);
    for (Col = 0; Col < Len; Col++)
	for (i = IsNotRational; i <= MaxCoord; i++)
	    IRIT_SWAP(CagdRType,
		 Points[i][Col],
		 Points[i][Length - Col - 1]);

    /* Reverse the knot vector if it exists: */
    if (Crv -> GType == CAGD_CBSPLINE_TYPE &&
	Crv -> KnotVector != NULL) {
	KV = BspKnotReverse(Crv -> KnotVector,
			    Crv -> Order + CAGD_CRV_PT_LST_LEN(Crv));
	IritFree(ReversedCrv -> KnotVector);
	ReversedCrv -> KnotVector = KV;
    }

    return ReversedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve in which the first two (UV or XY)) coordinates are     M
* reversed from the input Crv.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be UV reversed.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   UV reversed curve of Crv.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvReverse				                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvReverseUV, reverse                                                M
*****************************************************************************/
CagdCrvStruct *CagdCrvReverseUV(const CagdCrvStruct *Crv)
{
    int i;
    CagdCrvStruct
        *RevCrv = CagdCrvCopy(Crv);
    CagdRType
        **Points = RevCrv -> Points;

    for (i = 0; i < RevCrv -> Length; i++) {
        IRIT_SWAP(CagdRType, Points[1][i], Points[2][i]);
    }

    return RevCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subdivides the given curve at all C^1 potential discontinuity locations. M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  To subdivide at all C^1 discontinuity locations.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    Curve segments result from the subdivision.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvSubdivAtParams, BspKnotAllC1Discont                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvSubdivAtAllC1Discont                                              M
*****************************************************************************/
CagdCrvStruct *CagdCrvSubdivAtAllC1Discont(const CagdCrvStruct *Crv)
{
    int i, n, Proximity;
    CagdRType *C1Disconts;
    CagdPtStruct *Pt,
	*Pts = NULL;
    CagdCrvStruct *Crvs;

    if (CAGD_IS_BEZIER_CRV(Crv) ||
	(C1Disconts = BspKnotAllC1Discont(Crv -> KnotVector,
					  Crv -> Order,
				          Crv -> Length, &n)) == NULL)
        return CagdCrvCopy(Crv);

    for (i = n - 1; i >= 0; i--) {
        Pt = CagdPtNew();
	Pt -> Pt[0] = C1Disconts[i];
	IRIT_LIST_PUSH(Pt, Pts);
    }
    IritFree(C1Disconts);

    Crvs  = CagdCrvSubdivAtParams(Crv, Pts, IRIT_EPS, &Proximity);

    CagdPtFreeList(Pts);

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve representing the same curve as Crv but with its degree M
* raised by one.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To raise its degree.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve with same geometry as Crv but with one degree  M
*                     higher.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDegreeRaise, degree raising                                       M
*****************************************************************************/
CagdCrvStruct *CagdCrvDegreeRaise(const CagdCrvStruct *Crv)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvDegreeRaise(Crv);
	case CAGD_CBSPLINE_TYPE:
	    return CagdCrvBlossomDegreeRaise(Crv);
	case CAGD_CPOWER_TYPE:
	    return PwrCrvDegreeRaise(Crv);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve representing the same curve as Crv but with its degree M
* raised by one.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To raise its degree.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve with same geometry as Crv but with one degree  M
*                     higher.                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDegreeReduce, degree raising                                      M
*****************************************************************************/
CagdCrvStruct *CagdCrvDegreeReduce(const CagdCrvStruct *Crv)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvDegreeReduce(Crv);
	case CAGD_CBSPLINE_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_BSPLINE_NO_SUPPORT);
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new curve representing the same curve as Crv but with its degree M
* raised to NewOrder							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        To raise its degree.                                         M
*   NewOrder:   Expected new order of the raised curve.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A curve with same geometry as Crv but with order that  M
*                     is equal to NewOrder.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvDegreeRaiseN, degree raising                                      M
*****************************************************************************/
CagdCrvStruct *CagdCrvDegreeRaiseN(const CagdCrvStruct *Crv, int NewOrder)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvDegreeRaiseN(Crv, NewOrder);
	case CAGD_CBSPLINE_TYPE:
	    return CagdCrvBlossomDegreeRaiseN(Crv, NewOrder);
	case CAGD_CPOWER_TYPE:
	    return PwrCrvDegreeRaiseN(Crv, NewOrder);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface representing the same surface as Srf but with its    M
* degree raised by one.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To raise its degree.                                          M
*   Dir:       Direction of degree raising. Either U or V.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with same geometry as Srf but with one       M
*                     degree higher.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfDegreeRaise, BspSrfDegreeRaise, TrimSrfDegreeRaise                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfDegreeRaise, degree raising                                       M
*****************************************************************************/
CagdSrfStruct *CagdSrfDegreeRaise(const CagdSrfStruct *Srf,
				  CagdSrfDirType Dir)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfDegreeRaise(Srf, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return CagdSrfBlossomDegreeRaise(Srf, Dir);
	case CAGD_SPOWER_TYPE:
	    return PwrSrfDegreeRaise(Srf, Dir);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface, identical to the original but with higher degrees,  M
* as prescribed by NewUOrder, NewVOrder.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise its degree.                                         M
*   NewUOrder:  New U order of Srf.					     M
*   NewVOrder:  New V order of Srf.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with higher degrees as prescribed by	     M
*                     NewUOrder/NewVOrder.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BzrSrfDegreeRaise, TrimSrfDegreeRaise                M
*   BspSrfDegreeRaise, BzrSrfDegreeRaiseN, BspSrfDegreeRaiseN                M
**                                                                           *
* KEYWORDS:                                                                  M
*   CagdSrfDegreeRaiseN, degree raising                                      M
*****************************************************************************/
CagdSrfStruct *CagdSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				   int NewUOrder,
				   int NewVOrder)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
            return BzrSrfDegreeRaiseN(Srf, NewUOrder, NewVOrder);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfDegreeRaiseN(Srf, NewUOrder, NewVOrder);
	case CAGD_SPOWER_TYPE:
            return PwrSrfDegreeRaiseN(Srf, NewUOrder, NewVOrder);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts an isoparametric curve from the surface Srf in direction Dir at   M
* the parameter value of t.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract an isoparametric curve from.                       M
*   t:         Parameter value of extracted isoparametric curve.             M
*   Dir:       Direction of extracted isoparametric curve. Either U or V.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   An isoparametric curve of Srf. This curve inherit the M
*                      order and continuity of surface Srf in direction Dir. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfCrvFromSrf, BspSrfCrvFromSrf, CagdCrvFromMesh, BzrSrfCrvFromMesh,  M
*   BspSrfCrvFromMesh, TrngCrvFromTriSrf                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvFromSrf, isoparametric curves, curve from surface                 M
*****************************************************************************/
CagdCrvStruct *CagdCrvFromSrf(const CagdSrfStruct *Srf,
			      CagdRType t,
			      CagdSrfDirType Dir)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfCrvFromSrf(Srf, t, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfCrvFromSrf(Srf, t, Dir);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts one boundary curve of the given surface.	                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract the boundary from.		                     M
*   Bndry:     The boundary to extract.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The extracted boundary curve.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBndryCrvFromSrf, isoparametric curves, curve from surface            M
*****************************************************************************/
CagdCrvStruct *CagdBndryCrvFromSrf(const CagdSrfStruct *Srf,
				   CagdSrfBndryType Bndry)
{
    CagdRType UMin, UMax, VMin, VMax;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    switch (Bndry) {
        default:
	    assert(0);
	case CAGD_U_MIN_BNDRY:
	    return CagdCrvFromSrf(Srf, UMin, CAGD_CONST_U_DIR);
	case CAGD_U_MAX_BNDRY:
	    return CagdCrvFromSrf(Srf, UMax, CAGD_CONST_U_DIR);
	case CAGD_V_MIN_BNDRY:
	    return CagdCrvFromSrf(Srf, VMin, CAGD_CONST_V_DIR);
	case CAGD_V_MAX_BNDRY:
	    return CagdCrvFromSrf(Srf, VMax, CAGD_CONST_V_DIR);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts the four boundary curves of the given surface.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract the boundary from.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct **:  A pointer to a static vector of four curve pointers,  M
*		       representing the four boundaries of surface Srf       M
*		       in order of UMin, UMax, VMin, VMax.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBndryCrvsFromSrf, isoparametric curves, curve from surface           M
*****************************************************************************/
CagdCrvStruct **CagdBndryCrvsFromSrf(const CagdSrfStruct *Srf)
{
    static CagdCrvStruct *Crvs[4];
    CagdRType UMin, UMax, VMin, VMax;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    Crvs[0] = CagdCrvFromSrf(Srf, UMin, CAGD_CONST_U_DIR);
    Crvs[1] = CagdCrvFromSrf(Srf, UMax, CAGD_CONST_U_DIR);
    Crvs[2] = CagdCrvFromSrf(Srf, VMin, CAGD_CONST_V_DIR);
    Crvs[3] = CagdCrvFromSrf(Srf, VMax, CAGD_CONST_V_DIR);

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a curve from the mesh of surface Srf in direction Dir at index    M
* Index.                                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract a curve from.  		                     M
*   Index:     Index along the mesh of Srf to extract the curve from.        M
*   Dir:       Direction of extracted curve. Either U or V.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve from Srf. This curve inherit the  order and   M
*                      continuity of surface Srf in direction Dir. However,  M
*                      thiscurve is not on surface Srf, in general.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvFromSrf, BzrSrfCrvFromSrf, BspSrfCrvFromSrf, BzrSrfCrvFromMesh,   M
*   BspSrfCrvFromMesh, CagdCrvToMesh                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvFromMesh, isoparametric curves, curve from mesh                   M
*****************************************************************************/
CagdCrvStruct *CagdCrvFromMesh(const CagdSrfStruct *Srf,
			       int Index,
			       CagdSrfDirType Dir)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfCrvFromMesh(Srf, Index, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfCrvFromMesh(Srf, Index, Dir);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Substitutes a row/column of surface Srf from the given curve Crv at        M
* surface direction Dir and mesh index Index. Curve must have the same       M
* PtType/Length as the surface in the selected direction.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To substitute into the surface Srf.                           M
*   Index:     Of mesh where the curve Crv should be substituted in.         M
*   Dir:       Either U or V.                                                M
*   Srf:       That a row or a column of should be replaced by Crv.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvFromSrf, CagdCrvFromMesh                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvToMesh, curve from mesh                                           M
*****************************************************************************/
void CagdCrvToMesh(const CagdCrvStruct *Crv,
		   int Index,
		   CagdSrfDirType Dir,
		   CagdSrfStruct *Srf)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j,
	Length = Crv -> Length,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *CrvP, *SrfP;

    if (Crv -> PType != Srf -> PType ||
	Length != (Dir == CAGD_CONST_U_DIR ? VLength : ULength))
	CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    if (Index + 1 > ULength)
		CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i] + Index * CAGD_NEXT_U(Srf);
		for (j = 0; j < Length; j++) {
		    *SrfP = *CrvP++;
		    SrfP += CAGD_NEXT_V(Srf);
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    if (Index + 1 > VLength)
		CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i] + Index * CAGD_NEXT_V(Srf);
		for (j = 0; j < Length; j++) {
		    *SrfP = *CrvP++;
		    SrfP += CAGD_NEXT_U(Srf);
		}
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface - subdivides it into two sub-surfaces at given parametric  M
* value t in the given direction Dir.                                        M
*   Returns pointer to first surface in a list of two subdivided surfaces.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To subdivide at the prescibed parameter value t.               M
*   t:        The parameter to subdivide the curve Crv at.                   M
*   Dir:      Direction of subdivision. Either U or V.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A list of the two surfaces resulting from the process  M
*                     of subdivision.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfSubdivAtParam, subdivision                                        M
*****************************************************************************/
CagdSrfStruct *CagdSrfSubdivAtParam(const CagdSrfStruct *Srf,
				    CagdRType t,
				    CagdSrfDirType Dir)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfSubdivAtParam(Srf, t, Dir);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfSubdivAtParam(Srf, t, Dir);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface - extracts a sub-region within the domain specified by t1  M
* and t2, in the direction Dir.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract a sub-region from.                                 M
*   t1, t2:    Parametric domain boundaries of sub-region.                   M
*   Dir:       Direction of region extraction. Either U or V.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Sub-region extracted from Srf from t1 to t2.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfRegionFromSrf, regions, subdivision                               M
*****************************************************************************/
CagdSrfStruct *CagdSrfRegionFromSrf(const CagdSrfStruct *Srf,
				    CagdRType t1,
				    CagdRType t2,
				    CagdSrfDirType Dir)
{
    CagdRType TMin, TMax, R1, R2;
    CagdSrfStruct *Srfs, *CpSrf;
    CagdBType
	OpenEnd = FALSE;

    if (t1 > t2)
	IRIT_SWAP(CagdRType, t1, t2);

    if (Dir == CAGD_CONST_U_DIR)
	CagdSrfDomain(Srf, &TMin, &TMax, &R1, &R2);
    else
	CagdSrfDomain(Srf, &R1, &R2, &TMin, &TMax);

    if (!CAGD_IS_BEZIER_SRF(Srf)) {         /* No limits on Bezier surfaces! */
	CAGD_DOMAIN_T_VERIFY(t1, TMin, TMax);
	CAGD_DOMAIN_T_VERIFY(t2, TMin, TMax);
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    /* Update t2 to be between t1 and TMax as it will come back     */
	    /* after the first subdivision to be between zero and one.      */
	    t2 = 1.0 - (1.0 - t2) / (1.0 - t1);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    OpenEnd = BspSrfHasOpenECDir(Srf, Dir);
	    break;
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }

    if (!OpenEnd &&
	(Dir == CAGD_CONST_U_DIR ? CAGD_IS_UPERIODIC_SRF(Srf)
				 : CAGD_IS_VPERIODIC_SRF(Srf))) {
	Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    }
    else
        CpSrf = NULL;

    if (!IRIT_APX_EQ_EPS(t1, TMin, CAGD_EPS_ROUND_KNOT) || !OpenEnd) {
	Srfs = CagdSrfSubdivAtParam(Srf, t1, Dir);
	if (CpSrf != NULL)
	    CagdSrfFree(CpSrf);
	Srf = CpSrf = Srfs -> Pnext;
	Srfs -> Pnext = NULL;
	CagdSrfFree(Srfs);			   /* Free the first region. */
    }

    if (IRIT_APX_EQ_EPS(t2, TMax, CAGD_EPS_ROUND_KNOT) && OpenEnd)
	return CpSrf != NULL ? CpSrf : CagdSrfCopy(Srf);
    else {
	Srfs = CagdSrfSubdivAtParam(Srf, t2, Dir);

	if (CpSrf != NULL)
	    CagdSrfFree(CpSrf);

    	CagdSrfFree(Srfs -> Pnext);		  /* Free the second region. */
    	Srfs -> Pnext = NULL;
	return Srfs;				/* Returns the first region. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface - refines it at the given n knots as defined by vector t.  M
*   If Replace is TRUE, the values in t replaces current knot vector.	     M
*   Returns pointer to refined surface (Note a Bezier surface will be        M
* converted into a Bspline surface).                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To refine.                                                    M
*   Dir:       Direction of refinement. Either U or V.                       M
*   Replace:   If TRUE, t holds knots in exactly the same length as the      M
*              length of the knot vector of Srf and t simply replaces the    M
*              knot vector.                                                  M
*   t:         Vector of knots with length of n.                             M
*   n:         Length of vector t.                                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A refined curve of Srf after insertion of all the      M
*                     knots as specified by vector t of length n.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfRefineAtParams, refinement, subdivision                           M
*****************************************************************************/
CagdSrfStruct *CagdSrfRefineAtParams(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     CagdBType Replace,
				     CagdRType *t,
				     int n)
{
    CagdSrfStruct *BspSrf, *TSrf;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
    	    BspSrf = CagdCnvrtBzr2BspSrf(Srf);
	    TSrf = BspSrfKnotInsertNDiff(BspSrf, Dir, Replace, t, n);
	    CagdSrfFree(BspSrf);
	    return TSrf;
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfKnotInsertNDiff(Srf, Dir, Replace, t, n);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve Crv and a parameter value t, returns the (unit) tangent      M
* direction of Crv at t.                                                     M
*   The unnormalized normal does not equal dC/dt in its magnitude, only in   M
* its direction.							     M 
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute (unit) tangent vector for.                         M
*   t:         Location where to evaluate the tangent of Crv.                M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, returned is an unnormalized vector in the right     M
*	       direction of the tangent.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit tanegnt  M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvTangent, tangent                                                  M
*****************************************************************************/
CagdVecStruct *CagdCrvTangent(const CagdCrvStruct *Crv,
			      CagdRType t,
			      CagdBType Normalize)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvTangent(Crv, t, Normalize);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvTangent(Crv, t, Normalize);
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve Crv and a parameter value t, returns the (unit) binormal     M
* direction of Crv at t.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute (unit) binormal vector for.                        M
*   t:         Location where to evaluate the binormal of Crv.               M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit binormal M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvBiNormal, binormal                                                M
*****************************************************************************/
CagdVecStruct *CagdCrvBiNormal(const CagdCrvStruct *Crv,
			       CagdRType t,
			       CagdBType Normalize)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvBiNormal(Crv, t, Normalize);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvBiNormal(Crv, t, Normalize);
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve Crv and a parameter value t, returns the (unit) normal       M
* direction of Crv at t.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute (unit) normal vector for.                          M
*   t:         Location where to evaluate the normal of Crv.                 M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit normal   M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvNormal, normal                                                    M
*****************************************************************************/
CagdVecStruct *CagdCrvNormal(const CagdCrvStruct *Crv,
			     CagdRType t,
			     CagdBType Normalize)
{
    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrvNormal(Crv, t, Normalize);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrvNormal(Crv, t, Normalize);
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a curve Crv and a parameter value t, returns the (unit) normal       M
* direction of Crv at t, that is consistent over inflection points.          M
*   That is, this normal is not flipped over inflection points and is always M
* 90 rotation from the tangent vector.					     M
*   Needless to say, this function is for two dimensional palanr curves.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To compute (unit) normal vector for.                          M
*   t:         Location where to evaluate the normal of Crv.                 M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit normal   M
*                     information.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvNormalXY, normal                                                  M
*****************************************************************************/
CagdVecStruct *CagdCrvNormalXY(const CagdCrvStruct *Crv,
			       CagdRType t,
			       CagdBType Normalize)
{
    CagdRType R;
    CagdVecStruct
	*Vec = NULL;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Vec = BzrCrvTangent(Crv, t, FALSE);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Vec = BspCrvTangent(Crv, t, FALSE);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    /* Rotate 90 degrees: (x, y) -> (y, -x). */
    R = Vec -> Vec[0];
    Vec -> Vec[0] = Vec -> Vec[1];
    Vec -> Vec[1] = -R;

    if (Normalize)
        IRIT_PT_NORMALIZE(Vec -> Vec);

    return Vec;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface Srf and a parameter values u, v, returns the (unit)        M
* tangent vector of Srf in direction Dir.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To compute (unit) tangent vector for.                         M
*   u, v:      Location where to evaluate the tangent of Srf.                M
*   Dir:       Direction of tangent. Either U or V.			     *
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit tangent  M
*                     information.                                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfTangent, BspSrfTangent					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfTangent, tangent                                                  M
*****************************************************************************/
CagdVecStruct *CagdSrfTangent(const CagdSrfStruct *Srf,
			      CagdRType u,
			      CagdRType v,
			      CagdSrfDirType Dir,
			      CagdBType Normalize)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfTangent(Srf, u, v, Dir, Normalize);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfTangent(Srf, u, v, Dir, Normalize);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface Srf and a parameter values u, v, returns the (unit) normal M
* vector of Srf.                                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To compute (unit) normal vector for.                          M
*   u, v:      Location where to evaluate the normal of Srf.                 M
*   Normalize: If TRUE, attempt is made to normalize the returned vector.    M
*              If FALSE, length is a function of given parametrization.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the unit normal   M
*                     information.                                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfNormal, BspSrfNormal, SymbSrfNormalSrf, TrngTriSrfNrml,	     M
* BspSrfMeshNormals							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfNormal, normal                                                    M
*****************************************************************************/
CagdVecStruct *CagdSrfNormal(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v,
			     CagdBType Normalize)
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrfNormal(Srf, u, v, Normalize);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrfNormal(Srf, u, v, Normalize);
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }
}
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a new parametric direction OrthoUVDir that is orthogonal, in    M
* Eculidean space, to given parametric UVDir at parametric position UV of    M
* Srf.									     M
*   Clearly both Srf(OrthoUVDir) and Srf(UVDir) are in the tangent space.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to compute orthogonal direction to, in its tangent plane. M
*   UV:    Location on Srf where to compute the orthogonal direction.        M
*   UVDir: Direction to compute its orthogonal direction in Euclidean space. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdUVType *:   UV direction that is orthogonal to UVDir, in Euclidean   M
*		    space, allocated statically.  NULL, if error.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfUVDirOrthoE3                                                      M
*****************************************************************************/
CagdUVType *CagdSrfUVDirOrthoE3(const CagdSrfStruct *Srf,
				const CagdUVType *UV,
				const CagdUVType *UVDir)
{
    IRIT_STATIC_DATA CagdUVType OrthoUVDir;
    int i;
    CagdRType Desc;
    CagdVecStruct *V;
    CagdVType dSdU, dSdV, Nrml, E3UVDir, E3OrthoUVDir;

    /* Compute the two tangent vectors and normal of Srf at UV. */
    V = CagdSrfTangent(Srf, (*UV)[0], (*UV)[1], CAGD_CONST_V_DIR, FALSE);
    IRIT_VEC_COPY(dSdU, V -> Vec);
    V = CagdSrfTangent(Srf, (*UV)[0], (*UV)[1], CAGD_CONST_U_DIR, FALSE);
    IRIT_VEC_COPY(dSdV, V -> Vec);

    IRIT_CROSS_PROD(Nrml, dSdU, dSdV);

    /* Find UVDir and its orthogonal in Srf tangent/Euclidean space. */
    for (i = 0; i < 3; i++)
        E3UVDir[i] = (*UVDir)[0] * dSdU[i] + (*UVDir)[1] * dSdV[i];

    IRIT_CROSS_PROD(E3OrthoUVDir, Nrml, E3UVDir);
    if (IRIT_PT_APX_EQ_ZERO_EPS(E3OrthoUVDir, IRIT_UEPS))
	return NULL;

    /* We now need to solve for the parameteric direction that would yield */
    /* E3OrthUVDir in the Euclidean space, at location UV on the surface.  */
    /*  We have three dependent equations in X, Y, and Z and just two      */
    /* unknowns, U and V.  Select the two equations that are the largest.  */
    if (IRIT_FABS(Nrml[2]) > IRIT_FABS(Nrml[1]) &&
	IRIT_FABS(Nrml[2]) > IRIT_FABS(Nrml[0])) {
        /* Use the XY equations. */
	if (IRIT_FABS(Desc = dSdU[0] * dSdV[1] - dSdU[1] * dSdV[0]) < IRIT_UEPS)
	    return NULL;

	OrthoUVDir[0] = (E3OrthoUVDir[0] * dSdV[1] - E3OrthoUVDir[1] * dSdV[0])
								       / Desc;
	OrthoUVDir[1] = (dSdU[0] * E3OrthoUVDir[1] - dSdU[1] * E3OrthoUVDir[0])
								       / Desc;
    }
    else if (IRIT_FABS(Nrml[1]) > IRIT_FABS(Nrml[2]) &&
	     IRIT_FABS(Nrml[1]) > IRIT_FABS(Nrml[0])) {
        /* Use the XZ equations. */
	if (IRIT_FABS(Desc = dSdU[0] * dSdV[2] - dSdU[2] * dSdV[0]) < IRIT_UEPS)
	    return NULL;

	OrthoUVDir[0] = (E3OrthoUVDir[0] * dSdV[2] - E3OrthoUVDir[2] * dSdV[0])
								       / Desc;
	OrthoUVDir[1] = (dSdU[0] * E3OrthoUVDir[2] - dSdU[2] * E3OrthoUVDir[0])
								       / Desc;
    }
    else {
        /* Use the YZ equations. */
	if (IRIT_FABS(Desc = dSdU[1] * dSdV[2] - dSdU[2] * dSdV[1]) < IRIT_UEPS)
	    return NULL;

	OrthoUVDir[0] = (E3OrthoUVDir[1] * dSdV[2] - E3OrthoUVDir[2] * dSdV[1])
								       / Desc;
	OrthoUVDir[1] = (dSdU[1] * E3OrthoUVDir[2] - dSdU[2] * E3OrthoUVDir[1])
								       / Desc;
     }

#ifdef DEBUG
    /* Check we got it right... */
    for (i = 0; i < 3; i++)
	E3OrthoUVDir[i] = OrthoUVDir[0] * dSdU[i] + OrthoUVDir[1] * dSdV[i];

    IRIT_VEC_NORMALIZE(E3OrthoUVDir);
    IRIT_VEC_NORMALIZE(E3UVDir);
    if (IRIT_FABS(IRIT_DOT_PROD(E3OrthoUVDir, E3UVDir)) > IRIT_EPS)
	IRIT_WARNING_MSG("CagdSrfFindUVDirOrthoE3 orthogonality failed\n");
#endif /* DEBUG */

    IRIT_VEC2D_NORMALIZE(OrthoUVDir);

    return &OrthoUVDir;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface that is the reversed surface of Srf by reversing the M
* control mesh and the knot vector (if Bspline surface) of Srf in the U      M
* direction. See also BspKnotReverse.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be reversed.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Reversed surface of Srf.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfReverse2, CagdSrfReverseDir                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfReverse, reverse                                                  M
*****************************************************************************/
CagdSrfStruct *CagdSrfReverse(const CagdSrfStruct *Srf)
{
    return CagdSrfReverseDir(Srf, CAGD_CONST_U_DIR);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface that is the reversed surface of Srf by reversing the M
* control mesh and the knot vector (if Bspline surface) of Srf in the Dir    M
* direction. See also BspKnotReverse.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be reversed.                                               M
*   Dir:       Direction to reverse the Mesh along. Either U or V.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Reversed surface of Srf.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfReverse2                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfReverseDir, reverse                                               M
*****************************************************************************/
CagdSrfStruct *CagdSrfReverseDir(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, Len, Row, Col,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	*ReversedSrf = CagdSrfCopy(Srf);
    CagdRType *KV,
	**Points = ReversedSrf -> Points;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
	    break;
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }

    /* Reverse the Mesh: */
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    Len = (ULength >> 1);
	    for (Row = 0; Row < VLength; Row++)
		for (Col = 0; Col < Len; Col++)
		    for (i = IsNotRational; i <= MaxCoord; i++)
			IRIT_SWAP(CagdRType,
			     Points[i][Row * ULength + Col],
			     Points[i][Row * ULength + ULength - Col - 1]);
	    break;
	case CAGD_CONST_V_DIR:
	    Len = (VLength >> 1);
	    for (Col = 0; Col < ULength; Col++)
	        for (Row = 0; Row < Len; Row++)
		    for (i = IsNotRational; i <= MaxCoord; i++)
			IRIT_SWAP(CagdRType,
			     Points[i][Row * ULength + Col],
			     Points[i][(VLength - Row - 1) * ULength + Col]);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
    }

    /* Reverse the U/V knot vector if it exists: */
    if (Srf -> GType == CAGD_SBSPLINE_TYPE) {
        switch (Dir) {
	    case CAGD_CONST_U_DIR:
	        KV = BspKnotReverse(Srf -> UKnotVector,
				    Srf -> UOrder + CAGD_SRF_UPT_LST_LEN(Srf));
		IritFree(ReversedSrf -> UKnotVector);
		ReversedSrf -> UKnotVector = KV;
		break;
	    case CAGD_CONST_V_DIR:
	        KV = BspKnotReverse(Srf -> VKnotVector,
				    Srf -> VOrder + CAGD_SRF_VPT_LST_LEN(Srf));
		IritFree(ReversedSrf -> VKnotVector);
		ReversedSrf -> VKnotVector = KV;
		break;
            default:
	        assert(0);

	}
    }

    return ReversedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface that is the reversed surface of Srf by flipping the  M
* U and the V directions of the surface.				     M
* See also BspKnotReverse.		                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To be reversed.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Reversed surface of Srf.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfReverse, CagdSrfReverseDir                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfReverse2, reverse                                                 M
*****************************************************************************/
CagdSrfStruct *CagdSrfReverse2(const CagdSrfStruct *Srf)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, Row, Col,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	*ReversedSrf = CagdSrfCopy(Srf);
    CagdRType
	* const *Points = Srf -> Points;
    CagdRType
	**RevPoints = ReversedSrf -> Points;

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	case CAGD_SBSPLINE_TYPE:
	    break;
	case CAGD_SPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }

    /* Reverse the Mesh: */
    for (Row = 0; Row < VLength; Row++)
	for (Col = 0; Col < ULength; Col++)
	    for (i = IsNotRational; i <= MaxCoord; i++)
		RevPoints[i][Col * VLength + Row] =
		    Points[i][Row * ULength + Col];

    /* Swap the U and the V knot vectors if the exists: */
    if (Srf -> GType == CAGD_SBSPLINE_TYPE) {
	IRIT_SWAP(CagdRType *,
	     ReversedSrf -> UKnotVector, ReversedSrf -> VKnotVector);
    }

    /* And swap the orders and lengths. */
    IRIT_SWAP(int, ReversedSrf -> UOrder, ReversedSrf -> VOrder);
    IRIT_SWAP(int, ReversedSrf -> ULength, ReversedSrf -> VLength);

    return ReversedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examines if the given curve is totally on one side of the line	     M
* prescribed by points (X1, Y1) and (X2, Y2), in the XY plane.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   Curve to examine if totally on one side of the line, in XY plane. M
*   X1, Y1:   First points defining the line in the XY plane.		     M
*   X2, Y2:   First points defining the line in the XY plane.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if Crv totlally on one side of the line between        M
*		 (X1, Y1) and (X2, Y2), in the XY plane, or FALSE otherwise. M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvOnOneSideOfLine                                                   M
*****************************************************************************/
CagdBType CagdCrvOnOneSideOfLine(const CagdCrvStruct *Crv,
				 CagdRType X1,
				 CagdRType Y1,
				 CagdRType X2,
				 CagdRType Y2)
{
    CagdBType
	IsRational = CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	Len = Crv -> Length;
    CagdRType R,
	Sign = 0.0;
    CagdRType
	* const *Pts = Crv -> Points;
    CagdLType Ln;

    /* Make sure the two points are not one. */
    if (IRIT_APX_EQ_EPS(X1, X2, IRIT_UEPS) && IRIT_APX_EQ_EPS(Y1, Y2, IRIT_UEPS))
	return FALSE;

    /* Build the line equation. */
    Ln[0] = Y2 - Y1;
    Ln[1] = X1 - X2;
    R = 1.0 / sqrt(IRIT_SQR(Ln[0]) + IRIT_SQR(Ln[1]));
    IRIT_PT2D_SCALE(Ln, R);
    Ln[2] = -(Ln[0] * X1 + Ln[1] * Y1);

    /* Examine the sign of all control points of Crv with respect to line. */
    for (i = 0; i < Len; i++) {
        if (Sign == 0.0)
	    Sign = Ln[0] * Pts[1][i] + Ln[1] * Pts[2][i] + Ln[2];
	else if (Sign * (Ln[0] * Pts[1][i] +
			 Ln[1] * Pts[2][i] +
			 Ln[2] * (IsRational ? Pts[0][i] : 1.0)) < 0.0)
	    return FALSE;
    }    

    return TRUE;
}
