/******************************************************************************
* SrfPGeom.c - fast surface primitive-geometry intersection testing.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 95.					      *
******************************************************************************/

#include "irit_sm.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "user_loc.h"

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugCountHierarchyTraversal, FALSE);
IRIT_STATIC_DATA int
    _DebugCountHTraversal = 0;
#endif /* DEBUG */

typedef struct IntrSrfHierarchyStruct {
    struct IntrSrfHierarchyStruct *Right, *Left;
    CagdBBoxStruct BBox;
    CagdRType BBoxRadius;
    CagdPType BBoxCenter;
    CagdPType BBoxCorners[8];
    CagdPolygonStruct *Triangles;
} IntrSrfHierarchyStruct;

IRIT_STATIC_DATA CagdRType
    GlblMinRayParam = IRIT_INFNTY,
    GlblMinDistSqr = IRIT_INFNTY,
    GlblMaxDistSqr = 0;
IRIT_STATIC_DATA CagdUVType
    GlblUV;

static void IntrSrfHierarchyUpdateBBoxCorners(IntrSrfHierarchyStruct *ISH);
static VoidPtr IntrSrfHierarchyPreprocessSrfAux(CagdPolygonStruct *Tris);
static void IntrSrfHierarchyTestRayAux(IntrSrfHierarchyStruct *Handle,
				       CagdPType RayOrigin,
				       CagdVType RayDir,
				       CagdUVType InterUV);
static CagdBType RayIntersectBBox(CagdPType RayOrigin,
				  CagdVType RayDir,
				  CagdBBoxStruct *BBox);
static int RayIntersectTriangle(CagdPType RayOrigin,
				CagdVType RayDir,
				CagdPolygonStruct *Tri);
static void IntrSrfHierarchyTestPtAux(IntrSrfHierarchyStruct *ISH,
				      CagdPType Pt,
				      CagdBType Nearest);
static CagdBType PtMinMaxDist2BBox(CagdPType Pt,
				   CagdBType Nearest,
				   IntrSrfHierarchyStruct *ISH);
static int PtMinMaxTriangle(CagdPType Pt,
			    CagdBType Nearest,
			    CagdPolygonStruct *Tri);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Preprocess a surface for fast computation of ray-surface intersection.   M
