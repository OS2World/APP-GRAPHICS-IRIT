/******************************************************************************
* PlyStrct.c - constructs adjacency data structure of polygonal meshes.       *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 2003		                              *
******************************************************************************/

#include <math.h>
#include "geom_loc.h"

#define IHT_VERTEX_KEY(V)      (V -> Coord[0] * 0.301060 + \
				V -> Coord[1] * 0.280791 + \
				V -> Coord[2] * 0.190886)
#define IHT_EDGE_KEY(E)	       ((IHT_VERTEX_KEY(GlblVList[E.VIdx1].V) + \
				 IHT_VERTEX_KEY(GlblVList[E.VIdx2].V)) * 0.5)

typedef struct GMPolyAdjIdxStruct {
    struct GMPolyAdjIdxStruct *Pnext;
    int Idx;
} GMPolyAdjIdxStruct;

typedef struct GMPolyAdjVrtxStruct {
    IPVertexStruct *V;
    GMPolyAdjIdxStruct *EList;
} GMPolyAdjVrtxStruct;

typedef struct GMPolyAdjEdgeStruct {
    IPPolygonStruct *Pl1, *Pl2;
    int VIdx1, VIdx2;
} GMPolyAdjEdgeStruct;

typedef struct GMPolyAdjStruct {
    int NumVertices, NumEdges;
    IrtRType EqlEps;
    GMPolyAdjVrtxStruct *VList;
    GMPolyAdjEdgeStruct *EList;
    IPObjectStruct *PObj;
} GMPolyAdjStruct;

IRIT_STATIC_DATA IrtRType
    GlblEqlEps = IRIT_EPS;
IRIT_STATIC_DATA GMPolyAdjVrtxStruct
    *GlblVList;

