/******************************************************************************
* Mvar_Der.c - Compute derivatives of multi-variates.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, computes its partial derivative multi-variate in    M
* direction Dir.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   Dir:      Direction of differentiation.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Differentiated multi-variate in direction Dir.         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDeriveBound, MvarBzrMVDerive, MvarBspMVDerive                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDerive, multi-variates                                             M
*****************************************************************************/
MvarMVStruct *MvarMVDerive(const MvarMVStruct *MV, MvarMVDirType Dir)
{
    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	    return MvarBzrMVDerive(MV, Dir);
	case MVAR_BSPLINE_TYPE:
	    return MvarBspMVDerive(MV, Dir);
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier multi-variate, computes its partial derivative		     M
* multi-variate in direction Dir.					     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.	     M
*   Then:								     M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
*   This function computes the derivative of a rational function component-  M
* wise with out taking into consideration the quotient rule.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   Dir:      Direction of differentiation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Differentiated multi-variate in direction Dir. A       M
*		      Bezier multi-variate.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDeriveBound, MvarMVDerive, MvarBspMVDerive                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVDerive, multi-variates                                          M
*****************************************************************************/
MvarMVStruct *MvarBzrMVDerive(const MvarMVStruct *MV, MvarMVDirType Dir)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int l, DIndex,
	Length = MV -> Lengths[Dir],
	*Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim),
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdRType **DPoints;
    MvarMVStruct
        *DerivedMV = NULL;

    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);

    if (Length > 1)
        MV -> Lengths[Dir]--;
    DerivedMV = MvarBzrMVNew(MV -> Dim, MV -> Lengths, MV -> PType);
    if (Length > 1)
        MV -> Lengths[Dir]++;

    DPoints = DerivedMV -> Points;
    DIndex = 0;
    do {
	int Index = MvarGetPointsMeshIndices(MV, Indices),
	    NextIndex = Index + MVAR_NEXT_DIM(MV, Dir);

	for (l = IsNotRational; l <= MaxCoord; l++)
	    DPoints[l][DIndex] = Length < 2 ? 0.0
			                    : (Length - 1) *
						 (MV -> Points[l][NextIndex] -
						  MV -> Points[l][Index]);
    }
    while (MVAR_INCREMENT_MESH_INDICES(DerivedMV, Indices, DIndex));

    IritFree(Indices);

    return DerivedMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline multi-variate, computes its partial derivative             M
* multi-variate in direction Dir.					     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.	     M
*   Then:								     M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*   This function computes the derivative of a rational function component-  M
* wise with out taking into consideration the quotient rule.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   Dir:      Direction of differentiation.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   Differentiated multi-variate in direction Dir. A       M
*		      Bspline multi-variate.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDeriveBound, MvarBzrMVDerive, MvarMVDerive                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVDerive, multi-variates                                          M
*****************************************************************************/
MvarMVStruct *MvarBspMVDerive(const MvarMVStruct *MV, MvarMVDirType Dir)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, DIndex,
	NewLength, NewOrder,
	Length = MV -> Lengths[Dir],
	Order = MV -> Orders[Dir],
	*Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim),
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdRType **DPoints,
	*KV = MV -> KnotVectors[Dir],
	* const *Points = MV -> Points;
    MvarMVStruct *CpMV,
        *DerivedMV = NULL;

    if (MvarBspMVIsPeriodic(MV)) {
        MV = CpMV = MvarCnvrtPeriodic2FloatMV(MV);
	Length = MV -> Lengths[Dir];
	KV = MV -> KnotVectors[Dir];
	Points = MV -> Points;
    }
    else
        CpMV = NULL;

    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);

    NewLength = Order < 2 ? Length : Length - 1;
    NewOrder = IRIT_MAX(Order - 1, 1);

    MV -> Lengths[Dir] = NewLength;
    MV -> Orders[Dir] = NewOrder;
    DerivedMV = MvarBspMVNew(MV -> Dim, MV -> Lengths, MV -> Orders,
			     MV -> PType);
    MV -> Lengths[Dir] = Length;
    MV -> Orders[Dir] = Order;

    for (i = 0; i < MV -> Dim; i++) {
	if (i == Dir)
	    CAGD_GEN_COPY(DerivedMV -> KnotVectors[i],
			  &MV -> KnotVectors[i][Order < 2 ? 0 : 1],
			  sizeof(CagdRType) * (NewLength + NewOrder));
	else
	    CAGD_GEN_COPY(DerivedMV -> KnotVectors[i], MV -> KnotVectors[i],
			  sizeof(CagdRType) * (MV -> Lengths[i] +
					       MV -> Orders[i]));
    }

    DPoints = DerivedMV -> Points;
    DIndex = 0;

    do {
	int l,
	    Index = MvarGetPointsMeshIndices(MV, Indices),
	    NextIndex = Index + MVAR_NEXT_DIM(MV, Dir);
	CagdRType
	    Denom = KV[Indices[Dir] + Order] - KV[Indices[Dir] + 1];

	if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
	    Denom = IRIT_UEPS;

	for (l = IsNotRational; l <= MaxCoord; l++) {
	    DPoints[l][DIndex] =
	        Order < 2 ? 0.0 : (Order - 1) *
				      (Points[l][NextIndex] -
				       Points[l][Index]) / Denom;
	}
    }
    while (MVAR_INCREMENT_MESH_INDICES(DerivedMV, Indices, DIndex));

    IritFree(Indices);

    if (CpMV != NULL)
	MvarMVFree(CpMV);

    return DerivedMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, computes its partial derivative multi-variate in    M
