/*****************************************************************************
*   Default triangular surface drawing routine common to graphics drivers.   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, Aug. 1996.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "trng_lib.h"
#include "ip_cnvrt.h"
#include "grap_loc.h"

static IPObjectStruct *IGGenTriangSrfPolygons(IPObjectStruct *PObj,
					      IrtRType FineNess);
static void IGDrawTriangSrfSketch(IPObjectStruct *PObj);
static void IGDrawTriangSrfPolylines(IPObjectStruct *PObj);
static IPObjectStruct *IGGenTriangSrfPolylines(IPObjectStruct *PObj,
					       IrtRType FineNess);


/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single triangular surface object using current modes and	     M
* transformations.							     M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlmesh" M
* attributes of PObj. Polygonal approximation is saved under "_polygons".    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A triangular surface object to draw.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawTriangSrf                                                          M
*****************************************************************************/
void IGDrawTriangSrf(IPObjectStruct *PObj)
{
    if (IGGlblDrawSurfaceMesh &&
	(_IGDrawCtlMeshPreFunc == NULL || _IGDrawCtlMeshPreFunc())) {
	IPObjectStruct *PObjCtlMesh;
	IPPolygonStruct *PCtlMesh, *PTemp;

	if ((PObjCtlMesh = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    TrngTriangSrfStruct *TSrf,
		*TriSrfs = PObj -> U.TriSrfs;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (TSrf = TriSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
		PCtlMesh = IPTriSrf2CtlMesh(TSrf);

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
	 IGGlblDrawSurfaceSilh) &&
	(_IGDrawSrfPolysPreFunc == NULL || _IGDrawSrfPolysPreFunc())) {
	IGDrawTriangGenSrfPolygons(PObj);

	if (_IGDrawSrfPolysPostFunc != NULL)
	    _IGDrawSrfPolysPostFunc();
    }

    if (IGGlblDrawSurfaceWire &&
	(_IGDrawSrfWirePreFunc == NULL || _IGDrawSrfWirePreFunc())) {
	IGDrawTriangSrfPolylines(PObj);

	if (_IGDrawSrfWirePostFunc != NULL)
	    _IGDrawSrfWirePostFunc();
    }

    if (IGGlblDrawSurfaceSketch &&
	(_IGDrawSketchPreFunc == NULL || _IGDrawSketchPreFunc())) {
	IGDrawTriangSrfSketch(PObj);

	if (_IGDrawSketchPostFunc != NULL)
	    _IGDrawSketchPostFunc();
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the polygonal approximation of the triangular surface on the   M
* fly if needed and display it.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A triangular surface(s) object.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawTriangGenSrfPolygons                                               M
*****************************************************************************/
void IGDrawTriangGenSrfPolygons(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolygons;

    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								    == NULL) {
	PObjPolygons = IGGenTriangSrfPolygons(PObj, 1.0);

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
	    PObjPolygons = IGGenTriangSrfPolygons(PObj,
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
*   PObj:      A triangular surface(s) object.                               *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polygonal approximation.                        *
*****************************************************************************/
static IPObjectStruct *IGGenTriangSrfPolygons(IPObjectStruct *PObj,
					      IrtRType FineNess)
{
    int OldVal;
    TrngTriangSrfStruct *TSrf,
        *TriSrfs = PObj -> U.TriSrfs;
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

    for (TSrf = TriSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
	PPolygons = IPTriSrf2Polygons(TSrf,
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
*   Generates a sketch like drawing of the triangular surface on the fly if  *
* needed and display it.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A triangular surface(s) object.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTriangSrfSketch(IPObjectStruct *PObj)
{
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the polyline approximation of the triangular surface on the    *
* fly if needed and display it.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A triangular surface(s) object.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawTriangSrfPolylines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								    == NULL) {
	if ((PObjPolylines = IGGenTriangSrfPolylines(PObj, 1.0)) == NULL)
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
	    PObjPolylines = IGGenTriangSrfPolylines(PObj,
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
*   PObj:      A triangular surface(s) object.                               *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polyline approximation.                         *
*****************************************************************************/
static IPObjectStruct *IGGenTriangSrfPolylines(IPObjectStruct *PObj,
					       IrtRType FineNess)
{
    IrtRType RelativeFineNess;
    IPObjectStruct
	*PObjPolylines = NULL;
    IPPolygonStruct *PPolylines,  *PPolylineTemp;
    TrngTriangSrfStruct *TSrf,
        *TriSrfs = PObj -> U.TriSrfs;

    RelativeFineNess = IGGlblPolylineOptiApprox ?
				       1.0 / (FineNess + IRIT_EPS) : FineNess;

    PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
    PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolylines);
    for (TSrf = TriSrfs; TSrf != NULL; TSrf = TSrf -> Pnext) {
        int NumOfIso[3],
	    Res = (int) (IRIT_MAX(IGGlblNumOfIsolines * FineNess, 2));

	NumOfIso[0] = -Res;
	NumOfIso[1] = -Res;
	NumOfIso[2] = -Res;
	PPolylines = IPTriSrf2Polylines(TSrf, NumOfIso,
					(int) (IGGlblPllnFineness *
					                   RelativeFineNess),
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
