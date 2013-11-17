/*****************************************************************************
*   Routines to	test all edges for visibility relative to the global polygon *
* list,	and output the visible ones, to	graphics screen	or as text file	     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber				Ver 2.0, Jan. 1989   *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <ctype.h>
#include <string.h>
#include "program.h"
#include "misc_lib.h"

#define	MAX_POLYLINE_SIZE	50	 /* Maximum size of output polyline. */
#define EDGE_ON_PLANE_EPS	-0.0001 /* Epsilon considered edge on plane. */

IRIT_STATIC_DATA EdgeStruct *VisOutEdgeHashTable[EDGE_HASH_TABLE_SIZE];
IRIT_STATIC_DATA EdgeStruct *HidOutEdgeHashTable[EDGE_HASH_TABLE_SIZE];

static void SaveCurrentMatrix(FILE *OutFile);
static void OutputEdge(EdgeStruct *PEtemp, EdgeStruct *OutEdgeHashTable[]);
static void EmitOutEdgeHashTable(FILE *OutFile,
				 EdgeStruct *OutEdgeHashTable[]);
static char *Real2Str(IrtRType R);
static EdgeStruct *FoundMatchEdge(int *Collinear,
				  EdgeStruct *PEdge,
				  EdgeStruct *OutEdgeHashTable[]);
static int CollinearPoints(IrtRType Pt1[3], IrtRType Pt2[3], IrtRType Pt3[3]);
static int VisibleEdge(int EdgeMinY,
		       EdgeStruct *PEdge,
		       IPPolygonStruct *PolyHashTbl[]);
