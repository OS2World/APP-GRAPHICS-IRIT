/******************************************************************************
* geomsmth.c - functions to smooth poly data.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Daniel Ghosalker and Gershon Elber, July 2011.		      *
******************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_loc.h"

static int GMIsPointPolyInterPoly(const IPVertexStruct *VS, 
				  IrtPtType Point, 
				  const char *Name, 
				  int Val);
static int GMIsInterLineLineXY(const IrtPtType A1, 
			       const IrtPtType A2, 
			       const IrtPtType B1, 
			       const IrtPtType B2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Move all Vertices that are not on the boundary of the polygons to new   M
* (averages of their 1-rings) positions, smoothing the shape of the mesh.    M
*                                                                            *
* PARAMETERS:                                                                M
*   PolyObj:   Polygonal object to smooth, in place.			     M
*   NumTimes:  Number of times to perform this smoothing algoritham.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *: The smoothed out polygons, in place. Same as PolyObj.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMPolyMeshSmoothing							     M
*****************************************************************************/
IPObjectStruct *GMPolyMeshSmoothing(IPObjectStruct *PolyObj, int NumTimes)
{
    IPPolyVrtxIdxStruct
	*PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PolyObj, TRUE, 0); 
    IPVertexStruct
	**Vertices = PVIdx -> Vertices; 
    int i, j, 
	**VPolys = PVIdx -> Polygons; 
    IrtPtType *Pt, 
	*NewPoints = (IrtPtType *) IritMalloc(sizeof(IrtPtType) *
					                   PVIdx -> NumVrtcs); 
    const IPPolygonStruct *Pl; 

    for (j = 0; j < NumTimes; j++) {
        Pt = NewPoints;

	for (i = 0; i < PVIdx -> NumVrtcs; i++, Pt++) {
	    IPVertexStruct *NP, 
		*V1 = Vertices[i]; 

	    if ((NP = GMGet1RingPoly2VrtxIdx(i, PVIdx)) == NULL) {
	        /* This vertex is a boundary vertex. */
	        assert(GMIsVertexBoundary(i, PVIdx));
		IRIT_PT_COPY(Pt, V1 -> Coord); 
		continue; 
	    }
	    GMComputeAverageVertex(NP, *Pt, "_GMNeighborVertex", 1); 

	    /*	 A case where there is an intersection cut between the    */
	    /* new middle point connected to the neighboring vertices.    */
	    if (NP != NULL && 
		GMIsPointPolyInterPoly(NP -> Pnext, *Pt, 
				       "_GMNeighborVertex", 1)) {
		IrtPtType TmpPt;

		if (GMFindPtInsidePolyKernel(NP, TmpPt))
		    IRIT_PT_COPY(*Pt, TmpPt);

	    }
	    if (NP != NULL)
		IPFreeVertexList(NP); 
	}

	/* Copy back all smoothed data back into original poly mesh. */
	for (Pl = PolyObj -> U.Pl, i = 0; 
	     Pl != NULL; 
	     Pl = Pl -> Pnext, i++) {
	    IPVertexStruct *V, 
		*V1 = Pl -> PVertex; 
	    int j = 0; 

	    V = V1; 
	    do {
		IRIT_PT_COPY(V -> Coord, NewPoints[VPolys[i][j]]); 
		j++; 
		V = V -> Pnext; 
	    }
	    while (V != V1 && V != NULL); 
	}
    }

    IritFree(NewPoints); 
    IPPolyVrtxIdxFree(PVIdx); 

    return PolyObj; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Finds the normal of polygon that can be none-convex.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   VL:    List of vertices of the Polygon.				     M
