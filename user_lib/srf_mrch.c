/******************************************************************************
* SrfMrch.c - piecewise linear marching on freeform surfaces.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 97.					      *
******************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "bool_lib.h"
#include "geom_lib.h"
#include "user_loc.h"

#define USER_POLY_MRCH_EPS	1e-3

static IPVertexStruct *UserMarchOnPolygonsB2B(UserSrfMarchType MarchType,
					      const IPPolygonStruct *PlHead,
					      const IPPolygonStruct *PlPrev,
					      IPVertexStruct *VHead,
					      IPVertexStruct *VPrev,
					      CagdRType Length,
					      int Info);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Marches and create a polyline on the surface of given Length.	     M
* NSrf, DuSrf, and DvSrf are computed locally if not provided which would    M
* reduce the efficiency of a sequence of surface marching procedures, on the M
* same surface.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   MarchType:   Type of march - isoparametric, lines of curvature, etc.     M
*                for USER_SRF_MARCH_PRIN_CRVTR type, it is assumed that      M
*		 SymbEvalSrfCurvPrep was invoked on Srf before this function M
*		 is called for the proper preparations.			     M
*   UVOrig:      Origin on surface where the march starts.                   M
*   DirOrig:     Direction to march on surface (projected to tangent plane). M
*                If, however, MarchType == SRF_MARCH_ISO_PARAM, Dir contains M
*		 the direction in the parametric, UV, space.		     M
*   Srf:         Surface to march on.                                        M
*   NSrf:        Normal field of surface to march on (optional).             M
*   DuSrf:       Partial with respect to u (optional).                       M
*   DvSrf:       Partial with respect to v (optional).                       M
*   Length:      Length of March.                                            M
*   FineNess:    Number of estimated steps along the approximated march.     M
*   ClosedInU:	 TRUE if surface is closed in U direction, FALSE otherwise.  M
*   ClosedInV:	 TRUE if surface is closed in V direction, FALSE otherwise.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A piecewise linear approximation of the march.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbEvalSrfCurvPrep, UserMarchOnPolygons                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMarchOnSurface			                                     M
*****************************************************************************/
IPPolygonStruct *UserMarchOnSurface(UserSrfMarchType MarchType,
				    const CagdUVType UVOrig,
				    const CagdVType DirOrig,
				    const CagdSrfStruct *Srf,
				    const CagdSrfStruct *NSrf,
				    const CagdSrfStruct *DuSrf,
				    const CagdSrfStruct *DvSrf,
				    CagdRType Length,
				    CagdRType FineNess,
				    CagdBType ClosedInU,
				    CagdBType ClosedInV)
{
    int i;
    CagdRType Wu, Wv, UMin, UMax, VMin, VMax,
	NumOfSteps = IRIT_MAX(FineNess, 3),
	SrfMarchStep = Length / NumOfSteps;
    CagdUVType UV;
    CagdVType Dir;
    IPVertexStruct
	*VHead = NULL;
    CagdSrfStruct
	*CpNSrf = NULL,
        *CpDuSrf = NULL,
        *CpDvSrf = NULL;

    /* We modify Dir and UV so copy them out. */
    IRIT_PT_COPY(Dir, DirOrig);
    IRIT_GEN_COPY(UV, UVOrig, 2 * sizeof(CagdRType));

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Iterate at most twice as many steps as estimated (change can occur    */
    /* due to changes in the speed of the advancement).			     */
    for (i = (int) (NumOfSteps * 2); i > 0 && Length >= 0.0; i--) {
	CagdRType *R, K1, K2, Det;
	CagdVType Du, Dv, D1, D2, Nrml, TangentDir;

	VHead = IPAllocVertex2(VHead);
	R = CagdSrfEval(Srf, UV[0], UV[1]);
	CagdCoerceToE3(VHead -> Coord, &R, -1, Srf -> PType);

	switch (MarchType) {
	    case USER_SRF_MARCH_PRIN_CRVTR:
	        SymbEvalSrfCurvature(Srf, UV[0], UV[1], TRUE, &K1, &K2, D1, D2);
		IRIT_PT_NORMALIZE(D1);
		IRIT_PT_NORMALIZE(D2);
		if (IRIT_FABS(K1 = IRIT_DOT_PROD(D1, Dir)) >
		    IRIT_FABS(K2 = IRIT_DOT_PROD(D2, Dir))) {
		    if (K1 < 0)
		        IRIT_PT_SCALE(D1, -1);
		    IRIT_PT_COPY(Dir, D1);
		    IRIT_PT_NORMALIZE(D1);
		    Wu = D1[0];
		    Wv = D1[1];
		}
		else {
		    if (K2 < 0)
		        IRIT_PT_SCALE(D2, -1);
		    IRIT_PT_COPY(Dir, D2);
		    IRIT_PT_NORMALIZE(D2);
		    Wu = D2[0];
		    Wv = D2[1];
		}
	        break;
	    case USER_SRF_MARCH_ISOCLINES:
	    case USER_SRF_MARCH_ORTHOCLINES:
		if (DuSrf == NULL)
		    DuSrf = CpDuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
		if (DvSrf == NULL)
		    DvSrf = CpDvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

		if (NSrf == NULL)
		    NSrf = CpNSrf = SymbSrfNormalSrf(Srf);

		R = CagdSrfEval(DuSrf, UV[0], UV[1]);
		CagdCoerceToE3(Du, &R, -1, DuSrf -> PType);

		R = CagdSrfEval(DvSrf, UV[0], UV[1]);
		CagdCoerceToE3(Dv, &R, -1, DvSrf -> PType);

		R = CagdSrfEval(NSrf, UV[0], UV[1]);
		CagdCoerceToE3(Nrml, &R, -1, NSrf -> PType);
		IRIT_PT_NORMALIZE(Nrml);

		if (Dir == NULL) {
		    IRIT_STATIC_DATA IrtVecType
		        ZAxis = { 0, 0, 1 };

		    IRIT_CROSS_PROD(TangentDir, ZAxis, Nrml);
		}
		else {
		    IRIT_CROSS_PROD(TangentDir, Dir, Nrml);
		}

		if (MarchType == USER_SRF_MARCH_ORTHOCLINES) {
		    IRIT_CROSS_PROD(Dir, TangentDir, Nrml);
		    IRIT_VEC_COPY(TangentDir, Dir);
		}

		IRIT_PT_NORMALIZE(TangentDir);

		/* Figure out weights of Du and Dv to produce a vector in    */
		/* dir TangentDir, solving "Wu Du + Wv Dv = TangentDir".     */
		if (IRIT_FABS(Nrml[0]) > IRIT_FABS(Nrml[1]) &&
		    IRIT_FABS(Nrml[0]) > IRIT_FABS(Nrml[2])) {
		    /* Solve in the YZ plane. */
		    Det = Du[1] * Dv[2] - Du[2] * Dv[1];
		    Wu = (TangentDir[1] * Dv[2] - TangentDir[2] * Dv[1]) / Det;
		    Wv = (TangentDir[2] * Du[1] - TangentDir[1] * Du[2]) / Det;
		}
		else if (IRIT_FABS(Nrml[1]) > IRIT_FABS(Nrml[0]) &&
			 IRIT_FABS(Nrml[1]) > IRIT_FABS(Nrml[2])) {
		    /* Solve in the XZ plane. */
		    Det = Du[0] * Dv[2] - Du[2] * Dv[0];
		    Wu = (TangentDir[0] * Dv[2] - TangentDir[2] * Dv[0]) / Det;
		    Wv = (TangentDir[2] * Du[0] - TangentDir[0] * Du[2]) / Det;
		}
		else {
		    /* Solve in the XY plane. */
		    Det = Du[0] * Dv[1] - Du[1] * Dv[0];
		    Wu = (TangentDir[0] * Dv[1] - TangentDir[1] * Dv[0]) / Det;
		    Wv = (TangentDir[1] * Du[0] - TangentDir[0] * Du[1]) / Det;
		}
	        break;
	    default:
	    case USER_SRF_MARCH_ISO_PARAM:
		if (DuSrf == NULL)
		    DuSrf = CpDuSrf =CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
		if (DvSrf == NULL)
		    DvSrf = CpDvSrf =CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

		R = CagdSrfEval(DuSrf, UV[0], UV[1]);
		CagdCoerceToE3(Du, &R, -1, DuSrf -> PType);

		R = CagdSrfEval(DvSrf, UV[0], UV[1]);
		CagdCoerceToE3(Dv, &R, -1, DvSrf -> PType);

		Wu = Dir[0] / sqrt(IRIT_DOT_PROD(Du, Du));
		Wv = Dir[1] / sqrt(IRIT_DOT_PROD(Dv, Dv));
	        break;
	}

	UV[0] += Wu * SrfMarchStep;
	UV[1] += Wv * SrfMarchStep;

	/* Check for overflow (outside surface domain) and wrap around if   */
	/* the surface is indeed closed.  Otherwise, break this loop.       */
	if (UV[0] > UMax || UV[0] < UMin) {
	    if (ClosedInU) {
		if (UV[0] > UMax)
		    UV[0] = UMin + (UV[0] - UMax);
		else
		    UV[0] = UMax - (UMin - UV[0]);
	    }

	    if (UV[0] > UMax || UV[0] < UMin)
	        break;
	}
	if (UV[1] > VMax || UV[1] < VMin) {
	    if (ClosedInV) {
		if (UV[1] > VMax)
		    UV[1] = VMin + (UV[1] - VMax);
		else
		    UV[1] = VMax - (VMin - UV[1]);
	    }

	    if (UV[1] > VMax || UV[1] < VMin)
	        break;
	}

	Length -= SrfMarchStep;
    }

    if (CpDuSrf != NULL)
        CagdSrfFree(CpDuSrf);
    if (CpDvSrf != NULL)
        CagdSrfFree(CpDvSrf);
    if (CpNSrf != NULL)
        CagdSrfFree(CpNSrf);

    return IPAllocPolygon(0, VHead, NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Marches and create a polyline on the given polygonal mesh, of given	     M
* Length.								     M
*   Mesh is assumed to have adjacency information, and normals and curvature M
* information at the vertices, if MarchType is SILHOUTETE or CURVATURE,      M
* respectively.  Further, the mesh is assumed to consist of triangles only,  M
* and is regularized.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:	 This polygonal object we march on.                          M
*   MarchType:   Type of march - isoparametric, lines of curvature, etc.     M
*   PlHead:      Polygon from the polygonal mesh with start at.              M
*   VHead:       Starting vertex of the march.  Must be on Pl.		     M
*   Length:      Length of March.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   A piecewise linear approximation of the march.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolGenAdjacencies, UserMarchOnSurface                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserMarchOnPolygons			                                     M
*****************************************************************************/
IPPolygonStruct *UserMarchOnPolygons(const IPObjectStruct *PObj,
				     UserSrfMarchType MarchType,
				     const IPPolygonStruct *PlHead,
				     IPVertexStruct *VHead,
				     CagdRType Length)
{
    int i, j, k, l;
    IrtRType R, Nz, Val, *Weights, Dist;
    IrtVecType VTmp, PrinDir;
    IPVertexStruct *V[3], *VNext, *VStart;
    IPPolygonStruct *PlNext,
	*PlMarch = NULL;

    V[0] = PlHead -> PVertex;
    V[1] = V[0] -> Pnext;
    V[2] = V[1] -> Pnext;

    if (V[2] -> Pnext != NULL && V[2] -> Pnext != V[0]) {
        USER_FATAL_ERROR(USER_ERR_EXPCT_REG_TRIANG);
	return NULL;
    }

    switch (MarchType) {
	case USER_SRF_MARCH_ISO_PARAM:
	default:
	    /* Trace the segment in the +/-X and +/-Y axes. */
            for (k = 0; k < 2; k++) {
		Val = VHead -> Coord[k];
		for (i = 0; i < 3; i++) {
		    j = i + 1 > 2 ? 0 : i + 1;
		    if ((V[i] -> Coord[k] >= Val &&
			 V[j] -> Coord[k] <= Val) ||
			(V[i] -> Coord[k] <= Val &&
			 V[j] -> Coord[k] >= Val)) {
		        VNext = IPAllocVertex2(NULL),
			VStart = IPAllocVertex(VHead -> Tags, VHead -> PAdj,
					       VNext);
			IRIT_PT_COPY(VStart -> Coord, VHead -> Coord);
			IRIT_VEC_COPY(VStart -> Normal, VHead -> Normal);
			IP_SET_NORMAL_VRTX(VStart);

			/* Constructs point on the edge of the polygon. */
			if (V[i] -> Coord[k] == V[j] -> Coord[k])
			    R = 0.5;
			else
			    R = (Val - V[j] -> Coord[k]) /
				        (V[i] -> Coord[k] - V[j] -> Coord[k]);
			IRIT_PT_BLEND(VNext -> Coord, V[i] -> Coord,
				 V[j] -> Coord, R);
			IRIT_VEC_BLEND(VNext -> Normal, V[i] -> Normal,
				  V[j] -> Normal, R);
			IRIT_VEC_NORMALIZE(VNext -> Normal);
			IP_SET_NORMAL_VRTX(VNext);

			Dist = IRIT_PT_PT_DIST(VStart -> Coord, VNext -> Coord);

			if ((Length < Dist ||
			     (PlNext = V[i] -> PAdj) == NULL)) {
			    if (Length < Dist) {
			        R = Length / Dist;
				IRIT_PT_BLEND(VNext -> Coord, VNext -> Coord,
					 VStart -> Coord, R);
				IRIT_VEC_BLEND(VNext -> Normal, VNext -> Normal,
					  VStart -> Normal, R);
				IRIT_VEC_NORMALIZE(VNext -> Normal);
			    }

			    /* Done - push this polyline and continue. */
			    PlMarch = IPAllocPolygon(0, VStart, PlMarch);
			    continue;
			}

			UserMarchOnPolygonsB2B(MarchType, PlNext, PlHead,
					       VNext, VHead,
					       Length - Dist, k + 1);
			PlMarch = IPAllocPolygon(0, VStart, PlMarch);
		    }
		}
	    }
	    break;
	case USER_SRF_MARCH_PRIN_CRVTR:
	    /* Go in the two principal directions. */
	    for (k = 0; k < 2; k++) {
		IrtVecType V1, V2, N, PrinDirs[3];

	        for (i = 0; i < 3; i++) {
		    const char
		        *StrDir = AttrGetStrAttrib(V[i] -> Attr,
						   k == 0 ? "D1" : "D2");

		    if (StrDir == NULL ||
			sscanf(StrDir, "%lf, %lf, %lf", &PrinDirs[i][0],
			       &PrinDirs[i][1], &PrinDirs[i][2]) != 3) {
		        return PlMarch;
		    }
		}

		Weights = GMBaryCentric3Pts(V[0] -> Coord,
					    V[1] -> Coord,
					    V[2] -> Coord,
					    VHead -> Coord);

		/* Estimate the principle direction. */
		IRIT_VEC_BLEND_BARYCENTRIC(PrinDir,
				      PrinDirs[0], PrinDirs[1], PrinDirs[2],
				      Weights);

		IRIT_PT_SUB(V1, V[0] -> Coord, V[1] -> Coord);
		IRIT_PT_SUB(V2, V[1] -> Coord, V[2] -> Coord);
		IRIT_CROSS_PROD(N, V1, V2);
		if ((R = IRIT_VEC_LENGTH(N)) > IRIT_EPS) {
		    /* Make PrinDir Orthogonal to the polygon's normal. */
		    R = 1.0 / R;
		    IRIT_PT_SCALE(N, R);

		    R = IRIT_DOT_PROD(PrinDir, N);
		    IRIT_VEC_SCALE(N, R);
		    IRIT_VEC_SUB(PrinDir, PrinDir, N);
		}

		/* Find the closest boundary point to ray VHead/PrinDir.   */
		/* Do it twice - in the +PrinDir and then in the -PrinDir. */
		for (l = 0; l < 2; l++) {
		    for (i = 0; i < 3; i++) {    /* The three edges of poly. */
		        IrtVecType BndryDir;
			IrtPtType Pt1, Pt2;
			IrtRType t1, t2;

			j = i + 1 > 2 ? 0 : i + 1;

			IRIT_VEC_SUB(BndryDir, V[j] -> Coord, V[i] -> Coord);
			GM2PointsFromLineLine(V[i] -> Coord, 
					      BndryDir,
					      VHead -> Coord,
					      PrinDir,
					      Pt1, &t1, Pt2, &t2);
			if (t2 > 0 && t1 >= 0.0 && t1 <= 1.0) {
			    VNext = IPAllocVertex2(NULL),
			    VStart = IPAllocVertex(VHead -> Tags,
						   VHead -> PAdj,
						   VNext);
			    IRIT_PT_COPY(VStart -> Coord, VHead -> Coord);
			    IRIT_VEC_COPY(VStart -> Normal, VHead -> Normal);
			    IP_SET_NORMAL_VRTX(VStart);

			    IRIT_PT_COPY(VNext -> Coord, Pt1);
			    IRIT_VEC_BLEND(VNext -> Normal, V[j] -> Normal,
				      V[i] -> Normal, t1);
			    IRIT_VEC_NORMALIZE(VNext -> Normal);
			    IP_SET_NORMAL_VRTX(VNext);

			    Dist = IRIT_PT_PT_DIST(VStart -> Coord, VNext -> Coord);

			    if ((PlNext = V[i] -> PAdj) == NULL ||
				Length < Dist) {
			        if (Length < Dist) {
				    R = Length / Dist;
				    IRIT_PT_BLEND(VNext -> Coord, VNext -> Coord,
					     VStart -> Coord, R);
				    IRIT_VEC_BLEND(VNext -> Normal, VNext -> Normal,
					      VStart -> Normal, R);
				    IRIT_VEC_NORMALIZE(VNext -> Normal);
				}

				/* Done - push this polyline and continue. */
				PlMarch = IPAllocPolygon(0, VStart, PlMarch);
				continue;
			    }

			    UserMarchOnPolygonsB2B(MarchType, PlNext, PlHead,
						   VNext, VHead, Length - Dist,
						   l == 0 ? (k + 1) : -(k + 1));
			    PlMarch = IPAllocPolygon(0, VStart, PlMarch);
			    break;
			}
		    }
		    IRIT_VEC_SCALE(PrinDir, -1.0);
		}
	    }
	    break;
	case USER_SRF_MARCH_ISOCLINES:
	    /* This is "silhouette" from +Z direction.  Find the locations   */
	    /* on the boundary of the triangle that presents the same Z      */
	    /* conefficients as VHead -> Normal[2].			     */
	    Nz = VHead -> Normal[2];
	    for (i = 0; i < 3; i++) {
		if (!IP_HAS_NORMAL_VRTX(V[i]))
		    USER_FATAL_ERROR(USER_ERR_NO_NRML_INFO);

	        j = i + 1 > 2 ? 0 : i + 1;
	        if ((V[i] -> Normal[2] >= Nz && V[j] -> Normal[2] <= Nz) ||
		    (V[i] -> Normal[2] <= Nz && V[j] -> Normal[2] >= Nz)) {
		    VNext = IPAllocVertex2(NULL),
		    VStart = IPAllocVertex(VHead -> Tags, VHead -> PAdj,
					   VNext);
		    IRIT_PT_COPY(VStart -> Coord, VHead -> Coord);
		    IRIT_VEC_COPY(VStart -> Normal, VHead -> Normal);
		    IP_SET_NORMAL_VRTX(VStart);

		    /* Constructs the point on the edge of the polygon. */
		    if (V[i] -> Normal[2] == V[j] -> Normal[2])
		        R = 0.5;
		    else
		        R = (Nz - V[j] -> Normal[2]) /
			    (V[i] -> Normal[2] - V[j] -> Normal[2]);
		    IRIT_PT_BLEND(VNext -> Coord, V[i] -> Coord, V[j] -> Coord, R);
		    IRIT_VEC_BLEND(VNext -> Normal, V[i] -> Normal, V[j] -> Normal,
			      R);
		    IRIT_VEC_NORMALIZE(VNext -> Normal);
		    IP_SET_NORMAL_VRTX(VNext);

		    Dist = IRIT_PT_PT_DIST(VStart -> Coord, VNext -> Coord);

		    if ((Length < Dist || (PlNext = V[i] -> PAdj) == NULL)) {
		        if (Length < Dist) {
			    R = Length / Dist;
			    IRIT_PT_BLEND(VNext -> Coord, VNext -> Coord,
				     VStart -> Coord, R);
			    IRIT_VEC_BLEND(VNext -> Normal, VNext -> Normal,
				      VStart -> Normal, R);
			    IRIT_VEC_NORMALIZE(VNext -> Normal);
			}

		        /* Done - push this polyline and continue. */
		        PlMarch = IPAllocPolygon(0, VStart, PlMarch);
			continue;
		    }

		    UserMarchOnPolygonsB2B(MarchType, PlNext, PlHead,
					   VNext, VHead, Length - Dist, 0);
		    PlMarch = IPAllocPolygon(0, VStart, PlMarch);
		}
	    }
	    break;
	case USER_SRF_MARCH_ORTHOCLINES:
	    /* This is orthogonal to the "silhouette" from +Z direction.     */
	    /* This is simply the direction of the projection of +Z on the   */
	    /* Tangent plane.						     */
	    PrinDir[0] = PrinDir[1] = 0.0;
	    PrinDir[2] = 1.0;
	    R = IRIT_DOT_PROD(PrinDir, VHead -> Normal);
	    IRIT_VEC_COPY(VTmp, VHead -> Normal);
	    IRIT_VEC_SCALE(VTmp, R);
	    IRIT_VEC_SUB(PrinDir, PrinDir, VTmp);

	    for (l = 0; l < 2; l++) {
		for (i = 0; i < 3; i++) {    /* The three edges of poly. */
		    IrtVecType BndryDir;
		    IrtPtType Pt1, Pt2;
		    IrtRType t1, t2;

		    j = i + 1 > 2 ? 0 : i + 1;

		    IRIT_VEC_SUB(BndryDir, V[j] -> Coord, V[i] -> Coord);
		    GM2PointsFromLineLine(V[i] -> Coord, 
					  BndryDir,
					  VHead -> Coord,
					  PrinDir,
					  Pt1, &t1, Pt2, &t2);
		    if (t2 > 0 && t1 >= 0.0 && t1 <= 1.0) {
			VNext = IPAllocVertex2(NULL),
			VStart = IPAllocVertex(VHead -> Tags,
					       VHead -> PAdj,
					       VNext);
			IRIT_PT_COPY(VStart -> Coord, VHead -> Coord);
			IRIT_VEC_COPY(VStart -> Normal, VHead -> Normal);
			IP_SET_NORMAL_VRTX(VStart);

			IRIT_PT_COPY(VNext -> Coord, Pt1);
			IRIT_VEC_BLEND(VNext -> Normal, V[j] -> Normal,
				  V[i] -> Normal, t1);
			IRIT_VEC_NORMALIZE(VNext -> Normal);
			IP_SET_NORMAL_VRTX(VNext);

			Dist = IRIT_PT_PT_DIST(VStart -> Coord, VNext -> Coord);

			if ((PlNext = V[i] -> PAdj) == NULL ||
			    Length < Dist) {
			    if (Length < Dist) {
				R = Length / Dist;
				IRIT_PT_BLEND(VNext -> Coord, VNext -> Coord,
					 VStart -> Coord, R);
				IRIT_VEC_BLEND(VNext -> Normal, VNext -> Normal,
					  VStart -> Normal, R);
				IRIT_VEC_NORMALIZE(VNext -> Normal);
			    }

			    /* Done - push this polyline and continue. */
			    PlMarch = IPAllocPolygon(0, VStart, PlMarch);
			    continue;
			}

			UserMarchOnPolygonsB2B(MarchType, PlNext, PlHead,
					       VNext, VHead, Length - Dist,
					       0);
			PlMarch = IPAllocPolygon(0, VStart, PlMarch);
			break;
		    }
		}
		IRIT_VEC_SCALE(PrinDir, -1.0);
	    }
	    break;
    }

    return PlMarch;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Marches and create a polyline on the given polygonal mesh, of given	     *
* Length, from vertex on the boundary of the polygon to another boundary.    *
*   Mesh is assumed to have adjacency information, and normals and curvature *
* information at the vertices, if MarchType is ORTHO/ISOCLINES or CURVATURE, *
* respectively.  Further, the mesh is assumed to consist of triangles only.  *
*                                                                            *
* PARAMETERS:                                                                *
*   MarchType:   Type of march - isoparametric, lines of curvature, etc.     *
*   PlHead:      Current polygon of the march.			             *
*   PlPrev:      Previous polygon of the march.			             *
*   VHead:       Current vertex of the march.  Must be on PlHead.	     *
*   VPrev:       Previous vertex of the march.  Must be on PlPrev.	     *
*   Length:      Length of March.                                            *
*   Info:        For Curvature marching, 1 or 2 for first or second          *
*		        principle direction, negative to opposite direction. *
*                For isoparam marching, 1 or 2 for X or Y direction,	     *
*					     negative to opposite direction. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   A piecewise linear approximation of the march.       *
*                                                                            *
* SEE ALSO:                                                                  *
*   BoolGenAdjacencies, UserMarchOnSurface                                   *
*                                                                            *
* KEYWORDS:                                                                  *
*   UserMarchOnPolygons			                                     *
*****************************************************************************/
static IPVertexStruct *UserMarchOnPolygonsB2B(UserSrfMarchType MarchType,
					      const IPPolygonStruct *PlHead,
					      const IPPolygonStruct *PlPrev,
					      IPVertexStruct *VHead,
					      IPVertexStruct *VPrev,
					      CagdRType Length,
					      int Info)
{
    while (TRUE) {
        int i, j, k;
        IrtRType R, Nz, Val, Dist;
	IrtVecType LastDir, Dir, VTmp, PrinDir, PrinDirs[2];
	IPVertexStruct *V[3],
	    *VTail = VHead,
	    *VNext = IPAllocVertex2(NULL);

	IRIT_VEC_SUB(LastDir, VHead -> Coord, VPrev -> Coord);

        V[0] = PlHead -> PVertex;
	V[1] = V[0] -> Pnext;
	V[2] = V[1] -> Pnext;

	if (V[2] -> Pnext != NULL && V[2] -> Pnext != V[0]) {
	    USER_FATAL_ERROR(USER_ERR_EXPCT_REG_TRIANG);
	    return NULL;
	}

	switch (MarchType) {
	    case USER_SRF_MARCH_ISO_PARAM:
	    default:
	        /* Trace the segment in the +/-X and +/-Y axes. */
		k = IRIT_ABS(Info - 1);
		Val = VHead -> Coord[k];
		for (i = 0; i < 3; i++) {
		    if (!IP_HAS_NORMAL_VRTX(V[i]))
		        USER_FATAL_ERROR(USER_ERR_NO_NRML_INFO);

		    j = i + 1 > 2 ? 0 : i + 1;
		    if (PlPrev != V[i] -> PAdj &&
			((V[i] -> Coord[k] >= Val &&
			  V[j] -> Coord[k] <= Val) ||
			 (V[i] -> Coord[k] <= Val &&
			  V[j] -> Coord[k] >= Val))) {
		        /* Constructs the point on the edge of the polygon. */
		        if (V[i] -> Coord[k] == V[j] -> Coord[k])
			    R = 0.5;
			else
			    R = (Val - V[j] -> Coord[k]) /
			        (V[i] -> Coord[k] - V[j] -> Coord[k]);

			IRIT_PT_BLEND(VNext -> Coord,
				 V[i] -> Coord, V[j] -> Coord, R);
			IRIT_VEC_SUB(Dir, VNext -> Coord, VHead -> Coord);
			if (IRIT_DOT_PROD(Dir, LastDir) > 0)
			    break;
		    }
		}
		break;
	    case USER_SRF_MARCH_PRIN_CRVTR:
	        for (i = 0; i < 3; i++) { /* Find the edge we are crossing. */
		    j = i + 1 > 2 ? 0 : i + 1;

		    if (GMCollinear3Vertices(V[i], VHead, V[j])) {
		        const char *StrDir;

			StrDir = AttrGetStrAttrib(V[i] -> Attr,
					       IRIT_FABS(Info) == 1 ? "D1" : "D2");
			if (StrDir == NULL ||
			    sscanf(StrDir, "%lf, %lf, %lf", &PrinDirs[0][0],
				   &PrinDirs[0][1], &PrinDirs[0][2]) != 3) {
			    /* Could be a boundary - abort now if so. */
			    IPFreeVertex(VNext);
			    return VHead;
			}

			StrDir = AttrGetStrAttrib(V[j] -> Attr,
					       IRIT_FABS(Info) == 1 ? "D1" : "D2");
			if (StrDir == NULL ||
			    sscanf(StrDir, "%lf, %lf, %lf", &PrinDirs[1][0],
				   &PrinDirs[1][1], &PrinDirs[1][2]) != 3) {
			    /* Could be a boundary - abort now if so. */
			    IPFreeVertex(VNext);
			    return VHead;
			}

			if (IRIT_PT_APX_EQ(V[i] -> Coord, V[j] -> Coord))
			    R = 0.5;
			else
			    R = IRIT_PT_PT_DIST(VHead -> Coord, V[j] -> Coord) /
			        IRIT_PT_PT_DIST(V[i] -> Coord, V[j] -> Coord);

			IRIT_VEC_BLEND(PrinDir, PrinDirs[0], PrinDirs[1], R);
			if (Info < 0)
			    IRIT_VEC_SCALE(PrinDir, -1.0);
			break;
		    }
		}
		if (i >= 3)
		    USER_FATAL_ERROR(USER_ERR_NO_CRVTR_INFO);

		/* Find the closest boundary point to ray VHead/PrinDir.     */
		for (i = 0; i < 3; i++) {        /* The three edges of poly. */
		    IrtVecType BndryDir;
		    IrtPtType Pt1, Pt2;
		    IrtRType t1, t2;

		    j = i + 1 > 2 ? 0 : i + 1;

		    IRIT_VEC_SUB(BndryDir, V[j] -> Coord, V[i] -> Coord);
		    GM2PointsFromLineLine(V[i] -> Coord, 
					  BndryDir,
					  VHead -> Coord,
					  PrinDir,
					  Pt1, &t1, Pt2, &t2);
		    if (PlPrev != V[i] -> PAdj && t1 >= 0.0 && t1 <= 1.0) {
		        R = 1.0 - t1;
			break;
		    }
		}
		break;
	    case USER_SRF_MARCH_ISOCLINES:
	        /* This is "silhouette" from +Z direction.  Find locations   */
		/* on the boundary of triangle that presents the same Z      */
		/* coefficients as VHead -> Normal[2].			     */
		Nz = VHead -> Normal[2];
		for (i = 0; i < 3; i++) {
		    if (!IP_HAS_NORMAL_VRTX(V[i]))
		        USER_FATAL_ERROR(USER_ERR_NO_NRML_INFO);

		    j = i + 1 > 2 ? 0 : i + 1;
		    if (PlPrev != V[i] -> PAdj &&
			((V[i] -> Normal[2] >= Nz &&
			  V[j] -> Normal[2] <= Nz) ||
			 (V[i] -> Normal[2] <= Nz &&
			  V[j] -> Normal[2] >= Nz))) {
		        /* Constructs the point on the edge of the polygon. */
		        if (V[i] -> Normal[2] == V[j] -> Normal[2])
			    R = 0.5;
			else
			    R = (Nz - V[j] -> Normal[2]) /
			        (V[i] -> Normal[2] - V[j] -> Normal[2]);

			IRIT_PT_BLEND(VNext -> Coord,
				 V[i] -> Coord, V[j] -> Coord, R);
			IRIT_VEC_SUB(Dir, VNext -> Coord, VHead -> Coord);
			if (IRIT_DOT_PROD(Dir, LastDir) > 0)
			    break;
		    }
		}
		break;
	    case USER_SRF_MARCH_ORTHOCLINES:
		PrinDir[0] = PrinDir[1] = 0.0;
		PrinDir[2] = 1.0;
		R = IRIT_DOT_PROD(PrinDir, VHead -> Normal);
		IRIT_VEC_COPY(VTmp, VHead -> Normal);
		IRIT_VEC_SCALE(VTmp, R);
		IRIT_VEC_SUB(PrinDir, PrinDir, VTmp);

		/* Find the closest boundary point to ray VHead/PrinDir.     */
		for (i = 0; i < 3; i++) {        /* The three edges of poly. */
		    IrtVecType BndryDir;
		    IrtPtType Pt1, Pt2;
		    IrtRType t1, t2;

		    j = i + 1 > 2 ? 0 : i + 1;

		    if (GMCollinear3Vertices(V[i], VHead, V[j]))
			continue;

		    IRIT_VEC_SUB(BndryDir, V[j] -> Coord, V[i] -> Coord);
		    GM2PointsFromLineLine(V[i] -> Coord, 
					  BndryDir,
					  VHead -> Coord,
					  PrinDir,
					  Pt1, &t1, Pt2, &t2);
		    if (PlPrev != V[i] -> PAdj && t1 >= 0.0 && t1 <= 1.0) {
		        R = 1.0 - t1;
			break;
		    }
		}
		break;
	}

	if (i >= 3) {  			 /* Failed to find the other edge... */
	    IPFreeVertex(VNext);
	    return VHead;
	}

        /* Update the next vertices of the iteration. */
        IRIT_PT_BLEND(VNext -> Coord, V[i] -> Coord, V[j] -> Coord, R);
        IRIT_VEC_BLEND(VNext -> Normal, V[i] -> Normal, V[j] -> Normal, R);
        IRIT_VEC_NORMALIZE(VNext -> Normal);
	IP_SET_NORMAL_VRTX(VNext);

        Dist = IRIT_PT_PT_DIST(VHead -> Coord, VNext -> Coord);
	VTail -> Pnext = VNext;
	VTail = VNext;

	if (Dist <= IRIT_UEPS || Length < Dist || V[i] -> PAdj == NULL) {
	    if (Length < Dist) {
	        R = Length / Dist;
		IRIT_PT_BLEND(VNext -> Coord, VNext -> Coord, VHead -> Coord, R);
		IRIT_VEC_BLEND(VNext -> Normal, VNext -> Normal, VHead -> Normal, R);
		IRIT_VEC_NORMALIZE(VNext -> Normal);
	    }

	    return VHead;		     /* Done - return this polyline. */
	}

        Length -= Dist;
	PlPrev = PlHead;
        PlHead = V[i] -> PAdj;
	VPrev = VHead;
	VHead = VNext;
    }

    return NULL;
}
