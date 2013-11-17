/******************************************************************************
* MSSPTS.c - minimum spanning sphere.   				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Ramanathan M., April 2006.					      *
******************************************************************************/
#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_loc.h"

static int GMMinSpanSphereWithPt(IrtE3PtStruct *DTPts,
				 int NumOfPoints,
				 IrtE3PtStruct *Q,
				 IrtE3PtStruct *Center,
				 IrtRType *RadiusSqr);
static int GMMinSpanSphereWith2Pts(IrtE3PtStruct *DTPts,
				   int NumOfPoints,
				   IrtE3PtStruct *Q1,
				   IrtE3PtStruct *Q2,
				   IrtE3PtStruct *Center,
				   IrtRType *RadiusSqr);
static int GMMinSpanSphereWith3Pts(IrtE3PtStruct *DTPts,
				   int NumOfPoints,
				   IrtE3PtStruct *Q1,
				   IrtE3PtStruct *Q2,
				   IrtE3PtStruct *Q3,
				   IrtE3PtStruct *Center,
				   IrtRType *RadiusSqr);
static int GMMinSpanSphereWith4Pts(IrtE3PtStruct *DTPts,
				   IrtE3PtStruct *Q1,
				   IrtE3PtStruct *Q2,
				   IrtE3PtStruct *Q3,
				   IrtE3PtStruct *Q4,
				   IrtE3PtStruct *Center,
				   IrtRType *RadiusSqr);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning sphere (MSS) computation of a set of points.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   DTPts:           The set of point to compute their MSS.                  M
