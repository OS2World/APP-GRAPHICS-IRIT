/******************************************************************************
* MS_Circ.c - minimum spanning circle/cone.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 2002.					      *
******************************************************************************/

#include "irit_sm.h"
#include "misc_lib.h"
#include "geom_loc.h"

#define GM_SWAP_VECTORS(V1, V2) { \
	     IRIT_SWAP(IrtRType, V1[0], V2[0]); \
	     IRIT_SWAP(IrtRType, V1[1], V2[1]); \
	     IRIT_SWAP(IrtRType, V1[2], V2[2]); }

static void GMMinSpanCircWithPt(IrtE2PtStruct *DTPts,
				int NumOfPoints,
				IrtE2PtStruct *Q,
				IrtE2PtStruct *Center,
				IrtRType *Radius);
static void GMMinSpanCircWith2Pts(IrtE2PtStruct *DTPts,
				  int NumOfPoints,
				  IrtE2PtStruct *Q1,
				  IrtE2PtStruct *Q2,
				  IrtE2PtStruct *Center,
				  IrtRType *Radius);
static int GMMinSpanConeWithPt(IrtVecType *DTPts,
			       int NumOfPoints,
			       IrtVecType Q,
			       IrtVecType Center,
			       IrtRType *Radius);
static void GMMinSpanConeWith2Pts(IrtVecType *DTPts,
				  int NumOfPoints,
				  IrtVecType Q1,
				  IrtVecType Q2,
				  IrtVecType Center,
				  IrtRType *Radius);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning circle (MSC) computation of a set of points.	     M