* direction Dir.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   Dir:      Direction of differentiation.			             M
*   MinMax:   Bounds on the derivative values of MV in direction Dir.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarBzrMVDeriveBound, MvarBspMVDeriveBound                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDeriveBound, multi-variates                                        M
*****************************************************************************/
void MvarMVDeriveBound(const MvarMVStruct *MV,
		       MvarMVDirType Dir,
		       CagdRType MinMax[2])
{
    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	    MvarBzrMVDeriveBound(MV, Dir, MinMax);
	    break;
	case MVAR_BSPLINE_TYPE:
	    MvarBspMVDeriveBound(MV, Dir, MinMax);
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar Bezier multi-variate, computes bounds to its partial        M
* derivative in direction Dir.						     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.	     M
*   Then:								     M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   Dir:      Direction of differentiation.                                  M
*   MinMax:   Bounds on the derivative values of MV in direction Dir.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarMVDeriveBound, MvarBspMVDeriveBound                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVDeriveBound, multi-variates                                     M
*****************************************************************************/
void MvarBzrMVDeriveBound(const MvarMVStruct *MV,
			  MvarMVDirType Dir,
			  CagdRType MinMax[2])
{
    int DIndex,
	Length = MV -> Lengths[Dir],
        *LowBound = (int *) IritMalloc(sizeof(int) * MV -> Dim),
        *UpBound = (int *) IritMalloc(sizeof(int) * MV -> Dim),
        *Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);

    assert(MVAR_NUM_OF_MV_COORD(MV) == 1);

    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);

    DIndex = 0;
    MinMax[0] = IRIT_INFNTY;
    MinMax[1] = -IRIT_INFNTY;

    IRIT_ZAP_MEM(LowBound, sizeof(int) * MV -> Dim);
    CAGD_GEN_COPY(UpBound, MV -> Lengths, MV -> Dim * sizeof(int));
    UpBound[Dir]--;

    do {
	int Index = MvarGetPointsMeshIndices(MV, Indices),
	    NextIndex = Index + MVAR_NEXT_DIM(MV, Dir);
	CagdRType Der;
	    Der = Length < 2 ? 0.0 : (Length - 1) *
	                                        (MV -> Points[1][NextIndex] -
						 MV -> Points[1][Index]);
	if (MinMax[0] > Der)
	    MinMax[0] = Der;
	if (MinMax[1] < Der)
	    MinMax[1] = Der;
    }
    while (MVAR_INC_BOUND_MESH_INDICES(MV, Indices, LowBound, UpBound, DIndex));

    IritFree(Indices);
    IritFree(LowBound);
    IritFree(UpBound);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar B-spline multi-variate, computes bounds to its partial      M
