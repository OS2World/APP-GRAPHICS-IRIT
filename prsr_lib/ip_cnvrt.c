/*****************************************************************************
* Conversion routines from curves and surfaces to polygons and polylines.    *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Apr 1992    *
*****************************************************************************/

#include "irit_sm.h"
#include "prsr_loc.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"

#define MIN_VALID_CRV_PARAM_DOMAIN	1e-3

IRIT_STATIC_DATA int
    GlblPolyListCirc = FALSE,
    GlblGenTriOnly = FALSE;
IRIT_STATIC_DATA IrtRType
    GlblCubicBzrTol = 0.01;
IRIT_STATIC_DATA IPPolygonStruct
    *GlblSrfApproxPolyList = NULL;
IRIT_STATIC_DATA CagdPlgErrorFuncType
    GlblPolygonErrFunc = NULL;
IRIT_GLOBAL_DATA int
    IPGlblGenDegenPolys = FALSE;

static CagdPolygonStruct *IPGenTriangle(CagdBType ComputeNormals,
					CagdBType ComputeUV,
					const CagdRType *Pt1,
					const CagdRType *Pt2,
					const CagdRType *Pt3,
					const CagdRType *Nl1,
					const CagdRType *Nl2,
					const CagdRType *Nl3,
					const CagdRType *UV1,
					const CagdRType *UV2,
					const CagdRType *UV3,
					CagdBType *GenPoly);
static CagdPolygonStruct *IPGenRectangle(CagdBType ComputeNormals,
					 CagdBType ComputeUV,
					 const CagdRType *Pt1,
					 const CagdRType *Pt2,
					 const CagdRType *Pt3,
					 const CagdRType *Pt4,
					 const CagdRType *Nl1,
					 const CagdRType *Nl2,
					 const CagdRType *Nl3,
					 const CagdRType *Nl4,
					 const CagdRType *UV1,
					 const CagdRType *UV2,
					 const CagdRType *UV3,
					 const CagdRType *UV4,
					 CagdBType *GenPoly);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a cagd polyline to irit polyline. Old cagd polylines    M
