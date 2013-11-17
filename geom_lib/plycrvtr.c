/******************************************************************************
* PlyCrvtr.c - principal curvatures for polygonal data sets.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, January 2004                                      *
******************************************************************************/

#include <math.h>
#include "geom_loc.h"

#define GMPL_SMALL_Z_ROT		0.30106			  /* Radian.*/
#define GMPL_MAX_EXPAND_RINGS		5
#define GMPL_CRVTR_MAX_FIT		1000
#define GMPL_MAX_NRML_BLEND_ANGLE	45

#define GMPL_BOUND_CRVTR(k)	\
	(IRIT_FABS( k ) < IRIT_UEPS ? 0.0 \
			       : IRIT_BOUND(k, -IRIT_INFNTY, IRIT_INFNTY) )

IRIT_STATIC_DATA int
    GMPlUseCubicFit = FALSE;

static IrtPtType *GMPlEstimateQuadratic(IPPolyVrtxIdxStruct *PVIdx,
					IrtHmgnMatType Mat,
					int VrtxIdx,
					int NumOfRings);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimates the Gaussian and Mean curvature values for the given           M
* triangular regular mesh and initializes the corresponding attributes:	     M
* for each vertex: "K1Curv", "K2Curv", "D1", "D2" and "K" and "H".	     M
*   Uses a least sqaures osculating quadratic function in the estimate.	     M
*   Mesh is assumed to be a triangular regular mesh.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:    The triangular two-manifold mesh data.		             M
*   NumOfRings:  Number of rings around a vertex in the paraboloid fitting.  M
*   EstimateNrmls:  If TRUE estimate normals to the vertices on the fly.     M
*		 This functions needs these normals for its proper work.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPlCrvtrSetFitDegree, SymbEvalSrfCurvPrep, SymbEvalSrfCurvature         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPlCrvtrSetCurvatureAttr                                                M
*****************************************************************************/
void GMPlCrvtrSetCurvatureAttr(IPPolygonStruct *PolyList,
			       int NumOfRings,
			       int EstimateNrmls)
{
    int i;
    IPPolygonStruct *Pl;
    IPObjectStruct
        *PObj = IPGenPOLYObject(PolyList);
    IPPolyVrtxIdxStruct
	*PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObj, TRUE, 0);
    IPVertexStruct
	**Vertices = PVIdx -> Vertices;

    if (EstimateNrmls)
	GMBlendNormalsToVertices(PObj -> U.Pl, GMPL_MAX_NRML_BLEND_ANGLE);

    /* Verify triangles only with normals. */
    for (Pl = PolyList; Pl != NULL; Pl = Pl -> Pnext) {
        int i = 0;
        IPVertexStruct
	    *V = Pl -> PVertex;

        do {
	    i++;
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);


	if (i != 3) {
	    GEOM_FATAL_ERROR(GEOM_ERR_TRIANGLES_ONLY);
	    return;
	}
    }

    for (i = 0; i < PVIdx -> NumVrtcs; i++) {
        char SDirection[IRIT_LINE_LEN_LONG];
        int Rings;
	IrtRType Theta, A, B, C, k1, k2, D, H, K;
	IrtVecType Dir, E3Dir;
	IrtPtType
	    *Quad = NULL;
        IrtHmgnMatType InvMat, Mat;
	IPVertexStruct
	    *V = Vertices[i];

	if (!IP_HAS_NORMAL_VRTX(V)) {
	    IRIT_WARNING_MSG("A vertex with no normal detected and ignored.\n");

	    AttrSetRealAttrib(&V -> Attr, "KCurv", 0.0);
	    AttrSetRealAttrib(&V -> Attr, "HCurv", 0.0);
	    AttrSetRealAttrib(&V -> Attr, "K1Curv", 0.0);
	    AttrSetRealAttrib(&V -> Attr, "K2Curv", 0.0);
	    AttrSetStrAttrib(&V -> Attr, "D1", "0,0,0");
	    AttrSetStrAttrib(&V -> Attr, "D2", "0,0,0");

	    continue;
	}

	GMGenTransMatrixZ2Dir(InvMat, V -> Coord, V -> Normal, 1.0);
	MatInverseMatrix(InvMat, Mat);

	for (Rings = NumOfRings;
	     Rings <= NumOfRings + GMPL_MAX_EXPAND_RINGS;
	     Rings++) {
	    if ((Quad = GMPlEstimateQuadratic(PVIdx, Mat, i, Rings)) != NULL)
	        break;
	}
	if (Quad == NULL) {
	    IRIT_WARNING_MSG("Failed to compute quadratic osculating fit; ignored.\n");

	    AttrSetRealAttrib(&V -> Attr, "KCurv", 0.0);
	    AttrSetRealAttrib(&V -> Attr, "HCurv", 0.0);
	    AttrSetRealAttrib(&V -> Attr, "K1Curv", 0.0);
	    AttrSetRealAttrib(&V -> Attr, "K2Curv", 0.0);
	    AttrSetStrAttrib(&V -> Attr, "D1", "0,0,0");
	    AttrSetStrAttrib(&V -> Attr, "D2", "0,0,0");

	    continue;
	}

	A = Quad[3][0];
	B = Quad[4][0];
	C = Quad[5][0];

	/* Compute the curvature values of the osculating quadratic. */
	K = 4 * A * C - IRIT_SQR(B);
	H = A + C;

	AttrSetRealAttrib(&V -> Attr, "KCurv", GMPL_BOUND_CRVTR(K));
	AttrSetRealAttrib(&V -> Attr, "HCurv", GMPL_BOUND_CRVTR(H));

	D = IRIT_SQR(H) - K;
	D = D < 0 ? 0 : sqrt(D);
	k1 = H + D;
	k2 = H - D;

	AttrSetRealAttrib(&V -> Attr, "K1Curv", GMPL_BOUND_CRVTR(k1));
	AttrSetRealAttrib(&V -> Attr, "K2Curv", GMPL_BOUND_CRVTR(k2));

	/* Compute the curvature directions of the osculating quadratic. */
	/* See "Geometric Nodeling with Splines - An Introduction by     */
	/* Cohen/Riesenfeld/Elber, page 85 on rotation of implicit quad. */
	Theta = atan2(B, A - C) * 0.5;

	Dir[0] = cos(Theta);
	Dir[1] = sin(Theta);
	Dir[2] = 0.0;
	IRIT_VEC2D_NORMALIZE(Dir);
	MatMultVecby4by4(E3Dir, Dir, InvMat);
	sprintf(SDirection, "%g, %g, %g", E3Dir[0], E3Dir[1], E3Dir[2]);
	AttrSetStrAttrib(&V -> Attr, "D1", SDirection);

	IRIT_SWAP(IrtRType, Dir[0], Dir[1]);
	Dir[0] = -Dir[0];
	MatMultVecby4by4(E3Dir, Dir, InvMat);
	sprintf(SDirection, "%g, %g, %g", E3Dir[0], E3Dir[1], E3Dir[2]);
	AttrSetStrAttrib(&V -> Attr, "D2", SDirection);
    }

    /* Propagate the curvature properties to all the vertices in PolyList. */
    for (Pl = PolyList; Pl != NULL; Pl = Pl -> Pnext) {
        IPVertexStruct
	    *V = Pl -> PVertex;

	do {
	    int VIndex = AttrGetIntAttrib(V -> Attr, "_VIdx");

	    VIndex = IRIT_ABS(VIndex) - 1;

	    if (!IP_ATTR_IS_BAD_INT(VIndex)) {
	        IPVertexStruct
		    *VOrig = Vertices[VIndex];

		if (V != VOrig) {
		    IP_ATTR_FREE_ATTRS(V -> Attr);
		    V -> Attr = IP_ATTR_COPY_ATTRS(VOrig -> Attr);
		}
	    }
	    else {
	        GEOM_FATAL_ERROR(GEOM_ERR_MISS_VRTX_IDX);
	    }

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    IPPolyVrtxIdxFree(PVIdx);

    PObj -> U.Pl = NULL;
    IPFreeObject(PObj);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Estimates a canonic osculating quadratic at the origin                   *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:       Data structure of mesh.				     *
*   Mat:	 The matrix that take the vertex to the origin and its       *
*		 normal to the Z axis.					     *
*   VrtxIdx:     Index of vertex to process.				     *
*   NumOfRings:  Number of rings around a vertex in the paraboloid fitting.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtPtType *:    The fitted quadratic or NULL if failed.                  *
*****************************************************************************/
static IrtPtType *GMPlEstimateQuadratic(IPPolyVrtxIdxStruct *PVIdx,
					IrtHmgnMatType Mat,
					int VrtxIdx,
					int NumOfRings)
{
    int j,
	*Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, VrtxIdx, NumOfRings);
    IrtPtType ParamDomainPts[GMPL_CRVTR_MAX_FIT], 
	      EuclideanPts[GMPL_CRVTR_MAX_FIT];
    IPVertexStruct
	**Vertices = PVIdx -> Vertices;
    IPVertexStruct
        *V = Vertices[VrtxIdx];

    for (j = 0; Nbrs[j] >= 0; j++) {
	MatMultPtby4by4(ParamDomainPts[j], Vertices[Nbrs[j]] -> Coord, Mat);
	EuclideanPts[j][0] = ParamDomainPts[j][2];
	if (j >= GMPL_CRVTR_MAX_FIT - 2)
	    break;
    }

    if (j <= 2) /* At least 3 constraints are required. */
	return NULL;

#define GMPL_QUAD_ORIGIN
#ifdef GMPL_QUAD_ORIGIN
    MatMultPtby4by4(ParamDomainPts[j], V -> Coord, Mat);
    EuclideanPts[j][0] = ParamDomainPts[j][2];
    j++;
#else
    IRIT_ZAP_MEM(ParamDomainPts[j], sizeof(IrtPtType));
    EuclideanPts[j++][0] = 0.0;
#endif /* GMPL_QUAD_ORIGIN */

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugGMPLCrvtrPrintData, FALSE) {
	    int i;

	    IRIT_INFO_MSG("INPUT DATA TO FITTING CODE:\n");

	    for (i = 0; i < j; i++) {
	        IRIT_WARNING_MSG_PRINTF("\t[ %f  %f    %f]\n",
					ParamDomainPts[i][0],
					ParamDomainPts[i][1],
					EuclideanPts[i][0]);
	    }
	}
    }
#endif /* DEBUG */

    if (GMPlUseCubicFit && j > 10)    /* Needs >=10 constraints for a cubic. */
        return GMSrfCubicQuadOnly(ParamDomainPts, EuclideanPts, FALSE, 1, j);
    else
        return GMSrfQuadricQuadOnly(ParamDomainPts, EuclideanPts, FALSE, 1, j);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the degree for the continuous function we fit at the vertex.        M
*                                                                            *
* PARAMETERS:                                                                M
*   UseCubic:   TRUE to use cubic fit, FALSE for a quadratic                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Old value of fitting degree.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPlCrvtrSetCurvatureAttr                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPlCrvtrSetFitDegree                                                    M
*****************************************************************************/
int GMPlCrvtrSetFitDegree(int UseCubic)
{
    int OldFit = UseCubic;

    GMPlUseCubicFit = UseCubic;

    return OldFit;
}

