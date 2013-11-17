/*****************************************************************************
*   A matching code between two freeform curves.			     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Shmuel Cohen			       Ver 0.1, May 1995.    *
*****************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define CAGD_CM_MAX_SAMPLE_NUM		500
#define CAGD_CM_BISCT_MIN_IRIT_DOT_PROD	0.001

typedef struct LinkStruct {
    int x, y;
} LinkStruct;

IRIT_STATIC_DATA int 
    GlblShiftFlag = 1,
    GlblNumberOfSamples,
    GlblRotationFlag = FALSE,
    GlblAllowNegativeNorm = FALSE,
    GlblIgnoreUpperBoundLimit = FALSE;

static CagdBType FindTangentsVec(
			      const CagdCrvStruct *Crv,
			      CagdVType TangentsVector[CAGD_CM_MAX_SAMPLE_NUM],
			      CagdVType CrvEval[CAGD_CM_MAX_SAMPLE_NUM]);
static void MatchingTest(CagdVType TangentsVectorsCrv1[CAGD_CM_MAX_SAMPLE_NUM],
			 CagdVType TangentsVectorsCrv2[CAGD_CM_MAX_SAMPLE_NUM],
			 CagdRType TangentsDotProdRes[CAGD_CM_MAX_SAMPLE_NUM],
			 int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
			 int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM]);
static void RotateVector(CagdVType SrcVec,
			 CagdVType DestVec,
			 CagdRType Angle);
static void CopyTwoArrays(int *Dest1, int *Dest2, int *Src1, int *Src2);
static CagdRType AngleBetweenVectors(CagdVType Vector1, CagdVType Vector2);
static void FindWayInMat(int i,
			 int j,
			 int *Cnt,
			 LinkStruct MatchMat[CAGD_CM_MAX_SAMPLE_NUM]
                                            [CAGD_CM_MAX_SAMPLE_NUM],
			 CagdRType MaxMat[CAGD_CM_MAX_SAMPLE_NUM + 1]
                                         [CAGD_CM_MAX_SAMPLE_NUM + 1],
			 int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
			 int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM]);
static CagdBType IfCrvIsClose(const CagdCrvStruct *Crv);
static CagdRType FindNewMatching(CagdVType TanVecCrv1[CAGD_CM_MAX_SAMPLE_NUM],
				 CagdVType TanVecCrv2[CAGD_CM_MAX_SAMPLE_NUM],
				 CagdVType Crv1Eval[CAGD_CM_MAX_SAMPLE_NUM],
				 CagdVType Crv2Eval[CAGD_CM_MAX_SAMPLE_NUM],
				 int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
				 int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM],
				 CagdMatchNormFuncType MatchNormFunc);
static int FindBestMatching(CagdVType TanVecCrv1[CAGD_CM_MAX_SAMPLE_NUM],
			    CagdVType TanVecCrv2[CAGD_CM_MAX_SAMPLE_NUM],
			    CagdVType Crv1Eval[CAGD_CM_MAX_SAMPLE_NUM],
			    CagdVType Crv2Eval[CAGD_CM_MAX_SAMPLE_NUM],
			    int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
			    int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM],
			    CagdMatchNormFuncType MatchNormFunc);
static CagdCrvStruct *CreateReparamCrv(int *Vect,
				       int *Vecr,
				       int Order,
				       int Len,
				       int Reduce);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the tangent vector to the curve at each parameter.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   The input curve.                                                  *
*   TangentsVector:   An array for the tangent at each parameter.            *
*   CrvEval:   The value of the curve at each parameter.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:   TRUE for Sucssess.                                          *
*****************************************************************************/
static CagdBType FindTangentsVec(
			      const CagdCrvStruct *Crv,
			      CagdVType TangentsVector[CAGD_CM_MAX_SAMPLE_NUM],
			      CagdVType CrvEval[CAGD_CM_MAX_SAMPLE_NUM])

