/*****************************************************************************
*   Default trimmed surface drawing routine common to graphics drivers.	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "trim_lib.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"
#include "grap_loc.h"

static IPObjectStruct *IGGenTrimSrfPolygons(IPObjectStruct *PObj,
					    IrtRType FineNess);

static void IGDrawTrimSrfGenSketch(IPObjectStruct *PObj);
static IPObjectStruct *IGGenTrimSrfSketches(IPObjectStruct *PObj,
					    IrtRType FineNess);

static void IGDrawTrimSrfGenPolylines(IPObjectStruct *PObj);
static IPObjectStruct *IGGenTrimSrfPolylines(IPObjectStruct *PObj,
					     IrtRType FineNess);

static void IGDrawTrimSrfGenDiscontinuities(IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single trimmed surface object using current modes and		     M
* transformations.							     M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlmesh" M
* attributes of PObj. Polygonal approximation is saved under "_polygons".    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A trimmed surface object to draw.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawTrimSrf                                                            M
*****************************************************************************/
void IGDrawTrimSrf(IPObjectStruct *PObj)
{
    if (PObj -> U.TrimSrfs == NULL || PObj -> U.TrimSrfs -> Srf == NULL)
        return;

    if (IGGlblDrawSurfaceMesh &&
	(_IGDrawCtlMeshPreFunc == NULL || _IGDrawCtlMeshPreFunc())) {
	IPObjectStruct *PObjCtlMesh;
	IPPolygonStruct *PCtlMesh, *PTemp;

	if ((PObjCtlMesh = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    TrimSrfStruct *TSrf,
		*TrimSrfs = PObj -> U.TrimSrfs;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (TSrf = TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
		PCtlMesh = IPTrimSrf2CtlMesh(TSrf);

		PTemp = IPGetLastPoly(PCtlMesh);
		PTemp -> Pnext = PObjCtlMesh -> U.Pl;
		PObjCtlMesh -> U.Pl = PCtlMesh;
	    }
	    if (IGGlblCacheGeom)
		AttrSetObjectObjAttrib(PObj, "_ctlmesh", PObjCtlMesh, FALSE);
	}

        if (PObjCtlMesh != NULL) {
	    IGDrawPolyFuncPtr(PObjCtlMesh);

	    if (!IGGlblCacheGeom)
		IPFreeObject(PObjCtlMesh);
	}

	if (_IGDrawCtlMeshPostFunc != NULL)
	    _IGDrawCtlMeshPostFunc();
    }

    if ((IGGlblDrawSurfacePoly ||
	 IGGlblDrawSurfaceBndry ||
	 IGGlblDrawSurfaceSilh ||
	 IGGlblDrawSurfaceDiscont ||
	 IGGlblDrawSurfaceContours ||
	 IGGlblDrawSurfaceIsophotes) &&
	(_IGDrawSrfPolysPreFunc == NULL || _IGDrawSrfPolysPreFunc())) {
        if (IGGlblDrawSurfaceDiscont)
	    IGDrawTrimSrfGenDiscontinuities(PObj);

	IGDrawTrimSrfGenPolygons(PObj);

	if (IGGlblDrawSurfaceContours)
	    IGDrawPolyContours(PObj);

	if (IGGlblDrawSurfaceIsophotes)
	    IGDrawPolyIsophotes(PObj);

	if (_IGDrawSrfPolysPostFunc != NULL)
	    _IGDrawSrfPolysPostFunc();
    }

    if (IGGlblDrawSurfaceWire &&
	(_IGDrawSrfWirePreFunc == NULL || _IGDrawSrfWirePreFunc())) {
	IGDrawTrimSrfGenPolylines(PObj);

	if (_IGDrawSrfWirePostFunc != NULL)
	    _IGDrawSrfWirePostFunc();
    }

    if (IGGlblDrawSurfaceSketch &&
	(_IGDrawSketchPreFunc == NULL || _IGDrawSketchPreFunc())) {
	IGDrawTrimSrfGenSketch(PObj);

	if (_IGDrawSketchPostFunc != NULL)
	    _IGDrawSketchPostFunc();
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the polygonal approximation of the trimmed surface on the fly  M
* if needed and display it.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A trimmed surface(s) object.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawTrimSrfGenPolygons                                                 M
*****************************************************************************/
void IGDrawTrimSrfGenPolygons(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolygons;

    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								    == NULL) {
	PObjPolygons = IGGenTrimSrfPolygons(PObj, 1.0);

	if (IGGlblCacheGeom)
	    AttrSetObjectObjAttrib(PObj, "_PolygonsHiRes",
				   PObjPolygons, FALSE);
	else {
	    if (PObjPolygons != NULL) {
		IGDrawPolyFuncPtr(PObjPolygons);
		
		IPFreeObject(PObjPolygons);
	    }
	    return;
	}

	if (IRIT_APX_EQ(IGGlblRelLowresFineNess, 1.0)) {
	    AttrSetObjectPtrAttrib(PObj, "_PolygonsLoRes", PObjPolygons);
	}
	else {
	    PObjPolygons = IGGenTrimSrfPolygons(PObj, IGGlblRelLowresFineNess);

	    AttrSetObjectObjAttrib(PObj, "_PolygonsLoRes",
				   PObjPolygons, FALSE);
	}
    }

    PObjPolygons = IGGetObjPolygons(PObj);

    IGGlblLastLowResDraw = IGGlblManipulationActive;

    if (PObjPolygons != NULL)
	IGDrawPolyFuncPtr(PObjPolygons);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a polygonal approximation for the given object with the        *
* prescribed fineness.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A trimmed surface(s) object.                                  *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polygonal approximation.                        *
*****************************************************************************/
static IPObjectStruct *IGGenTrimSrfPolygons(IPObjectStruct *PObj,
					    IrtRType FineNess)
{
    int OldVal;
    TrimSrfStruct *TSrf,
	*TSrfs = PObj -> U.TrimSrfs;
    IrtRType RelativeFineNess;
    IPPolygonStruct *PPolygons, *PPolygonTemp;
    IPObjectStruct *PObjPolygons;

    OldVal = CagdSrf2PolygonStrip(IGGlblPolygonStrips);

    PObjPolygons = IPAllocObject("", IP_OBJ_POLY, NULL);
    PObjPolygons -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    IP_SET_POLYGON_OBJ(PObjPolygons);
    RelativeFineNess = AttrGetObjectRealAttrib(PObj, "resolution");
    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	RelativeFineNess = 1.0;
    RelativeFineNess *= IGGlblPolygonOptiApprox ? 1.0 / (FineNess + IRIT_EPS)
					        : FineNess;

    for (TSrf = TSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
        int HasTexture = IGInitSrfTexture(PObj);
	IrtRType t;

	t = AttrGetObjectRealAttrib(PObj, "u_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TSrf -> Attr, "u_resolution", t);
	t = AttrGetObjectRealAttrib(PObj, "v_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TSrf -> Attr, "v_resolution", t);

	PPolygons = IPTrimSrf2Polygons(TSrf, IGGlblFourPerFlat,
				       IGGlblPlgnFineness * RelativeFineNess,
				       HasTexture || IGGlblAlwaysGenUV, TRUE,
				       IGGlblPolygonOptiApprox);

	if (HasTexture) {
	    CagdRType UMin, UMax, VMin, VMax,
		UScale = AttrGetObjectRealAttrib(PObj, "_ImageScaleX"),
		VScale = AttrGetObjectRealAttrib(PObj, "_ImageScaleY");
	    IPPolygonStruct *Poly;

	    /* Go over all polygons and set the proper UV domain. */
	    TrimSrfDomain(TSrf, &UMin, &UMax, &VMin, &VMax);
	    
	    for (Poly = PPolygons; Poly != NULL; Poly = Poly -> Pnext) {
		IPVertexStruct
		    *V = Poly -> PVertex;

		do {
		    float
			*Uv = AttrGetUVAttrib(V -> Attr, "uvvals");

		    if (!IRIT_APX_EQ(UScale, 0.0) &&
			!IRIT_APX_EQ(VScale, 0.0)) {
		        Uv[0] = (float) ((Uv[0] - UMin) * UScale /
					                       (UMax - UMin));
			Uv[1] = (float) ((Uv[1] - VMin) * VScale /
					                       (VMax - VMin));
		    }

		    V = V -> Pnext;
		}
		while (V != NULL && V != Poly -> PVertex);
	    }
	}

	if (PPolygons != NULL) {
	    PPolygonTemp = IPGetLastPoly(PPolygons);
	    PPolygonTemp -> Pnext = PObjPolygons -> U.Pl;
	    PObjPolygons -> U.Pl = PPolygons;
	}
    }

    CagdSrf2PolygonStrip(OldVal);

    PObjPolygons -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    AttrSetObjectIntAttrib(PObjPolygons, "_srf_polys", TRUE);

    return PObjPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a sketch like drawing of the trimmed surface on the fly if     *
* needed and display it.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A trimmed surface(s) object.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTrimSrfGenSketch(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjSketches;

    if ((PObjSketches = AttrGetObjectObjAttrib(PObj, "_sketches")) == NULL) {
	PObjSketches = IGGenTrimSrfSketches(PObj, 1.0);

	if (IGGlblCacheGeom)
	    AttrSetObjectObjAttrib(PObj, "_sketches", PObjSketches, FALSE);
    }

    IGSketchDrawSurface(PObjSketches);

    if (!IGGlblCacheGeom)
	IPFreeObject(PObjSketches);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a sketch like drawing for the given object with the prescribed *
* fineness.    		                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A surface(s) object.                                          *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The sketch data.	                             *
*****************************************************************************/
static IPObjectStruct *IGGenTrimSrfSketches(IPObjectStruct *PObj,
					    IrtRType FineNess)
{
    IrtRType
        RelativeFineNess = AttrGetObjectRealAttrib(PObj, "res_sketch");
    IPObjectStruct *PPtsObj1, *PPtsObj2, *PPtsObj;

    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	RelativeFineNess = 1.0;

    PPtsObj1 = IGSketchGenSrfSketches(PObj -> U.TrimSrfs -> Srf,
				      RelativeFineNess * FineNess,
				      PObj, FALSE);
    if (PPtsObj1 -> U.Pl -> PVertex != NULL) {
        PPtsObj1 -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	AttrSetObjectPtrAttrib(PPtsObj1, "_SphCones",
			       GMSphConeQueryInit(PPtsObj1));
    }

    PPtsObj2 = IGSketchGenSrfSketches(PObj -> U.TrimSrfs -> Srf,
				      RelativeFineNess * FineNess,
				      PObj, TRUE);

    PPtsObj = IPGenLISTObject(PPtsObj1);
    IPListObjectInsert(PPtsObj, 1, PPtsObj2);
    IPListObjectInsert(PPtsObj, 2, NULL);

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the polyline approximation of the trimmed surface on the fly   *
* if needed and display it.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A trimmed surface(s) object.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTrimSrfGenPolylines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								    == NULL) {
	if ((PObjPolylines = IGGenTrimSrfPolylines(PObj, 1.0)) == NULL)
	    return;

	if (IGGlblCacheGeom)
	    AttrSetObjectObjAttrib(PObj, "_IsolinesHiRes",
				   PObjPolylines, FALSE);
	else {
	    if (PObjPolylines != NULL) {
		IGDrawPolyFuncPtr(PObjPolylines);
		
		IPFreeObject(PObjPolylines);
	    }
	    return;
	}

	if (IRIT_APX_EQ(IGGlblRelLowresFineNess, 1.0)) {
	    AttrSetObjectPtrAttrib(PObj, "_IsolinesLoRes", PObjPolylines);
	}
	else {
	    PObjPolylines = IGGenTrimSrfPolylines(PObj,
						  IGGlblRelLowresFineNess);
	    AttrSetObjectObjAttrib(PObj, "_IsolinesLoRes",
				   PObjPolylines, FALSE);
	}
    }

    PObjPolylines = IGGetObjIsoLines(PObj);

    IGGlblLastLowResDraw = IGGlblManipulationActive;

    if (PObjPolylines != NULL)
	IGDrawPolyFuncPtr(PObjPolylines);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a polyline approximation for the given object with the         *
* prescribed fineness.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A trimmed surface(s) object.                                  *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polyline approximation.                         *
*****************************************************************************/
static IPObjectStruct *IGGenTrimSrfPolylines(IPObjectStruct *PObj,
					     IrtRType FineNess)
{
    IrtRType RelativeFineNess;
    IPObjectStruct
	*PObjPolylines = NULL;

    RelativeFineNess = IGGlblPolylineOptiApprox ?
				       1.0 / (FineNess + IRIT_EPS) : FineNess;

    if (IGGlblNumOfIsolines > 0) {
	IPPolygonStruct *PPolylines,  *PPolylineTemp;
	TrimSrfStruct *TSrf,
	    *TrimSrfs = PObj -> U.TrimSrfs;

	PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjPolylines);
	for (TSrf = TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    int NumOfIso[2],
		Res = (int) (IRIT_MAX(IGGlblNumOfIsolines * FineNess, 2));

	    NumOfIso[0] = -Res;
	    NumOfIso[1] = -Res;
	    PPolylines = IPTrimSrf2Polylines(TSrf, NumOfIso,
					     IGGlblPllnFineness * 
					                     RelativeFineNess,
					     IGGlblPolylineOptiApprox,
					     TRUE, TRUE);

	    if (PPolylines != NULL) {
		PPolylineTemp = IPGetLastPoly(PPolylines);
		PPolylineTemp -> Pnext = PObjPolylines -> U.Pl;
		PObjPolylines -> U.Pl = PPolylines;
	    }
	}
    }

    if (PObjPolylines != NULL)
        PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

    return PObjPolylines;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the (tangent) discontinuities' polyline approximation of the   *
* surface on the fly if needed and display it.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A surface(s) object.                                          *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTrimSrfGenDiscontinuities(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjDisconts;

    if ((PObjDisconts = AttrGetObjectObjAttrib(PObj, "_Disconts")) == NULL) {
        CagdCrvStruct
	    *DiscontCrvs = BspSrfC1DiscontCrvs(PObj -> U.TrimSrfs -> Srf);
	IPPolygonStruct *PPolyline;
	CagdCrvStruct *Crv;

	PObjDisconts = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjDisconts -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjDisconts);
	for (Crv = DiscontCrvs; Crv != NULL; Crv = Crv -> Pnext) {
	    IrtRType Param;
	    CagdCrvStruct *TCrv, *TCrvs;
	    TrimIsoInterStruct **Inters;

	    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2)
	        TCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, TRUE);
	    else
	        TCrv = CagdCrvCopy(Crv);

	    Param = AttrGetRealAttrib(Crv -> Attr, "C1DiscVal");
	    Inters = TrimIntersectTrimCrvIsoVals(PObj -> U.TrimSrfs,
				  AttrGetIntAttrib(Crv -> Attr, "C1DiscDir"),
				  &Param, 1, TRUE);

	    if ((TCrvs = TrimCrvTrimParamList(TCrv, Inters[0])) != NULL) {
		for (TCrv = TCrvs; TCrv != NULL; TCrv = TCrv -> Pnext) {
		    PPolyline = IPCurve2Polylines(TCrv, IGGlblPllnFineness,
						  IGGlblPolylineOptiApprox);

		    PObjDisconts -> U.Pl = IPAppendPolyLists(PPolyline,
						       PObjDisconts -> U.Pl);
		}

	        CagdCrvFree(TCrvs);
	    }
	    else
	        CagdCrvFree(TCrv);

	    IritFree(Inters);
	}

	if (IGGlblCacheGeom)
	    AttrSetObjectObjAttrib(PObj, "_Disconts", PObjDisconts, FALSE);
	else {
	    if (PObjDisconts != NULL) {
	        IGDrawPolyFuncPtr(PObjDisconts);

		IPFreeObject(PObjDisconts);
	    }
	    return;
	}
    }

    IGDrawPolyFuncPtr(PObjDisconts);
}
