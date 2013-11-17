/******************************************************************************
* Register.c - routines to register object against another object.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 2000.					      *
******************************************************************************/

#include "irit_sm.h"
#include "user_lib.h"

#define USER_REGIS_NUM_DERSRF_EPS	1e-4
#define USER_REGIS_NUM_DERIV_EPS	1e-8
#define USER_REGIS_MAX_NUM_OF_ITERS	1000
#define USER_REGIS_SRF_SAMP_SIZE	100

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugRegPrintErr, FALSE);
#endif /* DEBUG */

static void GetMatrixFromTransRots(IrtHmgnMatType Mat,
				   IrtPtType Rots,
				   IrtPtType Trans);
static void ApplyMatrixToPointSet(IrtHmgnMatType Mat,
				  int n,
				  IrtPtType *OrigPtsSet,
				  IrtPtType *MappedPtsSet);
static IrtRType DistanceTwoPtSets(int n1,
				  IrtPtType *PtsSet1,
				  int n2,
				  IrtPtType *PtsSet2);
static IrtRType *EvalTransGradient(IrtRType CrntDist,
				   int n1,
				   IrtPtType *PtsSet1,
				   int n2,
				   IrtPtType *PtsSet2);
static IrtRType *EvalRotGradient(IrtRType CrntDist,
				 int n1,
				 IrtPtType *PtsSet1,
				 int n2,
				 IrtPtType *PtsSet2);
static int SrfUpdateClosestPoint(const IrtPtType Pt,
				 const CagdSrfStruct *Srf,
				 CagdRType *u,
				 CagdRType *v,
				 IrtRType Tolerance,
				 IrtRType *FinalDist);
static IrtRType *UserSrfSafeEval(const CagdSrfStruct *Srf,
				 CagdRType u,
				 CagdRType v);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Constructs a transformation matrix to follow the prescribed rotation and *