{
    int i;
    CagdRType StartCrvPar, EndCrvPar, Par, *Pt, DPt[CAGD_MAX_PT_SIZE], DPar;
    CagdCrvStruct
	*TanCrv = CagdCrvDeriveScalar(Crv);
    CagdPointType
	PType = TanCrv -> PType;

    CagdCrvDomain(Crv, &StartCrvPar, &EndCrvPar);

    DPar = (EndCrvPar -  StartCrvPar) / (GlblNumberOfSamples - 1);
    for (i = 0, Par = StartCrvPar;
	 i < GlblNumberOfSamples;
	 i++, Par += DPar) { 
	if (Par > EndCrvPar)      /* Due to floating point roundoff errors. */
	    Par = EndCrvPar;

	Pt = CagdCrvEval(TanCrv, Par);
	IRIT_GEN_COPY(DPt, Pt, sizeof(CagdRType) * CAGD_MAX_PT_SIZE);

	Pt = CagdCrvEval(Crv , Par);
	CagdCoerceToE3(CrvEval[i], &Pt, -1, TanCrv -> PType);

	if (CAGD_IS_RATIONAL_CRV(TanCrv)) {
	    int j;

	    for (j = 1; j <= IRIT_MIN(CAGD_NUM_OF_PT_COORD(PType), 3); j++) {
		if (Pt[0] == 0)
		    TangentsVector[i][j - 1] = 0.0;
		else
		    TangentsVector[i][j - 1] =
		        (DPt[j] * Pt[0] - Pt[j] * DPt[0]) / IRIT_SQR(Pt[0]);
	    }
	}
	else {
	    Pt = DPt;
	    CagdCoerceToE3(TangentsVector[i], &Pt, -1, PType);
	}

	IRIT_PT_NORMALIZE(TangentsVector[i]);
    }

    CagdCrvFree(TanCrv);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks the matching with the tangents' inner product norm.               *
*                                                                            *
* PARAMETERS:                                                                *
*   TangentsVectorsCrv1:  An array for the first curve's tangents.           *
*   TangentsVectorsCrv2:  An array for the second curve's tangents.          *
*   TangentsDotProdRes:   The inner product resulting vector.                *
*   NewIndex1:            The vector of the new matching to the first curve. *
*   NewIndex2:            The vector of the new matching to the second curve.*
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MatchingTest(CagdVType TangentsVectorsCrv1[CAGD_CM_MAX_SAMPLE_NUM],
			 CagdVType TangentsVectorsCrv2[CAGD_CM_MAX_SAMPLE_NUM],
			 CagdRType TangentsDotProdRes[CAGD_CM_MAX_SAMPLE_NUM],
			 int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
			 int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM])
{
    int i;
    CagdRType Sum;

    Sum = 0;
    for (i = 0; i < 2 * GlblNumberOfSamples && NewIndex1[i] != -1; i++) {
	TangentsDotProdRes[i] =
	    IRIT_DOT_PROD(TangentsVectorsCrv1[NewIndex1[i]],
		     TangentsVectorsCrv2[NewIndex2[i]]);
	Sum += TangentsDotProdRes[i];          

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMatch2Crv, FALSE) {
	        if (TangentsDotProdRes[i] <= 0)
		    IRIT_WARNING_MSG_PRINTF("NO MATCHING at sample point number: %d with %d\n",
					    NewIndex1[i], NewIndex2[i]);
	    }
	}
