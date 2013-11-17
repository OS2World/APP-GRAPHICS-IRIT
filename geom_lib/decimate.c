/******************************************************************************
* Decimate.c - An implementation of modified Schroeder's decimation algorithm *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by:  Olga Tebeleva                           Ver 1.0, May. 1996     *
******************************************************************************/

#include <stdio.h>
#include "allocate.h"   
#include "geom_loc.h"   
 
#define HASH_SIZE       3000

typedef struct _PolygonStruct {
    struct _VertexStruct *Vrt[3];
    struct _PolygonStruct *PNext; 
    struct _PolygonStruct *PPrev;
    IrtPtType Normal, Center; 
    IrtRType  Areas;
    int ParallEdges;  
} PolygonStruct;

typedef struct _PolyListStruct {
    struct _PolyListStruct *PNext;
    struct _PolyListStruct *PPrev;
    PolygonStruct *Poly;
    int Attended;
} PolyListStruct;


typedef struct _VertexStruct{ 
    struct _VertexStruct *PNext;
    PolyListStruct *AdjPoly;
    IrtPtType V,N;
    int AdjPolyNum;
    int Deleted;  
    int DcmCnt;  
} VertexStruct;

IRIT_STATIC_DATA int 
    NumberOfPassages = 4, 
    VertexDcmRatio = 3;

IRIT_STATIC_DATA IrtRType 
    DistToAvrgPln = 0.01, 
    MinAspectRatio = 0.05;
 
IRIT_STATIC_DATA IrtRType
    DomainMin = 0.0,
    DomainSize = 1.0,
    Xmin = -1.0,
    Xmax =  1.0,
    Ymin = -1.0,
    Ymax =  1.0,
    Zmin = -1.0,
    Zmax =  1.0; 

typedef VertexStruct *TriangleStruct[3];

IRIT_STATIC_DATA VertexStruct *HashTable[HASH_SIZE];
IRIT_STATIC_DATA PolygonStruct *PolyLst;
IRIT_STATIC_DATA IrtPtType PlaneNormal;
IRIT_STATIC_DATA int DcmPolyNum;
IRIT_STATIC_DATA IrtPtType NullVct = {0, 0, 0};
 
static int HashFunction(IrtRType, IrtRType, IrtRType);
static VertexStruct *InsertVertex(IPVertexStruct*);
static void DecimateMesh(void);
static void Initialize(void);
static int IsSuit(VertexStruct*);
static void RemovingWithChecking(VertexStruct*);
static VertexStruct **FindStarShape(VertexStruct*, int*);
static TriangleStruct *Retriangulate(VertexStruct**, int , int*);
static int SplitVertLoop(VertexStruct**, 
			 int,
    			 TriangleStruct*,
			 int*);
static int CanBeSplitted(VertexStruct **, 
			 int, 
                         int, 
			 int, 
			 IrtRType*);
static void DeleteAdjPoly(VertexStruct*);
static void ResetAdjPolyList(VertexStruct*);
static void CalculateParameters(PolygonStruct*);
static void InsertPolygon(VertexStruct*, 
                	  VertexStruct*, 
                	  VertexStruct*);
