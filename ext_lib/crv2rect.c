/******************************************************************************
* crv2rect.c - function of curves' loop to Rectangle mesh.       	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Daniel Ghosalker and Gershon Elber, July 2011.		      *
******************************************************************************/

#include "irit_sm.h"
#include "geom_lib.h"
#include "ext_loc.h"

#define IRT_C2S_NUM_STEPS			100.0
#define IRT_C2S_2ND_IS_1ST_EDGE_ENDPT(V1, V2)	(V1 == V2 || V1 -> Pnext == V2)
#define IRT_C2S_R2I_ROUND(X)			((int)(X + 0.5)) 
#define IRT_C2S_MAX_DIST(SizeRectangle)	(SizeRectangle / 8.001)

static IrtRType IrtExtC2SFindZDirection(const IPVertexStruct *VS); 
static IrtRType IrtExtC2SCurveLen(const CagdCrvStruct *Crv, 
				  IrtRType Start, 
				  IrtRType End); 
static IPObjectStruct * IrtExtC2SGetCrv(const IPVertexStruct *V, 
					IrtRType *Start, 
					IrtRType *End); 
static IPVertexStruct *IrtExtC2SBreakEdgeMiddle(IPVertexStruct *V, 
						IPObjectStruct *C, 
						IrtRType Start, 
						IrtRType End, 
						IrtRType Alpha); 
static IPVertexStruct *IrtExtC2SFindNextRefinedVertex(IPVertexStruct *V, 
						      IPVertexStruct *VEnd,
						      IrtRType Side); 
static void IrtExtC2SBreakToTwo(IPPolygonStruct *Pl, 
				IPVertexStruct *VS, 
				IPVertexStruct *VE); 
static IrtRType IrtExtC2SPerpendicular(const IPVertexStruct *V1, 
				       const IPVertexStruct *V2,
				       int IsForward);  
static int IrtExtC2SIsEndVert(IPVertexStruct *V);
static int IrtExtC2SHasRefinedPoint(IPVertexStruct **V);
static void IrtExtC2SBreakEdge(IPVertexStruct *V1, IrtRType MaxLen); 
static IPVertexStruct *IrtExtC2SFindMatchVertex(IPVertexStruct *VE,
						IrtRType Side);
static int IrtExtC2SFindSplitEdge(IPVertexStruct *V1, 
				  IPVertexStruct **VS, 
				  IPVertexStruct **VE);
static IPVertexStruct *IrtExtC2SDuplicateVertexAfter(IPVertexStruct *V);  
static IrtRType IrtExtC2SDistLineCurve(const CagdCrvStruct *Crv, 
				       IrtRType Start, 
				       IrtRType End);
static IPPolygonStruct *IrtExtC2SConvertPolysToRectangles(IPPolygonStruct
							                *Pls); 
static void IrtExtC2SLimitSplitEdgeLen(IPPolygonStruct *Pl, 
				       IrtRType MaxLen, 
				       IrtRType MaxDis);
static IPPolygonStruct *IrtExtC2SSplitLimitEdgeLenToRectangles(
							IPPolygonStruct *Pls, 
							IrtRType MaxLen); 
static int IrtExtC2SEvalCrvPt(const CagdCrvStruct *Crv, 
			      CagdPType Point, 
			      CagdRType t); 
static void IrtExtC2SSetCrv(IPVertexStruct *V, 
			    const CagdCrvStruct *Crv,
			    int IsReverse,
			    const IrtPtType Point);
static IrtRType IrtExtC2SRelativePoint(const CagdCrvStruct *Crv, 
				       CagdPType Point, 
				       IrtRType Start, 
				       IrtRType End, 
				       IrtRType Alpha); 
static CagdCrvStruct *IrtExtC2SSplitDiscont(CagdCrvStruct *CrvList,
					    CagdRType AngularDeviation); 
static IPObjectStruct *IrtExtC2SCnvPolyToFF(const IPPolygonStruct *Pl,
					    int GenSrfs,
					    const char *Name); 

static IPVertexStruct *IrtExtC2SCreatePolyFromLongCrvList(
						 const char** ErrorMsg,
						 const CagdCrvStruct *CrvList,
						 IrtRType MaxDes); 
static IPPolygonStruct *IrtExtC2SCreatePolyFromCrv(const char** ErrorMsg, 
						   CagdCrvStruct **CrvList,  
						   IrtRType MaxDes,
						   CagdRType AngularDeviation);
static IPObjectStruct *IrtExtC2SArrangePolygon(IPPolygonStruct *Pls,
						const char* Name);
static void IrtExtC2SSetEdgeUniqueId(IPObjectStruct *PolyObj);
static int IrtExtC2SUpdateEdgeMorePerpendicular(IPVertexStruct *VB1, 
						 IPVertexStruct *VB2, 
						 IrtRType *MaxOrtho, 
						 IrtRType *MaxLen, 
						 IrtRType Side);