#	endif /* DEBUG */
    }

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMatch2Crv2, FALSE) {
	    IRIT_INFO_MSG_PRINTF("\nThe sum after testing matching is: %lf\n",
				    Sum);
	}
    }
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Copies two integer vectors.                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Dest1: The first destination vector.                                     *
*   Dest2: The second destination vector.                                    *
*   Src1:  The first source vector.                                          *
*   Src2:  The second source vector.                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CopyTwoArrays(int *Dest1, int *Dest2, int *Src1, int *Src2)
{
    IRIT_GEN_COPY(Dest1, Src1, sizeof(int) * 2 * CAGD_CM_MAX_SAMPLE_NUM);
    IRIT_GEN_COPY(Dest2, Src2, sizeof(int) * 2 * CAGD_CM_MAX_SAMPLE_NUM);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  find the angle between two vectors.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Vector1: The first input vector.                                         *
*   vector2: The second input vector.                                        *
*                                                                            *
* RETURN VALUE:                                                              *
*  CagdRType: The angle between the two inputvectors                         *
*****************************************************************************/
static CagdRType AngleBetweenVectors(CagdVType Vector1, CagdVType Vector2)
{
    CagdRType
	Cos = IRIT_DOT_PROD(Vector1, Vector2);

    return acos(Cos);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  rotate vector in input angle.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   SrcVec: The input vector.                                                *
*   DestVec: The vector after the rotation.                                  *
*   Angle: the rotation angle.						     *
*									     *
* RETURN VALUE:                                                              *
*  void                                                                      *
*****************************************************************************/
static void RotateVector(CagdVType SrcVec,
			 CagdVType DestVec,
			 CagdRType Angle)
{
    DestVec[0] = SrcVec[0] * cos(Angle) - 
                 SrcVec[1] * sin(Angle);
    DestVec[1] = SrcVec[0] * sin(Angle) +
                 SrcVec[1] * cos(Angle);
    DestVec[2] = SrcVec[2];
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the path in the cost matrix (MaxMat).                              *
*                                                                            *
* PARAMETERS:                                                                *
*   i:         Line.                                                         *
*   j:         Column.                                                       *
*   Cnt:       Place in NewIndex1 and NewIndex2.                             *
*   MatchMat:  The path matrix.                                              *
*   MaxMat:    Contains the costs.                                           *
*   NewIndex1: The vector of the new matching to the first curve.            *
*   NewIndex2: The vector of the new matching to the second curve            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FindWayInMat(int i,
			 int j,
			 int *Cnt,
			 LinkStruct MatchMat[CAGD_CM_MAX_SAMPLE_NUM]
                                            [CAGD_CM_MAX_SAMPLE_NUM],
			 CagdRType MaxMat[CAGD_CM_MAX_SAMPLE_NUM + 1]
                                         [CAGD_CM_MAX_SAMPLE_NUM + 1],
			 int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
			 int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM])
{
     int k, l;

     if (i != 1 || j != 1) {
	 k = MatchMat[i - 1][j - 1].x;
	 l = MatchMat[i - 1][j - 1].y;
	 FindWayInMat(k < 1 ? 1 : k, l < 1 ? 1 : l,
		      Cnt, MatchMat, MaxMat, NewIndex1, NewIndex2);
     }

     NewIndex1[*Cnt] = i - 1;
     NewIndex2[(*Cnt)++] = j - 1;
     NewIndex1[*Cnt] =
	 NewIndex2[*Cnt] = -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks if the input curve is closed.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:  The input curve to check.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  TRUE - closed curve, FALSE - not a closed curve.             *
*****************************************************************************/
static CagdBType IfCrvIsClose(const CagdCrvStruct *Crv)
{
    CagdRType *Pt, StartPara, EndPara;
    CagdPType Begin, End;

    CagdCrvDomain(Crv, &StartPara, &EndPara);
    Pt = CagdCrvEval(Crv, StartPara);    
    CagdCoerceToE3(Begin, &Pt, -1, Crv -> PType);

    Pt = CagdCrvEval(Crv, EndPara);
    CagdCoerceToE3(End, &Pt, -1, Crv -> PType);
 
    return IRIT_PT_PT_DIST(Begin, End) < IRIT_EPS;
} 

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the distance norm to the matching.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   T1: A pointer to tangent to the first curve at i-th point.               M
*   T2: A pointer to tangent to the second curve at j-th point.              M
*   P1:	A pointer to value of the first curve at i-th point.                 M
*   P2: A pointer to value of the second curve at j-th point.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: A numeric matching value, the smaller the better.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMatchBisectorNorm, CagdMatchRuledNorm, CagdMatchMorphNorm,           M
*   CagdMatchingTwoCurves  						     M
*									     *
* KEYWORDS:                                                                  M
*   CagdMatchDistNorm, correspondance, matching                              M
*****************************************************************************/
CagdRType CagdMatchDistNorm(const CagdVType T1,
			    const CagdVType T2,
			    const CagdVType P1,
			    const CagdVType P2)
{
    CagdVType V;

    GlblIgnoreUpperBoundLimit = TRUE;

    IRIT_PT_SUB(V, P1, P2);
    return IRIT_PT_LENGTH(V);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the distance norm to the matching.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   T1: A pointer to tangent to the first curve at i-th point.               M
*   T2: A pointer to tangent to the second curve at j-th point.              M
*   P1:	A pointer to value of the first curve at i-th point.                 M
*   P2: A pointer to value of the second curve at j-th point.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: A numeric matching value, the smaller the better.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMatchDistNorm, CagdMatchRuledNorm, CagdMatchMorphNorm,               M
*   CagdMatchingTwoCurves  						     M
*									     *
* KEYWORDS:                                                                  M
*   CagdMatchBisectorNorm, correspondance, matching                          M
*****************************************************************************/
CagdRType CagdMatchBisectorNorm(const CagdVType T1,
				const CagdVType T2,
				const CagdVType P1,
				const CagdVType P2)
{
    CagdVType N1, N2;
    CagdPType ClosestPt1, ClosestPt2;
    CagdRType Param1, Param2;

    N1[0] = T1[1];
    N1[1] = -T1[0];
    N2[0] = -T2[1];
    N2[1] = T2[0];

    N2[2] = N1[2] = 0.0;

    /* If normals are collinear, measure the distance between the one */
    /* normal ray to the other point.				      */
    if (IRIT_FABS(IRIT_DOT_PROD(T1, N2)) < CAGD_CM_BISCT_MIN_IRIT_DOT_PROD)
	return GMDistPointLine(P1, P2, N2) + GMDistPointLine(P2, P1, N1);
    else {
	if (GM2PointsFromLineLine(P1, N1, P2, N2,
				  ClosestPt1, &Param1,
				  ClosestPt2, &Param2)) {
	    return IRIT_FABS(Param1 - Param2);
	}
	else
	    return GMDistPointLine(P1, P2, N2) + GMDistPointLine(P2, P1, N1);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the default morphing norm to the matching.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   T1: A pointer to tangent to the first curve at i-th point.               M
*   T2: A pointer to tangent to the second curve at j-th point.              M
*   P1:	A pointer to value of the first curve at i-th point.                 M
*   P2: A pointer to value of the second curve at j-th point.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: -1 for no matching or the cost of the matching for the point  M
*		between zero and one.			                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMatchDistNorm, CagdMatchBisectorNorm, CagdMatchRuledNorm,            M
*   CagdMatchingTwoCurves  						     M
*									     *
* KEYWORDS:                                                                  M
*   CagdMatchMorphNorm, correspondance, matching                             M
*****************************************************************************/
CagdRType CagdMatchMorphNorm(const CagdVType T1,
			     const CagdVType T2,
			     const CagdVType P1,
			     const CagdVType P2)
{
    CagdRType
	MultRes = IRIT_DOT_PROD(T1, T2);

    GlblIgnoreUpperBoundLimit = FALSE;

    if (!GlblAllowNegativeNorm && MultRes < 0)  
        return -1.0;
    return 1.0 - MultRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the default ruled norm to the matching.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   T1: A pointer to tangent to the first curve at i-th point.               M
*   T2: A pointer to tangent to the second curve at j-th point.              M
*   P1:	A pointer to value of the first curve at i-th point.                 M
*   P2: A pointer to value of the second curve at j-th point.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: -1 for no matching or the cost of the matching for the point  M
*		between zero and one.			                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMatchDistNorm, CagdMatchBisectorNorm, CagdMatchMorphNorm             M
*   CagdMatchingTwoCurves  						     M
*									     *
* KEYWORDS:                                                                  M
*   CagdMatchRuledNorm, correspondance, matching                             M
*****************************************************************************/
CagdRType CagdMatchRuledNorm(const CagdVType T1,
			     const CagdVType T2,
			     const CagdVType P1,
			     const CagdVType P2)
{
    CagdPType RuledVec, CrossVec1, CrossVec2;
    CagdRType MultRes;

    GlblIgnoreUpperBoundLimit = FALSE;

    MultRes = IRIT_DOT_PROD(T1, T2);

    IRIT_PT_SUB(RuledVec, P1, P2);
    IRIT_CROSS_PROD(CrossVec1, T1, RuledVec);
    IRIT_CROSS_PROD(CrossVec2, T2, RuledVec);

    if (!GlblAllowNegativeNorm && IRIT_DOT_PROD(CrossVec1, CrossVec2) < 0)  
        return -1.0;
    return 1.0 - MultRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds matching cost for all samples.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   TanVecCrv1: An array for the first curve tangents.                       *
*   TanVecCrv2: An array for the second curve tangents.                      *
*   Crv1Eval:   An array for the first curve values.                         *
*   Crv2Eval:   An array for the second curve values.                        *
*   NewIndex1:  The vector of the new matching to the first curve.           *
*   NewIndex2:  The vector of the new matching to the second curve.          *
*   MatchNormFunc: A pointer to the matching norm function.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   The matching cost for the all sample point.                 *
*****************************************************************************/
static CagdRType FindNewMatching(CagdVType TanVecCrv1[CAGD_CM_MAX_SAMPLE_NUM],
				 CagdVType TanVecCrv2[CAGD_CM_MAX_SAMPLE_NUM],
				 CagdVType Crv1Eval[CAGD_CM_MAX_SAMPLE_NUM],
				 CagdVType Crv2Eval[CAGD_CM_MAX_SAMPLE_NUM],
				 int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
				 int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM],
				 CagdMatchNormFuncType MatchNormFunc)
{
    /* The only reason these variables are static is to prevent stack */
    /* overlow on small machines like Windows NT or OS2.	      */
    IRIT_STATIC_DATA CagdRType
        DotProdMat[CAGD_CM_MAX_SAMPLE_NUM + 1][CAGD_CM_MAX_SAMPLE_NUM + 1],
	MaxMat[CAGD_CM_MAX_SAMPLE_NUM + 1][CAGD_CM_MAX_SAMPLE_NUM + 1];
    IRIT_STATIC_DATA LinkStruct
        MatchMat[CAGD_CM_MAX_SAMPLE_NUM][CAGD_CM_MAX_SAMPLE_NUM];
    int i, j,
	Cnt = 0;
    CagdRType Sum, Min, Res;

    for (i = 0; i < GlblNumberOfSamples + 1; i++)
        DotProdMat[i][0] =
	    DotProdMat[0][i] = 
		MaxMat[i][0] =
		    MaxMat[0][i] = 2 * GlblNumberOfSamples;

    /* Init. the DotProd matrix. */
    for (i = 0; i < GlblNumberOfSamples; i++) {
	CagdRType
	    *DotProdVec = &DotProdMat[i + 1][1];

	for (j = 0; j < GlblNumberOfSamples; j++) {
	    Res = MatchNormFunc(TanVecCrv1[i], TanVecCrv2[j],
				Crv1Eval[i], Crv2Eval[j]);
	    if (!GlblAllowNegativeNorm && Res == -1)
		*DotProdVec++ = 2 * GlblNumberOfSamples;
	    else
		*DotProdVec++ = Res;
       }
    }
    MaxMat[0][0] = 0;
    for (i = 1; i <= GlblNumberOfSamples; i++) {
	CagdRType
	    *DotProdVec = &DotProdMat[i][1],
	    *MaxVec1 = &MaxMat[i - 1][1],
	    *MaxVec = &MaxMat[i][1];
	LinkStruct
	    *MatchVec = MatchMat[i - 1];

        for (j = 1;
	     j <= GlblNumberOfSamples;
	     j++, MatchVec++, MaxVec1++, MaxVec++) {
	    Min = IRIT_MIN(MaxVec1[-1], IRIT_MIN(*MaxVec1, MaxVec[-1]));
	    *MaxVec = *DotProdVec++ + Min;
	    if (Min == MaxVec1[-1]) {
		MatchVec -> x = i - 1;
		MatchVec -> y = j - 1;
	    }
	    else if (Min == MaxVec[-1]) {
		MatchVec -> x = i;
		MatchVec -> y = j - 1;
	    }
	    else {
		MatchVec -> x = i - 1;
		MatchVec -> y = j;
	    }
	}
    }

    Sum = MaxMat[GlblNumberOfSamples][GlblNumberOfSamples];

    if (GlblIgnoreUpperBoundLimit || Sum < 2 * GlblNumberOfSamples) 
        FindWayInMat(GlblNumberOfSamples, GlblNumberOfSamples ,&Cnt,
		     MatchMat, MaxMat, NewIndex1, NewIndex2);

    return Sum;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds the best cost for the matching (if exist) after checking all       *
* the samples.                                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   TanVecCrv1: An array for the first curve tangents.                       *
*   TanVecCrv2: An array for the second curve tangents.                      *
*   Crv1Eval:   An array for the first curve values.                         *
*   Crv2Eval:   An array for the second curve values.                        *
*   NewIndex1:  The vector of the new matching to the first curve.           *
*   NewIndex2:  The vector of the new matching to the second curve.          *
*   MatchNormFunc: A pointer to the matching norm function.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    -1 for no matching between the curves.                           *
*****************************************************************************/
static int FindBestMatching(CagdVType TanVecCrv1[CAGD_CM_MAX_SAMPLE_NUM],
			    CagdVType TanVecCrv2[CAGD_CM_MAX_SAMPLE_NUM],
			    CagdVType Crv1Eval[CAGD_CM_MAX_SAMPLE_NUM],
			    CagdVType Crv2Eval[CAGD_CM_MAX_SAMPLE_NUM],
			    int NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
			    int NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM],
			    CagdMatchNormFuncType MatchNormFunc)
{
    int i, j, ShiftIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
              ShiftIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM],
              RotateIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
              RotateIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM],
	Shift1 = -1, 
        Shift2 = -1;
    CagdVType ShiftTanVec[2 * CAGD_CM_MAX_SAMPLE_NUM],
              RotateTanVec[2 * CAGD_CM_MAX_SAMPLE_NUM];
    CagdRType TmpSum, Angle,
	ShiftSum = 2 * GlblNumberOfSamples,
        RotateSum = 2 * GlblNumberOfSamples;

    for (i = 0; i < GlblNumberOfSamples; i++) {
	IRIT_PT_COPY(ShiftTanVec[i], TanVecCrv2[i]);
	IRIT_PT_COPY(ShiftTanVec[i + GlblNumberOfSamples], TanVecCrv2[i]);
	IRIT_PT_COPY(RotateTanVec[i], TanVecCrv2[i]);
	IRIT_PT_COPY(RotateTanVec[i + GlblNumberOfSamples], TanVecCrv2[i]);
    }

    for (i = 0; i < GlblShiftFlag; i++) {
	for (j = 0; j < GlblNumberOfSamples; j++)
	    NewIndex1[j] = NewIndex2[j] = j;
	NewIndex1[GlblNumberOfSamples] = NewIndex2[GlblNumberOfSamples] = -1;

	TmpSum = FindNewMatching(TanVecCrv1,
				 &ShiftTanVec[GlblNumberOfSamples - i],
				 Crv1Eval, Crv2Eval, NewIndex1, NewIndex2,
				 MatchNormFunc);

	if (TmpSum < ShiftSum) {
	    ShiftSum = TmpSum;
	    Shift1 = i;
	    CopyTwoArrays(ShiftIndex1, ShiftIndex2, NewIndex1, NewIndex2);
	}
        
        /* Using rotation. */
	if (GlblRotationFlag) {
	    Angle = AngleBetweenVectors(TanVecCrv1[0],
				      RotateTanVec[GlblNumberOfSamples - i]);
	    for (j = 0; j < GlblNumberOfSamples; j++)
		RotateVector(ShiftTanVec[j], RotateTanVec[j], -Angle); 
	    for (j = 0; j < GlblNumberOfSamples; j++)
		NewIndex1[j] = NewIndex2[j] = j;
	    NewIndex1[GlblNumberOfSamples] =
	        NewIndex2[GlblNumberOfSamples] = -1;
	    /* Check the matching after rotation. */
	    TmpSum = FindNewMatching(TanVecCrv1,
				     &RotateTanVec[GlblNumberOfSamples - i],
				     Crv1Eval, Crv2Eval, NewIndex1, NewIndex2,
				     MatchNormFunc);

	    if (TmpSum < RotateSum) {
		RotateSum = TmpSum;
		Shift2 = i;
		CopyTwoArrays(RotateIndex1, RotateIndex2,
			      NewIndex1, NewIndex2);
	    }
	}
    }

#ifdef DEBUG
    {
         IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMatch2Crv, FALSE) {
	     IRIT_INFO_MSG_PRINTF("\nShiftSum = :%lf at %d\n",
				     ShiftSum, Shift1);
	     IRIT_INFO_MSG_PRINTF("\nRotateSum = :%lf at %d\n",
				     RotateSum, Shift2);
	 }
    }
#endif /* DEBUG */

    if (Shift1 == -1 && Shift2 == -1) 
	return -1;

    if (ShiftSum <= RotateSum)
	CopyTwoArrays(NewIndex1, NewIndex2, ShiftIndex1, ShiftIndex2);
    else
	CopyTwoArrays(NewIndex1, NewIndex2, RotateIndex1, RotateIndex2);

    return ShiftSum <= RotateSum ? Shift1 : Shift2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Builds the reparametrization Bspline curve from Vect and Vecr,           *
* with order of Order and with Reduce degrees of freedom.                    *
*                                                                            *
* PARAMETERS:                                                                *
*   Vect:   The new parametrization of the first curve.                      *
*   Vecr:   The new parametrization of the second curve.                     *
*   Order:  Of the reparametrization curves.                                 *
*   Len:    The length of Vect and Vecr.                                     *
*   Reduce: The degree of freedom.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:   The reparametrization curve.                          *
*****************************************************************************/
static CagdCrvStruct *CreateReparamCrv(int *Vect,
				       int *Vecr,
				       int Order,
				       int Len,
				       int Reduce)
{
    int i, j;
    CagdRType Delta, r, *Kv, NewVecr[2 * CAGD_CM_MAX_SAMPLE_NUM],
	NewVect[2 * CAGD_CM_MAX_SAMPLE_NUM];
    CagdCrvStruct *Crv;
    CagdCtlPtStruct *Mesh,
	*PtList = NULL;

    CagdMatchingFixVector(Vect, NewVect, Len);
    CagdMatchingFixVector(Vecr, NewVecr, Len);
    CagdMatchingVectorTransform(NewVect, 0, CAGD_CM_MAX_SAMPLE_NUM - 1, Len);
    CagdMatchingVectorTransform(NewVecr, 0, CAGD_CM_MAX_SAMPLE_NUM - 1, Len);
    
    for (i = Len - 1; i >= 0; i--) {
        Mesh = CagdCtlPtNew(CAGD_PT_E1_TYPE);
	Mesh -> Coords[1] = NewVecr[i];
	IRIT_LIST_PUSH(Mesh, PtList); 
    }
    
    Delta = ((CagdRType) Len) / (Reduce - Order);
    Kv = (CagdRType *) IritMalloc((Reduce + Order) * sizeof(CagdRType));
    
    for (i = j = 0; i < Order; i++, j++)
        Kv[j]  = NewVect[0];
    for (r = Delta * 0.5; j < Reduce; r += Delta)
        Kv[j++] = NewVect[(int) r];
    for (i = 0; i < Order; i++)
        Kv[j++] = NewVect[Len - 1];
   
    /* Interpolate a bspline curve from the points. */
    Crv = BspCrvInterpolate(PtList, NewVect, Kv, Reduce, Order, FALSE);
    CagdCtlPtFreeList(PtList);
    IritFree(Kv);

    /* Fix the curve to be monotone. */
    if (Crv != NULL)
	CagdMatchingFixCrv(Crv);

    /* Return the new parametrization curve. */
    return Crv;
}


/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fix the input integer vector, so that NewVec is increasingly monotone.   M
*                                                                            *
* PARAMETERS:                                                                M
*   OldVec:   The input vector.                                              M
*   NewVec:   The output (fixed) vector                                      M
*   Len:      The length of the vector.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMatchingFixVector, correspondance, matching                          M
*****************************************************************************/
void CagdMatchingFixVector(int *OldVec, CagdRType *NewVec, int Len) 
{
    int j,
	i = 1;

    NewVec[0] = OldVec[0];
    while (i < Len) {
	if (OldVec[i] != OldVec[i - 1]) {
	    NewVec[i] = OldVec[i];
	    i++;
	}
	else {
	    CagdRType Delta;

	    for (j = i; OldVec[j] == OldVec[i - 1] && j < Len; j++);
	    if (j >= Len)
		Delta = 1.0 / (j - i + 1.0);
	    else
		Delta = (OldVec[j] - OldVec[i]) / ((CagdRType) (j - i + 1));
	    j--;
	    for ( ; i <= j; i++)
		NewVec[i] = NewVec[i - 1] + Delta;
	}
    }

    if (!IRIT_APX_EQ(NewVec[Len - 1], OldVec[Len - 1])) {
	CagdRType
	    R = OldVec[Len - 1] / NewVec[Len - 1];

	for (i = 0; i < Len; i++)
	    NewVec[i] *= R;
    }

    for (i = 1; i < Len; i++)
	if (NewVec[i - 1] > NewVec[i])
	    IRIT_WARNING_MSG("CrvMatch: CagdMatchingFixVector: Resulting vector is not monotone.\n");
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fix the input curve to be monotone.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   The input curve to be fixed.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMatchingFixCrv, correspondance, matching                             M
*****************************************************************************/
void CagdMatchingFixCrv(CagdCrvStruct *Crv)
{
    int i, j;
    CagdRType t;
 
    for (j = 0; j < Crv -> Length; j++)
        for (i = 0; i < Crv -> Length - 1; i++)
	    if (Crv -> Points[1][i] > Crv -> Points[1][i + 1]) {
		t = Crv -> Points[1][i];
		Crv -> Points[1][i] = Crv -> Points[1][i + 1];
		Crv -> Points[1][i + 1] = t;
	    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Affine transform a set of points, so its new end locations are NewBegin  M
* and NewEnd, respectively.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Poly:     A pointer to points to change.			             M
*   Len:      The length of the input poly.                                  M
*   NewBegin: The new begin.                                                 M
*   NewEnd:   The new end.                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMatchingPolyTransform, correspondance, matching                      M
*****************************************************************************/
void CagdMatchingPolyTransform(CagdRType **Poly,
			       int Len,
			       CagdRType NewBegin,
			       CagdRType NewEnd)
{
    int i;
    CagdRType Begin, End;

    Begin = Poly[1][0];
    End = Poly[1][Len - 1];
    for (i = 0; i < Len; i++)
	Poly[1][i] = (Poly[1][i] - Begin) * (NewEnd - NewBegin) / 
						     (End - Begin) + NewBegin;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Affine transform a set of vectors, so its new end locations are NewBegin M
* and NewEnd, respectively.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Vec:      The input and the output vector.                               M
*   NewBegin: The new begin.                                                 M
*   NewEnd:   The new end.                                                   M
*   Len:      The length of the input vector.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMatchingVectorTransform, correspondance, matching                    M
*****************************************************************************/
void CagdMatchingVectorTransform(CagdRType *Vec,
				 CagdRType NewBegin,
				 CagdRType NewEnd,
				 int Len)
{
    int i;
    CagdRType Begin, End;

    Begin = Vec[0];
    End = Vec[Len - 1];

    for (i = 0; i < Len; i++)
	Vec[i] = (Vec[i] - Begin) * (NewEnd - NewBegin) / (End - Begin)
								+ NewBegin;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets two freeform curves, Crv1, nd Crv2, computes a new parametrization  M
* to Crv2 using composition between Crv2 and a computed reparametrization    M
* that establishes a matching correspondance between Crv1 and Crv2.	     M
*	                                                                     *
* PARAMETERS:                                                                M
*   Crv1:   The first curve.                                                 M
*   Crv2:   The second curve.                                                M
*   Reduce: The degrees of freedom of the reparametrization curve. The       M
*	    larger this number is, the better the reparametrization will be  M
*	    at the cost of more computation. Must be less than SampleSet     M
*   SampleSet:     Number of samples the two curves are sampled at. The      M
*	    larger this number is, the better the reparametrization will be  M
*   ReparamOrder:  Order of reparametrization curve.		             M
*   RotateFlag: use or not use rotation in finding best matching	     M
*   AllowNegativeNorm:  If TRUE, negative norms are locally allowed.	     M
*   ReturnReparamFunc:  If TRUE, return the reparamterization function       M
*		    instead of Crv2 reparametrized.			     M
*   MatchNormFunc: A pointer to the matching norm.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The second curve, Crv2, after reparametrization that    M
*                    matches the first curve.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMatchDistNorm, CagdMatchBisectorNorm, CagdMatchMorphNorm             M
*   CagdMatchRuledNorm  						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdMatchingTwoCurves, correspondance, matching                          M
*****************************************************************************/
CagdCrvStruct *CagdMatchingTwoCurves(CagdCrvStruct *Crv1,
				     CagdCrvStruct *Crv2,
				     int Reduce,
				     int SampleSet,
				     int ReparamOrder,
				     int RotateFlag,
				     int AllowNegativeNorm,
				     int ReturnReparamFunc,
				     CagdMatchNormFuncType MatchNormFunc)
{
    int i, Len, Shift, NewIndex1[2 * CAGD_CM_MAX_SAMPLE_NUM],
	NewIndex2[2 * CAGD_CM_MAX_SAMPLE_NUM];
    CagdVType Crv1Eval[CAGD_CM_MAX_SAMPLE_NUM],
              Crv2Eval[CAGD_CM_MAX_SAMPLE_NUM],
	      TanVecCrv1[CAGD_CM_MAX_SAMPLE_NUM],
	      TanVecCrv2[CAGD_CM_MAX_SAMPLE_NUM];  
    CagdRType StartCrv2Par, StartCrv1Par, EndCrv2Par, EndCrv1Par, ShiftPar,
	TangentsDotProdRes[CAGD_CM_MAX_SAMPLE_NUM];
    CagdCrvStruct *TCrv, *NewParaCrv;

    Crv2 = CagdCrvCopy(Crv2);

    GlblNumberOfSamples =
        (SampleSet > CAGD_CM_MAX_SAMPLE_NUM) ? CAGD_CM_MAX_SAMPLE_NUM
					     : SampleSet;
    GlblRotationFlag = RotateFlag;
    GlblShiftFlag = IfCrvIsClose(Crv2) ? GlblNumberOfSamples : 1;
    GlblAllowNegativeNorm = AllowNegativeNorm;

    if (MatchNormFunc == NULL)
	MatchNormFunc = CagdMatchMorphNorm;

    /* Find the tangents at each sample point. */
    if (FindTangentsVec(Crv1, TanVecCrv1, Crv1Eval) == 0 ||
        FindTangentsVec(Crv2, TanVecCrv2, Crv2Eval) == 0) {
	CAGD_FATAL_ERROR(CAGD_ERR_CANNOT_COMP_VEC_FIELD);
        return NULL;
    }

    /* Init. the new matching vectors */
    for (i = 0; i < GlblNumberOfSamples; i++)
        NewIndex1[i] =
	    NewIndex2[i] = i;
    NewIndex1[GlblNumberOfSamples] =
	NewIndex2[GlblNumberOfSamples] = -1;

    MatchingTest(TanVecCrv1, TanVecCrv2,
		 TangentsDotProdRes, NewIndex1, NewIndex2);

    if ((Shift = FindBestMatching(TanVecCrv1, TanVecCrv2, Crv1Eval, Crv2Eval,
				  NewIndex1, NewIndex2,
				  MatchNormFunc)) == -1) {
	TCrv = CagdCrvReverse(Crv2);
	CagdCrvFree(Crv2);
	Crv2 = TCrv;
	if (FindTangentsVec(Crv1, TanVecCrv1, Crv1Eval) == 0 ||
	    FindTangentsVec(Crv2, TanVecCrv2, Crv2Eval) == 0) {
	    CAGD_FATAL_ERROR(CAGD_ERR_CANNOT_COMP_VEC_FIELD);
	    return NULL;
	}

	for (i = 0; i < GlblNumberOfSamples; i++)
	    NewIndex1[i] =
		NewIndex2[i] = i;
	NewIndex1[GlblNumberOfSamples] =
	    NewIndex2[GlblNumberOfSamples] = -1;
	    
	if ((Shift = FindBestMatching(TanVecCrv1, TanVecCrv2, Crv1Eval,
				      Crv2Eval, NewIndex1, NewIndex2, 
				      MatchNormFunc)) == -1) {
	    return NULL;
	}
    }

    if (Shift != 0) {
	CagdCrvStruct *H1Crv2, *H2Crv2;

	CagdCrvDomain(Crv2, &StartCrv2Par, &EndCrv2Par);
	ShiftPar = StartCrv2Par + ((CagdRType) (GlblNumberOfSamples - Shift))
					* (EndCrv2Par -  StartCrv2Par)
					/ ((CagdRType) GlblNumberOfSamples);
	H2Crv2 = CagdCrvRegionFromCrv(Crv2, StartCrv2Par , ShiftPar);
	H1Crv2 = CagdCrvRegionFromCrv(Crv2, ShiftPar, EndCrv2Par);
	CagdCrvFree(Crv2);
	Crv2 = CagdMergeCrvCrv(H1Crv2, H2Crv2, TRUE);
	CagdCrvFree(H1Crv2);
	CagdCrvFree(H2Crv2);

	BspKnotAffineTrans2(Crv2 -> KnotVector,
			    Crv2 -> Length + Crv2 -> Order,  
			    StartCrv2Par, EndCrv2Par);
    }
    for (Len = 0;
	 Len < 2 * GlblNumberOfSamples && NewIndex1[Len+1] != -1;
	 Len++);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugMatch2Crv, FALSE) {
	    for (i = 0;
		 i < 2 * GlblNumberOfSamples && NewIndex1[i] != -1;
		 i++)
		IRIT_INFO_MSG_PRINTF("\n%d %d",
				     NewIndex1[i], NewIndex2[i]);
	}
    }
#endif /* DEBUG */
   
    if ((NewParaCrv = CreateReparamCrv(NewIndex1, NewIndex2, ReparamOrder,
				       Len, Reduce)) == NULL)
	return NULL;

    /* Find the domain of the two curves. */
    CagdCrvDomain(Crv1, &StartCrv1Par, &EndCrv1Par);
    CagdCrvDomain(Crv2, &StartCrv2Par, &EndCrv2Par);

    /* Scale the matching curve. */
    CagdMatchingVectorTransform(NewParaCrv -> KnotVector,
				StartCrv1Par, EndCrv1Par,
				NewParaCrv -> Length + NewParaCrv -> Order);

    CagdMatchingPolyTransform(NewParaCrv -> Points, NewParaCrv -> Length,
		  StartCrv2Par, EndCrv2Par);

    if (ReturnReparamFunc) {
	return NewParaCrv;
    }
    else {
	TCrv = SymbComposeCrvCrv(Crv2, NewParaCrv);
	CagdCrvFree(NewParaCrv);
	CagdCrvFree(Crv2);

	return TCrv;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Check if macthed points on the given two curves are closer as is or when M
* one of the curves is reversed.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:   The two curvs to consider.                                 M
*   n:		  Number of samples to take on the curves.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if needs to reverse, FALSE if as is is better.             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvTwoCrvsOrient                                                     M
*****************************************************************************/
int CagdCrvTwoCrvsOrient(CagdCrvStruct *Crv1, CagdCrvStruct *Crv2, int n)
{
    CagdRType TMin, TMax, TMin2, TMax2, t, dt,
	Dist1 = 0.0,
	Dist2 = 0.0;

    CagdCrvDomain(Crv1, &TMin, &TMax);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);

    if (!IRIT_APX_EQ(TMin2, TMin) ||!IRIT_APX_EQ(TMax2, TMax))
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_DOMAIN);

    dt = (TMax - TMin) / n;
    for (t = TMin; t < TMax; t += dt) {
        CagdRType *R;
	CagdPType Pt1, Pt2, Pt2r;

	R = CagdCrvEval(Crv1, t);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, t);
	CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);
	R = CagdCrvEval(Crv2, TMax - t);
	CagdCoerceToE3(Pt2r, &R, -1, Crv2 -> PType);

	Dist1 += IRIT_PT_PT_DIST_SQR(Pt1, Pt2);
	Dist2 += IRIT_PT_PT_DIST_SQR(Pt1, Pt2r);
    }

    return Dist1 > Dist2;
}
