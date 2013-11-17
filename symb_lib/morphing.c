/******************************************************************************
* Morphing.c - A simple tool to morph between two compatible crvs/srfs.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Jul. 92.					      *
******************************************************************************/

#include "symb_loc.h"
#include "misc_lib.h"
#include "geom_lib.h"

#define LINE_IRIT_EPS 1e-3  /* Tolerance of a curve to be considered a line. */

static void InterpolateVector(CagdRType x1,
			      CagdRType y1,
			      CagdRType x2,
			      CagdRType y2,
			      CagdRType *x,
			      CagdRType *y,
			      CagdRType Blend);
static void TangencyFilterCrvMorph(CagdCrvStruct **CrvSeq1,
				   CagdCrvStruct **CrvSeq2);
static CagdCrvStruct *CrvMorphSeqCornerCut(const CagdCrvStruct *Crv,
					   CagdRType MinDist);
static CagdCrvStruct *CrvMorphOneCornerCut(const CagdCrvStruct *Crv,
					   CagdRType Blend);
static void DistanceFilterCrvMorph(CagdCrvStruct *MorphSeq,
				   CagdRType MinDist);
static CagdRType *ComputeCrvCentroid(const CagdCrvStruct *Crv);
static CagdRType EstimateIntDistCrvCrv(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two compatible curves (See function CagdMakeCrvsCompatible),         M
* computes a convex blend between them according to Blend which must be      M
* between zero and one.							     M
*   Returned is the new blended curve.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  The two curves to blend.                                    M
*   Blend:       A parameter between zero and one                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Crv2 * Blend + Crv1 * (1 - Blend).                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTwoCrvsMorphingCornerCut, SymbTwoCrvsMorphingMultiRes		     M
*   SymbTwoSrfsMorphing, TrivTwoTVsMorphing				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTwoCrvsMorphing, morphing                                            M
*****************************************************************************/
CagdCrvStruct *SymbTwoCrvsMorphing(const CagdCrvStruct *Crv1,
				   const CagdCrvStruct *Crv2,
				   CagdRType Blend)
{
    int i, j,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv1 -> PType),
	Length = Crv1 -> Length,
	Order = Crv1 -> Order;
    CagdRType **NewPoints,
	Blend1 = 1.0 - Blend;
    CagdRType
	* const *Points1 = Crv1 -> Points,
	* const *Points2 = Crv2 -> Points;
    CagdCrvStruct *NewCrv;

    if (Crv1 -> PType != Crv2 -> PType ||
	Crv1 -> GType != Crv2 -> GType ||
	Order != Crv2 -> Order ||
	Length != Crv2 -> Length) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	return NULL;
    }
	
    NewCrv = CagdCrvNew(Crv1 -> GType, Crv1 -> PType, Length);
    NewCrv -> Order = Order;
    NewPoints = NewCrv -> Points;
    if (Crv1 -> KnotVector != NULL)
	NewCrv -> KnotVector = BspKnotCopy(NULL, Crv1 -> KnotVector,
					   Length + Order);

    for (i = !CAGD_IS_RATIONAL_PT(Crv1 -> PType); i <= MaxAxis; i++) {
	CagdRType
	    *Pts1 = &Points1[i][0],
	    *Pts2 = &Points2[i][0],
	    *NewPts = &NewPoints[i][0];

	for (j = Length - 1; j >= 0; j--)
	    *NewPts++ = *Pts1++ * Blend1 + *Pts2++ * Blend;
    }

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two compatible curves (See function CagdMakeCrvsCompatible),         M
* computes a morph between them using corner cutting approach.		     M
*   Returned is the new blended curve.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  The two curves to blend.                                    M
*   MinDist:     Minimal maximum distance between adjacent curves to make    M
*                sure motion is visible. The curves will move at most twice  M
*                that much in their maximal distance (roughly).		     M
*   SameLength:  If TRUE, length of curves is preserved, otherwise BBOX is.  M
*   FilterTangencies: If TRUE, attempt is made to eliminate the intermediate M
*                line representation.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The blended curve.                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTwoCrvsMorphing, SymbTwoCrvsMorphingMultiRes,                        M
*   SymbTwoSrfsMorphing, TrivTwoTVsMorphing                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTwoCrvsMorphingCornerCut, morphing       		             M
*****************************************************************************/
CagdCrvStruct *SymbTwoCrvsMorphingCornerCut(const CagdCrvStruct *Crv1,
					    const CagdCrvStruct *Crv2,
					    CagdRType MinDist,
					    CagdBType SameLength,
					    CagdBType FilterTangencies)
{
    int i, MorphListLength1, MorphListLength2,
	Length = Crv1 -> Length;
    CagdPointType
        PType = Crv1 -> PType;
    CagdPType Pl1, Pl2, Centroid1, Centroid2, TranslateFactor;
    CagdVType Vl1, Vl2, VTmp;
    CagdRType Scale1, Scale2, RotateFactor, *Centroid;
    CagdCrvStruct *Crv, *Crv1MorphList, *Crv2MorphList, *TmpList, *TCrv;

    if (PType != Crv2 -> PType ||
	Crv1 -> GType != Crv2 -> GType ||
	Crv1 -> Order != Crv2 -> Order ||
	Length != Crv2 -> Length) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	return NULL;
    }

    /* Make sure the curve is not closed. If so move first point epsilon. */
    CagdCoerceToE3(Pl1, Crv1 -> Points, 0, PType);
    CagdCoerceToE3(Pl2, Crv1 -> Points, Crv1 -> Length - 1, PType);
    if (IRIT_PT_PT_DIST_SQR(Pl1, Pl2) < IRIT_SQR(LINE_IRIT_EPS))
        Crv1 -> Points[2][0] -= LINE_IRIT_EPS * 10;
    CagdCoerceToE3(Pl1, Crv2 -> Points, 0, PType);
    CagdCoerceToE3(Pl2, Crv2 -> Points, Crv2 -> Length - 1, PType);
    if (IRIT_PT_PT_DIST_SQR(Pl1, Pl2) < IRIT_SQR(LINE_IRIT_EPS))
        Crv2 -> Points[2][0] -= LINE_IRIT_EPS * 10;

    /* Compute the morphing sequence to a line. */
    Crv1MorphList = CrvMorphSeqCornerCut(Crv1, MinDist);
    Crv2MorphList = CrvMorphSeqCornerCut(Crv2, MinDist);

    CagdCoerceToE3(Pl1, Crv1MorphList -> Points, 0, PType);
    CagdCoerceToE3(Vl1, Crv1MorphList -> Points, Length - 1, PType);
    IRIT_PT_SUB(Vl1, Vl1, Pl1);
    Scale1 = IRIT_PT_LENGTH(Vl1);
    IRIT_PT_NORMALIZE(Vl1);

    CagdCoerceToE3(Pl2, Crv2MorphList -> Points, 0, PType);
    CagdCoerceToE3(Vl2, Crv2MorphList -> Points, Length - 1, PType);
    IRIT_PT_SUB(Vl2, Vl2, Pl2);
    Scale2 = IRIT_PT_LENGTH(Vl2);
    IRIT_PT_NORMALIZE(Vl2);

    if (!SameLength) {
	CagdRType Scale1a, Scale2a,
	    MidScale = sqrt(Scale1 * Scale2);
	CagdBBoxStruct BBox1a, BBox1b, BBox2a, BBox2b;

	/* Preserve size as possible. */
	CagdCrvBBox(Crv1MorphList, &BBox1a);
	CagdCrvBBox((CagdCrvStruct *) CagdListLast(Crv1MorphList), &BBox1b);
	CagdCrvBBox(Crv2MorphList, &BBox2a);
	CagdCrvBBox((CagdCrvStruct *) CagdListLast(Crv2MorphList), &BBox2b);

	Scale1a = BBox1a.Max[0] - BBox1a.Min[0] >
	              BBox1a.Max[1] - BBox1a.Min[1] ?
	    (BBox1b.Max[0] - BBox1b.Min[0]) / (BBox1a.Max[0] - BBox1a.Min[0]) :
	    (BBox1b.Max[1] - BBox1b.Min[1]) / (BBox1a.Max[1] - BBox1a.Min[1]);
	Scale2a = BBox2a.Max[0] - BBox2a.Min[0] >
	              BBox2a.Max[1] - BBox2a.Min[1] ?
	    (BBox2b.Max[0] - BBox2b.Min[0]) / (BBox2a.Max[0] - BBox2a.Min[0]) :
	    (BBox2b.Max[1] - BBox2b.Min[1]) / (BBox2a.Max[1] - BBox2a.Min[1]);

	MidScale *= sqrt(Scale1a * Scale2a);
	Scale1 = MidScale / Scale1;
	Scale2 = MidScale / Scale2;
    }
    else {
	CagdRType
	    MidScale = sqrt(Scale1 * Scale2);

	/* Preserve length as possible. */

	Scale1 = MidScale / Scale1;
	Scale2 = MidScale / Scale2;
    }

    Centroid = ComputeCrvCentroid(Crv1);
    IRIT_PT_COPY(Centroid1, Centroid);
    Centroid = ComputeCrvCentroid(Crv2);
    IRIT_PT_COPY(Centroid2, Centroid);
    IRIT_PT_SUB(TranslateFactor, Centroid2, Centroid1);

    IRIT_CROSS_PROD(VTmp, Vl1, Vl2)
    RotateFactor = atan2(VTmp[2], IRIT_DOT_PROD(Vl1, Vl2));

    /* Apply the linear transform from one curve to the other, using the    */
    /* Translation, scaling and rotation factors computed above.	    */
    MorphListLength1 = CagdListLength(Crv1MorphList);
    for (i = 0, Crv = Crv1MorphList, TmpList = NULL;
	 i < MorphListLength1;
	 i++, Crv = Crv -> Pnext) {
	CagdRType
	    t = (MorphListLength1 - i) / ((CagdRType) MorphListLength1),
	    t2 = t * 0.5;
	IrtHmgnMatType RotateMat, ScaleMat, TranslateMat, Mat;

	Centroid = ComputeCrvCentroid(Crv);

	MatGenMatTrans(-Centroid[0], -Centroid[1], -Centroid[2], TranslateMat);
	MatGenMatRotZ1(RotateFactor * t2, RotateMat);
	MatMultTwo4by4(Mat, TranslateMat, RotateMat);

	MatGenMatUnifScale(1.0 + (Scale1 - 1.0) * t, ScaleMat);
	MatMultTwo4by4(Mat, Mat, ScaleMat);

	MatGenMatTrans(Centroid1[0], Centroid1[1], Centroid1[2], TranslateMat);
	MatMultTwo4by4(Mat, Mat, TranslateMat);

	MatGenMatTrans(TranslateFactor[0] * t2,
		       TranslateFactor[1] * t2,
		       TranslateFactor[2] * t2, TranslateMat);
	MatMultTwo4by4(Mat, Mat, TranslateMat);
 	TCrv = CagdCrvMatTransform(Crv, Mat);
	IRIT_LIST_PUSH(TCrv, TmpList);
    }
    CagdCrvFreeList(Crv1MorphList);
    Crv1MorphList = CagdListReverse(TmpList);

    MorphListLength2 = CagdListLength(Crv2MorphList);
    for (i = 0, Crv = Crv2MorphList, TmpList = NULL;
	 i < MorphListLength2;
	 i++, Crv = Crv -> Pnext) {
	CagdRType
	    t = (MorphListLength2 - i) / ((CagdRType) MorphListLength2),
	    t2 = t * 0.5;
	IrtHmgnMatType RotateMat, ScaleMat, TranslateMat, Mat;

	Centroid = ComputeCrvCentroid(Crv);

	MatGenMatTrans(-Centroid[0], -Centroid[1], -Centroid[2], TranslateMat);
	MatGenMatRotZ1(-RotateFactor * t2, RotateMat);
	MatMultTwo4by4(Mat, TranslateMat, RotateMat);

	MatGenMatUnifScale(1.0 + (Scale2 - 1.0) * t, ScaleMat);
	MatMultTwo4by4(Mat, Mat, ScaleMat);

	MatGenMatTrans(Centroid2[0], Centroid2[1], Centroid2[2], TranslateMat);
	MatMultTwo4by4(Mat, Mat, TranslateMat);

	MatGenMatTrans(-TranslateFactor[0] * t2,
		       -TranslateFactor[1] * t2,
		       -TranslateFactor[2] * t2, TranslateMat);
	MatMultTwo4by4(Mat, Mat, TranslateMat);
	TCrv = CagdCrvMatTransform(Crv, Mat);
	IRIT_LIST_PUSH(TCrv, TmpList);
    }
    CagdCrvFreeList(Crv2MorphList);
    Crv2MorphList = CagdListReverse(TmpList);

    if (FilterTangencies) {
	TangencyFilterCrvMorph(&Crv1MorphList, &Crv2MorphList);
    }

    /* Merge the two lists into one. */
    Crv1MorphList = CagdListReverse(Crv1MorphList);
    for (Crv = Crv1MorphList; Crv -> Pnext != NULL; Crv = Crv -> Pnext);
    Crv -> Pnext = Crv2MorphList;

    DistanceFilterCrvMorph(Crv1MorphList, MinDist);

    return Crv1MorphList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two compatible curves (See function CagdMakeCrvsCompatible),         M