* then the specified translation.                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:          To update with the proper rotation and translation.        *
*   Rots:         Amount of rotation to apply - x, then y, and then z.       *
*   Trans:        Amount of translation to apply.                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void GetMatrixFromTransRots(IrtHmgnMatType Mat,
				   IrtPtType Rots,
				   IrtPtType Trans)
{
    IrtHmgnMatType TMat;

    MatGenUnitMat(Mat);

    MatGenMatTrans(Trans[0], Trans[1], Trans[2], TMat);
    MatMultTwo4by4(Mat, Mat, TMat);

    MatGenMatRotX1(Rots[0], TMat);
    MatMultTwo4by4(Mat, Mat, TMat);
    MatGenMatRotY1(Rots[1], TMat);
    MatMultTwo4by4(Mat, Mat, TMat);
    MatGenMatRotZ1(Rots[2], TMat);
    MatMultTwo4by4(Mat, Mat, TMat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Maps the given point set using the prescribed transformation.            *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:            The transformation matrix.                               *
*   n:              Number of points in each set.                            *
*   OrigPtsSet:     Original point set - not modified.                       *
*   MappedPtsSet:   mapped point set will be placed here.                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ApplyMatrixToPointSet(IrtHmgnMatType Mat,
				  int n,
				  IrtPtType *OrigPtsSet,
				  IrtPtType *MappedPtsSet)
{
    int i;

    for (i = 0; i < n; i++)
        MatMultPtby4by4(MappedPtsSet[i], OrigPtsSet[i], Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the distance between the two sets.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   n:           Number of points in each set.                               *
*   PtsSet1:     The first set of points                                     *
*   PtsSet2:     The second set of points                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static IrtRType DistanceTwoPtSets(int n1,
				  IrtPtType *PtsSet1,
				  int n2,
				  IrtPtType *PtsSet2)
{
    int i, j;
    IrtRType
	SumDist = 0.0;

    for (i = 0; i < n1; i++) {
        IrtRType
	    MaxIDistSqr = IRIT_INFNTY;

        for (j = 0; j < n2; j++) {
	    IrtRType
		d = IRIT_PT_PT_DIST_SQR(PtsSet1[i], PtsSet2[j]);

	    if (MaxIDistSqr > d)
	        MaxIDistSqr = d;
	}

	SumDist += sqrt(MaxIDistSqr);
    }

    return SumDist / n1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the translational gradient of the given two sets.                *
*                                                                            *
* PARAMETERS:                                                                *
*   CrntDist:    The current distance between the two sets.                  *
*   n1:          Number of points in PtsSet1.                                *
*   PtsSet1:     The first set of points                                     *
*   n2:          Number of points in PtsSet2.                                *
*   PtsSet2:     The second set of points                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *:   Gradient as a vector of size three, allocated statically.  *
*****************************************************************************/
static IrtRType *EvalTransGradient(IrtRType CrntDist,
				   int n1,
				   IrtPtType *PtsSet1,
				   int n2,
				   IrtPtType *PtsSet2)
{
    IRIT_STATIC_DATA IrtPtType
	Gradient;
    int i, l;

    for (l = 0; l < 3; l++) {
        for (i = 0; i < n1; i++)
	    PtsSet1[i][l] += USER_REGIS_NUM_DERIV_EPS;

	Gradient[l] = (DistanceTwoPtSets(n1, PtsSet1, n2, PtsSet2)
					- CrntDist) / USER_REGIS_NUM_DERIV_EPS;

        for (i = 0; i < n1; i++)
	    PtsSet1[i][l] -= USER_REGIS_NUM_DERIV_EPS;
    }

    return Gradient;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compute the translational gradient of the given two sets.                *
*                                                                            *
* PARAMETERS:                                                                *
*   CrntDist:    The current distance between the two sets.                  *
*   n1:          Number of points in PtsSet1.                                *
*   PtsSet1:     The first set of points                                     *
*   n2:          Number of points in PtsSet2.                                *
*   PtsSet2:     The second set of points                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *:   Gradient as a vector of size three, allocated statically.  *
*****************************************************************************/
static IrtRType *EvalRotGradient(IrtRType CrntDist,
				 int n1,
				 IrtPtType *PtsSet1,
				 int n2,
				 IrtPtType *PtsSet2)
{
    IRIT_STATIC_DATA IrtPtType
	Gradient;
    int i, l;
    IrtHmgnMatType Mat, InvMat;

    for (l = 0; l < 3; l++) {
        switch (l) {
	    case 0:
	        MatGenMatRotX1(USER_REGIS_NUM_DERIV_EPS, Mat);
	        MatGenMatRotX1(-USER_REGIS_NUM_DERIV_EPS, InvMat);
	        break;
	    case 1:
	        MatGenMatRotY1(USER_REGIS_NUM_DERIV_EPS, Mat);
	        MatGenMatRotY1(-USER_REGIS_NUM_DERIV_EPS, InvMat);
	        break;
	    case 2:
	        MatGenMatRotZ1(USER_REGIS_NUM_DERIV_EPS, Mat);
	        MatGenMatRotZ1(-USER_REGIS_NUM_DERIV_EPS, InvMat);
	        break;
	}

        for (i = 0; i < n1; i++)
	    MatMultPtby4by4(PtsSet1[i], PtsSet1[i], Mat);

	Gradient[l] = (DistanceTwoPtSets(n1, PtsSet1, n2, PtsSet2)
					- CrntDist) / USER_REGIS_NUM_DERIV_EPS;

        for (i = 0; i < n1; i++)
	    MatMultPtby4by4(PtsSet1[i], PtsSet1[i], InvMat);
    }

    return Gradient;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A default test convergance function for UserRegisterTwoPointSets.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Dist:   The current distance between the two points sets.                M
*   i:      The index of the current iteration, starting with zero.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if should quit (accuracy is good enough), FALSE otherwise.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserRegisterTwoPointSets                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserRegisterTestConvergance                                              M
*****************************************************************************/
int UserRegisterTestConvergance(IrtRType Dist, int i)
{
    return i >= USER_REGIS_MAX_NUM_OF_ITERS;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two sets of points, one is a rigid transformation of the other,    M
* find a transformation that takes the first set of points to the second.    M
*   A local greedy approach is taken here that guarantee local minimum and   M
* hence it is assumed the two sets are pretty close and similar.	     M
*   No assumption is made as to the order of the points in the set.  	     M
* However, and while the two sets need not have the same number of points,   M
* the optimal solution is achieved at zero distance when each point in one   M
* set is at a corresponding point location of the second set.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   n1:            Number of points in PtsSet1.                              M
*   PtsSet1:       The vector of points of first set.  Might be changed in   M
*		   place.				                     M
*   n2:            Number of points in PtsSet2.                              M
*   PtsSet2:       The vector of points in second set. Might be changed in   M
*		   place.				                     M
*   AlphaConverge: Convergance factor between almost zero (slow and stable)  M
*		   and one (fast but unstable).				     M
*   Tolerance:     Tolerance termination condition, in L infinity sense.     M
*   RegisterTestConvergance:  A call back function to check for the          M
*		   convergance of this iterative process.		     M
*   RegMat:        Computed transformation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Error of match between the two corresponding point sets.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserRegisterTestConvergance                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserRegisterTwoPointSets                                                 M
*****************************************************************************/
IrtRType UserRegisterTwoPointSets(int n1,
				  IrtPtType *PtsSet1,
				  int n2,
				  IrtPtType *PtsSet2,
				  IrtRType AlphaConverge,
				  IrtRType Tolerance,
				  UserRegisterTestConverganceFuncType
				      RegisterTestConvergance,
				  IrtHmgnMatType RegMat)
{
    int i;
    IrtRType LastDist, CrntDist, TmpDist;
    IrtPtType Trans2, Trans, Rot, TmpTrans, TmpRot,
	*PtsSet1t = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * n1);
    IrtHmgnMatType TMat;

    IRIT_PT_RESET(Trans);
    IRIT_PT_RESET(Trans2);
    IRIT_PT_RESET(Rot);

    if (RegisterTestConvergance == NULL)
        RegisterTestConvergance = UserRegisterTestConvergance;

    /* Compute (negative) center of gravity to both sets. */
    for (i = 0; i < n1; i++)
        IRIT_PT_SUB(Trans, Trans, PtsSet1[i]);
    IRIT_PT_SCALE(Trans, 1.0 / n1);

    for (i = 0; i < n2; i++)
        IRIT_PT_SUB(Trans2, Trans2, PtsSet2[i]);
    IRIT_PT_SCALE(Trans2, 1.0 / n2);

    /* Bring the center of point set 2 to the origin. */
    GetMatrixFromTransRots(RegMat, Rot, Trans2);
    ApplyMatrixToPointSet(RegMat, n2, PtsSet2, PtsSet2);

    /* Bring point set 1 to the best position we have until now. */
    GetMatrixFromTransRots(RegMat, Rot, Trans);
    ApplyMatrixToPointSet(RegMat, n1, PtsSet1, PtsSet1t);
    CrntDist = LastDist = DistanceTwoPtSets(n1, PtsSet1t, n2, PtsSet2);

    for (i = 0; i < USER_REGIS_MAX_NUM_OF_ITERS; i++) {
        IrtRType R, *TGradient, *RGradient;

#	ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugRegPrintErr)
	        IRIT_INFO_MSG_PRINTF("\terror = %15.13lf\n", CrntDist);
#	endif /* DEBUG */

	/* Call back to check if we are to quit. */
	if (CrntDist < Tolerance || RegisterTestConvergance(CrntDist, i))
	    break;

	/* Compute the translational gradient and apply it. */
	TGradient = EvalTransGradient(CrntDist, n1, PtsSet1t, n2, PtsSet2);

	/* Compute the rotational gradient and apply it. */
	RGradient = EvalRotGradient(CrntDist, n1, PtsSet1t, n2, PtsSet2);

	R = sqrt(IRIT_VEC_SQR_LENGTH(TGradient) + IRIT_VEC_SQR_LENGTH(RGradient));
	R = -AlphaConverge * CrntDist / (IRIT_SQR(R) + IRIT_UEPS);
	IRIT_VEC_SCALE(TGradient, R);
	IRIT_VEC_SCALE(RGradient, R);

	IRIT_PT_ADD(TmpTrans, Trans, TGradient);
	IRIT_PT_ADD(TmpRot, Rot, RGradient);
	GetMatrixFromTransRots(TMat, TmpRot, TmpTrans);

	/* Bring point set 1 to the best position we have until now. */
	ApplyMatrixToPointSet(TMat, n1, PtsSet1, PtsSet1t);
	TmpDist = DistanceTwoPtSets(n1, PtsSet1t, n2, PtsSet2);

	/* If we are successful improving - update new orientation. */
	if (TmpDist < LastDist) {
	    IRIT_HMGN_MAT_COPY(RegMat, TMat);
	    LastDist = CrntDist;
	    CrntDist = TmpDist;
	    IRIT_PT_COPY(Trans, TmpTrans);
	    IRIT_PT_COPY(Rot, TmpRot);
	    if (AlphaConverge < 0.75)
	        AlphaConverge *= 2.0;
	}
	else {
	    ApplyMatrixToPointSet(RegMat, n1, PtsSet1, PtsSet1t);

	    if (AlphaConverge < 0.001)
	        break;
	    else 
	        AlphaConverge *= 0.25;
	}

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugRegisterPts, FALSE) {
	        IRIT_INFO_MSG_PRINTF(
		    "Iter %d, Dist = %f Trans = %f %f %f Rot = %f %f %f \n",
		    i, CrntDist, Trans[0], Trans[1], Trans[2],
		    Rot[0], Rot[1], Rot[2]);
		IRIT_INFO_MSG_PRINTF(
			"    Alpha = %f, Gradient = %f %f %f :: %f %f %f\n",
			AlphaConverge,
			TGradient[0], TGradient[1], TGradient[2],
			RGradient[0], RGradient[1], RGradient[2]);
	    }
	}
#	endif /* DEBUG */
    }

    IritFree(PtsSet1t);

    /* Bring back to point set 2 originial center location. */
    MatGenMatTrans(-Trans2[0], -Trans2[1], -Trans2[2], TMat);
    MatMultTwo4by4(RegMat, RegMat, TMat);

    return CrntDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Numerically improve the given approximate location on Srf(u, v) to the   *
* closest location to point Pt.                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:          Point to find locally closest point on Srf.                 *
*   Srf:         Surface to numerically march on.                            *
*   u, v:        Starting location to numerically march from.                *
*   Tolerance:   Tolerance termination condition, in L infinity sense.       *
*   FinalDist:   Final distance computed.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:     TRUE if improved, FALSE if failed.                              *
*****************************************************************************/
static int SrfUpdateClosestPoint(const IrtPtType Pt,
				 const CagdSrfStruct *Srf,
				 CagdRType *u,
				 CagdRType *v,
				 IrtRType Tolerance,
				 IrtRType *FinalDist)
{
    CagdRType *R, t, Det, Wu, Wv, StartDist, Dist, LastDist,
	UMin, UMax, VMin, VMax, u2, v2;
    IrtPtType SrfPt;
    IrtVecType SrfNrml, SrfDu, SrfDv, TmpVec;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Eval position. */
    R = UserSrfSafeEval(Srf, *u, *v);
    CagdCoerceToE3(SrfPt, &R, -1, Srf -> PType);
    Dist = LastDist = StartDist = IRIT_PT_PT_DIST(Pt, SrfPt);

    while (Dist > Tolerance) {
	/* Eval Du. */
	u2 = *u + USER_REGIS_NUM_DERSRF_EPS;
	if (u2 >= UMax) {
	    u2 = *u - USER_REGIS_NUM_DERSRF_EPS;
	    R = UserSrfSafeEval(Srf, u2, *v);
	    CagdCoerceToE3(SrfDu, &R, -1, Srf -> PType);

	    IRIT_PT_SUB(SrfDu, SrfPt, SrfDu);
	}
	else {
	    R = UserSrfSafeEval(Srf, u2, *v);
	    CagdCoerceToE3(SrfDu, &R, -1, Srf -> PType);

	    IRIT_PT_SUB(SrfDu, SrfDu, SrfPt);
	}
	IRIT_PT_SCALE(SrfDu, 1.0 / USER_REGIS_NUM_DERSRF_EPS);

	/* Eval Dv. */
	v2 = *v + USER_REGIS_NUM_DERSRF_EPS;
	if (v2 >= VMax) {
	    v2 = *v - USER_REGIS_NUM_DERSRF_EPS;
	    R = UserSrfSafeEval(Srf, *u, v2);
	    CagdCoerceToE3(SrfDv, &R, -1, Srf -> PType);
	    IRIT_PT_SUB(SrfDv, SrfPt, SrfDv);
	}
	else {
	    R = UserSrfSafeEval(Srf, *u, v2);
	    CagdCoerceToE3(SrfDv, &R, -1, Srf -> PType);
	    IRIT_PT_SUB(SrfDv, SrfDv, SrfPt);
	}
	IRIT_PT_SCALE(SrfDv, 1.0 / USER_REGIS_NUM_DERSRF_EPS);

	/* Eval normal. */
	IRIT_CROSS_PROD(SrfNrml, SrfDu, SrfDv);
	IRIT_VEC_NORMALIZE(SrfNrml);

	/* Project the vector (Pt - SrfPt) onto the tangent plane. */
	IRIT_PT_SUB(TmpVec, Pt, SrfPt);
	t = IRIT_DOT_PROD(TmpVec, SrfNrml);
	IRIT_VEC_SCALE(SrfNrml, t);
	IRIT_PT_SUB(TmpVec, TmpVec, SrfNrml); /* Now TmpVec is in tan pln. */

	/* Find the weights (Wu, Wv) in "Wu SrfDu + Wv SrfDv = TmpVec". */
	if (IRIT_FABS(SrfNrml[0]) > IRIT_FABS(SrfNrml[1]) &&
	    IRIT_FABS(SrfNrml[0]) > IRIT_FABS(SrfNrml[2])) {
	    /* Solve in the YZ plane. */
	    Det = SrfDu[1] * SrfDv[2] - SrfDu[2] * SrfDv[1];
	    Wu = (TmpVec[1] * SrfDv[2] - TmpVec[2] * SrfDv[1]) / Det;
	    Wv = (TmpVec[2] * SrfDu[1] - TmpVec[1] * SrfDu[2]) / Det;
	}
	else if (IRIT_FABS(SrfNrml[1]) > IRIT_FABS(SrfNrml[0]) &&
		 IRIT_FABS(SrfNrml[1]) > IRIT_FABS(SrfNrml[2])) {
	    /* Solve in the XZ plane. */
	    Det = SrfDu[0] * SrfDv[2] - SrfDu[2] * SrfDv[0];
	    Wu = (TmpVec[0] * SrfDv[2] - TmpVec[2] * SrfDv[0]) / Det;
	    Wv = (TmpVec[2] * SrfDu[0] - TmpVec[0] * SrfDu[2]) / Det;
	}
	else {
	    /* Solve in the XY plane. */
	    Det = SrfDu[0] * SrfDv[1] - SrfDu[1] * SrfDv[0];
	    Wu = (TmpVec[0] * SrfDv[1] - TmpVec[1] * SrfDv[0]) / Det;
	    Wv = (TmpVec[1] * SrfDu[0] - TmpVec[0] * SrfDu[1]) / Det;
	}

	/* Compute new (u, v) positions. */
	Wu += *u;
	Wu = IRIT_BOUND(Wu, UMin, UMax);
	Wv += *v;
	Wv = IRIT_BOUND(Wv, VMin, VMax);

	/* Eval surface at new position. */
	R = UserSrfSafeEval(Srf, Wu, Wv);
	CagdCoerceToE3(SrfPt, &R, -1, Srf -> PType);

	/* Did we improve anything? */
	if ((Dist = IRIT_PT_PT_DIST(Pt, SrfPt)) >= LastDist) {
#	    ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugRegPrintErr)
		    IRIT_INFO_MSG_PRINTF(
			    "Point distance = %15.13lf (Start = %15.13lf)\n",
			    LastDist, StartDist);
#	    endif /* DEBUG */

	    *FinalDist = LastDist;

	    return StartDist > LastDist;
	}
	else {
	    *u = Wu;
	    *v = Wv;
	    LastDist = Dist;
	}
    }

#   ifdef DEBUG
        IRIT_IF_DEBUG_ON_PARAMETER(_DebugRegPrintErr)
	    IRIT_INFO_MSG_PRINTF(
		"Point distance = %15.13lf (Start = %15.13lf)\n",
		LastDist, StartDist);
#   endif /* DEBUG */

    *FinalDist = LastDist;

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A test convergance function for UserRegisterTwoPointSets for surface -   M
* point-set registration.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Dist:   The current distance between the two points sets.                M
*   i:      The index of the current iteration, starting with zero.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE if should quit (accuracy is good enough), FALSE otherwise.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserRegisterTwoPointSets, UserRegisterTestConvergance                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserRegisterTestSrfConvergance                                           M
*****************************************************************************/
int UserRegisterTestSrfConvergance(IrtRType Dist, int i)
{
    return i >= 20;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a set of points, that is a rigid transformation of a given surface M
* find a transformation that takes the set of points to the surface.         M
*   A local greedy approach is taken here that guarantee local minimum and   M
* hence it is assumed the two sets are pretty close and similar.	     M
*   Corrections by marching on the surface are conducted on the fly. 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   n:             Number of points in PtsSet1.                              M
*   PtsSet:        The vector of points of first set.  Might be changed in   M
*		   place.				                     M
*   Srf:           A surface the points set is on upto rigid motion.         M
*   AlphaConverge: Convergance factor between almost zero (slow and stable)  M
*		   and one (fast but unstable).				     M
*   Tolerance:     Tolerance termination condition, in L infinity sense.     M
*   RegisterTestConvergance:  A call back function to check for the          M
*		   convergance of this iterative process.		     M
*   RegMat:        Computed transformation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:    Maximal error as the maximal distance between two           M
*		 corresponding points, or IRIT_INFNTY if failed to converge. M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserRegisterTestConvergance                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserRegisterPointSetSrf                                                  M
*****************************************************************************/
IrtRType UserRegisterPointSetSrf(int n,
				 IrtPtType *PtsSet,
				 const CagdSrfStruct *Srf,
				 IrtRType AlphaConverge,
				 IrtRType Tolerance,
				 UserRegisterTestConverganceFuncType
				                      RegisterTestConvergance,
				 IrtHmgnMatType RegMat)
{
    int i, j, l;
    CagdRType *R, UMin, UMax, VMin, VMax, u, v, du, dv, Err, Dist,
        *InitDists = (CagdRType *) IritMalloc(sizeof(CagdRType) * n);
    CagdUVType
	*SrfUVVals = (CagdUVType *) IritMalloc(sizeof(CagdUVType) * n);
    IrtPtType *Pts, *PtsTmp1, *PtsTmp2,
	**SrfGridPts = (IrtPtType **) IritMalloc(sizeof(IrtPtType *) *
						 USER_REGIS_SRF_SAMP_SIZE);
    IrtHmgnMatType Mat;

    /* Sample the surface on a grid to find approximate closest point. */
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    du = (UMax - UMin) / (USER_REGIS_SRF_SAMP_SIZE - 1);
    dv = (VMax - VMin) / (USER_REGIS_SRF_SAMP_SIZE - 1);

    for (j = 0, v = VMin; j < USER_REGIS_SRF_SAMP_SIZE; j++, v += dv) {
	Pts = SrfGridPts[j] = (IrtPtType *) IritMalloc(sizeof(IrtPtType) *
						    USER_REGIS_SRF_SAMP_SIZE);

	for (i = 0, u = UMin; i < USER_REGIS_SRF_SAMP_SIZE; i++, u += du) {
	    R = UserSrfSafeEval(Srf, u, v);
	    CagdCoerceToE3(Pts[i], &R, -1, Srf -> PType);
	}
    }

    /* Compute closest points on surface to points in given set. */
    for (l = 0; l < n; l++) {
	int MinIndex[2];
	IrtRType
	    MinDist = IRIT_INFNTY;

	MinIndex[0] = MinIndex[1] = -1;
	for (j = 0; j < USER_REGIS_SRF_SAMP_SIZE; j++) {
	    for (i = 0; i < USER_REGIS_SRF_SAMP_SIZE; i++) {
		IrtRType
		    Dist = IRIT_PT_PT_DIST(PtsSet[l], SrfGridPts[j][i]);

		if (MinDist > Dist) {
		    MinDist = Dist;
		    MinIndex[0] = i;
		    MinIndex[1] = j;
		}
	    }
	}

	SrfUVVals[l][0] = MinIndex[0] * du + UMin;
	SrfUVVals[l][1] = MinIndex[1] * dv + VMin;
	if (!SrfUpdateClosestPoint(PtsSet[l], Srf,
				   &SrfUVVals[l][0], &SrfUVVals[l][1],
				   Tolerance / 10.0, &Dist)) {
	    SrfUVVals[l][0] = IRIT_INFNTY;   /* Ignore - convergance failed. */
#	    ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugRegPrintErr)
		    IRIT_INFO_MSG_PRINTF(
			    "Srf point %d (out of %d) noconvrgance ignored.\n",
			    l, n);
#	    endif /* DEBUG */
	}
	InitDists[l] = Dist;
    }

    /* Find out the maximum error points and purge all points more than      */
    /* a quarter that much as outlairs.					     */
    Dist = 0.0;
    for (l = 0; l < n; l++) {
        if (Dist < InitDists[l])
	    Dist = InitDists[l];
    }
    Dist *= 0.25;
    for (l = 0; l < n; l++) {
	if (Dist < InitDists[l]) {
	    SrfUVVals[l][0] = IRIT_INFNTY;     /* Mark to ignore as outlair. */
#	    ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugRegPrintErr)
		    IRIT_INFO_MSG_PRINTF(
			    "Srf point %d (out of %d) outlair is ignored.\n",
			    l, n);
#	    endif /* DEBUG */
	}	    
    }
    IritFree(InitDists);

    /* Purge away ignored points. */
    for (i = j = l = 0; j < n; i++, j++) {
	while (j < n - 1 && SrfUVVals[j][0] == IRIT_INFNTY)
	    j++;

	SrfUVVals[i][0] = SrfUVVals[j][0];
	SrfUVVals[i][1] = SrfUVVals[j][1];
    }
    n -= j - i;
    
    /* Free what is no longer needed. */
    for (j = 0; j < USER_REGIS_SRF_SAMP_SIZE; j++)
	IritFree(SrfGridPts[j]);
    IritFree(SrfGridPts);

    /* Iterate between registration of the point set to the surface and     */
    /* between numerical marching on the surface.			    */
    PtsTmp1 = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * n);
    PtsTmp2 = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * n);
    MatGenUnitMat(RegMat);
    for (i = 0; i < USER_REGIS_MAX_NUM_OF_ITERS; i++) {
	for (l = 0; l < n; l++) {
	    R = UserSrfSafeEval(Srf, SrfUVVals[l][0], SrfUVVals[l][1]);
	    CagdCoerceToE3(PtsTmp1[l], &R, -1, Srf -> PType);
	}
	IRIT_GEN_COPY(PtsTmp2, PtsSet, sizeof(IrtPtType) * n);
	Err = UserRegisterTwoPointSets(n, PtsTmp2, n, PtsTmp1, AlphaConverge,
			  Tolerance, UserRegisterTestSrfConvergance, Mat);
	MatMultTwo4by4(RegMat, Mat, RegMat);

#	ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugRegPrintErr)
	        IRIT_INFO_MSG_PRINTF("Iter %d) ERROR = %15.13lf\n",
					i, Err);
#	endif /* DEBUG */
	if (Err < Tolerance)
	    break;

	for (l = 0; l < n; l++) {
	    MatMultPtby4by4(PtsSet[l], PtsSet[l], Mat);
	    
	    SrfUpdateClosestPoint(PtsSet[l], Srf,
				  &SrfUVVals[l][0], &SrfUVVals[l][1],
				  Tolerance / 10.0, &Dist);
	}
    }

    IritFree(PtsTmp1);
    IritFree(PtsTmp2);
    IritFree(SrfUVVals);

    return Err;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Safe evaluation of Srf, after making sure (u, v) is in the domain of Srf.*
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    To evaluate.                                                     *
*   u, v:   Location of evaluation.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtRType *:  Evaluated location.                                         *
*****************************************************************************/
static IrtRType *UserSrfSafeEval(const CagdSrfStruct *Srf,
				 CagdRType u,
				 CagdRType v)
{
    CagdRType UMin, UMax, VMin, VMax;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    u = IRIT_BOUND(u, UMin, UMax);
    v = IRIT_BOUND(v, VMin, VMax);

    return CagdSrfEval(Srf, u, v);
    
}
