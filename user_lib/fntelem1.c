/******************************************************************************
* FntElem1.c - some routines to handle finite elements evaluations using Bsp. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 2007.					      *
******************************************************************************/

#include "irit_sm.h"
#include "user_loc.h"

#define USER_FE_DPHI_I_DU	(DUEval[i].BasisFuncsVals[mU0] * \
				  VEval[j].BasisFuncsVals[mV0])

#define USER_FE_DPHI_J_DU	(DUEval[i].BasisFuncsVals[nU0] * \
				  VEval[j].BasisFuncsVals[nV0])

#define USER_FE_DPHI_I_DV	( UEval[i].BasisFuncsVals[mU0] * \
				 DVEval[j].BasisFuncsVals[mV0])

#define USER_FE_DPHI_J_DV	( UEval[i].BasisFuncsVals[nU0] * \
				 DVEval[j].BasisFuncsVals[nV0])

static int UserFEGetGaussPts(CagdRType *KnotVector,
			     int Order,
			     int Length,
			     int IntegRes,
			     CagdRType **GaussPts,
			     CagdRType **Weights);
static void UserFEKIntegrateOne(UserFEKElementStruct *K,
				CagdSrfStruct *Srf,
				CagdBspBasisFuncEvalStruct *UEval,
				CagdBspBasisFuncEvalStruct *VEval,
				CagdBspBasisFuncEvalStruct *DUEval,
				CagdBspBasisFuncEvalStruct *DVEval,
				CagdRType *J1,
				int UEvalLen,
				int VEvalLen,
				int m,
				int n,
				IrtRType E,
				IrtRType Nu);
static IrtRType UserFECIntegrateOne(UserFEInterIntervalStruct *Interval,
				    CagdCrvStruct *Crv1,
				    CagdCrvStruct *Crv2,
				    CagdBspBasisFuncEvalStruct *Eval1,
				    CagdBspBasisFuncEvalStruct *Eval2,
				    int UEvalLen,
				    int VEvalLen,
				    int m,
				    int n);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes all the necessary Gauss evaluation points for the numeric       *
