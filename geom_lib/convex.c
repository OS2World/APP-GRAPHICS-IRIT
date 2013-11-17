/******************************************************************************
* Convex.c - test convexity and converts polygons to convex ones.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March 1990.					      *
******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "allocate.h"
#include "misc_lib.h"
#include "geom_loc.h"

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugCnvxPolyBeforeAfter, FALSE);
#endif /* DEBUG */

/* Used to hold edges (V | V -> Pnext) that intersect with level y = Ylevel. */
typedef struct InterYVrtxList {
    IrtBType InterYType;
    IPVertexStruct *V;
    struct InterYVrtxList *Pnext;
} InterYVrtxList;

#define CONVEX_IRIT_EPS	1e-8      /* Collinearity of two normalized vectors. */

#define	INTER_Y_NONE	0		       /* Y level intersection type. */
#define	INTER_Y_START	1
#define	INTER_Y_MIDDLE	2

#define LOOP_ABOVE_Y	0      /* Type of open loops extracted from polygon. */
#define LOOP_BELOW_Y	1

#define CNVX_MAX_NO_SCALE_SIZE	10
#define CNVX_MIN_NO_SCALE_SIZE	0.1

/* Used to sort and combine the polygons above Ylevel together if possible.  */
typedef struct SortPolysInX {
    IPVertexStruct *VMinX, *VMaxX;
    int Reverse; 	 /* If TRUE than VMinX vertex is AFTER VMaxX vertex. */
} SortPolysInX;

/* The following are temporary flags used to mark vertices that were visited */
/* by the loop tracing, at list once. As each vertex may be visited two at   */
/* the most (as starting & as end point of open loop), this is enough.	     */
/* INTER_TAG is used to mark vertices that created brand new with intersected*/
/* with the line y = Ylevel. Those are used to detect INTERNAL edges - if    */
/* at list one end of it is INTER_TAG, that edge is INTERNAL.		     */
#define INTER_TAG   0x40
#define VISITED_TAG 0x80

#define	IS_INTER_VRTX(Vrtx)	((Vrtx)->Tags & INTER_TAG)
#define	SET_INTER_VRTX(Vrtx)	((Vrtx)->Tags |= INTER_TAG)
#define	RST_INTER_VRTX(Vrtx)	((Vrtx)->Tags &= ~INTER_TAG)

#define	IS_VISITED_VRTX(Vrtx)	((Vrtx)->Tags & VISITED_TAG)
#define	SET_VISITED_VRTX(Vrtx)	((Vrtx)->Tags |= VISITED_TAG)
#define	RST_VISITED_VRTX(Vrtx)	((Vrtx)->Tags &= ~VISITED_TAG)

IRIT_STATIC_DATA int
    GlblHandleNormals = TRUE,
    GlblUpdateRayDirToPointToVertex = TRUE;

static void UpdatePolyScale(IPPolygonStruct *Pl, IrtRType PolyScale);
static int SplitPolyIntoTwo(IPPolygonStruct *Pl,
			    IPVertexStruct *V,
			    IPPolygonStruct **Pl1,
			    IPPolygonStruct **Pl2);
static int VrtxDistanceInPoly(IPVertexStruct *V1, IPVertexStruct *V2);
static void UpdateRayDirToPointToVertex(IPVertexStruct *VRay,
					IrtVecType RayDir,
					IPPolygonStruct *Pl);
static IPVertexStruct *FindRayPolyInter(IPPolygonStruct *Pl,
					IPVertexStruct *VRay,
					IrtPtType RayDir,
					IrtPtType PInter);
static void TestConvexityDir(IPPolygonStruct *Pl);

