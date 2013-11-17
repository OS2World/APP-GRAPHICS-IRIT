/******************************************************************************
* MvarKant.c - Uniqueness of roots using Kantorovich theorem		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Carmi Grushko, Feb. 2011.					      *
******************************************************************************/

#include "mvar_loc.h"

IRIT_GLOBAL_DATA int
    _MVGlblZeroApplyKantorovichTest = FALSE,
     /* Should Kantorovich subdivided around the unique root's box ? */
    _MVGlblZeroApplyKantorovichTestAroundBox = FALSE;
IRIT_GLOBAL_DATA 
    IrtRType KantBallFractionThreshold = 0.1;


/* A square, symmetric matrix data structure, implemented using a            */
/* "triangular" array.							     */
typedef struct {
    CagdRType *Data;
    int N;
} TriangularArrayStruct;

static TriangularArrayStruct *TriangularArrayCreateAux(int N);
static void TriangularArrayDestroy(TriangularArrayStruct *Matrix);
static CagdRType *TriangularArrayAtAux(TriangularArrayStruct *Matrix,
				       int r,
				       int c);
static CagdRType MvarKantMaxSqrDerive(const MvarMVStruct *MV, 
				      MvarMVDirType DirectionB);
static void MvarKantCachePab(const MvarMVStruct *MV, 
    			     MvarMVGradientStruct* Gradient, 
		             TriangularArrayStruct *PCache);
static CagdRType MvarKantBoundSplineLipschitz(MvarMVDirType DirectionI,  
					      TriangularArrayStruct* PCache);
static CagdRType MvarKantBoundJacobian(MvarMVStruct * const *F, 
    				       MvarMVGradientStruct** Gradients, 
				       int Dim);
static CagdRType MvarKantMatrixInfNorm(IrtGnrlMatType Matrix, int Dim);
static IrtBType MvarKantComputeBetaEtha(MvarMVStruct * const * F, 
					MvarMVGradientStruct** Gradients, 
					int Dim, 
					CagdRType *Beta, 
					CagdRType *Etha, 
					CagdRType Pt[]);
static IrtBType MvarKantApplyKantorovichTest(MvarMVStruct * const * F, 
					     int n, 
					     CagdRType DivisionPoint[], 
					     CagdRType *DivisionRadius);
static void MvarZeroSubdivideAroundBoxAux2(MvarMVStruct *MVsSubdomain1[], 
					   MvarMVStruct *MVsSubdomain2[], 
					   int NumOfMVs, 
					   IrtRType t, 
					   int Dir, 
					   IrtBType KeepWorkingOnUpper);
static MvarMVStruct ***MvarKantSubdivideAroundBox(MvarMVStruct **MVs, 
						  int NumOfMVs, 
						  IrtRType DivisionPoint[], 
						  IrtRType DivisionRadius,
						  int* SubdomainMVsCount);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates an array of size that is enough for storage of an N by N         *
