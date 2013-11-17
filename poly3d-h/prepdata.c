/*****************************************************************************
*   Routines to	prepare objects according to view file matrix:		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan. 1989   *
*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include "program.h"
#include "misc_lib.h"

/* #define DEBUG1			      Print edge/hash table content. */

#define SAME_VERTEX(V1, V2)	(IRIT_APX_EQ(V1 -> Coord[0], V2 -> Coord[0]) && \
				 IRIT_APX_EQ(V1 -> Coord[1], V2 -> Coord[1]) && \
				 IRIT_APX_EQ(V1 -> Coord[2], V2 -> Coord[2]))

IRIT_STATIC_DATA int MinYLevel, MaxYLevel, CrntYLevel, PrintYLevel;

#ifdef DEBUG1
static void PrintEdgeContent(EdgeStruct *PEdge);
static void DrawEdgeHashTable(void);
static void DrawEdge(EdgeStruct *PEdge);
#endif /* DEBUG1 */

static void PrepareOneObject(IPObjectStruct *PObject);
static int PrepareOnePolygon(IPPolygonStruct *PPolygon,
			     int IsPolygon,
			     IrtHmgnMatType ViewMat);
static void UpdateBBoxPolygon(IPPolygonStruct *PPolygon);
static int UpdateEqnPolygon(IPPolygonStruct *PPolygon, int *ReversePoly);
static void GenEdgesFromPoly(IPPolygonStruct *PPolygon, int IsPolygon);
static void InsertEdgeToHashTbl1(EdgeStruct *PEdge);
static void IntersectAllEdges(void);
static void InsertEdgeToHashTbl2(EdgeStruct *PEdge);
static int IntersectEdgeList(EdgeStruct *PEdge,
			     EdgeStruct *PEList,
			     int TestYMin);
static int IntersectEdgeEdge(EdgeStruct *PEdge1,
			     EdgeStruct *PEdge2,
			     EdgeStruct **PEdgeNew1,
			     EdgeStruct **PEdgeNew2);
static void PrintPolyContent(IPPolygonStruct *PPoly);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to prepare the given object for hidden surface analysis.	     M
*   If NumOfObjects == 0 then all the objects defined by the data sturcture  M
* are drawn.								     M
*   If GlblNumEdge != 0 then only GlblNumEdge first edges of each polygons   M
* are tested for visibility (usefull in case in input polygons has known     M
* repetition edges sequence which is redundent).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObjects:     To prepare for hidden surface analysis.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PrepareViewData                                                          M
*****************************************************************************/
void PrepareViewData(IPObjectStruct *PObjects)
{
    int	i;

    IritCPUTime(TRUE);

    for	(i = 0; i < EDGE_HASH_TABLE_SIZE; i++)
	EdgeHashTable[i] = NULL;
    for	(i = 0; i < POLY_HASH_TABLE_SIZE; i++)
	PolyHashTable[i] = NULL;

    if (!GlblQuiet)
	fprintf(stderr, "\nPass 2, Edges =      ");

    while (PObjects) {
	if (IP_IS_POLY_OBJ(PObjects))
	    PrepareOneObject(PObjects);
	PObjects = PObjects -> Pnext;
    }

    if (!GlblQuiet)
	fprintf(stderr, ",  %6.2f seconds.", IritCPUTime(FALSE));

    IntersectAllEdges();       /* Break edges to visibily uniform sub-edges. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to prepare one object PObject for hidden surface analysis.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PObject:    Object to prepare for freeform analysis.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrepareOneObject(IPObjectStruct *PObject)
{
    int Level,
	Visible = TRUE;
    IPPolygonStruct *Ptemp,
	*PList = PObject -> U.Pl;
    IrtHmgnMatType ViewMat;
    IPObjectStruct *MatObj;

    if ((MatObj = AttrGetObjectObjAttrib(PObject, "_animation_mat")) != NULL &&
	IP_IS_MAT_OBJ(MatObj)) {
	IrtRType
	    RVisible = AttrGetObjectRealAttrib(PObject, "_isvisible");

	if (!IP_ATTR_IS_BAD_REAL(RVisible))
	    Visible = RVisible > 0.0;

	if (Visible)
	    MatMultTwo4by4(ViewMat, *MatObj -> U.Mat, GlblViewMat);
    }
    else
        IRIT_GEN_COPY(ViewMat, GlblViewMat, sizeof(IrtHmgnMatType));

    if (ATTR_OBJ_IS_INVISIBLE(PObject) || !Visible)
	return;

    while (PList) {
	Ptemp = PList -> Pnext;

	if (!GMIsConvexPolygon2(PList)) {
	    IRIT_STATIC_DATA int Printed = FALSE;

	    if (!Printed) {
		IRIT_WARNING_MSG(
			"\nWARNING: Non convex polygon(s) might be in data (see CONVEX in IRIT),\n\t\t\t\toutput can be wrong as the result!\n     ");
		Printed = TRUE;
	    }
	}

	if (PrepareOnePolygon(PList, IP_IS_POLYGON_OBJ(PObject), ViewMat)) {
	    /* And add polygon into polygon hash table sorted by Ymin: */
	    Level = (int) ((PList -> BBox[0][1] + 1.0) * POLY_HASH_TABLE_SIZE2);
	    Level = IRIT_BOUND(Level, 0, POLY_HASH_TABLE_SIZE1); /* Be safe. */
	    PList -> Pnext = PolyHashTable[Level];
	    PolyHashTable[Level] = PList;  /* Concat. it to poly hash table. */
	}

	PList =	Ptemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to prepare one poly PPolygon.					     *