static GMPolyAdjIdxStruct *GMNewPolyAdjIdx(GMPolyAdjIdxStruct *Pnext, int Idx);
static int CmpTwoVertices(VoidPtr VP1, VoidPtr VP2);
static int CmpTwoEdges(VoidPtr EP1, VoidPtr EP2);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates one PolyAdjIdx struct.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Pnext:   Pointer to next struct in linked list.                          *
*   Idx:     New index info.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   GMPolyAdjIdxStruct *:   New allocated struct.                            *
*****************************************************************************/
static GMPolyAdjIdxStruct *GMNewPolyAdjIdx(GMPolyAdjIdxStruct *Pnext, int Idx)
{
    GMPolyAdjIdxStruct
	*RetVal = (GMPolyAdjIdxStruct *) IritMalloc(sizeof(GMPolyAdjIdxStruct));

    RetVal -> Pnext = Pnext;
    RetVal -> Idx = Idx;

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare two edges if same or not.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   VP1, VP2:   Two vertices to compare.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      -1, 0, +1 if V1 less, equal, greater than V2.                  *
*****************************************************************************/
static int CmpTwoEdges(VoidPtr EP1, VoidPtr EP2)
{
    int i1,
	i2 = 0;
    GMPolyAdjEdgeStruct
        *E1 = (GMPolyAdjEdgeStruct *) EP1,
        *E2 = (GMPolyAdjEdgeStruct *) EP2;

    if (((i1 = CmpTwoVertices(GlblVList[E1 -> VIdx1].V,
			      GlblVList[E2 -> VIdx1].V)) == 0 &&
	 (i2 = CmpTwoVertices(GlblVList[E1 -> VIdx2].V,
			      GlblVList[E2 -> VIdx2].V)) == 0) ||
	(CmpTwoVertices(GlblVList[E1 -> VIdx1].V,
			GlblVList[E2 -> VIdx2].V) == 0 &&
	 CmpTwoVertices(GlblVList[E1 -> VIdx1].V,
			GlblVList[E2 -> VIdx2].V) == 0))
        return 0;

    return i1 == 0 ? i2 : i1;	
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compare two vertices if same or not.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   VP1, VP2:   Two vertices to compare.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      -1, 0, +1 if V1 less, equal, greater than V2.                  *
*****************************************************************************/
static int CmpTwoVertices(VoidPtr VP1, VoidPtr VP2)
{
    IPVertexStruct
	*V1 = (IPVertexStruct *) VP1,
	*V2 = (IPVertexStruct *) VP2;
    IrtRType
	*Coord1 = V1 -> Coord,
	*Coord2 = V2 -> Coord;

    if (IRIT_PT_APX_EQ_EPS(Coord1, Coord2, GlblEqlEps)) {
	int VIndex1 = AttrGetIntAttrib(V1 -> Attr, "_Vidx"),
	    VIndex2 = AttrGetIntAttrib(V2 -> Attr, "_Vidx");

	if (!IP_ATTR_IS_BAD_INT(VIndex1))
	    AttrSetIntAttrib(&V2 -> Attr, "_Vidx", VIndex1);
	else if (!IP_ATTR_IS_BAD_INT(VIndex2))
	    AttrSetIntAttrib(&V1 -> Attr, "_Vidx", VIndex2);
	else
	    GEOM_FATAL_ERROR(GEOM_ERR_VRTX_MTCH_FAILED);

	return 0;
    }
    else {
        int i;

        for (i = 0; i < 3; i++) {
	    if (Coord1[i] < Coord2[i])
	        return -1;
	    else if (Coord1[i] > Coord2[i])
	        return 1;
	}

	return 0; /* Should never get here. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs an adjacency information data structure for the given	     M
* polygonal mesh in GMPolyAdjStruct:                                         M
*   VList:  A list of vertices and for each all edges using the vertex.      M
*   EList:  A list of edges, each referencing the two vertices using it.     M
*   PObj:   A reference to the original model.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    A polygonal mesh to compute adjacency information for.          M
*   EqlEps:  Epsilon for point equality.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:   A reference to the data structure holding the adjacency info. M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyAdjacncyVertex, GMPolyAdjacncyFree                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyAdjacncyGen                                                        M
*****************************************************************************/
VoidPtr GMPolyAdjacncyGen(IPObjectStruct *PObj, IrtRType EqlEps)
{
    int MaxVrtcs, MaxEdges, VIndex, EIndex, OldCirc;
    IrtRType Min, Max;
    IPVertexStruct *V;
    IPPolygonStruct
	*Pl = PObj -> U.Pl;
    GMPolyAdjStruct *PAdj;
    GMPolyAdjEdgeStruct *EList;
    IritHashTableStruct *VIHT, *EIHT;
    GMBBBboxStruct *BBox;

    if (!IP_IS_POLY_OBJ(PObj) || !IP_IS_POLYGON_OBJ(PObj))
	return NULL;

    OldCirc = IPSetPolyListCirc(TRUE);
    GMVrtxListToCircOrLin(PObj -> U.Pl, TRUE);
    BoolGenAdjacencies(PObj);
    GMVrtxListToCircOrLin(PObj -> U.Pl, FALSE);
    IPSetPolyListCirc(OldCirc);

    /* Get bounds on the number of vertices/edges possible. */
    for (MaxVrtcs = 0, Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext)
	MaxVrtcs += IPVrtxListLen(Pl -> PVertex);
    if ((MaxEdges = MaxVrtcs) < 3)
	return NULL;

    /* Create a hash table to hold vertices and detect identities. */
    BBox = GMBBComputePolyListBbox(PObj -> U.Pl);
    Min = IRIT_MIN(IRIT_MIN(BBox -> Min[0], BBox -> Min[1]), BBox -> Min[2]);
    Max = IRIT_MAX(IRIT_MAX(BBox -> Max[0], BBox -> Max[1]), BBox -> Max[2]);
    VIHT = IritHashTableCreate(Min, Max, EqlEps, MaxVrtcs);
    EIHT = IritHashTableCreate(Min, Max, EqlEps, MaxEdges);

    PAdj = (GMPolyAdjStruct *) IritMalloc(sizeof(GMPolyAdjStruct));
    PAdj -> VList = GlblVList = (GMPolyAdjVrtxStruct *)
			   IritMalloc(sizeof(GMPolyAdjVrtxStruct) * MaxVrtcs);
    PAdj -> EList = EList = (GMPolyAdjEdgeStruct *)
			   IritMalloc(sizeof(GMPolyAdjEdgeStruct) * MaxEdges);
    PAdj -> EqlEps = GlblEqlEps = EqlEps;

    /* Insert the vertices into the vertex list. */
    for (Pl = PObj -> U.Pl, VIndex = 0; Pl != NULL; Pl = Pl -> Pnext) {
        V = Pl -> PVertex;
        do {
	    if (!IritHashTableInsert(VIHT, V, CmpTwoVertices,
				     IHT_VERTEX_KEY(V), FALSE)) {
	        /* It is new data to hash table - prepare new info for V. */
	        GlblVList[VIndex].V = V;
	        GlblVList[VIndex].EList = NULL;
		AttrSetIntAttrib(&V -> Attr, "_Vidx", VIndex++);
	    }

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    /* Insert the edges into the edge list. */
    for (Pl = PObj -> U.Pl, EIndex = 0; Pl != NULL; Pl = Pl -> Pnext) {
        int VIdx1, VIdx2;

	V = Pl -> PVertex;
	VIdx1 = AttrGetIntAttrib(V -> Attr, "_Vidx");

        do {
	    IPVertexStruct
		*Vnext = V -> Pnext ? V -> Pnext : Pl -> PVertex;
	    VIdx2 = AttrGetIntAttrib(Vnext -> Attr, "_Vidx");

	    if (IP_ATTR_IS_BAD_INT(VIdx1) || IP_ATTR_IS_BAD_INT(VIdx2))
	        GEOM_FATAL_ERROR(GEOM_ERR_VRTX_MTCH_FAILED);

	    if (CmpTwoVertices(V, Vnext) < 0) {
		EList[EIndex].VIdx1 = VIdx2;
		EList[EIndex].VIdx2 = VIdx1;
	    }
	    else {
		EList[EIndex].VIdx1 = VIdx1;
		EList[EIndex].VIdx2 = VIdx2;
	    }

	    if (!IritHashTableInsert(EIHT, &EList[EIndex], CmpTwoEdges,
				     IHT_EDGE_KEY(EList[EIndex]), FALSE)) {
	        /* Insert this edge into the edge lists of its two vertices. */
	        EList[EIndex].Pl1 = Pl;
	        EList[EIndex].Pl2 = V -> PAdj;
		GlblVList[VIdx1].EList = 
			    GMNewPolyAdjIdx(GlblVList[VIdx1].EList, EIndex);
		GlblVList[VIdx2].EList =
			    GMNewPolyAdjIdx(GlblVList[VIdx2].EList, EIndex);
		EIndex++;	        
	    }

	    VIdx1 = VIdx2;
	    V = Vnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    PAdj -> NumVertices = VIndex;
    PAdj -> NumEdges = EIndex;

    IritHashTableFree(VIHT);
    IritHashTableFree(EIHT);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpPolyObj, FALSE) {
	    int i;
	    GMPolyAdjIdxStruct *E;

	    IRIT_INFO_MSG("VERTICES:\n");
	    for (i = 0; i < PAdj -> NumVertices; i++) {
	        IRIT_INFO_MSG_PRINTF("%4d) %10.7lg %10.7lg %10.7lg :: Edges ",
				     i,
				     GlblVList[i].V -> Coord[0],
				     GlblVList[i].V -> Coord[1],
				     GlblVList[i].V -> Coord[2]);
		for (E = GlblVList[i].EList; E != NULL; E = E -> Pnext)
		    IRIT_INFO_MSG_PRINTF(" %4d", E -> Idx);
		IRIT_INFO_MSG("\n");
	    }

	    IRIT_INFO_MSG("EDGES:\n");
	    for (i = 0; i < PAdj -> NumEdges; i++) {
	        IRIT_INFO_MSG_PRINTF("%4d) %4d %4d\n",
					i, EList[i].VIdx1, EList[i].VIdx2);
	    }
	}
    }
#   endif /* DEBUG */

    return PAdj;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the adjacency information of a vertex - all the edges that share it. M
*   Invokes the given call back function on all edges.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   V:   Vertex to find all edges that share it.                             M
*   PolyAdj:   Data struct to use, as constructed by GMPolyAdjacncyGen.      M
*   AdjVertexFunc:  Call be function to invoke on every edge.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyAdjacncyGen, GMPolyAdjacncyFree                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyAdjacncyVertex                                                     M
*****************************************************************************/
void GMPolyAdjacncyVertex(IPVertexStruct *V,
			  VoidPtr PolyAdj,
			  GMPolyAdjacncyVertexFuncType AdjVertexFunc)
{
    int VIdx = AttrGetIntAttrib(V -> Attr, "_Vidx");
    GMPolyAdjStruct
        *PAdj = (GMPolyAdjStruct *) PolyAdj;
    GMPolyAdjIdxStruct *E;
    GMPolyAdjVrtxStruct
	*VList = PAdj -> VList;
    GMPolyAdjEdgeStruct
	*EList = PAdj -> EList;
#   ifdef DEBUG
    IrtRType
        *Coord = V -> Coord;
#   endif /* DEBUG */
 
    if (IP_ATTR_IS_BAD_INT(VIdx))
	GEOM_FATAL_ERROR(GEOM_ERR_VRTX_MTCH_FAILED);

    for (E = VList[VIdx].EList; E != NULL; E = E -> Pnext) {
        GMPolyAdjEdgeStruct
	    *Edge = &EList[E -> Idx];
        IPVertexStruct
	    *V1 = VList[Edge -> VIdx1].V,
	    *V2 = VList[Edge -> VIdx2].V;

#       ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpPolyObj, FALSE) {
	        if (!IRIT_PT_APX_EQ_EPS(V1 -> Coord, Coord, PAdj -> EqlEps) &&
		    !IRIT_PT_APX_EQ_EPS(V2 -> Coord, Coord, PAdj -> EqlEps))
		    GEOM_FATAL_ERROR(GEOM_ERR_VRTX_MTCH_FAILED);
	    }
	}
#	endif /* DEBUG */

	AdjVertexFunc(V1, V2, Edge -> Pl1, Edge -> Pl2);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the adjacency data structure associated with a polygonal model.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyAdj:   Data struct to free, as constructed by GMPolyAdjacncyGen.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyAdjacncyGen, GMPolyAdjacncyVertex                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyAdjacncyFree                                                       M
*****************************************************************************/
void GMPolyAdjacncyFree(VoidPtr PolyAdj)
{
    int i;
    GMPolyAdjStruct
        *PAdj = (GMPolyAdjStruct *) PolyAdj;
    GMPolyAdjIdxStruct *E;
    GMPolyAdjEdgeStruct
        *EList = PAdj -> EList;
    GMPolyAdjVrtxStruct
        *VList = PAdj -> VList;

    for (i = 0; i < PAdj -> NumVertices; i++) {
	while (VList[i].EList != NULL) {
	    E = VList[i].EList -> Pnext;
	    IritFree(VList[i].EList);
	    VList[i].EList = E;
	}
    }
    IritFree(VList);
    IritFree(EList);
    IritFree(PAdj);
}
