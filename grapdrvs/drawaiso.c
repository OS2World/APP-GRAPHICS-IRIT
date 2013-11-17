/*****************************************************************************
*   Surface drawing routine using adaptive isocurves.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, Dec. 2001.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"
#include "grap_loc.h"

IRIT_STATIC_DATA int
    GlblRuledSrfApprox = FALSE;

static IPPolygonStruct *IritSurface2AdapIso(CagdSrfStruct *Srf,
					    CagdSrfDirType Dir,
					    IrtRType Eps);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Surface object using current modes and transformations.	     M
*   Surface must be with either E3 or P3 point type and must be a NURB srf.  M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlmesh" M
* attributes of PObj. Adaptive isocurves are saved under "_adap_iso" and     M
* polygons under "_polygons.".						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A surface object to draw.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawSurfaceAIso                                                        M
*****************************************************************************/
void IGDrawSurface(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines, *PObjCtlMesh, *PObjPolygons;
    IPPolygonStruct *PPolylines, *PCtlMesh, *PPolygons, *PPolyTemp;
    IrtRType
	Tol = IGGlblPllnFineness < 1.0 ? 1.0 / IGGlblPllnFineness
				       : IGGlblPllnFineness;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_isoline")) == NULL &&
	IGGlblNumOfIsolines > 0) {
	CagdSrfStruct *Srf,
	    *Srfs = PObj -> U.Srfs;

	PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjPolylines);
	for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	    int NumOfIso[2];

	    NumOfIso[0] = -IGGlblNumOfIsolines;
	    NumOfIso[1] = -IGGlblNumOfIsolines;
	    PPolylines = IPSurface2Polylines(Srf, NumOfIso, Tol,
					     SYMB_CRV_APPROX_UNIFORM);

	    if (PPolylines != NULL) {
		for (PPolyTemp = PPolylines;
		     PPolyTemp -> Pnext;
		     PPolyTemp = PPolyTemp -> Pnext);
		PPolyTemp -> Pnext = PObjPolylines -> U.Pl;
		PObjPolylines -> U.Pl = PPolylines;
	    }
	}
	AttrSetObjectObjAttrib(PObj, "_isoline", PObjPolylines, FALSE);
    }

    if (IGGlblDrawStyle == IG_STATE_DRAW_STYLE_SOLID) {
	if (IGGlblDrawSurfacePoly) {
	    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_polygons"))
								    == NULL) {
		CagdSrfStruct *Srf,
		    *Srfs = PObj -> U.Srfs;

		PObjPolygons = IPAllocObject("", IP_OBJ_POLY, NULL);
		PObjPolygons -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
		IP_SET_POLYGON_OBJ(PObjPolygons);

		for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		    PPolygons = IPSurface2Polygons(Srf, IGGlblFourPerFlat,
						   IGGlblPlgnFineness, FALSE,
						   TRUE,
						     IGGlblPolygonOptiApprox);

		    if (PPolygons != NULL) {
			if (PPolygons) {
			    for (PPolyTemp = PPolygons;
				 PPolyTemp -> Pnext;
				 PPolyTemp = PPolyTemp -> Pnext);
			    PPolyTemp -> Pnext = PObjPolygons -> U.Pl;
			    PObjPolygons -> U.Pl = PPolygons;
			}
		    }
		}
		AttrSetObjectObjAttrib(PObj, "_polygons", PObjPolygons, FALSE);
	    }

	    IGDrawPoly(PObjPolygons);
	}
	else {
	    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_adap_iso"))
								    == NULL) {
		CagdSrfStruct *Srf,
		    *Srfs = PObj -> U.Srfs;

		PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
		PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
		IP_SET_POLYLINE_OBJ(PObjPolylines);

		for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		    PPolylines = IritSurface2AdapIso(Srf,
					     (CagdSrfDirType) IGGlblAdapIsoDir,
					     1.0 / IGGlblNumOfIsolines);

		    if (PPolylines) {
			for (PPolyTemp = PPolylines;
			     PPolyTemp -> Pnext;
			     PPolyTemp = PPolyTemp -> Pnext);
			PPolyTemp -> Pnext = PObjPolylines -> U.Pl;
			PObjPolylines -> U.Pl = PPolylines;
		    }
		}
		AttrSetObjectObjAttrib(PObj, "_adap_iso", PObjPolylines, FALSE);
	    }

	    IGDrawPolylineNormal(PObjPolylines);
	}
    }
    else
        IGDrawPoly(PObjPolylines);

    if (IGGlblDrawSurfaceMesh) {
	if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    CagdSrfStruct *Srf,
		*Srfs = PObj -> U.Srfs;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		PCtlMesh = IPSurface2CtlMesh(Srf);

		for (PPolyTemp = PCtlMesh;
		     PPolyTemp -> Pnext;
		     PPolyTemp = PPolyTemp -> Pnext);
		PPolyTemp -> Pnext = PObjCtlMesh -> U.Pl;
		PObjCtlMesh -> U.Pl = PCtlMesh;
	    }
	    AttrSetObjectObjAttrib(PObj, "_ctlmesh", PObjCtlMesh, FALSE);
	}

	IGDrawPoly(AttrGetObjectObjAttrib(PObj, "_ctlmesh"));
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Creates an adaptive isocurve coverage to a given surface.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:                Surface to convert to adaptive isocurve's coverage.  *
*   Dir:                Direction of isocurves. Either U or V.               *
*   Eps:                Accuracy of coverage.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:  A list of polylines approximating the coverage.      *
*****************************************************************************/
static IPPolygonStruct *IritSurface2AdapIso(CagdSrfStruct *Srf,
					    CagdSrfDirType Dir,
					    IrtRType Eps)
{
    IrtRType
	Tol = IGGlblPllnFineness < 1.0 ? 1.0 / IGGlblPllnFineness
				       : IGGlblPllnFineness;
    CagdCrvStruct *Coverage, *OneCoverage, *Crv;
    IPPolygonStruct
	*IsoPoly = NULL;
    CagdSrfStruct *NSrf;

    if (Dir == CAGD_NO_DIR)
	Dir = Srf -> UOrder == 2 ? CAGD_CONST_U_DIR : CAGD_CONST_V_DIR;

    if (GlblRuledSrfApprox) {
	int IsBspline = TRUE;
	CagdSrfStruct *RuledSrfs, *NormalSrf, *TSrf;

	if (CAGD_IS_BEZIER_SRF(Srf)) {
	    IsBspline = FALSE;
	    Srf = CagdCnvrtBzr2BspSrf(Srf);
	}
	RuledSrfs = SymbPiecewiseRuledSrfApprox(Srf, TRUE, Eps, Dir);

	Coverage = NULL;
	NormalSrf = SymbSrfNormalSrf(Srf);
	for (TSrf = RuledSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    CagdRType UMin, UMax, VMin, VMax;

	    CagdSrfDomain(TSrf, &UMin, &UMax, &VMin, &VMax);
	    if (Dir == CAGD_CONST_V_DIR)
		NSrf = CagdSrfRegionFromSrf(NormalSrf, UMin, UMax,
					    CAGD_CONST_U_DIR);
	    else
		NSrf = CagdSrfRegionFromSrf(NormalSrf, VMin, VMax,
					    CAGD_CONST_V_DIR);
	    OneCoverage = SymbAdapIsoExtract(TSrf, NSrf, NULL, Dir,
					     Eps, FALSE, FALSE);
	    CagdSrfFree(NSrf);

	    for (Crv = OneCoverage; Crv -> Pnext != NULL; Crv = Crv -> Pnext);
	    Crv -> Pnext = Coverage;
	    Coverage = OneCoverage;
	}
	CagdSrfFreeList(RuledSrfs);
	CagdSrfFree(NormalSrf);
	if (!IsBspline)
	    CagdSrfFree(Srf);

	/* Scan the adaptive isoline list. Note that normal curve is paired */
	/* after Euclidean curve, so we can step two curves at a time.      */
	for (Crv = Coverage; Crv != NULL; Crv = Crv -> Pnext -> Pnext) {
	    CagdCrvStruct
		*NCrv = Crv -> Pnext;
	    IPPolygonStruct
		*Poly = IPCurve2Polylines(Crv, 1, SYMB_CRV_APPROX_UNIFORM);

	    if (Poly != NULL) {
		IrtRType
		    *Nrml1 = Poly -> PVertex -> Normal,
		    *Nrml2 = Poly -> PVertex -> Pnext -> Normal;

		CagdCoerceToE3(Nrml1, NCrv -> Points, 0, NCrv -> PType);
		CagdCoerceToE3(Nrml2, NCrv -> Points, NCrv -> Length - 1,
			       NCrv -> PType);
		IRIT_PT_NORMALIZE(Nrml1);
		IRIT_PT_SCALE(Nrml1, -1.0);
		IRIT_PT_NORMALIZE(Nrml2);
		IRIT_PT_SCALE(Nrml2, -1.0);

		Poly -> Pnext = IsoPoly;
		IsoPoly = Poly;
	    }
	}
    }
    else {
	NSrf = SymbSrfNormalSrf(Srf);

	Coverage = SymbAdapIsoExtract(Srf, NSrf, NULL, Dir, Eps, FALSE, FALSE);

	CagdSrfFree(NSrf);

	/* Scan the adaptive isoline list. Note that normal curve is paired */
	/* after Euclidean curve, so we can step two curves at a time.      */
	for (Crv = Coverage; Crv != NULL; Crv = Crv -> Pnext -> Pnext) {
	    IPVertexStruct *VP, *VN;
	    IPPolygonStruct
		*Poly = IPCurve2Polylines(Crv, Tol, SYMB_CRV_APPROX_UNIFORM),
		*NPoly = IPCurve2Polylines(Crv -> Pnext, Tol,
					   SYMB_CRV_APPROX_UNIFORM);

	    if (Poly != NULL && NPoly != NULL) {
		for (VP = Poly -> PVertex, VN = NPoly -> PVertex;
		     VP != NULL;
		     VP = VP -> Pnext, VN = VN -> Pnext) {
		    IRIT_PT_COPY(VP -> Normal, VN -> Coord);
		    IRIT_PT_NORMALIZE(VP -> Normal);
		    IRIT_PT_SCALE(VP -> Normal, -1.0);
		}

		IPFreePolygonList(NPoly);

		Poly -> Pnext = IsoPoly;
		IsoPoly = Poly;
	    }
	}
    }

    CagdCrvFreeList(Coverage);

    return IsoPoly;    
}
