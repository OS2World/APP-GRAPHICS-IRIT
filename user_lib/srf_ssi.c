/******************************************************************************
* Srf_ssi.c - surface surface intersection routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 99.					      *
******************************************************************************/

#include "iritprsr.h"
#include "allocate.h"
#include "ip_cnvrt.h"
#include "bool_lib.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "user_loc.h"

#define SRF_SRF_INTER_MERGE_EPS 1e-9

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugSrfSrfInterRecCalls, FALSE);
#endif /* DEBUG */

static void UserSrfSrfInterAux(CagdSrfStruct *Srf1,
			       CagdSrfStruct *Srf2,
			       CagdRType Eps,
			       int AlignSrfs,
			       IPPolygonStruct **Pl1,
			       IPPolygonStruct **Pl2);
static void AlignSrfsOrthogonal(CagdSrfStruct **Srf1,
				CagdSrfStruct **Srf2,
				int OrthoScale);
static void UpdateUVValues(IPVertexStruct *InterVertices,
			   IPVertexStruct *TriangVertices);
static IPPolygonStruct *ConvertFlatSrfToPolys(CagdSrfStruct *Srf);

#ifdef DEBUG
IRIT_STATIC_DATA int
    GlblSSIRecurCalls = 0;
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection curves, if any, of the two given surfaces.     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf1, CSrf2: Two surfaces to compute their intersection points.         M
*   Euclidean:   TRUE for curves in Euclidean space, FALSE for pairs of      M
*		 curves in parametric space.				     M
*   Eps:         Accuracy of computation.  Currently measured in the         M
*		 parametric domain of the surfaces.			     M
*   AlignSrfs:   If non zero surface are aligned by rotation so that         M
*		 bounding boxes are axes parallel to increase chances of     M
*		 no overlap.  If AlignSrfs > 1, stretching is also applied   M
*		 to the surfaces to try and make them orthogonal.	     M
*   Crvs1:       Intersection curves of the first surface.		     M
*		 If Euclidean is TRUE the 3-space curves are returned.       M
*		 If Euclidean is FALSE, curves are returned in UV space.     M
*   Crvs2:       Intersection curves of the second surface.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:	 TRUE if found intersection, FALSE otherwise.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvCrvInter    		                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfSrfInter                                                          M
*****************************************************************************/
int UserSrfSrfInter(const CagdSrfStruct *CSrf1,
		    const CagdSrfStruct *CSrf2,
		    int Euclidean,
		    CagdRType Eps,
		    int AlignSrfs,
		    CagdCrvStruct **Crvs1,
		    CagdCrvStruct **Crvs2)
{
    CagdSrfStruct *TSrf;
    IPPolygonStruct *Pl1, *Pl2;
    CagdSrfStruct *Srf1, *Srf2;

    Srf1 = CagdSrfCopy(CSrf1);
    Srf2 = CagdSrfCopy(CSrf2);

    if (CAGD_IS_BSPLINE_SRF(Srf1) && !BspSrfHasOpenEC(Srf1)) {
	TSrf = BspSrfOpenEnd(Srf1);
	CagdSrfFree(Srf1);
	Srf1 = TSrf;
    }
    if (CAGD_IS_BEZIER_SRF(Srf1)) {
	TSrf = CagdCnvrtBzr2BspSrf(Srf1);
	CagdSrfFree(Srf1);
	Srf1 = TSrf;
    }

    if (CAGD_IS_BSPLINE_SRF(Srf2) && !BspSrfHasOpenEC(Srf2)) {
	TSrf = BspSrfOpenEnd(Srf2);
	CagdSrfFree(Srf2);
	Srf2 = TSrf;
    }
    if (CAGD_IS_BEZIER_SRF(Srf2)) {
	TSrf = CagdCnvrtBzr2BspSrf(Srf2);
	CagdSrfFree(Srf2);
	Srf2 = TSrf;
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugSrfSrfInterRecCalls) 
        GlblSSIRecurCalls = 0;
#endif /* DEBUG */

    UserSrfSrfInterAux(Srf1, Srf2, Eps, AlignSrfs, &Pl1, &Pl2);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugSrfSrfInterRecCalls) 
        IRIT_INFO_MSG_PRINTF("UserSrfSrfInterAux was invoked %d times\n",
			     GlblSSIRecurCalls);
