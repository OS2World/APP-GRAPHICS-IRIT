/******************************************************************************
* MrchCube.c - An implementation of the marching cube algorithm.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*						Gershon Elber, Dec 1992.      *
******************************************************************************/

#include <stdio.h>
#include "irit_sm.h"
#include "triv_loc.h"
#include "mrchcube.h"

#define MARCH_CUBE_EPS	1e-8

#define MC_COMP_V_POS(X, Y, Z, Pos) { \
			CCS -> _VrtxPos[Pos][0] = CCS -> Vrtx0Lctn[0] + X; \
			CCS -> _VrtxPos[Pos][1] = CCS -> Vrtx0Lctn[1] + Y; \
			CCS -> _VrtxPos[Pos][2] = CCS -> Vrtx0Lctn[2] + Z; }

typedef struct MCEdgeStruct {
    struct MCEdgeStruct *Pnext;			        /* To next in chain. */
    _MCInterStruct *V[2];
} MCEdgeStruct;

IRIT_STATIC_DATA CagdBType
    GlblHasGradient = TRUE;
IRIT_STATIC_DATA MCEdgeStruct
    *GlblEdgePool = NULL;

static MCEdgeStruct *MCEdgeMalloc(void);
static void MCEdgeFree(MCEdgeStruct *p);
static MCPolygonStruct *MCMergeEdgesToPolys(MCEdgeStruct *AllEList);
static int MCComputeEdgeInter(MCCubeCornerScalarStruct *CCS,
			      IrtRType Threshold,
			      int VrtxIndex1,
			      int VrtxIndex2,
			      IrtRType *Pos,
			      IrtRType *Nrml);
static void MCAppendEList(MCEdgeStruct *NewEdges, MCEdgeStruct **EdgeList);
static MCEdgeStruct *MCGetEdgesFromFace(MCCubeCornerScalarStruct *CCS,
					int InterEdgeIndex1,
					int InterEdgeIndex2,
					int InterEdgeIndex3,
					int InterEdgeIndex4);
