/******************************************************************************
* PlyImprt.c - importance measures over polygonal meshes.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep 2004.	                                      *
******************************************************************************/

#include <math.h>
#include "geom_loc.h"

static IrtRType GMPlFindSilImp(IPVertexStruct *VOrig, IPPolyPtrStruct *Pls);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimates the importance of vertices of a polygonal mesh based on the    M
* probability of their adjacent edges to posses silhouettes.		     M
*   Mesh is assumed to be a triangular regular mesh.			     M
*   Each vertex in the mesh is assigned a new "SilImp" attribute which is    M
* a positive value.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:    The triangular two-manifold mesh data.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPlCrvtrSetCurvatureAttr						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPlSilImportanceAttr                                                    M
*****************************************************************************/
void GMPlSilImportanceAttr(IPPolygonStruct *PolyList)
{
    int i;
    IPPolygonStruct *Pl;
    IPObjectStruct
        *PObj = IPGenPOLYObject(PolyList);
    IPPolyVrtxIdxStruct
	*PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObj, TRUE, 0);
    IPVertexStruct
	**Vertices = PVIdx -> Vertices;
    IPPolyPtrStruct
	**PPolys = PVIdx -> PPolys;

    for (i = 0; i < PVIdx -> NumVrtcs; i++) {
        int j,
	    *Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, i, 1);
	IrtRType Wgt,
	    Weights = 0.0,
	    SilImp = 0.0;
        IPVertexStruct
	    *V1 = Vertices[i];
	IPPolyPtrStruct
	    *Pls = PPolys[i];

	for (j = 0; Nbrs[j] >= 0; j++) {
	    IPVertexStruct
		*V2 = Vertices[Nbrs[j]];

	    Wgt = IRIT_PT_PT_DIST(V1 -> Coord, V2 -> Coord);
	    Weights += Wgt;

	    SilImp += GMPlFindSilImp(V2, Pls) * Wgt;
	}

	SilImp /= Weights;
	AttrSetRealAttrib(&V1 -> Attr, "SilImp", SilImp);
    }

    /* Mark vertices that are on a topological range of the importance func. */
    for (i = 0; i < PVIdx -> NumVrtcs; i++) {
        IPVertexStruct
	    *V1 = Vertices[i];
        int j,
	    NumHigherNeighbors = 0,
	    *Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, i, 1);
	IrtRType
	    SilImp = AttrGetRealAttrib(V1 -> Attr, "SilImp");

	for (j = 0; Nbrs[j] >= 0; j++) {
	    IPVertexStruct
		*V2 = Vertices[Nbrs[j]];

	    if (AttrGetRealAttrib(V2 -> Attr, "SilImp") > SilImp)
		NumHigherNeighbors++;
	}

	if (NumHigherNeighbors < 3)
	    AttrSetRealAttrib(&V1 -> Attr, "SilImpRange", TRUE);
    }

    /* Propagate the importance properties to all the vertices in PolyList. */
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

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPlyimpMapToUVImpSpace, FALSE) {
	    for (Pl = PolyList; Pl != NULL; Pl = Pl -> Pnext) {
	        IPVertexStruct
		    *V = Pl -> PVertex;

		do {
		    float
		        *UV = AttrGetUVAttrib(V -> Attr, "uvvals");
		    IrtRType
		        SilImp = AttrGetRealAttrib(V -> Attr, "SilImp");

		    if (UV != NULL) {
		        V -> Coord[0] = UV[0];
			V -> Coord[1] = UV[1];
			V -> Coord[2] = SilImp;
		    }
		    else {
		        IRIT_WARNING_MSG(
			    "Vertex with no UV values detected and ignored\n");
		    }

		    V = V -> Pnext;
		}
		while (V != NULL && V != Pl -> PVertex);
	    }
	}
    }
#   endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the importance function along the shared edge of the two        *
* polygons that share vertex V.  Exactly two such polygons should be in Pls. *
*                                                                            *
* PARAMETERS:                                                                *
*   VOrig: Vertex to look for in the polygonal list.                         *
*   Pls:   List of polygons around another vertex that V is its neighbor.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  The importance as silhouette probability of the shared edge.  *
*****************************************************************************/
static IrtRType GMPlFindSilImp(IPVertexStruct *VOrig, IPPolyPtrStruct *Pls)
{
    int NumMatches = 0;
    IrtRType R,
	*N1 = NULL,
	*N2 = NULL,
	*OrigCoord = VOrig -> Coord;
    IPPolyPtrStruct *Pl;

    for (Pl = Pls; Pl != NULL && NumMatches < 2; Pl = Pl -> Pnext) {
        IPVertexStruct
	    *VHead = Pl -> Poly -> PVertex,
	    *V = VHead;

	do {
	    if (IRIT_PT_APX_EQ_EPS(V -> Coord, OrigCoord, IRIT_EPS)) {
		/* Found a match - keep a reference to it. */
	        if (NumMatches == 0)
		    N1 = Pl -> Poly -> Plane;
		else if (NumMatches == 1)
		    N2 = Pl -> Poly -> Plane;
		if (++NumMatches == 2)
		    break;
	    }

	    V = V -> Pnext;
	}
	while (V != VHead && V != NULL);
    }
    
    if (NumMatches < 2) {
        GEOM_FATAL_ERROR(GEOM_ERR_MISS_VRTX_IDX);
        return 0.0;
    }

    R = IRIT_DOT_PROD(N1, N2);
    return acos(IRIT_BOUND(R, -1.0, 1.0)) / M_PI;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extract the silhouette importance range of a polygonal mesh with	     M
* "SilImp" attribute.  See also GMPlSilImportanceAttr.			     M
*   Mesh is assumed to be a triangular regular mesh.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyList:    The triangular two-manifold mesh data.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  the extracted geometry.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPlSilImportanceAttr						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPlSilImportanceRange                                                   M
*****************************************************************************/
IPPolygonStruct *GMPlSilImportanceRange(IPPolygonStruct *PolyList)
{
    IPPolygonStruct *Pl,
	*PlRange = NULL;

    for (Pl = PolyList; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *V = Pl -> PVertex,
	    *VNext = V -> Pnext;

	do {
	    if (AttrGetRealAttrib(V -> Attr, "SilImpRange") == TRUE &&
		AttrGetRealAttrib(VNext -> Attr, "SilImpRange") == TRUE) {
		int i,
		    DMaxIdx = 0;
		IrtRType Dp[3];

		/* We have a range edge.  make sure we extract it only once */
		/* by extracting the edge of IRIT_MAX(Dx, Dy, Dz) is positive.   */
		IRIT_PT_SUB(Dp, V -> Coord, VNext -> Coord);
		for (i = 1; i < 3; i++) {
		    if (IRIT_FABS(Dp[i]) > IRIT_FABS(Dp[DMaxIdx]))
			DMaxIdx = i;
		}
		if (Dp[DMaxIdx] > 0.0) {
		    IPVertexStruct
			*V1 = IPCopyVertex(V),
			*V2 = IPCopyVertex(VNext);

		    V1 -> Pnext = V2;
		    PlRange = IPAllocPolygon(0, V1, PlRange);
		}
	    }

	    V = VNext;
	    VNext = V -> Pnext == NULL ? Pl -> PVertex : V -> Pnext;
	}
	while (V != Pl -> PVertex);
    }

    return GMMergePolylines(PlRange, IRIT_EPS);
}