#endif /* DEBUG */

    if (Euclidean) {
	IPPolygonStruct *Pl;
	IPVertexStruct *V;

	for (Pl = Pl1; Pl != NULL; Pl = Pl -> Pnext) {
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		CagdRType
		    *R = CagdSrfEval(Srf1, V -> Coord[0], V -> Coord[1]);

		CagdCoerceToE3(V -> Coord, &R, -1, Srf1 -> PType);
	    }
	}
	for (Pl = Pl2; Pl != NULL; Pl = Pl -> Pnext) {
	    for (V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
		CagdRType
		    *R = CagdSrfEval(Srf2, V -> Coord[0], V -> Coord[1]);

		CagdCoerceToE3(V -> Coord, &R, -1, Srf2 -> PType);
	    }
	}
    }

    if (Pl1 != NULL) {
	Pl1 = GMMergePolylines(Pl1, SRF_SRF_INTER_MERGE_EPS);
	*Crvs1 = UserPolylines2LinBsplineCrvs(Pl1, TRUE);
	IPFreePolygonList(Pl1);
    }
    else
	*Crvs1 = NULL;

    if (Pl2 != NULL) {
	Pl2 = GMMergePolylines(Pl2, SRF_SRF_INTER_MERGE_EPS);
	*Crvs2 = UserPolylines2LinBsplineCrvs(Pl2, TRUE);
	IPFreePolygonList(Pl2);
    }
    else
	*Crvs2 = NULL;

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return *Crvs1 != NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of CagdSrfSrfInter to compute the edges of            *
* intersections.							     *
*****************************************************************************/
static void UserSrfSrfInterAux(CagdSrfStruct *Srf1,
			       CagdSrfStruct *Srf2,
			       CagdRType Eps,
			       int AlignSrfs,
			       IPPolygonStruct **SSIPl1,
			       IPPolygonStruct **SSIPl2)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2,
	t, Du1, Dv1, Du2, Dv2;
    CagdSrfStruct *Srf1a, *Srf1b, *Srf2a, *Srf2b;
    CagdBBoxStruct BBox1, BBox2;

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugSrfSrfInterRecCalls) 
        GlblSSIRecurCalls++;