* computes a morph between them using multiresolution decomposition.	     M
*   Returned is a list of new blended curves.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:  The two curves to blend.                                    M
*   BlendStep:   A step size of the blending.	                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of blended curves.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTwoCrvsMorphing, SymbTwoCrvsMorphingCornerCut,			     M
*   SymbTwoSrfsMorphing, TrivTwoTVsMorphing                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTwoCrvsMorphingMultiRes, morphing      			             M
*****************************************************************************/
CagdCrvStruct *SymbTwoCrvsMorphingMultiRes(const CagdCrvStruct *Crv1,
					   const CagdCrvStruct *Crv2,
					   CagdRType BlendStep)
{
    CagdRType t;
    CagdCrvStruct *MorphSeq, *TCrv;
    SymbMultiResCrvStruct *CrvMultRes, *Crv1MultRes, *Crv2MultRes;

    if (Crv1 -> PType != Crv2 -> PType ||
	Crv1 -> GType != Crv2 -> GType ||
	Crv1 -> Order != Crv2 -> Order ||
	Crv1 -> Length != Crv2 -> Length) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRVS_INCOMPATIBLE);
	return NULL;
    }
    if (CAGD_IS_RATIONAL_CRV(Crv1)) {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    Crv1MultRes = SymbCrvMultiResDecomp(Crv1, FALSE),
    Crv2MultRes = SymbCrvMultiResDecomp(Crv2, FALSE);
    CrvMultRes = SymbCrvMultiResCopy(Crv1MultRes);
    MorphSeq = CagdCrvCopy(Crv1);

    for (t = BlendStep; t < 1.0; t += BlendStep) {
	int i;

	for (i = 0; i < Crv1MultRes -> Levels; i++) {
	    int l;
	    CagdCrvStruct
		*Crv1MR = Crv1MultRes -> HieCrv[i],
		*Crv2MR = Crv2MultRes -> HieCrv[i],
		*CrvMR = CrvMultRes -> HieCrv[i];
	    CagdRType
		**Points1 = Crv1MR -> Points,
		**Points2 = Crv2MR -> Points,
		**Points = CrvMR -> Points;

	    for (l = 0; l < CrvMR -> Length; l++) {
		InterpolateVector(Points1[1][l], Points1[2][l],
				  Points2[1][l], Points2[2][l],
				  &Points[1][l], &Points[2][l], t);
	    }
	}

	TCrv = SymbCrvMultiResCompos(CrvMultRes);
	IRIT_LIST_PUSH(TCrv, MorphSeq);
    }

    SymbCrvMultiResFree(CrvMultRes);
    SymbCrvMultiResFree(Crv1MultRes);
    SymbCrvMultiResFree(Crv2MultRes);

    return CagdListReverse(MorphSeq);
}
/*****************************************************************************
* DESCRIPTION:                                                               *
* Interpolate between vectors (X1, Y1) and (X2, Y2) by rotating and scaling. *
*                                                                            *
* PARAMETERS:                                                                *
*   X1, Y1:   Coefficients of first vector.                                  *
*   X2, Y2:   Coefficients of second vector.                                 *
*   X, Y:     Place to substitute interpolated vector.                       *
*   Blend:    Blending coefficients. Between zero and one.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InterpolateVector(CagdRType x1,
			      CagdRType y1,
			      CagdRType x2,
			      CagdRType y2,
			      CagdRType *x,
			      CagdRType *y,
			      CagdRType Blend)
{
    CagdRType Size1, Size2, Size, Angle, c, s;
    CagdVType V1, V2, V;

    V1[0] = x1;
    V1[1] = y1;
    V1[2] = 0.0;
    Size1 = IRIT_PT_LENGTH(V1);
    if (Size1 > IRIT_PT_NORMALIZE_ZERO) {
	V1[0] /= Size1;
	V1[1] /= Size1;
    }
    V2[0] = x2;
    V2[1] = y2;
    V2[2] = 0.0;
    Size2 = IRIT_PT_LENGTH(V2);
    if (Size2 > IRIT_PT_NORMALIZE_ZERO) {
	V2[0] /= Size2;
	V2[1] /= Size2;
    }

    IRIT_CROSS_PROD(V, V1, V2)
    Angle = atan2(V[2], IRIT_DOT_PROD(V1, V2)) * Blend;
    c = cos(Angle);
    s = sin(Angle);
    V[0] = V1[0] * c - V1[1] * s;
    V[1] = V1[0] * s + V1[1] * c;
    V[2] = 0.0;
    Size = IRIT_PT_LENGTH(V);
    if (Size > IRIT_PT_NORMALIZE_ZERO) {
	V[0] /= Size;
	V[1] /= Size;
    }
    *x = V[0] * (Size1 * (1.0 - Blend) + Size2 * Blend);
    *y = V[1] * (Size1 * (1.0 - Blend) + Size2 * Blend);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes correspondance between the two sequences and eliminate all curves *
* That a tangency correspondance is found.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvSeq1, CrvSeq2:  Two curves morph3d into a line to filter out          *
*                      tangencies.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TangencyFilterCrvMorph(CagdCrvStruct **CrvSeq1,
				   CagdCrvStruct **CrvSeq2)
{
    while ((*CrvSeq1) -> Pnext != NULL && (*CrvSeq2) -> Pnext != NULL) {
	CagdRType *R;
	CagdCrvStruct
	    *DCrv1 = CagdCrvDerive((*CrvSeq1) -> Pnext),
	    *DCrv2 = CagdCrvDerive((*CrvSeq2) -> Pnext),
	    *TanTan = SymbCrvDotProd(DCrv1, DCrv2);

	CagdCrvFree(DCrv1);
	CagdCrvFree(DCrv2);

	R = SymbExtremumCntPtVals(TanTan -> Points, TanTan -> Length, TRUE);

	CagdCrvFree(TanTan);

	if (R[1] > 0) {
	    CagdCrvStruct *TCrv;

	    TCrv = *CrvSeq1;
	    *CrvSeq1 = (*CrvSeq1) -> Pnext;
	    CagdCrvFree(TCrv);
	    TCrv = *CrvSeq2;
	    *CrvSeq2 = (*CrvSeq2) -> Pnext;
	    CagdCrvFree(TCrv);
	}
	else
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes an estimate distance between two adjacent curves and equalize it. *
*                                                                            *
* PARAMETERS:                                                                *
*   MorphSeq:  The sequence of morphed curves to filter to an approximate    *
*              equal distance.						     *
*   MinDist:   Distance between adjacent curves should be between this and   *
*              twice this much.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DistanceFilterCrvMorph(CagdCrvStruct *MorphSeq,
				   CagdRType MinDist)
{
    CagdCrvStruct *NextCrv;
    CagdRType
	IntMinDist = MinDist * MorphSeq -> Length;

    if (MorphSeq == NULL || MorphSeq -> Pnext == NULL)
        return;

    while ((NextCrv = MorphSeq -> Pnext) != NULL) {
	CagdRType IntDist;

	if ((IntDist = EstimateIntDistCrvCrv(MorphSeq, NextCrv)) <
								IntMinDist &&
	    NextCrv -> Pnext != NULL) {
	    MorphSeq -> Pnext = NextCrv -> Pnext;
	    CagdCrvFree(NextCrv);
	}
	else if (IntDist > IntMinDist * 2.0) {
	    CagdCrvStruct
	        *MidCrv = SymbCrvAdd(MorphSeq, NextCrv);

	    CagdCrvTransform(MidCrv, NULL, 0.5);

	    MidCrv -> Pnext = NextCrv;
	    MorphSeq -> Pnext = MidCrv;
	}
	else
	    MorphSeq = MorphSeq -> Pnext;
    }
}


/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes an approximation to the given curve's centroid as an average of   *
* its control points.                                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:        To compute centroid for.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   Centroid point.                                           *
*****************************************************************************/
static CagdRType *ComputeCrvCentroid(const CagdCrvStruct *Crv)
{
    IRIT_STATIC_DATA CagdRType Centroid[3];
    int i, j,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType),
	Length = Crv -> Length;
    CagdRType
	* const *Points = Crv -> Points;

    for (j = 0; j < MaxAxis; j++)
      Centroid[j] = 0.0;

    for (i = 0; i < Length; i++) {
	for (j = 1; j <= MaxAxis; j++)
	    Centroid[j - 1] += Points[j][i];
    }

    for (j = 0; j < MaxAxis; j++)
	Centroid[j] /= Length;

    return Centroid;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes a sequence of morphed curves from the given curve and upto a line *
