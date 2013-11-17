/******************************************************************************
* AnalyFit.c - Analytic function fit to a given point set.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 2002.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "extra_fn.h"
#include "geom_loc.h"

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugPrintInputSolver, FALSE);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits a bilinear surface to the set of given points as		     M
* F(u,v) = A + B * u + C * v + D * u * v,  A,B,C,D points in R^3.            M
*                                                                            *
* PARAMETERS:                                                                M
*   ParamDomainPts:  Array of UV points prescribing the parametric values.   M
*   EuclideanPts:    Array of XYZ points defining the Euclidean values of    M
*		     the ParamDomainPts with obviously the same order.       M
*   FirstAtOrigin:   If TRUE, the first points is set to be at U = V = 0.    M
*   NumPts:          Number of points in ParamDomainPts and EuclideanPts.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtPtType *:     Array of four points values, A, B, C, D.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSrfQuadricFit                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSrfBilinearFit                                                         M
*****************************************************************************/
IrtPtType *GMSrfBilinearFit(IrtPtType *ParamDomainPts,
			    IrtPtType *EuclideanPts,
			    int FirstAtOrigin,
			    int NumPts)
{
    static IrtPtType Sol[4];
    int i, j, MatSize;
    IrtRType *M, *B, *Mat;

    /* Bring the origin to the first UV point if so requested. */
    if (FirstAtOrigin) {
	for (i = NumPts - 1; i >= 0; i--) {
	    ParamDomainPts[i][0] -= ParamDomainPts[0][0];
	    ParamDomainPts[i][1] -= ParamDomainPts[0][1];
	}
    }

    /* Build the matrices for the least squares solution. */
    MatSize = sizeof(IrtRType) * 4 * NumPts;
    M = Mat = (IrtRType *) IritMalloc(MatSize);

    for (i = 0; i < NumPts; i++, M += 4) {
	M[0] = 1;
	M[1] = ParamDomainPts[i][0]; /* U */
	M[2] = ParamDomainPts[i][1]; /* V */
	M[3] = ParamDomainPts[i][0] * ParamDomainPts[i][1]; /* U * V */
    }

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSolver) {
        for (i = 0; i < NumPts; i++) {
	    IRIT_INFO_MSG("[");
	    for (j = 0; j < 4; j++) {
		IRIT_INFO_MSG_PRINTF("%9.6f ", Mat[i * 4 + j]);
	    }
	    IRIT_INFO_MSG("]\n");
	}
    }
#   endif /* DEBUG */

    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumPts, 4)) < IRIT_UEPS) {
	IritFree(Mat);
	return NULL;
    }
    
    /* Solve for the coefficients of this curve's axis. */
    B = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumPts);
    for (i = 0; i < 3; i++) { /* For X, Y, Z */
	IrtRType V[4];

	for (j = 0; j < NumPts; j++)
	    B[j] = EuclideanPts[j][i];

	SvdLeastSqr(NULL, V, B,	NumPts, 4);

	for (j = 0; j < 4; j++)
	    Sol[j][i] = V[j];
    }

    IritFree(Mat);
    IritFree(B);

    return Sol;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits a quadric surface to the set of given points as		     M
* F(u,v) = A + B * u + C * v + D * u * u + E * u * v + F * v * v,	     M
*						 A,B,C,D,E,F points in R^3.  M
*                                                                            *
* PARAMETERS:                                                                M
*   ParamDomainPts:  Array of UV points prescribing the parametric values.   M
*   EuclideanPts:    Array of XYZ points defining the Euclidean values of    M
*		     the ParamDomainPts with obviously the same order.       M
*   FirstAtOrigin:   If TRUE, the first points is set to be at U = V = 0.    M
*   NumPts:          Number of points in ParamDomainPts and EuclideanPts.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtPtType *:     Array of six point values, A,B,C,D,E,F in order.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSrfBilinearFit, GMSrfQuadricQuadOnly, GMSrfCubicQuadOnly               M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSrfQuadricFit                                                          M
*****************************************************************************/
IrtPtType *GMSrfQuadricFit(IrtPtType *ParamDomainPts,
			   IrtPtType *EuclideanPts,
			   int FirstAtOrigin,
			   int NumPts)
{
    static IrtPtType Sol[6];
    int i, j, MatSize;
    IrtRType *M, *B, *Mat;

    /* Bring the origin to the first UV point if so requested. */
    if (FirstAtOrigin) {
	for (i = NumPts - 1; i >= 0; i--) {
	    ParamDomainPts[i][0] -= ParamDomainPts[0][0];
	    ParamDomainPts[i][1] -= ParamDomainPts[0][1];
	}
    }

    /* Build the matrices for the least squares solution. */
    MatSize = sizeof(IrtRType) * 6 * NumPts;
    M = Mat = (IrtRType *) IritMalloc(MatSize);

    for (i = 0; i < NumPts; i++, M += 6) {
	M[0] = 1;
	M[1] = ParamDomainPts[i][0]; /* U */
	M[2] = ParamDomainPts[i][1]; /* V */
	M[3] = ParamDomainPts[i][0] * ParamDomainPts[i][0]; /* U * U */
	M[4] = ParamDomainPts[i][0] * ParamDomainPts[i][1]; /* U * V */
	M[5] = ParamDomainPts[i][1] * ParamDomainPts[i][1]; /* V * V */
    }

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSolver) {
        for (i = 0; i < NumPts; i++) {
	    IRIT_INFO_MSG_PRINTF("[");
	    for (j = 0; j < 6; j++) {
		IRIT_INFO_MSG_PRINTF("%9.6f ", Mat[i * 6 + j]);
	    }
	    IRIT_INFO_MSG_PRINTF("]\n");
	}
    }
