/******************************************************************************
* visible.c - surface visibility analysis routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 99.					      *
******************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "allocate.h"
#include "attribut.h"
#include "iritprsr.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "geom_lib.h"
#include "rndr_lib.h"
#include "user_loc.h"

#define USER_MOLD_MATCH_REDUCE	0.2
#define USER_MIN_LEN_POLY	1e-3

static IPPolygonStruct *UserMoldMatchPolyline(IPPolygonStruct *Pl,
					      IPPolygonStruct **Pls);
static CagdSrfStruct *UserMoldRuledSrf(const CagdSrfStruct *Srf,
				       const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2, 
				       CagdCrvStruct *Reparam,
				       int Reparam2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a decomposition of surface Srf into regions that are visible    M
* with normals deviating in a cone of size as set via ConeAngle.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To decompose into visibility cones.                         M
*   Resolution:  Of polygonal approximation of surface Srf.                  M
*   ConeAngle:   Of coverage over the unit sphere.  ConeAngle prescribes the M
*		 opening angle of the cone. In Radians.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A list of trimmed surface regions of surface Srf that M
*		       are visible from a selected direction (available as   M
*		       "VIewDir" attribute on them) ad that the union of the M
*		       list covers the entire surface Srf.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserViewingConeSrfDomains, UserVisibilityClassify                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfVisibConeDecomp                                                   M
*****************************************************************************/
IPObjectStruct *UserSrfVisibConeDecomp(const CagdSrfStruct *Srf,
				       CagdRType Resolution,
				       CagdRType ConeAngle)
{
    CagdRType
	ConeSize = sin(ConeAngle);
    CagdSrfStruct
        *NSrf = SymbSrfNormalSrf(Srf);
    IPObjectStruct *PTmp,
	*RetObjs = NULL,
        *SpherePtsDist = GMPointCoverOfUnitHemiSphere(ConeSize),
        *SrfDomains = UserViewingConeSrfDomains(Srf,
						NSrf,
						SpherePtsDist -> U.Pl,
						Resolution,
						ConeAngle,
						FALSE);

    for (PTmp = SrfDomains; PTmp != NULL; PTmp = PTmp -> Pnext) {
	TrimSrfStruct
	    *TSrfList = TrimSrfsFromContours(Srf, PTmp -> U.Pl);
	IPObjectStruct *PObj,
	    *ViewDirObj = AttrGetObjectObjAttrib(PTmp, "ViewDir");

	/* Eliminate the trimmed surfaces in PObj that are outside the     */
	/* viewing cone, in place.					   */
	TSrfList = UserVisibilityClassify(
				    AttrGetObjectObjAttrib(PTmp, "SclrSrf"),
				    TSrfList);
	if (TSrfList != NULL) {
	    PObj = IPGenTRIMSRFObject(TSrfList);
	    if (ViewDirObj != NULL)
	        AttrSetObjectObjAttrib(PObj, "ViewDir", ViewDirObj, TRUE);
	    IRIT_LIST_PUSH(PObj, RetObjs);
	}
    }

    CagdSrfFree(NSrf);
    IPFreeObjectList(SrfDomains);
    IPFreeObject(SpherePtsDist);

    return RetObjs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a decomposition of surface Srf into a set of TrimmedSrfs into      M
* regions that are inside the normal viewing cone as prescribed by SclrSrf,  M
* eliminate all regions (trimmed surfaces) that are outside the viewing      M
*  cone in the given list, TrimmedSrfs.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   SclrSrf:	   The scalar surface computed symbollically in		     M
*		   UserViewingConeSrfDomains, forming this decomosition.     M
*   TrimmedSrfs:   To verify they are within the viewing code of ViewingDir. M
*		   Trimmed surfaces inside this list that are outside the    M
*		   viewing cone are eliminated, in place.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *: The trimmed regions inside the viewing cone, as a       M
*		   subset of TrimmedSrfs.			             M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserViewingConeSrfDomains, UserSrfVisibConeDecomp                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserVisibilityClassify                                                   M
*****************************************************************************/
TrimSrfStruct *UserVisibilityClassify(const IPObjectStruct *SclrSrf,
				      TrimSrfStruct *TrimmedSrfs)
{
    TrimSrfStruct *TSrf,
	*PrevTSrf = NULL;

    if (SclrSrf == NULL || !IP_IS_SRF_OBJ(SclrSrf)) {
	UserFatalError(USER_ERR_MISSING_ATTRIB);
	return NULL;
    }

    for (TSrf = TrimmedSrfs; TSrf != NULL; ) {
	CagdRType
	    *UVInside = TrimPointInsideTrimmedCrvs(TSrf -> TrimCrvList, TSrf),
	    *R = CagdSrfEval(SclrSrf -> U.Srfs, UVInside[0], UVInside[1]);

	if (R[1] > 0) {
	    if (PrevTSrf != NULL) {
		PrevTSrf -> Pnext = TSrf -> Pnext;
		TrimSrfFree(TSrf);
		TSrf = PrevTSrf -> Pnext;
	    }
	    else {
		TrimmedSrfs = TrimmedSrfs -> Pnext;
		TrimSrfFree(TSrf);
		TSrf = TrimmedSrfs;
	    }
	}
	else {
	    PrevTSrf = TSrf;
	    TSrf = TSrf ->Pnext;
	}
    }

    return TrimmedSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the domain in given surface Srf that is inside the given cones  M
* of directions ConeDirs and opening angle ConeAngle.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           To compute its visibility.                                M
*   NSrf:	   The normal surface computed symbollically for Srf.        M
*   ConeDirs:      Direction of cone's axes.                                 M
*   Resolution:    Of Cone - normal surface intersection, for polygonal      M
*		   approximation accuracy.				     M
*   ConeAngle:     The opening angle of the cone, in Radians.		     M
*   Euclidean:     Contours are in Euclidean (TRUE) or parametric (FALSE)    M
*		   space of Srf.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Set of piecewise linear contours, approximating the  M
*		   intersections of the normal viewing cones and the         M
*		   original surface.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserVisibilityClassify, UserSrfVisibConeDecomp                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserViewingConeSrfDomains                                                M
*****************************************************************************/
IPObjectStruct *UserViewingConeSrfDomains(const CagdSrfStruct *Srf,
					  const CagdSrfStruct *NSrf,
					  const IPPolygonStruct *ConeDirs,
					  CagdRType Resolution,
					  CagdRType ConeAngle,
					  CagdRType Euclidean)
{
    int OldCirc;
    IPVertexStruct
        *VPts = ConeDirs -> PVertex;
    IPObjectStruct
	*CntrObjList = NULL;

    if (ConeAngle < 0.0 || ConeAngle >= M_PI / (2 + IRIT_EPS)) {
	USER_FATAL_ERROR(USER_ERR_WRONG_ANGLE);
	return NULL;
    }

    for ( ; VPts != NULL; VPts = VPts -> Pnext) {
	IRIT_STATIC_DATA IrtPlnType
	    Plane = { 1.0, 0.0, 0.0, 0.0 };
	int i;
	CagdBType HasPosVals, HasNegVals;
	CagdRType *R;
	IrtHmgnMatType Mat, InvMat;
	CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *TmpSrf, *SclrSrf;

	GMGenMatrixZ2Dir(Mat, VPts -> Coord);
	MatTranspMatrix(Mat, InvMat);		     /* Compute the inverse. */

	TmpSrf = CagdSrfMatTransform(NSrf, InvMat);

	SymbSrfSplitScalar(TmpSrf, &SrfW, &SrfX, &SrfY, &SrfZ);
	CagdSrfFree(TmpSrf);
	if (SrfW != NULL) {
	    USER_FATAL_ERROR(USER_ERR_RATIONAL_NO_SUPPORT);
	    return NULL;
	}

	TmpSrf = SymbSrfMult(SrfX, SrfX);
	CagdSrfFree(SrfX);
	SrfX = TmpSrf;

	TmpSrf = SymbSrfMult(SrfY, SrfY);
	CagdSrfFree(SrfY);
	SrfY = TmpSrf;

	TmpSrf = SymbSrfMult(SrfZ, SrfZ);
	CagdSrfFree(SrfZ);
	SrfZ = SymbSrfScalarScale(TmpSrf, IRIT_SQR(tan(ConeAngle)));
	CagdSrfFree(TmpSrf);

	TmpSrf = SymbSrfAdd(SrfX, SrfY);
	CagdSrfFree(SrfX);
	CagdSrfFree(SrfY);

	SclrSrf = SymbSrfSub(TmpSrf, SrfZ);
	CagdSrfFree(TmpSrf);
	CagdSrfFree(SrfZ);

	R = SclrSrf -> Points[1];
	HasPosVals = HasNegVals = FALSE;
	for (i = SclrSrf -> ULength * SclrSrf -> VLength; i >= 0; i--) {
	    if (*R > 0.0)
	        HasPosVals = TRUE;
	    if (*R++ < 0.0)
	        HasNegVals = TRUE;
	}
	if (HasPosVals && HasNegVals) {
	    IPPolygonStruct *Cntrs, *Cntr;

	    OldCirc = IPSetPolyListCirc(TRUE);
	    Cntrs = UserCntrSrfWithPlane(SclrSrf, Plane, Resolution);
	    IPSetPolyListCirc(OldCirc);

	    if (Cntrs != NULL) {
		IPObjectStruct *CntrObj, *VecObj, *SclrSrfObj;
		IPVertexStruct *V;

		if (Euclidean) {
		    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
			for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
			    CagdRType
			        *P = CagdSrfEval(Srf, V -> Coord[1],
						      V -> Coord[2]);

			    CagdCoerceToE3(V -> Coord, &P, -1, Srf -> PType);
			}
		    }
		}
		else {
		    for (Cntr = Cntrs; Cntr != NULL; Cntr = Cntr -> Pnext) {
			for (V = Cntr -> PVertex; V != NULL; V = V -> Pnext) {
			    V -> Coord[0] = V -> Coord[1];
			    V -> Coord[1] = V -> Coord[2];
			    V -> Coord[2] = 0.0;
			}
		    }
		}

		CntrObj = IPGenPOLYObject(Cntrs);
		IP_SET_POLYLINE_OBJ(CntrObj);

		/* Save the viewing direction as an attribute. */
		VecObj = IPGenVECObject(&VPts -> Coord[0],
					&VPts -> Coord[1],
					&VPts -> Coord[2]);
		AttrSetObjectObjAttrib(CntrObj, "ViewDir", VecObj, FALSE);

		SclrSrfObj = IPGenSRFObject(SclrSrf);
		AttrSetObjectObjAttrib(CntrObj, "SclrSrf", SclrSrfObj, FALSE);

		IRIT_LIST_PUSH(CntrObj, CntrObjList);
	    }
	    else {
	        CagdSrfFree(SclrSrf);
	    }
	}
	else {
	    CagdSrfFree(SclrSrf);
	}
    }

    return CntrObjList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the views on the unit sphere at which the number of silhouette  M
* curves is changing.  This is a subset of the events typically considered   M
* in aspect graphs and, for example, events such as silhouette cusps are not M
* considered.								     M
*   The viewing directions where the number of silhouette curves/loops can   M
* be modified are views in the tangent plane of the surface at parabolic     M
* points of the surfaces, viewed from the zero principal curvatures'         M
* direction. 								     M
*   Hence, we simply derive the parabolic lines of the surface and then      M
* remap them to the direction of the zero principal curvature there.         M
*                                                                            *
* PARAMETERS:                                                                M
*   PSrf:       To compute the aspect graph's topology partitioning for.     M
*   Tolerance:  Accuracy of parabolic piecewise linear approximation.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Polylines on the unit sphere, depicting the aspect  M
*		graph's partitioning lines.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfGaussCurvature, UserCntrSrfWithPlane, MvarSrfSilhInflections      M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfTopoAspectGraph                                                   M
*****************************************************************************/
IPPolygonStruct *UserSrfTopoAspectGraph(CagdSrfStruct *PSrf,
					CagdRType Tolerance)
{
    IRIT_STATIC_DATA IrtPlnType
	GaussPlane = { 1.0, 0.0, 0.0, 1.050964e-10 };
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdSrfStruct
	*GaussSrf = SymbSrfGaussCurvature(PSrf, TRUE);
    IPPolygonStruct *Pl, *Plls2,
	*Plls = UserCntrSrfWithPlane(GaussSrf, GaussPlane, Tolerance);

    CagdSrfFree(GaussSrf);

    BspMultComputationMethod(OldInterpFlag);

    if (Plls == NULL)
	return NULL;

    /* Evaluate the principal curvatures/directions along the parabolic     */
    /* curve.  This is somewhat redundant as we have the parabolic lines    */
    /* already but is more robust to precisely evaluate the surface there.  */
    SymbEvalSrfCurvPrep(PSrf, TRUE);
    for (Pl = Plls; Pl != NULL; Pl = Pl -> Pnext) {
	CagdRType K1, K2;
	CagdVType D1, D2, V1;
	IPVertexStruct *V,
	    *VPrev = NULL;

	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    SymbEvalSrfCurvature(PSrf, V -> Coord[1], V -> Coord[2], FALSE,
				 &K1, &K2, D1, D2);

	    /* Copy direction with zero curvature as the viewing direction. */
	    if (IRIT_FABS(K1) < IRIT_FABS(K2)) {
		IRIT_VEC_COPY(V -> Coord, D1);
	    }
	    else {
		IRIT_VEC_COPY(V -> Coord, D2);
	    }

	    /* Check the orientation for consistency.		            */
	    if (VPrev != NULL) {
		IRIT_VEC_COPY(V1, VPrev -> Coord);
		IRIT_VEC_SCALE(V1, -1);

		if (IRIT_DOT_PROD(V -> Coord, V1) >
		    IRIT_DOT_PROD(V -> Coord, VPrev -> Coord))
		    IRIT_VEC_SCALE(V -> Coord, -1);
	    }

	    VPrev = V;
	} 
    }
    SymbEvalSrfCurvPrep(PSrf, FALSE);

    /* For each polyline with +V vertices, duplicate and make -V vertices. */
    Plls2 = IPCopyPolygonList(Plls);
    for (Pl = Plls2; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct *V;

	for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    IRIT_VEC_SCALE(V -> Coord, -1);
	}
    }
    IPGetLastPoly(Plls) -> Pnext = Plls2;

    return Plls;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the regions of planar curve Crv that are visible from a view    M
