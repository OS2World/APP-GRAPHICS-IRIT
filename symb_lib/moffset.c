/******************************************************************************
* Offset.c - computes offset approximation to curves and surfaces.            *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, March. 93.					      *
******************************************************************************/

#include "symb_loc.h"
#include "extra_fn.h"

#define OFFSET_INFLECTION_IRIT_EPS	1e-5
#define MAX_OFFSET_MATCH_ITERS		10

static CagdCrvStruct *SymbCrvCrvConvAux(CagdCrvStruct *Crv1,
					CagdCrvStruct *Crv2,
					CagdRType OffsetDist,
					CagdRType Tolerance,
					CagdVType T1,
					CagdVType T2);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an offset to a freeform curve using matching of tangent fields. M
* The given curve is split at all its inflection points, made sure it spans  M
* less than 90 degrees, and then is matched against an arc of the proper     M
* angular span of tangents.						     M
*   Unlike other offset methods, this method allways preserves the distance  M
* between the original curve ans its offset. The error in this methods can   M
* surface only in the non orthogonality of the offset direction.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          To approximate its offset curve with distance OffsetDist.  M
*   OffsetDist:   Amount of offset. Negative denotes other offset direction. M
*   Tolerance:    Of angular discrepancy that is allowed.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The offset curve approximation.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,  M
*   SymbCrvAdapOffset, SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset,       M
*   SymbCrvCrvConvolution						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvMatchingOffset                                                    M
*****************************************************************************/
CagdCrvStruct *SymbCrvMatchingOffset(CagdCrvStruct *Crv,
				     CagdRType OffsetDist,
				     CagdRType Tolerance)
{
    CagdCrvStruct *OffCrv,
	*OffCrvList = NULL,
	*OrigCrv = Crv;
    CagdPtStruct *Pt,
	*Pts = SymbCrv2DInflectionPts(Crv, OFFSET_INFLECTION_IRIT_EPS);

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
	CagdCrvStruct
	    *Crv1 = CagdCrvSubdivAtParam(Crv, Pt -> Pt[0]),
	    *Crv2 = Crv1 -> Pnext,
	    *Off1Crv = SymbCrvCrvConvolution(Crv1, NULL, OffsetDist, Tolerance);

	IRIT_LIST_PUSH(Off1Crv, OffCrvList);

	if (Crv != OrigCrv)
	    CagdCrvFree(Crv);
	Crv = Crv2;

	CagdCrvFree(Crv1);
    }
    CagdPtFreeList(Pts);

    OffCrv = SymbCrvCrvConvolution(Crv, NULL, OffsetDist, Tolerance);
    IRIT_LIST_PUSH(OffCrv, OffCrvList);
    if (Crv != OrigCrv)
	CagdCrvFree(Crv);

    OffCrvList = CagdListReverse(OffCrvList);    
    OffCrv = CagdMergeCrvList(OffCrvList, TRUE);
    CagdCrvFreeList(OffCrvList);

    return OffCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the convolution of the given two curves by matching their       M
