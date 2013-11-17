/******************************************************************************
* MvCones.c - Tools to construct and intersect MV (anti-)cones & vectors.     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Haniel and Gershon Elber, May 2005.			      *
******************************************************************************/

#include "mvar_loc.h"

#define MVAR_ORTHO2_VALID_VAL 0.99

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Add two multivariate vectors.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   VRes:    Result.  Can be one of V1 or V2.                                M
*   V1, V2:  Two input vectors.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecDotProd, MvarVecSqrLength, MvarVecLength, MvarVecScale            M
*   MvarVecNormalize, MvarVecBlend                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecAdd                                                               M
*****************************************************************************/
void MvarVecAdd(MvarVecStruct *VRes,
		const MvarVecStruct *V1,
		const MvarVecStruct *V2)
{
    int i;

    assert(V1 -> Dim == V2 -> Dim && VRes -> Dim == V2 -> Dim);

    for (i = 0; i < V1 -> Dim; i++)
	VRes -> Vec[i] = V1 -> Vec[i] + V2 -> Vec[i];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Subtract two multivariate vectors.                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   VRes:    Result.  Can be one of V1 or V2.                                M
*   V1, V2:  Two input vectors.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecDotProd, MvarVecSqrLength, MvarVecLength, MvarVecScale            M
*   MvarVecNormalize, MvarVecBlend, MvarVecAdd                               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecSub                                                               M
*****************************************************************************/
void MvarVecSub(MvarVecStruct *VRes,
		const MvarVecStruct *V1,
		const MvarVecStruct *V2)
{
    int i;

    assert(V1 -> Dim == V2 -> Dim && VRes -> Dim == V2 -> Dim);

    for (i = 0; i < V1 -> Dim; i++)
	VRes -> Vec[i] = V1 -> Vec[i] - V2 -> Vec[i];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the dot product of two multivariate vectors.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   V1, V2:  Two input vectors.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  The dot product.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecSqrLength, MvarVecLength, MvarVecScale                M
*   MvarVecNormalize, MvarVecBlend                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecDotProd                                                           M
*****************************************************************************/
CagdRType MvarVecDotProd(const MvarVecStruct *V1, const MvarVecStruct *V2)
{
    int i;
    CagdRType
	DotProd = 0.0;

    assert(V1 -> Dim == V2 -> Dim);

    for (i = 0; i < V1 -> Dim; i++)
        DotProd += V1 -> Vec[i] * V2 -> Vec[i];

    return DotProd;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the length squared of a multivariate vector.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   V:   Vector to compute its length.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Computed length squared.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecDotProd, MvarVecLength, MvarVecScale                  M
*   MvarVecNormalize, MvarVecBlend, MvarVecSqrLength2                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecSqrLength                                                         M
*****************************************************************************/
CagdRType MvarVecSqrLength(const MvarVecStruct *V)
{
    return MvarVecDotProd(V, V);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the length squared of a multivariate vector.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   v:    Vector to compute its length.                                      M
*   Dim:  Lentgth of vector v.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Computed length squared.                                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecDotProd, MvarVecLength, MvarVecScale                  M
*   MvarVecNormalize, MvarVecBlend, MvarVecSqrLength                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecSqrLength2                                                        M
*****************************************************************************/
CagdRType MvarVecSqrLength2(const CagdRType *v, int Dim)
{
    int j;
    CagdRType
        SumSqr = 0.0;

    for (j = 0; j < Dim; j++, v++)
        SumSqr += IRIT_SQR(*v);

    return SumSqr;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the length of a multivariate vector.      		             M
*                                                                            *
* PARAMETERS:                                                                M
*   V:   Vector to compute its length.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Computed length. 	                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecDotProd, MvarVecSqrLength, MvarVecScale               M
*   MvarVecNormalize, MvarVecBlend                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecLength                                                            M
*****************************************************************************/
CagdRType MvarVecLength(const MvarVecStruct *V)
{
    return sqrt(MvarVecSqrLength(V));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Scale a given multivariate vector V by a scaling factor ScaleFactor.     M
*                                                                            *
* PARAMETERS:                                                                M
*   V:             Vector to scale, in place.				     M
*   ScaleFactor:   Scaling factor to use.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:  The input vector, scaled.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecDotProd, MvarVecSqrLength, MvarVecLength,             M
*   MvarVecNormalize, MvarVecBlend                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecScale                                                             M
*****************************************************************************/
MvarVecStruct *MvarVecScale(MvarVecStruct *V, CagdRType ScaleFactor)
{
    int i;

    for (i = 0; i < V -> Dim; i++)
        V -> Vec[i] *= ScaleFactor;

    return V;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the blend of the to given multivariate vectors as		     M
*	V1 * t + V2 * (1-t).				                     V
*                                                                            *
* PARAMETERS:                                                                M
*   VRes:    Result.  Can be one of V1 or V2.                                M
*   V1, V2:  Two input vectors to blend.				     M
*   t:	     Blending factor.						     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecSqrLength, MvarVecLength, MvarVecScale                M
*   MvarVecNormalize, MvarVecDotProd                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecBlend                                                             M
*****************************************************************************/
void MvarVecBlend(MvarVecStruct *VRes,
		  const MvarVecStruct *V1,
		  const MvarVecStruct *V2,
		  CagdRType t)
{
    int i;
    CagdRType
	t1 = 1.0 - t;

    assert(V1 -> Dim == V2 -> Dim && VRes -> Dim == V2 -> Dim);

    for (i = 0; i < V1 -> Dim; i++)
        VRes -> Vec[i] = V1 -> Vec[i] * t + V2 -> Vec[i] * t1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Normalize a given multivariate vector to a unit length, in place.        M
*                                                                            *
* PARAMETERS:                                                                M
*   V:   Vector to normalize.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if the input is the ZERO vector.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecAdd, MvarVecDotProd, MvarVecSqrLength, MvarVecLength,             M
*   MvarVecScale                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecNormalize                                                         M
*****************************************************************************/
int MvarVecNormalize(MvarVecStruct *V)
{
    CagdRType
	VecLengthFactor = MvarVecLength(V);

    if (VecLengthFactor > IRIT_UEPS) {
	MvarVecScale(V, 1.0 / VecLengthFactor);
	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Updates Dir to be the closest vector to Dir that is orthogonal to Vec.   M
*   In essence, apply a Graham Shmidt step.  Vectors need not be unit size.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Dir:   Vector to update in place so it will be orthgoonal to Vec.        M
*   Vec:   Vector to make sure Dir is made orthogonal to.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecOrthogonal2                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecOrthogonalize                                                     M
*****************************************************************************/
int MvarVecOrthogonalize(MvarVecStruct *Dir, const MvarVecStruct *Vec)
{
    int i,
        Dim = Dir -> Dim;
    CagdRType R,
        LenVec = MvarVecLength(Vec);

    assert(Vec -> Dim == Dim);

    if (LenVec == 0.0)
        return TRUE;

    R = MvarVecDotProd(Dir, Vec) / IRIT_SQR(LenVec);

    for (i = 0; i < Dim; i++)
        Dir -> Vec[i] -= Vec -> Vec[i] * R;

    assert(IRIT_APX_EQ(MvarVecDotProd(Dir, Vec), 0.0));

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Derives a unit vector Dir that is orthogonal to both Vec1, and Vec2.     M
*   Note that in R^2 there is no such vector, in R^3 only one such vector    M
* and in R^n, n > 3, there are infinitly many of which we find one.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Dir:          Newly computed unit vector will be kep here.               M
*   Vec1, Vec2:   Two vectors we must be orthogonal to. Assumed unit length. M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:   TRUE if successful, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecOrthogonalize                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVecOrthogonal2                                                       M
*****************************************************************************/
int MvarVecOrthogonal2(MvarVecStruct *Dir,
		       const MvarVecStruct *Vec1,
		       const MvarVecStruct *Vec2)
{
    int i, j,
	Dim = Dir -> Dim;
    MvarVecStruct *Vec2O;

    if (Dim < 3)
        return FALSE;

    IritRandomInit(301060); 	       /* Make the randomness consistent... */

    /* Make Vec2 and orthogonal vector to Vec1. */
    Vec2O = MvarVecCopy(Vec2);
    MvarVecOrthogonalize(Vec2O, Vec1);

    /* Create a random vector and make sure its projection on Vec1/2 is     */
    /* large enough (and if not randomize again.			    */
    for (i = 0; i < 1000; i++) {  /* Allow that many trials before failing. */
	CagdRType d1, d2;

        for (j = 0; j < Dim; j++)
	    Dir -> Vec[j] = IritRandom(-1.0, 1.0);

	MvarVecNormalize(Dir);
	d1 = MvarVecDotProd(Dir, Vec1);
	d2 = MvarVecDotProd(Dir, Vec2O);

	if (IRIT_ABS(d1) < MVAR_ORTHO2_VALID_VAL &&
	    IRIT_ABS(d2) < MVAR_ORTHO2_VALID_VAL &&
	    MvarVecOrthogonalize(Dir, Vec1) &&
	    MvarVecOrthogonalize(Dir, Vec2O)) {
	    assert(IRIT_APX_EQ(MvarVecDotProd(Dir, Vec1), 0.0) &&
		   IRIT_APX_EQ(MvarVecDotProd(Dir, Vec2), 0.0));

#	    ifdef DEBUG
	        if (i > 2)
	            fprintf(stderr, "Orthogonalize the vector in MvarVecOrthogonal2 after %d steps.\n", i);
#	    endif /* DEBUG */

	    MvarVecFree(Vec2O);

	    return TRUE;
	}
    }

#ifdef DEBUG
    fprintf(stderr, "Failed to orthogonalize the vector in MvarVecOrthogonal2.\n");
#endif /* DEBUG */

    MvarVecFree(Vec2O);

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Normalize a given multivariate plane's normal direction to a unit        M
* length, in place.						             M
*                                                                            *
* PARAMETERS:                                                                M
*   Pln:   Plane to normalize its normal direction.                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE if the input is the ZERO vector.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarVecNormalize                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarPlaneNormalize                                                       M
*****************************************************************************/
int MvarPlaneNormalize(MvarPlaneStruct *Pln)
{
    int i,
	Dim = Pln -> Dim;
    CagdRType
	DProd = 0.0,
	*V = Pln -> Pln;

    for (i = 0; i < Dim; i++, V++)
	DProd += IRIT_SQR(*V);

    if (DProd != 0.0) {
        DProd = 1.0 / sqrt(DProd);

	for (i = 0, V = Pln -> Pln; i < Dim; i++)
	    *V++ *= DProd;

	return TRUE;
    }
    else
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the intersection of a line and a hyperplane, in R^Dim.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   P, V:   Point and direction of line to intersect with hyperplane.        M
*	    Both P and V are of length Dim.				     M
*   Pln:    Hyperplane to intersect with line.                               M
*   Param:  Will be updated with the parameter along which the intersection  M
*           has occured, as Inter = P + V * t.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVecStruct *:  The intersection point.                                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarLinePlaneInter                                                       M
*****************************************************************************/
MvarVecStruct *MvarLinePlaneInter(const MvarVecStruct *P,
				  const MvarVecStruct *V,
				  const MvarPlaneStruct *Pln,
				  CagdRType *Param)
{
    int i,
	Dim = P -> Dim;
    CagdRType t,
	A = 0.0,       /* We will reduce problem to linear form: At + B = 0. */
        B = 0.0;
    MvarVecStruct *InterPt;

    /* Substitute the line as "P + V * t" into the plane and find t. */
    for (i = 0; i < Dim; i++) {
        A += V -> Vec[i] * Pln -> Pln[i];
	B += P -> Vec[i] * Pln -> Pln[i];
    }

    t = A == 0.0 ? IRIT_INFNTY : -(B + Pln -> Pln[Dim]) / A;

    /* Compute the intersecting location. */
    InterPt = MvarVecNew(Dim);
    for (i = 0; i < Dim; i++)
        InterPt -> Vec[i] = P -> Vec[i] + V -> Vec[i] * t;

    return InterPt;
}