* using corner cutting. Curve is assumed to be at the origin.                *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:        To corner cut to a line.                                     *
*   MinDist:    Minimal maximum distance between adjacent curves to make     *
*               sure motion is visible. The curves will move at most twice   *
*               that much in their maximal distance (roughly).		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *: List of curves representing the corner cut morph.	     *
*****************************************************************************/
static CagdCrvStruct *CrvMorphSeqCornerCut(const CagdCrvStruct *Crv,
					   CagdRType MinDist)
{
    int i,
	Length = Crv -> Length;
    CagdPType Pt, Pl;
    CagdVType Vl;
    CagdCrvStruct
	*LastCrv = CagdCrvCopy(Crv),
	*MorphList = CagdCrvCopy(Crv);
    CagdRType
	IntMinDist = MinDist * Crv -> Length * 10.0;

    while (TRUE) {
	CagdCrvStruct
	    *TCrv = CrvMorphOneCornerCut(LastCrv, MinDist);

	CagdCoerceToE3(Pl, TCrv -> Points, 0, TCrv -> PType);
	CagdCoerceToE3(Vl, TCrv -> Points, TCrv -> Length - 1, TCrv -> PType);
	IRIT_PT_SUB(Vl, Vl, Pl);

	for (i = 1; i < Length - 1; i++) {
	    CagdCoerceToE3(Pt, TCrv -> Points, i, TCrv -> PType);
	    if (GMDistPointLine(Pt, Pl, Vl) > LINE_IRIT_EPS)
	        break;
	}
	if (i >= Length - 1) {
	    IRIT_LIST_PUSH(TCrv, MorphList);
	    break;
	}
	else {
	    CagdRType
	        DistBound = EstimateIntDistCrvCrv(TCrv, MorphList);

	    CagdCrvFree(LastCrv);

	    if (DistBound < IntMinDist) {
		LastCrv = TCrv;
	    }
	    else {
		IRIT_LIST_PUSH(TCrv, MorphList);
		LastCrv = CagdCrvCopy(TCrv);
	    }
	}
    }

    if (LastCrv != NULL)
	CagdCrvFree(LastCrv);

    return MorphList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Estimates the integrated distance between two curves as the sum of         *
* distances between corresponding control points. Curves are assumed         *
* compatible.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1, Crv2:  Two curves to estimate integral distance between.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Integral distance bound.                                    *
*****************************************************************************/
static CagdRType EstimateIntDistCrvCrv(const CagdCrvStruct *Crv1,
				       const CagdCrvStruct *Crv2)
{
    int i,
	Length = Crv1 -> Length;
    CagdRType
	IntDist = 0.0,
	* const *Points1 = Crv1 -> Points,
	* const *Points2 = Crv2 -> Points;

    if (Crv1 -> PType != CAGD_PT_E2_TYPE && Crv1 -> PType != CAGD_PT_E3_TYPE) {
	SYMB_FATAL_ERROR(SYMB_ERR_UNSUPPORT_PT);
	return 0.0;
    }
    
    for (i = 0; i < Length; i++)
    {
	IntDist += IRIT_FABS(Points1[1][i] - Points2[1][i]) +
	           IRIT_FABS(Points1[2][i] - Points2[2][i]) +
		   (Points1[3] != NULL ? IRIT_FABS(Points1[3][i] - Points2[3][i])
				       : 0.0);
    }

    return IntDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Given a curve computes a smoother version of it using corner cutting.      *
* The curve is assumed to be positioned around the origin.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:	 The curve to corner cut.                                    *
*   Blend:       A parameter between zero and one. Zero will have zero       *
*		 affect on the curve, one the maximum corner cutting.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  The smoothed curve.                                    *
*****************************************************************************/
static CagdCrvStruct *CrvMorphOneCornerCut(const CagdCrvStruct *Crv,
					   CagdRType Blend)
{
    int i, j,
	Length = Crv -> Length,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *SmoothedCrv;
    CagdRType * const *Pts, **SmthPts, MinDist, TotalLength, TotalNewLength,
	*Nodes = CagdCrvNodes(Crv);

    SmoothedCrv = CagdCrvCopy(Crv);
    SmthPts = SmoothedCrv -> Points;
    Pts = Crv -> Points;

    MinDist = IRIT_INFNTY;
    TotalLength = 0.0;
    for (i = 1; i < Length; i++) {
	CagdRType
	    Dist = sqrt(IRIT_SQR(SmthPts[1][i] - SmthPts[1][i - 1]) +
			IRIT_SQR(SmthPts[2][i] - SmthPts[2][i - 1]) +
			IRIT_SQR(SmthPts[3][i] - SmthPts[3][i - 1]));

	TotalLength += Dist;
	if (MinDist > Dist)
	    MinDist = Dist;
    }

    for (i = 1; i < Length - 1; i++) {
	CagdRType
	    Dist1 = sqrt(IRIT_SQR(Pts[1][i] - Pts[1][i - 1]) +
			 IRIT_SQR(Pts[2][i] - Pts[2][i - 1]) +
			 IRIT_SQR(Pts[3][i] - Pts[3][i - 1])),
	    Dist2 = sqrt(IRIT_SQR(Pts[1][i] - Pts[1][i + 1]) +
			 IRIT_SQR(Pts[2][i] - Pts[2][i + 1]) +
			 IRIT_SQR(Pts[3][i] - Pts[3][i + 1])),
	    BlendNeighbor1 = Blend * IRIT_SQR(MinDist / Dist2),
	    BlendNeighbor2 = Blend * IRIT_SQR(MinDist / Dist1),
	    BlendSelf = 1.0 - BlendNeighbor1 - BlendNeighbor2;

	for (j = 1; j <= MaxAxis; j++)
	    SmthPts[j][i] = Pts[j][i] * BlendSelf +
		            Pts[j][i - 1] * BlendNeighbor1 +
		            Pts[j][i + 1] * BlendNeighbor2;
    }
    IritFree(Nodes);

    TotalNewLength = 0.0;
    for (i = 1; i < Length; i++) {
	CagdRType
	    Dist = sqrt(IRIT_SQR(SmthPts[1][i] - SmthPts[1][i - 1]) +
			IRIT_SQR(SmthPts[2][i] - SmthPts[2][i - 1]) +
			IRIT_SQR(SmthPts[3][i] - SmthPts[3][i - 1]));

	TotalNewLength += Dist;
    }

    CagdCrvTransform(SmoothedCrv, NULL, TotalLength / TotalNewLength);

    return SmoothedCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two compatible surfaces (See function CagdMakeSrfsCompatible),       M
* computes a convex blend between them according to Blend which must be      M
* between zero and one.							     M
*   Returned is the new blended surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  The two surfaces to blend.                                  M
*   Blend:       A parameter between zero and one                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Srf2 * Blend + Srf1 * (1 - Blend).                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTwoCrvsMorphing, SymbTwoCrvsMorphingCornerCut,			     M
*   SymbTwoCrvsMorphingMultiRes, TrivTwoTVsMorphing                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbTwoSrfsMorphing, morphing                                            M
*****************************************************************************/
CagdSrfStruct *SymbTwoSrfsMorphing(const CagdSrfStruct *Srf1,
				   const CagdSrfStruct *Srf2,
				   CagdRType Blend)
{
    int i, j,
	MaxAxis = CAGD_NUM_OF_PT_COORD(Srf1 -> PType),
	ULength = Srf1 -> ULength,
	VLength = Srf1 -> VLength,
	UOrder = Srf1 -> UOrder,
	VOrder = Srf1 -> VOrder;
    CagdRType **NewPoints,
	* const *Points1 = Srf1 -> Points,
	* const *Points2 = Srf2 -> Points,
	Blend1 = 1.0 - Blend;
    CagdSrfStruct *NewSrf;

    if (Srf1 -> PType != Srf2 -> PType ||
	Srf1 -> GType != Srf2 -> GType ||
	UOrder != Srf2 -> UOrder ||
	VOrder != Srf2 -> VOrder ||
	ULength != Srf2 -> ULength ||
	VLength != Srf2 -> VLength) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRFS_INCOMPATIBLE);
	return NULL;
    }
	
    NewSrf = CagdSrfNew(Srf1 -> GType, Srf1 -> PType, ULength, VLength);
    NewSrf -> UOrder = UOrder;
    NewSrf -> VOrder = VOrder;
    NewPoints = NewSrf -> Points;
    if (Srf1 -> UKnotVector != NULL)
	NewSrf -> UKnotVector = BspKnotCopy(NULL, Srf1 -> UKnotVector,
					    ULength + UOrder);
    if (Srf1 -> VKnotVector != NULL)
	NewSrf -> VKnotVector = BspKnotCopy(NULL, Srf1 -> VKnotVector,
					    VLength + VOrder);

    for (i = !CAGD_IS_RATIONAL_PT(Srf1 -> PType); i <= MaxAxis; i++) {
	CagdRType
	    *Pts1 = &Points1[i][0],
	    *Pts2 = &Points2[i][0],
	    *NewPts = &NewPoints[i][0];

	for (j = ULength * VLength - 1; j >= 0; j--)
	    *NewPts++ = *Pts1++ * Blend1 + *Pts2++ * Blend;
    }

    return NewSrf;
}
