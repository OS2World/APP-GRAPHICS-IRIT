/******************************************************************************
* cbsp_fit.c - SDM fitting b-spline curve to cloud of points.                 *
* See also: Wenping Wang, Helmut Pottmann and Yang Liu,                       *
* "Fitting B-Spline curves to Point Clouds by Squared Distance Minimization"  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Yuri Pekelny, Oct. 2005.					      *
******************************************************************************/
#include "cagd_lib.h"

#ifdef BSP_CRV_FITTING_DEBUG
#include <stdio.h>
#endif

typedef CagdCrvStruct *(*InitFittingCrvCalculatorFuncType)(CagdPType *PtList,
                                                           int NumOfPoints,
                                                           int Length, 
                                                           int Order,
                                                           CagdBType Periodic);

/* Regulation term calculator. */
typedef CagdRType (*RegTermCalculatorFuncType)(CagdCrvStruct *Crv);

/* Regulation matrix calculator.                                          */
/* The caclulated matrix and vector should be added to the input A and b. */
typedef void (*RegMatrixCalculatorFuncType)(CagdCrvStruct *Crv,
                                            CagdRType *A,
                                            CagdRType *b,
                                            CagdRType lambda);

static void SDMatrixCalc(int Length,
                         int NumOfPoints,
                         CagdPType *Points,
                         CagdRType *Basis,
                         CagdPType *FootPoints,
                         CagdRType *Distances, 
                         CagdPType *Tangents, 
                         CagdPType *Normals, 
                         CagdRType *Curvatures,
                         CagdBType *IsOuter,
                         CagdRType *A,
                         CagdRType *b);
static CagdRType CagdPDError(const CagdPType Point, const CagdPType FootPoint);
static CagdRType CagdSDError(const CagdPType Point,
                             const CagdPType FootPoint,
                             const CagdRType Distance, 
                             const CagdPType Tangent, 
                             const CagdPType Normal, 
                             const CagdRType Curvature);
static CagdRType PDMErrorCalc(int NumOfPoints,
                              CagdPType *Points,
                              CagdPType *FootPoints);
static CagdRType SDMErrorCalc(int NumOfPoints,
                              CagdPType *Points,
                              CagdPType *FootPoints,
                              CagdRType *Distances, 
                              CagdPType *Tangents, 
                              CagdPType *Normals, 
                              CagdRType *Curvatures,
                              CagdBType *IsOuter);
static CagdRType SignedDistanceAux(CagdPType *Point, 
                                   CagdPType *FootPoint, 
                                   CagdPType *Normal);
static void FootPointsCalculatorAux(CagdCrvStruct *BspCrv,
                                    CagdPType *PtList,
                                    int NumOfPoints,
                                    CagdRType *Basis,
                                    CagdPType *FootPoints,
                                    CagdRType *Distances,
                                    CagdPType *Tangents,
                                    CagdPType *Normals,
                                    CagdRType *Curvatures,
                                    CagdBType *IsOuter);
static void MarkZeroLinesAux(CagdRType *A, 
                             CagdBType *ZeroLines, 
                             int n,
                             CagdRType Threshold);
static void ApplyZeroConstraintsAux(CagdRType *A, 
                                    CagdRType *b, 
                                    CagdBType *ZeroLines, 
                                    int n);
static void BringCloserNonMovedPointsAux(CagdBType *ZeroLines, 
                                         CagdCrvStruct *CurrCrv, 
                                         CagdRType *X);
static void UpdateCurveAux(CagdCrvStruct *CurrCrv, CagdRType *Deltas);
static CagdCrvStruct *CagdBspCrvSDMFitting(
                                  CagdPType *PtList,
                                  int NumOfPoints,
                                  CagdCrvStruct *InitCrv,
                                  RegTermCalculatorFuncType CalcRegularization,
                                  RegMatrixCalculatorFuncType CalcRegMatrix,
                                  int MaxIterations,
                                  CagdRType ErrorLimit,
                                  CagdRType ErrorChangeLimit,
                                  CagdRType Lambda);
static CagdCrvStruct *CagdBspCrvPDMFitting(
                                  CagdPType *PtList,
                                  int NumOfPoints,
                                  CagdCrvStruct *InitCrv,
                                  RegTermCalculatorFuncType CalcRegularization,
                                  RegMatrixCalculatorFuncType CalcRegMatrix,
                                  int MaxIterations,
                                  CagdRType ErrorLimit,
                                  CagdRType ErrorChangeLimit,
                                  CagdRType Lambda);
static CagdCrvStruct *LeastSquareInitCrvCalculator(CagdPType *PtList, 
                                                   int NumOfPoints,
                                                   int Length, 
                                                   int Order,
                                                   CagdBType Periodic);
static CagdCrvStruct *BBoxPerimeterInitCrvCalculator(CagdPType *PtList, 
                                                     int NumOfPoints,
                                                     int Length, 
                                                     int Order,
                                                     CagdBType Periodic);
static void PDMatrixCalc(int Length,
                         int NumOfPoints,
                         CagdPType *Points,
                         CagdRType *Basis,
                         CagdPType *FootPoints,
                         CagdRType *A,
                         CagdRType *b);
static CagdRType Energy2Calc(CagdCrvStruct *Crv);
static void Energy2MatrixCalc(CagdCrvStruct *Crv,
                              CagdRType *A,
                              CagdRType *b,
                              CagdRType Lambda);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates b-spline curve that fits (approximates) the given points      M