static int IrtExtC2SOptOrAnd(int Option, int B1, int B2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Order the curves in CrvList for a Boolean Sum operation or simply in     M
* sequence.  Note the curves may also be reversed, if so required.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:        List of curves to Order.				     M
*   IsBoolSumOrder: True, if need to order to fit a coming Boolean Sum       M 
*		    operations. False, to chain them in a simple sequence.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The CrvList afrer Ordering.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtExtC2SOrderCurves						     M
*****************************************************************************/
CagdCrvStruct *IrtExtC2SOrderCurves(CagdCrvStruct *CrvList, 
				    int IsBoolSumOrder) 
{
    int i, j,
	Len = CagdListLength(CrvList); 
    IrtRType 
	*Ends = (IrtRType *) IritMalloc(sizeof(IrtRType) * 2 * Len); 
    CagdCrvStruct 
	**Curve = (CagdCrvStruct **) IritMalloc(sizeof(CagdCrvStruct *) * Len),
	*Res = CagdCrvCopy(CrvList), 
	*ResI = Res; 
    IrtPtType BeforePoint, 
	*Points = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * 2 * Len); 

    Curve[0] = CrvList; 
    for (i = 0; i < Len - 1; i++)
	Curve[i + 1] = Curve[i] -> Pnext; 

    /* Initialize Point and Ends */
    for (i = 0; i < Len; i++) {
	CagdCrvDomain(Curve[i], &(Ends[2 * i]), &(Ends[2 * i + 1])); 
	IrtExtC2SEvalCrvPt(Curve[i], Points[2 * i], Ends[2 * i]); 
	IrtExtC2SEvalCrvPt(Curve[i], Points[2 * i + 1] , Ends[2 * i + 1]); 
    }

    IRIT_PT_COPY(BeforePoint, Points[1]); 
    for (i = 1; i < Len; i++) {
	IrtRType 
	    MinDis = IRIT_INFNTY; 
	int Minj2, MinjOdd,
	    Minj = 0; 

	for (j = 2 * i; j < Len * 2; j++) {
	    IrtRType 
		Dis = IRIT_PT_PT_DIST(BeforePoint, Points[j]); 

	    if (Dis < MinDis) {
		MinDis = Dis; 
		Minj = j; 
	    }
	}

	Minj2 = (Minj >> 1);
	MinjOdd = Minj & 0x01;

	if ((!MinjOdd) ^ (IsBoolSumOrder && i > 1))
	    ResI -> Pnext = CagdCrvCopy(Curve[Minj2]); 
	else
	    ResI -> Pnext = CagdCrvReverse(Curve[Minj2]); 

	IRIT_PT_COPY(BeforePoint, Points[Minj2 * 2 + (1 - MinjOdd)]); 

	/* Remove the curve we "use". */
	IRIT_SWAP(CagdCrvStruct *, Curve[Minj2], Curve[i]); 
	IRIT_PT_SWAP(Points[Minj2 * 2], Points[i * 2]); 
	IRIT_PT_SWAP(Points[Minj2 * 2 + 1], Points[i * 2 + 1]); 
	ResI = ResI -> Pnext; 
    }

    return Res; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a list of curves that form a closed loop, tiles the region         M
* enclosed by that loop with rectangles with prescribed sizes.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:            The list of input curve.			     M
*   AngularDeviation:   Threshold of angular deviation to subdivide the      M
*                       input curves at, at discontinuities of that many     M
*			degrees.					     M
*   MaxDist:    	Maximum allowed distance of linear edge from input   M
*                       curve.						     M
*   OutputType:    	Set the output type: either Polygonal rectangles,    M
*                       Curve lists (of four boundaries each), or surfaces.  M
*   SizeRectangle:	Maximum allowed length of an edge of a polygons.     M
*   NumSmoothingSteps:  Number of times to perform smoothing algorithm.      M
*   ErrorMsg:		The message to print in case of error.		     M
*   Name:		The name of the basic (root) object.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:	A list object that holds polygons, or curves, or     M
*                       surfaces, depending on OutputType. NULL is returned  M
*                       in case of error (and ErrorMsg set).		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   IrtExtC2SGeneral							     M
*****************************************************************************/
IPObjectStruct *IrtExtC2SGeneral(CagdCrvStruct **CrvList,
				 IrtRType AngularDeviation,
				 IrtRType MaxDist,
				 int OutputType,
				 IrtRType SizeRectangle,
				 int NumSmoothingSteps,
				 const char **ErrorMsg,
				 const char *Name)
{
    IPObjectStruct *Os, 
	*ResultObject = NULL;
    IPPolygonStruct *Pl; 

    *ErrorMsg = NULL;

    /*   Converts the input set of curves to polylines in a closed loop or   */
    /* returns NULL if an error (i.e. not a closed loop).		     */
    /*   The number of polylines will be similar to the number of input      */
    /* curves unless discontinuities are detected (and divided at) or a      */
    /* <3 curve(s) is given in which case 3 polylines are returned.          */
    /*   Every edge (V, V -> Pnext) in the polyline(s) will hold the curve   */
    /* as an "IrtExtC2SCurve" attribute.				     */
    if ((Pl = IrtExtC2SCreatePolyFromCrv(ErrorMsg, CrvList, 
					 IRT_C2S_MAX_DIST(SizeRectangle), 
					 AngularDeviation)) == NULL) {
	*ErrorMsg = "Crv2RectRgns: Conversion to polylines failed.";
	return NULL;
    }

    /* Refine boundary polylines so each edge is at most twice SizeRectangle.*/
    /* If the curve is farther than MaxDist from the polyline, the polylines */
    /* will be further refined as well.					     */
    IrtExtC2SLimitSplitEdgeLen(Pl, SizeRectangle * 2.0, MaxDist);

    /* Tile the interior of the loop with rectangles or triangles of at most */
    /* twice SizeRectangle.						     */
    if ((Pl = IrtExtC2SSplitLimitEdgeLenToRectangles(Pl, SizeRectangle * 2.0))
						       		     == NULL) {
	*ErrorMsg = "Crv2RectRgns: Edge size limit split failed.";
	return NULL;
    }

    /* Converts all rectangles/triangles in Pl to four/three rectangles of   */
    /* half the size.							     */
    Pl = IrtExtC2SConvertPolysToRectangles(Pl); 

    Os = IPGenPOLYObject(Pl);

    /* Apply smoothing if so desired. */
    if (NumSmoothingSteps > 0)
	GMPolyMeshSmoothing(Os, NumSmoothingSteps); 

    /* Compute unique ID so neighboring rectangles will have the same ID on  */
    /* their shared edge.  Vertex V defining edge (V, V -> Pnext) will have  */
    /* unique ID 'IrtExtC2SUniqueEdgeID'. 				     */
    IrtExtC2SSetEdgeUniqueId(Os);

    switch (OutputType) {
	default:
	case 0:	    
	    /* Note this function uses Pl in place. */
	    ResultObject = IrtExtC2SArrangePolygon(Pl, Name);
	    break;
	case 1:
	    ResultObject = IrtExtC2SCnvPolyToFF(Pl, FALSE, Name);
	    IPFreePolygonList(Pl); 
	    break;
	case 2:
	    ResultObject = IrtExtC2SCnvPolyToFF(Pl, TRUE, Name);
	    IPFreePolygonList(Pl); 
	    break;
    }

    Os -> U.Pl = NULL;
    IPFreeObject(Os);

    return ResultObject;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculates the coordinates of the point that its relative position 	     *
* is Crv(Alpha * End + (1-Alpha) * Start), in arc-length measure.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   The curve we taking the points from.				     *
*   Point: The coordinate of the point in the middle.			     *
*   Start: The parameter of the start of the line.			     *
*   End:   The parameter of the end of the line.			     *
*   Alpha: Relative position in the curve of the point we are return.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   The coordinates of the point in the middle of the curve.     *
*****************************************************************************/
static IrtRType IrtExtC2SRelativePoint(const CagdCrvStruct *Crv, 
				       CagdPType Point, 
				       IrtRType Start, 
				       IrtRType End, 
				       IrtRType Alpha)
{
    int i; 
    CagdPType Point1, Point2; 
    IrtRType
	Val = 0.0, 
	Len = 0.0, 
	Step = (End - Start) / IRT_C2S_NUM_STEPS, 
	circLen = IrtExtC2SCurveLen(Crv, Start, End); 

    Alpha = IRIT_BOUND(Alpha, 0.0, 1.0); 
 
    IrtExtC2SEvalCrvPt(Crv, Point1, Start); 

    for (i = 1; Len < circLen * Alpha; i++) {
	IrtExtC2SEvalCrvPt(Crv, Point2, Start + i * Step); 
	Val = IRIT_PT2D_DIST(Point1, Point2); 
	Len += Val; 
	if (Len < circLen * Alpha)
	    IRIT_PT_COPY(Point1, Point2); 
    }

    if (Len > (Val + circLen) * Alpha) {
	IRIT_PT_COPY(Point, Point1); 
	return Start + (i - 2) * Step; 
    }
    else {
	IRIT_PT_COPY(Point, Point2); 
	return Start + (i - 1) * Step; 
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluate the XY location of the curve at parameter t.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   The curve we evaluate.					     *
*   Point: Evaluated location of curve at parameter t will be saved here.    *
*   t:     The parameter of the curve to evaluate at.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   True if successful, false otherwise.				     *
*****************************************************************************/
static int IrtExtC2SEvalCrvPt(const CagdCrvStruct *Crv, 
			       CagdPType Point, 
			       CagdRType t)
{
    CagdRType
	*R = CagdCrvEval(Crv, t); 

    CagdCoerceToE2(Point, &R, -1, Crv -> PType); 
    Point[2] = 0; 

    return TRUE; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check if an XY planar polygon is clockwise or counter-clockwise when     *
* looking from  +Z.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   VS: Cyclic List of vertex of the Polygon.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: positive if the polygon go clockwise, negative opposite 	     *
*   and zero If the polygon's projection on XY plane is a degenerate.	     *
*****************************************************************************/
static IrtRType IrtExtC2SFindZDirection(const IPVertexStruct *VS)
{
    const IPVertexStruct
        *V = VS; 
    IrtRType 
	Sum = 0.0; 

    do {
	IrtPtType C1, C2; 

	IRIT_PT2D_SUB(C1, V -> Coord, VS -> Coord); 
	IRIT_PT2D_SUB(C2, V -> Pnext -> Coord, VS -> Coord); 
	Sum += IRIT_CROSS_PROD_2D(C1, C2); 

	V = V -> Pnext; 
    }
    while (V != VS); 

    return Sum; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculates the length of the curve between the parameters Start to End,  *
* in arc-length measure.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   The curve we compute the length for.				     *
*   Start: The parameter of the start of the curve.			     *
*   End:   The parameter of the end of the curve.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:  The length of Crv from parameter Start to End.		     *
*****************************************************************************/
static IrtRType IrtExtC2SCurveLen(const CagdCrvStruct *Crv, 
				  IrtRType Start, 
				  IrtRType End)
{
    int i; 
    CagdPType Point1, Point2; 
    IrtRType
	Len = 0.0, 
	Step = (End - Start) / IRT_C2S_NUM_STEPS; 
    
    IrtExtC2SEvalCrvPt(Crv, Point1, Start); 
    
    for (i = 1; i <= IRT_C2S_NUM_STEPS; i++) {
	IrtExtC2SEvalCrvPt(Crv, Point2, Start + i * Step); 
	Len += IRIT_PT2D_DIST(Point1, Point2); 
	IRIT_PT2D_COPY(Point1, Point2); 
    }

    return Len; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculates the max distance of the curve between parameters	Start to     *
* End, to the line between these end points.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   The curve we compute the distance for.			     *
*   Start: The parameter of the start of the curve.			     *
*   End:   The parameter of the end of the curve.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType:   The max distance of Crv from line parameter Start to End.    *
*****************************************************************************/
static IrtRType IrtExtC2SDistLineCurve(const CagdCrvStruct *Crv, 
				       IrtRType Start, 
				       IrtRType End)
{
    int i; 
    CagdPType PointS, Point, PointE, Vector; 
    IrtRType
	MaxDist = 0.0, 
	Step = (End - Start) / IRT_C2S_NUM_STEPS; 
    
    IrtExtC2SEvalCrvPt(Crv, PointS, Start);
    IrtExtC2SEvalCrvPt(Crv, PointE, End); 
    IRIT_PT_SUB(Vector, PointE, PointS);

    for (i = 1; i < IRT_C2S_NUM_STEPS; i++) {
	IrtRType Des;

	IrtExtC2SEvalCrvPt(Crv, Point, Start + i * Step); 
	Des = GMDistPointLine(Point, PointS, Vector); 
	MaxDist = IRIT_MAX(MaxDist, Des);
    }

    return MaxDist; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns the freeform curve that is associated with the edge		     *
* (V, V -> Pnext) and returns the curves domain Start/End, or NULL if not    *
* exists.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         The edge that holds the curve.				     *
*   Start:     The parameter of the starting location along the curve will   *
*              be updated here.						     *
*   End:       The parameter of the end location along the curve will be     *
*              updated here.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct: The freeform curve that is associated with the edge	     *
*		    (V, V -> Pnext) or NULL if not found.		     *
*****************************************************************************/
static IPObjectStruct *IrtExtC2SGetCrv(const IPVertexStruct *V, 
				       IrtRType *Start, 
				       IrtRType *End)
{
    IPObjectStruct *Res;  

    Res = AttrGetObjAttrib(V -> Attr, "IrtExtC2SCurve"); 

    if (Res != NULL)
	CagdCrvDomain(Res -> U.Crvs, Start, End);

    return Res; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Returns a new vertex in 						     *
* (Alpha * V -> Coord +	(1-Alpha) * V -> Pnext -> Coord) of edge 	     *
* (V, V -> Pnext) if C is NULL.						     *
*   Otherwise (C != NULL), returns a new vertex in 			     *
* C(Alpha * End + (1-Alpha) * Start), in arc-length measure.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:     The vertex we will put the new vertex to.			     *
*   C:     The curve we taking the points from.				     *
*   Start: The parameter of the start of the line.			     *
*   End:   The parameter of the end of the line.			     *
*   Alpha: The Relation between the new vertex coordinate and the edge ends. *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:  The new vertex.					     *
*****************************************************************************/
static IPVertexStruct *IrtExtC2SBreakEdgeMiddle(IPVertexStruct *V, 
						IPObjectStruct *C, 
						IrtRType Start, 
						IrtRType End, 
						IrtRType Alpha)
{
    IPVertexStruct *NewV; 

    NewV = IPAllocVertex2(V -> Pnext); 
    IRIT_VEC_COPY(NewV -> Normal, V -> Normal);

    V -> Pnext = NewV; 
    if (C != NULL) {
        IrtRType Mid; 
	IPObjectStruct *Crv1, *Crv2;

	Mid = IrtExtC2SRelativePoint(C -> U.Crvs, V -> Pnext -> Coord,
				      Start, End, Alpha); 
	Crv1 = IPGenCRVObject(CagdCrvRegionFromCrv(C -> U.Crvs, Start, Mid));
	Crv2 = IPGenCRVObject(CagdCrvRegionFromCrv(C -> U.Crvs, Mid, End));
	AttrSetObjAttrib(&V  -> Attr, "IrtExtC2SCurve", Crv1, FALSE); 
	AttrSetObjAttrib(&V -> Pnext -> Attr, "IrtExtC2SCurve", Crv2, FALSE); 
    } 
    else {
	AttrFreeOneAttribute(&NewV -> Attr, "IrtExtC2SCurve");
	IRIT_PT_BLEND(NewV -> Coord, V -> Coord, 
		      NewV -> Pnext -> Coord,  Alpha); 
    }
    
    return NewV; 
} 

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Duplicated Vertex V and put the new vertex after V.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         The vertex we duplicated.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:  The new vertex.					     *
*****************************************************************************/
static IPVertexStruct *IrtExtC2SDuplicateVertexAfter(IPVertexStruct *V)
{
    IPVertexStruct *NewV; 
    
    NewV = IPAllocVertex2(V -> Pnext);
    IRIT_VEC_COPY(NewV -> Normal, V -> Normal); 
    IRIT_PT_COPY(NewV -> Coord, V -> Coord); 
    NewV -> Attr = IP_ATTR_COPY_ATTRS(V -> Attr);
    
    V -> Pnext = NewV; 

    return NewV; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Examine if there exist a vertex that is not an end-point vertex of the   *
* line/curve it currently on, vertex that was added as part of refinement.   *
*                                                                            *
* PARAMETERS:                                                                *
*   V1:  Cyclic List of vertex.	If a vertex that is not an end-point is	     *
*        found, V1 is set to the vertex just before it.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if a refined vertex was found, FALSE otherwise.		     *
*****************************************************************************/
static int IrtExtC2SHasRefinedPoint(IPVertexStruct **V1)
{
    IPVertexStruct
	*V = *V1;

    do {
	if (!IrtExtC2SIsEndVert(V)) {
	    *V1 = V;
	    return TRUE;
	}
	V = V -> Pnext;
    }
    while (V != *V1);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a set of general planar polygons, Pls, each defining a closed      *
* region, returns a new list of only rectangles tiling the same domain,      *
* by selecting a centroid location in each input polygon and connecting that *
* centroid location to all the middle of all edges.  An input n-gon will be  *
* divided into n rectangles.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pls:       Polygonal object to split into rectangles, in place.          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: Return list of polygon after spliting.		     *
*****************************************************************************/
static IPPolygonStruct *IrtExtC2SConvertPolysToRectangles(IPPolygonStruct
							                *Pls)
{
    IPPolygonStruct *Pl, 
	*ResPl = NULL; 
    int IsCirc = IPSetPolyListCirc(FALSE); 

    IPSetPolyListCirc(IsCirc); 	     /* Restore state, now that we know it. */
    
    do {
	int Len; 
	IrtPtType Mid; 
	IPVertexStruct *V, *VN, *VNN, *VNNN, *V1; 

	IRIT_LIST_POP(Pl, Pls);
	V1 = Pl -> PVertex; 
	Len = IPVrtxListLen(V1);

	if (Len > 2) {
	    IRIT_PT_RESET(Mid);

	    /* Compute a centroid point. */
	    if (!GMFindPtInsidePolyKernel(V1, Mid))
		GMComputeAverageVertex(V1, Mid, NULL, 0);

	    /*  Insert two vertices in the middle of every edge of Pl.       */
	    V = V1;
	    VN = V1 -> Pnext;
	    do {
	        IPObjectStruct *C; 
		IrtRType Start, End; 

		C = IrtExtC2SGetCrv(V, &Start, &End); 

		/* Insert a new vertex between V and V -> Pnext. */
		IrtExtC2SBreakEdgeMiddle(V, C, Start, End, 0.5);

		/* Duplicated the middle point between V and V -> Pnext. */
		IrtExtC2SDuplicateVertexAfter(V -> Pnext); 
		AttrFreeOneAttribute(&V -> Pnext -> Attr, "IrtExtC2SCurve");

		V = VN;
		VN = VN -> Pnext;
	    }
	    while (V != NULL && V != V1);
 
	    /*   At this point every original edge has two identical         */
	    /* interior vertex.  We march on every 2nd such interior         */
	    /* vertex using V and extract rectangular domains.               */
	    /*   Add the centroid of the polygon with three boundary         */
	    /* vertices, forming a rectangle.				     */
	    for (V = V1 -> Pnext -> Pnext; 
		 V -> Pnext != V1; 
		 V = VNNN) {
		IPPolygonStruct *NewPl;

		VNN = V -> Pnext -> Pnext; 
		VNNN = VNN -> Pnext; 
		VNN -> Pnext = IPAllocVertex2(V);/* Make rectangle circular. */

		IRIT_PT_COPY(VNN -> Pnext -> Coord, Mid); 

		NewPl = IPAllocPolygon(Pl -> Tags, V, NULL); 
		IRIT_PLANE_COPY(NewPl -> Plane, Pl -> Plane); 
		IP_SET_PLANE_POLY(NewPl); 

		NewPl -> Attr = IP_ATTR_COPY_ATTRS(Pl -> Attr); 
		IP_RST_BBOX_POLY(NewPl); 
		IRIT_LIST_PUSH(NewPl, ResPl);
	    }
	    V -> Pnext -> Pnext -> Pnext = IPAllocVertex2(V); 
	    IRIT_PT_COPY(V -> Pnext -> Pnext -> Pnext -> Coord, Mid); 
	    IRIT_LIST_PUSH(Pl, ResPl);
	}
    }
    while (Pls != NULL);

    return ResPl;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Splits the input polygon at all edges with length larger than MaxLen.    *
* The output will have no edge longer than MaxLen, and no edge will have its *
* curve further away than MaxDist.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:	        Input polygon.  Must have original precise curves associated *
*               with each edge (V, V -> next) as "IrtExtC2SCurve" attrib.   *
*   MaxLen:	Maximum allowed length of an edge in the polygon.	     *
*   MaxDist:	Maximum allowed distance of an edge from its curve.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void IrtExtC2SLimitSplitEdgeLen(IPPolygonStruct *Pl, 
				       IrtRType MaxLen, 
				       IrtRType MaxDist)
{
    IPVertexStruct *V, *VN, 
	*V1 = Pl -> PVertex; 
    int Begin = TRUE; 

    for (V = V1, VN = V1 -> Pnext; 
         (V != NULL && V != V1) || Begin; 
         VN = V -> Pnext) {
        IrtRType Start, End; 
        IPObjectStruct *C; 

	C = IrtExtC2SGetCrv(V, &Start, &End); 
	assert(C != NULL);

        if (IrtExtC2SCurveLen(C -> U.Crvs, Start, End) > MaxLen || 
	    IrtExtC2SDistLineCurve(C -> U.Crvs, Start, End) > MaxDist)  {
	    /* Break edge (V, V -> Pnext) in the middle (and its curve), in */
	    /* place.							    */
            IrtExtC2SBreakEdgeMiddle(V, C, Start, End, 0.5);
	}
	else {
            V = V -> Pnext; 
            Begin = FALSE; 
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find vertex that is not in the end of an edge or have concave angle,     *
* and that is after V and before VEnd including VEnd.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:      The start of the list of vertices to search.		     *
*   VEnd:   The end of the list of vertices to search.			     *
*   Side:   positive if the polygon go clockwise, negative otherwise.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *: Vertex just before found vertex, or NULL if list empty.*
*****************************************************************************/
static IPVertexStruct *IrtExtC2SFindNextRefinedVertex(IPVertexStruct *V, 
						      IPVertexStruct *VEnd,
						      IrtRType Side)
{
    if (V == VEnd) 
	return NULL;

    for ( ; V != VEnd; V = V -> Pnext) {
	if (!IrtExtC2SIsEndVert(V) || 
	     IrtExtC2SPerpendicular(V, V -> Pnext -> Pnext, TRUE) * Side 
								< IRIT_UEPS)
	    return V; 
    }

    return VEnd; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Split a polygon to two new polygons by the edge (VS, VE).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:  The polygon we will Split.					     *
*   VS:  The first vertex we will Split by.  VS is in Pl.		     *
*   VE:  The second vertex we will Split by.  VE is in Pl.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void IrtExtC2SBreakToTwo(IPPolygonStruct *Pl, 
				IPVertexStruct *VS, 
				IPVertexStruct *VE)
{
    IPVertexStruct 
        *VSPrev = IPGetLastVrtx(VS),             /* Get pevious vertices... */
	*VEPrev = IPGetLastVrtx(VE), 
	*NewVS = IPCopyVertex(VS), 
	*NewVE = IPCopyVertex(VE);

    AttrFreeOneAttribute(&NewVS -> Attr, "IrtExtC2SCurve");
    AttrFreeOneAttribute(&NewVE -> Attr, "IrtExtC2SCurve");

    IRIT_PT_COPY(NewVS -> Coord, VS -> Coord); 
    IRIT_PT_COPY(NewVE -> Coord, VE -> Coord); 

    NewVE -> Pnext = VS; 
    NewVS -> Pnext = VE; 

    VSPrev -> Pnext =  NewVS;
    VEPrev -> Pnext =  NewVE;

    Pl -> PVertex = NewVS;
    Pl -> Pnext = IPAllocPolygon(Pl -> Tags, NewVE, Pl -> Pnext); 

    IRIT_PLANE_COPY(Pl -> Pnext -> Plane, Pl -> Plane); 
    IP_SET_PLANE_POLY(Pl -> Pnext); 

    Pl -> Pnext -> Attr = IP_ATTR_COPY_ATTRS(Pl -> Attr); 

    IP_RST_BBOX_POLY(Pl -> Pnext); 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Calculates the cross product of the normalized perpendicular between     *
* vectors (V1, V1 -> Pnext) and (V2, V1) if IsForward or (V2, V1 -> Pnext)   *
* otherwise.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   V1, V2:	The edge to examine orthogonality to polygon at V1.	     *
*   IsForward:  True if we take the edge (V2, V1) and false if we need	     *
*		(V2, V1 -> Pnext).					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType: The cross product of the normalized perpendicular	between      *
*             V to V2.							     *
*****************************************************************************/
static IrtRType IrtExtC2SPerpendicular(const IPVertexStruct *V1, 
				       const IPVertexStruct *V2,
				       int IsForward)
{
    IrtPtType X, Y; 

    IRIT_PT2D_SUB(X, V1 -> Coord, V1 -> Pnext -> Coord); 

    if (IsForward) {
	IRIT_PT2D_SUB(Y, V2 -> Coord, V1 -> Coord); 
    }
    else {
	IRIT_PT2D_SUB(Y, V2 -> Coord, V1 -> Pnext -> Coord); 
    }

    IRIT_PT2D_SAFE_NORMALIZE(X); 
    IRIT_PT2D_SAFE_NORMALIZE(Y); 

    return IRIT_CROSS_PROD_2D(X, Y); 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Query if vertex V->Pnext is an end-point vertex of the line/curve we     *
* are currently on.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         Vertex to examine if a terminating vertex.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if an end-point vertex, FALSE otherwise.		     *
*****************************************************************************/
static int IrtExtC2SIsEndVert(IPVertexStruct *V)
{
    IrtRType R;
    IrtPtType X, Y;
    IPVertexStruct *VNN,
	*VN = V -> Pnext;

    if (AttrGetIntAttrib(VN -> Attr, "IrtExtC2SOriginalVertex") == TRUE)
	return TRUE;
    
    if (AttrGetObjAttrib(V -> Attr, "IrtExtC2SCurve") != NULL &&
        AttrGetObjAttrib(VN -> Attr, "IrtExtC2SCurve") != NULL)
	return FALSE;

    VNN =  VN -> Pnext;
    IRIT_PT_SUB(X, V -> Coord, VN -> Coord); 
    IRIT_PT_SUB(Y, VN -> Coord, VNN -> Coord); 

    R = IRIT_CROSS_PROD_2D(X, Y);
    return !IRIT_APX_EQ(R, 0.0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Break edge (V1, V1 -> Pnext) until all the edges are smaller than MaxLen.*
*                                                                            *
* PARAMETERS:                                                                *
*   V1:        The Polygon we want to break.				     *
*   MaxLen:    The max size of the edge we want.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void IrtExtC2SBreakEdge(IPVertexStruct *V1, IrtRType MaxLen)
{
    IPVertexStruct *V = V1, 
	*VEnd = V1 -> Pnext; 
    
    while (V != VEnd) {
	IrtRType Dis; 
	Dis = IRIT_PT_PT_DIST(V -> Coord, V -> Pnext -> Coord); 
	if (Dis > MaxLen) {
	    IPVertexStruct *NewV; 

	    NewV = IPAllocVertex2(V -> Pnext);
	    AttrFreeOneAttribute(&NewV -> Attr, "IrtExtC2SCurve");

	    NewV -> Normal[0] = 0.0; 
	    NewV -> Normal[1] = 0.0; 
	    NewV -> Normal[2] = 1.0; 

	    IRIT_PT_BLEND(NewV -> Coord, V -> Coord, 
			  NewV -> Pnext -> Coord,  0.5); 
	    V -> Pnext = NewV; 
	}
	else 
	    V = V -> Pnext; 
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find a matching vertex VN to given vertex VE, as a new edge (VN, VE).    *
*   Returns the vertex that is most perpendicular to the polygon at VE.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   VE:	    The vertex to examine and also the cyclic list of vertices to    *
*	    select from.						     *
*   Side:   Positive if the polygon go clockwise, negative otherwise.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   Matched vertex.					     *
*****************************************************************************/
static IPVertexStruct *IrtExtC2SFindMatchVertex(IPVertexStruct *VE,
						IrtRType Side)
{
    IrtRType
	MaxOrtho = 0.0,
	MaxLen = 0.0;
    IPVertexStruct
	*Res = NULL, 
	*V = VE -> Pnext, 
	*VN = V -> Pnext; 

    while (!IRT_C2S_2ND_IS_1ST_EDGE_ENDPT(VN, VE)) {
        if (IrtExtC2SUpdateEdgeMorePerpendicular(V, VE, &MaxOrtho, &MaxLen,
						  Side)) {
	    Res = VN;
	}

	V = VN; 
        VN = V -> Pnext; 
    }

    return Res; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks if a two vertices VB1 -> Pnext and VB2 -> Pnext, so edge	     *
*   (VB1 -> Pnext, VB2 -> Pnext) is the edge that the more perpendicular one *
*   to the polygon at VB1 -> Pnext and VB2 -> Pnext then the previous	     *
*   results. If the tow edges are optimal then longest edge is returned.     *
*                                                                            *
* PARAMETERS:                                                                *
*   VB1, VB2:  The edge defined as (VB1 -> Pnext, VB2 -> Pnext) to test.     *
*   MaxOrtho:  Maximal orthogonality value we have so far.		     *
*              Will be updated if new edge is more perpendicular.	     *
*   MaxLen:    Maximal edge length we have so far.			     *
*              Will be updated if new edge is more perpendicular.	     *
*   Side:      Positive if the polygon go clockwise, negative otherwise.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if more perpendicular, FALSE otherwise.			     *
*****************************************************************************/
static int IrtExtC2SUpdateEdgeMorePerpendicular(IPVertexStruct *VB1, 
						IPVertexStruct *VB2, 
						IrtRType *MaxOrtho, 
						IrtRType *MaxLen, 
						IrtRType Side)
{
    IrtRType Len, OrthoAfter1, OrthoAfter2, OrthoBefore1, OrthoBefore2, 
	     Ortho1, Ortho2;
    IPVertexStruct 
	*V1 = VB1 -> Pnext, 
	*V2 = VB2 -> Pnext;

    /* Compute orthogonality values at the two end points. */
    OrthoBefore1 = IrtExtC2SPerpendicular(VB1, V2, FALSE); 
    OrthoBefore2 = IrtExtC2SPerpendicular(VB2, V1, FALSE);

    OrthoAfter1 = IrtExtC2SPerpendicular(V1, V2, TRUE); 
    OrthoAfter2 = IrtExtC2SPerpendicular(V2, V1, TRUE);

    Ortho1 = IrtExtC2SPerpendicular(VB1, V1 -> Pnext, TRUE);
    Ortho2 = IrtExtC2SPerpendicular(VB2, V2 -> Pnext, TRUE);

    Len = IRIT_PT_PT_DIST_SQR(V1 -> Coord, V2 -> Coord);
    /* Check if edge is oriented, legal and intersection free. */
    if (!IRT_C2S_2ND_IS_1ST_EDGE_ENDPT(V1, V2) && 
	!IRT_C2S_2ND_IS_1ST_EDGE_ENDPT(V2, V1) && 
	 IrtExtC2SOptOrAnd(Ortho1  * Side < -IRIT_UEPS,
			     OrthoAfter1 * Side > -IRIT_UEPS,
			     OrthoBefore1 * Side > -IRIT_UEPS) &&
	 IrtExtC2SOptOrAnd(Ortho2  * Side < -IRIT_UEPS,
			     OrthoAfter2 * Side > -IRIT_UEPS,
			     OrthoBefore2 * Side > -IRIT_UEPS) &&
	!IRIT_APX_EQ(Len, 0.0) &&
	!GMIsInterLinePolygon(V1, V1 -> Coord, V2 -> Coord)) {
	IrtRType ValOrtho;

	OrthoAfter1 = IRIT_ABS(OrthoAfter1); 
	OrthoAfter2 = IRIT_ABS(OrthoAfter2);
	OrthoBefore1 = IRIT_ABS(OrthoBefore1);
	OrthoBefore2 = IRIT_ABS(OrthoBefore2);

	ValOrtho = IRIT_MAX(OrthoAfter1, OrthoBefore1) +
	           IRIT_MAX(OrthoAfter2, OrthoBefore2);

	/* Check if the edge is more perpendicular that we have             */
	/* so far, or if equal that is longer.		                    */
	if (*MaxOrtho + IRIT_EPS < ValOrtho ||
	    (IRIT_APX_EQ(*MaxOrtho, ValOrtho) &&
	     Len > *MaxLen)) {
	    *MaxOrtho = ValOrtho; 
	    *MaxLen = Len;
	    return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   If Operator return or of B1, B2 else return and of B1, B2		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Operator:  Operator to perform.					     *
*   B1, B2:    The parameters of the operator.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:  TRUE if the Operator return TRUE with B1, B2, FALSE otherwise.     *
*****************************************************************************/
static int IrtExtC2SOptOrAnd(int Operator, int B1, int B2)
{
    if (Operator)
	return B1 || B2;
    else
	return B1 && B2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find a two vertices VS and VE, so edge (VS, VE) is the edge that the     *
* most perpendicular one to the polygon.				     *
*   If more then one optimal edge exist in orthogonality measure, the        *
* longest edge is returned.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   VList:   Cyclic list of vertex to select from.			     *
*   VS, VE:  The edge we return.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE if found edge, FALSE otherwise.				     *
*****************************************************************************/
static int IrtExtC2SFindSplitEdge(IPVertexStruct *VList, 
				  IPVertexStruct **VS, 
				  IPVertexStruct **VE)
{
    IrtRType Side, 
	MaxOrtho = 0.0,
	MaxLen = 0.0; 
    IPVertexStruct  *V1, *V2,
	*V = VList;
    int Len = IPVrtxListLen(V);

    *VE = NULL; 
    *VS = NULL; 

    Side = -IrtExtC2SFindZDirection(V);	    /* Compute orientation. */

    if (IrtExtC2SHasRefinedPoint(&V)) {
	V1 = V;

	/*   Traverse every possible interior edge to add between two       */
	/* existing vertices in VList.		        		    */
	do  {
	    for (V2 = IrtExtC2SFindNextRefinedVertex(V1 -> Pnext -> Pnext, 
						      V -> Pnext, Side); 
		 V2 != NULL; 
		 V2 = IrtExtC2SFindNextRefinedVertex(V2 -> Pnext, V -> Pnext, 
						      Side)) {

	        if (IrtExtC2SUpdateEdgeMorePerpendicular(V1, V2, &MaxOrtho, 
							  &MaxLen, Side)) {
		    *VS = V1 -> Pnext;
		    *VE = V2 -> Pnext;
		}
	    }

	    V1 = IrtExtC2SFindNextRefinedVertex(V1 -> Pnext -> Pnext, 
						 V -> Pnext, Side);
	}
	while (V1 != V && V1 != NULL);

	if (*VS == NULL) {
	    *VS = V -> Pnext;
	    /* IrtExtC2SFindMatchVertex expects the prev. vertex to V. */
	    *VE = IrtExtC2SFindMatchVertex(V, Side); 
	}
	return *VE != NULL;
    }
    else if (Len > 4) {
	/* Find an edge from VS to VE that is completely inside polygon. */
	V1 = V;
	do {
	    *VE = IrtExtC2SFindMatchVertex(V1, Side);
	    V1 = V1 -> Pnext;
	}
	while (*VE == NULL && V != V1);
	*VS = V1;

	return *VE != NULL;
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Splits all polygons that has edge length larger than MaxLen while all    *
* interior regions are converted to rectangles or triangles.		     *
*   The output will have no edges in no polygons with length larger than     *
*  MaxLen.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pls:	List of polygons, divided in-place.			     *
*   MaxLen:	Maximum allowed length of an edge of a rectangle.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *: Return list of polygon after Spliting.		     *
*****************************************************************************/
static IPPolygonStruct *IrtExtC2SSplitLimitEdgeLenToRectangles(
							 IPPolygonStruct *Pls,
							 IrtRType MaxLen)
{
    IPPolygonStruct *Pl,
	*NewPls = NULL;

    while (Pls != NULL) {
	int Len;
	IPVertexStruct *V1,
	    *VS = NULL, 
	    *VE = NULL;

	IRIT_LIST_POP(Pl, Pls);

	V1 = Pl -> PVertex; 
	Len = IPVrtxListLen(V1);

	if (Len > 2 && GMPolyOnePolyArea(Pl) > IRIT_EPS) {
	    if (!IrtExtC2SFindSplitEdge(V1, &VS, &VE) ||
		 IRT_C2S_2ND_IS_1ST_EDGE_ENDPT(VE, VS) || 
		 IRT_C2S_2ND_IS_1ST_EDGE_ENDPT(VS, VE)) {
		/* This polygon is small enough - skip it. */
		IRIT_LIST_PUSH(Pl, NewPls); 
	    }
	    else {
    		IrtExtC2SBreakToTwo(Pl, VE, VS);
		IrtExtC2SBreakEdge(Pl -> PVertex, MaxLen); 
		IrtExtC2SBreakEdge(Pl -> Pnext -> PVertex, MaxLen); 
		IRIT_LIST_PUSH(Pl -> Pnext, Pls);
		IRIT_LIST_PUSH(Pl, Pls);
	    }
	}
	else { 
	    IPFreePolygon(Pl);
	}
    }

    return NewPls;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts mesh with rectilinear regions to object lists of loops 	     *
*   of four curves per region. save on every curve a UniqueId in	     *
*   attribute "IrtExtC2SUniqueEdgeId" so every two Identity edges have      *
*   the same UniqueId.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:        A mesh with rectilinear regions only.			     *
*   GenSrfs:   TRUE to generate surfaces, FALSE for list of 4-curves.	     *
*   Name:      The name of the objects.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  A new allocated list of four curves' regions.	     *
*****************************************************************************/
static IPObjectStruct *IrtExtC2SCnvPolyToFF(const IPPolygonStruct *Pl,
					    int GenSrfs,
					    const char *Name)
{
    IPObjectStruct *FFListObj, 
        *Res = IPGenListObject(Name, NULL, NULL); 
    int j = 1;
    char SubName[IRIT_LINE_LEN_LONG];

    for ( ; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *V = Pl -> PVertex; 
	CagdCrvStruct
	    *CrvList = NULL;

	do {
	    IPObjectStruct *Crv; 
	    IrtRType T1, T2; 
	    CagdCrvStruct *CTmp;
	    IrtPtType P1, P2;
	    
	    Crv = IrtExtC2SGetCrv(V, &T1, &T2); 
	    if (Crv != NULL) {
		IrtExtC2SEvalCrvPt(Crv -> U.Crvs, P1, T1);
		IrtExtC2SEvalCrvPt(Crv -> U.Crvs, P2, T2);
	    }

	    /*   A shared edge that connects interior island and outside     */
	    /* boundary might move, possibly due to smoothing.		     */
	    if (Crv == NULL || 
	       !IRIT_PT_APX_EQ(P1, V -> Coord) || 
	       !IRIT_PT_APX_EQ(P2, V -> Pnext -> Coord)) {
		/* The edge can be a line. */
		CagdPtStruct Pt1, Pt2; 
 
		IRIT_PT_COPY(Pt1.Pt, V -> Coord); 
		IRIT_PT_COPY(Pt2.Pt, V -> Pnext -> Coord); 
		CTmp = CagdMergePtPt(&Pt1, &Pt2); 
		IRIT_LIST_PUSH(CTmp, CrvList);
	    }
	    else {
		/* The edge is a boundary curve. */
		if (GenSrfs)
		    CTmp = CagdCoerceCrvTo(Crv -> U.Crvs, 
					   CAGD_PT_E2_TYPE, FALSE); 
		else
		    CTmp = CagdCrvCopy(Crv -> U.Crvs);
		IRIT_LIST_PUSH(CTmp, CrvList); 
	    }

	    AttrSetIntAttrib(&CTmp -> Attr, "IrtExtC2SUniqueEdgeId", 
			     AttrGetIntAttrib(V -> Attr,
					      "IrtExtC2SUniqueEdgeId"));

	    V = V -> Pnext; 
	}
	while (V != Pl -> PVertex);

	FFListObj = NULL;
	if (GenSrfs) {
	    CagdCrvStruct 
		*CrvList2 = IrtExtC2SOrderCurves(CrvList, TRUE); 
	    CagdSrfStruct *Srf;

	    CagdCrvFreeList(CrvList);
	    assert(CagdListLength(CrvList2) == 4);

	    Srf = CagdBoolSumSrf(CrvList2, 
				 CrvList2 -> Pnext -> Pnext, 
				 CrvList2 -> Pnext -> Pnext -> Pnext, 
				 CrvList2 -> Pnext);

	    CrvList2 -> Pnext -> Pnext -> Pnext -> Pnext = NULL;
	    CagdCrvFreeList(CrvList2);

	    if (Srf != NULL) {
		sprintf(SubName, "%s_Srf%d", Name, j++);
		FFListObj = IPGenSrfObject(SubName, Srf, NULL);
	    }
	}
	else {
	    sprintf(SubName, "%s_Crv%d", Name, j++);
	    FFListObj = IPGenCrvObject(SubName, CrvList, NULL);
	}

	if (FFListObj != NULL)
	    IPListObjectAppend(Res, FFListObj);
    }

    return Res; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Adds to edge (V, V -> Pnext) attribute information "IrtExtC2SCurve"     *
* about curve Crv.							     *
*   Also tags the vertex as "IrtExtC2SOriginalVertex" - original vertex.    *
*                                                                            *
* PARAMETERS:                                                                *
*   V:		    The vertex to update.				     *
*   Crv:	    The curve to add, possibly reversed.		     *
*   IsReverse:      TRUE to reverse, FALSE to add as is.		     *
*   Point:	    Coordinate of the vertex after V.			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void IrtExtC2SSetCrv(IPVertexStruct *V, 
			    const CagdCrvStruct *Crv,
			    int IsReverse,
			    const IrtPtType Point)
{
    IPObjectStruct *Obj;
    CagdCrvStruct
	*Crv2 = IsReverse ? CagdCrvReverse(Crv) : CagdCrvCopy(Crv);

    Crv2 -> Pnext = NULL;
    Obj = IPGenCRVObject(Crv2);
    AttrSetObjAttrib(&V -> Attr, "IrtExtC2SCurve", Obj, FALSE);
    AttrSetIntAttrib(&V -> Attr, "IrtExtC2SOriginalVertex", TRUE);

    if (Point != NULL) {
	V -> Normal[0] = 0.0; 
	V -> Normal[1] = 0.0; 
	V -> Normal[2] = 1.0; 
	IRIT_PT_COPY(V -> Coord, Point); 
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Split every curve in CrvList that has C1 discontinuities, at the         *
* discontinuity points.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvList:           The curve we split at C1 discontinuities.	     *
*   AngularDeviation:  Threshold of angular deviation to subdivide, in       *
*                      degrees.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *: The modified CrvList, in place.			     *
*****************************************************************************/
static CagdCrvStruct *IrtExtC2SSplitDiscont(CagdCrvStruct *CrvList,
					    CagdRType AngularDeviation)
{
    CagdRType
	CosAngularDeviation = cos(IRIT_DEG2RAD(AngularDeviation));
    CagdCrvStruct *Crv,
	*NewCrvList = NULL;

    do {
	int i, NumC1Knots;
	CagdRType *C1Knots;

	IRIT_LIST_POP(Crv, CrvList);

	if (CAGD_IS_BEZIER_CRV(Crv) ||
	    (C1Knots = BspKnotAllC1Discont(Crv -> KnotVector, Crv -> Order,
					   Crv -> Length,
					   &NumC1Knots)) == NULL) {
	    IRIT_LIST_PUSH(Crv, NewCrvList);
	    continue;
	}

	for (i = 0; i < NumC1Knots && Crv != NULL; i++) {
	    CagdVecStruct Tan1, Tan2;

	    /* Divide Crv if at parameter C1Knots[i] the potential C1       */
	    /* discontinuity is real.					    */
	    Tan1 = *CagdCrvTangent(Crv, C1Knots[i] - IRIT_EPS, TRUE);
	    Tan2 = *CagdCrvTangent(Crv, C1Knots[i] + IRIT_EPS, TRUE);

	    if (IRIT_DOT_PROD(Tan1.Vec, Tan2.Vec) < CosAngularDeviation) {
		/* Subdivide the curve. */
		CagdCrvStruct
		    *TCrv = BspCrvSubdivAtParam(Crv, C1Knots[i]);

		CagdCrvFree(Crv);
		Crv = TCrv -> Pnext;
		TCrv -> Pnext = NULL;
		IRIT_LIST_PUSH(TCrv, NewCrvList);
	    }
	}
	if (Crv != NULL) {
	    IRIT_LIST_PUSH(Crv, NewCrvList);
	}

	IritFree(C1Knots);
    }
    while (CrvList != NULL);

    NewCrvList = CagdListReverse(NewCrvList);

    return NewCrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given OrgCrvList, create a close polygon from the end points of the      *
* curves and save on every vertex V of edge (V, V -> Pnext) an attribute     *
* IrtExtC2SCurve" with a copy of the curve.			             *
*   Assume the length of CrvList is larger than two.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   ErrorMsg:    The message to return in case of an error.		     *
*   OrgCrvList:  The input curves.					     *
*   MaxDist:     The largest distance to consider two points as the same.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   The created polygon, or NULL if error.		     *
*****************************************************************************/
static IPVertexStruct *IrtExtC2SCreatePolyFromLongCrvList(
					    const char **ErrorMsg, 
					    const CagdCrvStruct *OrgCrvList,  
					    IrtRType MaxDist)
{
    CagdPType Start, End, Start2, End2, Next; 
    CagdRType TMin, TMax, TMin2, TMax2; 
    CagdCrvStruct *Curves, *CrvList; 
    IPVertexStruct *V1, *VNext; 
    int i,
	Len = CagdListLength(OrgCrvList); 

    assert(Len > 2);

    CrvList = CagdCrvCopyList(OrgCrvList);
    
    /* Handle the first curve. */ 
    IRIT_LIST_POP(Curves, CrvList); 
    CagdCrvDomain(Curves, &TMin, &TMax); 
    IrtExtC2SEvalCrvPt(Curves, Start, TMin); 
    IrtExtC2SEvalCrvPt(Curves, End, TMax); 

    CagdCrvDomain(CrvList, &TMin2, &TMax2); 
    IrtExtC2SEvalCrvPt(CrvList, Start2, TMin2); 
    IrtExtC2SEvalCrvPt(CrvList, End2, TMax2);  

    V1 = IPAllocVertex2(NULL); 

    if (IRIT_PT_APX_EQ_EPS(Start, Start2, MaxDist) ||
        IRIT_PT_APX_EQ_EPS(Start, End2, MaxDist)) {
	IrtExtC2SSetCrv(V1, Curves, TRUE, End); 
	IRIT_PT_COPY(Next, Start);
    } 
    else if (IRIT_PT_APX_EQ_EPS(End, Start2, MaxDist) ||
	     IRIT_PT_APX_EQ_EPS(End, End2, MaxDist)) {
	IrtExtC2SSetCrv(V1, Curves, FALSE, Start);
	IRIT_PT_COPY(Next, End);
    } 
    else {
        *ErrorMsg = "Crv2RectRgns: Curve List not a loop."; 
	IPFreeVertexList(V1);
        return NULL; 
    }
    VNext = V1;

    CagdCrvFree(Curves);

    /* Handle the rest of the curves. */ 
    for (i = 1; i < Len; i++) {
	IPVertexStruct *VNew;
	IRIT_LIST_POP(Curves, CrvList); 
        CagdCrvDomain(Curves, &TMin, &TMax); 
        IrtExtC2SEvalCrvPt(Curves, Start, TMin); 
        IrtExtC2SEvalCrvPt(Curves, End, TMax); 
        VNew = IPAllocVertex2(NULL); 
        VNext -> Pnext = VNew; 

        if (IRIT_PT_APX_EQ_EPS(Start, Next, MaxDist)) {
	    IrtExtC2SSetCrv(VNew, Curves, FALSE, Start); 
	    IRIT_PT_COPY(Next, End);
	} 
	else if (IRIT_PT_APX_EQ_EPS(End, Next, MaxDist)) {
	    IrtExtC2SSetCrv(VNew, Curves, TRUE, End); 
	    IRIT_PT_COPY(Next, Start);
	}
	else {
	    *ErrorMsg = "Crv2RectRgns: Curve List is not a loop."; 
	    IPFreeVertexList(V1);
	    return NULL; 
	}
	CagdCrvFree(Curves);
	VNext = VNew; 
    }

    /* Handle the last curve. */ 
    VNext -> Pnext = V1; 

    if (!IRIT_PT_APX_EQ_EPS(Next, V1 -> Coord, MaxDist)) {
	*ErrorMsg = "Crv2RectRgns: Curve List is not a loop."; 
	IPFreeVertexList(V1);
	return NULL; 
    }

    return V1; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Make from OrgCrvList a polygon using the curves end points and save on   *
* every vertex V of edge (V, V -> Pnext) an attribute "IrtExtC2SCurve"       *
* with a copy of the curve.					             *
*                                                                            *
* PARAMETERS:                                                                *
*   ErrorMsg:		The message to return in case of error.		     *
*   OrgCrvList:		The input curves.  These curves might be changed     *
*                       in-place to make them consistent input for the rest  *
*                       of the rectangular tiling algorithm.		     *
*   MaxDist:		The maximum distance that we consider two end	     *
*			points as similar.				     *
*   AngularDeviation:   Threshold of angular deviation to subdivide the      *
*			curve at, in degrees.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPPolygonStruct *:   The created polygon.				     *
*****************************************************************************/
static IPPolygonStruct *IrtExtC2SCreatePolyFromCrv(const char **ErrorMsg, 
						   CagdCrvStruct **OrgCrvList,
						   IrtRType MaxDist,
						   CagdRType AngularDeviation)
{
    IPVertexStruct *V; 
    IPPolygonStruct *Pl; 
    CagdCrvStruct *CrvList2, *Curve2, 
	*CrvList = *OrgCrvList; 
    IrtRType T1, T2, T3, T3B, T4, T5, Start1, Start2, End1, End2; 
    IrtPtType Tmp, S1, S2, E1, E2; 
    int Len; 
    CagdPtStruct P1, P2;

    /* Split the input curves at C1 discontinuities of more than	     */
    /* AngularDeviation angles, in degrees,				     */
    CrvList = IrtExtC2SSplitDiscont(CrvList, AngularDeviation);

    Len = CagdListLength(CrvList); 
    switch (Len) {
	case 2:
	    /* Get the two curves' end points. */
	    Curve2 = CrvList -> Pnext;
	    CagdCrvDomain(CrvList, &Start1, &End1); 
	    IrtExtC2SEvalCrvPt(CrvList, S1, Start1); 
	    IrtExtC2SEvalCrvPt(CrvList, E1, End1); 

	    CagdCrvDomain(Curve2, &Start2, &End2); 
	    IrtExtC2SEvalCrvPt(Curve2, S2, Start2); 
	    IrtExtC2SEvalCrvPt(Curve2, E2, End2);

	    if (IRIT_PT_APX_EQ_EPS(S1, S2, MaxDist) &&
		IRIT_PT_APX_EQ_EPS(E1, E2, MaxDist)) {
		CagdCrvStruct
		    *TCrv = CagdCrvReverse(CrvList -> Pnext);

		CagdCrvFree(CrvList -> Pnext);
		CrvList -> Pnext = TCrv;

		IRIT_PT_SWAP(S2, E2);
	    }

	    /* We have two curves that match starting and end locations. */
	    if (IRIT_PT_APX_EQ_EPS(S1, E2, MaxDist) ||
		IRIT_PT_APX_EQ_EPS(E1, S2, MaxDist)) {
		/* Split the two curves to two parts each, in the middle.   */
		/* Note the TI's are ordered along the loop.		    */
		T1 = Start1;
		T3 = End1;
		T3B = Start2;
		T5 = End2;
		T2 = IrtExtC2SRelativePoint(CrvList, Tmp, T1, T3, 0.5); 
		T4 = IrtExtC2SRelativePoint(CrvList -> Pnext, 
					    Tmp, T3B, T5, 0.5); 
		CrvList2 = CagdCrvRegionFromCrv(CrvList, T1, T2); 
		CrvList2 -> Pnext = CagdCrvRegionFromCrv(CrvList, T2, T3); 
		CrvList2 -> Pnext -> Pnext = 
			      CagdCrvRegionFromCrv(CrvList -> Pnext, T3B, T4);
		CrvList2 -> Pnext -> Pnext -> Pnext = 
			       CagdCrvRegionFromCrv(CrvList -> Pnext, T4, T5);
		CagdCrvFreeList(CrvList);
		CrvList = CrvList2;
	    }
	    else {
		IRIT_PT_COPY(P1.Pt, E1);
		IRIT_PT_COPY(P2.Pt, S2);
		CrvList -> Pnext = CagdMergePtPt(&P1, &P2);
		CrvList -> Pnext -> Pnext = Curve2;
		IRIT_PT_COPY(P1.Pt, E2);
		IRIT_PT_COPY(P2.Pt, S1);
		Curve2 -> Pnext = CagdMergePtPt(&P1, &P2);
	    }
	    break;
	case 1:  /* Split the curve to 4 parts. */
	    CagdCrvDomain(CrvList, &T1, &T5); 
	    T2 = IrtExtC2SRelativePoint(CrvList, Tmp, T1, T5, 0.25); 
	    T3 = IrtExtC2SRelativePoint(CrvList, Tmp, T1, T5, 0.5); 
	    T4 = IrtExtC2SRelativePoint(CrvList, Tmp, T1, T5, 0.75); 
	    CrvList2 =  CagdCrvRegionFromCrv(CrvList, T1, T2); 
	    CrvList2 -> Pnext = CagdCrvRegionFromCrv(CrvList, T2, T3); 
	    CrvList2 -> Pnext -> Pnext = CagdCrvRegionFromCrv(CrvList, T3, T4);
	    CrvList2 -> Pnext -> Pnext -> Pnext = 
				        CagdCrvRegionFromCrv(CrvList, T4, T5);
	    CagdCrvFreeList(CrvList);
	    CrvList = CrvList2;
	    break;
	default:
	    break;
    }

    V = IrtExtC2SCreatePolyFromLongCrvList(ErrorMsg, CrvList, MaxDist); 

    if (V == NULL) {	  	/* If failed - try to reorder the curves. */ 
	CrvList2 = IrtExtC2SOrderCurves(CrvList, FALSE);
	CagdCrvFreeList(CrvList);
	CrvList = CrvList2;

	/* Try again after the reorder. */
	V = IrtExtC2SCreatePolyFromLongCrvList(ErrorMsg, CrvList, MaxDist);
	if (V == NULL)
	    return NULL;
    }

    *OrgCrvList = CrvList;

    Pl = IPAllocPolygon(0, V, NULL); 
    IRIT_PT4D_RESET(Pl -> Plane); 
    Pl -> Plane[2] = 1.0; 
    return Pl; 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prepare the output is a format of rectangular polygons:		     *
* Set all vertex normals to be the plane normals and convert to object       *
* list of polygons.		  					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pls:   A mesh to arrange.						     *
*   Name:  The name of the root object.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  List object holding the polygons.		     *
*****************************************************************************/
static IPObjectStruct *IrtExtC2SArrangePolygon(IPPolygonStruct *Pls,
					       const char *Name)
{
    IPObjectStruct *PolygonObj,
	*Res = IPGenListObject(Name, NULL, NULL); 
    IPPolygonStruct *Pl;
    char SubName[IRIT_LINE_LEN_LONG];
    int i = 1;

    while (Pls != NULL) {
	IPVertexStruct *V;

	IRIT_LIST_POP(Pl, Pls);
	V = Pl -> PVertex;

	/* Set all vertex normals to be the plane normals. */
	do {
	    IRIT_VEC_COPY(V -> Normal, Pl -> Plane);
	    IP_SET_NORMAL_VRTX(V);

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);

	sprintf(SubName, "%s_Pl%d", Name, i++);
	PolygonObj = IPGenPolyObject(SubName, Pl, NULL);
	IPListObjectAppend(Res, PolygonObj);
    }

    return Res;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Set in all edges a UniqueId so every two Identical edges will have the   *
* same UniqueId. The new information is saved as attribute		     *
* "IrtExtC2SUniqueEdgeId". 						     *
*                                                                            *
* PARAMETERS:                                                                *
*   PolyObj:   A mesh to set the UniqueId in.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void IrtExtC2SSetEdgeUniqueId(IPObjectStruct *PolyObj)
{
    IPPolyVrtxIdxStruct
	*PVIdx = IPCnvPolyToPolyVrtxIdxStruct(PolyObj, TRUE, 0); 
    int UniqueId = 1;  
    IPPolygonStruct
	*Pl = PVIdx -> PObj -> U.Pl;

    while (Pl != NULL) {
	IPVertexStruct 
	    *V = Pl -> PVertex;
	
	do {
	    if (AttrGetIntAttrib(V -> Attr, "IrtExtC2SUniqueEdgeId") 
							== IP_ATTR_BAD_INT) {
		IPVertexStruct *OtherV;

		AttrSetIntAttrib(&V -> Attr, "IrtExtC2SUniqueEdgeId", 
								    UniqueId);
		OtherV = GMFindAdjacentEdge(PVIdx, 
		    IRIT_ABS(AttrGetIntAttrib(V -> Attr, "_VIdx")) - 1, 
		    IRIT_ABS(AttrGetIntAttrib(V -> Pnext -> Attr, "_VIdx")) - 1);
		if (OtherV != NULL)
		    AttrSetIntAttrib(&OtherV -> Attr, 
				     "IrtExtC2SUniqueEdgeId", UniqueId);
		UniqueId++;
	    }
	    
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);

	Pl = Pl -> Pnext;
    }

    /*	Euler theorem: the number of edges plus one equals to the number of */
    /* vertices plus the number of polygons, If all polygons are connected  */
    /* and on the same plane.						    */
    /*   Assumes no holes in the synthesizedtiling.			    */
    assert(UniqueId == PVIdx -> NumPlys + PVIdx -> NumVrtcs);

    IPPolyVrtxIdxFree(PVIdx); 
}
