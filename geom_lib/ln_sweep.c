/******************************************************************************
* Line plane sweep algorithm implementation - actual code.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by:  Gershon Elber                              Ver 1.0, June 1993  *
******************************************************************************/

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_loc.h"

#define LS_XBBOX_OVERLAP(L1, L2) (L1 -> _MaxVals[0] > L2 -> _MinVals[0] && \
				  L2 -> _MaxVals[0] > L1 -> _MinVals[0])

#if defined(ultrix) && defined(mips)
static int LsSortCompare(VoidPtr Ptr1, VoidPtr Ptr2);
#else
static int LsSortCompare(const VoidPtr Ptr1, const VoidPtr Ptr2);
#endif /* ultrix && mips (no const support) */

static void LsInitialize(GMLsLineSegStruct **Lines);
static void LsIntersect(GMLsLineSegStruct *Lines);
static int LsIntersectOne(GMLsLineSegStruct *L1, GMLsLineSegStruct *L2,
			  IrtRType *t1, IrtRType *t2);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes all intersections between all given lines, in the plane.          M
*   The Lines segments are updated so the Inters slot holds the list of      M
* intersections with the other segments, NULL if none.			     M
*   Returned is a list with possibly  a different order than that is given.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Lines:    To compute all intersections against each other, in the plane. M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMLineSweep, line sweep, line line intersections                         M
*****************************************************************************/
void GMLineSweep(GMLsLineSegStruct **Lines)
{
    if (Lines == NULL || *Lines == NULL)
	return;

    LsInitialize(Lines);
    LsIntersect(*Lines);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* A comparison routine for sorting two LsLineSegStructs with according to    *
* _MinVals[1].	                                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Ptr1, Ptr2:  Two pointers to two  LsLineSegStruct structures to compare. *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, <0 as a result of the relation between the two structures. *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int LsSortCompare(VoidPtr Ptr1, VoidPtr Ptr2)
#else
static int LsSortCompare(const VoidPtr Ptr1, const VoidPtr Ptr2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = ((*(GMLsLineSegStruct **) Ptr1)) -> _MinVals[1] -
	       ((*(GMLsLineSegStruct **) Ptr2)) -> _MinVals[1];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Initialize the necessary data structures for the plane sweep algorithm.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Lines:    To intersect against each other. Do initialize Lines in place. *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void LsInitialize(GMLsLineSegStruct **Lines)
{
    int i;
    GMLsLineSegStruct *Line, **LineArray, **l;

    for (i = 0, Line = *Lines; Line != NULL; Line = Line -> Pnext, i++) {
	IrtRType Size;

	Line -> _MinVals[0] = IRIT_MIN(Line -> Pts[0][0], Line -> Pts[1][0]);
	Line -> _MinVals[1] = IRIT_MIN(Line -> Pts[0][1], Line -> Pts[1][1]);
	Line -> _MaxVals[0] = IRIT_MAX(Line -> Pts[0][0], Line -> Pts[1][0]);
	Line -> _MaxVals[1] = IRIT_MAX(Line -> Pts[0][1], Line -> Pts[1][1]);

	/* Computes the vector from first point to second. */
	Line -> _Vec[0] = Line -> Pts[1][0] - Line -> Pts[0][0];
	Line -> _Vec[1] = Line -> Pts[1][1] - Line -> Pts[0][1];

	/* Computes the line equation for the segment as Ax + By + C = 0,    */
	/* sqrt(A^2 + B^2) = 1.						     */
	Line -> _ABC[0] = Line -> _Vec[1];
	Line -> _ABC[1] = -Line -> _Vec[0];
	Size = sqrt(IRIT_SQR(Line -> _ABC[0]) + IRIT_SQR(Line -> _ABC[1]));
	if (Size > 0.0) {
	    Line -> _ABC[0] /= Size;
	    Line -> _ABC[1] /= Size;
	    Line -> _ABC[2] = -(Line -> _ABC[0] * Line -> Pts[0][0] +
				Line -> _ABC[1] * Line -> Pts[0][1]);
	}
	else {
	    /* Zero length segment. Sets the line equation to never yield an */
	    /* intersection.						     */
	    Line -> _ABC[0] = Line -> _ABC[1] = 0.0;
	    Line -> _ABC[2] = 1.0;
	}

	Line -> Inters = NULL;
    }

    LineArray = (GMLsLineSegStruct **)
	IritMalloc(sizeof(GMLsLineSegStruct *) * i);
    for (Line = *Lines, l = LineArray; Line != NULL; Line = Line -> Pnext)
	*l++ = Line;
    qsort(LineArray, i, sizeof(GMLsLineSegStruct *), LsSortCompare);
    *Lines = *LineArray;
    for (l = LineArray; --i; l++)
	l[0] -> Pnext = l[1];
    l[0] -> Pnext = NULL;
    IritFree(LineArray);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The actual intersections. Not a real plane sweep, but close...	     *
* Marches along the given list and intersect every line with all lines in    *
* the domain that can affect it.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   Lines:    To compute all intersections against each other, in the plane. M
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void LsIntersect(GMLsLineSegStruct *Lines)
{
    IRIT_STATIC_DATA int
	IdNumber = 0;
    GMLsLineSegStruct *Line, *Line2;

    for (Line = Lines; Line -> Pnext != NULL; Line = Line -> Pnext) {
	IrtRType t, t2,
	    MaxY = Line -> _MaxVals[1];

	for (Line2 = Line -> Pnext; Line2 != NULL; Line2 = Line2 -> Pnext) {
	    if (Line2 -> _MinVals[1] > MaxY)
		break; /* Cannot intersect any more */

	    if (Line -> Id != Line2 -> Id &&
		LS_XBBOX_OVERLAP(Line, Line2) &&
		LsIntersectOne(Line, Line2, &t, &t2)) {
		GMLsIntersectStruct
		    *Inter = (GMLsIntersectStruct *)
			IritMalloc(sizeof(GMLsIntersectStruct)),
		    *Inter2 = (GMLsIntersectStruct *)
			IritMalloc(sizeof(GMLsIntersectStruct));

		Inter -> t = t;
		Inter -> OtherT = t2;
		Inter -> OtherSeg = Line2;
		Inter -> Id = IdNumber;
		Inter -> Pnext = Line -> Inters;
		Line -> Inters = Inter;

		Inter2 -> t = t2;
		Inter2 -> OtherT = t;
		Inter2 -> OtherSeg = Line;
		Inter2 -> Id = IdNumber++;
		Inter2 -> Pnext = Line2 -> Inters;
		Line2 -> Inters = Inter2;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a single intersection between two line segments.		     *
*   Returns TRUE if found an intersection and sets t1/t2 to the parameter    *
* values of the intersection (t == 0 for first point, t == 1 for second).    *
*                                                                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   L1:       First line to compute intersection with L2.                    *
*   L2:       Second line to compute intersection with L1.                   *
*   t1:       Parameter value of intersection location along L1.	     *
*   t2:       Parameter value of intersection location along L2.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      TRUE if L1 and L2 actualy intersect, FALSE otherwise.          *
*****************************************************************************/
static int LsIntersectOne(GMLsLineSegStruct *L1,
			  GMLsLineSegStruct *L2,
			  IrtRType *t1,
			  IrtRType *t2)
{
    IrtRType Dist11, Dist12, Dist21, Dist22, XDiff, YDiff, Det;

    Dist11 = L1 -> _ABC[0] * L2 -> Pts[0][0] +
	     L1 -> _ABC[1] * L2 -> Pts[0][1] +
	     L1 -> _ABC[2];
    Dist12 = L1 -> _ABC[0] * L2 -> Pts[1][0] +
	     L1 -> _ABC[1] * L2 -> Pts[1][1] +
	     L1 -> _ABC[2];
    if (Dist11 * Dist12 > 0.0)/* L2's two end points are on same size of L1. */
	return FALSE;

    Dist21 = L2 -> _ABC[0] * L1 -> Pts[0][0] +
	     L2 -> _ABC[1] * L1 -> Pts[0][1] +
	     L2 -> _ABC[2];
    Dist22 = L2 -> _ABC[0] * L1 -> Pts[1][0] +
	     L2 -> _ABC[1] * L1 -> Pts[1][1] +
	     L2 -> _ABC[2];
    if (Dist21 * Dist22 > 0.0)/* L1's two end points are on same size of L2. */
	return FALSE;

    /* Solve the following linear system for t1 and t2.		      */
    /*								      */
    /* L1X1 + t1 L1Vx = L2X1 + t2 L2Vx				      */
    /* L1Y1 + t1 L1Vy = L2Y1 + t2 L2Vy				      */
    /*								      */
    /* or,							      */
    /*								      */
    /* t1 L1Vx - t2 L2Vx = L2X1 - L1X1				      */
    /* t1 L1Vy - t2 L2Vy = L2Y1 - L1Y1				      */
    /*								      */
    Det = L1 -> _Vec[1] * L2 -> _Vec[0] - L1 -> _Vec[0] * L2 -> _Vec[1];
    if (Det == 0.0)
	return FALSE;

    XDiff = L2 -> Pts[0][0] - L1 -> Pts[0][0];
    YDiff = L2 -> Pts[0][1] - L1 -> Pts[0][1];

    *t1 = (YDiff * L2 -> _Vec[0] - XDiff * L2 -> _Vec[1]) / Det;
    *t2 = (L1 -> _Vec[0] * YDiff - L1 -> _Vec[1] * XDiff) / Det;

    return *t1 > 0.0 && *t1 <= 1.0 && *t2 > 0.0 && *t2 <= 1.0;
}
