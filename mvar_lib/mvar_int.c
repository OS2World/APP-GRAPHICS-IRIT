/******************************************************************************
* Mvar_int.c - Interpolation/fitting routines used by modules of mvar_lib.    *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec 11.					      *
******************************************************************************/

#include <string.h>
#include "cagd_lib.h"
#include "mvar_loc.h"

IRIT_STATIC_DATA int
    GlblLineFitSortAxis = -1;

#if defined(ultrix) && defined(mips)
static int LineFitSortCmpr(VoidPtr VPt1, VoidPtr VPt2);
#else
static int LineFitSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points, vecList, computes a Bspline curve of order Order    M
* that interpolates or least square approximates the set of points.	     M
*   The size of the control polygon of the resulting Bspline curve defaults  M
* to the number of points in PtList (if CrvSize = 0).			     M
*   However, this number is can smaller to yield a least square              M
* approximation.							     M
*   The created curve can be parametrized as specified by ParamType.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   vecList:      List of points to interpolate/least square approximate.    M
*                 All points are assumed of same dimension (not tested.).    M
*   Order:        Of interpolating/approximating curve.                      M
*   CrvSize:      Number of degrees of freedom (control points) of the       M
*                 interpolating/approximating curve.                         M
*   ParamType:    Type of parametrization.                                   M
*   Periodic:     Constructed curve should be Periodic.	 Periodic	     M
*		  necessitates uniform knot sequence in ParamType.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Constructed interpolating/approximating curve.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpPts, BspCrvInterpolate, BspCrvInterpPts2                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspCrvInterpVecs, interpolation, least square approximation          M
*****************************************************************************/
CagdCrvStruct *MvarBspCrvInterpVecs(const MvarVecStruct *vecList,
				    int Order,
				    int CrvSize,
				    CagdParametrizationType ParamType,
				    CagdBType Periodic)
{
    int i,
        Dim = vecList -> Dim,
	NumPts = CagdListLength(vecList);
    CagdPointType
        PType = CAGD_MAKE_PT_TYPE(FALSE, Dim);
    CagdRType *KV, *PtKnots;
    CagdCtlPtStruct
	*CtlPt = NULL,
	*CtlPtList = NULL;
    const MvarVecStruct *Vec;
    CagdCrvStruct *Crv;

    if (NumPts < 2 ||
	NumPts < Order ||
	CrvSize < Order ||
	Dim > CAGD_PT_MAX_SIZE_TYPE)
	return NULL;

    /* Convert to control points in a linear list */
    for (Vec = vecList; Vec != NULL; Vec = Vec -> Pnext) {
	if (CtlPtList == NULL)
	    CtlPtList = CtlPt = CagdCtlPtNew(PType);
	else {
	    CtlPt -> Pnext = CagdCtlPtNew(PType);
	    CtlPt = CtlPt -> Pnext;
	}
	for (i = 0; i < Dim; i++)
	    CtlPt -> Coords[i + 1] = Vec -> Vec[i];
    }

    BspCrvInterpBuildKVs(CtlPtList, Order, CrvSize, ParamType, Periodic,
			 &PtKnots, &KV);

    Crv = BspCrvInterpolate(CtlPtList, PtKnots, KV, CrvSize, Order, Periodic);
    CagdCtlPtFreeList(CtlPtList);

    IritFree(PtKnots);
    IritFree(KV);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two points in line fit for sorting purposes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VPt1, VPt2:  Two pointers to points.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two points.              *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int LineFitSortCmpr(VoidPtr VPt1, VoidPtr VPt2)
#else
static int LineFitSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = (*((MvarPtStruct **) VPt1)) -> Pt[GlblLineFitSortAxis] -
	       (*((MvarPtStruct **) VPt2)) -> Pt[GlblLineFitSortAxis];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sorts given list of points based on their increasing order in axis Axis. M
* Sorting is done in place.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecList:  List of points to sort.					     M
*   Axis:    Axis to sort along: 1,2,3 for X,Y,Z.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:   Sorted list of points, in place.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecSortAxis                                                          M
*****************************************************************************/
MvarVecStruct *MvarVecSortAxis(MvarVecStruct *VecList, int Axis)
{
    int l, 
	Len = CagdListLength(VecList);
    MvarVecStruct *Vec, **VecVec;

    if (Len < 2)
	return VecList;
        
    VecVec = (MvarVecStruct **) IritMalloc(sizeof(MvarVecStruct *) * Len);

    /* Sort all points in order of the most significant axis. */
    for (l = 0; l < Len; l++) {
	Vec = VecList;
	VecList = VecList -> Pnext;
	Vec -> Pnext = NULL;
	VecVec[l] = Vec;
    }

    GlblLineFitSortAxis = Axis - 1;
    qsort(VecVec, Len, sizeof(MvarVecStruct *), LineFitSortCmpr);

    VecList = VecVec[0];
    for (l = 0; l < Len - 1; l++)
	VecVec[l] -> Pnext = VecVec[l + 1];
    IritFree(VecVec);

    return VecList;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the closest point on a line to point, in R^n.	     M
*  The line is prescribed using a point on it (Pl) and a unit vector (Vl).   M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the closest to on the line.                       M
*   Pl, Vl:        Position and direction that defines the line. Vl is       M
*		   assumed a unit length vector.	                     M
*   ClosestPoint:  Where closest point found on the line is to be saved.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMPointFromPointLine,     		                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPointFromPointLine, point line distance                              M
*****************************************************************************/
void MvarPointFromPointLine(const MvarVecStruct *Point,
			    const MvarVecStruct *Pl,
			    const MvarVecStruct *Vl,
			    MvarVecStruct *ClosestPoint)
{
    int i,
	Dim = Point -> Dim;
    MvarVecStruct
	*V1 = MvarVecNew(Pl -> Dim);
    IrtRType CosAlpha;

    assert(Pl -> Dim == Dim && Vl -> Dim == Dim);

    MvarVecSub(V1, Point, Pl);

    /* Find angle between the two vectors, including magnitudes of V1 & Vl. */
    CosAlpha = MvarVecDotProd(V1, Vl);

    /* Find P1 - the closest point to Point on the line: */
    for (i = 0; i < Dim; i++)
        ClosestPoint -> Vec[i] = Pl -> Vec[i] + Vl -> Vec[i] * CosAlpha;

    MvarVecFree(V1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the minimal point distance to a given line in R^n.    M
* The line is prescribed using a point on it (Pl) and a unit vector (Vl).    M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:         To find the closest to on the line.                       M
*   Pl, Vl:        Position and direction that defines the line. Vl is       M
*		   assumed a unit length vector.	                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   The computed minimal distance.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMDistPointLine			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarDistPointLine, point line distance                                   M
*****************************************************************************/
IrtRType MvarDistPointLine(const MvarVecStruct *Point,
			   const MvarVecStruct *Pl,
			   const MvarVecStruct *Vl)
{
    CagdRType Err;
    MvarVecStruct
	*VRes = MvarVecNew(Point -> Dim),
	*ClosestPoint = MvarVecNew(Point -> Dim);

    MvarPointFromPointLine(Point, Pl, Vl, ClosestPoint);
    MvarVecSub(VRes, Point, ClosestPoint);

    Err = MvarVecLength(VRes);

    MvarVecFree(VRes);
    MvarVecFree(ClosestPoint);

    return Err;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given set of vecs, VecList, fits a line using least squares fit to them.   M
*                                                                            *
* PARAMETERS:                                                                M
*   VecList:      List of vectors to interpolate/least square approximate.   M
*   LineDir:      A unit vector of the line.                                 M
*   LinePos:      A point on the computed line.            		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Average distance between a vector and the fitted line,     M
*		  or IRIT_INFNTY if failed.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdLineFitToPts			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarLineFitToPts, interpolation, least square approximation              M
*****************************************************************************/
CagdRType MvarLineFitToPts(const MvarVecStruct *VecList,
			   MvarVecStruct *LineDir,
			   MvarVecStruct *LinePos)
{
    int i, SortAxis, Dim,
	Len = CagdListLength(VecList);
    CagdPointType PType;
    CagdRType AverageDist;
    MvarVecStruct *Vec, *Vecs, *Min, *Max;
    CagdCrvStruct *Crv;

    if (Len < 2)
	return IRIT_INFNTY;
    
    Vecs = MvarVecCopyList(VecList);

    /* Make sure all points are of same dimension and figure it out. */
    Dim = Vecs -> Dim;
    for (Vec = Vecs -> Pnext; Vec != NULL; Vec = Vec -> Pnext) {
        if (Dim != Vec -> Dim) {
	    MvarVecFreeList(Vecs);
	    return IRIT_INFNTY;
	}
    }

    /* We use the cagd lib here that supports up to: */
    if (Dim > CAGD_NUM_OF_PT_COORD(CAGD_PT_MAX_SIZE_TYPE)) {
        MvarVecFreeList(Vecs);
	return IRIT_INFNTY;
    }
    PType = CAGD_MAKE_PT_TYPE(FALSE, Dim);

    /* Sort all points in order of the most significant axis. */
    Min = MvarVecNew(Dim);
    Max = MvarVecNew(Dim);
    for (i = 0; i < Dim; i++) {
        Min -> Vec[i] = IRIT_INFNTY;
	Max -> Vec[i] = -IRIT_INFNTY;
    }
    for (Vec = Vecs; Vec != NULL; Vec = Vec -> Pnext) {
	for (i = 0; i < Dim; i++) {
	    if (Max -> Vec[i] < Vec -> Vec[i])
		Max -> Vec[i] = Vec -> Vec[i];
	    if (Min -> Vec[i] > Vec -> Vec[i])
		Min -> Vec[i] = Vec -> Vec[i];
	}
    }

    /* Find the axis with the largest deviation to sort along. */
    SortAxis = 0;
    for (i = 0; i < Dim; i++) {
        if (Max -> Vec[i] - Min -> Vec[i] >
	    Max -> Vec[SortAxis] - Min -> Vec[SortAxis])
	    SortAxis = i;
    }
    Vecs = MvarVecSortAxis(Vecs, SortAxis);

    /* Fit a linear Bspline with two control points and use to find dir. */
    Crv = MvarBspCrvInterpVecs(Vecs, 2, 2, CAGD_CENTRIPETAL_PARAM, FALSE);
    CagdCoercePointTo(LinePos -> Vec, PType, Crv -> Points, 0, Crv -> PType);
    CagdCoercePointTo(LineDir -> Vec, PType, Crv -> Points, 1, Crv -> PType);
    MvarVecSub(LineDir, LineDir, LinePos);
    MvarVecNormalize(LineDir);
    CagdCrvFree(Crv);

    /* Compute average distance of the points to the line as error estimate. */
    for (AverageDist = 0.0, Vec = Vecs; Vec != NULL; Vec = Vec -> Pnext) {
        AverageDist += MvarDistPointLine(Vec, LinePos, LineDir);
    }
    AverageDist /= Len;

    MvarVecFreeList(Vecs);
    MvarVecFree(Min);
    MvarVecFree(Max);

    return AverageDist;
}

#ifdef DEBUG_MVAR_TEST_LINE_LSQ_FIT

main()
{
    int i, j,
        Dim = 5,
        N = 100;
    CagdRType Err;
    MvarVecStruct *Vec,
        *LinePos = MvarVecNew(Dim),
        *LineDir = MvarVecNew(Dim),
        *VecList = NULL;

    for (i = 0; i < N; i++) {
	CagdRType
	    Scl = IritRandom(0, 10);

        Vec = MvarVecNew(Dim);

	printf("Point[%d]:", i);

	/* A test case of all points on main diagonal. */
	for (j = 0; j < Dim; j++) {
	    Vec -> Vec[j] = j + (1 + IritRandom(-0.01, 0.01)) * Scl * (j + 1);

	    printf(" %8.5f", Vec -> Vec[j]);
	}
	printf("\n");

	IRIT_LIST_PUSH(Vec, VecList)
    }

    Err =  MvarLineFitToPts(VecList, LineDir, LinePos);

    printf("Line Position:");
    for (j = 0; j < Dim; j++)
        printf("%8.5f ", LinePos -> Vec[j]);
    printf("(Err = %8.5f)\nLine Direction:", Err);
    for (j = 0; j < Dim; j++)
        printf("%8.5f ", LineDir -> Vec[j]);

    MvarVecFreeList(VecList);
    MvarVecFree(LinePos);
    MvarVecFree(LineDir);
}

#endif /* DEBUG_MVAR_TEST_LINE_LSQ_FIT */