static IPPolygonStruct *TransformToIPPolygon(PolygonStruct*);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Function set parameter of the maximal tolerant distance from the removed   M
* vertex to the average plane.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dist: Threshold distance value.					     M
*                                                                            *
* RETURN VALUE:     							     M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMDecimateObject                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDecimateObjSetDistParam, polygonal decimation, data reduction          M
*****************************************************************************/
void GMDecimateObjSetDistParam(IrtRType Dist)
{
    DistToAvrgPln = Dist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Function set parameter of the maximal number of the decimation passages    M
*                                                                            *
* PARAMETERS:                                                                M
*   PassNum: Passages number.						     M
*                                                                            *
* RETURN VALUE:     							     M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMDecimateObject                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDecimateObjSetPassNumParam, polygonal decimation, data reduction       M
*****************************************************************************/
void GMDecimateObjSetPassNumParam(int PassNum)
{
    NumberOfPassages = PassNum;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Function set parameter of the maximal number of the participation of each  M
* vertex in the triangulation process.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   DcmRatio: Maximal participation number.				     M
*                                                                            *
* RETURN VALUE:     							     M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMDecimateObject                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDecimateObjSetDcmRatioParam, polygonal decimation, data reduction      M
*****************************************************************************/
void GMDecimateObjSetDcmRatioParam(int DcmRatio)
{
    VertexDcmRatio = DcmRatio;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Function set parameter of the maximal number of the aspect ratio of the    M
* loop splitting.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MinAspRatio: minimal aspect ratio.					     M
*                                                                            *
* RETURN VALUE:     							     M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMDecimateObject                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMDecimateObjSetMinAspRatioParam, polygonal decimation, data reduction   M
*****************************************************************************/
void GMDecimateObjSetMinAspRatioParam(IrtRType MinAspRatio)
{
    MinAspectRatio = MinAspRatio;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* This function implement the decimation of the polygonal mesh by applying   M
* Schroeder's decimation scheme. The percentage of the decimation depends on M
* setting of the global variables: NumberOfPassages, VertexDcmRatio and      M
* DistToAvrgPln. If Splitting option is set to TRUE, retriangilation of the  M
* vertices loop is performed by using recursive splitting algorithm where    M
* minimal aspect ratio parametr is equal to AspectRatio. Otherwise, simple   M
* sequential triangilation is exploited.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   IPObj:     Input polygonal object in IRIT format.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  The decimated object.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMDecimateObjSetMinAspRatioParam, GMDecimateObjSetDcmRatioParam,         M
*   GMDecimateObjSetPassNumParam, GMDecimateObjSetDistParam		     M
*                                                                            *
* KEYWORDS:           					                     M
*   GMDecimateObject, Polygonal mech decimation, vertices loop splitting     M
*****************************************************************************/
IPObjectStruct *GMDecimateObject(IPObjectStruct *IPObj)
{
    IPPolygonStruct *IPPoly;
    int OrgPolyNum= 0,
	i;

    /* Initalize structure and free memory if they were created */
    PolyLst = NULL;

    for (i = 0; i < HASH_SIZE; i++) {
	while (HashTable[i] != NULL) {
	    VertexStruct *Vt;

	    Vt = HashTable[i];
	    HashTable[i] = Vt -> PNext;
	    IritFree(Vt);
	}
    }

    Xmin = Ymin = Zmin = IRIT_INFNTY;
    Xmax = Ymax = Zmax = -IRIT_INFNTY;
    for (IPPoly = IPObj -> U.Pl; IPPoly != NULL; IPPoly = IPPoly -> Pnext) {
	IPVertexStruct
	    *IPVertex = IPPoly -> PVertex;

	do {
	    if (Xmin > IPVertex -> Coord[0])
	        Xmin = IPVertex -> Coord[0];
	    if (Xmax < IPVertex -> Coord[0])
	        Xmax = IPVertex -> Coord[0];
	    if (Ymin > IPVertex -> Coord[1])
	        Ymin = IPVertex -> Coord[1];
	    if (Ymax < IPVertex -> Coord[1])
	        Ymax = IPVertex -> Coord[1];
	    if (Zmin > IPVertex -> Coord[2])
	        Zmin = IPVertex -> Coord[2];
	    if (Zmax < IPVertex -> Coord[2])
	        Zmax = IPVertex -> Coord[2];

	    IPVertex = IPVertex -> Pnext;
	}
	while (IPVertex != NULL && IPVertex != IPPoly -> PVertex);
    }
    DomainMin = Xmin + Ymin + Zmin;
    DomainSize = IRIT_EPS + Xmax + Ymax + Zmax - DomainMin;

    for (IPPoly = IPObj -> U.Pl; IPPoly != NULL; IPPoly = IPPoly -> Pnext) {
	VertexStruct *V[3];
	IPVertexStruct
	    *IPVertex = IPPoly -> PVertex;

	V[0] = InsertVertex(IPVertex);
	IPVertex = IPVertex -> Pnext;
	V[1] = InsertVertex(IPVertex);
	IPVertex = IPVertex -> Pnext;
	do {
	    V[2] = InsertVertex(IPVertex);
	    IPVertex = IPVertex -> Pnext;
	    InsertPolygon(V[0], V[1], V[2]);
	    OrgPolyNum++;
	    V[1] = V[2];
	}
	while (IPVertex != NULL && IPVertex != IPPoly -> PVertex);
    }

    DecimateMesh();

    DcmPolyNum = 0;
    IPPoly = TransformToIPPolygon(PolyLst);
#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDecimate, FALSE) {
	    IRIT_INFO_MSG_PRINTF(
                    "Object %s: Decimation percentage = %f %% \n",
		    IP_GET_OBJ_NAME(IPObj),
		    ((IrtRType) OrgPolyNum - DcmPolyNum) / OrgPolyNum * 100.0);
	}
    }
#endif /* DEBUG */

    IPObj = IPGenPOLYObject(IPPoly);
    return IPObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Insert vertex into global structure, location is determined according to  *
*  a hash function using three coordinates of the vertex.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   IPVertex: Input vertex in IRIT format.			      	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   VertexStruct *: Inserted vertex in the internal format.	             *
*****************************************************************************/
static VertexStruct *InsertVertex(IPVertexStruct *IPVertex)
{
    VertexStruct *Vt;
    int HashIndex;

    HashIndex = HashFunction(IPVertex -> Coord[0],
			     IPVertex -> Coord[1],
			     IPVertex -> Coord[2]);

    /* Check consistency of the hash index */
    if (HashIndex >= HASH_SIZE) {
        GEOM_FATAL_ERROR(GEOM_ERR_DECIM_BDRY_FAILED);
    }
 
    /* Verify vertex existance */
    for (Vt = HashTable[HashIndex]; Vt != NULL; Vt = Vt -> PNext) {
   	if (IRIT_PT_APX_EQ_EPS(Vt -> V, IPVertex -> Coord, IRIT_EPS * (Xmax - Xmin)))
	    return Vt;
    } 

    /* Create new  */
    Vt = (VertexStruct*)IritMalloc(sizeof(VertexStruct));
    IRIT_PT_COPY(Vt -> V, IPVertex -> Coord);
    IRIT_PT_COPY(Vt -> N, IPVertex -> Normal);
    Vt -> AdjPoly = NULL;
    Vt -> Deleted = FALSE;
    Vt -> AdjPolyNum = 0;
    Vt -> DcmCnt = 0;
    Vt -> PNext = HashTable[HashIndex];
    HashTable[HashIndex] = Vt;

    return Vt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Auxiliary function computing hash function value.                         *
*****************************************************************************/
static int HashFunction(IrtRType x, IrtRType y, IrtRType z)
{
    return (int) (((x + y + z - DomainMin) / DomainSize) * HASH_SIZE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Traversal all vertices in the mesh NumberOfPassages times, deletion all   *
*  vertices that are suitable candidates for removal.			     *
*  									     *
* PARAMETERS:         							     *
*    None								     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void DecimateMesh(void)
{
    int i, j;
    VertexStruct *Vt;

    for (i = 0; i < NumberOfPassages; i++) {
	Initialize();
	for (j = 0; j < HASH_SIZE; j++) 
	    for (Vt = HashTable[j]; Vt != NULL; Vt = Vt -> PNext)
		if (IsSuit(Vt))
		    RemovingWithChecking(Vt);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  For each vertex initialize counter of participation to zero.		     *
*  									     *
* PARAMETERS:     							     *
*    None								     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void Initialize(void)
{
    int i;
    VertexStruct *Vt;

    for (i = 0; i < HASH_SIZE; i++)
	for (Vt = HashTable[i]; Vt != NULL; Vt = Vt -> PNext)
	    Vt -> DcmCnt = 0;

}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check the vertex as a candidate for removing.    			     *
*  									     *
* PARAMETERS:                                                                *
*   Vertex: Input vertex that needs to be checked.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: TRUE if this vertex satisfy a threshold distance criterion.         *
*****************************************************************************/
static int IsSuit(VertexStruct *Vt)
{
    IrtPtType ComCent, Pt,
	NullVect = {0.0, 0.0, 0.0};
    IrtRType ComArea, Dist;
    PolyListStruct *PolyLs; 

  
    if (Vt -> Deleted == TRUE)
	return FALSE;
    if (Vt -> DcmCnt > VertexDcmRatio || Vt -> AdjPolyNum < 3)
	return FALSE;

    /* average plane computation */
    IRIT_PT_COPY(PlaneNormal,NullVect);
    IRIT_PT_COPY(ComCent,NullVect);
    ComArea = 0.0;
    for (PolyLs = Vt -> AdjPoly; 
	 PolyLs != NULL; 
	 PolyLs = PolyLs -> PNext) {
	PolygonStruct *PolyPtr =  PolyLs -> Poly;

	PlaneNormal[0] += PolyPtr -> Normal[0] * PolyPtr -> Areas;
        PlaneNormal[1] += PolyPtr -> Normal[1] * PolyPtr -> Areas;
        PlaneNormal[2] += PolyPtr -> Normal[2] * PolyPtr -> Areas;
        ComCent[0] += PolyPtr -> Center[0] * PolyPtr -> Areas;
        ComCent[1] += PolyPtr -> Center[1] * PolyPtr -> Areas;
        ComCent[2] += PolyPtr -> Center[2] * PolyPtr -> Areas;
        ComArea += PolyPtr -> Areas;
    }

    IRIT_PT_SCALE(PlaneNormal, 1.0 / ComArea);
    IRIT_PT_SCALE(ComCent, 1.0 / ComArea);
    if (!IRIT_PT_APX_EQ(PlaneNormal, NullVct)) 
	IRIT_PT_NORMALIZE(PlaneNormal);
    IRIT_PT_SUB(Pt, Vt -> V, ComCent);
    Dist = IRIT_DOT_PROD(PlaneNormal, Pt);   /* distance to average plane */
 
    if (IRIT_FABS(Dist) < DistToAvrgPln)
	return TRUE;    

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Trying to remove the vertex with the following retriangulation verifying *
*   some additional criteria : star shape existance and possibility of the   *
*   non-overlapping splitting if the recursive splitting algorithm is used.  *
*  									     *
* PARAMETERS:                                                                *
*   Vertex: Input vertex that needs to be removed.			     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void RemovingWithChecking(VertexStruct *Vt)
{
    VertexStruct **VList;
    int VNum, k, NumOfTriangles;
    TriangleStruct *TList;

    if ((VList = FindStarShape(Vt, &VNum)) == NULL )
	return;

    if ((TList = Retriangulate(VList, VNum, &NumOfTriangles)) == NULL) {
	IritFree(VList); 
	ResetAdjPolyList(Vt);
	return;
    }

    DeleteAdjPoly(Vt);

    Vt -> Deleted = TRUE;  /* Set vertex status as deleted */

    for (k = 0; k < VNum; k++)
	VList[k] -> DcmCnt++; 

    for (k = 0; k < NumOfTriangles; k++)
        InsertPolygon(TList[k][0], TList[k][1], TList[k][2]);

    IritFree(TList); 
    IritFree(VList); 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Rearrange vertices surrounding the vertex into star-shape form (vertices *
*   loop). The fuction return null pointer if such rearrangement can not be  *
*   done (the case of the external edge).				     *
*  									     *
* PARAMETERS:                                                                *
*   Vt: The checked vertex. 						     *
*   N:  The number of the obtained vertices in the vertices loop.	     *
*  									     *
* RETURN VALUE:                                                              *
*   VertexStruct **:  The list of the vertices that form the vertices loop.  *
*****************************************************************************/
static VertexStruct **FindStarShape(VertexStruct *Vt, int *N)
{
    VertexStruct **VList;
    int i, 
	Vnum = 0;

    VList = (VertexStruct**) IritMalloc(
             sizeof(VertexStruct*) * (Vt -> AdjPolyNum + 1));

    /* Append first two vertices */
    for (i = 0; i < 3; i++)
	if (Vt -> AdjPoly -> Poly -> Vrt[i] != Vt) { 
	    VList[Vnum++] = Vt -> AdjPoly -> Poly -> Vrt[i];
            Vt -> AdjPoly -> Attended = TRUE; 
	}
 
    /* Append consequently all other vertices */
    for (i = 0; i < Vt -> AdjPolyNum; i++) {   
	PolyListStruct *Pl;
	int j, Corresp = FALSE;
 
	for (Pl = Vt -> AdjPoly -> PNext; 
	     Pl != NULL && Corresp == FALSE;
             Pl = Pl -> PNext) { 
	    if (Pl -> Attended == FALSE) 
		for (j = 0; j < 3; j++)
		    if (Pl -> Poly -> Vrt[j] == VList[Vnum - 1]) {  
			VList[Vnum++] = (Pl -> Poly -> Vrt[(j+1)%3] == Vt) ?
					 Pl -> Poly -> Vrt[(j+2)%3]:
					 Pl -> Poly -> Vrt[(j+1)%3];
			Pl -> Attended = TRUE;
			Corresp = TRUE;
			break;
		    } 
	    if (Corresp) break;
	}
	if (VList[Vnum - 1] == VList[0])
	    break;
    } 

    /* If vertices loop unclosed */
    if (VList[Vnum-1] != VList[0] || Vnum < 4) { 
	IritFree(VList); 
	ResetAdjPolyList(Vt);
	return NULL;
    }

    *N = Vnum - 1;
    return VList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Retriangulate vertices loop by recursive splitting of the vertices loop. *
*  									     *
* PARAMETERS:                                                                *
*   VList: The vertices loop. 						     *
*   VNum:  The number of the vertices in the loop.			     *
*   N:     The number of the obtained triangles after triangulation.	     *
*  									     *
* RETURN VALUE:                                                              *
*   TriangleStruct*: The list of the triangles obtained after triangulation. *
*****************************************************************************/
static TriangleStruct *Retriangulate(VertexStruct **VList, int VNum ,int *N)
{
    int TNum = 0;
    TriangleStruct *TList;

    TList = (TriangleStruct*)IritMalloc(sizeof(TriangleStruct) * VNum);
    if (!SplitVertLoop(VList, VNum, TList, &TNum))
	return NULL;

    *N = TNum;
    return TList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Recursive splitting of the vertices loop.		       		     *
* 									     *
* PARAMETERS:                                                                *
*   VList: The vertices loop. 						     *
*   VNum:  The number of the vertices in the loop.			     *
*   TList: The list of the obtained triangles. 				     *
*   TNum:  The number of the obtained triangles after triangulation.	     *
*  									     *
* RETURN VALUE:                                                              *
*   int: TRUE if splitting succeeded					     *
*****************************************************************************/
static int SplitVertLoop(VertexStruct **VList, 
			 int VNum,
    			 TriangleStruct *TList,
			 int *TNum)
{
    int i, j, VSize1, VSize2;
    int Opti = -1, 
	Optj = -1;
    VertexStruct **VList1, **VList2;

    if ( VNum == 3 ) {
	TList[*TNum][0] = VList[0];
	TList[*TNum][1] = VList[1];
	TList[*TNum][2] = VList[2];
	(*TNum)++;
	return TRUE;
    } 

    /* Find optimal splitting */
    for (i = 0; i < VNum - 2; i++) {
	IrtRType
	    MaxDist = -IRIT_INFNTY;

	for (j = i + 2; j < VNum; j++) {
	    IrtRType AspectRatio;

	    if (i == 0 && j == VNum - 1)
	        continue;
	    if (CanBeSplitted(VList, VNum, i, j, &AspectRatio) &&
		AspectRatio > MinAspectRatio && AspectRatio > MaxDist) {
		Opti = i;
		Optj = j;
		MaxDist = AspectRatio; 
	    }
	} 
    }    

    /* split into two lists */
    if (Opti == -1)
        return FALSE;
    VSize1 = Optj - Opti + 1;
    VSize2 = VNum - VSize1 + 2;
    VList1 = (VertexStruct**) IritMalloc(sizeof(VertexStruct*) * VSize1);
    VList2 = (VertexStruct**) IritMalloc(sizeof(VertexStruct*) * VSize2);
    for (i = 0; i < VSize1; i++) 
	VList1[i] = VList[i + Opti];
    for (i = 0; i < VSize2; i++) 
	VList2[i] = VList[(i + Optj) % VNum];

 
    if (!SplitVertLoop(VList1, VSize1, TList, TNum) ||
        !SplitVertLoop(VList2, VSize2, TList, TNum))
	return FALSE;
 
    IritFree(VList1);
    IritFree(VList2);
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checking the consistency of the given splitting. 		             *
*  									     *
* PARAMETERS:                                                                *
*   VList: The vertices loop. 						     *
*   VNum:  The number of the vertices in the loop.			     *
*   i:     The index of the first vertex of the given splitting.	     *
*   j:     The index of the second vertex of the given splitting.	     *
*   AspectRatio: The minimal aspect ratio (relation between minimal distance *
*       from a vertex to the split plane and length of the split line), that *
*       correspondent to given splitting				     *
*  									     *
* RETURN VALUE:                                                              *
*   int: TRUE if this splitting is consistent				     *
*****************************************************************************/
static int CanBeSplitted(VertexStruct **VList, 
			 int VNum, 
                         int i, 
			 int j, 
			 IrtRType *AspectRatio)
{
    IrtRType CtlDist, Dist, D, Len, 
	MinDist = IRIT_INFNTY;
    int k;
    IrtPtType SplitPlane, P;

    /* Compute split plane */
    IRIT_PT_SUB(P, VList[i] -> V, VList[j] -> V);
    IRIT_CROSS_PROD(SplitPlane, P, PlaneNormal);
    Len = IRIT_PT_LENGTH(SplitPlane);
    if (!IRIT_PT_APX_EQ(SplitPlane, NullVct)) 
	IRIT_PT_NORMALIZE(SplitPlane);  
    D = IRIT_DOT_PROD(SplitPlane, VList[i] -> V);

    /* check consistency for first half of the splitted list */
    CtlDist = IRIT_DOT_PROD(SplitPlane, VList[i + 1] -> V) - D;
    MinDist = IRIT_MIN(MinDist, IRIT_FABS(CtlDist));
    for (k = i + 2; k < j; k ++ ) {
	Dist = IRIT_DOT_PROD(SplitPlane, VList[k] -> V) - D;
	if (Dist * CtlDist <= 0)
	    return FALSE;
	MinDist = IRIT_MIN(MinDist, IRIT_FABS(Dist));
    }
      
    /* check consistency for second half of the splitted list */
    Dist = IRIT_DOT_PROD(SplitPlane, VList[(j + 1) % VNum] -> V) - D;
    if (CtlDist * Dist > 0)
        return FALSE;
    MinDist = IRIT_MIN(MinDist, IRIT_FABS(Dist));
    CtlDist = Dist;

    for (k = (j + 2) % VNum; k != i; k = (k + 1) % VNum ) {
	Dist = IRIT_DOT_PROD(SplitPlane, VList[k] -> V) - D;
	if (CtlDist * Dist <= 0)
	    return FALSE;
	MinDist = IRIT_MIN(MinDist, IRIT_FABS(Dist));
    }
 
    *AspectRatio = MinDist / Len;
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Delete all polygons, adjasted to this vertex; delete all references to   *
*   these polygons from the polygon lists of all surrounding vertices.	     *
*  									     *
* PARAMETERS:                                                                *
*   Vt: Removed vertex 							     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void DeleteAdjPoly(VertexStruct *Vt)
{
    PolyListStruct *Pl, *PlNd;

    for (Pl = Vt -> AdjPoly; Pl != NULL;) { 
	PolygonStruct *Poly;
	int i;

	/* Remove connections.*/
	Poly = Pl -> Poly;
	if (Poly -> PPrev != NULL) { 
	    PolygonStruct *PolyPtr = Poly -> PPrev;

            PolyPtr -> PNext = Poly -> PNext;
            if (Poly -> PNext != NULL) {
		PolyPtr = Poly -> PNext;
		PolyPtr -> PPrev = Poly -> PPrev;
            }
	}
        else { 
	    PolyLst = Poly -> PNext;
            if (PolyLst != NULL) PolyLst -> PPrev = NULL;
        }
 
	for (i = 0; i < 3; i++)	{ 
	    VertexStruct *V = Poly -> Vrt[i];
	    PolyListStruct *PlLst = V -> AdjPoly;

	    if (V != Vt)  
                while (PlLst != NULL) {
		    if (PlLst -> Poly == Poly) {
			if (PlLst -> PPrev != NULL) { 
			    PlNd = PlLst -> PPrev;
			    PlNd -> PNext = PlLst -> PNext;
			    if (PlLst -> PNext != NULL) {
				PlNd = PlLst -> PNext;
				PlNd -> PPrev = PlLst -> PPrev;
			    }
			}
			else { 
			    V -> AdjPoly = PlLst -> PNext;
			    if (PlLst -> PNext != NULL)
			        V -> AdjPoly -> PPrev = NULL;
			}
			PlNd = PlLst;
			PlLst = PlLst -> PNext;
			IritFree(PlNd);
			V -> AdjPolyNum--;
			break;
		    }  
		    else
		        PlLst = PlLst -> PNext;
		}
	}

	IritFree(Poly);  
	PlNd = Pl; 
	Pl = Pl -> PNext;
	IritFree(PlNd); 
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Reset list of the polygon adjacent to the vertex.			     *
*  									     *
* PARAMETERS:                                                                *
*   Vt: Given vertex.							     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void ResetAdjPolyList(VertexStruct *Vt)
{
    PolyListStruct *Pl;

    for (Pl = Vt -> AdjPoly; Pl != NULL; Pl = Pl -> PNext)
	Pl -> Attended = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculate parametrs of the polygon: normal, area, center.		     *
*  									     *
* PARAMETERS:                                                                *
*   Polygon: Given polygon.						     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void CalculateParameters(PolygonStruct *Polygon)
{
    IrtPtType Side1, Side2, Side3,
	NullVct = { 0, 0, 0 };
    IrtRType  Per, a, b, c;

    IRIT_PT_SUB(Side1, Polygon -> Vrt[1] -> V, Polygon -> Vrt[0] -> V);
    IRIT_PT_SUB(Side2, Polygon -> Vrt[2] -> V, Polygon -> Vrt[1] -> V);
    IRIT_PT_SUB(Side3, Polygon -> Vrt[0] -> V, Polygon -> Vrt[2] -> V);
    IRIT_CROSS_PROD(Polygon -> Normal, Side1, Side2);

    if (IRIT_PT_APX_EQ(Polygon -> Normal, NullVct)) 
	Polygon -> ParallEdges = TRUE;
    else 
	IRIT_PT_NORMALIZE(Polygon -> Normal);  

    a = IRIT_PT_LENGTH(Side1);
    b = IRIT_PT_LENGTH(Side2);
    c = IRIT_PT_LENGTH(Side3);
    Per = (a + b + c) * 0.5;

    Polygon -> Areas = sqrt (Per * (Per - a) * (Per - b) * (Per - c));
    IRIT_PT_ADD(Polygon -> Center, Polygon -> Vrt[0] -> V, 
	   Polygon -> Vrt[1] -> V);
    IRIT_PT_ADD(Polygon -> Center, Polygon -> Center, 
	   Polygon -> Vrt[2] -> V);
    IRIT_PT_SCALE(Polygon -> Center, 1.0 / 3.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Insert new polygon into polygon list.	    			     *
*  									     *
* PARAMETERS:                                                                *
*   V1: First vertex of the created polygon.				     *
*   V2: Second vertex of the created polygon.				     *
*   V3: Third vertex of the created polygon.				     *
*                                                                            *
* RETURN VALUE:     							     *
*    void			                                             *
*****************************************************************************/
static void InsertPolygon(VertexStruct *V1, 
                	  VertexStruct *V2, 
                	  VertexStruct *V3)
{
    PolygonStruct *Poly;
    PolyListStruct *PolyPtr;
    IrtPtType Vect1, Vect2, V, P;
  

    Poly = (PolygonStruct*)IritMalloc(sizeof(PolygonStruct));
    Poly -> PNext = PolyLst;
    if (PolyLst != NULL)
	PolyLst -> PPrev = Poly;
    Poly -> PPrev = NULL;
    Poly -> ParallEdges = FALSE;
    PolyLst = Poly; 

    /* check correct order of the vertices */      
    IRIT_PT_SUB(Vect1, V2 -> V, V1 -> V);
    IRIT_PT_SUB(Vect2, V3 -> V, V2 -> V);
    IRIT_CROSS_PROD(V, Vect1, Vect2);
    IRIT_PT_ADD(P, V1 -> V, V);
    if ( IRIT_DOT_PROD(V, P) - IRIT_DOT_PROD(V, V1 -> V) >= 0 ) {
	Poly -> Vrt[0] = V1;
	Poly -> Vrt[1] = V2; 
	Poly -> Vrt[2] = V3; 
    }
    else {
	Poly -> Vrt[0] = V1;
	Poly -> Vrt[1] = V3; 
	Poly -> Vrt[2] = V2; 
    }
    CalculateParameters(Poly);

    PolyPtr = (PolyListStruct*)IritMalloc(sizeof(PolyListStruct));
    PolyPtr -> Poly = Poly;
    PolyPtr -> Attended = FALSE;
    PolyPtr -> PNext = V1 -> AdjPoly;
    PolyPtr -> PPrev = NULL;
    if (V1 -> AdjPoly != NULL)
	V1 -> AdjPoly -> PPrev = PolyPtr;
    V1 -> AdjPoly = PolyPtr;
    V1 -> AdjPolyNum++;

    PolyPtr = (PolyListStruct*)IritMalloc(sizeof(PolyListStruct));
    PolyPtr -> Poly = Poly;
    PolyPtr -> Attended = FALSE;
    PolyPtr -> PNext = V2 -> AdjPoly;
    PolyPtr -> PPrev = NULL;
    if (V2 -> AdjPoly != NULL)
	V2 -> AdjPoly -> PPrev = PolyPtr;
    V2 -> AdjPoly = PolyPtr;
    V2 -> AdjPolyNum++;

    PolyPtr = (PolyListStruct*)IritMalloc(sizeof(PolyListStruct));
    PolyPtr -> Poly = Poly;
    PolyPtr -> Attended = FALSE;
    PolyPtr -> PNext = V3 -> AdjPoly;
    PolyPtr -> PPrev = NULL;
    if (V3 -> AdjPoly != NULL)
	V3 -> AdjPoly -> PPrev = PolyPtr;
    V3 -> AdjPoly = PolyPtr;
    V3 -> AdjPolyNum++;
} 

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Transform list of polygons into format IPPolygonStruct.		     *
*  									     *
* PARAMETERS:                                                                *
*   PolyLst: List of polygons.						     *
*  									     *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: List of polygons in IPPolygonStruct format.	     *
*****************************************************************************/
static IPPolygonStruct *TransformToIPPolygon(PolygonStruct *PolyLst)
{
    IPPolygonStruct *NewPoly,
	*IPPolyLst = NULL;
    int DoCirc = IPSetPolyListCirc(FALSE);

    IPSetPolyListCirc(DoCirc);			 /* Restore original value. */

    while (PolyLst) {
	PolygonStruct *Poly;
	IPVertexStruct *V1, *V2, *V3;

	Poly = PolyLst;
	PolyLst = PolyLst -> PNext;
	DcmPolyNum++;
	if (Poly -> ParallEdges) {
    	    IritFree(Poly); 
	    continue; 
	}

	V1 = IPAllocVertex2(NULL);
	IRIT_PT_COPY(V1 -> Coord, Poly -> Vrt[0] -> V);
	IRIT_PT_COPY(V1 -> Normal, Poly -> Vrt[0] -> N);
	IP_SET_NORMAL_VRTX(V1);

	V2 = IPAllocVertex2(V1);
	IRIT_PT_COPY(V2 -> Coord, Poly -> Vrt[1] -> V);
	IRIT_PT_COPY(V2 -> Normal, Poly -> Vrt[1] -> N);
	IP_SET_NORMAL_VRTX(V2);

	V3 = IPAllocVertex2(V2);
	IRIT_PT_COPY(V3 -> Coord, Poly -> Vrt[2] -> V);
	IRIT_PT_COPY(V3 -> Normal, Poly -> Vrt[2] -> N);
	IP_SET_NORMAL_VRTX(V3);

	if (DoCirc)
	    V1 -> Pnext = V3;

	NewPoly = IPAllocPolygon(0, V3, IPPolyLst);
	IRIT_PT_COPY(NewPoly -> Plane, Poly -> Normal);
	NewPoly -> Plane[3] = -IRIT_DOT_PROD(Poly -> Normal, Poly -> Center);
	NewPoly -> Attr = NULL;
	IPPolyLst = NewPoly;	
    	IritFree(Poly);  
    }

    return IPPolyLst; 
}
