/*****************************************************************************
*   Default model drawing routine common to graphics drivers.		     *
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
#include "mdl_lib.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"
#include "grap_loc.h"

static IPObjectStruct *IGGenModelPolygons(IPObjectStruct *PObj,
					    IrtRType FineNess);
static void IGDrawModelSketch(IPObjectStruct *PObj);
static IPObjectStruct *IGGenModelSketches(IPObjectStruct *PObj,
					  IrtRType FineNess);
static void IGDrawModelPolylines(IPObjectStruct *PObj);
static IPObjectStruct *IGGenModelPolylines(IPObjectStruct *PObj,
					     IrtRType FineNess);
static void IGDrawModelGenDiscontinuities(IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single model object using current modes and transformations.	     M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlmesh" M
* attributes of PObj. Polygonal approximation is saved under "_polygons".    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A model object to draw.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawModel                                                              M
*****************************************************************************/
void IGDrawModel(IPObjectStruct *PObj)
{
    if (IGGlblDrawSurfaceMesh &&
	(_IGDrawCtlMeshPreFunc == NULL || _IGDrawCtlMeshPreFunc())) {
	IPObjectStruct *PObjCtlMesh;
	IPPolygonStruct *PCtlMesh, *PTemp;

	if ((PObjCtlMesh = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    TrimSrfStruct *TSrf,
		*TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

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

	    TrimSrfFreeList(TrimSrfs);
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
	    IGDrawModelGenDiscontinuities(PObj);

	IGDrawModelGenPolygons(PObj);

	if (IGGlblDrawSurfaceContours)
	    IGDrawPolyContours(PObj);

	if (IGGlblDrawSurfaceIsophotes)
	    IGDrawPolyIsophotes(PObj);

	if (_IGDrawSrfPolysPostFunc != NULL)
	    _IGDrawSrfPolysPostFunc();
    }

    if (IGGlblDrawSurfaceWire &&
	(_IGDrawSrfWirePreFunc == NULL || _IGDrawSrfWirePreFunc())) {
	IGDrawModelPolylines(PObj);

	if (_IGDrawSrfWirePostFunc != NULL)
	    _IGDrawSrfWirePostFunc();
    }

    if (IGGlblDrawSurfaceSketch &&
	(_IGDrawSketchPreFunc == NULL || _IGDrawSketchPreFunc())) {
	IGDrawModelSketch(PObj);

	if (_IGDrawSketchPostFunc != NULL)
	    _IGDrawSketchPostFunc();
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the polygonal approximation of the model on the fly if         M
* needed and display it.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A model(s) object.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawModelGenPolygons                                                   M
*****************************************************************************/
void IGDrawModelGenPolygons(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolygons;

    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								    == NULL) {
	PObjPolygons = IGGenModelPolygons(PObj, 1.0);

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
	    PObjPolygons = IGGenModelPolygons(PObj, IGGlblRelLowresFineNess);

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
*   PObj:      A model(s) object.                                            *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polygonal approximation.                        *
*****************************************************************************/
static IPObjectStruct *IGGenModelPolygons(IPObjectStruct *PObj,
					  IrtRType FineNess)
{
    int OldVal;
    TrimSrfStruct *TSrf,
	*TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);
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
					        : FineNess;;

    for (TSrf = TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
        int HasTexture = IGInitSrfTexture(PObj);
	IrtRType t;

	t = AttrGetObjectRealAttrib(PObj, "u_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TSrf -> Attr, "u_resolution", t);
	t = AttrGetObjectRealAttrib(PObj, "v_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&TSrf -> Attr, "v_resolution", t);

	PPolygons = IPTrimSrf2Polygons(TSrf, IGGlblFourPerFlat,
				       RelativeFineNess * IGGlblPlgnFineness,
				       HasTexture, TRUE,
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

		    Uv[0] = (float) ((Uv[0] - UMin) * UScale / (UMax - UMin));
		    Uv[1] = (float) ((Uv[1] - VMin) * VScale / (VMax - VMin));

		    V = V -> Pnext;
		}
		while (V != NULL && V != Poly -> PVertex);
	    }
	}

	if (PPolygons != NULL) {
	    if (PPolygons) {
		PPolygonTemp = IPGetLastPoly(PPolygons);
		PPolygonTemp -> Pnext = PObjPolygons -> U.Pl;
		PObjPolygons -> U.Pl = PPolygons;
	    }
	}
    }
    TrimSrfFreeList(TrimSrfs);

    CagdSrf2PolygonStrip(OldVal);

    AttrSetObjectIntAttrib(PObjPolygons, "_srf_polys", TRUE);

    return PObjPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a sketch like drawing of the model on the fly if needed        *
* and display it.                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A model(s) object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawModelSketch(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjSketches;

    if ((PObjSketches = AttrGetObjectObjAttrib(PObj, "_sketches")) == NULL) {
	PObjSketches = IGGenModelSketches(PObj, 1.0);

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
*   PObj:      A model(s) object.                                            *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The sketch data.	                             *
*****************************************************************************/
static IPObjectStruct *IGGenModelSketches(IPObjectStruct *PObj,
					  IrtRType FineNess)
{
    int i = 0;
    IrtRType
        RelativeFineNess = AttrGetObjectRealAttrib(PObj, "res_sketch");
    IPObjectStruct *PPtsObj;
    TrimSrfStruct *TSrf,
	*TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	RelativeFineNess = 1.0;

    PPtsObj = IPGenLISTObject(NULL);

    for (TSrf = TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	IPObjectStruct
	    *PSketch = IGSketchGenSrfSketches(TSrf -> Srf,
					      RelativeFineNess * FineNess,
					      PObj, FALSE);
	if (i == 0)
	    AttrSetObjectPtrAttrib(PSketch, "_SphCones",
				   GMSphConeQueryInit(PSketch));

	IPListObjectInsert(PPtsObj, i++, PSketch);

	PSketch = IGSketchGenSrfSketches(TSrf -> Srf,
					 RelativeFineNess * FineNess,
					 PObj, TRUE);

	IPListObjectInsert(PPtsObj, i++, PSketch);
    }

    TrimSrfFreeList(TrimSrfs);

    IPListObjectInsert(PPtsObj, i++, NULL);

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the polyline approximation of the model on the fly if          *
* needed and display it.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A model(s) object.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawModelPolylines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								    == NULL) {
	if ((PObjPolylines = IGGenModelPolylines(PObj, 1.0)) == NULL)
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
	    PObjPolylines = IGGenModelPolylines(PObj, IGGlblRelLowresFineNess);
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
*   PObj:      A model(s) object.                                            *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polyline approximation.                         *
*****************************************************************************/
static IPObjectStruct *IGGenModelPolylines(IPObjectStruct *PObj,
					     IrtRType FineNess)
{
    IPObjectStruct
	*PObjPolylines = NULL;

    if (IGGlblNumOfIsolines > 0) {
        IrtRType RelativeFineNess;
	IPPolygonStruct *PPolylines,  *PPolylineTemp;
	TrimSrfStruct *TSrf,
	    *TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

	RelativeFineNess = IGGlblPolylineOptiApprox ?
				       1.0 / (FineNess + IRIT_EPS) : FineNess;

	PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjPolylines);
	for (TSrf = TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    int NumOfIso[2];

	    NumOfIso[0] = -IGGlblNumOfIsolines;
	    NumOfIso[1] = -IGGlblNumOfIsolines;
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

	TrimSrfFreeList(TrimSrfs);
    }

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
static void IGDrawModelGenDiscontinuities(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjDisconts;

    if ((PObjDisconts = AttrGetObjectObjAttrib(PObj, "_Disconts")) == NULL) {
	IPPolygonStruct *PPolyline;
	CagdCrvStruct *Crv, *DCrvs;
	TrimSrfStruct *TSrf,
	    *TrimSrfs = MdlCnvrtMdl2TrimmedSrfs(PObj -> U.Mdls);

	PObjDisconts = IPAllocObject("", IP_OBJ_POLY, NULL);
	PObjDisconts -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	IP_SET_POLYLINE_OBJ(PObjDisconts);
	for (TSrf = TrimSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	    DCrvs = BspSrfC1DiscontCrvs(TSrf -> Srf);

	    for (Crv = DCrvs; Crv != NULL; Crv = Crv -> Pnext) {
	        IrtRType Param;
		CagdCrvStruct *TCrv, *TCrvs;
		TrimIsoInterStruct **Inters;

		if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2)
		    TCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, TRUE);
		else
		    TCrv = CagdCrvCopy(Crv);

		Param = AttrGetRealAttrib(Crv -> Attr, "C1DiscVal");
		Inters = TrimIntersectTrimCrvIsoVals(TSrf,
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
	    CagdCrvFree(DCrvs);
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
