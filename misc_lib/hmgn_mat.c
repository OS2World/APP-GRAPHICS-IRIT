/*****************************************************************************
* 4x4 homogeneous matrix manipulation routines.				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			       Ver 1.0,	June 1988    *
*****************************************************************************/

#include <math.h>
#include "irit_sm.h"
#include "misc_loc.h"
#include "extra_fn.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 unit matrix:                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:      Matrix to initialize as a unit matrix.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlUnitMat                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenUnitMat, transformations, unit matrix                              M
*****************************************************************************/
void MatGenUnitMat(IrtHmgnMatType Mat)
{
    int i, j;

    for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
	    if (i == j)
		Mat[i][j] = 1.0;
	    else
		Mat[i][j] = 0.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Test if the given matrix is a unit matrix to within Eps.                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:  Matrix to test if a unit matrix.                                   M
*   Eps:  Epsilon of test.					             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     TRUE if unit matrix to within epsilon, FALSE otherwise.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGenUnitMat, MatGnrlIsUnitMatrix                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatIsUnitMatrix                                                          M
*****************************************************************************/
int MatIsUnitMatrix(IrtHmgnMatType Mat, IrtRType Eps)
{
    int i, j;

    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++) {
	    if (!IRIT_APX_EQ_EPS(Mat[i][j], i == j, Eps))
	        return FALSE;
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to translate in Tx, Ty, Tz amounts.       M
*									     *
* PARAMETERS:                                                                M
*   Tx, Ty, Tz:  Translational amounts requested.                            M
*   Mat:         Matrix to initialize as a translation matrix.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatTrans, transformations, translation                             M
*****************************************************************************/
void MatGenMatTrans(IrtRType Tx, IrtRType Ty, IrtRType Tz, IrtHmgnMatType Mat)
{
    MatGenUnitMat(Mat);                             /* Make it unit matrix. */
    Mat[3][0] = Tx;
    Mat[3][1] = Ty;
    Mat[3][2] = Tz;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Scale x, y, z in Sx, Sy, Sz amounts.   M
*									     *
* PARAMETERS:                                                                M
*   Sx, Sy, Sz:  Scaling factors requested.                                  M
*   Mat:         Matrix to initialize as a scaling matrix.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatScale, transformations, scaling                                 M
*****************************************************************************/
void MatGenMatScale(IrtRType Sx, IrtRType Sy, IrtRType Sz, IrtHmgnMatType Mat)
{
    MatGenUnitMat(Mat);                              /* Make it unit matrix. */
    Mat[0][0] = Sx;
    Mat[1][1] = Sy;
    Mat[2][2] = Sz;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to uniformly scale Scale amount.	     M
*									     *
* PARAMETERS:                                                                M
*   Scale:       Uniform scaling factor requested.                           M
*   Mat:         Matrix to initialize as a scaling matrix.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatUnifScale, transformations, scaling                             M
*****************************************************************************/
void MatGenMatUnifScale(IrtRType Scale, IrtHmgnMatType Mat)
{
    MatGenMatScale(Scale, Scale, Scale, Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Rotate around the X axis by Teta       M
* radians.                                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Teta:       Amount of rotation, in radians.                              M
*   Mat:        Matrix to initialize as a rotation matrix.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatRotX1, transformations, rotation                                M
*****************************************************************************/
void MatGenMatRotX1(IrtRType Teta, IrtHmgnMatType Mat)
{
    IrtRType CTeta, STeta;

    CTeta = cos(Teta);
    STeta = sin(Teta);
    MatGenMatRotX(CTeta, STeta, Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Rotate around the X axis by Teta,      M
* given the sin and cosine of Teta                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   CosTeta, SinTeta:  Amount of rotation, given as sine and cosine of Teta. M
*   Mat:               Matrix to initialize as a rotation matrix.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatRotX, transformations, rotation                                 M
*****************************************************************************/
void MatGenMatRotX(IrtRType CosTeta, IrtRType SinTeta, IrtHmgnMatType Mat)
{
    MatGenUnitMat(Mat);                              /* Make it unit matrix. */
    Mat[1][1] = CosTeta;
    Mat[1][2] = SinTeta;
    Mat[2][1] = -SinTeta;
    Mat[2][2] = CosTeta;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Rotate around the Y axis by Teta       M
* radians.                                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Teta:       Amount of rotation, in radians.                              M
*   Mat:        Matrix to initialize as a rotation matrix.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatRotY1, transformations, rotation                                M
*****************************************************************************/
void MatGenMatRotY1(IrtRType Teta, IrtHmgnMatType Mat)
{
    IrtRType CTeta, STeta;

    CTeta = cos(Teta);
    STeta = sin(Teta);
    MatGenMatRotY(CTeta, STeta, Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Rotate around the Y axis by Teta,      M
* given the sin and cosine of Teta                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   CosTeta, SinTeta:  Amount of rotation, given as sine and cosine of Teta. M
*   Mat:               Matrix to initialize as a rotation matrix.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatRotY, transformations, rotation                                 M
*****************************************************************************/
void MatGenMatRotY(IrtRType CosTeta, IrtRType SinTeta, IrtHmgnMatType Mat)
{
    MatGenUnitMat(Mat);                              /* Make it unit matrix. */
    Mat[0][0] = CosTeta;
    Mat[0][2] = -SinTeta;
    Mat[2][0] = SinTeta;
    Mat[2][2] = CosTeta;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Rotate around the Z axis by Teta       M
* radians.                                                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Teta:       Amount of rotation, in radians.                              M
*   Mat:        Matrix to initialize as a rotation matrix.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatRotZ1, transformations, rotation                                M
*****************************************************************************/
void MatGenMatRotZ1(IrtRType Teta, IrtHmgnMatType Mat)
{
    IrtRType CTeta, STeta;

    CTeta = cos(Teta);
    STeta = sin(Teta);
    MatGenMatRotZ(CTeta, STeta, Mat);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to generate a 4*4 matrix to Rotate around the Z axis by Teta,      M
* given the sin and cosine of Teta                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   CosTeta, SinTeta:  Amount of rotation, given as sine and cosine of Teta. M
*   Mat:               Matrix to initialize as a rotation matrix.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatGenMatRotZ, transformations, rotation                                 M
*****************************************************************************/
void MatGenMatRotZ(IrtRType CosTeta, IrtRType SinTeta, IrtHmgnMatType Mat)
{
    MatGenUnitMat(Mat);                              /* Make it unit matrix. */
    Mat[0][0] = CosTeta;
    Mat[0][1] = SinTeta;
    Mat[1][0] = -SinTeta;
    Mat[1][1] = CosTeta;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to multiply two 4by4 matrices.                                   M
* MatRes may be one of Mat1 or Mat2 - it is only updated in the end.         M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix product.                                   M
*   Mat1, Mat2:  The two operand of the matrix product.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlMultTwo4by4, MatSubTwo4by4, MatAddTwo4by4                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatMultTwo4by4, transformations, matrix product                          M
*****************************************************************************/
void MatMultTwo4by4(IrtHmgnMatType MatRes,
		    IrtHmgnMatType Mat1,
		    IrtHmgnMatType Mat2)
{
    int i, j, k;
    IrtHmgnMatType MatResTemp;

    for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++) {
	   MatResTemp[i][j] = 0;
	   for (k = 0; k < 4; k++)
	       MatResTemp[i][j] += Mat1[i][k] * Mat2[k][j];
	}
    for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
	    MatRes[i][j] = MatResTemp[i][j];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to add two 4by4 matrices.					     M
* MatRes may be one of Mat1 or Mat2.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix addition.                                  M
*   Mat1, Mat2:  The two operand of the matrix addition.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlAddTwo4by4, MatSubTwo4by4, MatMultTwo4by4                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatAddTwo4by4, transformations, matrix addition                          M
*****************************************************************************/
void MatAddTwo4by4(IrtHmgnMatType MatRes,
		   IrtHmgnMatType Mat1,
		   IrtHmgnMatType Mat2)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            MatRes[i][j] = Mat1[i][j] + Mat2[i][j];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to subtract two 4by4 matrices.				     M
* MatRes may be one of Mat1 or Mat2.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix subtraction.                               M
*   Mat1, Mat2:  The two operand of the matrix subtraction.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlSubTwo4by4, MatAddTwo4by4, MatMultTwo4by4                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatSubTwo4by4, transformations, matrix subtraction                       M
*****************************************************************************/
void MatSubTwo4by4(IrtHmgnMatType MatRes,
		   IrtHmgnMatType Mat1,
		   IrtHmgnMatType Mat2)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    MatRes[i][j] = Mat1[i][j] - Mat2[i][j];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to scale a 4by4 matrix.					     M
* MatRes may be Mat.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MatRes:      Result of matrix scaling.                                   M
*   Mat:         The two operand of the matrix scaling.                      M
*   Scale:       Scalar value to multiple matrix with.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatGnrlScaleMat, MatSubTwo4by4, MatAddTwo4by4, MatMultTwo4by4            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatScale4by4, transformations, matrix scaling                            M
*****************************************************************************/
void MatScale4by4(IrtHmgnMatType MatRes,
		  IrtHmgnMatType Mat,
		  const IrtRType *Scale)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
	    MatRes[i][j] = Mat[i][j] * (*Scale);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compare two homogeneous matrices for approximate equality.               M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat1, Mat2:   Two homogeneous matrices to compare.                       M
*   Eps:	  Tolerance of comaprison.                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if similar, FALSE otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatSameTwo4by4                                                           M
*****************************************************************************/
int MatSameTwo4by4(IrtHmgnMatType Mat1,
		   IrtHmgnMatType Mat2,
		   IrtRType Eps)
{
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
	    if (IRIT_APX_EQ_EPS(Mat1[i][j], Mat2[i][j], Eps))
	        return FALSE;
	}
    }

    return TRUE; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to multiply an XYZ Vector by 4by4 matrix:                          M
*   The Vector has only 3 components (X, Y, Z) and it is assumed that W = 0. M
*   VRes may be Vec as it is only updated in the end.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   VecRes:    Result of vector - matrix product.                            M
*   Vec:       Vector to transfrom using Matrix.                             M
*   Mat:       Transformation matrix.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatMultPtby4by4, MatMultWVec2by4by4                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatMultVecby4by4, transformations, vector matrix product                 M
*****************************************************************************/
void MatMultVecby4by4(IrtVecType VecRes,
		      const IrtVecType Vec,
		      IrtHmgnMatType Mat)
{
    IrtVecType VecTemp;

    VecTemp[0] = Vec[0] * Mat[0][0] + Vec[1] * Mat[1][0] + Vec[2] * Mat[2][0];
    VecTemp[1] = Vec[0] * Mat[0][1] + Vec[1] * Mat[1][1] + Vec[2] * Mat[2][1];
    VecRes[2]  = Vec[0] * Mat[0][2] + Vec[1] * Mat[1][2] + Vec[2] * Mat[2][2];

    IRIT_PT2D_COPY(VecRes, VecTemp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to multiply a XYZ point by 4by4 matrix:                            M
*   The point has only 3 components (X, Y, Z) and it is assumed that W = 1.  M
*   PtRes may be Pt as it is only updated in the end.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   PtRes:     Result of point - matrix product.                             M
*   Pt:        Point to transfrom using Matrix.                              M
*   Mat:       Transformation matrix.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatMultVecby4by4, MatMultWVec2by4by4                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatMultPtby4by4, transformations, vector matrix product                  M
*****************************************************************************/
void MatMultPtby4by4(IrtPtType PtRes,
		     const IrtPtType Pt,
		     IrtHmgnMatType Mat)
{
    IrtRType CalcW;
    IrtPtType PtTemp;

    PtTemp[0] = Pt[0] * Mat[0][0] + Pt[1] * Mat[1][0] + Pt[2] * Mat[2][0] +
		                                                Mat[3][0];
    PtTemp[1] = Pt[0] * Mat[0][1] + Pt[1] * Mat[1][1] + Pt[2] * Mat[2][1] +
		                                                Mat[3][1];
    PtTemp[2] = Pt[0] * Mat[0][2] + Pt[1] * Mat[1][2] + Pt[2] * Mat[2][2] +
		                                                Mat[3][2];

    if ((CalcW = Pt[0] * Mat[0][3] +
	         Pt[1] * Mat[1][3] +
	         Pt[2] * Mat[2][3] + Mat[3][3]) == 1.0) {
        IRIT_PT_COPY(PtRes, PtTemp);
    }
    else {
        CalcW = CalcW == 0.0 ? IRIT_INFNTY : 1.0 / CalcW;

	PtRes[0] = PtTemp[0] * CalcW;
	PtRes[1] = PtTemp[1] * CalcW;
	PtRes[2] = PtTemp[2] * CalcW;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to multiply a WXYZ Vector by 4by4 matrix:                          M
*   The Vector has only 4 components (X, Y, Z, W).                           M
*   VRes may be Vec as it is only updated in the end.                        M
*                                                                            *
* PARAMETERS:                                                                M
*   VRes:      Result of vector - matrix product.                            M
*   Vec:       Vector to transfrom using Matrix.                             M
*   Mat:       Transformation matrix.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatMultPtby4by4, MatMultVec2by4by4                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatMultWVecby4by4, transformations, vector matrix product                M
*****************************************************************************/
void MatMultWVecby4by4(IrtRType VRes[4],
		       const IrtRType Vec[4],
		       IrtHmgnMatType Mat)
{
    IrtPtType VTemp;

    VTemp[0] = Vec[0] * Mat[0][0] + Vec[1] * Mat[1][0] + Vec[2] * Mat[2][0] +
							 Vec[3] * Mat[3][0];
    VTemp[1] = Vec[0] * Mat[0][1] + Vec[1] * Mat[1][1] + Vec[2] * Mat[2][1] +
							 Vec[3] * Mat[3][1];
    VTemp[2] = Vec[0] * Mat[0][2] + Vec[1] * Mat[1][2] + Vec[2] * Mat[2][2] +
							 Vec[3] * Mat[3][2];
    VRes[3]  = Vec[0] * Mat[0][3] + Vec[1] * Mat[1][3] + Vec[2] * Mat[2][3] +
							 Vec[3] * Mat[3][3];

    IRIT_VEC_COPY(VRes, VTemp);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the determinant of a 4 by 4 matrix.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:      Input matrix.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType: Determinant of Mat.                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatDeterminantMatrix, determinant                                        M
*****************************************************************************/
IrtRType MatDeterminantMatrix(IrtHmgnMatType Mat)
{
    const IrtRType
	*Mat0 = Mat[0],
	*Mat1 = Mat[1],
	*Mat2 = Mat[2],
	*Mat3 = Mat[3];

    return
	Mat0[0] * (Mat1[1] * (Mat2[2] * Mat3[3] - Mat2[3] * Mat3[2]) +
                   Mat1[2] * (Mat2[3] * Mat3[1] - Mat2[1] * Mat3[3]) +
                   Mat1[3] * (Mat2[1] * Mat3[2] - Mat2[2] * Mat3[1])) -
        Mat0[1] * (Mat1[0] * (Mat2[2] * Mat3[3] - Mat2[3] * Mat3[2]) +
                   Mat1[2] * (Mat2[3] * Mat3[0] - Mat2[0] * Mat3[3]) +
                   Mat1[3] * (Mat2[0] * Mat3[2] - Mat2[2] * Mat3[0])) +
        Mat0[2] * (Mat1[0] * (Mat2[1] * Mat3[3] - Mat2[3] * Mat3[1]) +
                   Mat1[1] * (Mat2[3] * Mat3[0] - Mat2[0] * Mat3[3]) +
                   Mat1[3] * (Mat2[0] * Mat3[1] - Mat2[1] * Mat3[0])) -
        Mat0[3] * (Mat1[0] * (Mat2[1] * Mat3[2] - Mat2[2] * Mat3[1]) +
                   Mat1[1] * (Mat2[2] * Mat3[0] - Mat2[0] * Mat3[2]) +
                   Mat1[2] * (Mat2[0] * Mat3[1] - Mat2[1] * Mat3[0]));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the INVERSE of a given matrix M which is not modified.  M
*   The matrix is assumed to be 4 by 4 (transformation matrix).		     M
*   Return TRUE if inverted matrix (InvM) do exists.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:          Original matrix to invert.                                   M
*   InvM:       Inverted matrix will be placed here. Can be same as M.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if inverse exists, FALSE otherwise.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatTranspMatrix                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatInverseMatrix, transformations, matrix inverse                        M
*****************************************************************************/
int MatInverseMatrix(IrtHmgnMatType M, IrtHmgnMatType InvM)
{
    IrtHmgnMatType A;
    int i, j, k;
    IrtRType V;

    IRIT_HMGN_MAT_COPY(A, M);		/* Prepare temporary copy of M in A. */
    MatGenUnitMat(InvM);			     /* Make it unit matrix. */

    for (i = 0; i < 4; i++) {
	V = A[i][i];				      /* Find the new pivot. */
	k = i;
	for (j = i + 1; j < 4; j++)
	    if (IRIT_FABS(A[j][i]) > IRIT_FABS(V)) {
	        /* Find maximum on col i, row i+1..n */
	        V = A[j][i];
	        k = j;
	    }
	j = k;

	if (i != j)
	    for (k = 0; k < 4; k++) {
		IRIT_SWAP(IrtRType, A[i][k], A[j][k]);
		IRIT_SWAP(IrtRType, InvM[i][k], InvM[j][k]);
            }

	for (j = i + 1; j < 4; j++) {	 /* Eliminate col i from row i+1..n. */
            V = A[j][i] / A[i][i];
	    for (k = 0; k < 4; k++) {
                A[j][k]    -= V * A[i][k];
                InvM[j][k] -= V * InvM[i][k];
	    }
	}
    }

    for (i = 3; i >= 0; i--) {			       /* Back Substitution. */
	if (A[i][i] == 0)
	    return FALSE;					   /* Error. */

	for (j = 0; j < i; j++) {	 /* Eliminate col i from row 1..i-1. */
            V = A[j][i] / A[i][i];
	    for (k = 0; k < 4; k++) {
                /* A[j][k] -= V * A[i][k]; */
                InvM[j][k] -= V * InvM[i][k];
	    }
	}
    }

    for (i = 0; i < 4; i++)		    /* Normalize the inverse Matrix. */
	for (j = 0; j < 4; j++)
            InvM[i][j] /= A[i][i];

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to compute the TRANSPOSE of a given matrix M which is not modified.M
*   The matrix is assumed to be 4 by 4 (transformation matrix).		     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:          Original matrix to transpose.                                M
*   TranspM:    Transposed matrix will be placed here.  Can be same as M.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatInverseMatrix                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatTranspMatrix, transformations, matrix transpose                       M
*****************************************************************************/
void MatTranspMatrix(IrtHmgnMatType M, IrtHmgnMatType TranspM)
{
    int i, j;
    IrtHmgnMatType A;

    IRIT_HMGN_MAT_COPY(A, M);		/* Prepare temporary copy of M in A. */

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
	    TranspM[i][j] = A[j][i];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to estimate the scaling factor in a matrix by computing the        M
* SVD decomposition of the matrix and if fails, use average of the scale of  M
* the X, Y, and Z unit vectors.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   M:          Matrix to estimate scaling factors (assume positive scales). M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Estimated Scaling factor (returns positive scale values).    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatRotateFactorMatrix, MatTranslateFactorMatrix                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatScaleFactorMatrix, transformations		                     M
*****************************************************************************/
IrtRType MatScaleFactorMatrix(IrtHmgnMatType M)
{
    IrtRType
	Scale = 0.0;
    IrtRType u[3][3], v[3][3], S[3];
    IrtHmgnMatType Mat;

    IRIT_HMGN_MAT_COPY(Mat, M);

    if (SvdMatrix4x4(Mat, u, S, v))
        Scale = S[0] + S[1] + S[2];
    else {
        IrtVecType V, VRes, VRef;

	V[0] = 0;
	V[1] = 0;
	V[2] = 0;
	MatMultPtby4by4(VRef, V, Mat);

	V[0] = 0;
	V[1] = 0;
	V[2] = 1;
	MatMultPtby4by4(VRes, V, Mat);
	IRIT_PT_SUB(VRes, VRes, VRef);
	Scale += IRIT_PT_LENGTH(VRes);

	V[0] = 0;
	V[1] = 1;
	V[2] = 0;
	MatMultPtby4by4(VRes, V, Mat);
	IRIT_PT_SUB(VRes, VRes, VRef);
	Scale += IRIT_PT_LENGTH(VRes);

	V[0] = 1;
	V[1] = 0;
	V[2] = 0;
	MatMultPtby4by4(VRes, V, Mat);
	IRIT_PT_SUB(VRes, VRes, VRef);
	Scale += IRIT_PT_LENGTH(VRes);
    }

    return Scale / 3.0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to estimate the rotation factor in a matrix.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   M:          Matrix to extract rotation factors.                          M
*   RotMat:     The rotational factors of matrix M.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatScaleFactorMatrix, MatTranslateFactorMatrix                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatRotateFactorMatrix, transformations		                     M
*****************************************************************************/
void MatRotateFactorMatrix(IrtHmgnMatType M, IrtHmgnMatType RotMat)
{
    int i, j;
    IrtRType u[3][3], v[3][3], S[3], Scl;
    IrtHmgnMatType Mat, TmpMat;

    /* Create an orthogonal matrix by canceling the scale/translation. */
    IRIT_HMGN_MAT_COPY(Mat, M);
    if ((Scl = MatScaleFactorMatrix(Mat)) == 0.0) {
        MatGenUnitMat(RotMat);
        return;
    }
    MatGenMatUnifScale(1.0 / Scl, TmpMat);
    MatMultTwo4by4(Mat, Mat, TmpMat);
    IRIT_VEC_RESET(Mat[3]);

    SvdMatrix4x4(Mat, u, S, v);

    MatGenUnitMat(RotMat);
    for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	    RotMat[i][j] = Mat[i][j] / S[j];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Routine to estimate the translation factors in a matrix.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   M:          Matrix to extract rotation factors.                          M
*   Trans:      The translation factors of matrix M.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void		                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MatScaleFactorMatrix, MatRotateFactorMatrix                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MatTranslateFactorMatrix, transformations		                     M
*****************************************************************************/
void MatTranslateFactorMatrix(IrtHmgnMatType M, IrtVecType Trans)
{
    Trans[0] = M[3][0];
    Trans[1] = M[3][1];
    Trans[2] = M[3][2];
}