* integration.                                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   KnotVector:  Of the space to compute Gauss points for.                   *
*   Order:       Of the space to compute Gauss points for.                   *
*   Length:      Number of control points in this sub-space.                 *
*   IntegRes:    Resolution of integration - number of sample per interval.  *
*   GaussPts:    The computed Guass points vector, allocated dynamically.    *
*   Weights:     The weights associated with each point, allocated dynam.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *: *   Size: of Gauss points/weights returned vectors.         *
*****************************************************************************/
static int UserFEGetGaussPts(CagdRType *KnotVector,
			     int Order,
			     int Length,
			     int IntegRes,
			     CagdRType **GaussPts,
			     CagdRType **Weights)
{
    IRIT_STATIC_DATA IrtRType /* All gauss points normalized to domain [0, 1].*/
        GaussPoints2[2] = { 0.21132486540519, 0.78867513459481 },
        GaussWeights2[2] = { 0.5, 0.5 },
	GaussPoints3[3] = { 0.11270166537926, 0.5, 0.88729833462074 },
	GaussWeights3[3] = { 0.27777777777778, 0.44444444444444,
			     0.27777777777778 },
	GaussPoints4[4] = { 0.069431844202524, 0.33000947820757,
			    0.66999052179243, 0.93056815579748 },
	GaussWeights4[4] = { 0.17392742256873, 0.32607257743127,
			     0.32607257743127, 0.17392742256873 };
    int i, j,
        n = Length - Order + 1,
        NRes = n * IntegRes;
    CagdRType *Pts, *Wgt, *t, dt;

    *GaussPts = Pts = (CagdRType *) IritMalloc(sizeof(CagdRType) * NRes);
    *Weights = Wgt = (CagdRType *) IritMalloc(sizeof(CagdRType) * NRes);

    for (i = 0, t = &KnotVector[Order - 1]; i < n; i++, t++) {
        dt = t[1] - t[0];

        switch (IntegRes) {
	    case 2:
	        for (j = 0; j < 2; j++) {
		    *Pts++ = dt * GaussPoints2[j] + t[0];
		    *Wgt++ = dt * GaussWeights2[j];
		}
		break;
	    case 3:
	        for (j = 0; j < 3; j++) {
		    *Pts++ = dt * GaussPoints3[j] + t[0];
		    *Wgt++ = dt * GaussWeights3[j];
		}
		break;
	    case 4:
	        for (j = 0; j < 4; j++) {
		    *Pts++ = dt * GaussPoints4[j] + t[0];
		    *Wgt++ = dt * GaussWeights4[j];
		}
		break;
	    default:
	        assert(0);
		USER_FATAL_ERROR(USER_ERR_INVALID_SIZE);
		break;
	}
    }

    return NRes;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates K entry - int( L Phi(m) * S * ( L Phi(n)^T ) ), where L is a   *
* differentiation operator. See also UserFEBuildKMat.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   K:        Points to the current K element to update.                     *
*   Srf:      The surface we work so hard for...			     *
*   UEval:    Evaluation of all U basis functions at all Gauss points.       *
*   VEval:    Evaluation of all V basis functions at all Gauss points.       *
*   DUEval:   Evaluation of all derivatives of U basis funcs at Gauss pts.   *
*   DVEval:   Evaluation of all derivatives of V basis funcs at Gauss pts.   *
*   J1:       (The reciproal of the) Jacobian of srf at all Gauss points     *
*   UEvalLen: Size of UEval, DUEval vectors.				     *
*   VEvalLen: Size of VEval, DVEval vectors.				     *
*   m, n:     indices of the two basis functions to consider.                *
*   E:        Young module value.                                            *
*   Nu:       Poisson coefficient, between 0.0 and 0.5.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserFEKIntegrateOne(UserFEKElementStruct *K,
				CagdSrfStruct *Srf,
				CagdBspBasisFuncEvalStruct *UEval,
				CagdBspBasisFuncEvalStruct *VEval,
				CagdBspBasisFuncEvalStruct *DUEval,
				CagdBspBasisFuncEvalStruct *DVEval,
				CagdRType *J1,
				int UEvalLen,
				int VEvalLen,
				int m,
				int n,
				IrtRType E,
				IrtRType Nu)
{
    int i, j, mU, mV, nU, nV, MinI, MinJ, IStart, JStart,
	ULength = Srf -> ULength,
        UOrder = Srf -> UOrder,
        VOrder = Srf -> VOrder;
    IrtRType
	Nu2 = (1.0 - Nu) * 0.5;

    IRIT_ZAP_MEM(K, sizeof(UserFEKElementStruct));

    /* Decompose indices m & n into their U and V components: Phi(m) equals */
    /* B_mU(u) * B_mV(v) where mV = m / ULength and mU = m % ULength.       */
    mU = m % ULength;
    mV = m / ULength;
    nU = n % ULength;
    nV = n / ULength;

    /* Make sure there is some overlap between Phi(m) and Phi(n) and if not */
    /* return now.							    */
    if (IRIT_ABS(mU - nU) >= UOrder || IRIT_ABS(mV - nV) >= VOrder)
        return;
	
    /* Find out the beginning of the range we need to process for the i'th  */
    /* function in U and the j'th in V.					    */
    MinI = IRIT_MAX(mU, nU);
    for (IStart = 0; IStart < UEvalLen; IStart++) {
        if (UEval[IStart].FirstBasisFuncIndex + UOrder > MinI)
	    break;
    }
    MinJ = IRIT_MAX(mV, nV);
    for (JStart = 0; JStart < VEvalLen; JStart++) {
        if (VEval[JStart].FirstBasisFuncIndex + VOrder > MinJ)
	    break;
    }

    /* Now accumulate all Gauss points values, computing the integration.   */
    /*   Could be optimized by exploiting the symmetry of the matrix.       */
    for (j = JStart; j < VEvalLen; j++) {
        int mU0, mV0, nU0, nV0;

	assert(VEval[j].FirstBasisFuncIndex == DVEval[j].FirstBasisFuncIndex);
	mV0 = mV - VEval[j].FirstBasisFuncIndex;
	nV0 = nV - VEval[j].FirstBasisFuncIndex;
	/* Are we beyond the support of either basis function!? */
	if (mV0 < 0 || nV0 < 0)
	    break;
	assert(mV0 >= 0 && mV0 < VOrder && nV0 >= 0 && nV0 < VOrder);

	for (i = IStart; i < UEvalLen; i++) {
	    IrtRType EJ;

	    assert(UEval[i].FirstBasisFuncIndex == 
		   DUEval[i].FirstBasisFuncIndex);
	    mU0 = mU - UEval[i].FirstBasisFuncIndex;
	    nU0 = nU - UEval[i].FirstBasisFuncIndex;
	    /* Are we beyond the support of either basis function!? */
	    if (mU0 < 0 || nU0 < 0)
	        break;
	    assert(mU0 >= 0 && mU0 < UOrder && nU0 >= 0 && nU0 < UOrder);

	    /* Estimate the Jacobian. */
	    EJ = E * J1[i + j * UEvalLen];

#	    ifdef DEBUG
	    {
	        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpFuncValues, FALSE) {
		    if (i == IStart && j == JStart)
		        fprintf(stderr, "\n");
		    fprintf(stderr,
		      "At [%d %d] :: dPhi(%d)dx = %8.5f, dPhi(%d)dx = %8.5f\n",
		      i, j, m, USER_FE_DPHI_I_DU, n, USER_FE_DPHI_J_DU);
		    fprintf(stderr,
		      "         :: dPhi(%d)dy = %8.5f, dPhi(%d)dy = %8.5f\n",
		      m, USER_FE_DPHI_I_DV, n, USER_FE_DPHI_J_DV);
		}
	    }
#	    endif /* DEBUG */

	    K -> k[0][0] += (USER_FE_DPHI_I_DU * USER_FE_DPHI_J_DU +
			     USER_FE_DPHI_I_DV * USER_FE_DPHI_J_DV * Nu2) * EJ;

	    K -> k[1][0] += (USER_FE_DPHI_I_DU * USER_FE_DPHI_J_DV * Nu +
			     USER_FE_DPHI_I_DV * USER_FE_DPHI_J_DU * Nu2) * EJ;

	    K -> k[0][1] += (USER_FE_DPHI_I_DV * USER_FE_DPHI_J_DU * Nu +
			     USER_FE_DPHI_I_DU * USER_FE_DPHI_J_DV * Nu2) * EJ;

	    K -> k[1][1] += (USER_FE_DPHI_I_DV * USER_FE_DPHI_J_DV +
			     USER_FE_DPHI_I_DU * USER_FE_DPHI_J_DU * Nu2) * EJ;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the elements of the stiffness matrix K as the integral of the  M
* basis function's derivatives of surface Srf.  Let B be a linear vector of  M
* all n polynomial patches' basis functions of Srf.  Let S be the matrix     M
*                 E      [1          Nu          0]			     V
*          S = --------- [Nu         1           0] ,			     V
*              (1- Nu^2) [0          0  (1-Nu) / 2]			     V
* let L be the differentiation operator of				     M
*            L = [d/du      0         d/dv]				     V
*                [0         d/dv      d/dx] ,				     V
* and let M be the application of L to all basis functions in B.  M is a     M
* matrix of size (2n x 3).  Finally compute and return the integrals of      M
* K = M S M^T , K is a matrix of size (2n x 2n).			     M
*   Because the integration is to be conducted in spatial space instead of   M
* parametric domain, we must normalize by the determinant of the Jacobian    M
* hence the integration is conducted numerically.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To compute its K stiffness matrix.                            M
*   IntegRes:  Resolution of integration - number of sample per interval.    *
*   E:         Young Module value.                                           M
*   Nu:        Poisson coefficient, between 0.0 and 0.5.                     M
*   Size:      Number of degree-of-freedom (basis functions) Srf contains.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFEKElementStruct *: Vector of size n^2 with the (n x n) elmnts of K. M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat2, UserFEBuildC1Mat, UserFEBuildC2Mat, UserFEEvalRHSC     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEBuildKMat                                                          M
*****************************************************************************/
UserFEKElementStruct *UserFEBuildKMat(CagdSrfStruct *Srf,
				      int IntegRes,
				      IrtRType E,
				      IrtRType Nu,
				      int *Size)
{
    int i, j, UGaussSize, VGaussSize,
        UOrder = Srf -> UOrder,
        VOrder = Srf -> VOrder,
        ULength = Srf -> ULength,
        VLength = Srf -> VLength,
        n = ULength * VLength;
    UserFEKElementStruct *KPtr,
        *K = (UserFEKElementStruct *)
			    IritMalloc(sizeof(UserFEKElementStruct) * IRIT_SQR(n));
    CagdRType *UGauss, *VGauss, *UWeight, *VWeight, *J1, *J1Ptr, NuSqr1;
    CagdBspBasisFuncEvalStruct *UEvals, *VEvals, *DUEvals, *DVEvals;
    CagdSrfStruct *DuSrf, *DvSrf;

    if (CAGD_IS_BEZIER_SRF(Srf))
        Srf = CagdCnvrtBzr2BspSrf(Srf);
    else
        Srf = CagdSrfCopy(Srf);

    *Size = n;

    UGaussSize = UserFEGetGaussPts(Srf -> UKnotVector, UOrder, ULength, 
				   IntegRes, &UGauss, &UWeight);
    VGaussSize = UserFEGetGaussPts(Srf -> VKnotVector, VOrder, VLength,
				   IntegRes, &VGauss, &VWeight);

    /* Evalue all basis functions at the Gauss points. */
    UEvals = BspBasisFuncMultEval(Srf -> UKnotVector, ULength + UOrder,
				  UOrder, FALSE, UGauss, UGaussSize,
				  CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    VEvals = BspBasisFuncMultEval(Srf -> VKnotVector, VLength + VOrder,
				  VOrder, FALSE, VGauss, VGaussSize,
				  CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    DUEvals = BspBasisFuncMultEval(Srf -> UKnotVector, ULength + UOrder,
				   UOrder, FALSE, UGauss, UGaussSize,
				   CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);
    DVEvals = BspBasisFuncMultEval(Srf -> VKnotVector, VLength + VOrder,
				   VOrder, FALSE, VGauss, VGaussSize,
				   CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

    /* Evaluate the partials and 1/Jacobian at all Gauss points. */
    J1 = IritMalloc(sizeof(UserFEKElementStruct) * UGaussSize * VGaussSize);
    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

    /* The case of Nu == 1 is only used in testing. */
    NuSqr1 = Nu == 1.0 ? 1.0 : 1.0 / (1.0 - IRIT_SQR(Nu));

    for (j = 0, J1Ptr = J1; j < VGaussSize; j++) {
        for (i = 0; i < UGaussSize; i++) {
	    CagdRType *R, t;
	    CagdPType DsDu, DsDv;

	    R = CagdSrfEval(DuSrf, UGauss[i], VGauss[j]);
	    CagdCoerceToE2(DsDu, &R, -1, DuSrf -> PType);

	    R = CagdSrfEval(DvSrf, UGauss[i], VGauss[j]);
	    CagdCoerceToE2(DsDv, &R, -1, DvSrf -> PType);

	    t = DsDu[0] * DsDv[1] - DsDu[1] * DsDv[0];

	    //#	    define DEBUG_DISABLE_JACOBIAN_NU_FACTOR
#	    ifdef DEBUG_DISABLE_JACOBIAN_NU_FACTOR
	        *J1Ptr++ = UWeight[i] * VWeight[j];
#	    else
	        *J1Ptr++ = (DsDu[0] * DsDv[1] - DsDu[1] * DsDv[0])
			   * UWeight[i] * VWeight[j] * NuSqr1;
#	    endif /* DEBUG_DISABLE_JACOBIAN_NU_FACTOR */
	}
    }
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    for (j = 0, KPtr = K; j < n; j++) {
        for (i = 0; i < n; i++, KPtr++) {
	    /* Handle the Phi(i) x Phi(j). */
	    UserFEKIntegrateOne(KPtr, Srf, UEvals, VEvals, DUEvals, DVEvals,
				J1, UGaussSize, VGaussSize, i, j, E, Nu);
	}
    }

    BspBasisFuncMultEvalFree(UEvals, UGaussSize);
    BspBasisFuncMultEvalFree(VEvals, VGaussSize);
    BspBasisFuncMultEvalFree(DUEvals, UGaussSize);
    BspBasisFuncMultEvalFree(DVEvals, VGaussSize);

    IritFree(UGauss);
    IritFree(VGauss);
    IritFree(UWeight);
    IritFree(VWeight);
    IritFree(J1);

    CagdSrfFree(Srf);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpFEKMatrix, FALSE) {
	    for (j = 0; j < *Size; j++) {
	        int l;

	        for (l = 0; l < 2; l++) {
		    for (i = 0; i < *Size; i++) {
		        printf("%s%8.5f %8.5f",
			       i == 0 ? "" : "   ",
			       K[i + j * *Size].k[l][0],
			       K[i + j * *Size].k[l][1]);
		    }
		    printf("\n");
		}
		for (i = 1; i < *Size; i++)
		    printf("                    ");
		printf("                 \n");
	    }
	    printf("\n\n");
	}
    }
#   endif /* DEBUG */

    return K;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the elements of the stiffness matrix K as the integral of the  M
* basis function's derivatives of surface Srf.  Let B be a linear vector of  M
* all n polynomial patches' basis functions of Srf.  Let S be the matrix     M
*                 E      [1          Nu          0]			     V
*          S = --------- [Nu         1           0] ,			     V
*              (1- Nu^2) [0          0  (1-Nu) / 2]			     V
* let L be the differentiation operator of				     M
*            L = [d/du      0         d/dv]				     V
*                [0         d/dv      d/dx] ,				     V
* and let M be the application of L to all basis functions in B.  M is a     M
* matrix of size (2n x 3).  Finally compute and return the integrals of      M
* K = M S M^T , K is a matrix of size (2n x 2n).			     M
*   Because the integration is to be conducted in spatial space instead of   M
* parametric domain, we must normalize by the determinant of the Jacobian    M
* hence the integration is conducted numerically.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:    Current surface control points.                               M
*   ULength, VLength:  Dimensions of Points vectors.			     M
*   UOrder, VOrder:    Surface Order in U and V.			     M
*   EndCond:   End conditions - open, float, etc.			     M
*   IntegRes:  Resolution of integration - number of sample per interval.    *
*   E:         Young Module value.                                           M
*   Nu:        Poisson coefficient, between 0.0 and 0.5.                     M
*   Size:      Number of degree-of-freedom (basis functions) Srf contains.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFEKElementStruct *: Vector of size n^2 with the (n x n) elmnts of K. M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEBuildKMat2                                                         M
*****************************************************************************/
UserFEKElementStruct *UserFEBuildKMat2(CagdPType *Points,
				       int ULength,
				       int VLength,
				       int UOrder,
				       int VOrder,
				       CagdEndConditionType EndCond,
				       int IntegRes,
				       IrtRType E,
				       IrtRType Nu,
				       int *Size)
{
    int i;
    CagdSrfStruct
        *Srf = BspSrfNew(ULength, VLength, UOrder, VOrder,
			 CAGD_PT_E2_TYPE);
    UserFEKElementStruct *K;

    switch (EndCond) {
	case CAGD_END_COND_OPEN:
	    BspKnotUniformOpen(ULength, UOrder, Srf -> UKnotVector);
	    BspKnotUniformOpen(VLength, VOrder, Srf -> VKnotVector);
	    break;
        case CAGD_END_COND_FLOAT:
	default:
	    BspKnotUniformFloat(ULength, UOrder, Srf -> UKnotVector);
	    BspKnotUniformFloat(VLength, VOrder, Srf -> VKnotVector);
	    break;
    }

    for (i = 0; i < ULength * VLength; i++) {
        Srf -> Points[1][i] = Points[i][0];
        Srf -> Points[2][i] = Points[i][1];
    }

    K = UserFEBuildKMat(Srf, IntegRes, E, Nu, Size);

    CagdSrfFree(Srf);

    return K;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns TRUE if given point Pt is inside surface Srf, FALSE otherwise.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to test the inclusion of Pt in, in R^2.                   M
*   Pt:    2D point to test if inside Srf.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if inside, FALSE otherwise.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEPointInsideSrf                                                     M
*****************************************************************************/
CagdBType UserFEPointInsideSrf(CagdSrfStruct *Srf, CagdPType Pt)
{
    CagdRType *R,
        *UV = MvarDistSrfPoint(Srf, Pt, TRUE, 0.01, IRIT_EPS);
    CagdPType SrfPt;

    if (UV == NULL)
        return FALSE;

    R = CagdSrfEval(Srf, UV[0], UV[1]);
    CagdCoerceToE2(SrfPt, &R, -1, Srf -> PType);

    return IRIT_PT_APX_EQ_E2_EPS(Pt, SrfPt, IRIT_EPS);    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Derives the interval(s) where Crv1 and Crv2 intersect, two boundary      M
* curves of Srf1 and Srf2.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:    A boundary curve of Srf1 to determine its interesction interval M
*	     with Crv2.						             M
*   Srf1:    The first surface to examine for collision with Srf2.           M
*   Crv2:    A boundary curve of Srf2 to determine its interesction interval M
*	     with Crv1.						             M
*   Srf2:    The second surface to examine for collision with Srf1.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFEInterIntervalStruct *:  A list of elements, each holding one	     M
*				continuous interval of intersection,	     M
*				specifying the proper intervals in both	     M
*				Crv1 and Crv2.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEGetInterInterval                                                   M
*****************************************************************************/
UserFEInterIntervalStruct *UserFEGetInterInterval(CagdCrvStruct *Crv1,
						  CagdSrfStruct *Srf1,
						  CagdCrvStruct *Crv2,
						  CagdSrfStruct *Srf2)
{
    CagdRType PrevT1, PrevT2, TMax1, TMax2, TMid1, TMid2, *R;
    CagdPType Crv1MidPt, Crv2MidPt;
    CagdPtStruct
        *Pt = CagdPtNew(),
        *Pts = CagdCrvCrvInter(Crv1, Crv2, IRIT_EPS);
    UserFEInterIntervalStruct *Dmn,
        *Dmns = NULL;

    CagdCrvDomain(Crv1, &PrevT1, &TMax1);
    CagdCrvDomain(Crv2, &PrevT2, &TMax2);

    /* Place the intervals' maximums as last point to examine. */
    Pt -> Pt[0] = TMax1;
    Pt -> Pt[1] = TMax2;
    if (Pts != NULL)
        ((CagdPtStruct *) CagdListLast(Pts)) -> Pnext = Pt;
    else
	Pts = Pt;

    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
        TMid1 = (PrevT1 + Pt -> Pt[0]) * 0.5;
        TMid2 = (PrevT2 + Pt -> Pt[1]) * 0.5;

	R = CagdCrvEval(Crv1, TMid1);
	CagdCoerceToE3(Crv1MidPt, &R, -1, Crv1 -> PType);
	R = CagdCrvEval(Crv2, TMid1);
	CagdCoerceToE3(Crv2MidPt, &R, -1, Crv2 -> PType);

	if (UserFEPointInsideSrf(Srf1, Crv2MidPt) &&
	    UserFEPointInsideSrf(Srf2, Crv1MidPt)) {
	    /* This interval is overlapping - keep it. */
	    Dmn = (UserFEInterIntervalStruct *)
	        IritMalloc(sizeof(UserFEInterIntervalStruct));
	    Dmn -> T1Min = PrevT1;
	    Dmn -> T1Max = Pt -> Pt[0];
	    Dmn -> Antipodal1 = TMid1;  /* Not exactly but will do for now. */

	    Dmn -> T2Min = PrevT2;
	    Dmn -> T2Max = Pt -> Pt[1];
	    Dmn -> Antipodal2 = TMid2;  /* Not exactly but will do for now. */

	    IRIT_VEC2D_SUB(Dmn -> ProjNrml, Crv2MidPt, Crv1MidPt);
	    IRIT_VEC2D_NORMALIZE(Dmn -> ProjNrml);
	    Dmn -> ProjNrml[2] = 0.0;

	    IRIT_LIST_PUSH(Dmn, Dmns);
	}

	PrevT1 = Pt -> Pt[0];
	PrevT2 = Pt -> Pt[1];
    }

    CagdPtFreeList(Pts);

    return CagdListReverse(Dmns);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates C1 entry = int( Phi_1(m) * Phi_2(n) ), over given intersection *
* interval.  Both Phi_1(m) * Phi_2(n) share the same parameter space.        *
*                                                                            *
* PARAMETERS:                                                                *
*   C1:           Points to the current C element to update.                 *
*   Interval:     To integrate over, based on T1Min/Max values.		     *
*   Crv1:         The relevant boundary curve, specifying the working space. *
*   Eval:	  Evaluation of all basis functions at all Gauss points of   *
*		  the space.						     *
*   GaussPts:     Parameter values Eval was evalued at.			     *
*   GaussWeights: Weights values (Gauss integ.) at which Eval was evalued.   *
*   EvalLen:      Size of Eval and GaussPts vectors.			     *
*   m, n:         Indices of the two basis functions to consider.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void UserFEC1IntegrateOne(UserFECElementStruct *C1,
				 UserFEInterIntervalStruct *Interval,
				 CagdCrvStruct *Crv1,
				 CagdBspBasisFuncEvalStruct *Eval,
				 CagdRType *GaussPts,
				 CagdRType *GaussWeights,
				 int EvalLen,
				 int m,
				 int n)
{
    int i, m0, n0, MaxMN, IStart,
        Length = Crv1 -> Length,
	Order = Crv1 -> Order;
    CagdRType
	RetVal = 0.0,
        *KV = Crv1 -> KnotVector,
        **CrvPts = Crv1 -> Points,
        Interval1TMin = Interval -> T1Min,
        Interval1TMax = Interval -> T1Max;

    /* Make sure there is some overlap between Phi(m) and Phi(n) and if not */
    /* return now.							    */
    if (IRIT_ABS(m - n) >= Order)
        return;

    /* Find out the beginning of the range we need to process for m'th and  */
    /* n'th.  Note we start at IRIT_MAX(m, n) as we dont care if either is 0.*/
    MaxMN = IRIT_MAX(m, n);
    for (IStart = 0; IStart < EvalLen; IStart++) {
        if (Eval[IStart].FirstBasisFuncIndex + Order > MaxMN)
	    break;
    }

    for (i = IStart; i < EvalLen; i++) {
	/* Accumulate only evaluations that are in domain of intersection.  */
	if (GaussPts[i] < Interval1TMin)
	    continue;
	else if (GaussPts[i] > Interval1TMax)
	    break;

        m0 = m - Eval[i].FirstBasisFuncIndex;
        n0 = n - Eval[i].FirstBasisFuncIndex;
	
	/* Are we beyond the support of either basis function!? */
	if (m0 < 0 || n0 < 0)
	    break;
	assert(m0 >= 0 && m0 < Order && n0 >= 0 && n0 < Order);

#       ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpFuncValues, FALSE) {
	        if (i == IStart)
		    fprintf(stderr, "\n");
		fprintf(stderr,
			"At [%d] :: dPhi(%d)dx = %8.5f, dPhi(%d)dx = %8.5f\n",
			i, m, Eval[i].BasisFuncsVals[m0],
			   n, Eval[i].BasisFuncsVals[n0]);
	    }
	}
#       endif /* DEBUG */

	RetVal += Eval[i].BasisFuncsVals[m0] *
		  Eval[i].BasisFuncsVals[n0] * GaussWeights[i];
    }

    /* Project the result onto the normal to the intersection. */
    C1 -> c[0] += RetVal * Interval -> ProjNrml[0];
    C1 -> c[1] += RetVal * Interval -> ProjNrml[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the elements of the C1 intersection matrix as the integral of  M
* the basis function's products along the intersecting boundary.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:    A boundary curve of Srf1 to determine its interesction interval M
*	     with Crv2.  Has M control points.			             M
*   Srf1:    The first surface to examine for collision with Srf2.           M
*   Crv2:    A boundary curve of Srf2 to determine its interesction interval M
*	     with Crv1.						             M
*   Srf2:    The second surface to examine for collision with Srf1.          M
*   IntegRes:  Resolution of integration - number of sample per interval.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFECElementStruct *: Vector of size (M x M) elements of C1.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat, UserFEBuildC2Mat, UserFEEvalRHSC                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEBuildC1Mat                                                         M
*****************************************************************************/
UserFECElementStruct *UserFEBuildC1Mat(CagdCrvStruct *Crv1,
				       CagdSrfStruct *Srf1,
				       CagdCrvStruct *Crv2,
				       CagdSrfStruct *Srf2,
				       int IntegRes)
{
    int i, j, GaussSize,
	M = Crv1 -> Length;
    CagdRType *GaussPts, *GaussWeights, *KV1;
    UserFECElementStruct
        *C1 = (UserFECElementStruct *)
		           IritMalloc(sizeof(UserFECElementStruct) * IRIT_SQR(M)),
        *C1Ptr = C1;
    UserFEInterIntervalStruct *II,
        *InterIntervals = UserFEGetInterInterval(Crv1, Srf1, Crv2, Srf2);
    CagdBspBasisFuncEvalStruct *Evals;

    if (CAGD_IS_BEZIER_CRV(Crv1))
        KV1 = BspKnotUniformOpen(Crv1 -> Length, Crv1 -> Order, NULL);
    else
        KV1 = Crv1 -> KnotVector;
    GaussSize = UserFEGetGaussPts(KV1, Crv1 -> Order,
				  Crv1 -> Length, IntegRes, &GaussPts,
				  &GaussWeights);
    Evals = BspBasisFuncMultEval(KV1,
				 Crv1 -> Length + Crv1 -> Order,
				 Crv1 -> Order, FALSE, GaussPts, GaussSize,
				 CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    for (i = 0; i < M; i++) {
        for (j = 0; j < M; j++, C1Ptr++) {
	    C1Ptr -> c[0] = C1Ptr -> c[1] = 0.0;
	    for (II = InterIntervals; II != NULL; II = II -> Pnext) {
	        UserFEC1IntegrateOne(C1Ptr, II, Crv1, Evals, GaussPts,
				     GaussWeights, GaussSize, i, j);
	    }
	}
    }

    /* Free all auxiliary data structures. */
    while (InterIntervals != NULL) {
        IRIT_LIST_POP(II, InterIntervals);
        IritFree(II);
    }
    BspBasisFuncMultEvalFree(Evals, GaussSize);
    IritFree(GaussPts);
    IritFree(GaussWeights);

    if (CAGD_IS_BEZIER_CRV(Crv1))
        IritFree(KV1);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpFEC1Matrix, TRUE) {
	    for (i = 0; i < M; i++) {
	        for (j = 0; j < M; j++) {
		    printf("%s%8.6f %8.6f",
			   j == 0 ? "" : "   ",
			   C1[j + i * M].c[0],
			   C1[j + i * M].c[1]);
		}
		printf("\n");
	    }
	    printf("\n\n");
	}
    }
#   endif /* DEBUG */

    return C1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the elements of the C1 intersection matrix as the integral of  M
* the basis function's products along the intersecting boundary.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Pts:     Crv1's control points (M).                                  M
*   Crv1Length:  Dimensions of Crv1Pts vector.				     M
*   Crv1Order:   Order of Crv1.						     M
*   Srf1Pts:     Srf1's control points.                                      M
*   Srf1ULength, Srf1VLength:  Dimensions of Srf1Pts vector.		     M
*   Srf1UOrder, Srf1VOrder:    Orders of Srf1.				     M
*   Crv2Pts:     Curve2's control points.                                    M
*   Crv2Length:  Dimensions of Crv2Pts vector.				     M
*   Crv2Order:   Order of Curve2.					     M
*   Srf2Pts:     Srf2's control points.                                      M
*   Srf2ULength, Srf2VLength:  Dimensions of Srf2Pts vector.		     M
*   Srf2UOrder, Srf2VOrder:    Orders of Srf2.				     M
*   EndCond:     Float or open, in reconstructed Crv1/2, Srf1/2.	     M
*   IntegRes:    Resolution of integration - number of sample per interval.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFECElementStruct *: Vector of size (M x M) elements of C1.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat, UserFEBuildC2Mat, UserFEBuildC1Mat, UserFEEvalRHSC      M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEBuildC1Mat2                                                        M
*****************************************************************************/
UserFECElementStruct *UserFEBuildC1Mat2(CagdPType *Crv1Pts,
					int Crv1Length,
					int Crv1Order,
					CagdPType *Srf1Pts,
					int Srf1ULength,
					int Srf1VLength,
					int Srf1UOrder,
					int Srf1VOrder,
					CagdPType *Crv2Pts,
					int Crv2Length,
					int Crv2Order,
					CagdPType *Srf2Pts,
					int Srf2ULength,
					int Srf2VLength,
					int Srf2UOrder,
					int Srf2VOrder,
					CagdEndConditionType EndCond,
					int IntegRes)
{
    int i;
    CagdCrvStruct
        *Crv1 = BspCrvNew(Crv1Length, Crv1Order, CAGD_PT_E2_TYPE),
        *Crv2 = BspCrvNew(Crv2Length, Crv2Order, CAGD_PT_E2_TYPE);
    CagdSrfStruct
        *Srf1 = BspSrfNew(Srf1ULength, Srf1VLength, Srf1UOrder, Srf1VOrder,
			  CAGD_PT_E2_TYPE),
        *Srf2 = BspSrfNew(Srf2ULength, Srf2VLength, Srf2UOrder, Srf2VOrder,
			  CAGD_PT_E2_TYPE);
    UserFECElementStruct *C1;

    switch (EndCond) {
	case CAGD_END_COND_OPEN:
	    BspKnotUniformOpen(Crv1Length, Crv1Order, Crv1 -> KnotVector);
	    BspKnotUniformOpen(Crv2Length, Crv2Order, Crv2 -> KnotVector);
	    BspKnotUniformOpen(Srf1ULength, Srf1UOrder, Srf1 -> UKnotVector);
	    BspKnotUniformOpen(Srf1VLength, Srf1VOrder, Srf1 -> VKnotVector);
	    BspKnotUniformOpen(Srf2ULength, Srf2UOrder, Srf2 -> UKnotVector);
	    BspKnotUniformOpen(Srf2VLength, Srf2VOrder, Srf2 -> VKnotVector);
	    break;
        case CAGD_END_COND_FLOAT:
	default:
	    BspKnotUniformFloat(Crv1Length, Crv1Order, Crv1 -> KnotVector);
	    BspKnotUniformFloat(Crv2Length, Crv2Order, Crv2 -> KnotVector);
	    BspKnotUniformFloat(Srf1ULength, Srf1UOrder, Srf1 -> UKnotVector);
	    BspKnotUniformFloat(Srf1VLength, Srf1VOrder, Srf1 -> VKnotVector);
	    BspKnotUniformFloat(Srf2ULength, Srf2UOrder, Srf2 -> UKnotVector);
	    BspKnotUniformFloat(Srf2VLength, Srf2VOrder, Srf2 -> VKnotVector);
	    break;
    }

    for (i = 0; i < Crv1Length; i++)
        Crv1 -> Points[1][i] = Crv1Pts[i][0];
    for (i = 0; i < Crv2Length; i++)
        Crv2 -> Points[1][i] = Crv1Pts[i][0];

    for (i = 0; i < Srf1ULength * Srf1VLength; i++) {
        Srf1 -> Points[1][i] = Srf1Pts[i][0];
        Srf1 -> Points[2][i] = Srf1Pts[i][1];
    }
    for (i = 0; i < Srf2ULength * Srf2VLength; i++) {
        Srf2 -> Points[1][i] = Srf2Pts[i][0];
        Srf2 -> Points[2][i] = Srf2Pts[i][1];
    }

    C1 = UserFEBuildC1Mat(Crv1, Srf1, Crv2, Srf2, IntegRes);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return C1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates C2 entry = int( Phi_1(m) * Phi_2(n) ), over given intersection *
* interval.  Both Phi_1(m) * Phi_2(n) share the same parameter space.        *
*                                                                            *
* PARAMETERS:                                                                *
*   C2:           Points to the current C element to update.                 *
*   Interval:     To integrate over, based on T1Min/Max values.		     *
*   Crv1:         The first boundary curve, specifying the working space.    *
*   Crv2:         The second boundary curve, specifying the working space.   *
*   Eval1:	  Evaluation of all basis functions at all Gauss points of   *
*		  the space of Crv1.					     *
*   Gauss1Pts:    Parameter values Eval was evalued at, for Crv1.	     *
*   Gauss1Weights: Weights values (Gauss integ.) at which Eval was evalued,  *
*		  for Crv1.						     *
*   Eval1Len:     Size of Eval1 and Gauss1Pts/Weights vectors.		     *
*   m, n:         Indices of the two basis functions to consider.            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	                                                             *
*****************************************************************************/
static void UserFEC2IntegrateOne(UserFECElementStruct *C2,
				 UserFEInterIntervalStruct *Interval,
				 CagdCrvStruct *Crv1,
				 CagdCrvStruct *Crv2,
				 CagdBspBasisFuncEvalStruct *Eval1,
				 CagdRType *Gauss1Pts,
				 CagdRType *Gauss1Weights,
				 int Eval1Len,
				 int m,
				 int n)
{
    int i, m0, n0, IStart, Index2First,
        Length1 = Crv1 -> Length,
        Length2 = Crv2 -> Length,
        Order1 = Crv1 -> Order,
	Order2 = Crv2 -> Order;
    CagdRType DtRatio, t2, *Eval2, *KV2,
	RetVal = 0.0,
        **Crv1Pts = Crv1 -> Points,
        **Crv2Pts = Crv2 -> Points,
        Interval1TMin = Interval -> T1Min,
        Interval1TMax = Interval -> T1Max,
        Interval2TMin = Interval -> T2Min,
        Interval2TMax = Interval -> T2Max,
        Dt1 = Interval1TMax - Interval1TMin,
        Dt2 = Interval2TMax - Interval2TMin;

    if (Dt1 <= 0.0 || Dt2 <= 0.0)
        return;
    else
        DtRatio = Dt2 / Dt1;

    if (CAGD_IS_BEZIER_CRV(Crv2))
        KV2 = BspKnotUniformOpen(Crv2 -> Length, Crv2 -> Order, NULL);
    else
        KV2 = Crv2 -> KnotVector;

    /* Find out the beginning of the range we need to process for m'th and   */
    /* n'th.  Note we start at IRIT_MAX(m, n) as we dont care if either is 0.*/
    for (IStart = 0; IStart < Eval1Len; IStart++) {
        if (Eval1[IStart].FirstBasisFuncIndex + Order1 > m)
	    break;
    }

    for (i = IStart; i < Eval1Len; i++) {
	/* Accumulate only evaluations that are in domain of intersection.  */
        if (Gauss1Pts[i] < Interval1TMin)
	    continue;
	else if (Gauss1Pts[i] > Interval1TMax)
	    break;

        m0 = m - Eval1[i].FirstBasisFuncIndex;
	
	/* Are we beyond the support of either basis function!? */
	if (m0 < 0)
	    break;
	assert(m0 >= 0 && m0 < Order1);

	/* Map the parameter value of Gauss1Pts[i] to second curve param    */
	/* and evaluate basis funcs of second curve at the mapped location. */
	t2 = Interval2TMin + DtRatio * (Gauss1Pts[i] - Interval1TMin);
	Eval2 = BspCrvCoxDeBoorBasis(KV2, Order2, Length2, FALSE, t2,
				     &Index2First);
	n0 = n - Index2First;
	if (n0 < 0 || n0 >= Order2)
	    continue; /* No overlap. */

#       ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpFuncValues, FALSE) {
	        if (i == IStart)
		    fprintf(stderr, "\n");
		fprintf(stderr,
			"At [%d] :: dPhi(%d)dx = %8.5f, dPhi(%d)dx = %8.5f\n",
			i, m, Eval1[i].BasisFuncsVals[m0],
			   n, Eval2[n0]);
	    }
	}
#       endif /* DEBUG */

	RetVal += Eval1[i].BasisFuncsVals[m0] * Eval2[n0] * Gauss1Weights[i];
    }

    /* Project the result onto the normal to the intersection. */
    C2 -> c[0] += RetVal * Interval -> ProjNrml[0];
    C2 -> c[1] += RetVal * Interval -> ProjNrml[1];

    if (CAGD_IS_BEZIER_CRV(Crv2))
        IritFree(KV2);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the elements of the C1 intersection matrix as the integral of  M
* the basis function's products along the intersecting boundary.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:    A boundary curve of Srf1 to determine its interesction interval M
*	     with Crv2.	 Has N control points.			             M
*   Srf1:    The first surface to examine for collision with Srf2.           M
*   Crv2:    A boundary curve of Srf2 to determine its interesction interval M
*	     with Crv1.	 Has M control points.			             M
*   Srf2:    The second surface to examine for collision with Srf1.          M
*   IntegRes:  Resolution of integration - number of sample per interval.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFECElementStruct *: Vector of size (N x M) elements of C2.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat, UserFEBuildC1Mat, UserFEEvalRHSC                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEBuildC2Mat                                                         M
*****************************************************************************/
UserFECElementStruct *UserFEBuildC2Mat(CagdCrvStruct *Crv1,
				       CagdSrfStruct *Srf1,
				       CagdCrvStruct *Crv2,
				       CagdSrfStruct *Srf2,
				       int IntegRes)
{
    int i, j, GaussSize,
        M = Crv1 -> Length,
	N = Crv2 -> Length;
    CagdRType *GaussPts, *GaussWeights, *KV1;
    UserFECElementStruct
        *C2 = (UserFECElementStruct *)
			   IritMalloc(sizeof(UserFECElementStruct) * M * N),
        *C2Ptr = C2;
    UserFEInterIntervalStruct *II,
        *InterIntervals = UserFEGetInterInterval(Crv1, Srf1, Crv2, Srf2);
    CagdBspBasisFuncEvalStruct *Evals;

    if (CAGD_IS_BEZIER_CRV(Crv1))
        KV1 = BspKnotUniformOpen(Crv1 -> Length, Crv1 -> Order, NULL);
    else
        KV1 = Crv1 -> KnotVector;
    GaussSize = UserFEGetGaussPts(KV1, Crv1 -> Order,
				  Crv1 -> Length, IntegRes, &GaussPts,
				  &GaussWeights);
    Evals = BspBasisFuncMultEval(KV1,
				 Crv1 -> Length + Crv1 -> Order,
				 Crv1 -> Order, FALSE, GaussPts, GaussSize,
				 CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++, C2Ptr++) {
	    C2Ptr -> c[0] = C2Ptr -> c[1] = 0.0;
	    for (II = InterIntervals; II != NULL; II = II -> Pnext) {
	        UserFEC2IntegrateOne(C2Ptr, II, Crv1, Crv2, Evals, GaussPts,
				     GaussWeights, GaussSize, i, j);
	    }
	}
    }

    /* Free all auxiliary data structures. */
    while (InterIntervals != NULL) {
        IRIT_LIST_POP(II, InterIntervals);
        IritFree(II);
    }
    BspBasisFuncMultEvalFree(Evals, GaussSize);
    IritFree(GaussPts);
    IritFree(GaussWeights);

    if (CAGD_IS_BEZIER_CRV(Crv1))
        IritFree(KV1);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpFEC2Matrix, TRUE) {
	    for (i = 0; i < M; i++) {
	        for (j = 0; j < N; j++) {
		    printf("%s%8.6f %8.6f",
			   j == 0 ? "" : "   ",
			   C2[j + i * M].c[0],
			   C2[j + i * M].c[1]);
		}
		printf("\n");
	    }
	    printf("\n\n");
	}
    }
#   endif /* DEBUG */

    return C2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the elements of the C2 intersection matrix as the integral of  M
* the basis function's products along the intersecting boundary.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Pts:     Crv1's control points.                                      M
*   Crv1Length:  Dimensions of Crv1Pts vector (M).			     M
*   Crv1Order:   Order of Crv1.						     M
*   Srf1Pts:     Srf1's control points.                                      M
*   Srf1ULength, Srf1VLength:  Dimensions of Srf1Pts vector.		     M
*   Srf1UOrder, Srf1VOrder:    Orders of Srf1.				     M
*   Crv2Pts:     Curve2's control points.                                    M
*   Crv2Length:  Dimensions of Crv2Pts vector (N).			     M
*   Crv2Order:   Order of Curve2.					     M
*   Srf2Pts:     Srf2's control points.                                      M
*   Srf2ULength, Srf2VLength:  Dimensions of Srf2Pts vector.		     M
*   Srf2UOrder, Srf2VOrder:    Orders of Srf2.				     M
*   EndCond:     Float or open, in reconstructed Crv1/2, Srf1/2.	     M
*   IntegRes:    Resolution of integration - number of sample per interval.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   UserFECElementStruct *: Vector of size (N x M) elements of C2.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat, UserFEBuildC2Mat, UserFEBuildC1Mat, UserFEEvalRHSC      M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEBuildC2Mat2                                                        M
*****************************************************************************/
UserFECElementStruct *UserFEBuildC2Mat2(CagdPType *Crv1Pts,
					int Crv1Length,
					int Crv1Order,
					CagdPType *Srf1Pts,
					int Srf1ULength,
					int Srf1VLength,
					int Srf1UOrder,
					int Srf1VOrder,
					CagdPType *Crv2Pts,
					int Crv2Length,
					int Crv2Order,
					CagdPType *Srf2Pts,
					int Srf2ULength,
					int Srf2VLength,
					int Srf2UOrder,
					int Srf2VOrder,
					CagdEndConditionType EndCond,
					int IntegRes)
{
    int i;
    CagdCrvStruct
        *Crv1 = BspCrvNew(Crv1Length, Crv1Order, CAGD_PT_E2_TYPE),
        *Crv2 = BspCrvNew(Crv2Length, Crv2Order, CAGD_PT_E2_TYPE);
    CagdSrfStruct
        *Srf1 = BspSrfNew(Srf1ULength, Srf1VLength, Srf1UOrder, Srf1VOrder,
			  CAGD_PT_E2_TYPE),
        *Srf2 = BspSrfNew(Srf2ULength, Srf2VLength, Srf2UOrder, Srf2VOrder,
			  CAGD_PT_E2_TYPE);
    UserFECElementStruct *C2;

    switch (EndCond) {
	case CAGD_END_COND_OPEN:
	    BspKnotUniformOpen(Crv1Length, Crv1Order, Crv1 -> KnotVector);
	    BspKnotUniformOpen(Crv2Length, Crv2Order, Crv2 -> KnotVector);
	    BspKnotUniformOpen(Srf1ULength, Srf1UOrder, Srf1 -> UKnotVector);
	    BspKnotUniformOpen(Srf1VLength, Srf1VOrder, Srf1 -> VKnotVector);
	    BspKnotUniformOpen(Srf2ULength, Srf2UOrder, Srf2 -> UKnotVector);
	    BspKnotUniformOpen(Srf2VLength, Srf2VOrder, Srf2 -> VKnotVector);
	    break;
        case CAGD_END_COND_FLOAT:
	default:
	    BspKnotUniformFloat(Crv1Length, Crv1Order, Crv1 -> KnotVector);
	    BspKnotUniformFloat(Crv2Length, Crv2Order, Crv2 -> KnotVector);
	    BspKnotUniformFloat(Srf1ULength, Srf1UOrder, Srf1 -> UKnotVector);
	    BspKnotUniformFloat(Srf1VLength, Srf1VOrder, Srf1 -> VKnotVector);
	    BspKnotUniformFloat(Srf2ULength, Srf2UOrder, Srf2 -> UKnotVector);
	    BspKnotUniformFloat(Srf2VLength, Srf2VOrder, Srf2 -> VKnotVector);
	    break;
    }

    for (i = 0; i < Crv1Length; i++)
        Crv1 -> Points[1][i] = Crv1Pts[i][0];
    for (i = 0; i < Crv2Length; i++)
        Crv2 -> Points[1][i] = Crv1Pts[i][0];

    for (i = 0; i < Srf1ULength * Srf1VLength; i++) {
        Srf1 -> Points[1][i] = Srf1Pts[i][0];
        Srf1 -> Points[2][i] = Srf1Pts[i][1];
    }
    for (i = 0; i < Srf2ULength * Srf2VLength; i++) {
        Srf2 -> Points[1][i] = Srf2Pts[i][0];
        Srf2 -> Points[2][i] = Srf2Pts[i][1];
    }

    C2 = UserFEBuildC2Mat(Crv1, Srf1, Crv2, Srf2, IntegRes);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return C2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the right hand side (RHS) of the i'th C constraint - of the    M
* i'th basis function.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   C:      The current row of C1 matrix of M elements. 		     M
*   Crv1:    A boundary curve of Srf1 to determine its interesction interval M
*	     with Crv2.  Has M control points.			             M
*   Crv2:    A boundary curve of Srf2 to determine its interesction interval M
*	     with Crv1.	 Has N control points.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Current value of constraint. Should be zero if satisified.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   UserFEBuildKMat, UserFEBuildC2Mat, UserFEBuildC1Mat                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserFEEvalRHSC                                                           M
*****************************************************************************/
IrtRType UserFEEvalRHSC(UserFECElementStruct *C,
			CagdCrvStruct *Crv1,
			CagdCrvStruct *Crv2)
{
    int i,
	M = Crv1 -> Length,
	N = Crv2 -> Length;
    CagdRType
	RetVal = 0.0,
	**Crv1Pts = Crv1 -> Points,
	**Crv2Pts = Crv2 -> Points;

    for (i = 0; i < M; i++, C++)
        RetVal += C -> c[0] * Crv1Pts[1][i] + C -> c[1] * Crv1Pts[2][i];
    for (i = 0; i < N; i++, C++)
        RetVal -= C -> c[0] * Crv2Pts[1][i] + C -> c[1] * Crv2Pts[2][i];

    return RetVal;
}

#ifdef DEBUG_TEST_FE_DATA

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the definite integral of scalar surface Srf over its domain.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   to compute its definite integral over its domain.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:                                                               *
*****************************************************************************/
static CagdRType UserFESrfIntegrate(CagdSrfStruct *Srf)
{
    CagdRType UMin, UMax, VMin, VMax, *R;
    CagdSrfStruct
	*Srf1 = CagdSrfIntegrate(Srf, CAGD_CONST_U_DIR),
	*Srf2 = CagdSrfIntegrate(Srf1, CAGD_CONST_V_DIR);

    CagdSrfDomain(Srf2, &UMin, &UMax, &VMin, &VMax);
    R = CagdSrfEval(Srf2, UMax, VMax);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    return R[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the inner product values of the given Bspline function space.   *
*   Computes entry ij as "Int( Phi(i) * Phi(j) )" analytically.	 This code   *
* is slow and should not be used other then for testing.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    To conisder its function space.                                  *
*   i, j:   Indices of two functions to compute the inner product for.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static UserFEKElementStruct *UserFEEvalInnerProduct(CagdSrfStruct *Srf,
						    int i,
						    int j)
{
    STATIC_DATA UserFEKElementStruct RetK;
    CagdSrfStruct *SrfI, *DuSrfI, *DvSrfI, *SrfJ, *DuSrfJ, *DvSrfJ, *TSrf;

    SrfI = CagdSrfCopy(Srf);
    SrfI -> Points[1][i] = 1.0;
    DuSrfI = CagdSrfDerive(SrfI, CAGD_CONST_U_DIR);
    DvSrfI = CagdSrfDerive(SrfI, CAGD_CONST_V_DIR);
    CagdSrfFree(SrfI);

    SrfJ = CagdSrfCopy(Srf);
    SrfJ -> Points[1][j] = 1.0;
    DuSrfJ = CagdSrfDerive(SrfJ, CAGD_CONST_U_DIR);
    DvSrfJ = CagdSrfDerive(SrfJ, CAGD_CONST_V_DIR);
    CagdSrfFree(SrfJ);

    TSrf = SymbSrfMult(DuSrfI, DuSrfJ);
    RetK.k[0][0] = UserFESrfIntegrate(TSrf);
    CagdSrfFree(TSrf);

    TSrf = SymbSrfMult(DuSrfI, DvSrfJ);
    RetK.k[1][0] = UserFESrfIntegrate(TSrf);
    CagdSrfFree(TSrf);

    TSrf = SymbSrfMult(DvSrfI, DuSrfJ);
    RetK.k[0][1] = UserFESrfIntegrate(TSrf);
    CagdSrfFree(TSrf);

    TSrf = SymbSrfMult(DvSrfI, DvSrfJ);
    RetK.k[1][1] = UserFESrfIntegrate(TSrf);
    CagdSrfFree(TSrf);

    CagdSrfFree(DuSrfI);
    CagdSrfFree(DvSrfI);
    CagdSrfFree(DuSrfJ);
    CagdSrfFree(DvSrfJ);

    return &RetK;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints the inner product values of the given Bspline function space.     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    To conisder its function space.                                  *
*   i, j:   Indices of two functions to compute the inner product for.       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserFEPrintInnerProducts(CagdSrfStruct *Srf)
{
    int i, j, l,
	Size = Srf -> ULength * Srf -> VLength;
    UserFEKElementStruct *K;

    /* Prepare a scalar zero-thoughout srf. */
    Srf = CagdCoerceSrfTo(Srf, CAGD_PT_E1_TYPE, FALSE);
    IRIT_ZAP_MEM(Srf -> Points[1], sizeof(CagdRType) * Size);

    printf("******************* Analytic K Evaluation *******************:\n");
    for (j = 0; j < Size; j++) {
        for (l = 0; l < 2; l++) {
	    for (i = 0; i < Size; i++) {
	        K = UserFEEvalInnerProduct(Srf, i, j);

	        printf("%s%8.5f %8.5f",
		       i == 0 ? "" : " | ", K -> k[l][0], K -> k[l][1]);
	    }
	    printf("\n");
	}
	for (i = 1; i < Size; i++)
	    printf("--------------------");
	printf("-----------------\n");
    }
    printf("\n");

    CagdSrfFree(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verifyes the inner product values of the given Bspline function space.   *
*                                                                            *
* PARAMETERS:                                                                *
*   KAll:   K Matrix to verify.                                              *
*   Srf:    For which K was computed.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UserFEVerifyInnerProducts(UserFEKElementStruct *KSrf,
				      CagdSrfStruct *Srf)
{
    int i, j, m, n,
	Size = Srf -> ULength * Srf -> VLength;
    UserFEKElementStruct *K;

    /* Prepare a scalar zero-thoughout srf. */
    Srf = CagdCoerceSrfTo(Srf, CAGD_PT_E1_TYPE, FALSE);
    IRIT_ZAP_MEM(Srf -> Points[1], sizeof(CagdRType) * Size);

    for (j = 0; j < Size; j++) {
        for (i = 0; i < Size; i++, KSrf++) {
	    K = UserFEEvalInnerProduct(Srf, i, j);

	    for (n = 0; n < 2; n++) {
	        for (m = 0; m < 2; m++) {
		    if (!IRIT_APX_EQ(KSrf -> k[m][n], K -> k[m][n]))
		        fprintf(stderr,
				"Error in K[%d][%d].k[%d][%d] :: %f vs. %f\n",
				i, j, m, n, KSrf -> k[m][n], K -> k[m][n]);

		}
	    }
	}
    }

    CagdSrfFree(Srf);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test routines for the FE code.			                     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   main                                                                     M
*****************************************************************************/
void main(void)
{
    int Size;
    CagdRType
        Refs[2] = { 1.0 / 3.0, 2.0 / 3.0 };
    CagdPType
        CtlPts[] = { { 0, 0, 0 },
		     { 1, 0, 0 },
		     { 0, 1, 0 },
		     { 1, 1, 0 } };
    CagdSrfStruct
        *Srf2 = CagdPrimPlaneSrf(0, 0, 1, 1, 0),
        *Srf3 = CagdSrfBlossomDegreeRaiseN(Srf2, 3, 3),
        *Srf4 = CagdSrfBlossomDegreeRaiseN(Srf3, 4, 4),
        *Srf5a = CagdSrfRefineAtParams(Srf4, CAGD_CONST_U_DIR, FALSE,
				       Refs, 2),
        *Srf5 = CagdSrfRefineAtParams(Srf5a, CAGD_CONST_V_DIR, FALSE,
				      Refs, 2);
    UserFEKElementStruct *K;
    UserFECElementStruct *C1, *C2;

    fprintf(stderr, "Bilinear evaluations...\n");
    K = UserFEBuildKMat(Srf2, 3, 1.0, 0.0, &Size);
    IritFree(K);
    K = UserFEBuildKMat(Srf2, 3, 1.0, 0.5, &Size);
    IritFree(K);
    K = UserFEBuildKMat(Srf2, 3, 1.0, 1.0, &Size);
    //UserFEPrintInnerProducts(Srf2);
    UserFEVerifyInnerProducts(K, Srf2);
    IritFree(K);

    /* Construct surface manually. */
    K = UserFEBuildKMat2(CtlPts, 2, 2, 2, 2, CAGD_END_COND_OPEN,
			 3, 1.0, 0.0, &Size);
    IritFree(K);
    K = UserFEBuildKMat2(CtlPts, 2, 2, 2, 2, CAGD_END_COND_OPEN,
			 3, 1.0, 1.0, &Size);
    //UserFEPrintInnerProducts(Srf2);
    UserFEVerifyInnerProducts(K, Srf2);
    IritFree(K);

    fprintf(stderr, "Biquadratic evaluations...\n");
    K = UserFEBuildKMat(Srf3, 3, 1.0, 0.0, &Size);
    IritFree(K);
    K = UserFEBuildKMat(Srf3, 3, 1.0, 1.0, &Size);
    //UserFEPrintInnerProducts(Srf3);
    UserFEVerifyInnerProducts(K, Srf3);
    IritFree(K);

    fprintf(stderr, "Bicubic evaluations...\n");
    K = UserFEBuildKMat(Srf4, 4, 1.0, 0.0, &Size);
    IritFree(K);
    K = UserFEBuildKMat(Srf4, 4, 1.0, 1.0, &Size);
    //UserFEPrintInnerProducts(Srf4);
    UserFEVerifyInnerProducts(K, Srf4);
    IritFree(K);

#ifdef FE_BI_QUARTIC /* Slow to test - a few seconds. */
    fprintf(stderr, "Biquartic evaluations...\n");
    K = UserFEBuildKMat(Srf5, 4, 1.0, 0.0, &Size);
    IritFree(K);
    K = UserFEBuildKMat(Srf5, 4, 1.0, 1.0, &Size);
    //UserFEPrintInnerProducts(Srf5);
    UserFEVerifyInnerProducts(K, Srf5);
    IritFree(K);
#endif /* FE_BI_QUARTIC */

#ifdef FE_2000_EVALS /* Slow to test - a few seconds. */
    {
        int i;

	fprintf(stderr, "Starting 2000 evaluations...");
	for (i = 0; i < 2000; i++) {
	    K = UserFEBuildKMat(Srf5, 4, 1.0, 0.0, &Size);
	    IritFree(K);
	}
	fprintf(stderr, "done\n");
    }
#endif /* FE_2000_EVALS */

    /* Build C1 tests. */
    {
        IrtHmgnMatType Mat;
        CagdCrvStruct *Crv4, *Crv4r;
	CagdSrfStruct *Srf4r;

	Srf4 -> Points[2][0] = 0.0001;
	Srf4 -> Points[2][1] = -1.0 / 6.0;
	Srf4 -> Points[2][2] = -1.0 / 6.0;
	Srf4 -> Points[2][3] = 0.0001;

	MatGenMatScale(1.0, -1.0, 1.0,  Mat);
	Srf4r = CagdSrfMatTransform(Srf4,  Mat);

	Crv4 = CagdCrvFromMesh(Srf4, 0, CAGD_CONST_V_DIR);
	Crv4r = CagdCrvFromMesh(Srf4r, 0, CAGD_CONST_V_DIR);

	fprintf(stderr, "C1 Bicubic evaluations...\n");
	C1 = UserFEBuildC1Mat(Crv4, Srf4, Crv4r, Srf4r, 4);
	IritFree(C1);

	fprintf(stderr, "C2 Bicubic evaluations...\n");
	C2 = UserFEBuildC2Mat(Crv4, Srf4, Crv4r, Srf4r, 4);
	IritFree(C2);

	CagdCrvFree(Crv4);
	CagdCrvFree(Crv4r);
	CagdSrfFree(Srf4r);
    }

    CagdSrfFree(Srf2);
    CagdSrfFree(Srf3);
    CagdSrfFree(Srf4);
    CagdSrfFree(Srf5a);
    CagdSrfFree(Srf5);
}

#endif /* DEBUG_TEST_FE_DATA */