*   cloud using SD error function and Energy2 regulation function.           M
*   There are three stop conditions:                                         M
*    1) Maximum iterations.                                                  M
*    2) Error = 0 (i.e. curve passes through all the points)                 M
*    3) Error change = 0 (the iteration gives no improvement)                M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       Points cloud we want to approximate.                       M
*   NumOfPoints:  Number of points in PtList.                                M
*   InitCrv:      Initial fitting curve.                                     M
*   AgorithmType: Fitting algorithm type (CAGD_PDM_FITTING,                  M
*                 CAGD_SDM_FITTING, etc).                                    M
*   MaxIter:      Maximum iterations to perform (stop condition).            M
*   ErrorLimit:   Minimum error (stop condition).                            M
*   ErrorChangeLimit: Minimum error change (stop condition).                 M
*   Lambda:       Weight of regularization term.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: Output b-spline curve.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBsplineCrvFittingWithInitCrv                                         M
*****************************************************************************/
CagdCrvStruct *CagdBsplineCrvFittingWithInitCrv(CagdPType *PtList,
                                                int NumOfPoints,
                                                CagdCrvStruct *InitCrv,
                                                CagdBspFittingType AgorithmType,
                                                int MaxIter,
                                                CagdRType ErrorLimit,
                                                CagdRType ErrorChangeLimit,
                                                CagdRType Lambda)
{
    switch (AgorithmType) {
        case CAGD_PDM_FITTING:
            return CagdBspCrvPDMFitting(PtList,
                                        NumOfPoints,
                                        InitCrv,
                                        Energy2Calc,
                                        Energy2MatrixCalc,
                                        MaxIter,
                                        ErrorLimit,
                                        ErrorChangeLimit,
                                        Lambda);
            break;
        case CAGD_SDM_FITTING:
            return CagdBspCrvSDMFitting(PtList,
                                        NumOfPoints,
                                        InitCrv,
                                        Energy2Calc,
                                        Energy2MatrixCalc,
                                        MaxIter,
                                        ErrorLimit,
                                        ErrorChangeLimit,
                                        Lambda);
            break;
        default:
            return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates b-spline curve that fits (approximates) the given points      M
*   cloud using SD error function and Energy2 regulation function.           M
*   There are three stop conditions:                                         M
*    1) 100 iterations                                                       M
*    2) Error = 0.1                                                          M
*    3) Error change = 0.005                                                 M
*   The initial curve is a least square approximating b-spline.              M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       Points cloud we want to approximate.                       M
*   NumOfPoints:  Number of points in PtList.                                M
*   Length:       The desired length of the output b-spline curve.           M
*   Order:        The desired order of the output b-spline curve.            M
*   IsPeriodic:   TRUE for periodic output curve, FALSE for open end.        M
*   AgorithmType: Fitting algorithm type (CAGD_PDM_FITTING,                  M
*                 CAGD_SDM_FITTING, etc).                                    M
*   MaxIter:      Maximum iterations to perform (stop condition).            M
*   ErrorLimit:   Minimum error (stop condition).                            M
*   ErrorChangeLimit: Minimum error change (stop condition).                 M
*   Lambda:       Weight of regularization term.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: Output b-spline curve.                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBsplineCrvFitting                                                    M
*****************************************************************************/
CagdCrvStruct *CagdBsplineCrvFitting(CagdPType *PtList,
                                     int NumOfPoints,
                                     int Length, 
	                             int Order,
                                     CagdBType IsPeriodic,
                                     CagdBspFittingType AgorithmType,
                                     int MaxIter,
                                     CagdRType ErrorLimit,
                                     CagdRType ErrorChangeLimit,
                                     CagdRType Lambda)
{
    CagdCrvStruct 
        *FittingCrv = NULL, 
        *InitCrv = NULL;

    if (IsPeriodic)
        InitCrv = BBoxPerimeterInitCrvCalculator(PtList, NumOfPoints, 
                                                 Length, Order, IsPeriodic);
    else 
        InitCrv = LeastSquareInitCrvCalculator(PtList, NumOfPoints, 
                                               Length, Order, IsPeriodic);

    if (InitCrv != NULL) {
        switch (AgorithmType) {
            case CAGD_PDM_FITTING:
                FittingCrv = CagdBspCrvPDMFitting(PtList,
                                                  NumOfPoints,
                                                  InitCrv,
                                                  Energy2Calc,
                                                  Energy2MatrixCalc,
                                                  MaxIter,
                                                  ErrorLimit,
                                                  ErrorChangeLimit,
                                                  Lambda);
                break;
            case CAGD_SDM_FITTING:
                FittingCrv = CagdBspCrvSDMFitting(PtList,
                                                  NumOfPoints,
                                                  InitCrv,
                                                  Energy2Calc,
                                                  Energy2MatrixCalc,
                                                  MaxIter,
                                                  ErrorLimit,
                                                  ErrorChangeLimit,
                                                  Lambda);
                break;
        }
        CagdCrvFree(InitCrv);
    }
    
    return FittingCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the SD error minimization equation Matrix and                 M
* offset vector, i.e. A and b of the Ax=b equation.                          M
*   The caluclated values are ADDED to the A and b parameters                M
*   The order of variables in x is assumed to be:                            M
*   (D1_x,D2_x,...,DLength_x,D1_y,D2_y,...,DLength_y)                        M
*   NOTE:  The size of each array except Basis must be 'NumOfPoints'.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:      Fitting curve Length.                                       M
*   NumOfPoints: Number of points in the points cloud.                       M
*   Points:      Points cloud. Array of points with size = NumOfPoints.      M
*   Basis:       Array of size (NumOfPoints * Length) containing basis       M
*                function coefficients at the foot points                    M
*   FootPoints:  Footpoints (the closest points on the curve) array.         M
*   Distances:   Array of distances between each point to the                M
*                corresponding footpoint.                                    M
*   Tangents:    Array of curve tangents at each footpoint.                  M
*   Normals:     Array of curve normals at each footpoint.                   M
*   Curvatures:  Array of curve curvature radiuses at each footpoint.        M
*   IsOuter:     Array of booleans that indicates outer points.              M
*   A:           Input/output matrix (2Length * 2Length).                    M
*   b:           Input/output offset vector (2Length * 1).                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SDMatrixCalc, SD error                                                   M
*****************************************************************************/
static void SDMatrixCalc(int Length,
                         int NumOfPoints,
                         CagdPType *Points,
                         CagdRType *Basis,
                         CagdPType *FootPoints,
                         CagdRType *Distances, 
                         CagdPType *Tangents, 
                         CagdPType *Normals, 
                         CagdRType *Curvatures,
                         CagdBType *IsOuter,
                         CagdRType *A,
                         CagdRType *b)
{
    int i = 0, 
        j = 0, 
        k = 0;
    CagdRType NkxNkx, NkxNky, NkyNky, CPVdotNk, DkDkmCk,
        CosTetta, Bij, BijNkxNkx, BijNkxNky, BijNkyNky,
        DkCPVdotTkDkmCk = -IRIT_INFNTY,
	DkTkxTkxDkmCk = -IRIT_INFNTY,
	DkTkxTkyDkmCk = -IRIT_INFNTY,
	DkTkyTkyDkmCk = -IRIT_INFNTY;
    CagdPType 
        CurrPointVec;

    for (k = 0; k < NumOfPoints; ++k) {
        NkxNkx = Normals[k][0] * Normals[k][0];
        NkxNky = Normals[k][0] * Normals[k][1];
        NkyNky = Normals[k][1] * Normals[k][1];
        IRIT_VEC_SUB(CurrPointVec, FootPoints[k], Points[k]);
        CPVdotNk = IRIT_DOT_PROD(CurrPointVec, Normals[k]);
        if (Distances[k] < 0) {
            if (Curvatures[k] != 0)
                DkDkmCk = Distances[k] / (Distances[k] - Curvatures[k]);
            else
                DkDkmCk = 0;
            DkTkxTkxDkmCk = Tangents[k][0] * Tangents[k][0] * DkDkmCk;
            DkTkxTkyDkmCk = Tangents[k][0] * Tangents[k][1] * DkDkmCk;
            DkTkyTkyDkmCk = Tangents[k][1] * Tangents[k][1] * DkDkmCk;
            DkCPVdotTkDkmCk = IRIT_DOT_PROD(CurrPointVec, Tangents[k]) * DkDkmCk;
        }
        if (IsOuter[k]) {
            IRIT_VEC_SAFE_NORMALIZE(CurrPointVec);
            CosTetta = IRIT_FABS(IRIT_DOT_PROD(CurrPointVec ,Tangents[k]));
        }
        else {
            CosTetta = 0;
        }
        for (i = 0; i < Length; ++i) {
            for (j = 0; j < Length; ++j) {
                int /* Index of Dj,x from d(eSD,k(D))/d(Di,x) in A. */
                    IndexDixDjx = (i * 2 * Length) + j,
                    /* Index of Dj,y from d(eSD,k(D))/d(Di,x) in A. */
                    IndexDixDjy = (i * 2 * Length) + Length + j,
                    /* Index of Dj,x from d(eSD,k(D))/d(Di,y) in A. */
                    IndexDiyDjx = ((Length + i) * 2 * Length) + j,
                    /* Index of Dj,y from d(eSD,k(D))/d(Di,y) in A. */
                    IndexDiyDjy = ((Length + i) * 2 * Length) + Length + j;

                Bij = Basis[k * Length + i] * Basis[k * Length + j];
                
                BijNkxNkx = Bij * NkxNkx;       /* Bi(tk)*(Nk,x^2)*Bj(tk). */
                BijNkxNky = Bij * NkxNky;    /* Bi(tk)*(Nk,x*Nk,y)*Bj(tk). */
                BijNkyNky = Bij * NkyNky;       /* Bi(tk)*(Nk,y^2)*Bj(tk). */
                A[IndexDixDjx] += (1 - CosTetta) * BijNkxNkx + CosTetta * Bij;
                A[IndexDixDjy] += (1 - CosTetta) * BijNkxNky;
                A[IndexDiyDjx] += (1 - CosTetta) * BijNkxNky;
                A[IndexDiyDjy] += (1 - CosTetta) * BijNkyNky + CosTetta * Bij;

                if (Distances[k] < 0) {
                    /* d/(d-p)*Bi(tk)*(Tk,x^2)*Bj(tk). */
                    A[IndexDixDjx] += (1 - CosTetta) * Bij * DkTkxTkxDkmCk;
                    /* d/(d-p)*Bi(tk)*(Tk,x*Tk,y)*Bj(tk). */
                    A[IndexDixDjy] += (1 - CosTetta) * Bij * DkTkxTkyDkmCk;
                    /* d/(d-p)*Bi(tk)*(Tk,x^Nk,y)*Bj(tk). */
                    A[IndexDiyDjx] += (1 - CosTetta) * Bij * DkTkxTkyDkmCk;
                    /* d/(d-p)*Bi(tk)*(Tk,y^2)*Bj(tk). */
                    A[IndexDiyDjy] += (1 - CosTetta) * Bij * DkTkyTkyDkmCk;
                }
            }
        }

        /* Constants */
        for (i = 0; i < Length; ++i) {
            int /* Index of constant from d(eSD,k(D))/d(Di,x) in b. */
                IndexDix = i,
                /* Index of constant from d(eSD,k(D))/d(Di,y) in b. */
                IndexDiy = Length + i;

            /* -Bi(tk)*(Nk,x)*(P(tk)-Xk)'*Nk. */
            b[IndexDix] -= (1 - CosTetta) * 
                Basis[k * Length + i] * Normals[k][0] * CPVdotNk+ 
                CosTetta * 
                Basis[k * Length + i] * (FootPoints[k][0] - Points[k][0]);

            /* -Bi(tk)*(Nk,y)*(P(tk)-Xk)'*Nk. */
            b[IndexDiy] -= (1 - CosTetta) * 
                Basis[k * Length + i] * Normals[k][1] * CPVdotNk + 
                CosTetta * 
                Basis[k * Length + i] * (FootPoints[k][1] - Points[k][1]);

            if (Distances[k] < 0) {
                /* -d/(d-p)*Bi(tk)*(Tk,x)*(P(tk)-Xk)'*Tk. */
                b[IndexDix] -= (1 - CosTetta) * 
                    Basis[k * Length + i] * Tangents[k][0] * DkCPVdotTkDkmCk;

                /* -d/(d-p)*Bi(tk)*(Tk,y)*(P(tk)-Xk)'*Tk. */
                b[IndexDiy] -= (1 - CosTetta) * 
                    Basis[k * Length + i] * Tangents[k][1] * DkCPVdotTkDkmCk;
            }
        }
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes PD (Point Distance) error for a single point, which is:           M
*                                                                            *
*                         2                                                  V
*   e      =  ||P(tk)-Xk||                                                   V
*    PD,k                                                                    V
*                                                                            *
* PARAMETERS:                                                                M
*   Point:      (Xk)   Points to calculate the error for.                    M
*   FootPoint: (P(tk)) Footpoint (the closest point to Xk on the curve)      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: PD error.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPDError                                                              M
*****************************************************************************/
static CagdRType CagdPDError(const CagdPType Point, const CagdPType FootPoint)
{
    CagdPType CurrPointVec;

    /* (P(tk)-Xk). */
    IRIT_VEC_SUB(CurrPointVec, FootPoint, Point);

    /* ||P(tk)-Xk||^2. */
    return IRIT_DOT_PROD(CurrPointVec, CurrPointVec);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes SD (Squared Distance) error for a single point, which is:         M
*                                                                            *
*              /   dk             T     2               T       2            V
*              |  -----[(P(tk)-Xk) * Tk]  +  [(P(tk)-Xk)  *  Nk]  ,if dk<0   V
*   e      =  <   dk-pk                                                      V
*    SD,k      |               T     2                                       V
*              |  [(P(tk) - Xk] * Ni]                         ,if 0=<dk<pk   V
*              \                                                             V
*                                                                            *
* PARAMETERS:                                                                M
*   Point:      (Xk)   Points to calculate the error for.                    M
*   FootPoint: (P(tk)) Footpoint (the closest point to Xk on the curve)      M
*   Distance:   (dk)   Distance between Point and FootPoint                  M
*   Tangent:    (Tk)   Curve tangent at Footpoint.                           M
*   Normal:     (Nk)   Curve normal at Footpoint.                            M
*   Curvature:  (pk)   Curve curvature at Footpoint.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: SD error.                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSDError                                                              M
*****************************************************************************/
static CagdRType CagdSDError(const CagdPType Point,
                             const CagdPType FootPoint,
                             const CagdRType Distance, 
                             const CagdPType Tangent, 
                             const CagdPType Normal, 
                             const CagdRType Curvature)
{
    CagdRType 
        Error = 0;
    CagdPType CurrPointVec;

    /* (P(tk)-Xk). */
    IRIT_VEC_SUB(CurrPointVec, FootPoint, Point);

    /* [(P(tk)-Xk)'*Nk]^2. */
    Error += IRIT_SQR(IRIT_DOT_PROD(CurrPointVec, Normal));
    if (Distance < 0) {
        /* d/(d-p)*[(P(tk)-Xk)'*Tk]^2. */
        Error += Distance * 
            IRIT_SQR(IRIT_DOT_PROD(CurrPointVec, Tangent)) /
            (Distance - Curvature);
    }
    return Error;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes PD Minimization method error, which is:                           M
*                                                                            *
*   NumOfPoints                                                              V
*      ----                                                                  V
*   1  \                                                                     V
*   -  /     e                                                               V
*   2  ----   PD,k                                                           V
*       k=1                                                                  V
*                         2                                                  V
*   e      =  ||P(tk)-Xk||                                                   V
*    PD,k                                                                    V
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfPoints:     Number of points in the points cloud.                   M
*   Points:      (X) Points cloud. Array of points with size = NumOfPoints.  M
*   FootPoints: (P(t)) Footpoints (the closest points on the curve) array.   M
*                    Array size must be 'NumOfPoints'.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: PD error.                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSDError                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   PDMErrorCalc                                                             M
*****************************************************************************/
static CagdRType PDMErrorCalc(int NumOfPoints,
                              CagdPType *Points,
                              CagdPType *FootPoints)
{
    int k = 0;
    CagdRType
	Error = 0;

    for (k = 0; k < NumOfPoints; ++k) {
        Error += CagdPDError(Points[k], FootPoints[k]);
    }

    return 0.5 * Error;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes SD Minimization method error, which is:                           M
*                                                                            *
*   NumOfPoints                                                              V
*      ----                                                                  V
*   1  \                                                                     V
*   -  /     e                                                               V
*   2  ----   SD,k                                                           V
*       k=1                                                                  V
*              /   dk             T     2               T       2            V
*              |  -----[(P(tk)-Xk) * Tk]  +  [(P(tk)-Xk)  *  Nk]  ,if dk<0   V
*   e      =  <   dk-pk                                                      V
*    SD,k      |              T     2                                        V
*              |  [(P(tk) - Xk] * Ni]                         ,if 0=<dk<pk   V
*              \                                                             V
*                                                                            M
*   NOTE:  The size of each array must be 'NumOfPoints'.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   NumOfPoints: Number of points in the points cloud.                       M
*   Points:      (X) Points cloud. Array of points with size = NumOfPoints.  M
*   FootPoints:  (P(t)) Footpoints (the closest points on the curve) array.  M
*   Distances:   (d) Array of distances between each point to the            M
*                corresponding footpoint.                                    M
*   Tangents:    (T) Array of curve tangents at each footpoint.              M
*   Normals:     (N) Array of curve normals at each footpoint.               M
*   Curvatures:  (p) Array of curve curvature radiuses at each footpoint.    M
*   IsOuter:     Array of booleans that indicates outer points.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: SD error.                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSDError                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SDMErrorCalc                                                             M
*****************************************************************************/
static CagdRType SDMErrorCalc(int NumOfPoints,
                              CagdPType *Points,
                              CagdPType *FootPoints,
                              CagdRType *Distances, 
                              CagdPType *Tangents, 
                              CagdPType *Normals, 
                              CagdRType *Curvatures,
                              CagdBType *IsOuter)
{
    int k = 0;
    CagdRType 
        Error = 0,
        SDError = 0,
        PDError = 0,
        CosTetta = 0;
    CagdPType CurrPointVec;

    for (k = 0; k < NumOfPoints; ++k) {
        SDError = CagdSDError(Points[k], FootPoints[k], Distances[k],
                              Tangents[k], Normals[k], Curvatures[k]); 
        if (IsOuter[k]) {
            PDError = CagdPDError(Points[k], FootPoints[k]);
            IRIT_VEC_SUB(CurrPointVec, FootPoints[k], Points[k]);
            IRIT_VEC_SAFE_NORMALIZE(CurrPointVec);
            CosTetta = IRIT_FABS(IRIT_DOT_PROD(CurrPointVec ,Tangents[k]));
            Error += (CosTetta * PDError + (1 - CosTetta) * SDError);
        }
        else {
            Error += SDError;
        }
    }
    return 0.5 * Error;
}

/*****************************************************************************
* AUXILIARY:                                                                 M
* Auxiliary function for FootPointsCalculatorAux.                            M
*                                                                            *
* DESCRIPTION:                                                               M
*   Calculates signed distance from FootPoint to Point.                      M
*   The distance is positive iff the Point is on the side of the normal      M
*   at FootPoint.                                                            M
*   The function assumes that the points and the normal are in the XY plane. M
*                                                                            *
* PARAMETERS:                                                                M
*   Point:     A point.                                                      M
*   FootPoint: A closet point on a curve to Point.                           M
*   Normal:    Curve's normal at FootPoint                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  Signed distance from FootPoint to point.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   FootPointsCalculatorAux, CagdBspCrvSDMFitting                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SignedDistanceAux                                                        M
*****************************************************************************/
static CagdRType SignedDistanceAux(CagdPType *Point, 
                                   CagdPType *FootPoint, 
                                   CagdPType *Normal)
{
    int sign = 1;
    CagdRType 
        d = IRIT_PT_PT_DIST(*Point, *FootPoint);
    CagdPType PointVec;

    if (d == 0)
        return 0;

    /* Find the sign from dot product with the normal. */
    IRIT_VEC_SUB(PointVec, *Point, *FootPoint);
    if (IRIT_DOT_PROD_2D(PointVec, *Normal) < 0)
        sign = -1;
    return d * sign;
}

/*****************************************************************************
* AUXILIARY:                                                                 M
* Auxiliary function for CagdBspCrvSDMFitting.                               M
*                                                                            *
* DESCRIPTION:                                                               M
*   Calculates for each point in the input list its footpoint (the closest   M
*   point on the input curve). For each footpoint the function calculates    M
*   its distance from the point, its basis function values, tangent,         M
*   normal and curvature radius and the indicator if the closest point       M
*   is the curve endpoint (IsOuter).                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   BspCrv:      Input b-spline curve.                                       M
*   PtList:      Input points array.                                         M
*   NumOfPoints: Number of points in PtList                                  M
*   Basis:       Initialized array of size (NumOfPoints * BspCrv -> Length)  M
*                or NULL if there is no need to calculate basis function     M
*                values.                                                     M
*   FootPoints, Distances, Tangents, Normals, Curvatures, IsOuter:           M
*                Initialized arrays with of size (NumOfPoints)               M
*                or NULL if there is no need to calculate the values.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvSDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   FootPointsCalculatorAux                                                  M
*****************************************************************************/
static void FootPointsCalculatorAux(CagdCrvStruct *BspCrv,
                                    CagdPType *PtList,
                                    int NumOfPoints,
                                    CagdRType *Basis,     /* NULL - no fill. */
                                    CagdPType *FootPoints,/* NULL - no fill. */
                                    CagdRType *Distances, /* NULL - no fill. */
                                    CagdPType *Tangents,  /* NULL - no fill. */
                                    CagdPType *Normals,   /* NULL - no fill. */
                                    CagdRType *Curvatures,/* NULL - no fill. */
                                    CagdBType *IsOuter)   /* NULL - no fill. */
{
    int k, j, IndexFirst;
    CagdBType IsPeriodic;
    CagdRType TMin, TMax, *CurrEval1, *B;
    CagdPType CurrFootPoint, CurrNormal;
    CagdVecStruct *Vec;
    CagdVType TVec, NVec;
    CagdCrvStruct 
        *CurvatureSqr = NULL,
        *OpenEndCrv = BspCrv;

    if ((Basis == NULL) && (FootPoints == NULL) && (Distances == NULL) &&
        (Tangents == NULL) && (Normals == NULL) && (Curvatures == NULL) &&
        (IsOuter == NULL))                           /* Nothing to compute. */
        return;                                       

    IsPeriodic = CAGD_IS_PERIODIC_CRV(BspCrv);
    if (IsPeriodic) {
        OpenEndCrv = CagdCrvCopy(BspCrv);
        OpenEndCrv = CagdCnvrtPeriodic2FloatCrv(OpenEndCrv);
        OpenEndCrv = CagdCnvrtFloat2OpenCrv(OpenEndCrv);
    }

    /* Prepare for the multiple tangent/normal/curvature calculations. */
    if ((Normals != NULL) || (Tangents != NULL) || 
        (Distances != NULL) || (Curvatures != NULL))
        SymbEvalCrvCurvPrep(OpenEndCrv, TRUE);

    /* Prepare for Basis calculation. */
    if (Basis != NULL) {
        IRIT_ZAP_MEM(Basis, sizeof(CagdRType) * NumOfPoints * BspCrv -> Length);
    }

    /* Prepare for IsOuter calculation. */
    if (IsOuter != NULL) {
        CagdCrvDomain(BspCrv, &TMin, &TMax);
    }

    for (k = 0; k < NumOfPoints; ++k) {
        /* Calculate the nearest point (footpoint). */
        int MultInterpFlag = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
        CagdRType 
            t = SymbDistCrvPoint(OpenEndCrv, PtList[k], TRUE, 1e-7);

        BspMultComputationMethod(MultInterpFlag);

        if ((FootPoints != NULL) || (Distances != NULL)) {
            CurrEval1 = CagdCrvEval(BspCrv, t);
            IRIT_PT_SET(CurrFootPoint, CurrEval1[1], CurrEval1[2], CurrEval1[3]);
        }
        if (FootPoints != NULL) {
            IRIT_VEC_COPY(FootPoints[k], CurrFootPoint);
        }

        /* Calculate curvature. */
        if (Curvatures != NULL) {
            SymbEvalCrvCurvature(OpenEndCrv, t, &(Curvatures[k]));
            if (Curvatures[k] != 0)
                Curvatures[k] = 1.0/Curvatures[k];
            else
                Curvatures[k] = IRIT_INFNTY;
        }

        /* Calculate normal. */
        if ((Normals != NULL) || (Tangents != NULL) || (Distances != NULL)) {
            SymbEvalCrvCurvTN(NVec, TVec, FALSE);
            IRIT_VEC_SAFE_NORMALIZE(TVec);
            if ((NVec[0] == 0) && (NVec[1] == 0)) {
                /* SymbEvalCrvCurvTN failed to compute the normal */
                Vec = CagdCrvNormalXY(BspCrv, t, FALSE);
                IRIT_VEC_SAFE_NORMALIZE(Vec -> Vec);
                IRIT_PT_SET(CurrNormal, Vec -> Vec[0], Vec -> Vec[1], Vec -> Vec[2]);
            }
            else {
                IRIT_VEC_SAFE_NORMALIZE(NVec);
                IRIT_VEC_COPY(CurrNormal, NVec);
            }
        }
        if (Normals != NULL) {
            IRIT_VEC_COPY(Normals[k], CurrNormal);
        }

        /* Calculate signed distance. */
        if (Distances != NULL) {
            Distances[k] = 
                SignedDistanceAux(&PtList[k], &CurrFootPoint, &CurrNormal);
        }

        /* Calculate tangent. */
        if (Tangents != NULL) {
            IRIT_VEC_COPY(Tangents[k], TVec);
        }

        /* Calculate basis function values at t. */
        if (Basis != NULL) {
            B = BspCrvCoxDeBoorBasis(BspCrv -> KnotVector, BspCrv -> Order, 
                                    BspCrv -> Length, BspCrv -> Periodic, 
                                    t, &IndexFirst);
            for (j = 0; j<BspCrv -> Order; ++j) {
                Basis[k * BspCrv -> Length + 
                      ((j + IndexFirst) % BspCrv -> Length)] = B[j];
            }
        }

        /* Check if outer point (the footpoint is one of curve endpoints). */
        if (IsOuter != NULL) {
            IsOuter[k] = (!IsPeriodic) && ((t == TMin) || (t == TMax));
        }

#ifdef BSP_CRV_FITTING_DEBUG
        IRIT_INFO_MSG_PRINTF("#%d:%c=%.3f ", k, IsOuter != NULL && IsOuter[k]?'T':'t', t);
        if (FootPoints != NULL)
            IRIT_INFO_MSG_PRINTF("P(t)=(%.3f,%.3f) ",
				    FootPoints[k][0], FootPoints[k][1]);
        if (Distances != NULL)
            IRIT_INFO_MSG_PRINTF("d=%.3f ", Distances[k]);
        if (Tangents != NULL)
            IRIT_INFO_MSG_PRINTF("T=(%.3f,%.3f) ",
				    Tangents[k][0], Tangents[k][1]);
        if (Normals != NULL)
           IRIT_INFO_MSG_PRINTF("N=(%.3f,%.3f) ",
				    Normals[k][0], Normals[k][1]);
        if (Curvatures != NULL)
            IRIT_INFO_MSG_PRINTF("p=%.3f", Curvatures[k]);
        IRIT_INFO_MSG("\n");
#endif
    }

    if (CurvatureSqr != NULL)
        CagdCrvFree(CurvatureSqr);
    if (OpenEndCrv != BspCrv)
        CagdCrvFree(OpenEndCrv);
}

/*****************************************************************************
* AUXILIARY:                                                                 M
* Auxiliary function for CagdBspCrvSDMFitting.                               M
*                                                                            *
* DESCRIPTION:                                                               M
*   Finds all lines in the input matrix 'A' which are close to zero.         M
*   For each line in 'A' with all values smaller that 'Threshold'            M
*   (the absolute value), the value of 'ZeroLines' is set TRUE.              M
*                                                                            *
* PARAMETERS:                                                                M
*   A:          Input matrix n*n.                                            M
*   ZeroLines:  Output array n*1.                                            M
*   n:          A and ZeroLines size.                                        M
*   Threshold:  Threshold value. values smaller that Threshold are           M
*               considered as zeros,                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvSDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MarkZeroLinesAux                                                         M
*****************************************************************************/
static void MarkZeroLinesAux(CagdRType *A, 
                             CagdBType *ZeroLines, 
                             int n, 
                             CagdRType Threshold)
{
    int i, j;

    IRIT_ZAP_MEM(ZeroLines, sizeof(CagdBType) * n);
    for (i = 0; i < n; ++i) {
        CagdBType AllZeros = TRUE;
        for (j = 0; (AllZeros) && (j < n); ++j)
            if (IRIT_FABS(A[i * n + j]) > Threshold)
                AllZeros = FALSE;
        ZeroLines[i] = AllZeros;
    }
}

/*****************************************************************************
* AUXILIARY:                                                                 M
* Auxiliary function for CagdBspCrvSDMFitting.                               M
*                                                                            *
* DESCRIPTION:                                                               M
*   Receives 'A' and 'b' from the Ax=b equation and applies zero             M
*   constraints for all variables marked TRUE in 'ZeroLines'.                M
*   For instance, if ZeroLines[i] == TRUE then b[i] will be set to 0,        M
*   A's ith row and column will be set to zeros and A(i,i) will be set       M
*   to 1. This will result in x[i] == 0 in the equation solution.            M
*                                                                            *
* PARAMETERS:                                                                M
*   A:          Input/output matrix n*n.                                     M
*   b:          Input/output contrants vector n*1.                           M
*   ZeroLines:  Input vector n*1 with TRUE values for each variable which    M
*               solution should be 0.                                        M
*   n:          A, b and ZeroLines size.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvSDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   ApplyZeroConstraintsAux                                                  M
*****************************************************************************/
static void ApplyZeroConstraintsAux(CagdRType *A, 
                                    CagdRType *b, 
                                    CagdBType *ZeroLines, 
                                    int n)
{
    int i, j;

    for (i = 0; i < n; ++i) {
        if (ZeroLines[i]) {
#ifdef BSP_CRV_FITTING_DEBUG
            if (i < n/2)
                IRIT_INFO_MSG_PRINTF("Zero constraint was applied on Dx%d\n", i);
            else
                IRIT_INFO_MSG_PRINTF(
                        "Zero constraint was applied on Dy%d\n", i - (n / 2));
#endif
            b[i] = 0;
            for (j = 0; j < n; ++j) {
                A[i * n + j] = 0;
                A[j * n + i] = 0;
            }
            A[i * n + i] = 1;
        }
    }
}

/*****************************************************************************
* AUXILIARY:                                                                 M
* Auxiliary function for CagdBspCrvSDMFitting.                               M
*                                                                            *
* DESCRIPTION:                                                               M
*   This function changes the deltas 'X' vector so that the control          M
*   points which were forced not to move would move closer to the            M
*   rest of the curve control points.                                        M
*   ZeroLines indicates which 'X' deltas were forced to be 0.                M
*   The deltas are changed only if both X and Y deltas of a point were       M
*   forced to be 0.  These points are moved according to the following       M
*   algorithm:                                                               M
*   1) Inner control points that were forced not to move:                    M
*     The deltas for each such point are changed so that the point would     M
*     move half way towards the line connecting the nearest right and left   M
*     neighbour points that weren't forced not to move.                      M
*   2) Endpoints that were forced not to move:                               M
*     The deltas for each such point are changed so that the point would     M
*     move half the way to the nearest control point that wasn't forced      M
*     not to move.                                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   ZeroLines:  Input vector 2*CurrCrv->Length x 1 with TRUE values for      M
*               each X variable which solution was forced to be 0.           M
*   CurrCrv:    Current curve (before adding 'X' deltas).                    M
*   X:          Control points deltas, that would be added to CurrCrv's      M
*               control points. Input/output 2*CurrCrv->Length x 1 vector.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvSDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BringCloserNonMovedPointsAux                                             M
*****************************************************************************/
static void BringCloserNonMovedPointsAux(CagdBType *ZeroLines, 
                                         CagdCrvStruct *CurrCrv, 
                                         CagdRType *X)
{
    int i, j, prev, next,
        Length = CurrCrv -> Length,
        *PrevNotForced = NULL,
        *NextNotForced = NULL;
    CagdBType 
        IsPeriodic = CAGD_IS_PERIODIC_CRV(CurrCrv),
        AllZeros = TRUE,
        AllNonZeros = TRUE;
    CagdRType 
        *CrvPointsX = CurrCrv -> Points[1],
        *CrvPointsY = CurrCrv -> Points[2];

    /* Check that not all points were forced not to move. */
    for (i = 0; (AllZeros || AllNonZeros) && (i < 2 * Length); ++i) {
        AllZeros &= ZeroLines[i];
        AllNonZeros &= !ZeroLines[i];
    }
    if (AllZeros || AllNonZeros)
        return; /* There is nothing we can do. */

    PrevNotForced = IritMalloc(sizeof(int) * Length);
    NextNotForced = IritMalloc(sizeof(int) * Length);

    /*   Find the previous and next not forced control points.        */
    /*   For not forced points, the previous and the next will be     */
    /*   equal to themselves.                                         */
    /*   For forced startpoints, the prevous will be -1.              */
    /*   For forced endpoints, the next will be Length.               */
    prev = -1;
    for (i = 0; i <= Length - 1; ++i) {
        if (!ZeroLines[i] || !ZeroLines[i + Length]) {
            prev = i;
        }
        PrevNotForced[i] = prev;
    }
    next = Length;
    for (i = Length - 1; i >= 0; --i) {
        if (!ZeroLines[i] || !ZeroLines[i + Length]) {
            next = i;
        }
        NextNotForced[i] = next;
    }

    /* For periodic curves, replace -1 and Length with the nearest  */
    /* periodic neighbours.                                         */
    if (IsPeriodic) {
        for (i = 0; PrevNotForced[i] == -1; ++i) {
            PrevNotForced[i] = PrevNotForced[Length - 1];
        }
        for (i = Length - 1; PrevNotForced[i] == Length; --i) {
            NextNotForced[i] = NextNotForced[0];
        }
    }

    /* Moving the non-moved control points closer to the rest of the curve. */
    for (i = 0; i <= Length - 1; ++i) {
        if (ZeroLines[i] && ZeroLines[i + Length]) {
            /* i-th control point was forced not to move. */

            if (PrevNotForced[i] == - 1) {
                /* All starting lines are zeros. */
                CagdRType XMove = (i + 1 == NextNotForced[i] ?
                    (CrvPointsX[i + 1] + X[i + 1] - CrvPointsX[i])/2 :
                    (CrvPointsX[i + 1] - CrvPointsX[i])/2);
                CagdRType YMove = (i + 1 == NextNotForced[i] ?
                    (CrvPointsY[i + 1] + X[i + Length + 1] - CrvPointsY[i])/2 :
                    (CrvPointsY[i + 1] - CrvPointsY[i])/2);
                for (j = 0; j <= i; ++j) {
                    X[j] += XMove;
                    X[j + Length] += YMove;
                }
            } 
            else if (NextNotForced[i] == Length) {
                /* All ending lines are zeros. */
                CagdRType XMove = (i - 1 == PrevNotForced[i] ?
                    (CrvPointsX[i - 1] + X[i - 1] - CrvPointsX[i])/2 :
                    (CrvPointsX[i - 1] - CrvPointsX[i])/2);
                CagdRType YMove = (i - 1 == PrevNotForced[i] ?
                    (CrvPointsY[i - 1] + X[i + Length - 1] - CrvPointsY[i])/2 :
                    (CrvPointsY[i - 1] - CrvPointsY[i])/2);
                for (j = i; j <= Length - 1; ++j) {
                    X[j] += XMove;
                    X[j + Length] += YMove;
                }
            } 
            else {
                int NumOfNotMovedPoints = 
                    NextNotForced[i] - PrevNotForced[i] - 1;
                int currPointNum = i - PrevNotForced[i];
                CagdRType XTarget, YTarget;

                if (NumOfNotMovedPoints < 0) {             /* Periodic case. */
                    NumOfNotMovedPoints += Length;
                }
                if (currPointNum < 0) {                    /* Periodic case. */
                    currPointNum += Length;
                }

                /* Calculate target point: */
                /* Prev + ((Next - Prev) / numOfPoints) * currPointNum. */
                XTarget = 
                    CrvPointsX[PrevNotForced[i]] + X[PrevNotForced[i]] + 
                   (CrvPointsX[NextNotForced[i]] + X[NextNotForced[i]] - 
                    CrvPointsX[PrevNotForced[i]] - X[PrevNotForced[i]]) *
                    currPointNum / (NumOfNotMovedPoints + 1);
                YTarget = 
                    CrvPointsY[PrevNotForced[i]] + X[PrevNotForced[i] + Length] + 
                   (CrvPointsY[NextNotForced[i]] + X[NextNotForced[i] + Length] - 
                    CrvPointsY[PrevNotForced[i]] - X[PrevNotForced[i] + Length]) *
                    currPointNum / (NumOfNotMovedPoints + 1);
                /* Move half way to the target point. */
                X[i] = (XTarget - CrvPointsX[i])/2;
                X[i + Length] = (YTarget - CrvPointsY[i])/2;
            }
        }
    }

    IritFree(PrevNotForced);
    IritFree(NextNotForced);
}

/*****************************************************************************
* AUXILIARY:                                                                 M
* Auxiliary function for CagdBspCrvSDMFitting.                               M
*                                                                            *
* DESCRIPTION:                                                               M
*   Updates b-spline curve according to the control points deltas vector.    M
*                                                                            *
* PARAMETERS:                                                                M
*   CurrCrv:     B-spline curve to update.                                   M
*   Deltas:      Control points deltas vector.                               M
*                Must be a vector of size 2 * CurrCrv -> Length with the     M
*                following deltas order:                                     M
*                (D1_x,D2_x,...,DLength_x,D1_y,D2_y,...,DLength_y)           M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvSDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UpdateCurveAux                                                           M
*****************************************************************************/
static void UpdateCurveAux(CagdCrvStruct *CurrCrv, CagdRType *Deltas)
{
    int i,
        Length = CurrCrv -> Length;

    for (i = 0; i < Length; ++i) {
        CurrCrv -> Points[1][i] += Deltas[i];              /* Add X deltas. */
        CurrCrv -> Points[2][i] += Deltas[Length + i];     /* Add Y deltas. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates b-spline curve that fits (approximates) the given points      M
*   Using input init curve, SDM fitting method and specified regularization  M
*   term.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:        Points cloud we want to approximate.                      M
*   NumOfPoints:   Number of points in PtList.                               M
*   InitCrv:       Pointer to an initial fitting curve.                      M
*                  Its order and length will be taken as                     M
*                  order and length of the desired fitting curve.            M
*   CalcRegularization: Pointer to a function that should be used for        M
*                  calculating regularization term value.                    M
*   CalcRegMatrix: Pointer to a function that should be used for             M
*                  calculating regularization term minimization matrix.      M
*   MaxIterations: Maximum iterations to perform (stop condition).           M
*   ErrorLimit:    Minimum error (stop condition).                           M
*   ErrorChangeLimit: Minimum error change (stop condition).                 M
*   Lambda:        Weight of regularization term.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: B-spline curve that fits the input points cloud.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvPDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBspCrvSDMFitting, points cloud approximization                       M
*****************************************************************************/
static CagdCrvStruct *CagdBspCrvSDMFitting(
                                  CagdPType *PtList,
                                  int NumOfPoints,
                                  CagdCrvStruct *InitCrv,
                                  RegTermCalculatorFuncType CalcRegularization,
                                  RegMatrixCalculatorFuncType CalcRegMatrix,
                                  int MaxIterations,
                                  CagdRType ErrorLimit,
                                  CagdRType ErrorChangeLimit,
                                  CagdRType Lambda)
{
    int i = 0,
#ifdef BSP_CRV_FITTING_DEBUG
        i2 = 0, 
        j2 = 0,
#endif
        Length = InitCrv -> Length,
        n = 2 * Length;
    CagdBType 
        *IsOuter = (CagdBType*) IritMalloc(sizeof(CagdBType) * NumOfPoints),
        *ZeroLines = (CagdBType*) IritMalloc(sizeof(CagdBType) * n);
    CagdRType 
        *Distances  = (CagdRType*) IritMalloc(sizeof(CagdRType) * NumOfPoints),
        *Curvatures = (CagdRType*) IritMalloc(sizeof(CagdRType) * NumOfPoints),
        *Basis = (CagdRType*) IritMalloc(sizeof(CagdRType) * 
					 NumOfPoints * Length),
        *A = (CagdRType*) IritMalloc(sizeof(CagdRType) * n * n),
        *b = (CagdRType*) IritMalloc(sizeof(CagdRType) * n),
        *X = (CagdRType*) IritMalloc(sizeof(CagdRType) * n),
        CurrError = 0.0,
        PrevError = IRIT_INFNTY;
    CagdPType 
        *FootPoints = (CagdPType*) IritMalloc(sizeof(CagdPType) * NumOfPoints),
        *Tangents   = (CagdPType*) IritMalloc(sizeof(CagdPType) * NumOfPoints),
        *Normals    = (CagdPType*) IritMalloc(sizeof(CagdPType) * NumOfPoints);
    CagdCrvStruct 
        *CurrCrv = CagdCrvCopy(InitCrv);

    do {
        if (i > 0) {
            IRIT_ZAP_MEM(A, sizeof(CagdRType) * n * n);
            IRIT_ZAP_MEM(b, sizeof(CagdRType) * n);
            IRIT_ZAP_MEM(X, sizeof(CagdRType) * n);

            SDMatrixCalc(Length, NumOfPoints, PtList, Basis, FootPoints, 
                         Distances, Tangents, Normals, Curvatures, IsOuter, 
                         A, b);

#ifdef BSP_CRV_FITTING_DEBUG		
            IRIT_INFO_MSG("Basis=\n");
            for (i2 = 0; i2 < NumOfPoints; ++i2) {
                IRIT_INFO_MSG("(");
                for (j2 = 0; j2 < Length; ++j2)
                    IRIT_INFO_MSG_PRINTF(
                        Basis[i2 * Length + j2] != 0 ? "%.03f " : "0 ", 
                        Basis[i2 * Length + j2]);
                IRIT_INFO_MSG(")\n");
            }

            IRIT_INFO_MSG("A");
            for (i2 = 0; i2 < n; ++i2) {
                IRIT_INFO_MSG("(");
                for (j2 = 0; j2 < n; ++j2)
                    IRIT_INFO_MSG_PRINTF(A[i2 * n + j2] != 0 ? "%5.02f" : " 0   ",
					    A[i2 * n + j2]);
                IRIT_INFO_MSG(")\n");
            }
            IRIT_INFO_MSG("b(");
            for (i2 = 0; i2 < n; ++i2)
                IRIT_INFO_MSG_PRINTF(b[i2] != 0 ? "%4.02f " : " 0   ",
					b[i2]);
            IRIT_INFO_MSG(")\n");
#endif
            MarkZeroLinesAux(A, ZeroLines, n, 0.2);

            if (Lambda != 0)
                (*CalcRegMatrix)(CurrCrv, A, b, Lambda);

            ApplyZeroConstraintsAux(A, b, ZeroLines, n);

#ifdef BSP_CRV_FITTING_DEBUG
            IRIT_INFO_MSG("A'");
            for (i2 = 0; i2 < n; ++i2) {
                IRIT_INFO_MSG("(");
                for (j2 = 0; j2 < n; ++j2)
                    IRIT_INFO_MSG_PRINTF(A[i2 * n + j2] != 0 ? "%5.02f" : " 0   ",
					    A[i2 * n + j2]);
                IRIT_INFO_MSG(")\n");
            }
            IRIT_INFO_MSG("b'(");
            for (i2 = 0; i2 < n; ++i2)
                IRIT_INFO_MSG_PRINTF(b[i2] != 0 ? "%4.02f " : " 0   ",
					b[i2]);
            IRIT_INFO_MSG(")\n");
#endif
            IritQRUnderdetermined(A, NULL, NULL, n, n);
            IritQRUnderdetermined(NULL, X, b, n, n);

            BringCloserNonMovedPointsAux(ZeroLines, CurrCrv, X);

#ifdef BSP_CRV_FITTING_DEBUG		
            IRIT_INFO_MSG("D=");
            for (i2 = 0; i2 < Length; ++i2) {
                IRIT_INFO_MSG_PRINTF(ZeroLines[i2] ? "(%f*" : "(%f", X[i2]);
                IRIT_INFO_MSG_PRINTF(ZeroLines[i2 + Length] ? ",%f*)\n" : ",%f)\n",
					X[i2 + Length]);
            }
#endif
            UpdateCurveAux(CurrCrv, X);
        } /* if (i > 0) */

        if (i++ >= MaxIterations)
            break;

        FootPointsCalculatorAux(CurrCrv, PtList, NumOfPoints, Basis, 
            FootPoints, Distances, Tangents, Normals, Curvatures, IsOuter);


        PrevError = CurrError;
        CurrError = 
            (SDMErrorCalc(NumOfPoints, PtList, FootPoints, Distances,
                Tangents, Normals, Curvatures, IsOuter) +
             (Lambda != 0 ? Lambda * (*CalcRegularization)(CurrCrv) : 0)) 
             / NumOfPoints;

        IRIT_INFO_MSG_PRINTF("Iteration %d, Average error per point = %f.\n", i, CurrError);
    }
    while ((CurrError > ErrorLimit) &&
	   ((i == 1) || (IRIT_FABS(PrevError - CurrError) > ErrorChangeLimit)));

    IritFree(FootPoints);
    IritFree(Distances);
    IritFree(Tangents);
    IritFree(Normals);
    IritFree(Curvatures);
    IritFree(IsOuter);
    IritFree(A);
    IritFree(b);

    return CurrCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates b-spline curve that fits (approximates) the given points      M
*   Using input init curve, PDM fitting method and specified regularization  M
*   term.                                                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:        Points cloud we want to approximate.                      M
*   NumOfPoints:   Number of points in PtList.                               M
*   InitCrv:       Pointer to an initial fitting curve.                      M
*                  Its order and length will be taken as                     M
*                  order and length of the desired fitting curve.            M
*   CalcRegularization: Pointer to a function that should be used for        M
*                  calculating regularization term value.                    M
*   CalcRegMatrix: Pointer to a function that should be used for             M
*                  calculating regularization term minimization matrix.      M
*   MaxIterations: Maximum iterations to perform (stop condition).           M
*   ErrorLimit:    Minimum error (stop condition).                           M
*   ErrorChangeLimit: Minimum error change (stop condition).                 M
*   Lambda:        Weight of regularization term.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: B-spline curve that fits the input points cloud.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBspCrvSDMFitting                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBspCrvPDMFitting, points cloud approximization                       M
*****************************************************************************/
static CagdCrvStruct *CagdBspCrvPDMFitting(
                                  CagdPType *PtList,
                                  int NumOfPoints,
                                  CagdCrvStruct *InitCrv,
                                  RegTermCalculatorFuncType CalcRegularization,
                                  RegMatrixCalculatorFuncType CalcRegMatrix,
                                  int MaxIterations,
                                  CagdRType ErrorLimit,
                                  CagdRType ErrorChangeLimit,
                                  CagdRType Lambda)
{
    int i = 0,
#ifdef BSP_CRV_FITTING_DEBUG
        i2 = 0, 
        j2 = 0,
#endif
        Length = InitCrv -> Length,
        n = 2 * Length;
    CagdBType 
        *ZeroLines = (CagdBType*) IritMalloc(sizeof(CagdBType) * n);
    CagdRType 
        *Basis = (CagdRType*) IritMalloc(sizeof(CagdRType) * 
                                         NumOfPoints * Length),
        *A = (CagdRType*) IritMalloc(sizeof(CagdRType) * n * n),
        *b = (CagdRType*) IritMalloc(sizeof(CagdRType) * n),
        *X = (CagdRType*) IritMalloc(sizeof(CagdRType) * n),
        CurrError = 0.0,
        PrevError = IRIT_INFNTY;
    CagdPType 
        *FootPoints = (CagdPType*)IritMalloc(sizeof(CagdPType) * NumOfPoints);
    CagdCrvStruct 
        *CurrCrv = CagdCrvCopy(InitCrv);

    do {
        if (i > 0) {
            IRIT_ZAP_MEM(A, sizeof(CagdRType) * n * n);
            IRIT_ZAP_MEM(b, sizeof(CagdRType) * n);
            IRIT_ZAP_MEM(X, sizeof(CagdRType) * n);

            PDMatrixCalc(Length, NumOfPoints, PtList, Basis, FootPoints, A, b);

#ifdef BSP_CRV_FITTING_DEBUG		
            IRIT_INFO_MSG("Basis=\n");
            for (i2 = 0; i2 < NumOfPoints; ++i2) {
                IRIT_INFO_MSG("(");
                for (j2 = 0; j2 < Length; ++j2)
                    IRIT_INFO_MSG_PRINTF(
                        Basis[i2 * Length + j2] != 0 ? "%.03f " : "0 ", 
                        Basis[i2 * Length + j2]);
                IRIT_INFO_MSG(")\n");
            }

            IRIT_INFO_MSG("A");
            for (i2 = 0; i2 < n; ++i2) {
                IRIT_INFO_MSG("(");
                for (j2 = 0; j2 < n; ++j2)
                    IRIT_INFO_MSG_PRINTF(A[i2 * n + j2] != 0 ? "%5.02f" : " 0   ",
					    A[i2 * n + j2]);
                IRIT_INFO_MSG(")\n");
            }
            IRIT_INFO_MSG("b(");
            for (i2 = 0; i2 < n; ++i2)
                IRIT_INFO_MSG_PRINTF(b[i2] != 0 ? "%4.02f " : " 0   ",
					b[i2]);
            IRIT_INFO_MSG(")\n");
#endif
            MarkZeroLinesAux(A, ZeroLines, n, 2);

            if (Lambda != 0)
                (*CalcRegMatrix)(CurrCrv, A, b, Lambda);

            ApplyZeroConstraintsAux(A, b, ZeroLines, n);

#ifdef BSP_CRV_FITTING_DEBUG
            IRIT_INFO_MSG("A'");
            for (i2 = 0; i2 < n; ++i2) {
                IRIT_INFO_MSG("(");
                for (j2 = 0; j2 < n; ++j2)
                    IRIT_INFO_MSG_PRINTF(A[i2 * n + j2] != 0 ? "%5.02f" : " 0   ",
					 A[i2 * n + j2]);
                IRIT_INFO_MSG(")\n");
            }
            IRIT_INFO_MSG_PRINTF("b'(");
            for (i2 = 0; i2 < n; ++i2)
                IRIT_INFO_MSG_PRINTF(b[i2] != 0 ? "%4.02f " : " 0   ",
				     b[i2]);
            IRIT_INFO_MSG(")\n");
#endif
            IritQRUnderdetermined(A, NULL, NULL, n, n);
            IritQRUnderdetermined(NULL, X, b, n, n);

            BringCloserNonMovedPointsAux(ZeroLines, CurrCrv, X);

#ifdef BSP_CRV_FITTING_DEBUG		
            IRIT_INFO_MSG("D=");
            for (i2 = 0; i2 < Length; ++i2) {
                IRIT_INFO_MSG_PRINTF(ZeroLines[i2] ? "(%f*" : "(%f", X[i2]);
                IRIT_INFO_MSG_PRINTF(ZeroLines[i2 + Length] ? ",%f*)\n" : ",%f)\n", X[i2 + Length]);
            }
#endif
            UpdateCurveAux(CurrCrv, X);
        } /* if (i > 0) */

        if (i++ >= MaxIterations)
            break;

        FootPointsCalculatorAux(CurrCrv, PtList, NumOfPoints, Basis, 
            FootPoints, NULL, NULL, NULL, NULL, NULL);


        PrevError = CurrError;
        CurrError = 
            (PDMErrorCalc(NumOfPoints, PtList, FootPoints) +
             (Lambda != 0 ? Lambda * (*CalcRegularization)(CurrCrv) : 0)) 
            / NumOfPoints;

        IRIT_INFO_MSG_PRINTF("Iteration %d, Average error per point = %f.\n", i, CurrError);
    }
    while ((CurrError > ErrorLimit) &&
	   ((i == 1) || (IRIT_FABS(PrevError - CurrError) > ErrorChangeLimit)));

    IritFree(FootPoints);
    IritFree(A);
    IritFree(b);

    return CurrCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Computes an initial b-spline fitting curve that least square              M
*  approximates the input points.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:        Points cloud.                                             M
*   NumOfPoints:   Number of points in PtList.                               M
*   Length:        The desired length of the output b-spline curve.          M
*   Order:         The desired order of the output b-spline curve.           M
*   Periodic:      TRUE for periodic output curve, FALSE for open end.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: The calculated b-spline curve.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   LeastSquareInitCrvCalculator, InitFittingCrvCalculatorFuncType           M
*****************************************************************************/
static CagdCrvStruct *LeastSquareInitCrvCalculator(CagdPType *PtList, 
                                                   int NumOfPoints,
                                                   int Length, 
                                                   int Order,
                                                   CagdBType Periodic)
{
    int i;
    CagdPtStruct 
        *Pts = NULL;
    CagdCrvStruct *InitCrv;

    for (i = 0; i < NumOfPoints; i++) {
        CagdPtStruct
            *Pt = CagdPtNew();
        IRIT_LIST_PUSH(Pt, Pts);
        Pt -> Pt[0] = PtList[i][0];
        Pt -> Pt[1] = PtList[i][1];
        Pt -> Pt[2] = PtList[i][2];
    }
    InitCrv = BspCrvInterpPts(Pts, Order, Length, CAGD_UNIFORM_PARAM, Periodic);
    CagdPtFreeList(Pts);

    return InitCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an initial b-spline fitting curve which control points lies on    M
* a perimeter of the points cloud bounding box                               M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:        Points cloud.                                             M
*   NumOfPoints:   Number of points in PtList.                               M
*   Length:        The desired length of the output b-spline curve.          M
*   Order:         The desired order of the output b-spline curve.           M
*   Periodic:      TRUE for periodic output curve, FALSE for open end.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: B-spline curve with control points equally spread on    M
*                    a perimeter of the points cloud bounding box.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BBoxPerimeterInitCrvCalculator, InitFittingCrvCalculatorFuncType,        M
*   bounding box diagonal                                                    M
*****************************************************************************/
static CagdCrvStruct *BBoxPerimeterInitCrvCalculator(CagdPType *PtList, 
                                                     int NumOfPoints,
                                                     int Length, 
                                                     int Order,
                                                     CagdBType Periodic)
{
    int i, Axis;
    CagdRType XLength, YLength, PerimeterLength, SegmentLength, CurrPos, Coof,
              *PointsCloud[4];
    CagdBBoxStruct 
        *BBox = CagdBBoxNew();
    CagdCrvStruct 
        *NewCrv = BspPeriodicCrvNew(Length, Order, Periodic, CAGD_PT_E2_TYPE);

    if (NumOfPoints < 1)
        IRIT_FATAL_ERROR("Points list should contain at least 1 point.");

    /* Create uniform knot vector. */
    if (Periodic)
        BspKnotUniformPeriodic(Length, Order, NewCrv -> KnotVector);
    else
        BspKnotUniformOpen(Length, Order, NewCrv -> KnotVector);

    /* Calculate top-left and bottom-right points of the points cloud bbox. */
    PointsCloud[0] = NULL; /* No weights */
    for (Axis = 1; Axis <= 3; ++Axis) {
        PointsCloud[Axis] = 
            (CagdRType *) IritMalloc(sizeof(CagdRType) * NumOfPoints);
    }
    for (i = 0; i < NumOfPoints; ++i) {
        for (Axis = 1; Axis <= 3; ++Axis) {
            PointsCloud[Axis][i] = PtList[i][Axis - 1];
        }
    }
    CagdPointsBBox(PointsCloud, NumOfPoints, 3, BBox -> Min, BBox -> Max);

    /* Spread the control points on the bounding box perimeter.   */
    /* The first point is the left bottom corner, the rest of the */
    /* points are spread clockwise:                               */
    /*      2>                                                    */
    /*  ^ +---+                                                   */
    /*  1 |   | 3                                                 */
    /*    *---+ v                                                 */
    /*     <4                                                     */
    XLength = BBox -> Max[0] - BBox -> Min[0];
    YLength = BBox -> Max[1] - BBox -> Min[1];
    PerimeterLength = (XLength + YLength) * 2;
    SegmentLength = PerimeterLength / Length;
    for (CurrPos = 0, i = 0; i < Length; CurrPos += SegmentLength, ++i) {
        if (CurrPos < XLength + YLength) {
            if (CurrPos < YLength){                         /* 1:left side   */
                Coof = CurrPos / YLength;
                NewCrv -> Points[1][i] = BBox -> Min[0];
                NewCrv -> Points[2][i] = BBox -> Min[1] + 
                    (BBox -> Max[1] - BBox -> Min[1]) * Coof;
            }
            else {                                          /* 2:top side    */
                Coof = (CurrPos - YLength) / XLength;
                NewCrv -> Points[1][i] = BBox -> Min[0] +
                    (BBox -> Max[0] - BBox -> Min[0]) * Coof;
                NewCrv -> Points[2][i] = BBox -> Max[1];
            }
        }
        else {
            if (CurrPos < XLength + YLength * 2) {          /* 3:right side  */
                Coof = (CurrPos - XLength - YLength) / YLength;
                NewCrv -> Points[1][i] = BBox -> Max[0];
                NewCrv -> Points[2][i] = BBox -> Max[1] -
                    (BBox -> Max[1] - BBox -> Min[1]) * Coof;
            }
            else {                                          /* 4:bottom side */
                Coof = (CurrPos - XLength - YLength * 2) / XLength;
                NewCrv -> Points[1][i] = BBox -> Max[0] -
                    (BBox -> Max[0] - BBox -> Min[0]) * Coof;
                NewCrv -> Points[2][i] = BBox -> Min[1];
            }
        }
    }
    
    IritFree(BBox);
    for (Axis = 1; Axis <= 3; ++Axis) {
        IritFree(PointsCloud[Axis]);
    }
    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the PD error minimization equation Matrix and                 M
*   offset vector, i.e. A and b of the Ax=b equation.                        M
*   The caluclated values are ADDED to the A and b parameters                M
*   The order of variables in x is assumed to be:                            M
*   (D1_x,D2_x,...,DLength_x,D1_y,D2_y,...,DLength_y)                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Length:      Fitting curve Length.                                       M
*   NumOfPoints: Number of points in the points cloud.                       M
*   Points:      Points cloud. Array of points with size = NumOfPoints.      M
*   Basis:       Array of size (NumOfPoints * Length) containing basis       M
*                function coefficients at the foot points                    M
*   FootPoints:  Footpoints (the closest points on the curve) array          M
*                of size NumOfPoints.                                        M
*   A:           Input/output matrix (2Length * 2Length).                    M
*   b:           Input/output offset vector (2Length * 1).                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PDMatrixCalc, PD error                                                   M
*****************************************************************************/
static void PDMatrixCalc(int Length,
                         int NumOfPoints,
                         CagdPType *Points,
                         CagdRType *Basis,
                         CagdPType *FootPoints,
                         CagdRType *A, /* MATRIX */
                         CagdRType *b)
{
    int i = 0, 
        j = 0, 
        k = 0;

    for (k = 0; k < NumOfPoints; ++k) {
        for (i = 0; i < Length; ++i) {
            for (j = 0; j < Length; ++j) {
                int 
                /* index of Dj,x from d(eSD,k(D))/d(Di,x) in A. */
                    IndexDixDjx = (i * 2 * Length) + j,
                /* index of Dj,y from d(eSD,k(D))/d(Di,y) in A. */
                    IndexDiyDjy = ((Length + i) * 2 * Length) + Length + j;
                CagdRType 
                    Bij = Basis[k * Length + i] * Basis[k * Length + j];
                
                A[IndexDixDjx] += Bij;
                A[IndexDiyDjy] += Bij;

            }
        }

        /* Constants */
        for (i = 0; i < Length; ++i) {
            int 
            /* index of constant from d(eSD,k(D))/d(Di,x) in b. */
                IndexDix = i,
            /* index of constant from d(eSD,k(D))/d(Di,y) in b. */
                IndexDiy = Length + i;

            b[IndexDix] -= 
                Basis[k * Length + i] * (FootPoints[k][0] - Points[k][0]);
            b[IndexDiy] -=
                Basis[k * Length + i] * (FootPoints[k][1] - Points[k][1]);
        }
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the curve second derivative energy integral:                  M
*    /             2                                                         V
*    | ||Crv''(t)||  dt                                                      V
*    /                                                                       V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to derive and integrate.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: The caluculated integral value.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   Energy2Calc, RegTermCalculatorFuncType                                   M
*****************************************************************************/
static CagdRType Energy2Calc(CagdCrvStruct *Crv)
{
    CagdRType *Energy;
    CagdCrvStruct 
        *CrvD, *CrvDD, *CrvDD2, *IntegrCrvDD2,
        *OpenEndCrv = Crv;

    if (CAGD_IS_PERIODIC_CRV(Crv)) {
        OpenEndCrv = CagdCrvCopy(Crv);
        OpenEndCrv = CagdCnvrtPeriodic2FloatCrv(OpenEndCrv);
        OpenEndCrv = CagdCnvrtFloat2OpenCrv(OpenEndCrv);
    }
    CrvD = CagdCrvDerive(OpenEndCrv);
    CrvDD = CagdCrvDerive(CrvD);
    CrvDD2 = SymbCrvDotProd(CrvDD, CrvDD);
    IntegrCrvDD2 = CagdCrvIntegrate(CrvDD2);
    Energy = CagdCrvEval(IntegrCrvDD2, 1.0);

    CagdCrvFree(CrvD);
    CagdCrvFree(CrvDD);
    CagdCrvFree(CrvDD2);
    CagdCrvFree(IntegrCrvDD2);
    if (Crv != OpenEndCrv)
        CagdCrvFree(OpenEndCrv);

    return Energy[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the curve second derivative energy (see Energy2Calc)          M
*   minimization matrix.                                                     M
*   The calculated coefficients are ADDED to A and b.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Input b-spline curve.                                         M
*   A:         Input/output initialized matrix,                              M
*              which size = 2 * Crv -> Length x 2 * Crv -> Length.           M
*   b:         Input/output initialized offset vector,                       M
*              which size = 2 * Crv -> Length x 1.                           M
*   Lambda:    Weight.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Energy2MatrixCalc, RegMatrixCalculatorFuncType                           M
*****************************************************************************/
static void Energy2MatrixCalc(CagdCrvStruct *Crv,
                              CagdRType *A,
                              CagdRType *b,
                              CagdRType Lambda)
{
    int i, j, n, k;
    CagdRType 
        *KV, *a, *c, **IProds;
    CagdCrvStruct 
        *CrvD, *CrvDD;
    
    if (Lambda == 0)
        return; /* The calculations are redandant. */

    n = Crv -> Length;
    k = Crv -> Order;
    if (k <= 2)
        IRIT_FATAL_ERROR("Curve order must be > 2.");

    KV = Crv -> KnotVector;
    CrvD = CagdCrvDerive(Crv);
    CrvDD = CagdCrvDerive(CrvD);
    IProds = SymbBspBasisInnerProdMat(CrvDD -> KnotVector, 
                                      CrvDD -> Order + CrvDD -> Length, 
                                      CrvDD -> Order, 
                                      CrvDD -> Order);
    /* Calculate a and c vectors. */
    a = (CagdRType*)IritMalloc(sizeof(CagdRType) * n);
    c = (CagdRType*)IritMalloc(sizeof(CagdRType) * n);
    IRIT_ZAP_MEM(a, sizeof(CagdRType) * n);
    IRIT_ZAP_MEM(c, sizeof(CagdRType) * n);
    if (CAGD_IS_PERIODIC_CRV(Crv)) {
        for (i = 0; i < n; ++i) {
            CagdRType 
                tmp = KV[i + k] - KV[i + 2];
            if ((tmp != 0) && (KV[(i + k + 1)] != KV[i + 2])) {
                a[i] = 1.0 / (tmp * (KV[i + k + 1] - KV[i + 2]));
            }
            if ((tmp != 0) && (KV[i + k] != KV[i + 1])) {
                c[i] = 1.0 / (tmp * (KV[i + k] - KV[i + 1]));
            }
        }
        for (i = 0; i < n; ++i) {
            for (j = 0; j < n; ++j) {
                /* Index of Dj,x in equation d(fs)/d(Di,x). */
                int BaseIndexDixDjx = (i * 2 * n);
                /* Index of Dj,y in equation d(fs)/d(Di,y). */
                int BaseIndexDiyDjy = ((n + i) * 2 * n) + n;
                /* Indexes of the constant (free cooficent). */
                int IndexDixConst = i;
                int IndexDiyConst = n + i;

                CagdRType
                    IntergalCoof = 2.0 * (k - 1) * (k - 2) * Lambda *
                                ((a[(i - 2 + n) % n] * IProds[(i - 2 + n) % n][j])
                                + (c[i] * IProds[i][j])
                                - ((a[(i - 1 + n) % n] + c[(i - 1 + n) % n]) * 
                                   IProds[(i - 1 + n) % n][j])),
                    tmp = IntergalCoof * (k - 1) * (k - 2),
                    DjCoof = tmp * c[j],
                    Djp1Coof = tmp * (a[j] + c[j]),
                    Djp2Coof = tmp * a[j];

                A[BaseIndexDixDjx + j]     += DjCoof;
                A[BaseIndexDixDjx + (j + 1) % n] -= Djp1Coof;
                A[BaseIndexDixDjx + (j + 2) % n] += Djp2Coof;
                A[BaseIndexDiyDjy + j]     += DjCoof;
                A[BaseIndexDiyDjy + (j + 1) % n] -= Djp1Coof;
                A[BaseIndexDiyDjy + (j + 2) % n] += Djp2Coof;
                b[IndexDixConst] -= IntergalCoof * CrvDD -> Points[1][j];
                b[IndexDiyConst] -= IntergalCoof * CrvDD -> Points[2][j];
            }
        }
    }
    else {
        for (i = 0; i < n; ++i) {
            CagdRType 
                tmp = KV[i + k] - KV[i + 2];
            if ((tmp != 0) && (i < n - 1) && (KV[i + k + 1] != KV[i + 2])) {
                a[i] = 1.0 / (tmp * (KV[i + k + 1] - KV[i + 2]));
            }
            if ((tmp != 0) && (KV[i + k] != KV[i + 1])) {
                c[i] = 1.0 / (tmp * (KV[i + k] - KV[i + 1]));
            }
        }

        for (i = 0; i < n; ++i) {
            for (j = 0; j < n - 2; ++j) {
                /* Index of Dj,x in equation d(fs)/d(Di,x). */
                int IndexDixDjx = (i * 2 * n) + j;
                /* Index of Dj,y in equation d(fs)/d(Di,y). */
                int IndexDiyDjy = ((n + i) * 2 * n) + n + j;
                /* Indexes of the constant (free cooficent). */
                int IndexDixConst = i;
                int IndexDiyConst = n + i;

                CagdRType 
                    IntergalCoof = 2.0 * (k - 1) * (k - 2) * Lambda *
                                ((i > 1 ? a[i - 2] * IProds[i - 2][j] : 0)
                                + (i < n - 2 ? c[i] * IProds[i][j] : 0)
                                - ((i > 0 && i < n - 1) ? 
                                (a[i - 1] + c[i - 1]) * IProds[i - 1][j] : 0)),
                    tmp = IntergalCoof * (k - 1) * (k - 2),
                    DjCoof = tmp * c[j],
                    Djp1Coof = tmp * (a[j] + c[j]),
                    Djp2Coof = tmp * a[j];

                A[IndexDixDjx]     += DjCoof;
                A[IndexDixDjx + 1] -= Djp1Coof;
                A[IndexDixDjx + 2] += Djp2Coof;
                A[IndexDiyDjy]     += DjCoof;
                A[IndexDiyDjy + 1] -= Djp1Coof;
                A[IndexDiyDjy + 2] += Djp2Coof;
                b[IndexDixConst] -= IntergalCoof * CrvDD -> Points[1][j];
                b[IndexDiyConst] -= IntergalCoof * CrvDD -> Points[2][j];
            }
        }
    }

    for (i = 0; i < CrvDD -> Length; ++i)
        IritFree(IProds[i]);
    IritFree(IProds);
    IritFree(a);
    IritFree(c);
    CagdCrvFree(CrvD);
    CagdCrvFree(CrvDD);
}

#ifdef CAGD_FIT_ENERGY_1_CALC

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the curve first derivative energy integral:                   M
*    /            2                                                          V
*    | ||Crv'(t)||  dt                                                       V
*    /                                                                       V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       A curve to derive and integrate.                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: The caluculated integral value.                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   Energy1Calc, RegTermCalculatorFuncType                                   M
*****************************************************************************/
static CagdRType Energy1Calc(CagdCrvStruct *Crv)
{
    CagdRType *Energy;
    CagdCrvStruct
        *OpenEndCrv = Crv,
        *CrvD = NULL,
        *CrvD2 = NULL,
        *IntegrCrvD2 = NULL;

    if (CAGD_IS_PERIODIC_CRV(Crv)) {
        OpenEndCrv = CagdCrvCopy(Crv);
        OpenEndCrv = CagdCnvrtPeriodic2FloatCrv(OpenEndCrv);
        OpenEndCrv = CagdCnvrtFloat2OpenCrv(OpenEndCrv);
    }
    CrvD = CagdCrvDerive(OpenEndCrv);
    CrvD2 = SymbCrvDotProd(CrvD, CrvD),
    IntegrCrvD2 = CagdCrvIntegrate(CrvD2);

    Energy = CagdCrvEval(IntegrCrvD2, 1.0);

    if (Crv != OpenEndCrv)
        CagdCrvFree(OpenEndCrv);
    CagdCrvFree(CrvD);
    CagdCrvFree(CrvD2);
    CagdCrvFree(IntegrCrvD2);

    return Energy[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Calculates the curve first derivative energy (see Energy1Calc)           M
*   minimization matrix.                                                     M
*   The calculated coefficients are ADDED to A and b.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Input b-spline curve.                                         M
*   A:         Input/output initialized matrix,                              M
*              which size = 2 * Crv -> Length x 2 * Crv -> Length.           M
*   b:         Input/output initialized offset vector,                       M
*              which size = 2 * Crv -> Length x 1.                           M
*   Lambda:    Weight.							     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Energy1MatrixCalc, RegMatrixCalculatorFuncType                           M
*****************************************************************************/
static void Energy1MatrixCalc(CagdCrvStruct *Crv,
                              CagdRType *A,
                              CagdRType *b,
                              CagdRType Lambda)
{
    int i, j,
        n = Crv -> Length,
        k = Crv -> Order;
    CagdRType 
        *KV = Crv -> KnotVector;
    CagdCrvStruct
        *CrvD = CagdCrvDerive(Crv);
    CagdRType 
        **IProds = SymbBspBasisInnerProdMat(CrvD -> KnotVector, 
                                            CrvD -> Order + CrvD -> Length, 
                                            CrvD -> Order, 
                                            CrvD -> Order);

    if (CAGD_IS_PERIODIC_CRV(Crv)) {
        CagdRType 
            *a = (CagdRType*)IritMalloc(sizeof(CagdRType) * (n + 1));
        IRIT_ZAP_MEM(a, sizeof(CagdRType) * (n + 1));
        for (i = 0; i < n + 1; ++i) {
            if (KV[i + k - 1] != KV[i]) {
                a[i] = 1.0 / (KV[i + k - 1] - KV[i]);
            }
        }
        for (i = 0; i < n; ++i) {
            for (j = 0; j < n; ++j) {
                int 
                /* Index of Dj,x in equation d(fs)/d(Di,x). */
                    BaseIndexDixDjx = (i * 2 * n),
                /* Index of Dj,y in equation d(fs)/d(Di,y). */
                    BaseIndexDiyDjy = ((n + i) * 2 * n) + n,
                /* Indexes of the constant (free cooficent). */
                    IndexDixConst = i,
                    IndexDiyConst = n + i;

                CagdRType IntergalCoof = 2.0 * (k - 1) * Lambda *
                    (IProds[i % n][(j + 1) % n] * a[i] - 
                     IProds[(i + 1) % n][(j + 1) % n] * a[i + 1]);
                CagdRType DsCoof = 
                    IntergalCoof * (k - 1) * a[j + 1];

                A[BaseIndexDixDjx + (j + 1) % n] += DsCoof;
                A[BaseIndexDixDjx + j] -= DsCoof;
                A[BaseIndexDiyDjy + (j + 1) % n] += DsCoof;
                A[BaseIndexDiyDjy + j] -= DsCoof;
                b[IndexDixConst] -= IntergalCoof * CrvD -> Points[1][j];
                b[IndexDiyConst] -= IntergalCoof * CrvD -> Points[2][j];
            }
        }
        IritFree(a);
    } else {
        for (i = 0; i < n; ++i) {
            for (j = 0; j < n - 1; ++j) {
                if ( !(((i > 0) && (KV[i + k - 1] == KV[i])) ||
                    ((i < n - 1) && (KV[i + k] == KV[i + 1])) ||
                    (KV[j + k] == KV[j + 1])) ) {


                    int /* Index of Dj,x in equation d(fs)/d(Di,x). */
                        IndexDixDjx = (i * 2 * n) + j,
                        /* Index of Dj,y in equation d(fs)/d(Di,y). */
                        IndexDiyDjy = ((n + i) * 2 * n) + n + j,
                        /* Indexes of the constant (free cooficent). */
                        IndexDixConst = i,
                        IndexDiyConst = n + i;

                    CagdRType
		        IntergalCoof = 2.0 * (k - 1) * Lambda *
                        ((i > 0 ? IProds[i - 1][j]/(KV[i + k - 1] - KV[i]) : 0) 
                        - (i < n - 1 ? IProds[i][j]/(KV[i + k] - KV[i + 1]) : 0));
                    CagdRType DsCoof = 
                        IntergalCoof * (k - 1) / (KV[j + k] - KV[j + 1]);

                    A[IndexDixDjx + 1] += DsCoof;
                    A[IndexDixDjx] -= DsCoof;
                    A[IndexDiyDjy + 1] += DsCoof;
                    A[IndexDiyDjy] -= DsCoof;
                    b[IndexDixConst] -= IntergalCoof * CrvD -> Points[1][j];
                    b[IndexDiyConst] -= IntergalCoof * CrvD -> Points[2][j];
                }
            }
        }
    }

    for (i = 0; i < CrvD -> Length; ++i)
        IritFree(IProds[i]);
    IritFree(IProds);
    CagdCrvFree(CrvD);
}

#endif /* CAGD_FIT_ENERGY_1_CALC */

#ifdef CAGD_FIT_DIAGONAL_CRV_CALC

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes an initial b-spline fitting curve which is a diagonal of          M
* the points cloud bounding box.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:        Points cloud.                                             M
*   NumOfPoints:   Number of points in PtList.                               M
*   Length:        The desired length of the output b-spline curve.          M
*   Order:         The desired order of the output b-spline curve.           M
*   Periodic:      TRUE for periodic output curve, FALSE for open end.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: B-spline curve which is a bounding box diagonal [/]     M
*                    of the points cloud.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BBoxDiagonalInitCrvCalculator, InitFittingCrvCalculatorFuncType,         M
*   bounding box diagonal                                                    M
*****************************************************************************/
static CagdCrvStruct *BBoxDiagonalInitCrvCalculator(CagdPType *PtList, 
                                                    int NumOfPoints,
                                                    int Length, 
                                                    int Order,
                                                    CagdBType Periodic)
{
    int i, Axis;
    CagdRType *PointsCloud[4];
    CagdPType *Pt1, *Pt2;
    CagdBBoxStruct 
        *BBox = CagdBBoxNew();
    CagdCrvStruct 
        *NewCrv = BspPeriodicCrvNew(Length, Order, Periodic, CAGD_PT_E2_TYPE);

    if (NumOfPoints < 1)
        IRIT_FATAL_ERROR("Points list should contain at least 1 point.");

    /* Create uniform knot vector. */
    if (Periodic)
        BspKnotUniformPeriodic(Length, Order, NewCrv -> KnotVector);
    else
        BspKnotUniformOpen(Length, Order, NewCrv -> KnotVector);

    /* Calculate top-left and bottom-right points of the points cloud bbox. */
    PointsCloud[0] = NULL; /* No weights. */
    for (Axis = 1; Axis <= 3; ++Axis) {
        PointsCloud[Axis] = 
            (CagdRType *) IritMalloc(sizeof(CagdRType) * NumOfPoints);
    }
    for (i = 0; i < NumOfPoints; ++i) {
        for (Axis = 1; Axis <= 3; ++Axis) {
            PointsCloud[Axis][i] = PtList[i][Axis - 1];
        }
    }
    CagdPointsBBox(PointsCloud, NumOfPoints, 3, BBox -> Min, BBox -> Max);
    Pt1 = &(BBox -> Min);
    Pt2 = &(BBox -> Max);

    /* Create linear polygon from Pt1 to Pt2. */
    for (i = 0; i < Length; ++i) {
        for (Axis = 1; Axis <= 2; ++Axis) {
            CagdRType 
                Coof = (CagdRType)i / (Length - 1);
            NewCrv -> Points[Axis][i] = 
                (*Pt1)[Axis - 1] +
                ((*Pt2)[Axis - 1] - (*Pt1)[Axis - 1]) * Coof;
        }
    }
    
    IritFree(BBox);
    for (Axis = 1; Axis <= 3; ++Axis) {
        IritFree(PointsCloud[Axis]);
    }
    return NewCrv;
}

#endif /* CAGD_FIT_DIAGONAL_CRV_CALC */