* derivative in direction Dir.						     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.	     M
*   Then:								     M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   Dir:      Direction of differentiation.                                  M
*   MinMax:   Bounds on the derivative values of MV in direction Dir.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarBzrMVDeriveBound, MvarMVDeriveBound                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVDeriveBound, multi-variates                                     M
*****************************************************************************/
void MvarBspMVDeriveBound(const MvarMVStruct *MV,
			  MvarMVDirType Dir,
			  CagdRType MinMax[2])
{
    int DIndex,
	Dim = MV -> Dim,
        Length = MV -> Lengths[Dir],
	Order = MV -> Orders[Dir],
	*Indices = (int *) IritMalloc(sizeof(int) * Dim * 3),
        *LowBound = &Indices[Dim],
        *UpBound = &Indices[Dim * 2];
    CagdRType
	*KV = MV -> KnotVectors[Dir],
	* const *Points = MV -> Points;
    MvarMVStruct *CpMV;

    assert(MVAR_NUM_OF_MV_COORD(MV) == 1);

    if (MvarBspMVIsPeriodic(MV)) {
        MV = CpMV = MvarCnvrtPeriodic2FloatMV(MV);
	Length = MV -> Lengths[Dir];
	KV = MV -> KnotVectors[Dir];
	Points = MV -> Points;
    }
    else
        CpMV = NULL;

    DIndex = 0;
    MinMax[0] = IRIT_INFNTY;
    MinMax[1] = -IRIT_INFNTY;

    /* Clear Indices and LowBound. */
    IRIT_ZAP_MEM(Indices, sizeof(int) * Dim * 2);

    CAGD_GEN_COPY(UpBound, MV -> Lengths, Dim * sizeof(int));

    UpBound[Dir]--;

    do {
	int Index = MvarGetPointsMeshIndices(MV, Indices),
	    NextIndex = Index + MVAR_NEXT_DIM(MV, Dir);
	CagdRType Der,
	    Denom = KV[Indices[Dir] + Order] - KV[Indices[Dir] + 1];

	Der = Order < 2 ? 0.0 : (Order - 1) *
				      (Points[1][NextIndex] -
				       Points[1][Index]) /
						   IRIT_MAX(Denom, IRIT_UEPS);
	if (MinMax[0] > Der)
	    MinMax[0] = Der;
	if (MinMax[1] < Der)
	    MinMax[1] = Der;
    }
    while (MVAR_INC_BOUND_MESH_INDICES(MV, Indices, LowBound, UpBound, DIndex));

    IritFree(Indices);

    if (CpMV != NULL)
	MvarMVFree(CpMV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, computes its partial derivative multi-variate in    M
* all directions.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   MinMax:   Bounds on the derivative values of MV in all directions.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarBzrMVDeriveBound, MvarBspMVDeriveBound                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDeriveAllBounds, multi-variates                                    M
*****************************************************************************/
void MvarMVDeriveAllBounds(const MvarMVStruct *MV, CagdMinMaxType *MinMax)
{
    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	    MvarBzrMVDeriveAllBounds(MV, MinMax);
	    break;
	case MVAR_BSPLINE_TYPE:
	    MvarBspMVDeriveAllBounds(MV, MinMax);
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar Bezier multi-variate, computes bounds to its partial        M
* derivative in all directions.						     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.	     M
*   Then:								     M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   MinMax:   Bounds on the derivative values of MV in all directions.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarMVDeriveBound, MvarBspMVDeriveBound                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrMVDeriveAllBounds, multi-variates                                 M
*****************************************************************************/
void MvarBzrMVDeriveAllBounds(const MvarMVStruct *MV, CagdMinMaxType *MinMax)
{
    int i,
	Index = 0,
	Dim = MV -> Dim,
	*Orders = MV -> Orders,
	*Lengths = MV -> Lengths,
	*Indices = (int *) IritMalloc(sizeof(int) * Dim);

    assert(MVAR_NUM_OF_MV_COORD(MV) == 1);

    for (i = 0; i < Dim; i++) {
        MinMax[i][0] = IRIT_INFNTY;
	MinMax[i][1] = -IRIT_INFNTY;
    }

    IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);	         /* Clear Indices. */

    do {
        int Dir, NextIndex, Len1,
	    Index = MvarGetPointsMeshIndices(MV, Indices);

	for (Dir = 0; Dir < Dim; Dir++) {
	    CagdRType Der;

	    if (Indices[Dir] >= (Len1 = (Lengths[Dir] - 1))) {
		if (Orders[Dir] <= 1) /* Zero derivative for constant func.*/
		    MinMax[Dir][0] = MinMax[Dir][1] = 0.0;
	        continue;       /* Last location in this direction - skip. */
	    }

	    NextIndex = Index + MVAR_NEXT_DIM(MV, Dir);

	    Der = (Len1 < 1 ? 0.0 : Len1) * (MV -> Points[1][NextIndex] -
						     MV -> Points[1][Index]);
	    if (MinMax[Dir][0] > Der)
	        MinMax[Dir][0] = Der;
	    if (MinMax[Dir][1] < Der)
	        MinMax[Dir][1] = Der;
	}
    }
    while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));

    IritFree(Indices);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a scalar B-spline multi-variate, computes bounds to its partial      M
* derivative in all directions.						     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one.	     M
*   Then:								     M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multi-Variate to differentiate.                                M
*   MinMax:   Bounds on the derivative values of MV in all directions.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarBzrMVDeriveBound, MvarMVDeriveBound                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVDeriveAllBounds, multi-variates                                 M
*****************************************************************************/
void MvarBspMVDeriveAllBounds(const MvarMVStruct *MV, IrtMinMaxType *MinMax)
{
    int i,
	Index = 0,
	Dim = MV -> Dim,
        *Lengths = MV -> Lengths,
	*Orders = MV -> Orders,
	*Indices = (int *) IritMalloc(sizeof(int) * Dim);
    CagdRType
	**KVs = MV -> KnotVectors,
	* const *Points = MV -> Points;
    MvarMVStruct *CpMV;

    assert(MVAR_NUM_OF_MV_COORD(MV) == 1);

    for (i = 0; i < Dim; i++) {
        MinMax[i][0] = IRIT_INFNTY;
	MinMax[i][1] = -IRIT_INFNTY;
    }

    if (MvarBspMVIsPeriodic(MV)) {
        MV = CpMV = MvarCnvrtPeriodic2FloatMV(MV);
	Lengths = MV -> Lengths;
	KVs = MV -> KnotVectors;
	Points = MV -> Points;
    }
    else
        CpMV = NULL;

    IRIT_ZAP_MEM(Indices, sizeof(int) * Dim);	         /* Clear Indices. */

    do {
        int Dir, NextIndex;

        for (Dir = 0; Dir < Dim; Dir++) {
	    CagdRType Der, Denom;
	    int Order = Orders[Dir];

	    if (Indices[Dir] >= Lengths[Dir] - 1) {
		if (Order <= 1)    /* Zero derivative for a constant func. */
		    MinMax[Dir][0] = MinMax[Dir][1] = 0.0;
	        continue;       /* Last location in this direction - skip. */
	    }

	    NextIndex = Index + MVAR_NEXT_DIM(MV, Dir);
	    Denom = KVs[Dir][Indices[Dir] + Order] -
	            KVs[Dir][Indices[Dir] + 1];

	    Der = (Order < 2 ? 0.0 : Order - 1) * (Points[1][NextIndex] -
						   Points[1][Index]) /
						   IRIT_MAX(Denom, IRIT_UEPS);
	    if (MinMax[Dir][0] > Der)
	        MinMax[Dir][0] = Der;
	    if (MinMax[Dir][1] < Der)
	        MinMax[Dir][1] = Der;
	}
    }
    while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));

    IritFree(Indices);

    if (CpMV != NULL)
	MvarMVFree(CpMV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Builds a gradient for the given scalar multivariate.		     M
*   If the input is rational, returned is a dynamically allocated vector of  M
* scalar multivariate functions each representing Dm/Dui, i from 1 to Dim.   M
*   The returned partial derivative are differentiated directly without the  M
* quotient rule which must be applied manually.				     M
*   Otherwise, if the input is polynomial, the gradient is returned as one   M
* vector function.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:    Input scalar field to compute its gradient function.              M
*   Orig:  If orig TRUE and input is polynomial, the original scalar MV is   M
*	   also placed as last, additional, dimension (for faster evaluation M
*	   of MV and its gradient).					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVGradientStruct *: The gradient function of the input scalar field. M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarMVFreeGradient, MvarMVEvalGradient                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVPrepGradient                                                       M
*****************************************************************************/
MvarMVGradientStruct *MvarMVPrepGradient(const MvarMVStruct *MV,
					 CagdBType Orig)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_MV(MV);
    int i, 
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVGradientStruct *MVGrad;

    if (MV -> Dim >= MVAR_MAX_PT_COORD) {
	MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH);
	return NULL;
    }

    MVGrad = (MvarMVGradientStruct *) IritMalloc(sizeof(MvarMVGradientStruct));
    IRIT_ZAP_MEM(MVGrad, sizeof(MvarMVGradientStruct));

    MVGrad -> Dim = MV -> Dim;
    MVGrad -> IsRational = IsRational;
    MVGrad -> MV = MvarMVCopy(MV);
    MVGrad -> HasOrig = FALSE;
    MVGrad -> MVGrad = NULL;

    for (i = 0; i < MV -> Dim; i++)
        MVGrad -> MVRGrad[i + 1] = MvarMVDerive(MV, i);

    if (!IsRational) {
	if (Orig) {
	    MVGrad -> Dim++;
	    if (MVGrad -> Dim >= MVAR_MAX_PT_COORD)
		MVAR_FATAL_ERROR(MVAR_ERR_DIM_TOO_HIGH);
	    MVGrad -> MVRGrad[MV -> Dim + 1] = MvarMVCopy(MV);
	    MVGrad -> HasOrig = TRUE;
	}

	if (MaxCoord == 1) {
	    MVGrad -> MVGrad = MvarMVMergeScalar(MVGrad -> MVRGrad);

	    for (i = 0; i < MVGrad -> Dim; i++)
		MvarMVFree(MVGrad -> MVRGrad[i + 1]);
	}
    }

    return MVGrad;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free an gradient function.				                     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVGrad:   Gradient function to free.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarMVPrepGradient, MvarMVEvalGradient                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVFreeGradient                                                       M