static int VisiblePointOnePoly(IrtRType MidPt[3], IPPolygonStruct *PPoly);
static int ZCrossProd(IrtRType Pt1[3], IrtRType Pt2[3], IrtRType Pt3[3]);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to test for visibility all edges in EdgeHashTable and	display	or   M
* output the visible ones only.	It is assumed that only	totally	visible	or   M
* invisible edges are in table (Pass 3 broke all other kind of edges).	     M
*   Hidden information will also be dumped out with hidden attributes if     M
* GlblOutputHiddenData is TRUE.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   OutFile:     Where output should go to.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   OutVisibleEdges, visibility                                              M
*****************************************************************************/
void OutVisibleEdges(FILE *OutFile)
{
    int	i;
    EdgeStruct *PEtemp, *PEtail;

    IritCPUTime(TRUE);

    if (!GlblQuiet)
	fprintf(stderr, "\nPass 4, Edges [%5d] =      ", EdgeCount);
    for (i = 0; i < EDGE_HASH_TABLE_SIZE; i++)
	VisOutEdgeHashTable[i] = HidOutEdgeHashTable[i] = NULL;

    if (!MatInverseMatrix(GlblViewMat, GlblViewMat)) {
	IRIT_WARNING_MSG_PRINTF(
	        "\nNo inverse for matrix transformation, output is in screen space\n");
	MatGenUnitMat(GlblViewMat);	     /* No inverse transform at all. */
    }

    IRIT_DATA_HEADER(OutFile, "Poly3d-h");

    EdgeCount =	0;

    /* Output the viewing matrices. */
    SaveCurrentMatrix(OutFile);

    for (i = 0; i < EDGE_HASH_TABLE_SIZE; i++)
	if ((PEtemp = EdgeHashTable[i]) != NULL)/* If any edge in that list. */
	    while (PEtemp) {
		EdgeCount++;
		if (!GlblQuiet)
		    fprintf(stderr, "\b\b\b\b\b%5d", EdgeCount);
		PEtail = PEtemp	-> Pnext;	   /* OutputEdge destroy it. */
		if ((!PEtemp -> Internal || GlblInternal)) {
		    if (VisibleEdge(i, PEtemp, PolyHashTable))
			OutputEdge(PEtemp, VisOutEdgeHashTable);
		    else if (GlblOutputHiddenData)
			OutputEdge(PEtemp, HidOutEdgeHashTable);
		}
		PEtemp = PEtail;
	    }


    /* Output the visible data: */
    if (GlblOutputHasRGB) {
	fprintf(OutFile,
		"[OBJECT [COLOR %d] [RGB %d,%d,%d] [WIDTH %8.6f] Visible\n",
		GlblOutputColor,
		GlblOutputRGB[0], GlblOutputRGB[1], GlblOutputRGB[2],
		GlblOutputWidth);
    }
    else {
	fprintf(OutFile, "[OBJECT [COLOR %d] [WIDTH %8.6f] Visible\n",
		GlblOutputColor, GlblOutputWidth);
    }
    /* Output all visible data. */
    EmitOutEdgeHashTable(OutFile, VisOutEdgeHashTable);
    fprintf(OutFile, "]\n\n");

    /* Output the hidden data if requested: */
    if (GlblOutputHiddenData) {
	if (GlblOutputHasRGB) {
	    fprintf(OutFile, "[OBJECT [HIDDEN] [COLOR %d] [RGB %d,%d,%d] [WIDTH %8.6f] Hidden\n",
		    HIDDEN_COLOR,
		    GlblOutputRGB[0] / HIDDEN_COLOR_RATIO,
		    GlblOutputRGB[1] / HIDDEN_COLOR_RATIO,
		    GlblOutputRGB[2] / HIDDEN_COLOR_RATIO,
		    GlblOutputWidth / HIDDEN_WIDTH_RATIO);
	}
	else {
	    fprintf(OutFile, "[OBJECT [HIDDEN] [COLOR %d] [WIDTH %8.6f] Hidden\n",
		    HIDDEN_COLOR, GlblOutputWidth / HIDDEN_WIDTH_RATIO);
	}
	/* Output all hidden data. */
	EmitOutEdgeHashTable(OutFile, HidOutEdgeHashTable);
	fprintf(OutFile, "]\n\n");
    }

    if (!GlblQuiet)
	fprintf(stderr, ",  %6.2f seconds.", IritCPUTime(FALSE));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Saves current view transformation.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   OutFile:     Where output should go to.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SaveCurrentMatrix(FILE *OutFile)
{
    int	i, j;

    fprintf(OutFile, "[OBJECT MATRICES\n    [OBJECT VIEW_MAT\n\t[MATRIX");
    for (i = 0; i < 4; i++) {
	fprintf(OutFile, "\n\t    ");
	for (j = 0; j < 4; j++)
	    fprintf(OutFile, "%12f ", IPViewMat[i][j]);
    }
    fprintf(OutFile, "\n\t]\n    ]\n");

    if (IPWasPrspMat) {
	fprintf(OutFile, "    [OBJECT PRSP_MAT\n\t[MATRIX");
	for (i = 0; i < 4; i++) {
	    fprintf(OutFile, "\n\t    ");
	    for (j = 0; j < 4; j++)
		fprintf(OutFile, "%12f ", IPPrspMat[i][j]);
	}
	fprintf(OutFile, "\n\t]\n    ]\n");
    }

    fprintf(OutFile, "]\n\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to output one	edge to	the OutEdgeHashTable.			     *
*   The edge is inserted into OutEdgeHashTable to be able to match it into   *
* the longest path possible - connecting edges into edge sequence.	     *
*   Each edge is inserted by its Ymin, which will cause the paths be in	     *
* increased Y order...							     *
*                                                                            *
* PARAMETERS:                                                                *
*   PEtemp:           Edge to insert into hash table.                        *
*   OutEdgeHashTable: The hash table to put PEtemp in.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void OutputEdge(EdgeStruct *PEtemp, EdgeStruct *OutEdgeHashTable[])
{
    int	Level;

    Level = (int) ((PEtemp -> Vertex[0]	-> Coord[1] + 1.0) *
							EDGE_HASH_TABLE_SIZE2);
    Level = IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1);
    PEtemp -> Pnext = OutEdgeHashTable[Level];
    OutEdgeHashTable[Level] = PEtemp;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to scan the OutEdgeHashTable,	and Emit the longest paths found in  *
* it by	searching for consecutive edges	and combining collinear edges.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   OutFile:            Where output should go to.                           *
*   OutEdgeHashTable:   Hash table to scan.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void EmitOutEdgeHashTable(FILE *OutFile, EdgeStruct *OutEdgeHashTable[])
{
    int	i, j, Collinear, Count;
    IrtRType Vec[MAX_POLYLINE_SIZE][3];
    EdgeStruct *PEtemp, *PEnext;

    for	(i = 0; i < EDGE_HASH_TABLE_SIZE; i++)	 /* Scan all the hash table. */
	while (OutEdgeHashTable[i]) {		 /* Scan all edges in entry. */
	    PEtemp = OutEdgeHashTable[i];	  /* Take edge out of table. */
	    OutEdgeHashTable[i]	= OutEdgeHashTable[i] -> Pnext;

	    /* Print first vertices of polyline: */
	    Count = 0;
	    MatMultPtby4by4(Vec[Count++], PEtemp -> Vertex[0] -> Coord,
								GlblViewMat);
	    MatMultPtby4by4(Vec[Count++], PEtemp -> Vertex[1] -> Coord,
								GlblViewMat);

	    while ((PEnext = FoundMatchEdge(&Collinear, PEtemp,
					    OutEdgeHashTable)) != NULL) {
		if (Collinear)			    /* Overwrite last entry. */
		    MatMultPtby4by4(Vec[Count - 1],
				     PEnext -> Vertex[1] -> Coord,
				     GlblViewMat);
		else
		    MatMultPtby4by4(Vec[Count++],
				     PEnext -> Vertex[1] -> Coord,
				     GlblViewMat);
		if (Count >= MAX_POLYLINE_SIZE)
		    break;
		PEtemp = PEnext;
	    }

	    fprintf(OutFile, "    [POLYLINE %d\n", Count);
	    for (j = 0; j < Count; j++)
		fprintf(OutFile, "\t[%s %s %s]\n",
			Real2Str(Vec[j][0]),
			Real2Str(Vec[j][1]),
			Real2Str(Vec[j][2]));

	    fprintf(OutFile, "    ]\n");
	}
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Converts a real number into a string.					     *
*   The routine maintains 3 different buffers simultanuously so up to 3      *
* calls can be issued from same printf...				     *
*                                                                            *
* PARAMETERS:                                                                *
*   R:         To convert to a string.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   char *:    A string representation for R.                                *
*****************************************************************************/
static char *Real2Str(IrtRType R)
{
    IRIT_STATIC_DATA int
	i = 0;
    IRIT_STATIC_DATA char Buffer[3][IRIT_LINE_LEN_LONG];
    int j, k;

    if (IRIT_FABS(R) < 1e-6)
	R = 0.0;		    /* Round off very small numbers. */

    sprintf(Buffer[i], "%-8.6g", R);

    for (k = 0; !isdigit(Buffer[i][k]) && k < IRIT_LINE_LEN_LONG; k++);
    if (k >= IRIT_LINE_LEN_LONG) {
	IRIT_WARNING_MSG("Conversion of real number failed.\n");
	Poly3dhExit(3);
    }

    for (j = (int) strlen(Buffer[i]) - 1; Buffer[i][j] == ' ' && j > k; j--);
    if (strchr(Buffer[i], '.') != NULL)
	for (; Buffer[i][j] == '0' && j > k; j--);
    Buffer[i][j+1] = 0;

    j = i;
    i = (i + 1) % 3;
    return Buffer[j];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to scan the OutEdgeHashTable for a match edge	if any,	delete it    *
* from HashTable and return it.						     *
*   If collinear with PEdge sets Collinear to TRUE.			     *
*   Returns NULL if no match found (end of polyline).			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Collinear:        Is this edge collinear with PEdge?                     *
*   PEdge:            To search for a match.                                 *
*   OutEdgeHashTable: Hash teable to searc.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   EdgeStruct *:     Matched edge, or NULL if none found.                   *
*****************************************************************************/
static EdgeStruct *FoundMatchEdge(int *Collinear,
				  EdgeStruct *PEdge,
				  EdgeStruct *OutEdgeHashTable[])
{
    int	Level;
    EdgeStruct *PEtemp, *PElast;

    Level = (int) ((PEdge -> Vertex[1] -> Coord[1] + 1.0) *
							EDGE_HASH_TABLE_SIZE2);
    Level = IRIT_BOUND(Level, 0, EDGE_HASH_TABLE_SIZE1);
    PEtemp = PElast = OutEdgeHashTable[Level];
    while (PEtemp) {			   /* First scan for collinear edge. */
	if (IRIT_PT_APX_EQ(PEdge -> Vertex[1] -> Coord,
		      PEtemp -> Vertex[0] -> Coord) &&
	    CollinearPoints(PEdge -> Vertex[0] -> Coord,
			    PEdge -> Vertex[1] -> Coord,
			    PEtemp -> Vertex[1] -> Coord)) {
	    *Collinear = TRUE;
	    /* Delete that edge	from hash table	structure: */
	    if (PEtemp == PElast)
		OutEdgeHashTable[Level] = OutEdgeHashTable[Level] -> Pnext;
	    else
		PElast -> Pnext = PEtemp -> Pnext;

	    if (IRIT_PT_APX_EQ(PEtemp -> Vertex[0] -> Coord,
			  PEtemp -> Vertex[1] -> Coord))
		return FoundMatchEdge(Collinear, PEdge, OutEdgeHashTable);
	    else
	        return PEtemp;
	}
	PElast = PEtemp;
	PEtemp = PEtemp	-> Pnext;
    }

    PEtemp = PElast = OutEdgeHashTable[Level];
    while (PEtemp) {			    /* Next scan for any match edge. */
	if (IRIT_PT_APX_EQ(PEdge -> Vertex[1] -> Coord,
		      PEtemp -> Vertex[0] -> Coord)) {
	    *Collinear = FALSE;
	    /* Delete that edge	from hash table	structure: */
	    if (PEtemp == PElast)
		OutEdgeHashTable[Level] = OutEdgeHashTable[Level] -> Pnext;
	    else
	        PElast -> Pnext = PEtemp -> Pnext;

	    if (IRIT_PT_APX_EQ(PEtemp -> Vertex[0] -> Coord,
			  PEtemp -> Vertex[1] -> Coord))
		return FoundMatchEdge(Collinear, PEdge, OutEdgeHashTable);
	    else
		return PEtemp;
	}
	PElast = PEtemp;
	PEtemp = PEtemp	-> Pnext;
    }

    return NULL;			      /* No match - end of polyline. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test if three points are collinear.			     *
*   Algorithm: cross product should be zero if collinear...		     *
*   Note this routine does not return TRUE if Pt2 is not between Pt1..Pt3.   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Pt2, Pt3:  The three points to test for collinearity.               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        TRUE if collinear, FALSE otherwise.                          *
*****************************************************************************/
static int CollinearPoints(IrtRType Pt1[3], IrtRType Pt2[3], IrtRType Pt3[3])
{
    int	i;
    IrtRType Xout, Yout, Zout, U[3], V[3], temp;

    for	(i = 0; i < 3; i++) {
	U[i] = Pt2[i] -	Pt1[i];
	V[i] = Pt3[i] -	Pt2[i];
    }
    temp = sqrt(IRIT_SQR(U[0]) + IRIT_SQR(U[1]) + IRIT_SQR(U[2]));
    if (IRIT_APX_EQ(temp, 0.0))
	return TRUE;
    for	(i = 0; i < 3; i++)
	U[i] = U[i] / temp;

    temp = sqrt(IRIT_SQR(V[0]) + IRIT_SQR(V[1]) + IRIT_SQR(V[2]));
    if (IRIT_APX_EQ(temp, 0.0))
	return TRUE;
    for	(i = 0; i < 3; i++)
	V[i] = V[i] / temp;

    /* Xoutput = Uy * Vz - Uz *	Vy. */
    Xout = U[1]	/* Uy */  * V[2] /* Vz */  -
	   U[2]	/* Uz */  * V[1] /* Vy */;

    /* Youtput = Uz * Vx - Ux *	Vz. */
    Yout = U[2]	/* Uz */  * V[0] /* Vx */  -
	   U[0]	/* Ux */  * V[2] /* Vz */;

    /* Zoutput = Ux * Vy - Uy *	Vx. */
    Zout = U[0]	/* Ux */  * V[1] /* Vy */  -
	   U[1]	/* Uy */  * V[0] /* Vx */;

    return IRIT_APX_EQ(Xout, 0.0) &&
	   IRIT_APX_EQ(Yout, 0.0) &&
	   IRIT_APX_EQ(Zout, 0.0) &&
	   ((IRIT_MIN(Pt1[0], Pt3[0]) < Pt2[0]) &&
	    (IRIT_MAX(Pt1[0], Pt3[0]) > Pt2[0]) &&
	    (IRIT_MIN(Pt1[1], Pt3[1]) < Pt2[1]) &&
	    (IRIT_MAX(Pt1[1], Pt3[1]) > Pt2[1]));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test the visibility of the	given edge relative to all polygons  *
* in polygon list. Return TRUE if the edge is visible. It is assumed that    *
* the edge is whole visible or whole invisible (Pass 3 broke the edge if     *
* that whas not	true). Also it is assumed the polygons are all convex.	     *
*   A short cut is made to test the edge only against the needed polygons    *
* only, using the polygon hash table and Y level sorting.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   EdgeMinY:      Index into the hash table of polygons.                    *
*   PEdge:         Edge to examine its visibility.                           *
*   PolyHashTbl:   Hash table of polygons.                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:           TRUE if visible, FALSE otherwise.                         *
*****************************************************************************/
static int VisibleEdge(int EdgeMinY,
		       EdgeStruct *PEdge,
		       IPPolygonStruct *PolyHashTbl[])
{
    int	EdgeMaxY, i;
    IrtRType MidPt[3];
    IPPolygonStruct *PPolyList, *PPolyLast;

    EdgeMaxY = (int) (IRIT_MIN((IRIT_MAX(PEdge -> Vertex[0] -> Coord[1],
			       PEdge -> Vertex[1] -> Coord[1]) + 1.0) *
						EDGE_HASH_TABLE_SIZE2,
			  EDGE_HASH_TABLE_SIZE1));

    for	(i = 0; i < 3; i++)		    /* Calc a mid point on the edge: */
	MidPt[i] = (PEdge -> Vertex[0] -> Coord[i] +
		    PEdge -> Vertex[1] -> Coord[i]) * 0.5;
    MidPt[2] -= EDGE_ON_PLANE_EPS * 3;

    /* Test the	edge only in the interval 0..EdgeMaxY as these are the only  */
    /* which might hide	that edge. Also	polygons with MaxY less	than	     */
    /* EdgeMinY	are deleted from PolyHashTbl as it is assumed the edges      */
    /* are scanned in increasing order of the EdgeHashTable.		     */
    for	(i = 0; i <= EdgeMaxY; i++) {
	PPolyList = PPolyLast =	PolyHashTbl[i];
	while (PPolyList) {
	    if ((PPolyList -> BBox[1][1] + 1.0) *
					    EDGE_HASH_TABLE_SIZE2 < EdgeMinY) {
		/* Delete this polygon!	*/
		if (PPolyList == PPolyLast) {
		    PolyHashTbl[i] = PPolyLast = PPolyList -> Pnext;
		    PPolyList =	PPolyList -> Pnext;
		}
		else {
		    PPolyList =	PPolyList -> Pnext;
		    PPolyLast -> Pnext = PPolyList;
		}
	    }
	    else {	     /* Polygon still active - test edge against it. */
		/* If found one	polygon	that hides this	edge return FALSE... */

		if (!VisiblePointOnePoly(MidPt, PPolyList))
		    return FALSE;
		PPolyLast = PPolyList;
		PPolyList = PPolyList -> Pnext;
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to test the visibility of the	given edge relative to one polygon   *
* by testing a single point on the polygon.				     *
*   Returns TRUE if the edge is visible. It is assumed that the edge is      *
* completely visible or completely invisible (Pass 3 broke the edge if that  *
* was not true).							     *
*   Also it is assumed the polygons are all convex.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   MidPt:     A middle point on the examined edge,for visibility testing.   *
*   PPoly:     List of polygons to verify MidPt's visibility against.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if visible, FALSE otherwise.                             *
*****************************************************************************/
static int VisiblePointOnePoly(IrtRType MidPt[3], IPPolygonStruct *PPoly)
{
    IPVertexStruct *PList;

    /* Test if edges is	out of polygon boundary	box: */
    if (MidPt[0] > PPoly -> BBox[1][0] || MidPt[0] < PPoly -> BBox[0][0] ||
	MidPt[1] > PPoly -> BBox[1][1] || MidPt[1] < PPoly -> BBox[0][1])
	return TRUE;

    if (PPoly -> Plane[0] * MidPt[0] +
	PPoly -> Plane[1] * MidPt[1] +
	PPoly -> Plane[2] * MidPt[2] +
	PPoly -> Plane[3] > EDGE_ON_PLANE_EPS)
	return TRUE;				/* Edge in front of polygon. */

    /* We cannt	escape it - we now must	find if	the point is included in */
    /* polygon area. We	assume the polygon is convex and use the fact	 */
    /* that the	polygon	list is	ordered	such that the cross product	 */
    /* of two consecutive vertices and the point is positive for all	 */
    /* consecutive vertices pairs iff the point	is in the polygon.	 */
    PList = PPoly -> PVertex;
    while (PList -> Pnext) {
	if (ZCrossProd(PList -> Coord, PList -> Pnext -> Coord, MidPt) < 0)
	    return TRUE;				  /* Out of polygon! */
	PList =	PList -> Pnext;
    }
    /* Now test	last polygon edge (last	point, first point in poly list) */
    if (ZCrossProd(PList -> Coord, PPoly -> PVertex -> Coord, MidPt) < 0)
	return TRUE;					  /* Out of polygon! */

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Routine to evaluate the cross	product	of 3 points projected to Z = 0 plane *
* and return the sign of the result (Only Z component).			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt1, Pt2, Pt3:  Three points to compute Z component of cross product.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        Sign of Z component of cross product.                        *
*****************************************************************************/
static int ZCrossProd(IrtRType Pt1[3], IrtRType Pt2[3], IrtRType Pt3[3])
{
    IrtRType Zout;

    /* U = Pt2 - Pt1,  V = Pt3 - Pt2,		Zoutput	= Ux * Vy - Uy * Vx. */
    Zout = (Pt2[0] - Pt1[0]) /*	Ux */  * (Pt3[1] - Pt2[1]) /* Vy */  -
	   (Pt2[1] - Pt1[1]) /*	Uy */  * (Pt3[0] - Pt2[0]) /* Vx */;
    if (IRIT_APX_EQ(Zout, 0.0))
	return 0;
    if (Zout < 0.0)
	return -1;
    else
	return 1;
}