* Algorithm is based on Section 4.7 of "Computational Geometry, Algorithms   M
* and Applications" by M. de Berg et. al.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   DTPts:           The set of point to compute their MSC.                  M
*   NumOfPoints:     Number of points in set DTPts.			     M
*   Center:          Of computed MSC.					     M
*   Radius:	     Of computed MSC.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanCone                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMinSpanCirc, minimum spanning circle                                   M
*****************************************************************************/
int GMMinSpanCirc(IrtE2PtStruct *DTPts,
		  int NumOfPoints,
		  IrtE2PtStruct *Center,
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
    IRIT_PT2D_BLEND(Center -> Pt, DTPts[0].Pt, DTPts[1].Pt, 0.5);
    RadiusSqr = IRIT_PT2D_DIST_SQR(DTPts[0].Pt, DTPts[1].Pt) * 0.25;

    /* And examine the rest if inside. */
    for (i = 2; i < NumOfPoints; i++) {
	IrtRType
	    RadSqr =  IRIT_PT2D_DIST_SQR(DTPts[i].Pt, Center -> Pt);

	if (RadSqr > RadiusSqr)
	    GMMinSpanCircWithPt(DTPts, i, &DTPts[i], Center, &RadiusSqr);
    }

    *Radius = sqrt(RadiusSqr);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanCirc.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   DTPts:           The set of point to compute their MSC.                  *
*   NumOfPoints:     Number of points in set DTPts.			     *
*   Q:		     Extra point that must be on the boundary of the MSC.    *
*   Center:          Of computed MSC.					     *
*   RadiusSqr:	     Of computed MSC.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void			                                             *
*****************************************************************************/
static void GMMinSpanCircWithPt(IrtE2PtStruct *DTPts,
				int NumOfPoints,
				IrtE2PtStruct *Q,
				IrtE2PtStruct *Center,
				IrtRType *RadiusSqr)
{
    int i;

    if (NumOfPoints < 1) {
        GEOM_FATAL_ERROR(GEOM_ERR_MSC_TOO_FEW_PTS);
	return;
    }

    /* Compute a trivial bound to first two point. */
    IRIT_PT2D_BLEND(Center -> Pt, DTPts[0].Pt, Q -> Pt, 0.5);
    *RadiusSqr = IRIT_PT2D_DIST_SQR(DTPts[0].Pt, Q -> Pt) * 0.25;

    /* And examine the rest if inside. */
    for (i = 1; i < NumOfPoints; i++) {
	IrtRType
	    RadSqr = IRIT_PT2D_DIST_SQR(DTPts[i].Pt, Center -> Pt);

	if (RadSqr > *RadiusSqr)
	    GMMinSpanCircWith2Pts(DTPts, i, &DTPts[i], Q, Center, RadiusSqr);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanCirc.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   DTPts:           The set of point to compute their MSC.                  *
*   NumOfPoints:     Number of points in set DTPts.			     *
*   Q1, Q2:	     Extra points that must be on the boundary of the MSC.   *
*   Center:          Of computed MSC.					     *
*   RadiusSqr:	     Of computed MSC.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void			                                             *
*****************************************************************************/
static void GMMinSpanCircWith2Pts(IrtE2PtStruct *DTPts,
				  int NumOfPoints,
				  IrtE2PtStruct *Q1,
				  IrtE2PtStruct *Q2,
				  IrtE2PtStruct *Center,
				  IrtRType *RadiusSqr)
{
    int i;

    /* Compute a trivial bound to first two point. */
    IRIT_PT2D_BLEND(Center -> Pt, Q1 -> Pt, Q2 -> Pt, 0.5);
    *RadiusSqr = IRIT_PT2D_DIST_SQR(Q1 -> Pt, Q2 -> Pt) * 0.25;

    /* And examine the rest if inside. */
    for (i = 0; i < NumOfPoints; i++) {
	IrtRType
	    RadSqr = IRIT_PT2D_DIST_SQR(DTPts[i].Pt, Center -> Pt);

	if (RadSqr > *RadiusSqr) {
	    IrtPtType C, Pt1, Pt2, Pt3;

	    /* Compute the circle through Q1, Q2, and DTPts[i]. */
	    IRIT_PT2D_COPY(Pt1, Q1 -> Pt);
	    IRIT_PT2D_COPY(Pt2, Q2 -> Pt);
	    IRIT_PT2D_COPY(Pt3, DTPts[i].Pt);
	    Pt1[2] = Pt2[2] = Pt3[2] = 0.0;

	    if (!GMCircleFrom3Points(C, RadiusSqr, Pt1, Pt2, Pt3)) {
		/* Three points are collinear. */
	        GEOM_FATAL_ERROR(GEOM_ERR_MSC_COLIN_CIRC);
		return;
	    }
	    *RadiusSqr = IRIT_SQR(*RadiusSqr);

	    Center -> Pt[0] = C[0];
	    Center -> Pt[1] = C[1];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning cone (MSC) computation of a set of vectors.	     M
* Find a central vector as the average of all given vectors and find the     M
* vector with maximal angular distance from it.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   DTVecs:          The set of vectors to compute their MSC.		     M
*   VecsNormalized:  TRUE if vectors are normalized, FALSE otherwise.        M
*   NumOfVecs:       Number of vectors in set DTVecs.			     M
*   ConeAxis:        Of computed MSC.					     M
*   ConeAngle:	     Of computed MSC, in radians.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanCirc, GMMinSpanCone                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMinSpanConeAvg                                                         M
*****************************************************************************/
int GMMinSpanConeAvg(IrtVecType *DTVecs,
		     int VecsNormalized,
		     int NumOfVecs,
		     IrtVecType ConeAxis,
		     IrtRType *ConeAngle)
{
    int i;
    IrtRType IProd,
	MinIProd = 1.0;
    IrtVecType *NrmlDTVecs;

    if (NumOfVecs < 2) {
        GEOM_FATAL_ERROR(GEOM_ERR_MSC_TOO_FEW_PTS);
	return FALSE;
    }

    if (!VecsNormalized) {
        NrmlDTVecs = (IrtVecType *) IritMalloc(NumOfVecs
					                * sizeof(IrtVecType));
	IRIT_GEN_COPY(NrmlDTVecs, DTVecs, NumOfVecs * sizeof(IrtVecType));
	for (i = 0; i < NumOfVecs; i++) {
	    IRIT_VEC_NORMALIZE(NrmlDTVecs[i]);
	}
    }
    else
        NrmlDTVecs = DTVecs;
    
    /* Compute the center of the cone. */
    IRIT_VEC_RESET(ConeAxis);
    for (i = 0; i < NumOfVecs; i++) {
	IRIT_VEC_ADD(ConeAxis, ConeAxis, NrmlDTVecs[i]);
    }
    IRIT_VEC_NORMALIZE(ConeAxis);

    /* Compute the angular openning of the cone. */
    for (i = 0; i < NumOfVecs; i++) {
        IProd = IRIT_DOT_PROD(ConeAxis, NrmlDTVecs[i]);
	if (MinIProd > IProd)
	    MinIProd = IProd;
    }
    *ConeAngle = acos(MinIProd);

    if (!VecsNormalized)
	IritFree(NrmlDTVecs);
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Minimum spanning cone (MSC) computation of a set of vectors.	     M
* Algorithm is based on the Minimum Spanning Circle in Section 4.7 of        M
* "Computational Geometry, Algorithms and Applications" by M. de Berg et. al.M
*                                                                            *
* PARAMETERS:                                                                M
*   DTVecs:          The set of vectors to compute their MSC.  		     M
*   VecsNormalized:  TRUE if vectors are normalized, FALSE otherwise.        M
*   NumOfVecs:       Number of vectors in set DTVecs.			     M
*   ConeAxis:        Of computed MSC.					     M
*   ConeAngle:	     Of computed MSC, in radians.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:          TRUE if successful, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMMinSpanCirc, GMMinSpanConeAvg                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMMinSpanCone                                                            M
*****************************************************************************/
int GMMinSpanCone(IrtVecType *DTVecs,
		  int VecsNormalized,
		  int NumOfVecs,
		  IrtVecType ConeAxis,
		  IrtRType *ConeAngle)
{
    int i, j, Found;
    IrtRType ConeCosAngle;
    IrtVecType *NrmlDTVecs;

    if (NumOfVecs < 2) {
        IRIT_VEC_COPY(ConeAxis, DTVecs[0]);
	*ConeAngle = 0.0;
	return TRUE;
    }

    if (!VecsNormalized) {
        NrmlDTVecs = (IrtVecType *) IritMalloc(NumOfVecs * sizeof(IrtVecType));
	IRIT_GEN_COPY(NrmlDTVecs, DTVecs, NumOfVecs * sizeof(IrtVecType));

	for (i = 0; i < NumOfVecs; i++) {
	    IRIT_VEC_NORMALIZE(NrmlDTVecs[i]);
	}
    }
    else
        NrmlDTVecs = DTVecs;

    /* Compute an initial guess out of two non colinear vectors. */
    Found = FALSE;
    for (i = 0; !Found && i < NumOfVecs - 1; i++) {
        for (j = i + 1; !Found && j < NumOfVecs; j++) {
	    if (IRIT_DOT_PROD(NrmlDTVecs[i], NrmlDTVecs[j]) > -1.0 + IRIT_UEPS) {
	        IRIT_PT_BLEND(ConeAxis, NrmlDTVecs[i], NrmlDTVecs[j], 0.5);
		IRIT_PT_NORMALIZE(ConeAxis);
		Found = TRUE;

		/* Place i'th and j'th vectors in places 0 and 1. */
		if (i != 0) {
		    GM_SWAP_VECTORS(NrmlDTVecs[i], NrmlDTVecs[0]);
		}
		if (j != 1) {
		    GM_SWAP_VECTORS(NrmlDTVecs[j], NrmlDTVecs[1]);
		}
	    }
	}
    }
    if (!Found) {
        if (!VecsNormalized)
	    IritFree(NrmlDTVecs);
	return FALSE;
    }

    ConeCosAngle = IRIT_DOT_PROD(NrmlDTVecs[0], ConeAxis);

    /* And examine the rest if inside. */
    for (i = 2; i < NumOfVecs; i++) {
	IrtRType
	    CosAng = IRIT_DOT_PROD(NrmlDTVecs[i], ConeAxis);

	if (CosAng < ConeCosAngle)
	    if (!GMMinSpanConeWithPt(NrmlDTVecs, i, NrmlDTVecs[i], ConeAxis,
				     &ConeCosAngle)) {
	        if (!VecsNormalized)
		    IritFree(NrmlDTVecs);
		return FALSE;
	    }
    }

    *ConeAngle = acos(ConeCosAngle);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugGMTestSpanCone, FALSE) {
	    for (i = j = 0; i < NumOfVecs; i++) {
		IrtRType
		    d = IRIT_DOT_PROD(ConeAxis, NrmlDTVecs[i]);

		if (IRIT_APX_EQ_EPS(d, ConeCosAngle, IRIT_UEPS))
		    j++;
	        else if (d < ConeCosAngle)
		    printf("failed to fit a cone to vector %d = %f %f %f\n", i,
			   NrmlDTVecs[i][0],
			   NrmlDTVecs[i][1],
			   NrmlDTVecs[i][2]);
	    }
	    if (j < 2 || j > 3)  /* Should be supported by 2 or 3 vectors. */
	        printf("Cone is supported by %d vectors\n", j);
	}
    }
#   endif /* DEBUG */

    if (!VecsNormalized)
	IritFree(NrmlDTVecs);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanCone.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   DTVecs:          The set of vector to compute their MSC.                 *
*   NumOfVecs:       Number of vectors in set DTVecs.			     *
*   Q:		     Extra vector that must be on the boundary of the MSC.   *
*   ConeAxis:        Of computed MSC.					     *
*   ConeCosAngle:    Of computed MSC.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	FALSE if failed, TRUE otherwise.                             *
*****************************************************************************/
static int GMMinSpanConeWithPt(IrtVecType *DTVecs,
			       int NumOfVecs,
			       IrtVecType Q,
			       IrtVecType ConeAxis,
			       IrtRType *ConeCosAngle)
{
    int i;

    if (NumOfVecs < 1) {
        GEOM_FATAL_ERROR(GEOM_ERR_MSC_TOO_FEW_PTS);
	return FALSE;
    }

    /* Compute a trivial bound with given point and first vector. */
    if (IRIT_DOT_PROD(DTVecs[0], Q) < -1.0 + IRIT_UEPS)
        return FALSE;

    IRIT_PT_BLEND(ConeAxis, DTVecs[0], Q, 0.5);
    IRIT_PT_NORMALIZE(ConeAxis);
    *ConeCosAngle = IRIT_DOT_PROD(DTVecs[0], ConeAxis);

    /* And examine the rest if inside. */
    for (i = 1; i < NumOfVecs; i++) {
	IrtRType
	    CosAng = IRIT_DOT_PROD(DTVecs[i], ConeAxis);

	if (CosAng < *ConeCosAngle)
	    GMMinSpanConeWith2Pts(DTVecs, i, DTVecs[i], Q, ConeAxis,
				  ConeCosAngle);
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of GMMinSpanCone.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   DTVecs:          The set of vector to compute their MSC.                 *
*   NumOfVecs:       Number of vectors in set DTVecs.			     *
*   Q1, Q2:	     Extra vectors that must be on the boundary of the MSC.  *
*   ConeAxis:        Of computed MSC.					     *
*   ConeCosAngle:    Of computed MSC.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void			                                             *
*****************************************************************************/
static void GMMinSpanConeWith2Pts(IrtVecType *DTVecs,
				  int NumOfVecs,
				  IrtVecType Q1,
				  IrtVecType Q2,
				  IrtVecType ConeAxis,
				  IrtRType *ConeCosAngle)
{
    int i, j;

    /* Compute a trivial bound with given two vectors. */
    IRIT_PT_BLEND(ConeAxis, Q1, Q2, 0.5);
    IRIT_PT_NORMALIZE(ConeAxis);
    *ConeCosAngle = IRIT_DOT_PROD(Q1, ConeAxis);

    /* And examine the rest if inside. */
    for (i = 0; i < NumOfVecs; i++) {
	IrtRType
	    CosAng = IRIT_DOT_PROD(DTVecs[i], ConeAxis);

	if (CosAng < *ConeCosAngle) {
	    IrtRType R1, R2;
	    IrtPlnType Plane;

	    /* Compute the cone through Q1, Q2, and DTVecs[i]. */
	    GMPlaneFrom3Points(Plane, Q1, Q2, DTVecs[i]);
	    IRIT_VEC_COPY(ConeAxis, Plane);
	    R1 = IRIT_DOT_PROD(ConeAxis, Q1);
	    if (R1 < 0.0) {
		IRIT_VEC_SCALE(ConeAxis, -1);
		R1 = -R1;
	    }

	    /* Find a vector inside cone and reorient cone, if needed. */
	    for (j = 0; j < i; j++) {
	        R2 = IRIT_DOT_PROD(ConeAxis, DTVecs[j]);
		if (!IRIT_APX_EQ_EPS(R1, R2, IRIT_UEPS))
		    break;
	    }
	    if (j < i && R1 > R2) {
		IRIT_VEC_SCALE(ConeAxis, -1);
		R1 = -R1;
	    }
	    *ConeCosAngle = R1;
	}
    }
}