*   Returns TRUE iff this object is a valid POLYGON (not a POLYLINE!).	     *
*									     *
* PARAMETERS:                                                                *
*   PPolygon:     Poly to prepare for hideden surface analysis.              *
*   IsPolygon:    TRUE if polygon, FALSE otherwise (polyline etc.).          *
*   ViewMat:	  To transform the geometry.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:          TRUE if successful, FALSE otherwise.                       *
*****************************************************************************/
static int PrepareOnePolygon(IPPolygonStruct *PPolygon,
			     int IsPolygon,
			     IrtHmgnMatType ViewMat)
{
    int	i, Reverse;
    IrtRType CpCoord[3];
    IPVertexStruct
	*PList = PPolygon -> PVertex;

    /* Make a circular polygon list into a linear list by breaking the end.  */
    while (PList != NULL && PList -> Pnext != PPolygon -> PVertex)
	PList = PList -> Pnext;
    if (PList)
	PList -> Pnext = NULL;
    PList = PPolygon -> PVertex;

    while (PList) {
	/* Convert the coordinate to screen space (in IrtRType pres.). */
	MatMultPtby4by4(CpCoord, PList -> Coord, ViewMat);
	for (i = 0; i < 3; i++)
	    PList -> Coord[i] = CpCoord[i];

	PList =	PList -> Pnext;
    }
    if (IsPolygon) {
	/* Find plane equation of poly, and let know if need to reverse. */
	if (!UpdateEqnPolygon(PPolygon, &Reverse))
	    return FALSE;
	UpdateBBoxPolygon(PPolygon);/* Find X, Y extrem in screen space. */
	GenEdgesFromPoly(PPolygon, TRUE);	  /* Generate all its edges. */
	if (Reverse)
    	    IPReverseVrtxList(PPolygon);
	return TRUE;
    }
    else {
	GenEdgesFromPoly(PPolygon, FALSE);	  /* Generate all its edges. */
	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to update polygon boundary box in screen space.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PPolygon:     To update bounding box for, in palce.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateBBoxPolygon(IPPolygonStruct *PPolygon)
{
    IrtRType *Coord, Xmin, Xmax, Ymin, Ymax; /* Bounding box of the polygon. */
    IPVertexStruct
	*PList = PPolygon -> PVertex;

    Xmin = Xmax	= PList -> Coord[0];
    Ymin = Ymax	= PList	-> Coord[1];
    PList = PList -> Pnext;
    while (PList) {
	Coord = PList -> Coord;
	if (Coord[0] > Xmax)
	    Xmax = Coord[0];
	if (Coord[0] < Xmin)
	    Xmin = Coord[0];
	if (Coord[1] > Ymax)
	    Ymax = Coord[1];
	if (Coord[1] < Ymin)
	    Ymin = Coord[1];
	PList =	PList -> Pnext;
    }
    PPolygon ->	BBox[0][0] = Xmin;
    PPolygon -> BBox[1][0] = Xmax;
    PPolygon ->	BBox[0][1] = Ymin;
    PPolygon -> BBox[1][1] = Ymax;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to update plane equation of the given	polygon.		     *
*   It is assumed that at least 3 points in polygon exists. The tuple that   *
* has the largest vector length is selected maximum accuracy.		     *
*   We IGNORE PLANE if was in data file.				     *
*   In addition, a test is made if all vertices are ordered such that the    *
* cross product of each 3 consecutive vertices (projected onto Z=0 plane)    *
* is allways positive. The polygon must be convex, so result might be either *
* all positive or all negative.						     *
*   In the later case the order of the plane equation is reversed.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PPolygon:       Polygon to update plane for, and possibly reverse.       *
*   ReversedPoly:   TRUE if plane equation was reversed.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:            TRUE if successful, FALSE otherwise.                     *
*****************************************************************************/
static int UpdateEqnPolygon(IPPolygonStruct *PPolygon, int *ReversedPoly)
{
    IRIT_STATIC_DATA int
	PolygonCount = 0;
    int	i;
    IrtRType V1[3], V2[3], *Coord, *CoordNext, *CoordNextNext,
	Plane[3], Len, MaxPlane[3],
	MaxLen = 0.0;
    IPVertexStruct
	*PList = PPolygon -> PVertex;

    PolygonCount++;

    *ReversedPoly = FALSE;

    do {       /* Search for 3 consequtive non-collinear point from polygon: */
	Coord = PList -> Coord;
	CoordNext = PList -> Pnext -> Coord;
	CoordNextNext = PList -> Pnext -> Pnext -> Coord;
	for (i = 0; i < 3; i++) {   /* Prepare two vectors on polygon plane. */
	    V1[i] = Coord[i] - CoordNext[i];
	    V2[i] = CoordNext[i] - CoordNextNext[i];
	}

	/* Find plane normal by a cross product of the two vectors on plane: */
	Plane[0] = V1[1] * V2[2] - V1[2] * V2[1];
	Plane[1] = V1[2] * V2[0] - V1[0] * V2[2];
	Plane[2] = V1[0] * V2[1] - V1[1] * V2[0];

	/* Find vector Len. - we are looking for the biggest: */
	Len = sqrt(IRIT_SQR(Plane[0]) +
		   IRIT_SQR(Plane[1]) +
		   IRIT_SQR(Plane[2]));
	if (Len > MaxLen) {
	    for (i = 0; i < 3; i++)
		MaxPlane[i] = Plane[i];
	    MaxLen = Len;
	}
	PList = PList -> Pnext;				  /* Try next tuple. */
    }
    while (PList -> Pnext -> Pnext != NULL);

    if (IRIT_FABS(MaxLen) < IRIT_SQR(IRIT_EPS)) {
	/* Fail to find 3 non-collinear pts. */
	if (GlblMore) {
	    IRIT_WARNING_MSG_PRINTF(
	        "\nError: Invalid polygon (%d) found in file (zero edge length/collinear vertices):\n",
		PolygonCount);
	    PrintPolyContent(PPolygon);
	}
	return FALSE;
    }

    for (i = 0; i < 3; i++)
	PPolygon -> Plane[i] = MaxPlane[i] / MaxLen;

    /* Make sure the Z component of the	plane is positive: */
    if (PPolygon -> Plane[2] < 0.0) {
	for (i = 0; i < 3; i++)
	    PPolygon -> Plane[i] = (-PPolygon -> Plane[i]);
	*ReversedPoly = TRUE;
    }
    else
        if (GlblBackFacing)
	    return FALSE;

    PPolygon ->	Plane[3] =
	(- Coord[0] * PPolygon -> Plane[0]
	 - Coord[1] * PPolygon -> Plane[1]
	 - Coord[2] * PPolygon -> Plane[2]);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to generate all the edges from the given polygon in screen space.  *
*   Edges are inserted to an edge hash table of EDGE_HASH_TABLE_SIZE	     *
* entries.								     *
*   If global variable GlblNumEdge != 0 then only the first PGlblNumEdge     *
* edges are generated.							     *
*   If edge is INTERNAL it is marked as so.				     *
*   If this is polyline, the last edge is NOT generated.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PPolygon:    To generateedge from.                                       *
*   IsPolygon:   TRUE for polygon, FALSE otherwise (polyline etc.).          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GenEdgesFromPoly(IPPolygonStruct *PPolygon, int IsPolygon)
{
    int	CountEdges = GlblNumEdge;
    EdgeStruct *PEdge;
    IPVertexStruct
	*PList = PPolygon -> PVertex;

    if (!PList || !PList -> Pnext)
	return;					 /* If less than 2 vertices. */

    while (PList -> Pnext) {
	PEdge =	(EdgeStruct *) IritMalloc(sizeof(EdgeStruct));
	PEdge -> Vertex[0] = PList;
	PEdge -> Vertex[1] = PList -> Pnext;
	PEdge -> Internal = IP_IS_INTERNAL_VRTX(PList);
	PEdge -> Pnext = NULL;
	InsertEdgeToHashTbl1(PEdge);

	if (!--CountEdges)
	    return;

	PList =	PList -> Pnext;
    }
    /* Close the contour to first vertex in list (if not polyline): */
    if (IsPolygon) {
	PEdge = (EdgeStruct *) IritMalloc(sizeof(EdgeStruct));
	PEdge -> Vertex[0] = PList;
	PEdge -> Vertex[1] = PPolygon -> PVertex;
	PEdge -> Internal = IP_IS_INTERNAL_VRTX(PList);
	PEdge -> Pnext = NULL;
	InsertEdgeToHashTbl1(PEdge);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to insert new	edge to	edge hash table	structure sorted (hashed) by *
* the edge Y min value.							     *
*   The edge is tested for duplicated entry (if interior edge it is entered  *
* twice from its two adjacent polygons).				     *
*   The edge is updated such that Ymin will be Vertex[0], Ymax Vertex[1].    *
*                                                                            *
* PARAMETERS:                                                                *
*   PEdge:      Edge to insert into hash table.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertEdgeToHashTbl1(EdgeStruct *PEdge)
{
    int	Level;
    IrtRType Xmin, Xmax, Ymin, Ymax;
    EdgeStruct *PEtemp;

    if (PEdge -> Vertex[0] -> Coord[1] > PEdge -> Vertex[1] -> Coord[1]) {
        IRIT_SWAP(IPVertexStruct *, PEdge -> Vertex[0], PEdge -> Vertex[1]);
    }

#if defined(__WINNT__) && defined(NDEBUG)
#   if _MSC_VER >= 1400  /* Visual 8, 2005 */
        /* A bug in MS VC++ 8.0 optimization version creates wrong code     */
	/* unless we call this dummy function.  Strange indeed...	    */
        IritSleep(0);
#   endif /* _MSC_VER >= 1400 */
#endif /*  __WINNT__ && NDEBUG */

    Xmin = IRIT_MIN(PEdge -> Vertex[0] -> Coord[0],
	       PEdge -> Vertex[1] -> Coord[0]);
    Xmax = IRIT_MAX(PEdge -> Vertex[0] -> Coord[0],
	       PEdge -> Vertex[1] -> Coord[0]);
    Ymin = PEdge -> Vertex[0] -> Coord[1];
    Ymax = PEdge -> Vertex[1] -> Coord[1];

    assert(Ymin <= Ymax);

    if (GlblClipScreen && ((Ymin > 1.0) || (Ymax < -1.0) ||
			   (Xmin > 1.0) || (Xmax < -1.0))) {
	IritFree(PEdge);			   /* Out of screen. */
    }
    else {
	/* Normalize [-1..1] to	[0..EDGE_HASH_TABLE_SIZE]: */
	Level =	(int) ((Ymin + 1.0) * EDGE_HASH_TABLE_SIZE2);
	Level =	IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1);/* Be 100% safe. */

	/* Look	for duplicate entry - it must have the same two	vertices: */
	PEtemp = EdgeHashTable[Level];
	while (PEtemp) {
	    /* Test to see if same edge	by comparing vertices pointers.	*/
	    if ((SAME_VERTEX(PEdge -> Vertex[0], PEtemp -> Vertex[0]) &&
		 SAME_VERTEX(PEdge -> Vertex[1], PEtemp -> Vertex[1])) ||
		(SAME_VERTEX(PEdge -> Vertex[0], PEtemp -> Vertex[1]) &&
		 SAME_VERTEX(PEdge -> Vertex[1], PEtemp -> Vertex[0]))) {
		IritFree(PEdge);
		return;					/* Ignore new entry! */
	    }
	    PEtemp = PEtemp -> Pnext;
	}

	if (!GlblQuiet)
	    fprintf(stderr, "\b\b\b\b\b%5d", ++EdgeCount);
	PEdge -> Pnext = EdgeHashTable[Level];	     /* Concat to main list. */
	EdgeHashTable[Level] = PEdge;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to collect all edges in hash table into one big list and intersect *
* them beween themselves. 						     *
*   The resulting edges are inserted back into the hash table.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IntersectAllEdges(void)
{
    IrtRType Ymin;
    int	i, Level;
    EdgeStruct *PEtemp,
	*PEmain = NULL;

    IritCPUTime(TRUE);

    EdgeCount =	0;
    MinYLevel =	EDGE_HASH_TABLE_SIZE; /* Set "clip" levels in table entries. */
    MaxYLevel =	0;

    /* Clear the hash table and	collect	all edges into one big list: */
    for	(i = EDGE_HASH_TABLE_SIZE1; i >= 0; i--)
	if ((PEtemp = EdgeHashTable[i]) != NULL) {
	    while (PEtemp -> Pnext)
		PEtemp = PEtemp -> Pnext;
	    PEtemp -> Pnext = PEmain;
	    PEmain = EdgeHashTable[i];
	    EdgeHashTable[i] = NULL;
	    if (i > MaxYLevel)
		MaxYLevel = i;
	    if (i < MinYLevel)
		MinYLevel = i;
	}

    PrintYLevel	= CrntYLevel = 0;	 /* Have to start from some place... */
    if (!GlblQuiet)
	fprintf(stderr, "\nPass 3, Level [%5d] =      ", MaxYLevel);

    while (PEmain) { /* Insert back after intersecting with all other edges. */
	PEtemp = PEmain	-> Pnext;      /* As PEmain->Pnext might be changed. */
	InsertEdgeToHashTbl2(PEmain);
	PEmain = PEtemp;

	/* Now test to see if we can update current y level: */
	if (CrntYLevel < MaxYLevel && PEmain != NULL) {
	    Ymin = IRIT_MIN(PEmain -> Vertex[0] -> Coord[1],
		       PEmain -> Vertex[1] -> Coord[1]);

	    /* Normalize [-1..1] to [0..EDGE_HASH_TABLE_SIZE]: */
	    Level = (int) ((Ymin + 1.0) * EDGE_HASH_TABLE_SIZE2);
	    Level = IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1);/* Be safe. */

	    if (Level > CrntYLevel)
		CrntYLevel = Level;
	}
    }

    if (!GlblQuiet)
	fprintf(stderr, ",  %6.2f seconds.", IritCPUTime(FALSE));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to insert old	edge to	edge hash table	structure sorted (hashed) by *
* the edge Y min value.	The edge is tested for intersections with other	     *
* edges	allready in structure and both edges are broken	if found one.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PEdge:     To back insert after intersection testing.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertEdgeToHashTbl2(EdgeStruct *PEdge)
{
    int	i, Level, UpperLevel,
	FoundIntersection = FALSE;
    IrtRType Ymin, Ymax;
    EdgeStruct *PEtemp;

    Ymin = PEdge -> Vertex[0] -> Coord[1];
    Ymax = PEdge -> Vertex[1] -> Coord[1];

    assert(Ymin <= Ymax);

    /* Normalize [-1..1] to [0..EDGE_HASH_TABLE_SIZE]: */
    Level = (int) ((Ymin + 1.0)	* EDGE_HASH_TABLE_SIZE2);
    Level = IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1); /* To be 100% safe. */
    UpperLevel = 1 + (int) ((Ymax + 1.0) * EDGE_HASH_TABLE_SIZE2);
    UpperLevel = IRIT_BOUND(UpperLevel, 0, EDGE_HASH_TABLE_SIZE1);

    if (CrntYLevel > PrintYLevel) {
	PrintYLevel = CrntYLevel;
	if (!GlblQuiet)
	    fprintf(stderr, "\b\b\b\b\b%5d", PrintYLevel);
    }

    /* Test for	intersections while we find intersections... */
    for	(i = MinYLevel; i <= UpperLevel; i++)
	if (EdgeHashTable[i] &&
	    ((FoundIntersection = IntersectEdgeList(PEdge, EdgeHashTable[i],
						    i == MinYLevel)) != 0))
	    break;

    if (FoundIntersection) {	   /* Call recursively with the edge pieces: */
	while (PEdge) {
	    PEtemp = PEdge -> Pnext;  /* As Pedge->Pnext might point to new. */
	    InsertEdgeToHashTbl2(PEdge);   /* Place after the recursive ins. */
	    PEdge = PEtemp;
	}
    }
    else {		      /* Its a single edge - insert it in its place: */
	EdgeCount++;
	PEdge -> Pnext = EdgeHashTable[Level];	     /* Concat to main list. */
	EdgeHashTable[Level] = PEdge;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to scan all edges in list and	intersect everything against	     *
* the given edge. Intersected edges are	broken into two.		     *
*   The broken edge is updated into a list of teo edges.		     *
*   This routine returns TRUE after the first intersection found, no test is *
* made for ALL intersections if	more than one exists.			     *
*   A test is made if MinYLevel can be updated if TestYMin == TRUE.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   PEdge:      Edge to examine for intersection.                            *
*   PEList:     List of edges to examine for intersectiong with PEdge.       *
*   TestYMin:   Index Y level.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        TRUE if found intersection, FALSE otherwise.                 *
*****************************************************************************/
static int IntersectEdgeList(EdgeStruct *PEdge,
			     EdgeStruct *PEList,
			     int TestYMin)
{
    int	Level,
	UpdateYMin = TRUE;
    IrtRType Ymin, Ymax;
    EdgeStruct *PEdgeNew, *PEListNew;

    if (!PEdge || !PEList)
	return FALSE;				       /* NULL entry - quit. */

    while (PEList) {
	if (IntersectEdgeEdge(PEdge, PEList, &PEdgeNew,	&PEListNew)) {
	    PEdge -> Pnext = PEdgeNew;
	    /* PEListNew can be	inserted to the	hash table with	no check as  */
	    /* it cannot intersect anything - it is part of checked edge!    */
	    if (PEListNew) {
		Ymin = PEListNew -> Vertex[0] -> Coord[1];
		/* Normalize [-1..1] to	[0..EDGE_HASH_TABLE_SIZE]: */
		Level =	(int) ((Ymin + 1.0) * EDGE_HASH_TABLE_SIZE2);
		Level =	IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1);
		EdgeCount++;
		PEListNew -> Pnext = EdgeHashTable[Level];
		EdgeHashTable[Level] = PEListNew;
	    }
	    return TRUE;
	}
	if (TestYMin &&	UpdateYMin) {
	    Ymax = PEList -> Vertex[1] -> Coord[1];
	    /* Normalize [-1..1] to [0..EDGE_HASH_TABLE_SIZE]: */
	    Level = (int) ((Ymax + 1.0)	* EDGE_HASH_TABLE_SIZE2);
	    Level = IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1);
	    if (Level >= CrntYLevel)
		UpdateYMin = FALSE;
	}
	PEList = PEList	-> Pnext;
    }
    if (TestYMin && UpdateYMin)			/* No need to test any more. */
	do
	    MinYLevel++;
	while (MinYLevel < CrntYLevel && !EdgeHashTable[MinYLevel]);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test if two edges intersects. If they do, it brakes the bottom  *
* edge into two pieces, leaving the lower part (with the same Ymin) in	     *
* original struct and allocates and updates new struct with upper edge part. *
*   Returns TRUE if found intersection, FALSE otherwise.		     *
*   The intersection is tested in the XY axes (Z is ignored).		     *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   PEdge1, PEdge2:  Two edge to intersect against each other.               *
*   PEdgeNew1:       If PEdge1 is broken into two.                           *
*   PEdgeNew2:       If PEdge2 is broken into two.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:             TRUE if found intersection, FALSE otherwise.            *
*****************************************************************************/
static int IntersectEdgeEdge(EdgeStruct *PEdge1,
			     EdgeStruct *PEdge2,
			     EdgeStruct **PEdgeNew1,
			     EdgeStruct **PEdgeNew2)
{
    int	i, OneInter1, OneInter2;
    IrtRType Xmin1, Xmax1, Ymin1, Ymax1, Xmin2, Xmax2, Ymin2, Ymax2,
	  a1, b11, b12, a2, b21, b22, det, t1, t2, Z1, Z2;
    /* To speed	up the intensive access	of the coordinates: */
    IrtRType
	*Crd10 = PEdge1 -> Vertex[0] -> Coord,
	*Crd11 = PEdge1 -> Vertex[1] -> Coord,
	*Crd20 = PEdge2 -> Vertex[0] -> Coord,
	*Crd21 = PEdge2 -> Vertex[1] -> Coord;

    Xmin1 = IRIT_MIN(Crd10[0], Crd11[0]);
    Xmax1 = IRIT_MAX(Crd10[0], Crd11[0]);
    Ymin1 = Crd10[1];
    Ymax1 = Crd11[1];

    Xmin2 = IRIT_MIN(Crd20[0], Crd21[0]);
    Xmax2 = IRIT_MAX(Crd20[0], Crd21[0]);
    Ymin2 = Crd20[1];
    Ymax2 = Crd21[1];
    if ((Xmin1 > Xmax2)	|| (Xmax1 < Xmin2) ||/* Test if out of Boundary Box. */
	(Ymin1 > Ymax2)	|| (Ymax1 < Ymin2))
	return FALSE;

    /* Let the line equations of the two edges be defined as:		     */
    /* L1 = p11	+ t1 * (pt12 - pt11) , t1 = [0..1]			     */
    /* L2 = p21	+ t2 * (pt22 - pt21) , t2 = [0..1]			     */
    /* at intersection point (if any) we have:				     */
    /* pt11 + t1 * (pt12 - pt11) == pt21 + t2 *	(pt22 -	pt21)  for x, y	     */
    /* or two equations	(for x, y) with two unknown (t1, t2) to solve:	     */
    /* a1 = b11	* t1 + b12 * t2		from x				     */
    /* a2 = b21	* t1 + b22 * t2		from y				     */
    /* and we have interesection if both t1, t2	in the range [0..1]	     */
    a1 =  Crd10[0] - Crd20[0];
    b11	= Crd10[0] - Crd11[0];
    b12	= Crd21[0] - Crd20[0];
    a2 =  Crd10[1] - Crd20[1];
    b21	= Crd10[1] - Crd11[1];
    b22	= Crd21[1] - Crd20[1];

    /* If the detereminant is zero, the	two lines are parellel - no inter. */
    if (IRIT_APX_EQ((det = b11 * b22 - b21 * b12), 0.0))
	return FALSE;

    t1 = (a1 * b22 - a2	* b12) / det;
    t2 = (b11 *	a2 - b21 * a1) / det;

    /* Test if intersection is happening in one	edge END - in that case	*/
    /* we break	only the second	edge into two parts.			*/
    OneInter1 =	((t1 < 1.0) && (t1 > 0.0) &&
		 !(IRIT_APX_EQ(t1, 0.0) || IRIT_APX_EQ(t1, 1.0)) &&
		  (IRIT_APX_EQ(t2, 0.0) || IRIT_APX_EQ(t2, 1.0)));
    OneInter2 =	((t2 < 1.0) && (t2 > 0.0) &&
		 !(IRIT_APX_EQ(t2, 0.0) || IRIT_APX_EQ(t2, 1.0)) &&
		  (IRIT_APX_EQ(t1, 0.0) || IRIT_APX_EQ(t1, 1.0)));

    /* If out of 0..1 range in one of edges - no intersection: */
    if ((!(OneInter1 ||	OneInter2)) &&
	((t1 >=	1.0) ||	(t1 <= 0.0) || (t2 >= 1.0) || (t2 <= 0.0) ||
	 IRIT_APX_EQ(t1, 0.0) || IRIT_APX_EQ(t1, 1.0) ||
	 IRIT_APX_EQ(t2, 0.0) || IRIT_APX_EQ(t2, 1.0))) return FALSE;

    /* If we are here, we have intersection - find the bottom edge and split */
    /* it - allocated new edge struct and update to new upper (in Y) part.   */
    Z1 = Crd10[2] * (1.0 - t1) + Crd11[2] * t1;
    Z2 = Crd20[2] * (1.0 - t2) + Crd21[2] * t2;
    if (!OneInter2 && Z1 < Z2) {
	*PEdgeNew1 = (EdgeStruct *) IritMalloc(sizeof(EdgeStruct));
	(*PEdgeNew1) -> Pnext = NULL;
	(*PEdgeNew1) -> Internal = PEdge1 -> Internal;
	(*PEdgeNew1) ->	Vertex[0] = IPAllocVertex2(NULL);
	for (i = 0; i < 2; i++)
	    (*PEdgeNew1) -> Vertex[0] -> Coord[i] =
		Crd10[i] * (1.0	- t1) +	Crd11[i] * t1;
	(*PEdgeNew1) -> Vertex[0] -> Coord[2] = Z1;
	/* Now update the second vertex	of both	PEdge1 & PEdgeNew1:	   */
	/* Note	we assume Vertex[0] -> Coord[1]	< Vertex[1] -> Coord[1]	as */
	/* all input edges are sorted this way when entered to hash table. */
	(*PEdgeNew1) ->	Vertex[1] = PEdge1 -> Vertex[1];
	PEdge1 -> Vertex[1] = (*PEdgeNew1) -> Vertex[0];

    }
    else
	*PEdgeNew1 = NULL;

    if (!OneInter1 && Z2 < Z1) {
	*PEdgeNew2 = (EdgeStruct *) IritMalloc(sizeof(EdgeStruct));
	(*PEdgeNew2) -> Pnext = NULL;
	(*PEdgeNew2) -> Internal = PEdge2 -> Internal;
	(*PEdgeNew2) ->	Vertex[0] = IPAllocVertex2(NULL);
	for (i = 0; i < 2; i++)
	    (*PEdgeNew2) -> Vertex[0] -> Coord[i] =
		Crd20[i] * (1.0	- t2) +	Crd21[i] * t2;
	(*PEdgeNew2) -> Vertex[0] -> Coord[2] = Z2;
	/* Now update the second vertex	of both	PEdge2 & PEdgeNew2: */
	(*PEdgeNew2) ->	Vertex[1] = PEdge2 -> Vertex[1];
	PEdge2 -> Vertex[1] = (*PEdgeNew2) -> Vertex[0];
    }
    else
	*PEdgeNew2 = NULL;

    return (*PEdgeNew1 != NULL) || (*PEdgeNew2 != NULL);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print the content of a given edge:				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PPoly:      To print content of.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintPolyContent(IPPolygonStruct *PPoly)
{
    IPVertexStruct
	*PList = PPoly -> PVertex;

    while (PList) {
	fprintf(stderr, "   %12f %12f %12f\n",
	    PList -> Coord[0],
	    PList -> Coord[1],
	    PList -> Coord[2]);
	PList =	PList -> Pnext;
    }
}

#ifdef DEBUG1

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to print the content of a given edge:				     *
*                                                                            *
* PARAMETERS:                                                                *
*   PEdge:      To print content of                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void PrintEdgeContent(EdgeStruct *PEdge)
{
    fprintf(stderr, "   %11f %11f %11f : %11f %11f %11f\n",
	PEdge -> Vertex[0] -> Coord[0],
	PEdge -> Vertex[0] -> Coord[1],
	PEdge -> Vertex[0] -> Coord[2],
	PEdge -> Vertex[1] -> Coord[0],
	PEdge -> Vertex[1] -> Coord[1],
	PEdge -> Vertex[1] -> Coord[2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to dump all the segments in the EdgeHashTable as IRIT data file so *
* it can be displayed, to stdout.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawEdgeHashTable(void)
{
    int	i;
    EdgeStruct *PEtemp;

    for	(i=0; i<EDGE_HASH_TABLE_SIZE; i++) {
	PEtemp = EdgeHashTable[i];
	while(PEtemp) {
	    DrawEdge(PEtemp);
	    PEtemp = PEtemp -> Pnext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to dump one segment as IRIT data file so it can be displayed, to   *
* stdout.			                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DrawEdge(EdgeStruct *PEdge)
{
    printf("    [POINTLIST 2\n\t[%lf %lf %lf]\n\t[%lf %lf %lf]\n    ]\n",
	   PEdge->Vertex[0]->Coord[0],
	   PEdge->Vertex[0]->Coord[1],
	   PEdge->Vertex[0]->Coord[2],
	   PEdge->Vertex[1]->Coord[0],
	   PEdge->Vertex[1]->Coord[1],
	   PEdge->Vertex[1]->Coord[2]);
    printf("    [POLYLINE 2\n\t[%lf %lf %lf]\n\t[%lf %lf %lf]\n    ]\n",
	   PEdge->Vertex[0]->Coord[0],
	   PEdge->Vertex[0]->Coord[1],
	   PEdge->Vertex[0]->Coord[2],
	   PEdge->Vertex[1]->Coord[0],
	   PEdge->Vertex[1]->Coord[1],
	   PEdge->Vertex[1]->Coord[2]);
}

#endif /* DEBUG1 */
