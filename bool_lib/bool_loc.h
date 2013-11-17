/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*   Definitions, local to modules, of Boolean operation modules:	     *
*****************************************************************************/

#ifndef BOOL_LOC_H
#define BOOL_LOC_H

#include "bool_lib.h"

/*   The following structure is used to keep the intersecting segments of    */
/* each polygons, which the other object polygons. they are saved as a       */
/* list of segments, which can form a closed loop (if totally internal) or   */
/* an open one (in which the two ends intersects the polygon boundaries).    */
/*   Note all the polygons of the two given objects must be convex!	     */
typedef struct InterSegmentStruct {
    IrtPtType PtSeg[2];		       /* The two end points of the segment. */
    /* If intersect polygon vertex, point on it. If internal to poly, NULL.  */
    IPVertexStruct *V[2];
    IPPolygonStruct *Pl;       /* Point on the (other) intersecting polygon. */
    struct InterSegmentStruct *Pnext;
} InterSegmentStruct;

/* Used to hold list of InterSegment polylines: */
typedef struct InterSegListStruct {
    InterSegmentStruct *PISeg,		  /* Point to InterSegment Polyline. */
	*PISegMaxX;			   /* Used in closed loops handling. */
    struct InterSegListStruct *Pnext;		  /* Point to next polyline. */
} InterSegListStruct;

/* Used in sorting of open loops: */
typedef struct SortOpenStruct {
    IrtRType Key;
    InterSegListStruct *PLSeg;		/* Point to open loop with this key. */
    struct SortOpenStruct *Pnext;		   /* Point to next in list. */
} SortOpenStruct;

/* The following are temporary flags used to mark the original vertices in   */
/* the resulting object. Used to detected edges originated in the input	     */
/* object, and used to propagate the adjacencies.			     */
#define ORIGINAL_TAG 0x10

#define	IS_ORIGINAL_VRTX(Vrtx)	((Vrtx) -> Tags & ORIGINAL_TAG)
#define	SET_ORIGINAL_VRTX(Vrtx)	((Vrtx) -> Tags |= ORIGINAL_TAG)
#define	RST_ORIGINAL_VRTX(Vrtx)	((Vrtx) -> Tags &= ~ORIGINAL_TAG)


/* The following are temporary flags used to mark the polygons in the	     */
/* adjacencies propagation. Can use bit 4-7 of IPPolygonStruct Tags only.    */
#define COMPLETE_TAG   0x10 /* Complete Tag - Polygon has no intersection.   */
#define IN_OUTPUT_TAG  0x20 /* InOutput Tag - Polygon should be in output.   */
#define ADJ_PUSHED_TAG 0x40 /* AdjPushed Tag - Polygon has been pushed.	     */
#define COPLANAR_TAG   0x80 /* Set if this poly found coplanar in boolean.   */
 
#define	IS_COMPLETE_POLY(Poly)		((Poly) -> Tags & COMPLETE_TAG)
#define	SET_COMPLETE_POLY(Poly)		((Poly) -> Tags |= COMPLETE_TAG)
#define	RST_COMPLETE_POLY(Poly)		((Poly) -> Tags &= ~COMPLETE_TAG)
#define	IS_INOUTPUT_POLY(Poly)		((Poly) -> Tags & IN_OUTPUT_TAG)
#define	SET_INOUTPUT_POLY(Poly)		((Poly) -> Tags |= IN_OUTPUT_TAG)
#define	RST_INOUTPUT_POLY(Poly)		((Poly) -> Tags &= ~IN_OUTPUT_TAG)
#define	IS_ADJPUSHED_POLY(Poly)		((Poly) -> Tags & ADJ_PUSHED_TAG)
#define	SET_ADJPUSHED_POLY(Poly)	((Poly) -> Tags |= ADJ_PUSHED_TAG)
#define	RST_ADJPUSHED_POLY(Poly)	((Poly) -> Tags &= ~ADJ_PUSHED_TAG)
#define	IS_COPLANAR_POLY(Poly)		((Poly) -> Tags & COPLANAR_TAG)
#define	SET_COPLANAR_POLY(Poly)		((Poly) -> Tags |= COPLANAR_TAG)
#define	RST_COPLANAR_POLY(Poly)		((Poly) -> Tags &= ~COPLANAR_TAG)

#define	RST_BOOL_FLAGS_POLY(Poly)	((Poly) -> Tags &= ~(COMPLETE_TAG | \
							     IN_OUTPUT_TAG | \
							     ADJ_PUSHED_TAG | \
							     COPLANAR_TAG))

#define ADJ_STACK_SIZE	131071		/* Adjacency of polygons stack size. */