*   NumOfPoints:     Number of points in set DTPts.			     M
*   Center:          Of computed MSS.					     M
*   Radius:	     Of computed MSS.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanCirc                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMinSpanSphere, minimum spanning sphere                                 M
*****************************************************************************/
int GMMinSpanSphere(IrtE3PtStruct *DTPts,
		    int NumOfPoints,
		    IrtE3PtStruct *Center,
		    IrtRType *Radius)
{
    int i;
    IrtRType RadiusSqr;
  
    if (NumOfPoints < 2) {
        *Center = *DTPts;
	*Radius = 0.0;
	return TRUE;
    }
 
    /* Compute a trivial bound to first two point. */
    IRIT_PT_BLEND(Center -> Pt, DTPts[0].Pt, DTPts[1].Pt, 0.5);
    RadiusSqr = IRIT_PT_PT_DIST_SQR(DTPts[0].Pt, DTPts[1].Pt) * 0.25;

    /* And examine the rest if inside. */
    for (i = 2; i < NumOfPoints; i++) {
	IrtRType
	    RadSqr = IRIT_PT_PT_DIST_SQR(DTPts[i].Pt, Center -> Pt);

	if (RadSqr > RadiusSqr) {
	    if (!GMMinSpanSphereWithPt(DTPts, i, &DTPts[i], 
				       Center, &RadiusSqr)){
	        *Radius= IRIT_INFNTY;
		return FALSE;
	    }
	}
    }

    *Radius = sqrt(RadiusSqr);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanSphere.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   DTPts:           The set of point to compute their MSS.                  *
*   NumOfPoints:     Number of points in set DTPts.			     *
*   Q:		     Extra point that must be on the boundary of the MSS.    *
*   Center:          Of computed MSS.					     *
*   RadiusSqr:	     Of computed MSS.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int			                                                     *
*****************************************************************************/
static int GMMinSpanSphereWithPt(IrtE3PtStruct *DTPts,
				 int NumOfPoints,
				 IrtE3PtStruct *Q,
				 IrtE3PtStruct *Center,
				 IrtRType *RadiusSqr)
{
    int j;
   
    if (NumOfPoints < 1) {
        GEOM_FATAL_ERROR(GEOM_ERR_MSC_TOO_FEW_PTS);
	return FALSE;
    }

    /* Compute a trivial bound to first two point. */
    IRIT_PT_BLEND(Center -> Pt, DTPts[0].Pt, Q -> Pt, 0.5);
    *RadiusSqr = IRIT_PT_PT_DIST_SQR(DTPts[0].Pt, Q -> Pt) * 0.25;

    /* And examine the rest if inside. */
    for (j = 1; j < NumOfPoints; j++) {
	IrtRType
	    RadSqr = IRIT_PT_PT_DIST_SQR(DTPts[j].Pt, Center -> Pt);

	if (RadSqr > *RadiusSqr) {
	    if (!GMMinSpanSphereWith2Pts(DTPts, j, &DTPts[j], Q, 
					 Center, RadiusSqr)) {
	        *RadiusSqr = IRIT_INFNTY;
		return FALSE;
	    }
	}
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanSphere.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   DTPts:           The set of point to compute their MSS.                  *
*   NumOfPoints:     Number of points in set DTPts.			     *
*   Q1, Q2:	     Extra points that must be on the boundary of the MSS.   *
*   Center:          Of computed MSS.					     *
*   RadiusSqr:	     Of computed MSS.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int			                                                     *
*****************************************************************************/
static int GMMinSpanSphereWith2Pts(IrtE3PtStruct *DTPts,
				   int NumOfPoints,
				   IrtE3PtStruct *Q1,
				   IrtE3PtStruct *Q2,
				   IrtE3PtStruct *Center,
				   IrtRType *RadiusSqr)
{
    int k;

    /* Compute a trivial bound to first two point. */
    IRIT_PT_BLEND(Center -> Pt, Q1 -> Pt, Q2 -> Pt, 0.5);
    *RadiusSqr = IRIT_PT_PT_DIST_SQR(Q2 -> Pt, Q1 -> Pt) * 0.25;

    /* And examine the rest if inside. */
    for (k = 0; k < NumOfPoints; k++) {
	IrtRType
	    RadSqr = IRIT_PT_PT_DIST_SQR(DTPts[k].Pt, Center -> Pt);

	if (RadSqr > *RadiusSqr) {
	    /* Compute the sphere through Q1, Q2, and DTPts[k]. */
	    if(!GMMinSpanSphereWith3Pts(DTPts, k, Q1, Q2, &DTPts[k], 
					Center, RadiusSqr)) {
	        *RadiusSqr = IRIT_INFNTY;
		return FALSE;
	    }
	}
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanSphere.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   DTPts:           The set of point to compute their MSS.                  *
*   NumOfPoints:     Number of points in set DTPts.			     *
*   Q1, Q2, Q3:	     Extra points that must be on the boundary of the MSS.   *
*   Center:          Of computed MSS.					     *
*   RadiusSqr:	     Of computed MSS.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int		        	                                             *
*****************************************************************************/
static int GMMinSpanSphereWith3Pts(IrtE3PtStruct *DTPts,
				   int NumOfPoints,
				   IrtE3PtStruct *Q1,
				   IrtE3PtStruct *Q2,
				   IrtE3PtStruct *Q3,
				   IrtE3PtStruct *Center,
				   IrtRType *RadiusSqr)
{
    int l;
    IrtE3PtStruct GMPts[3];
    IrtRType Cent[3];

    IRIT_PT_COPY(GMPts[0].Pt, Q1 -> Pt);
    IRIT_PT_COPY(GMPts[1].Pt, Q2 -> Pt);
    IRIT_PT_COPY(GMPts[2].Pt, Q3 -> Pt);
	    
    if (!GMSphereWith3Pts(GMPts, Cent, RadiusSqr)) {
        /* Three points are collinear. */
        GEOM_FATAL_ERROR(GEOM_ERR_MSC_COLIN_CIRC);
	return FALSE;
    }
    else {
	IRIT_PT_COPY(Center -> Pt, Cent);
    }
    /* And examine the rest if inside. */
    for (l = 0; l < NumOfPoints; l++) {
	IrtRType
	    RadSqr = IRIT_PT_PT_DIST_SQR(DTPts[l].Pt, Center -> Pt);

	if (RadSqr > *RadiusSqr) {
	    /* Compute the sphere through Q1, Q2, and DTPts[l]. */
	    if(!GMMinSpanSphereWith4Pts(DTPts, Q1, Q2, Q3, &DTPts[l], 
					Center, RadiusSqr)) {
	        *RadiusSqr = IRIT_INFNTY;
		return FALSE;
	    }
	}
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanSphere.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   DTPts:           The set of point to compute their MSS.                  *
*   Q1, Q2, Q3, Q4:  Points that must be on the boundary of the MSS.         *
*   Center:          Of computed MSS.					     *
*   RadiusSqr:	     Of computed MSS.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int		        	                                             *
*****************************************************************************/
static int GMMinSpanSphereWith4Pts(IrtE3PtStruct *DTPts,
				   IrtE3PtStruct *Q1,
				   IrtE3PtStruct *Q2,
				   IrtE3PtStruct *Q3,
				   IrtE3PtStruct *Q4,
				   IrtE3PtStruct *Center,
				   IrtRType *RadiusSqr)
{
    IrtE3PtStruct GMPts[4];
    IrtRType Cent[3];

    /* Compute the sphere through Q1, Q2, Q3, and Q4. */
    IRIT_PT_COPY(GMPts[0].Pt, Q1 -> Pt);
    IRIT_PT_COPY(GMPts[1].Pt, Q2 -> Pt);
    IRIT_PT_COPY(GMPts[2].Pt, Q3 -> Pt);
    IRIT_PT_COPY(GMPts[3].Pt, Q4 -> Pt);

    if (!GMSphereWith4Pts(GMPts, Cent, RadiusSqr)) {
        *RadiusSqr = IRIT_INFNTY;
	return FALSE;
    }

    IRIT_PT_COPY(Center -> Pt, Cent);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given three points, compute the sphere through the set of points.        M
*   Initially, the three points are rotated to a plane paraellel to XY-plane M
*   and then using GMCircleFrom3Points, the circle through them is computed. M 
*   The center is then rotated back to form the center of the sphere in 3D.  M 
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:             The set of point to compute their sphere.               M
*   Center:          Of computed sphere.				     M
*   RadiusSqr:	     Of computed Sphere.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMCircleFrom3Points                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSphereWith3Pts                                                         M
*****************************************************************************/
int GMSphereWith3Pts(IrtE3PtStruct *Pts, IrtRType *Center, IrtRType *RadiusSqr)
{
    int i;
    IrtRType Radius;
    IrtPtType V1, V2, Nrml, NewPts[3], Cent;
    IrtHmgnMatType Mat, InvMat;

    IRIT_PT_SUB(V1, Pts[2].Pt, Pts[1].Pt);
    IRIT_PT_SUB(V2, Pts[1].Pt, Pts[0].Pt);
    IRIT_CROSS_PROD(Nrml, V1, V2);
    IRIT_VEC_SAFE_NORMALIZE(Nrml);

    GMGenMatrixZ2Dir(Mat, Nrml);
    MatTranspMatrix(Mat, InvMat);
    
    for (i = 0; i < 3; i++)
        MatMultPtby4by4(NewPts[i], Pts[i].Pt, InvMat);

    if (!GMCircleFrom3Points(Cent, &Radius, NewPts[0], NewPts[1], NewPts[2])) {
        /* Three points are collinear. */
        GEOM_FATAL_ERROR(GEOM_ERR_MSC_COLIN_CIRC);
	return FALSE;
    }

    *RadiusSqr = IRIT_SQR(Radius);

    Cent[2] = NewPts[0][2];                  /* Place at the proper Z level. */
    MatMultPtby4by4(Center, Cent, Mat);
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given four points, compute the sphere through the set of points.         M
*   It is identified by solving the following equidistant conditions.        M
*      < P - P1, P - P1 > = < P - P2, P - P2 >,		                     V
*      < P - P1, P - P1 > = < P - P3, P - P3 >,                              V
*      < P - P1, P - P1 > = < P - P4, P - P4 >,                              V
*  or,									     M
*      2(P2 - P1) P = P2^2 - P1^2,                                           V
*      2(P3 - P1) P = P3^2 - P1^2,                                           V
*      2(P4 - P1) P = P4^2 - P1^2.                                           V
*   We can solve for P (the sphere's center) using the Cramer's rule.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pts:             The set of point to compute their sphere.               M
*   Center:          Of computed sphere.				     M
*   RadiusSqr:	     Of computed Sphere.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanSphere                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSphereWith4Pts                                                         M
*****************************************************************************/
int GMSphereWith4Pts(IrtE3PtStruct *Pts, IrtRType *Center, IrtRType *RadiusSqr)
{
    IrtRType Det, Det1, Scl;
    IrtPtType b;
    IrtGnrlMatType A, M;

    /* Allocate two matrices of size (3x3) together. */
    A = (IrtGnrlMatType) IritMalloc(9 * 2 * sizeof(IrtRType));
    M = &A[9];
   
    IRIT_PT_SUB(&A[0], Pts[1].Pt, Pts[0].Pt);
    IRIT_PT_SUB(&A[3], Pts[2].Pt, Pts[0].Pt);
    IRIT_PT_SUB(&A[6], Pts[3].Pt, Pts[0].Pt);
    Scl = 2.0;
    MatGnrlScaleMat(A, A, &Scl, 3);

    /* Initialising AMatrix and computing using Cramer's rule. */
    if ((Det = MatGnrlDetMatrix(A, 3)) == 0) {
        *RadiusSqr = 0.0;
	IritFree(A);
	return FALSE;
    }
    Det1 = 1.0 / Det;

    b[0] = IRIT_PT_SQR_LENGTH(Pts[1].Pt) - IRIT_PT_SQR_LENGTH(Pts[0].Pt);
    b[1] = IRIT_PT_SQR_LENGTH(Pts[2].Pt) - IRIT_PT_SQR_LENGTH(Pts[0].Pt);
    b[2] = IRIT_PT_SQR_LENGTH(Pts[3].Pt) - IRIT_PT_SQR_LENGTH(Pts[0].Pt);

    /* Computing DetX. */
    IRIT_GEN_COPY(M, A, 9 * sizeof(IrtRType));
    M[0] = b[0];
    M[3] = b[1];
    M[6] = b[2];
    Center[0] = MatGnrlDetMatrix(M, 3);

    /* Computing DetY. */ 
    IRIT_GEN_COPY(M, A, 9 * sizeof(IrtRType));
    M[1] = b[0];
    M[4] = b[1];
    M[7] = b[2];
    Center[1] = MatGnrlDetMatrix(M, 3);

    /* Computing DetZ. */ 
    IRIT_GEN_COPY(M, A, 9 * sizeof(IrtRType));
    M[2] = b[0];
    M[5] = b[1];
    M[8] = b[2];
    Center[2] = MatGnrlDetMatrix(M, 3);

    IRIT_PT_SCALE(Center, Det1);
    *RadiusSqr = IRIT_PT_PT_DIST_SQR(Pts[0].Pt, Center);
    IritFree(A);
    return TRUE;
}
