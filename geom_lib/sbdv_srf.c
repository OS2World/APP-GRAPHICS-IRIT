/******************************************************************************
* Subdivision.c -  performs subdivision of polygonal mesh.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology		      *
*******************************************************************************
* Written by Sagit Asman, Jul. 2011.					      *
******************************************************************************/

#include "geom_loc.h"

/* The function supports maximum 4 edges per face in result object.	     */
/* (Only triangles / quadrangles).					     */
#define GM_SUB_SRFS_MAX_NUM_OF_EDGES_PER_FACE	4			 

/* GMSubSrfsVrtcsStruct - holds vectors for face vertices, edge vertices and */
/* vertex vertices of polygons that are created in subdivision scheme.	     */
typedef struct GMSubSrfsVrtcsStruct {
    IrtPtType *PFacePts;     /* Arrays of size NumOfFacePts. */
    IrtPtType **EdgePts;     /* Arrays of size NumOfFaces X NumOfEdgesPts. */
    IrtPtType *VertexPts;    /* Arrays of size NumOfVertexPts. */
    int NumOfFacePts;
    int NumOfFaces;
    int NumOfEdgesPts;
    int NumOfVertexPts;
} GMSubSrfsVrtcsStruct;

/* GMSubParamStruct - holds required parameters for GMSubSrfs function (type */
/* & amount of parameters may be different for different subdivision schemes)*/
typedef struct GMSubSrfsDataStruct {
    IPObjectStruct *OriginalObj; /* A pointer to original polygonal object.  */
    int CreateFacePts; /* TRUE - if scheme creates face points, else FALSE.  */
    int IsTriangleOnly; /* TRUE - if scheme is defined for triangle meshes   */
			/* only, else FALSE.			             */
    IrtRType WCoefficient;  /* Tension parameter of butterfly subdivision    */
			    /* scheme.					     */
} GMSubSrfsDataStruct;

/* GMSubParamStruct - holds required parameters for Callback functions of    */
/* subdivision process (CreateVerticesFuncType and CreateRefineObjFuncType). */
typedef struct GMSubParamStruct {
    GMSubSrfsVrtcsStruct *GMSubSrfsVrtcs; /* Holds new vertices.	     */
    const IPPolygonStruct *Pl;	  /* Pointer to original Polygon list.	     */
    IPPolyVrtxIdxStruct *PVIdx;	  /* Data structure of original mesh.	     */
    IrtRType WCoefficient;        /* Tension parameter of butterfly scheme.  */
} GMSubParamStruct;

typedef void (*GMCreateVerticesFuncType)(GMSubParamStruct *GMSubParam);
typedef IPObjectStruct*(*GMCreateRefineObjFuncType)(
						 GMSubParamStruct *GMSubParam);
static int GMObjectHasUptoNGonsOnly(const IPObjectStruct *PolyObj, int n);
static void GMSubSrfsVrtcsNew(GMSubSrfsVrtcsStruct *GMSubSrfsVrtcs,
			      int NumOfFacePts,
			      int NumOfPlygs,
			      int NumOfEdgesPts,
			      int NumOfVertexPts);
static void GMSubSrfsVrtcsFree(GMSubSrfsVrtcsStruct *GMSubSrfsVrtcs);
static IPObjectStruct *GMSubGeneralSubdivSrfs(
				GMSubSrfsDataStruct *GMSubSrfsData, 
			        GMCreateVerticesFuncType CreateVerticesFunc,
				GMCreateRefineObjFuncType CreateRefineObjFunc);
static void GMSubCreateCCVertices(GMSubParamStruct *GMSubParam);
static void GMSubCreateCCFacePts(const IPPolyVrtxIdxStruct *PVIdx, 
  				 GMSubSrfsVrtcsStruct *GMSubCCVrtcs);
static void GMSubCreateCCEdgePts(const IPPolyVrtxIdxStruct *PVIdx,
				 const IPPolygonStruct *Pl,
				 GMSubSrfsVrtcsStruct *GMSubCCVrtcs);
static void GMSubCreateCCVertexPts(IPPolyVrtxIdxStruct *PVIdx, 
				   const IPPolygonStruct *Pl,
				   GMSubSrfsVrtcsStruct *GMSubCCVrtcs);
static void GMSubCreateLoopVertices(GMSubParamStruct *GMSubParam);
static void GMSubCreateLoopEdgePts(const IPPolyVrtxIdxStruct *PVIdx,
				   const IPPolygonStruct *Pl,
				   GMSubSrfsVrtcsStruct *GMSubLoopVrtcs);
static void GMSubCreateLoopVertexPts(IPPolyVrtxIdxStruct *PVIdx,
				     const IPPolygonStruct *Pl,
				     GMSubSrfsVrtcsStruct *GMSubLoopVrtcs);
static void GMSubCreateButterflyVertices(GMSubParamStruct *GMSubParam);
static void GMSubCreateButterflyEdgePts(
				       IPPolyVrtxIdxStruct *PVIdx,
				       const IPPolygonStruct *Pl,
				       IrtRType ButterflyWCoef,
				       GMSubSrfsVrtcsStruct *GMSubBtrflyVrtcs);
static void GMSubCreateButterflyVertexPts(
				       const IPPolyVrtxIdxStruct *PVIdx,
				       GMSubSrfsVrtcsStruct *GMSubBtrflyVrtcs);
static IPObjectStruct* GMSubCreateRefineObject(GMSubParamStruct *GMSubParam);

