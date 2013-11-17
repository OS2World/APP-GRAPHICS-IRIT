/******************************************************************************
* Numerical Recipes' Jacobi code to diagonalize a matrix.		      *
* "Numerical Recipes in C", by William H. Press el al., University of	      *
* Cambridge, 1988, ISBN 0521-35465-X.					      *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "irit_sm.h"
#include "misc_lib.h"
#include "extra_fn.h"

static void JacobiNR(IrtRType *a[],
		     int n,
		     IrtRType *d,
		     IrtRType *v[],
		     int *nrot);

#define JACOBI_NR_ROTATE(a,i,j,k,l) g=a[i][j]; \
				    h=a[k][l]; \
	                            a[i][j]=g-s*(h+g*tau); \
		                    a[k][l]=h+s*(g-h*tau);
#define JACOBI_NR_MALLOC_VECTOR(n)  IritMalloc(sizeof(IrtRType) * (n + 1))
#define JACOBI_NR_FREE_VECTOR(p)  IritFree((p))

/*****************************************************************************
* DESCRIPTION:                                                               M
* Performs a diagonalization of a NxN matrix, as U D V, D diagonal and U and M
* V orthogonal matrices U = V^-1 = V^T.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   M:      Matrix to diagonalize.                                           M
*   U:      The left orthogonal (=rotation) matrix.                          M
*   D:      The diagonal matrix.                                             M
*   V:      The eight orthogonal matrix.                                     M
*   n:      Dimension of the matrices as n times n.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   JacobiMatrixDiag4x4                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   JacobiMatrixDiagNxN                                                      M
*****************************************************************************/
void JacobiMatrixDiagNxN(IrtRType *M[],
			 IrtRType *U[],
			 IrtRType *D[],
			 IrtRType *V[],
			 int n)
{
    int i, j, nrot;
    IrtRType
	**MM = JACOBI_NR_MALLOC_VECTOR(n),
	*DD = JACOBI_NR_MALLOC_VECTOR(n),
	**VV = JACOBI_NR_MALLOC_VECTOR(n);

    /* Allocate some local memory. */
    for (i = 0; i <= n; i++) {
	MM[i] = JACOBI_NR_MALLOC_VECTOR(n);
	VV[i] = JACOBI_NR_MALLOC_VECTOR(n);
    }
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
	    MM[j + 1][j + 1] = M[i][j];

    /* Go to the real work. */
    JacobiNR(MM, 4, DD, VV, &nrot);
    IRIT_ZAP_MEM(D, n * n * sizeof(IrtRType));
    for (i = 0; i < n; i ++) {
        D[i][i] = DD[i + 1];
	for (j = 0; j < n; j ++)
	    U[i][j] = V[j][i] = VV[i + 1][j + 1];
    }

    /* Free local memory. */
    for (i = 0; i <= n; i++) {
	JACOBI_NR_FREE_VECTOR(MM[i]);
	JACOBI_NR_FREE_VECTOR(VV[i]);
    }
    JACOBI_NR_FREE_VECTOR(MM);
    JACOBI_NR_FREE_VECTOR(VV);
    JACOBI_NR_FREE_VECTOR(DD);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* performs a diagonalization of a 4x4 matrix, as U D V, D diagonal and U and M
* V orthogonal matrices U = V^-1 = V^T.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   M:      Matrix to diagonalize.                                           M
*   U:      The left orthogonal (=rotation) matrix.                          M
*   D:      The diagonal matrix.                                             M
*   V:      The eight orthogonal matrix.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   JacobiMatrixDiagNxN                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   JacobiMatrixDiag4x4                                                      M
*****************************************************************************/
void JacobiMatrixDiag4x4(IrtRType M[4][4],
			 IrtRType U[4][4],
			 IrtRType D[4][4],
			 IrtRType V[4][4])
{
    int i, j, nrot;
    IrtRType aa[5][5], vv[5][5], d[5], *a[5], *v[5];

    for (i = 0; i < 5; i++) {
        a[i] = aa[i];
        v[i] = vv[i];
    }

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    a[i + 1][j + 1] = M[i][j];
    JacobiNR(a, 4, d, v, &nrot);
    IRIT_ZAP_MEM(D, 16 * sizeof (IrtRType));
    for (i = 0; i < 4; i ++) {
        D[i][i] = d[i + 1];
	for (j = 0; j < 4; j ++)
	    U[i][j] = V[j][i] = v[i + 1][j + 1];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  From "Numerical Recipes"; Changed function prototype to ease interface.   *
*                                                                            *
* PARAMETERS:                                                                *
*   a:    N.S.F.I.                                                           *
*   n:    N.S.F.I.                                                           *
*   d:    N.S.F.I.                                                           *
*   v:    N.S.F.I.                                                           *
*   nrot: N.S.F.I.                                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void JacobiNR(IrtRType *a[],
		     int n,
		     IrtRType *d,
		     IrtRType *v[],
		     int *nrot)
{
    int j, iq, ip, i;
    IrtRType tresh, theta, tau, t, sm, s, h, g, c, *b, *z;

    b = JACOBI_NR_MALLOC_VECTOR(n);
    z = JACOBI_NR_MALLOC_VECTOR(n);
    for (ip = 1; ip <= n; ip++) {
        for (iq = 1; iq <= n; iq++) v[ip][iq] = 0.0;
	v[ip][ip] = 1.0;
    }
    for (ip = 1; ip <= n; ip++) {
        b[ip] = d[ip] = a[ip][ip];
	z[ip] = 0.0;
    }
    *nrot = 0;
    for (i = 1; i <= 50; i++) {
        sm = 0.0;
	for (ip = 1; ip <= n - 1; ip++) {
	    for (iq = ip + 1; iq <= n; iq++)
	        sm += IRIT_FABS(a[ip][iq]);
	}
	if (sm == 0.0) {
	    JACOBI_NR_FREE_VECTOR(z);
	    JACOBI_NR_FREE_VECTOR(b);
	    return;
	}
	if (i < 4)
	    tresh = 0.2 * sm / (n*n);
	else
	    tresh = 0.0;
	for (ip = 1; ip <= n-1; ip++) {
	    for (iq = ip+1; iq <= n; iq++) {
	        g = 100.0 * IRIT_FABS(a[ip][iq]);
		if (i > 4 && IRIT_FABS(d[ip]) + g == IRIT_FABS(d[ip])
		    && IRIT_FABS(d[iq]) + g == IRIT_FABS(d[iq]))
		    a[ip][iq] = 0.0;
		else if (IRIT_FABS(a[ip][iq]) > tresh) {
		    h = d[iq] - d[ip];
		    if (IRIT_FABS(h) + g == IRIT_FABS(h))
		        t = (a[ip][iq]) / h;
		    else {
		        theta = 0.5 * h / a[ip][iq];
			t = 1.0 / (IRIT_FABS(theta) + sqrt(1.0 + theta * theta));
			if (theta < 0.0)
			    t = -t;
		    }
		    c = 1.0 / sqrt(1 + t * t);
		    s = t * c;
		    tau = s / (1.0 + c);
		    h = t * a[ip][iq];
		    z[ip] -= h;
		    z[iq] += h;
		    d[ip] -= h;
		    d[iq] += h;
		    a[ip][iq] = 0.0;
		    for (j = 1; j <= ip-1; j++) {
		        JACOBI_NR_ROTATE(a, j, ip, j, iq)
		    }
		    for (j = ip+1; j <= iq-1; j++) {
		        JACOBI_NR_ROTATE(a, ip, j, j, iq)
		    }
		    for (j = iq+1; j <= n; j++) {
		        JACOBI_NR_ROTATE(a, ip, j, iq, j)
		    }
		    for (j = 1; j <= n; j++) {
		        JACOBI_NR_ROTATE(v, j, ip, j, iq)
		    }
		    ++(*nrot);
		}
	    }
	}
	for (ip = 1; ip <= n; ip++) {
	    b[ip] += z[ip];
	    d[ip] = b[ip];
	    z[ip] = 0.0;
	}
    }
    IRIT_FATAL_ERROR("Too many iterations in Jacobi routine");
}

#ifdef DEBUG_DIAG_MATRIX

void mul(IrtRType mat1[4][4], IrtRType mat2[4][4], IrtRType mat3[4][4])
{
    int i, j, k;

    IRIT_ZAP_MEM(mat3, 16 * sizeof(IrtRType));
    for (i = 0; i < 4; i ++)
	for (j = 0; j < 4; j ++)
	    for (k = 0; k < 4; k ++)
	        mat3 [i] [j] += mat1 [i] [k] * mat2 [k] [j];
}

void main(void)
{
    int i, j;
    IrtRType M[4][4], U[4][4], D[4][4], V[4][4], tmp1[4][4], tmp2[4][4];

    printf ("Mat (0,0): ");
    scanf ("%lf", & M [0] [0]);
    printf ("Mat (0,1): ");
    scanf ("%lf", & M [0] [1]);
    M [1] [0] = M [0] [1];
    printf ("Mat (0,2): ");
    scanf ("%lf", & M [0] [2]);
    M [2] [0] = M [0] [2];
    printf ("Mat (0,3): ");
    scanf ("%lf", & M [0] [3]);
    M [3] [0] = M [0] [3];
    printf ("Mat (1,1): ");
    scanf ("%lf", & M [1] [1]);
    printf ("Mat (1,2): ");
    scanf ("%lf", & M [1] [2]);
    M [2] [1] = M [1] [2];
    printf ("Mat (1,3): ");
    scanf ("%lf", & M [1] [3]);
    M [3] [1] = M [1] [3];
    printf ("Mat (2,2): ");
    scanf ("%lf", & M [2] [2]);
    printf ("Mat (2,3): ");
    scanf ("%lf", & M [2] [3]);
    M [3] [2] = M [2] [3];
    printf ("Mat (3,3): ");
    scanf ("%lf", & M [3] [3]);

    JacobiMatrixDiag4x4(M, U, D, V);

    printf ("\nOriginal matrix M:\n           0         1         2\n");
    for (i = 0; i < 4; i ++) {
        printf ("%3d  ", i);
	for (j = 0; j < 4; j ++)
           printf (" %9.3f", M [i] [j]);
	printf ("\n");
    }
    printf ("\nMatrix U:\n           0         1         2\n");
    for (i = 0; i < 4; i ++) {
	printf ("%3d  ", i);
	for (j = 0; j < 4; j ++)
	    printf (" %9.3f", U [i] [j]);
	printf ("\n");
    }
    printf ("\nMatrix D:\n           0         1         2\n");
    for (i = 0; i < 4; i ++) {
	printf ("%3d  ", i);
	for (j = 0; j < 4; j ++)
	    printf (" %9.3f", D [i] [j]);
	printf ("\n");
    }
    printf ("\nMatrix V:\n           0         1         2\n");
    for (i = 0; i < 4; i ++) {
	printf ("%3d  ", i);
	for (j = 0; j < 4; j ++)
	    printf (" %9.3f", V [i] [j]);
	printf ("\n");
    }
    mul(U, D, tmp1);
    mul(tmp1, V, tmp2);
    printf("\nMatrix UDV:\n           0         1         2\n");
    for (i = 0; i < 4; i ++) {
	printf ("%3d  ", i);
	for (j = 0; j < 4; j ++)
	    printf (" %9.3f", tmp2 [i] [j]);
	printf ("\n");
    }
}

#endif /* DEBUG_DIAG_MATRIX */
