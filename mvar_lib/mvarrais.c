/******************************************************************************
* MvarRais.c - Degree raising for multivariate functions.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July 1997.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new multivariate representing the same curve as MV but with its  M
* degree raised by the NewOrders prescription.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To raise its degree.                                         M
*   NewOrders:  A vector prescribing the new orders of MV.  Length of this   M
*		vector is MV -> Dim.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A multivariate with same geometry as MV but with        M
*                    higher degrees.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDegreeRaise, MvarMVPwrDegreeRaise, MvarMVDegreeRaiseN2             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDegreeRaiseN, degree raising                                       M
*****************************************************************************/
MvarMVStruct *MvarMVDegreeRaiseN(const MvarMVStruct *MV, int *NewOrders)
{
    CagdBType 
	IsBezier = FALSE;
    CagdPointType
	PType = MV -> PType;
    int i, j, k, l, NewLen, StepSize, RStepSize, *Indices, *RIndices,
	Dim = MV -> Dim;
    MvarMVStruct *RMV, *TMV,
	*CpMV = NULL;
    CagdBlsmAlphaCoeffStruct *A;

    if (MVAR_IS_POWER_MV(MV)) {  /* Power degree raising simply adds zeros. */
        for (i = 0; i < Dim; i++) {
	    if (MV -> Orders[i] < NewOrders[i]) {
	        RMV = MvarMVPwrDegreeRaise(MV, i,
					   NewOrders[i] - MV -> Orders[i]);
		if (CpMV != NULL)
		    MvarMVFree(CpMV);
		MV = CpMV = RMV;
	    }
	}

        return CpMV != NULL ? CpMV : MvarMVCopy(MV);
    }

    if (MVAR_IS_BEZIER_MV(MV)) {         /* Convert to a Bspline trivariate. */
	IsBezier = TRUE;
	MV = CpMV = MvarCnvrtBzr2BspMV(MV);
    }
    else
        CpMV = NULL;

    for (i = 0; i < Dim; i++) {
        CagdRType t;

        if (BspKnotC0Discont(MV -> KnotVectors[i], MV -> Orders[i],
			     MV -> Lengths[i], &t)) {
	    MvarMVStruct
	        *MVs = MvarMVSubdivAtParam(MV, t, i),
	        *RMV1 = MvarMVDegreeRaiseN(MVs, NewOrders),
	        *RMV2 = MvarMVDegreeRaiseN(MVs -> Pnext, NewOrders);

	    if (CpMV != NULL)
	        MvarMVFree(CpMV);

	    MvarMVFreeList(MVs);
	    RMV = MvarMergeMVMV(RMV1, RMV2, i, TRUE);
	    MvarMVFree(RMV1);
	    MvarMVFree(RMV2);

	    return RMV;
	}
    }

    Indices = (int *) IritMalloc(sizeof(int) * Dim);
    RIndices = (int *) IritMalloc(sizeof(int) * Dim);

    /* We now have an open end Bspline mvar to deal with - compute degree   */
    /* raising matrices for this setup and apply them - axis by axis.       */

    for (k = 0; k < Dim; k++) {
        /* Skip direction that requires no degree raising. */
	if (MV -> Orders[k] >= NewOrders[k])
	    continue;

	A = CagdBlossomDegreeRaiseNMat(MV -> KnotVectors[k], MV -> Orders[k],
				       NewOrders[k], MV -> Lengths[k]);
	NewLen = A -> NewLength;
	
	/* Allocate space for the new raised multivariate, using MV vectors. */
	IRIT_SWAP(int, MV -> Lengths[k], NewLen);
	IRIT_SWAP(int, MV -> Orders[k], NewOrders[k]);
	RMV = MvarBspMVNew(Dim, MV -> Lengths, MV -> Orders, MV -> PType);
	IRIT_SWAP(int, MV -> Lengths[k], NewLen);
	IRIT_SWAP(int, MV -> Orders[k], NewOrders[k]);

	/* Update the knot vectors. */
	for (i = 0; i < Dim; i++) {
	    if (i == k) {
	        IRIT_GEN_COPY(RMV -> KnotVectors[i], A -> NewKV,
			 sizeof(CagdRType) * (NewLen + RMV -> Orders[i]));
	    }
	    else {
	        CAGD_GEN_COPY(RMV -> KnotVectors[i], MV -> KnotVectors[i],
			      sizeof(CagdRType) * (MV -> Lengths[i]
						          + MV -> Orders[i]));
	    }
	}

	RStepSize = MVAR_NEXT_DIM(RMV, k);
	StepSize = MVAR_NEXT_DIM(MV, k);

	/* And apply the blossom alpha matrix to the control points. */
	for (i = !CAGD_IS_RATIONAL_PT(PType);
	     i <= CAGD_NUM_OF_PT_COORD(PType);
	     i++) {
	    int Index = 0,
	        RIndex = 0;

	    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);
	    IRIT_ZAP_MEM(RIndices, sizeof(int) * RMV -> Dim);

	    do {
	        CagdRType
		    *np = &RMV -> Points[i][RIndex],
		    *p = &MV -> Points[i][Index];

		for (l = 0; l < NewLen; l++, np += RStepSize) {
		    CagdRType
		        *BlendVals = &A -> Rows[l][A -> ColIndex[l]],
		        *pp = &p[A -> ColIndex[l] * StepSize];

		    *np = 0.0;
		    for (j = 0; j < A -> ColLength[l]; j++, pp += StepSize)
		        *np += *pp * *BlendVals++;
		}

		MVAR_INC_SKIP_MESH_INDICES(RMV, RIndices, k, RIndex);
	    }
	    while (MVAR_INC_SKIP_MESH_INDICES(MV, Indices, k, Index));
	}

	CagdBlsmFreeAlphaCoef(A);

	/* Update MV in case we need to raise degree in a different axis. */
	if (CpMV != NULL)
	    MvarMVFree(CpMV);
	MV = CpMV = RMV;
    }

    IritFree(Indices);
    IritFree(RIndices);

    if (IsBezier) {
	TMV = MvarCnvrtBsp2BzrMV(MV);
	if (CpMV != NULL)
	    MvarMVFree(CpMV);
	MV = CpMV = TMV;
    }

    return CpMV != NULL ? CpMV : MvarMVCopy(MV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new multivariate representing the same curve as MV but with its  M
* degree raised by the NewOrders prescription.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To raise its degree.                                         M
*   NewOrders:  A vector prescribing the new orders of MV.  Length of this   M
*		vector is MV -> Dim.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A multivariate with same geometry as MV but with        M
*                    higher degrees.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDegreeRaise, MvarMVPwrDegreeRaise, MvarMVDegreeRaiseN              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDegreeRaiseN2, degree raising                                      M
*****************************************************************************/
MvarMVStruct *MvarMVDegreeRaiseN2(MvarMVStruct *MV, int *NewOrders)
{
    CagdBType
	IsBezier = MVAR_IS_BEZIER_MV(MV);
    int i, j, *Orders,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *UnitMV, *CpMV,
	*RaisedMV = NULL;

    if (MVAR_IS_POWER_MV(MV)) {  /* Power degree raising simply adds zeros. */
	CpMV = MvarMVCopy(MV);

        for (i = 0; i < CpMV -> Dim; i++) {
	    if (NewOrders[i] < CpMV -> Orders[i]) {
	        MVAR_FATAL_ERROR(MVAR_ERR_WRONG_ORDER);
		return NULL;
	    }
	    else if (NewOrders[i] > CpMV -> Orders[i]) {
	        RaisedMV = MvarMVPwrDegreeRaise(CpMV, i,
						NewOrders[i] - MV -> Orders[i]);
		MvarMVFree(CpMV);
		CpMV = RaisedMV;
	    }
	}

	return CpMV;
    }

    /* Prepare a unit valued multivariate that is constant in all axes but  */
    /* the one we would like to raise degree, where that degree is linear.  */
    Orders = (int *) IritMalloc(MV -> Dim * sizeof(int));
    for (i = 0; i < MV -> Dim; i++) {
        if (NewOrders[i] < MV -> Orders[i]) {
	    MVAR_FATAL_ERROR(MVAR_ERR_WRONG_ORDER);
	    return NULL;
	}

	Orders[i] = NewOrders[i] - MV -> Orders[i] + 1;
    }

    if (IsBezier)
        UnitMV = MvarBzrMVNew(MV -> Dim, Orders,
			      MVAR_MAKE_PT_TYPE(FALSE, MaxCoord));
    else {
        UnitMV = MvarBspMVNew(MV -> Dim, Orders, Orders,
			      MVAR_MAKE_PT_TYPE(FALSE, MaxCoord));

	for (i = 0; i < MV -> Dim; i++) {
	    CagdRType Min, Max,
	        *KV = UnitMV -> KnotVectors[i];

	    MvarMVDomain(MV, &Min, &Max, i);

	    for (j = 0; j < Orders[i]; j++)
	        *KV++ = Min;
	    for (j = 0; j < Orders[i]; j++)
	        *KV++ = Max;
	}
    }

    IritFree(Orders);

    for (j = 0; j < MVAR_CTL_MESH_LENGTH(UnitMV); j++)
	for (i = 1; i <= MaxCoord; i++)
	    UnitMV -> Points[i][j] = 1.0;

    if (IsBezier)
        RaisedMV = MvarBzrMVMult(MV, UnitMV);
    else
        RaisedMV = MvarBspMVMult(MV, UnitMV);

    MvarMVFree(UnitMV);

    return RaisedMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new multivariate representing the same curve as MV but with its  M
* degree raised by one.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To raise its degree.                                           M
*   Dir:      Direction of degree raising.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A multivariate with same geometry as MV but with one    M
*                    degree higher.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDegreeRaiseN, MvarMVDegreeRaise2, MvarMVPwrDegreeRaise             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDegreeRaise, degree raising                                        M
*****************************************************************************/
MvarMVStruct *MvarMVDegreeRaise(const MvarMVStruct *MV, MvarMVDirType Dir)
{
    IRIT_STATIC_DATA int
        AllocDim = 0,
	*Orders = NULL;

    if (MVAR_IS_POWER_MV(MV)) {  /* Power degree raising simply adds zeros. */
        return MvarMVPwrDegreeRaise(MV, Dir, 1);
    }

    if (AllocDim < MV -> Dim) {
        if (Orders == NULL)
	    IritFree(Orders);
	AllocDim = MV -> Dim * 2;
	Orders = (int *) IritMalloc(sizeof(int) * AllocDim);
    }

    IRIT_GEN_COPY(Orders, MV -> Orders, sizeof(int) * MV -> Dim);
    Orders[Dir]++;

    return MvarMVDegreeRaiseN(MV, Orders);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new multivariate representing the same curve as MV but with its  M
* degree raised by one.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To raise its degree.                                           M
*   Dir:      Direction of degree raising.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A multivariate with same geometry as MV but with one    M
*                    degree higher.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDegreeRaise, , MvarMVDegreeRaise3, MvarMVDegreeRaiseN,	     M
*   MvarMVPwrDegreeRaise					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDegreeRaise2, degree raising                                       M
*****************************************************************************/
MvarMVStruct *MvarMVDegreeRaise2(MvarMVStruct *MV, MvarMVDirType Dir)
{
    CagdBType
	IsBezier = MV -> GType == MVAR_BEZIER_TYPE;
    int i, j, *Orders,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *UnitMV,
	*RaisedMV = NULL;

    if (MVAR_IS_POWER_MV(MV)) {  /* Power degree raising simply adds zeros. */
        return MvarMVPwrDegreeRaise(MV, Dir, 1);
    }

    /* Prepare a unit valued multivariate that is constant in all axes but  */
    /* the one we would like to raise degree, where that degree is linear.  */
    Orders = (int *) IritMalloc(MV -> Dim * sizeof(int));
    for (i = 0; i < MV -> Dim; i++)
	Orders[i] = i == Dir ? 2 : 1;

    if (IsBezier) {
	UnitMV = MvarBzrMVNew(MV -> Dim, Orders,
			      MVAR_MAKE_PT_TYPE(FALSE, MaxCoord));
    }
    else {
	UnitMV = MvarBspMVNew(MV -> Dim, Orders, Orders,
			      MVAR_MAKE_PT_TYPE(FALSE, MaxCoord));

	/* Set up open end condition knot vectors. */
	for (i = 0; i < MV -> Dim; i++) {
	    CagdRType Min, Max;

	    MvarMVDomain(MV, &Min, &Max, i);

	    if (i == Dir) {
		UnitMV -> KnotVectors[i][0] =
		    UnitMV -> KnotVectors[i][1] = Min;
		UnitMV -> KnotVectors[i][2] =
		    UnitMV -> KnotVectors[i][3] = Max;
	    }
	    else {
		UnitMV -> KnotVectors[i][0] = Min;
		UnitMV -> KnotVectors[i][1] = Max;
	    }
	}
    }
    IritFree(Orders);

    /* The multivariate's value is one throughout. */
    for (j = 0; j < MVAR_CTL_MESH_LENGTH(UnitMV); j++)
	for (i = 1; i <= MaxCoord; i++)
	    UnitMV -> Points[i][j] = 1.0;

    if (IsBezier)
	RaisedMV = MvarBzrMVMult(MV, UnitMV);
    else
	RaisedMV = MvarBspMVMult(MV, UnitMV);

    MvarMVFree(UnitMV);

    return RaisedMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new multivariate representing the same curve as MV but with its  M
* degree raised by one.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To raise its degree.                                           M
*   Dir:      Direction of degree raising.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A multivariate with same geometry as MV but with one    M
*                    degree higher.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDegreeRaiseN, MvarMVDegreeRaise, MvarMVDegreeRaise2,               M
*   MvarMVPwrDegreeRaise						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDegreeRaise3, degree raising                                       M
*****************************************************************************/
MvarMVStruct *MvarMVDegreeRaise3(MvarMVStruct *MV, MvarMVDirType Dir)
{
    CagdBType IsBezier,
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, j, l, StepSize, *Indices, *RIndices, NewLen,
	Length = MV -> Lengths[Dir],
	Order = MV -> Orders[Dir],
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdRType *KV, *NewKV, *BlossomValues;
    MvarMVStruct
	*RaisedMV = NULL;

    if (Dir < 0 || Dir >= MV -> Dim) {
	MVAR_FATAL_ERROR(MVAR_ERR_DIR_NOT_VALID);
	return NULL;
    }

    if (MVAR_IS_POWER_MV(MV)) {  /* Power degree raising simply adds zeros. */
        return MvarMVPwrDegreeRaise(MV, Dir, 1);
    }

    if ((IsBezier = MVAR_IS_BEZIER_MV(MV)) != FALSE)
        KV = BspKnotUniformOpen(MV -> Lengths[Dir], MV -> Orders[Dir], NULL);
    else
        KV = MV -> KnotVectors[Dir];

    /* Allocate a vector for the new knot vector that will be sufficient    */
    /* and populate it:						            */
    NewKV = BspKnotDegreeRaisedKV(KV, Length, Order, Order + 1, &NewLen);
    NewLen -= Order + 1; /* Get num. of control pnts we should have. */

    i = MV -> Lengths[Dir];
    MV -> Lengths[Dir] = NewLen;
    MV -> Orders[Dir]++;
    if (IsBezier)
        RaisedMV = MvarBzrMVNew(MV -> Dim, MV -> Lengths, MV -> PType);
    else
        RaisedMV = MvarBspMVNew(MV -> Dim, MV -> Lengths, MV -> Orders,
				MV -> PType);
    MV -> Orders[Dir]--;
    MV -> Lengths[Dir] = i;

    if (!IsBezier) {
        /* Update the knot vectors. */
        for (i = 0; i < MV -> Dim; i++) {
	    if (i == Dir) {
	        IritFree(RaisedMV -> KnotVectors[i]);
		RaisedMV -> KnotVectors[i] = NewKV;
	    }
	    else {
	        CAGD_GEN_COPY(RaisedMV -> KnotVectors[i], MV -> KnotVectors[i],
			      sizeof(CagdRType) * (MV -> Lengths[i]
						           + MV -> Orders[i]));
	    }
	}
    }

    /* Update the control mesh. */
    Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    RIndices = (int *) IritMalloc(sizeof(int) * RaisedMV -> Dim);
    BlossomValues = (CagdRType *) IritMalloc(sizeof(CagdRType) * Order);

    StepSize = MVAR_NEXT_DIM(MV, Dir);

    for (i = IsNotRational; i <= MaxCoord; i++) {
	int Index = 0,
	    RIndex = 0;

	IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);
	IRIT_ZAP_MEM(RIndices, sizeof(int) * RaisedMV -> Dim);

	do {
	    CagdRType
		*RPts = &RaisedMV -> Points[i][RIndex],
		*Pts = &MV -> Points[i][Index];

	    for (l = 0; l < NewLen; l++) {
		*RPts = 0.0;
		IRIT_GEN_COPY(BlossomValues, &NewKV[l + 2],
			 sizeof(CagdRType) * Order);

		for (j = 0; j < Order; j++) {
		    *RPts += CagdBlossomEval(Pts, StepSize,
					     Order, IsBezier ? NULL : KV,
					     Length + Order,
					     BlossomValues,
					     Order - 1);

		    /* Only one value is changing in each step! */
		    BlossomValues[j] = NewKV[l + 1 + j];
		}
		*RPts /= Order;
		RPts += MVAR_NEXT_DIM(MV, Dir);
	    }
	    MVAR_INC_SKIP_MESH_INDICES(RaisedMV, RIndices, Dir, RIndex);
	}
	while (MVAR_INC_SKIP_MESH_INDICES(MV, Indices, Dir, Index));
    }

    IritFree(Indices);
    IritFree(RIndices);
    IritFree(BlossomValues);

    if (IsBezier) {
        IritFree(KV);
	IritFree(NewKV);
    }

    return RaisedMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increase the order of the given power basis multivariate in direction    M
* Dir by IncOrder amount.  IncOrder amount is at least one.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Multivariate to increase its order in direction Dir.           M
*   Dir:      Direction to increase the order.				     M
*   IncOrder: By how much to increase the order, at least one.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   New multivariate with higher order.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVDegreeRaise, MvarMVDegreeRaiseN, MvarMVDegreeRaise2                M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVPwrDegreeRaise                                                     M
*****************************************************************************/
MvarMVStruct *MvarMVPwrDegreeRaise(const MvarMVStruct *MV,
				   int Dir,
				   int IncOrder)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, *Orders, Index, RaisedIndex, *Indices, *RaisedIndices,
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    CagdRType * const *Points, **RaisedPoints;
    MvarMVStruct
	*RaisedMV = NULL;

    Orders = (int *) IritMalloc(MV -> Dim * sizeof(int));
    for (i = 0; i < MV -> Dim; i++)
	Orders[i] = i == Dir ? MV -> Orders[i] + IncOrder : MV -> Orders[i];

    RaisedMV = MvarPwrMVNew(MV -> Dim, Orders, MV -> PType);

    /* Copy the ceofficients as is and place zeros at the end. */
    Points = MV -> Points;
    RaisedPoints = RaisedMV -> Points;

    Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    RaisedIndices = (int *) IritMalloc(sizeof(int) * RaisedMV -> Dim);

    IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);
    IRIT_ZAP_MEM(RaisedIndices, sizeof(int) * RaisedMV -> Dim);

    Index = RaisedIndex = 0;

    do {
	for (i = IsNotRational; i <= MaxCoord; i++)
	    RaisedPoints[i][RaisedIndex] = Points[i][Index];

	MVAR_INCREMENT_MESH_INDICES(RaisedMV, RaisedIndices, RaisedIndex);

	/* Place zeros at the end as needed. */
	while (RaisedIndices[Dir] >= MV -> Lengths[Dir]) {
	    for (i = IsNotRational; i <= MaxCoord; i++)
	        RaisedPoints[i][RaisedIndex] = 0.0;

	    MVAR_INCREMENT_MESH_INDICES(RaisedMV, RaisedIndices, RaisedIndex);
	}
    }
    while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));

    IritFree(Indices);
    IritFree(RaisedIndices);
    IritFree(Orders);

    return RaisedMV;
}