/******************************************************************************
* DESCRIPTION:                                                                M
*   Refines a polygonal object according to Catmull-Clark subdivision rules.  M
*   One iteration is performmed.					      M
*                                                                             *
* PARAMETERS:		                                                      M
*   OriginalObj:    A pointer to the original polygonal object.	    	      M
*                                                                             *
* RETURN VALUE:                                                               M
*   IPObjectStruct *: Pointer to refined polygonal object after subdivision.  M
*                                                                             *
* KEYWORDS:                                                                   M
*   GMSubCatmullClark							      M
******************************************************************************/
IPObjectStruct *GMSubCatmullClark(IPObjectStruct *OriginalObj)
{
    GMSubSrfsDataStruct GMSubSrfsData;

    GMSubSrfsData.OriginalObj = OriginalObj;
    GMSubSrfsData.CreateFacePts = TRUE;
    GMSubSrfsData.IsTriangleOnly = FALSE;
    return GMSubGeneralSubdivSrfs(&GMSubSrfsData, GMSubCreateCCVertices, 
				  GMSubCreateRefineObject);
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Refines a polygonal object according to Loop subdivision rules.	      M
*   One iteration is performmed.					      M
*                                                                             *
* PARAMETERS:		                                                      M
*   OriginalObj:    A pointer to the original polygonal object.	    	      M
*                                                                             *
* RETURN VALUE:                                                               M
*   IPObjectStruct *: Pointer to refined polygonal object after subdivision.  M
*                                                                             *
* KEYWORDS:                                                                   M
*   GMSubLoop								      M
******************************************************************************/
IPObjectStruct *GMSubLoop(IPObjectStruct *OriginalObj)
{
    GMSubSrfsDataStruct GMSubSrfsData;

    GMSubSrfsData.OriginalObj = OriginalObj;
    GMSubSrfsData.CreateFacePts = FALSE;
    GMSubSrfsData.IsTriangleOnly = TRUE;
    return GMSubGeneralSubdivSrfs(&GMSubSrfsData, GMSubCreateLoopVertices,
				  GMSubCreateRefineObject);
}

/******************************************************************************
* DESCRIPTION:                                                                M
*   Refines a polygonal object according to Butterfly subdivision rules.      M
*   One iteration is performmed.					      M
*                                                                             *
* PARAMETERS:		                                                      M
*   OriginalObj:    A pointer to the original polygonal object.	    	      M
*   ButterflyWCoef: The scalar butterfly blending coeefficient.               M
*                                                                             *
* RETURN VALUE:                                                               M
*   IPObjectStruct *: pointer to refined polygonal object after subdivision.  M
*                                                                             *
* KEYWORDS:                                                                   M
*   GMSubButterfly							      M
******************************************************************************/
IPObjectStruct *GMSubButterfly(IPObjectStruct *OriginalObj, 
			       IrtRType ButterflyWCoef)
{
    GMSubSrfsDataStruct GMSubSrfsData;

    GMSubSrfsData.OriginalObj = OriginalObj;
    GMSubSrfsData.CreateFacePts = FALSE;
    GMSubSrfsData.IsTriangleOnly = TRUE;
    GMSubSrfsData.WCoefficient = ButterflyWCoef;
    return GMSubGeneralSubdivSrfs(&GMSubSrfsData, GMSubCreateButterflyVertices,
				  GMSubCreateRefineObject);
}

/******************************************************************************
* DESCRIPTION:								      *
*   Main entry function. Refines a polygonal object according to selected     *
*   subdivision scheme. One iteration is performmed.			      *
*   The routine supports Catmull-Clarck subdivision scheme, Loop subdivision  *
*   scheme / Butterfly subdivision scheme.				      *
*                                                                             *
* PARAMETERS:								      *
*   GMSubSrfsData:  Holds required parameters according to selected	      *
*   		    scheme.				 		      *
*									      *
* RETURN VALUE:								      *
*   IPObjectStruct *: Pointer to refined polygonal object after subdivision.  *
*                                                                             *
* KEYWORDS:                                                                   *
*   GMSubSrfs								      *
******************************************************************************/
static IPObjectStruct *GMSubGeneralSubdivSrfs(
				 GMSubSrfsDataStruct *GMSubSrfsData, 
			         GMCreateVerticesFuncType CreateVerticesFunc,
				 GMCreateRefineObjFuncType CreateRefineObjFunc)
{
    int i, NumOfFacePts;
    GMSubSrfsVrtcsStruct GMSubSrfsVrtcs;
    GMSubParamStruct GMSubParam;
    IPPolyVrtxIdxStruct *PVIdx;
    IPPolygonStruct *Pl;
    IPObjectStruct *PObjNGons,
	*CCObj = NULL;
 
    if (GMSubSrfsData -> IsTriangleOnly) {
	/* Check if Object contains only triangles. */
	if (GMObjectHasUptoNGonsOnly(GMSubSrfsData -> OriginalObj, 3))
	    PObjNGons = GMSubSrfsData -> OriginalObj;
	else 
	    /* Creates a new polygonal objects that contains only triangles. */
	    PObjNGons = 
		  GMConvertPolysToTriangles(GMSubSrfsData -> OriginalObj);
    }
    else {
	/* Check if Object contains only quadrangles. */
	if (GMObjectHasUptoNGonsOnly(GMSubSrfsData -> OriginalObj, 4))
	    PObjNGons = GMSubSrfsData -> OriginalObj;
        else 
	    /* Creates a new polygonal objects that contains only   */
	    /* quadrangles.					    */
	    PObjNGons = GMConvertPolysToNGons(GMSubSrfsData -> OriginalObj, 4);
    }
  
    /* Set polygon index. */   
    for (i = 0, Pl = PObjNGons -> U.Pl; Pl != NULL; Pl = Pl -> Pnext, i++)
	Pl -> IAux = i;

    /* Convert polygonal mesh to PolyIdx structure. */
    PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PObjNGons, TRUE, 0);
    
    /* Allocate memory for new vertices vector and for new faces & edges    */
    /* vectors .							    */
    if (GMSubSrfsData -> CreateFacePts == TRUE) 
	/* Number of face points to allocate. */
	NumOfFacePts = PVIdx -> NumPlys;
    else    /* There are no face points according to selected scheme. */
	NumOfFacePts = 0;

    GMSubSrfsVrtcsNew(&GMSubSrfsVrtcs, NumOfFacePts, PVIdx -> NumPlys,
		      GM_SUB_SRFS_MAX_NUM_OF_EDGES_PER_FACE,
		      PVIdx -> NumVrtcs);

    /* Assign values to param structure. */
    GMSubParam.GMSubSrfsVrtcs = &GMSubSrfsVrtcs;
    GMSubParam.Pl = PObjNGons -> U.Pl;
    GMSubParam.PVIdx = PVIdx;
    GMSubParam.WCoefficient = GMSubSrfsData -> WCoefficient;
    
    /* Creates new face points, edge points and vertex points. */
    CreateVerticesFunc(&GMSubParam);
  
    /* Creates a new polygonal object out of calculated vertices coords. */
    CCObj = CreateRefineObjFunc(&GMSubParam); 

    /* Free memory. */
    GMSubSrfsVrtcsFree(&GMSubSrfsVrtcs); 
    IPPolyVrtxIdxFree(PVIdx);
    if (PObjNGons != GMSubSrfsData -> OriginalObj)
	IPFreeObject(PObjNGons);

    return CCObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests if a polygonal objects contains only polygons of upto n vertices.  *
*                                                                            *
* PARAMETERS:                                                                *
*   PolyObj:   Polygonal object to test.				     *
*   n:	       Maximal number of vertices.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if contains only polygons of upto n vertices, FALSE       *
*	      otherwise.						     *
*****************************************************************************/
static int GMObjectHasUptoNGonsOnly(const IPObjectStruct *PolyObj, int n)
{
    int i,
	HasUpToNGons = TRUE;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    /* Count number of vertices of each polygon. */ 
    for (Pl = PolyObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	for (i = 0, V = Pl -> PVertex; V != NULL; V = V -> Pnext) {
	    i++;
	    if (V -> Pnext == Pl -> PVertex)
		break;           /* For circular lists. */
        }
	if (i > n) {
	    HasUpToNGons = FALSE;
	    break;
	}
    }
    return HasUpToNGons;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates the memory required for all vectors of vertices that are 	     *
*   created in Catmull-Clark subdivision.				     * 
*                                                                            *
* PARAMETERS:                                                                *
*   GMSubSrfsVrtcs:     Holds vectors of vertices.		             *
*   NumOfFacePts:	Number of face vertices to allocate.                 *
*   NumOfPlygs:		First dimension of edge vertices array to allocate . *
*   NumOfEdgesPts:	Second dimension of edge vertices array to allocate. *
*   NumOfVertexPts:	Number of vertex vertices to allocate.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubSrfsVrtcsNew(GMSubSrfsVrtcsStruct *GMSubSrfsVrtcs,
			      int NumOfFacePts,
			      int NumOfFaces,
			      int NumOfEdgesPts,
			      int NumOfVertexPts)
{
    int i;

    /* Allocate memory for new vertices vertex, edge vertex and face vertex */
    /* vectors .							    */
    GMSubSrfsVrtcs -> VertexPts = (IrtPtType *) 
	IritMalloc(sizeof(IrtPtType) * (NumOfVertexPts + 1));
    if (NumOfFacePts > 0)
	GMSubSrfsVrtcs -> PFacePts = (IrtPtType *) 
	    IritMalloc(sizeof(IrtPtType) * (NumOfFacePts + 1));
    else
	/* No face points according to selected scheme.*/
	GMSubSrfsVrtcs -> PFacePts = NULL;

    /* Allocate 2 dimension array for EdgePts vector.(for each polygon	    */
    /* NumOfEdgesPts edge vertices).				            */
    GMSubSrfsVrtcs -> EdgePts = (IrtPtType **) 
	IritMalloc((NumOfFaces + 1) * sizeof(IrtPtType *));

    for (i = 0; i < (NumOfFaces + 1); i++) {
	GMSubSrfsVrtcs -> EdgePts[i] = (IrtPtType *) 
	    IritMalloc(NumOfEdgesPts * sizeof(IrtPtType));
    }

    GMSubSrfsVrtcs -> NumOfFacePts = NumOfFacePts;
    GMSubSrfsVrtcs -> NumOfFaces = NumOfFaces;
    GMSubSrfsVrtcs -> NumOfEdgesPts = NumOfEdgesPts;
    GMSubSrfsVrtcs -> NumOfVertexPts = NumOfVertexPts;
    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Deallocates and frees vertices vectors of GMSubSrfsVrtcsStruct structure.  *
*                                                                            * 
* PARAMETERS:                                                                *
*   GMSubSrfsVrtcs:  To be deallocated.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GMSubSrfsVrtcsFree(GMSubSrfsVrtcsStruct *GMSubSrfsVrtcs)
{
    int i;

    /* Free memory. */
    IritFree(GMSubSrfsVrtcs -> VertexPts);
    if (GMSubSrfsVrtcs -> PFacePts != NULL)
	IritFree(GMSubSrfsVrtcs -> PFacePts);
    for (i = 0; i < ((GMSubSrfsVrtcs -> NumOfFaces) + 1); i++) {
	IritFree(GMSubSrfsVrtcs -> EdgePts[i]);
    }
    IritFree(GMSubSrfsVrtcs -> EdgePts);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates face points, edge points and vertex points,	according to	     *
*   Catmull-Clark subdivision rules.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   GMSubParam:   Holds subdivision functions parameters.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateCCVertices(GMSubParamStruct *GMSubParam)
{
    /* Create face points for each old polygon. */
    GMSubCreateCCFacePts(GMSubParam -> PVIdx, GMSubParam -> GMSubSrfsVrtcs);
    
    /* Create edge points for each old edge. */
    GMSubCreateCCEdgePts(GMSubParam -> PVIdx, GMSubParam -> Pl, 
			 GMSubParam -> GMSubSrfsVrtcs);
    
    /* Create new vertex point for each old vertex. */
    GMSubCreateCCVertexPts(GMSubParam -> PVIdx, GMSubParam -> Pl,
			   GMSubParam -> GMSubSrfsVrtcs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates face point for each old polygon, according to Catmull-Clark	     *
*   subdivision	rules.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.	             *
*   GMSubCCVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateCCFacePts(const IPPolyVrtxIdxStruct *PVIdx, 
				 GMSubSrfsVrtcsStruct *GMSubCCVrtcs)
{
    int i, j, VertexInd, *VrtxVec;
    IrtPtType p;
    IPVertexStruct *V;

    /* Create face point for each old polygon.				    */
    /* Face point defined as the average of every point in the polygon.	    */
    for (i = 0; i < GMSubCCVrtcs -> NumOfFacePts; i++) {
	
	VrtxVec = PVIdx -> Polygons[i];		    /* Polygon i.	    */
	IRIT_PT_RESET(p);			    /* initialize p with 0. */
	for (j = 0; VrtxVec[j] != -1; j++) {
	    /* New face point calculation. */
	    VertexInd = VrtxVec[j];
	    V = PVIdx -> Vertices[VertexInd];
	
	    /* Sum of all control-points in that face. */
	    IRIT_PT_ADD(p, p, V -> Coord);
	}
	
	/* New face point coordinates calculation -	       */
	/* is the sum divided by the number of control-points. */
	IRIT_PT_SCALE(p, 1.0 / j);
	
	/* Copy face vertex coords to pFaceVertex array. */
        IRIT_PT_COPY(GMSubCCVrtcs -> PFacePts[i], p);

	#ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "Polygon %d Face Point  = [%f %f %f]\n", i,
		    p[0], p[1], p[2]);
	#endif /* DEBUG_GM_PRINT_CC */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates edge point for each old edge, according to Catmull-Clark	     *
*   subdivision	rules.						 	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.		     *
*   Pl:			Pointer to original Polygon list.		     *
*   GMSubCCVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateCCEdgePts(const IPPolyVrtxIdxStruct *PVIdx,
				 const IPPolygonStruct *Pl,
				 GMSubSrfsVrtcsStruct *GMSubCCVrtcs)
{
    int i, j, Num, OtherPolygonIndex, VertexInd, VertexNextInd, *VrtxVec;
    IrtPtType MidPtEdge, MidPtFace, NewEdgePt;
    const IPPolygonStruct *OtherPolygon;
    IPVertexStruct *V, *VNext;
    
    /* Create edge points for each old edge. Edge point is defined as the    */
    /* average of the two control points on either side of the edge, and the */
    /* face-points of the touching faces.				     */

    for (i = 0; i < GMSubCCVrtcs -> NumOfFaces; i++) {
   	/* Calculate edge points for Polygon i. */
	VrtxVec = PVIdx -> Polygons[i];
	#ifdef DEBUG_GM_PRINT_CC
	fprintf(stderr, "\n Polygon %d \n", i);
	#endif /* DEBUG_GM_PRINT_CC */
	
	/* Count number of vertices in polygon i. */
        for (Num = 0; VrtxVec[Num] != -1; Num++); 

	for (j = 0; VrtxVec[j] != -1; j++) {
	     /* Initialize array with irrelevant value. */
	    IRIT_PT_SET(GMSubCCVrtcs -> EdgePts[i][j],  
			IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY);
	    /* Init. */
	    IRIT_PT_RESET(MidPtEdge);
	    IRIT_PT_RESET(MidPtFace);
	    IRIT_PT_RESET(NewEdgePt);
	    
	    /* Edges points calculation.	       */
	    /* (for each old edge - V and V -> Pnext). */
	    VertexInd = VrtxVec[j];
	    V = PVIdx -> Vertices[VertexInd];

	    VertexNextInd = VrtxVec[(j + 1) % Num];
	    VNext = PVIdx -> Vertices[VertexNextInd];

    	    /* Sum of the two control points on either side of the edge. */
	    IRIT_PT_ADD(MidPtEdge, V -> Coord, VNext -> Coord);
	    /* Average of two control points on either side of the edge. */
	    /* (midpoint edge).						 */
	    IRIT_PT_SCALE(MidPtEdge, 0.5);
	    /* Find other polygon using current edge. */
	    OtherPolygon = GMFindAdjacentPoly(PVIdx, V, VNext);
	    
	    /* Check boundary. */
	    if (OtherPolygon != NULL) {
		/* Other polygon index. */
		OtherPolygonIndex = OtherPolygon -> IAux;
		/* Sum of face-points of the touching faces (Meaning - Sum */
		/* of New face point of current	polygon and New face point */
		/* of adjacent polygon).			           */
		IRIT_PT_ADD(MidPtFace, GMSubCCVrtcs -> PFacePts[i], 
			    GMSubCCVrtcs -> PFacePts[OtherPolygonIndex]);
		/* Average of face-points of the touching faces. */
		IRIT_PT_SCALE(MidPtFace, 0.5);
		
		/* New edge point - average of midpoints (edge and faces).  */
		IRIT_PT_ADD(NewEdgePt, MidPtFace, MidPtEdge);
		IRIT_PT_SCALE(NewEdgePt, 0.5);
	    }
	    else {  /* Edge belongs to only one face - boundary - Edge      */
		    /* point is the midpoint of the original edge.	    */
		IRIT_PT_COPY(NewEdgePt, MidPtEdge);
	    }
	    /* New edge point. */
	    IRIT_PT_COPY(GMSubCCVrtcs -> EdgePts[i][j], NewEdgePt);
	    #ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "edge %d-%d vertex = [%f %f %f]\n", VertexInd,
		    VertexNextInd, NewEdgePt[0], NewEdgePt[1], 
		    NewEdgePt[2]);
	    #endif /* DEBUG_GM_PRINT_CC */
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates vertex point for each old vertex, according to Catmull-Clark     *
*   subdivision	rules.						 	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.		     *
*   Pl:			Pointer to original Polygon list.		     *
*   GMSubCCVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateCCVertexPts(IPPolyVrtxIdxStruct *PVIdx,
				   const IPPolygonStruct *Pl,
				   GMSubSrfsVrtcsStruct *GMSubCCVrtcs)
{
    int i, j, EdgeIndex, HoleEdgeIndex;
    IrtPtType MidPtEdge, FacePtAvg, OldVertexPt, MidPtOldV, HoleVAvg;
    IPPolyPtrStruct *PolyPtr, *PolyArray;
    const IPPolygonStruct *PolyList, *OtherPolygon;
    IPVertexStruct *V, *VNext, *VOld;
    
    /* Create new vertex point for each old vertex.			     */
    /* For each old vertex there are n polygons sharing it.		     */
    /* (or n edges that connect to that vertex.)			     */
    /* The new vertex is ((n - 3) /n) times the old vertex + (1/n) times the */
    /* average of the face points for those adjoining polygons		     */
    /* + (2/n) times the average of the midpoints of the edges that touch    */
    /* the old vertex.							     */
    /* For vertex points that are on the border of a hole, the new	     */
    /* coordinates are calculated as follows: in all the edges the point     */
    /* belongs to, only take in account the middles of the edges that are on */
    /* the border of the hole and calculate the average between these points */
    /* and the old vertex coordinates (also on the hole boundary).	     */
 
    for (i = 0; i < GMSubCCVrtcs -> NumOfVertexPts; i++) {
	/* Init variables. */
	IRIT_PT_RESET(MidPtEdge);
	IRIT_PT_RESET(FacePtAvg);
	IRIT_PT_RESET(HoleVAvg);
	IRIT_PT_RESET(GMSubCCVrtcs -> VertexPts[i]);
	IRIT_PT_RESET(MidPtOldV);

	/* Polygon list for each vertex. */
	PolyArray = PVIdx -> PPolys[i];
	j = 0;
	EdgeIndex = 0;
	HoleEdgeIndex = 0;
	for (PolyPtr = PolyArray; 
	     PolyPtr != NULL; 
	     PolyPtr = PolyPtr -> Pnext, j++) {
	    
	    PolyList = PolyPtr -> Poly;
	    /* Sum of face-points of the touching faces. */
	    IRIT_PT_ADD(FacePtAvg, FacePtAvg, 
			GMSubCCVrtcs -> PFacePts[PolyList -> IAux]);
	    /* Sum of midpoints of the edges that touch Old Vertex. */
	    VOld = PVIdx -> Vertices[i];
	    
	    /* Find edges that connect to VOld - Those edges are all        */
	    /* appearences of VOld and VOldNext, Find also former	    */
	    /* vertex to VOld and consider VOldFormer - VOld egde only in   */
	    /* case there is no other polygon using this edge. (if there is */
	    /* one like this, this edge will appear as VOld and VOldNext).  */ 
	    for (V = PolyList -> PVertex; V != NULL; V = V -> Pnext) {
	    	/* Find the chain VOldFormer - VOld - VOldNext. */
		VNext = V -> Pnext;
		if (IRIT_PT_APX_EQ (VNext -> Coord, VOld -> Coord)) {
		    /* Check if VOldFormer - VOld edge is a boundary. */
		    OtherPolygon = GMFindAdjacentPoly(PVIdx, V, VNext);
		    if (OtherPolygon == NULL) {	 /* This is the only	    */
						 /* appearance of	    */
						 /* VOldFormer - VOld edge. */
			/* Sum of surround edge midpoints. */
			IRIT_PT_ADD(MidPtOldV, V -> Coord, VNext -> Coord);

			/* Average of two control points on either side of  */
			/* the edge (midpoint edge).			    */
			IRIT_PT_SCALE(MidPtOldV, 0.5);
			IRIT_PT_ADD(MidPtEdge, MidPtEdge, MidPtOldV);
			IRIT_PT_RESET(MidPtOldV);

			/* Extra ordinary vertices average. */
			IRIT_PT_COPY(HoleVAvg, MidPtEdge);
			EdgeIndex++;
			HoleEdgeIndex++;
		    }
		   
		    /* VOld and VOld -> Pnext edge (VNext is VOld). */
		    IRIT_PT_ADD(MidPtOldV, VNext -> Coord,
				VNext -> Pnext -> Coord);

		    /* Average of two control points on	either side of the  */
		    /* edge (midpoint edge).		   		    */
		    IRIT_PT_SCALE(MidPtOldV, 0.5);

		     /* Sum of surround edge midpoints. */
		    IRIT_PT_ADD(MidPtEdge, MidPtEdge, MidPtOldV);
		    OtherPolygon = GMFindAdjacentPoly(PVIdx, VNext,
						      VNext -> Pnext);
		    if (OtherPolygon == NULL) {
			/* Extra ordinary vertices average. */
			IRIT_PT_ADD(HoleVAvg, HoleVAvg, MidPtOldV);
			HoleEdgeIndex++;
		    }
		    IRIT_PT_RESET(MidPtOldV);
		    EdgeIndex++;

		}
	    
		if (V -> Pnext == PolyList -> PVertex)
		    break;           /* For circular lists. */
	    }
	}
	/* Check for extraordinary point - A point is on the border of a    */
	/* hole if the number of faces the point belongs to is not equal    */
	/* to number of edges a point belongs to.			    */
	if (EdgeIndex != j) {
	    /* Average of middele edges on the border and old vertex. */
	    IRIT_PT_ADD(HoleVAvg, HoleVAvg, VOld -> Coord);
	    IRIT_PT_SCALE(HoleVAvg, 1.0 / (HoleEdgeIndex + 1));	   
	    IRIT_PT_COPY(GMSubCCVrtcs -> VertexPts[i], HoleVAvg);
	    #ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "Vertex %d Vertex Point  = [%f %f %f]\n", 
		    i, GMSubCCVrtcs -> VertexPts[i][0], 
		    GMSubCCVrtcs -> VertexPts[i][1], 
		    GMSubCCVrtcs -> VertexPts[i][2]);
	    #endif /* DEBUG_GM_PRINT_CC */
	}
	else if (j >= 3) {
	    /* 1/n times the average of face points - therefore divide the  */
	    /* sum of face points twice by j: first to compute average of   */
	    /* face points and second to compute 1/n times this average.    */
	    IRIT_PT_SCALE(FacePtAvg, 1.0 / IRIT_SQR(j));	   

	    /* Calculate 2/n times the average of the midpoins of the edges */
	    /* - Calculate the average - divide the sum of edges midpoints  */
	    /* by EdgeIndex and then multiply the average by 2/n.	    */

	    IRIT_PT_SCALE(MidPtEdge, 2.0 / (EdgeIndex * j)); 

	    /* (n-3)/n times the old vertex. */
	    IRIT_PT_COPY(OldVertexPt, VOld -> Coord);
	    IRIT_PT_SCALE(OldVertexPt, (j - 3.0) /  j);

	    /* New vertex. */
	    IRIT_PT_ADD(GMSubCCVrtcs -> VertexPts[i], 
			GMSubCCVrtcs -> VertexPts[i], FacePtAvg);
	    IRIT_PT_ADD(GMSubCCVrtcs -> VertexPts[i],
			GMSubCCVrtcs -> VertexPts[i], MidPtEdge);
	    IRIT_PT_ADD(GMSubCCVrtcs -> VertexPts[i],
			GMSubCCVrtcs -> VertexPts[i], OldVertexPt);
	    #ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "Vertex %d Vertex Point  = [%f %f %f]\n", i,
		    GMSubCCVrtcs -> VertexPts[i][0], 
		    GMSubCCVrtcs -> VertexPts[i][1],
		    GMSubCCVrtcs -> VertexPts[i][2]);
	    #endif /* DEBUG_GM_PRINT_CC */
	}
	else
	    IRIT_PT_SET(GMSubCCVrtcs -> VertexPts[i], IRIT_INFNTY, IRIT_INFNTY,
			IRIT_INFNTY);
    }
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Creates a new polygonal object out of vertices coordinates calculated     *
*   according to Catmull-Clarck subdivision scheme / Loop subdivision scheme  *
*   / Butterfly subdivision scheme, etc.		                      *
*                                                                             *
* PARAMETERS:								      *
*   GMSubParam:   Holds subdivision functions parameters.		      *
*									      *
* RETURN VALUE:                                                               *
*   IPObjectStruct *: Pointer to the new polygonal object.                    *
******************************************************************************/
static IPObjectStruct *GMSubCreateRefineObject(GMSubParamStruct *GMSubParam)
{
    int i, j, Num, Index, VertexInd, *VrtxVec;
    IPObjectStruct *CCPolyObj;
    IPVertexStruct *VEdge1, *VVertex, *VEdge2,
        *VFace = NULL;
    IrtPtType p;
    const GMSubSrfsVrtcsStruct *GMSubdivVrtcs;

    CCPolyObj = IPGenPOLYObject(NULL);
    IRIT_PT_SET(p, IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY);
    GMSubdivVrtcs = GMSubParam -> GMSubSrfsVrtcs;
    
    /* Create new  quadrilaterals / triangles  for each old polygon.   */
    /* (number of quadrilaterals as the number of edges per face).     */
    for (i = 0; i < GMSubdivVrtcs -> NumOfFaces; i++) {
	VrtxVec = GMSubParam -> PVIdx -> Polygons[i];

	/* Count number of vertices in polygon i. */
	for (Num = 0; VrtxVec[Num] != -1; Num++); 

	for (j = 0; VrtxVec[j] != -1; j++) {
	    /* Calculate vertex and edge index. */
	    Index = (j + 1) % Num;
	    VertexInd = VrtxVec[Index];
	    
	    if ((IRIT_PT_APX_EQ(GMSubdivVrtcs -> EdgePts[i][j], p)) || 
		(IRIT_PT_APX_EQ(GMSubdivVrtcs -> EdgePts[i][Index], p)) ||
		(IRIT_PT_APX_EQ(GMSubdivVrtcs -> VertexPts[VertexInd], p)))
		continue;   /* One of those points was not calculated. */
	    
	     if (GMSubdivVrtcs -> PFacePts != NULL){ 
		/* Allocates new Vertex Structure for new Face vertex. */
		VFace = IPAllocVertex2(NULL);

		/* Copy face vertex coordinates to new vertex structure. */
		IRIT_PT_COPY(VFace -> Coord, GMSubdivVrtcs -> PFacePts[i]);
	     }
    	
	    /* Allocates new Vertex Structure for new Edge vertex. */
	    VEdge1 = IPAllocVertex2(NULL);

	    /* Copy edge vertex coordinates to new vertex structure. */
	    IRIT_PT_COPY(VEdge1 -> Coord, GMSubdivVrtcs -> EdgePts[i][j]);

	    /* Allocates new Vertex Structure for new Vertex vertex. */
	    VVertex = IPAllocVertex2(NULL);
    	    
	    /* Copy vertex vertex coordinates to new vertex structure. */
	    IRIT_PT_COPY(VVertex -> Coord, 
			 GMSubdivVrtcs -> VertexPts[VertexInd]);
	    
	    /* Allocates new Vertex Structure for new Edge vertex. */
	    VEdge2 = IPAllocVertex2(NULL);
	   
	    /* Copy edge vertex coordinates to new vertex structure. */
	    IRIT_PT_COPY(VEdge2 -> Coord,  GMSubdivVrtcs -> EdgePts[i][Index]);
	   	    
	    /* To produce quadrilaterals (Catmull-Clarck subdivision) - Each */
	    /* face point connects to an edge point, which connects to a new */
	    /* vertex point, which connects to the edge point of the 	     */
	    /* adjoining edge, which returns to the face point.		     */
	    /* For triangles (Loop/Butterfly) each triangle is split into 4  */
	    /* smaller triangles. 3 new triangles are formed by connecting   */
	    /* vertex-edge-edge (there are no face points) and the last new  */
	    /* triangle sits in the center,connecting the three edge points. */
	    VEdge1 -> Pnext = VVertex;
	    VVertex -> Pnext = VEdge2;

	    /* Produce quadrilaterals - edge-vertex-edge-face */
	    if (GMSubdivVrtcs -> PFacePts != NULL){ 
		VEdge2 -> Pnext = VFace;
		VFace -> Pnext = VEdge1;
	    }
	    else {
		/* No face points - Produce triangles - vertex-edge-edge. */
		VEdge2 -> Pnext = VEdge1; 
	    }

	    CCPolyObj -> U.Pl = IPAllocPolygon(0, VEdge1, CCPolyObj -> U.Pl);
	    IPUpdatePolyPlane(CCPolyObj -> U.Pl);
	}

	/* Only For triangles schemes - Create the interior triangle. */
	if (GMSubdivVrtcs -> PFacePts == NULL &&
	    !IRIT_PT_APX_EQ(GMSubdivVrtcs -> EdgePts[i][0], p) &&
	    !IRIT_PT_APX_EQ(GMSubdivVrtcs -> EdgePts[i][1], p) &&
	    !IRIT_PT_APX_EQ(GMSubdivVrtcs -> EdgePts[i][2], p)){ 
	
	    IPVertexStruct *VEdge3;
	    /* Allocates new Vertex Structure for first Edge vertex. */
	    VEdge1 = IPAllocVertex2(NULL);

	    /* Copy edge vertex coordinates to new vertex structure. */
	    IRIT_PT_COPY(VEdge1 -> Coord, GMSubdivVrtcs -> EdgePts[i][0]);

	    /* Allocates new Vertex Structure for second Edge vertex. */
	    VEdge2 = IPAllocVertex2(NULL);

	    /* Copy edge vertex coordinates to new vertex structure. */
	    IRIT_PT_COPY(VEdge2 -> Coord, GMSubdivVrtcs -> EdgePts[i][1]);

	    /* Allocates new Vertex Structure for third Edge vertex. */
	    VEdge3 = IPAllocVertex2(NULL);

	    /* Copy edge vertex coordinates to new vertex structure. */
	    IRIT_PT_COPY(VEdge3 -> Coord, GMSubdivVrtcs -> EdgePts[i][2]);

	    /* Connect the three edge points. */
	    VEdge1 -> Pnext = VEdge2;
	    VEdge2 -> Pnext = VEdge3;
	    VEdge3 -> Pnext = VEdge1;
	    CCPolyObj -> U.Pl = IPAllocPolygon(0, VEdge1, CCPolyObj -> U.Pl);
	    IPUpdatePolyPlane(CCPolyObj -> U.Pl);
	}
    }
    return CCPolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates face points, edge points and vertex points,	according to	     *
*   Loop subdivision rules.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   GMSubParam:   Holds subdivision functions parameters.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateLoopVertices(GMSubParamStruct *GMSubParam)
{
        
    /* Create edge points for each old edge. */
    GMSubCreateLoopEdgePts(GMSubParam -> PVIdx, GMSubParam -> Pl, 
			 GMSubParam -> GMSubSrfsVrtcs);
    
    /* Create new vertex point for each old vertex. */
    GMSubCreateLoopVertexPts(GMSubParam -> PVIdx, GMSubParam -> Pl,
			   GMSubParam -> GMSubSrfsVrtcs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates edge point for each old edge, according to Loop Subdivision	     *
*   rules.						 	     `	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.		     *
*   Pl:			Pointer to original Polygon list.		     *
*   GMSubLoopVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateLoopEdgePts(const IPPolyVrtxIdxStruct *PVIdx,
				   const IPPolygonStruct *Pl,
				   GMSubSrfsVrtcsStruct *GMSubLoopVrtcs)
{
    int i, j, VertexInd, VertexNextInd, VertexOtherInd,	*VrtxVec;
    IrtPtType EdgePtsAvg, OtherPtsFace, NewEdgePt;
    const IPPolygonStruct *OtherPolygon;
    const IPVertexStruct *V, *VNext, *VOther, *VAdjOther;
    
    /* Create edge points for each old edge. These points are three eighths  */
    /* of the sum of the two end points of the edge plus one eight of the    */
    /* sum of the two other points that form the two triangles that share    */
    /* the edge.							     */
    for (i = 0; i < GMSubLoopVrtcs -> NumOfFaces; i++) {
   	/* Calculate edge points for Polygon i. */
	VrtxVec = PVIdx -> Polygons[i];
	#ifdef DEBUG_GM_PRINT_CC
	fprintf(stderr, "\n Polygon %d \n", i);
	#endif /* DEBUG_GM_PRINT_CC */
	for (j = 0; VrtxVec[j] != -1; j++) {
	     /* Initialize array with irrelevant value. */
	    IRIT_PT_SET(GMSubLoopVrtcs -> EdgePts[i][j],  
			IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY);
	    /* Init. */
	    IRIT_PT_RESET(EdgePtsAvg);
	    IRIT_PT_RESET(OtherPtsFace);
	    IRIT_PT_RESET(NewEdgePt);
	    
	    /* Edges points calculation.	       */
	    /* (for each old edge - V and V -> Pnext). */
	    VertexInd = VrtxVec[j];
	    V = PVIdx -> Vertices[VertexInd];

	    VertexNextInd = VrtxVec[(j + 1) % 3];
	    VNext = PVIdx -> Vertices[VertexNextInd];
  
	    /* Other vertex in the current triangle. */
	    VertexOtherInd = VrtxVec[(j + 2) % 3];
	    VOther = PVIdx -> Vertices[VertexOtherInd];

	    /* Sum of the two end points of the edge. */
	    IRIT_PT_ADD(EdgePtsAvg, V -> Coord, VNext -> Coord);
	    
	    /* Find other polygon using current edge. */
	    OtherPolygon = GMFindAdjacentPoly(PVIdx, V, VNext);

	    if (OtherPolygon != NULL) {
		/* Find other point in adjacent polygon. */
		VAdjOther = GMFindThirdPointInTriangle(OtherPolygon, V, VNext);

		/* Sum of the two other points that form the two triangles   */
		/* that share the edge.					     */
		IRIT_PT_ADD(OtherPtsFace, VOther -> Coord, VAdjOther -> Coord) 
			  
		/* Weighted Average of the sum. */
		IRIT_PT_SCALE(OtherPtsFace, 0.125);

		/* Weighted Average of the sum of end points of the edge. */
		IRIT_PT_SCALE(EdgePtsAvg, 0.375);
		
		/* New edge point. */
		IRIT_PT_ADD(NewEdgePt, OtherPtsFace, EdgePtsAvg);
	    }
	    else { /* Extra ordinary vertices and boundaries.               */
		   /* Edge point is the midpoint the original edge.         */
		IRIT_PT_SCALE(EdgePtsAvg, 0.5);
		IRIT_PT_COPY(NewEdgePt, EdgePtsAvg);
	    }
	    IRIT_PT_COPY(GMSubLoopVrtcs -> EdgePts[i][j], NewEdgePt);

	    #ifdef DEBUG_GM_PRINT_CC
	        fprintf(stderr, "edge %d-%d vertex = [%f %f %f]\n", VertexInd,
		      VertexNextInd, NewEdgePt[0], NewEdgePt[1], NewEdgePt[2]);
	    #endif /* DEBUG_GM_PRINT_CC */
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates vertex point for each old vertex, according to Loop Subdivision  *
*   rules.						 		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.		     *
*   Pl:			Pointer to original Polygon list.		     *
*   GMSubLoopVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateLoopVertexPts(IPPolyVrtxIdxStruct *PVIdx,
				     const IPPolygonStruct *Pl,
				     GMSubSrfsVrtcsStruct *GMSubLoopVrtcs)
{
    int i, j, NumOfNeighbors, nfaces, *Nbrs;
    IrtRType s, vscale;
    IrtPtType SumOfNeighbors, OldVertexPt;
    IPPolyPtrStruct *PolyArray;
    const IPPolygonStruct *OtherPolygon1, *OtherPolygon2;
    IPVertexStruct *VOld;
    
    /* Create new vertex point for each old vertex. A given vertex has n     */
    /* neighbor vertices. The new vertex point is one minus n times s times  */
    /* the old vertex plus s times the sum of the neighboring vertices,	     */
    /* where s is a scaling factor.s is 1/n(5/8 - (3/8 + 1/4cos(2PI/n))^2).  */
    for (i = 0; i < GMSubLoopVrtcs -> NumOfVertexPts; i++) {
	/* Init variables. */
	IRIT_PT_RESET(SumOfNeighbors);
	IRIT_PT_RESET(GMSubLoopVrtcs -> VertexPts[i]);

	/* Count number of faces the vertex belongs to. */
	PolyArray = PVIdx -> PPolys[i];
	nfaces = 0;
	while (PolyArray != NULL) {
	    nfaces++;
	    PolyArray = PolyArray -> Pnext;
        }

	/* Find 1-ring neigbors of current old vertex. */   
	NumOfNeighbors = 0;
	Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, i, 1);
	/* Calculate sum of the neighboring vertices of old vertex. */
	for (j = 0; Nbrs[j] >= 0; j++) {
	    IPVertexStruct
		*V = PVIdx -> Vertices[Nbrs[j]];

	    IRIT_PT_ADD(SumOfNeighbors, SumOfNeighbors, V -> Coord);
	    NumOfNeighbors++;
	 }
	VOld = PVIdx -> Vertices[i];

	/* Old Vertex Coord. */
	IRIT_PT_COPY(OldVertexPt, VOld -> Coord);

	/* Check boundary. */
	if (NumOfNeighbors >= 3 && NumOfNeighbors == nfaces) {
	    /* Calculate scaling factor s. */
	    s = 0.375 + (0.25 * cos((2.0 * M_PI) / NumOfNeighbors));
	    s = (1.0 / NumOfNeighbors) * (0.625 - IRIT_SQR(s));
	}
	else {	/* extra ordinary vertices and boundaries. */
	    /* Only take in account the neighbors that are on the border. */
	    IRIT_PT_RESET(SumOfNeighbors);
	    NumOfNeighbors = 0;
	    for (j = 0; Nbrs[j] >= 0; j++) {
		IPVertexStruct
		    *Vadj = PVIdx -> Vertices[Nbrs[j]];

		OtherPolygon1 = GMFindAdjacentPoly(PVIdx, VOld, Vadj);
		OtherPolygon2 = GMFindAdjacentPoly(PVIdx, Vadj, VOld);
		if (OtherPolygon1 == NULL || OtherPolygon2 == NULL) {
		    IRIT_PT_ADD(SumOfNeighbors, SumOfNeighbors, Vadj -> Coord);
		    NumOfNeighbors++;
		}
	    }
	    /* Scaling factor for extraordinary vertices. */
	    s = 0.125;
	}

	/* Calculate vscale - one minus n times s times. */
	vscale = 1 - (s * NumOfNeighbors);
	IRIT_PT_SCALE(OldVertexPt, vscale);

	/* s times the sum of the neighboring vertices. */
	IRIT_PT_SCALE(SumOfNeighbors, s);

	/* New vertex. */
	IRIT_PT_ADD(GMSubLoopVrtcs -> VertexPts[i], OldVertexPt, 
		    SumOfNeighbors);

	#ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "Vertex %d Vertex Point  = [%f %f %f]\n", i,
		    GMSubLoopVrtcs -> VertexPts[i][0],
		    GMSubLoopVrtcs -> VertexPts[i][1], 
		    GMSubLoopVrtcs -> VertexPts[i][2]);
	#endif /* DEBUG_GM_PRINT_CC */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates face points, edge points and vertex points,	according to	     *
*   Butterfly subdivision rules.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   GMSubParam:   Holds subdivision functions parameters.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateButterflyVertices(GMSubParamStruct *GMSubParam)
{
        
    /* Create edge points for each old edge. */
    GMSubCreateButterflyEdgePts(GMSubParam -> PVIdx, GMSubParam -> Pl,  
				GMSubParam -> WCoefficient, 
				GMSubParam -> GMSubSrfsVrtcs);
    
    /* Create new vertex point for each old vertex. */
    GMSubCreateButterflyVertexPts(GMSubParam -> PVIdx, 
				  GMSubParam -> GMSubSrfsVrtcs);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates edge point for each old edge, according to Butterfly Subdivision *
*   rules.						 	     `	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.		     *
*   Pl:			Pointer to original Polygon list.		     *
*   ButterflyWCoef	Tension parameter.				     *
*   GMSubBtrflyVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateButterflyEdgePts(IPPolyVrtxIdxStruct *PVIdx,
					const IPPolygonStruct *Pl,
					IrtRType ButterflyWCoef,
					GMSubSrfsVrtcsStruct *GMSubBtrflyVrtcs)
{
    int i, j, VertexInd, VertexNextInd, VertexOtherInd, VValence, VNextValence,
	*VrtxVec, *Nbrs;
    IrtPtType APtsAvg, BPtsAvg, CPtsAvg, NewEdgePt;
    const IPPolygonStruct *OtherPolygon, *OtherPolygon_1, *OtherPolygon_2,
	  *OtherPolygon_3, *OtherPolygon_4;   
    const IPVertexStruct *V, *VNext, *VOther, *VAdjOther, *VAdj_1, *VAdj_2,
			 *VAdj_3, *VAdj_4;
    
    /* Create edge points for each old edge - sum up the vertices in the    */
    /* stencil area around that edge, which is shaped like a butterfly,     */
    /* weighting each one by a predetermined weight.			    */

    for (i = 0; i < GMSubBtrflyVrtcs -> NumOfFaces; i++) {
   	/* Calculate edge points for Polygon i. */
	VrtxVec = PVIdx -> Polygons[i];

	#ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "\n Polygon %d \n", i);
	#endif /* DEBUG_GM_PRINT_CC */

	for (j = 0; VrtxVec[j] != -1; j++) {
	     /* Initialize array with irrelevant value. */
	    IRIT_PT_SET(GMSubBtrflyVrtcs -> EdgePts[i][j],  
			IRIT_INFNTY, IRIT_INFNTY, IRIT_INFNTY);

	    /* Init. */
	    IRIT_PT_RESET(APtsAvg);
	    IRIT_PT_RESET(BPtsAvg);
	    IRIT_PT_RESET(CPtsAvg);
	    IRIT_PT_RESET(NewEdgePt);
	    
	    /* Edges points calculation.	       */
	    /* (for each old edge - V and V -> Pnext). */
	    VertexInd = VrtxVec[j];
	    V = PVIdx -> Vertices[VertexInd];
	    
	    VertexNextInd = VrtxVec[(j + 1) % 3];
	    VNext = PVIdx -> Vertices[VertexNextInd];
	    
	    /* other vertex in the current triangle. */
	    VertexOtherInd = VrtxVec[(j + 2) % 3];
	    VOther = PVIdx -> Vertices[VertexOtherInd];
	    
	    /* Find other polygon using current edge. */
	    OtherPolygon = GMFindAdjacentPoly(PVIdx, V, VNext);

	    /* APtsAvg - Sum of the two end points of the edge. */
	    IRIT_PT_ADD(APtsAvg, V -> Coord, VNext -> Coord);

	    /* Weighted Average of the former sum. */
	    IRIT_PT_SCALE(APtsAvg, 0.5);

	    /* New edge point. */
	    IRIT_PT_ADD(NewEdgePt, NewEdgePt, APtsAvg);

	    /* Find edge's endpoints valence. */
	    Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, VertexInd, 1);
	    VValence = 0;

	    /* Count 1-ring neigbors of vertex. */   
	    while (Nbrs[VValence] >= 0)
		VValence++;
	    Nbrs = IPCnvPolyVrtxNeighbors(PVIdx, VertexNextInd, 1);
	    VNextValence = 0;
	    while (Nbrs[VNextValence] >= 0)
		VNextValence++;
	    /* If either of the edges' endpoints is of a valence less than 5 */
	    /* edge point will be the midpoint the original edge.	     */
	    if (OtherPolygon != NULL && VValence >= 5 && VNextValence >= 5) {
		/* Find other point in adjacent polygon. */
		VAdjOther = GMFindThirdPointInTriangle(OtherPolygon, V, VNext);

		/* BPtsAvg - Sum of the two other points that form the two   */
		/* triangles that share the edge.			     */
		IRIT_PT_ADD(BPtsAvg, VOther -> Coord, VAdjOther -> Coord) 
		/* Weighted Average of the former sum. */
		IRIT_PT_SCALE(BPtsAvg, 0.125 + (2 * ButterflyWCoef));
		/* Find 4 polygons adjacent to the two triangles that share  */
		/* the current edge.					     */
		/* Current triangle is (V, VNext, VOther).		     */
		OtherPolygon_1 = GMFindAdjacentPoly(PVIdx, VOther, V);
		OtherPolygon_2 = GMFindAdjacentPoly(PVIdx, VNext, VOther);
		/* Adjacent triangle is (VNext, V, VAdjOther).		     */
		OtherPolygon_3 = GMFindAdjacentPoly(PVIdx, V, VAdjOther);
		OtherPolygon_4 = GMFindAdjacentPoly(PVIdx, VAdjOther, VNext);
		if (OtherPolygon_1 != NULL && OtherPolygon_2 && OtherPolygon_3
		    && OtherPolygon_4) {
		    /* Find other point in adjacent polygon. */
		    VAdj_1 = GMFindThirdPointInTriangle(OtherPolygon_1, V, 
							VOther); 

		    /* Find other point in adjacent polygon. */
		    VAdj_2 = GMFindThirdPointInTriangle(OtherPolygon_2, 
							VNext, VOther); 

		    /* Find other point in adjacent polygon. */
		    VAdj_3 = GMFindThirdPointInTriangle(OtherPolygon_3, 
							V, VAdjOther); 

		    /* Find other point in adjacent polygon. */
		    VAdj_4 = GMFindThirdPointInTriangle(OtherPolygon_4, 
							VNext, VAdjOther); 
		    /* Verify that there are 4 different vertices. */
		    if (!IRIT_PT_APX_EQ(VAdj_1 -> Coord, VAdj_2 -> Coord) && 
			!IRIT_PT_APX_EQ(VAdj_3 -> Coord, VAdj_4 -> Coord)) {
			/* CPtsAvg - Sum those vertices. */  
			IRIT_PT_ADD(CPtsAvg, VAdj_1 -> Coord, VAdj_2 -> Coord);
			IRIT_PT_ADD(CPtsAvg, CPtsAvg, VAdj_3 -> Coord);
			IRIT_PT_ADD(CPtsAvg, CPtsAvg, VAdj_4 -> Coord);
			/* Weighted Average of the former sum. */
			IRIT_PT_SCALE(CPtsAvg, -0.0625 - ButterflyWCoef);
			/* Add BPtsAvg. */
			IRIT_PT_ADD(NewEdgePt, NewEdgePt, BPtsAvg);
			/* New edge point is sum up the vertices in the      */
			/* stencil shaped area.				     */
			IRIT_PT_ADD(NewEdgePt, NewEdgePt, CPtsAvg);
		    }
		}
	    }
	    IRIT_PT_COPY(GMSubBtrflyVrtcs -> EdgePts[i][j], NewEdgePt);

	    #ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "edge %d-%d vertex = [%f %f %f]\n", VertexInd,
	    	    VertexNextInd, NewEdgePt[0], NewEdgePt[1], NewEdgePt[2]);
	    #endif /* DEBUG_GM_PRINT_CC */
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates vertex point for each old vertex, according to Butterfly	     *
*   Subdivision rules.						 	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PVIdx:		Data structure of original mesh.		     *
*   GMSubBtrflyVrtcs:	Holds face, edge and vertex vertices.	             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void							             *
*****************************************************************************/
static void GMSubCreateButterflyVertexPts(
					const IPPolyVrtxIdxStruct *PVIdx,
					GMSubSrfsVrtcsStruct *GMSubBtrflyVrtcs)
{
    int i;
    IPVertexStruct *VOld;
    
    /* Butterfly scheme retains the old vertex of the original mesh,	     */
    /* therefore this routine only copies the origina vertices to the data   */
    /* structure of the new vertices.					     */
 						     
    for (i = 0; i < GMSubBtrflyVrtcs -> NumOfVertexPts; i++) {
	/* Init variables. */
        IRIT_PT_RESET(GMSubBtrflyVrtcs -> VertexPts[i]);
	VOld = PVIdx -> Vertices[i];
	IRIT_PT_COPY(GMSubBtrflyVrtcs -> VertexPts[i], VOld -> Coord);
	   
        #ifdef DEBUG_GM_PRINT_CC
	    fprintf(stderr, "Vertex %d Vertex Point  = [%f %f %f]\n", i, 
		    GMSubBtrflyVrtcs -> VertexPts[i][0],
		    GMSubBtrflyVrtcs -> VertexPts[i][1], 
		    GMSubBtrflyVrtcs -> VertexPts[i][2]);
	#endif /* DEBUG_GM_PRINT_CC */
    }
}