* tangents and reparametrizing Crv2.					     M
*   If Crv2 is NULL, an Arc of radius OffsetDist is used, resulting in an    M
* offset operation of Crv1.						     M
*   Both Crv1 and Crv2 are assumed to have no inflection points and to       M
* span the same angular domain. That is Crv1'(0) || Crv2'(0) and similarly   M
* Crv1'(1) || Crv2'(1), where || denotes parallel.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curves to convolve.				     M
*   OffsetDist:   Amount of offset, if Crv2 == NULL. Negative value denotes  M
*		  other offset/convolution direction.			     M
*   Tolerance:    Of angular discrepancy that is allowed.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:    The offset curve approximation.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvOffset, SymbCrvSubdivOffset, SymbSrfOffset, SymbSrfSubdivOffset,  M
*   SymbCrvAdapOffset, SymbCrvAdapOffsetTrim, SymbCrvLeastSquarOffset,       M
*   SymbCrvMatchingOffset						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvCrvConvolution                                                    M
*****************************************************************************/
CagdCrvStruct *SymbCrvCrvConvolution(CagdCrvStruct *Crv1,
				     CagdCrvStruct *Crv2,
				     CagdRType OffsetDist,
				     CagdRType Tolerance)
{
    int i;
    CagdCrvStruct
	*Crv1E2 = CagdCoerceCrvTo(Crv1, CAGD_PT_E2_TYPE, FALSE),
	*DCrv1E2 = CagdCrvDerive(Crv1E2);
    CagdRType
	**Points = DCrv1E2 -> Points,
	*XPts = Points[1],
	*YPts = Points[2];

    CagdCrvFree(Crv1E2);

    /* Scans the curve's Hodograph for a vector that is more than 90 degrees */
    /* away from the initial vector, and if found one, must subdivide Crv.   */
    for (i = 1; i < DCrv1E2 -> Length;  i++)
	if (XPts[0] * XPts[i] + YPts[0] * YPts[i] < 0)
	    break;

    if (i < DCrv1E2 -> Length) {
	CagdRType TMin, TMax;
	CagdCrvStruct *ConvCrv, *Crv1Subdiv, *ConvCrv1Sub1,
				*Crv2Subdiv, *ConvCrv1Sub2;

	CagdCrvDomain(Crv1, &TMin, &TMax);

	Crv1Subdiv = CagdCrvSubdivAtParam(Crv1, (TMin + TMax) * 0.5);
	if (Crv2 != NULL)
	    Crv2Subdiv = CagdCrvSubdivAtParam(Crv2, (TMin + TMax) * 0.5);
	else
	    Crv2Subdiv = NULL;

	ConvCrv1Sub1 =
	    SymbCrvCrvConvolution(Crv1Subdiv,
				  Crv2 != NULL ? Crv2Subdiv : NULL,
				  OffsetDist, Tolerance);
	ConvCrv1Sub2 =
	    SymbCrvCrvConvolution(Crv1Subdiv -> Pnext,
				  Crv2 != NULL ? Crv2Subdiv -> Pnext : NULL,
				  OffsetDist, Tolerance);
	CagdCrvFreeList(Crv1Subdiv);
	if (Crv2 != NULL)
	    CagdCrvFreeList(Crv2Subdiv);

	ConvCrv = CagdMergeCrvCrv(ConvCrv1Sub1, ConvCrv1Sub2, TRUE);
	CagdCrvFree(ConvCrv1Sub1);
	CagdCrvFree(ConvCrv1Sub2);

	CagdCrvFree(DCrv1E2);

	return ConvCrv;
    }
    else {
	/* Curve does not span more than 90 degs - approximate by matching. */
	CagdVType T1, T2;

	T1[0] = -YPts[0];
	T1[1] = XPts[0];
	T1[2] = 0.0;
	T2[0] = -YPts[DCrv1E2 -> Length - 1];
	T2[1] = XPts[DCrv1E2 -> Length - 1];
	T2[2] = 0.0;
	CagdCrvFree(DCrv1E2);

	return SymbCrvCrvConvAux(Crv1, Crv2, OffsetDist, Tolerance, T1, T2);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   An Auxiliary function of SymbCrvCrvConvolution. Matches a given curve(s) *
* with no inflection points and angular tangent span between T1 and T2 of    *
* less than 90 degrees to an arc of same angular span and return the         *
* approximated convolved curve.						     *
*****************************************************************************/
static CagdCrvStruct *SymbCrvCrvConvAux(CagdCrvStruct *Crv1,
					CagdCrvStruct *Crv2,
					CagdRType OffsetDist,
					CagdRType Tolerance,
					CagdVType T1,
					CagdVType T2)
{
    CagdBType NewCrv2;
    int NumIters = 0,
	Reduce = 3,
	SampleSet = 15;
    CagdRType
	MaxError = 0.0;
    CagdPtStruct Start, Center, End;
    CagdCrvStruct *CTmp, *Crv1Match, *ConvCrv;

    if (Crv2 == NULL) {
	/* Constructs the proper arc with the right orientation as detected  */
	/* by the cross product of the two angular tangents T1, and T2.	     */
	NewCrv2 = TRUE;

	IRIT_PT_NORMALIZE(T1);
	IRIT_PT_SCALE(T1, IRIT_FABS(OffsetDist));
	IRIT_PT_NORMALIZE(T2);
	IRIT_PT_SCALE(T2, IRIT_FABS(OffsetDist));
	IRIT_PT_COPY(Start.Pt, T1);
	IRIT_PT_COPY(End.Pt, T2);
	IRIT_PT_RESET(Center.Pt);
	CTmp = BzrCrvCreateArc(&Start, &Center, &End);
	Crv2 = CagdCoerceCrvTo(CTmp, CAGD_PT_P2_TYPE, FALSE);
	CagdCrvFree(CTmp);

	if (T1[0] * T2[1] - T1[1] * T2[0] > 0) {
	    /* Reverse the generated arc by 180 rotation along Z. */
	    CagdCrvTransform(Crv2, NULL, -1);
	}
	else
	    OffsetDist *= -1.0;
    }
    else
	NewCrv2 = FALSE;

    do {
	int i;
	CagdCrvStruct *DCrv1Match, *ErrorCrv, *ErrorCrvSqr, *ErrorCrvE1,
		*DCrv1MatchLenSqr, *RecipDCrv1MatchLenSqr;
	CagdRType *Pts;

	if ((Crv1Match = CagdMatchingTwoCurves(Crv2, Crv1, Reduce, SampleSet,
					       2, FALSE, FALSE, FALSE,
					       NULL)) != NULL) {
	    DCrv1Match = CagdCrvDerive(Crv1Match);

	    /* Compute the error functional as				   */
	    /*                <Crv2(t), DCrv1Match(t)>^2	 	   */
	    /* ErrorCrv(t) = ----------------------------		   */
	    /*               <DCrv1Match(t), DCrv1Match(t)>		   */
	    DCrv1MatchLenSqr = SymbCrvDotProd(DCrv1Match, DCrv1Match);
	    RecipDCrv1MatchLenSqr = SymbCrvInvert(DCrv1MatchLenSqr);
	    CagdCrvFree(DCrv1MatchLenSqr);

	    ErrorCrv = SymbCrvDotProd(Crv2, DCrv1Match);
	    CagdCrvFree(DCrv1Match);
	    ErrorCrvSqr = SymbCrvMult(ErrorCrv, ErrorCrv);
	    CagdCrvFree(ErrorCrv);

	    ErrorCrv = SymbCrvMult(ErrorCrvSqr, RecipDCrv1MatchLenSqr);
	    CagdCrvFree(ErrorCrvSqr);
	    CagdCrvFree(RecipDCrv1MatchLenSqr);

	    ErrorCrvE1 = CagdCoerceCrvTo(ErrorCrv, CAGD_PT_E1_TYPE, FALSE);
	    CagdCrvFree(ErrorCrv);

	    MaxError = 0.0;

	    /* Get a bound on the maximal angular error. */
	    for (i = 0, Pts = ErrorCrvE1 -> Points[1];
		 i < ErrorCrvE1 -> Length;
		 i++, Pts++) {
		if (MaxError < *Pts)
		    MaxError = *Pts;
	    }
	    CagdCrvFree(ErrorCrvE1);

	    MaxError = sqrt(MaxError);
	    MaxError /= IRIT_FABS(OffsetDist);
	    /* Convert to degrees relative to the 90 orthogonal. */
	    MaxError = IRIT_FABS((M_PI_DIV_2 - acos(MaxError)) * 
							   IRIT_RAD2DEG_CNVRT);
	}
	else
	    MaxError = IRIT_INFNTY;

	if (MaxError > Tolerance && ++NumIters < MAX_OFFSET_MATCH_ITERS) {
	    /* Going for another round - free current match/set new params. */
	    if (Crv1Match != NULL)
		CagdCrvFree(Crv1Match);

	    Reduce += Reduce;
	    SampleSet += SampleSet;
	}
    }
    while (MaxError > Tolerance && NumIters < MAX_OFFSET_MATCH_ITERS);

    if (Crv1Match == NULL) {
	SYMB_FATAL_ERROR(SYMB_ERR_MATCH_FAILED);
	return NULL;
    }

    if (OffsetDist > 0)
	ConvCrv = SymbCrvAdd(Crv1Match, Crv2);
    else
	ConvCrv = SymbCrvSub(Crv1Match, Crv2);

    if (NewCrv2)
	CagdCrvFree(Crv2);
    CagdCrvFree(Crv1Match);

    return ConvCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes an elevated surface emenating from the given C^1 continuous     M
* curve in all directions like a fire front.  The surface gets away from Crv M
* in a slope of 45 degrees.  This elevated surface is an approximation of    M
* the real envelope only, as prescribed by Tolerance.                        M
*   If the given curve is closed, it is assume to be C^1 at the end point as M
* well.  For a close curve, two surfaces are actually returned - one for the M
* inside and one for the outside firefront.				     M
*   This function employs SymbCrvSubdivOffset for the offset computations.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         The curve to process.                                       M
*   Height:      The height of the elevated surface (also the width of the   M
*		 offset operation.					     M
*   Tolerance:   Accuracy of the elevated surface approximation.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A freeform surface approximating the elevated surface M
*		for open curve Crv, or two surfaces for the case of closed   M
*		curve Crv.                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbEnvOffsetFromCrv                                                     M
*****************************************************************************/
CagdSrfStruct *SymbEnvOffsetFromCrv(const CagdCrvStruct *Crv,
				    CagdRType Height,
				    CagdRType Tolerance)
{
    CagdBType ClosedCurve,
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdPType Trans, PtStart, PtEnd;
    CagdRType TMin, TMax, *R;
    CagdCrvStruct *TCrv, *Crv1, *Crv2, *CrvOff1, *CrvOff2;
    CagdSrfStruct *Srf1, *Srf2, *TSrf, *TSrf2;

    /* Check if the curve is closed or not. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    R = CagdCrvEval(Crv, TMin);
    CagdCoerceToE3(PtStart, &R, -1, Crv -> PType);
    R = CagdCrvEval(Crv, TMax);
    CagdCoerceToE3(PtEnd, &R, -1, Crv -> PType);
    ClosedCurve = IRIT_PT_APX_EQ(PtStart, PtEnd);

    /* Make sure we have a two dimensional curve, for the offset. */
    if (MaxCoord != 2) {
	Crv1 = CagdCoerceCrvTo(Crv, IsNotRational ? CAGD_PT_E2_TYPE
						  : CAGD_PT_P2_TYPE, FALSE);
	Crv2 = CagdCrvReverse(Crv1);
    }
    else {
	Crv1 = CagdCrvCopy(Crv);
	Crv2 = CagdCrvReverse(Crv);
    }

    /* Compute offsets in the plane and translate to proper heights. */
    TCrv = SymbCrvSubdivOffset(Crv1, Height, Tolerance, FALSE);
    CrvOff1 = CagdCoerceCrvTo(TCrv, IsNotRational ? CAGD_PT_E3_TYPE
						  : CAGD_PT_P3_TYPE, FALSE);
    CagdCrvFree(TCrv);
    TCrv = SymbCrvSubdivOffset(Crv2, Height, Tolerance, FALSE);
    CrvOff2 = CagdCoerceCrvTo(TCrv, IsNotRational ? CAGD_PT_E3_TYPE
						  : CAGD_PT_P3_TYPE, FALSE);
    CagdCrvFree(TCrv);

    Trans[0] = Trans[1] = 0.0;
    Trans[2] = Height;
    CagdCrvTransform(CrvOff1, Trans, 1.0);
    CagdCrvTransform(CrvOff2, Trans, 1.0);

    /* Create the end closing half cones, for open curve segments. */
    if (ClosedCurve) {
	int Len1, i;
	CagdRType **Points;

	/* Make sure end points of CrvOff1, CrvOff2 are identical as well. */
	Points = CrvOff1 -> Points;
	Len1 = CrvOff1 -> Length - 1;
	for (i = IsNotRational; i <= MaxCoord; i++)
	    Points[i][0] =
	        Points[i][Len1] = (Points[i][0] + Points[i][Len1]) * 0.5;
	Points = CrvOff2 -> Points;
	Len1 = CrvOff2 -> Length - 1;
	for (i = IsNotRational; i <= MaxCoord; i++)
	    Points[i][0] =
	        Points[i][Len1] = (Points[i][0] + Points[i][Len1]) * 0.5;

	Srf1 = CagdRuledSrf(Crv1, CrvOff1, 2, 2);
	Srf2 = CagdRuledSrf(Crv2, CrvOff2, 2, 2);
	Srf1 -> Pnext = Srf2;
    }
    else {
	CagdPtStruct Pt1, Pt2;
	CagdCrvStruct *DiagLine;
	IrtHmgnMatType Mat;
	CagdVecStruct *Tan;
	CagdSrfStruct *HalfCone;

	/* Create half a cone with the proper dimensions. */
	IRIT_PT_RESET(Pt1.Pt);
	Pt2.Pt[0] = Height;
	Pt2.Pt[1] = 0.0;
	Pt2.Pt[2] = Height;
	DiagLine = CagdMergePtPt(&Pt1, &Pt2);

	if (IsNotRational)
	    TSrf = CagdSurfaceRevPolynomialApprox(DiagLine);
	else
	    TSrf = CagdSurfaceRev(DiagLine);
	CagdCrvFree(DiagLine);

	HalfCone = CagdSrfRegionFromSrf(TSrf, 0.0, 2.0, CAGD_CONST_U_DIR);
	CagdSrfFree(TSrf);

	/* Compute the two side walls. */
	Srf1 = CagdRuledSrf(Crv1, CrvOff1, 2, 2);
	Srf2 = CagdRuledSrf(Crv2, CrvOff2, 2, 2);

	/* Position the first half cone at the end of the first side wall   */
	/* and merge with the first side wall.				    */
	Tan = CagdCrvTangent(Crv, TMax, TRUE);
	MatGenMatRotZ1(atan2(Tan -> Vec[1], Tan -> Vec[0]) - M_PI_DIV_2, Mat);
	TSrf = CagdSrfMatTransform(HalfCone, Mat);
	CagdSrfTransform(TSrf, PtEnd, 1.0);
	TSrf2 = CagdMergeSrfSrf(Srf1, TSrf, CAGD_CONST_U_DIR, TRUE, FALSE);
	CagdSrfFree(Srf1);
	CagdSrfFree(TSrf);

	/* Merge in the second side wall. */
	Srf1 = CagdMergeSrfSrf(TSrf2, Srf2, CAGD_CONST_U_DIR, TRUE, FALSE);
	CagdSrfFree(TSrf2);
	CagdSrfFree(Srf2);

	/* Position the second half cone at the beginning of the first side */
	/* wall and merge at the end of the second side wall.		    */
	Tan = CagdCrvTangent(Crv, TMin, TRUE);
	MatGenMatRotZ1(atan2(Tan -> Vec[1], Tan -> Vec[0]) + M_PI_DIV_2, Mat);
	TSrf = CagdSrfMatTransform(HalfCone, Mat);
	CagdSrfFree(HalfCone);
	CagdSrfTransform(TSrf, PtStart, 1.0);
	TSrf2 = CagdMergeSrfSrf(Srf1, TSrf, CAGD_CONST_U_DIR, TRUE, FALSE);
	CagdSrfFree(Srf1);
	CagdSrfFree(TSrf);

	Srf1 = TSrf2;
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(CrvOff1);
    CagdCrvFree(CrvOff2);

    return Srf1;
}

