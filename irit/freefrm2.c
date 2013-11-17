/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Module to provide the required interfact for the cagd library for the    *
* free form surfaces and curves.					     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "program.h"
#include "allocate.h"
#include "attribut.h"
#include "objects.h"
#include "geom_lib.h"
#include "user_lib.h"
#include "ip_cnvrt.h"
#include "bool_lib.h"
#include "freeform.h"

IRIT_STATIC_DATA IPObjectStruct
    *GlblGeom2PolyObj = NULL;
IRIT_STATIC_DATA int
    GlblGeom2PolyNormals = FALSE,
    GlblGeom2PolyOptimal = FALSE;

static IPObjectStruct *ComputeCurveIsoLines(IPObjectStruct *PObj,
					    int Optimal);
static IPObjectStruct *ComputeSurfaceIsoLines(IPObjectStruct *PObj,
					      int Optimal);
static IPObjectStruct *ComputeTrimSrfIsoLines(IPObjectStruct *PObj,
					      int Optimal);
static IPObjectStruct *ComputeTrivarIsoLines(IPObjectStruct *PObj,
					     int Optimal);
static IPObjectStruct *ComputeTriSrfIsoLines(IPObjectStruct *PObj,
					     int Optimal);
static void Geometry2PolygonsAux(IPObjectStruct *Obj, IrtHmgnMatType Mat);
static void Geometry2PolylinesAux(IPObjectStruct *Obj, IrtHmgnMatType Mat);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to reverse a surface by flipping the U and V directions.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfObj:     Surface to reverse.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    Reversed surface. The normal of the reversed        M
*                        surface is flipped with respected to the original   M
*                        surface, SrfObj, by 180 degrees.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SurfaceReverse                                                           M
*****************************************************************************/
IPObjectStruct *SurfaceReverse(IPObjectStruct *SrfObj)
{
    IPObjectStruct
	*RetVal = NULL;

    if (IP_IS_SRF_OBJ(SrfObj)) {
	CagdSrfStruct
	    *RevSrf = CagdSrfReverse2(SrfObj -> U.Srfs);

	if (RevSrf != NULL)
	    RetVal = IPGenSRFObject(RevSrf);
    }
    else if (IP_IS_TRIMSRF_OBJ(SrfObj)) {
	TrimSrfStruct
	    *RevSrf = TrimSrfReverse2(SrfObj -> U.TrimSrfs);

	if (RevSrf != NULL)
	    RetVal = IPGenTRIMSRFObject(RevSrf);
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert curve(s) to a piecewise linear polyline approximation   *
* and convert its control polygon into polyline as well.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:       Curves to approximate using piecewise linear approximation.  *
*   Optimal:    Do we want an optimal sampling but expensive approach?       *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:   A polyline approximation to PObj.                    *
*****************************************************************************/
static IPObjectStruct *ComputeCurveIsoLines(IPObjectStruct *PObj, int Optimal)
{
    IrtRType
	Resolution = GetResolutionReal();
    IPObjectStruct *PObjPoly;
    CagdCrvStruct *Crv;

    if (!IP_IS_CRV_OBJ(PObj))
	IRIT_FATAL_ERROR("Curve was expected.");

    if (Optimal == 0 && Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;

    PObjPoly = IPGenPOLYObject(NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPoly -> Attr, PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPoly);
    for (Crv = PObj -> U.Crvs; Crv != NULL; Crv = Crv -> Pnext) {
	IPPolygonStruct *Poly;

	Poly = IPCurve2Polylines(Crv, Resolution,
				 (SymbCrvApproxMethodType) Optimal);

	PObjPoly -> U.Pl = IPAppendPolyLists(Poly, PObjPoly -> U.Pl);
    }

    return PObjPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a surface to a set of piecewise linear polyline	     *
* approximation and convert its control mesh into set of polyline as well.   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        Surfaces to apporximate as a mesh of piecewise linear       *
*		 isocurves.                                                  *
*   Optimal:     Do we want an optimal sampling but expensive approach?      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A polyline object approximating PObj.                 *
*****************************************************************************/
static IPObjectStruct *ComputeSurfaceIsoLines(IPObjectStruct *PObj,
					      int Optimal)
{
    IrtRType
	Resolution = GetResolutionReal();
    IPObjectStruct *PObjPolys;
    CagdSrfStruct *Srf;

    if (!IP_IS_SRF_OBJ(PObj))
	IRIT_FATAL_ERROR("Surface was expected.");

    if (Optimal == 0 && Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;

    PObjPolys = IPGenPOLYObject(NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPolys -> Attr, PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolys);
    for (Srf = PObj -> U.Srfs; Srf != NULL; Srf = Srf -> Pnext) {
	int NumOfIso[2];
	IPPolygonStruct *Polys, *PolyTmp;

	NumOfIso[0] = NumOfIso[1] = (int) -Resolution;
	Polys = IPSurface2Polylines(Srf, NumOfIso, Resolution,
				    (SymbCrvApproxMethodType) Optimal);

	for (PolyTmp = Polys;
	     PolyTmp -> Pnext != NULL;
	     PolyTmp = PolyTmp -> Pnext);
	PolyTmp -> Pnext = PObjPolys -> U.Pl;
	PObjPolys -> U.Pl = Polys;
    }

    return PObjPolys;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a trimmed surface to a set of piecewise linear polyline *
* approximation and convert its control mesh into set of polyline as well.   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        Trimmed surfaces to apporximate as a mesh of piecewise      *
*		  linear isocurves.                                          *
*   Optimal:     Do we want an optimal sampling but expensive approach?      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A polyline object approximating PObj.                 *
*****************************************************************************/
static IPObjectStruct *ComputeTrimSrfIsoLines(IPObjectStruct *PObj,
					      int Optimal)
{
    IrtRType
	Resolution = GetResolutionReal();
    IPObjectStruct *PObjPolys;
    TrimSrfStruct *TrimSrf;

    if (!IP_IS_TRIMSRF_OBJ(PObj))
	IRIT_FATAL_ERROR("Trimmed surface was expected.");

    if (Optimal == 0 && Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;

    PObjPolys = IPGenPOLYObject(NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPolys -> Attr, PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolys);
    for (TrimSrf = PObj -> U.TrimSrfs;
	 TrimSrf != NULL;
	 TrimSrf = TrimSrf -> Pnext) {
	int NumOfIso[2];
	IPPolygonStruct *Polys, *PolyTmp;

	NumOfIso[0] = NumOfIso[1] = (int) -Resolution;
	Polys = IPTrimSrf2Polylines(TrimSrf, NumOfIso, Resolution,
				    (SymbCrvApproxMethodType) Optimal,
				    TRUE, TRUE);

	for (PolyTmp = Polys;
	     PolyTmp -> Pnext != NULL;
	     PolyTmp = PolyTmp -> Pnext);
	PolyTmp -> Pnext = PObjPolys -> U.Pl;
	PObjPolys -> U.Pl = Polys;
    }

    return PObjPolys;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a trimmed surface to a set of piecewise linear polyline *
* approximation and convert its control mesh into set of polyline as well.   *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        Trimmed surfaces to apporximate as a mesh of piecewise      *
*		  linear isocurves.                                          *
*   Optimal:     Do we want an optimal sampling but expensive approach?      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A polyline object approximating PObj.                 *
*****************************************************************************/
static IPObjectStruct *ComputeTrivarIsoLines(IPObjectStruct *PObj,
					     int Optimal)
{
    IrtRType
	Resolution = GetResolutionReal();
    IPObjectStruct *PObjPolys;
    TrivTVStruct *Trivar;

    if (!IP_IS_TRIVAR_OBJ(PObj))
	IRIT_FATAL_ERROR("Trivariate function was expected.");

    if (Optimal == 0 && Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;

    PObjPolys = IPGenPOLYObject(NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPolys -> Attr, PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolys);
    for (Trivar = PObj -> U.Trivars;
	 Trivar != NULL;
	 Trivar = Trivar -> Pnext) {
	int NumOfIso[3],
	    Res = (int) IRIT_MAX(Resolution * 0.5, 2);
	IPPolygonStruct *Polys, *PolyTmp;

	NumOfIso[0] = NumOfIso[1] = NumOfIso[2] = -Res;
	Polys = IPTrivar2Polylines(Trivar, NumOfIso, Resolution,
				   (SymbCrvApproxMethodType) Optimal);

	for (PolyTmp = Polys;
	     PolyTmp -> Pnext != NULL;
	     PolyTmp = PolyTmp -> Pnext);
	PolyTmp -> Pnext = PObjPolys -> U.Pl;
	PObjPolys -> U.Pl = Polys;
    }

    return PObjPolys;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to convert a triangular surface to a set of piecewise linear	     *
* polyline approximation and convert its control mesh into set of polyline   *
* as well.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:        Triangular surfaces to apporximate as a mesh of piecewise   *
*		 linear isocurves.                                           *
*   Optimal:     Do we want an optimal sampling but expensive approach?      *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A polyline object approximating PObj.                 *
*****************************************************************************/
static IPObjectStruct *ComputeTriSrfIsoLines(IPObjectStruct *PObj,
					     int Optimal)
{
    IrtRType
	Resolution = GetResolutionReal();
    IPObjectStruct *PObjPolys;
    TrngTriangSrfStruct *TriSrf;

    if (!IP_IS_TRISRF_OBJ(PObj))
	IRIT_FATAL_ERROR("Triangular surface was expected.");

    if (Optimal == 0 && Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;

    PObjPolys = IPGenPOLYObject(NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPolys -> Attr, PObj -> Attr);
    IP_SET_POLYLINE_OBJ(PObjPolys);
    for (TriSrf = PObj -> U.TriSrfs;
	 TriSrf != NULL;
	 TriSrf = TriSrf -> Pnext) {
	int NumOfIso[3];
	IPPolygonStruct *Polys, *PolyTmp;

	NumOfIso[0] = NumOfIso[1] = NumOfIso[2] = (int) -Resolution;
	Polys = IPTriSrf2Polylines(TriSrf, NumOfIso, Resolution,
				   (SymbCrvApproxMethodType) Optimal);

	for (PolyTmp = Polys;
	     PolyTmp -> Pnext != NULL;
	     PolyTmp = PolyTmp -> Pnext);
	PolyTmp -> Pnext = PObjPolys -> U.Pl;
	PObjPolys -> U.Pl = Polys;
    }

    return PObjPolys;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a surface to a set of polygons approximating it.	     M
*   Result is saved as an attribute on the surface.			     M
*   If, however, approximation already exists, no computation is performed.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Surface to approximate using polygons.                        M
*   Normals:   Compute normals as well, if TRUE.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeSurfacePolygons                                                   M
*****************************************************************************/
void ComputeSurfacePolygons(IPObjectStruct *PObj, int Normals)
{
  int	OldOnlyTri, OldMergeCoplanarPolys,
	Resolution = GetResolution(FALSE),
	FourPerFlat = GetFourPerFlat(),
	PolyApproxOpt = GetPolyApproxOptimal(),
	PolyApproxUV = GetPolyApproxUV(),
	PolyApproxTri = GetPolyApproxTri(),
	PolyMergeCoplanarPolys = GetPolyMergeCoplanar(),
        BoolUVBooleanState = BoolSetParamSurfaceUVVals(TRUE);
    IrtRType t,
	RelResolution = AttrGetObjectRealAttrib(PObj, "resolution"),
	Tolerance = GetPolyApproxTol();
    IPPolygonStruct *Polys;
    IPObjectStruct *PObjPoly;

    BoolSetParamSurfaceUVVals(BoolUVBooleanState);/* Restore bool_lib state. */

    if (AttrGetObjectObjAttrib(PObj, "_polygons") != NULL)
	return;

    if (Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

#ifndef __MSDOS__
    /* Make the resolution more reasonable (very slow on MSDOS). */
    Resolution *= 2;
#endif /* __MSDOS__ */

    if (AttrGetObjectStrAttrib(PObj, "twoperflat") != NULL)
	FourPerFlat = FALSE;
    if (AttrGetObjectStrAttrib(PObj, "fourperflat") != NULL)
	FourPerFlat = TRUE;

    t = AttrGetObjectRealAttrib(PObj, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        AttrSetRealAttrib(&PObj -> U.Srfs -> Attr, "u_resolution", t);
    t = AttrGetObjectRealAttrib(PObj, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        AttrSetRealAttrib(&PObj -> U.Srfs -> Attr, "v_resolution", t);

    OldOnlyTri = IPSurface2PolygonsGenTriOnly(PolyApproxTri);
    CagdSrfSetMakeOnlyTri(PolyApproxTri);
    OldMergeCoplanarPolys = CagdSrf2PolygonMergeCoplanar(PolyMergeCoplanarPolys);

    Polys = IPSurface2Polygons(PObj -> U.Srfs, FourPerFlat,
			       PolyApproxOpt ? Tolerance : Resolution,
			       PolyApproxUV || BoolUVBooleanState,
			       Normals, PolyApproxOpt);

    IPSurface2PolygonsGenTriOnly(OldOnlyTri);
    CagdSrfSetMakeOnlyTri(OldOnlyTri);
    CagdSrf2PolygonMergeCoplanar(OldMergeCoplanarPolys);

    PObjPoly = IPGenPolyObject("", Polys, NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPoly -> Attr, PObj -> Attr);
    AttrSetObjectObjAttrib(PObj, "_polygons", PObjPoly, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a triangular surface to a set of polygons		     M
* approximating it.							     M
*   Result is saved as an attribute on the surface.			     M
*   If, however, approximation already exists, no computation is performed.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Triangular surface to approximate using polygons.             M
*   Normals:   Compute normals as well, if TRUE.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeTriSrfPolygons                                                    M
*****************************************************************************/
void ComputeTriSrfPolygons(IPObjectStruct *PObj, int Normals)
{
    int	OldOnlyTri,
	Resolution = GetResolution(FALSE),
	PolyApproxOpt = GetPolyApproxOptimal(),
	PolyApproxUV = GetPolyApproxUV(),
	PolyApproxTri = GetPolyApproxTri(),
        BoolUVBooleanState = BoolSetParamSurfaceUVVals(TRUE);
    IrtRType
        RelResolution = AttrGetObjectRealAttrib(PObj, "resolution");
    IPPolygonStruct *Polys;
    IPObjectStruct *PObjPoly;

    BoolSetParamSurfaceUVVals(BoolUVBooleanState);/* Restore bool_lib state. */

    if (AttrGetObjectObjAttrib(PObj, "_polygons") != NULL)
	return;

    if (Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

#ifndef __MSDOS__
    /* Make the resolution more reasonable (very slow on MSDOS). */
    Resolution *= 2;
#endif /* __MSDOS__ */

    OldOnlyTri = IPSurface2PolygonsGenTriOnly(PolyApproxTri);

    /* No optimal tesselation support so comment that out here: */
    Polys = IPTriSrf2Polygons(PObj -> U.TriSrfs,
			      /* PolyApproxOpt ? Tolerance : */ Resolution,
			      PolyApproxUV || BoolUVBooleanState,
			      Normals, PolyApproxOpt);

    CagdSrfSetMakeOnlyTri(OldOnlyTri);

    PObjPoly = IPGenPolyObject("", Polys, NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPoly -> Attr, PObj -> Attr);
    AttrSetObjectObjAttrib(PObj, "_polygons", PObjPoly, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a trimmed surface to a set of polygons approximating it.M
*   Result is saved as an attribute on the trimmed surface.		     M
*   If, however, approximation already exists, no computation is performed.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Trimmed surface to approximate using polygons.                M
*   Normals:   Compute normals as well, if TRUE.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeTrimSrfPolygons                                                   M
*****************************************************************************/
void ComputeTrimSrfPolygons(IPObjectStruct *PObj, int Normals)
{
    int	OldOnlyTri,
	Resolution = GetResolution(FALSE),
	FourPerFlat = GetFourPerFlat(),
	PolyApproxOpt = GetPolyApproxOptimal(),
	PolyApproxUV = GetPolyApproxUV(),
	PolyApproxTri = GetPolyApproxTri();
    IrtRType t,
	RelResolution = AttrGetObjectRealAttrib(PObj, "resolution"),
	Tolerance =  GetPolyApproxTol();
    IPPolygonStruct *Polys;
    IPObjectStruct *PObjPoly;

    if (AttrGetObjectObjAttrib(PObj, "_polygons") != NULL)
	return;

    if (Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

#ifndef __MSDOS__
    /* Make the resolution more reasonable (very slow on MSDOS). */
    Resolution *= 2;
#endif /* __MSDOS__ */

    if (AttrGetObjectStrAttrib(PObj, "twoperflat") != NULL)
	FourPerFlat = FALSE;
    if (AttrGetObjectStrAttrib(PObj, "fourperflat") != NULL)
	FourPerFlat = TRUE;

    t = AttrGetObjectRealAttrib(PObj, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        AttrSetRealAttrib(&PObj -> U.TrimSrfs -> Attr, "u_resolution", t);
    t = AttrGetObjectRealAttrib(PObj, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        AttrSetRealAttrib(&PObj -> U.TrimSrfs -> Attr, "v_resolution", t);

    OldOnlyTri = IPSurface2PolygonsGenTriOnly(PolyApproxTri);

    Polys = IPTrimSrf2Polygons(PObj -> U.TrimSrfs, FourPerFlat,
			       PolyApproxOpt ? Tolerance : Resolution,
			       PolyApproxUV, Normals, PolyApproxOpt);

    CagdSrfSetMakeOnlyTri(OldOnlyTri);

    PObjPoly = IPGenPolyObject("", Polys, NULL);
    IP_ATTR_SAFECOPY_ATTRS(PObjPoly -> Attr, PObj -> Attr);
    AttrSetObjectObjAttrib(PObj, "_polygons", PObjPoly, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a trivariate function to a set of polygons            M
* approximating it.							     M
*   Result is saved as an attribute on the trimmed surface.		     M
*   If, however, approximation already exists, no computation is performed.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Trivariate function surface to approximate using polygons.    M
*   Normals:   Compute normals as well, if TRUE.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ComputeTrivarPolygons                                                    M
*****************************************************************************/
void ComputeTrivarPolygons(IPObjectStruct *PObj, int Normals)
{
    int	OldOnlyTri,
	Resolution = GetResolution(FALSE),
	FourPerFlat = GetFourPerFlat(),
	PolyApproxOpt = GetPolyApproxOptimal(),
	PolyApproxUV = GetPolyApproxUV(),
	PolyApproxTri = GetPolyApproxTri();
    IrtRType t,
	RelResolution = AttrGetObjectRealAttrib(PObj, "resolution"),
	Tolerance =  GetPolyApproxTol();
    IPPolygonStruct *Polys;
    IPObjectStruct *PObjPoly;

    if (AttrGetObjectObjAttrib(PObj, "_polygons") != NULL)
	return;

    if (Resolution < MIN_FREE_FORM_RES)
	Resolution = MIN_FREE_FORM_RES;
    if (!IP_ATTR_IS_BAD_REAL(RelResolution))
	Resolution = IRIT_REAL_TO_INT(Resolution * RelResolution);

#ifndef __MSDOS__
    /* Make the resolution more reasonable (very slow on MSDOS). */
    Resolution *= 2;
#endif /* __MSDOS__ */

    if (AttrGetObjectStrAttrib(PObj, "twoperflat") != NULL)
	FourPerFlat = FALSE;
    if (AttrGetObjectStrAttrib(PObj, "fourperflat") != NULL)
	FourPerFlat = TRUE;

    t = AttrGetObjectRealAttrib(PObj, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        AttrSetRealAttrib(&PObj -> U.TrimSrfs -> Attr, "u_resolution", t);
    t = AttrGetObjectRealAttrib(PObj, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        AttrSetRealAttrib(&PObj -> U.TrimSrfs -> Attr, "v_resolution", t);

    OldOnlyTri = IPSurface2PolygonsGenTriOnly(PolyApproxTri);

    Polys = IPTrivar2Polygons(PObj -> U.Trivars, FourPerFlat,
			       PolyApproxOpt ? Tolerance : Resolution,
			       PolyApproxUV, Normals, PolyApproxOpt);

    CagdSrfSetMakeOnlyTri(OldOnlyTri);

    PObjPoly = IPGenPolyObject("", Polys, NULL);
    PObjPoly -> Attr = IP_ATTR_COPY_ATTRS(PObj -> Attr);
    AttrSetObjectObjAttrib(PObj, "_polygons", PObjPoly, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a surface/list of surfaces into set of polygons.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A geometry to convert and approximate using polygons.	     M
*   RNormals:   Do we want normals as well (at vertices)?                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A polygonal object approximating Obj.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   Geometry2Polygons                                                        M
*****************************************************************************/
IPObjectStruct *Geometry2Polygons(IPObjectStruct *Obj, IrtRType *RNormals)
{
    IrtHmgnMatType Mat;
    int OldTraverseCopyObjState = IPTraverseObjectCopy(TRUE);

    GlblGeom2PolyObj = NULL;
    GlblGeom2PolyNormals= IRIT_REAL_PTR_TO_INT(RNormals);

    MatGenUnitMat(Mat);

    IPTraverseObjHierarchy(Obj, Obj, Geometry2PolygonsAux, Mat, FALSE);

    IPTraverseObjectCopy(OldTraverseCopyObjState);

    return GlblGeom2PolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of Geometry2Polygons. Do the leaves.                  *
*****************************************************************************/
static void Geometry2PolygonsAux(IPObjectStruct *Obj, IrtHmgnMatType Mat)
{
    IPObjectStruct
	*RetVal = NULL;

    if (IP_IS_POLY_OBJ(Obj)) {
	if (IP_IS_POLYGON_OBJ(Obj))
	    RetVal = IPCopyObject(NULL, Obj, FALSE);
    }
    else if (IP_IS_SRF_OBJ(Obj) ||
	     IP_IS_TRISRF_OBJ(Obj) ||
	     IP_IS_TRIMSRF_OBJ(Obj)) {
	if (AttrGetObjectObjAttrib(Obj, "_polygons") != NULL)
	    AttrFreeOneAttribute(&Obj -> Attr, "_polygons");

	if (IP_IS_SRF_OBJ(Obj))
	    ComputeSurfacePolygons(Obj, GlblGeom2PolyNormals);
	else if (IP_IS_TRISRF_OBJ(Obj))
	    ComputeTriSrfPolygons(Obj, GlblGeom2PolyNormals);
	else if (IP_IS_TRIMSRF_OBJ(Obj))
	    ComputeTrimSrfPolygons(Obj, GlblGeom2PolyNormals);

	RetVal = IPCopyObject(NULL, AttrGetObjectObjAttrib(Obj, "_polygons"),
									FALSE);

	if (!GlblGeom2PolyNormals) {
	    IPPolygonStruct *Pl;

	    for (Pl = RetVal -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
		IPVertexStruct
		    *V = Pl -> PVertex;

		do {
		    IP_RST_NORMAL_VRTX(V);

		    V = V -> Pnext;
		}
		while (V != NULL && V != Pl -> PVertex);
	    }
	}
    }
    else if (IP_IS_TRIVAR_OBJ(Obj)) {
	if (AttrGetObjectObjAttrib(Obj, "_polygons") != NULL)
	    AttrFreeOneAttribute(&Obj -> Attr, "_polygons");

	ComputeTrivarPolygons(Obj, GlblGeom2PolyNormals);

	RetVal = IPCopyObject(NULL, AttrGetObjectObjAttrib(Obj, "_polygons"),
									FALSE);

	if (!GlblGeom2PolyNormals) {
	    IPPolygonStruct *Pl;

	    for (Pl = RetVal -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
		IPVertexStruct
		    *V = Pl -> PVertex;

		do {
		    IP_RST_NORMAL_VRTX(V);

		    V = V -> Pnext;
		}
		while (V != NULL && V != Pl -> PVertex);
	    }
	}
    }
    else {
	IRIT_WNDW_PUT_STR("Unconvertable to polygons object ignored");
	return;
    }

    if (RetVal != NULL && RetVal -> U.Pl != NULL) {
        IPObjectStruct
	    *PObjTmp = GMTransformObject(RetVal, Mat);

	IPFreeObject(RetVal);
	RetVal = PObjTmp;

	if (GlblGeom2PolyObj != NULL) {
	    IPGetLastPoly(RetVal -> U.Pl) -> Pnext = GlblGeom2PolyObj -> U.Pl;
	    GlblGeom2PolyObj -> U.Pl = RetVal -> U.Pl;
	    RetVal -> U.Pl = NULL;
	    IPFreeObject(RetVal);
	}
	else {
	    GlblGeom2PolyObj = RetVal;
	    IP_ATTR_FREE_ATTRS(GlblGeom2PolyObj -> Attr);
	}
    }
    else if (RetVal != NULL)
	IPFreeObject(RetVal);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a surface/curve/list of these into set of polylines.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A geometry to convert and approximate using polylines.	     M
*   Optimal:   Do we want an optimal sampling but expensive approach?        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A polyline object approximating Obj.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   Geometry2Polylines                                                       M
*****************************************************************************/
IPObjectStruct *Geometry2Polylines(IPObjectStruct *Obj, IrtRType *Optimal)
{
    IrtHmgnMatType Mat;
    int OldTraverseCopyObjState = IPTraverseObjectCopy(TRUE);

    GlblGeom2PolyObj = NULL;
    GlblGeom2PolyOptimal = IRIT_REAL_PTR_TO_INT(Optimal) ?
			SYMB_CRV_APPROX_TOLERANCE : SYMB_CRV_APPROX_UNIFORM;

    MatGenUnitMat(Mat);

    IPTraverseObjHierarchy(Obj, Obj, Geometry2PolylinesAux, Mat, FALSE);

    IPTraverseObjectCopy(OldTraverseCopyObjState);

    return GlblGeom2PolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of Geometry2Polylines. Do the leaves.                 *
*****************************************************************************/
static void Geometry2PolylinesAux(IPObjectStruct *Obj, IrtHmgnMatType Mat)
{
    IPObjectStruct
	*RetVal = NULL;

    if (IP_IS_POLY_OBJ(Obj)) {
	if (IP_IS_POLYGON_OBJ(Obj)) {
	    RetVal = IPCopyObject(NULL, Obj, FALSE);
	    IP_SET_POLYLINE_OBJ(RetVal);
	}
	else
	    RetVal = IPCopyObject(NULL, Obj, FALSE);
    }
    else if (IP_IS_CRV_OBJ(Obj)) {
	RetVal = ComputeCurveIsoLines(Obj, GlblGeom2PolyOptimal);
    }
    else if (IP_IS_SRF_OBJ(Obj)) {
	RetVal = ComputeSurfaceIsoLines(Obj, GlblGeom2PolyOptimal);
    }
    else if (IP_IS_TRIMSRF_OBJ(Obj)) {
	RetVal = ComputeTrimSrfIsoLines(Obj, GlblGeom2PolyOptimal);
    }
    else if (IP_IS_TRIVAR_OBJ(Obj)) {
	RetVal = ComputeTrivarIsoLines(Obj, GlblGeom2PolyOptimal);
    }
    else if (IP_IS_TRISRF_OBJ(Obj)) {
	RetVal = ComputeTriSrfIsoLines(Obj, GlblGeom2PolyOptimal);
    }
    else {
	IRIT_WNDW_PUT_STR("Unconvertable to polylines object ignored");
    }

    if (RetVal != NULL) {
        IPObjectStruct
	    *PObjTmp = GMTransformObject(RetVal, Mat);

	IPFreeObject(RetVal);
	RetVal = PObjTmp;

	if (GlblGeom2PolyObj != NULL) {
	    IPGetLastPoly(RetVal -> U.Pl) -> Pnext = GlblGeom2PolyObj -> U.Pl;
	    GlblGeom2PolyObj -> U.Pl = RetVal -> U.Pl;
	    RetVal -> U.Pl = NULL;
	    IPFreeObject(RetVal);
	}
	else {
	    GlblGeom2PolyObj = RetVal;
	    IP_ATTR_FREE_ATTRS(GlblGeom2PolyObj -> Attr);
	}
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to convert a surface/curve/list of these into set of pointlist.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:       A geometry to convert and approximate using points.	     M
*   Optimal:   Do we want an optimal sampling but expensive approach?        M
*   Merge:     TRUE to merge all points into one large point list.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A polyline object approximating Obj.                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   Geometry2Polylines                                                       M
*****************************************************************************/
IPObjectStruct *Geometry2PointList(IPObjectStruct *Obj,
				   IrtRType *Optimal,
				   IrtRType *Merge)
{
    IPObjectStruct
        *OPts = Geometry2Polylines(Obj, Merge);
    
    IP_SET_POINTLIST_OBJ(OPts);

    if (IRIT_REAL_PTR_TO_INT(Merge)) {
        IPPolygonStruct *TPts,
	    *Pts = OPts -> U.Pl;

        while (Pts -> Pnext != NULL) {
	    TPts = Pts -> Pnext;

	    /* Chain the point lists. */
	    IPGetLastVrtx(TPts -> PVertex) -> Pnext = Pts -> PVertex;
	    Pts -> PVertex = TPts -> PVertex;
	    TPts -> PVertex = NULL;

	    /* Remove the next poly. */
	    Pts -> Pnext = TPts -> Pnext;
	    IPFreePolygon(TPts);
	}
    }

    return OPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to return extremum value of control mesh/polygon.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:      Object to bound itsextremum possible values.                   M
*   Min:      TRUE for minimum, FALSE for maximum.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A control points with extremum values. Rational are  M
*                       Correctly porjected back to Euclidean space.         M
*                                                                            *
* KEYWORDS:                                                                  M
*   ExtremumControlPointVals                                                 M
*****************************************************************************/
IPObjectStruct *ExtremumControlPointVals(IPObjectStruct *Obj, CagdRType *Min)
{
    CagdBType
	FindMin = IRIT_REAL_PTR_TO_INT(Min);
    CagdRType *Extremum,
	**Points = NULL;
    int Len = 0;
    CagdPointType
	PType  = CAGD_PT_E1_TYPE;

    if (IP_IS_SRF_OBJ(Obj)) {
	Points = Obj -> U.Srfs -> Points;
	Len = Obj -> U.Srfs -> ULength * Obj -> U.Srfs -> VLength;
	PType = Obj -> U.Srfs -> PType;
    }
    else if (IP_IS_CRV_OBJ(Obj)) {
	Points = Obj -> U.Crvs -> Points;
	Len = Obj -> U.Crvs -> Length;
	PType = Obj -> U.Crvs -> PType;
    }
    else {
	IRIT_NON_FATAL_ERROR("Extremum allowed on curves/surfaces only");
	return NULL;
    }

    Extremum = SymbExtremumCntPtVals(Points, Len, FindMin);

    return IPGenCTLPTObject(PType, Extremum);
}
 
/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an exact rational quadratic circle parallel to the XY plane.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Position:   Center of circle.                                            M
*   Radius:     Radius of circle.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A rational quadratic Bspline curve object 	     M
*			representing a circle.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenCircleCurveObject                                                     M
*****************************************************************************/
IPObjectStruct *GenCircleCurveObject(IrtVecType Position, IrtRType *Radius)
{
    int i;
    CagdPtStruct Pos;
    CagdCrvStruct *CircCrv;
    IPObjectStruct *CrvObj;

    for (i = 0; i < 3; i++)
	Pos.Pt[i] = Position[i];
    CircCrv = BspCrvCreateCircle(&Pos, *Radius);

    if (CircCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(CircCrv);

    return CrvObj;
}
 
/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an approximated cubic polynomial circle parallel to the XY plane.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Position:   Center of circle.                                            M
*   Radius:     Radius of circle.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A cubic polynomial Bspline curve object 	     M
*			representing a circle.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenPCircleCurveObject                                                    M
*****************************************************************************/
IPObjectStruct *GenPCircleCurveObject(IrtVecType Position, IrtRType *Radius)
{
    int i;
    CagdPtStruct Pos;
    CagdCrvStruct *CircCrv;
    IPObjectStruct *CrvObj;

    for (i = 0; i < 3; i++)
	Pos.Pt[i] = Position[i];
    CircCrv = BspCrvCreatePCircle(&Pos, *Radius);

    if (CircCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(CircCrv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an approximated spiral curve.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfLoops:    Number of loops in the spiral - can be fractional.        M
*   Pitch:         Essentially the size of the spiral.  A Pitch of one will  M
*		   construct a roughly size-one spiral curve.		     M
*   RSampling:     Number of samples to compute on the spiral.  Should be    M
*		   several hundreds for a reasonable result.		     M
*   RCtlPtsPerLoop: Number of control points to use per loop. Use at least 5 M
*                  for a reasonable approximation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A cubic polynomial Bspline curve object 	     M
*			representing a spiral.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateApproxSpiral                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSpiralCurveObject                                                     M
*****************************************************************************/
IPObjectStruct *GenSpiralCurveObject(CagdRType *NumOfLoops,
				     CagdRType *Pitch,
				     CagdRType *RSampling,
				     CagdRType *RCtlPtsPerLoop)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
	*SpiralCrv = BspCrvCreateApproxSpiral(*NumOfLoops, *Pitch,
					      IRIT_REAL_PTR_TO_INT(RSampling),
					      IRIT_REAL_PTR_TO_INT(RCtlPtsPerLoop));

    if (SpiralCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(SpiralCrv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an approximated helix curve.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfLoops:    Number of loops in the helix - can be fractional.         M
*   Pitch:         Essentially the size of the helix.  A Pitch of one will   M
*		   construct a roughly size-one helix curve.		     M
*   Radius:        Radius of helix.  If radius is negative, the radius will  M
*		   change monotonically from zero to abs(Radius) at the end. M
*   RSampling:     Number of samples to compute on the helix.  Should be     M
*		   several hundreds for a reasonable result.		     M
*   RCtlPtsPerLoop: Number of control points to use per loop. Use at least 5 M
*                  for a reasonable approximation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A cubic polynomial Bspline curve object 	     M
*			representing a helix.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateApproxHelix                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenHelixCurveObject                                                      M
*****************************************************************************/
IPObjectStruct *GenHelixCurveObject(CagdRType *NumOfLoops,
				    CagdRType *Pitch,
				    CagdRType *Radius,
				    CagdRType *RSampling,
				    CagdRType *RCtlPtsPerLoop)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
        *HelixCrv = BspCrvCreateApproxHelix(*NumOfLoops, *Pitch, *Radius,
					    IRIT_REAL_PTR_TO_INT(RSampling),
					    IRIT_REAL_PTR_TO_INT(RCtlPtsPerLoop));

    if (HelixCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(HelixCrv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an approximated sine curve.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfCycles:     Number of cycles in the sine - can be fractional.       M
*   RSampling:       Number of samples to compute on the sine.  Should be    M
*		     several hundreds for a reasonable result.		     M
*   RCtlPtsPerCycle: Number of control points to use per cycle. Use at least M
*                    5 for a reasonable approximation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A cubic polynomial Bspline curve object 	     M
*			representing a sine wave.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvCreateApproxSine                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSineCurveObject                                                       M
*****************************************************************************/
IPObjectStruct *GenSineCurveObject(CagdRType *NumOfCycles,
				   CagdRType *RSampling,
				   CagdRType *RCtlPtsPerCycle)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
	*SineCrv = BspCrvCreateApproxSine(*NumOfCycles,
					  IRIT_REAL_PTR_TO_INT(RSampling),
					  IRIT_REAL_PTR_TO_INT(RCtlPtsPerCycle));

    if (SineCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(SineCrv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an arbitrary arc specified by Two end points and Center. Arc must  M
* be less than 180 degree.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Start:    Location of arc.                                               M
*   Center:   Location of arc.                                               M
*   End:      Location of arc.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A curve representing the requested arc.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenArcCurveObject                                                        M
*****************************************************************************/
IPObjectStruct *GenArcCurveObject(IrtVecType Start,
				  IrtVecType Center,
				  IrtVecType End)
{
    int i;
    CagdPtStruct StartPt, CenterPt, EndPt;
    CagdCrvStruct *ArcCrv;
    IPObjectStruct *CrvObj;

    for (i = 0; i < 3; i++) {
	StartPt.Pt[i] = Start[i];
	CenterPt.Pt[i] = Center[i];
	EndPt.Pt[i] = End[i];
    }
    ArcCrv = BzrCrvCreateArc(&StartPt, &CenterPt, &EndPt);

    if (ArcCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(ArcCrv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Creates an arbitrary arc specified by Two end points and Center. Arc must  M
* be less than 360 degree.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Center:      Location of center of arc.                                  M
*   Radius:      Of arc.                                                     M
*   StartAngle:  Of arc.                                                     M
*   EndAngle:    Of arc.                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A curve representing the requested arc.               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenArc2CurveObject                                                       M
*****************************************************************************/
IPObjectStruct *GenArc2CurveObject(IrtVecType Center,
				   IrtRType *Radius,
				   IrtRType *StartAngle,
				   IrtRType *EndAngle)
{
    int i;
    CagdPtStruct CenterPt;
    CagdCrvStruct *ArcCrv;
    IPObjectStruct *CrvObj;

    for (i = 0; i < 3; i++)
	CenterPt.Pt[i] = Center[i];

    ArcCrv = CagdCrvCreateArc(&CenterPt, *Radius, *StartAngle, *EndAngle);

    if (ArcCrv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(ArcCrv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a ruled surface out of the two provided curves.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj1, Obj2:  Two polys/curves to rule a surface between.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A ruled surface object.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenRuledSrfObject                                                        M
*****************************************************************************/
IPObjectStruct *GenRuledSrfObject(IPObjectStruct *Obj1, IPObjectStruct *Obj2)
{
    IPObjectStruct
	*SrfObj = NULL;

    if (IP_IS_POLY_OBJ(Obj1) && IP_IS_POLY_OBJ(Obj2)) {
	SrfObj = PrimGenRULEDObject(Obj1, Obj2);
    }
    else if (IP_IS_CRV_OBJ(Obj1) && IP_IS_CRV_OBJ(Obj2)) {
	CagdSrfStruct
	    *RuledSrf = CagdRuledSrf(Obj1 -> U.Crvs, Obj2 -> U.Crvs, 2, 2);

	if (RuledSrf == NULL)
	    return NULL;

	SrfObj = IPGenSRFObject(RuledSrf);
    }
    else {
	IRIT_FATAL_ERROR("RuledSrf: improper input geometry to ruled surface!");
    }

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a ruled trivariate out of the two provided surfaces.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj1, Obj2:  Two surfaces to rule into a trivariate between.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A ruled trivariate object.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenRuledTVObject                                                         M
*****************************************************************************/
IPObjectStruct *GenRuledTVObject(IPObjectStruct *Obj1, IPObjectStruct *Obj2)
{
    IPObjectStruct
	*TVObj = NULL;

    if (IP_IS_SRF_OBJ(Obj1) && IP_IS_SRF_OBJ(Obj2)) {
	TrivTVStruct
	    *RuledTV = TrivRuledTV(Obj1 -> U.Srfs, Obj2 -> U.Srfs, 2, 2);

	if (RuledTV == NULL)
	    return NULL;

	TVObj = IPGenTRIVARObject(RuledTV);
    }
    else {
	IRIT_FATAL_ERROR("RuledTV: improper input geometry to ruled trivariate!");
    }

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a boolean sum surface out of a single boundary curve.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   BndryCrv:  A curve to fill its interior with a surface. Better be close. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A filling surface object.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBoolOneSrfObject                                                      M
*****************************************************************************/
IPObjectStruct *GenBoolOneSrfObject(IPObjectStruct *BndryCrv)
{
    IPObjectStruct *SrfObj;
    CagdSrfStruct
	*BoolSumSrf = CagdOneBoolSumSrf(BndryCrv -> U.Crvs);

    if (BoolSumSrf == NULL)
	return NULL;

    SrfObj = IPGenSRFObject(BoolSumSrf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a boolean sum surface out of the four provided curves.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2, Crv3, Crv4:  Four curves to construct a Boolean sum between. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The Boolean sum surface.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenBoolSumSrfObject                                                      M
*****************************************************************************/
IPObjectStruct *GenBoolSumSrfObject(IPObjectStruct *Crv1,
				    IPObjectStruct *Crv2,
				    IPObjectStruct *Crv3,
				    IPObjectStruct *Crv4)
{
    IPObjectStruct *SrfObj;
    CagdSrfStruct
	*BoolSumSrf = CagdBoolSumSrf(Crv1 -> U.Crvs,
				     Crv2 -> U.Crvs,
				     Crv3 -> U.Crvs,
				     Crv4 -> U.Crvs);

    if (BoolSumSrf == NULL)
	return NULL;

    SrfObj = IPGenSRFObject(BoolSumSrf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a boolean sum volume out of a single boundary surface.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   BndryCrv:  A surface to fill its interior with a volumetric trivariate.  M
               surface should NOT have the two caps.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: A filling volume object.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTrivBoolOneSrfObject                                                  M
*****************************************************************************/
IPObjectStruct *GenTrivBoolOneSrfObject(IPObjectStruct *BndrySrf)
{
    IPObjectStruct *TVObj;
    TrivTVStruct
	*BoolSumTV = MvarTrivarBoolOne(BndrySrf -> U.Srfs);

    if (BoolSumTV == NULL)
	return NULL;

    TVObj = IPGenTRIVARObject(BoolSumTV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a boolean sum volume out of the six provided surfaces.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2, Srf3, Srf4, Srf5, Srf6:  Six surfaces curves to construct a  M
* Boolean sum volume.  Srf1-4 are thesides and Srf5 and Srf6 are the caps.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The Boolean sum surface.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenTrivBoolSumSrfObject                                                  M
*****************************************************************************/
IPObjectStruct *GenTrivBoolSumSrfObject(IPObjectStruct *Srf1,
					IPObjectStruct *Srf2,
					IPObjectStruct *Srf3,
					IPObjectStruct *Srf4,
					IPObjectStruct *Srf5,
					IPObjectStruct *Srf6)
{
    IPObjectStruct *TVObj;
    TrivTVStruct
	*BoolSumTV = MvarTrivarBoolSum(Srf1 -> U.Srfs,
				       Srf2 -> U.Srfs,
				       Srf3 -> U.Srfs,
				       Srf4 -> U.Srfs,
				       IP_IS_SRF_OBJ(Srf5)
				           ? Srf5 -> U.Srfs : NULL,
				       IP_IS_SRF_OBJ(Srf6)
				           ? Srf6 -> U.Srfs : NULL);

    if (BoolSumTV == NULL)
	return NULL;

    TVObj = IPGenTRIVARObject(BoolSumTV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a surface out of the provided curve list.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:     A list of curves to approximate a surface through.          M
*   OtherOrder:  Other order of surface.                                     M
*   OtherEC:	 End conditions of other direction.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A surface approximately traversing through the given M
*                       curves.                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSrfFromCrvsObject                                                     M
*****************************************************************************/
IPObjectStruct *GenSrfFromCrvsObject(IPObjectStruct *CrvList,
				     IrtRType *OtherOrder,
				     IrtRType *OtherEC)
{
    int i,
	NumCrvs = 0;
    IPObjectStruct *SrfObj, *CrvObj;
    CagdSrfStruct *Srf;
    CagdCrvStruct *Crv,
	*Crvs = NULL;

    if (!IP_IS_OLST_OBJ(CrvList))
	IRIT_FATAL_ERROR("SFROMCRVS: Not object list object!");

    while ((CrvObj = IPListObjectGet(CrvList, NumCrvs)) != NULL) {
	if (!IP_IS_CRV_OBJ(CrvObj)) {
	    IRIT_NON_FATAL_ERROR("SFROMCRVS: List contains non curve object(s).");
	    return NULL;
	}
	if (CrvObj -> U.Crvs -> Pnext != NULL) {
	    IRIT_NON_FATAL_ERROR("SFROMCRVS: nested curve lists are disallowed.");
	    return NULL;
	}
	NumCrvs++;
    }

    /* Chain all curves into a single list and invoke the srf constructor: */
    for (i = 0; i < NumCrvs; i++) {
	CrvObj = IPListObjectGet(CrvList, i);
	Crv = CagdCrvCopy(CrvObj -> U.Crvs);
	IRIT_LIST_PUSH(Crv, Crvs);
    }

    Crvs = CagdListReverse(Crvs);
    switch (IRIT_REAL_PTR_TO_INT(OtherEC)) {
	case KV_UNIFORM_OPEN:
	    Srf = CagdSrfFromCrvs(Crvs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				  CAGD_END_COND_OPEN, NULL);
	    break;
	case KV_UNIFORM_FLOAT:
	    Srf = CagdSrfFromCrvs(Crvs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				  CAGD_END_COND_FLOAT, NULL);
	    break;
	case KV_UNIFORM_PERIODIC:
	    Srf = CagdSrfFromCrvs(Crvs, IRIT_REAL_PTR_TO_INT(OtherOrder),
				  CAGD_END_COND_PERIODIC, NULL);
	    break;
	default:
	    assert(0);
	    Srf = NULL;
	    break;
    }

    CagdCrvFreeList(Crvs);

    if (Srf == NULL)
	return NULL;

    SrfObj = IPGenSRFObject(Srf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Construct a surface interpolating the provided curve list.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:     A list of curves to interpolate a surface through.          M
*   OtherOrder:  Other order of surface.                                     M
*   OtherEC:	 End conditions of other direction.                          M
*   OtherParam:  Parametrization type in other direction, i.e. chord length  M
*                or uniform, etc.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A surface traversing through the given curves.       M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSrfInterpolateCrvsObject                                              M
*****************************************************************************/
IPObjectStruct *GenSrfInterpolateCrvsObject(IPObjectStruct *CrvList,
					    IrtRType *OtherOrder,
					    IrtRType *OtherEC,
					    IrtRType *OtherParam)
{
    int i,
	NumCrvs = 0;
    IPObjectStruct *SrfObj, *CrvObj;
    CagdSrfStruct *Srf;
    CagdCrvStruct *Crv,
	*Crvs = NULL;

    if (!IP_IS_OLST_OBJ(CrvList))
	IRIT_FATAL_ERROR("SINTRPCRVS: Not object list object!");

    while ((CrvObj = IPListObjectGet(CrvList, NumCrvs)) != NULL) {
	if (!IP_IS_CRV_OBJ(CrvObj)) {
	    IRIT_NON_FATAL_ERROR("SINTRPCRVS: List contains non curve object(s).");
	    return NULL;
	}
	if (CrvObj -> U.Crvs -> Pnext != NULL) {
	    IRIT_NON_FATAL_ERROR("SINTRPCRVS: nested curve lists are disallowed.");
	    return NULL;
	}
	NumCrvs++;
    }

    /* Chain all curves into a single list and invoke the srf constructor: */
    for (i = 0; i < NumCrvs; i++) {
	CrvObj = IPListObjectGet(CrvList, i);
	Crv = CagdCrvCopy(CrvObj -> U.Crvs);
	IRIT_LIST_PUSH(Crv, Crvs);
    }

    Crvs = CagdListReverse(Crvs);
    switch (IRIT_REAL_PTR_TO_INT(OtherEC)) {
	case KV_UNIFORM_OPEN:
	    Srf = CagdSrfInterpolateCrvs(Crvs,
					 IRIT_REAL_PTR_TO_INT(OtherOrder),
					 CAGD_END_COND_OPEN,
					 IRIT_REAL_PTR_TO_INT(OtherParam),
					 NULL);
	    break;
	case KV_UNIFORM_FLOAT:
	    Srf = CagdSrfInterpolateCrvs(Crvs,
					 IRIT_REAL_PTR_TO_INT(OtherOrder),
					 CAGD_END_COND_FLOAT,
					 IRIT_REAL_PTR_TO_INT(OtherParam),
					 NULL);
	    break;
	case KV_UNIFORM_PERIODIC:
	    Srf = CagdSrfInterpolateCrvs(Crvs,
					 IRIT_REAL_PTR_TO_INT(OtherOrder),
					 CAGD_END_COND_PERIODIC,
					 IRIT_REAL_PTR_TO_INT(OtherParam),
					 NULL);
	    break;
	default:
	    assert(0);
	    Srf = NULL;
	    break;
    }

    CagdCrvFreeList(Crvs);

    if (Srf == NULL)
	return NULL;

    SrfObj = IPGenSRFObject(Srf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a Sweep surface out of the CrossSection curve, Axis curve and   M
* optional Scaling curve and Scaler which scales the CrossSection.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrossSection:  Cross section(s) of sweep.                                M
*   Axis:          Axis curve of sweep.                                      M
*   Frame:         Orientation specification. Either an orientation curve    M
*		   Or a vector specification.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A sweep surface.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSweepSrfObject                                                        M
*****************************************************************************/
IPObjectStruct *GenSweepSrfObject(IPObjectStruct *CrossSection,
				  IPObjectStruct *Axis,
				  IPObjectStruct *Frame)
{
    IrtRType
	R = 0.0;

    return GenSweepScaleSrfObject(CrossSection, Axis, NULL, Frame, &R);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a Sweep surface out of the CrossSection curve, Axis curve and   M
* optional Scaling curve and Scaler which scales the CrossSection.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrossSection:  Cross section of sweep.                                   M
*   Axis:          Axis curve of sweep.                                      M
*   Scale:         Either a numeric constant scale, or a scaling scalar      M
*		   curve, or NULL for no scaling.			     M
*   Frame:         Orientation specification. Either an orientation curve    M
*		   Or a vector specification.				     M
*   RRefine:       Refinement control.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A sweep surface.                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenSweepScaleSrfObject                                                   M
*****************************************************************************/
IPObjectStruct *GenSweepScaleSrfObject(IPObjectStruct *CrossSection,
				       IPObjectStruct *Axis,
				       IPObjectStruct *Scale,
				       IPObjectStruct *Frame,
				       IrtRType *RRefine)
{
    int FrameIsCrv = IP_IS_CRV_OBJ(Frame),
	HasFrame = FrameIsCrv || IP_IS_VEC_OBJ(Frame),
	Refine = IRIT_REAL_PTR_TO_INT(RRefine);
    CagdCrvStruct
	*AxisCrv = Axis -> U.Crvs,
	*CrossSecCrvs = NULL;
    CagdSrfStruct *SweepSrf;

    if (IP_IS_OLST_OBJ(CrossSection)) {
	int i = 0;
	IPObjectStruct *CrossSecObj;
	CagdCrvStruct *TCrv;

	/* Extract all the cross sections into a linked list. */
        while ((CrossSecObj = IPListObjectGet(CrossSection, i++)) != NULL) {
	    if (!IP_IS_CRV_OBJ(CrossSecObj)) {
		IRIT_NON_FATAL_ERROR("SWEEPSRF: Cross section list contains non curve object(s).");
		return NULL;
	    }
	    TCrv = CagdCrvCopy(CrossSecObj -> U.Crvs);
	    IRIT_LIST_PUSH(TCrv, CrossSecCrvs);
	}

	CrossSecCrvs = (CagdCrvStruct *) CagdListReverse(CrossSecCrvs);
    }
    else
	CrossSecCrvs = CagdCrvCopy(CrossSection -> U.Crvs);

    if (Refine > 0) {
	CagdCrvStruct
	    *TCrv = CagdSweepAxisRefine(AxisCrv, NULL, Refine);

	AxisCrv = TCrv;
    }

    SweepSrf = CagdSweepSrf(CrossSecCrvs, AxisCrv,
			    (Scale != NULL &&
			        IP_IS_CRV_OBJ(Scale)) ? Scale -> U.Crvs : NULL,
			    (Scale != NULL &&
			        IP_IS_NUM_OBJ(Scale)) ? Scale -> U.R : 1.0,
			    HasFrame ? (FrameIsCrv
					        ? (VoidPtr) Frame -> U.Crvs
					        : (VoidPtr) Frame -> U.Vec)
				     : NULL,
			    FrameIsCrv);

    CagdCrvFreeList(CrossSecCrvs);

    if (AxisCrv != Axis -> U.Crvs)
	CagdCrvFree(AxisCrv);

    return SweepSrf == NULL ? NULL : IPGenSRFObject(SweepSrf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an approximation to the offset of a (planar) curve or a (trimmed) M
* surface, or a single poly, or a polygonal model.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:        Object to approximate its offset by Offset amount.           M
*   Offset:     Amount of offset, either a number or a scalar distance crv.  M
*   Tolerance:  Accuracy of offset.  Forces smoothing in case of polygonal   M
*               offset, if not zero.                                         M
*   BezInterp:  Do we want Bezier interpolation or approximation?  Attempts  M
*		miter joints in polygonal offset in R^3, if TRUE.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An offset approximating to Obj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenOffsetObject                                                          M
*****************************************************************************/
IPObjectStruct *GenOffsetObject(IPObjectStruct *Obj,
				IPObjectStruct *Offset,
				IrtRType *Tolerance,
				IrtRType *BezInterp)
{
    if (IP_IS_POLY_OBJ(Obj)) {
	IPPolygonStruct
	    *Pl = Obj -> U.Pl -> Pnext == NULL ?  /* A single polygon/line? */
	        GMPolyOffset(Obj -> U.Pl, IP_IS_POLYGON_OBJ(Obj),
			     Offset -> U.R, NULL) :
	        GMPolyOffset3D(Obj -> U.Pl, Offset -> U.R,
			       IRIT_REAL_PTR_TO_INT(Tolerance),
			       IRIT_REAL_PTR_TO_INT(BezInterp), NULL);
	IPObjectStruct 
	    *PlObj = IPGenPOLYObject(Pl);

	if (IP_IS_POLYGON_OBJ(Obj))
	    IP_SET_POLYGON_OBJ(PlObj);
	else
	    IP_SET_POLYLINE_OBJ(PlObj);

	return PlObj;
    }
    else if (IP_IS_SRF_OBJ(Obj)) {
    	IPObjectStruct *SrfObj;
    	CagdSrfStruct
	    *OffsetSrf = SymbSrfSubdivOffset(Obj -> U.Srfs, Offset -> U.R,
					     *Tolerance);

    	if (OffsetSrf == NULL)
	    return NULL;

    	SrfObj = IPGenSRFObject(OffsetSrf);

    	return SrfObj;
    }
    else if (IP_IS_TRIMSRF_OBJ(Obj)) {
    	IPObjectStruct *TSrfObj;
    	CagdSrfStruct
	    *OffsetSrf = SymbSrfSubdivOffset(Obj -> U.TrimSrfs -> Srf,
					     Offset -> U.R, *Tolerance);

    	if (OffsetSrf == NULL)
	    return NULL;

    	TSrfObj = IPGenTRIMSRFObject(TrimSrfNew(OffsetSrf,
			     TrimCrvCopyList(Obj -> U.TrimSrfs -> TrimCrvList),
					      TRUE));

    	return TSrfObj;
    }
    else if (IP_IS_CRV_OBJ(Obj)) {
    	IPObjectStruct *CrvObj;
    	CagdCrvStruct *OffsetCrv;

	if (IP_IS_CRV_OBJ(Offset))
	    OffsetCrv = SymbCrvVarOffset(Obj -> U.Crvs, Offset -> U.Crvs,
					 IRIT_REAL_PTR_TO_INT(BezInterp));
	else
	    OffsetCrv = SymbCrvSubdivOffset(Obj -> U.Crvs, Offset -> U.R,
					    *Tolerance,
					    IRIT_REAL_PTR_TO_INT(BezInterp));

    	if (OffsetCrv == NULL)
	    return NULL;

    	CrvObj = IPGenCRVObject(OffsetCrv);

    	return CrvObj;
    }
    else {
	IRIT_NON_FATAL_ERROR("Offset allowed on polys/curves/surfaces only");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an approximation to the offset of a (planar) curve.		     M
*   This offset is computed and approximated to with a given tolerance       M
* Epsilon that is related to the square of the distance between the original M
* curve and its offset approximation.					     M
*   If Trim then regions in the curve with curvature that is larger than     M
* offset distance required will be trimmed out.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:        Curve to approximate its offset by Offset amount.            M
*   Offset:     Amount of offset, either a number or a scalar distance crv.  M
*   Epsilon:    Accuracy of offset.                                          M
*   Trim:       Should we deal with and trim self intersecting loops?        M
*   BezInterp:  Do we want Bezier interpolation or approximation?            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An offset approximating to Obj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenAOffsetObject                                                         M
*****************************************************************************/
IPObjectStruct *GenAOffsetObject(IPObjectStruct *Obj,
				 IPObjectStruct *Offset,
				 IrtRType *Epsilon,
				 IrtRType *Trim,
				 IrtRType *BezInterp)
{
    if (IP_IS_SRF_OBJ(Obj)) {
	IRIT_NON_FATAL_ERROR("Adaptive offset is not supported for surfaces");
	return NULL;
    }
    else if (IP_IS_CRV_OBJ(Obj)) {
    	IPObjectStruct *CrvObj;
    	CagdCrvStruct *OffsetCrv;

	if (IRIT_APX_EQ(*Trim, 0.0)) {
	    if (IP_IS_CRV_OBJ(Offset))
	        OffsetCrv = SymbCrvAdapVarOffset(Obj -> U.Crvs,
						 Offset -> U.Crvs,
						 *Epsilon, NULL,
						 IRIT_REAL_PTR_TO_INT(BezInterp));
	    else
	        OffsetCrv = SymbCrvAdapOffset(Obj -> U.Crvs, Offset -> U.R,
					      *Epsilon, NULL,
					      IRIT_REAL_PTR_TO_INT(BezInterp));
	}
	else				    
	    OffsetCrv = SymbCrvAdapOffsetTrim(Obj -> U.Crvs, Offset -> U.R,
					      *Epsilon, NULL,
					      IRIT_REAL_PTR_TO_INT(BezInterp));

    	if (OffsetCrv == NULL)
	    return NULL;
	else
	    CrvObj = IPGenCRVObject(OffsetCrv);

    	return CrvObj;
    }
    else {
	IRIT_NON_FATAL_ERROR("Offset allowed on curves/surfaces only");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tests if the offset curve OffsetObj has local self intersections with    M
* respect to original crv Obj.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:        Curve to examine if its offset has local self intersections. M
*   OffsetObj:  offset approximation of Obj, to test for self intersections. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    NULL if no self intersections, or a list of points  M
*                        each holding a pair of domain values in the self    M
*                        intersection region.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenAOffsetObject                                                         M
*****************************************************************************/
IPObjectStruct *HasLclSelfInterOffsetObject(IPObjectStruct *Obj,
					    IPObjectStruct *OffsetObj)
{
    CagdPtStruct *SIDmns, *Dmn;

    if (SymbIsOffsetLclSelfInters(Obj -> U.Crvs, OffsetObj -> U.Crvs,
				  &SIDmns)) {
        IrtRType
	    Zr = 0.0;
        IPObjectStruct
	    *Pts = IPGenLISTObject(NULL);

	for (Dmn = SIDmns;
	     Dmn != NULL && Dmn -> Pnext != NULL;
	     Dmn = Dmn -> Pnext -> Pnext) {
	    IPObjectStruct
	        *Pt = IPGenPTObject(&Dmn -> Pt[0], &Dmn -> Pnext -> Pt[0], &Zr);

	    IPListObjectAppend(Pts, Pt);
	}

	CagdPtFreeList(SIDmns);

	return Pts;
    }
    else
        return IPGenLISTObject(NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an approximation to the offset of a (planar) curve.		     M
*   This offset is computed and approximated using a least sqaure fit.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:          Curve to approximate its offset by Offset amount.          M
*   Offset:       Amount of offset.                                          M
*   NumOfSamples: To sample the offset curve.                                M
*   NumOfDOF:     Number of control points in resulting approximation.       M
*   Order:        Of resulting approximation.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An offset approximating to Obj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenLeastSqrOffsetObject                                                  M
*****************************************************************************/
IPObjectStruct *GenLeastSqrOffsetObject(IPObjectStruct *Obj,
					IrtRType *Offset,
					IrtRType *NumOfSamples,
					IrtRType *NumOfDOF,
					IrtRType *Order)
{
    CagdRType Tolerance;

    if (IP_IS_CRV_OBJ(Obj)) {
    	CagdCrvStruct
	    *OffsetCrv = SymbCrvLeastSquarOffset(Obj -> U.Crvs,
						 *Offset,
						 IRIT_REAL_PTR_TO_INT(NumOfSamples),
						 IRIT_REAL_PTR_TO_INT(NumOfDOF),
						 IRIT_REAL_PTR_TO_INT(Order),
						 &Tolerance);

    	if (OffsetCrv == NULL)
	    return NULL;
	else
	    return IPGenCRVObject(OffsetCrv);
    }
    else {
	IRIT_NON_FATAL_ERROR("LOffset allowed on curves only");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an approximation to the offset of a (planar) curve.		     M
*   This offset is computed and approximated using a tangent field matcing.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:          Curve to approximate its offset by Offset amount.          M
*   Offset:       Amount of offset.                                          M
*   Tolerance:    Accuracy of offset, measured in offset direction (normal   M
*		  of curve) angular error, in radians.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An offset approximating to Obj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GenMatchingOffsetObject                                                  M
*****************************************************************************/
IPObjectStruct *GenMatchingOffsetObject(IPObjectStruct *Obj,
					IrtRType *Offset,
					IrtRType *Tolerance)
{
    if (IP_IS_CRV_OBJ(Obj)) {
    	CagdCrvStruct
	    *OffsetCrv = SymbCrvMatchingOffset(Obj -> U.Crvs,
					       *Offset,
					       *Tolerance);

    	if (OffsetCrv == NULL)
	    return NULL;
	else
	    return IPGenCRVObject(OffsetCrv);
    }
    else {
	IRIT_NON_FATAL_ERROR("MOffset allowed on curves only");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Trim OffObj in all locations it is closer than TrimAmount to Crv.  Tol   M
* controls the accuracy of the solution.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:        Original curve or surface.                                   M
*   OffObj:     Approximated offset curve or surface.                        M
*   ParamsObj:  Relevant parameters.					     M
*		If Obj and OffObj are curves then this should be:	     M
*		  (Method, Tol, TrimAmount, NumerTol) where		     M
*                  + Method is 1 for distanc map based trimming,	     M
*			    or 2 for self-inersection via u-v elimination.   M
*		   + Tol is the subdivision tolerances of the data (see also M
*		     CONTOUR).						     M
*		   + TrimAmount sets the amount to trim OffObj, if closer    M
*		     than that to Obj.					     M
*		   + NumerTol if Positive, sets the numerical marching       M
*		     improvement step that is applied derived clipped	     M
*		     regions.						     M
*		If Obj and OffObj are surfaces then this should be:	     M
*		  (TrimAmount, Validate, Euclidean, SubdivTol, NumerTol)     M
*		   where:						     M
*                  + TrimAmount sets the amount to trim OffObj, if closer    M
*		     than that to Obj.					     M
*		   + Validate activates a validation criteria to filter      M
*		     interior self intersections and extract only valid self M
*		     intersection curves that delineate between valid and    M
*		     invalid (to be purged) offset regions.	             M
*		   + Euclidean controls whether the output will be in the    M
*		     parmaetric space or Euclidean space of OffObj.          M
*		   + SubdivTol/NumerTol subdivision/numerical tolerances of  M
*		     the multivariate solver.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  List of curve/trimmed surface segments, after the     M
*		       trimming.				             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvTrimGlblOffsetSelfInter, MvarSrfTrimGlblOffsetSelfInter           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimOffsetObject                                                         M
*****************************************************************************/
IPObjectStruct *TrimOffsetObject(IPObjectStruct *Obj,
				 IPObjectStruct *OffObj,
				 IPObjectStruct *ParamsObj)
{
    if (!IP_IS_OLST_OBJ(ParamsObj)) {
        IRIT_NON_FATAL_ERROR("TOFFSET: Expected third, parameters, object.");
	return NULL;
    }

    if (IP_IS_CRV_OBJ(Obj) && IP_IS_CRV_OBJ(OffObj)) {
	IPObjectStruct
	    *PObjMethod = IPListObjectGet(ParamsObj, 0),
	    *PObjTol = IPListObjectGet(ParamsObj, 1),
	    *PObjTrimAmount = IPListObjectGet(ParamsObj, 2),
	    *PObjNumerTol = IPListObjectGet(ParamsObj, 3);
	CagdCrvStruct *TCrv, *TrimCrvs;

	if (PObjMethod == NULL || !IP_IS_NUM_OBJ(PObjMethod) ||
	    PObjTol == NULL || !IP_IS_NUM_OBJ(PObjTol) ||
	    PObjTrimAmount == NULL || !IP_IS_NUM_OBJ(PObjTrimAmount) ||
	    PObjNumerTol == NULL || !IP_IS_NUM_OBJ(PObjNumerTol)) {
	    IRIT_NON_FATAL_ERROR("TOFFSET: Expected four/five numeric parameters in third TOFFSET list-parameter.");
	    return NULL;
	}

	if (PObjMethod -> U.R == 1.0)
	    TrimCrvs = SymbCrvTrimGlblOffsetSelfInter(Obj -> U.Crvs,
						      OffObj -> U.Crvs,
						      PObjTol -> U.R,
						      PObjTrimAmount -> U.R,
						      PObjNumerTol -> U.R);
	else
	    TrimCrvs = MvarCrvTrimGlblOffsetSelfInter(Obj -> U.Crvs,
						      OffObj -> U.Crvs,
						      PObjTrimAmount -> U.R,
						      PObjTol -> U.R,
						      PObjNumerTol -> U.R);

	if (TrimCrvs != NULL) {
	    int i = 0;
	    IPObjectStruct
	        *NewPObj = IPGenLISTObject(NULL);

	    while (TrimCrvs != NULL) {
	        IRIT_LIST_POP(TCrv, TrimCrvs);

		IPListObjectInsert(NewPObj, i++, IPGenCRVObject(TCrv));
	    }

	    IPListObjectInsert(NewPObj, i, NULL);

	    return NewPObj;
	}
	else
	    return NULL;
    }
    else if (IP_IS_SRF_OBJ(Obj) && IP_IS_SRF_OBJ(OffObj)) {
	IPObjectStruct
	    *PObjTrimAmount = IPListObjectGet(ParamsObj, 0),
	    *PObjValidate = IPListObjectGet(ParamsObj, 1),
	    *PObjEuclidean = IPListObjectGet(ParamsObj, 2),
	    *PObjSubdivTol = IPListObjectGet(ParamsObj, 3),
	    *PObjNumerTol = IPListObjectGet(ParamsObj, 4),
	    *PObjNumerImp = IPListObjectGet(ParamsObj, 5);
	IPObjectStruct *Plls;

	if (PObjTrimAmount == NULL || !IP_IS_NUM_OBJ(PObjTrimAmount) ||
	    PObjValidate == NULL || !IP_IS_NUM_OBJ(PObjValidate) ||
	    PObjEuclidean == NULL || !IP_IS_NUM_OBJ(PObjEuclidean) ||
	    PObjSubdivTol == NULL || !IP_IS_NUM_OBJ(PObjSubdivTol) ||
	    PObjNumerTol == NULL || !IP_IS_NUM_OBJ(PObjNumerTol) ||
	    PObjNumerImp == NULL || !IP_IS_NUM_OBJ(PObjNumerImp)) {
	    IRIT_NON_FATAL_ERROR("TOFFSET: Expected six numeric parameters in third TOFFSET parameter.");
	    return NULL;
	}

	Plls = MvarSrfTrimGlblOffsetSelfInter(Obj -> U.Srfs,
					      OffObj -> U.Srfs,
					      PObjTrimAmount -> U.R,
					      (int) (PObjValidate -> U.R),
					      (int) (PObjEuclidean -> U.R),
					      PObjSubdivTol -> U.R,
					      PObjNumerTol -> U.R,
					      PObjNumerImp -> U.R != 0.0);
	if (Plls -> U.Pl == NULL) {
	    IPFreeObject(Plls);
	    return NULL;
	}

	if (!((int) (PObjEuclidean -> U.R)) && PObjNumerImp -> U.R == 0.0) {
	    IPPolygonStruct *Pl;

	    /* Zero the Z component, keeping just UV of offset surface. */
	    for (Pl = Plls -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	        IPVertexStruct
		    *V = Pl -> PVertex;

		for ( ; V != NULL; V = V -> Pnext)
		    V -> Coord[2] = 0.0;
	    }
	}

	return Plls;
    }
    else {
	IRIT_NON_FATAL_ERROR("TOFFSET: Expected curves or surfaces only.");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes surfaces to fill in the gaps between the two given parallel       M
* surfaces.								     M
*   Together with the given two surfaces, the result is a closed shell.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1Obj, Srf2Obj:  The two parallel surfaces to fill the gaps of their   M
*                boundaries.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    An offset approximating to Obj.                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ParallelSrfs2Shell                                                       M
*****************************************************************************/
IPObjectStruct *ParallelSrfs2Shell(IPObjectStruct *Srf1Obj,
				   IPObjectStruct *Srf2Obj)
{
    if (IP_IS_SRF_OBJ(Srf1Obj) && IP_IS_SRF_OBJ(Srf2Obj)) {
        CagdSrfStruct *TSrf,
	    *BndrySrfs = SymbSrfCloseParallelSrfs2Shell(Srf1Obj -> U.Srfs,
							Srf2Obj -> U.Srfs);

    	if (BndrySrfs == NULL)
	    return NULL;
	else {
	    int i;
	    IPObjectStruct
	        *NewPObj = IPGenLISTObject(NULL);

	    for (i = 0; BndrySrfs != NULL; ) {
	        IRIT_LIST_POP(TSrf, BndrySrfs);

		IPListObjectInsert(NewPObj, i++, IPGenSRFObject(TSrf));
	    }

	    IPListObjectInsert(NewPObj, i, NULL);

	    return NewPObj;
	}
    }
    else {
	IRIT_NON_FATAL_ERROR("SShell allowed on surfaces only");
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Affine reparametrization, effectively changing the domain of the curve.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:         A curve to change its parametric domain.                    M
*   TMin, TMax:  New domain for Obj.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A curve identical to Obj but with parametric domain   M
*                      from TMin to TMax.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvReparametrization                                                     M
*****************************************************************************/
IPObjectStruct *CrvReparametrization(IPObjectStruct *Obj,
				     IrtRType *TMin,
				     IrtRType *TMax)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
	*Crv = CAGD_IS_BEZIER_CRV(Obj -> U.Crvs) ?
			  CagdCnvrtBzr2BspCrv(Obj -> U.Crvs) :
			  CagdCrvCopy(Obj -> U.Crvs);
    int ParamType1 = IRIT_REAL_PTR_TO_INT(TMin),
	ParamType2 = IRIT_REAL_PTR_TO_INT(TMax),
	Len = CAGD_CRV_PT_LST_LEN(Crv) + Crv -> Order;
    CagdRType MinDomain, MaxDomain,
	*KV = Crv -> KnotVector;

    switch (ParamType1) {
	case CAGD_NIELSON_FOLEY_PARAM:
	case CAGD_CHORD_LEN_PARAM:
	case CAGD_CENTRIPETAL_PARAM:
	case CAGD_UNIFORM_PARAM:
	    if (ParamType1 != ParamType2) {
		IRIT_NON_FATAL_ERROR("SREPARAM: Wrong parametrization spec.");
		return NULL;
	    }
	    BspReparameterizeCrv(Crv, ParamType1);
	    break;
	default:
	    CagdCrvDomain(Crv, &MinDomain, &MaxDomain);

	    /* Translate to 0.0, scale, and translate to new location. */
	    BspKnotAffineTrans(KV, Len, -MinDomain, 1.0);
	    BspKnotScale(KV, Len, (*TMax - *TMin) / (MaxDomain - MinDomain));
	    BspKnotAffineTrans(KV, Len, *TMin, 1.0);
	    break;
    }

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Affine reparametrization, effectively changing the domain of the surface.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:         A surface to change its parametric domain.                  M
*   RDir:        Direction of reparametrization. Either U or V.		     M
*   TMin, TMax:  New domain for Obj.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A surface identical to Obj but with parametric        M
*                      domain from TMin to TMax in direction RDir.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfReparametrization                                                     M
*****************************************************************************/
IPObjectStruct *SrfReparametrization(IPObjectStruct *Obj,
				     IrtRType *RDir,
				     IrtRType *TMin,
				     IrtRType *TMax)
{
    IPObjectStruct
	*SrfObj = NULL;
    CagdSrfDirType
	Dir = (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(RDir);
    int ParamType1 = IRIT_REAL_PTR_TO_INT(TMin),
	ParamType2 = IRIT_REAL_PTR_TO_INT(TMax);

    if (IP_IS_SRF_OBJ(Obj)) {
        CagdSrfStruct
	    *Srf = CAGD_IS_BEZIER_SRF(Obj -> U.Srfs) ?
			  CagdCnvrtBzr2BspSrf(Obj -> U.Srfs) :
			  CagdSrfCopy(Obj -> U.Srfs);

	switch (ParamType1) {
	    case CAGD_NIELSON_FOLEY_PARAM:
	    case CAGD_CHORD_LEN_PARAM:
	    case CAGD_CENTRIPETAL_PARAM:
	    case CAGD_UNIFORM_PARAM:
		if (ParamType1 != ParamType2) {
		    IRIT_NON_FATAL_ERROR("SREPARAM: Wrong parametrization spec.");
		    return NULL;
	        }
		BspReparameterizeSrf(Srf, Dir, ParamType1);
		break;
	    default:
		switch (Dir) {
		    case CAGD_CONST_U_DIR:
			BspKnotAffineTransOrder2(Srf -> UKnotVector,
						 Srf -> UOrder,
						 CAGD_SRF_UPT_LST_LEN(Srf) +
								 Srf -> UOrder,
						 *TMin, *TMax);
		        break;
		    case CAGD_CONST_V_DIR:
			BspKnotAffineTransOrder2(Srf -> VKnotVector,
						 Srf -> VOrder,
						 CAGD_SRF_VPT_LST_LEN(Srf) +
								 Srf -> VOrder,
						 *TMin, *TMax);
		        break;
		    default:
			IRIT_NON_FATAL_ERROR("SREPARAM: Wrong parametric direction.");
			return NULL;
		}
		break;
	}

	SrfObj = IPGenSRFObject(Srf);
    }
    else if (IP_IS_TRIMSRF_OBJ(Obj)) {
        CagdRType UMin, UMax, VMin, VMax;
        TrimSrfStruct
	    *TrimSrf = Obj -> U.TrimSrfs;

	switch (ParamType1) {
	    case CAGD_NIELSON_FOLEY_PARAM:
	    case CAGD_CHORD_LEN_PARAM:
	    case CAGD_CENTRIPETAL_PARAM:
	    case CAGD_UNIFORM_PARAM:
		if (ParamType1 != ParamType2) {
		    IRIT_NON_FATAL_ERROR("SREPARAM: Wrong parametrization spec.");
		    return NULL;
	        }
		TrimSrf = TrimSrfCopy(TrimSrf);
		BspReparameterizeSrf(TrimSrf -> Srf, Dir, ParamType1);
		break;
	    default:
		TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);
	
		switch (Dir) {
		    case CAGD_CONST_U_DIR:
			TrimSrf = TrimAffineTransTrimSrf(TrimSrf,
						    *TMin, *TMax, VMin, VMax);
		        break;
		    case CAGD_CONST_V_DIR:
			TrimSrf = TrimAffineTransTrimSrf(TrimSrf,
						    UMin, UMax, *TMin, *TMax);
		        break;
		    default:
		        assert(0);
		}
		break;
	}

	SrfObj = IPGenTRIMSRFObject(TrimSrf);
    }

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Affine reparametrization, effectively changing the domain of the trivar.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:         A trivariate to change its parametric domain.               M
*   RDir:        Direction of reparametrization. Either U, V or W.	     M
*   TMin, TMax:  New domain for Obj.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A trivar identical to Obj but with parametric         M
*                      domain from TMin to TMax in direction RDir.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivReparametrization                                                    M
*****************************************************************************/
IPObjectStruct *TrivReparametrization(IPObjectStruct *Obj,
				      IrtRType *RDir,
				      IrtRType *TMin,
				      IrtRType *TMax)
{
    IPObjectStruct *TVObj;
    TrivTVDirType
	Dir = (TrivTVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    TrivTVStruct
	*TV = TRIV_IS_BEZIER_TV(Obj -> U.Trivars) ?
			  TrivCnvrtBzr2BspTV(Obj -> U.Trivars) :
			  TrivTVCopy(Obj -> U.Trivars);
    int Len;
    CagdRType *KV, MinDomain, MaxDomain, MinUDomain, MaxUDomain,
	MinVDomain, MaxVDomain, MinWDomain, MaxWDomain;

    TrivTVDomain(TV, &MinUDomain, &MaxUDomain,
		     &MinVDomain, &MaxVDomain,
		     &MinWDomain, &MaxWDomain);

    switch (Dir) {
        case TRIV_CONST_U_DIR:
            Len = TRIV_TV_UPT_LST_LEN(TV) + TV -> UOrder;
	    KV = TV -> UKnotVector;
	    MinDomain = MinUDomain;
	    MaxDomain = MaxUDomain;
	    break;
        case TRIV_CONST_V_DIR:
            Len = TRIV_TV_VPT_LST_LEN(TV) + TV -> VOrder;
	    KV = TV -> VKnotVector;
	    MinDomain = MinVDomain;
	    MaxDomain = MaxVDomain;
	    break;
        case TRIV_CONST_W_DIR:
            Len = TRIV_TV_WPT_LST_LEN(TV) + TV -> WOrder;
	    KV = TV -> WKnotVector;
	    MinDomain = MinWDomain;
	    MaxDomain = MaxWDomain;
	    break;
	default:
	    IRIT_NON_FATAL_ERROR("TREPARAM: Wrong parametric direction.");
	    return NULL;
    }

    /* Translate to 0.0, scale, and translate to new location. */
    BspKnotAffineTrans(KV, Len, -MinDomain, 1.0);
    BspKnotScale(KV, Len, (*TMax - *TMin) / (MaxDomain - MinDomain));
    BspKnotAffineTrans(KV, Len, *TMin, 1.0);

    TVObj = IPGenTRIVARObject(TV);

    return TVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Affine reparametrization, effectively changing the domain of the multivar. M
*                                                                            *
* PARAMETERS:                                                                M
*   Obj:         A multivariate to change its parametric domain.             M
*   RDir:        Direction of reparametrization. 			     M
*   TMin, TMax:  New domain for Obj.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A multivariate identical to Obj but with parametric   M
*                      domain from TMin to TMax in direction RDir.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarReparametrization                                                    M
*****************************************************************************/
IPObjectStruct *MvarReparametrization(IPObjectStruct *Obj,
				      IrtRType *RDir,
				      IrtRType *TMin,
				      IrtRType *TMax)
{
    IPObjectStruct *MVObj;
    MvarMVDirType
	Dir = (MvarMVDirType) IRIT_REAL_PTR_TO_INT(RDir);
    MvarMVStruct
	*MV = MVAR_IS_BEZIER_MV(Obj -> U.MultiVars) ?
			  MvarCnvrtBzr2BspMV(Obj -> U.MultiVars) :
			  MvarMVCopy(Obj -> U.MultiVars);
    int Len;
    CagdRType *KV, MinDomain, MaxDomain;

    if (Dir < 0 || Dir >= MV -> Dim) {
	IRIT_WNDW_PUT_STR("Direction is out of domain");
	return NULL;
    }

    MvarMVDomain(MV, &MinDomain, &MaxDomain, Dir);
    Len = MV -> Lengths[Dir] + MV -> Orders[Dir];
    KV = MV -> KnotVectors[Dir];

    /* Translate to 0.0, scale, and translate to new location. */
    BspKnotAffineTrans(KV, Len, -MinDomain, 1.0);
    BspKnotScale(KV, Len, (*TMax - *TMin) / (MaxDomain - MinDomain));
    BspKnotAffineTrans(KV, Len, *TMin, 1.0);

    MVObj = IPGenMULTIVARObject(MV);

    return MVObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes curvature properties of the given curve.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Curve to evaluate its curvature properties.                  M
*   Eps:        Accuracy of computation.                                     M
*   Operation:  Action to take:		                                     M
*		1 - to compute the curvature square field as the Y axis of   M
*		    a 2D curve (X axis being the original parametrization).  M
*		2 - to compute the parameter values of the inflection points.M
*		3 - to subdivide the curve at the inflection points.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Either extremum curvature locations if Eps > 0, or    M
*                      curvature field square curve if Eps < 0.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCurvaturePts                                                          M
*****************************************************************************/
IPObjectStruct *CrvCurvaturePts(IPObjectStruct *PObj,
				IrtRType *Eps,
				IrtRType *Operation)
{ 
    IPObjectStruct *NewPObj, *PtObj;

    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Crvs -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(PObj -> U.Crvs -> PType) > 3) {
	IRIT_NON_FATAL_ERROR(
	    "CCRVTR: Only 2 or 3 dimensional curves are supported.");
	return NULL;
    }

    if (IRIT_REAL_PTR_TO_INT(Operation) == 1) {
	CagdRType TMin, TMax;
	CagdCrvStruct *CrvtrCrv2D,
	    *CrvtrCrv = SymbCrv3DCurvatureSqr(PObj -> U.Crvs);

	CagdCrvDomain(PObj -> U.Crvs, &TMin, &TMax);
	CrvtrCrv2D = SymbPrmtSclrCrvTo2D(CrvtrCrv, TMin, TMax);
	CagdCrvFree(CrvtrCrv);

	NewPObj = IPGenCRVObject(CrvtrCrv2D);
    }
    else if (IRIT_REAL_PTR_TO_INT(Operation) == 2) {
	int i;
	CagdPtStruct *IPtsTmp,
	    *IPts = SymbCrvExtremCrvtrPts(PObj -> U.Crvs, *Eps);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i,
			       PtObj = IPGenNUMValObject(IPtsTmp -> Pt[0]));
	    PtObj -> Attr = AttrCopyAttributes(IPtsTmp -> Attr);
	}

	CagdPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }
    else { /* Operation == 3. */
        int Proximity;
	CagdPtStruct
	    *IPts = SymbCrvExtremCrvtrPts(PObj -> U.Crvs, *Eps);
	CagdCrvStruct
	    *Crvs = CagdCrvSubdivAtParams(PObj -> U.Crvs, IPts, *Eps * 10,
					  &Proximity);

	CagdPtFreeList(IPts);

	NewPObj = IPLnkListToListObject(Crvs, IP_OBJ_CURVE);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Compute the inflection points of a curve.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       Curve to compute inflection points for.                      M
*   Eps:        Accuracy of computation.                                     M
*   Operation:  Action to take:		                                     M
*		1 - to compute the curve whose zero set is the inflection    M
*		    points set.						     M
*		2 - to compute the parameter values of the inflection points.M
*		3 - to subdivide the curve at the inflection points.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Either inflection locations if Operation == 1,	     M
*                      curvature field sign curve if Operation == 2, or      M
*		       curve segments with no interior inflection points if  M
*		       Operation == 3.					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvInflectionPts                                                         M
*****************************************************************************/
IPObjectStruct *CrvInflectionPts(IPObjectStruct *PObj,
				 IrtRType *Eps,
				 IrtRType *Operation)
{
    IPObjectStruct *NewPObj;

    if (IRIT_REAL_PTR_TO_INT(Operation) == 1) {
	CagdRType TMin, TMax;
	CagdCrvStruct *InflectCrv2D,
	    *InflectCrv = SymbCrv2DCurvatureSign(PObj -> U.Crvs);

	CagdCrvDomain(PObj -> U.Crvs, &TMin, &TMax);
	InflectCrv2D = SymbPrmtSclrCrvTo2D(InflectCrv, TMin, TMax);
	CagdCrvFree(InflectCrv);

	NewPObj = IPGenCRVObject(InflectCrv2D);
    }
    else if (IRIT_REAL_PTR_TO_INT(Operation) == 2) {
	int i;
	CagdPtStruct *IPtsTmp,
	    *IPts = SymbCrv2DInflectionPts(PObj -> U.Crvs, *Eps);

	NewPObj = IPGenLISTObject(NULL);

	for (IPtsTmp = IPts, i = 0;
	     IPtsTmp != NULL;
	     IPtsTmp = IPtsTmp -> Pnext, i++) {
	    IPListObjectInsert(NewPObj, i,
			       IPGenNUMValObject(IPtsTmp -> Pt[0]));
	}

	CagdPtFreeList(IPts);

	IPListObjectInsert(NewPObj, i, NULL);
    }
    else { /* Operation == 3. */
        int Proximity;
	CagdPtStruct
	    *IPts = SymbCrv2DInflectionPts(PObj -> U.Crvs, *Eps);
	CagdCrvStruct
	  *Crvs = CagdCrvSubdivAtParams(PObj -> U.Crvs, IPts, *Eps * 10,
					&Proximity);

	CagdPtFreeList(IPts);

	NewPObj = IPLnkListToListObject(Crvs, IP_OBJ_CURVE);
    }

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes curvature properties of the given curve.  If the given curve is a M
* planar vector curve, its signed curvature is evaluated.  Otherwise, if the M
* given curve is scalar, it is assumed to be a signed curvature field and a  M
* planar curve is reconstructed from it.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Curve to evaluate its curvature properties, or evaluate a crv M
*	       from curvature signatures.				     M
*              If a scalar curve, Crvtr->Curve reconstruction is assumed.    M
*              Otherwise, Curve->Crvtr sampling operation is assumed.        M
*   RSamples:  Samples in case of Curve->Crvtr, accuracy of computation in   M
*	       case of Crvtr->Curve.                                         M
*   ROrder:    Order of reconstructed approximation.                         M
*   RArcLenPeriodic:   If sampling curvature field of a curve, parameterize  M
*	       the samples according to arc-length, if TRUE.  If a curve is  M
*	       reconstructed from a curvature field, a closed curve is made  M
*	       (periodic) if TRUE.				             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Either extremum curvature locations if Eps > 0, or    M
*                      curvature field square curve if Eps < 0.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   CrvCurvatureFunction                                                     M
*****************************************************************************/
IPObjectStruct *CrvCurvatureFunction(IPObjectStruct *PObj,
				     IrtRType *RSamples,
				     IrtRType *ROrder,
				     IrtRType *RArcLenPeriodic)
{
    CagdCrvStruct *RetCrv,
	*Crv = PObj -> U.Crvs;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) == 1) {
	RetCrv = SymbSignedCrvtrGenCrv(Crv,
				       *RSamples,
				       IRIT_REAL_PTR_TO_INT(ROrder),
				       IRIT_REAL_PTR_TO_INT(RArcLenPeriodic));
    }
    else
        RetCrv = SymbCrvGenSignedCrvtr(Crv,
				       IRIT_REAL_PTR_TO_INT(RSamples),
				       IRIT_REAL_PTR_TO_INT(ROrder),
				       IRIT_REAL_PTR_TO_INT(RArcLenPeriodic));

    return IPGenCRVObject(RetCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes curvature properties of the given surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Surface to compute curvature properties for.                   M
*   RPtType:  Point type of output.                                          M
*   RDir:     Either curvature in U or V or total upper bound.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A curvature bound field for surface PObj.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfCurvatureBounds                                                       M
*****************************************************************************/
IPObjectStruct *SrfCurvatureBounds(IPObjectStruct *PObj,
				   IrtRType *RPtType,
				   IrtRType *RDir)
{ 
    IPObjectStruct *NewPObj;
    CagdSrfStruct *TSrf, *CrvtrSrfBound;
    CagdPointType
	PtType = (CagdPointType) IRIT_REAL_PTR_TO_INT(RPtType);
    CagdSrfDirType
	Dir = (CagdSrfDirType)  IRIT_REAL_PTR_TO_INT(RDir);

    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) > 3) {
	IRIT_NON_FATAL_ERROR(
	    "SCRVTR: Only 2 or 3 dimensional curves are supported.");
	return NULL;
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	case CAGD_CONST_V_DIR:
	    CrvtrSrfBound = SymbSrfIsoDirNormalCurvatureBound(PObj -> U.Srfs,
									Dir);
	    break;
	default:
	    CrvtrSrfBound = SymbSrfCurvatureUpperBound(PObj -> U.Srfs);
	    break;
    }

    switch (PtType) {
	case CAGD_PT_P1_TYPE:
	    break;
	case CAGD_PT_E1_TYPE:
	case CAGD_PT_E3_TYPE:
	case CAGD_PT_P3_TYPE:
	    TSrf = CagdCoerceSrfTo(CrvtrSrfBound, PtType, TRUE);
	    CagdSrfFree(CrvtrSrfBound);
	    CrvtrSrfBound = TSrf;
	    break;
	default:
	    CagdSrfFree(CrvtrSrfBound);
	    IRIT_NON_FATAL_ERROR("SCRVTR: Wrong point type coercion.");
	    return NULL;
    }

    NewPObj = IPGenSRFObject(CrvtrSrfBound);

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes radial curvatures linesfor the given surface and direction.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:                 Surface to compute radial curvature for.	     M
*   ViewDir:		  Direction from which to compute the radial crvtr.  M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*   MergeTol:		  Tolerance of point to polyline merge, negative for M
*			  performing no merge into polyline.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A curvature bound field for surface PObj.            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfRadialCurvature                                                       M
*****************************************************************************/
IPObjectStruct *SrfRadialCurvature(IPObjectStruct *PObj,
				   IrtVecType ViewDir,
				   IrtRType *SubdivTol,
				   IrtRType *NumerTol,
				   IrtRType *MergeTol)
{ 
    IPObjectStruct *ObjPtList;
    MvarPtStruct
	*MVPts = MvarSrfRadialCurvature(PObj -> U.Srfs, ViewDir,
					*SubdivTol, *NumerTol);

    if (MVPts == NULL)
	return NULL;

    ObjPtList = MvarCnvrtMVPtsToCtlPts(MVPts, *MergeTol);

    MvarPtFreeList(MVPts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes U/V-extreme implicit curve locations.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:             Surface to compute radial curvature for.		     M
*   Dir:	      U/V direction from which to compute the extreme pts.   M
*   SubdivTol, NumerTol:  Tolerances of computation.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Extreme locations along U or Vurface PObj, in the    M
*			parametric space of the surface.	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   ImplicitCrvExtreme                                                       M
*****************************************************************************/
IPObjectStruct *ImplicitCrvExtreme(IPObjectStruct *PSrf,
				   IrtRType *Dir,
				   IrtRType *SubdivTol,
				   IrtRType *NumerTol)
{ 
    IPObjectStruct *ObjPtList;
    MvarPtStruct
	*MVPts = MvarImplicitCrvExtreme(PSrf -> U.Srfs,
				        IRIT_REAL_PTR_TO_INT(Dir),
					*SubdivTol, *NumerTol);

    if (MVPts == NULL)
	return NULL;

    ObjPtList = MvarCnvrtMVPtsToCtlPts(MVPts, -1.0);

    MvarPtFreeList(MVPts);

    return ObjPtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the Gauss curvature surface of the given surface.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Surface to compute its gauss curvature surface for.           M
*   NumerOnly: If TRUE, only the numerator component of K is returned.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A Gauss curvature surface of PObj.                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfGaussCurvature                                                        M
*****************************************************************************/
IPObjectStruct *SrfGaussCurvature(IPObjectStruct *PObj, IrtRType *NumerOnly)
{ 
    IPObjectStruct *NewPObj;
    CagdSrfStruct *GaussSrf;

    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) > 3) {
	IRIT_NON_FATAL_ERROR(
	    "SGauss: Only 2 or 3 dimensional surfaces are supported.");
	return NULL;
    }
    
    GaussSrf = SymbSrfGaussCurvature(PObj -> U.Srfs,
				     IRIT_REAL_PTR_TO_INT(NumerOnly));

    NewPObj = IPGenSRFObject(GaussSrf);

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the Mean curvature square surface of the given surface.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:      Surface to compute its mean curvature square surface for.     M
*   NumerOnly: If TRUE, computes only the (rational) numerator of the mean   M
*	       curvature (note it is not the square of the mean curvature).  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A Mean curvature square surface of PObj.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfMeanCurvature                                                         M
*****************************************************************************/
IPObjectStruct *SrfMeanCurvature(IPObjectStruct *PObj, IrtRType *NumerOnly)
{ 
    IPObjectStruct *NewPObj;
    CagdSrfStruct *MeanSrf;

    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) > 3) {
	IRIT_NON_FATAL_ERROR(
	    "SMean: Only 2 or 3 dimensional surfaces are supported.");
	return NULL;
    }

    if (IRIT_APX_EQ(IRIT_REAL_PTR_TO_INT(NumerOnly), 0.0))
	MeanSrf = SymbSrfMeanCurvatureSqr(PObj -> U.Srfs);
    else
        MeanSrf = SymbSrfMeanNumer(PObj -> U.Srfs);

    NewPObj = IPGenSRFObject(MeanSrf);

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the isoparametric focal surface of the given surface.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Surface to compute its focal surface using isoparametric	     M
*	      normal curvature direction.				     M
*   RDir:     Either iso focal surface in U or V.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   An iso focal surface of surface PObj.                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SrfIsoFocalSrf                                                        M
*****************************************************************************/
IPObjectStruct *SrfIsoFocalSrf(IPObjectStruct *PObj, IrtRType *RDir)
{ 
    IPObjectStruct *NewPObj;
    CagdSrfStruct *IsoFocalSrf;
    CagdSrfDirType
	Dir = (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(RDir);

    if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) > 3) {
	IRIT_NON_FATAL_ERROR(
	    "IsoFocal: Only 2 or 3 dimensional surfaces are supported.");
	return NULL;
    }

    IsoFocalSrf = SymbSrfIsoFocalSrf(PObj -> U.Srfs, Dir);

    NewPObj = IPGenSRFObject(IsoFocalSrf);

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the (mean) evolute curve/surface of the given curve/surface.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:     Curve/Surface to compute its (mean) evolute for.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   A (mean) evolute curve/surface of PObj.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   FreeformEvolute                                                          M
*****************************************************************************/
IPObjectStruct *FreeformEvolute(IPObjectStruct *PObj)
{ 
    IPObjectStruct *NewPObj;

    if (IP_IS_CRV_OBJ(PObj)) {
	CagdCrvStruct *EvoluteCrv, *EvoluteCrvAux;

	if (CAGD_NUM_OF_PT_COORD(PObj -> U.Crvs -> PType) < 2 ||
	    CAGD_NUM_OF_PT_COORD(PObj -> U.Crvs -> PType) > 3) {
	    IRIT_NON_FATAL_ERROR(
		"EVOLUTE: Only 2 or 3 dimensional curves are supported.");
	    return NULL;
	}

	EvoluteCrvAux = SymbCrv3DRadiusNormal(PObj -> U.Crvs);
	EvoluteCrv = SymbCrvAdd(PObj -> U.Crvs, EvoluteCrvAux);
	CagdCrvFree(EvoluteCrvAux);

	NewPObj = IPGenCRVObject(EvoluteCrv);
    }
    else if (IP_IS_SRF_OBJ(PObj)) {
	CagdSrfStruct *EvoluteSrf, *EvoluteSrfAux;

	if (CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) < 2 ||
	    CAGD_NUM_OF_PT_COORD(PObj -> U.Srfs -> PType) > 3) {
	    IRIT_NON_FATAL_ERROR(
		"EVOLUTE: Only 2 or 3 dimensional surfaces are supported.");
	    return NULL;
	}

	EvoluteSrfAux = SymbSrfMeanEvolute(PObj -> U.Srfs);
	EvoluteSrf = SymbSrfAdd(PObj -> U.Srfs, EvoluteSrfAux);
	CagdSrfFree(EvoluteSrfAux);

	NewPObj = IPGenSRFObject(EvoluteSrf);
    }
    else
	NewPObj = NULL;

    return NewPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two surfaces into one in specified Dir and SameEdge flag. 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  Two surfaces to merge.                                      M
*   Dir:         Direction of merge. Either U or V.                          M
*   SameEdge:    Do Srf1 and Srf2 share a common edge?                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A surface result of the merge.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MergeSrfSrf                                                              M
*****************************************************************************/
IPObjectStruct *MergeSrfSrf(IPObjectStruct *Srf1,
			    IPObjectStruct *Srf2,
			    IrtRType *Dir,
			    IrtRType *SameEdge)
{
    IPObjectStruct *SrfObj;
    CagdSrfStruct
	*Srf = CagdMergeSrfSrf(Srf1 -> U.Srfs, Srf2 -> U.Srfs,
			       (CagdSrfDirType) IRIT_REAL_PTR_TO_INT(Dir),
			       IRIT_REAL_PTR_TO_INT(SameEdge), TRUE);

    SrfObj = IPGenSRFObject(Srf);

    return SrfObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merge two curves/ctl points into one curve by adding a linear segment      M
* between the first end point to second start point.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj1, PObj2:  Either curve or a control point to merge.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Merged curve.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MergeCurvesAndCtlPoints                                                  M
*****************************************************************************/
IPObjectStruct *MergeCurvesAndCtlPoints(IPObjectStruct *PObj1,
					IPObjectStruct *PObj2)
{
    IPObjectStruct *CrvObj;
    CagdCrvStruct
	*Crv = NULL;
    CagdPtStruct Pt1, Pt2;

    if (IP_IS_CRV_OBJ(PObj1)) {
	if (IP_IS_CRV_OBJ(PObj2)) {
	    Crv = CagdMergeCrvCrv(PObj1 -> U.Crvs, PObj2 -> U.Crvs, TRUE);
	}
	else if (IP_IS_CTLPT_OBJ(PObj2)) {
	    CagdRType
		*Coords2 = PObj2 -> U.CtlPt.Coords;

	    CagdCoerceToE3(Pt2.Pt, &Coords2, -1, PObj2 -> U.CtlPt.PtType);
	    Crv = CagdMergeCrvPt(PObj1 -> U.Crvs, &Pt2);
	}
	else
	    IRIT_FATAL_ERROR("Curve/CtlPt was expected.");
    }
    else if (IP_IS_CTLPT_OBJ(PObj1)) {
	if (IP_IS_CRV_OBJ(PObj2)) {
	    CagdRType
		*Coords1 = PObj1 -> U.CtlPt.Coords;

	    CagdCoerceToE3(Pt1.Pt, &Coords1, -1, PObj1 -> U.CtlPt.PtType);
	    Crv = CagdMergePtCrv(&Pt1, PObj2 -> U.Crvs);
	}
	else if (IP_IS_CTLPT_OBJ(PObj2)) {
	    Crv = CagdMergeCtlPtCtlPt(&PObj1 -> U.CtlPt, &PObj2 -> U.CtlPt, 0);
	}
	else
	    IRIT_FATAL_ERROR("Curve/CtlPt was expected.");
    }
    else
	IRIT_FATAL_ERROR("Curve/CtlPt was expected.");

    if (Crv == NULL)
	return NULL;

    CrvObj = IPGenCRVObject(Crv);

    return CrvObj;
}
