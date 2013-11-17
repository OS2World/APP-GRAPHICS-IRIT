/******************************************************************************
* MvarEval.c - multi-variate function handling routines - evaluation routines.*
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 97.					      *
******************************************************************************/

#include <string.h>
#include "mvar_loc.h"

#define MVAR_NUMER_GRAD_EPS 1e-8

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given multivariate function at a given location.	     M
*   Same functionality as MvarMVEval but a different implementation.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To evaluate at given Params parametric location.               M
*   Params:   Parametric location to evaluate MV at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the multi-variate's point type. If for example	     M
*		  multi-variate point type is P2, the W, X, and Y will be    M
*		  saved in the first three locations of the returned vector. M
*		  The first location (index 0) of the returned vector is     M
*		  reserved for the rational coefficient W and XYZ always     M
*		  starts at second location of the returned vector           M
*		  (index 1).				                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVEval, MvarMVEvalGradient, MvarMVEvalGradient2	                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVEval2, evaluation, multi-variates                                  M
*****************************************************************************/
CagdRType *MvarMVEval2(const MvarMVStruct *MV, CagdRType *Params)
{
    IRIT_STATIC_DATA int
	AllocOrder = 2,
	AllocDim = -1,
	*Indices = NULL,
	*IndexFirst = NULL,
	*IndexLast = NULL;
    IRIT_STATIC_DATA CagdRType Pt[MVAR_MAX_PT_COORD],
	**BasisFuncs = NULL,
	*Min = NULL,
	*Max = NULL;
    int i, l, MaxOrder,
	Dim = MV -> Dim,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV),
        IsScalar = (IsNotRational && (MaxCoord == 1));

    /* Statically allocate all auxiliary memory space. */
    for (i = MaxOrder = 0; i < Dim; i++) {
        if (MaxOrder < MV -> Orders[i])
	    MaxOrder = MV -> Orders[i];
    }
    if (AllocDim < Dim || AllocOrder < MaxOrder) {
        if (Min != NULL) {
	    IritFree(Min);
	    IritFree(Max);
	    IritFree(Indices);
	    for (i = 0; i < AllocDim; i++)
	        IritFree(BasisFuncs[i]);
	    IritFree(BasisFuncs);
	    IritFree(IndexFirst);
	    IritFree(IndexLast);
	}

	AllocDim = Dim * 2;
	AllocOrder = MaxOrder * 2;

	Min = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
	Max = (CagdRType *) IritMalloc(sizeof(CagdRType) * AllocDim);
	Indices = (int *) IritMalloc(sizeof(int) * AllocDim);
	BasisFuncs = (CagdRType **) IritMalloc(sizeof(CagdRType *) * AllocDim);
	for (i = 0; i < AllocDim; i++)
	    BasisFuncs[i] = (CagdRType *)
	        IritMalloc(sizeof(CagdRType) * AllocOrder);
        IndexFirst = (int *) IritMalloc(sizeof(int) * AllocDim);
        IndexLast = (int *) IritMalloc(sizeof(int) * AllocDim);
    }

    IRIT_ZAP_MEM(Pt, sizeof(CagdRType) * MVAR_MAX_PT_COORD);

    /* Time to do the real evaluations. */
    if (MVAR_IS_BEZIER_MV(MV)) {
	int Index = 0;

	/* Now we have all auxiliary data sets - make sure domain is valid: */
	for (i = 0; i < Dim; i++) {
	    if (Params[i] < -IRIT_EPS || Params[i] > 1.0 + IRIT_EPS)
	        MVAR_FATAL_ERROR(MVAR_ERR_WRONG_DOMAIN);

	    if (Params[i] > 1.0)
	        Params[i] = 1.0;
	    else if (Params[i] < 0.0)
	        Params[i] = 0.0;
	}

	for (i = 0; i < Dim; i++) {
	    CagdRType
		*BzrBasisFuncs = BzrCrvEvalBasisFuncs(MV -> Orders[i],
						      Params[i]);

	    CAGD_GEN_COPY(BasisFuncs[i], BzrBasisFuncs,
			  sizeof(CagdRType) * MV -> Orders[i]);
	}

	IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);
       
	/* Go over all non zero basis functions and sum up contributions.   */
	/* We treat the first dimension differently for better performance. */
	do {
	    CagdRType BasisWeights0,
		* const *Points = MV -> Points;

	    /* Compute the product of the basis functions without first dim. */
	    switch (Dim) {
		case 1:
		    BasisWeights0 = 1.0;
		    break;
		case 2:
		    BasisWeights0 = BasisFuncs[1][Indices[1]];
		    break;
		case 3:
		    BasisWeights0 = BasisFuncs[1][Indices[1]] * 
		                    BasisFuncs[2][Indices[2]];
		    break;
		case 4:
		    BasisWeights0 = BasisFuncs[1][Indices[1]] * 
		                    BasisFuncs[2][Indices[2]] * 
		                    BasisFuncs[3][Indices[3]];
		    break;
		case 5:
		    BasisWeights0 = BasisFuncs[1][Indices[1]] * 
		                    BasisFuncs[2][Indices[2]] * 
		                    BasisFuncs[3][Indices[3]] * 
		                    BasisFuncs[4][Indices[4]];
		    break;
	        default:
	            for (i = 1, BasisWeights0 = 1.0; i < Dim; i++)
		        BasisWeights0 *= BasisFuncs[i][Indices[i]];
	    }

	    /* Now take care of all the first dimension at once. */
	    if (IsScalar) {
	        CagdRType
		    *BFuncs0 = &BasisFuncs[0][0],
		    *XPts = &Points[1][Index];

		for (i = MV -> Lengths[0]; --i >= 0; )
		    Pt[1] += BasisWeights0 * *BFuncs0++ * *XPts++;
	    }
	    else {
	        for (i = MV -> Lengths[0]; --i >= 0; ) {
		    CagdRType
		        BasisWeights = BasisWeights0 * BasisFuncs[0][i];

		    for (l = IsNotRational; l <= MaxCoord; l++)
		        Pt[l] += BasisWeights * Points[l][Index + i];
		}
	    }

	    Index += MVAR_NEXT_DIM(MV, 1);
	}
	while (MVAR_INC_SKIP_MESH_INDICES_1ST(MV, Indices));
    }
    else if (MVAR_IS_BSPLINE_MV(MV)) {
        CagdBType Periodic;
	int Index;
	CagdRType * const 
	    *Points = MV -> Points;

	/* Now we have all auxiliary data sets - make sure domain is valid: */
	MvarMVDomain(MV, Min, Max, -1);
	for (i = 0; i < Dim; i++) {
	    if (Params[i] < Min[i] - IRIT_EPS || Params[i] > Max[i] + IRIT_EPS)
	        MVAR_FATAL_ERROR(MVAR_ERR_WRONG_DOMAIN);

	    if (Params[i] > Max[i] - IRIT_UEPS * 2)
	        Params[i] = Max[i] - IRIT_UEPS * 2;
	    else if (Params[i] < Min[i])
	        Params[i] = Min[i];
	}

	Periodic = MvarBspMVIsPeriodic(MV);

        for (i = 0; i < Dim; i++) {
	    CagdRType
	        *BasisFunc = BspCrvCoxDeBoorBasis(MV -> KnotVectors[i],
						  MV -> Orders[i],
						  MV -> Lengths[i],
						  MV -> Periodic[i],
						  Params[i], &IndexFirst[i]);

	    CAGD_GEN_COPY(BasisFuncs[i], BasisFunc,
			  sizeof(CagdRType) * MV -> Orders[i]);
	}

	/* Go over all non zero basis functions and sum up contributions. */
	if (Periodic) {
	    CAGD_GEN_COPY(Indices, IndexFirst, sizeof(int) * Dim);
	    for (i = 0; i < Dim; i++)
	        IndexLast[i] = IndexFirst[i] + MV -> Orders[i];

	    do {
	        CagdRType
		    BasisWeights = 1.0;

		for (i = 0; i < Dim; i++)
		    BasisWeights *= BasisFuncs[i][Indices[i] - IndexFirst[i]];

		Index = MvarGetPointsPeriodicMeshIndices(MV, Indices);

		for (l = IsNotRational; l <= MaxCoord; l++)
		    Pt[l] += BasisWeights * Points[l][Index];
	    }
	    while (MVAR_INC_BOUND_MESH_INDICES(MV, Indices, IndexFirst, 
					       IndexLast, Index));
	}
	else {
	    /* Initialize Index with the corner offset. */
	    Index = MvarGetPointsMeshIndices(MV, IndexFirst);
	    IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);

	    if (IsScalar) {
	      CagdRType BasisPt, *BF0, *BF1, *BF2, *BF3, *BF4,
		    *Pt1 = &Pt[1],
		    *Pts = Points[1];

		switch (Dim) {
		    case 0:
		        do {
			    *Pt1 += Pts[Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 1:
		        BF0 = BasisFuncs[0];
		        do {
			    *Pt1 += BF0[Indices[0]] *
				    Pts[Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 2:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
		        do {
			    *Pt1 += BF0[Indices[0]] *
				    BF1[Indices[1]] *
				    Pts[Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 3:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			BF2 = BasisFuncs[2];
			do {
			    *Pt1 += BF0[Indices[0]] *
				    BF1[Indices[1]] *
				    BF2[Indices[2]] *
				    Pts[Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 4:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			BF2 = BasisFuncs[2];
			BF3 = BasisFuncs[3];
			do {
			    *Pt1 += BF0[Indices[0]] *
				    BF1[Indices[1]] *
				    BF2[Indices[2]] *
				    BF3[Indices[3]] *
				    Pts[Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 5:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			BF2 = BasisFuncs[2];
			BF3 = BasisFuncs[3];
			BF4 = BasisFuncs[4];
			do {
			    *Pt1 += BF0[Indices[0]] *
				    BF1[Indices[1]] *
				    BF2[Indices[2]] *
				    BF3[Indices[3]] *
				    BF4[Indices[4]] *
				    Pts[Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    default:
		        do {
			    for (i = 0, BasisPt = Pts[Index]; i < Dim; i++)
			        BasisPt *= BasisFuncs[i][Indices[i]];
			    *Pt1 += BasisPt;
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		}
	    }
	    else {
	        CagdRType BasisWeights, *BF0, *BF1, *BF2, *BF3, *BF4;

		switch (Dim) {
		    case 0:
		        do {
			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 1:
		        BF0 = BasisFuncs[0];
		        do {
			    BasisWeights = BF0[Indices[0]];

			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += BasisWeights * Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 2:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			do {
			    BasisWeights = BF0[Indices[0]] *
				           BF1[Indices[1]];
			
			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += BasisWeights * Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 3:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			BF2 = BasisFuncs[2];
			do {
			    BasisWeights = BF0[Indices[0]] *
				           BF1[Indices[1]] *
				           BF2[Indices[2]];
			
			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += BasisWeights * Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 4:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			BF2 = BasisFuncs[2];
			BF3 = BasisFuncs[3];
			do {
			    BasisWeights = BF0[Indices[0]] *
					   BF1[Indices[1]] *
					   BF2[Indices[2]] *
					   BF3[Indices[3]];

			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += BasisWeights * Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    case 5:
		        BF0 = BasisFuncs[0];
			BF1 = BasisFuncs[1];
			BF2 = BasisFuncs[2];
			BF3 = BasisFuncs[3];
			BF4 = BasisFuncs[4];
			do {
			    BasisWeights = BF0[Indices[0]] *
					   BF1[Indices[1]] *
					   BF2[Indices[2]] *
					   BF3[Indices[3]] *
					   BF4[Indices[4]];

			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += BasisWeights * Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		    default:
		        do {
			    for (i = 0, BasisWeights = 1.0; i < Dim; i++)
			        BasisWeights *= BasisFuncs[i][Indices[i]];

			    for (l = IsNotRational; l <= MaxCoord; l++)
			        Pt[l] += BasisWeights * Points[l][Index];
			}
			while (MVAR_INCREMENT_MESH_ORDER_INDICES(MV, Indices,
								 Index));
			break;
		}
	    }
	}
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_MVAR);
    }

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given multivariate function at a given location.	     M
*   Same functionality as MvarMVEval2 but a different and faster	     M
* implementation for Bezier evaluations.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To evaluate at given Params parametric location.               M
*   Params:   Parametric location to evaluate MV at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the multi-variate's point type. If for example	     M
*		  multi-variate point type is P2, the W, X, and Y will be    M
*		  saved in the first three locations of the returned vector. M
*		  The first location (index 0) of the returned vector is     M
*		  reserved for the rational coefficient W and XYZ always     M
*		  starts at second location of the returned vector           M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVEval2, MvarMVEvalGradient, MvarMVEvalGradient2                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVEval, evaluation, multi-variates                                   M
*****************************************************************************/
CagdRType *MvarMVEval(const MvarMVStruct *MV, CagdRType *Params)
{
    /* Time to do the real evaluations. */
    if (MVAR_IS_BEZIER_MV(MV)) {
        IRIT_STATIC_DATA int
	    AllocMeshSize = -1;
	IRIT_STATIC_DATA CagdRType Pt[MVAR_MAX_PT_COORD],
	    *Mesh[MVAR_MAX_PT_COORD] = { NULL };
	int i, j, k, l, MSize,
	    Dim = MV -> Dim,
	    MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
	CagdBType
	    IsNotRational = !MVAR_IS_RATIONAL_MV(MV);

	MSize = MVAR_CTL_MESH_LENGTH(MV);
	if (AllocMeshSize < MSize / MV -> Lengths[0]) {
	    if (Mesh[0] != NULL) {
	        for (i = 0; i < MVAR_MAX_PT_COORD; i++)
		    IritFree(Mesh[i]);
	    }

	    AllocMeshSize = 2 * MSize / MV -> Lengths[0];
	    for (i = 0; i < MVAR_MAX_PT_COORD; i++)
	        Mesh[i] = (CagdRType *)
		    IritMalloc(sizeof(CagdRType) * AllocMeshSize);
	}

	/* Now we have all auxiliary data sets - make sure domain is valid: */
	for (i = 0; i < Dim; i++) {
	    if (Params[i] < -IRIT_EPS || Params[i] > 1.0 + IRIT_EPS)
	        MVAR_FATAL_ERROR(MVAR_ERR_WRONG_DOMAIN);

	    if (Params[i] > 1.0)
	        Params[i] = 1.0;
	    else if (Params[i] < 0.0)
	        Params[i] = 0.0;
	}

	for (i = 0; i < Dim; i++) {
	    int Order = MV -> Orders[i],
		Length = MV -> Lengths[i];
	    CagdRType
		*BasisFuncs = BzrCrvEvalBasisFuncs(Order, Params[i]);

	    MSize /= Length;

	    for (l = IsNotRational; l <= MaxCoord; l++) {
	        CagdRType r,
		    *mi = i == 0 ? MV -> Points[l] : Mesh[l],
		    *mo = Mesh[l];

	        for (j = 0; j < MSize; j++) {
		    for (r = 0.0, k = Length; --k >= 0; ) {
		        r += mi[k] * BasisFuncs[k];
		    }
		    *mo++ = r;
		    mi += Length;
		}
	    }
	}

	for (l = IsNotRational; l <= MaxCoord; l++)
	    Pt[l] = Mesh[l][0];
	return Pt;
    }
    else if (MVAR_IS_BSPLINE_MV(MV)) {
        return MvarMVEval2(MV, Params);
    }
    else {
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_MVAR);
	return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the gradient of the given multivariate function at a given       M
* location, numerically.  Allowed for scalar multivariates only.             M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To evaluate its gradient at given Params parametric location.  M
*   Params:   Parametric location to evaluate MV at.                         M
*   HasOrig:  TRUE if the cached gradient also contains the original scalar  M
*	      field, as last, additional, coordinate.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the multi-variate's point type. If for example	     M
*		  multi-variate point type is P2, the W, X, and Y will be    M
*		  saved in the first three locations of the returned vector. M
*		  The first location (index 0) of the returned vector is     M
*		  reserved for the rational coefficient W and XYZ always     M
*		  starts at second location of the returned vector           M
*		  (index 1).				                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVEval, MvarMVEvalTanPlane, MvarMVEvalGradient                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVEvalGradient2, evaluation, multi-variates, gradient                M
*****************************************************************************/
CagdRType *MvarMVEvalGradient2(const MvarMVStruct *MV,
			       CagdRType *Params,
			       int *HasOrig)
{
    IRIT_STATIC_DATA CagdRType Grad[MVAR_MAX_PT_COORD];
    int i;
    CagdRType *R, Pos;
    MvarMVGradientStruct *MVGrad;

    if (MVAR_NUM_OF_MV_COORD(MV) != 1) {
	MVAR_FATAL_ERROR(MVAR_ERR_SCALAR_PT_EXPECTED);
	return NULL;
    }

    /* If we have a gradient structure around - use it. */
    if ((MVGrad = AttrGetPtrAttrib(MV -> Attr, "Gradient")) != NULL) {
        R = MvarMVEvalGradient(MVGrad, Params, 0);
	if (HasOrig != NULL)
	    *HasOrig = MVGrad -> HasOrig;
	CAGD_GEN_COPY(Grad, R, sizeof(CagdRType) * MVGrad -> Dim);
	return Grad;
    }

    /* No gradient structure - approximate the gradient numerically. */
    if (HasOrig != NULL)
        *HasOrig = FALSE;

    /* Get multivariate value at current location. */
    R = MvarMVEval(MV, Params);
    if (MVAR_IS_RATIONAL_MV(MV))
	Pos = R[1] / R[0];
    else
	Pos = R[1];

    /* Now get multivariate value at (current location + Eps) in all         */
    /* dimensions, computing the gradient.                                   */
    for (i = 0; i < MV -> Dim; i++) {
        CagdRType DtEps, Eps, OldParam, TMin, TMax;

	MvarMVDomain(MV, &TMin, &TMax, i);
	DtEps = IRIT_MAX((TMax - TMin) * MVAR_NUMER_GRAD_EPS, IRIT_UEPS);

	OldParam = Params[i];
	if (Params[i] + DtEps >= TMax) {
	    Eps = -DtEps;
	    Params[i] -= DtEps;
	}
	else {
	    Eps = DtEps;
	    Params[i] += DtEps;
	}

	R = MvarMVEval(MV, Params);
	if (MVAR_IS_RATIONAL_MV(MV))
	    Grad[i] = (R[1] / R[0] - Pos) / Eps;
	else
	    Grad[i] = (R[1] - Pos) / Eps;

	Params[i] = OldParam;
    }

    return Grad;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the tangent hyperplane of the given multivariate function at a   M
* given location, numerically.  Assumes a scalar multivariates of n          M
* parameters in a space of dimension n+1 (An explicit surface in E3).	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To evaluate its gradient at given Params parametric location.  M
*   Params:   Parametric location to evaluate MV at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPlaneStruct *:  A hyperplane, allocated dynammically.  The tanegnt   M
*		is normalized so that its last (independent coefficient is   M
*		one: "A1 X1 + A2 X2 + ... + An Xn + 1".  The size, n,  is    M
*		to the dimension of the multivariate.			     M
*									     *
* SEE ALSO:                                                                  M
*   MvarMVEval, MvarMVEvalGradient2                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVEvalTanPlane, evaluation, multi-variates, gradient                 M
*****************************************************************************/
MvarPlaneStruct *MvarMVEvalTanPlane(const MvarMVStruct *MV, CagdRType *Params)
{
    int i, HasOrig,
	Dim = MV -> Dim;
    MvarPlaneStruct
	*Pln = MvarPlaneNew(Dim);
    CagdRType *R,	
	*Grad = MvarMVEvalGradient2(MV, Params, &HasOrig);

    for (i = 0; i < Dim; i++)
	Pln -> Pln[i] = -Grad[i];
    Pln -> Pln[Dim] = 1.0;

    if (HasOrig) {
        Pln -> Pln[Dim + 1] = -Grad[Dim]; /* Free plane coefficient. */
    }
    else {
        /* Evaluate original and compute the free plane coefficient. */
        R = MvarMVEval(MV, Params);
	Pln -> Pln[Dim + 1] = MVAR_IS_RATIONAL_MV(MV) ? -R[1] / R[0] : -R[1];
    }

    for (i = 0; i < Dim; i++)
	Pln -> Pln[Dim + 1] -= Pln -> Pln[i] * Params[i];
	
    return Pln;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract an isoparametric sub multivariate out of the given tensor product  M
* multivariate, or expand its dimension by one.         		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:    To extract an isoparametric multi-variate from at parameter	     M
*	   value t in direction Dir, or exapand its dimension by one.	     M
*   t:     Parameter value at which to extract the isosurface (if Dir >= 0). M
*   Dir:   Direction of isosurface extraction.  If Dir is negative, however, M
*	   its absolute value defines the order of a new axis added as last  M
*	   and new dimension to the given MV.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A multi - variate with one less (or more) dimensions.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVReverse, MvarMVFromMesh, MvarPromoteMVToMV			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVFromMV, multi-variates                                             M
*****************************************************************************/
MvarMVStruct *MvarMVFromMV(const MvarMVStruct *MV,
			   CagdRType t,
			   MvarMVDirType Dir)
{
    MvarMVStruct *NewMV;
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, j, *NewLengths,
	*Lengths = MV -> Lengths,
	*Orders = MV -> Orders,
	Dim = MV -> Dim,
	NewDim = Dim + (Dir >= 0 ? -1 : 1),
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);

    if (Dir >= MV -> Dim || NewDim < 1) {
	MVAR_FATAL_ERROR(MVAR_ERR_DIR_NOT_VALID);
	return NULL;
    }
    if (Dir >= 0 && !MvarParamInDomain(MV, t, Dir)) {
	MVAR_FATAL_ERROR(MVAR_ERR_WRONG_DOMAIN);
	return NULL;
    }

    NewLengths = (int *) IritMalloc(sizeof(int) * NewDim);
    if (Dir >= 0) {
	for (i = 0; i < NewDim; i++)
	    NewLengths[i] = Lengths[i < Dir ? i : i + 1];
    }
    else {
	CAGD_GEN_COPY(NewLengths, Lengths, sizeof(int) * Dim);
	NewLengths[NewDim - 1] = -Dir;
    }

    if (MVAR_IS_BSPLINE_MV(MV)) {
	int *NewOrders = (int *) IritMalloc(sizeof(int) * NewDim);

	if (Dir >= 0) {
	    for (i = 0; i < NewDim; i++)
	        NewOrders[i] = Orders[i < Dir ? i : i + 1];
	}
	else {
	    CAGD_GEN_COPY(NewOrders, Orders, sizeof(int) * Dim);
	    NewOrders[NewDim - 1] = -Dir;
	}

	NewMV = MvarBspMVNew(NewDim, NewLengths, NewOrders, MV -> PType);

	IritFree(NewOrders);

	/* Copy/Create the knot vectors. */
	if (Dir >= 0) {
	    for (i = 0; i < NewDim; i++) {
	        int l = i < Dir ? i : i + 1;

	        if ((NewMV -> Periodic[i] = MV -> Periodic[l]) != FALSE) {
		    IritFree(NewMV -> KnotVectors[l]);
		    NewMV -> KnotVectors[l] = IritMalloc(sizeof(CagdRType) *
				          (MVAR_MVAR_ITH_PT_LST_LEN(NewMV, i) +
					   NewMV -> Orders[i]));
		}

	        CAGD_GEN_COPY(NewMV -> KnotVectors[i],
			      MV -> KnotVectors[l],
			      sizeof(CagdRType) *
			          (MVAR_MVAR_ITH_PT_LST_LEN(NewMV, i) +
				   NewMV -> Orders[i]));
	    }
	}
	else {
	    for (i = 0; i < NewDim - 1; i++) {
	        if ((NewMV -> Periodic[i] = MV -> Periodic[i]) != FALSE) {
		    IritFree(NewMV -> KnotVectors[i]);
		    NewMV -> KnotVectors[i] = IritMalloc(sizeof(CagdRType) *
				          (MVAR_MVAR_ITH_PT_LST_LEN(NewMV, i) +
					   NewMV -> Orders[i]));
		}

	        CAGD_GEN_COPY(NewMV -> KnotVectors[i], MV -> KnotVectors[i],
			      sizeof(CagdRType) *
			          (MVAR_MVAR_ITH_PT_LST_LEN(NewMV, i) +
				   NewMV -> Orders[i]));
	    }

	    BspKnotUniformOpen(-Dir, -Dir, NewMV -> KnotVectors[NewDim - 1]);
	}
    }
    else if (MVAR_IS_BEZIER_MV(MV)) {
	NewMV = MvarBzrMVNew(NewDim, NewLengths, MV -> PType);
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    IritFree(NewLengths);

    /* Create the new control mesh. */

    if (Dir >= 0) {  /* Evaluate the old MV at the given t in direction Dir. */
        CagdRType
	    *BasisFuncs = NULL;
	int IndexFirst, Index, NewIndex,
	    *LowerBound = (int *) IritMalloc(sizeof(int) * Dim),
	    *UpperBound = (int *) IritMalloc(sizeof(int) * Dim),
	    *Indices = (int *) IritMalloc(sizeof(int) * Dim),
	    *NewIndices = (int *) IritMalloc(sizeof(int) * NewDim);

	/* Original indices follows all indices except in Dir which follows  */
	/* only zero!							     */
	IRIT_ZAP_MEM(LowerBound, sizeof(int) * Dim);
	CAGD_GEN_COPY(UpperBound, Lengths, sizeof(int) * Dim);
	UpperBound[Dir] = 0;
	IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);
	IRIT_ZAP_MEM(NewIndices, sizeof(int) * NewDim);

	if (MV -> GType == MVAR_BSPLINE_TYPE)
	    BasisFuncs = BspCrvCoxDeBoorBasis(MV -> KnotVectors[Dir],
					      MV -> Orders[Dir],
					      MV -> Lengths[Dir],
					      MV -> Periodic[Dir],
					      t, &IndexFirst);

	Index = NewIndex = 0;
	do {
	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CagdRType
		    *MVP = &MV -> Points[i][Index],
		    *NewMVP = &NewMV -> Points[i][NewIndex];

		if (MV -> GType == MVAR_BSPLINE_TYPE) {
		    BSP_CRV_EVAL_VEC_AT_PARAM(NewMVP, MVP,
					      MVAR_NEXT_DIM(MV, Dir),
					      MV -> Orders[Dir],
					      MV -> Lengths[Dir],
					      t, BasisFuncs, IndexFirst);
		}
		else {
		    *NewMVP = BzrCrvEvalVecAtParam(MVP, MVAR_NEXT_DIM(MV, Dir),
						   MV -> Lengths[Dir], t);
		}
	    }
	    MVAR_INCREMENT_MESH_INDICES(NewMV, NewIndices, NewIndex);
	}
	while (MVAR_INC_BOUND_MESH_INDICES(MV, Indices,
					   LowerBound, UpperBound, Index));

	IritFree(LowerBound);
	IritFree(UpperBound);
	IritFree(Indices);
	IritFree(NewIndices);
    }
    else {
	int OldSize = MVAR_CTL_MESH_LENGTH(MV);

	/* Duplicate the old MV in the new MV (-Dir) times. */
	for (i = IsNotRational; i <= MaxCoord; i++) {
	    for (j = 0; j < -Dir; j++)
	        CAGD_GEN_COPY(&NewMV -> Points[i][OldSize * j],
			      MV -> Points[i], sizeof(CagdRType) * OldSize);
	}
    }

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract an isoparametric sub multi variate out of the given tensor product M
* multi-variate, or expand its dimension by one.         		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:    To extract an isoparametric multi-variate from as a sub-mesh	     M
*	   in direction Dir, or exapand its dimension by one.	             M
*   Index: Index of sub mesh of MV's mesh in direction Dir.		     M
*   Dir:   Direction of isosurface extraction.  If Dir is negative, however, M
*	   its absolute value defines the order of a new axis added as last  M
*	   and new dimension to the given MV.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A multi - variate with one less (or more) dimensions.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVReverse, MvarMVFromMVm, MvarPromoteMVToMV			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVFromMesh, multi-variates                                           M
*****************************************************************************/
MvarMVStruct *MvarMVFromMesh(const MvarMVStruct *MV,
			     int Index,
			     MvarMVDirType Dir)
{
    MvarMVStruct *NewMV;
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, *NewLengths, *LowerBound, *UpperBound, *Indices, *NewIndices,
	OldIndex, NewIndex,
	*Orders = MV -> Orders,
	*Lengths = MV -> Lengths,
	Dim = MV -> Dim,
	NewDim = Dim + (Dir >= 0 ? -1 : 1),
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);

    if (Dir < 0) {
	return MvarMVFromMV(MV, 0.0, Dir);       /* Expand dimension by one. */
    }
    if (Dir >= MV -> Dim || NewDim < 1) {
	MVAR_FATAL_ERROR(MVAR_ERR_DIR_NOT_VALID);
	return NULL;
    }
    if (Index < 0 || Index >= MV -> Lengths[Dir]) {
	MVAR_FATAL_ERROR(MVAR_ERR_INDEX_NOT_IN_MESH);
	return NULL;
    }

    NewLengths = (int *) IritMalloc(sizeof(int) * NewDim);
    for (i = 0; i < NewDim; i++)
        NewLengths[i] = Lengths[i < Dir ? i : i + 1];

    if (MVAR_IS_BSPLINE_MV(MV)) {
	int *NewOrders = (int *) IritMalloc(sizeof(int) * NewDim);

	for (i = 0; i < NewDim; i++)
	    NewOrders[i] = Orders[i < Dir ? i : i + 1];

	NewMV = MvarBspMVNew(NewDim, NewLengths, NewOrders, MV -> PType);

	IritFree(NewOrders);

	/* Copy/Create the knot vectors. */
	for (i = 0; i < NewDim; i++) {
	    int l = i < Dir ? i : i + 1;

	    if ((NewMV -> Periodic[i] = MV -> Periodic[l]) != FALSE) {
	        IritFree(NewMV -> KnotVectors[l]);
		NewMV -> KnotVectors[l] = IritMalloc(sizeof(CagdRType) *
				          (MVAR_MVAR_ITH_PT_LST_LEN(NewMV, i) +
					   NewMV -> Orders[i]));
	    }

	    CAGD_GEN_COPY(NewMV -> KnotVectors[i],
			  MV -> KnotVectors[l],
			  sizeof(CagdRType) * (NewMV -> Lengths[i] +
					       NewMV -> Orders[i]));
	}
    }
    else if (MVAR_IS_BEZIER_MV(MV)) {
	NewMV = MvarBzrMVNew(NewDim, NewLengths, MV -> PType);
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    IritFree(NewLengths);

    /* Create the new control mesh. */
    LowerBound = (int *) IritMalloc(sizeof(int) * Dim);
    UpperBound = (int *) IritMalloc(sizeof(int) * Dim);
    Indices = (int *) IritMalloc(sizeof(int) * Dim);
    NewIndices = (int *) IritMalloc(sizeof(int) * NewDim);

    IRIT_ZAP_MEM(LowerBound, sizeof(int) * Dim);
    CAGD_GEN_COPY(UpperBound, Lengths, sizeof(int) * Dim);
    UpperBound[Dir] = 0;
    IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);
    IRIT_ZAP_MEM(NewIndices, sizeof(int) * NewDim);

    /* Original indices follows all indices except in Dir which follows      */
    /* only zero!							     */
    OldIndex = NewIndex = 0;
    do {
	for (i = IsNotRational; i <= MaxCoord; i++) {
	    CagdRType
	        *MVP = &MV -> Points[i][OldIndex],
	        *NewMVP = &NewMV -> Points[i][NewIndex];

	    *NewMVP = MVP[Index * MVAR_NEXT_DIM(MV, Dir)];
	}
	MVAR_INCREMENT_MESH_INDICES(NewMV, NewIndices, NewIndex);
    }
    while (MVAR_INC_BOUND_MESH_INDICES(MV, Indices, LowerBound, UpperBound,
				       OldIndex));

    IritFree(LowerBound);
    IritFree(UpperBound);
    IritFree(Indices);
    IritFree(NewIndices);

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a curve into a multivariate function.                   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Curve to convert into the multi-variate MV.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: A multi variate function representation to Crv.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCrvToMV, multi-variates                                              M
*****************************************************************************/
MvarMVStruct *MvarCrvToMV(const CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    MvarMVStruct *MV;

    if (CAGD_IS_BEZIER_CRV(Crv)) {
	MV = MvarBzrMVNew(1, &Crv -> Length, (MvarPointType) Crv -> PType);
    }
    else if (CAGD_IS_POWER_CRV(Crv)) {
	MV = MvarPwrMVNew(1, &Crv -> Length, (MvarPointType) Crv -> PType);
    }
    else if (CAGD_IS_BSPLINE_CRV(Crv)) {
	MV = MvarBspMVNew(1, &Crv -> Length, &Crv -> Order,
			  (MvarPointType) Crv -> PType);

	if ((MV -> Periodic[0] = Crv -> Periodic) != FALSE) {
	    IritFree(MV -> KnotVectors[0]);
	    MV -> KnotVectors[0] = BspKnotCopy(NULL, Crv -> KnotVector,
				CAGD_CRV_PT_LST_LEN(Crv) + Crv -> Order);
	}
	else {
	    CAGD_GEN_COPY(MV -> KnotVectors[0], Crv -> KnotVector,
		sizeof(CagdRType) * (CAGD_CRV_PT_LST_LEN(Crv) + Crv -> Order));
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(MV -> Points[i], Crv -> Points[i],
		      sizeof(CagdRType) * Crv -> Length);

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Converts a multivariate function into a curve.  If the multivariate is  M
* of dimension higher than one, the lowest dimension is employed in the      M
* conversion.					                     	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:     A multivariate of at least dimension one to convert to a curve.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A curve representation the given multivariate (or its   M
*		lowest dimension if higher dim.).		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVToCrv, multi-variates                                              M
*****************************************************************************/
CagdCrvStruct *MvarMVToCrv(const MvarMVStruct *MV)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdCrvStruct *Crv;

    if (MV -> Dim < 1) {
	MVAR_FATAL_ERROR(MVAR_ERR_MVS_INCOMPATIBLE);
	return NULL;
    }

    if (MVAR_IS_BEZIER_MV(MV)) {
	Crv = BzrCrvNew(MV -> Lengths[0], (CagdPointType) MV -> PType);
    }
    else if (MVAR_IS_POWER_MV(MV)) {
	Crv = PwrCrvNew(MV -> Lengths[0], (CagdPointType) MV -> PType);
    }
    else if (MVAR_IS_BSPLINE_MV(MV)) {
	Crv = BspCrvNew(MV -> Lengths[0], MV -> Orders[0],
			(CagdPointType) MV -> PType);

	if ((Crv -> Periodic = MV -> Periodic[0]) != FALSE) {
	    IritFree(Crv -> KnotVector);
	    Crv -> KnotVector = BspKnotCopy(NULL, MV -> KnotVectors[0],
			MVAR_MVAR_ITH_PT_LST_LEN(MV, 0) + MV -> Orders[0]);
	}
	else {
	    CAGD_GEN_COPY(Crv -> KnotVector, MV -> KnotVectors[0],
		      sizeof(CagdRType) * (MVAR_MVAR_ITH_PT_LST_LEN(MV, 0) +
					   MV -> Orders[0]));
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(Crv -> Points[i], MV -> Points[i],
		      sizeof(CagdRType) * Crv -> Length);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a surface into a multivariate function.                   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to convert into the multi-variate MV.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: A multi variate function representation to Srf.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarSrfToMV, multi-variates                                              M
*****************************************************************************/
MvarMVStruct *MvarSrfToMV(const CagdSrfStruct *Srf)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, Lengths[2],
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    MvarMVStruct *MV;

    Lengths[0] = Srf -> ULength;
    Lengths[1] = Srf -> VLength;

    if (CAGD_IS_BEZIER_SRF(Srf)) {
	MV = MvarBzrMVNew(2, Lengths, (MvarPointType) Srf -> PType);
    }
    else if (CAGD_IS_POWER_SRF(Srf)) {
	MV = MvarPwrMVNew(2, Lengths, (MvarPointType) Srf -> PType);
    }
    else if (CAGD_IS_BSPLINE_SRF(Srf)) {
	int Orders[2];

	Orders[0] = Srf -> UOrder;
	Orders[1] = Srf -> VOrder;
	MV = MvarBspMVNew(2, Lengths, Orders, (MvarPointType) Srf -> PType);

	if ((MV -> Periodic[0] = Srf -> UPeriodic) != FALSE) {
	    IritFree(MV -> KnotVectors[0]);
	    MV -> KnotVectors[0] = BspKnotCopy(NULL, Srf -> UKnotVector,
				CAGD_SRF_UPT_LST_LEN(Srf) +  Srf -> UOrder);
	}
	else {
	    CAGD_GEN_COPY(MV -> KnotVectors[0], Srf -> UKnotVector,
		sizeof(CagdRType) * (CAGD_SRF_UPT_LST_LEN(Srf) +
				     Srf -> UOrder));
	}

	if ((MV -> Periodic[1] = Srf -> VPeriodic) != FALSE) {
	    IritFree(MV -> KnotVectors[1]);
	    MV -> KnotVectors[1] = BspKnotCopy(NULL, Srf -> VKnotVector,
				CAGD_SRF_VPT_LST_LEN(Srf) + Srf -> VOrder);
	}
	else {
	    CAGD_GEN_COPY(MV -> KnotVectors[1], Srf -> VKnotVector,
		sizeof(CagdRType) * (CAGD_SRF_VPT_LST_LEN(Srf) +
				     Srf -> VOrder));
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(MV -> Points[i], Srf -> Points[i],
		      sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a multivariate function into a surface.  If the multivariate is   M
* of dimension higher than two, the lowest two dimensions are employed in    M
* the conversion.				                  	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:    A multivariate of dimension two or more to convert to a surface.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: A surface representation the given multivariate (or     M
*		its lowest two dimensions if higher dim.).		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVToSrf, multi-variates                                              M
*****************************************************************************/
CagdSrfStruct *MvarMVToSrf(const MvarMVStruct *MV)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdSrfStruct *Srf;

    if (MV -> Dim < 2) {
	MVAR_FATAL_ERROR(MVAR_ERR_MVS_INCOMPATIBLE);
	return NULL;
    }

    if (MVAR_IS_BEZIER_MV(MV)) {
	Srf = BzrSrfNew(MV -> Lengths[0], MV -> Lengths[1], 
			(CagdPointType) MV -> PType);
    }
    else if (MVAR_IS_POWER_MV(MV)) {
	Srf = PwrSrfNew(MV -> Lengths[0], MV -> Lengths[1],
			(CagdPointType) MV -> PType);
    }
    else if (MVAR_IS_BSPLINE_MV(MV)) {
	Srf = BspSrfNew(MV -> Lengths[0], MV -> Lengths[1],
			MV -> Orders[0], MV -> Orders[1],
			(CagdPointType) MV -> PType);

	if ((Srf -> UPeriodic = MV -> Periodic[0]) != FALSE) {
	    IritFree(Srf -> UKnotVector);
	    Srf -> UKnotVector = BspKnotCopy(NULL, MV -> KnotVectors[0],
			MVAR_MVAR_ITH_PT_LST_LEN(MV, 0) + MV -> Orders[0]);
	}
	else {
	    CAGD_GEN_COPY(Srf -> UKnotVector, MV -> KnotVectors[0],
		sizeof(CagdRType) * (MVAR_MVAR_ITH_PT_LST_LEN(MV, 0) + 
				     MV -> Orders[0]));
	}

	if ((Srf -> VPeriodic = MV -> Periodic[1]) != FALSE) {
	    IritFree(Srf -> VKnotVector);
	    Srf -> VKnotVector = BspKnotCopy(NULL, MV -> KnotVectors[1],
			MVAR_MVAR_ITH_PT_LST_LEN(MV, 1) + MV -> Orders[1]);
	}
	else {
	    CAGD_GEN_COPY(Srf -> VKnotVector, MV -> KnotVectors[1],
		sizeof(CagdRType) * (MVAR_MVAR_ITH_PT_LST_LEN(MV, 1) + 
				     MV -> Orders[1]));
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(Srf -> Points[i], MV -> Points[i],
		      sizeof(CagdRType) * Srf -> ULength * Srf -> VLength);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a trivar into a multivariate function.                   	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivar to convert into the multi-variate MV.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: A multi variate function representation to TV.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarTVToMV, multi-variates                                               M
*****************************************************************************/
MvarMVStruct *MvarTVToMV(const TrivTVStruct *TV)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, Lengths[3],
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    MvarMVStruct *MV;

    Lengths[0] = TV -> ULength;
    Lengths[1] = TV -> VLength;
    Lengths[2] = TV -> WLength;

    if (TRIV_IS_BEZIER_TV(TV)) {
	MV = MvarBzrMVNew(3, Lengths, (MvarPointType) TV -> PType);
    }
    else if (TRIV_IS_POWER_TV(TV)) {
	MV = MvarPwrMVNew(3, Lengths, (MvarPointType) TV -> PType);
    }
    else if (TRIV_IS_BSPLINE_TV(TV)) {
	int Orders[3];

	Orders[0] = TV -> UOrder;
	Orders[1] = TV -> VOrder;
	Orders[2] = TV -> WOrder;
	MV = MvarBspMVNew(3, Lengths, Orders, (MvarPointType) TV -> PType);

	if ((MV -> Periodic[0] = TV -> UPeriodic) != FALSE) {
	    IritFree(MV -> KnotVectors[0]);
	    MV -> KnotVectors[0] = BspKnotCopy(NULL, TV -> UKnotVector,
				      TRIV_TV_UPT_LST_LEN(TV) + TV -> UOrder);
	}
	else {
	    CAGD_GEN_COPY(MV -> KnotVectors[0], TV -> UKnotVector,
		sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) + TV -> UOrder));
	}

	if ((MV -> Periodic[1] = TV -> VPeriodic) != FALSE) {
	    IritFree(MV -> KnotVectors[1]);
	    MV -> KnotVectors[1] = BspKnotCopy(NULL, TV -> VKnotVector,
				      TRIV_TV_VPT_LST_LEN(TV) + TV -> VOrder);
	}
	else {
	    CAGD_GEN_COPY(MV -> KnotVectors[1], TV -> VKnotVector,
		sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) + TV -> VOrder));
	}

	if ((MV -> Periodic[2] = TV -> WPeriodic) != FALSE) {
	    IritFree(MV -> KnotVectors[2]);
	    MV -> KnotVectors[2] = BspKnotCopy(NULL, TV -> WKnotVector,
				      TRIV_TV_WPT_LST_LEN(TV) + TV -> WOrder);
	}
	else {
	    CAGD_GEN_COPY(MV -> KnotVectors[2], TV -> WKnotVector,
		sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) + TV -> WOrder));
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(MV -> Points[i], TV -> Points[i],
		      sizeof(CagdRType) *
		          TV -> ULength * TV -> VLength * TV -> WLength);

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a multivariate function into a trivar. If the multivariate is     M
* of dimension higher than three, the lowest three dimensions are employed   M
* in the conversion.				                      	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:   A multivariate of dimension three or more to convert to a trivar.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: A trivar representation the given multivariate (or its   M
*		 lowest three dimensions if higher dim.).		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVToTV, multi-variates                                               M
*****************************************************************************/
TrivTVStruct *MvarMVToTV(const MvarMVStruct *MV)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    TrivTVStruct *TV;

    if (MV -> Dim < 3) {
	MVAR_FATAL_ERROR(MVAR_ERR_MVS_INCOMPATIBLE);
	return NULL;
    }

    if (MVAR_IS_BEZIER_MV(MV)) {
	TV = TrivBzrTVNew(MV -> Lengths[0], MV -> Lengths[1], MV -> Lengths[2],
			  (CagdPointType) MV -> PType);
    }
    else if (MVAR_IS_POWER_MV(MV)) {
	TV = TrivPwrTVNew(MV -> Lengths[0], MV -> Lengths[1], MV -> Lengths[2],
			  (CagdPointType) MV -> PType);
    }
    else if (MVAR_IS_BSPLINE_MV(MV)) {
	TV = TrivBspTVNew(MV -> Lengths[0], MV -> Lengths[1], MV -> Lengths[2],
			  MV -> Orders[0], MV -> Orders[1], MV -> Orders[2],
			  (CagdPointType) MV -> PType);

	if ((TV -> UPeriodic = MV -> Periodic[0]) != FALSE) {
	    IritFree(TV -> UKnotVector);
	    TV -> UKnotVector = BspKnotCopy(NULL, MV -> KnotVectors[0],
				      TRIV_TV_UPT_LST_LEN(TV) + TV -> UOrder);
	}
	else {
	    CAGD_GEN_COPY(TV -> UKnotVector, MV -> KnotVectors[0],
		sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) + TV -> UOrder));
	}

	if ((TV -> VPeriodic = MV -> Periodic[1]) != FALSE) {
	    IritFree(TV -> VKnotVector);
	    TV -> VKnotVector = BspKnotCopy(NULL, MV -> KnotVectors[1],
				      TRIV_TV_VPT_LST_LEN(TV) + TV -> VOrder);
	}
	else {
	    CAGD_GEN_COPY(TV -> VKnotVector, MV -> KnotVectors[1],
		sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) + TV -> VOrder));
	}

	if ((TV -> WPeriodic = MV -> Periodic[2]) != FALSE) {
	    IritFree(TV -> WKnotVector);
	    TV -> WKnotVector = BspKnotCopy(NULL, MV -> KnotVectors[2],
				      TRIV_TV_WPT_LST_LEN(TV) + TV -> WOrder);
	}
	else {
	    CAGD_GEN_COPY(TV -> WKnotVector, MV -> KnotVectors[2],
		sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) + TV -> WOrder));
	}
    }
    else {
	MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	return NULL;
    }

    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(TV -> Points[i], MV -> Points[i],
		      sizeof(CagdRType) *
		          TV -> ULength * TV -> VLength * TV -> WLength);

    return TV;
}
