/******************************************************************************
* Numerical Recipes' SVD singular value decomposition code.		      *
* "Numerical Recipes in C", by William H. Press el al., University of	      *
* Cambridge, 1988, ISBN 0521-35465-X.					      *
*   Really ugly piece of code, that works (bug for underdetermined cases).    *
******************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "extra_fn.h"

#define SVD_TOL 1.0e-5
/* #define SVD_TEST_VALIDITY */

#define SIGN2(a, b)	((b) >= 0.0 ? IRIT_FABS(a) : -IRIT_FABS(a))
#define PYTHAG(a, b)	sqrt(IRIT_SQR(a) + IRIT_SQR(b))

#define SVD_VECTOR(n)	((IrtRType *) IritMalloc(sizeof(IrtRType) * (n + 1)))
#define SVD_FREE_VEC(p)	IritFree(p)

/*****************************************************************************
* DESCRIPTION:                                                               M
* Perform singular value decomposition on the 3x3 main minor of the affine   M
* transformation matrix.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:        Source matrix to decompose.                                    M
*   U:        Left orthonormal matrix.                                       M
*   S:	      Singular components.					     M
*   V:        Right orthonormal matrix.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   FALSE if failed, TRUE otherwise.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SvdLeastSqr, SvdMatrixNxN                                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SvdMatrix4x4, singular value decomposition, linear systems, matrices     M
*****************************************************************************/
int SvdMatrix4x4(IrtHmgnMatType M,
		 IrtRType U[3][3],
		 IrtVecType S,
		 IrtRType V[3][3])
{
    IrtRType c, f, h, s, x, y, z, rv1[3],
	anorm = 0.0, 
	g = 0.0, 
	scale = 0.0;
    int flag, its, i, j, jj, k,
    	l = 0,
	nm = 0;

    for (i = 0; i < 3; ++i) {
	for (j = 0; j < 3; ++j) {
	    U[i][j] = M[i][j];
	}
        S[i] = 0;
    }

    /* 1. Householder reduction to bidiagonal form. */
    for (i = 0; i < 3; ++i) {
	l = i + 1;
	rv1[i] = scale * g;
	g = s = scale = 0;
	for (k = i; k < 3; ++k) 
	    scale += IRIT_FABS(U[k][i]);
	if (scale != 0.0) {
	    for (k = i; k < 3; ++k) {
		U[k][i] /= scale;
		s += U[k][i] * U[k][i];
	    }
	    f = U[i][i];
	    g = (f >= 0 ? -sqrt(s) : sqrt(s));
	    h = f * g - s;
	    U[i][i] = f - g;
	    for (j = l; j < 3; ++j) {
		for (s = 0, k = i; k < 3; ++k) 
		    s += U[k][i] * U[k][j];
		f = s / h;
		for (k = i; k < 3; ++k) 
		    U[k][j] += f * U[k][i];
	    }
	    for (k = i; k < 3; ++k) 
		U[k][i] *= scale;
	}
	S[i] = scale * g;

	g = s = scale = 0;
	for (k = l; k < 3; ++k) 
	    scale += IRIT_FABS(U[i][k]);
	if (scale != 0.0) {
	    for (k = l; k < 3; ++k) {
		U[i][k] /= scale;
		s += U[i][k] * U[i][k];
	    }
	    f = U[i][l];
	    g = (f >= 0 ? -sqrt(s) : sqrt(s));
	    h = f * g - s;
	    U[i][l] = f - g;
	    for (k = l; k < 3; ++k) 
		rv1[k] = U[i][k] / h;
	    for (j = l; j < 3; ++j) {
		for (s = 0, k = l; k < 3; ++k) 
		    s += U[j][k] * U[i][k];
		for (k = l; k < 3; ++k) 
		    U[j][k] += s*rv1[k];
	    }
	    for (k = l; k < 3; ++k) 
		U[i][k] *= scale;
	}
	anorm = IRIT_MAX(anorm, (IRIT_FABS(S[i]) + IRIT_FABS(rv1[i])));
    }
    
    /* 2. Accumulation of right-hand transform V. */
    for (i = 3; --i >= 0; ) {
	if (g != 0.0) {
	    for (j = l; j < 3; ++j)
		/* double div to avoid underflow. */
		V[j][i] = ((U[i][j] / U[i][l]) / g);
	    for (j = l; j < 3; ++j) {
		for (s = 0, k = l; k < 3; ++k)
		    s += U[i][k] * V[k][j];
		for (k = l; k < 3; ++k) 
		    V[k][j] += s * V[k][i];
	    }
	}
	for (j = l; j < 3; ++j) 
	    V[i][j] = V[j][i] =0;
	V[i][i] = 1;
	g = rv1[i];
	l = i;
    }
    
    /* 3. Accumulation of left-hand transform U. */
    for (i = 3; --i >= 0;) {
	l = i + 1;
	g = S[i];
	for (j = l; j < 3; ++j) 
	    U[i][j] = 0;
	if (g != 0.0) {
	    g = 1 / g;
	    for (j = l; j < 3; ++j) {
		for (s = 0, k = l; k < 3; ++k) 
		    s += U[k][i] * U[k][j];
		f = (s / U[i][i]) * g;
		for (k = i; k < 3; ++k) 
		    U[k][j] += f * U[k][i];
	    }
	    for (j = i; j < 3; ++j) 
		U[j][i] *= g;
	} 
	else {
	    for (j = i; j < 3; ++j)
		U[j][i] = 0;
	}
	++U[i][i];
    }
    
    /* 4. Diagonalization of bidiagonal form. */
    for (k = 3; --k >= 0;) {                   /* Loop over singular values. */
	for (its = 1; its <= 30; ++its) {   /* Loop over allowed iterations. */
	    flag = 1;
	    for (l = k; l >= 0; --l) {                /* Test for splitting. */
		nm = l - 1;
		if ((IRIT_FABS(rv1[l]) + anorm) == anorm) {
		    flag = 0;
		    break;
		}
		if ((IRIT_FABS(S[nm]) + anorm) == anorm) 
		    break;
	    }
	    if (flag) {
		c = 0;
		s = 1;
		for (i = l; i <= k; ++i) {
		    f = s * rv1[i];
		    rv1[i] *= c;
		    if ((IRIT_FABS(f) + anorm) == anorm) 
			break;
		    g = S[i];
		    h = PYTHAG(f, g);
		    S[i] = h;
		    h = 1 / h;
		    c = g * h;
		    s = (-f * h);
		    for (j = 0; j < 3; ++j) {
			y = U[j][nm];
			z = U[j][i];
			U[j][nm] = y * c + z * s;
			U[j][i] = z * c - y * s;
		    }
		}
	    }
	    z = S[k];
	    if (l == k) {                                    /* Convergence. */
		if (z < 0) {           /* Singular val is made non negative. */
		    S[k] = -z;
		    for (j = 0; j < 3; ++j) 
			V[j][k] = -V[j][k];
		}
		break;
	    }
	    if (its == 30) {
	        fprintf(stderr, "No convergence in 30 SVDCMP iterations\n");
		return FALSE;
	    }
	    x = S[l];                        /* Shift from bottom 2x2 minor. */
	    nm = k - 1;
	    y = S[nm];
	    g = rv1[nm];
	    h = rv1[k];
	    f = ((y - z) * (y + z) + (g - h) * (g + h))/(2 * h * y);
	    g = PYTHAG(f, 1);
	    f = ((x - z) * (x + z) +
		 h * ((y / (f + (f >= 0 ? g : -g))) - h)) / x;
	    c = s = 1;                            /* Next QR transformation. */
	    for (j = l; j <= nm; ++j) {
		i = j + 1;
		g = rv1[i];
		y = S[i];
		h = s * g;
		g = c * g;
		z = PYTHAG(f, h);
		rv1[j] = z;
		c = f / z;
		s = h / z;
		f = x * c + g * s;
		g = g * c - x * s;
		h = y * s;
		y = y * c;
		for (jj = 0; jj < 3; ++jj) {
		    x = V[jj][j];
		    z = V[jj][i];
		    V[jj][j] = x * c + z * s;
		    V[jj][i] = z * c - x * s;
		}
		z = PYTHAG(f, h);
		S[j] = z;
		if (z != 0.0) {
		    z = 1 / z;
		    c = f * z;
		    s = h * z;
		}
		f = (c * g) + (s * y);
		x = (c * y) - (s * g);
		for (jj = 0; jj < 3; ++jj) {
		    y = U[jj][j];
		    z = U[jj][i];
		    U[jj][j] = y * c + z * s;
		    U[jj][i] = z * c - y * s;
		}
	    }
	    rv1[l] = 0;
	    rv1[k] = f;
	    S[k] = x;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Perform singular value decomposition on an NxN matrix M as U S V.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:        Source matrix to decompose.                                    M
*   U:        Left orthonormal matrix, size n x n.                           M
*   S:	      A vector holding the singular components.			     M
*   V:        Right orthonormal matrix, size n.                              M
*   n:        size of square matrix, size n x n.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SvdLeastSqr, SvdDecomp, SvdMatrix3x3                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SvdMatrixNxN, singular value decomposition, linear systems, matrices     M
*****************************************************************************/
void SvdMatrixNxN(IrtRType *M, IrtRType *U, IrtRType *S, IrtRType *V, int n)
{
    int i, j;
    IrtRType **SVD_U, **SVD_V, *SVD_W;

    SVD_U = (IrtRType **) IritMalloc((n + 1) * sizeof(IrtRType *));
    SVD_V = (IrtRType **) IritMalloc((n + 1) * sizeof(IrtRType *));
    for (i = 0; i <= n; i++) {
        SVD_U[i] = SVD_VECTOR(n);
        SVD_V[i] = SVD_VECTOR(n);
    }
    SVD_W = SVD_VECTOR(1 + n);

    for (j = 0; j < n; j++)
        for (i = 0; i < n; i++)
	    SVD_U[i + 1][j + 1] = M[i + j * n];

    SvdDecomp(SVD_U, n, n, SVD_W, SVD_V);
    for (j = 0; j < n; j++) {
        S[j] = SVD_W[j + 1];
        for (i = 0; i < n; i++) {
	    U[i + j * n] = SVD_U[i + 1][j + 1];
	    V[i + j * n] = SVD_V[i + 1][j + 1];
	}
    }

    for (i = 0; i <= n; i++) {
        SVD_FREE_VEC(SVD_U[i]);
	SVD_FREE_VEC(SVD_V[i]);
    }
    IritFree(SVD_U);
    IritFree(SVD_V);
    IritFree(SVD_W);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   SVD decomposition of matrix A as U W V, U of size m x m, V of size n x n M
* and W is diagonal with the signular values, stored as a vector.            M
*   Due to the Fortran style of this code all indices are from 1 to k, so    M
* an array from 1 to k actually have k + 1 elements in this C translation.   M
*                                                                            *
* PARAMETERS:                                                                M
*   a:      Input matrix also serving to store U orthogonal output, of       M
*	    dimension m x n, m >= n.				             M
*   m, n:   The dimensions of a.					     M
*   w:      The diagonal singular values, as a vector of length m.	     M
*   v:      The V orthogonal output, dimensions n x n.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SvdMatrixNxN                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SvdDecomp                                                                M
*****************************************************************************/
void SvdDecomp(IrtRType **a, int m, int n, IrtRType *w, IrtRType **v)
{
    int flag, i, its, j, k, l, nm;
    IrtRType c, f, h, s, x, y, z, *rv1,
	anorm = 0.0,
	g = 0.0,
	scale = 0.0;

    if (m < n)
	IRIT_FATAL_ERROR("SVDCMP: You must augment A with extra zero rows");

    rv1 = SVD_VECTOR(n);
    for (i = 1; i <= n; i++) {
	l = i + 1;
	rv1[i] = scale * g;
	g = s = scale = 0.0;
	if (i <= m) {
	    for (k = i; k <= m; k++)
		scale += IRIT_FABS(a[k][i]);
	    if (scale != 0.0) {
		for (k = i; k <= m; k++) {
		    a[k][i] /= scale;
		    s += a[k][i] * a[k][i];
		}
		f = a[i][i];
		g = -SIGN2(sqrt(s), f);
		h = f * g - s;
		a[i][i] = f - g;
		if (i != n) {
		    for (j = l; j <= n; j++) {
			for (s = 0.0, k = i; k <= m; k++)
			    s += a[k][i] * a[k][j];
			f = s / h;
			for (k = i; k <= m; k++)
			    a[k][j] += f * a[k][i];
		    }
		}
		for (k = i; k <= m; k++)
		    a[k][i] *= scale;
	    }
	}
	w[i] = scale * g;

	g = s = scale = 0.0;
	if (i <= m && i != n) {
	    for (k = l; k <= n; k++)
		scale += IRIT_FABS(a[i][k]);
	    if (scale != 0.0) {
		for (k = l; k <= n; k++) {
		    a[i][k] /= scale;
		    s += a[i][k] * a[i][k];
		}
		f = a[i][l];
		g = -SIGN2(sqrt(s), f);
		h = f * g - s;
		a[i][l] = f - g;
		for (k = l; k <= n; k++)
		    rv1[k] = a[i][k] / h;
		if (i != m) {
		    for (j = l; j <= m; j++) {
			for (s = 0.0, k = l; k <= n; k++)
			    s += a[j][k] * a[i][k];
			for (k = l; k <= n; k++)
			    a[j][k] += s * rv1[k];
		    }
		}
		for (k = l; k <= n; k++)
		    a[i][k] *= scale;
	    }
	}
	s = IRIT_FABS(w[i]) + IRIT_FABS(rv1[i]);
	anorm = IRIT_MAX(anorm, s);
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintSvd1, FALSE) {
	    IRIT_INFO_MSG("SVD Stage 1:\n");
	    for (i = 1; i <= m; i++) {
	        for (j = 1; j <= n; j++)
		    IRIT_INFO_MSG_PRINTF("%9.6f ", a[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    for (i = n; i >= 1; i--) {
	if (i < n) {
	    if (g != 0.0) {
		for (j = l; j <= n; j++)
		    v[j][i] = (a[i][j] / a[i][l]) / g;
		for (j = l; j <= n; j++) {
		    for (s = 0.0, k = l; k <= n; k++)
			s += a[i][k] * v[k][j];
		    for (k = l; k <= n; k++)
			v[k][j] += s * v[k][i];
		}
	    }
	    for (j = l; j <= n; j++)
		v[i][j] = v[j][i] = 0.0;
	}
	v[i][i] = 1.0;
	g = rv1[i];
	l = i;
    }
#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintSvd2, FALSE) {
	    IRIT_INFO_MSG_PRINTF("SVD Stage 2:\n");
	    for (i = 1; i <= n; i++) {
	        for (j = 1; j <= n; j++)
		    IRIT_INFO_MSG_PRINTF("%9.6f ", v[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    for (i = n; i >= 1; i--) {
	l = i + 1;
	g = w[i];
	if (i < n)
	    for (j = l;j <= n;j++)
		a[i][j] = 0.0;
	if (g != 0.0) {
	    g = 1.0 / g;
	    if (i != n) {
		for (j = l; j <= n; j++) {
		    for (s = 0.0, k = l; k <= m; k++)
			s += a[k][i] * a[k][j];
		    f = (s / a[i][i]) * g;
		    for (k = i; k <= m; k++)
			a[k][j] += f * a[k][i];
		}
	    }
	    for (j = i; j <= m; j++)
		a[j][i] *= g;
	}
	else {
	    for (j = i; j <= m; j++)
		a[j][i] = 0.0;
	}
	++a[i][i];
    }

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintSvd3, FALSE) {
	    IRIT_INFO_MSG("SVD Stage 3:\n");
	    for (i = 1; i <= m; i++) {
	        for (j = 1; j <= n; j++)
		     IRIT_INFO_MSG_PRINTF("a[%d][%d] = %17.14f\n",
					  i, j, a[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	    IRIT_INFO_MSG("W vector:\n");
	    for (i = 1; i <= m; i++)
	        IRIT_INFO_MSG_PRINTF("w[%d] = %17.14f\n", i, w[i]);
	    IRIT_INFO_MSG("\nRV1 vector:\n");
	    for (i = 1; i <= n; i++)
	        IRIT_INFO_MSG_PRINTF("rv1[%d] = %17.14f ", i, rv1[i]);
	    IRIT_INFO_MSG("\n");
	}
    }
#   endif /* DEBUG */

    for (k = n; k >= 1; k--) {
	for (its = 1; its <= 30; its++) {
	    flag = 1;
	    for (l = k; l >= 1; l--) {
		nm = l-1;

		if (IRIT_FABS(rv1[l]) + anorm == anorm) {
		    flag = 0;
		    break;
		}
		if (IRIT_FABS(w[nm]) + anorm == anorm) {
		    break;
		}
	    }

	    if (flag) {
		c = 0.0;
		s = 1.0;
		for (i = l; i <= k; i++) {
		    f = s * rv1[i];
		    if (IRIT_FABS(f) + anorm != anorm) {
			g = w[i];
			h = PYTHAG(f, g);
			w[i] = h;
			h = 1.0 / h;
			c = g * h;
			s = (-f * h);
			for (j = 1; j <= m; j++) {
			    y = a[j][nm];
			    z = a[j][i];
			    a[j][nm] = y * c + z * s;
			    a[j][i] = z * c - y * s;
			}
		    }
		}
	    }
	    z = w[k];
	    if (l == k) {
		if (z < 0.0) {
		    w[k] = -z;
		    for (j = 1; j <= n; j++)
			v[j][k] = (-v[j][k]);
		}
		break;
	    }

	    if (its == 30) {
	        fprintf(stderr, "No convergence in 30 SVDCMP iterations\n");
		return;
	    }
	    x = w[l];
	    nm = k - 1;
	    y = w[nm];
	    g = rv1[nm];
	    h = rv1[k];
	    f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
	    g = PYTHAG(f, 1.0);
	    f = ((x - z) * (x + z) + h * ((y / (f + SIGN2(g, f))) - h)) / x;
	    c = s = 1.0;
	    for (j = l; j <= nm; j++) {
	        IrtRType **aa, **ab, **vv, **vw;

		i = j + 1;
		g = rv1[i];
		y = w[i];
		h = s * g;
		g = c * g;
		z = PYTHAG(f, h);
		rv1[j] = z;
		c = f / z;
		s = h / z;
		f = x * c + g * s;
		g = g * c - x * s;
		h = y * s;
		y = y * c;

		for (vv = &v[0], vw = &v[n]; ++vv <= vw; ) {
		    (*vv)[j] = (x = (*vv)[j]) * c + (*vv)[i] * s;
		    (*vv)[i] = (*vv)[i] * c - x * s;
		}

		z = PYTHAG(f, h);
		w[j] = z;
		if (z != 0.0) {
		    z = 1.0 / z;
		    c = f * z;
		    s = h * z;
		}
		f = (c * g) + (s * y);
		x = (c * y) - (s * g);

		for (aa = &a[0], ab = &a[m]; ++aa <= ab; ) {
		    (*aa)[j] = (y = (*aa)[j]) * c + (*aa)[i] * s;
		    (*aa)[i] = (*aa)[i] * c - y * s;
		}
	    }
	    rv1[l] = 0.0;
	    rv1[k] = f;
	    w[k] = x;
	}
    }
    SVD_FREE_VEC(rv1);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintSvd9, FALSE) {
	    IRIT_INFO_MSG("SVD Stage 9:\n");
	    for (i = 1; i <= m; i++) {
	        for (j = 1; j <= n; j++)
		    IRIT_INFO_MSG_PRINTF("%9.6f ", a[i][j]);
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Least square solves A x = b.						     M
*   The vector X is of size Nx, vector b is of size NData and matrix A is    M
* of size Nx by NData.							     M
*   Uses singular value decomposition.					     M
*   If A != NULL an SVD decomposition is computed, otherwise (A == NULL) a   M
* solution is computed for the given b and is placed in x.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   A:         The matrix of size Nx by NData.                               M
*   x:         The vector of sought solution of size Nx.                     M
*   b:         The vector of coefficients of size NData.                     M
*   NData, Nx:  Dimensions of input.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The reciprocal of the condition number, if A != NULL, zero    M
*	       otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SvdMatrix4x4                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SvdLeastSqr, singular value decomposition, linear systems                M
*****************************************************************************/
IrtRType SvdLeastSqr(IrtRType *A, IrtRType *x, IrtRType *b, int NData, int Nx)
{
    IRIT_STATIC_DATA IrtRType
	**SVD_U = NULL,
	**SVD_V = NULL,
	*SVD_W = NULL;
    int i, j;

    if (A != NULL) {
	IrtRType Min, Max;

	if (SVD_U != NULL) {		 /* Free old instance of aux. data. */
	    IritFree(SVD_U[0]);
	    IritFree(SVD_U);
	    IritFree(SVD_V[0]);
	    IritFree(SVD_V);
	    SVD_FREE_VEC(SVD_W);
	}

	SVD_U = (IrtRType **) IritMalloc((NData + 1) * sizeof(IrtRType *));
	SVD_V = (IrtRType **) IritMalloc((Nx + 1) * sizeof(IrtRType *));
	SVD_W = SVD_VECTOR(1 + IRIT_MAX(NData, Nx));

	SVD_U[0] = (IrtRType *) IritMalloc((NData + 1) * (Nx + 1) * 
					                    sizeof(IrtRType));
	for (i = 1; i <= NData; i++)
	    SVD_U[i] = SVD_U[i - 1] + Nx + 1;

	SVD_V[0] = (IrtRType *) IritMalloc((Nx + 1) * (Nx + 1) * 
					                    sizeof(IrtRType));
	for (i = 1; i <= Nx; i++)
	    SVD_V[i] = SVD_V[i - 1] + Nx + 1;

	for (i = 0; i < NData; i++)
	    IRIT_GEN_COPY(&SVD_U[i + 1][1], &A[i * Nx], sizeof(IrtRType) * Nx);

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintSvdInput, FALSE) {
		int i, j;

		IRIT_INFO_MSG("\nThe Input Matrix:\n");
		for (i = 1; i <= NData; i++) {
		    for (j = 1; j <= Nx; j++)
		        IRIT_INFO_MSG_PRINTF("%16.14f ", SVD_U[i][j]);
		    IRIT_INFO_MSG("\n");
		}
	    }
	}
#	endif /* DEBUG */

	SvdDecomp(SVD_U, NData, Nx, SVD_W, SVD_V);

	Min = Max = SVD_W[1];
	for (i = 2; i <= Nx; i++) {
	    if (Min > SVD_W[i])
	        Min = SVD_W[i];
	    if (Max < SVD_W[i])
	        Max = SVD_W[i];

	}

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintSvdUVW, FALSE) {
	        int k,
		    NMax = IRIT_MAX(NData, Nx);
		IrtRType
		    **Mult = (IrtRType **) IritMalloc((NMax + 1) *
						      sizeof(IrtRType *));

		for (i = 0; i <= NMax; i++)
		    Mult[i] = SVD_VECTOR(NMax);
		    
		IRIT_INFO_MSG("\nThe W vector:\n");
		for (i = 1; i <= Nx; i++)
		    IRIT_INFO_MSG_PRINTF("%9.6f ", SVD_W[i]);

		IRIT_INFO_MSG("\n\nThe U matrix:\n");
		for (i = 1; i <= NData; i++) {
		    for (j = 1; j <= Nx; j++)
		        IRIT_INFO_MSG_PRINTF("%9.6f ", SVD_U[i][j]);
		    IRIT_INFO_MSG("\n");
		}

		IRIT_INFO_MSG("\nThe V matrix:\n");
		for (i = 1; i <= Nx; i++) {
		    for (j = 1; j <= Nx; j++)
		        IRIT_INFO_MSG_PRINTF("%9.6f ", SVD_V[i][j]);
		    IRIT_INFO_MSG("\n");
		}
		IRIT_INFO_MSG_PRINTF("\nCondition number: %.14f (%.14f,  %.14f)\n",
		                        Min / Max, Min, Max);

		/* Multiply matrices to see if we restore original matrix. */
		for (i = 1; i <= NData; i++)
		    for (j = 1; j <= Nx; j++)
		        Mult[i][j] = SVD_U[i][j] * SVD_W[j];

			IRIT_INFO_MSG("\nThe UWV product matrix:\n");
		for (i = 1; i <= NData; i++) {
		    for (j = 1; j <= Nx; j++) {
		        IrtRType
			    Val = 0.0;

			for (k = 1; k <= Nx; k++)
			    Val += Mult[i][k] * SVD_V[j][k];/* V^T actually! */

			IRIT_INFO_MSG_PRINTF("%9.6f ", Val);
		    }
		    IRIT_INFO_MSG("\n");
		}

		for (i = 0; i <= NMax; i++)
		    SVD_FREE_VEC(Mult[i]);
		IritFree(Mult);
	    }
	}
#	endif /* DEBUG */

#	ifdef DEBUG
	{
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugSVDTestValidity, FALSE) {
	        int k,
		    NMax = IRIT_MAX(NData, Nx);
		IrtRType
		    **Mult = (IrtRType **) IritMalloc((NMax + 1) *
						      sizeof(IrtRType *));

		for (i = 0; i <= NMax; i++)
		    Mult[i] = SVD_VECTOR(NMax);

		for (i = 1; i <= NData; i++)
		    for (j = 1; j <= Nx; j++)
		        Mult[i][j] = SVD_U[i][j] * SVD_W[j];

		for (i = 1; i <= NData; i++) {
		    for (j = 1; j <= Nx; j++) {
		        IrtRType
			    Val = 0.0;

			for (k = 1; k <= Nx; k++)
			    Val += Mult[i][k] * SVD_V[j][k];/* V^T actually! */

			if (!IRIT_APX_EQ(A[(i - 1) * Nx + (j - 1)], Val))
			    return -IRIT_INFNTY;
		    }
		}

		for (i = 0; i <= NMax; i++)
		    SVD_FREE_VEC(Mult[i]);
		IritFree(Mult);
	    }
	}
#	endif /* DEBUG */

	return Min / Max; /* The reciprocal of the codition number. */
    }
    else {
	IRIT_STATIC_DATA int 
	    TVecSize = 0;
	IRIT_STATIC_DATA IrtRType
	    *TVec = NULL;

	if (x == NULL) {
	    /* Free everything - we are done here. */
	    if (SVD_U != NULL) {	 /* Free old instance of aux. data. */
	        IritFree(SVD_U[0]);
		IritFree(SVD_U);
		IritFree(SVD_V[0]);
		IritFree(SVD_V);
		SVD_FREE_VEC(SVD_W);
	    }
	    SVD_U = SVD_V = NULL;
	    SVD_W = NULL;
	}

	if (TVecSize < Nx) {
	    if (TVec != NULL)
		SVD_FREE_VEC(TVec);

	    TVecSize = Nx * 2;
	    TVec = SVD_VECTOR(TVecSize);
	}

	for (j = 1; j <= Nx; j++) {
	    IrtRType
		s = 0.0;

	    if (SVD_W[j]) {
		for (i = 1; i <= NData; i++)
		    s += SVD_U[i][j] * b[i - 1];
		s /= SVD_W[j];
	    }
	    TVec[j] = s;
	}
	for (j = 1; j <= Nx; j++) {
	    IrtRType
		s = 0.0;
	    
	    for (i = 1; i <= Nx; i++)
		s += SVD_V[j][i] * TVec[i];
	    x[j - 1] = s;
	}

	return 0.0;
    }
}
