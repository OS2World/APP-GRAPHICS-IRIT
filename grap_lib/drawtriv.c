/*****************************************************************************
*   Default surface drawing routine common to graphics drivers.		     *
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
#include "ip_cnvrt.h"
#include "geom_lib.h"
#include "grap_loc.h"

static IPObjectStruct *IGGenTrivarSrfPolygons(IPObjectStruct *PObj,
					      IrtRType FineNess);
static void IGDrawTrivarSrfSketch(IPObjectStruct *PObj);
static IPObjectStruct *IGGenTrivarSrfSketch(IPObjectStruct *PObj,
					    IrtRType FineNess);
static void IGDrawTrivarSrfPolylines(IPObjectStruct *PObj);
static IPObjectStruct *IGGenTrivarSrfPolylines(IPObjectStruct *PObj,
					       IrtRType FineNess);
static void IGGenTrivarSrfDiscontinuities(IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single trivariate function object using current modes and 	     M
* transformations.							     M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlmesh" M
* attributes of PObj. Polygonal approximation is saved under "_polygons".    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A trivariate function object to draw.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawTrivar                                                             M
*****************************************************************************/
void IGDrawTrivar(IPObjectStruct *PObj)
{
    if (IGGlblDrawSurfaceMesh &&
	(_IGDrawCtlMeshPreFunc == NULL || _IGDrawCtlMeshPreFunc())) {
	IPObjectStruct *PObjCtlMesh;
	IPPolygonStruct *PCtlMesh, *PTemp;

	if ((PObjCtlMesh = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    TrivTVStruct *TV,
		*TVs = PObj -> U.Trivars;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (TV = TVs; TV != NULL; TV = TV -> Pnext) {
		PCtlMesh = IPTrivar2CtlMesh(TV);

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
	    IGGenTrivarSrfDiscontinuities(PObj);

	IGDrawTrivarGenSrfPolygons(PObj);

	if (IGGlblDrawSurfaceContours)
	    IGDrawPolyContours(PObj);

	if (IGGlblDrawSurfaceIsophotes)
	    IGDrawPolyIsophotes(PObj);

	if (_IGDrawSrfPolysPostFunc != NULL)
	    _IGDrawSrfPolysPostFunc();
    }

    if (IGGlblDrawSurfaceWire &&
	(_IGDrawSrfWirePreFunc == NULL || _IGDrawSrfWirePreFunc())) {
	IGDrawTrivarSrfPolylines(PObj);

	if (_IGDrawSrfWirePostFunc != NULL)
	    _IGDrawSrfWirePostFunc();
    }

    if (IGGlblDrawSurfaceSketch &&
	(_IGDrawSketchPreFunc == NULL || _IGDrawSketchPreFunc())) {
	IGDrawTrivarSrfSketch(PObj);

	if (_IGDrawSketchPostFunc != NULL)
	    _IGDrawSketchPostFunc();
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the polygonal approximation of the trivariate on the fly       M
* if needed and display it.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A trivariate(s) object.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawTrivarGenSrfPolygons                                               M
*****************************************************************************/
void IGDrawTrivarGenSrfPolygons(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolygons;

    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								    == NULL) {
	PObjPolygons = IGGenTrivarSrfPolygons(PObj, 1.0);

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
	    PObjPolygons = IGGenTrivarSrfPolygons(PObj,
						  IGGlblRelLowresFineNess);

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
*   PObj:      A trivariate(s) object.                                       *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polygonal approximation.                        *
*****************************************************************************/
static IPObjectStruct *IGGenTrivarSrfPolygons(IPObjectStruct *PObj,
					      IrtRType FineNess)
{
    int OldVal;
    TrivTVStruct *TV,
	*TVs = PObj -> U.Trivars;
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
    RelativeFineNess *= FineNess;

    for (TV = TVs; TV != NULL; TV = TV -> Pnext) {
	IrtRType t;

	t = AttrGetObjectRealAttrib(PObj, "u_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TV -> Attr, "u_resolution", t);
	t = AttrGetObjectRealAttrib(PObj, "v_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TV -> Attr, "v_resolution", t);
	t = AttrGetObjectRealAttrib(PObj, "w_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TV -> Attr, "w_resolution", t);

	PPolygons = IPTrivar2Polygons(TV, IGGlblFourPerFlat,
				      IGGlblPlgnFineness * RelativeFineNess,
				      IGGlblAlwaysGenUV, TRUE,
				      IGGlblPolygonOptiApprox);

	if (PPolygons != NULL) {
	    if (PPolygons) {
		PPolygonTemp = IPGetLastPoly(PPolygons);
		PPolygonTemp -> Pnext = PObjPolygons -> U.Pl;
		PObjPolygons -> U.Pl = PPolygons;
	    }
	}
    }

    CagdSrf2PolygonStrip(OldVal);

    PObjPolygons -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    AttrSetObjectIntAttrib(PObjPolygons, "_srf_polys", TRUE);

    return PObjPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a sketch like drawing of the trivariate on the fly if needed   *
* and display it.                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A trivariate(s) object.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTrivarSrfSketch(IPObjectStruct *PObj)
{
   IPObjectStruct *PObjSketches;

    if ((PObjSketches = AttrGetObjectObjAttrib(PObj, "_sketches")) == NULL) {
	PObjSketches = IGGenTrivarSrfSketch(PObj, 1.0);

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
*   PObj:      A trivariate(s) object.                                       *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The sketch data.	                             *
*****************************************************************************/
static IPObjectStruct *IGGenTrivarSrfSketch(IPObjectStruct *PObj,
					    IrtRType FineNess)
{
    int i, j;
    IrtRType
        RelativeFineNess = AttrGetObjectRealAttrib(PObj, "res_sketch");
    CagdSrfStruct **Srfs;
    IPObjectStruct *PPtsObj;

    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	RelativeFineNess = 1.0;

    Srfs = TrivBndrySrfsFromTV(PObj -> U.Trivars);

    PPtsObj = IPGenLISTObject(NULL);

    for (i = j = 0; i < 6; i++) {
	IPObjectStruct
	    *PSketch = IGSketchGenSrfSketches(Srfs[i],
					      RelativeFineNess * FineNess,
					      PObj, FALSE);

	if (j == 0)
	    AttrSetObjectPtrAttrib(PSketch, "_SphCones",
				   GMSphConeQueryInit(PSketch));

	IPListObjectInsert(PPtsObj, j++, PSketch);

	PSketch = IGSketchGenSrfSketches(Srfs[i],
					 RelativeFineNess * FineNess,
					 PObj, TRUE);

	IPListObjectInsert(PPtsObj, j++, PSketch);

	CagdSrfFree(Srfs[i]);
    }

    IPListObjectInsert(PPtsObj, j++, NULL);

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the polyline approximation of the trivariate on the fly        *
* if needed and display it.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A trivariate(s) object.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTrivarSrfPolylines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								    == NULL) {
	if ((PObjPolylines = IGGenTrivarSrfPolylines(PObj, 1.0)) == NULL)
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
	    PObjPolylines = IGGenTrivarSrfPolylines(PObj,
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
*   PObj:      A trivariate(s) object.                                       *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polyline approximation.                         *
*****************************************************************************/
static IPObjectStruct *IGGenTrivarSrfPolylines(IPObjectStruct *PObj,
					       IrtRType FineNess)
{
    IrtRType RelativeFineNess;
    IPObjectStruct
	*PObjPolylines = NULL;
    IPPolygonStruct *PPolylines,  *PPolylineTemp;
    TrivTVStruct *TV,
        *TVs = PObj -> U.Trivars;

    RelativeFineNess = IGGlblPolylineOptiApprox ?
				       1.0 / (FineNess + IRIT_EPS) : FineNess;

    PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
    PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolylines);
    for (TV = TVs; TV != NULL; TV = TV -> Pnext) {
        int NumOfIso[3],
	    Res = (int) (IRIT_MAX(IGGlblNumOfIsolines * FineNess / 2, 2));

	NumOfIso[0] = -Res;
	NumOfIso[1] = -Res;
	NumOfIso[2] = -Res;
	PPolylines = IPTrivar2Polylines(TV, NumOfIso,
					IGGlblPllnFineness * RelativeFineNess,
					IGGlblPolylineOptiApprox);

	if (PPolylines != NULL) {
	    PPolylineTemp = IPGetLastPoly(PPolylines);
	    PPolylineTemp -> Pnext = PObjPolylines -> U.Pl;
	    PObjPolylines -> U.Pl = PPolylines;
	}
    }

    PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

    return PObjPolylines;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the (tangent) discontinuities' polyline approximation of the   *
* surface on the fly if needed.			                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A surface(s) object.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void					                             *
*****************************************************************************/
static void IGGenTrivarSrfDiscontinuities(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjDisconts;

    if ((PObjDisconts = AttrGetObjectObjAttrib(PObj, "_Disconts")) == NULL) {
        int i;
        CagdCrvStruct *Crv, *Crvs,
	    *DiscontCrvs = NULL;
	IPPolygonStruct *PPolyline;
	CagdSrfStruct
	    **Srfs = TrivBndrySrfsFromTV(PObj -> U.Trivars);

	for (i = 0; i < 6; i++) {
	    Crvs = BspSrfC1DiscontCrvs(Srfs[i]);

	    DiscontCrvs = CagdListAppend(DiscontCrvs, Crvs);

	    CagdSrfFree(Srfs[i]);
	}

	PObjDisconts = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjDisconts -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjDisconts);
	for (Crv = DiscontCrvs; Crv != NULL; Crv = Crv -> Pnext) {
	    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2) {
	        CagdCrvStruct
		    *TCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, TRUE);

		PPolyline = IPCurve2Polylines(TCrv,
					      IGGlblPllnFineness,
					      IGGlblPolylineOptiApprox);
		CagdCrvFree(TCrv);
	    }
	    else
	        PPolyline = IPCurve2Polylines(Crv,
					      IGGlblPllnFineness,
					      IGGlblPolylineOptiApprox);

	    PObjDisconts -> U.Pl = IPAppendPolyLists(PPolyline,
						     PObjDisconts -> U.Pl);
	}

	PObjDisconts -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

	AttrSetObjectObjAttrib(PObj, "_Disconts", PObjDisconts, FALSE);
    }
}
