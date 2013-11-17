/******************************************************************************
* ScanCnvt.c - Scan conversion related routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 1993.					      *
******************************************************************************/

#include "irit_sm.h"
#include "allocate.h"
#include "iritprsr.h"
#include "geom_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Visits all pixels of the given triangle and invokes ApplyFunc on each    M
* such pixel.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt1, Pt2, Pt3:    The three coordinates of the triangle.                 M
*   ApplyFunc:        The function that will be invoked on every pixel       M
*		      that is visited in this triangle.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMScanConvertTriangle			                             M
*****************************************************************************/
void GMScanConvertTriangle(int Pt1[2],
			   int Pt2[2],
			   int Pt3[2],
			   GMScanConvertApplyFuncType ApplyFunc)
{
    int x, y, *Min, *Mid, *Max;
    IrtRType x1, x2, Dx1, Dx2;

    /* Sort the three points by Y: */
    if (Pt1[1] <= Pt2[1] && Pt1[1] <= Pt3[1]) {
        Min = Pt1;
	if (Pt2[1] <= Pt3[1]) {
	    Mid = Pt2;
	    Max = Pt3;
	}
	else {
	    Mid = Pt3;
	    Max = Pt2;
	}
    }
    else if (Pt2[1] <= Pt1[1] && Pt2[1] <= Pt3[1]) {
        Min = Pt2;
	if (Pt1[1] <= Pt3[1]) {
	    Mid = Pt1;
	    Max = Pt3;
	}
	else {
	    Mid = Pt3;
	    Max = Pt1;
	}
    }
    else {
        Min = Pt3;
	if (Pt1[1] <= Pt2[1]) {
	    Mid = Pt1;
	    Max = Pt2;
	}
	else {
	    Mid = Pt2;
	    Max = Pt1;
	}
    }

    /* Scan convert between Min and Mid. */
    Dx1 = (Max[0] - Min[0]) / ((IrtRType) (Max[1] - Min[1]));
    if (Mid[1] != Min[1]) {
        x1 = x2 = Min[0];
        Dx2 = (Mid[0] - Min[0]) / ((IrtRType) (Mid[1] - Min[1]));
	for (y = Min[1]; y <= Mid[1]; y++) {
	    int XMin = (int) (IRIT_MIN(x1, x2) + 0.5),
	        XMax = (int) (IRIT_MAX(x1, x2) + 0.5);
	
	    for (x = XMin; x <= XMax; x++)
	        ApplyFunc(x, y);
	    x1 += Dx1;
	    x2 += Dx2;
	}
	x1 -= Dx1;
	x2 -= Dx2;
        Dx2 = (Max[0] - Mid[0]) / ((IrtRType) (Max[1] - Mid[1]));
    }
    else {
        Dx2 = (Max[0] - Mid[0]) / ((IrtRType) (Max[1] - Mid[1]));
        y = Mid[1];
        x1 = Min[0];
	x2 = Mid[0];

	x1 -= Dx1;
	x2 -= Dx2;
    }

    /* Scan convert between Mid and Max. */
    if (Max[1] != Mid[1]) {
	for ( ; y <= Max[1]; y++) {
	    int XMin, XMax;

	    x1 += Dx1;
	    x2 += Dx2;

	    XMin = (int) (IRIT_MIN(x1, x2) + 0.5),
	    XMax = (int) (IRIT_MAX(x1, x2) + 0.5);

	    for (x = XMin; x <= XMax; x++)
	        ApplyFunc(x, y);
	}
    }
}
