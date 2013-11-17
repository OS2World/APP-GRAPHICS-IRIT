/******************************************************************************
* QR factorization of matrices.						      *
* See "matrix Computation", by Gene H. Golub and Charles F. Van Loan.         *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
*					 Written by Gershon Elber, April 1999 *
******************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"
#include "extra_fn.h"

#define IDXA(x, y)	A[(x) + (y) * n]
#define IDXAt(x, y)	At[(x) + (y) * n]
#define IDXQ(x, y)	Q[(x) + (y) * n]
#define IDXR(x, y)	R[(x) + (y) * m]
#define IDXJ(x, y)	J[(x) + (y) * n]
#define IDXK(x, y)	K[(x) + (y) * n]

#define SIGN2(a, b)	((b) >= 0.0 ? IRIT_FABS(a) : -IRIT_FABS(a))

/*****************************************************************************
* DESCRIPTION:                                                               M
* Performs a QR factorization of matrix A.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   A:     The matrix of size n by m (m <= n), must be preallocated          M
*	   dynamically by the user.  	                                     M
*   n, m:  Dimensions of matrix A.                                           M
*   Q, R:  The computed decomposition matrices, must be preallocated         M
*	   dynamically by the user.  Q is n by m (like A), R is m by m.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if Singular, FALSE otherwise.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   SvdLeastSqr                                                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritQRFactorization, QR factorization, linear systems, matrices          M
*****************************************************************************/
int IritQRFactorization(IrtRType *A,
			int n,
			int m,
			IrtRType *Q,
			IrtRType *R)
{
    int i, j, k, l,
	Singular = FALSE;
    IrtRType Scale, Sigma, Sum, Tau,
	*OrigQ = Q,
	*HSize = (IrtRType *) IritMalloc(sizeof(IrtRType) * n),
	*RDiag = (IrtRType *) IritMalloc(sizeof(IrtRType) * n),
	*HVec = (IrtRType *) IritMalloc(sizeof(IrtRType) * n),
	*AA = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * n),
	*J = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * n),
	*K = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * n);

    IRIT_ZAP_MEM(AA, sizeof(IrtRType) * n * n);
    IRIT_GEN_COPY(AA, A, sizeof(IrtRType) * n * m);
    A = AA;

    Q = K;
    K = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * n);

    for (k = 0; k < n - 1; k++) {
	Scale = 0.0;

	for (i = k; i < n; i++)
	    Scale = IRIT_MAX(Scale, IRIT_FABS(IDXA(i, k)));

	if (Scale == 0.0)
	    HSize[k] = RDiag[k] = 0.0;
	else {
	    for (Sum = 0.0, i = k; i < n; i++) {
		IDXA(i, k) /= Scale;
		Sum += IRIT_SQR(IDXA(i, k));
	    }
	    Sigma = SIGN2(sqrt(Sum), IDXA(k, k));
	    IDXA(k, k) += Sigma;
	    HSize[k] = Sigma * IDXA(k, k);
	    RDiag[k] = -Scale * Sigma;
	    for (j = k + 1; j < n; j++) {
		for (Sum = 0.0, i = k; i < n; i++)
		    Sum += IDXA(i, k) * IDXA(i, j);
		Tau = Sum / HSize[k];
		for (i = k; i < n; i++)
		    IDXA(i, j) -= Tau * IDXA(i, k);
	    }
	}
    }

    RDiag[n - 1] = IDXA(n - 1, n - 1);

    /* Copy A to R, update diagonal of R and zero everything below it. */
    for (i = 0; i < m; i++)
	for (j = 0; j < m; j++)
	    IDXR(i, j) = i >= j ? (i == j ? RDiag[i] : 0.0) : IDXA(i, j);

    for (i = 0; i < m; i++)
        if (IRIT_FABS(IDXR(i, i)) < IRIT_UEPS) {
	    Singular = TRUE;
	    break;
	}

#ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintQRFactor, FALSE) {
	    printf("*------------- R ------------*   Sing = %d\n", Singular);
	    for (i = 0; i < m; i++) {
	        for (j = 0; j < m; j++)
		    printf("%9.6f ", IDXR(i, j));
		printf("\n");
	    }
	    printf(" *-HSIZE-*\n");
	    for (i = 0; i < n - 1; i++)
	        printf(" %9.6f\n", HSize[i]);
	}
    }