#ifdef DEBUG
static void CnvxPrintPoly(IPPolygonStruct *P);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to prepare a transformation martix to rotate such that Dir is    M
* parallel to the Z axes. Used by the convex decomposition to rotate the     M
* polygons to be XY plane parallel.					     M
*    Algorithm: form a 4 by 4 matrix from Dir as follows:		     M
*                |  Tx  Ty  Tz  0 |   A transformation which takes the coord V
*                |  Bx  By  Bz  0 |  system into T, N & B as required.	     V
* [X  Y  Z  1] * |  Nx  Ny  Nz  0 |					     V
*                |  0   0   0   1 |					     V
*   N is exactly Dir, but we got freedom on T & B which must be on	     M
* a plane perpendicular to N and perpendicular between them but thats all!   M
*   T is therefore selected using this (heuristic ?) algorithm:		     M
*   Let P be the axis of which the absolute N coefficient is the smallest.   M
*   Let B be (N cross P) and T be (B cross N).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:     To place the constructed homogeneous transformation.            M
*   Dir:     To derive a transformation such that Dir goes to Z axis.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMGenRotateMatrix, transformations                                       M
*****************************************************************************/
void GMGenRotateMatrix(IrtHmgnMatType Mat, const IrtVecType Dir)
{
    int i, j;
    IrtRType R;
    IrtVecType DirN, T, B, P;

    IRIT_PT_COPY(DirN, Dir);
    IRIT_PT_NORMALIZE(DirN);
    IRIT_PT_RESET(P);
    for (i = 1, j = 0, R = IRIT_FABS(DirN[0]); i < 3; i++)
	if (R > IRIT_FABS(DirN[i])) {
	    R = DirN[i];
	    j = i;
	}
    P[j] = 1.0;/* Now P is set to the axis with the biggest angle from DirN. */

    GMVecCrossProd(B, DirN, P);			      /* calc the bi-normal. */
    IRIT_PT_NORMALIZE(B);
    GMVecCrossProd(T, B, DirN);				/* calc the tangent. */

    MatGenUnitMat(Mat);
    for (i = 0; i < 3; i++) {
	Mat[i][0] = T[i];
	Mat[i][1] = B[i];
	Mat[i][2] = DirN[i];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If TRUE, normals of vertices will be treated.  If FALSE, only positions  M
* are considered in the convex decompositions.                               M
*   This function also affects the propagation of attributes as well such as M
* rgb color and uv coordinates.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   HandleNormals:   TRUE for treating normals, FALSE to ignore them.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Previous state of normal handling.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvexPolyObjectN, GMConvertPolysToTriangles, ConvexPolyObject,        M
* ConvexPolygon, SplitNonConvexPoly, GMConvexRaysToVertices                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMConvexPolyNormals, normals, rgb color, uv coordinates                  M
*****************************************************************************/
int GMConvexPolyNormals(int HandleNormals)
{
    int OldVal = GlblHandleNormals;

    GlblHandleNormals = HandleNormals;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   If TRUE, ray will be fired to vertices. If FALSE, ray as angle bisectors M
* will be used.   				                             M
*                                                                            *
* PARAMETERS:                                                                M
*   RaysToVertices:   TRUE for rays to vertices, FALSE bisector rays.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Previous state of raay casting.	                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvexPolyObjectN, GMConvertPolysToTriangles, ConvexPolyObject,        M
* ConvexPolygon, SplitNonConvexPoly, GMConvexPolyNormals                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMConvexRaysToVertices                                                   M
*****************************************************************************/
int GMConvexRaysToVertices(int RaysToVertices)
{
    int OldVal = GlblUpdateRayDirToPointToVertex;

    GlblUpdateRayDirToPointToVertex = RaysToVertices;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to test all polygons in a given object for convexity, and split  M
* non convex ones, non destructively - the original object is not modified.  M
*   This function will introduce new vertices to the split polygons.         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       To test for convexity of its polygons.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  A duplicate of PObj, but with convex polygons only.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvertPolysToTriangles, ConvexPolyObject, ConvexPolygon,              M
* SplitNonConvexPoly							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMConvexPolyObjectN, convexity, convex polygon                           M
*****************************************************************************/
IPObjectStruct *GMConvexPolyObjectN(IPObjectStruct *PObj)
{
    IPObjectStruct
	*PObjCopy = IPCopyObject(NULL, PObj, FALSE);

    GMConvexPolyObject(PObjCopy);

    return PObjCopy;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to test all polygons in a given object for convexity, and split  M
* non convex ones, in place.                                                 M
*   This function will introduce new vertices to the split polygons.         M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:       To test for convexity of its polygons, and split into convex M
*               polygons non convex polygons found, in place. Either a       M
*		polygonal object or a list of polygonal objects.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvertPolysToTriangles, GMConvexPolyObjectN, GMIsConvexPolygon,       M
* GMSplitNonConvexPoly							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMConvexPolyObject, convexity, convex polygon                            M
*****************************************************************************/
void GMConvexPolyObject(IPObjectStruct *PObj)
{
    IPPolygonStruct *Pl, *PlSplit, *PlTemp,
	*PlPrev = NULL;
    GMBBBboxStruct *BBox;
    CagdRType PolyScale, PolyScale1;

    if (IP_IS_OLST_OBJ(PObj)) {
	int i = 0;
	IPObjectStruct *Obj;

	while ((Obj = IPListObjectGet(PObj, i++)) != NULL)
	    GMConvexPolyObject(Obj);

	return;
    }

    if (!IP_IS_POLY_OBJ(PObj) || IP_IS_POLYLINE_OBJ(PObj))
        return;

    /* Make sure object is normalized around the unit size cube as we use    */
    /* absolute tolerances and epsilons.				     */
    BBox = GMBBComputeBboxObject(PObj);
    PolyScale = IRIT_MAX(IRIT_MAX(BBox -> Max[0] - BBox -> Min[0],
			BBox -> Max[1] - BBox -> Min[1]),
		    BBox -> Max[2] - BBox -> Min[2]);
    if (PolyScale > CNVX_MAX_NO_SCALE_SIZE ||
	PolyScale < CNVX_MIN_NO_SCALE_SIZE)
	PolyScale1 = 1.0 / PolyScale;
    else {
        PolyScale = 0.0;
	PolyScale1 = 0.0;
    }

    Pl = PObj -> U.Pl;
    while (Pl != NULL) {
	if (!GMIsConvexPolygon(Pl)) {
#	    ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugCnvxPolyBeforeAfter) {
		    IRIT_INFO_MSG("Polygon before CONVEX Split:\n");
		    CnvxPrintPoly(Pl);
		}
#	    endif /* DEBUG */
	    if (PolyScale > 0.0)
	        UpdatePolyScale(Pl, PolyScale1);
	    PlSplit = GMSplitNonConvexPoly(Pl);
	    if (PolyScale1 > 0.0)
	        UpdatePolyScale(Pl, PolyScale);

	    /* Filters out length edges etc. */
	    GMCleanUpPolygonList(&PlSplit, CONVEX_IRIT_EPS);

	    if (PlSplit != NULL) {	       /* Do we have valid polygons? */
	        if (PolyScale > 0.0) {
		    for (PlTemp = PlSplit;
		         PlTemp != NULL;
			 PlTemp = PlTemp -> Pnext)
		        UpdatePolyScale(PlTemp, PolyScale);
		}
#	        ifdef DEBUG
		    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCnvxPolyBeforeAfter) {
		        IRIT_INFO_MSG("Polygons after CONVEX Split:\n");
			for (PlTemp = PlSplit;
			     PlTemp != NULL;
			     PlTemp = PlTemp -> Pnext)
			    CnvxPrintPoly(PlTemp);
		    }
#	        endif /* DEBUG */

		if (Pl == PObj -> U.Pl)
		    PObj -> U.Pl = PlSplit;			   /* First. */
		else
		    PlPrev -> Pnext = PlSplit;

		PlTemp = PlSplit;
		while (PlTemp -> Pnext != NULL)
		    PlTemp = PlTemp -> Pnext;
		PlTemp -> Pnext = Pl -> Pnext;
		PlPrev = PlTemp;
		IPFreePolygon(Pl);			/* Free old polygon. */
		Pl = PlPrev -> Pnext;
	    }
	    else {
		if (Pl == PObj -> U.Pl) {
		    PObj -> U.Pl = Pl -> Pnext;
		    IPFreePolygon(Pl);			/* Free old polygon. */
		    Pl = PObj -> U.Pl;
		}
		else {
		    PlPrev -> Pnext = Pl -> Pnext;
		    IPFreePolygon(Pl);			/* Free old polygon. */
		    Pl = PlPrev -> Pnext;
		}
	    }
	}
	else {
	    PlPrev = Pl;
	    Pl = Pl -> Pnext;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scale the polygon by PolyScale factor.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:          Polygon to scale.                                           *
*   PolyScale:   Scale factor.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdatePolyScale(IPPolygonStruct *Pl, IrtRType PolyScale)
{
    IPVertexStruct
	*V = Pl -> PVertex;

    do {
	IRIT_PT_SCALE(V -> Coord, PolyScale);

	V = V -> Pnext;
    }
    while (V != NULL && V != Pl -> PVertex);    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to test if the given polygon is convex (by IRIT definition) or     M
* not.	For both closed and open vertex lists.				     M
*   Algorithm: The polygon is convex iff the normals generated from cross    M
* products of two consecutive edges points to the same direction. The same   M
* direction is tested by a positive dot product.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:         To test for convexity.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if PL convex, FALSE otherwise.                          M
* SEE ALSO:                                                                  M
*   GMIsConvexPolygon                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsConvexPolygon2, convexity, files, parser                             M
*****************************************************************************/
int GMIsConvexPolygon2(const IPPolygonStruct *Pl)
{
    IrtRType Size, V1[3], V2[3], LastNormal[3], Normal[3];
    IPVertexStruct *VNext, *VNextNext,
	*V = Pl -> PVertex;

    LastNormal[0] = LastNormal[1] = LastNormal[2] = 0.0;

    do {
	if ((VNext = V -> Pnext) == NULL)
	    VNext = Pl -> PVertex;
	if ((VNextNext = VNext -> Pnext) == NULL)
	    VNextNext = Pl -> PVertex;

	IRIT_PT_SUB(V1, VNext -> Coord, V -> Coord);
	if ((Size = IRIT_PT_LENGTH(V1)) > IRIT_EPS) {
	    Size = 1.0 / Size;
	    IRIT_PT_SCALE(V1, Size);
	}
	IRIT_PT_SUB(V2, VNextNext -> Coord, VNext -> Coord);
	if ((Size = IRIT_PT_LENGTH(V2)) > IRIT_EPS) {
	    Size = 1.0 / Size;
	    IRIT_PT_SCALE(V2, Size);
	}
	IRIT_CROSS_PROD(Normal, V1, V2);

	if (V != Pl -> PVertex) {
	    if (IRIT_PT_LENGTH(Normal) > CONVEX_IRIT_EPS &&
		IRIT_DOT_PROD(Normal, LastNormal) < -CONVEX_IRIT_EPS)
		return FALSE;
	}

	IRIT_PT_COPY(LastNormal, Normal);

	V = VNext;
    }
    while (V != Pl -> PVertex && V != NULL);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to test if the given polygon is convex or not.		     M
* Algorithm: The polygon is convex iff the normals generated from cross      M
* products of two consecutive edges points to the same direction.            M
*   Note a 5 star polygon satisfies this constraint but it is self           M
* intersecting and we assume given polygon is not self intersecting.         M
*   The computed direction is alos verified against the polygon's plane      M
* normal.								     M
*   The routine returns TRUE iff the polygon is convex. In addition the      M
* polygon CONVEX tag (see IPPolygonStruct) is also updated.		     M
*   If the polygon is already marked as convex, nothing is tested!	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        To test its convexity condition.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       TRUE if convex, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvertPolysToTriangles, GMConvexPolyObject, GMConvexPolyObjectN,      M
* GMSplitNonConvexPoly, GMIsConvexPolygon2				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMIsConvexPolygon, convexity, convex polygon                             M
*****************************************************************************/
int GMIsConvexPolygon(IPPolygonStruct *Pl)
{
    int FirstTime = TRUE;
    IrtRType Size,
	NormalSign = 0.0;
    IrtVecType V1, V2, PolyNormal, Normal;
    IPVertexStruct *VNext,
	*V = Pl -> PVertex;

    if (IP_IS_CONVEX_POLY(Pl))
	return TRUE;			       /* Nothing to do around here. */

    /* Copy only A, B, C from Ax+By+Cz+D = 0: */
    IRIT_PT_COPY(PolyNormal, Pl -> Plane);

    do {
	VNext = V -> Pnext;

	IRIT_PT_SUB(V1, VNext -> Coord, V -> Coord);
	if ((Size = IRIT_PT_LENGTH(V1)) > IRIT_UEPS) {
	    Size = 1.0 / Size;
	    IRIT_PT_SCALE(V1, Size);
	}
	IRIT_PT_SUB(V2, VNext -> Pnext -> Coord, VNext -> Coord);
	if ((Size = IRIT_PT_LENGTH(V2)) > IRIT_UEPS) {
	    Size = 1.0 / Size;
	    IRIT_PT_SCALE(V2, Size);
	}
	GMVecCrossProd(Normal, V1, V2);

	if (IRIT_PT_LENGTH(Normal) < CONVEX_IRIT_EPS) {
	    V = VNext;
	    continue;				   /* Skip collinear points. */
	}

	if (FirstTime) {
	    FirstTime = FALSE;
	    NormalSign = IRIT_DOT_PROD(Normal, PolyNormal);
	}
	else if (NormalSign * IRIT_DOT_PROD(Normal, PolyNormal) < 0.0) {
	    IP_RST_CONVEX_POLY(Pl);
	    return FALSE;		  /* Different signs --> not convex. */
	}

	V = VNext;
    }
    while (V != Pl -> PVertex);

    IP_SET_CONVEX_POLY(Pl);

    if (NormalSign < 0.0)
        IPReverseVrtxList(Pl);

    return TRUE;	/* All signs are the same --> the polygon is convex. */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to split non convex polygon into a list of convex ones.	     M
* 1. Remove a polygon from GlblList. If non exists stop.		     M
* 2. Search for non convex corner. If not found stop - polygon is convex.    M
*    Otherwise let the non convex corner found be V(i).			     M
* 3. Fire a ray from V(i) in the opposite direction to V(i-1). Find the      M
*    closest intersection of the ray with polygon boundary P.		     M
* 4. Split the polygon into two at V(i)-P edge and push the two new polygons M
*    on the GlblList.							     M
* 5. Goto 1.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pl:        Non convex polygon to split into convex ones.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:  A list of convex polygons resulting from splitting   M
*                       up Pl.                                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMConvertPolysToTriangles, GMConvexPolyObject, GMConvexPolyObjectN,	     M
*   GMIsConvexPolygon						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSplitNonConvexPoly, convexity, convex polygon                          M
*****************************************************************************/
IPPolygonStruct *GMSplitNonConvexPoly(IPPolygonStruct *Pl)
{
    int IsConvex;
    IrtRType Size;
    IPPolygonStruct *GlblList, *Pl1, *Pl2,
	*GlblSplitPl = NULL;
    IrtVecType V1, V2, PolyNormal, Normal;
    IPVertexStruct *V, *VNext;

    TestConvexityDir(Pl);

    GlblList = IPAllocPolygon(0, IPCopyVertexList(Pl -> PVertex), NULL);
    IRIT_PLANE_COPY(GlblList -> Plane, Pl -> Plane);
    IP_SET_PLANE_POLY(GlblList);
    GlblList -> Attr = IP_ATTR_COPY_ATTRS(Pl -> Attr);

    /* Copy only A, B, C from Ax+By+Cz+D = 0 plane equation: */
    IRIT_PT_COPY(PolyNormal, Pl -> Plane);

    while (GlblList != NULL) {
	Pl = GlblList;
	GlblList = GlblList -> Pnext;
	Pl -> Pnext = NULL;

#	ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCnvxPolyBeforeAfter) {
	        IRIT_INFO_MSG("Examined 0 polygon:\n");
		CnvxPrintPoly(Pl);
	    }
#	endif /* DEBUG */

	/* Filters out length edges etc. */
	GMCleanUpPolygonList(&Pl, CONVEX_IRIT_EPS);
	if (Pl == NULL)
	    continue;

#	ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCnvxPolyBeforeAfter) {
	        IRIT_INFO_MSG("Examined polygon:\n");
		CnvxPrintPoly(Pl);
	    }
#	endif /* DEBUG */

	IsConvex = TRUE;
	V = Pl -> PVertex;
	do {
	    VNext = V -> Pnext;

	    IRIT_PT_SUB(V1, VNext -> Coord, V -> Coord);
 	    if ((Size = IRIT_PT_LENGTH(V1)) > IRIT_UEPS) {
		Size = 1.0 / Size;
		IRIT_PT_SCALE(V1, Size);
	    }
	    IRIT_PT_SUB(V2, VNext -> Pnext -> Coord, VNext -> Coord);
 	    if ((Size = IRIT_PT_LENGTH(V2)) > IRIT_UEPS) {
		Size = 1.0 / Size;
		IRIT_PT_SCALE(V2, Size);
	    }
	    GMVecCrossProd(Normal, V1, V2);
	    if (IRIT_PT_LENGTH(Normal) < CONVEX_IRIT_EPS) {
		V = VNext;
		continue;			   /* Skip collinear points. */
	    }

	    if (IRIT_DOT_PROD(Normal, PolyNormal) < 0.0 &&
		SplitPolyIntoTwo(Pl, V, &Pl1, &Pl2)) {
		Pl -> PVertex = NULL; /* Dont free vertices - used in Pl1/2. */
		IPFreePolygon(Pl);
		Pl1 -> Pnext = GlblList;       /* Push polygons on GlblList. */
		GlblList = Pl1;
		Pl2 -> Pnext = GlblList;
		GlblList = Pl2;
#	        ifdef DEBUG
		    IRIT_IF_DEBUG_ON_PARAMETER(_DebugCnvxPolyBeforeAfter) {
		        IRIT_INFO_MSG("First splitted polygons:\n");
			CnvxPrintPoly(Pl1);
			IRIT_INFO_MSG("Second splitted polygons:\n");
			CnvxPrintPoly(Pl2);
		    }
#	        endif /* DEBUG */

		IsConvex = FALSE;
		break;
	    }

	    V = VNext;
	}
	while (V != Pl -> PVertex);

	if (IsConvex) {
	    IP_SET_CONVEX_POLY(Pl);
	    Pl -> Pnext = GlblSplitPl;
	    GlblSplitPl = Pl;
	}
    }

    return GlblSplitPl;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Split polygon Pl at the vertex specified by V -> Pnext, given V, into    *
* two, by firing a ray from V -> Pnext in direction approximately orthogonal *
* to V and finding closest intersection point, P, with the polygon Pl.       *
* (V -> Pnext, P) is the edge to split the polygon at.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:        Polygon to split at vertex V -> Pnext.                        *
*   V:         One vertex before the vertex to split Pl at.                  *
*   Pl1, Pl2:  Where the two new polygons should end up.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:       TRUE if succesful, FALSE otehrwise.                           *
*****************************************************************************/
static int SplitPolyIntoTwo(IPPolygonStruct *Pl,
			    IPVertexStruct *V,
			    IPPolygonStruct **Pl1,
			    IPPolygonStruct **Pl2)
{
    IrtPtType Vl1, Vl2, Vl, PInter;
    IPVertexStruct *VInter, *VNew1, *VNew2;

    IRIT_PT_SUB(Vl1, V -> Pnext -> Coord, V -> Coord);
    IRIT_PT_SUB(Vl2, V -> Pnext -> Pnext -> Coord, V -> Pnext -> Coord);
    IRIT_PT_NORMALIZE(Vl1);
    IRIT_PT_NORMALIZE(Vl2);
    IRIT_PT_SUB(Vl, Vl1, Vl2);
    IRIT_PT_NORMALIZE(Vl);

    /* See if can update the ray to point to a vertex in the polygon. */
    if (GlblUpdateRayDirToPointToVertex)
	UpdateRayDirToPointToVertex(V -> Pnext, Vl, Pl);

    VInter = FindRayPolyInter(Pl, V -> Pnext, Vl, PInter);
    V = V -> Pnext;

    if (VInter == NULL || VInter == V || VInter -> Pnext == V)
	return FALSE;

    /* Make the two polygon's vertices lists. */
    VNew1 = IPAllocVertex(V -> Tags, NULL, V -> Pnext);
    IRIT_PT_COPY(VNew1 -> Coord, V -> Coord);
    if (GlblHandleNormals) {
        IRIT_PT_COPY(VNew1 -> Normal, V -> Normal);
	VNew1 -> Attr = IP_ATTR_COPY_ATTRS(V -> Attr);
	IP_SET_NORMAL_VRTX(VNew1);
    }
    IP_SET_INTERNAL_VRTX(V);
    if (IRIT_PT_APX_EQ_EPS(VInter -> Coord, PInter, CONVEX_IRIT_EPS)) {
	/* Intersection points is close to VInter point. */
	VNew2 = IPAllocVertex(VInter -> Tags, NULL, VInter -> Pnext);
	IRIT_PT_COPY(VNew2 -> Coord, VInter -> Coord);
	if (GlblHandleNormals) {
	    IRIT_PT_COPY(VNew2 -> Normal, VInter -> Normal);
	    VNew2 -> Attr = IP_ATTR_COPY_ATTRS(VInter -> Attr);
	    IP_SET_NORMAL_VRTX(VNew2);
	}
	VInter -> Pnext = VNew1;
	IP_SET_INTERNAL_VRTX(VInter);
	V -> Pnext = VNew2;
    }
    else if (IRIT_PT_APX_EQ_EPS(VInter -> Pnext -> Coord, PInter, CONVEX_IRIT_EPS)) {
	/* Intersection points is close to VInter -> Pnext point. */
	VNew2 = IPAllocVertex(VInter -> Pnext -> Tags,
			      NULL, VInter -> Pnext -> Pnext);
	IRIT_PT_COPY(VNew2 -> Coord, VInter -> Pnext -> Coord);
	if (GlblHandleNormals) {
	    IRIT_PT_COPY(VNew2 -> Normal, VInter -> Pnext -> Normal);
	    VNew2 -> Attr = IP_ATTR_COPY_ATTRS(VInter -> Pnext -> Attr);
	    IP_SET_NORMAL_VRTX(VNew2);
	}
	VInter -> Pnext -> Pnext = VNew1;
	IP_SET_INTERNAL_VRTX(VInter -> Pnext);
	V -> Pnext = VNew2;
    }
    else {
	/* PInter is in the middle of (VInter, VInter -> Pnext) edge: */
	VNew2 = IPAllocVertex(VInter -> Tags, NULL, VInter -> Pnext);
	IRIT_PT_COPY(VNew2 -> Coord, PInter);
	VInter -> Pnext = IPAllocVertex2(VNew1);
	IRIT_PT_COPY(VInter -> Pnext -> Coord, PInter);
	if (GlblHandleNormals) {
	    GMInterpVrtxNrmlBetweenTwo(VNew2, VInter, VNew2 -> Pnext);
	    GMInterpVrtxRGBBetweenTwo(VNew2, VInter, VNew2 -> Pnext);
	    GMInterpVrtxUVBetweenTwo(VNew2, VInter, VNew2 -> Pnext);
	    IP_SET_NORMAL_VRTX(VNew2);

	    IRIT_PT_COPY(VInter -> Pnext -> Normal, VNew2 -> Normal);
	    VInter -> Pnext -> Attr = IP_ATTR_COPY_ATTRS(VNew2 -> Attr);
	    IP_SET_NORMAL_VRTX(VInter -> Pnext);
	}
	IP_SET_INTERNAL_VRTX(VInter -> Pnext);
	V -> Pnext = VNew2;
    }

    *Pl1 = IPAllocPolygon(0, VNew1, NULL);
    IRIT_PLANE_COPY((*Pl1) -> Plane, Pl -> Plane);
    IP_SET_PLANE_POLY(*Pl1);
    (*Pl1) -> Attr = IP_ATTR_COPY_ATTRS(Pl -> Attr);

    *Pl2 = IPAllocPolygon(0, VNew2, NULL);
    IRIT_PLANE_COPY((*Pl2) -> Plane, Pl -> Plane);
    IP_SET_PLANE_POLY(*Pl2);
    (*Pl2) -> Attr = IP_ATTR_COPY_ATTRS(Pl -> Attr);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes number of vertices between V1 and V2 along the shortest path.   *
*                                                                            *
* PARAMETERS:                                                                *
*   V1, V2:   The two vertices in one vertex loop of a polygon.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:                                                                     *
*****************************************************************************/
static int VrtxDistanceInPoly(IPVertexStruct *V1, IPVertexStruct *V2)
{
    int VLen = 1;
    IPVertexStruct
	*V1Orig = V1,
	*V2Orig = V2;

    for ( ;
	  V1 != V2Orig && V2 != V1Orig;
	  V1 = V1 -> Pnext, V2 = V2 -> Pnext, VLen++);

    return VLen;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   See if one can update the ray in direction RayDir emanating from         *
* position VRay to point to a real vertex in polygon Pl.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   VRay:     Source position of the ray.                                    *
*   RayDir:   Unit direction of ray from V.                                  *
*   Pl:       Polygon to search for best point to point to instead of Dir.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateRayDirToPointToVertex(IPVertexStruct *VRay,
					IrtVecType RayDir,
					IPPolygonStruct *Pl)
{
    int MinVDist = 32767;
    IrtRType Len, CosAngle,
	MaxCosAngle = 0;			          /* cos(M_PI / 2). */
    IrtVecType Dir, BestDir;
    IPVertexStruct 
	*V = Pl -> PVertex;

    do {
        IRIT_VEC_SUB(Dir, V -> Coord, VRay -> Coord);
	Len = IRIT_VEC_LENGTH(Dir);

	if (Len > IRIT_EPS &&
	    ((CosAngle = IRIT_DOT_PROD(Dir, RayDir) / Len) >= MaxCosAngle)) {
	    int VDist = VrtxDistanceInPoly(VRay, V);

	    if (IRIT_APX_EQ(CosAngle, MaxCosAngle)) {
	        if (VDist < MinVDist) {
		    IRIT_VEC_COPY(BestDir, Dir);
		    MaxCosAngle = CosAngle;
		    MinVDist = VDist;
		}
	    }
	    else {
	        IRIT_VEC_COPY(BestDir, Dir);
		MaxCosAngle = CosAngle;
		MinVDist = VDist;
	    }
	}

        V = V -> Pnext;
    }
    while (V != NULL && V != Pl -> PVertex);

    if (MaxCosAngle > 0) {
	IRIT_VEC_NORMALIZE(BestDir);
	IRIT_VEC_COPY(RayDir, BestDir);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds where a ray first intersect a given polygon. The ray starts at one *
* of the polygon's vertices and so distance less than IRIT_EPS is ignored.   *
*   Returns the vertex V in which (V, V -> Pnext) has the closest	     *
* intersection with the ray PRay, DRay at Inter.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:         Polygon to test for intersection with ray.                   *
*   VRay:       Origin of ray.                                               *
*   RayDir:     Direction of ray.                                            *
*   PInter:     location of intersection is to be placed herein.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexStruct *:   Vertex V such that edge (V, V -> Pnext) has the      *
*                       needed intersection.                                 *
*****************************************************************************/
static IPVertexStruct *FindRayPolyInter(IPPolygonStruct *Pl,
					IPVertexStruct *VRay,
					IrtPtType RayDir,
					IrtPtType PInter)
{
    int MinVDist = 32767;
    IrtRType t1, t2,
	MinT = IRIT_INFNTY;
    IrtPtType Vl, Ptemp1, Ptemp2, PRay;
    IPVertexStruct *VNext,
	*V = Pl -> PVertex,
	*VInter = NULL;

    IRIT_PT_COPY(PRay, VRay -> Coord);

    do {
	VNext = V -> Pnext;
	if (V != VRay && VNext != VRay) {
	    IRIT_PT_SUB(Vl, VNext -> Coord, V -> Coord);
	    if (GMDistPointLine(PRay, V -> Coord, Vl) > IRIT_UEPS) {
		/* Only if the point the ray is shoot from is not on line: */
		GM2PointsFromLineLine(PRay, RayDir, V -> Coord, Vl,
				      Ptemp1, &t1, Ptemp2, &t2);
		if (IRIT_PT_PT_DIST_SQR(Ptemp1, Ptemp2) <
						 IRIT_SQR(IRIT_UEPS * 10.0) &&
		    t1 > IRIT_UEPS && (t1 <= MinT || IRIT_APX_EQ(t1, MinT)) &&
		    (t2 <= 1.0 || IRIT_APX_EQ(t2, 1.0)) &&
		    (t2 >= 0.0 || IRIT_APX_EQ(t2, 0.0))) {
		    int VDist = VrtxDistanceInPoly(VRay, V);

		    if (IRIT_APX_EQ(t1, MinT)) {
		        if (VDist < MinVDist) {
			    IRIT_PT_COPY(PInter, Ptemp2);
			    VInter = V;
			    MinT = t1;
			    MinVDist = VDist;
			}
		    }
		    else {
		        IRIT_PT_COPY(PInter, Ptemp2);
			VInter = V;
			MinT = t1;
			MinVDist = VDist;
		    }
		}
	    }
	}
	V = VNext;
    }
    while (V != Pl -> PVertex && V -> Pnext != NULL);

    return VInter;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test convexity direction - a cross product of two edges of a convex      *
* corner of the polygon should point in the normal direction. if this is not *
* the case - the polygon vertices are reversed.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:         To test of convexity direction.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TestConvexityDir(IPPolygonStruct *Pl)
{
    int Coord = 0;
    IrtVecType V1, V2, Normal;
    IPVertexStruct *V, *VExtrem;

    /* Find the minimum component direction of the normal and used that axes */
    /* to find an extremum point on the polygon - that extrmum point must be */
    /* a convex corner so we can find the normal direction of convex corner. */
    if (IRIT_FABS(Pl -> Plane[1]) < IRIT_FABS(Pl -> Plane[Coord]))
	Coord = 1;
    if (IRIT_FABS(Pl -> Plane[2]) < IRIT_FABS(Pl -> Plane[Coord]))
	Coord = 2;
    V = VExtrem = Pl -> PVertex;
    do {
	if (V -> Coord[Coord] > VExtrem -> Coord[Coord])
	    VExtrem = V;
	V = V -> Pnext;
    }
    while (V != Pl -> PVertex && V != NULL);

    /* Make sure next vertex is not at the extremum value: */
    V = VExtrem;
    while (IRIT_APX_EQ(VExtrem -> Coord[Coord], VExtrem -> Pnext -> Coord[Coord])) {
	VExtrem = VExtrem -> Pnext;
	if (V == VExtrem) {
	    /* Cycled the entire polygon and is too tiny to process all the  */
	    /* vertices are pretty much the same value in axes [Coord].      */
	    return;
	}
    }

    /* O.K. V form a convex corner - evaluate its two edges cross product:   */
    for (V = Pl -> PVertex; V -> Pnext != VExtrem; V = V -> Pnext); /* Prev. */
    IRIT_PT_SUB(V1, VExtrem -> Coord, V -> Coord);
    IRIT_PT_SUB(V2, VExtrem -> Pnext -> Coord, VExtrem -> Coord);
    GMVecCrossProd(Normal, V1, V2);

    if (IRIT_DOT_PROD(Normal, Pl -> Plane) < 0.0)
	IPReverseVrtxList(Pl);
}


#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the content of the given vertex list, to standard output.	     *
*                                                                            *
* PARAMETERS:                                                                *
*   P:          Polygon to print its vertex list.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CnvxPrintPoly(IPPolygonStruct *P)
{
    IPVertexStruct
	*VFirst = P -> PVertex,
	*V = VFirst;

    if (V == NULL)
	return;

    printf("VERTEX LIST:\n");
    do {
	printf("%12lg %12lg %12lg, Tags = %02x\n",
	    V -> Coord[0], V -> Coord[1], V -> Coord[2], V -> Tags);
	V = V -> Pnext;
    }
    while (V != NULL && V != VFirst);

    if (V == NULL)
	printf("Loop is not closed (NULL terminated)\n");
}

#endif /* DEBUG */

