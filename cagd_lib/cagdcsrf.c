/******************************************************************************
* CagdCSrf.c - Construct a surface using a set of curves.                     *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

static CagdSrfStruct *CagdPrepSurface(const CagdCrvStruct *CrvList,
				      int OtherOrder,
				      CagdEndConditionType OtherEC,
				      CagdCrvStruct ***RetCrvVec,
				      int *NumCrvs);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A preprocessing function to prepare the surface and auxiliary data for   *
* the surface thru curves function.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvList:      List of curves to construct a surface with.                *
*   OtherOrder:   Other order of surface.				     *
*   OtherEC:	  End condition in the other (non CrvList) srf direction.    *
*   RetCrvVec:    Preprocessed list of curves as a vector.                   *
*   NumCrvs:      Number of curves in the returned vector.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfStruct *:  Surface to hold the fit.                               *
*****************************************************************************/
static CagdSrfStruct *CagdPrepSurface(const CagdCrvStruct *CrvList,
				      int OtherOrder,
				      CagdEndConditionType OtherEC,
				      CagdCrvStruct ***RetCrvVec,
				      int *NumCrvs)
{
    CagdBType
	OtherPeriodic = OtherEC == CAGD_END_COND_PERIODIC;
    int i, UOrder, VOrder;
    CagdCrvStruct **CrvVec;
    const CagdCrvStruct *Crv;
    CagdSrfStruct *Srf;

    if (CrvList == NULL || CrvList -> Pnext == NULL)
        return NULL;

    OtherOrder = IRIT_ABS(OtherOrder);

    /* Find out how many curves we have and put them in a linear vector.    */
    /* Note the vector have a COPY of the curves so we can modify them.     */
    for (*NumCrvs = 0, Crv = CrvList;
	 Crv != NULL;
	 (*NumCrvs)++, Crv = Crv -> Pnext);
    CrvVec = *RetCrvVec = (CagdCrvStruct **)
                               IritMalloc(sizeof(CagdCrvStruct *) * *NumCrvs);

    for (i = 0, Crv = CrvList;
	 i < *NumCrvs;
	 i++, Crv = Crv -> Pnext)
	CrvVec[i] = CagdCrvCopy(Crv);

    /* Traverse vector in a O(n) fashion and make all curves compatible.   */
    for (i = 0; i < *NumCrvs - 1; i++)
        CagdMakeCrvsCompatible(&CrvVec[i], &CrvVec[i + 1], TRUE, TRUE);
    for (i = *NumCrvs - 2; i >= 0; i--)
        CagdMakeCrvsCompatible(&CrvVec[i], &CrvVec[i + 1], TRUE, TRUE);

    /* Construct the surface. All required information is now available.    */
    UOrder = CrvVec[0] -> Order;
    VOrder = IRIT_MIN(*NumCrvs, OtherOrder);
    if (*NumCrvs == VOrder &&
	CAGD_IS_BEZIER_CRV(CrvVec[0]) &&
	OtherEC == CAGD_END_COND_OPEN) {
    	/* Allocate a bezier surface. */
	Srf = BzrSrfNew(CrvVec[0] -> Length, *NumCrvs, CrvVec[0] -> PType);
    }
    else {
	/* Allocate a bspline surface. */
	Srf = BspPeriodicSrfNew(CrvVec[0] -> Length, *NumCrvs, UOrder, VOrder,
				CrvVec[0] -> Periodic, OtherPeriodic,
				CrvVec[0] -> PType);
	if (CAGD_IS_BEZIER_CRV(CrvVec[0]))
	    BspKnotUniformOpen(Srf -> ULength, Srf -> UOrder,
			       Srf -> UKnotVector);
	else {
	    BspKnotCopy(Srf -> UKnotVector, CrvVec[0] -> KnotVector,
			CAGD_CRV_PT_LST_LEN(CrvVec[0]) + UOrder);
	}

	switch (OtherEC) {
	    case CAGD_END_COND_OPEN:
	        BspKnotUniformOpen(*NumCrvs, VOrder, Srf -> VKnotVector);
		break;
	    case CAGD_END_COND_FLOAT:
	        BspKnotUniformFloat(*NumCrvs, VOrder, Srf -> VKnotVector);
		break;
	    case CAGD_END_COND_PERIODIC:
	        BspKnotUniformPeriodic(*NumCrvs, VOrder, Srf -> VKnotVector);
		break;
            default:
	        assert(0);
	}
    }

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface using a set of curves. Curves are made to be	     M
* compatible and then each is substituted into the new surface's mesh as a   M
* row.									     M
*   If the OtherOrder is less than the number of curves, number of curves is M
* used.	 If OtherOrder is negative, the absolute value is employed and a     M
* periodic surface is constructed in the other direction.		     M
*   A knot vector is formed with OtherEC end conditions for the other        M
* direction								     M
*   Note, however, that only the first and the last curves are interpolated  M
* if open end conditions are selected and OtherOrder is greater than 2.      M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:      List of curves to consturct a surface with.                M
*   OtherOrder:   Other order of surface.				     M
*   OtherEC:	  End condition in the other (non CrvList) srf direction.    M
*   OtherParamVals: If not NULL, updated with other direction set parameters M
*                 of the curves in the new surfaces.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Constructed surface from curves.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfInterpolateCrvs                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfFromCrvs, surface constructors                                    M
*****************************************************************************/
CagdSrfStruct *CagdSrfFromCrvs(const CagdCrvStruct *CrvList,
			       int OtherOrder,
			       CagdEndConditionType OtherEC,
			       IrtRType *OtherParamVals)
{
    CagdBType IsNotRational;
    int i, j, NumCrvs, MaxCoord, Length;
    CagdRType **SrfPoints;
    CagdCrvStruct **CrvVec;
    CagdSrfStruct
	*Srf = CagdPrepSurface(CrvList, OtherOrder, OtherEC,
			       &CrvVec, &NumCrvs);

    /* Substitute each curve as a row into the surface mesh and delete it. */
    SrfPoints = Srf -> Points;
    i = 0;
    MaxCoord = CAGD_NUM_OF_PT_COORD(CrvVec[0] -> PType),
    IsNotRational = !CAGD_IS_RATIONAL_CRV(CrvVec[0]);
    Length = CrvVec[0] -> Length;

    for (j = 0; j < NumCrvs; j++) {
	int k;
	CagdRType
	    **CrvPoints = CrvVec[j] -> Points;

    	for (k = IsNotRational; k <= MaxCoord; k++)
	    CAGD_GEN_COPY(&SrfPoints[k][i], CrvPoints[k],
			  sizeof(CagdRType) * Length);

	CagdCrvFree(CrvVec[j]);
	i += Length;
    }

    if (OtherParamVals != NULL) {
        CagdRType *Nodes, *KV;

        if (CAGD_IS_BSPLINE_SRF(Srf))
	    KV = Srf -> VKnotVector;
	else
	    KV = BspKnotUniformOpen(Srf ->  VLength, Srf ->  VOrder, NULL);

	Nodes = BspKnotNodes(KV,  Srf ->  VLength + Srf ->  VOrder,
			     Srf ->  VOrder);

	IRIT_GEN_COPY(OtherParamVals, Nodes,
		      sizeof(IrtRType) * Srf -> VLength);
	IritFree(Nodes);
	if (KV != Srf -> VKnotVector)
	  IritFree(KV);
    }

    IritFree(CrvVec);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes parameters to interpolate the given curves at, as a surface.    M
* Estimate a middle point from each curve and set parameters based on chord  M
* length from each middle point to the next.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:      List of curves to consturct a surface with.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  Vectors of parameters normalized to [0, 1] of parameters,  M
*		  of size of number of curves, allocated dynamically.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfFromCrvs, CagdSrfInterpolateCrvs, CagdSrfInterpolateCrvs          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfInterpolateCrvsChordLenParams, surface constructors               M
*****************************************************************************/
CagdRType *CagdSrfInterpolateCrvsChordLenParams(const CagdCrvStruct *CrvList)
{
    int i, j, NumCrvs;
    CagdRType *Params, MinDiff;
    CagdPType Pt, LastPt;
    const CagdCrvStruct *Crv;
    CagdBBoxStruct BBox;

    assert(CrvList != NULL);

    for (NumCrvs = 0, Crv = CrvList;
	 Crv != NULL;
	 NumCrvs++, Crv = Crv -> Pnext);

    Params = (CagdRType *) IritMalloc(NumCrvs * sizeof(CagdRType));

    Params[0] = 0;
    CagdCrvBBox(CrvList, &BBox);
    IRIT_PT_BLEND(LastPt, BBox.Min, BBox.Max, 0.5);

    for (i = 1, Crv = CrvList -> Pnext; Crv != NULL; i++, Crv = Crv -> Pnext) {
        CagdCrvBBox(Crv, &BBox);
	IRIT_PT_BLEND(Pt, BBox.Min, BBox.Max, 0.5);
	Params[i] = Params[i - 1] + IRIT_PT_PT_DIST(LastPt, Pt);
	IRIT_PT_COPY(LastPt, Pt);
    }
    assert(i == NumCrvs);

    /* Verify a monotone sequence. */
    MinDiff = Params[NumCrvs - 1] / 100.0;
    for (i = 1; i < NumCrvs; i++) {
        if (Params[i] - Params[i - 1] < MinDiff) {
	    for (j = i; j < NumCrvs; j++)
	        Params[j] += Params[NumCrvs - 1] / 100.0;
	}
    }

    /* Normalize to [0, 1]. */
    for (i = 1; i < NumCrvs; i++)
        Params[i] /= Params[NumCrvs - 1];

    return Params;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a surface using a set of curves. Curves are made to be	     M
* compatible and then interpolated by the created surfaces.		     M
*   If the OtherOrder is less than the number of curves, number of curves is M
* used.	 If OtherOrder is negative, the absolute value is employed and a     M
* periodic surface is constructed in the other direction.		     M
*   A knot vector is formed with OtherEC end conditions for the other        M
* direction.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:      List of curves to consturct a surface with.                M
*   OtherOrder:   Other order of surface.				     M
*   OtherEC:	  End condition in the other (non CrvList) srf direction.    M
*   OtherParam:   Currently only Chord length and uniform are supported.     M
*   OtherParamVals: If not NULL, updated with other direction set parameters M
*                 of the curves in the new surfaces.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  Constructed surface from curves.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfFromCrvs, CagdSrfInterpolateCrvsChordLenParams                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfInterpolateCrvs, surface constructors, interpolation              M
*****************************************************************************/
CagdSrfStruct *CagdSrfInterpolateCrvs(const CagdCrvStruct *CrvList,
				      int OtherOrder,
				      CagdEndConditionType OtherEC,
				      CagdParametrizationType OtherParam,
				      IrtRType *OtherParamVals)
{
    CagdBType IsNotRational, FreeKV;
    int i, j, p, NumCrvs, VOrder, MaxCoord, Length;
    CagdRType **SrfPoints, *KV, *Params;
    CagdCrvStruct **CrvVec;
    CagdSrfStruct
	*Srf = CagdPrepSurface(CrvList, OtherOrder, OtherEC,
			       &CrvVec, &NumCrvs);
    VOrder = Srf -> VOrder;

    /* Interpolate all i'th ctlpts in all curves as a row into surface mesh. */
    SrfPoints = Srf -> Points;
    i = 0;
    MaxCoord = CAGD_NUM_OF_PT_COORD(CrvVec[0] -> PType),
    IsNotRational = !CAGD_IS_RATIONAL_CRV(CrvVec[0]);
    Length = CrvVec[0] -> Length;

    /* Get the knot vector to be used in the interpolation. */
    if (CAGD_IS_BSPLINE_SRF(Srf)) {
	KV = Srf -> VKnotVector;
	FreeKV = FALSE;
    }
    else {
	KV = BspKnotUniformOpen(NumCrvs, VOrder, NULL);
	FreeKV = TRUE;
    }

    /* Compute the other parameter values and possible redo the knots. */
    if (OtherParam == CAGD_CHORD_LEN_PARAM &&
	OtherEC != CAGD_END_COND_PERIODIC) {
        Params = CagdSrfInterpolateCrvsChordLenParams(CrvList);
	if (CAGD_IS_BSPLINE_SRF(Srf)) {
	    IritFree(Srf -> VKnotVector);
	    KV = Srf -> VKnotVector = BspPtSamplesToKV(Params, NumCrvs,
						       VOrder, NumCrvs);
	}
    }
    else
        Params = BspKnotNodes(KV, Srf ->  VLength + VOrder, VOrder);

    if (OtherParamVals != NULL)
        IRIT_GEN_COPY(OtherParamVals, Params,
		      sizeof(IrtRType) * Srf -> VLength);

    for (p = 0; p < Length; p++) {
        int k;
	CagdCtlPtStruct *CtlPt,
	    *CtlPtList = NULL;
	CagdCrvStruct *InterpCrv;

        for (j = 0; j < NumCrvs; j++) {
	    CagdRType
	        **CrvPoints = CrvVec[j] -> Points;

	    CtlPt = CagdCtlPtNew(CrvVec[0] -> PType);
	    for (k = IsNotRational; k <= MaxCoord; k++)
	        CtlPt -> Coords[k] = CrvPoints[k][p];

	    IRIT_LIST_PUSH(CtlPt, CtlPtList);
	}
	CtlPtList = CagdListReverse(CtlPtList);

        InterpCrv = BspCrvInterpolate(CtlPtList, Params, KV,
				      Srf -> VLength,
				      Srf -> VOrder,
				      Srf -> VPeriodic);
	CagdCtlPtFreeList(CtlPtList);

	if (InterpCrv == NULL) {
	    CagdSrfFree(Srf);
	    Srf = NULL;
	    break;
	}

	/* Copy interpolated data into the surfaces. */
        for (j = 0, i = p; j < NumCrvs; j++, i += Length) {
	    for (k = IsNotRational; k <= MaxCoord; k++)
	        SrfPoints[k][i] = InterpCrv -> Points[k][j];
	}

	CagdCrvFree(InterpCrv);
    }

    IritFree(Params);
    if (FreeKV)
        IritFree(KV);     

    for (j = 0; j < NumCrvs; j++)
	CagdCrvFree(CrvVec[j]);
    IritFree(CrvVec);

    return Srf;
}
