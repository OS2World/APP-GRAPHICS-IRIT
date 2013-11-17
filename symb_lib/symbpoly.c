/******************************************************************************
* SymbPoly.c - Generic Curve/Surface to polygon/polylines conversion routines.*
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include "symb_loc.h"
#include "geom_lib.h"

#define CURVE_PTS_DIST_RESOLUTION 10
#define SYMB_REL_TOL_BOUND	  1e-7
#define SYMB_CRV_POLY_EPS	  1e-4
#define SYMB_CRV_POLY_OUTREACH	  1000

#define SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, Axis, Inx) \
    (WPoints == NULL ? Points[Axis][Inx] : Points[Axis][Inx] / WPoints[Inx]);

IRIT_STATIC_DATA CagdCrvStruct
    *GlblDeriv1MagSqrCrv = NULL,
    *GlblCrvtrCrv = NULL;
IRIT_STATIC_DATA CagdRType GlblCrvtrCrvTMin, GlblCrvtrCrvTMax;
IRIT_STATIC_DATA SymbCrv2PolylineTlrncErrorFuncType
    SymbCrv2PolylineTlrncErrorFunc = NULL;

static CagdPolylineStruct *SymbCrv2OptiTlrncPolyline(const CagdCrvStruct *Crv,
						     CagdRType Tolerance);
static CagdPtStruct *SymbCrv2OptiTlrncPolyAux(const CagdCrvStruct *Crv,
					      CagdRType Tolerance,
					      CagdRType TMin,
					      CagdRType TMax,
					      CagdBType AddFirstPt);
static CagdPolylineStruct *SymbCrv2OptiCrvtrPolyline(const CagdCrvStruct *Crv,
						     int SamplesPerCurve);
static CagdRType CrvCurvatureEvalFunc(CagdRType t);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single surface to set of triangles approximating it.  M
*   FineNess is a fineness control on result and the larger it is more       M
* triangles may result.							     M
*   A value of 10 is a good starting value.				     M
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
*   CagdPolygonStruct *:   A list of polygons with optional normal and/or    M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrf2Polygons, IritSurface2Polygons, IritTrimSrf2Polygons,             M
*   BspSrf2Polygons, TrimSrf2Polygons			                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2Polygons, polygonization, surface approximation                  M
*****************************************************************************/
CagdPolygonStruct *SymbSrf2Polygons(const CagdSrfStruct *Srf,
				    int FineNess,
				    CagdBType ComputeNormals,
				    CagdBType FourPerFlat,
				    CagdBType ComputeUV)
{
    /* Make sure we do not deal with constant surfaces. */
    if (Srf -> UOrder < 2 || Srf -> VOrder < 2) {
        SYMB_FATAL_ERROR(SYMB_ERR_POLY_CONST_SRF);
	return NULL;
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrf2Polygons(Srf, FineNess, ComputeNormals, FourPerFlat,
								   ComputeUV);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrf2Polygons(Srf, FineNess, ComputeNormals, FourPerFlat,
								   ComputeUV);
	case CAGD_SPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single surface to NumOfIsolines polylines in each   M
* parametric direction with SamplesPerCurve in each isoparametric curve.     M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. Attempt is made to extract isolines along C1           M
* discontinuities first.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           Srf to extract isoparametric curves from.                 M
*   NumOfIsocurves: To extarct from Srf in each (U or V) direction.          M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the	     M
*		       isocurve's curvature.				     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *: List of polylines representing a piecewise linear  M
*                         approximation of the extracted isoparamteric       M
*                         curves or NULL is case of an error.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2Polylines, BzrSrf2Polylines, IritSurface2Polylines,	             M
*   IritTrimSrf2Polylines, TrimSrf2Polylines			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2Polylines, polylines, isoparametric curves                       M
*****************************************************************************/
CagdPolylineStruct *SymbSrf2Polylines(const CagdSrfStruct *Srf,
				      int NumOfIsocurves[2],
				      CagdRType TolSamples,
				      SymbCrvApproxMethodType Method)
{
    CagdCrvStruct *Crv, *Crvs, *TCrvs;
    CagdPolylineStruct *Poly, *Polys;

    switch (Method) {
	case SYMB_CRV_APPROX_TOLERANCE:
	case SYMB_CRV_APPROX_CURVATURE:
	    Crvs = SymbSrf2Curves(Srf, NumOfIsocurves);

	    TCrvs = SymbCrvsSplitPoleParams(Crvs, SYMB_CRV_POLY_EPS,
					    SYMB_CRV_POLY_OUTREACH);
	    CagdCrvFreeList(Crvs);
	    Crvs = TCrvs;

	    Polys = NULL;
	    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	        if (Method == SYMB_CRV_APPROX_TOLERANCE) {
		    Poly = SymbCrv2OptiTlrncPolyline(Crv, TolSamples);
		}
		else {
		    Poly = SymbCrv2OptiCrvtrPolyline(Crv,
						IRIT_MAX((int) TolSamples, 2));
		}
		Poly -> Pnext = Polys;
		Polys = Poly;
	    }

	    CagdCrvFreeList(Crvs);
	    return Polys;	
	default:
	case SYMB_CRV_APPROX_UNIFORM:
	    switch (Srf -> GType) {
		case CAGD_SBEZIER_TYPE:
		    return BzrSrf2Polylines(Srf, NumOfIsocurves,
					    (int) TolSamples);
		case CAGD_SBSPLINE_TYPE:
		    return BspSrf2Polylines(Srf, NumOfIsocurves,
					    (int) TolSamples);
		case CAGD_SPOWER_TYPE:
		    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
		    return NULL;
		default:
		    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);
		    return NULL;
	    }
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
*   BspSrf2PCurves, BzrSrf2Curves, CagdSrf2Curves	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrf2Curves, curves, isoparametric curves                             M
*****************************************************************************/
CagdCrvStruct *SymbSrf2Curves(const CagdSrfStruct *Srf, int NumOfIsocurves[2])
{
    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    return BzrSrf2Curves(Srf, NumOfIsocurves);
	case CAGD_SBSPLINE_TYPE:
	    return BspSrf2Curves(Srf, NumOfIsocurves);
	case CAGD_SPOWER_TYPE:
	    SYMB_FATAL_ERROR(SYMB_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_SRF);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to approx. a single curve as a polyline with TolSamples          M
* samples/tolerance. Polyline is always E3 CagdPolylineStruct type.	     M
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           To approximate as a polyline.                             M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the	     M
*		       isocurve's curvature.				     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*   OptiLin:       If TRUE, optimize linear curves.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A polyline representing the piecewise linear      M
*                          approximation from, or NULL in case of an error.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrCrv2Polyline, IritCurve2Polylines                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv2Polyline, piecewise linear approximation, polyline               M
*****************************************************************************/
CagdPolylineStruct *SymbCrv2Polyline(const CagdCrvStruct *Crv,
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method,
				     CagdBType OptiLin)
{
    int i, n;
    CagdCrvStruct *Crvs, *TCrv;
    CagdPolylineStruct *P,
	*PList = NULL;

    Crvs = SymbCrvSplitPoleParams(Crv, SYMB_CRV_POLY_EPS,
				  SYMB_CRV_POLY_OUTREACH);
    n = CagdListLength(Crvs);

    assert(TolSamples > 0);

    for (i = 0, Crv = Crvs; i < n; i++, Crv = Crv -> Pnext) {
        switch (Method) {
	    case SYMB_CRV_APPROX_TOLERANCE:
	        P = SymbCrv2OptiTlrncPolyline(Crv, TolSamples);
		break;
	    case SYMB_CRV_APPROX_CURVATURE:
		if (Crv -> Order > 2) {
		    P = SymbCrv2OptiCrvtrPolyline(Crv, (int) TolSamples);
		    /* else do uniform sampling on linear curves. */
		    break;
		}
	    default:
	    case SYMB_CRV_APPROX_UNIFORM:
		switch (Crv -> GType) {
		    case CAGD_CBEZIER_TYPE:
		        P = BzrCrv2Polyline(Crv, (int) TolSamples);
			break;
		    case CAGD_CBSPLINE_TYPE:
			P = BspCrv2Polyline(Crv, (int) TolSamples,
					    NULL, OptiLin);
			break;
		    case CAGD_CPOWER_TYPE:
			TCrv = CagdCnvrtPwr2BzrCrv(Crv);
		        P = BzrCrv2Polyline(TCrv, (int) TolSamples);
			CagdCrvFree(TCrv);
			break;
		    default:
			SYMB_FATAL_ERROR(SYMB_ERR_UNDEF_CRV);
			return NULL;
	    }
	}

	IRIT_LIST_PUSH(P, PList);
    }

    CagdCrvFreeList(Crvs);

    return CagdListReverse(PList);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If the given curve is huge, simpleselect Samples points out if its       M
* control polygon.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           To select Samples points out of its control polygon.      M
*   Samples:       Number of samples to grab.                                M
*   AddFirstPt:	   If TRUE, first point on curve is also added.              M
*   AddLastPt:	   If TRUE, last point on curve is also added.               M
*   AddParamVals:  TRUE to add parameter values, as attributes.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   Taken samples.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbHugeCrv2Polyline                                                     M
*****************************************************************************/
CagdPtStruct *SymbHugeCrv2Polyline(const CagdCrvStruct *Crv,
				   int Samples,
				   CagdBType AddFirstPt,
				   CagdBType AddLastPt,
				   CagdBType AddParamVals)
{
    int i, l,
	Len = Crv -> Length,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType * const
        *Points = Crv -> Points;
    CagdRType const
        *WPts = Points[0];
    CagdRType s,
        *NodeVals = CagdCrvNodes(Crv),
        Ds = ((CagdRType) Len) / Samples,
        LastS = Len - IRIT_EPS - (AddLastPt ? 0.0 : Ds);
    CagdPtStruct
	*RetPtList = NULL;

    /* Return a subset of the control polygon. */
    for (s = AddFirstPt ? Ds : 0.0; s < LastS; s += Ds) {
        CagdPtStruct
	    *PtNew = CagdPtNew();

	i = (int) s;

	IRIT_LIST_PUSH(PtNew, RetPtList);

	for (l = 0; l < IRIT_MIN(MaxCoord, 3); l++)
	    PtNew -> Pt[l] = WPts != NULL ? Points[l + 1][i] / WPts[i]
					  : Points[l + 1][i];
	for (l = IRIT_MIN(MaxCoord, 3); l < 3; l++)
	    PtNew -> Pt[l] = 0.0;

	if (AddParamVals) {
	    /* Add parameter value as real attribute! */
	    AttrSetRealAttrib(&RetPtList -> Attr, "SaveParamVals",
			      NodeVals[i]);
	}
    }

    IritFree(NodeVals);

    return CagdListReverse(RetPtList);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the error function to be used by optimal tolerance based approx. of M
* curves into polylines.  This function should return TRUE if tolerance is   M
* met, FALSE if the curve is to be further divided.  This function, if       M
* defined, is invoked in addition to the tolerance testing.                  M
*                                                                            *
* PARAMETERS:                                                                M
*   ErrorFunc:      New error function to use.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbCrv2PolylineTlrncErrorFuncType:  Old error function reference.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrv2Polyline					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrv2PolylineSetTlrncErrorFunc, error handling                        M
*****************************************************************************/
SymbCrv2PolylineTlrncErrorFuncType SymbCrv2PolylineSetTlrncErrorFunc(
				 SymbCrv2PolylineTlrncErrorFuncType ErrorFunc)
{
    SymbCrv2PolylineTlrncErrorFuncType
	OldErrorFunc = SymbCrv2PolylineTlrncErrorFunc;

    SymbCrv2PolylineTlrncErrorFunc = ErrorFunc;

    return OldErrorFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a single curve to piecewise linear polyline.            *
*   Polyline is always E3 of CagdPolylineStruct type.		 	     *
*   Curve is adaptively sampled and result is guaranteed to be within        *
* Tolerance from the real curve.					     *
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     *
*   If Crv has an integer "SaveParamVals" attribute, a polyline is returned  *
* as a ptr attribute "SaveParamVals" with the parameter values of the point  *
* of the piecewise linear approximation.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:          To approximate as a polyline.                              *
*   Tolerance:    Maximal distance between the curve and its piecewise       *
*		  linear approximation.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolylineStruct *:   A polyline close within Tolerance to Crv.        *
*****************************************************************************/
static CagdPolylineStruct *SymbCrv2OptiTlrncPolyline(const CagdCrvStruct *Crv,
						     CagdRType Tolerance)
{
    CagdBType BBoxPosWeights,
        SaveParamVals = !IP_ATTR_IS_BAD_INT(AttrGetIntAttrib(Crv -> Attr,
							     "SaveParamVals"));
    CagdRType TMin, TMax, m;
    CagdCrvStruct *CpCrv;
    CagdPtStruct *PtList;
    CagdPolylineStruct
        *Params = NULL,
        *Poly = NULL;
    CagdBBoxStruct BBox;

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv))
        Crv = CpCrv = BspCrvOpenEnd(Crv);
    else
        CpCrv = NULL;

    /* Ignore zero weights while deriving the bbox. */
    BBoxPosWeights = CagdIgnoreNonPosWeightBBox(TRUE);
    CagdCrvBBox(Crv, &BBox);
    CagdIgnoreNonPosWeightBBox(BBoxPosWeights);

    m = IRIT_MAX(BBox.Max[0] - BBox.Min[0], BBox.Max[1] - BBox.Min[1]);
    if (Tolerance < SYMB_REL_TOL_BOUND * m)
        Tolerance = SYMB_REL_TOL_BOUND * m;

    if (CAGD_IS_BEZIER_CRV(Crv)) {
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv);

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);
	Crv = CpCrv = TCrv;
    }

    if (SaveParamVals) {
        CagdCrvDomain(Crv, &TMin, &TMax);
    }
    else {
        TMin = 1.0;
        TMax = -1.0;
    }

    PtList = SymbCrv2OptiTlrncPolyAux(Crv, Tolerance, TMin, TMax, TRUE);

    if (PtList != NULL) {
        CagdPtStruct
	    *PtNew = CagdPtNew();

	/* Add one last closing point. */
	((CagdPtStruct *) CagdListLast(PtList)) -> Pnext = PtNew;

	CagdCoerceToE3(PtNew -> Pt, Crv -> Points,
		       Crv -> Length - 1, Crv -> PType);
	if (SaveParamVals)
	    AttrSetRealAttrib(&PtNew -> Attr, "SaveParamVals", TMax);

	Poly = CagdCnvrtPtList2Polyline(PtList, &Params);
	if (Params != NULL)
	    AttrSetPtrAttrib(&Poly -> Attr, "SaveParamVals", Params);

	CagdPtFreeList(PtList);
    }

    if (CpCrv != Crv)
        CagdCrvFree(CpCrv);

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function of SymbCrv2OptiTlrncPolyline.                           *
*   Gets a curve and test if a line from first point to last point is close  *
* enough (less than Tolerance) to all other control points.  Otherwise,      *
* the curve is subdivided and this function recurses on the two sub curves.  *
*   Last point of curve is never concatenated onto the returned list.	     *
*   if TMin < TMax parameter values are saved as real attributes with the    *
* name of "SaveParamVals". 						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:          To approximate as a polyline.                              *
*   Tolerance:    Maximal distance between the curve and its piecewise       *
*		  linear approximation.					     *
*   TMin, TMax:   Domain of curve to save parameter values.  If, however,    *
*		  TMin > TMax, no parameter values are saved.		     *
*   AddFirstPt:	  If TRUE, first point on curve is also added.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPtStruct *:  A list of points approximating Crv to within Tolerance. *
*****************************************************************************/
static CagdPtStruct *SymbCrv2OptiTlrncPolyAux(const CagdCrvStruct *Crv,
					      CagdRType Tolerance,
					      CagdRType TMin,
					      CagdRType TMax,
					      CagdBType AddFirstPt)
{
    CagdPType LinePt1, LinePt2;
    CagdVType LineDir, Vec;
    CagdRType t1, t2,
	* const *Points = Crv -> Points;
    CagdRType
	const *WPoints = Points[0];
    int i,
	MaxDim = CAGD_NUM_OF_PT_COORD(Crv -> PType),
	Len = Crv -> Length;
    IrtLnType Line;
    IrtPlnType Plane1, Plane2;
    CagdPtStruct
        *RetPtList = AddFirstPt ? CagdPtNew() : NULL;

    if (MaxDim != 2 && MaxDim != 3) {
	if (MaxDim > 3)
	    MaxDim = 3;
	else {
	    SYMB_FATAL_ERROR(SYMB_ERR_ONLY_2D_OR_3D);
	    return NULL;
	}
    }

    LinePt1[0] = SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, 1, 0);
    LinePt1[1] = SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, 2, 0);
    LinePt1[2] = MaxDim == 2 ? 0.0
                             : SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, 3, 0);
    LinePt2[0] = SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, 1, Len - 1);
    LinePt2[1] = SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, 2, Len - 1);
    LinePt2[2] = MaxDim == 2 ? 0.0
                             : SYMB_CRV_OPTI_TOL_PT_COORD(WPoints, Points, 3, Len - 1);
    LineDir[0] = LinePt2[0] - LinePt1[0];
    LineDir[1] = LinePt2[1] - LinePt1[1];
    LineDir[2] = MaxDim == 2 ? 0.0 : LinePt2[2] - LinePt1[2];

    if (MaxDim == 2) {
	if (IRIT_VEC_SQR_LENGTH(LineDir) < IRIT_UEPS) {
	    /* The two points are identical - select a general line as alternative. */
	    Line[0] = 1.280791;
	    Line[1] = 2.161188;
	    Line[2] = -Line[0] * LinePt1[1] - Line[1] * LinePt1[2];
	}
	else {
	    /* Construct a normalized line equation as "Ax + By + C = 0". */
	    GMLineFrom2Points(Line, LinePt1, LinePt2);
	}

	IRIT_PLANE_RESET(Plane1);
	IRIT_PLANE_RESET(Plane2);
    }
    else {
	if (IRIT_VEC_SQR_LENGTH(LineDir) < IRIT_UEPS) {
	    /* The two points are identical - select a general line as alternative. */
	    LineDir[0] = 1.280791;
	    LineDir[1] = 2.161188;
	    LineDir[2] = 3.301060;
	}

        /* Construct two normalized orthogonal plane equations through the   */
	/* 3-space line as "Ax + By + Cz + D = 0".			     */
        IRIT_VEC_RESET(Vec);      /* Create Vec to be orthogonal to LineDir. */
        if (IRIT_FABS(LineDir[0]) <= IRIT_FABS(LineDir[1]) &&
	    IRIT_FABS(LineDir[0]) <= IRIT_FABS(LineDir[2]))
	    Vec[0] = 1.0;
        else if (IRIT_FABS(LineDir[1]) <= IRIT_FABS(LineDir[0]) &&
		 IRIT_FABS(LineDir[1]) <= IRIT_FABS(LineDir[2]))
	    Vec[1] = 1.0;
	else
	    Vec[2] = 1.0;
	IRIT_CROSS_PROD(Plane1, Vec, LineDir);
	IRIT_VEC_NORMALIZE(Plane1);
	Plane1[3] = -IRIT_DOT_PROD(Plane1, LinePt1);

	IRIT_CROSS_PROD(Plane2, Plane1, LineDir);
	IRIT_VEC_NORMALIZE(Plane2);
	Plane2[3] = -IRIT_DOT_PROD(Plane2, LinePt1);
    }

    if (AddFirstPt) {
	IRIT_PT_COPY(RetPtList -> Pt, LinePt1);
	if (TMin < TMax) {
	    /* Add parameter value as real attribute! */
	    AttrSetRealAttrib(&RetPtList -> Attr, "SaveParamVals", TMin);
	}
    }

    if (MaxDim == 2) {
        /* Check rigorously against all interior ctl pts. */
        if (WPoints == NULL) {
	    for (i = 1; i < Len - 1; i++) {
	        if (IRIT_FABS(Line[0] * Points[1][i] +
			      Line[1] * Points[2][i] +
			      Line[2]) > Tolerance)
		    break;
	    }
	}
	else {
	    for (i = 1; i < Len - 1; i++) {
	        if (IRIT_FABS(Line[0] * Points[1][i] +
			      Line[1] * Points[2][i] +
			      Line[2] * WPoints[i]) >
					Tolerance * IRIT_FABS(WPoints[i]))
		    break;
	    }
	}
    }
    else {
        /* Check rigorously against all interior ctl pts. */
        if (WPoints == NULL) {
	    for (i = 1; i < Len - 1; i++) {
	        if (IRIT_FABS(Plane1[0] * Points[1][i] +
			 Plane1[1] * Points[2][i] +
			 Plane1[2] * Points[3][i] +
			 Plane1[3]) > Tolerance ||
		    IRIT_FABS(Plane2[0] * Points[1][i] +
			 Plane2[1] * Points[2][i] +
			 Plane2[2] * Points[3][i] +
			 Plane2[3]) > Tolerance)
		    break;
	    }
	}
	else {
	    for (i = 1; i < Len - 1; i++) {
	        if (IRIT_FABS(Plane1[0] * Points[1][i] +
			 Plane1[1] * Points[2][i] +
			 Plane1[2] * Points[3][i] +
			 Plane1[3] * WPoints[i]) > Tolerance * IRIT_FABS(WPoints[i]) ||
		    IRIT_FABS(Plane2[0] * Points[1][i] +
			 Plane2[1] * Points[2][i] +
			 Plane2[2] * Points[3][i] +
			 Plane2[3] * WPoints[i]) > Tolerance * IRIT_FABS(WPoints[i]))
		    break;
	    }
	}
    }

    CagdCrvDomain(Crv, &t1, &t2);
    
    if ((t2 - t1 > IRIT_EPS && i < Len - 1) ||/* Tolerance has not been met. */
	(SymbCrv2PolylineTlrncErrorFunc != NULL &&
	 !SymbCrv2PolylineTlrncErrorFunc(Crv))) {
        CagdCrvStruct *Crv1, *Crv2;
	CagdRType t;
	CagdPtStruct *RetPtList1, *RetPtList2;

	if (Len >= SYMB_MAX_CRV_SUBDIV_LEN) {
	    /* Return a subset of the control polygon instead. */
	    return SymbHugeCrv2Polyline(Crv, SYMB_MAX_CRV_SUBDIV_LEN,
					AddFirstPt, FALSE, TMin < TMax);
	}

	if (CAGD_IS_BSPLINE_CRV(Crv) && Crv -> Length > Crv -> Order) {
	    t = Crv -> KnotVector[(Crv -> Length + Crv -> Order) >> 1];
	    if (TMin > TMax) {
		/* No parameter values are needed */
		TMin = t + 1.0;
		TMax = t - 1.0;
	    }
	}
	else if (TMin > TMax) {
	    /* No parameter values are needed */
	    CagdCrvDomain(Crv, &t1, &t2);
	    t = (t1 + t2) * 0.5;
	    TMin = t + 1.0;
	    TMax = t - 1.0;
	}
	else {
	    CagdCrvDomain(Crv, &TMin, &TMax);
	    t = (TMin + TMax) * 0.5;
	}
	Crv1 = CagdCrvSubdivAtParam(Crv, t);
	Crv2 = Crv1 -> Pnext;
	Crv1 -> Pnext = NULL;

	RetPtList1 = SymbCrv2OptiTlrncPolyAux(Crv1, Tolerance, TMin, t, FALSE);
	RetPtList2 = SymbCrv2OptiTlrncPolyAux(Crv2, Tolerance, t, TMax, TRUE);
	if (RetPtList == NULL)
	    RetPtList = CagdListAppend(RetPtList1, RetPtList2);
	else
	    RetPtList -> Pnext = CagdListAppend(RetPtList1, RetPtList2);

	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
    }

    return RetPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a single curve to polyline with SamplesPerCurve	     *