* point (View[2] == 1) or direction (View[2] == 0) View (x, y, w).  Return   M
* is a list of visible curve segments.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:      Planar curve to compute its visible regions from View.        M
*   View:      As (x, y, w) where w == 0 for parallel view direction and     M
*	       w = 1 for a point view.					     M
*   Tolerance: Of computation. 						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   List of curve segments of Crv that are visible from   M
*	               View.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvOMDiagExtreme, UserCrvAngleMap, UserCrvViewMap                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvVisibleRegions                                                    M
*****************************************************************************/
CagdCrvStruct *UserCrvVisibleRegions(const CagdCrvStruct *CCrv,
				     const CagdRType *View,
				     CagdRType Tolerance)
{
    int Proximity;
    CagdRType *R;
    CagdVType V, RayDir;
    CagdPType RayPt;
    CagdPtStruct *Pts1, *Pts2, *Pt;
    CagdCrvStruct *Crv, *TCrv, *DCrv, *Crvs, *AllCrv,
	*RetCrvs = NULL;

    if (CAGD_IS_BEZIER_CRV(CCrv))
        Crv = CagdCnvrtBzr2BspCrv(CCrv);
    else if (CAGD_IS_PERIODIC_CRV(CCrv))
        Crv = CagdCnvrtPeriodic2FloatCrv(CCrv);
    else
	Crv = CagdCrvCopy(CCrv);

    if (!BspCrvHasOpenEC(Crv)) {
        TCrv = CagdCnvrtFloat2OpenCrv(Crv);
	CagdCrvFree(Crv);
	Crv = TCrv;
    }
    AllCrv = CagdCrvCopy(Crv);

    /* Stage 1: find at all locations on the curve where the view line is   */
    /* tangent to curve Crv.						    */
    DCrv = CagdCrvDerive(Crv);

    if (IRIT_APX_EQ(View[2], 0.0)) {    /* View is a parallel viewing direction. */
	/* Compute a normal direction to View and derive the zeros of the   */
	/* dot-product with the tangent field.				    */
        V[0] =  View[1];
        V[1] = -View[0];
	V[2] = 0.0;
	TCrv = SymbCrvVecDotProd(DCrv, V);
	Pts1 = SymbCrvZeroSet(TCrv, 1, Tolerance, TRUE);
	CagdCrvFree(TCrv);
    }
    else {				       /* View is a point location. */
        CagdCrvStruct *TCrvW, *TCrvX, *TCrvY, *TCrvZ;

	/* Compute the zeros of < C - View, C' > */
        V[0] = -View[0];
        V[1] = -View[1];
	V[2] = 0.0;
        CagdCrvTransform(Crv, V, 1.0);
	TCrv = SymbCrvCrossProd(DCrv, Crv);
	SymbCrvSplitScalar(TCrv, &TCrvW, &TCrvX, &TCrvY, &TCrvZ);
	CagdCrvFree(TCrv);
	CagdCrvFree(TCrvW);
	CagdCrvFree(TCrvX);
	CagdCrvFree(TCrvY);
	Pts1 = SymbCrvZeroSet(TCrvZ, 1, Tolerance, TRUE);
	CagdCrvFree(TCrvZ);
    }

    CagdCrvFree(DCrv);
    CagdCrvFree(Crv);

    /* Stage 2: find all locations on the curve where the view line through */
    /* a detected curve tangency point in stage 1 pierces another curve     */
    /* location behind the tangency location.				    */
    for (Pt = Pts1, Pts2 = NULL; Pt != NULL; Pt = Pt -> Pnext) {
	R = CagdCrvEval(AllCrv, Pt -> Pt[0]);
	CagdCoerceToE2(V, &R, -1, AllCrv -> PType);

        if (IRIT_APX_EQ(View[2], 0.0)) {/* View is a parallel viewing direction. */
	    RayDir[0] = -View[0];
	    RayDir[1] = -View[1];
	}
	else {				       /* View is a point location. */
	    IRIT_PT2D_SUB(RayDir, V, View);
	}
	IRIT_PT2D_COPY(RayPt, V);

	/* Compute the curve view-line intersections and keep those that    */
	/* are behind the tangency location.				    */
	Pts2 = CagdListAppend(SymbCrvRayInter(AllCrv, RayPt,
					      RayDir, Tolerance),
			      Pts2);
    }

    /* Merge the parametric locations, sort them out, split the curves and  */
    /* test their visibility.						    */
    Pts1 = CagdPtsSortAxis(CagdListAppend(Pts1, Pts2), 1);
    if ((Crvs = CagdCrvSubdivAtParams(AllCrv, Pts1,
				      0.0, &Proximity)) == AllCrv)
	AllCrv = CagdCrvCopy(Crvs);	      /* No subdivision took place. */
    CagdPtFreeList(Pts1);

    while (Crvs != NULL) {
        CagdRType TMin, TMax, DSqr;
	CagdPType PtE2;

	IRIT_LIST_POP(Crv, Crvs);

	/* Eval the curve segment in the middle and see if a ray from this  */
	/* middle point to the viewer intersect someone.		    */
	CagdCrvDomain(Crv, &TMin, &TMax);
	R = CagdCrvEval(Crv, (TMin + TMax) * 0.5);
	CagdCoerceToE2(RayPt, &R, -1, Crv -> PType);
	
        if (IRIT_APX_EQ(View[2], 0.0)) {/* View is a parallel viewing direction. */
	    IRIT_VEC2D_COPY(RayDir, View);
	    DSqr = -1.0;
	}
	else {				       /* View is a point location. */
	    IRIT_PT2D_SUB(RayDir, View, RayPt);
	    DSqr = IRIT_PT2D_DIST_SQR(View, RayPt);
	}

	if ((Pts1 = SymbCrvRayInter(AllCrv, RayPt,
				    RayDir, Tolerance)) != NULL) {
	    for (Pt = Pts1; Pt != NULL; Pt = Pt -> Pnext) {
	        R = CagdCrvEval(AllCrv, Pt -> Pt[0]);
		CagdCoerceToE2(PtE2, &R, -1, Crv -> PType);

		if (IRIT_APX_EQ(View[2], 0.0)) {  /* Parallel viewing direction. */
		    if (!IRIT_PT_APX_EQ_E2(PtE2, RayPt))
		        break;
		}
		else {			       /* View is a point location. */
		    if (IRIT_PT2D_DIST_SQR(PtE2, RayPt) < DSqr)
		        break;
		}
	    }
	    if (Pt == NULL) {
	        /* All intersection points are behind the View point. */
	        IRIT_LIST_PUSH(Crv, RetCrvs);
	    }
	    else 
	        CagdCrvFree(Crv);

	    CagdPtFreeList(Pts1);
	}
	else
	    IRIT_LIST_PUSH(Crv, RetCrvs);
    }

    CagdCrvFree(AllCrv);

    return RetCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the parallel/orthogonality/angular map of the given planar      M
* closed curve.							             M
*   The orthogonality map is the set of pairs of points of Crv that have     M
* an orthogonal normal and is a 2D map of size [D x D] where D is the domain M
* of Crv.								     M
*   Similarly the parallel map is the set of pairs of points of Crv that     M
* have the same normal direction, and the angular map identifies pairs of    M
* points with a prescribed fixed angle between their normals.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Planar closed curve to compute its para/ortho/angular map.    M
*   Tolerance: Of computation.  If negative the function whose zero set is   M
*	       the orthogonal map, is returned. 			     M
*   Angle:     0 for parallel maps, 90 for orthogonal maps, or general       M
*	       for general angles.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Polyline(s) of (r, t) points such that the normals   M
*	       at C(r) and C(t) are orthogonal.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvOMDiagExtreme, UserCrvViewMap, UserCrvVisibleRegions              M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvAngleMap                                                          M
*****************************************************************************/
IPObjectStruct *UserCrvAngleMap(const CagdCrvStruct *Crv,
				CagdRType Tolerance,
				CagdRType Angle)
{
    IRIT_STATIC_DATA IrtPlnType
	XPlane = { 1.0, 0.0, 0.0, 1.190886e-10 };
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    IPPolygonStruct *Plls;
    CagdCrvStruct *DCrv, *CpCrv;
    CagdSrfStruct *USrf, *VSrf, *TSrf, *ProdSrf, *WSrf, *XSrf, *YSrf, *ZSrf;

    if (CAGD_IS_PERIODIC_CRV(Crv))
        Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
    else
	CpCrv = NULL;

    DCrv = CagdCrvDerive(Crv);
    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    USrf = CagdPromoteCrvToSrf(DCrv, CAGD_CONST_U_DIR);
    VSrf = CagdPromoteCrvToSrf(DCrv, CAGD_CONST_V_DIR);
    CagdCrvFree(DCrv);

    if (IRIT_APX_EQ(Angle, 0)) {
	TSrf = SymbSrfCrossProd(USrf, VSrf);

	/* Extract the Z component of the cross product. */
        SymbSrfSplitScalar(TSrf, &WSrf, &XSrf, &YSrf, &ZSrf);
	CagdSrfFree(TSrf);
	if (XSrf != NULL)
	    CagdSrfFree(XSrf);
	if (YSrf != NULL)
	    CagdSrfFree(YSrf);

	if (WSrf != NULL) {
	    ProdSrf = SymbSrfMergeScalar(WSrf, ZSrf, NULL, NULL);
	    CagdSrfFree(WSrf);
	    CagdSrfFree(ZSrf);
	}
        else
	    ProdSrf = ZSrf;
    }
    else if (IRIT_APX_EQ(Angle, 90)) {
        ProdSrf = SymbSrfDotProd(USrf, VSrf);
    }
    else {
        CagdSrfStruct *TSrf2, *TSrf3, *TSrf4;

        /* Compute the cosine of the angle square between the vectors and  */
        /* as <V1, V2>^2 / (<V1, V1> <V2, V2>).				   */
        TSrf = SymbSrfDotProd(USrf, VSrf);
	TSrf4 = SymbSrfMult(TSrf, TSrf);
	CagdSrfFree(TSrf);
	if (CAGD_IS_RATIONAL_SRF(TSrf4)) {
	    SymbSrfSplitScalar(TSrf4, &WSrf, &XSrf, &YSrf, &ZSrf);
	    TSrf = XSrf;
	    CagdSrfFree(WSrf);
	    CagdSrfFree(TSrf4);
	}
	else
	    TSrf = TSrf4;

	TSrf2 = SymbSrfDotProd(USrf, USrf);
	TSrf3 = SymbSrfDotProd(VSrf, VSrf);
	TSrf4 = SymbSrfMult(TSrf2, TSrf3);
	CagdSrfFree(TSrf2);
	CagdSrfFree(TSrf3);
	if (CAGD_IS_RATIONAL_SRF(TSrf4)) {
	    SymbSrfSplitScalar(TSrf4, &WSrf, &XSrf, &YSrf, &ZSrf);
	    CagdSrfFree(WSrf);
	    CagdSrfFree(TSrf4);
	    TSrf4 = XSrf;
	}

	/* And subtract (compare with) cos^2(Angle). */
	CagdSrfTransform(TSrf4, NULL, IRIT_SQR(cos(IRIT_DEG2RAD(Angle))));

	ProdSrf = SymbSrfSub(TSrf, TSrf4);
	CagdSrfFree(TSrf);
	CagdSrfFree(TSrf4);
    }

    CagdSrfFree(USrf);
    CagdSrfFree(VSrf);

    BspMultComputationMethod(OldInterpFlag);

    if (Tolerance <= 0.0)
	return IPGenSRFObject(ProdSrf);

    Plls = UserCntrSrfWithPlane(ProdSrf, XPlane, Tolerance);

    CagdSrfFree(ProdSrf);

    return IPGenPOLYLINEObject(Plls);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the view map of the given planar closed curve, C(t), with       M
* respect to view directions that are prescribed by vector curve	     M
* ViewCrv(Theta).							     M
*   The view map is defined as the zero of M = < N(t), ViewCrv(Theta) >,     M
* where N(t) is the normal field of C(t).				     M
*   Seeking the visible portions, the problem could be partially addressed   M
* using the following constraints:					     M
* 	< C(t) - C(r), ViewCrv'(Theta) > = 0,				     V
* finding all points C(t) behind or in front C(r), for some r, such that     M
*       < C(t) - C(r), ViewCrv(Theta) > > 0,				     V
* for all matched r values.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Planar closed curve to compute its view map.		     M
*   ViewCrv:   Planar viewing direction curve in vector space.		     M
*   SubTol:    Used only if TrimInvisible TRUE to control the subdivision    M
*	       Tolerance of the solver.	  If negative, the solution set is   M
*              returned as a cloud of points.				     M
*   NumTol:    Of computation.  If negative the function M whose zero set is M
*	       the view map, is returned.	 			     M
*   TrimInvisible:  If TRUE, trim the regions that are invisible from V.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:   Polyline(s) of (Theta, t) or (Theta, t, r) points    M
*              such that normals at C(t) are orthogonal to ViewCrv(Theta),   M
*              and C(t) exactly in front of C(r) if (Theta, t, r).           M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvOMDiagExtreme, UserCrvAngleMap, UserCrvVisibleRegions             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvViewMap                                                           M
*****************************************************************************/
IPObjectStruct *UserCrvViewMap(const CagdCrvStruct *Crv,
			       const CagdCrvStruct *ViewCrv,
			       CagdRType SubTol,
			       CagdRType NumTol,
			       CagdBType TrimInvisible)
{
    IRIT_STATIC_DATA IrtPlnType
	XPlane = { 1.0, 0.0, 0.0, 1.190886e-10 };
    CagdBType
	OldInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdRType VMin, VMax, UWMin, UWMax;
    IPPolygonStruct *Plls;
    CagdCrvStruct *NCrv, *CpCrv;
    CagdSrfStruct *USrf, *VSrf, *ProdSrf;

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv))
	Crv = CpCrv = BspCrvOpenEnd(Crv);
    else
	CpCrv = NULL;

    NCrv = SymbCrv2DUnnormNormal(Crv);

    USrf = CagdPromoteCrvToSrf(NCrv, CAGD_CONST_U_DIR);
    CagdCrvDomain(ViewCrv, &VMin, &VMax);
    BspKnotAffineTrans2(USrf -> VKnotVector, USrf -> VLength + USrf -> VOrder,
			VMin, VMax);
    VSrf = CagdPromoteCrvToSrf(ViewCrv, CAGD_CONST_V_DIR);
    CagdCrvDomain(NCrv, &UWMin, &UWMax);
    BspKnotAffineTrans2(VSrf -> UKnotVector, VSrf -> ULength + VSrf -> UOrder,
			UWMin, UWMax);
    CagdCrvFree(NCrv);

    ProdSrf = SymbSrfDotProd(USrf, VSrf);
    CagdSrfFree(USrf);
    CagdSrfFree(VSrf);

    if (TrimInvisible) {
        MvarConstraintType 
	    Constraints[4] = {
	        MVAR_CNSTRNT_ZERO,
		MVAR_CNSTRNT_ZERO,
		MVAR_CNSTRNT_POSITIVE,
		MVAR_CNSTRNT_POSITIVE
	    };
	int i;
	CagdCrvStruct
	    *DViewCrv = CagdCrvDerive(ViewCrv);
        MvarMVStruct *MV, *MV4, *MV5, *Finals[4],
	    *MV1 = MvarCrvToMV(Crv),
	    *MV2 = MvarCrvToMV(ViewCrv),
	    *MV3 = MvarCrvToMV(DViewCrv);
	MvarPtStruct *MVPts, *MVPt;
	IPObjectStruct *Pts;

	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);

	MV = MvarSrfToMV(ProdSrf);
	Finals[0] = MvarPromoteMVToMV2(MV, 3, 0);
	MvarMVFree(MV);

	MV =  MvarPromoteMVToMV2(MV2, 3, 1);
	MvarMVFree(MV2);
	MV2 = MV;
	MV =  MvarPromoteMVToMV2(MV3, 3, 1);
	MvarMVFree(MV3);
	MV3 = MV;
	BspKnotAffineTrans2(MV2 -> KnotVectors[0],
			    MV2 -> Lengths[0] + MV2 -> Orders[0],
			    UWMin, UWMax);
	BspKnotAffineTrans2(MV2 -> KnotVectors[2],
			    MV2 -> Lengths[2] + MV2 -> Orders[2],
			    UWMin, UWMax);
	BspKnotAffineTrans2(MV3 -> KnotVectors[0],
			    MV3 -> Lengths[0] + MV3 -> Orders[0],
			    UWMin, UWMax);
	BspKnotAffineTrans2(MV3 -> KnotVectors[2],
			    MV3 -> Lengths[2] + MV -> Orders[2],
			    UWMin, UWMax);

	MV4 = MvarPromoteMVToMV2(MV1, 3, 0);
	MV5 = MvarPromoteMVToMV2(MV1, 3, 2);
	MvarMVFree(MV1);

	MV = MvarMVSub(MV4, MV5);
	MvarMVFree(MV4);
	MvarMVFree(MV5);
	BspKnotAffineTrans2(MV -> KnotVectors[1],
			    MV -> Lengths[1] + MV -> Orders[1], VMin, VMax);

	Finals[1] = MvarMVDotProd(MV, MV3);
	Finals[2] = MvarMVDotProd(MV, MV2);
	MvarMVFree(MV);
	MvarMVFree(MV2);
	MvarMVFree(MV3);
	
	MVPts = MvarMVsZeros(Finals, Constraints, 2, fabs(SubTol), NumTol);

	MvarMVFree(Finals[0]);
	MvarMVFree(Finals[1]);
	MvarMVFree(Finals[2]);

	/* Extract just the t/Theta data set. */
	Pts = IPGenLISTObject(NULL);
	for (i = 0; MVPts != NULL; ) {
	    IRIT_LIST_POP(MVPt, MVPts);
	    IPListObjectInsert(Pts, i++,
			       IPGenPTObject(&MVPt -> Pt[0],
					     &MVPt -> Pt[1],
					     &MVPt -> Pt[2]));
	    MvarPtFree(MVPt);
	}
	IPListObjectInsert(Pts, i++, NULL);

	if (SubTol < 0.0)
	    return Pts;
	else {
	    Plls = GMMatchPointListIntoPolylines(Pts, SubTol * 2);
	    IPFreeObject(Pts);

	    return IPGenPOLYLINEObject(Plls);
	}
    }
    else {
	if (CpCrv != NULL)
	    CagdCrvFree(CpCrv);

        BspMultComputationMethod(OldInterpFlag);

	if (NumTol <= 0.0)
	    return IPGenSRFObject(ProdSrf);

	Plls = UserCntrSrfWithPlane(ProdSrf, XPlane, NumTol);

	CagdSrfFree(ProdSrf);

	return IPGenPOLYLINEObject(Plls);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the diagonal extremes of the Orthogonality map of curve Crv.    M
* Uses a [DiagExtRes x 1] Z-buffer to extract th diagnoal extremes.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve for which we computed the orthogonality map and now    M
*		seek the diagonal extremes.				     M
*   OM:         The computed othogonality map of Crv.			     M
*   DiagExtRes: The resolution of the Z-buffer to use to extract the	     M
*		diagonal extreme.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:    A polyline object with two polylines, upper	     M
*		diagonal extreme followed by lower diagonal extreme.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserCrvAngleMap                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserCrvOMDiagExtreme                                                     M
*****************************************************************************/
IPObjectStruct *UserCrvOMDiagExtreme(const CagdCrvStruct *Crv,
				     const IPObjectStruct *OM,
				     int DiagExtRes)
{
    int n;
    CagdRType TMin, TMax;
    IrtHmgnMatType Mat1, Mat2;
    IRndrZBuffer1DPtrType CrntZBuf,
	UprZBuf = IRndr1DInitialize(DiagExtRes + 1, 0, DiagExtRes,
				    -DiagExtRes, 2 * DiagExtRes, TRUE),
	LwrZBuf = IRndr1DInitialize(DiagExtRes + 1, 0, DiagExtRes,
				    -DiagExtRes, 2 * DiagExtRes, FALSE);
    IPPolygonStruct *Pl, *VUpr, *VLwr;
    IPObjectStruct *RetObj, *TObj, *OMTmp;

    /* Set up our 1D Z-buffers... */
    IRndr1DClearDepth(UprZBuf, 2 * DiagExtRes);
    IRndr1DClearDepth(LwrZBuf, -DiagExtRes);

    /* OM is coming in YZ space - map it back to XY space. */
    MatGenMatRotX1(-M_PI_DIV_2, Mat1);
    MatGenMatRotY1(-M_PI_DIV_2, Mat2);
    MatMultTwo4by4(Mat2, Mat1, Mat2);

    /* Map the OM to ([0, DiagExtRes], 0, [0, DiagExtRes]). */
    CagdCrvDomain(Crv, &TMin, &TMax);
    MatGenMatTrans(-TMin, -TMin, 0.0, Mat1);
    MatMultTwo4by4(Mat2, Mat2, Mat1);

    MatGenMatUnifScale(DiagExtRes / (TMax - TMin), Mat1);
    MatMultTwo4by4(Mat2, Mat2, Mat1);

    MatGenMatRotX1(M_PI_DIV_2, Mat1);
    MatMultTwo4by4(Mat2, Mat2, Mat1);

    /* Scan convert the polylines into the Z buffer, three times.  Once as  */
    /* is and twice with the OM moved down and up domain amount.	    */
    for (n = 0; n < 3; n++) {
	switch (n) {
	    case 0:
	        OMTmp = GMTransformObject(OM, Mat2);
	        break;
	    case 1:
		MatGenMatTrans(0.0, 0.0, -DiagExtRes, Mat1);
		MatMultTwo4by4(Mat1, Mat2, Mat1);
	        OMTmp = GMTransformObject(OM, Mat1);
	        break;
	    case 2:
		MatGenMatTrans(0.0, 0.0, DiagExtRes, Mat1);
		MatMultTwo4by4(Mat1, Mat2, Mat1);
	        OMTmp = GMTransformObject(OM, Mat1);
	        break;
	    default:
		OMTmp = NULL;
	}

        for (Pl = OMTmp -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	    int IncX;
	    IPVertexStruct *V, *VNext,
	        *VLast = NULL;

	    if (Pl == NULL || (V = Pl -> PVertex) == NULL)
	        continue;
	    if ((VNext = V -> Pnext) == NULL)
	        continue;

	    IncX = VNext -> Coord[0] > V -> Coord[0];
	    CrntZBuf = V -> Coord[2] > V -> Coord[0] ? UprZBuf : LwrZBuf;

	    for ( ; V != NULL; V = V -> Pnext) {
	        if (VLast != NULL) {
		    {
		        IRndr1DPutLine(CrntZBuf, V -> Coord[0], V -> Coord[2],
				       VLast -> Coord[0], VLast -> Coord[2]);
		    }
		}

		VLast = V;
	    }
	}

	IPFreeObject(OMTmp);
    }

    /* Extract the computed depths and create the two returned polylines. */
    VUpr = IRndr1DUpperEnvAsPolyline(UprZBuf, TRUE);
    VLwr = IRndr1DUpperEnvAsPolyline(LwrZBuf, TRUE);
    VUpr -> Pnext = VLwr;

    IRndr1DDestroy(UprZBuf);
    IRndr1DDestroy(LwrZBuf);

    TObj = IPGenPOLYLINEObject(VUpr);
    MatInverseMatrix(Mat2, Mat1);
    RetObj = GMTransformObject(TObj, Mat1);
    IPFreeObject(TObj);

    return RetObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute and trim regions in Srf that have normals with angular deviation M
* of more (less) than Theta Degrees from the prescribed direction VDir.	     M
*   Computation is based on extractions of Isoclines of Srf from VDir.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to examine its normals' angular deviation.            M
*   VDir:      View direction vector.			                     M
*   Theta:     Angular deviation to examine, in degrees, in range (0, 90).   M
*   MoreThanTheta:  TRUE to seek regions of more than theta degrees normal   M
8	       deviation from VDir, FALSE for less than.		     M
*   Tolerance: Accuracy of computation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:    List of trimmed surfaces' regions of regions of Srf  M
*			that present angular deviations of less (more) than  M
*			Theta to VDir.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfIsocline, UserMoldRuledRelief2Srf                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMoldReliefAngle2Srf                                                  M
*****************************************************************************/
TrimSrfStruct *UserMoldReliefAngle2Srf(const CagdSrfStruct *Srf,
				       const CagdVType VDir,
				       CagdRType Theta,
				       int MoreThanTheta,
				       CagdRType Tolerance)
{
    CagdRType CosTheta;
    CagdVType VDirNormalized;
    IPPolygonStruct
	*Pls = SymbSrfIsocline(Srf, VDir, Theta, Tolerance, FALSE);
    TrimSrfStruct
	*TSrfs = TrimSrfsFromContours(Srf, Pls),
	*RetTSrfs = NULL;

    IPFreePolygonList(Pls);

    IRIT_VEC_COPY(VDirNormalized, VDir);
    IRIT_VEC_NORMALIZE(VDirNormalized);

    CosTheta = cos(IRIT_DEG2RAD(Theta));
    MoreThanTheta = MoreThanTheta != 0;   /* Make it is either zero or one. */

    while (TSrfs != NULL) {
	TrimSrfStruct *TSrf;
	CagdVecStruct *Nrml;
	CagdRType *UV;

	IRIT_LIST_POP(TSrf, TSrfs);

	UV = TrimPointInsideTrimmedCrvs(TSrf -> TrimCrvList, TSrf);

	Nrml = CagdSrfNormal(TSrf -> Srf, UV[0], UV[1], TRUE);

	if ((IRIT_FABS(IRIT_DOT_PROD(VDirNormalized, Nrml -> Vec)) > CosTheta)
							    ^ MoreThanTheta) {
	    IRIT_LIST_PUSH(TSrf, RetTSrfs);
	}
	else
	    TrimSrfFree(TSrf);
    }

    return RetTSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute and add ruled surfaces at all trimmed relief angles' boundaries. M
*   Computation is based on extractions of Isoclines of Srf from VDir.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to examine its normals' angular deviation.            M
*   VDir:      View direction vector.			                     M
*   Theta:     Angular deviation to examine, in degrees, in range (0, 90).   M
*   Tolerance: Accuracy of computation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    List of ruled surfaces that tangently extends from   M
*		        the computed isoclines at the prescribed VDir/Theta. M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfIsocline, UserMoldReliefAngle2Srf                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMoldRuledRelief2Srf                                                  M
*****************************************************************************/
CagdSrfStruct *UserMoldRuledRelief2Srf(const CagdSrfStruct *Srf,
				       const CagdVType VDir,
				       CagdRType Theta,
				       CagdRType Tolerance)
{
    IPPolygonStruct *Pl1, *Pl2, *E3Poly,
	*NewPls = NULL,
	*Pls = SymbSrfIsocline(Srf, VDir, Theta, Tolerance, FALSE);
    CagdSrfStruct
	*RuledList = NULL;

    /* Constructed E3 polylines evaluated from parametric domain polylines. */
    while (Pls != NULL) {
        IPVertexStruct *V;
	IPPolygonStruct *E3Poly;

	IRIT_LIST_POP(Pl1, Pls);

	if (GMPolyLength(Pl1) < USER_MIN_LEN_POLY ||
	    IPVrtxListLen(Pl1 -> PVertex) < 3) {
	    IPFreePolygon(Pl1);
	    continue;
	}

        E3Poly = IPCopyPolygon(Pl1);

	for (V = E3Poly -> PVertex; V != NULL; V = V -> Pnext) {
	    CagdRType
		*R = CagdSrfEval(Srf, V -> Coord[0], V -> Coord[1]);

	    CagdCoerceToE3(V -> Coord, &R, -1, Srf -> PType);
	}

	AttrSetPtrAttrib(&Pl1 -> Attr, "E3Poly", E3Poly);

	IRIT_LIST_PUSH(Pl1, NewPls);
    }

    Pls = NewPls;

    /* Match the polylines into pairs and construct the ruled surfaces. */
    while (Pls != NULL) {
        int Len;
	CagdCrvStruct *Crv1, *Crv2, *Reparam;
	CagdSrfStruct *RuledSrf;

        /* Classify the isoclines into matching pairs. */
        IRIT_LIST_POP(Pl1, Pls);
	if ((Pl2 = UserMoldMatchPolyline(Pl1, &Pls)) == NULL) {
	    E3Poly = (IPPolygonStruct *) AttrGetPtrAttrib(Pl1 -> Attr,
							  "E3Poly");
	    IPFreePolygon(E3Poly);
	    IPFreePolygon(Pl1);
	    continue;
	}

	/* Match the two curves. */
	Crv1 = IPPolyline2Curve(Pl1, 3);
	Crv2 = IPPolyline2Curve(Pl2, 3);
	Len = IRIT_MAX(Crv1 -> Length, Crv2 -> Length);
	if (CagdCrvTwoCrvsOrient(Crv1, Crv2,
				 (int) (Len * USER_MOLD_MATCH_REDUCE))) {
	    CagdCrvStruct
		*TCrv = CagdCrvReverse(Crv2);

	    CagdCrvFree(Crv2);
	    Crv2 = TCrv;
	}

	Reparam = CagdMatchingTwoCurves(Crv1, Crv2,
					(int) IRIT_MAX(Len * USER_MOLD_MATCH_REDUCE,
						  3),
					Len, 3, FALSE, TRUE, TRUE,
					CagdMatchDistNorm);

	E3Poly = (IPPolygonStruct *) AttrGetPtrAttrib(Pl1 -> Attr, "E3Poly");
	IPFreePolygon(E3Poly);
	IPFreePolygon(Pl1);
	E3Poly = (IPPolygonStruct *) AttrGetPtrAttrib(Pl2 -> Attr, "E3Poly");
	IPFreePolygon(E3Poly);
	IPFreePolygon(Pl2);

	/* Constructs two ruled surfaces along Crv1 and Crv2. */
	if ((RuledSrf = UserMoldRuledSrf(Srf, Crv1, Crv2,
					 Reparam, TRUE)) != NULL) {
	    IRIT_LIST_PUSH(RuledSrf, RuledList);
	}
	if ((RuledSrf = UserMoldRuledSrf(Srf, Crv2, Crv1,
					 Reparam, FALSE)) != NULL) {
	    IRIT_LIST_PUSH(RuledSrf, RuledList);
	}

	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);
	CagdCrvFree(Reparam);
    }

    return RuledList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the closest match to Pl, in Pls.  Search is a simple O(n^2) test of *
* closest points in Pl to points in polylines Pls.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:  Polyline to search a closest match in Pls.                          *
*   Pls: List of currently available polylines.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   Found match that is also removed from Pls, NULL     *
*			 otherwise.                                          *
*****************************************************************************/
static IPPolygonStruct *UserMoldMatchPolyline(IPPolygonStruct *Pl,
					      IPPolygonStruct **Pls)
{
    int Len = IPVrtxListLen(Pl -> PVertex);
    IrtRType
	MinDist = IRIT_INFNTY;
    IPPolygonStruct *Pl2,
	*ClosestPl = NULL,
	*E3Poly = (IPPolygonStruct *) AttrGetPtrAttrib(Pl -> Attr, "E3Poly");

    for (Pl2 = *Pls; Pl2 != NULL; Pl2 = Pl2 -> Pnext) {
        int Index;
	IrtRType
	    d = 0.0;
        IPVertexStruct *MinV, *V;
	IPPolygonStruct *MinPl,
	    *E3Poly2 = (IPPolygonStruct *) AttrGetPtrAttrib(Pl2 -> Attr,
							    "E3Poly");

	/* Compute the average distance from Pl to Pl2. */
	for (V = E3Poly -> PVertex; V != NULL; V = V -> Pnext) {
	    d += UserMinDistPointPolylineList(V -> Coord, E3Poly2,
					      &MinPl, &MinV, &Index);
	}
	d /= Len;

	if (d < MinDist) {
	    ClosestPl = Pl2;
	    MinDist = d;
	}
    }

    if (ClosestPl == NULL) {
        USER_FATAL_ERROR(USER_ERR_WRONG_ANGLE);
	return NULL;
    }

    /* Remove ClosestPl from the linked list. */
    if (ClosestPl == *Pls)
        *Pls = ClosestPl -> Pnext;
    else {
	for (Pl = *Pls;
	     Pl != NULL && Pl -> Pnext != ClosestPl;
	     Pl = Pl -> Pnext);
	if (Pl != NULL && Pl -> Pnext != NULL)
	    Pl -> Pnext = Pl -> Pnext -> Pnext;
    }

    return ClosestPl;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Constructs a ruled surface,from Crv1 half way to Crv2 toward.            *
*   The extent of the ruling is defined by the intersection of of the two    *
* tangent planes at matched points (Assuming Reparam of Crv2).               *
*                                                                            *
*      Srf(Crv1(t))    and   Srf(Crv2(Reparam(t)))			     *
*                                                                            *
* and the closest point on the intersection line to the matched points is    *
* selected.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf: 	   Freeform surface all this computation originated from.    *
*   Crv1, Crv2:    Pair of curves to rule from Crv1 half way to Crv2.	     *
*   Reparam:       Reparametrization function.				     *
*   Reparam2:      If TRUE Crv2(Reparam(t), otherwise Crv1(Reparam(t)).      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:  The ruled surface along Crv1.                          *
*****************************************************************************/
static CagdSrfStruct *UserMoldRuledSrf(const CagdSrfStruct *Srf,
				       const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2, 
				       CagdCrvStruct *Reparam,
				       int Reparam2)
{
    int i,
	Len = Crv1 -> Length;
    CagdRType UMin, UMax, VMin, VMax, TMin, TMax, Dt, t, UV[2], **Points1;
    CagdVType OrientNrml;
    CagdVecStruct *Vec;
    IPVertexStruct
	*VList = NULL;
    IPPolygonStruct *Pl;
    CagdCrvStruct *Crv1E3, *Crv2E3;
    CagdSrfStruct *RuledSrf;

    CagdCrvDomain(Crv1, &TMin, &TMax);
    Dt = (TMax - TMin - IRIT_UEPS) / (Len - 1);

    Crv1E3 = CagdCoerceCrvTo(Crv1, CAGD_PT_E3_TYPE, FALSE);
    Points1 = Crv1E3 -> Points;

    for (i = 0, t = TMin; i < Len; i++, t += Dt) {
	CagdRType *R, r;
	CagdPType Pt1, Pt2;
	CagdVType V1, V2;
	IrtPlnType Pln1, Pln2, Pln3;

	/* Evaluate position and normal along the first curve. */
	if (!Reparam2) {
	    R = CagdCrvEval(Reparam, t);
	    r = R[1];
	}
	else
	    r = t;
	R = CagdCrvEval(Crv1, r);
	CagdCoerceToE2(UV, &R, -1, Crv1 -> PType);

	Vec = CagdSrfNormal(Srf, UV[0], UV[1], TRUE);
	IRIT_VEC_COPY(Pln1, Vec -> Vec);

	/* So we could orient the ruled srf. */
	if (i == 0)
	    IRIT_VEC_COPY(OrientNrml, Vec -> Vec);

	R = CagdSrfEval(Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt1, &R, -1, Srf -> PType);
	Points1[1][i] = Pt1[0];
	Points1[2][i] = Pt1[1];
	Points1[3][i] = Pt1[2];

	Pln1[3] = -IRIT_DOT_PROD(Pln1, Pt1);

	/* Evaluate position and normal along the second curve. */
	if (Reparam2) {
	    R = CagdCrvEval(Reparam, t);
	    r = R[1];
	}
	else
	    r = t;
	R = CagdCrvEval(Crv2, r);
	CagdCoerceToE2(UV, &R, -1, Crv1 -> PType);

	Vec = CagdSrfNormal(Srf, UV[0], UV[1], TRUE);
	IRIT_VEC_COPY(Pln2, Vec -> Vec);

	R = CagdSrfEval(Srf, UV[0], UV[1]);
	CagdCoerceToE3(Pt2, &R, -1, Srf -> PType);

	Pln2[3] = -IRIT_DOT_PROD(Pln2, Pt2);

	/* Compute a third vertical plane through Pt1 and Pt2 */
	IRIT_PT_SUB(V1, Pt1, Pt2);  /* Compute a vec between the two points. */
	IRIT_VEC_ADD(V2, Pln1, Pln2);          /* Compute an average normal. */
	IRIT_VEC_NORMALIZE(V2);
	IRIT_CROSS_PROD(Pln3, V1, V2);

	Pln3[3] = -IRIT_DOT_PROD(Pln3, Pt1);

	/* Get the point as the intersection of three planes. */
	if (!GMPointFrom3Planes(Pln1, Pln2, Pln3, Pt1)) {
	    /* Singularity could accur if isoclines are really far. */
	    IPFreeVertexList(VList);
	    USER_FATAL_ERROR(USER_ERR_WRONG_ANGLE);
	    return NULL;
	}

	VList = IPAllocVertex2(VList);
	IRIT_PT_COPY(VList -> Coord, Pt1);
    }

    Pl = IPAllocPolygon(0, IPReverseVrtxList2(VList), NULL);
    Crv2E3 = IPPolyline2Curve(Pl, 3);

    IPFreePolygon(Pl);

    RuledSrf = CagdRuledSrf(Crv1E3, Crv2E3, 2, 2);

    /* Orient ruled surface to follow the orientation of original surface. */
    CagdSrfDomain(RuledSrf, &UMin, &UMax, &VMin, &VMax);
    Vec = CagdSrfNormal(RuledSrf, UMin, VMin, FALSE);
    if (IRIT_DOT_PROD(Vec -> Vec, OrientNrml) < 0) {
        CagdSrfStruct
	    *TSrf = CagdSrfReverseDir(RuledSrf, CAGD_CONST_U_DIR);

	CagdSrfFree(RuledSrf);
	RuledSrf = TSrf;
    }

    CagdCrvFree(Crv1E3);
    CagdCrvFree(Crv2E3);

    return RuledSrf;
}


#ifdef DEBUG_MAIN_VISIBILITY

void main(int argc, char **argv)
{
    IPObjectStruct *PObj;
    IrtRType
	Size = 0.6,
	Resolution = 20;

    if (argc > 1 && strcmp(argv[1], "-s") == 0) {
	Size = atof(argv[2]);
	argv += 2;
	argc -= 2;
    }
    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
	Resolution = atof(argv[2]);
	argv += 2;
	argc -= 2;
    }

    IRIT_INFO_MSG_PRINTF("Doing decomposition of size %f, resolution %f\n",
			    Size, Resolution);

    if ((PObj = IPGetDataFiles(&argv[1], 1, TRUE, TRUE)) != NULL &&
	IP_IS_SRF_OBJ(PObj)) {
	int FileIndex = 1;
	IPObjectStruct *PTmp,
	    *PTrimSrfObjs = UserSrfVisibConeDecomp(PObj -> U.Srfs,
						   Resolution, Size);

	for (PTmp = PTrimSrfObjs; PTmp != NULL; PTmp = PTmp -> Pnext) {
	    int Handler;
	    char FileName[IRIT_LINE_LEN_LONG];

	    sprintf(FileName, "ViewDir%d.itd", FileIndex++);
	    if ((Handler = IPOpenDataFile(FileName,
						FALSE, FALSE)) >= 0) {
		IPPutObjectToHandler(Handler, PTmp);
		IPCloseStream(Handler, TRUE);
	    }
	}
	IPFreeObjectList(PTrimSrfObjs);
    }
    else {
	IRIT_WARNING_MSG("Expecting a surface");
    }
}

#endif /* DEBUG_MAIN_VISIBILITY */
