/*****************************************************************************
*   Sketching relation routines for freeform entities.            	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 0.1, Oct. 1997.  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "user_lib.h"
#include "ip_cnvrt.h"
#include "grap_loc.h"
#include "geom_lib.h"

/* #define SKETCH_SCAN_ALL_POINT	     1 */
#define SURFACE_SKETCH_POINT_DIST    10
#define STROKE_POLY_FINE_NESS        0.01
#define FULL_LENGTH_THRESHOLD	     0.1
#define SKTCH_IMP_FACTOR	     100
#define NRML_SRF_IMPORTANCE_EPS	     1e-4
#define SRF_STROKE_IMP_REL_LEN	     3

#define SKETCH_PT_DIST_FINENESS	     \
    (IGGlblPolylineOptiApprox == SYMB_CRV_APPROX_TOLERANCE ? \
                               1.0 / IGGlblPllnFineness : IGGlblPllnFineness)

IRIT_STATIC_DATA int GlblVrtxImportanceCount;
IRIT_STATIC_DATA IrtRType GlblVrtxImportanceVal;
IRIT_STATIC_DATA IrtVecType GlblLight0, GlblLight1;
IRIT_STATIC_DATA IPVertexStruct *GlblVrtxImportance;
IRIT_STATIC_DATA IPObjectStruct
    *GlblPPlObj = NULL;

static void IGSketchDrawStrokes(IPObjectStruct *PObjSketches);
static int SphConeQueryCone(IrtVecType ConeDir, IrtRType ConeAngle);
static void SphConeCallBack(IPVertexStruct *V);
static int EvalSrfImportance(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v);
static void MarchOnSrf(CagdSrfStruct *Srf,
		       CagdSrfStruct *NSrf,
		       CagdSrfStruct *DuSrf,
		       CagdSrfStruct *DvSrf,
		       IPVertexStruct *V,
		       CagdUVType UV,
		       IrtRType Length,
		       IrtRType FineNess,
		       int SketchType,
		       IrtHmgnMatType Mat);
static IPObjectStruct *MarchOnSrfOnce(CagdSrfStruct *Srf,
				      CagdSrfStruct *NSrf,
				      CagdSrfStruct *DuSrf,
				      CagdSrfStruct *DvSrf,
				      IPVertexStruct *V,
				      CagdUVType UV,
				      IrtRType Length,
				      IrtRType FineNess,
				      int SketchType,
				      IrtHmgnMatType Mat);
static void MarchOnPoly(IPObjectStruct *PObj,
			IPPolygonStruct *PlHead,
			IPVertexStruct *VHead,
			IrtRType Length,
			IrtRType FineNess,
			int SketchType,
			IrtHmgnMatType Mat);
static IPObjectStruct *MarchOnPolyOnce(IPObjectStruct *PObj,
				       IPPolygonStruct *PlHead,
				       IPVertexStruct *VHead,
				       IrtRType Length,
				       IrtRType FineNess,
				       int SketchType,
				       IrtHmgnMatType Mat);