#   endif /* DEBUG */

    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumPts, 6)) < IRIT_UEPS) {
	IritFree(Mat);
	return NULL;
    }
    
    /* Solve for the coefficients of this fit. */
    B = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumPts);
    for (i = 0; i < 3; i++) { /* For X, Y, Z */
	IrtRType V[6];

	for (j = 0; j < NumPts; j++)
	    B[j] = EuclideanPts[j][i];

	SvdLeastSqr(NULL, V, B,	NumPts, 6);

	for (j = 0; j < 6; j++)
	    Sol[j][i] = V[j];
    }

    IritFree(Mat);
    IritFree(B);

    return Sol;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits a quadric surface (quad terms only) to the set of given points as   M
* F(u,v) = A + B * u + C * v + D * u * u + E * u * v + F * v * v,	     M
*						 A,B,C,D,E,F points in R^3.  M
*                                                                            *
* PARAMETERS:                                                                M
*   ParamDomainPts:  Array of UV points prescribing the parametric values.   M
*   EuclideanPts:    Array of XYZ points defining the Euclidean values of    M
*		     the ParamDomainPts with obviously the same order.       M
*   FirstAtOrigin:   If TRUE, the first points is set to be at U = V = 0.    M
*   NumEucDim:       Number of Euclidean dimension. 1 for scalar surface and M
*		     upto 3 for parametric surface in R^3.		     M
*   NumPts:          Number of points in ParamDomainPts and EuclideanPts.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtPtType *:     Array of six point values, A,B,C,D,E,F in order,        M
		     where A = B = C = 0 always.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSrfBilinearFit, GMSrfQuadricFit, GMSrfCubicQuadOnly                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSrfQuadricQuadOnly                                                     M
*****************************************************************************/
IrtPtType *GMSrfQuadricQuadOnly(IrtPtType *ParamDomainPts,
				IrtPtType *EuclideanPts,
				int FirstAtOrigin,
				int NumEucDim,
				int NumPts)
{
    static IrtPtType Sol[6];
    int i, j, MatSize;
    IrtRType *M, *B, *Mat;

    /* Bring the origin to the first UV point if so requested. */
    if (FirstAtOrigin) {
	for (i = NumPts - 1; i >= 0; i--) {
	    ParamDomainPts[i][0] -= ParamDomainPts[0][0];
	    ParamDomainPts[i][1] -= ParamDomainPts[0][1];
	}
    }

    /* Build the matrices for the least squares solution. */
    MatSize = sizeof(IrtRType) * 3 * NumPts;
    M = Mat = (IrtRType *) IritMalloc(MatSize);

    for (i = 0; i < NumPts; i++, M += 3) {
	M[0] = ParamDomainPts[i][0] * ParamDomainPts[i][0]; /* U * U */
	M[1] = ParamDomainPts[i][0] * ParamDomainPts[i][1]; /* U * V */
	M[2] = ParamDomainPts[i][1] * ParamDomainPts[i][1]; /* V * V */
    }

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSolver) {
        for (i = 0; i < NumPts; i++) {
	    IRIT_INFO_MSG("[");
	    for (j = 0; j < 3; j++) {
		IRIT_INFO_MSG_PRINTF("%9.6f ", Mat[i * 3 + j]);
	    }
	    IRIT_INFO_MSG("]\n");
	}
    }
