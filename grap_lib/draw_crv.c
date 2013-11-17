/*****************************************************************************
*   Default curve drawing routine common to graphics drivers.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, June 1993.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "grap_loc.h"

static IPObjectStruct *IGGenCurvePolylines(IPObjectStruct *PObj,
					   IrtRType FineNess);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single Curve object using current modes and transformations.	     M
*   Curve must be with either E3 or P3 point type and must be a NURB curve.  M
*   Piecewise linear approximation is cashed under "_isoline" and "_ctlpoly" M
* attributes of PObj.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A curve object to draw.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawCurve                                                              M
*****************************************************************************/
void IGDrawCurve(IPObjectStruct *PObj)
{
    if (PObj -> U.Crvs == NULL)
        return;

    if (IGGlblDrawSurfaceMesh &&
	(_IGDrawCtlMeshPreFunc == NULL || _IGDrawCtlMeshPreFunc())) {
	IPObjectStruct *PObjCtlMesh;
	IPPolygonStruct *PCtlMesh;

	if ((PObjCtlMesh = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    CagdCrvStruct *Crv,
		*Crvs = PObj -> U.Crvs;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
		if ((PCtlMesh = IPCurve2CtlPoly(Crv)) != NULL) {
		    /* A linear Bezier can yield no control polygon. */
		    PCtlMesh -> Pnext = PObjCtlMesh -> U.Pl;
		    PObjCtlMesh -> U.Pl = PCtlMesh;
		}
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

    if (IGGlblDrawSurfaceWire &&
	(_IGDrawSrfWirePreFunc == NULL || _IGDrawSrfWirePreFunc())) {
        IGDrawCurveGenPolylines(PObj);

	if (_IGDrawSrfWirePostFunc != NULL)
	    _IGDrawSrfWirePostFunc();
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the polyline approximation of the curve on the fly if          M
* needed and display it.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A curve(s) object.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawCurveGenPolylines                                                  M
*****************************************************************************/
void IGDrawCurveGenPolylines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								    == NULL) {

	PObjPolylines = IGGenCurvePolylines(PObj, 1.0);

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
	    PObjPolylines = IGGenCurvePolylines(PObj, IGGlblRelLowresFineNess);
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
*   PObj:      A curve(s) object.                                            *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polyline approximation.                         *
*****************************************************************************/
static IPObjectStruct *IGGenCurvePolylines(IPObjectStruct *PObj,
					   IrtRType FineNess)
{
    IrtRType RelativeFineNess;
    IPObjectStruct
	*PObjPolylines = NULL;
    IPPolygonStruct *PPolyline;
    CagdCrvStruct *Crv,
        *Crvs = PObj -> U.Crvs;

    RelativeFineNess = IGGlblPolylineOptiApprox ?
				       1.0 / (FineNess + IRIT_EPS) : FineNess;

    PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
    PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolylines);
    for (Crv = Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2) {
	    CagdCrvStruct
		*TCrv = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, TRUE);

	    PPolyline = IPCurve2Polylines(TCrv,
				      IGGlblPllnFineness * RelativeFineNess,
				      IGGlblPolylineOptiApprox);
	    CagdCrvFree(TCrv);
	}
	else
	    PPolyline = IPCurve2Polylines(Crv,
				      IGGlblPllnFineness * RelativeFineNess,
				      IGGlblPolylineOptiApprox);

	PObjPolylines -> U.Pl = IPAppendPolyLists(PPolyline,
						  PObjPolylines -> U.Pl);
    }

    PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);

    return PObjPolylines;
}