#endif /* DEBUG */

    /* Compute Q matrix as the product of sequence of Householder matrices. */
    IRIT_ZAP_MEM(Q, sizeof(IrtRType) * n * n);
    for (i = 0; i < n; i++)
	IDXQ(i, i) = 1.0;

    for (l = 0; l < n - 1 && HSize[l] != 0.0; l++) {
	for (i = 0; i < n; i++)
	    HVec[i] = i < l ? 0.0 : IDXA(i, l);

	for (i = 0; i < n; i++)
	    for (j = 0; j < n; j++)
		IDXJ(i, j) = (i == j ? 1.0 : 0.0) -
						 HVec[i] * HVec[j] / HSize[l];

	/* Now multiply with current accumulation of Q. */
	IRIT_ZAP_MEM(K, sizeof(IrtRType) * n * n);
	for (i = 0; i < n; i++)
	    for (j = 0; j < n; j++)
		for (k = 0; k < n; k++)
		    IDXK(i, j) += IDXQ(i, k) * IDXJ(k, j);
	IRIT_SWAP(IrtRType *, K, Q);			
    }

    IRIT_GEN_COPY(OrigQ, Q, sizeof(IrtRType) * n * m);

    IritFree(HSize);
    IritFree(RDiag);
    IritFree(HVec);
    IritFree(J);
    IritFree(K);
    IritFree(Q);
    IritFree(A);

    return Singular;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Solve for Ax = b when R is upper diagonal using back substitution.         M