*****************************************************************************/
void MvarMVFreeGradient(MvarMVGradientStruct *MVGrad)
{
    if (MVGrad -> MVGrad == NULL) {
	int i;

	for (i = 0; i < MVGrad -> Dim; i++)
	    MvarMVFree(MVGrad -> MVRGrad[i + 1]);
    }
    else {
        MvarMVFree(MVGrad -> MVGrad);
    }

    MvarMVFree(MVGrad -> MV);

    IritFree(MVGrad);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluates the gradient function at the given parametric location.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVGrad:   Input gradient function to evaluate at.                        M
*   Params:   Parametric location to evaluate gradient at.                   M
*   Axis:     If the input function whose gradient we seek is scalar, Axis   M
*             will always be zero.  However we can also handle input vector  M
*             functions in which case Axis specifies which function in the   M
*             vector to compute the gradient for - 0 for X, 1 for Y, etc.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   The gradient at Params.			             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarMVFreeGradient, MvarMVPrepGradient,                    M
*   MvarMVEvalGradient2, MvarMVBoundGradient				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVEvalGradient                                                       M
*****************************************************************************/
CagdRType *MvarMVEvalGradient(const MvarMVGradientStruct *MVGrad,
			      CagdRType *Params,
			      int Axis)
{
    IRIT_STATIC_DATA CagdRType Grad[MVAR_MAX_PT_COORD];
    CagdRType *R;

    if (MVGrad -> MVGrad == NULL) {
        int i;

	if (MVGrad -> IsRational) {
	    int MaxCoord = MVAR_NUM_OF_MV_COORD(MVGrad -> MV);
	    CagdRType Pos[MVAR_MAX_PT_COORD];

	    /* Evaluate multivariate itself. */
	    R = MvarMVEval(MVGrad -> MV, Params);
	    CAGD_GEN_COPY(Pos, R, sizeof(CagdRType) * (MaxCoord + 1));

	    for (i = 0; i < MVGrad -> Dim; i++) {
		R = MvarMVEval(MVGrad -> MVRGrad[i + 1], Params);

		/* Apply the quotient rule: */
		Grad[i] = (R[Axis + 1] * Pos[0] -
			   R[0] * Pos[Axis + 1]) / IRIT_SQR(Pos[0]);
	    }
	}
	else {
	    for (i = 0; i < MVGrad -> Dim; i++) {
		R = MvarMVEval(MVGrad -> MVRGrad[i + 1], Params);

		Grad[i] = R[Axis + 1];
	    }
	}
    }
    else {
        R = MvarMVEval(MVGrad -> MVGrad, Params);
        CAGD_GEN_COPY(Grad, &R[1], sizeof(CagdRType) * MVGrad -> Dim);
    }

    return Grad;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Provides a set of vectors that bounds the gradient function of given MV  M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Input MV function to compute bounds on its gradient.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVGradientStruct *:   Holding the set of vectors bounding the        M
*			      gradient of MV in the MVGrad slot.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDerive, MvarMVFreeGradient, MvarMVPrepGradient,                    M
*   MvarMVEvalGradient, MvarMVEvalGradient2				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVBoundGradient                                                      M
*****************************************************************************/
MvarMVGradientStruct *MvarMVBoundGradient(const MvarMVStruct *MV)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_MV(MV);
    int i, Bit, Len, *Lengths,
        Dim = MV -> Dim;
    CagdMinMaxType *Dmns;
    MvarMVGradientStruct *MVGrad;
    MvarMVStruct *MVG;

    assert(!IsRational);
    if (IsRational)
        return NULL;

    MVGrad = (MvarMVGradientStruct *) IritMalloc(sizeof(MvarMVGradientStruct));
    IRIT_ZAP_MEM(MVGrad, sizeof(MvarMVGradientStruct));

    MVGrad -> Dim = MV -> Dim;
    MVGrad -> IsRational = IsRational;
    MVGrad -> MV = NULL;
    MVGrad -> HasOrig = FALSE;

    Lengths = (int *) IritMalloc(sizeof(int) * Dim);
    for (i = 0; i < Dim; i++)
        Lengths[i] = 2;
    MVG = MVGrad -> MVGrad = MvarBzrMVNew(Dim, Lengths,
					  MVAR_MAKE_PT_TYPE(FALSE, Dim));
    IritFree(Lengths);

    /* Get bounds on the derivatives in all directions. */
    Dmns = (CagdMinMaxType *) IritMalloc(sizeof(CagdMinMaxType) * Dim);
    MvarMVDeriveAllBounds(MV, Dmns);
    for (i = 0; i < Dim; i++) {
	MVG -> Points[i + 1][0] = Dmns[i][0];
	MVG -> Points[i + 1][1] = Dmns[i][1];
    }
    IritFree(Dmns);

    /* Now spread the min max values over all 2^Dim ctrlpts. */
    Len = MVAR_CTL_MESH_LENGTH(MVG);
    for (i = 0, Bit = 1; i < Dim; i++, Bit <<= 1) {
        int j;
        CagdRType
	    Min = MVG -> Points[i + 1][0],
	    Max = MVG -> Points[i + 1][1];

	for (j = 0; j < Len; j++)
	    MVG -> Points[i + 1][j] = (j & Bit) ? Max : Min;
    }


    return MVGrad;
}