static void MCPurgeZeroLenEdges(MCEdgeStruct **AllEList);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given 8 cube corner values (scalars), compute the polygon(s) in this cube  M
* along the isosurface at Threshold. if CCS has gradient information, it is  M
* used to approximate normals at the vertices.				     M
*                                                                            M
*                                                                            M
*                                                                            V
*                     7            K           6                             V
*                      ***********************                               V
*                    * +                   * *                               V
*                L *   +                 *   *              Vertices 0 - 7   V
*                *     +     I         * J   *              Edges    A - L   V
*            4 *********************** 5     *                               V
*              *       +             *       *                               V
*              *       +             *       * G                             V
*              *       + H           *       *                               V
*              *       +             *       *                               V
*              *       +             * F     *                               V
*            E *       +       C     *       *                               V
*              *       ++++++++++++++*+++++++* 2                             V
*              *   D + 3             *     *                                 V
*              *   +                 *   * B                                 V
*              * +                   * *                                     V
*              ***********************                                       V
*             0           A           1                                      V
*                                                                            *
* PARAMETERS:                                                                M
*   CCS:          The cube's dimensions/information.                         M
*   Threshold:    Iso surface level.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MCPolygonStruct *:   List of polygons (not necessarily triangles), or    M
*                        possibly NULL.                                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   MCExtractIsoSurface, MCInitializeCube                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MCThresholdCube, marching cubes                                          M
*****************************************************************************/
MCPolygonStruct *MCThresholdCube(MCCubeCornerScalarStruct *CCS,
				 IrtRType Threshold)
{
    _MCInterStruct *Inter;
    MCEdgeStruct
	*AllEList = NULL;
    IrtRType
	Dim0 = CCS -> CubeDim[0],
	Dim1 = CCS -> CubeDim[1],
	Dim2 = CCS -> CubeDim[2];

    GlblHasGradient = CCS -> HasGradient;

    /* Computes the position of the 8 _Vertices. */
    MC_COMP_V_POS(0.0,  0.0,  0.0,  MC_VRTX_0);
    MC_COMP_V_POS(Dim0, 0.0,  0.0,  MC_VRTX_1);
    MC_COMP_V_POS(Dim0, Dim1, 0.0,  MC_VRTX_2);
    MC_COMP_V_POS(0.0,  Dim1, 0.0,  MC_VRTX_3);

    MC_COMP_V_POS(0.0,  0.0,  Dim2, MC_VRTX_4);
    MC_COMP_V_POS(Dim0, 0.0,  Dim2, MC_VRTX_5);
    MC_COMP_V_POS(Dim0, Dim1, Dim2, MC_VRTX_6);
    MC_COMP_V_POS(0.0,  Dim1, Dim2, MC_VRTX_7);

    /* Test and compute any edge that intersect the Threshold value. */
    CCS -> _Intersect = FALSE;

    Inter = &CCS -> _Inter[MC_EDGE_A];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_0, MC_VRTX_1,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_B];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_1, MC_VRTX_2,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_C];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_2, MC_VRTX_3,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_D];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_3, MC_VRTX_0,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_E];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_0, MC_VRTX_4,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_F];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_1, MC_VRTX_5,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_G];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_2, MC_VRTX_6,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_H];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_3, MC_VRTX_7,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_I];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_4, MC_VRTX_5,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_J];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_5, MC_VRTX_6,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_K];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_6, MC_VRTX_7,
					 Inter -> _Pos, Inter -> _Nrml);
    Inter = &CCS -> _Inter[MC_EDGE_L];
    Inter -> _HighV = MCComputeEdgeInter(CCS, Threshold, MC_VRTX_4, MC_VRTX_7,
					 Inter -> _Pos, Inter -> _Nrml);
    if (!CCS->_Intersect)
	return NULL;

    /* For each of the six faces, compute polygon interior edges for that   */
    /* face and save all of them into a global list. 			    */
    MCAppendEList(MCGetEdgesFromFace(CCS, MC_EDGE_D, MC_EDGE_C,
				          MC_EDGE_B, MC_EDGE_A), &AllEList);
    MCAppendEList(MCGetEdgesFromFace(CCS, MC_EDGE_A, MC_EDGE_F,
				          MC_EDGE_I, MC_EDGE_E), &AllEList);
    MCAppendEList(MCGetEdgesFromFace(CCS, MC_EDGE_B, MC_EDGE_G,
				          MC_EDGE_J, MC_EDGE_F), &AllEList);
    MCAppendEList(MCGetEdgesFromFace(CCS, MC_EDGE_C, MC_EDGE_H,
				          MC_EDGE_K, MC_EDGE_G), &AllEList);
    MCAppendEList(MCGetEdgesFromFace(CCS, MC_EDGE_D, MC_EDGE_E,
				          MC_EDGE_L, MC_EDGE_H), &AllEList);
    MCAppendEList(MCGetEdgesFromFace(CCS, MC_EDGE_I, MC_EDGE_J,
				          MC_EDGE_K, MC_EDGE_L), &AllEList);

    MCPurgeZeroLenEdges(&AllEList);

    return MCMergeEdgesToPolys(AllEList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocated an edge structure.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MCEdgeStruct *:   Allocated edge                                         *
*****************************************************************************/
static MCEdgeStruct *MCEdgeMalloc(void)
{
    if (GlblEdgePool != NULL) {
	MCEdgeStruct
	    *RetVal = GlblEdgePool;

	GlblEdgePool = GlblEdgePool -> Pnext;

	return RetVal;
    }
    else
	return (MCEdgeStruct *) IritMalloc(sizeof(MCEdgeStruct));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free an edge structure.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   p:    Edge to free.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MCEdgeFree(MCEdgeStruct *p)
{
    IRIT_LIST_PUSH(p, GlblEdgePool);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges all edges of this cube into polygons.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   AllEList:     Edges extracted from this cube, freed once done.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MCPolygonStruct *:   Polygons formed from this edges set.                *
*****************************************************************************/
static MCPolygonStruct *MCMergeEdgesToPolys(MCEdgeStruct *AllEList)
{
    MCPolygonStruct
	*AllPList = NULL;

    /* Merge the Vertices into polygons. */
    while (AllEList != NULL) {
	int NumOfVertices = 2;
	CagdBType
	    ClosedPoly = FALSE;
	MCPolygonStruct
	    *P = (MCPolygonStruct *) IritMalloc(sizeof(MCPolygonStruct));
	MCEdgeStruct *E2, *E2Prev,
	    *E = AllEList;
	_MCInterStruct
	    **V = E -> V;
	CagdPType
	    *PV = P -> V,
	    *PN = P -> N;

	IRIT_PT_COPY(PV[0], V[0] -> _Pos);
	IRIT_PT_COPY(PV[1], V[1] -> _Pos);
	if (GlblHasGradient) {
	    IRIT_PT_COPY(PN[0], V[0] -> _Nrml);
	    IRIT_PT_COPY(PN[1], V[1] -> _Nrml);
	}

	AllEList = AllEList -> Pnext;
	MCEdgeFree(E);

	while (!ClosedPoly) {
	    if (AllEList == NULL)
		return NULL;

	    for (E2 = E2Prev = AllEList; E2 != NULL; ) {
		CagdBType
		    FoundMatch = FALSE;

		V = E2 -> V;

		if (IRIT_PT_APX_EQ_EPS(PV[NumOfVertices - 1],
				  V[1] -> _Pos, MARCH_CUBE_EPS)) {
		    IRIT_PT_COPY(PV[NumOfVertices], V[0] -> _Pos);
		    if (GlblHasGradient)
			IRIT_PT_COPY(PN[NumOfVertices], V[0] -> _Nrml);
		    NumOfVertices++;
		    FoundMatch = TRUE;
		}
		else if (IRIT_PT_APX_EQ_EPS(PV[NumOfVertices - 1],
				       V[0] -> _Pos, MARCH_CUBE_EPS)) {
		    IRIT_PT_COPY(PV[NumOfVertices], V[1] -> _Pos);
		    if (GlblHasGradient)
			IRIT_PT_COPY(PN[NumOfVertices], V[1] -> _Nrml);
		    NumOfVertices++;
		    FoundMatch = TRUE;
		}

		if (FoundMatch) {
		    if (E2 == AllEList) {
			AllEList = AllEList -> Pnext;
		    }
		    else {
			E2Prev -> Pnext = E2 -> Pnext;
		    }
		    MCEdgeFree(E2);
		    E2 = E2Prev = AllEList;
		}
		else {
		    E2Prev = E2;
		    E2 = E2 -> Pnext;
		}

		if (IRIT_PT_APX_EQ_EPS(PV[0], PV[NumOfVertices - 1],
				  MARCH_CUBE_EPS)) {
		    P -> NumOfVertices = NumOfVertices;
		    P -> Pnext = AllPList;
		    AllPList = P;
		    ClosedPoly = TRUE;
		    break;
		}
	    }
	}
    }

    return AllPList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Drop zero length edges.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   AllEList:  To clean up.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MCPurgeZeroLenEdges(MCEdgeStruct **AllEList)
{
    MCEdgeStruct
	*E = *AllEList,
	*PrevE = E;

    while (E != NULL) {
	IrtPtType Pt;

	IRIT_PT_SUB(Pt, E -> V[0] -> _Pos, E -> V[1] -> _Pos);

	if (IRIT_PT_LENGTH(Pt) < MARCH_CUBE_EPS) {
	    if (E == *AllEList) {
		*AllEList = E -> Pnext;
		MCEdgeFree(E);
		E = PrevE = *AllEList;
	    }
	    else {
		PrevE -> Pnext = E -> Pnext;
		MCEdgeFree(E);
		E = PrevE -> Pnext;
	    }
	}
	else {
	    PrevE = E;
	    E = E -> Pnext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the intersection of a cube edge with the scalar threshold level,  *
* given the edge two indices.						     *
*   Sets CCS _Intersect if found intersection.				     *
*   Updates the Pos parameter to the intersection position or set it to      *
* IRIT_INFNTY if no Intersection is found.				     *
*   Returns the Index of the vertex with value ABOVE the Threshold level or  *
* MC_VRTX_NONE of no intersection.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   CCS:           Cube to consider.					     *
*   Threshold:     Iso surface level.					     *
*   VrtxIndex1, VrtxIndex2:    Two vertices of intersecting edge.            *
*   Pos:           Where position is to be saved.                            *
*   Nrml:          Where Normal at position is to be saved.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:           Index of edge's vertex above Threshold, or MC_VRTX_NONE   *
*                  if no intersection.                                       *
*****************************************************************************/
static int MCComputeEdgeInter(MCCubeCornerScalarStruct *CCS,
			      IrtRType Threshold,
			      int VrtxIndex1,
			      int VrtxIndex2,
			      IrtRType *Pos,
			      IrtRType *Nrml)
{
    int i, MaxIndex;
    IrtRType t, t1, *Pt1, *Pt2,
	Val1 = CCS -> Corners[VrtxIndex1],
	Val2 = CCS -> Corners[VrtxIndex2];

    if (Val1 > Val2) {
	if (Val2 >= Threshold || Val1 < Threshold)
	    return MC_VRTX_NONE;

	MaxIndex = VrtxIndex1;
    }
    else {
	if (Val1 >= Threshold || Val2 < Threshold)
	    return MC_VRTX_NONE;

	MaxIndex = VrtxIndex2;
    }

    t = (Val1 - Threshold) / (Val1 - Val2);
    t1 = 1.0 - t;

    Pt1 = CCS -> _VrtxPos[VrtxIndex2];
    Pt2 = CCS -> _VrtxPos[VrtxIndex1];
    for (i = 0; i < 3; i++)
	*Pos++ = *Pt1++ * t + *Pt2++ * t1;

    if (GlblHasGradient) {
	Nrml[0] = CCS -> GradientX[VrtxIndex2] * t +
		  CCS -> GradientX[VrtxIndex1] * t1;
	Nrml[1] = CCS -> GradientY[VrtxIndex2] * t +
		  CCS -> GradientY[VrtxIndex1] * t1;
	Nrml[2] = CCS -> GradientZ[VrtxIndex2] * t +
		  CCS -> GradientZ[VrtxIndex1] * t1;
	IRIT_PT_NORMALIZE(Nrml);
    }

    CCS -> _Intersect = TRUE;

    return MaxIndex;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Appends NewEdges to EdgeList						     *
*                                                                            *
* PARAMETERS:                                                                *
*   NewEdges:   Edges to append to EdgeList.                                 *
*   EdgeList:   Exists edge list to append NewEdges to.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MCAppendEList(MCEdgeStruct *NewEdges, MCEdgeStruct **EdgeList)
{
    MCEdgeStruct
	*E = NewEdges;

    if (E != NULL) {
	while (E -> Pnext != NULL)
	    E = E -> Pnext;
	E -> Pnext = *EdgeList;
	*EdgeList = NewEdges;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Extract intersection edges from a given face (if any).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   CCS:                     One voxel to consider.                          *
*   InterEdgeIndex1/2/3/4:   Four indices of face.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   MCEdgeStruct *:          List of edges of iso surface intersecting face. *
*****************************************************************************/
static MCEdgeStruct *MCGetEdgesFromFace(MCCubeCornerScalarStruct *CCS,
					int InterEdgeIndex1,
					int InterEdgeIndex2,
					int InterEdgeIndex3,
					int InterEdgeIndex4)
{
    CagdBType
	Inter1 = CCS -> _Inter[InterEdgeIndex1]._HighV != MC_VRTX_NONE,
	Inter2 = CCS -> _Inter[InterEdgeIndex2]._HighV != MC_VRTX_NONE,
	Inter3 = CCS -> _Inter[InterEdgeIndex3]._HighV != MC_VRTX_NONE,
	Inter4 = CCS -> _Inter[InterEdgeIndex4]._HighV != MC_VRTX_NONE;
    int i, j,
	NumOfInters = Inter1 + Inter2 + Inter3 + Inter4;

    /* We can have either two or four intersections. */
    if (NumOfInters == 0) {
	return NULL;
    }
    else if (NumOfInters == 2) {
	int NumOfVertices = 0;
	struct MCEdgeStruct
	    *Edge = MCEdgeMalloc();

	/* Make an edge from one intersection to the next. */
	if (Inter1)
	    Edge -> V[NumOfVertices++] = &CCS -> _Inter[InterEdgeIndex1];
	if (Inter2)
	    Edge -> V[NumOfVertices++] = &CCS -> _Inter[InterEdgeIndex2];
	if (Inter3)
	    Edge -> V[NumOfVertices++] = &CCS -> _Inter[InterEdgeIndex3];
	if (Inter4)
	    Edge -> V[NumOfVertices++] = &CCS -> _Inter[InterEdgeIndex4];

	if (NumOfVertices != 2) {
	    TRIV_FATAL_ERROR(TRIV_ERR_TWO_INTERSECTIONS);
	    return NULL;
	}

	Edge -> Pnext = NULL;

	return Edge;
    }
    else if (NumOfInters == 4) {
	int Indirect[4];
	CagdBType Found;
	struct MCEdgeStruct
	    *Edge1 = MCEdgeMalloc(),
	    *Edge2 = MCEdgeMalloc();

	Indirect[0] = InterEdgeIndex1;
	Indirect[1] = InterEdgeIndex2;
	Indirect[2] = InterEdgeIndex3;
	Indirect[3] = InterEdgeIndex4;

	/* Use the _HighV To make a decision. */

	/* Find first pair. */
	for (i = 0, Found = FALSE; i < 3 && !Found; i++) {
	    for (j = i + 1; j < 4 && !Found; j++) {
		if (CCS -> _Inter[Indirect[i]]._HighV ==
		    CCS -> _Inter[Indirect[j]]._HighV) 
		    Found = TRUE;
	    }
	}
	if (Found) {
	    i--;
	    j--;
	}
	else {
	    TRIV_FATAL_ERROR(TRIV_ERR_NO_MATCH_PAIR);
	    return NULL;
	}

	Edge1 -> V[0] = &CCS -> _Inter[Indirect[i]];
	Edge1 -> V[1] = &CCS -> _Inter[Indirect[j]];
	Indirect[i] = MC_EDGE_NONE;
	Indirect[j] = MC_EDGE_NONE;

	/* Find second pair. */
	for (i = j = 0; i < 4; i++) {
	    if (Indirect[i] != MC_EDGE_NONE)
		Edge2 -> V[j++] = &CCS -> _Inter[Indirect[i]];
	}

	Edge1 -> Pnext = Edge2;
	Edge2 -> Pnext = NULL;

	return Edge1;
    }
    else {
	TRIV_FATAL_ERROR(TRIV_ERR_2_OR_4_INTERS);
	return NULL;
    }
}