* square, symmetric matrix.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   N:   Dimension of the square, symmetric matrix.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType*:  A pointer to the create array                               *
*****************************************************************************/
static TriangularArrayStruct *TriangularArrayCreateAux(int N)
{
    TriangularArrayStruct
        *Matrix = (TriangularArrayStruct *)
                                    IritMalloc(sizeof(TriangularArrayStruct));

    Matrix -> Data = (CagdRType *) IritMalloc((N * (1 + N) / 2) *
					                   sizeof(CagdRType));
    Matrix -> N = N;
    return Matrix;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Destroys a square, symmetric matrix                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Matrix: The matrix to destroy.		                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   None                                                                     *
*****************************************************************************/
static void TriangularArrayDestroy(TriangularArrayStruct *Matrix)
{
    IritFree(Matrix -> Data);
    IritFree(Matrix);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Accessor for a square, symmetric matrix                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Matrix:  The matrix to access.		                             *
*   r:  Zero-based row number.                                               *
*   c:  Zero-based column number.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType *:   A pointer to the (r, c) element.                          *
*****************************************************************************/
static CagdRType *TriangularArrayAtAux(TriangularArrayStruct *Matrix,
				       int r,
				       int c)
{
    int n = Matrix -> N;

    if (r > c)
	IRIT_SWAP(int, r, c);

    return &(Matrix -> Data[c - r + r * (n + n - r + 1) / 2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a direction, computes the derivative in this direction and finds   *
* the maximal absolute value of the control points. Assumes all control      *
* points are scalars.		                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   MV:          The multi-variate to operate on.                            *
*   Dir:         Direction to differentiate.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Maximal absolute value of the control points of the         *
*                derivative.                                                 *
*****************************************************************************/
static CagdRType MvarKantMaxSqrDerive(const MvarMVStruct *MV, 
				      MvarMVDirType Dir)
{
    CagdRType
        MaxP = 0;
    MvarMVStruct
        *dMV = MvarMVDerive(MV, Dir);
    int i,
	PCount = dMV -> SubSpaces[dMV -> Dim];

    for (i = 0; i < PCount; i++) {
	CagdRType
	    P = dMV -> Points[1][i];

	if (P < 0)
	    P = -P;
	if (P > MaxP)
	    MaxP = P;
    }

    MvarMVFree(dMV);

    return MaxP * MaxP;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Runs over all pairs of directions a,b such that a<b, and computes a      *
* bound on max{ d MV / da db } using the control points.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   MV:      The multi-variate to operate on.                                *
*   Pcache:  The square, symmetric matrix to write results into.             *
*                                                                            *
* RETURN VALUE:                                                              *
*   None                                                                     *
*****************************************************************************/
static void MvarKantCachePab(const MvarMVStruct *MV, 
    			     MvarMVGradientStruct* Gradient, 
		             TriangularArrayStruct *PCache)
{
    int a,
	n = MV -> Dim;

    for (a = 0; a < n; a++) {
	int b;
	MvarMVStruct
	    *dMVa = MvarMVDerive(MV, a);

	for (b = a; b < n; b++)
	    *TriangularArrayAtAux(PCache, a, b) = MvarKantMaxSqrDerive(dMVa,
								       b);
	MvarMVFree(dMVa);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a bound for Lipschitz constant of d MV / da, based on the       *
* bounds on the second derivatives of MV in PCache.                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Dir:         Direction of differentiation of MV.                         *
*   Pcache:      The square, symmetric matrix that holds bounds on           *
*	  	  max{ d MV / da db }.		                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:  A bound for the Lipschitz constant of \partial_{x_i} f_j.    *
*****************************************************************************/
static CagdRType MvarKantBoundSplineLipschitz(MvarMVDirType Dir,  
					      TriangularArrayStruct *PCache)
{
    int u,
	n = PCache -> N;
    CagdRType
        Sum = 0;

    for (u = 0; u < n; u++)
	Sum += *TriangularArrayAtAux(PCache, Dir, u);

    return sqrt(Sum);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a bound for Lipschitz constant of the Jacobian of the system,   *
* based on Convex Hull property.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:    Set of functions defining the system.                            *
*   Dim:    Dimension of the problem (= number of function in the system).   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType: A bound for the Lipschitz constant of the Jacobian of the     *
*              system.                                                       *
*****************************************************************************/
static CagdRType MvarKantBoundJacobian(MvarMVStruct * const *MVs, 
    				       MvarMVGradientStruct** Gradients, 
				       int Dim)
{    
    int i, j;
    CagdRType
        Max = 0;
    IrtGnrlMatType
        J = (IrtGnrlMatType) IritMalloc(Dim * Dim * sizeof(CagdRType));
    TriangularArrayStruct
        *Cache = TriangularArrayCreateAux(Dim);

    for (i = 0; i < Dim; i++) {
	MvarKantCachePab(MVs[i], Gradients[i], Cache);

	for (j = 0; j < Dim; j++) {
	    J[i * Dim + j] = MvarKantBoundSplineLipschitz(j, Cache);
	}
    }
	
    for (i = 0; i < Dim; i++) {		
	CagdRType
	    Res = 0;

	for (j = 0; j < Dim; j++)
	    Res += J[i * Dim + j];

	if (Res > Max)
	    Max = Res;
    }

    TriangularArrayDestroy(Cache);
    IritFree(J);

    return Max * sqrt((CagdRType) Dim);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the L-inifinity (max) norm of a (square) matrix                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Matrix:   The matrix to operate on.                                      *
*   Dim:      Dimension of the matrix.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:  L-inifinity norm of the matrix.                              *
*****************************************************************************/
static CagdRType MvarKantMatrixInfNorm(IrtGnrlMatType Matrix, int Dim)
{
    int i, j;
    CagdRType
        Max = 0;

    for (i = 0; i < Dim; i++) {
	CagdRType
	    Sum = 0;

	for (j = 0; j < Dim; j++)
	    Sum += IRIT_ABS(Matrix[i * Dim + j]);

	Max = IRIT_MAX(Max, Sum);
    }

    return Max;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes Beta and Etha of a system at a specified point. For more        *
* information, refer to the document about using the Kantorovich theorem.    *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:   Functions that define the system.                                 *
*   Dim:   Dimension of the problem.                                         *
*   Beta:  A pointer to receive the computed Beta.                           *
*   Etha:  A poitner to receieve the computed Etha.                          *
*   Pt:    The coordinates to evaluate Beta and Etha at.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType:  TRUE if successfull, FALSE if the specified point cannot      *
*              be used (because the Jacobian is singular).                   *
*****************************************************************************/
static IrtBType MvarKantComputeBetaEtha(MvarMVStruct * const * MVs, 
					MvarMVGradientStruct **Gradients, 
					int Dim, 
					CagdRType *Beta, 
					CagdRType *Etha, 
					CagdRType Pt[])
{
    IrtBType Result;
    int i;
    IrtGnrlMatType 
	J = NULL, 
	MVsVal = NULL,
	invJ = NULL;

    /* Fill out a Jacobian matrix, and a MVs(Pt) matrix. */
    J = (IrtGnrlMatType) IritMalloc(sizeof(CagdRType) * Dim * Dim);
    MVsVal = (IrtGnrlMatType) IritMalloc(sizeof(CagdRType) * Dim);;

    for (i = 0; i<Dim; i++) {
	CagdRType
	    *v = MvarMVEvalGradient(Gradients[i], Pt, 0);

	IRIT_GEN_COPY(J + i * Dim, v, Dim * sizeof(CagdRType));
	MVsVal[i] = v[Dim];
    }

    /* Compute inverse Jacobian. */
    if (Dim == 2) {
	IrtRType 
	    Det = J[0] * J[3] - J[1] * J[2];

	if (IRIT_ABS(Det) < IRIT_UEPS)
	    /* Jacobian is singular. */
	    Result = FALSE; 
	else {	    
	    Result = TRUE; 
	    IRIT_SWAP(IrtRType, J[0], J[3]);
	    J[1] = -J[1];
	    J[2] = -J[2];

	    J[0] /= Det;
	    J[1] /= Det;
	    J[2] /= Det;
	    J[3] /= Det;

	    invJ = J;
	}
    }
    else if (Dim == 3) {
	IrtRType 
	    A = J[4] * J[8] - J[5] * J[7],
	    B = J[5] * J[6] - J[3] * J[8],
	    C = J[3] * J[7] - J[4] * J[6],

	    D = J[2] * J[7] - J[1] * J[8],
	    E = J[0] * J[8] - J[2] * J[6],
	    F = J[1] * J[6] - J[0] * J[7],

	    G = J[1] * J[5] - J[2] * J[4],
	    H = J[2] * J[3] - J[0] * J[5],
	    K = J[0] * J[4] - J[1] * J[3],

	    Det = J[0] * A + J[1] * B + J[2] * C,

	    InvDet = 1/Det;

	if (IRIT_ABS(Det) < IRIT_UEPS)
	    /* Jacobian is singular */
	    Result = FALSE;
	else {
	    Result = TRUE; 
	    J[0] = A * InvDet;
	    J[1] = D * InvDet;
	    J[2] = G * InvDet;
	    J[3] = B * InvDet;
	    J[4] = E * InvDet;
	    J[5] = H * InvDet;
	    J[6] = C * InvDet;
	    J[7] = F * InvDet;
	    J[8] = K * InvDet;
	    invJ = J;
	}
    } 
    else {
        invJ = (IrtGnrlMatType) IritMalloc(sizeof(CagdRType) * Dim * Dim);
	Result = MatGnrlInverseMatrix(J, invJ, Dim);
    }

    if (Result) {
	IrtRType Sum;
	int r, c;
	IrtVecGnrlType 
	    VT = invJ;

	/* Compute Beta. */
	*Beta = MvarKantMatrixInfNorm(invJ, Dim);

	/* Compute Etha. */
	*Etha = 0;
	for (r=0; r<Dim; r++) {	    
	    Sum = 0;
	    for (c = 0; c < Dim; c++, VT++)
		Sum += *VT * MVsVal[c];

	    Sum = IRIT_ABS(Sum);
	    if (Sum > *Etha)
		*Etha = Sum;
	}
    }

    /* Free resources. */
    IritFree(MVsVal);
    if (invJ != J)
	IritFree(invJ);
    IritFree(J);

    return Result;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Apply the Kantorovich test to determine if there is an area in the       *
* currrent domain that has a unique root.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*   F:            Functions that define the system.                          *
*   n:            Dimension of the problem (= number of functions).          *
*   DivisionPoint, DivisionRadius:  A pointer to return the Radius in which  *
*      there is a unique root, around the point specified in DivisionPoint.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   IrtBType:  TRUE if there is unique root somewhere, FALSE otherwise.      *
*****************************************************************************/
static IrtBType MvarKantApplyKantorovichTest(MvarMVStruct * const * F, 
					     int n, 
					     CagdRType DivisionPoint[], 
					     CagdRType *DivisionRadius)
{
    int i,
        Attempts = 10;
    CagdRType Gamma,
	Beta = 1.0,
	Etha = 1.0,
        *Pt = (CagdRType *) IritMalloc(sizeof(CagdRType) * n),
	*MinArray = (CagdRType *) IritMalloc(sizeof(CagdRType) * n),
	*MaxArray = (CagdRType *) IritMalloc(sizeof(CagdRType) * n),
	MaxVolume = 0;
    MvarMVGradientStruct
        **Gradients = (MvarMVGradientStruct **)
                            IritMalloc(sizeof(MvarMVGradientStruct *) * n);

    for (i = 0; i < n; i++)
	 Gradients[i] = MvarMVPrepGradient(F[i], TRUE);

    /* Compute Gamma. */
    Gamma = MvarKantBoundJacobian(F, Gradients, n);

    /* Compute Beta, Etha at points sampled uniformly in the domain	    */
    /* of the first function (which should be the same for all functions.   */
    MvarMVDomain(F[0], MinArray, MaxArray, -1);

    *DivisionRadius = -1;

    for (; Attempts; Attempts--) {
	CagdRType Alpha;

	for (i = 0; i<n; i++)
	    Pt[i] = IritRandom(MinArray[i], MaxArray[i]);

	if (!MvarKantComputeBetaEtha(F, Gradients, n, &Beta, &Etha, Pt))
	    continue;                              /* Jacobian is singular. */

	/* Compute Alpha and the uniqueness radius, if exists. */
	Alpha = Beta * Etha * Gamma;

	if (Alpha <= 0.5) {
	    IrtBType 
		Reject = TRUE;
	    CagdRType
	        Radius = (1 + sqrt(1 - 2 * Alpha)) / Beta / Gamma,
	        volume = 1;

	    for (i = 0; i < n; i++) {
		volume *= IRIT_MIN(Pt[i] + Radius, MaxArray[i]) +
		          IRIT_MAX(Pt[i] - Radius, MinArray[i]);

		if ((MaxArray[i] - Pt[i]) > Radius ||
		    (Pt[i] - MinArray[i]) > Radius)
		    Reject = FALSE;
	    }

	    if (MaxVolume < volume) {
		volume = MaxVolume;
		*DivisionRadius = Radius;
		IRIT_GEN_COPY(DivisionPoint, Pt, sizeof(CagdRType) * n);
	    }

	    MaxVolume = IRIT_MAX(MaxVolume, volume);

	    if (Reject) {
		IritFree(MinArray);
		IritFree(MaxArray);
		IritFree(Pt);
		for (i = 0; i < n; i++)
		    MvarMVFreeGradient(Gradients[i]);
		IritFree(Gradients);
		return TRUE;
	    }
	}
    }

    IritFree(MinArray);
    IritFree(MaxArray);
    IritFree(Pt);
    for (i = 0; i < n; i++)
	MvarMVFreeGradient(Gradients[i]);
    IritFree(Gradients);
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Used in MvarZeroSubdivideAroundBoxAux. Subdivides MVsSubdomain1 along    *
*  direction Dir at coordinate t. Depending on KeppWorkingOnUpper,           *
* MVsSubdomain1 becomes the subdomain with a coordinate smaller than t in    *
* direction Dir, while MVsSubdomain2 becomes the subdomain with a coordinate *
* larger than t in that direction.		                             *
*                                                                            *
* PARAMETERS:                                                                *
*   MVsSubdomain1:      [in/out] Vector of multivariate constraints.         *
*   MVsSubdomain2:      [out]    Vector of multivariate constraints.         *
*   NumOfMVs:                    Size of the MVs and Constraints vector.     *
*   Dir:                         Subdivide along this direction.             *
*   t:                           Subdivide at this parametric coordinate.    *
*   KeepWorkingOnUpper:          If TRUE, MVsSubdomain1 becomes with smaller *
*				 than t in Dir, otherwise MVsSubdomain2      *
*                                                                            *
* RETURN VALUE:                                                              *
*   None                                                                     *
*****************************************************************************/
static void MvarZeroSubdivideAroundBoxAux2(MvarMVStruct *MVsSubdomain1[], 
					   MvarMVStruct *MVsSubdomain2[], 
					   int NumOfMVs, 
					   IrtRType t, 
					   int Dir, 
					   IrtBType KeepWorkingOnUpper)
{
    int i;

    if (KeepWorkingOnUpper) {
	for (i = 0; i < NumOfMVs; i++) {
	    MVsSubdomain2[i] = MvarMVSubdivAtParam(MVsSubdomain1[i], t, Dir);
	    MVsSubdomain1[i] = MVsSubdomain2[i] -> Pnext;
	    MVsSubdomain2[i] -> Pnext = NULL;
	}
    }
    else {
	for (i = 0; i < NumOfMVs; i++) {
	    MVsSubdomain1[i] = MvarMVSubdivAtParam(MVsSubdomain1[i], t, Dir);
	    MVsSubdomain2[i] = MVsSubdomain1[i] -> Pnext;
	    MVsSubdomain1[i] -> Pnext = NULL;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given an n-cube, subdivides the domain around this cube (the cube        *
* itself is "discarded"). MVs is destroyed in the process, while the         *
* resulting sub-domains are returned.                                        *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:                     Multivariates to subdivide.                     *
*   NumOfMVs:                Size of the MVs and Constraints vector.         *
*   DivisionPoint[]:         Center of box to subdivide around.              *
*   DivisionRadius:          Radius (in L-inf norm) of the box to subdivide. *
*   SubdomainMVsCount: [out] number of resulting subdomains.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarMVStruct ***:        List of subdomains that are "around" the box    *
*****************************************************************************/
static MvarMVStruct ***MvarKantSubdivideAroundBox(MvarMVStruct **MVs, 
						  int NumOfMVs, 
						  IrtRType DivisionPoint[], 
						  IrtRType DivisionRadius,
						  int *SubdomainMVsCount)
{
    int Dir, mv, i,
	Dim = MVs[0] -> Dim,
        MVsArraySize = 0;
    MvarMVStruct
	/* Simple subdivision along some direction results in splitting MVs */
	/* into 2 new MVs; subdivision around an n-dimensional box results  */
	/* in up to 2*n new MVs.                                            */
        ***MVsArray = 
	    (MvarMVStruct ***) IritMalloc(2 * Dim * sizeof(MvarMVStruct **)),
        **InternalMVs = 
	    (MvarMVStruct **) IritMalloc(NumOfMVs * sizeof(MvarMVStruct *));
    IrtRType 
	*MinArray = (IrtRType *) IritMalloc(sizeof(IrtRType) * Dim),
	*MaxArray = (IrtRType *) IritMalloc(sizeof(IrtRType) * Dim);

    MvarMVDomain(MVs[0], MinArray, MaxArray, -1);

    for (i=0; i<NumOfMVs; i++)
	InternalMVs[i] = MvarMVCopy(MVs[i]);

    /* Create sub-domain MVs. */
    for (Dir=0; Dir<Dim; Dir++) {
	int i, sign;

	for (i=0, sign=1; i<2; i++, sign *= -1) {
	    IrtRType 
		t = DivisionPoint[Dir] + sign * DivisionRadius;

	    /* Subdivide in direction Dim at Pt+Radius and Pt-Radius. */
	    if (t < MaxArray[Dir]) {
		MVsArray[MVsArraySize] = 
		    (MvarMVStruct **) IritMalloc(NumOfMVs *
						      sizeof(MvarMVStruct *));
		MvarZeroSubdivideAroundBoxAux2(InternalMVs, 
					       MVsArray[MVsArraySize], 
					       NumOfMVs, t, Dir, 
					       sign != 1);
		MVsArraySize++;
	    }
	}
    }

    for (mv = 0; mv < NumOfMVs; mv++)
	MvarMVFree(InternalMVs[mv]);

    IritFree(MinArray);
    IritFree(MaxArray);
    IritFree(InternalMVs);

    *SubdomainMVsCount = MVsArraySize;

    return MVsArray;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Handle all aspects of root-uniqueness using Kantorovich; return a        M
* candidate point where a unique root exists, and find the rest of the roots M
* recursively, using MvarZeroMVsSubdiv.                                      M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:          Vector of multivariate constraints.                        M
*   Constraints:  Either an equality or an inequality type of constraint.    M
*   NumOfMVs:     Size of the MVs and Constraints vector.                    M
*   NumOfZeroMVs: Number of zero or equality constraints.                    M
*   ApplyNormalConeTest:  TRUE to apply normal cones' single intersection    M
*		  tests.						     M
*   SubdivTol:    Tolerance of the subdivision process.  Tolerance is        M
*		  measured in the parametric space of the multivariates.     M
*   Depth:        Of subdivision recursion.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   List of points on the solution set.  Dimension of the  M
*		      points will be the same as the dimensions of all MVs.  M
*                     NULL is returned.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarZeroGetRootsByKantorovich                                            M
*****************************************************************************/
MvarPtStruct *MvarZeroGetRootsByKantorovich(MvarMVStruct **MVs,
					    MvarConstraintType *Constraints,
					    int NumOfMVs,
					    int NumOfZeroMVs,
					    int ApplyNormalConeTest,
					    CagdRType SubdivTol,
					    int Depth)
{
    int Mv, i,
	SubdomainMVsCount = 0,
	Dim = MVs[0] -> Dim;
    MvarPtStruct
	*PtList = NULL;
    MvarMVStruct
	***SubdomainMVs = NULL;	

    if (!_MVGlblZeroApplyKantorovichTest)
        return NULL;

    /* Kantorovich Stopping Criterion. */
    if (Dim == NumOfZeroMVs) {
	IrtRType 
	    *DivisionPt = (IrtRType *) IritMalloc(sizeof(IrtRType) * Dim);
	IrtRType DivisionRadius;
	if (MvarKantApplyKantorovichTest(MVs, NumOfZeroMVs,
					    DivisionPt, &DivisionRadius)) {
	    IritFree( DivisionPt );
	    return MvarZeroGenPtMidMvar(MVs[0], TRUE);
	}

	if (_MVGlblZeroApplyKantorovichTestAroundBox &&
	    DivisionRadius > 0 &&
	    pow(DivisionRadius, Dim) / MvarMVVolumeOfDomain(MVs[0], Dim)  
		                            > KantBallFractionThreshold) {

	    SubdomainMVs = MvarKantSubdivideAroundBox(MVs, NumOfMVs,
					              DivisionPt, 
						      DivisionRadius,
					              &SubdomainMVsCount);

	    /* Set a candidate pt. in middle of the root-uniqueness box. */
	    PtList = MvarPtNew (Dim);
	    PtList -> Pt = DivisionPt;
	}
    }

    /* Recursively find roots in sub-domain MVs. */
    for (Mv = 0; Mv < SubdomainMVsCount; Mv++) {
	MvarPtStruct
	    *TempPtList = NULL;	

	TempPtList = MvarZeroMVsSubdiv(SubdomainMVs[Mv], Constraints,
				       NumOfMVs, NumOfZeroMVs,
				       ApplyNormalConeTest, SubdivTol,
				       Depth + 1);
        PtList = (MvarPtStruct *) CagdListAppend(PtList, TempPtList);
    }

    /* Clean up - free everything. */
    for (i = 0; i < SubdomainMVsCount; i++) {
	for (Mv = 0; Mv < NumOfMVs; Mv++)
	    MvarMVFree(SubdomainMVs[i][Mv]);

	IritFree(SubdomainMVs[i]);
    }
    IritFree(SubdomainMVs);        

    /* Return candidate points. */
    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the use (or not) of the Kantorovich test inside the multivariate    M
* subdivisions' zero set solver.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   KantorovichTest:   New setting for normal cone testing usage.            M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       Old setting for normal cone testing usage.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVsZeros, MvarMVsZerosDomainReduction,				     M
*   MvarMVsZerosGradPreconditioning, MvarMVsZerosSetCallBackFunc	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVsZerosKantorovichTest                                              M
*****************************************************************************/
int MvarMVsZerosKantorovichTest(int KantorovichTest)
{
    int OldVal = _MVGlblZeroApplyKantorovichTest;

    _MVGlblZeroApplyKantorovichTest = KantorovichTest;

    return OldVal;
}