* samples, using the curvature field of the curve.			     *
*   Polyline is always E3 of CagdPolylineStruct type.		 	     *
*   Curve is sampled at optimal locations as to minimize the L-infinity	     *
* error between the curve and its polyline approximation.		     *
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:                To approiximate as a polyline.                       *
*   SamplesPerCurve:    Number of samples one can take off the curve.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolylineStruct *:   A polyline with SamplesPerCurve samples that     *
*                           approixmates Crv.                                *
*****************************************************************************/
static CagdPolylineStruct *SymbCrv2OptiCrvtrPolyline(const CagdCrvStruct *Crv,
						     int SamplesPerCurve)
{
    int i,
	OldMultInterp = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType *TVals;
    CagdPolylineStruct
	*P = CagdPolylineNew(SamplesPerCurve);
    CagdCrvStruct *CTmp;
    CagdPolylnStruct
	*NewPolyline = P -> Polyline;

    GlblCrvtrCrv = SymbCrv3DCurvatureSqr(Crv);

    CTmp = CagdCrvDerive(Crv);
    GlblDeriv1MagSqrCrv = SymbCrvDotProd(CTmp, CTmp);
    CagdCrvFree(CTmp);
    CagdCrvDomain(Crv, &GlblCrvtrCrvTMin, &GlblCrvtrCrvTMax);

    TVals = GMDistPoint1DWithEnergy(SamplesPerCurve,
				    GlblCrvtrCrvTMin, GlblCrvtrCrvTMax,
				    CURVE_PTS_DIST_RESOLUTION,
				    CrvCurvatureEvalFunc);

    for (i = 0; i < SamplesPerCurve; i++) {	  /* Convert to E3 polyline. */
	CagdRType
	    *R = CagdCrvEval(Crv, TVals[i]);

	CagdCoerceToE3(NewPolyline[i].Pt, &R, -1, Crv -> PType);
    }

    CagdCrvFree(GlblCrvtrCrv);
    CagdCrvFree(GlblDeriv1MagSqrCrv);
    IritFree(TVals);

    BspMultComputationMethod(OldMultInterp);

    return P;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Evaluation function of curvature square. This function should return the   *
* square root of the curvature, scaled by the arclength using GlblCrvtrCrv   *
* GlblDeriv1MagSqrCrv that contain curvature square and arclength sqaure.    *
*                                                                            *
* PARAMETERS:                                                                *
*   t:          Parameter at which to evalate the global curvature field.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:    Rstimated curvature square.                                *
*****************************************************************************/
static CagdRType CrvCurvatureEvalFunc(CagdRType t)
{
    CagdRType CrvtrSqr, MagSqr, *R;

    if (t < GlblCrvtrCrvTMin)
	t = GlblCrvtrCrvTMin;
    if (t > GlblCrvtrCrvTMax)
	t = GlblCrvtrCrvTMax;

    R = CagdCrvEval(GlblCrvtrCrv, t);
    CrvtrSqr = IRIT_FABS( R[1] / R[0] );
    R = CagdCrvEval(GlblDeriv1MagSqrCrv, t);
    MagSqr = GlblDeriv1MagSqrCrv -> PType == CAGD_PT_E1_TYPE ? R[1]
							     : R[1] / R[0];

    return sqrt(sqrt(CrvtrSqr) * MagSqr);
}