* are freed!								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Polys:      Polygons in cagd_lib form to be converted in IRIT form.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Same polylines in IRIT format. 	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCagdPlgns2IritPlgns, CagdCnvrtPolyline2LinBspCrv                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCagdPllns2IritPllns                                                    M
*****************************************************************************/
IPPolygonStruct *IPCagdPllns2IritPllns(CagdPolylineStruct *Polys)
{
    int i, n;
    IPPolygonStruct
	*PHead = NULL;
    CagdPolylineStruct *CagdPoly;

    for (CagdPoly = Polys;
	 CagdPoly != NULL;
	 CagdPoly = CagdPoly -> Pnext) {
	IPVertexStruct *VHead;
	CagdPolylineStruct *Params;

	if ((n = CagdPoly -> Length) < 2)
	    continue;

	for (i = n - 1, VHead = NULL; i >= 0; i--) { /* Convert to vertices. */
	    VHead = IPAllocVertex2(VHead);

	    IRIT_PT_COPY(VHead -> Coord, CagdPoly -> Polyline[i].Pt);
	}

	PHead = IPAllocPolygon(0, VHead, PHead);

	if ((Params = AttrGetPtrAttrib(CagdPoly -> Attr, "SaveParamVals"))
							           != NULL) {
	    /* Copy the parameter values as attributes. */
	    for (i = 0, VHead = PHead -> PVertex;
		 i < n;
		 i++, VHead = VHead -> Pnext) {
	        AttrSetRealAttrib(&VHead -> Attr, "Param",
				  Params -> Polyline[i].Pt[0]);
	    }

	    CagdPolylineFree(Params);
	    AttrFreeOneAttribute(&CagdPoly -> Attr, "SaveParamVals");
	}

	PHead -> Attr = IP_ATTR_COPY_ATTRS(CagdPoly -> Attr);
    }

    CagdPolylineFreeList(Polys);

    return PHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a cagd polygon (triangle) into irit polygon. Old cagd   M
* polygons are freed!							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Polys:      Polygons in cagd library format to convert.                  M
*   ComputeUV:  Do we have UV values as well, at the vertices?               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Same polygons in IRIT format.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCagdPllns2IritPllns			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCagdPlgns2IritPlgns                                                    M
*****************************************************************************/
IPPolygonStruct *IPCagdPlgns2IritPlgns(CagdPolygonStruct *Polys,
				       CagdBType ComputeUV)
{
    CagdPolygonStruct *CagdPolygon;
    IPPolygonStruct *P,
	*PHead = NULL;

    for (CagdPolygon = Polys;
	 CagdPolygon != NULL;
	 CagdPolygon = CagdPolygon -> Pnext) {/* All polygons are triangles! */
	int i, j;
	IPVertexStruct *V, *VHead,
	    *VTail = NULL;

	if (CagdPolygon -> PolyType == CAGD_POLYGON_TYPE_POLYSTRIP) {
	    VHead = IPAllocVertex2(V = IPAllocVertex2(NULL));

	    /* Convert to new format first two base points of polygon strip. */
	    IRIT_PT_COPY(VHead -> Coord, CagdPolygon -> U.PolyStrip.FirstPt[0]);
	    IRIT_PT_COPY(V -> Coord, CagdPolygon -> U.PolyStrip.FirstPt[1]);

	    if (IRIT_PT_EQ_ZERO(CagdPolygon -> U.PolyStrip.FirstNrml[0])) {
		IP_RST_NORMAL_VRTX(VHead);
		IP_RST_NORMAL_VRTX(V);
	    }
	    else {
		IRIT_PT_COPY(VHead -> Normal,
			CagdPolygon -> U.PolyStrip.FirstNrml[0]);
		IRIT_PT_COPY(V -> Normal, CagdPolygon -> U.PolyStrip.FirstNrml[1]);

		IP_SET_NORMAL_VRTX(V);
		IP_SET_NORMAL_VRTX(VHead);
	    }

	    if (ComputeUV) {
		AttrSetUVAttrib(&VHead -> Attr, "uvvals",
				CagdPolygon -> U.PolyStrip.FirstUV[0][0],
				CagdPolygon -> U.PolyStrip.FirstUV[0][1]);

		AttrSetUVAttrib(&V -> Attr, "uvvals",
				CagdPolygon -> U.PolyStrip.FirstUV[1][0],
				CagdPolygon -> U.PolyStrip.FirstUV[1][1]);
	    }

	    /* Convert the strip itself. */
	    VTail = V;
	    for (i = 0; i < CagdPolygon -> U.PolyStrip.NumOfPolys; i++) {
		V = IPAllocVertex2(NULL);

		IRIT_PT_COPY(V -> Coord, CagdPolygon -> U.PolyStrip.StripPt[i]);

		if (IRIT_PT_EQ_ZERO(CagdPolygon -> U.PolyStrip.FirstNrml[0]))
		    IP_RST_NORMAL_VRTX(V);
		else {
		    IRIT_PT_COPY(V -> Normal,
			    CagdPolygon -> U.PolyStrip.StripNrml[i]);

		    IP_SET_NORMAL_VRTX(V);
		}

		if (ComputeUV) {
		    AttrSetUVAttrib(&V -> Attr, "uvvals",
				     CagdPolygon -> U.PolyStrip.StripUV[0][0],
				     CagdPolygon -> U.PolyStrip.StripUV[0][1]);
		}

		VTail -> Pnext = V;
		VTail = V;
	    }
	}
	else {
	    /* Either a triangle or a rectangle surface. */
	    j = CagdPolygon -> PolyType == CAGD_POLYGON_TYPE_TRIANGLE ? 2 : 3;
	    for (i = j, VHead = NULL; i >= 0; i--) {
		VHead = IPAllocVertex2(VHead);
		if (i == j)
		    VTail = VHead;

		IRIT_PT_COPY(VHead -> Coord, CagdPolygon -> U.Polygon[i].Pt);
		if (IRIT_PT_EQ_ZERO(CagdPolygon -> U.Polygon[i].Nrml)) {
		    IP_RST_NORMAL_VRTX(VHead);
		}
		else {
		    IRIT_PT_COPY(VHead -> Normal, CagdPolygon -> U.Polygon[i].Nrml);
		    IP_SET_NORMAL_VRTX(VHead);
		}

		if (ComputeUV) {
		    AttrSetUVAttrib(&VHead -> Attr, "uvvals",
				     CagdPolygon -> U.Polygon[i].UV[0],
				     CagdPolygon -> U.Polygon[i].UV[1]);
		}
	    }

	    if (_IPPolyListCirc)
		VTail -> Pnext = VHead;
	}

	P = IPAllocPolygon(0, VHead, PHead);
	IP_SET_CONVEX_POLY(P);
	if (CagdPolygon -> PolyType == CAGD_POLYGON_TYPE_POLYSTRIP)
	    IP_SET_STRIP_POLY(P);
	else {
	    IPUpdatePolyPlane(P);                  /* Update plane equation. */
	}

	PHead = P;
    }

    CagdPolygonFreeList(Polys);

    return PHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert one curve into a polyline with TolSamples	     M
* samples/tolerance.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:           To approximate as a polyline.                             M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the curve's M
*		       curvature.					     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polyline approximating Crv.  Can be more than one M
*			 polyline if Crv is C0 discont.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrCrv2Polyline, SymbCrv2Polyline                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCurve2Polylines, conversion, approximation                             M
*****************************************************************************/
IPPolygonStruct *IPCurve2Polylines(const CagdCrvStruct *Crv,
				   CagdRType TolSamples,
				   SymbCrvApproxMethodType Method)
{
    CagdRType TMin, TMax, t;
    CagdPolylineStruct *CagdPoly;
    CagdBType
	NewCrv = FALSE;
    CagdCrvStruct
	*TCrv = NULL;

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv)) {
	Crv = TCrv = BspCrvOpenEnd(Crv);
	NewCrv = TRUE;
    }

    CagdCrvDomain(Crv, &TMin, &TMax);
    if (TMax - TMin < MIN_VALID_CRV_PARAM_DOMAIN) {
	IPVertexStruct
	    *V2 = IPAllocVertex2(NULL),
	    *V1 = IPAllocVertex2(V2);
	IPPolygonStruct
	    *Pl = IPAllocPolygon(0, V1, NULL);

	/* Construct a two point polyline eith two end points of curve. */
	CagdCoerceToE3(V1 -> Coord, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE3(V2 -> Coord, Crv -> Points, Crv -> Length - 1,
		       Crv -> PType);
	if (NewCrv)
	    CagdCrvFree(TCrv);
	return Pl;
    }

    if (CAGD_IS_BSPLINE_CRV(Crv) && BspCrvKnotC0Discont(Crv, &t)) {
        CagdCrvStruct
	    *Crvs = CagdCrvSubdivAtParam(Crv, t);
	IPPolygonStruct *Pls;

	/* Split at discont. and invoke recursively. */
	Pls = IPAppendPolyLists(IPCurve2Polylines(Crvs, TolSamples, Method),
				IPCurve2Polylines(Crvs -> Pnext, TolSamples,
						  Method));

	CagdCrvFreeList(Crvs);

	if (NewCrv)
	    CagdCrvFree(TCrv);

	return Pls;
    }

    switch (Method) {
        case SYMB_CRV_APPROX_UNIFORM:
	    if (TolSamples < 2.0)
	        TolSamples = 2.0;

            CagdPoly = CagdCrv2Polyline(Crv, (int) TolSamples, TRUE);
	    break;
	case SYMB_CRV_APPROX_TOLERANCE:
        case SYMB_CRV_APPROX_CURVATURE:
	default:
            CagdPoly = SymbCrv2Polyline(Crv, TolSamples, Method, TRUE);
	    break;
    }

    if (NewCrv)
	CagdCrvFree(TCrv);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert one polyline into a curve, typically linear.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:       To convert to a curve.			                     M
*   Order:    Typically 2 for linear, but can be higher order as well.  In   M
*	      all cases ,the input polyline points serve as control points.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve.				                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtPolyline2LinBspCrv				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPolyline2Curve, conversion, approximation                              M
*****************************************************************************/
CagdCrvStruct *IPPolyline2Curve(const IPPolygonStruct *Pl, int Order)
{
    IPVertexStruct
	*V = Pl -> PVertex;
    int i,
	Len = IPVrtxListLen(V);
    CagdCrvStruct
	*Crv = BspCrvNew(Len, Order, CAGD_PT_E3_TYPE);
    CagdRType
	**Points = Crv -> Points;

    BspKnotUniformOpen(Len, Order, Crv -> KnotVector);

    for (i = 0; V != NULL; V = V -> Pnext, i++) {
	Points[1][i] = V -> Coord[0];
	Points[2][i] = V -> Coord[1];
	Points[3][i] = V -> Coord[2];
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single curve's control polygon into a polyline.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      To extract its control polygon as a polyline.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polyline representing Crv's control polygon.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCurve2CtlPoly, conversion                                              M
*****************************************************************************/
IPPolygonStruct *IPCurve2CtlPoly(const CagdCrvStruct *Crv)
{
    CagdPolylineStruct
	*CagdPoly = CagdCrv2CtrlPoly(Crv);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single surface into polylines with TolSamples samples M
* or tolerance per isoline curve as a polyline object.			     M
*   If NumOfIsolines has negative value, its absolute value is heuristically M
* used to derive a new NumOfIsolines number for it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           To approximate as a polyline.                             M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the	     M
*		       isocurve's curvature.				     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polyline object approximating Srf.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPCurve2Polylines					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSurface2Polylines, conversion, approximation                           M
*****************************************************************************/
IPPolygonStruct *IPSurface2Polylines(const CagdSrfStruct *Srf,
				     int NumOfIsolines[2],
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method)
{
    CagdRType t;
    CagdPolylineStruct *CagdPoly;
    CagdBType
	NewSrf = FALSE;
    CagdSrfStruct
	*TSrf = NULL;

    if (CAGD_IS_BSPLINE_SRF(Srf) && !BspSrfHasOpenEC(Srf)) {
	Srf = TSrf = BspSrfOpenEnd(Srf);
	NewSrf = TRUE;
    }

    if (NumOfIsolines[0] < 0) {
	if (Srf -> UOrder > 2)
	    NumOfIsolines[0] = ((Srf -> ULength - NumOfIsolines[0]) >> 1);
	else
	    NumOfIsolines[0] = -NumOfIsolines[0];
    }
    if (NumOfIsolines[0] < 0)
	NumOfIsolines[0] = 0;

    if (NumOfIsolines[1] < 0) {
	if (Srf -> VOrder > 2)
	    NumOfIsolines[1] = ((Srf -> VLength - NumOfIsolines[1]) >> 1);
	else
	    NumOfIsolines[1] = -NumOfIsolines[1];
    }
    if (NumOfIsolines[1] < 0)
	NumOfIsolines[1] = 0;

    /* Extract only boundary curves of non closed surfaces. */
    if (NumOfIsolines[0] == 0 && NumOfIsolines[1] == 0) {
	NumOfIsolines[0] = CagdIsClosedSrf(Srf, CAGD_CONST_U_DIR) ? 0 : 2;
	NumOfIsolines[1] = CagdIsClosedSrf(Srf, CAGD_CONST_V_DIR) ? 0 : 2;
    }

    if (CAGD_IS_BSPLINE_SRF(Srf)) {
	/* Search for C0 discontinuity and split the surface there. */
	if (BspSrfKnotC0Discont(Srf, CAGD_CONST_U_DIR, &t)) {
	    CagdSrfStruct
		*Srfs = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_U_DIR);
	    IPPolygonStruct *Pls;

	    /* Split at U discont. and invoke recursively. */
	    Pls = IPAppendPolyLists(IPSurface2Polylines(Srfs, NumOfIsolines,
							TolSamples, Method),
				    IPSurface2Polylines(Srfs -> Pnext,
							NumOfIsolines,
							TolSamples, Method));

	    CagdSrfFreeList(Srfs);

	    if (NewSrf)
		CagdSrfFree(TSrf);

	    return Pls;
	}
	else if (BspSrfKnotC0Discont(Srf, CAGD_CONST_V_DIR, &t)) {
	    CagdSrfStruct
		*Srfs = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_V_DIR);
	    IPPolygonStruct *Pls;

	    /* Split at V discont. and invoke recursively. */
	    Pls = IPAppendPolyLists(IPSurface2Polylines(Srfs, NumOfIsolines,
							TolSamples, Method),
				    IPSurface2Polylines(Srfs -> Pnext,
							NumOfIsolines,
							TolSamples, Method));

	    CagdSrfFreeList(Srfs);

	    if (NewSrf)
		CagdSrfFree(TSrf);

	    return Pls;
	}
    }

    switch (Method) {
        case SYMB_CRV_APPROX_UNIFORM:
	    if (TolSamples < 2.0)
	        TolSamples = 2.0;
            CagdPoly = CagdSrf2Polylines(Srf, NumOfIsolines,
					 (int) TolSamples);
	    break;
        case SYMB_CRV_APPROX_CURVATURE:
	default:
            CagdPoly = SymbSrf2Polylines(Srf, NumOfIsolines,
					 TolSamples, Method);
	    break;
    }

    if (NewSrf)
	CagdSrfFree(TSrf);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single surface's control mesh into a polylines object.M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To extract its control mesh as a polylines.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object representing Srf's control mesh. M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSurface2CtlMesh, conversion                                            M
*****************************************************************************/
IPPolygonStruct *IPSurface2CtlMesh(const CagdSrfStruct *Srf)
{
    CagdPolylineStruct
	*CagdPoly = CagdSrf2CtrlMesh(Srf);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back routine to create one triangular polygon, given its vertices,  *
* and, optionally, normals and uv coordinates.  Places the constructed       *
* polygon in a global polygonal list.				             *
*                                                                            *
* PARAMETERS:                                                                *
*   ComputeNormals:      If non zero then use Nl? parameters. Nl? are valid. *
*			 If positive also used to reorient the polygon.      *
*   ComputeUV:           If TRUE then use UV? parameters. UV? are valid.     *
*   Pt1, Pt2, Pt3:       Euclidean locations of vertices.                    *
*   Nl1, Nl2, Nl3:       Optional Normals of vertices (if ComputeNormals).   *
*   UV1, UV2, UV3:       Optional UV parametric location of vertices (if     *
*                        ComputeUV).                                         *
*   GenPoly:             Returns TRUE if a polygon was generated, FALSE      *
*		         otherwise.  Note this function can return NULL and  *
*		         still generate a polygon as a call back for         *
*		         CagdSrf2Polygons.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:  This call back function ALLWAYS RETURNS NULL.      *
*****************************************************************************/
static CagdPolygonStruct *IPGenTriangle(CagdBType ComputeNormals,
					CagdBType ComputeUV,
					const CagdRType *Pt1,
					const CagdRType *Pt2,
					const CagdRType *Pt3,
					const CagdRType *Nl1,
					const CagdRType *Nl2,
					const CagdRType *Nl3,
					const CagdRType *UV1,
				 	const CagdRType *UV2,
					const CagdRType *UV3,
					CagdBType *GenPoly)
{
    int Rvrsd, 
        KeepDegenerated = FALSE;
    IPPolygonStruct *Pl;
    IPVertexStruct *V1, *V2, *V3;

    if ((GlblPolygonErrFunc != NULL &&
	 GlblPolygonErrFunc(Pt1, Pt2, Pt3) < 0.0)) {
	*GenPoly = FALSE;
	return NULL;
    }
    else if (GMCollinear3Pts(Pt1, Pt2, Pt3)) {
        if (IPGlblGenDegenPolys) {
            KeepDegenerated = TRUE;
           *GenPoly = TRUE;
        } 
        else {
	    *GenPoly = FALSE;
	    return NULL;
        }
    }
    else
	*GenPoly = TRUE;

    if (ComputeNormals) {
	const CagdRType
	    *Nl = (Nl1 != NULL ? Nl1 : (Nl2 != NULL ? Nl2 : Nl3));

	if (Nl != NULL) {
	    CagdPType Pin;

	    IRIT_PT_ADD(Pin, Pt1, Nl);
	    Pl = PrimGenPolygon3Vrtx(Pt1, Pt2, Pt3, Pin, &Rvrsd, NULL);
	}
	else
	    Pl = PrimGenPolygon3Vrtx(Pt1, Pt2, Pt3, NULL, &Rvrsd, NULL);
    }
    else
        Pl = PrimGenPolygon3Vrtx(Pt1, Pt2, Pt3, NULL, &Rvrsd, NULL);

    V1 = Pl -> PVertex;
    if (Rvrsd) {
	V3 = V1 -> Pnext;
	V2 = V3 -> Pnext;
	if (!GlblPolyListCirc)
	    V2 -> Pnext = NULL;
    }
    else {
	V2 = V1 -> Pnext;
	V3 = V2 -> Pnext;
	if (!GlblPolyListCirc)
	    V3 -> Pnext = NULL;
    }

    if (ComputeNormals) {
        if (Nl1 == NULL)
	    Nl1 = Pl -> Plane;
	if (Nl2 == NULL)
	    Nl2 = Pl -> Plane;
	if (Nl3 == NULL)
	    Nl3 = Pl -> Plane;

	IRIT_VEC_COPY(V1 -> Normal, Nl1);
	IP_SET_NORMAL_VRTX(V1);
	
	IRIT_VEC_COPY(V2 -> Normal, Nl2);
	IP_SET_NORMAL_VRTX(V2);

	IRIT_VEC_COPY(V3 -> Normal, Nl3);
	IP_SET_NORMAL_VRTX(V3);
    }
    else {
        IP_RST_NORMAL_VRTX(V1);
        IP_RST_NORMAL_VRTX(V2);
        IP_RST_NORMAL_VRTX(V3);
    }

    if (ComputeUV) {
	AttrSetUVAttrib(&V1 -> Attr, "uvvals", UV1[0], UV1[1]);
	AttrSetUVAttrib(&V2 -> Attr, "uvvals", UV2[0], UV2[1]);
	AttrSetUVAttrib(&V3 -> Attr, "uvvals", UV3[0], UV3[1]);
    }

    if (KeepDegenerated) 
        AttrSetIntAttrib(&Pl -> Attr, IP_ATTRIB_DEGEN_POLY, TRUE);

    IRIT_LIST_PUSH(Pl, GlblSrfApproxPolyList);

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back routine to create one rectangular polygon, given its vertices, *
* and, optionally, normals and uv coordinates.  Places the constructed       *
* polygon in a global polygonal list.				             *
*                                                                            *
* PARAMETERS:                                                                *
*   ComputeNormals:      If TRUE then use Nl? parameters. Nl? are valid.     *
*   ComputeUV:           If TRUE then use UV? parameters. UV? are valid.     *
*   Pt1, Pt2, Pt3, Pt4:  Euclidean locations of vertices.                    *
*   Nl1, Nl2, Nl3, Nl4:  Optional Normals of vertices (if ComputeNormals).   *
*   UV1, UV2, UV3, UV4:  Optional UV parametric location of vertices (if     *
*                        ComputeUV).                                         *
*   GenPoly:             Returns TRUE if a polygon was generated, FALSE      *
*		         otherwise.  Note this function can return NULL and  *
*		         still generate a polygon as a call back for         *
*		         CagdSrf2Polygons.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:  This call back function ALLWAYS RETURNS NULL.      *
*****************************************************************************/
static CagdPolygonStruct *IPGenRectangle(CagdBType ComputeNormals,
					 CagdBType ComputeUV,
					 const CagdRType *Pt1,
					 const CagdRType *Pt2,
					 const CagdRType *Pt3,
					 const CagdRType *Pt4,
					 const CagdRType *Nl1,
					 const CagdRType *Nl2,
					 const CagdRType *Nl3,
					 const CagdRType *Nl4,
					 const CagdRType *UV1,
					 const CagdRType *UV2,
					 const CagdRType *UV3,
					 const CagdRType *UV4,
					 CagdBType *GenPoly)
{
    int Rvrsd,
        Degenerated = FALSE;
    IPPolygonStruct *Pl;
    IPVertexStruct *V1, *V2, *V3, *V4;

    if (GMCollinear3Pts(Pt1, Pt2, Pt3) ||
	GMCollinear3Pts(Pt2, Pt3, Pt4) ||
	GlblGenTriOnly ||
	(GlblPolygonErrFunc != NULL &&
	 GlblPolygonErrFunc(Pt1, Pt2, Pt3) < 0.0)) {
	*GenPoly = FALSE;
	return NULL;
    }
    else if (GMCollinear3Pts(Pt1, Pt2, Pt3) ||
             GMCollinear3Pts(Pt1, Pt2, Pt4) || 
             GMCollinear3Pts(Pt1, Pt3, Pt4) ||
	     GMCollinear3Pts(Pt2, Pt3, Pt4)) {
        if (IPGlblGenDegenPolys) {
            Degenerated = TRUE;
            *GenPoly = TRUE;
        }
        else {
	    *GenPoly = FALSE;
	    return NULL;
        }
    }
    else
	*GenPoly = TRUE;

    if (ComputeNormals) {
	const CagdRType
	    *Nl = (Nl1 != NULL ? Nl1 : (Nl2 != NULL ? Nl2 : (Nl3 != NULL ? Nl3
							              : Nl4)));

	if (Nl != NULL) {
	    CagdPType Pin;

	    IRIT_PT_ADD(Pin, Pt1, Nl);
	    Pl = PrimGenPolygon4Vrtx(Pt1, Pt2, Pt3, Pt4, Pin, &Rvrsd, NULL);
	}
	else
	    Pl = PrimGenPolygon4Vrtx(Pt1, Pt2, Pt3, Pt4, NULL, &Rvrsd, NULL);
    }
    else
        Pl = PrimGenPolygon4Vrtx(Pt1, Pt2, Pt3, Pt4, NULL, &Rvrsd, NULL);

    V1 = Pl -> PVertex;
    if (Rvrsd) {
	V4 = V1 -> Pnext;
	V3 = V4 -> Pnext;
	V2 = V3 -> Pnext;
	if (!GlblPolyListCirc)
	    V2 -> Pnext = NULL;
    }
    else {
	V2 = V1 -> Pnext;
	V3 = V2 -> Pnext;
	V4 = V3 -> Pnext;
	if (!GlblPolyListCirc)
	    V4 -> Pnext = NULL;
    }

    if (ComputeNormals) {
	if (Nl1 == NULL)
	    Nl1 = Pl -> Plane;
	if (Nl2 == NULL)
	    Nl2 = Pl -> Plane;
	if (Nl3 == NULL)
	    Nl3 = Pl -> Plane;
	if (Nl4 == NULL)
	    Nl4 = Pl -> Plane;

	IRIT_VEC_COPY(V1 -> Normal, Nl1);
	IP_SET_NORMAL_VRTX(V1);
	
	IRIT_VEC_COPY(V2 -> Normal, Nl2);
	IP_SET_NORMAL_VRTX(V2);

	IRIT_VEC_COPY(V3 -> Normal, Nl3);
	IP_SET_NORMAL_VRTX(V3);

	IRIT_VEC_COPY(V4 -> Normal, Nl4);
	IP_SET_NORMAL_VRTX(V4);
    }
    else {
        IP_RST_NORMAL_VRTX(V1);
        IP_RST_NORMAL_VRTX(V2);
        IP_RST_NORMAL_VRTX(V3);
        IP_RST_NORMAL_VRTX(V4);
    }

    if (ComputeUV) {
	AttrSetUVAttrib(&V1 -> Attr, "uvvals", UV1[0], UV1[1]);
	AttrSetUVAttrib(&V2 -> Attr, "uvvals", UV2[0], UV2[1]);
	AttrSetUVAttrib(&V3 -> Attr, "uvvals", UV3[0], UV3[1]);
	AttrSetUVAttrib(&V4 -> Attr, "uvvals", UV4[0], UV4[1]);
    } 

    if (Degenerated)
        AttrSetIntAttrib(&Pl -> Attr, IP_ATTRIB_DEGEN_POLY, TRUE);

    IRIT_LIST_PUSH(Pl, GlblSrfApproxPolyList);

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Controls the generation of triangles only, in the tesselation process.     M
*                                                                            *
* PARAMETERS:                                                                M
*   OnlyTri:     TRUE for triangles only, FALSE otherwise.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      Old value of flag.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSurface2Polygons                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSurface2PolygonsGenTriOnly, triangles                                  M
*****************************************************************************/
int IPSurface2PolygonsGenTriOnly(int OnlyTri)
{
    int OldVal = GlblGenTriOnly;

    GlblGenTriOnly = OnlyTri;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Controls the generation of degenerated triangles and rectangles, in the  M
* tesselation process.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   GenDegenPolys:      TRUE for creating degenerated triangles and	     M
*                       rectangles,  FALSE otherwise.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Old value of flag.                                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSurface2Polygons                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSurface2PolygonsGenDegenPolys, triangles, rectangles                   M
*****************************************************************************/
int IPSurface2PolygonsGenDegenPolys(int GenDegenPolys)
{
    int OldVal = IPGlblGenDegenPolys;

    IPGlblGenDegenPolys = GenDegenPolys;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the polygon approximation error function.  The error function       M
* will return a negative value if this triangle must be purged or otherwise  M
* a non negative error measure.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPlgErrorFuncType:  Old value of function.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPPolygonSetErrFunc                                                      M
*****************************************************************************/
CagdPlgErrorFuncType IPPolygonSetErrFunc(CagdPlgErrorFuncType Func)
{
    CagdPlgErrorFuncType
	OldFunc = GlblPolygonErrFunc;

    GlblPolygonErrFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to approximate a single surface by polygons.			     M
*   The polygonal approximation routines have call back functions to invoke  M
* for each new polygon and if these call back functions are used, this       M
* function might return NULL.  See IPGenTriangle and IPGenRectangle.         M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To approximate using polygons.                             M
*   FourPerFlat:  If TRUE, four triangle per flat surface patch are created, M
*                 otherwise two.  Only for uniform, non optimal, sampling.   M
*   FineNess:     Fineness control on polygonal approximation.  If !Optimal, M
*		  uniform sampling in the order of FineNess Sampling is      M
*		  used.  If Optimal, FineNess prescribes the maximal         M
*		  distance between the surface and its polygonal	     M
*		  approximation.					     M
*   ComputeUV:    Do we want UV parameter values with the vertices of the    M
*                 triangles?						     M
*   ComputeNrml:  Do we want normals to vertices!?			     M
*   Optimal:      If FALSE, then parametric space of Srf is sampled          M
*                 uniformely, order of FineNess samples per direction.       M
*		  If TRUE, the adaptively created polygonal approximation is M
*		  guaranteed to be within FineNess distance to the surface.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Resulting polygons that approximates Srf.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2Polygons, CagdSrfAdap2Polygons, IPGenTriangle, IPGenRectangle    M
*   IPSurface2PolygonsGenTriOnly				             M
*   									     *
* KEYWORDS:                                                                  M
*   IPSurface2Polygons, approximation, conversion                            M
*****************************************************************************/
IPPolygonStruct *IPSurface2Polygons(CagdSrfStruct *Srf,
				    int FourPerFlat,
				    IrtRType FineNess,
				    int ComputeUV,
				    int ComputeNrml,
				    int Optimal)
{
    CagdBType
	NewSrf = FALSE;
    CagdSrfMakeTriFuncType
	OldTriFunc = CagdSrfSetMakeTriFunc(IPGenTriangle);
    CagdSrfMakeRectFuncType
	OldRectFunc = CagdSrfSetMakeRectFunc(IPGenRectangle);
    CagdPolygonStruct *Polys;
    IPPolygonStruct *RetVal;

    GlblSrfApproxPolyList = NULL;
    /* The CagdSrf*2Polygons function might affect _IPPolyListCirc... */
    GlblPolyListCirc = _IPPolyListCirc;

    if (CAGD_IS_BSPLINE_SRF(Srf) && !BspSrfHasOpenEC(Srf)) {
	Srf = BspSrfOpenEnd(Srf);
	NewSrf = TRUE;
    }

    if (Optimal) {
        Polys = CagdSrfAdap2Polygons(Srf, FineNess, ComputeNrml,
				     FourPerFlat, ComputeUV, NULL);
    }
    else {
	Polys = CagdSrf2Polygons(Srf, (int) FineNess, ComputeNrml,
				 FourPerFlat, ComputeUV);
    }

    if (NewSrf)
	CagdSrfFree(Srf);

    CagdSrfSetMakeTriFunc(OldTriFunc);
    CagdSrfSetMakeRectFunc(OldRectFunc);

    RetVal = GlblSrfApproxPolyList;
    GlblSrfApproxPolyList = NULL;

    /* Could happen if we generate polygonal strips. */
    if (RetVal == NULL && Polys != NULL)
        RetVal = IPCagdPlgns2IritPlgns(Polys, ComputeUV);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to approximate a single trimmed surface by polygons.		     M
*   The polygonal approximation routines have call back functions to invoke  M
* for each new polygon and if these call back functions are used, this       M
* function might return NULL.  See IPGenTriangle and IPGenRectangle.         M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:      To approximate using polygons.                             M
*   FourPerFlat:  If TRUE, four triangle per flat surface patch are created, M
*                 otherwise only two.					     M
*   FineNess:     Fineness control on polygonal approximation. The larger    M
*                 this number is the finer the approximation becomes. 10 is  M
*                 a good compromise when Optimal is FALSE.		     M
*   ComputeUV:    Do we want UV parameter values with the vertices of the    M
*                 triangles?						     M
*   ComputeNrml:  Do we want normals to vertices!?			     M
*   Optimal:      If FALSE (0) then parametric space of TrimSrf is sampled   M
*                 uniformely.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Resulting polygons that approximates TrimSrf.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimSrf2Polygons2, IPGenTriangle, IPGenRectangle                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrimSrf2Polygons, approximation, conversion                            M
*****************************************************************************/
IPPolygonStruct *IPTrimSrf2Polygons(TrimSrfStruct *TrimSrf,
				    int FourPerFlat,
				    IrtRType FineNess,
				    int ComputeUV,
				    int ComputeNrml,
				    int Optimal)
{
    CagdSrfMakeTriFuncType
	OldTriFunc = CagdSrfSetMakeTriFunc(IPGenTriangle);
    CagdSrfMakeRectFuncType
	OldRectFunc = CagdSrfSetMakeRectFunc(IPGenRectangle);
    IPPolygonStruct *RetVal;

    GlblSrfApproxPolyList = NULL;
    /* The TrimSrf2Polygons2 function might affect _IPPolyListCirc... */
    GlblPolyListCirc = _IPPolyListCirc;

    if (Optimal) {
        TrimSrfAdap2Polygons(TrimSrf, FineNess, ComputeNrml, ComputeUV);
    }
    else {
	TrimSrf2Polygons2(TrimSrf, (int) FineNess, ComputeNrml, ComputeUV);
    }

    CagdSrfSetMakeTriFunc(OldTriFunc);
    CagdSrfSetMakeRectFunc(OldRectFunc);

    RetVal = GlblSrfApproxPolyList;
    GlblSrfApproxPolyList = NULL;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single trimmed surface into polylines with TolSamples M
* samples/tolerance, NumOfIsolines isolines into a polylines object.         M
*   If NumOfIsolines has negative value, its absolute value is heuristically M
* used to derive a new NumOfIsolines number for it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:       To approximate as a polyline.                             M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the curve's M
*		       curvature.					     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*   TrimmingCurves: Do we want the trimming curves as well.                  M
*   IsoParamCurves: Do we want trimmed isoparametric curves.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object approximating TrimSrf.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrimSrf2Polylines, conversion, approximation                           M
*****************************************************************************/
IPPolygonStruct *IPTrimSrf2Polylines(TrimSrfStruct *TrimSrf,
				     int NumOfIsolines[2],
				     CagdRType TolSamples,
				     SymbCrvApproxMethodType Method,
				     int TrimmingCurves,
				     int IsoParamCurves)
{
    CagdPolylineStruct
	*CagdPoly1 = NULL,
	*CagdPoly2 = NULL;

    if (IsoParamCurves) {
	if (NumOfIsolines[0] < 0) {
	    if (TrimSrf -> Srf -> UOrder > 2)
		NumOfIsolines[0] = ((TrimSrf -> Srf -> ULength -
				     NumOfIsolines[0]) >> 1);
	    else
		NumOfIsolines[0] = -NumOfIsolines[0];
	}
	if (NumOfIsolines[0] < 2)
	    NumOfIsolines[0] = 2;

	if (NumOfIsolines[1] < 0) {
	    if (TrimSrf -> Srf -> VOrder > 2)
		NumOfIsolines[1] = ((TrimSrf -> Srf -> VLength -
				     NumOfIsolines[1]) >> 1);
	    else
		NumOfIsolines[1] = -NumOfIsolines[1];
	}
	if (NumOfIsolines[1] < 2)
	    NumOfIsolines[1] = 2;

	switch (Method) {
	    case SYMB_CRV_APPROX_UNIFORM:
	    case SYMB_CRV_APPROX_CURVATURE:
	        if (TolSamples < 2.0)
		    TolSamples = 2.0;
		break;
	    default:
	        break;
	}

	CagdPoly1 = TrimSrf2Polylines(TrimSrf, NumOfIsolines,
				      TolSamples, Method);
    }

    if (TrimmingCurves) {
	CagdPoly2 = TrimCrvs2Polylines(TrimSrf, FALSE,
				       TolSamples, Method);
    }

    if (CagdPoly1 == NULL) {
	return IPCagdPllns2IritPllns(CagdPoly2);
    }
    else {
	CagdPolylineStruct
	    *CagdPolyLast = CagdListLast(CagdPoly1);

	CagdPolyLast -> Pnext = CagdPoly2;

	return IPCagdPllns2IritPllns(CagdPoly1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single trimmed surface's control mesh into a	     M
* polylines object.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrf:      To extract its control mesh as a polylines.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object representing TrimSrf's control   M
*		mesh.							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrimSrf2CtlMesh, conversion                                            M
*****************************************************************************/
IPPolygonStruct *IPTrimSrf2CtlMesh(TrimSrfStruct *TrimSrf)
{
    CagdPolylineStruct
	*CagdPoly = CagdSrf2CtrlMesh(TrimSrf -> Srf);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to approximate a single trivariate by polygons. Six faces of the   M
* trivariate are extracted as six surfaces that are displayed.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Trivar:       To approximate using polygons.                             M
*   FourPerFlat:  See IPSurface2Polygons.	                             M
*   FineNess:     See IPSurface2Polygons.	                             M
*   ComputeUV:    See IPSurface2Polygons.	                             M
*   ComputeNrml:  See IPSurface2Polygons.	                             M
*   Optimal:      See IPSurface2Polygons.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Resulting polygons that approximates Srf.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrivar2Polygons, approximation, conversion                             M
*****************************************************************************/
IPPolygonStruct *IPTrivar2Polygons(TrivTVStruct *Trivar,
				   int FourPerFlat,
				   IrtRType FineNess,
				   int ComputeUV,
				   int ComputeNrml,
				   int Optimal)
{
    int i;
    CagdRType t;
    CagdSrfStruct
	**Srfs = TrivBndrySrfsFromTV(Trivar);
    IPPolygonStruct
	*Polys = NULL;

    t = AttrGetRealAttrib(Trivar -> Attr, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t)) {
        AttrSetRealAttrib(&Srfs[2] -> Attr, "u_resolution", t);
        AttrSetRealAttrib(&Srfs[3] -> Attr, "u_resolution", t);
        AttrSetRealAttrib(&Srfs[4] -> Attr, "u_resolution", t);
        AttrSetRealAttrib(&Srfs[5] -> Attr, "u_resolution", t);
    }

    t = AttrGetRealAttrib(Trivar -> Attr, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t)) {
        AttrSetRealAttrib(&Srfs[0] -> Attr, "u_resolution", t);
        AttrSetRealAttrib(&Srfs[1] -> Attr, "u_resolution", t);
        AttrSetRealAttrib(&Srfs[4] -> Attr, "v_resolution", t);
        AttrSetRealAttrib(&Srfs[5] -> Attr, "v_resolution", t);
    }

    t = AttrGetRealAttrib(Trivar -> Attr, "w_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t)) {
        AttrSetRealAttrib(&Srfs[0] -> Attr, "v_resolution", t);
        AttrSetRealAttrib(&Srfs[1] -> Attr, "v_resolution", t);
        AttrSetRealAttrib(&Srfs[2] -> Attr, "v_resolution", t);
        AttrSetRealAttrib(&Srfs[3] -> Attr, "v_resolution", t);
    }

    for (i = 0; i < 6; i++) {
	IPPolygonStruct
	    *OnePolys = IPSurface2Polygons(Srfs[i], FourPerFlat, FineNess,
					   ComputeUV, ComputeNrml, Optimal),
	    *LastPoly = IPGetLastPoly(OnePolys);

	if (LastPoly != NULL) {
	    LastPoly -> Pnext = Polys;
	    Polys = OnePolys;
	}

	CagdSrfFree(Srfs[i]);
    }

    return Polys;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single trivariate function into polylines with	     M
* TolSamples/Tolerance per isocurve, NumOfIsolines isolines into a polylines M
* object.								     M
*   If NumOfIsolines has negative value, its absolute value is heuristically M
* used to derive a new NumOfIsolines number for it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Trivar:        To approximate as a polyline.                             M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the curve's M
*		       curvature.					     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object approximating Trivar.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrivar2Polylines, conversion, approximation                          M
*****************************************************************************/
IPPolygonStruct *IPTrivar2Polylines(TrivTVStruct *Trivar,
				    int NumOfIsolines[3],
				    CagdRType TolSamples,
				    SymbCrvApproxMethodType Method)
{
    int Axis;
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;
    IPPolygonStruct
	*Polys = NULL;

    TrivTVDomain(Trivar, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    for (Axis = 0; Axis < 3; Axis++) {
	CagdRType Min, Max;
	TrivTVDirType Dir;
	int i, NumOfSrfIsolines[2];

	switch (Axis) {
	    case 0:
	        Dir = TRIV_CONST_U_DIR;
		Min = UMin;
		Max = UMax;
		NumOfSrfIsolines[0] = NumOfIsolines[1];
		NumOfSrfIsolines[1] = NumOfIsolines[2];
		break;
	    case 1:
	        Dir = TRIV_CONST_V_DIR;
		Min = VMin;
		Max = VMax;
		NumOfSrfIsolines[0] = NumOfIsolines[0];
		NumOfSrfIsolines[1] = NumOfIsolines[2];
		break;
	    default:
	    case 2:
	        Dir = TRIV_CONST_W_DIR;
		Min = WMin;
		Max = WMax;
		NumOfSrfIsolines[0] = NumOfIsolines[0];
		NumOfSrfIsolines[1] = NumOfIsolines[1];
		break;
	}

	for (i = 0; i < IRIT_FABS(NumOfIsolines[Axis]); i++) {
	    CagdRType
		t = ((CagdRType) i) / (IRIT_FABS(NumOfIsolines[Axis]) - 1);
	    CagdSrfStruct
		*Srf = TrivSrfFromTV(Trivar, Min * (1.0 - t) + Max * t, Dir,
				     FALSE);
	    IPPolygonStruct
		*Poly = IPSurface2Polylines(Srf, NumOfSrfIsolines,
					    TolSamples, Method);

	    if (Polys == NULL)
		Polys = Poly;
	    else {
		IPPolygonStruct
		    *PolyLast = IPGetLastPoly(Poly);

		PolyLast -> Pnext = Polys;
		Polys = Poly;
	    }

	    CagdSrfFree(Srf);
	}
    }

    return Polys;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single trivariate's control mesh into a polylines     M
* object.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Trivar:     To extract its control mesh as a polylines.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object representing Trivar's   	     M
*		control mesh.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrivar2CtlMesh, conversion                                             M
*****************************************************************************/
IPPolygonStruct *IPTrivar2CtlMesh(TrivTVStruct *Trivar)
{
    CagdPolylineStruct
	*CagdPoly = TrivTV2CtrlMesh(Trivar);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to approximate a single triangular patch by polygons.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:       To approximate using polygons.                             M
*   FineNess:     See IPSurface2Polygons.	                             M
*   ComputeUV:    See IPSurface2Polygons.	                             M
*   ComputeNrml:  See IPSurface2Polygons.	                             M
*   Optimal:      See IPSurface2Polygons.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Resulting polygons that approximates Srf.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTriSrf2Polygons, approximation, conversion                             M
*****************************************************************************/
IPPolygonStruct *IPTriSrf2Polygons(TrngTriangSrfStruct *TriSrf,
				   IrtRType FineNess,
				   int ComputeUV,
				   int ComputeNrml,
				   int Optimal)
{
    CagdBType
	NewTriSrf = FALSE;
    IPPolygonStruct *PHead;
    CagdPolygonStruct *CagdPolygonHead;

    if (CAGD_IS_BSPLINE_SRF(TriSrf) && !TrngBspTriSrfHasOpenEC(TriSrf)) {
	TriSrf = TrngBspTriSrfOpenEnd(TriSrf);
	NewTriSrf = TRUE;
    }

    CagdPolygonHead = TrngTriSrf2Polygons(TriSrf, (int) FineNess,
					  ComputeNrml, ComputeUV);

    PHead = IPCagdPlgns2IritPlgns(CagdPolygonHead, ComputeUV);

    if (NewTriSrf)
	TrngTriSrfFree(TriSrf);

    return PHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single triangular patch function into polylines with  M
* SamplesPerCurve samples, NumOfIsolines isolines into a polylines object.   M
*   If NumOfIsolines has negative value, its absolute value is heuristically M
* used to derive a new NumOfIsolines number for it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:        To approximate as a polyline.                             M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   TolSamples:    Tolerance of approximation error (Method = 2) or          M
*                  Number of samples to compute on polyline (Method = 0, 1). M
*   Method:        0 - TolSamples are set uniformly in parametric space,     M
*                  1 - TolSamples are set optimally, considering the curve's M
*		       curvature.					     M
*		   2 - TolSamples sets the maximum error allowed between the M
*		       piecewise linear approximation and original curve.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object approximating TriSrf.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTriSrf2Polylines, conversion, approximation                            M
*****************************************************************************/
IPPolygonStruct *IPTriSrf2Polylines(TrngTriangSrfStruct *TriSrf,
				    int NumOfIsolines[3],
				    CagdRType TolSamples,
				    SymbCrvApproxMethodType Method)
{
    int i;
    CagdPolylineStruct *CagdPoly;

    for (i = 0; i < 3; i++) {
	if (NumOfIsolines[i] < 0) {
	    if (TriSrf -> Order > 2)
	        NumOfIsolines[i] = ((TriSrf -> Length - NumOfIsolines[i]) >> 1);
	    else
	        NumOfIsolines[i] = -NumOfIsolines[i];
	}
	if (NumOfIsolines[i] < 2)
	    NumOfIsolines[i] = 2;
    }

    if (TolSamples < 2.0)
	TolSamples = 2.0;

    CagdPoly = TrngTriSrf2Polylines(TriSrf, NumOfIsolines, TolSamples,
				    Method);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single triangular patch's control mesh into a	     M
* polylines object.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:     To extract its control mesh as a polylines.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A polylines object representing TriSrf's   	     M
*		control mesh.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTriSrf2CtlMesh, conversion                                             M
*****************************************************************************/
IPPolygonStruct *IPTriSrf2CtlMesh(TrngTriangSrfStruct *TriSrf)
{
    CagdPolylineStruct
	*CagdPoly = TrngTriSrf2CtrlMesh(TriSrf);

    return IPCagdPllns2IritPllns(CagdPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Sets the tolerance that is used by the Bezier to Cubic Bezier 	     M
* conversion routines IritCurvesToCubicBzrCrvs and			     M
* IritSurfacesToCubicBzrCrvs						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Tolerance:   Of approximation to use.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  Old value of cubic Bezier tolerance.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSetCurvesToCubicBzrTol, approximation                                  M
*****************************************************************************/
IrtRType IPSetCurvesToCubicBzrTol(IrtRType Tolerance)
{
    IrtRType
	OldVal = GlblCubicBzrTol;

    GlblCubicBzrTol = Tolerance;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Approximates an arbitrary list of curves into cubic Beziers curves.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crvs:          To approximate as cubic Bezier curves.                    M
*   CtlPolys:      If we want control polygons as well (DrawCtlPoly == TRUE) M
*                  they will be placed herein.				     M
*   DrawCurve:     Do we want to draw the curves?                            M
*   DrawCtlPoly:   Do we want to draw the control polygons?                  M
*   MaxArcLen:     Tolerance for cubic Bezier approximation. See function    M
*                  SymbApproxCrvAsBzrCubics.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The cubic Bezier approximation, or NULL if DrawCurve  M
*                      is FALSE.                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPCurvesToCubicBzrCrvs, conversion, approximation                        M
*****************************************************************************/
CagdCrvStruct *IPCurvesToCubicBzrCrvs(CagdCrvStruct *Crvs,
				      IPPolygonStruct **CtlPolys,
				      CagdBType DrawCurve,
				      CagdBType DrawCtlPoly,
				      CagdRType MaxArcLen)
{
    CagdCrvStruct *BzrCrvs, *BzrCrv, *Crv, *CubicBzrCrvs,
	*AllCubicBzrCrvs = NULL;

    if (DrawCtlPoly)
	*CtlPolys = NULL;

    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	if (DrawCtlPoly) {
	    IPPolygonStruct
		*CagdPoly = IPCagdPllns2IritPllns(CagdCrv2CtrlPoly(Crv));

	    CagdPoly -> Pnext = *CtlPolys;
	    *CtlPolys = CagdPoly;
	}

	if (DrawCurve) {
	    if (CAGD_IS_BEZIER_CRV(Crv)) {
		CubicBzrCrvs = SymbApproxCrvAsBzrCubics(Crv, GlblCubicBzrTol,
							MaxArcLen);
		AllCubicBzrCrvs = CagdListAppend(AllCubicBzrCrvs,
						 CubicBzrCrvs);
	    }
	    else if (CAGD_IS_BSPLINE_CRV(Crv)) {
		BzrCrvs = CagdCnvrtBsp2BzrCrv(Crv);

		for (BzrCrv = BzrCrvs;
		     BzrCrv != NULL;
		     BzrCrv = BzrCrv -> Pnext) {
		    CubicBzrCrvs = SymbApproxCrvAsBzrCubics(BzrCrv,
							    GlblCubicBzrTol,
							    MaxArcLen);
		    AllCubicBzrCrvs = CagdListAppend(AllCubicBzrCrvs,
						     CubicBzrCrvs);
		}
		CagdCrvFreeList(BzrCrvs);
	    }
	}
    }
    return AllCubicBzrCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Approximates an arbitrary list of surfaces into cubic Beziers curves.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:          To approximate as cubic Bezier curves.                    M
*   CtlMeshes:     If we want control meshes as well (DrawMesh == TRUE)      M
*                  they will be placed herein.				     M
*   DrawSurface:   Do we want to draw the surfaces?                          M
*   DrawMesh:      Do we want to draw the control meshes?                    M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   MaxArcLen:     Tolerance for cubic Bezier approximation. See function    M
*                  SymbApproxCrvAsBzrCubics.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The cubic Bezier approximation, or NULL if DrawSurface M
*                     is FALSE.                                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSurfacesToCubicBzrSrfs                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSurfacesToCubicBzrCrvs, conversion, approximation                      M
*****************************************************************************/
CagdCrvStruct *IPSurfacesToCubicBzrCrvs(CagdSrfStruct *Srfs,
					IPPolygonStruct **CtlMeshes,
					CagdBType DrawSurface,
					CagdBType DrawMesh,
					int NumOfIsolines[2],
					CagdRType MaxArcLen)
{
    CagdCrvStruct
	*AllCubicBzrCrvs = NULL;
    CagdSrfStruct *Srf;

    *CtlMeshes = NULL;

    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	if (DrawMesh) {
	    IPPolygonStruct
		*CagdPoly = IPCagdPllns2IritPllns(CagdSrf2CtrlMesh(Srf)),
		*CagdPolyTemp = CagdPoly;

	    if (CagdPolyTemp)
		while (CagdPolyTemp -> Pnext)
		    CagdPolyTemp = CagdPolyTemp -> Pnext;
	    CagdPolyTemp -> Pnext = *CtlMeshes;
	    *CtlMeshes = CagdPoly;
	}

	if (DrawSurface) {
	    CagdCrvStruct *CubicBzrCrvs,
		*Crvs = CagdSrf2Curves(Srf, NumOfIsolines);

	    CubicBzrCrvs = IPCurvesToCubicBzrCrvs(Crvs, NULL, TRUE, FALSE,
						  MaxArcLen);
	    CagdCrvFreeList(Crvs);
	    AllCubicBzrCrvs = CagdListAppend(AllCubicBzrCrvs, CubicBzrCrvs);
	}
    }
    return AllCubicBzrCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Approximates an arbitrary list of trimmed surfaces into cubic Beziers      M
* curves. Only isoparametric curves are extarcted (no trimming curves).      M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimSrfs:      To approximate as cubic Bezier curves.                    M
*   CtlMeshes:     If we want control meshes as well (DrawMesh == TRUE)      M
*                  they will be placed herein.				     M
*   DrawTrimSrf:   Do we want to draw the surfaces?                          M
*   DrawMesh:      Do we want to draw the control meshes?                    M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   MaxArcLen:     Tolerance for cubic Bezier approximation. See function    M
*                  SymbApproxCrvAsBzrCubics.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The cubic Bezier approximation, or NULL if DrawSurface M
*                     is FALSE.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrimSrfsToCubicBzrCrvs, conversion, approximation                      M
*****************************************************************************/
CagdCrvStruct *IPTrimSrfsToCubicBzrCrvs(TrimSrfStruct *TrimSrfs,
					IPPolygonStruct **CtlMeshes,
					CagdBType DrawTrimSrf,
					CagdBType DrawMesh,
					int NumOfIsolines[2],
					CagdRType MaxArcLen)
{
    CagdCrvStruct
	*AllCubicBzrCrvs = NULL;
    TrimSrfStruct *TrimSrf;

    if (DrawMesh)
	*CtlMeshes = NULL;

    for (TrimSrf = TrimSrfs; TrimSrf != NULL; TrimSrf = TrimSrf -> Pnext) {
	if (DrawMesh) {
	    IPPolygonStruct
		*CagdPoly = IPCagdPllns2IritPllns(
					   CagdSrf2CtrlMesh(TrimSrf -> Srf)),
		*CagdPolyTemp = CagdPoly;

	    if (CagdPolyTemp)
		while (CagdPolyTemp -> Pnext)
		    CagdPolyTemp = CagdPolyTemp -> Pnext;
	    CagdPolyTemp -> Pnext = *CtlMeshes;
	    *CtlMeshes = CagdPoly;
	}

	if (DrawTrimSrf) {
	    CagdCrvStruct *CubicBzrCrvs,
		*Crvs = TrimSrf2Curves(TrimSrf, NumOfIsolines);

	    CubicBzrCrvs = IPCurvesToCubicBzrCrvs(Crvs, NULL, TRUE, FALSE,
						  MaxArcLen);
	    CagdCrvFreeList(Crvs);
	    AllCubicBzrCrvs = CagdListAppend(AllCubicBzrCrvs, CubicBzrCrvs);
	}
    }
    return AllCubicBzrCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a single trivariate function into cubic Bezier curves.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Trivar:        To approximate as a set of cubic bezier curves.	     M
*   CtlMeshes:     If we want control meshes as well (DrawMesh == TRUE)      M
*                  they will be placed herein.				     M
*   DrawTrivar:    Do we want to draw the trivariate function?               M
*   DrawMesh:      Do we want to draw the control meshes?                    M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   MaxArcLen:     Tolerance for cubic Bezier approximation. See function    M
*                  SymbApproxCrvAsBzrCubics.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A list of curves approximating Trivar.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTrivarToCubicBzrCrvs, conversion, approximation                        M
*****************************************************************************/
CagdCrvStruct *IPTrivarToCubicBzrCrvs(TrivTVStruct *Trivar,
				      IPPolygonStruct **CtlMeshes,
				      CagdBType DrawTrivar,
				      CagdBType DrawMesh,
				      int NumOfIsolines[2],
				      CagdRType MaxArcLen)
{
    int Axis;
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;
    CagdCrvStruct
	*Crvs = NULL;

    TrivTVDomain(Trivar, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    *CtlMeshes = NULL;

    for (Axis = 0; Axis < 3; Axis++) {
	CagdRType Min, Max;
	TrivTVDirType Dir;
	int i, NumOfSrfIsolines[2];

	switch (Axis) {
	    case 0:
	        Dir = TRIV_CONST_U_DIR;
		Min = UMin;
		Max = UMax;
		NumOfSrfIsolines[0] = NumOfIsolines[1];
		NumOfSrfIsolines[1] = NumOfIsolines[2];
		break;
	    case 1:
	        Dir = TRIV_CONST_V_DIR;
		Min = VMin;
		Max = VMax;
		NumOfSrfIsolines[0] = NumOfIsolines[0];
		NumOfSrfIsolines[1] = NumOfIsolines[2];
		break;
	    default:
	    case 2:
	        Dir = TRIV_CONST_W_DIR;
		Min = WMin;
		Max = WMax;
		NumOfSrfIsolines[0] = NumOfIsolines[0];
		NumOfSrfIsolines[1] = NumOfIsolines[1];
		break;
	}

	for (i = 0; i < IRIT_FABS(NumOfIsolines[Axis]); i++) {
	    CagdRType
		t = ((CagdRType) i) / (IRIT_FABS(NumOfIsolines[Axis]) - 1);
	    CagdSrfStruct
		*Srf = TrivSrfFromTV(Trivar, Min * (1.0 - t) + Max * t, Dir,
				     FALSE);
	    IPPolygonStruct *CtlMesh;
	    CagdCrvStruct
		*Crv = IPSurfacesToCubicBzrCrvs(Srf, &CtlMesh, DrawTrivar,
						DrawMesh, NumOfSrfIsolines,
						MaxArcLen);

	    if (Crv == NULL)
		Crvs = Crv;
	    else {
		CagdCrvStruct
		    *CrvLast = CagdListLast(Crv);

		if (CrvLast) {
		    CrvLast -> Pnext = Crvs;
		    Crvs = Crv;
		}
	    }

	    if (*CtlMeshes == NULL)
		*CtlMeshes = CtlMesh;
	    else {
		IPPolygonStruct
		    *MeshLast = IPGetLastPoly(CtlMesh);

		if (MeshLast) {
		    MeshLast -> Pnext = *CtlMeshes;
		    *CtlMeshes = CtlMesh;
		}
	    }

	    CagdSrfFree(Srf);
	}
    }

    return Crvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Approximates an arbitrary list of triangular surfaces into cubic Beziers   M
* curves.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrfs:       To approximate as cubic Bezier curves.                    M
*   CtlMeshes:     If we want control meshes as well (DrawMesh == TRUE)      M
*                  they will be placed herein.				     M
*   DrawSurface:   Do we want to draw the surfaces?                          M
*   DrawMesh:      Do we want to draw the control meshes?                    M
*   NumOfIsolines: Number of isocurves to extract, in each direction.        M
*   MaxArcLen:     Tolerance for cubic Bezier approximation. See function    M
*                  SymbApproxCrvAsBzrCubics.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The cubic Bezier approximation, or NULL if DrawSurface M
*                     is FALSE.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPTriSrfsToCubicBzrCrvs, conversion, approximation                       M
*****************************************************************************/
CagdCrvStruct *IPTriSrfsToCubicBzrCrvs(TrngTriangSrfStruct *TriSrfs,
				       IPPolygonStruct **CtlMeshes,
				       CagdBType DrawSurface,
				       CagdBType DrawMesh,
				       int NumOfIsolines[3],
				       CagdRType MaxArcLen)
{
    CagdCrvStruct
	*AllCubicBzrCrvs = NULL;
    TrngTriangSrfStruct *TriSrf;

    *CtlMeshes = NULL;

    for (TriSrf = TriSrfs; TriSrf != NULL; TriSrf = TriSrf -> Pnext) {
	if (DrawMesh) {
	    IPPolygonStruct
		*CagdPoly =
		    IPCagdPllns2IritPllns(TrngTriSrf2CtrlMesh(TriSrf)),
		*CagdPolyTemp = CagdPoly;

	    if (CagdPolyTemp)
		while (CagdPolyTemp -> Pnext)
		    CagdPolyTemp = CagdPolyTemp -> Pnext;
	    CagdPolyTemp -> Pnext = *CtlMeshes;
	    *CtlMeshes = CagdPoly;
	}

	if (DrawSurface) {
	    CagdCrvStruct *CubicBzrCrvs,
		*Crvs = TrngTriSrf2Curves(TriSrf, NumOfIsolines);

	    CubicBzrCrvs = IPCurvesToCubicBzrCrvs(Crvs, NULL, TRUE, FALSE,
						  MaxArcLen);
	    CagdCrvFreeList(Crvs);
	    AllCubicBzrCrvs = CagdListAppend(AllCubicBzrCrvs, CubicBzrCrvs);
	}
    }
    return AllCubicBzrCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts an arbitrary list of surfaces into cubic integral Bezier          M
* surfaces, if possible.  Any rational or surface with degrees higher than   M
* cubic cannot be converted and is left as is in the NoConvertionSrfs list.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:          To approximate as cubic Bezier surfaces.                  M
*   NoConvertionSrfs:  List of surface that cannot be converted to bicubics. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The cubic Bezier approximation, or NULL if nothing     M
*                     has been converted.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPSurfacesToCubicBzrCrvs                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPSurfacesToCubicBzrSrfs, conversion, approximation                      M
*****************************************************************************/
CagdSrfStruct *IPSurfacesToCubicBzrSrfs(CagdSrfStruct *Srfs,
					CagdSrfStruct **NoConvertionSrfs)
{
    CagdSrfStruct *Srf, *TSrf, *TSrfs, *DSrf,
	*RetList = NULL;

    *NoConvertionSrfs = NULL;

    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	if (CAGD_IS_RATIONAL_SRF(Srf) ||
	    Srf -> UOrder > 4 ||
	    Srf -> VOrder > 4) {
	    /* Not exact convertion is possible for this surface. */
	    TSrf = CagdSrfCopy(Srf);
	    IRIT_LIST_PUSH(TSrf, (*NoConvertionSrfs));
	}
	else {
	    /* Convert Bspline surface to piecewise Bezier */
	    if (CAGD_IS_BEZIER_SRF(Srf))
		TSrfs = CagdSrfCopy(Srf);
	    else if (CAGD_IS_BSPLINE_SRF(Srf))
	        TSrfs = CagdCnvrtBsp2BzrSrf(Srf);
	    else
		TSrfs = NULL;

	    /* Convert lower order Bezier to bicubic Bezier. */
	    while (TSrfs != NULL) {
		TSrf = TSrfs;
		TSrfs = TSrfs -> Pnext;
		TSrf -> Pnext = NULL;

		while (TSrf -> UOrder < 4) {
		    DSrf = BzrSrfDegreeRaise(TSrf, CAGD_CONST_U_DIR);
		    CagdSrfFree(TSrf);
		    TSrf = DSrf;
		}
		while (TSrf -> VOrder < 4) {
		    DSrf = BzrSrfDegreeRaise(TSrf, CAGD_CONST_V_DIR);
		    CagdSrfFree(TSrf);
		    TSrf = DSrf;
		}

		IRIT_LIST_PUSH(TSrf, RetList);
	    } 
	}
    }

    return RetList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Forces the given list of polygons to have open list of vertices          M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:  Polygons to process, in place.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPOpenPolysToClosed                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPClosedPolysToOpen                                                      M
*****************************************************************************/
void IPClosedPolysToOpen(IPPolygonStruct *Pls)
{
    IPPolygonStruct *Pl;

    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *VStart = Pl -> PVertex,
	    *V = VStart;

	while (V -> Pnext != NULL && V -> Pnext != VStart)
	    V = V -> Pnext;
	V -> Pnext = NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Forces the given list of polygons to have closed list of vertices        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pls:  Polygons to process, in place.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IPClosedPolysToOpen                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   IPOpenPolysToClosed                                                      M
*****************************************************************************/
void IPOpenPolysToClosed(IPPolygonStruct *Pls)
{
    IPPolygonStruct *Pl;

    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *VStart = Pl -> PVertex,
	    *V = VStart;

	while (V -> Pnext != NULL && V -> Pnext != VStart)
	    V = V -> Pnext;
	V -> Pnext = VStart;
    }
}