/* FatalError types are defined below. Usually, they will cause empty result.*/
#define FTL_BOOL_NO_INTER	1	/* No intersection between operands. */
#define FTL_BOOL_CTRL_BRK	2	       /* Control break was pressed. */
#define FTL_BOOL_FPE		3		    /* Floating point error. */

#define BOOL_MID_BLEND		0.50123456789 /* Off mid point for stability!*/

#ifdef IRIT_DOUBLE
#define BOOL_IRIT_EPS		1e-10
#define BOOL_IRIT_EPS_SAME_PT	1e-4
#else
#define BOOL_IRIT_EPS		1e-6
#define BOOL_IRIT_EPS_SAME_PT	1e-4
#endif /* IRIT_DOUBLE */

#define BOOL_IRIT_REL_EPS	  (_BoolGlobalScale * BOOL_IRIT_EPS)
#define BOOL_IRIT_REL_EPS_SAME_PT (_BoolGlobalScale * BOOL_IRIT_EPS_SAME_PT)

#define BOOL_GM_BASIC_EPS	  (_BoolGlobalScale * IRIT_UEPS)

#define IRIT_BOOL_APX_EQ(x, y)  (IRIT_FABS((x) - (y)) < BOOL_IRIT_REL_EPS)
#define IRIT_BOOL_APX_EQ_EPS(x, y, Eps)	(IRIT_FABS((x) - (y)) < (Eps))
#define BOOL_ON_PLANE(V, Pl)		(IRIT_FABS(Pl[0] * V -> Coord[0] + \
					           Pl[1] * V -> Coord[1] + \
					           Pl[2] * V -> Coord[2] + \
					           Pl[3]) < BOOL_IRIT_EPS)
#define BOOL_IRIT_PT_APX_EQ(Pt1, Pt2)	(IRIT_BOOL_APX_EQ(Pt1[0], Pt2[0]) && \
					 IRIT_BOOL_APX_EQ(Pt1[1], Pt2[1]) && \
					 IRIT_BOOL_APX_EQ(Pt1[2], Pt2[2]))
#define IRIT_BOOL_PT_APX_EQ_EPS(Pt1, Pt2, Eps) \
				     (IRIT_BOOL_APX_EQ_EPS(Pt1[0], Pt2[0], Eps) && \
				      IRIT_BOOL_APX_EQ_EPS(Pt1[1], Pt2[1], Eps) && \
				      IRIT_BOOL_APX_EQ_EPS(Pt1[2], Pt2[2], Eps))

#define BOOL_L1_NORMALIZE(V)	{ IrtRType \
				     RV = 1 / MAX(IRIT_FABS(V[0]), \
					          MAX(IRIT_FABS(V[1]), \
						      IRIT_FABS(V[2]))); \
				  IRIT_PT_SCALE(V, RV); \
				}

#define BOOL_FATAL_ERROR(Msg)	_BoolActiveFatalErrorFunc(Msg)

IRIT_GLOBAL_DATA_HEADER int
    BoolHandleCoplanarPoly,	        /* Whether to handle coplanar polys. */
    BoolFoundCoplanarPoly,           /* If coplanar polygons actually found. */
    BoolOutputInterCurve,		/* Kind of output from Boolean oper. */
    BoolParamSurfaceUVVals;		 /* Should we interpolate UV values. */

IRIT_GLOBAL_DATA_HEADER IrtRType
    _BoolGlobalScale;

IRIT_GLOBAL_DATA_HEADER BoolFatalErrorFuncType
    _BoolActiveFatalErrorFunc;

/* Prototypes of local functions in Bool-Hi.c module: */
void FatalBooleanError(int ErrorType);
IPPolygonStruct *BooleanComputeRotatedPolys(IPPolygonStruct *Pl,
					    int CopyPl,
					    IrtHmgnMatType RotMat);

/* Prototypes of local functions in Bool-Low.c module: */
IPObjectStruct *BooleanLow1Out2(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanLow1In2(IPObjectStruct *PObj1, IPObjectStruct *PObj2);
IPObjectStruct *BooleanLowSelfInOut(IPObjectStruct *PObj, int InOut);
void BoolSortOpenInterList(IPPolygonStruct *Pl, InterSegListStruct **POpen);
int BoolLoopsFromInterList(IPPolygonStruct *Pl,
			   InterSegListStruct **PClosed,
			   InterSegListStruct **POpen);
IPVertexStruct *BoolCutPolygonAtRay(IPPolygonStruct *Pl, IrtPtType Pt);
IPObjectStruct *BoolExtractPolygons(IPObjectStruct *PObj, int AinB);
void BooleanPrepObject(IPObjectStruct *PObj);

#endif /* BOOL_LOC_H */