*   Nrml:  Computed normal of the polygon. Not normalized.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFindUnConvexPolygonNormal						     M
*****************************************************************************/
void GMFindUnConvexPolygonNormal(const IPVertexStruct *VL, IrtVecType Nrml)
{
    const IPVertexStruct
        *V = VL; 

    IRIT_PT_RESET(Nrml); 

    do {
        IrtPtType Tmp, C1, C2; 

	IRIT_PT_SUB(C1, V -> Coord, VL -> Coord); 
	IRIT_PT_SUB(C2, V -> Pnext -> Coord, VL -> Coord); 
	IRIT_CROSS_PROD(Tmp, C1, C2); 
	IRIT_PT_ADD(Nrml, Nrml, Tmp); 
	V = V -> Pnext; 
    }
    while (V != VL && V != NULL);  
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a point inside the kernel of the given polygon, if has any.     M
*                                                                            *
* PARAMETERS:                                                                M
*   VE:      Cyclic list of vertices of the Polygon.			     M
*   KrnlPt:  Computed interior kernel point.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if kernel is not empty, FALSE otherwise. 		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFindPtInsidePolyKernel						     M
*****************************************************************************/
int GMFindPtInsidePolyKernel(const IPVertexStruct *VE, IrtPtType KrnlPt)
{
    IrtRType 
	TotalWeight = 0.0; 
    const IPVertexStruct
        *V = VE; 
    IrtVecType Nrml; 

    GMFindUnConvexPolygonNormal(VE, Nrml);

    IRIT_PT_RESET(KrnlPt); 

    do {
	IrtPtType Vec1;

	IRIT_PT_SUB(Vec1, V -> Pnext -> Coord, V -> Coord); 
	if (!IRIT_PT_APX_EQ_ZERO_EPS(Vec1, IRIT_UEPS)) {
	    const IPVertexStruct
	        *V2 = V -> Pnext; 
	    IrtRType
	        MinT = -IRIT_INFNTY, 
	        MaxT = IRIT_INFNTY; 

	    /* Clip edge (V, V -> Pnext) againt all other edges of VE. */
	    do {
		IrtPtType Vec2, Tmp1, Tmp2; 
		IrtRType T1, T2; 

		IRIT_PT_SUB(Vec2, V2 -> Pnext -> Coord, V2 -> Coord); 
		if (!IRIT_PT_APX_EQ_ZERO_EPS(Vec2, IRIT_UEPS)) {
		    if (GM2PointsFromLineLine(V -> Coord, Vec1, 
					      V2 -> Coord, Vec2, 
					      Tmp1, &T1, Tmp2, &T2)) {
 		        IrtRType DP;
			IrtPtType Cross;

			/* update the end of the edge of the Kernel. */
			GMVecCrossProd(Cross, Vec1, Vec2); 
			if ((DP = GMVecDotProd(Cross, Nrml)) > -IRIT_UEPS)
			    MaxT = IRIT_MIN(MaxT, T1); 
			else if (DP < IRIT_UEPS)
			    MinT = IRIT_MAX(MinT, T1); 
		    }
		    else if (!GMCollinear3Pts(V -> Coord, 
					      V2 -> Coord, 
					      V2 -> Pnext -> Coord)) {
		        IrtPtType Cross, Vec3; 

			IRIT_PT_SUB(Vec3, V2 -> Coord, V -> Coord); 
			GMVecCrossProd(Cross, Vec1, Vec3);

			/* This edge not in the Kernel */
			if (GMVecDotProd(Cross, Nrml) < IRIT_UEPS)
			    MaxT = MinT - 1.0; 
		    }
		    else if (V > V2)
			/* This edge appears twice so take it off. */
			MaxT = MinT - 1.0; 
		}
		V2 = V2 -> Pnext; 
	    }
	    while (V2 != V && MaxT - MinT > IRIT_UEPS);

	    /* if the edge not empty accumulate its middle point, weighted  */
	    /* according to the length of the edge.			    */
	    if (MaxT - MinT > IRIT_UEPS) {
		IrtRType
		    Len = IRIT_PT_LENGTH(Vec1),
		    Weight = (MaxT - MinT) * Len;
		IrtPtType MidPt;

		TotalWeight += Weight;

		/* Compute middle point on kernel edge. */
	        IRIT_PT_SCALE(Vec1, (MinT + MaxT) * 0.5); 
		IRIT_PT_ADD(MidPt, Vec1, V -> Coord); 

		/* Accumulate the middle point, weighted. */
		IRIT_PT_SCALE(MidPt, Weight); 
	        IRIT_PT_ADD(KrnlPt, KrnlPt, MidPt); 
	    }
	}
	V = V -> Pnext;  
    }
    while (V != VE); 

    if (TotalWeight == 0.0)			       /* Kernel is empty. */
        return FALSE;

    IRIT_PT_SCALE(KrnlPt, 1.0 / TotalWeight); 

    return TRUE; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check if vertex with index Index is a boundary vertex in a mesh PVIdx.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Index:   The index of vertex we searching if it is a boundary vertex.    M
*   PVIdx:   The mesh in structure of the vertices indexed.	 	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if vertex V is a boundary vertex, FALSE otherwise.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsVertexBoundary							     M
*****************************************************************************/
int GMIsVertexBoundary(int Index, const IPPolyVrtxIdxStruct *PVIdx)
{
    const IPPolyPtrStruct
        *PPoly = PVIdx -> PPolys[Index];
    int i, k, 
        CountXor = 0;
    int * const
        *Polygons = PVIdx -> Polygons;

    for (k = 0; PPoly != NULL; PPoly = PPoly -> Pnext, k += 2) {
	int j, Num; 
	
	i = AttrGetIntAttrib(PPoly -> Poly -> Attr, "_PIdx");
 
	/* Count number of vertices in polygon. */
	for (Num = 0; Polygons[i][Num] != -1; Num++); 

	/* Count indices of prev and next vertices to vertex Index in poly. */
	for (j = 0; Polygons[i][j] != -1; j++) {
	    if (Polygons[i][j] == Index) {
		int PreVertex = Polygons[i][(j - 1 + Num) % Num],
		    PostVertex = Polygons[i][(j + 1) % Num];

		CountXor ^= PreVertex ^ PostVertex;
	    }
	}
    }

    /* If non-zero, no all vertices around vertex index Index were visited */
    /* twice.								   */
    return CountXor != 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check if lines from point Point to the vertices in the List of vertices, M
* VS, that also hold attribute Name with value Val interest with poly VS.    M
*                                                                            *
* PARAMETERS:                                                                M
*   VS:    List of planar vertices to examine.				     M
*   Point: The XY planar point to examine.				     M
*   Name:  The Name of the attribute to verify in each vertex or NULL to     M
*          consider all vertices.					     M
*   Val:   The expected Name attribute value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if a line from Point to some in VS intersect VS elsewhere,   M
*          FALSE otherwise.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsPointPolyInterPoly						     M
*****************************************************************************/
static int GMIsPointPolyInterPoly(const IPVertexStruct *VS, 
				  IrtPtType Point, 
				  const char *Name, 
				  int Val)
{
    const IPVertexStruct
        *V = VS; 

    do {
        if ((Name == NULL || AttrGetIntAttrib(V -> Attr, Name) == Val) &&
	    GMIsInterLinePolygon(VS, Point, V -> Coord))
	    return TRUE; 

	V = V -> Pnext; 
    }
    while (V != VS && V != NULL); 

    return FALSE; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check if a line V1V2 and polygon VS interest.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   VS:       Cyclic List of vertices of the Polygon.			     M
*   V1, V2:   The end points of the line.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if line and polygon do interest, FALSE otherwise.	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsInterLinePolygon						     M
*****************************************************************************/
int GMIsInterLinePolygon(const IPVertexStruct *VS, 
			 const IrtPtType V1, 
			 const IrtPtType V2)
{
    const IPVertexStruct 
	*V = VS; 
 
    do {
	if (GMIsInterLineLineXY(V -> Coord, V -> Pnext -> Coord, V1, V2))
	    return TRUE; 
	else {
	    IrtPtType Vec1, Vec2;

	    IRIT_PT_SUB(Vec1, V -> Pnext -> Coord, V1); 
	    IRIT_PT_SUB(Vec2, V2, V -> Pnext -> Coord);
	    if (IRIT_DOT_PROD(Vec1, Vec2) > IRIT_EPS &&
		GMCollinear3Pts(V1, V -> Pnext -> Coord, V2))
		return TRUE;
	}

	V = V -> Pnext; 
    }
    while (V != VS); 

    return FALSE; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check if the two lines A1A2 and B1B2 interest in the XY plane.	     M
*   End points intersections, upto IRIT_EPS, are ignored.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   A1, A2:  The two points of first line.				     M
*   B1, B2:  The two points of second line.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if the two lineintersect, FALSE otherwise.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsInterLineLineXY							     M
*****************************************************************************/
static int GMIsInterLineLineXY(const IrtPtType A1, 
			       const IrtPtType A2, 
			       const IrtPtType B1, 
			       const IrtPtType B2)
{
    IrtPtType Tmp1, Tmp3, A12, B12; 
    IrtRType T1, T2; 

    IRIT_PT_SUB(A12, A2, A1); 
    IRIT_PT_SUB(B12, B2, B1); 

    if (GM2PointsFromLineLine(A1, A12, B1, B12, Tmp1, &T1, Tmp3, &T2)) {
	/* Check if the intersection is located in sections. */
	return T1 > IRIT_EPS && T1 < 1.0 - IRIT_EPS &&
	       T2 > IRIT_EPS && T2 < 1.0 - IRIT_EPS;
    }
    else {				   /* Check if sections converging. */
	IrtPtType A1B1, B1A2, A1B2, B2A2, B1A1;

	if (!GMCollinear3Pts(A1, A2, B1))
	    return FALSE;
	IRIT_PT_SUB(A1B1, B1, A1); 
	IRIT_PT_SUB(B1A2, A2, B1);
	if (IRIT_DOT_PROD(A1B1, B1A2) > IRIT_EPS)
	    return TRUE;
	IRIT_PT_SUB(A1B2, B2, A1);
	IRIT_PT_SUB(B2A2, A2, B2);
	if (IRIT_DOT_PROD(A1B2, B2A2) > IRIT_EPS)
	    return TRUE;
	IRIT_PT_SUB(B1A1, A1, B1);
	IRIT_PT_SUB(A1B2, B2, A1);
	return IRIT_DOT_PROD(B1A1, A1B2) > IRIT_EPS;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the average location of the vertices in poly VS that also hold  M 
* attribute Name with value equal to Val.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   VS:     List of vertices of the Polygon.				     M
*   Point:  The resulting average location.				     M
*   Name:   The Name of the attribute to examine in each vertex.	     M
*           Can be NULL, in which case all vertices in VS are considered.    M
*   Val:    Desired value of attribute Name, in each vertex, if Name != NULL.M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if computed average is valid, FALSE otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPolyCentroid                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMComputeAverageVertex						     M
*****************************************************************************/
int GMComputeAverageVertex(const IPVertexStruct *VS, 
			   IrtPtType Point, 
			   const char *Name, 
			   int Val)
{
    const IPVertexStruct
        *V = VS; 
    int Count = 0; 

    IRIT_PT_RESET(Point); 

    do {
	if (Name == NULL || AttrGetIntAttrib(V -> Attr, Name) == Val) {
	    IRIT_PT_ADD(Point, Point, V -> Coord); 
	    Count++; 
	}

	V = V -> Pnext; 
    }
    while (V != VS && V != NULL);  

    if (Count == 0)
        return FALSE;

    IRIT_PT_SCALE(Point, 1.0 / Count); 
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a polygon that is the 1-ring of given vertex with index Index.   M
* sets an attribute "_GMNeighborVertex" with value TRUE if 1-ring vertex is  M
* an immediate neighbor of vertex Index, or FALSE otherwise.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Index:    The Index of the vertex.					     M
*   PVIdx:    The mesh in a vertices indexed structure.		 	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *: The 1-ring Polygon around the designated vertex,       M
*                     or NULL if vertex Index is on the boundary of mesh.    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGet1RingPoly2VrtxIdx						     M
*****************************************************************************/
IPVertexStruct *GMGet1RingPoly2VrtxIdx(int Index, 
				       const IPPolyVrtxIdxStruct *PVIdx)
{
    IRIT_STATIC_DATA int 
	MallocVertex = 0,
	MallocVMalloc = 0, 
	VersionNum = 0,
	*VListsLastIndex = NULL,
	*VListsVersionNum = NULL,
	*VMalloc = NULL;
    IRIT_STATIC_DATA IPVertexStruct
	**VLists = NULL;
    IPPolyPtrStruct
        *PPoly = PVIdx -> PPolys[Index];
    int i, VIndex, m,
	Len = CagdListLength(PPoly),
	CrntIndex = -1,
	NumVertex = PVIdx -> NumVrtcs, 
	**Polygons = PVIdx -> Polygons;
    IPVertexStruct *Res, *V, 
	**Vertices = PVIdx -> Vertices; 

    if (Len > MallocVMalloc) {
	if (VMalloc != NULL)
	    IritFree(VMalloc);
	VMalloc = (int *) IritMalloc(sizeof(int) * 2 * Len);
	MallocVMalloc = Len * 2;
    }
    if (NumVertex > MallocVertex) {
	if (VListsLastIndex != NULL)
	    IritFree(VListsLastIndex);
	if (VListsVersionNum != NULL)
	    IritFree(VListsVersionNum);
	if (VLists != NULL)
	    IritFree(VLists);
	VListsLastIndex = (int *) IritMalloc(sizeof(int) * 2 * NumVertex);
	VListsVersionNum = (int *) IritMalloc(sizeof(int) * 2 * NumVertex);
	VLists = (IPVertexStruct **)
		    IritMalloc(sizeof(IPVertexStruct *) * 2 * NumVertex);
	IRIT_ZAP_MEM(VListsVersionNum, sizeof(int) * 2 * NumVertex);
	VersionNum = 0;
	MallocVertex = 2 * NumVertex;
    }
    else if (VersionNum >= IRIT_MAX_INT) {
	IRIT_ZAP_MEM(VListsVersionNum, sizeof(int) * MallocVertex);
	VersionNum = 0;
    }

    VersionNum++;

    for (m = 0; PPoly != NULL; PPoly = PPoly -> Pnext, m++) {
	int j, Num; 

	i = AttrGetIntAttrib(PPoly -> Poly -> Attr, "_PIdx");
 
	/* Count number of vertices in polygon. */
	for (Num = 0; Polygons[i][Num] != -1; Num++); 

	for (j = 0; Polygons[i][j] != -1; j++) {
	    if (Polygons[i][j] == Index) {
		int k, 
		    PrevIndex = Polygons[i][(j - 1 + Num) % Num], 
		    NextIndex = Polygons[i][(j + 1) % Num]; 
		IPVertexStruct
		    *VList = NULL;

		/* The polygon is reversed. */
		if (VListsVersionNum[PrevIndex] == VersionNum) {
		    int Tmp;

		    if (VLists[PrevIndex] == NULL) {
		        /* If input turns out to be invalid - quit. */
			for (i = 0; i < j; i++)
			    if (VLists[VMalloc[i]] != NULL)
			        IPFreeVertexList(VLists[VMalloc[i]]);
			return NULL;
		    }
		    VList = VLists[PrevIndex];
		    VLists[PrevIndex] = NULL;
		    for (k = 0; k < Num - 2; k++) {
			int VIndex = (j - k - 1 + Num) % Num; 

			VList = IPAllocVertex2(VList); 
			AttrSetIntAttrib(&VList -> Attr, "_GMNeighborVertex", 
					 k == 0 ? TRUE : FALSE); 

			IRIT_PT_COPY(VList -> Coord, 
				     Vertices[Polygons[i][VIndex]] -> Coord); 
		    }
		    VListsVersionNum[PrevIndex] = 0;
		    Tmp = NextIndex;
		    NextIndex = VListsLastIndex[PrevIndex];
		    PrevIndex = Tmp;
		    while (VListsVersionNum[PrevIndex] == VersionNum) {
			IPVertexStruct *Tmp2;

			if (PrevIndex == NextIndex) {
			    /* Make vertices list cyclic and return result. */
			    IPGetLastVrtx(VLists[PrevIndex]) -> Pnext =
			                                    VLists[PrevIndex];
			    return VLists[PrevIndex];
			}
			if (VLists[PrevIndex] == NULL) {
			    /* If input turns out to be invalid - quit. */
			    for (i = 0; i < j; i++)
				if (VLists[VMalloc[i]] != NULL)
				    IPFreeVertexList(VLists[VMalloc[i]]);
			    return NULL;
			}
			Tmp2 = CagdListReverse(VLists[PrevIndex] -> Pnext);
			IPFreeVertex(VLists[PrevIndex]);
			VLists[PrevIndex] = NULL;
			IPGetLastVrtx(Tmp2) -> Pnext = VList;
			VList = IPAllocVertex2(Tmp2);
			PrevIndex = VListsLastIndex[PrevIndex];
		    }
		}
		else {
		    /* Build the complementary path from NextIndex to       */
		    /* PrevIndex excluding PrevIndex, (not through vertex)  */
		    /* Index in polygon Polygon[i].		            */
		    for (k = 0; k < Num - 2; k++) {
			int VIndex = (j + k + 1) % Num; 

			VList = IPAllocVertex2(VList); 
			AttrSetIntAttrib(&VList -> Attr, "_GMNeighborVertex", 
					 k == 0 ? TRUE : FALSE); 

			IRIT_PT_COPY(VList -> Coord, 
				     Vertices[Polygons[i][VIndex]] -> Coord); 
		    }
		    
		}

	        CrntIndex = PrevIndex; 
	        VLists[PrevIndex] = VList; 
	        VListsLastIndex[PrevIndex] = NextIndex; 
	        VListsVersionNum[PrevIndex] = VersionNum;

		/* Save vertex we malloc so if it boundary we can free it. */
		VMalloc[m] = PrevIndex;
	    }
	}
    }

    /* The mesh is a single point. */
    if (CrntIndex == -1) {
	for (i = 0; i < Len; i++)
	    if (VLists[VMalloc[i]] != NULL)
		IPFreeVertexList(VLists[VMalloc[i]]);
	return NULL; 
    }
    V = Res = VLists[CrntIndex]; 
    VLists[CrntIndex] = NULL;
    VIndex = VListsLastIndex[CrntIndex];

    /* Component of the collection paths to circular path. */
    while (VListsVersionNum[VIndex] == VersionNum && 
	   VIndex != CrntIndex) {
	VListsVersionNum[VIndex] = 0; 
	V = IPGetLastVrtx(V); 

	V -> Pnext = VLists[VIndex]; 
	VLists[VIndex] = NULL;
	VIndex = VListsLastIndex[VIndex]; 
    }	 

    /* Collected path does not create a circular path so the vertex is on  */
    /* the boundary, probably.					           */
    if (VListsVersionNum[VIndex] != VersionNum || 
	Res == NULL) {
	if (Res != NULL)
		IPFreeVertexList(Res);
	for (i = 0; i < Len; i++)
	    if (VLists[VMalloc[i]] != NULL)
		IPFreeVertexList(VLists[VMalloc[i]]);
	return NULL; 
    }

    IPGetLastVrtx(V) -> Pnext = Res;         /* Make vertices list cyclic. */
    return Res; 
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find out the adjacent edge of the givene edge defined by vertices	     M 
* (FirstVertexIndex, SecondVertexIndex).   The edge in the found polygon is  M
* assumed to be revsied as (SecondVertexIndex, FirstVertexIndex).            M
*                                                                            *
* PARAMETERS:                                                                M
*   PVIdx:   A mesh to search the adjacent edge in.			     M
*   FirstVertexIndex, SecondVertexIndex:   The edge to search its            M
*							      adjacent edge. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *: Pointer to the adjacent edge, or NULL if error.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMFindAdjacentPoly  						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFindAdjacentEdge  						     M
*****************************************************************************/
IPVertexStruct *GMFindAdjacentEdge(const IPPolyVrtxIdxStruct *PVIdx, 
				   int FirstVertexIndex, 
				   int SecondVertexIndex)
{
    const IPPolyPtrStruct
        *PPoly = PVIdx -> PPolys[SecondVertexIndex];
    int i;
    int * const 
        *Polygons = PVIdx -> Polygons;

    for ( ; PPoly != NULL; PPoly = PPoly -> Pnext) {
	int j, Num; 
	IPVertexStruct 
	    *V = PPoly -> Poly -> PVertex;
	
	i = AttrGetIntAttrib(PPoly -> Poly -> Attr, "_PIdx");
 
	/* Count number of vertices in polygon. */
	for (Num = 0; Polygons[i][Num] != -1; Num++); 

	/* Count indices of prev and next vertices to vertex Index in poly. */
	for (j = 0; Polygons[i][j] != -1; j++) {
	    if (Polygons[i][j] == SecondVertexIndex) {
		int NextVertex = Polygons[i][(j + 1) % Num];

		if (NextVertex == FirstVertexIndex)
		    return V;
	    }

	    V = V -> Pnext;
	}
    }
    return NULL;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find out the adjacent polygon that use the edge (V, V -> Pnext).	     M 
*   The edge in this polygon will be (V -> Pnext, V).		             M
*                                                                            *
* PARAMETERS:                                                                M
*   PVIdx:	 Data structure of original mesh.			     M
*   V, VNext:    The edge is defined from V to VNext.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *: Pointer to the adjacent polygon.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMFindAdjacentEdge  						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMFindAdjacentPoly  						     M
*****************************************************************************/
IPPolygonStruct *GMFindAdjacentPoly(const IPPolyVrtxIdxStruct *PVIdx,
				    const IPVertexStruct *V,
				    const IPVertexStruct *VNext)
					     
{
    int VIdx;
    IPPolyPtrStruct *PolyPtr, *PolyArray;
    IPPolygonStruct *Poly;
    IPVertexStruct *Ver;
    
    VIdx = AttrGetIntAttrib(VNext -> Attr, "_VIdx");
    VIdx = IRIT_ABS(VIdx) - 1;
    /* Polygon list for VNext. */
    PolyArray = PVIdx -> PPolys[VIdx];
    /* Find adjacent polygon in V Polygon list using (V,  V -> Pnext) edge. */
    for (PolyPtr = PolyArray; PolyPtr != NULL; PolyPtr = PolyPtr -> Pnext) {
        Poly = PolyPtr -> Poly;
        for (Ver = Poly -> PVertex; Ver != NULL; Ver = Ver -> Pnext) {
	    /* Find polygon with (V -> Pnext, V) edge. */
	    if (IRIT_PT_APX_EQ(Ver -> Coord, VNext -> Coord) && 
	        IRIT_PT_APX_EQ(Ver -> Pnext -> Coord, V -> Coord)) {
		return Poly;
	    }
	    if (Ver -> Pnext == Poly -> PVertex)
	        break;				     /* For circular lists. */
	}
    }
    return NULL;
}
