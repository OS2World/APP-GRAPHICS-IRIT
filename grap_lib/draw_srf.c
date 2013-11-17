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
#include "user_lib.h"
#include "ip_cnvrt.h"
#include "geom_lib.h"
#include "grap_loc.h"

#define RFLCT_LN_SIL_EPS	0.01

static IPObjectStruct *IGGenSurfacePolygons(IPObjectStruct *PObj,
					    IrtRType FineNess);

static void IGDrawSurfaceGenSketches(IPObjectStruct *PObj);
static IPObjectStruct *IGGenSurfaceSketches(IPObjectStruct *PObj,
					    IrtRType FineNess);

static void IGDrawSurfaceGenPolylines(IPObjectStruct *PObj);
static IPObjectStruct *IGGenSurfacePolylines(IPObjectStruct *PObj,
					     IrtRType FineNess);

static void IGDrawSurfaceGenDiscontinuities(IPObjectStruct *PObj);

static void IGDrawSurfaceRefLines(IPObjectStruct *PObj,
				  IPObjectStruct *LinesObj);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Draw a single surface object using current modes and transformations.	     M
*   Piecewise linear approximation is cashed under "_Isoline??Res" and       M
* "_ctlmesh" attributes of PObj. Polygonal approximation is saved under      M
* "_Polygons??Res".							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     A surface object to draw.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawSurface                                                            M
*****************************************************************************/
void IGDrawSurface(IPObjectStruct *PObj)
{
    IPObjectStruct *RflctLnObj;

    if (PObj -> U.Srfs == NULL)
        return;

    if (IGGlblDrawSurfaceMesh &&
	(_IGDrawCtlMeshPreFunc == NULL || _IGDrawCtlMeshPreFunc())) {
	IPObjectStruct *PObjCtlMesh;
	IPPolygonStruct *PCtlMesh, *PTemp;

	if ((PObjCtlMesh = AttrGetObjectObjAttrib(PObj, "_ctlmesh"))
								== NULL) {
	    CagdSrfStruct *Srf,
		*Srfs = PObj -> U.Srfs;

	    PObjCtlMesh = IPAllocObject("", IP_OBJ_POLY, NULL);
	    PObjCtlMesh -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	    IP_SET_POLYLINE_OBJ(PObjCtlMesh);
	    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
		if ((PCtlMesh = IPSurface2CtlMesh(Srf)) != NULL) {
		    PTemp = IPGetLastPoly(PCtlMesh);
		    PTemp -> Pnext = PObjCtlMesh -> U.Pl;
		    PObjCtlMesh -> U.Pl = PCtlMesh;
		}
	    }
	    if (PObjCtlMesh -> U.Pl == NULL) {
		IPFreeObject(PObjCtlMesh);
		PObjCtlMesh = NULL;
	    }

	    if (IGGlblCacheGeom && PObjCtlMesh != NULL)
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
	    IGDrawSurfaceGenDiscontinuities(PObj);

	IGDrawSurfaceGenPolygons(PObj);

	if (IGGlblDrawSurfaceContours)
	    IGDrawPolyContours(PObj);

	if (IGGlblDrawSurfaceIsophotes)
	    IGDrawPolyIsophotes(PObj);

	if (_IGDrawSrfPolysPostFunc != NULL)
	    _IGDrawSrfPolysPostFunc();
    }

    if (IGGlblDrawSurfaceWire &&
	(_IGDrawSrfWirePreFunc == NULL || _IGDrawSrfWirePreFunc())) {
	IGDrawSurfaceGenPolylines(PObj);

	if (_IGDrawSrfWirePostFunc != NULL)
	    _IGDrawSrfWirePostFunc();
    }

    if (IGGlblDrawSurfaceSketch &&
	(_IGDrawSketchPreFunc == NULL || _IGDrawSketchPreFunc())) {
	IGDrawSurfaceGenSketches(PObj);

	if (_IGDrawSketchPostFunc != NULL)
	    _IGDrawSketchPostFunc();
    }

    if (IGGlblDrawSurfaceRflctLns &&
	(RflctLnObj = AttrGetObjectObjAttrib(PObj, "RflctLines")) != NULL)
	IGDrawSurfaceRefLines(PObj, RflctLnObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates the polygonal approximation of the surface on the fly if       M
* needed and display it.                                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      A surface(s) object.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGDrawSurfaceGenPolygons                                                 M
*****************************************************************************/
void IGDrawSurfaceGenPolygons(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolygons, *PObjPl;

    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								    == NULL) {
	PObjPolygons = IGGenSurfacePolygons(PObj, 1.0);

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
	    PObjPolygons = IGGenSurfacePolygons(PObj, IGGlblRelLowresFineNess);

	    AttrSetObjectObjAttrib(PObj, "_PolygonsLoRes",
				   PObjPolygons, FALSE);
	}
    }

    PObjPolygons = IGGetObjPolygons(PObj);

    IGGlblLastLowResDraw = IGGlblManipulationActive;

    if (PObjPolygons != NULL) {
	/* Propagate geom info if has it only on original surface. */
	if (AttrGetObjectObjAttrib(PObjPolygons, "_Disconts") == NULL &&
	    (PObjPl = AttrGetObjectObjAttrib(PObj, "_Disconts")) != NULL)
	    AttrSetObjectObjAttrib(PObjPolygons, "_Disconts", 
			           IPCopyObject(NULL, PObjPl, FALSE), FALSE);

	if (AttrGetObjectObjAttrib(PObjPolygons, "_Contours") == NULL &&
	    (PObjPl = AttrGetObjectObjAttrib(PObj, "_Contours")) != NULL)
	    AttrSetObjectObjAttrib(PObjPolygons, "_Contours", 
				   IPCopyObject(NULL, PObjPl, FALSE), FALSE);

	if (AttrGetObjectObjAttrib(PObjPolygons, "_Isophotes") == NULL &&
	    (PObjPl = AttrGetObjectObjAttrib(PObj, "_Isophotes")) != NULL)
	    AttrSetObjectObjAttrib(PObjPolygons, "_Isophotes", 
				   IPCopyObject(NULL, PObjPl, FALSE), FALSE);

	IGDrawPolyFuncPtr(PObjPolygons);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a polygonal approximation for the given object with the        *
* prescribed fineness.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A surface(s) object.                                          *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polygonal approximation.                        *
*****************************************************************************/
static IPObjectStruct *IGGenSurfacePolygons(IPObjectStruct *PObj,
					    IrtRType FineNess)
{
    int OldVal;
    CagdSrfStruct *Srf,
        *Srfs = PObj -> U.Srfs;
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

    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
        int HasTexture = IGInitSrfTexture(PObj);
	IrtRType t;

	t = AttrGetObjectRealAttrib(PObj, "u_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&Srf -> Attr, "u_resolution", t);
	t = AttrGetObjectRealAttrib(PObj, "v_resolution");
	if (!IP_ATTR_IS_BAD_REAL(t))
	    AttrSetRealAttrib(&Srf -> Attr, "v_resolution", t);

	PPolygons = IPSurface2Polygons(Srf, IGGlblFourPerFlat,
				       RelativeFineNess * IGGlblPlgnFineness,
				       HasTexture || IGGlblAlwaysGenUV, TRUE,
				       IGGlblPolygonOptiApprox);

	if (HasTexture) {
	    CagdRType UMin, UMax, VMin, VMax,
		UScale = AttrGetObjectRealAttrib(PObj, "_ImageScaleX"),
		VScale = AttrGetObjectRealAttrib(PObj, "_ImageScaleY");
	    IPPolygonStruct *Poly;

	    /* Go over all polygons and set the proper UV domain. */
	    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

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

    if (PObjPolygons -> U.Pl != NULL &&
	IP_IS_STRIP_POLY(PObjPolygons -> U.Pl))
	IP_SET_POLYSTRIP_OBJ(PObjPolygons);

    return PObjPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates a sketch like drawing of the surface on the fly if needed      *
* and display it.                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A surface(s) object.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawSurfaceGenSketches(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjSketches;

    if ((PObjSketches = AttrGetObjectObjAttrib(PObj, "_sketches")) == NULL) {
	PObjSketches = IGGenSurfaceSketches(PObj, 1.0);

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
static IPObjectStruct *IGGenSurfaceSketches(IPObjectStruct *PObj,
					    IrtRType FineNess)
{
    IrtRType
        RelativeFineNess = AttrGetObjectRealAttrib(PObj, "res_sketch");
    IPObjectStruct *PPtsObj1, *PPtsObj2, *PPtsObj;

    if (IP_ATTR_IS_BAD_REAL(RelativeFineNess))
	RelativeFineNess = 1.0;

    PPtsObj1 = IGSketchGenSrfSketches(PObj -> U.Srfs,
				      RelativeFineNess * FineNess,
				      PObj, FALSE);
    if (PPtsObj1 -> U.Pl -> PVertex != NULL) {
        PPtsObj1 -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
	AttrSetObjectPtrAttrib(PPtsObj1, "_SphCones",
			       GMSphConeQueryInit(PPtsObj1));
    }

    PPtsObj2 = IGSketchGenSrfSketches(PObj -> U.Srfs,
				      RelativeFineNess * FineNess,
				      PObj, TRUE);

    PPtsObj = IPGenLISTObject(PPtsObj1);
    IPListObjectInsert(PPtsObj, 1, PPtsObj2);
    IPListObjectInsert(PPtsObj, 2, NULL);

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Generates the polyline approximation of the surface on the fly if        *
* needed and display it.                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      A surface(s) object.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawSurfaceGenPolylines(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjPolylines;

    if ((PObjPolylines = AttrGetObjectObjAttrib(PObj, "_IsolinesHiRes"))
								    == NULL) {
	if ((PObjPolylines = IGGenSurfacePolylines(PObj, 1.0)) == NULL)
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
	    PObjPolylines = IGGenSurfacePolylines(PObj,
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
*   PObj:      A surface(s) object.                                          *
*   FineNess:  Relative fineness to approximate PObj with.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:    The polyline approximation.                         *
*****************************************************************************/
static IPObjectStruct *IGGenSurfacePolylines(IPObjectStruct *PObj,
					     IrtRType FineNess)
{
    IrtRType RelativeFineNess;
    IPObjectStruct
	*PObjPolylines = NULL;
    IPPolygonStruct *PPolylines,  *PPolylineTemp;
    CagdSrfStruct *Srf,
        *Srfs = PObj -> U.Srfs;

    RelativeFineNess = IGGlblPolylineOptiApprox ?
				       1.0 / (FineNess + IRIT_EPS) : FineNess;

    PObjPolylines = IPAllocObject("", IP_OBJ_POLY, NULL);
    PObjPolylines -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolylines);
    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	int NumOfIso[2],
	    Res = (int) (IRIT_MAX(IGGlblNumOfIsolines * FineNess, 0));

	NumOfIso[0] = -Res;
	NumOfIso[1] = -Res;

	if (CAGD_NUM_OF_PT_COORD(Srf -> PType) < 2) {
	    CagdSrfStruct
		*TSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, TRUE);

	    PPolylines = IPSurface2Polylines(TSrf, NumOfIso,
					 IGGlblPllnFineness * RelativeFineNess,
					 IGGlblPolylineOptiApprox);
	    CagdSrfFree(TSrf);
	}
	else
	    PPolylines = IPSurface2Polylines(Srf, NumOfIso,
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
static void IGDrawSurfaceGenDiscontinuities(IPObjectStruct *PObj)
{
    IPObjectStruct *PObjDisconts;

    if ((PObjDisconts = AttrGetObjectObjAttrib(PObj, "_Disconts")) == NULL) {
        CagdCrvStruct
	    *DiscontCrvs = BspSrfC1DiscontCrvs(PObj -> U.Srfs);
	IPPolygonStruct *PPolyline;
	CagdCrvStruct *Crv;

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

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes and draw the reflection lines off the given surface as          *
* prescribed by the reflection lines in RflctLnObj.  RflctLnObj is assumed   *
* to hold a list of two elements, the lines' direction vector and a list of  *
* of points on the different lines to consider.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:         The surface object to compute and draw reflection lines    *
*		  for.                                                       *
*   LinesObj:     A list of a line direction and list of points, prescribing *
*		  the different lines to reflect off the surface object.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGDrawSurfaceRefLines(IPObjectStruct *PObj,
				  IPObjectStruct *LinesObj)
{
    IRIT_STATIC_DATA IrtPlnType
        Plane = { 1.0, 0.0, 0.0, 1.280791e-8 };
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    int i;
    CagdRType UMin, UMax, VMin, VMax;
    CagdVType ViewDir;
    IPObjectStruct *LinePt, *LinesPts, *LinesDir;
    CagdSrfStruct
	*Srf = PObj -> U.Srfs;

    /* Derive the view dir - inverse of view mat applied to +Z direction. */
    ViewDir[0] = ViewDir[1] = 0.0;
    ViewDir[2] = 1.0;
    MatMultVecby4by4(ViewDir, ViewDir, IGGlblInvCrntViewMat);
    IRIT_VEC_NORMALIZE(ViewDir);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    if (IP_IS_OLST_OBJ(LinesObj) &&
	(LinesDir = IPListObjectGet(LinesObj, 0)) != NULL &&
	IP_IS_VEC_OBJ(LinesDir) &&
	(LinesPts = IPListObjectGet(LinesObj, 1)) != NULL &&
	IP_IS_OLST_OBJ(LinesPts)) {
        for (i = 0; (LinePt = IPListObjectGet(LinesPts, i++)) != NULL; ) {
	    if (IP_IS_POINT_OBJ(LinePt)) {
	        IrtRType Resolution;
	        IPPolygonStruct *Pl, *PlPrev, *Cntrs;
	        IPObjectStruct
		    *PZeroObj = IPGenSRFObject(SymbRflctLnGen(Srf,
							      ViewDir,
							      LinePt -> U.Pt,
							      LinesDir -> U.Vec,
							      NULL));
		Resolution = IGGlblPlgnFineness;
		if (IGGlblManipulationActive)
		    Resolution *= IGGlblRelLowresFineNess;

		Cntrs = UserCntrSrfWithPlane(PZeroObj -> U.Srfs,
					     Plane, Resolution);

		IPFreeObject(PZeroObj);

		/* Filter out all unrelevant/redundant components. */
		for (Pl = PlPrev = Cntrs; Pl != NULL; ) {
		    IPVertexStruct *VPrev, *V;

		    for (V = VPrev = Pl -> PVertex; V != NULL; ) {
		        int Purge;
		        CagdRType t, t1, t2, *P;
			CagdPType PtE3, Pt1, Pt2;
			CagdVType RflctDir;
			CagdVecStruct *Nrml;

			V -> Coord[1] = IRIT_BOUND(V -> Coord[1], UMin, UMax);
			V -> Coord[2] = IRIT_BOUND(V -> Coord[2], VMin, VMax);

			P = CagdSrfEval(Srf, V -> Coord[1], V -> Coord[2]);
			CagdCoerceToE3(PtE3, &P, -1, Srf -> PType);

			Nrml = CagdSrfNormal(Srf, V -> Coord[1],
					          V -> Coord[2], TRUE);
			t = IRIT_DOT_PROD(Nrml -> Vec, ViewDir);
			if (IRIT_FABS(t) < RFLCT_LN_SIL_EPS)
			    Purge = TRUE;      /* Almost a silhouette point. */
			else {
			    /* Compute reflection direction... */
			    IRIT_VEC_COPY(RflctDir, Nrml -> Vec);
			    IRIT_VEC_SCALE(RflctDir, 2.0 * t);
			    IRIT_VEC_SUB(RflctDir, RflctDir, ViewDir);

			    /* Solve for intersection point of reflection   */
			    /* line and reflection direction off surface.   */
			    GM2PointsFromLineLine(PtE3, RflctDir,
						  LinePt -> U.Pt, 
						  LinesDir -> U.Vec,
						  Pt1, &t1, Pt2, &t2);

			    Purge = ((t < 0.0) ^ (t1 > 0.0));
			}

			if (Purge) {
			    if (V == Pl -> PVertex) {
			        Pl -> PVertex = V -> Pnext;
				IPFreeVertex(V);
				V = VPrev = Pl -> PVertex;
			    }
			    else {
			        /* Break the polyline there. */
			        VPrev -> Pnext = NULL;
				Pl -> Pnext = IPAllocPolygon(0, V -> Pnext,
							     Pl -> Pnext);
				IPFreeVertex(V);
				V = NULL;
			    }
			}
			else {
			    IRIT_PT_COPY(V -> Coord, PtE3);

			    VPrev = V;
			    V = V -> Pnext;
			}
		    }

		    if (Pl -> PVertex == NULL) {
		        /* We have a completely deleted polyline. */
		        if (Pl == Cntrs) {
			    Cntrs = Pl -> Pnext;
			    IPFreePolygon(Pl);
			    Pl = PlPrev = Cntrs;
			}
			else {
			    PlPrev -> Pnext = Pl -> Pnext;
			    IPFreePolygon(Pl);
			    Pl = PlPrev -> Pnext;
			}
		    }
		    else {
		        PlPrev = Pl;
		        Pl = Pl -> Pnext;
		    }
		}

		if (Cntrs != NULL) {
		    IPObjectStruct
		        *PCntrObj = IPGenPOLYLINEObject(Cntrs);

		    /* Draw the reflection lines. */
		    IP_SET_POLYLINE_OBJ(PCntrObj);
		    IGDrawPolyFuncPtr(PCntrObj);

		    IPFreeObject(PCntrObj);
		}
	    }
	}
    }

    SymbRflctLnFree(Srf, NULL);

    BspMultComputationMethod(OldInterpFlag);
}
