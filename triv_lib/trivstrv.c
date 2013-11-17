/******************************************************************************
* TrivSTrv.c - Construct a trivariate using a set of surfaces.                *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 91.					      *
******************************************************************************/

#include "triv_loc.h"

static TrivTVStruct *TrivPrepTrivar(const CagdSrfStruct *SrfList,
				    int OtherOrder,
				    CagdEndConditionType OtherEC,
				    CagdSrfStruct ***RetSrfVec,
				    int *NumSrfs);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A preprocessing function to prepare the surface and auxiliary data for   *
* the surface thru curves function.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   SrfList:      List of surfaces to construct a trivariate with.           *
*   OtherOrder:   Other, third, order of trivariate. 			     *
*   OtherEC:	  End condition in the other, third, trivar direction.       *
*   RetSrfVec:    Preprocessed list of surfaces as a vector.                 *
*   NumSrfs:      Number of surface in the returned vector.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrivTVStruct *:  Trivariate to hold the fit.                             *
*****************************************************************************/
static TrivTVStruct *TrivPrepTrivar(const CagdSrfStruct *SrfList,
				    int OtherOrder,
				    CagdEndConditionType OtherEC,
				    CagdSrfStruct ***RetSrfVec,
				    int *NumSrfs)
{
    int i, UOrder, VOrder, WOrder;
    const CagdSrfStruct *Srf;
    CagdSrfStruct **SrfVec;
    TrivTVStruct *TV;

    /* Find out how many curves we have and put them in a linear vector.    */
    /* Note the vector have a COPY of the curves so we can modify them.     */
    for (*NumSrfs = 0, Srf = SrfList;
	 Srf != NULL;
	 (*NumSrfs)++, Srf = Srf -> Pnext);
    SrfVec = *RetSrfVec = (CagdSrfStruct **)
                               IritMalloc(sizeof(CagdSrfStruct *) * *NumSrfs);
    for (i = 0, Srf = SrfList;
	 i < *NumSrfs;
	 i++, Srf = Srf -> Pnext)
	SrfVec[i] = CagdSrfCopy(Srf);

    /* Traverse vector in a O(n) fashion and make all curves compatible.   */
    for (i = 0; i < *NumSrfs - 1; i++)
        CagdMakeSrfsCompatible(&SrfVec[i], &SrfVec[i + 1],
			       TRUE, TRUE, TRUE, TRUE);
    for (i = *NumSrfs - 2; i >= 0; i--)
        CagdMakeSrfsCompatible(&SrfVec[i], &SrfVec[i + 1],
			       TRUE, TRUE, TRUE, TRUE);

    /* Construct the surface. All required information is now available.    */
    UOrder = SrfVec[0] -> UOrder;
    VOrder = SrfVec[0] -> VOrder;
    WOrder = IRIT_MIN(*NumSrfs, OtherOrder);
    if (*NumSrfs == WOrder &&
	SrfVec[0] -> GType == CAGD_SBEZIER_TYPE &&
	OtherEC == CAGD_END_COND_OPEN) {
    	/* Allocate a Bezier surface. */
	TV = TrivBzrTVNew(SrfVec[0] -> ULength, SrfVec[0] -> VLength,
			  *NumSrfs, SrfVec[0] -> PType);
    }
    else {
	/* Allocate a B-spline surface. */
	TV = TrivBspTVNew(SrfVec[0] -> ULength, SrfVec[0] -> VLength,
			  *NumSrfs, UOrder, VOrder, WOrder,
			  SrfVec[0] -> PType);
	if (SrfVec[0] -> GType == CAGD_SBEZIER_TYPE) {
	    BspKnotUniformOpen(UOrder, UOrder, TV -> UKnotVector);
	    BspKnotUniformOpen(VOrder, VOrder, TV -> VKnotVector);
	}
	else {
	    BspKnotCopy(TV -> UKnotVector, SrfVec[0] -> UKnotVector,
			CAGD_SRF_UPT_LST_LEN(SrfVec[0]) + UOrder);
	    BspKnotCopy(TV -> VKnotVector, SrfVec[0] -> VKnotVector,
			CAGD_SRF_VPT_LST_LEN(SrfVec[0]) + VOrder);
	}

	switch (OtherEC) {
	    case CAGD_END_COND_OPEN:
	        BspKnotUniformOpen(*NumSrfs, WOrder, TV -> WKnotVector);
		break;
	    case CAGD_END_COND_FLOAT:
	        BspKnotUniformFloat(*NumSrfs, WOrder, TV -> WKnotVector);
		break;
	    case CAGD_END_COND_PERIODIC:
	        IritFree(TV -> WKnotVector);	      /* Not long enough. */
	        TV -> WKnotVector = BspKnotUniformPeriodic(*NumSrfs, WOrder,
							   NULL);
		TV -> WPeriodic = TRUE;
		break;
	    default:
	        assert(0);
	}
    }

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a trivariate using a set of surfaces. Surfaces are made to be   M
* compatible and then each is substituted into the new trivariate's mesh as  M
* a row.								     M
*   If OtherOrder is less than the number of surfaces, number of surfaces is M
* used.									     M
*   A knot vector is formed with uniform open end for the other direction,   M
* so created TV interpolates the first and last surfaces only, if OtherOrder M
* is greater than 2.					                     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:      List of surfaces to construct a trivariate with.           M
*   OtherOrder:   Other, third, order of trivariate. 			     M
*   OtherEC:	  End condition in the other, third, trivar direction.       M
*   OtherParamVals: If not NULL, updated with other direction set parameters M
*                 of the surfaces in the new trivar.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  Constructed trivariate from surfaces.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVInterpolateSrfs				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVFromSrfs, trivar constructors                                      M
*****************************************************************************/
TrivTVStruct *TrivTVFromSrfs(const CagdSrfStruct *SrfList,
			     int OtherOrder,
			     CagdEndConditionType OtherEC,
			     IrtRType *OtherParamVals)
{
    CagdBType IsNotRational;
    int i, j, NumSrfs, MaxCoord, Length;
    CagdRType **TVPoints;
    CagdSrfStruct **SrfVec;
    TrivTVStruct
        *TV = TrivPrepTrivar(SrfList, OtherOrder, OtherEC, &SrfVec, &NumSrfs);

    /* Substitute each surface as a plane into trivar's mesh and delete. */
    TVPoints = TV -> Points;
    i = 0;
    MaxCoord = CAGD_NUM_OF_PT_COORD(SrfVec[0] -> PType),
    IsNotRational = !CAGD_IS_RATIONAL_SRF(SrfVec[0]);
    Length = SrfVec[0] -> ULength * SrfVec[0] -> VLength;

    for (j = 0; j < NumSrfs; j++) {
	int k;
	CagdRType
	    **SrfPoints = SrfVec[j] -> Points;

    	for (k = IsNotRational; k <= MaxCoord; k++)
	    CAGD_GEN_COPY(&TVPoints[k][i], SrfPoints[k],
			  sizeof(CagdRType) * Length);

	CagdSrfFree(SrfVec[j]);
	i += Length;
    }

    if (OtherParamVals != NULL && TV -> GType == CAGD_SBSPLINE_TYPE) {
        IrtRType
	    *Nodes = BspKnotNodes(TV -> WKnotVector,
				  TV ->  WLength + TV -> WOrder, TV -> WOrder);

	IRIT_GEN_COPY(OtherParamVals, Nodes,
		      sizeof(IrtRType) * TV -> WLength);
	IritFree(Nodes);
    }

    IritFree(SrfVec);

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes parameters to interpolate the given surfaces at, as a trivar.   M
* Estimate a middle point from each surface and set parameters based on      M
* chord length from each middle point to the next.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:      List of surfaces to consturct a trivariate volume with.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  Vectors of parameters normalized to [0, 1] of parameters,  M
*		  of size of number of surfaces, allocated dynamically.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVFromSrfs, CagdSrfInterpolateCrvs, CagdSrfInterpolateCrvs           M
*   CagdSrfInterpolateCrvsChordLenParams				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVInterpolateSrfsChordLenParams, surface constructors                M
*****************************************************************************/
CagdRType *TrivTVInterpolateSrfsChordLenParams(const CagdSrfStruct *SrfList)
{
    int i, j, NumSrfs;
    CagdRType *Params, MinDiff;
    CagdPType Pt, LastPt;
    const CagdSrfStruct *Srf;
    CagdBBoxStruct BBox;

    assert(SrfList != NULL);

    for (NumSrfs = 0, Srf = SrfList;
	 Srf != NULL;
	 NumSrfs++, Srf = Srf -> Pnext);

    Params = (CagdRType *) IritMalloc(NumSrfs * sizeof(CagdRType));

    Params[0] = 0;
    CagdSrfBBox(SrfList, &BBox);
    IRIT_PT_BLEND(LastPt, BBox.Min, BBox.Max, 0.5);

    for (i = 1, Srf = SrfList -> Pnext; Srf != NULL; i++, Srf = Srf -> Pnext) {
        CagdSrfBBox(Srf, &BBox);
	IRIT_PT_BLEND(Pt, BBox.Min, BBox.Max, 0.5);
	Params[i] = Params[i - 1] + IRIT_PT_PT_DIST(LastPt, Pt);
	IRIT_PT_COPY(LastPt, Pt);
    }
    assert(i == NumSrfs);

    /* Verify a monotone sequence. */
    MinDiff = Params[NumSrfs - 1] / 100.0;
    for (i = 1; i < NumSrfs; i++) {
        if (Params[i] - Params[i - 1] < MinDiff) {
	    for (j = i; j < NumSrfs; j++)
	        Params[j] += Params[NumSrfs - 1] / 100.0;
	}
    }

    /* Normalize to [0, 1]. */
    for (i = 1; i < NumSrfs; i++)
        Params[i] /= Params[NumSrfs - 1];

    return Params;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a trivariate using a set of surfaces. Surfaces are made to be   M
* compatible and then the trivariate is fitted to interpolate them.          M
*   If OtherOrder is less than the number of surfaces, number of surfaces is M
* used.									     M
*   A knot vector is formed with OtherEC end conditions for the other        M
* direction.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   SrfList:      List of surfaces to interpolate a trivariate through.      M
*   OtherOrder:   Other, third, depth order of trivariate. 		     M
*   OtherEC:	  End condition in the other, third, trivar direction.       M
*   OtherParam:   Currently only Chord length and uniform are supported.     M
*   OtherParamVals: If not NULL, updated with other direction set parameters M
*                 of the surfaces in the new trivar.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  Constructed trivariate from surfaces.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVFromSrfs					                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVInterpolateSrfs, trivar constructors, interpolation                M
*****************************************************************************/
TrivTVStruct *TrivTVInterpolateSrfs(const CagdSrfStruct *SrfList,
				    int OtherOrder,
				    CagdEndConditionType OtherEC,
				    CagdParametrizationType OtherParam,
				    IrtRType *OtherParamVals)
{
    CagdBType IsNotRational;
    int i, j, p, NumSrfs, FreeKV, MaxCoord, WOrder, Length;
    CagdRType **TVPoints, *KV, *Params;
    CagdSrfStruct **SrfVec;
    TrivTVStruct
        *TV = TrivPrepTrivar(SrfList, OtherOrder, OtherEC, &SrfVec, &NumSrfs);

    WOrder = TV -> WOrder;

    /* Interpolate all ij ctlpts in all surfaces as a depth column in TV. */
    TVPoints = TV -> Points;
    i = 0;
    MaxCoord = CAGD_NUM_OF_PT_COORD(SrfVec[0] -> PType),
    IsNotRational = !CAGD_IS_RATIONAL_SRF(SrfVec[0]);
    Length = SrfVec[0] -> ULength * SrfVec[0] -> VLength;

    /* Get the knot vector to be used in the interpolation. */
    if (TRIV_IS_BSPLINE_TV(TV)) {
	KV = TV -> WKnotVector;
	FreeKV = FALSE;
    }
    else {
	KV = BspKnotUniformOpen(NumSrfs, WOrder, NULL);
	FreeKV = TRUE;
    }

    /* Compute the other parameter values and possible redo the knots. */
    if (OtherParam == CAGD_CHORD_LEN_PARAM &&
	OtherEC != CAGD_END_COND_PERIODIC) {
        Params = TrivTVInterpolateSrfsChordLenParams(SrfList);
	if (TRIV_IS_BSPLINE_TV(TV)) {
	    IritFree(TV -> WKnotVector);
	    KV = TV -> WKnotVector = BspPtSamplesToKV(Params, NumSrfs,
						      WOrder, NumSrfs);
	}
    }
    else
        Params = BspKnotNodes(KV, TV -> WLength + WOrder, WOrder);
    if (OtherParamVals != NULL)
        IRIT_GEN_COPY(OtherParamVals, Params,
		      sizeof(IrtRType) * TV -> WLength);

    for (p = 0; p < Length; p++) {
        int k;
	CagdCtlPtStruct *CtlPt,
	    *CtlPtList = NULL;
	CagdCrvStruct *InterpCrv;

        for (j = 0; j < NumSrfs; j++) {
	    CagdRType
		**CrvPoints = SrfVec[j] -> Points;

	    CtlPt = CagdCtlPtNew(SrfVec[0] -> PType);
	    for (k = IsNotRational; k <= MaxCoord; k++)
	        CtlPt -> Coords[k] = CrvPoints[k][p];

	    IRIT_LIST_PUSH(CtlPt, CtlPtList);
	}
	CtlPtList = CagdListReverse(CtlPtList);

        InterpCrv = BspCrvInterpolate(CtlPtList, Params, KV,
				      TV -> WLength,
				      TV -> WOrder,
				      TV -> WPeriodic);
	CagdCtlPtFreeList(CtlPtList);

	if (InterpCrv == NULL) {
	    TrivTVFree(TV);
	    TV = NULL;
	    break;
	}

	/* Copy interpolated data into the surfaces. */
        for (j = 0, i = p; j < NumSrfs; j++, i += Length) {
	    for (k = IsNotRational; k <= MaxCoord; k++)
	        TVPoints[k][i] = InterpCrv -> Points[k][j];
	}

	CagdCrvFree(InterpCrv);
    }

    IritFree(Params);
    if (FreeKV)
        IritFree(KV);     

    for (j = 0; j < NumSrfs; j++)
	CagdSrfFree(SrfVec[j]);
    IritFree(SrfVec);

    return TV;
}