* Returns NULL if fails, otherwise a pointer to preprocessed data structure. M
*   The preprocessed data is in fact a hierarchy of bounding boxes extracted M
* while the surface is being polygonized.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To preprocess.                                               M
*   FineNess:   Control on accuracy, the higher the finer.                   M
*		The surface will be subdivided into approximately FineNess   M
*		regions in each of the two parametric directions.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  A handle on the preprocessed data, NULL if error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IntrSrfHierarchyFreePreprocess, IntrSrfHierarchyTestRay,                 M
*   IntrSrfHierarchyTestPt						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IntrSrfHierarchyPreprocessSrf, ray surface intersection                  M
*****************************************************************************/
VoidPtr IntrSrfHierarchyPreprocessSrf(const CagdSrfStruct *Srf,
				      IrtRType FineNess)
{
    int OldTriOnly = CagdSrfSetMakeOnlyTri(TRUE);
    CagdPolygonStruct
        *Tris = CagdSrfAdap2Polygons(Srf, FineNess, TRUE,
				     TRUE, TRUE, NULL);

    CagdSrfSetMakeOnlyTri(OldTriOnly);

    if (Tris == NULL)
        return NULL;

    return IntrSrfHierarchyPreprocessSrfAux(Tris);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of IntrSrfHierarchyPreprocessSrf.  Divides the list   *
* of polygons into a binary tree until nodes are small (2 triangles or one   *
* triangles engulfs several triangles).  Tris cannot be NULL here!           *
*****************************************************************************/
static VoidPtr IntrSrfHierarchyPreprocessSrfAux(CagdPolygonStruct *Tris)
{
    int SubdivDim;
    IrtRType SubdivVal, VMin, VMax;
    IrtVecType BBoxDim;
    CagdPolygonStruct *Tris1, *Tris2, *Tri;
    IntrSrfHierarchyStruct
	*ISH = (IntrSrfHierarchyStruct *)
				    IritMalloc(sizeof(IntrSrfHierarchyStruct));

    ISH -> Left = ISH -> Right = NULL;
    ISH -> Triangles = NULL;
    CagdPolygonListBBox(Tris, &ISH -> BBox);
    IntrSrfHierarchyUpdateBBoxCorners(ISH);

    /* If we have 2 triangles or less, stop now. */
    if (Tris -> Pnext == NULL || Tris -> Pnext -> Pnext == NULL) {
        ISH -> Triangles = Tris;
	return ISH;
    }

    /* Decide which dimension to use (the largest) to divide the space. */
    IRIT_VEC_SUB(BBoxDim, ISH -> BBox.Max, ISH -> BBox.Min);
    if (BBoxDim[0] > BBoxDim[1] && BBoxDim[0] > BBoxDim[2])
        SubdivDim = 0;
    else
        SubdivDim = BBoxDim[1] > BBoxDim[2] ? 1 : 2;
    SubdivVal = (ISH -> BBox.Min[SubdivDim] +
		 ISH -> BBox.Max[SubdivDim]) * 0.5;
    
    /* Divide the polygonal list into to based on SubdivDim/Val. */
    for (Tris1 = Tris2 = NULL; Tris != NULL; ) {
        IRIT_LIST_POP(Tri, Tris);

	assert(Tri -> PolyType == CAGD_POLYGON_TYPE_TRIANGLE);

	VMin = IRIT_MIN(IRIT_MIN(Tri -> U.Polygon[0].Pt[SubdivDim],
		       Tri -> U.Polygon[1].Pt[SubdivDim]),
		   Tri -> U.Polygon[2].Pt[SubdivDim]);
	VMax = IRIT_MAX(IRIT_MAX(Tri -> U.Polygon[0].Pt[SubdivDim],
		       Tri -> U.Polygon[1].Pt[SubdivDim]),
		   Tri -> U.Polygon[2].Pt[SubdivDim]);

	if (VMin >= SubdivVal)
	    IRIT_LIST_PUSH(Tri, Tris2)
    	else if (VMax <= SubdivVal)
	    IRIT_LIST_PUSH(Tri, Tris1)
	else {
	    if (VMax - SubdivVal > SubdivVal - VMin) /* Tri cross SubdivVal. */
	        IRIT_LIST_PUSH(Tri, Tris2)
	    else
	        IRIT_LIST_PUSH(Tri, Tris1);
	}
    }

    if (Tris1 == NULL || Tris2 == NULL) {
        /* All trinagles are concentrated in one side - done with hierarchy. */
        ISH -> Triangles = Tris1 == NULL ? Tris2 : Tris1;
    }
    else {
        /* Recursively process the two new lists. */
        ISH -> Left = IntrSrfHierarchyPreprocessSrfAux(Tris1);
	ISH -> Right = IntrSrfHierarchyPreprocessSrfAux(Tris2);
    }

    return ISH;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the 8 corners of a bbox into thg given hierarchy node, and      *
* derive the bbox center and radius.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   ISH:   Node to update its 8 corners/center/radius of the bbox.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IntrSrfHierarchyUpdateBBoxCorners(IntrSrfHierarchyStruct *ISH)
{
    int i, j, k, n;

    for (i = n = 0; i < 2; i++) {
	for (j = 0; j < 2; j++) {
	    for (k = 0; k < 2; k++, n++) {
	        ISH -> BBoxCorners[n][0] =
			     i == 0 ? ISH -> BBox.Min[0] : ISH -> BBox.Max[0];
	        ISH -> BBoxCorners[n][1] =
			     j == 0 ? ISH -> BBox.Min[1] : ISH -> BBox.Max[1];
	        ISH -> BBoxCorners[n][2] =
			     k == 0 ? ISH -> BBox.Min[2] : ISH -> BBox.Max[2];
	    }
	}
    }

    IRIT_PT_BLEND(ISH -> BBoxCenter, ISH -> BBox.Min, ISH -> BBox.Max, 0.5);
    ISH -> BBoxRadius = IRIT_PT_PT_DIST(ISH -> BBox.Min, ISH -> BBox.Max) * 0.5;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Releases the pre processed data structed created by the function         M
* IntrSrfHierarchyPreprocessSrf.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Handle:    As returned by IntrSrfHierarchyPreprocessSrf to release.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IntrSrfHierarchyPreprocessSrf, IntrSrfHierarchyTestRay,                  M
*   IntrSrfHierarchyTestPt						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IntrSrfHierarchyFreePreprocess, ray surface intersection                 M
*****************************************************************************/
void IntrSrfHierarchyFreePreprocess(VoidPtr Handle)
{
    IntrSrfHierarchyStruct
	*ISH = (IntrSrfHierarchyStruct *) Handle;

    if (ISH -> Left != NULL)
	IntrSrfHierarchyFreePreprocess(ISH -> Left);
    if (ISH -> Right != NULL)
	IntrSrfHierarchyFreePreprocess(ISH -> Right);
    if (ISH -> Triangles)
	CagdPolygonFreeList(ISH -> Triangles);
    IritFree(ISH);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the first intersection of a given ray with the given surface,   M
* if any. If TRUE is returned, the InterUV is updated to the interesection.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Handle:    As returned by IntrSrfHierarchyPreprocessSrf.                 M
*   RayOrigin: Starting point of ray.                                        M
*   RayDir:    Direction of ray.                                             M
*   InterUV:   The UV surface coordinates of the first ray surface 	     M
*	       intersection location is saved here.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType: TRUE if found intersection, FALSE otherwise.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   IntrSrfHierarchyFreePreprocess, IntrSrfHierarchyPreprocessSrf,           M
*   IntrSrfHierarchyTestPt						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IntrSrfHierarchyTestRay, ray surface intersection                        M
*****************************************************************************/
CagdBType IntrSrfHierarchyTestRay(VoidPtr Handle,
				 CagdPType RayOrigin,
				 CagdVType RayDir,
				 CagdUVType InterUV)
{
    IntrSrfHierarchyStruct
	*ISH = (IntrSrfHierarchyStruct *) Handle;

    GlblMinRayParam = IRIT_INFNTY;
    GlblUV[0] = GlblUV[1] = -IRIT_INFNTY;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCountHierarchyTraversal)
        _DebugCountHTraversal = 0;
#   endif /* DEBUG */

    IntrSrfHierarchyTestRayAux(ISH, RayOrigin, RayDir, InterUV);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCountHierarchyTraversal)
        fprintf(stderr, "Hierarchy ray-srf traversal of %d nodes\n",
		_DebugCountHTraversal);
#   endif /* DEBUG */

    InterUV[0] = GlblUV[0];
    InterUV[1] = GlblUV[1];

    return GlblMinRayParam < IRIT_INFNTY;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of IntrSrfHierarchyTestRayAux.	                     *
*****************************************************************************/
static void IntrSrfHierarchyTestRayAux(IntrSrfHierarchyStruct *ISH,
				       CagdPType RayOrigin,
				       CagdVType RayDir,
				       CagdUVType InterUV)
{
#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCountHierarchyTraversal)
        _DebugCountHTraversal++;
#   endif /* DEBUG */

    if (RayIntersectBBox(RayOrigin, RayDir, &ISH -> BBox)) {
	if (ISH -> Triangles != NULL) {
	    CagdPolygonStruct *Tri;

	    for (Tri = ISH -> Triangles; Tri != NULL; Tri = Tri -> Pnext)
	        RayIntersectTriangle(RayOrigin, RayDir, Tri);
	}
	else {
	    if (ISH -> Left != NULL)
		IntrSrfHierarchyTestRayAux(ISH -> Left, RayOrigin,
					   RayDir, InterUV);
	    if (ISH -> Right != NULL)
		IntrSrfHierarchyTestRayAux(ISH -> Right, RayOrigin,
					   RayDir, InterUV);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests if the given ray intersects the given bounding box.                *
*                                                                            *
* PARAMETERS:                                                                *
*   RayOrigin: Starting point of ray.                                        *
*   RayDir:    Direction of ray.                                             *
*   BBox:      To test against ray.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE if intersects, FALSE otherwise.                         *
*****************************************************************************/
static CagdBType RayIntersectBBox(CagdPType RayOrigin,
				  CagdVType RayDir,
				  CagdBBoxStruct *BBox)
{
    int i, j;

    for (i = 0; i < 3; i++) {
	CagdRType T1, T2;
	CagdPType P1, P2;

	if (RayDir[i] != 0) {
	    /* Computes the t values of the intersection locations of the  */
	    /* ray with the six faces of the bounding box.		   */
	    T1 = (BBox -> Min[i] - RayOrigin[i]) / RayDir[i];
	    T2 = (BBox -> Max[i] - RayOrigin[i]) / RayDir[i];

	    /* Compute intersection points. */ 
	    for (j = 0; j < 3; j++) {
		if (j == i) {
		    P1[j] = BBox -> Min[i];
		    P2[j] = BBox -> Max[i];
		}
		else {
		    P1[j] = RayOrigin[j] + T1 * RayDir[j];
		    P2[j] = RayOrigin[j] + T2 * RayDir[j];
		}
	    }

	    /* Is the intersction point of either P1 or P2 inside BBox? */
	    if (GM_BBOX_HOLD_PT(P1, BBox, IRIT_EPS) ||
		GM_BBOX_HOLD_PT(P2, BBox, IRIT_EPS))
	        return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests if the given ray intersects the given triangle. Updates the global *
* GlblMinRayParam and GlblUV if closest intersection so far.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   RayOrigin: Starting point of ray.                                        *
*   RayDir:    Direction of ray.                                             *
*   Tri:       Triangle to test against ray for intersection.                *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if succesful, FALSE otherwise.                           *
*****************************************************************************/
static int RayIntersectTriangle(CagdPType RayOrigin,
				CagdVType RayDir,
				CagdPolygonStruct *Tri)
{
    int i;
    CagdRType t, *w;
    CagdPType InterPoint;
    IrtPlnType Plane;

    if (!GMPlaneFrom3Points(Plane,
			    Tri -> U.Polygon[0].Pt,
			    Tri -> U.Polygon[1].Pt,
			    Tri -> U.Polygon[2].Pt) ||
	!GMPointFromLinePlane(RayOrigin, RayDir, Plane, InterPoint, &t))
        return FALSE;

    /* Test if new intersection is not closer than previous one. */
    if (t > GlblMinRayParam)
	return TRUE;

    if ((w = GMBaryCentric3Pts(Tri -> U.Polygon[0].Pt,
			       Tri -> U.Polygon[1].Pt,
			       Tri -> U.Polygon[2].Pt,
			       InterPoint)) == NULL)
	return FALSE;

    /* A new intersection. Update global state. */
    GlblMinRayParam = t;

    for (i = 0; i < 2; i++)
	GlblUV[i] = Tri -> U.Polygon[0].UV[i] * w[0] +
	            Tri -> U.Polygon[1].UV[i] * w[1] +
		    Tri -> U.Polygon[2].UV[i] * w[2];
	             

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the nearest/farthest location on the surface from point Pt.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Handle:    As returned by IntrSrfHierarchyPreprocessSrf.                 M
*   Pt:        To look for nearest/farthest location on the surface.         M
*   Nearest:   TRUE for nearest, FALSE for farthest.                         M
*   InterUV:   The UV surface coordinates of the nearest/farthest surface    M
*	       location is saved here.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType: TRUE if found min/max distance, FALSE otherwise.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IntrSrfHierarchyPreprocessSrf, IntrSrfHierarchyFreePreprocess,	     M
*   IntrSrfHierarchyTestRay                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   IntrSrfHierarchyTestPt, pt surface min/max distance                      M
*****************************************************************************/
CagdBType IntrSrfHierarchyTestPt(VoidPtr Handle,
				 CagdPType Pt,
				 CagdBType Nearest,
				 CagdUVType InterUV)
{
    IntrSrfHierarchyStruct
	*PtSrf = (IntrSrfHierarchyStruct *) Handle;

    if (Nearest)
        GlblMinDistSqr = IRIT_INFNTY;
    else
	GlblMaxDistSqr = 0.0;

    GlblUV[0] = GlblUV[1] = -IRIT_INFNTY;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCountHierarchyTraversal)
        _DebugCountHTraversal = 0;
#   endif /* DEBUG */

    IntrSrfHierarchyTestPtAux(PtSrf, Pt, Nearest);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCountHierarchyTraversal)
        fprintf(stderr, "Hierarchy pt-srf traversal of %d nodes\n",
		_DebugCountHTraversal);
#   endif /* DEBUG */

    InterUV[0] = GlblUV[0];
    InterUV[1] = GlblUV[1];

    if (Nearest)
        return GlblMinDistSqr < IRIT_INFNTY;
    else
        return GlblMaxDistSqr > 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of IntrSrfHierarchyTestPtAux.	                     *
*****************************************************************************/
static void IntrSrfHierarchyTestPtAux(IntrSrfHierarchyStruct *ISH,
				      CagdPType Pt,
				      CagdBType Nearest)
{
#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCountHierarchyTraversal)
        _DebugCountHTraversal++;
#   endif /* DEBUG */
	  
    if (PtMinMaxDist2BBox(Pt, Nearest, ISH)) {
	if (ISH -> Triangles != NULL) {
	    CagdPolygonStruct *Tri;

	    for (Tri = ISH -> Triangles; Tri != NULL; Tri = Tri -> Pnext)
	        PtMinMaxTriangle(Pt, Nearest, Tri);
	}
	else {
	    if (ISH -> Left != NULL)
		IntrSrfHierarchyTestPtAux(ISH -> Left, Pt, Nearest);
	    if (ISH -> Right != NULL)
		IntrSrfHierarchyTestPtAux(ISH -> Right, Pt, Nearest);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests for the minimal/maximal distance of Pt from given bounding box.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:        To look for nearest/farthest location on the surface.         *
*   Nearest:   TRUE for nearest, FALSE for farthest.                         *
*   ISH:       Hierarchy node to examine Pt distance to its bbox.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType: TRUE if should examine content of this BBOX, FALSE otherwise. *
*****************************************************************************/
static CagdBType PtMinMaxDist2BBox(CagdPType Pt,
				   CagdBType Nearest,
				   IntrSrfHierarchyStruct *ISH)
{
    int i;
    CagdRType d;

    if (Nearest) {
        /* Needs to examine this BBox's content only if its minimal distance */
        /* to Pt is smaller than GlblMinDistParam.			     */

        /* Get a bound on the minimal distance to the bbox by using a        */
        /* bounding sphere to thebbox.					     */
        if ((d = IRIT_PT_PT_DIST(Pt, ISH -> BBoxCenter) - ISH -> BBoxRadius) < 0)
	    return TRUE;
	
        return IRIT_SQR(d) < GlblMinDistSqr;
    }
    else {
        /* Needs to examine this BBox's content only if its maximal distance */
        /* to Pt is larger than GlblMaxDistParam.			     */
 
        /* Maximal distance to a bbox could occur at the vertices of the     */
        /* bbox only.							     */
        for (i = 0; i < 8; i++) {
	    if (IRIT_PT_PT_DIST_SQR(Pt, ISH -> BBoxCorners[i]) > GlblMaxDistSqr)
	        return TRUE;
        }
	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests if the given ray intersects the given triangle. Updates the global *
* GlblMinPtParam and GlblUV if closest intersection so far.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:        To look for nearest/farthest location to the triangle Tri.    *
*   Nearest:   TRUE for nearest, FALSE for farthest.                         *
*   Tri:       Triangle to find its nearest/farthest location to Pt.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if successful, FALSE otherwise.                          *
*****************************************************************************/
static int PtMinMaxTriangle(CagdPType Pt,
			    CagdBType Nearest,
			    CagdPolygonStruct *Tri)
{
    int i;
    CagdRType DSqr, t, *w;
    IrtPlnType Plane;

    if (Nearest) {
        CagdPType ClosestPoint;

        /* Nearest location can occur on one of the three vertices. */
	for (i = 0; i < 3; i++) {
	    DSqr = IRIT_PT_PT_DIST_SQR(Pt, Tri -> U.Polygon[i].Pt);
	    if (GlblMinDistSqr > DSqr) {
	        GlblMinDistSqr = DSqr;
		GlblUV[0] = Tri -> U.Polygon[i].UV[0];
		GlblUV[1] = Tri -> U.Polygon[i].UV[1];
	    }
	}

        /* Nearest location can also occur on one of the three edges. */
	for (i = 0; i < 3; i++) {
	    int j = (i + 1) % 3;
	    CagdRType R1, R12;
	    CagdVType V1, V2;

	    IRIT_VEC_SUB(V1, Tri -> U.Polygon[i].Pt, Tri -> U.Polygon[j].Pt);
	    if ((R1 = IRIT_VEC_LENGTH(V1)) <= 0.0)
		continue;
	    IRIT_VEC_SCALE(V1, 1.0 / R1);

	    IRIT_VEC_SUB(V2, Tri -> U.Polygon[i].Pt, Pt);

	    R12 = IRIT_DOT_PROD(V1, V2);

	    /* Ortho. projection of Pt on the line is between two vertices. */
	    if (R12 < R1 && R12 > 0.0) {
	        IRIT_VEC_SCALE(V1, R12);
		IRIT_PT_ADD(ClosestPoint, Tri -> U.Polygon[i].Pt, V1);

	        DSqr = IRIT_PT_PT_DIST_SQR(Pt, ClosestPoint);
		if (GlblMinDistSqr > DSqr) {
		    GlblMinDistSqr = DSqr;
		    t = R12 / R1;
		    GlblUV[0] = IRIT_BLEND(Tri -> U.Polygon[j].UV[0],
				           Tri -> U.Polygon[i].UV[0], t);
		    GlblUV[1] = IRIT_BLEND(Tri -> U.Polygon[j].UV[1],
				           Tri -> U.Polygon[i].UV[1], t);
		}
	    }
	}

        /* Nearest location can also occur inside the triangle. */
	if (GMPlaneFrom3Points(Plane,
			       Tri -> U.Polygon[0].Pt,
			       Tri -> U.Polygon[1].Pt,
			       Tri -> U.Polygon[2].Pt) &&
	    GMPointFromPointPlane(Pt, Plane, ClosestPoint) &&
	    ((w = GMBaryCentric3Pts(Tri -> U.Polygon[0].Pt,
				    Tri -> U.Polygon[1].Pt,
				    Tri -> U.Polygon[2].Pt,
				    ClosestPoint)) != NULL)) {
	    for (i = 0; i < 2; i++)
	        GlblUV[i] = Tri -> U.Polygon[0].UV[i] * w[0] +
		            Tri -> U.Polygon[1].UV[i] * w[1] +
		            Tri -> U.Polygon[2].UV[i] * w[2];
	}
    }

    return TRUE;
}