#   endif /* DEBUG */

    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumPts, 3)) < IRIT_UEPS) {
	IritFree((void *) Mat);
	return NULL;
    }
    
    /* Solve for the coefficients of this fit. */
    B = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumPts);
    for (i = 0; i < NumEucDim; i++) { /* For X, Y, Z */
	IrtRType V[3];

	for (j = 0; j < NumPts; j++)
	    B[j] = EuclideanPts[j][i];

	SvdLeastSqr(NULL, V, B,	NumPts, 3);

	for (j = 0; j < 3; j++)
	    Sol[j][i] = 0;
	for (j = 3; j < 6; j++)
	    Sol[j][i] = V[j - 3];
    }

    IritFree((void *) Mat);
    IritFree((void *) B);

    return Sol;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits a cubic surface (cubic and quad terms only) to the set of given     M
* points as								     M
*  F(u,v) = A + B * u + C * v + D * u^2 + E * u * v + F * v^2 +		     M
*           G * u^3 + H * u^2 * v + I * u * v^2 + J * v^3,		     M
*					 A,B,C,D,E,F,G,H,I,J points in R^3.  M
*                                                                            *
* PARAMETERS:                                                                M
*   ParamDomainPts:  Array of UV points prescribing the parametric values.   M
*   EuclideanPts:    Array of XYZ points defining the Euclidean values of    M
*		     the ParamDomainPts with obviously the same order.       M
*   FirstAtOrigin:   If TRUE, the first points is set to be at U = V = 0.    M
*   NumEucDim:       Number of Euclidean dimension. 1 for scalar surface and M
*		     upto 3 for parametric surface in R^3.		     M
*   NumPts:          Number of points in ParamDomainPts and EuclideanPts.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtPtType *:     Array of 10 point values, A,B,C,D,E,F,G,H,I,J in order, M
		     where A = B = C = 0 always.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   GMSrfBilinearFit, GMSrfQuadricFit, GMSrfQuadricQuadOnly                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMSrfCubicQuadOnly                                                       M
*****************************************************************************/
IrtPtType *GMSrfCubicQuadOnly(IrtPtType *ParamDomainPts,
			      IrtPtType *EuclideanPts,
			      int FirstAtOrigin,
			      int NumEucDim,
			      int NumPts)
{
    static IrtPtType Sol[10];
    int i, j, MatSize;
    IrtRType *M, *B, *Mat;

    /* Bring the origin to the first UV point if so requested. */
    if (FirstAtOrigin) {
	for (i = NumPts - 1; i >= 0; i--) {
	    ParamDomainPts[i][0] -= ParamDomainPts[0][0];
	    ParamDomainPts[i][1] -= ParamDomainPts[0][1];
	}
    }

    /* Build the matrices for the least squares solution. */
    MatSize = sizeof(IrtRType) * 7 * NumPts;
    M = Mat = (IrtRType *) IritMalloc(MatSize);

    for (i = 0; i < NumPts; i++, M += 7) {
	M[0] = ParamDomainPts[i][0] * ParamDomainPts[i][0]; /* U^2 */
	M[1] = ParamDomainPts[i][0] * ParamDomainPts[i][1]; /* U * V */
	M[2] = ParamDomainPts[i][1] * ParamDomainPts[i][1]; /* V^2 */

	M[3] = M[0] * ParamDomainPts[i][0]; /* U^3 */
	M[4] = M[0] * ParamDomainPts[i][1]; /* U^2*V */
	M[5] = M[2] * ParamDomainPts[i][0]; /* U*V^2 */
	M[6] = M[2] * ParamDomainPts[i][1]; /* V^3 */
    }

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSolver) {
        for (i = 0; i < NumPts; i++) {
	    IRIT_INFO_MSG("[");
	    for (j = 0; j < 7; j++) {
		IRIT_INFO_MSG_PRINTF("%9.6f ", Mat[i * 7 + j]);
	    }
	    IRIT_INFO_MSG("]\n");
	}
    }
#   endif /* DEBUG */

    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumPts, 7)) < IRIT_UEPS) {
	IritFree((void *) Mat);
	return NULL;
    }
    
    /* Solve for the coefficients of this fit. */
    B = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumPts);
    for (i = 0; i < NumEucDim; i++) { /* For X, Y, Z */
	IrtRType V[7];

	for (j = 0; j < NumPts; j++)
	    B[j] = EuclideanPts[j][i];

	SvdLeastSqr(NULL, V, B,	NumPts, 7);

	for (j = 0; j < 3; j++)
	    Sol[j][i] = 0;
	for (j = 3; j < 10; j++)
	    Sol[j][i] = V[j - 3];
    }

    IritFree((void *) Mat);
    IritFree((void *) B);

    return Sol;
}
