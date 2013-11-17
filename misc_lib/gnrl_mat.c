/*****************************************************************************
* General matrix manipulation routines.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			        Ver 1.0, Dec 2005    *
*****************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"
#include "extra_fn.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to copy a (n x n) matrix:		                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Dst:      Matrix to copy to.			                     M
*   Src:      Matrix to copy from.			                     M
*   n:	      Size of matrix Mat.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlCopy, transformations		                             M
*****************************************************************************/
void MatGnrlCopy(IrtGnrlMatType Dst, IrtGnrlMatType Src, int n)
{
    IRIT_GEN_COPY(Dst, Src, sizeof(IrtRType) * IRIT_SQR(n));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a (n x n) unit matrix:                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:      Matrix to initialize as a unit matrix.                         M
*   n:	      Size of matrix Mat.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGenUnitMat                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlUnitMat, transformations, unit matrix                             M
*****************************************************************************/
void MatGnrlUnitMat(IrtGnrlMatType Mat, int n)
{
    int i, j;

    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
	    if (i == j)
		*Mat++ = 1.0;
	    else
		*Mat++ = 0.0;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if the given matrix is a unit matrix to within Eps.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:    Matrix to test if a unit matrix.                                 M
*   Eps:    Epsilon of test.					             M
*   n:	    Size of matrix Mat.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if unit matrix to within epsilon, FALSE otherwise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlUnitMat, MatIsUnitMatrix                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlIsUnitMatrix                                                      M
*****************************************************************************/
int MatGnrlIsUnitMatrix(IrtGnrlMatType Mat, IrtRType Eps, int n)
{
    int i, j;

    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++, Mat++) {
	    if (!IRIT_APX_EQ_EPS(*Mat, i == j, Eps))
	        return FALSE;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to multiply two general matrices.                                M
* MatRes may be one of Mat1 or Mat2 - it is only updated in the end.         M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix product.                                   M
*   Mat1, Mat2:  The two operand of the matrix product.                      M
*   n:	         Size of matrices MatRes/Mat1/Mat2.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatMultTwoMat, MatGnrlAddTwoMat, MatGnrlSubTwoMat                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlMultTwoMat, transformations, matrix product                       M
*****************************************************************************/
void MatGnrlMultTwoMat(IrtGnrlMatType MatRes,
		       IrtGnrlMatType Mat1,
		       IrtGnrlMatType Mat2,
		       int n)
{
    int i, j, k, MatSize,
	ColRowSize = sizeof(IrtRType) * n;
    IrtGnrlMatType Mat1P, Mat2P,
	MatResTemp = (IrtGnrlMatType) IritMalloc(MatSize = ColRowSize * n),
	MatResP = MatResTemp;

    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
	    Mat1P = &Mat1[j * n];
	    Mat2P = &Mat2[i];
	    *MatResP = 0.0;
	    for (k = 0; k < n; k++, Mat2P += n)
	        *MatResP += *Mat1P++ * *Mat2P;
	    MatResP++;
	}
    }

    IRIT_GEN_COPY(MatRes, MatResTemp, MatSize);
    IritFree(MatResTemp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to add two general matrices.				     M
* MatRes may be one of Mat1 or Mat2.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix addition.                                  M
*   Mat1, Mat2:  The two operand of the matrix addition.                     M
*   n:	         Size of matrices MatRes/Mat1/Mat2.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatAddTwoMat, MatGntlMultTwomat, MatGnrlSubTwoMat, MatGnrlScaleMat       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlAddTwoMat, transformations, matrix addition                       M
*****************************************************************************/
void MatGnrlAddTwoMat(IrtGnrlMatType MatRes,
		      IrtGnrlMatType Mat1,
		      IrtGnrlMatType Mat2,
		      int n)
{
    int i;

    for (i = IRIT_SQR(n); i > 0; i--)
	*MatRes++ = *Mat1++ + *Mat2++;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to subtract two general matrices.				     M
*   MatRes may be one of Mat1 or Mat2.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix subtraction.                               M
*   Mat1, Mat2:  The two operand of the matrix subtraction.                  M
*   n:	         Size of matrices MatRes/Mat1/Mat2.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatSubTwoMat, MatGnrlAddTwoMat, MatGntlMultTwoMat, MatGnrlScaleMat       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlSubTwoMat, transformations, matrix subtraction                    M
*****************************************************************************/
void MatGnrlSubTwoMat(IrtGnrlMatType MatRes,
		      IrtGnrlMatType Mat1,
		      IrtGnrlMatType Mat2,
		      int n)
{
    int i;

    for (i = IRIT_SQR(n); i > 0; i--)
	*MatRes++ = *Mat1++ - *Mat2++;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to scale a general matrix, by a scalar value.		     M
* MatRes may be Mat.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix scaling.                                   M
*   Mat:         The two operand of the matrix scaling.                      M
*   Scale:       Scalar value to multiple matrix with.                       M
*   n:	         Size of matrices MatRes/Mat.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatScaleMat, MatGnrlAddTwoMat, MatGntlMultTwoMat                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlScaleMat, transformations, matrix scaling                         M
*****************************************************************************/
void MatGnrlScaleMat(IrtGnrlMatType MatRes,
		     IrtGnrlMatType Mat,
		     IrtRType *Scale,
		     int n)
{
    int i;

    for (i = IRIT_SQR(n); i > 0; i--)
	*MatRes++ = *Mat++ * (*Scale);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to mult a vector of len n by a general matrix of size (n x n),   M
* as VecRes = Mat * Vec.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecRes:    Result of vector - matrix product.  Can be Vec.               M
*   Mat:       General matrix.                            	             M
*   Vec:       Vector to transform using Matrix.                             M
*   n:	       Sizes of vectors Vec/VecRes and of matrix Mat.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatMultVecby4by4, MatGnrlMultVecbyMat2			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlMultVecbyMat, transformations, vector matrix product              M
*****************************************************************************/
void MatGnrlMultVecbyMat(IrtVecGnrlType VecRes,
			 IrtGnrlMatType Mat,
			 IrtVecGnrlType Vec,
			 int n)
{
    int i, j, VecSize;
    IrtVecGnrlType V,
	VecTemp = (IrtVecGnrlType) IritMalloc(VecSize = sizeof(IrtRType) * n),
	VT = VecTemp;

    for (j = 0; j < n; j++) {
        *VT = 0.0;
	V = Vec;
	for (i = 0; i < n; i++) {
	    *VT += *Mat++ * *V++;
	}
	VT++;
    }

    IRIT_GEN_COPY(VecRes, VecTemp, VecSize);
    IritFree(VecTemp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to mult a vector of len n by a general matrix of size (n x n),   M
* as VecRes = Vec * Mat.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   VecRes:    Result of vector - matrix product.  Can be Vec.               M
*   Vec:       Vector to transform using Matrix.                             M
*   Mat:       General matrix.                            	             M
*   n:	       Sizes of vectors Vec/VecRes and of matrix Mat.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatMultVecby4by4, MatGnrlMultVecbyMat			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlMultVecbyMat2, transformations, vector matrix product             M
*****************************************************************************/
void MatGnrlMultVecbyMat2(IrtVecGnrlType VecRes,
			  IrtVecGnrlType Vec,
			  IrtGnrlMatType Mat,
			  int n)
{
    int i, j, VecSize;
    IrtVecGnrlType V,
	VecTemp = (IrtVecGnrlType) IritMalloc(VecSize = sizeof(IrtRType) * n),
	VT = VecTemp;
    IrtGnrlMatType MT;

    for (i = 0; i < n; i++) {
        *VT = 0.0;
	MT = &Mat[i];
	V = Vec;
	for (j = 0; j < n; j++, MT += n) {
	    *VT += *MT * *V++;
	}
	VT++;
    }

    IRIT_GEN_COPY(VecRes, VecTemp, VecSize);
    IritFree(VecTemp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Routine to compute the INVERSE of a given matrix M which is not modified. M
* Return TRUE if inverted matrix (InvM) do exists.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:         Original matrix to invert.                                    M
*   InvM:      Inverted matrix will be placed here.                          M
*   n:	       Sizes of matrices M/InvM.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if inverse exists, FALSE otherwise.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatInverseMatrix, MatGnrlTranspMatrix                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlInverseMatrix, transformations, matrix inverse                    M
*****************************************************************************/
int MatGnrlInverseMatrix(IrtGnrlMatType M, IrtGnrlMatType InvM, int n)
{
    int i, j, MatSize,
	ColSize = sizeof(IrtRType) * n;
    IrtGnrlMatType SMP,
	U = (IrtGnrlMatType) IritMalloc(MatSize = ColSize * n),
	SM = (IrtGnrlMatType) IritMalloc(MatSize),
	V = (IrtGnrlMatType) IritMalloc(MatSize);
    IrtVecGnrlType SP,
	S = (IrtVecGnrlType) IritMalloc(ColSize);
      
    SvdMatrixNxN(M, U, S, V, n);

#   ifdef DEBUG_SVD
    {
        /* Convert diagonal of singular values into a full inverted matrix. */
        for (i = 0, SP = S, SMP = SM; i < n; i++) {
	    for (j = 0; j < n; j++) {
	        if (i == j)
		    *SMP++ = *SP++;
		else
		    *SMP++ = 0.0;
	    }
	}
	MatGnrlTranspMatrix(U, U, n);

	printf("U\n");
	MatGnrlPrintMatrix(U, n, stdout);
	printf("S\n");
	MatGnrlPrintMatrix(SM, n, stdout);
	printf("V\n");
	MatGnrlPrintMatrix(V, n, stdout);
         
	MatGnrlMultTwoMat(InvM, U, SM, n);
	MatGnrlMultTwoMat(InvM, InvM, V, n);

	MatGnrlTranspMatrix(InvM, InvM, n);
	printf("SVD test\n");
	MatGnrlPrintMatrix(InvM, n, stdout);

	MatGnrlTranspMatrix(U, U, n); /* Transpose back. */
    }
#   endif /* DEBUG_SVD */

    MatGnrlTranspMatrix(V, V, n);

    /* Convert diagonal of singular values into a full inverted matrix. */
    for (i = 0, SP = S, SMP = SM; i < n; i++) {
        for (j = 0; j < n; j++) {
	    if (i == j) {
	        if (*SP == 0.0) {
		    IritFree(U);
		    IritFree(S);
		    IritFree(SM);
		    IritFree(V);
		    return FALSE;
		}
		*SMP++ = 1.0 / *SP++;
	    }
	    else
		*SMP++ = 0.0;
	}
    }

    MatGnrlMultTwoMat(InvM, V, SM, n);
    MatGnrlMultTwoMat(InvM, InvM, U, n);
    MatGnrlTranspMatrix(InvM, InvM, n);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestGnrlMatInverse, FALSE) {
	    MatGnrlMultTwoMat(SM, M, InvM, n);
	    if (!MatGnrlIsUnitMatrix(SM, IRIT_EPS, n)) {
	        printf("Inverse matrix failed.\n");
	    }
	}
    }	
#   endif /* DEBUG */

    IritFree(U);
    IritFree(S);
    IritFree(SM);
    IritFree(V);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to compute the TRANSPOSE of matrix M which is not modified.      M
*                                                                            *
* PARAMETERS:                                                                M
*   M:         Original matrix to transpose.                                 M
*   TranspM:   Transposed matrix will be placed here.                        M
*   n:	       Sizes of matrices M/TranspM.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatTranspMatrix, MatGnrlTranspMatrix                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlTranspMatrix, transformations, matrix transpose                   M
*****************************************************************************/
void MatGnrlTranspMatrix(IrtGnrlMatType M, IrtGnrlMatType TranspM, int n)
{
    int i, j;

    for (j = 0; j < n; j++) {
        for (i = j; i < n; i++) {
	    IrtRType
		t = TranspM[j * n + i];

	    TranspM[j * n + i] = M[i * n + j];
	    M[i * n + j] = t;

	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a general matrix, computes its determinant, recorsively.  Note     M
* this process is exponential as a function of n (as you might expect) so    M
* use it with care (for small matrices only).				     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:       The square matrix of size (n x n) to compute its determinant.   M
*   n:       Size of the Matrix.     					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Value of the determinant.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlInverseMatrix, MatGnrlTranspMatrix                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlDetMatrix                                                         M
*****************************************************************************/
IrtRType MatGnrlDetMatrix(IrtGnrlMatType M, int n)
{
    int i, j, j1, j2;
    IrtRType Det;
    IrtGnrlMatType Minor;

    if (n == 1)
        return M[0];
    else if (n == 2)
        return M[0] * M[3] - M[1] * M[2];

    Minor = (IrtGnrlMatType) IritMalloc(IRIT_SQR(n) * sizeof(IrtRType));

    for (Det = 0.0, j1 = 0; j1 < n; j1++) {
        for (j2 = 0, i = 1; i < n; i++) {
	    int ni = n * i;

	    for (j = 0; j < n; j++) {
	        if (j == j1) 
		    continue;
		else
		    Minor[j2++] = M[ni + j];
	    }
	}
	Det += ((j1 & 0x01) ? -M[j1] : M[j1]) * MatGnrlDetMatrix(Minor, n - 1);
    }

    IritFree(Minor);

    return Det;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given m (m < n) vectors as rows in matrix M, and zeros at the rest of    M
* the rows, computes n-m new vectors of the orthogonal space to the space    M
* spanned by the input m vectors.   Uses SVD in the computation and saves    M
* the new, orthogonal, vectors in the last n-m rows of M, in place.          M
*                                                                            *
* PARAMETERS:                                                                M
*   M:     A matrix holding m (m < n) vectors as rows.                       M
*   n:     Size of marix M (and the space).				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlOrthogonalSubspace                                                M
*****************************************************************************/
int MatGnrlOrthogonalSubspace(IrtGnrlMatType M, int n)
{
    int m, MatSize,
	ColSize = sizeof(IrtRType) * n;
    IrtGnrlMatType
	U = (IrtGnrlMatType) IritMalloc(MatSize = ColSize * n),
	V = (IrtGnrlMatType) IritMalloc(MatSize);
    IrtVecGnrlType
	S = (IrtVecGnrlType) IritMalloc(ColSize);

    /* Compute m, the number of non zero rows in M. */
    for (m = 0; m < n; m++) {
        int j,
	    nm = n * m;

	for (j = 0; j < n; j++) {
	    if (!IRIT_APX_EQ(M[nm + j], 0.0))
	        break;
	}
	if (j >= n)
	    break;
    }
    if (m == n)
        return FALSE;

    SvdMatrixNxN(M, U, S, V, n);


#   ifdef DEBUG
    {
        int i, j, l;
	IrtRType *p, *q;

        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTestOrthoSubSpace, FALSE) {
	    /* Verify the last n-m vectors of V are part of a unit matrix. */
	    for (i = m; i < n; i++) {
	        int ni = n * i;

	        for (j = 0; j < n; j++) {
		    if (!IRIT_APX_EQ(V[ni + j], (i == j)))
		        printf("OrthoSubspace: V returned wrong vectors\n");
		}
	    }

	    /* Verify the first m vectors are the same as the input. */
	    for (i = 0, p = V, q = S; i < n; i++, q++) {
	        for (j = 0; j < n; j++) {
		    *p++ *= *q;
		}
	    }
	    MatGnrlTranspMatrix(V, V, n);
	    MatGnrlMultTwoMat(V, V, U, n);
	    for (i = 0; i < m; i++) {
	        IrtRType
		    *V1 = &V[i * n],
		    *V2 = &M[i * n];

	        for (j = 0; j < n; j++) {
		    if (!IRIT_APX_EQ(V1[j], V2[j]))
		        printf("OrthoSubspace: Failed to recover original\n");
		}
	    }

	    /* Verify the last n-m rows of U are orthogonal to input. */
	    for (i = m; i < n; i++) {                /* Last n-m rows of U. */
	        for (l = 0; l < m; l++) {             /* First m rows of M. */
		    IrtRType
			R = 0.0,
		        *V1 = &U[i * n],
		        *V2 = &M[l * n];
		    
		    for (j = 0; j < n; j++)
			R += V1[j] * V2[j];
		
		    if (!IRIT_APX_EQ(R, 0.0))
		        printf("OrthoSubspace: Failed to compute ortho vec\n");
		}
	    }
	}
    }	
#   endif /* DEBUG */

    /* Copy the new orthogonal vectors back to M. */
    IRIT_GEN_COPY(&M[n * m], &U[n * m], sizeof(IrtRType) * n * (n - m));


    IritFree(U);
    IritFree(S);
    IritFree(V);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to print matrix M to file F.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:         Matrix to print.		                                     M
*   n:	       Size of matrix Mat.					     M
*   F:         What file to print to.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGnrlPrintMatrix, transformations, matrix transpose                    M
*****************************************************************************/
void MatGnrlPrintMatrix(IrtGnrlMatType M, int n, FILE *F)
{
    int i, j;

    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
	    fprintf(F, "%10.7f ", M[j * n + i]);
	}
	fprintf(F, "\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Computes a 2x2 determinant.				                     M
*                                                                            *
* PARAMETERS:                                                                M
*   a11, a12, a21, a22:    Coefficients of the 2x2 matrix.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The determinant.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*    Mat2x2Determinant                                                       M
*****************************************************************************/
IrtRType Mat2x2Determinant(IrtRType a11,
			   IrtRType a12, 
			   IrtRType a21,
			   IrtRType a22)
{
    return a11 * a22 - a21 * a12;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Computes 3x3 determinant.				                     M
*                                                                            *
* PARAMETERS:                                                                M
*   a11, a12, a13, a21, a22, a23, a31, a32, a33: Coefficients of 3x3 matrix. M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:  The determinant.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   Mat3x3Determinant                                                        M
*****************************************************************************/
IrtRType Mat3x3Determinant(IrtRType a11,
			   IrtRType a12,
			   IrtRType a13,
			   IrtRType a21,
			   IrtRType a22,
			   IrtRType a23,
			   IrtRType a31,
			   IrtRType a32,
			   IrtRType a33)
{
    return a11 * Mat2x2Determinant(a22, a23, a32, a33) 
         - a12 * Mat2x2Determinant(a21, a23, a31, a33) 
         + a13 * Mat2x2Determinant(a21, a22, a31, a32);
}