*                                                                            *
* PARAMETERS:                                                                M
*   A:     The diagonal matrix of size n by n, must be preallocated          M
*	   dynamically by the user.  	                                     M
*   n:     Dimension of matrix A.                                            M
*   b:     A vector of size n.		                                     M
*   x:     The solution vector of size n.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if Singular, FALSE otheriwse.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritSolveLowerDiagMatrix, IritQRFactorization, IritQRUnderdetermined,    M
*   SvdLeastSqr							             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSolveUpperDiagMatrix, QR factorization, linear systems, matrices     M
*****************************************************************************/
int IritSolveUpperDiagMatrix(const IrtRType *A,
			     int n,
			     const IrtRType *b,
			     IrtRType *x)
{
    int i, j;

    for (i = n - 1; i >= 0; i--) {
	IrtRType
	    Sum = 0.0;

	if (IDXA(i, i) == 0.0)
	    return TRUE;

	for (j = i + 1; j < n; j++)
	    Sum += IDXA(i, j) * x[j];

	x[i] = (b[i] - Sum) / IDXA(i, i);
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Solve for Ax = b when R is lower diagonal using forward substitution.      M
*                                                                            *
* PARAMETERS:                                                                M
*   A:     The diagonal matrix of size n by n, must be preallocated          M
*	   dynamically by the user.  	                                     M
*   n:     Dimension of matrix A.                                            M
*   b:     A vector of size n.		                                     M
*   x:     The solution vector of size n.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if Singular, FALSE otheriwse.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritSolveUpperDiagMatrix, IritQRFactorization, IritQRUnderdetermined,    M
*   SvdLeastSqr							             M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritSolveLowerDiagMatrix, QR factorization, linear systems, matrices     M
*****************************************************************************/
int IritSolveLowerDiagMatrix(const IrtRType *A,
			     int n,
			     const IrtRType *b,
			     IrtRType *x)
{
    int i, j;

    for (i = 0; i < n; i++) {
	IrtRType
	    Sum = 0.0;

	if (IDXA(i, i) == 0.0)
	    return TRUE;

	for (j = 0; j < i; j++)
	    Sum += IDXA(i, j) * x[j];

	x[i] = (b[i] - Sum) / IDXA(i, i);
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Solve for Ax = b when the system is underdetermined (singular).  The       M
* solution is a minimum 2-norm solution.				     M
*   See "matrix Computation", by Gene H. Golub and Charles F. Van Loan, 3rd  M
* edition, pp 271-272.							     M
*   If A != NULL a QR factorization is computed, otherwise (A == NULL) a     M
* solution is computed for the given b and is placed in x.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   A:     The matrix of size m by n (m <= n), must be preallocated          M
*	   dynamically by the user.  	                                     M
*   x:     The solution vector of size n.                                    M
*   b:     A vector of size m.		                                     M
*   m, n:  Dimensions of matrix A. Becuase A is underdetermined m <= n.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if Singular, FALSE otheriwse.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritQRFactorization, IritSolveUpperDiagMatrix, SvdLeastSqr               M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritQRUnderdetermined, QR factorization, linear systems, matrices        M
*****************************************************************************/
int IritQRUnderdetermined(IrtRType *A,
			  IrtRType *x,
			  const IrtRType *b,
			  int m,
			  int n)
{
    int i, j, Singular;
    IRIT_STATIC_DATA IrtRType
	*Q = NULL,
	*R = NULL,
	*z = NULL;

    if (A != NULL) {
	if (Q != NULL) {
	    IritFree(Q);
	    IritFree(R);
	    IritFree(z);
	}
	Q = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * m);
	R = (IrtRType *) IritMalloc(sizeof(IrtRType) * m * m);
	z = (IrtRType *) IritMalloc(sizeof(IrtRType) * n);

	/* Note A is actually A^T, the way we traverse matrices here. */
	Singular = IritQRFactorization(A, n, m, Q, R);

	/* Computer R transpose, in place. */
	for (i = 1; i < m; i++) {
	    for (j = 0; j < i; j++) {
		IrtRType
		    *R1 = &IDXR(i, j),
		    *R2 = &IDXR(j, i);

		IRIT_SWAP(IrtRType, *R1, *R2);
	    }
	}

	return Singular;
    }
    else {
	Singular = IritSolveLowerDiagMatrix(R, m, b, z);

	IRIT_ZAP_MEM(x, sizeof(IrtRType) * n);
	for (i = 0; i < n; i++) {
	    for (j = 0; j < m; j++)
		x[i] += IDXQ(i, j) * z[j];
	}

	return Singular;
    }
}

#ifdef DEBUG_MAIN_QR_FACTORIZATION

/******************************************************************************
* AUXILIARY:			 					      *
* Test routine for IritQRFactorization.  Reads in a matrix and invokes        *
* IritQRFactorization.			 				      *
******************************************************************************/
void main(int argc, char **argv)
{
    int i, j, k, n, m, Singular;
    IrtRType *A, *Q, *R;

    scanf("%d %d", &n, &m);

    if (n >= 1 * m >= 1) {
	A = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * m);
	Q = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * m);
	R = (IrtRType *) IritMalloc(sizeof(IrtRType) * n * m);

	for (i = 0; i < n; i++)
	    for (j = 0; j < m; j++)
		scanf("%lf", &IDXA(i, j));
    }
    else {
	printf("Expecting a size between one and ten\n");
	A = Q = R = NULL;
	exit(1);
    }

    printf("Input matrix to QR factorization:\n");
    for (i = 0; i < n; i++) {
	for (j = 0; j < m; j++)
	    printf("%9.6f ", IDXA(i, j));
	printf("\n");
    }

    Singular = IritQRFactorization(A, n, m, Q, R);	       /* Go do it! */

    printf("\nQ matrix is %sSingular\n", Singular ? "" : "Non ");
    for (i = 0; i < n; i++) {
	for (j = 0; j < m; j++)
	    printf("%9.6f ", IDXQ(i, j));
	printf("\n");
    }

    printf("Q * Q^T matrix\n");
    for (i  = 0; i < n; i++) {
	for (j  = 0; j < m; j++) {
	    IrtRType
		Val = 0.0;

	    for (k  = 0; k < m; k++)
		Val += IDXQ(i, k) * IDXQ(j, k);
	    printf("%9.6f ", Val);
	}
	printf("\n");
    }

    printf("R matrix\n");
    for (i  = 0; i < m; i++) {
	for (j  = 0; j < m; j++)
	    printf("%9.6f ", IDXR(i, j));
	printf("\n");
    }

    printf("Q * R matrix\n");
    for (i  = 0; i < n; i++) {
	for (j  = 0; j < m; j++) {
	    IrtRType
		Val = 0.0;

	    for (k  = 0; k < m; k++)
		Val += IDXQ(i, k) * IDXR(k, j);
	    printf("%9.6f ", Val);
	}
	printf("\n");
    }

    IritFree(A);
    IritFree(Q);
    IritFree(R);
}

#endif /* DEBUG_MAIN_QR_FACTORIZATION */