#endif /* DEBUG */

    *SSIPl1 = *SSIPl2 = NULL;

    Srf1 = CagdSrfCopy(Srf1);
    Srf2 = CagdSrfCopy(Srf2);
    if (AlignSrfs)
	AlignSrfsOrthogonal(&Srf1,& Srf2, AlignSrfs > 1);

    /* If the two surfaces do not overlap in 3-space we cannot have inters. */
    CagdSrfBBox(Srf1, &BBox1);
    CagdSrfBBox(Srf2, &BBox2);
    if (BBox1.Min[0] > BBox2.Max[0] ||
	BBox1.Min[1] > BBox2.Max[1] ||
	BBox1.Min[2] > BBox2.Max[2] ||
	BBox2.Min[0] > BBox1.Max[0] ||
	BBox2.Min[1] > BBox1.Max[1] ||
	BBox2.Min[2] > BBox1.Max[2]) {
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);
	return;
    }

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    Du1 = UMax1 - UMin1;
    Dv1 = VMax1 - VMin1;
    Du2 = UMax2 - UMin2;
    Dv2 = VMax2 - VMin2;

    /* Subdivide the two surfaces if necessary. */
    if (Du1 > Dv1) {
	if (Du1 > Eps) {
	    t = Srf1 -> ULength > Srf1 -> UOrder
		? Srf1 -> UKnotVector[(Srf1 -> ULength + Srf1 -> UOrder) >> 1]
		: (UMin1 + UMax1) * 0.5;
	    
	    /* Subdivide in the U direction. */
	    Srf1a = CagdSrfSubdivAtParam(Srf1, t, CAGD_CONST_U_DIR);
	    Srf1b = Srf1a -> Pnext;
	    Srf1a -> Pnext = NULL;
	}
	else {
	    Srf1a = Srf1;
	    Srf1b = NULL;
	}
    }
    else {
	if (Dv1 > Eps) {
	    t = Srf1 -> VLength > Srf1 -> VOrder
	        ? Srf1 -> VKnotVector[(Srf1 -> VLength + Srf1 -> VOrder) >> 1]
		: (VMin1 + VMax1) * 0.5;

	    /* Subdivide in the V direction. */
	    Srf1a = CagdSrfSubdivAtParam(Srf1, t, CAGD_CONST_V_DIR);
	    Srf1b = Srf1a -> Pnext;
	    Srf1a -> Pnext = NULL;
	}
	else {
	    Srf1a = Srf1;
	    Srf1b = NULL;
	}
    }
    
    if (Du2 > Dv2) {
	if (Du2 > Eps) {
	    t = Srf2 -> ULength > Srf2 -> UOrder
		? Srf2 -> UKnotVector[(Srf2 -> ULength + Srf2 -> UOrder) >> 1]
		: (UMin2 + UMax2) * 0.5;

	    /* Subdivide in the U direction. */
	    Srf2a = CagdSrfSubdivAtParam(Srf2, t, CAGD_CONST_U_DIR);
	    Srf2b = Srf2a -> Pnext;
	    Srf2a -> Pnext = NULL;
	}
	else {
	    Srf2a = Srf2;
	    Srf2b = NULL;
	}
    }
    else {
	if (Dv2 > Eps) {
	    t = Srf2 -> VLength > Srf2 -> VOrder
		? Srf2 -> VKnotVector[(Srf2 -> VLength + Srf2 -> VOrder) >> 1]
		: (VMin2 + VMax2) * 0.5;

	    /* Subdivide in the V direction. */
	    Srf2a = CagdSrfSubdivAtParam(Srf2, t, CAGD_CONST_V_DIR);
	    Srf2b = Srf2a -> Pnext;
	    Srf2a -> Pnext = NULL;
	}
	else {
	    Srf2a = Srf2;
	    Srf2b = NULL;
	}
    }

    if (Srf1b == NULL && Srf2b == NULL) {
	/* The two surfaces are small enough - convert to polygons and       */
	/* intersect the pieces.				             */
	IPPolygonStruct *Poly1, *Poly2,
	    *Pl1 = ConvertFlatSrfToPolys(Srf1a),
	    *Pl2 = ConvertFlatSrfToPolys(Srf2a);

	/* Intersect the two sets of polygons. */
	for (Poly1 = Pl1; Poly1 != NULL; Poly1 = Poly1 -> Pnext) {
	    for (Poly2 = Pl2; Poly2 != NULL; Poly2 = Poly2 -> Pnext) {
		IPPolygonStruct *Inter2,
		    *Inter1 = BoolInterPolyPoly(Poly1, Poly2);

		if (Inter1 != NULL) {
		    /* Make a copy as we need two segments - one for each    */
		    /* surface's intersection edge.			     */
		    Inter2 = IPCopyPolygonList(Inter1);
			
		    UpdateUVValues(Inter1 -> PVertex, Poly1 -> PVertex);
		    IRIT_LIST_PUSH(Inter1, *SSIPl1);

		    UpdateUVValues(Inter2 -> PVertex, Poly2 -> PVertex);
		    IRIT_LIST_PUSH(Inter2, *SSIPl2);
		}
	    }
	}

	IPFreePolygonList(Pl1);
	IPFreePolygonList(Pl2);
    }
    else {
	IPPolygonStruct *SSIPl1Tmp, *SSIPl2Tmp;

	/* Recurse on the sub surfaces. */
	UserSrfSrfInterAux(Srf1a, Srf2a, Eps, AlignSrfs, SSIPl1, SSIPl2);
	if (Srf1b) {
	    UserSrfSrfInterAux(Srf1b, Srf2a, Eps, AlignSrfs,
			       &SSIPl1Tmp, &SSIPl2Tmp);
	    *SSIPl1 = IPAppendPolyLists(*SSIPl1, SSIPl1Tmp);
	    *SSIPl2 = IPAppendPolyLists(*SSIPl2, SSIPl2Tmp);
	    if (Srf2b) {
		UserSrfSrfInterAux(Srf1a, Srf2b, Eps, AlignSrfs,
				   &SSIPl1Tmp, &SSIPl2Tmp);
		*SSIPl1 = IPAppendPolyLists(*SSIPl1, SSIPl1Tmp);
		*SSIPl2 = IPAppendPolyLists(*SSIPl2, SSIPl2Tmp);

		UserSrfSrfInterAux(Srf1b, Srf2b, Eps, AlignSrfs,
				   &SSIPl1Tmp, &SSIPl2Tmp);
		*SSIPl1 = IPAppendPolyLists(*SSIPl1, SSIPl1Tmp);
		*SSIPl2 = IPAppendPolyLists(*SSIPl2, SSIPl2Tmp);
	    }
	}
	else {
	    if (Srf2b) {
		UserSrfSrfInterAux(Srf1a, Srf2b, Eps, AlignSrfs,
				   &SSIPl1Tmp, &SSIPl2Tmp);
		*SSIPl1 = IPAppendPolyLists(*SSIPl1, SSIPl1Tmp);
		*SSIPl2 = IPAppendPolyLists(*SSIPl2, SSIPl2Tmp);
	    }
	}
    }

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    if (Srf1b != NULL) {
	CagdSrfFree(Srf1a);
	CagdSrfFree(Srf1b);
    }

    if (Srf2b != NULL) {
	CagdSrfFree(Srf2a);
	CagdSrfFree(Srf2b);
    }
}
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Align the two surfaces, using linear transformations so that the normals *
* of the two surfaces at the center of the parametric domain, or orthogonal. *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:  Two surface to align, in place, so that their centeral      *
*		 normal is orthogonal.					     *
*   OrthoScale:  If TRUE objects are also scaled so as to have orthogonal    *
*		 normals.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void AlignSrfsOrthogonal(CagdSrfStruct **Srf1,
				CagdSrfStruct **Srf2,
				int OrthoScale)
{
    CagdRType UMin, UMax, VMin, VMax, R;
    IrtVecType V;
    IrtHmgnMatType Mat1, Mat2;
    CagdVecStruct N1, N2;
    CagdSrfStruct *TSrf;

    if (OrthoScale) {
	CagdSrfDomain(*Srf1, &UMin, &UMax, &VMin, &VMax);
	N1 = *CagdSrfNormal(*Srf1, (UMin + UMax) * 0.5,
			          (VMin + VMax) * 0.5, TRUE);
    
	CagdSrfDomain(*Srf2, &UMin, &UMax, &VMin, &VMax);
	N2 = *CagdSrfNormal(*Srf2, (UMin + UMax) * 0.5,
			          (VMin + VMax) * 0.5, TRUE);

	/* Two normals are parallel - little we can do here. */
	if (IRIT_FABS(IRIT_DOT_PROD(N1.Vec, N2.Vec)) >= 1.0 - IRIT_UEPS)
	    return;

	/* Bring the vector V into the Z axis. */
	IRIT_VEC_SUB(V, N1.Vec, N2.Vec);
	R = IRIT_VEC_LENGTH(V);
	IRIT_VEC_SCALE(V, 1.0 / R);
	GMGenMatrixZ2Dir(Mat1, V);
	MatTranspMatrix(Mat1, Mat2);		     /* Compute the inverse. */

	/* Scale in Z until N1 and N2 are orthogonal. */
	R = 0.5 * R;
	R = R / sqrt(1.0 - IRIT_SQR(R));
	MatGenMatScale(1.0, 1.0, IRIT_BOUND(R, 0.5, 2.0), Mat1);
	MatMultTwo4by4(Mat2, Mat2, Mat1);

	TSrf = CagdSrfMatTransform(*Srf1, Mat2);
	CagdSrfFree(*Srf1);
	*Srf1 = TSrf;

	TSrf = CagdSrfMatTransform(*Srf2, Mat2);
	CagdSrfFree(*Srf2);
	*Srf2 = TSrf;
    }

    /* Rotate N2 to the Z axis. */
    CagdSrfDomain(*Srf2, &UMin, &UMax, &VMin, &VMax);
    N2 = *CagdSrfNormal(*Srf2, (UMin + UMax) * 0.5, (VMin + VMax) * 0.5, TRUE);
    GMGenMatrixZ2Dir(Mat1, N2.Vec);
    MatTranspMatrix(Mat1, Mat2);		     /* Compute the inverse. */

    TSrf = CagdSrfMatTransform(*Srf1, Mat2);
    CagdSrfFree(*Srf1);
    *Srf1 = TSrf;

    TSrf = CagdSrfMatTransform(*Srf2, Mat2);
    CagdSrfFree(*Srf2);
    *Srf2 = TSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the UV values of the intersection points of the intersecting    *
* segment.  Updates the UV values into InterVertices, in place.              *
*                                                                            *
* PARAMETERS:                                                                *
*   InterVertices:   Two vertices of segment of interesection in R3.         *
*   TriangVertices:  The three vertices of intersecting triangle in R3       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateUVValues(IPVertexStruct *InterVertices,
			   IPVertexStruct *TriangVertices)
{
    int i;
    IPVertexStruct *V,
	*V1 = TriangVertices,
	*V2 = V1 -> Pnext,
	*V3 = V2 -> Pnext;
    float
	*UV1 = AttrGetUVAttrib(V1 -> Attr, "uvvals"),
	*UV2 = AttrGetUVAttrib(V2 -> Attr, "uvvals"),
	*UV3 = AttrGetUVAttrib(V3 -> Attr, "uvvals");

    if (UV1 == NULL || UV2 == NULL || UV3 == NULL) {
	USER_FATAL_ERROR(USER_ERR_MISSING_ATTRIB);
	return;
    }
	
    for (V = InterVertices; V != NULL; V = V -> Pnext) {
	IrtRType
	    *W = GMBaryCentric3Pts(V1 -> Coord,
				   V2 -> Coord,
				   V3 -> Coord,
				   V -> Coord);
	for (i = 0; i < 2; i++)
	    V -> Coord[i] = UV1[i] * W[0] + UV2[i] * W[1] + UV3[i] * W[2];
	V -> Coord[2] = 0.0;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Convert the given surface to two triangles.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:      An "almost flat" surface to convert to four polygons.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   The two triangels.                                  *
*****************************************************************************/
static IPPolygonStruct *ConvertFlatSrfToPolys(CagdSrfStruct *Srf)
{
    int ULen1 = Srf -> ULength - 1,
	VLen1 = Srf -> VLength - 1;
    CagdPointType
	PType = Srf -> PType;
    IrtRType UMin, UMax, VMin, VMax,
	**Points = Srf -> Points;
    IPVertexStruct
	*VList1 = IPAllocVertex2(IPAllocVertex2(IPAllocVertex2(NULL))),
	*VList2 = IPAllocVertex2(IPAllocVertex2(IPAllocVertex2(NULL)));
    IPPolygonStruct
        *Pl = IPAllocPolygon(0, VList1, IPAllocPolygon(0, VList2, NULL));

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax); \

    CagdCoerceToE3(VList1 -> Coord, Points, CAGD_MESH_UV(Srf, 0, 0), PType);
    CagdCoerceToE3(VList1 -> Pnext -> Coord, Points,
		   CAGD_MESH_UV(Srf, ULen1, 0), PType);
    CagdCoerceToE3(VList1 -> Pnext -> Pnext -> Coord, Points,
		   CAGD_MESH_UV(Srf, ULen1, VLen1), PType);
    AttrSetUVAttrib(&VList1 -> Attr, "uvvals", UMin, VMin);
    AttrSetUVAttrib(&VList1 -> Pnext -> Attr, "uvvals", UMax, VMin);
    AttrSetUVAttrib(&VList1 -> Pnext -> Pnext -> Attr, "uvvals", UMax, VMax);

    VList1 -> Pnext -> Pnext -> Pnext = VList1;

    CagdCoerceToE3(VList2 -> Coord, Points, CAGD_MESH_UV(Srf, 0, 0), PType);
    CagdCoerceToE3(VList2 -> Pnext -> Coord, Points,
		   CAGD_MESH_UV(Srf, ULen1, VLen1), PType);
    CagdCoerceToE3(VList2 -> Pnext -> Pnext -> Coord, Points,
		   CAGD_MESH_UV(Srf, 0, VLen1), PType);
    AttrSetUVAttrib(&VList2 -> Attr, "uvvals", UMin, VMin);
    AttrSetUVAttrib(&VList2 -> Pnext -> Attr, "uvvals", UMax, VMax);
    AttrSetUVAttrib(&VList2 -> Pnext -> Pnext -> Attr, "uvvals", UMin, VMax);
    VList2 -> Pnext -> Pnext -> Pnext = VList2;

    IPUpdatePolyPlane(Pl);
    IPUpdatePolyPlane(Pl -> Pnext);

    return Pl;
}