static void ProcessVertexImportance(IPVertexStruct *V1,
				    IPVertexStruct *V2,
				    IPPolygonStruct *Pl1,
				    IPPolygonStruct *Pl2);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Draw a sketch like drawing of a given object that is cached on.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObjSketches:     A sketches object.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IGSketchDrawStrokes(IPObjectStruct *PObjSketches)
{
    int i;
    VoidPtr
        SphCones = AttrGetObjectPtrAttrib(PObjSketches, "_SphCones");

    for (i = 0; i < 3; i++) {
	GlblLight0[i] = IGShadeParam.LightPos[0][i];
	GlblLight1[i] = IGShadeParam.LightPos[1][i];
    }
    if (IGSketchParam.SketchInvShd) {
	IRIT_VEC_SCALE(GlblLight0, -1.0);
	IRIT_VEC_SCALE(GlblLight1, -1.0);
    }
    MatMultVecby4by4(GlblLight0, GlblLight0, IGGlblInvCrntViewMat);
    IRIT_VEC_NORMALIZE(GlblLight0);
    MatMultVecby4by4(GlblLight1, GlblLight1, IGGlblInvCrntViewMat);
    IRIT_VEC_NORMALIZE(GlblLight1);

    if (GlblPPlObj == NULL)
	GlblPPlObj = IPGenPOLYLINEObject(NULL);

    IP_ATTR_FREE_ATTRS(GlblPPlObj -> Attr);
    GlblPPlObj -> Attr = IP_ATTR_COPY_ATTRS(PObjSketches -> Attr);

    if (SphCones == NULL) {
	IPVertexStruct *V;

	/* View independent importances strokes. */
	for (V = PObjSketches -> U.Pl -> PVertex;
	     V != NULL;
	     V = V -> Pnext) {
	    IPObjectStruct
		*PStrokeObj = AttrGetObjAttrib(V -> Attr, "_ImpStrokes");
	    IPPolygonStruct *Pl, *Pls;

	    Pls = PStrokeObj == NULL ? NULL : PStrokeObj -> U.Pl;

	    for (Pl = Pls; Pl != NULL; Pl = Pl -> Pnext) {
	        IPPolygonStruct
		    *PlNext = Pl -> Pnext;

		Pl -> Pnext = NULL;

		GlblPPlObj -> U.Pl = Pl;
		IGDrawPolyFuncPtr(GlblPPlObj);

		Pl -> Pnext = PlNext;
	    }
	}
    }
    else {
      /* View dependent shading and silhouette strokes. */
        GMSphConeQuery2GetVectors(SphCones, SphConeQueryCone,
				  SphConeCallBack);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing of a surface.		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSketches:     A sketches object.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchDrawSurface                                                      M
*****************************************************************************/
void IGSketchDrawSurface(IPObjectStruct *PObjSketches)
{
    int i = 0;
    IPObjectStruct *PTmp;

    do {
	if ((PTmp = IPListObjectGet(PObjSketches, i)) != NULL) {
	    IGSketchDrawStrokes(PTmp);
	    if (IGSketchParam.SketchImp &&
		(PTmp = IPListObjectGet(PObjSketches, i + 1)) != NULL)
	        IGSketchDrawStrokes(PTmp);
	}
	i += 2;
    }
    while (PTmp != NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if the given cone (Dir, Angle) contains valid strokes.              *
*                                                                            *
* PARAMETERS:                                                                *
*   ConeDir:      Direction of cone.                                         *
*   ConeAngle:    Angular span of cone.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if cone contain valid strokes, FALSE otherwise.       *
*****************************************************************************/
static int SphConeQueryCone(IrtVecType ConeDir, IrtRType ConeAngle)
{
    IrtVecType Vec;
    IrtRType
	SilPower = IRIT_MAX(acos(IGSketchParam.SilPower) -
						 IRIT_DEG2RAD(ConeAngle), 0.0),
	ShdPower = IRIT_MIN(acos(IGSketchParam.ShadePower) +
				          IRIT_DEG2RAD(ConeAngle), M_PI_DIV_2);

    MatMultVecby4by4(Vec, ConeDir, IPViewMat);
    IRIT_VEC_NORMALIZE(Vec);
    if (IGGlblBackFaceCull && Vec[2] < -sin(IRIT_DEG2RAD(ConeAngle)))
        return FALSE;
    if (IRIT_FABS(Vec[2]) < cos(SilPower))
	return TRUE;

    ShdPower = cos(ShdPower);
    return IRIT_DOT_PROD(ConeDir, GlblLight0) > ShdPower ||
	   IRIT_DOT_PROD(ConeDir, GlblLight1) > ShdPower;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function is invoked by the spherical cones subdivision data struct  *
* on every direction that contains a potentially valid stroke.               *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         Direction vector of stroke.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SphConeCallBack(IPVertexStruct *V)
{
    static IrtVecType
	ZVec = { 0.0, 0.0, 1.0 };
    IrtVecType Vec;
    IrtRType RTmp1, RTmp2;
    IPObjectStruct
        *PSilStrokeObj = AttrGetObjAttrib(V -> Attr, "_SilStrokes");
    IPPolygonStruct *Pl,
        *SilPls = PSilStrokeObj == NULL ? NULL : PSilStrokeObj -> U.Pl;


    MatMultVecby4by4(Vec, ZVec, IGGlblInvCrntViewMat);
    IRIT_VEC_NORMALIZE(Vec);

    RTmp1 = IGSketchParam.SilPower - IRIT_FABS(IRIT_DOT_PROD(V -> Normal, Vec));
    if (RTmp1 > 0.0) {
	for (Pl = SilPls; Pl != NULL; Pl = Pl -> Pnext) {
	    IPPolygonStruct
	        *PlNext = Pl -> Pnext;
	    IPVertexStruct *VLast,
		*VNext = NULL;

	    if (RTmp1 < FULL_LENGTH_THRESHOLD) {
		int i, Len;

		VLast = Pl -> PVertex;
		Len = IPVrtxListLen(VLast);

		Len = (int) (Len * RTmp1 / FULL_LENGTH_THRESHOLD);
		for (i = 0; i < Len; i++)
		    VLast = VLast -> Pnext;
		if (VLast != NULL) {
		    VNext = VLast -> Pnext;
		    VLast -> Pnext = NULL;
		}
	    }
	    else
		VLast = NULL;

	    Pl -> Pnext = NULL;

	    GlblPPlObj -> U.Pl = Pl;
	    IGDrawPolyFuncPtr(GlblPPlObj);

	    Pl -> Pnext = PlNext;

	    /* If we broke the polyline in the middle - reconnect it now. */
	    if (VLast != NULL)
		VLast -> Pnext = VNext;
	}
    }

    RTmp1 = IRIT_DOT_PROD(V -> Normal, GlblLight0) - IGSketchParam.ShadePower;
    RTmp2 = IRIT_DOT_PROD(V -> Normal, GlblLight1) - IGSketchParam.ShadePower;
    RTmp1 = IRIT_MAX(RTmp1, 0) + IRIT_MAX(RTmp2, 0);

    if (RTmp1 > 0.0) {
	IPObjectStruct
	    *PShdStrokeObj = AttrGetObjAttrib(V -> Attr, "_ShdStrokes");
	IPPolygonStruct *Pl,
	    *ShdPls = PShdStrokeObj == NULL ? SilPls : PShdStrokeObj -> U.Pl;

	for (Pl = ShdPls; Pl != NULL; Pl = Pl -> Pnext) {
	    IPPolygonStruct
	        *PlNext = Pl -> Pnext;
	    IPVertexStruct *VLast,
		*VNext = NULL;

	    if (RTmp1 < FULL_LENGTH_THRESHOLD) {
		int i, Len;

		VLast = Pl -> PVertex;
		Len = IPVrtxListLen(VLast);

		Len = (int) (Len * RTmp1 / FULL_LENGTH_THRESHOLD);
		for (i = 0; i < Len; i++)
		    VLast = VLast -> Pnext;
		if (VLast != NULL) {
		    VNext = VLast -> Pnext;
		    VLast -> Pnext = NULL;
		}
	    }
	    else
		VLast = NULL;

	    Pl -> Pnext = NULL;

	    GlblPPlObj -> U.Pl = Pl;
	    IGDrawPolyFuncPtr(GlblPPlObj);

	    Pl -> Pnext = PlNext;

	    /* If we broke the polyline in the middle - reconnect it now. */
	    if (VLast != NULL)
		VLast -> Pnext = VNext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates the importance of a given point and return TRUE if to retain   *
* it.  A callback function of SymbUniformAprxPtOnSrfDistrib                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   Surface we compute point distribution over.                       *
*   u, v:  (u, v) parametric location we are to examine.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE to retain the point, FALSE to purge it.                      *
*****************************************************************************/
static int EvalSrfImportance(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v)
{
    static CagdRType
	CachedSketchedImpDecay = IRIT_INFNTY;
    static CagdSrfStruct
      *NrmlSrf = NULL;
    static CagdSrfStruct const
	*CachedSrf = NULL;
    CagdRType UMin, UMax, VMin, VMax, du, dv, *R;
    IrtVecType N, Nu, Nv;

    if (CachedSrf != Srf) {
	if (NrmlSrf != NULL)
	    CagdSrfFree(NrmlSrf);
	NrmlSrf = SymbSrfNormalSrf(Srf);
	CachedSrf = Srf;
    }

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    R = CagdSrfEval(NrmlSrf, u, v);
    CagdCoerceToE3(N, &R, -1, NrmlSrf -> PType);
    IRIT_VEC_NORMALIZE(N);

    du = u > (UMin + UMax) * 0.5 ? -NRML_SRF_IMPORTANCE_EPS
				 : NRML_SRF_IMPORTANCE_EPS;
    R = CagdSrfEval(NrmlSrf, u + du, v);
    CagdCoerceToE3(Nu, &R, -1, NrmlSrf -> PType);
    IRIT_VEC_NORMALIZE(Nu);
    IRIT_VEC_SUB(Nu, N, Nu);
    du = sqrt(IRIT_DOT_PROD(Nu, Nu)) / NRML_SRF_IMPORTANCE_EPS;

    dv = v > (VMin + VMax) * 0.5 ? -NRML_SRF_IMPORTANCE_EPS
				 : NRML_SRF_IMPORTANCE_EPS;
    R = CagdSrfEval(NrmlSrf, u, v + dv);
    CagdCoerceToE3(Nv, &R, -1, NrmlSrf -> PType);
    IRIT_VEC_NORMALIZE(Nv);
    IRIT_VEC_SUB(Nv, N, Nv);
    dv = sqrt(IRIT_DOT_PROD(Nv, Nv)) / NRML_SRF_IMPORTANCE_EPS;

    if (!IRIT_APX_EQ(IGSketchParam.SketchImpDecay, CachedSketchedImpDecay)) {
        CachedSketchedImpDecay = IGSketchParam.SketchImpDecay;
    }

    return du + dv > IGSketchParam.SketchImpDecay;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing for the given surface with the           M
* fineness prescribed.                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       A surface.                                                    M
*   FineNess:  Relative fineness to approximate the sketches with.           M
*   PObj:      Object Srf originated from.				     M
*   Importance:  If TRUE, we should also compute importance for each point.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The sketch data.	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchGenSrfSketches                                                   M
*****************************************************************************/
IPObjectStruct *IGSketchGenSrfSketches(CagdSrfStruct *Srf,
				       IrtRType FineNess,
				       IPObjectStruct *PObj,
				       int Importance)
{
    int i, n;
    IrtRType
	Area = 1.0;
    IrtVecType Light0;
    IrtHmgnMatType LightMat, InvLightMat;
    CagdUVType *UV, *UVPts;
    CagdSrfStruct
	*NSrf = NULL,
	*DuSrf = NULL,
	*DvSrf = NULL,
	*MappedSrf = NULL,
	*MappedNSrf = NULL,
	*MappedDuSrf = NULL,
	*MappedDvSrf = NULL;
    IPObjectStruct *PPtsObj, *PObjPolygons;
    IPVertexStruct
	*VHead = NULL;
    TrimSrfStruct
	*TrimSrf = IP_IS_TRIMSRF_OBJ(PObj) ? PObj -> U.TrimSrfs : NULL;

    if ((PObjPolygons = AttrGetObjectObjAttrib(PObj, "_PolygonsHiRes"))
								    != NULL)
	Area = GMPolyObjectArea(PObjPolygons);

    /* Convert to the current view. */
    Srf = CagdSrfMatTransform(Srf, IGGlblCrntViewMat);

    /* Estimate number of points to employ in the point distribution. */
    n = (int) (FineNess * SKETCH_PT_DIST_FINENESS *
	       SURFACE_SKETCH_POINT_DIST * Area);

    UVPts = SymbUniformAprxPtOnSrfDistrib(Srf, FALSE, IRIT_MAX(n, 2),
					  Importance ? EvalSrfImportance
						     : NULL);

    CagdSrfEffiNrmlPrelude(Srf);

    if (Importance) {
	switch (IGSketchParam.SketchImpType) {
	    case IG_SKETCHING_ISOCLINES:
	    case IG_SKETCHING_ORTHOCLINES:
	    case IG_SKETCHING_ISO_PARAM:
	        DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
		DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
		NSrf = SymbSrfNormalSrf(Srf);
	        break;
	    case IG_SKETCHING_CURVATURE:
		SymbEvalSrfCurvPrep(Srf, TRUE);
	        break;
	}
    }
    else {
	switch (IGSketchParam.SketchSilType) {
	    case IG_SKETCHING_ISOCLINES:
	    case IG_SKETCHING_ORTHOCLINES:
	    case IG_SKETCHING_ISO_PARAM:
	        DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
		DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
		NSrf = SymbSrfNormalSrf(Srf);
	        break;
	    case IG_SKETCHING_CURVATURE:
		SymbEvalSrfCurvPrep(Srf, TRUE);
	        break;
	}
	switch (IGSketchParam.SketchShdType) {
	    case IG_SKETCHING_ISO_PARAM:
	        break;
	    case IG_SKETCHING_ISOCLINES:
	    case IG_SKETCHING_ORTHOCLINES:
		for (i = 0; i < 3; i++)
		    Light0[i] = IGShadeParam.LightPos[0][i];
		GMGenMatrixZ2Dir(LightMat, Light0);
		MatInverseMatrix(LightMat, InvLightMat);
		MappedSrf = CagdSrfMatTransform(Srf, InvLightMat);
		MappedDuSrf = CagdSrfDerive(MappedSrf, CAGD_CONST_U_DIR);
		MappedDvSrf = CagdSrfDerive(MappedSrf, CAGD_CONST_V_DIR);
		MappedNSrf = SymbSrfNormalSrf(MappedSrf);
	        break;
	    case IG_SKETCHING_CURVATURE:
		if (IGSketchParam.SketchSilType != IG_SKETCHING_CURVATURE)
		    SymbEvalSrfCurvPrep(Srf, TRUE);
	        break;
	}
    }

    for (i = 0, UV = UVPts; i < n; i++, UV++) {
	CagdRType *R;
	CagdVecStruct *Vec;

	/* Make sure we did not fail to generate all n points. */
	if (*UV[0] == -IRIT_INFNTY || *UV[1] == -IRIT_INFNTY)
	    break;

	if (TrimSrf && !TrimIsPointInsideTrimSrf(TrimSrf, *UV))
	    continue;

	Vec = CagdSrfEffiNrmlEval((*UV)[0], (*UV)[1], TRUE);

	VHead = IPAllocVertex2(VHead);
	IRIT_PT_COPY(VHead -> Normal, Vec -> Vec);

	R = CagdSrfEval(Srf, (*UV)[0], (*UV)[1]);
	CagdCoerceToE3(VHead -> Coord, &R, -1, Srf -> PType);

	if (Importance) {
	    MarchOnSrf(Srf, NSrf, DuSrf, DvSrf, VHead, (*UV),
		       IGGlblNormalSize,
		       SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
		       IG_SKETCH_TYPE_IMPORTANCE, NULL);
	}
	else {
	    MarchOnSrf(Srf, NSrf, DuSrf, DvSrf, VHead, (*UV),
		       IGGlblNormalSize * SRF_STROKE_IMP_REL_LEN,
		       SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
		       IG_SKETCH_TYPE_SILHOUETTE, NULL);

	    if (MappedSrf != NULL) {
	        IrtVecType Nrml;
		IrtPtType Coord;

		IRIT_PT_COPY(Coord, VHead -> Coord);
		IRIT_VEC_COPY(Nrml, VHead -> Normal);

	        /* Map the geometry to face light source 0. */
	        MatMultPtby4by4(VHead -> Coord, Coord, InvLightMat);
		MatMultVecby4by4(VHead -> Normal, Nrml, InvLightMat);

		MarchOnSrf(MappedSrf, MappedNSrf, MappedDuSrf, MappedDvSrf,
			   VHead, (*UV),
			   IGGlblNormalSize * SRF_STROKE_IMP_REL_LEN,
			   SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
			   IG_SKETCH_TYPE_SHADING, LightMat);

		/* Restore the geometry. */
		IRIT_PT_COPY(VHead -> Coord, Coord);
		IRIT_VEC_COPY(VHead -> Normal, Nrml);
	    }
	    else {
		MarchOnSrf(Srf, NSrf, DuSrf, DvSrf,
			   VHead,
			   (*UV), IGGlblNormalSize * SRF_STROKE_IMP_REL_LEN,
			   SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
			   IG_SKETCH_TYPE_SHADING, NULL);
	    }
	}

	MatMultVecby4by4(VHead -> Normal, VHead -> Normal,
			 IGGlblInvCrntViewMat);
	if (!IRIT_PT_EQ_ZERO(VHead -> Normal))
	    IRIT_PT_NORMALIZE(VHead -> Normal);
    }

    IritFree(UVPts);
    CagdSrfEffiNrmlPostlude();
    SymbEvalSrfCurvPrep(Srf, FALSE);

    PPtsObj = IPGenPOLYLINEObject(IPAllocPolygon(0, VHead, NULL));

    if (DuSrf != NULL) {
	CagdSrfFree(DuSrf);
	CagdSrfFree(DvSrf);
	CagdSrfFree(NSrf);
    }

    if (MappedDuSrf != NULL) {
	CagdSrfFree(MappedDuSrf);
	CagdSrfFree(MappedDvSrf);
	CagdSrfFree(MappedNSrf);
	CagdSrfFree(MappedSrf);
    }

    CagdSrfFree(Srf);

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Precompute the strokes of the given surface along the prescribed march.  *
*   Up to four strokes are developed and placed as "_*strokes" attributes    *
* under the Vertex V as well as returned.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        To trace the strokes along                                   *
*   NSrf:       Normal field of surface to march on (optional).              *
*   DuSrf:      Partial with respect to u (optional).                        *
*   DvSrf:      Partial with respect to v (optional).                        *
*   V:          Vertex holding position/normal of starting point.            *
*   UV:         Starting location of stroke.                                 *
*   Length:     Of stroke (real arc length).                                 *
*   FineNess:   Of marching steps on the surface.			     *
*   SketchType: Sketching type - silhouette, shading, importance, etc.	     *
*   Mat:        Additional transformation required to sketches, if not NULL. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void MarchOnSrf(CagdSrfStruct *Srf,
		       CagdSrfStruct *NSrf,
		       CagdSrfStruct *DuSrf,
		       CagdSrfStruct *DvSrf,
		       IPVertexStruct *V,
		       CagdUVType UV,
		       IrtRType Length,
		       IrtRType FineNess,
		       int SketchType,
		       IrtHmgnMatType Mat)
{
    IPObjectStruct *PRetObj;

    switch (SketchType) {
	case IG_SKETCH_TYPE_SILHOUETTE:
            PRetObj = MarchOnSrfOnce(Srf, NSrf, DuSrf, DvSrf, V, UV, Length,
				     FineNess, IGSketchParam.SketchSilType,
				     Mat);
	    AttrSetObjAttrib(&V -> Attr, "_SilStrokes", PRetObj, FALSE);
            break;
	case IG_SKETCH_TYPE_SHADING:
	    PRetObj = MarchOnSrfOnce(Srf, NSrf, DuSrf, DvSrf, V, UV, Length,
				     FineNess, IGSketchParam.SketchShdType,
				     Mat);
	    AttrSetObjAttrib(&V -> Attr, "_ShdStrokes", PRetObj, FALSE);
            break;
	case IG_SKETCH_TYPE_IMPORTANCE:
	    PRetObj = MarchOnSrfOnce(Srf, NSrf, DuSrf, DvSrf, V, UV, Length,
				     FineNess, IGSketchParam.SketchImpType,
				     Mat);
	    AttrSetObjAttrib(&V -> Attr, "_ImpStrokes", PRetObj, FALSE);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Precompute the strokes of the given surface along the prescribed march.  *
*   Up to four strokes are developed and placed as "_*strokes" attributes    *
* under the Vertex V as well as returned.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:        To trace the strokes along                                   *
*   NSrf:       Normal field of surface to march on (optional).              *
*   DuSrf:      Partial with respect to u (optional).                        *
*   DvSrf:      Partial with respect to v (optional).                        *
*   V:          Vertex holding position/normal of starting point.            *
*   UV:         Starting location of stroke.                                 *
*   Length:     Of stroke (real arc length).                                 *
*   FineNess:   Of marching steps on the surface.			     *
*   SketchType: Type of strokes to develope for this sketch.		     *
*   Mat:        Additional transformation required to sketches, if not NULL. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static IPObjectStruct *MarchOnSrfOnce(CagdSrfStruct *Srf,
				      CagdSrfStruct *NSrf,
				      CagdSrfStruct *DuSrf,
				      CagdSrfStruct *DvSrf,
				      IPVertexStruct *V,
				      CagdUVType UV,
				      IrtRType Length,
				      IrtRType FineNess,
				      int SketchType,
				      IrtHmgnMatType Mat)
{
    CagdBType ClosedInU, ClosedInV;
    CagdRType K1, K2, UMin, UMax, VMin, VMax;
    CagdVType D1, D2, D;
    IPPolygonStruct *Pl,
	*TracedPls = NULL;
    IPObjectStruct *PRetObj;
    UserSrfMarchType MarchType;
    IrtHmgnMatType SketchMap;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    ClosedInU = CagdIsClosedSrf(Srf, CAGD_CONST_U_DIR);
    ClosedInV = CagdIsClosedSrf(Srf, CAGD_CONST_V_DIR);

    switch (SketchType) {
	case IG_SKETCHING_ISO_PARAM:
	default:
	    MarchType = USER_SRF_MARCH_ISO_PARAM;

	    /* Prepare the isoparametric directions in parameter space. */
	    IRIT_PT_RESET(D1);
	    IRIT_PT_RESET(D2);
	    D1[0] = D2[1] = 1.0;
	    break;
	case IG_SKETCHING_CURVATURE:
	    MarchType = USER_SRF_MARCH_PRIN_CRVTR;

	    /* Prepare the principal directions on the tangent plane. */
	    SymbEvalSrfCurvature(Srf, UV[0], UV[1], TRUE, &K1, &K2, D1, D2);
	    break;
	case IG_SKETCHING_ISOCLINES:
	    MarchType = USER_SRF_MARCH_ISOCLINES;

	    /* This is the "silhouette" from +Z direction. */
	    IRIT_PT_RESET(D1);
	    IRIT_PT_RESET(D2);
	    D1[2] = D2[2] = 1.0;
	    break;
	case IG_SKETCHING_ORTHOCLINES:
	    MarchType = USER_SRF_MARCH_ORTHOCLINES;

	    /* This is orthogonal to the "silhouette" from +Z direction. */
	    IRIT_PT_RESET(D1);
	    IRIT_PT_RESET(D2);
	    D1[2] = D2[2] = 1.0;
	    break;
    }

    IRIT_PT_COPY(D, D1);
    if ((Pl = UserMarchOnSurface(MarchType, UV, D, Srf, NSrf, DuSrf, DvSrf,
				 Length, FineNess,
				 ClosedInU, ClosedInV)) != NULL)
        IRIT_LIST_PUSH(Pl, TracedPls);

    IRIT_PT_COPY(D, D1);
    IRIT_PT_SCALE(D, -1);
    if ((Pl = UserMarchOnSurface(MarchType, UV, D, Srf, NSrf, DuSrf, DvSrf,
				 Length, FineNess,
				 ClosedInU, ClosedInV)) != NULL)
	IRIT_LIST_PUSH(Pl, TracedPls);

    IRIT_PT_COPY(D, D2);
    if ((Pl = UserMarchOnSurface(MarchType, UV, D, Srf, NSrf, DuSrf, DvSrf,
				 Length, FineNess,
				 ClosedInU, ClosedInV)) != NULL)
	IRIT_LIST_PUSH(Pl, TracedPls);

    IRIT_PT_COPY(D, D2);
    IRIT_PT_SCALE(D, -1);
    if ((Pl = UserMarchOnSurface(MarchType, UV, D, Srf, NSrf, DuSrf, DvSrf,
				 Length, FineNess,
				 ClosedInU, ClosedInV)) != NULL)
	IRIT_LIST_PUSH(Pl, TracedPls);

    if (Mat == NULL)
        IRIT_HMGN_MAT_COPY(SketchMap, IGGlblInvCrntViewMat);
    else
        MatMultTwo4by4(SketchMap, Mat, IGGlblInvCrntViewMat);

    for (Pl = TracedPls; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct *V;

        for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    MatMultPtby4by4(V -> Coord, V -> Coord, SketchMap);
	    MatMultVecby4by4(V -> Normal, V -> Normal, SketchMap);
	    if (!IRIT_PT_EQ_ZERO(V -> Normal))
	        IRIT_PT_NORMALIZE(V -> Normal);
	}
    }

    PRetObj = IPGenPOLYLINEObject(TracedPls);

    return PRetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing of the polygonal object, on the fly if   M
* needed, and display it.                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjSketches:     A sketches object.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchDrawPolygons                                                     M
*****************************************************************************/
void IGSketchDrawPolygons(IPObjectStruct *PObjSketches)
{
    IPObjectStruct *PTmp;

    IGSketchDrawStrokes(IPListObjectGet(PObjSketches, 0));
    if (IGSketchParam.SketchImp &&
	(PTmp = IPListObjectGet(PObjSketches, 1)) != NULL)
        IGSketchDrawStrokes(PTmp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Generates a sketch like drawing for the given polygonal object.          M
*                                                                            *
* PARAMETERS:                                                                M
*   PlObj:      The polygonal object to process.                             M
*   FineNess:   Relative fineness to approximate the sketchs of PlObj with.  M
*   Importance: If TRUE, we should also compute importance for each point.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   The sketch data.	                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGSketchGenPolyImportanceSketches                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchGenPolySketches                                                  M
*****************************************************************************/
IPObjectStruct *IGSketchGenPolySketches(IPObjectStruct *PlObj,
					IrtRType FineNess,
					int Importance)
{
    int n, OldCirc;
    IrtHmgnMatType LightMat, InvLightMat;
    IPObjectStruct *PPtsObj, *PTmp1, *PTmp2,
	*MappedPlObj = NULL;
    IPPolygonStruct *Pl, *MappedPl;
    IPVertexStruct *V,
	*PtSktches = NULL;

    /* Convert to a regular polygonal model with triangles only. */
    PTmp2 = GMTransformObject(PlObj, IGGlblCrntViewMat);

    OldCirc = IPSetPolyListCirc(TRUE);
    GMVrtxListToCircOrLin(PTmp2 -> U.Pl, TRUE);

    for (Pl = PTmp2 -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        if (IPVrtxListLen(Pl ->PVertex) != 3)
	    break;
    }
    if (Pl != NULL) {
        PTmp1 = GMConvertPolysToTriangles(PTmp2);
	IPFreeObject(PTmp2);
	PTmp2 = GMRegularizePolyModel(PTmp1, TRUE);
	IPFreeObject(PTmp1);
    }

    /* Estimate number of points to employ in the point distribution. */
    n = (int) (FineNess * GMPolyObjectArea(PTmp2) *
	       SKETCH_PT_DIST_FINENESS * SURFACE_SKETCH_POINT_DIST);

    PlObj = GMPointCoverOfPolyObj(PTmp2, n, NULL, "_ptsktch");
    IPFreeObject(PTmp2);

    BoolGenAdjacencies(PlObj);
    GMVrtxListToCircOrLin(PlObj -> U.Pl, FALSE);
    IPSetPolyListCirc(OldCirc);

    if (Importance) {
        if (IGSketchParam.SketchImpType == IG_SKETCHING_CURVATURE)
	    GMPlCrvtrSetCurvatureAttr(PlObj -> U.Pl, 1, TRUE);
    }
    else {
	if (IGSketchParam.SketchShdType == IG_SKETCHING_ISOCLINES ||
	    IGSketchParam.SketchShdType == IG_SKETCHING_ORTHOCLINES) {
	    int i;
	    IrtVecType Light0;

	    for (i = 0; i < 3; i++)
		Light0[i] = IGShadeParam.LightPos[0][i];
	    GMGenMatrixZ2Dir(LightMat, Light0);
	    MatInverseMatrix(LightMat, InvLightMat);
	    MappedPlObj = GMTransformObject(PlObj, InvLightMat);
	    GMVrtxListToCircOrLin(MappedPlObj -> U.Pl, TRUE);
	    BoolGenAdjacencies(MappedPlObj);
	    GMVrtxListToCircOrLin(MappedPlObj -> U.Pl, FALSE);
	}

        if (IGSketchParam.SketchSilType == IG_SKETCHING_CURVATURE ||
	    IGSketchParam.SketchShdType == IG_SKETCHING_CURVATURE)
	    GMPlCrvtrSetCurvatureAttr(PlObj -> U.Pl, 1, TRUE);
    }

    MappedPl = MappedPlObj == NULL ? NULL : MappedPlObj -> U.Pl;
    for (Pl = PlObj -> U.Pl;
	 Pl != NULL;
	 Pl = Pl -> Pnext,
	     MappedPl = MappedPl == NULL ? NULL : MappedPl -> Pnext) {
        IPVertexStruct *PtSktch;

	if ((PtSktch = (IPVertexStruct *) AttrGetPtrAttrib(Pl -> Attr,
							"_ptsktch")) != NULL) {
	    for (V =  PtSktch; V != NULL; V = V -> Pnext) {
		if (Importance) {
		    MarchOnPoly(PlObj,
				Pl, V, IGGlblNormalSize,
				SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
				IG_SKETCH_TYPE_IMPORTANCE, NULL);
		}
		else {
		    MarchOnPoly(PlObj,
				Pl, V, IGGlblNormalSize,
				SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
				IG_SKETCH_TYPE_SILHOUETTE, NULL);

		    if (MappedPlObj != NULL) {
		        IrtVecType Nrml;
			IrtPtType Coord;

			IRIT_PT_COPY(Coord, V -> Coord);
			IRIT_VEC_COPY(Nrml, V -> Normal);

		        /* Map the geometry to face light source 0. */
		        MatMultPtby4by4(V -> Coord, Coord, InvLightMat);
			MatMultVecby4by4(V -> Normal, Nrml, InvLightMat);

			MarchOnPoly(MappedPlObj, MappedPl, V, IGGlblNormalSize,
				    SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
				    IG_SKETCH_TYPE_SHADING, LightMat);

			/* Restore the geometry. */
			IRIT_PT_COPY(V -> Coord, Coord);
			IRIT_VEC_COPY(V -> Normal, Nrml);
		    }
		    else {
			MarchOnPoly(PlObj, Pl, V, IGGlblNormalSize,
				    SKETCH_PT_DIST_FINENESS * STROKE_POLY_FINE_NESS,
				    IG_SKETCH_TYPE_SHADING, NULL);
		    }
		}

		/* Recover position and normal of vertex. */
		MatMultPtby4by4(V -> Coord, V -> Coord, IGGlblInvCrntViewMat);
		MatMultVecby4by4(V -> Normal, V -> Normal,
				 IGGlblInvCrntViewMat);
		if (!IRIT_PT_EQ_ZERO(V -> Normal))
		    IRIT_PT_NORMALIZE(V -> Normal);
	    }

	    AttrFreeOneAttribute(&Pl -> Attr, "_ptsktch");
	    PtSktches = IPAppendVrtxLists(PtSktch, PtSktches);

	}
    }

    IPFreeObject(PlObj);

    if (MappedPlObj != NULL)
	IPFreeObject(MappedPlObj);

    PPtsObj = IPGenPOLYLINEObject(IPAllocPolygon(0, PtSktches, NULL));

    return PPtsObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Precompute the strokes of the given polygonal model, assuming only       *
* trinagles, along the prescribed march.				     *
*   Up to four strokes are developed and placed as "_*strokes" attribute     *
* under the Vertex V as well as returned.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:	This polygonal object.                                       *
*   PlHead:     Triangle from where we start the marching.                   *
*   VHead:      Vertex from where we start the marching.                     *
*   Length:     Of stroke (real arc length).                                 *
*   FineNess:   Of marching steps on the surface.			     *
*   SketchType: Sketching type - silhouette, shading, importance, etc.	     *
*   Mat:        Additional transformation required to sketches, if not NULL. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Traced strokes as piecewise linear approximations.    *
*****************************************************************************/
static void MarchOnPoly(IPObjectStruct *PObj,
			IPPolygonStruct *PlHead,
			IPVertexStruct *VHead,
			IrtRType Length,
			IrtRType FineNess,
			int SketchType,
			IrtHmgnMatType Mat)
{
    IPObjectStruct *PRetObj;

    switch (SketchType) {
	case IG_SKETCH_TYPE_SILHOUETTE:
            PRetObj = MarchOnPolyOnce(PObj, PlHead, VHead, Length, FineNess,
				      IGSketchParam.SketchSilType, Mat);

	    AttrSetObjAttrib(&VHead -> Attr, "_SilStrokes", PRetObj, FALSE);
	    break;
	case IG_SKETCH_TYPE_SHADING:
	    PRetObj = MarchOnPolyOnce(PObj, PlHead, VHead, Length, FineNess,
				      IGSketchParam.SketchShdType, Mat);

	    AttrSetObjAttrib(&VHead -> Attr, "_ShdStrokes", PRetObj, FALSE);
	    break;
	case IG_SKETCH_TYPE_IMPORTANCE:
	    PRetObj = MarchOnPolyOnce(PObj, PlHead, VHead, Length, FineNess,
				      IGSketchParam.SketchImpType, Mat);

	    AttrSetObjAttrib(&VHead -> Attr, "_ImpStrokes", PRetObj, FALSE);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Precompute the strokes of the given polygonal model, assuming only       *
* trinagles, along the prescribed march.				     *
*   Up to four strokes are developed and placed as "_*strokes" attribute     *
* under the Vertex V as well as returned.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:	This polygonal object.                                       *
*   PlHead:     Triangle from where we start the marching.                   *
*   VHead:      Vertex from where we start the marching.                     *
*   Length:     Of stroke (real arc length).                                 *
*   FineNess:   Of marching steps on the surface.			     *
*   SketchType: Type of strokes to develope for this sketch.		     *
*   Mat:        Additional transformation required to sketches, if not NULL. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Traced strokes as piecewise linear approximations.    *
*****************************************************************************/
static IPObjectStruct *MarchOnPolyOnce(IPObjectStruct *PObj,
				       IPPolygonStruct *PlHead,
				       IPVertexStruct *VHead,
				       IrtRType Length,
				       IrtRType FineNess,
				       int SketchType,
				       IrtHmgnMatType Mat)
{
    IrtRType *Weights,
	Scl = 1.0 / (IRIT_EPS + MatScaleFactorMatrix(IGGlblInvCrntViewMat));
    IrtHmgnMatType SketchMap;
    IPVertexStruct *V[3];
    IPPolygonStruct
	*TracedPls = NULL;
    IPObjectStruct *PTracedObj;

    V[0] = PlHead -> PVertex;
    V[1] = V[0] -> Pnext;
    V[2] = V[1] -> Pnext;

    Weights = GMBaryCentric3Pts(V[0] -> Coord,
				V[1] -> Coord,
				V[2] -> Coord,
				VHead -> Coord);

    /* Estimate a normal. */
    IRIT_VEC_BLEND_BARYCENTRIC(VHead -> Normal, V[0] -> Normal,
			  V[1] -> Normal, V[2] -> Normal, Weights);

    switch (SketchType) {
	default:
	case IG_SKETCHING_ISO_PARAM:
	    TracedPls = UserMarchOnPolygons(PObj, USER_SRF_MARCH_ISO_PARAM,
					    PlHead, VHead, Length * Scl);
	    break;
	case IG_SKETCHING_CURVATURE:
	    TracedPls = UserMarchOnPolygons(PObj, USER_SRF_MARCH_PRIN_CRVTR,
					    PlHead, VHead, Length * Scl);
	    break;
	case IG_SKETCHING_ISOCLINES:
	    TracedPls = UserMarchOnPolygons(PObj, USER_SRF_MARCH_ISOCLINES,
					    PlHead, VHead, Length * Scl);
	    break;
	case IG_SKETCHING_ORTHOCLINES:
	    TracedPls = UserMarchOnPolygons(PObj, USER_SRF_MARCH_ORTHOCLINES,
					    PlHead, VHead, Length * Scl);
	    break;
    }

    if (Mat == NULL)
        IRIT_HMGN_MAT_COPY(SketchMap, IGGlblInvCrntViewMat);
    else
        MatMultTwo4by4(SketchMap, Mat, IGGlblInvCrntViewMat);

    PTracedObj = IPGenPOLYLINEObject(GMTransformPolyList(TracedPls,
							 SketchMap, FALSE));
    IPFreePolygonList(TracedPls);

    return PTracedObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for GMPolyAdjacncyVertex to process every edge of a   *
* given vertex.  The edge is provided as (V, V -> Pnext)		     *
*                                                                            *
* PARAMETERS:                                                                *
*   V1, V2:    Two vertices defining this edge.  Note the vertices are NOT   *
*	       necessarily chained together into a list.		     *
*   Pl1, Pl2:  The two polygons that share this edge.  The edge (V1, V2) is  *
*	       in both Pl1 and Pl2, with not necessarily the exact pointers  *
*	       IPVertexStruct of V1 and V2.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ProcessVertexImportance(IPVertexStruct *V1,
				    IPVertexStruct *V2,
				    IPPolygonStruct *Pl1,
				    IPPolygonStruct *Pl2)
{
    if (!IRIT_PT_APX_EQ_EPS(V1 -> Coord, GlblVrtxImportance -> Coord, IRIT_EPS) &&
	!IRIT_PT_APX_EQ_EPS(V2 -> Coord, GlblVrtxImportance -> Coord, IRIT_EPS))
        IRIT_WARNING_MSG("Edge does not match the given vertex - adj error!\n");

    if (Pl1 != NULL && Pl2 != NULL) {
        GlblVrtxImportanceCount++;
	GlblVrtxImportanceVal += acos(IRIT_DOT_PROD(Pl1 -> Plane, Pl2 -> Plane));
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Precompute the strokes that emphasize important features in the geometry M
* using an importance measure that looks at the diherdal angle of adjacent   M
* polygons.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:         Polygonal model to process for importance.	             M
*   SketchParams: NPR sketch parameters' info structure.		     M
*   FineNess:     Of marching steps on the surface.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Traced strokes as piecewise linear approximations.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   IGSketchGenPolySketches                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IGSketchGenPolyImportanceSketches                                        M
*****************************************************************************/
IPObjectStruct *IGSketchGenPolyImportanceSketches(IPObjectStruct *PObj,
					    IGSketchParamStruct *SketchParams,
					    IrtRType FineNess)
{
    IPObjectStruct *PSketch,
        *PTmp = IPCopyObject(NULL, PObj, FALSE);
    VoidPtr
	PAdj = GMPolyAdjacncyGen(PTmp, IRIT_EPS);
    IPPolygonStruct *Pl;

    for (Pl = PTmp -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        int PlImportanceCount = 0;
        IrtRType
	    PlImportanceVal = 0.0;
        IPVertexStruct *V;

	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    GlblVrtxImportanceVal = 0.0;
	    GlblVrtxImportanceCount = 0;
	    GlblVrtxImportance = V;

	    GMPolyAdjacncyVertex(V, PAdj, ProcessVertexImportance);

	    if (GlblVrtxImportanceCount > 0) {
		GlblVrtxImportanceVal /= (GlblVrtxImportanceCount * M_PI);
		GlblVrtxImportanceVal = SKTCH_IMP_FACTOR *
					pow(GlblVrtxImportanceVal,
					    SketchParams -> SketchImpDecay);
		if (SketchParams -> SketchImpFrntSprt < 90.0 &&
		    IP_HAS_NORMAL_VRTX(V)) {
		    static IrtRType
		        ImpFrntSprt = -1,
		        CosImpFrntSprt = 0;
		    IrtVecType N;

		    /* Cache the cosine of the angle se we do not recompute  */
		    /* it for every vertex.				     */
		    if (ImpFrntSprt != SketchParams -> SketchImpFrntSprt) {
		        ImpFrntSprt = SketchParams -> SketchImpFrntSprt;
		        CosImpFrntSprt = cos(IRIT_DEG2RAD(ImpFrntSprt));
		    }

		    /* Clip importance - bounding angular view deviation. */
		    MatMultVecby4by4(N, V -> Normal, IGGlblCrntViewMat);
		    IRIT_PT_NORMALIZE(N);
		    if (IRIT_FABS(N[2]) < CosImpFrntSprt)
		        GlblVrtxImportanceVal = 0.0;
		}

		AttrSetRealAttrib(&V -> Attr, "Imprt", GlblVrtxImportanceVal);
		PlImportanceVal += GlblVrtxImportanceVal;
		PlImportanceCount++;
	    }
	    else
	        IRIT_WARNING_MSG("Failed to compute Importance for vertex\n");
	}

	if (PlImportanceCount > 0) {
	    PlImportanceVal /= PlImportanceCount;
	    AttrSetRealAttrib(&Pl -> Attr, "Imprt", PlImportanceVal);
	}
	else
	    IRIT_WARNING_MSG("Failed to compute Importance for polygon\n");
    }

    GMPolyAdjacncyFree(PAdj);

    /* Now develope strokes to follow the computed importance factors: */
    PSketch = IGSketchGenPolySketches(PTmp, FineNess, TRUE);
    IPFreeObject(PTmp);

    return PSketch;
}
