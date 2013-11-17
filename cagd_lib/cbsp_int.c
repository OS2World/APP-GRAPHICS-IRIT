/******************************************************************************
* CBsp_Int.c - Bspline curve interpolation/approximation schemes.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb. 94.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "geom_lib.h"
#include "cagd_loc.h"
#include "extra_fn.h"

#define BSP_CRV_FIT_SAMPLE_RATIO	30

IRIT_STATIC_DATA int
    GlblLineFitSortAxis = 0;

static CagdRType UpdateAreaCrvConstraint(CagdCrvStruct *NewCrv,
					 CagdRType **CrntCrvPts,
					 CagdRType **CrntSpcInnerProds,
					 int Axis,
					 int Length,
					 int Order,
					 CagdBType Periodic,
					 const CagdRType *KV,
					 CagdRType *MatVector);
static void UpdateOneAreaCrvConstraint(CagdRType *YPts,
				       CagdRType **InnerProds,
				       int Length,
				       int Order,
				       CagdBType Periodic,
				       const CagdRType *KV,
				       CagdRType *MatVector);
static void CagdPtsNielsonMetric(const CagdCtlPtStruct *CtlPts,
				 CagdRType G[2][2]);
static CagdRType CagdPtsNielsonDist(CagdRType G[2][2],
				    CagdPType Pt1,
				    CagdPType Pt2);
static CagdRType CagdPtsNielsonAngle(CagdRType G[2][2],
				     CagdPType Pt1,
				     CagdPType Pt2,
				     CagdPType Pt3);
static CagdRType CagdPtsNielsonFoleyParam(CagdRType G[2][2],
					  const CagdCtlPtStruct *CtlPt0,
					  const CagdCtlPtStruct *CtlPt1,
					  const CagdCtlPtStruct *CtlPt2,
					  const CagdCtlPtStruct *CtlPt3);


#if defined(ultrix) && defined(mips)
static int LineFitSortCmpr(VoidPtr VPt1, VoidPtr VPt2);
#else
static int LineFitSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2);
#endif /* ultrix && mips (no const support) */

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points, PtList, computes a Bspline curve of order Order     M
* that interpolates or least square approximates the set of points.	     M
*   The size of the control polygon of the resulting Bspline curve defaults  M
* to the number of points in PtList (if CrvSize = 0).			     M
*   However, this number is can smaller to yield a least square              M
* approximation.							     M
*   The created curve can be parametrized as specified by ParamType.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       List of points to interpolate/least square approximate.    M
*   Order:        Of interpolating/approximating curve.                      M
*   CrvSize:      Number of degrees of freedom (control points) of the       M
*                 interpolating/approximating curve.                         M
*   ParamType:    Type of parametrization.                                   M
*   Periodic:     Constructed curve should be Periodic.	 Periodic	     M
*		  necessitates uniform knot sequence in ParamType.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Constructed interpolating/approximating curve.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpolate, BspCrvInterpPts2, MvarBspCrvInterpVecs                M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvInterpPts, interpolation, least square approximation               M
*****************************************************************************/
CagdCrvStruct *BspCrvInterpPts(const CagdPtStruct *PtList,
			       int Order,
			       int CrvSize,
			       CagdParametrizationType ParamType,
			       CagdBType Periodic)
{
    int i,
	NumPts = CagdListLength(PtList);
    CagdRType *KV, *PtKnots;
    CagdCtlPtStruct
	*CtlPt = NULL,
	*CtlPtList = NULL;
    const CagdPtStruct *Pt;
    CagdCrvStruct *Crv;

    if (NumPts < 2 || NumPts < Order || CrvSize < Order)
	return NULL;

    /* Convert to control points in a linear list */
    for (Pt = PtList; Pt != NULL; Pt = Pt -> Pnext) {
	if (CtlPtList == NULL)
	    CtlPtList = CtlPt = CagdCtlPtNew(CAGD_PT_E3_TYPE);
	else {
	    CtlPt -> Pnext = CagdCtlPtNew(CAGD_PT_E3_TYPE);
	    CtlPt = CtlPt -> Pnext;
	}
	for (i = 0; i < 3; i++)
	    CtlPt -> Coords[i + 1] = Pt -> Pt[i];
    }

    BspCrvInterpBuildKVs(CtlPtList, Order, CrvSize, ParamType, Periodic,
			 &PtKnots, &KV);

    Crv = BspCrvInterpolate(CtlPtList, PtKnots, KV, CrvSize, Order, Periodic);
    CagdCtlPtFreeList(CtlPtList);

    IritFree(PtKnots);
    IritFree(KV);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points, PtList, computes a Bspline curve of order Order     M
* that interpolates or least square approximates the set of points.	     M
*   The size of the control polygon of the resulting Bspline curve defaults  M
* to the number of points in PtList (if CrvSize = 0).			     M
*   However, this number is can smaller to yield a least square              M
* approximation.							     M
*   The created curve can be parametrized as specified by ParamType.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       List of points to interpolate/least square approximate.    M
*   Order:        Of interpolating/approximating curve.                      M
*   CrvSize:      Number of degrees of freedom (control points) of the       M
*                 interpolating/approximating curve.                         M
*   ParamType:    Type of parametrization.                                   M
*   Periodic:     Constructed curve should be Periodic.	 Periodic	     M
*		  necessitates uniform knot sequence in ParamType.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Constructed interpolating/approximating curve.        M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpolate, BspCrvInterpPts                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvInterpPts2, interpolation, least square approximation              M
*****************************************************************************/
CagdCrvStruct *BspCrvInterpPts2(const CagdCtlPtStruct *PtList,
				int Order,
				int CrvSize,
				CagdParametrizationType ParamType,
				CagdBType Periodic)
{
    int NumPts = CagdListLength(PtList);
    CagdRType *KV, *PtKnots;
    CagdCrvStruct *Crv;

    if (CrvSize == 0)
	CrvSize = IRIT_MAX(NumPts, Order);

    BspCrvInterpBuildKVs(PtList, Order, CrvSize, ParamType, Periodic,
			 &PtKnots, &KV);

    Crv = BspCrvInterpolate(PtList, PtKnots, KV, CrvSize, Order, Periodic);

    IritFree(PtKnots);
    IritFree(KV);

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Build knot sequence and sampling parameter for the given data to	     M
* interpolate, curve type and the prameterization type desired.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:     List of point to interpolate.			             M
*   Order:      Order ot interpolating curve.			             M
*   CrvSize:    Number of control points in interpolating curve.             M
*   ParamType:  Parametrization type: Uniform, chord length, etc.            M
*   Periodic:   TRUE for a periodic interpolating curve		             M
*   RetPtKnots: Parameter values assigned to the interpolation.              M
*   RetKV:      Knot sequence built for the interpolation, unless            M
*               CAGD_KV_NODAL_PARAM in which case the KV is assumed given.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvInterpBuildKVs                                                     M
*****************************************************************************/
void BspCrvInterpBuildKVs(const CagdCtlPtStruct *PtList,
			  int Order,
			  int CrvSize,
			  CagdParametrizationType ParamType,
			  CagdBType Periodic,
			  CagdRType **RetPtKnots,
			  CagdRType **RetKV)
{
    int i,
	NumPts = CagdListLength(PtList),
	Order1 = Order - 1,
	CrvPerSize = CrvSize + (Periodic ? Order1 : 0);
    const CagdCtlPtStruct *Pt, *LastPt;
    CagdRType *KV, *PtKnots, *R, *R2, t, G[2][2];

    if (Periodic)
        ParamType = CAGD_UNIFORM_PARAM;

    if (ParamType != CAGD_KV_NODAL_PARAM)
        R = PtKnots = (CagdRType *) IritMalloc(sizeof(CagdRType) * NumPts);

    /* Compute parameter values at which the curve will interpolate PtList. */
    switch (ParamType) {
        case CAGD_KV_NODAL_PARAM:
	    /* Assume RetKV has valid knot sequence & we keep it unchanged. */
	    *RetPtKnots = BspKnotNodes(*RetKV, CrvSize + Order, Order);
	    return;
	case CAGD_CHORD_LEN_PARAM:
	    *R++ = 0.0;
	    for (Pt = PtList; Pt -> Pnext != NULL; Pt = Pt -> Pnext, R++) {
		const CagdRType
		    *Coords = Pt -> Coords,
		    *NextCoords = Pt -> Pnext -> Coords;

		/* Coercion below due to const conversions. */
		*R = *(R - 1) + IRIT_UEPS +
		    CagdDistTwoCtlPt((CagdRType * const *) &Coords, -1,
				     (CagdRType * const *) &NextCoords,
				     -1, Pt -> PtType);
	    }
	    for (t = R[-1]; R != PtKnots; *--R /= t);
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    *R++ = 0.0;
	    for (Pt = PtList; Pt -> Pnext != NULL; Pt = Pt -> Pnext, R++) {
		const CagdRType
		    *Coords = Pt -> Coords,
		    *NextCoords = Pt -> Pnext -> Coords;

		/* Coercion below due to const conversions. */
		*R = *(R - 1) + IRIT_UEPS +
		     sqrt(CagdDistTwoCtlPt((CagdRType * const *) &Coords, -1,
					   (CagdRType * const *) &NextCoords,
					   -1, Pt -> PtType));
	    }
	    for (t = R[-1]; R != PtKnots; *--R /= t);
	    break;
	case CAGD_NIELSON_FOLEY_PARAM:
	    CagdPtsNielsonMetric(PtList, G);
	    *R++ = 0.0;
	    LastPt = NULL;
	    for (Pt = PtList; Pt -> Pnext != NULL; Pt = Pt -> Pnext, R++) {
	        *R = *(R - 1) + IRIT_UEPS +
		     CagdPtsNielsonFoleyParam(G, LastPt, Pt, Pt -> Pnext,
					      Pt -> Pnext -> Pnext);
		LastPt = Pt;
	    }
	    break;
	case CAGD_UNIFORM_PARAM:
	default:
	    /* For periodic cases, last point is the same as first point    */
	    /* so we better not put different conflicting constraint there. */
	    for (i = 0; i < NumPts; i++)
		*R++ = (Periodic || NumPts == 1)
					 ? ((CagdRType) i) / NumPts
				         : ((CagdRType) i) / (NumPts - 1);
	    break;
    }

    /* Construct the knot vector of the Bspline curve. */
    if (Periodic) {
        KV = (CagdRType *) IritMalloc(sizeof(CagdRType) *
				      (CrvPerSize + Order));

	for (i = CrvPerSize + Order1; i >= 0; i--)
	    KV[i] = (i - Order1) / ((CagdRType) CrvSize);
    }
    else if (CrvSize <= NumPts) {
	/* Overconstraint problem. */
        KV = BspPtSamplesToKV(PtKnots, NumPts, Order, CrvSize);
    }
    else {
	/* Under constraint problem. */
	CagdRType r, Index;

        KV = (CagdRType *) IritMalloc(sizeof(CagdRType) *
				      (CrvPerSize + Order));

	for (i = 0, R = KV, R2 = PtKnots; i < Order; i++)
	    *R++ = *R2;
	r = ((CagdRType) (NumPts - 1)) / (1 + CrvSize - Order);
	for (i = 0, Index = r; i < CrvSize - Order; i++, Index += r) {
	    CagdRType
		RIndex = IRIT_BOUND(Index, 0, NumPts - 1 - IRIT_UEPS);
	    int IIndex = (int) RIndex;
	    CagdRType
		Frac = RIndex - IIndex;

	    *R++ = PtKnots[IIndex] * (1.0 - Frac) + PtKnots[IIndex + 1] * Frac;
	}
	if (NumPts == 1)
	    for (i = 0, R2 = PtKnots; i < Order; i++)
	        *R++ = 1.0 + *R2;
	else
	    for (i = 0, R2 = &PtKnots[NumPts - 1]; i < Order; i++)
	        *R++ = *R2;
    }

    *RetPtKnots = PtKnots;
    *RetKV = KV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given NumPts parameter values (in PtsSamples), construct a knot vector   M
* for a curve of order CrvOrder and CrvLength control points to fit to this  M
* data set.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtsSamples: The parameter values of the data.                            M
*   NumPts:     Number of parameters in PtsSamples.                          M
*   CrvOrder:   Order of curve that will employ the constructed knot vector. M
*   CrvLength:  Length of curve that will employ constructed knot vector.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Constructed knot vector.                                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspKnotAverage                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspPtSamplesToKV                                                         M
*****************************************************************************/
CagdRType *BspPtSamplesToKV(const CagdRType *PtsSamples,
			    int NumPts,
			    int CrvOrder,
			    int CrvLength)
{
    int i,
	CrvOrder1 = CrvOrder - 1;
    CagdRType *R, *R2, *AveKV,
    	*KV = (CagdRType *) IritMalloc(sizeof(CagdRType) *
						      (CrvLength + CrvOrder));

    if (NumPts >= CrvLength) {
	AveKV = BspKnotAverage(PtsSamples, NumPts,
					      NumPts + CrvOrder1 - CrvLength);

	BspKnotAffineTrans(AveKV, CrvLength - CrvOrder + 2,
			   PtsSamples[0] - AveKV[0],
			   (PtsSamples[NumPts - 1] - PtsSamples[0]) /
			            (AveKV[CrvLength - CrvOrder1] - AveKV[0]));

	for (i = 0, R = KV, R2 = AveKV; i < CrvOrder; i++)
	    *R++ = *R2;
	for (i = 0; i < CrvLength - CrvOrder; i++)
	    *R++ = *++R2;
	for (i = 0, R2++; i < CrvOrder; i++)
	    *R++ = *R2;

	IritFree(AveKV);
    }
    else {
        BspKnotUniformOpen(CrvLength, CrvOrder, KV);
    }

    return KV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given set of points, PtList, parameter values the curve should interpolate M
* or approximate these points, Params, and the expected knot vector, KV,     M
* length Length and order Order of the Bspline curve, computes the Bspline   M
* curve's coefficients.							     M
*   All points in PtList are assumed of the same type.			     M
*   If Periodic, Order - 1 more constraints (and DOF's) are added so that    M
* the first Order - 1 points are the same as the last Order - 1 points.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       List of points to interpolate/least square approximate.    M
*   Params:       At which to interpolate the points in PtList.              M
*   KV:           Computed knot vector for the constructed curve.            M
*   Length:       Number of degrees of freedom (control points) of the       M
*                 interpolating/approximating curve.                         M
*   Order:        Of interpolating/approximating curve.                      M
*   Periodic:     Constructed curve should be Periodic.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Constructed interpolating/approximating curve, NULL   M
*		  if singular.						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvInterpolate, interpolation, least square approximation             M
*****************************************************************************/
CagdCrvStruct *BspCrvInterpolate(const CagdCtlPtStruct *PtList,
				 const CagdRType *Params,
				 const CagdRType *KV,
				 int Length,
				 int Order,
				 CagdBType Periodic)
{
    CagdBType
	MiddleCtlPtConstrained = FALSE;
    int i, j, k, l, MatSize, VecSize, Symmetry[3], CircSym, Area[3],
	NumPts = CagdListLength(PtList),
	AreaSum = 0,
	SymmetrySum = 0,
	Order1 = Order - 1;
    CagdRType *Mat, *InterpPts, *M, *R, TMin, TMax,
	**SymState = NULL,
	**AreaInnerProds = NULL,
	**AreaPts = NULL;
    const CagdRType *R2;
    const CagdCtlPtStruct *Pt;
    CagdCrvStruct *Crv;
    CagdPointType
	PtType = PtList -> PtType;

    /* Construct the Bspline curve and its knot vector. */
    Crv = BspPeriodicCrvNew(Length, Order, Periodic, PtType);
    CAGD_GEN_COPY(Crv -> KnotVector, KV,
		  (CAGD_CRV_PT_LST_LEN(Crv) + Order) * sizeof(CagdRType));

    /* Allocate the system of equations' matrix and clear it.  Note we       */
    /* allocate more than will probably needed due to possible symmetry      */
    /* constraints that might be imposed.				     */
    MatSize = sizeof(CagdRType) * Length * (NumPts + Length * 2);
    M = Mat = (CagdRType *) IritMalloc(MatSize);
    IRIT_ZAP_MEM(Mat, MatSize);

    /* Make sure all parameters are in the domain. */
    CagdCrvDomain(Crv, &TMin, &TMax);
    CAGD_VALIDATE_MIN_MAX_DOMAIN(TMax, TMin, TMax);

    for (l = 0, R2 = Params; l < NumPts; l++, R2++) {
        if (*R2 < TMin || *R2 >= TMax) {
	    R = (CagdRType *) R2;  /* Should not have values outside domain. */
	    *R = IRIT_BOUND(*R, TMin, TMax);		     /* Force a fix. */
	}
    }

    /* Construct the system of equations' matrix. */
    for (l = 0, R2 = Params, Pt = PtList;
	 l < NumPts;
	 l++, R2++, Pt = Pt -> Pnext) {
	if ((i = AttrGetIntAttrib(Pt -> Attr,
				  "Derivative")) != IP_ATTR_BAD_INT) {
	    int Index;
	    CagdRType *Line;

	    if (CAGD_IS_RATIONAL_PT(Pt -> PtType))
		CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);

	    switch (i) {
		case 1:
		    /* First derivative constraint.  Results from the        */
		    /* derivative formulation of a Bspline basis function:   */
		    /*                       Bi,n-1(t)    Bi+1,n-1(t)        */
		    /* B'i,n(t) = (n - 1) [ ----------- - ----------- ]      */
		    /*                      ti+n-1 - ti   ti+n - ti+1        */
		    Line = BspCrvCoxDeBoorBasis(&KV[1], Order1,
					      Length + (Periodic ? Order1 : 0),
					      FALSE, *R2, &Index);
		    for (j = Order1, k = Index + 1; j > 0; j--, k++) {
			CagdRType
			    w = (KV[k + Order1] - KV[k]) /
						    (Order1 == 0 ? 1 : Order1);

			if (w != 0.0)
			    w = *Line++ / w;

			if (Periodic)
			    M[k % Length] += w;
			else if (k < Length)
			    M[k] += w;

			if (Periodic)
			    M[(Length + k - 1) % Length] -= w;
			else if (k > 0 && k <= Length)
			    M[k - 1] -= w;
		    }
		    break;
		default:
		    CAGD_FATAL_ERROR(CAGD_ERR_WRONG_DERIV_ORDER);
		    return NULL;
	    }
	    M += Length;
	}
	else if ((i = AttrGetIntAttrib(Pt -> Attr,
				       "CtlPt")) != IP_ATTR_BAD_INT) {
	    M[i = IRIT_ABS(i) % Length] = 1.0;
	    M += Length;
	    if ((Length & 0x01) == 0x01 && i == (Length >> 1))
		MiddleCtlPtConstrained = TRUE;
	}
	else if ((i = AttrGetIntAttrib(Pt -> Attr,
				       "Symmetry")) != IP_ATTR_BAD_INT &&
		 (SymState = (CagdRType **) AttrGetPtrAttrib(Pt -> Attr,
						       "SymState")) != NULL) {
	    Symmetry[0] = i & CAGD_CONST_X_SYMMETRY ? TRUE : FALSE;
	    Symmetry[1] = i & CAGD_CONST_Y_SYMMETRY ? TRUE : FALSE;
	    Symmetry[2] = i & CAGD_CONST_Z_SYMMETRY ? TRUE : FALSE;
	    CircSym = i & CAGD_CONST_C_SYMMETRY ? TRUE : FALSE;
	    SymmetrySum = Symmetry[0] + Symmetry[1] + Symmetry[2] + CircSym;
	}
	else if ((i = AttrGetIntAttrib(Pt -> Attr,
				       "Area")) != IP_ATTR_BAD_INT &&
		 (AreaInnerProds = (CagdRType **) AttrGetPtrAttrib(Pt -> Attr,
						 "AreaInnerProds")) != NULL &&
		 (AreaPts = (CagdRType **) AttrGetPtrAttrib(Pt -> Attr,
							"AreaPts")) != NULL) {
	    Area[0] = i & CAGD_CONST_X_AREA ? TRUE : FALSE;
	    Area[1] = i & CAGD_CONST_Y_AREA ? TRUE : FALSE;
	    Area[2] = FALSE;
	    /* Allow exactly one area constraint. */
	    if (Area[0] && Area[1])
		Area[0] = FALSE;
	    if (!Area[0] && !Area[1])
		Area[1] = TRUE;
	    AreaSum = Area[0] + Area[1];
	}
	else {			  /* Regular point interpolation constraint. */
	    int Index;
	    CagdRType
		*Line = BspCrvCoxDeBoorBasis(KV, Order, Length, Periodic,
					     *R2, &Index);

	    for (j = Order, k = Index; j > 0; j--, k++)
		M[k % Length] = *Line++;
	    M += Length;
	}
    }

    /* If we have a symmetry and/or area constraints we need to solve each   */
    /* axis with its own rights as not all axes are solved the same.	     */
    if (SymmetrySum > 0 || AreaSum > 0) { /* Have symmetry/area constraints! */
        int NumCnstns, StartAxis, EndAxis, StepAxis;
        CagdRType
	    *MBase = M;

	VecSize = sizeof(CagdRType) * (NumPts + Length * 2);
	InterpPts = (CagdRType *) IritMalloc(VecSize);

	/* Should we solve X first or Y first? */
	if (Area[1] && !Area[0]) {
	    StartAxis = !CAGD_IS_RATIONAL_PT(PtType);
	    EndAxis = CAGD_NUM_OF_PT_COORD(PtType) + 1;
	    StepAxis = 1;
	}
	else {
	    StartAxis = CAGD_NUM_OF_PT_COORD(PtType);
	    EndAxis = !CAGD_IS_RATIONAL_PT(PtType) - 1;
	    StepAxis = -1;
	}

	for (i = StartAxis; i != EndAxis; i += StepAxis) {
	    int l;
	    CagdRType Avg2;

	    /* Place the regular (non symmetry) constraints. */
	    for (Pt = PtList, R = InterpPts, j = NumPts, NumCnstns = 0;
		 j > 0;
		 Pt = Pt -> Pnext, j--) {
	        if (AttrGetIntAttrib(Pt -> Attr,
				     "Symmetry") == IP_ATTR_BAD_INT &&
		    AttrGetIntAttrib(Pt -> Attr,
				     "Area") == IP_ATTR_BAD_INT) {
		    *R++ = Pt -> Coords[i];
		    NumCnstns++;
		}
	    }

	    /* Add the symmetry and/or area constraint. */
	    M = MBase;
	    if (SymmetrySum > 0 && i > 0 && i < 4) {
	        if (Symmetry[i - 1]) {
		    /* Coerce a point similarity, X(i) = X(n-i). */
		    for (j = 0; j < (Length >> 1); j++, M += Length) {
		        M[j] = 1.0;
			M[Length - 1 - j] = -1.0;
			*R++ = SymState[i][Length - 1 - j] - SymState[i][j];
			NumCnstns++;
		    }
		}
		else {
		    Avg2 = (SymState[i][Length - 1] + SymState[i][0]);
		    l = MiddleCtlPtConstrained ? (Length >> 1)
					       : ((Length + 1) >> 1);

		    /* Coerce a point reflect, X(i) + X(n-i) = Avg2. */
		    for (j = 0; j < l; j++, M += Length) {
		        if (j == Length - 1 - j) {
			    M[j] = 1.0;
			    *R++ = Avg2 * 0.5 - SymState[i][j];
			}
			else {
			    M[j] = 1.0;
			    M[Length - 1 - j] = 1.0;
			    *R++ = Avg2
			        - SymState[i][j] - SymState[i][Length - 1 - j];
			}
			NumCnstns++;
		    }
		}

		if (SymmetrySum > 1) {
		    l = (Length >> 1);

		    /* We have another axis that requires symmetry.  Reflect */
		    /* or coerce similarity, depending upon Symmetry[i].     */
		    for (j = 0; j < (l >> 1); j++, M += Length) {
		        if (Symmetry[i - 1]) {
			    M[j] = 1.0;
			    M[l - 1 - j] = -1.0;
			    *R++ = SymState[i][l - 1 - j] - SymState[i][j];
			    NumCnstns++;
			}
			else {
			    M[j] = 1.0;
			    M[l - 1 - j] = 1.0;
			    *R++ = -SymState[i][j] - SymState[i][l - 1 - j];
			    NumCnstns++;
			}
		    }
		}
	    }
	    if (AreaSum > 0 && i > 0 && i < 4) {
		if (Area[i - 1]) {
		    *R++ = UpdateAreaCrvConstraint(Crv, AreaPts,
						   AreaInnerProds, i,
						   Length, Order, Periodic,
						   KV, M);
		    M += Length;
		    NumCnstns++;
		}
	    }

#	    ifdef DEBUG
	    {
	        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSolver, FALSE) {
		    if (i == StartAxis)
		        IRIT_INFO_MSG("\n");
		    IRIT_INFO_MSG_PRINTF("Input to solver (axis = %d):\n",
					    i);
		    for (l = 0; l < NumCnstns; l++) {
		        IRIT_INFO_MSG("[");
			for (j = 0; j < Length; j++) {
			    IRIT_INFO_MSG_PRINTF("%9.6f ",
						    Mat[l * Length + j]);
			}
			IRIT_INFO_MSG_PRINTF("] %9.6f\n", InterpPts[l]);
		    }
		}
	    }
#	    endif /* DEBUG */

	    if (NumCnstns == 0) {
	        IRIT_ZAP_MEM(Crv -> Points[i],
			     Crv -> Length * sizeof(CagdRType));
	    }
	    else if (NumCnstns < Length) {
		/* System is underdetermined, use QR factorization to solve. */

		if (IritQRUnderdetermined(Mat, NULL, NULL,
					  NumCnstns, Length)) {
		    IritFree(Mat);
		    CagdCrvFree(Crv);
		    return NULL;
		}

		/* Solve for the coefficients of this curve's axis. */
		IritQRUnderdetermined(NULL, Crv -> Points[i], InterpPts,
				      NumCnstns, Length);
	    }
	    else {
		/* Compute SVD decomposition for Mat. */
		if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumCnstns, Length))
								 < IRIT_UEPS) {
		    IritFree(Mat);
		    CagdCrvFree(Crv);
		    return NULL;
		}
    
		/* Solve for the coefficients of this curve's axis. */
		SvdLeastSqr(NULL, Crv -> Points[i], InterpPts,
			    NumCnstns, Length);
	    }
	}

	IritFree(InterpPts);
    }
    else { /* No symmetry/area constriants. */
#	ifdef DEBUG
        {
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSolver, FALSE) {
	        IRIT_INFO_MSG("Input to solver\n");
		for (l = 0; l < NumPts; l++) {
		    IRIT_INFO_MSG("[");
		    for (j = 0; j < Length; j++) {
		        IRIT_INFO_MSG_PRINTF("%9.6f ", Mat[l * Length + j]);
		    }
		    IRIT_INFO_MSG("]\n");
		}
	    }
	}
#	endif /* DEBUG */

        if (NumPts < Length) {
	    /* The system is underdetermined, use QR factorization to solve. */

	    if (IritQRUnderdetermined(Mat, NULL, NULL, NumPts, Length)) {
		IritFree(Mat);
		CagdCrvFree(Crv);
		return NULL;
	    }

	    /* Solve for the coefficients of all the coordinates of curve. */
	    VecSize = sizeof(CagdRType) * NumPts;
	    InterpPts = (CagdRType *) IritMalloc(VecSize);
	    for (i = !CAGD_IS_RATIONAL_PT(PtType);
		 i <= CAGD_NUM_OF_PT_COORD(PtType);
		 i++) {
	        for (Pt = PtList, R = InterpPts, j = NumPts;
		     j > 0;
		     Pt = Pt -> Pnext, j--)
		    *R++ = Pt -> Coords[i];
	    
		IritQRUnderdetermined(NULL, Crv -> Points[i], InterpPts,
				      NumPts, Length);
	    }

	    IritFree(Mat);
	    IritFree(InterpPts);
	}
	else {
	    /* Compute SVD decomposition for Mat. */
	    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL, NumPts, Length))
								 < IRIT_UEPS) {
		IritFree(Mat);
		CagdCrvFree(Crv);
		return NULL;
	    }
    
	    /* Solve for the coefficients of all the coordinates of curve. */
	    VecSize = sizeof(CagdRType) * NumPts;
	    InterpPts = (CagdRType *) IritMalloc(VecSize);
	    for (i = !CAGD_IS_RATIONAL_PT(PtType);
		 i <= CAGD_NUM_OF_PT_COORD(PtType);
		 i++) {
	        for (Pt = PtList, R = InterpPts, j = NumPts;
		     j > 0;
		     Pt = Pt -> Pnext, j--)
		    *R++ = Pt -> Coords[i];
	    
		SvdLeastSqr(NULL, Crv -> Points[i], InterpPts,
			    NumPts, Length);
	    }
	    SvdLeastSqr(NULL, NULL, NULL, 0, 0);		/* Clean up. */

	    IritFree(Mat);
	    IritFree(InterpPts);
	}
    }

#   ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintOutputSolver, FALSE) {
	    IRIT_INFO_MSG("output of solver:\n");
	    CagdDbg(Crv);
	}
    }
#   endif /* DEBUG */

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits a curve to the give curve by sampling points on Crv and             M
* fitting a curve of orders Order and Size control points.                   M
*   Error is measured by the difference between the original and the fitted  M
* surface, as maximum error norm.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         Curve to fit a new curve to.                                M
*   Order:       Of the to be created curve.                                 M
*   Size:        Control polygon size the to be created curve.               M
*   ParamType:   Type of parametrization.                                    M
*   EndPtInterp: TRUE to force Crv's end point interpolation.  Has affect    M
*		 only if has open end-conditions.	                     M
*   EvalPts:     TRUE to evaluate the samples points at equal parametric     M
*		 interval, FALSE to simply copy the control points.	     M
*   Err:         The maximum error is updated into here                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Fitted surface.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpPts                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvFitLstSqr                                                          M
*****************************************************************************/
CagdCrvStruct *BspCrvFitLstSqr(const CagdCrvStruct *Crv,
			       int Order,
			       int Size, 
			       CagdParametrizationType ParamType,
			       int EndPtInterp,
			       int EvalPts,
			       CagdRType *Err)
{
    CagdPointType
	PtType = Crv -> PType;
    CagdBType
	Periodic = CAGD_IS_PERIODIC_CRV(Crv);
    int OldInterp,
	MaxDimension = CAGD_NUM_OF_PT_COORD(PtType),
	Samples = Order + Size * BSP_CRV_FIT_SAMPLE_RATIO;
    CagdRType TMin, TMax, t, dt, *KV, *R, *PtKnots;
    CagdCtlPtStruct
	*CtlPt = NULL,
	*CtlPtList = NULL;
    CagdCrvStruct *DCrv, *NewCrv;
    CagdBBoxStruct BBox;

    CagdCrvDomain(Crv, &TMin, &TMax);

    /* Sample the curve into a linear list */
    if (EvalPts) {
	dt = (TMax - TMin - IRIT_UEPS) / (Samples - 1);
	t = TMin;
	for (t = TMin; t <= TMax; t += dt) {
	    if (CtlPtList == NULL)
		CtlPtList = CtlPt = CagdCtlPtNew(Crv -> PType);
	    else {
		CtlPt -> Pnext = CagdCtlPtNew(Crv -> PType);
		CtlPt = CtlPt -> Pnext;
	    }

	    R = CagdCrvEval(Crv, t);
	    CAGD_GEN_COPY(CtlPt -> Coords, R,
		          sizeof(CagdRType) * (1 + MaxDimension));
	}
    }
    else {
	int i, j;
	CagdRType 
	    * const *Pts = Crv -> Points;

	for (i = 0; i < Crv -> Length; i++) {
	    if (CtlPtList == NULL)
		CtlPtList = CtlPt = CagdCtlPtNew(Crv -> PType);
	    else {
		CtlPt -> Pnext = CagdCtlPtNew(Crv -> PType);
		CtlPt = CtlPt -> Pnext;
	    }

	    for (j = !Periodic; j <= MaxDimension; j++)
		CtlPt -> Coords[j] = Pts[j][i];
	}
    }

    if (ParamType == CAGD_KV_NODAL_PARAM)
	KV = (CagdRType *) Crv -> KnotVector;         /* KV is not changed! */
    BspCrvInterpBuildKVs(CtlPtList, Order, Size, ParamType, Periodic,
			 &PtKnots, &KV);
    NewCrv = BspCrvInterpolate(CtlPtList, PtKnots, KV, Size, Order, Periodic);
    CagdCtlPtFreeList(CtlPtList);

    if (NewCrv != NULL) {
        /* Match the domains. */
        BspKnotAffineTransOrder2(NewCrv -> KnotVector, NewCrv -> Order,
				 CAGD_CRV_PT_LST_LEN(NewCrv)
				     + NewCrv -> Order,
				 TMin, TMax);

	/* Measure the error. */
	OldInterp = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

	DCrv = SymbCrvSub(Crv, NewCrv);
	CagdCrvBBox(DCrv, &BBox);
	CagdCrvFree(DCrv);

	t = IRIT_PT_LENGTH(BBox.Min);
	dt = IRIT_PT_LENGTH(BBox.Max);
	*Err = IRIT_MAX(t, dt);

	BspMultComputationMethod(OldInterp);

	if (EndPtInterp && BspCrvHasOpenEC(Crv) && BspCrvHasOpenEC(NewCrv)) {
	    int i;

	    /* Copy end points of original Crv to NewCrv. */
	    assert(Crv -> PType == NewCrv -> PType);

	    for (i = !CAGD_IS_RATIONAL_PT(Crv -> PType);
		 i <= CAGD_NUM_OF_PT_COORD(Crv -> PType);
		 i++) {
	        NewCrv -> Points[i][0] = Crv -> Points[i][0];
		NewCrv -> Points[i][NewCrv -> Length - 1] =
	                                Crv -> Points[i][Crv -> Length - 1];
	    }
	}
    }

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates one vector in the linear matrix of constraints, to coerce the    *
* area to remain unchanged, by solving for one axis of the modifying curve.  *
*   Let the area of the original curve, CC = CrntCrv, be A.  We require the  *
* area of modified curve (CC + DC) = (CrntCrv + DeltaCrv) to be A as well.   *
* Hence (A is actually twice the area here, but we seek its zero anyway):    *
*      _                                  _				     *
*     |                                  |				     *
* A = | (CC + DC) x (CC + DC)' dt = A +  | CC x DC' + DC x CC' + DC x DC' dt *
*     |                                  |				     *
*    -                                  -				     *
*      									     *
* Or we should constrain (CC = (Cx(t), Cy(t)),  DC = (Dx(t), Dy(t)):         *
*      _                                   				     *
*     |                                   				     *
* 0 = | Cx(t) Dy'(t) - Dx'(t) Cy(t) + Dx(t) Cy'(t) - Cx'(t) Dy(t)            *
*     |                                 + Dx(t) Dy'(t) - Dx'(t) Dy(t) dt     *
*    -                                  				     *
*      									     *
* Which, in turns,							     *
*   _                                  _   				     *
*  |                                  |  				     *
*  | Cx'(t) Dy(t) - Cx(t) Dy'(t) dt = | Dx(t) (Cy'(t) + Dy'(t) -             *
*  |                                  |            Dx'(t) (Cy(t) + Dy(t)) dt *
* -                                  - 				             *
*      									     *
* while noting that the left hand side is completely known.  The left hand   *
* side is the returned value of this function.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   NewCrv:        New curve we are solving for now, DC in the above.        *
*   CrntCrvPts:    Current curve's control points.                           *
*   CrntCrvDPts:   Current curve first derivative's control points.          *
*   CrntSpcInnerProds:   Inner product of all basis functions between the    *
*		   current curve function space and the space of its         *
*		   derivative curve, "Int(Bi,k(t) Bj,k-1(t) dt)", "int"      *
*		   denotes the integral of.  See SymbBspBasisInnerProdMat    *
*   Axis:          We are solving for in NewCrv, either 1 or 2 for X or Y.   *
*   Length:        Of the control points' vector.                            *
*   Order:	   Of (undifferentiated) curves.			     *
*   Periodic:	   TRUE if curve is periodic.				     *
*   KV:		   Knot vector of the function space of X(t) and Y(t).       *
*   MatVector:     Where to update the constraint into.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     Left hand side of the constraint - see above.	     *
*****************************************************************************/
static CagdRType UpdateAreaCrvConstraint(CagdCrvStruct *NewCrv,
					 CagdRType **CrntCrvPts,
					 CagdRType **CrntSpcInnerProds,
					 int Axis,
					 int Length,
					 int Order,
					 CagdBType Periodic,
					 const CagdRType *KV,
					 CagdRType *MatVector)
{
    int i,
	OAxis = 3 - Axis;
    CagdRType
	RetVal = 0.0,
	*TmpPts = (CagdRType *) IritMalloc(sizeof(CagdRType) * Length),
	*NOPts = NewCrv -> Points[OAxis],
	*CPts = CrntCrvPts[Axis],
	*COPts = CrntCrvPts[OAxis];

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugCompCrntArea, FALSE) {
	    int j,
		Len = Periodic ? Length + Order - 1 : Length;
	    CagdRType
	        Area = 0.0;

	    IRIT_INFO_MSG("Inner Products:\n");
	    for (i = 0; i < Len; i++) {
	        for (j = 0; j < Len; j++)
		    IRIT_INFO_MSG_PRINTF("%8.5f ", CrntSpcInnerProds[i][j]);
		IRIT_INFO_MSG("\n");
	    }

	    /* Verify area of current curve using its X(t), Y(t) coeffs. */
	    UpdateOneAreaCrvConstraint(COPts, CrntSpcInnerProds, Length, Order,
				       Periodic, KV, MatVector);
	    for (i = 0; i < Len; i++)
	        Area += MatVector[i] * CPts[i % Length];

	    IRIT_INFO_MSG_PRINTF("Area currently is %f\n",
				    IRIT_FABS(Area * 0.5));
	}
    }
#endif /* DEBUG */

    /* Compute the term free of our unknowns - the left hand side. */
    UpdateOneAreaCrvConstraint(NOPts, CrntSpcInnerProds, Length, Order,
			       Periodic, KV, MatVector);
    for (i = 0; i < Length; i++)
	RetVal -= MatVector[i] * CPts[i];

    /* Evaluate the right hand side of the above equation. */
    for (i = 0; i < Length; i++)
	TmpPts[i] = COPts[i] + NOPts[i];

    UpdateOneAreaCrvConstraint(TmpPts, CrntSpcInnerProds, Length, Order,
			       Periodic, KV, MatVector);

    IritFree(TmpPts);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Updates one vector in the linear matrix of constraints, with one         *
* constraint of the following form over the unknown curve X(t)               *
*      _                                  				     *
*     |                                   				     *
* A = | -X'(t) Y(t) + X(t) Y'(t) dt       				     *
*     |                                   				     *
*    -                                   				     *
*                                                                            *
* PARAMETERS:                                                                *
*   YPts:          The coefficients of the Y(t) curve.                       *
*   InnerProds:    Inner product of all basis functions between the          *
*		   current curve function space and the space of its         *
*		   derivative curve, "Int(Bi,k(t) Bj,k-1(t) dt)", "int"      *
*		   denotes the integral of.  See SymbBspBasisInnerProdMat    *
*   Length:        Of the control points' vector.                            *
*   Order:	   of (undifferentiated) curves.			     *
*   Periodic:	   TRUE if curve is periodic.				     *
*   KV:		   Knot vector of the function space of X(t) and Y(t).       *
*   MatVector:     Where to update the constraint into.                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   None			                                             *
*****************************************************************************/
static void UpdateOneAreaCrvConstraint(CagdRType *YPts,
				       CagdRType **InnerProds,
				       int Length,
				       int Order,
				       CagdBType Periodic,
				       const CagdRType *KV,
				       CagdRType *MatVector)
{
    int i, j,
	Order1 = Order - 1,
	Len = Periodic ? Length + Order1 : Length;

    /* Add the area constraint by moving points along this axis only,        */
    /* which is a linear constraint!.					     */
    IRIT_ZAP_MEM(MatVector, Length * sizeof(CagdRType));

    for (i = 0; i < Len; i++) {
	for (j = 0; j < Len; j++) {
	    MatVector[i % Length] += YPts[j % Length] * InnerProds[i][j];
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points, and a curve least square fitting them using the     M
* BspCrvInterpPts function, computes an error measure as a the maximal	     M
* distance between the curve and points (L1 norm).			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:        Curve that was fitted to the data set.                       M
*   PtList:     The data set.                                                M
*   ParamType:  Parameter values at with curve should interpolate PtList.    M
*   Periodic:   Constructed curve should be Periodic.  Periodic		     M
*		necessitates uniform knot sequence in ParamType.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   Error measured in the L1 norm.                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrvInterpPtsError, error estimation, interpolation, least square      M
*   approximation                                                            M
*****************************************************************************/
CagdRType BspCrvInterpPtsError(const CagdCrvStruct *Crv,
			       const CagdPtStruct *PtList,
			       CagdParametrizationType ParamType,
			       CagdBType Periodic)
{
    int i, NumPts;
    const CagdPtStruct *Pt;
    CagdRType *PtKnots, *R, t,
	MaxDist = 0.0;

    for (NumPts = 0, Pt = PtList; Pt != NULL; NumPts++, Pt = Pt -> Pnext);

    R = PtKnots = (CagdRType *) IritMalloc(sizeof(CagdRType) * NumPts);

    if (Periodic)
        ParamType = CAGD_UNIFORM_PARAM;

    /* Compute parameter values at which the curve interpolated PtList. */
    switch (ParamType) {
	case CAGD_CHORD_LEN_PARAM:
	    *R++ = 0.0;
	    for (Pt = PtList; Pt -> Pnext != NULL; Pt = Pt -> Pnext, R++)
		*R = *(R - 1) + IRIT_PT_PT_DIST(Pt -> Pt, Pt -> Pnext -> Pt);
	    for (t = R[-1]; R != PtKnots; *--R /= t);
	    break;
	case CAGD_CENTRIPETAL_PARAM:
	    *R++ = 0.0;
	    for (Pt = PtList; Pt -> Pnext != NULL; Pt = Pt -> Pnext, R++)
		*R = *(R - 1) + sqrt(IRIT_PT_PT_DIST(Pt -> Pt, Pt -> Pnext -> Pt));
	    for (t = R[-1]; R != PtKnots; *--R /= t);
	    break;
	case CAGD_UNIFORM_PARAM:
	default:
	    for (i = 0; i < NumPts; i++)
		*R++ = ((CagdRType) i) / (NumPts - 1);
	    break;
    }

    for (i = 0, Pt = PtList; i < NumPts; i++, Pt = Pt -> Pnext) {
	CagdRType Dist;

	R = CagdCrvEval(Crv, PtKnots[i]);
	R = &R[1];

	Dist = IRIT_PT_PT_DIST(R, Pt -> Pt);
	if (Dist > MaxDist)
	    MaxDist = Dist;
    }

    IritFree(PtKnots);

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the Nielson metric (See "Nielson G. M. (1987), Coordinate free  *
* scattered data interpolation, in: Chui, C., Schumaker, L. L., and Utreras, *
* F. eds. Topics un Multivariate Approximation, Academic Press, 175-184") of *
* the given points in E2. 			                             *
*                                                                            *
* PARAMETERS:                                                                *
*   CtlPts:      List of control points.                                     *
*   G:           The coefficients of the Nielson metric.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CagdPtsNielsonMetric(const CagdCtlPtStruct *CtlPts,
				 CagdRType G[2][2])
{
    int i;
    CagdRType Vx, Vy, Vxy, g, XCntr, YCntr;
    const CagdRType *R;
    CagdPType PtE3;
    const CagdCtlPtStruct *Pt;

    XCntr = YCntr = 0.0;
    for (i = 0, Pt = CtlPts; Pt != NULL; i++, Pt = Pt -> Pnext) {
        R = Pt -> Coords;
        CagdCoerceToE3(PtE3, (CagdRType * const *) &R, -1, Pt -> PtType);
	XCntr += PtE3[0];
	YCntr += PtE3[1];
    }
    XCntr /= i;
    YCntr /= i;

    Vx = Vy = Vxy = 0.0;
    for (Pt = CtlPts; Pt != NULL; Pt = Pt -> Pnext) {
        R = Pt -> Coords;
        CagdCoerceToE3(PtE3, (CagdRType * const *) &R, -1, Pt -> PtType);
	Vx  += IRIT_SQR(PtE3[0] - XCntr);
	Vy  += IRIT_SQR(PtE3[1] - YCntr);
	Vxy += (PtE3[0] - XCntr) * (PtE3[1] - YCntr);
    }
    Vx  /= i;
    Vy  /= i;
    Vxy /= i;

    g = Vx * Vy - IRIT_SQR(Vxy);
    if (g <= 0.0)
        g = IRIT_UEPS;
    G[0][0] = Vx / g;
    G[1][0] = G[0][1] = -Vxy / g;
    G[1][1] = Vy / g;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the Nielson distance between the given two points, using the      *
* provided metric G  (See "Nielson G. M. (1987), Coordinate free scattered   *
* data interpolation, in: Chui, C., Schumaker, L. L., and Utreras, F. eds.   *
* Topics in Multivariate Approximation, Academic Press, 175-184").	     *
*                                                                            *
* PARAMETERS:                                                                *
*   G:           The coefficients of the Nielson metric.                     *
*   Pt1, Pt2:    The two points to compute the Nielson distance between.     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     The computed distance.                                    *
*****************************************************************************/
static CagdRType CagdPtsNielsonDist(CagdRType G[2][2],
				    CagdPType Pt1,
				    CagdPType Pt2)
{
    CagdRType R;
    CagdVType Diff;

    IRIT_VEC_SUB(Diff, Pt1, Pt2);

    R = Diff[0] * G[0][0] * Diff[0] +
	Diff[1] * G[1][0] * Diff[0] +
	Diff[0] * G[0][1] * Diff[1] +
	Diff[1] * G[1][1] * Diff[1];

    return sqrt(IRIT_MAX(R, 0.0));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the Nielson angle between the given three points, using the       *
* provided metric G  (See "Nielson G. M. (1987), Coordinate free scattered   *
* data interpolation, in: Chui, C., Schumaker, L. L., and Utreras, F. eds.   *
* Topics in Multivariate Approximation, Academic Press, 175-184").	     *
*                                                                            *
* PARAMETERS:                                                                *
*   G:              The coefficients of the Nielson metric.                  *
*   Pt1, Pt2, Pt3:  The three points to compute the Nielson angle between.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     The computed angle.                                       *
*****************************************************************************/
static CagdRType CagdPtsNielsonAngle(CagdRType G[2][2],
				     CagdPType Pt1,
				     CagdPType Pt2,
				     CagdPType Pt3)
{
    CagdRType
        Di_1 = CagdPtsNielsonDist(G, Pt1, Pt2),
        Di   = CagdPtsNielsonDist(G, Pt2, Pt3),
        Di2  = CagdPtsNielsonDist(G, Pt1, Pt3),
	V = (IRIT_SQR(Di_1) + IRIT_SQR(Di) - IRIT_SQR(Di2)) / (2 * Di * Di_1);
 
    /* Might be slightly larger for 3 colinear pts, due to num. error. */
    V = IRIT_BOUND(V, -1.0, 1.0);

    return M_PI - acos(V);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Computes the Nielson+Foley parametrization of at the given location.       *
* (See also Nielson, G. M. and Foley, T. A. (1989), A survey of applications *
* of an affine invariant norm, in: Lyche, T. and Schumaker, L. L. eds.,      *
* Mathematical methods in Computer Aided Geometric Design, Academic Press,   *
* 445-467).								     *
*                                                                            *
* PARAMETERS:                                                                *
*   G:              The coefficients of the Nielson metric.                  *
*   CtlPt0, CtlPt1, CtlPt2, CltPt3:  Four consequetive points to compute the *
*		    parametrization between Pt1 and Pt2.  Note that for the  *
*		    first and last segments, Pt0 and Pt3 are NULL,           *
*		    respectively.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:     The computed parameterization for this interval.          *
*****************************************************************************/
static CagdRType CagdPtsNielsonFoleyParam(CagdRType G[2][2],
					  const CagdCtlPtStruct *CtlPt0,
					  const CagdCtlPtStruct *CtlPt1,
					  const CagdCtlPtStruct *CtlPt2,
					  const CagdCtlPtStruct *CtlPt3)
{
    CagdRType RetVal;
    const CagdRType *R;
    CagdPType Pt0, Pt1, Pt2, Pt3;

    if (CtlPt0 != NULL) {
        R = CtlPt0 -> Coords;
        CagdCoerceToE3(Pt0, (CagdRType * const *) &R, -1, CtlPt0 -> PtType);
    }
    R = CtlPt1 -> Coords;
    CagdCoerceToE3(Pt1, (CagdRType * const *) &R, -1, CtlPt1 -> PtType);
    R = CtlPt2 -> Coords;
    CagdCoerceToE3(Pt2, (CagdRType * const *) &R, -1, CtlPt2 -> PtType);
    if (CtlPt3 != NULL) {
	R = CtlPt3 -> Coords;
        CagdCoerceToE3(Pt3, (CagdRType * const *) &R, -1, CtlPt3 -> PtType);
    }

    if (CtlPt0 == NULL) {                        /* It is the first segment. */
        RetVal = CagdPtsNielsonDist(G, Pt1, Pt2) *
	       (1.0 + 1.5 * CagdPtsNielsonAngle(G, Pt1, Pt2, Pt3) *
		            CagdPtsNielsonDist(G, Pt2, Pt3) /
		            (CagdPtsNielsonDist(G, Pt1, Pt2) +
			     CagdPtsNielsonDist(G, Pt2, Pt3)));
    }
    else if (CtlPt3 == NULL) {                    /* It is the last segment. */
        RetVal = CagdPtsNielsonDist(G, Pt1, Pt2) *
	       (1.0 + 1.5 * CagdPtsNielsonAngle(G, Pt0, Pt1, Pt2) *
		            CagdPtsNielsonDist(G, Pt0, Pt1) /
		            (CagdPtsNielsonDist(G, Pt0, Pt1) +
			     CagdPtsNielsonDist(G, Pt1, Pt2)));
    }
    else {                                     /* It is an interior segment. */
        RetVal = CagdPtsNielsonDist(G, Pt1, Pt2) *
	       (1.0 + 1.5 * (CagdPtsNielsonAngle(G, Pt1, Pt2, Pt3) *
			     CagdPtsNielsonDist(G, Pt2, Pt3) /
			     (CagdPtsNielsonDist(G, Pt1, Pt2) +
			      CagdPtsNielsonDist(G, Pt2, Pt3)) +
			     CagdPtsNielsonAngle(G, Pt0, Pt1, Pt2) *
			     CagdPtsNielsonDist(G, Pt0, Pt1) /
			     (CagdPtsNielsonDist(G, Pt0, Pt1) +
			      CagdPtsNielsonDist(G, Pt1, Pt2))));
    }

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes zero and first moments of a curve.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:     To compute zero and first moment.                               M
*   n:       Number of samples the curve should be sampled at.               M
*   Loc:     Center of curve as zero moment.                                 M
*   Dir:     Main direction of curve as first moment.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrvFirstMoments                                                      M
*****************************************************************************/
void CagdCrvFirstMoments(const CagdCrvStruct *Crv,
			 int n,
			 CagdPType Loc,
			 CagdVType Dir)
{
    int i;
    CagdRType t, TMin, TMax, Dt, **Points;
    CagdPtStruct *Pt,
	*PtList = NULL;
    CagdCrvStruct *InterpCrv;

    if (n < 2)
	n = 2;

    CagdCrvDomain(Crv, &TMin, &TMax);
    t = TMin;
    Dt = (TMax - TMin) / (n - 1);
    for (i = 0; i < n; i++, t += Dt) {
	CagdRType
	    *R = CagdCrvEval(Crv, t);

	Pt = CagdPtNew();
	CagdCoerceToE3(Pt -> Pt, &R, -1, Crv -> PType);
	IRIT_LIST_PUSH(Pt, PtList);
    }

    InterpCrv = BspCrvInterpPts(PtList, 2, 2, CAGD_UNIFORM_PARAM,
				Crv -> Periodic);
    CagdPtFreeList(PtList);
    Points = InterpCrv -> Points;

    Loc[0] = (Points[1][0] + Points[1][1]) * 0.5;
    Loc[1] = (Points[2][0] + Points[2][1]) * 0.5;
    Loc[2] = (Points[3][0] + Points[3][1]) * 0.5;

    Dir[0] = (Points[1][1] - Points[1][0]);
    Dir[1] = (Points[2][1] - Points[2][0]);
    Dir[2] = (Points[3][1] - Points[3][0]);
    IRIT_PT_NORMALIZE(Dir);

    CagdCrvFree(InterpCrv);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes a reparametrization scalar Bspline curve, y(x), so that each at M
* each point in PtsList, the curve parameter value of X is evaluated into Y. M
*                                                                            *
* PARAMETERS:                                                                M
*   PtsList:       List of points on the reparametrization curve.	     M
*   Order:	   of reparametrization curve.				     M
*   DegOfFreedom:  of reparametrization curve (== number of coefficients).   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  Result of reparametrization curve, computed using      M
*		 list squares fit.			                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvInterpolate                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspMakeReparamCurve                                                      M
*****************************************************************************/
CagdCrvStruct *BspMakeReparamCurve(const CagdPtStruct *PtsList,
				   int Order,
				   int DegOfFreedom)
{
    int i, j,
	Len = CagdListLength(PtsList);
    CagdRType r, *Pts, Min, Max;
    const CagdPtStruct *Pt;
    CagdCtlPtStruct *PtE1List;
    CagdRType *Kv, Delta,
	*Params = (CagdRType *) IritMalloc(Len * sizeof(CagdRType));
    CagdCrvStruct *Crv;

    Min = Max = PtsList -> Pt[1];
    for (Pt = PtsList, PtE1List = NULL, i = 0; Pt != NULL; Pt = Pt -> Pnext) {
	CagdCtlPtStruct
	    *PtE1 = CagdCtlPtNew(CAGD_PT_E1_TYPE);

	Params[i++] = Pt -> Pt[0];
	PtE1 -> Coords[1] = Max = Pt -> Pt[1];
	IRIT_LIST_PUSH(PtE1, PtE1List); 
    }
    PtE1List = CagdListReverse(PtE1List);

    Delta = ((CagdRType) Len) / (DegOfFreedom - Order);
    if (DegOfFreedom <= Order || DegOfFreedom > Len || Delta < 2.0) {
	CagdCtlPtFreeList(PtE1List);
	IritFree(Params);
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
        return NULL;
    }

    Kv = (CagdRType *) IritMalloc((DegOfFreedom + Order) * sizeof(CagdRType));
    
    for (i = j = 0; i < Order; i++, j++)
        Kv[j] = Params[0];
    for (r = Delta * 0.5; j < DegOfFreedom; r += Delta)
        Kv[j++] = Params[(int) r];
    for (i = 0; i < Order; i++)
        Kv[j++] = Params[Len - 1];
   
    /* Interpolate a bspline curve from the points. */
    Crv = BspCrvInterpolate(PtE1List, Params, Kv, DegOfFreedom, Order, FALSE);
    CagdCtlPtFreeList(PtE1List);
    IritFree(Kv);
    IritFree(Params);

    /* Make sure the curve is monotone. */
    Pts = Crv -> Points[1];
    for (i = 1; i < Crv -> Length; i++) {
	if (Pts[i] < Pts[i - 1])
	    Pts[i] = Pts[i - 1] + IRIT_UEPS;
    }

    /* Make sure the range spaned is proper. */
    r = (Max - Min) / (Pts[Crv -> Length - 1] - Pts[0]);
    for (i = 1; i < Crv -> Length; i++)
	Pts[i] = (Pts[i] - Pts[0]) * r + Min;

    /* Return the new parametrization curve. */
    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to compare two points in line fit for sorting purposes.          *
*                                                                            *
* PARAMETERS:                                                                *
*   VPt1, VPt2:  Two pointers to points.                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   >0, 0, or <0 as the relation between the two points.              *
*****************************************************************************/
#if defined(ultrix) && defined(mips)
static int LineFitSortCmpr(VoidPtr VPt1, VoidPtr VPt2)
#else
static int LineFitSortCmpr(const VoidPtr VPt1, const VoidPtr VPt2)
#endif /* ultrix && mips (no const support) */
{
    IrtRType
	Diff = (*((CagdPtStruct **) VPt1)) -> Pt[GlblLineFitSortAxis] -
	       (*((CagdPtStruct **) VPt2)) -> Pt[GlblLineFitSortAxis];

    return IRIT_SIGN(Diff);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sorts given list of points based on their increasing order in axis Axis. M
* Sorting is done in place.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:  List of points to sort.					     M
*   Axis:    Axis to sort along: 1,2,3 for X,Y,Z.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *:   Sorted list of points, in place.                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtsSortAxis                                                          M
*****************************************************************************/
CagdPtStruct *CagdPtsSortAxis(CagdPtStruct *PtList, int Axis)
{
    int l, 
	Len = CagdListLength(PtList);
    CagdPtStruct *Pt, **PtVec;

    if (Len < 2)
	return PtList;
        
    PtVec = (CagdPtStruct **) IritMalloc(sizeof(CagdPtStruct *) * Len);

    /* Sort all points in order of the most significant axis. */
    for (l = 0; l < Len; l++) {
	Pt = PtList;
	PtList = PtList -> Pnext;
	Pt -> Pnext = NULL;
	PtVec[l] = Pt;
    }

    GlblLineFitSortAxis = Axis - 1;
    qsort(PtVec, Len, sizeof(CagdPtStruct *), LineFitSortCmpr);

    PtList = PtVec[0];
    for (l = 0; l < Len - 1; l++)
	PtVec[l] -> Pnext = PtVec[l + 1];
    IritFree(PtVec);

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given set of points, PtList, fits a line using least squares fit to them.  M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       List of points to interpolate/least square approximate.    M
*   LineDir:      A unit vector of the line.                                 M
*   LinePos:      A point on the computed line.            		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Average distance between a point and the fitted line,      M
*		  or IRIT_INFNTY if failed.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarLineFitToPts			                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdLineFitToPts, interpolation, least square approximation              M
*****************************************************************************/
CagdRType CagdLineFitToPts(CagdPtStruct *PtList,
			   CagdVType LineDir,
			   CagdPType LinePos)
{
    int i, SortAxis,
	Len = CagdListLength(PtList);
    CagdRType AverageDist;
    CagdPType Min, Max;
    CagdVType Origin;
    CagdPtStruct *Pt;
    CagdCrvStruct *Crv;

    if (Len < 2)
	return IRIT_INFNTY;
        
    PtList = CagdPtCopyList(PtList);

    /* Sort all points in order of the most significant axis. */
    Min[0] = Min[1] = Min[2] = IRIT_INFNTY;
    Max[0] = Max[1] = Max[2] = -IRIT_INFNTY;

    for (Pt = PtList; Pt != NULL; Pt = Pt -> Pnext) {
	for (i = 0; i < 3; i++) {
	    if (Max[i] < Pt -> Pt[i])
		Max[i] = Pt -> Pt[i];
	    if (Min[i] > Pt -> Pt[i])
		Min[i] = Pt -> Pt[i];
	}
    }
    if (Max[0] - Min[0] > Max[1] - Min[1] &&
	Max[0] - Min[0] > Max[1] - Min[1])
	SortAxis = 1;
    else if (Max[1] - Min[1] > Max[0] - Min[0] &&
	     Max[1] - Min[1] > Max[2] - Min[2])
	SortAxis = 2;
    else
	SortAxis = 3;

    PtList = CagdPtsSortAxis(PtList, SortAxis);

    /* Fit a linear Bspline with two control points and use to find dir. */
    Crv = BspCrvInterpPts(PtList, 2, 2, CAGD_CENTRIPETAL_PARAM, FALSE);
    CagdCoerceToE3(LinePos, Crv -> Points, 0, Crv -> PType);
    CagdCoerceToE3(LineDir, Crv -> Points, 1, Crv -> PType);
    IRIT_VEC_SUB(LineDir, LineDir, LinePos);
    IRIT_VEC_NORMALIZE(LineDir);
    CagdCrvFree(Crv);

    /* Compute the LinePos by assuming at zero and deriving average shift. */
    IRIT_PT_RESET(LinePos);
    IRIT_PT_RESET(Origin);
    for (Pt = PtList; Pt != NULL; Pt = Pt -> Pnext) {
	CagdPType ClosestPoint;

	GMPointFromPointLine(Pt -> Pt, Origin, LineDir, ClosestPoint);
	IRIT_PT_SUB(ClosestPoint, Pt -> Pt, ClosestPoint);
	IRIT_PT_ADD(LinePos, LinePos, ClosestPoint);
    }
    IRIT_PT_SCALE(LinePos, 1.0 / Len);

    /* Compute average distance of the points to the line as error estimate. */
    for (AverageDist = 0.0, Pt = PtList; Pt != NULL; Pt = Pt -> Pnext) {
        AverageDist += GMDistPointLine(Pt -> Pt, LinePos, LineDir);
    }
    AverageDist /= Len;

    CagdPtFreeList(PtList);

    return AverageDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given set of points, PtList, fits an (approximated) plane fit to them.     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:       List of points to fit an approximated plane.               M
*   Pln:          The fitted plane's coefficients.                           M
*   MVec:         Direction in the plane with the major change.              M
*   Cntr:         The centrid of the data.			             M
*   CN:		  Condition number of the fitting.  The smaller this number  M
*                 is the more planar the input is.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Average distance between a point and the fitted plane,     M
*		  or IRIT_INFNTY if failed.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneFitToPts, approximation				             M
*****************************************************************************/
CagdRType CagdPlaneFitToPts(CagdPtStruct *PtList,
			    IrtPlnType Pln,
			    IrtVecType MVec,
			    IrtPtType Cntr,
			    IrtRType *CN)
{
    int n = 0;
    CagdRType RetVal;
    CagdPType
        *Vec = (CagdPType *) IritMalloc(sizeof(CagdPType) *
					             CagdListLength(PtList));

    while (PtList != NULL) {
        IRIT_PT_COPY(Vec[n], PtList -> Pt);
	PtList = PtList -> Pnext;
	n++;
    }

    RetVal = CagdPlaneFitToPts3(Vec, n, Pln, MVec, Cntr, CN);

    IritFree(Vec);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given set of points, PtList, fits an (approximated) plane fit to them.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:       List of points to fit an approximated plane,               M
*                 in format as in  CagdCrvStruct or CagdSrfStruct.	     M
*   NumPts:       Length of the vector of Points.			     M
*   PType:        Type of points in Points.				     M
*   Pln:          The fitted plane's coefficients.                           M
*   MVec:         Direction in the plane with the major change.              M
*   Cntr:         The centrid of the data.			             M
*   CN:		  Condition number of the fitting.  The smaller this number  M
*                 is the more planar the input is.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Average distance between a point and the fitted plane,     M
*		  or IRIT_INFNTY if failed.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneFitToPts2, approximation				             M
*****************************************************************************/
CagdRType CagdPlaneFitToPts2(CagdRType **Points,
			     int NumPts,
			     CagdPointType PType,
			     IrtPlnType Pln,
			     IrtVecType MVec,
			     IrtPtType Cntr,
			     IrtRType *CN)
{

    int i;
    CagdRType RetVal;
    CagdPType
        *Vec = (CagdPType *) IritMalloc(sizeof(CagdPType) * NumPts);

    for (i = 0; i < NumPts; i++) {
        CagdCoerceToE3(Vec[i],  Points, i, PType);
    }

    RetVal = CagdPlaneFitToPts3(Vec, NumPts, Pln, MVec, Cntr, CN);

    IritFree(Vec);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given set of points, PtList, fits an (approximated) plane fit to them.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:       List of points to fit an approximated plane,               M
*   NumPts:       Length of the vector of Points.			     M
*   Pln:          The fitted plane's coefficients.                           M
*   MVec:         Direction in the plane with the major change.              M
*   Cntr:         The centrid of the data.			             M
*   CN:		  Condition number of the fitting.  The smaller this number  M
*                 is the more planar the input is.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Average distance between a point and the fitted plane,     M
*		  or IRIT_INFNTY if failed.				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPlaneFitToPts3, approximation				             M
*****************************************************************************/
CagdRType CagdPlaneFitToPts3(CagdPType *Points,
			     int NumPts,
			     IrtPlnType Pln,
			     IrtVecType MVec,
			     IrtPtType Cntr,
			     IrtRType *CN)
{
    int i, j, Axis;
    IrtRType R, **A, *S, **V;

    if (NumPts < 3)
        return IRIT_INFNTY;

    A = (IrtRType **) IritMalloc((NumPts + 1) * sizeof(IrtRType *));
    V = (IrtRType **) IritMalloc(4 * sizeof(IrtRType *));
    S = IritMalloc((NumPts + 1) * sizeof(IrtRType));
    for (i = 0; i <= NumPts; i++)
        A[i] = IritMalloc(sizeof(IrtRType) * 4);
    for (i = 0; i <= 3; i++)
        V[i] = IritMalloc(sizeof(IrtRType) * 4);

    /* Compute the centroid. */
    IRIT_PT_RESET(Cntr);
    for (i = 0; i < NumPts; i++) {
	IRIT_PT_ADD(Cntr, Cntr, Points[i]);
    }
    IRIT_PT_SCALE(Cntr, 1.0 / NumPts);

    for (i = 0; i < NumPts; i++) {
        for (j = 0; j < 3; j++) {
	    A[i + 1][j + 1] = Points[i][j] - Cntr[j];
	}
    }

    SvdDecomp(A, NumPts, 3, S, V);

    /* Fetch the column of V with the minimal singular value. */
    Axis = 1;
    for (i = 2; i <= 3; i++)
        if (S[i] < S[Axis])
	    Axis = i;
    *CN = S[Axis];

    Pln[0] = V[1][Axis];
    Pln[1] = V[2][Axis];
    Pln[2] = V[3][Axis];
    Pln[3] = -IRIT_DOT_PROD(Pln, Cntr);

    /* Fetch the column of V with the maximal singular value. */
    Axis = 1;
    for (i = 2; i <= 3; i++)
        if (S[i] > S[Axis])
	    Axis = i;
    *CN /= S[Axis];

    MVec[0] = V[1][Axis];
    MVec[1] = V[2][Axis];
    MVec[2] = V[3][Axis];

    /* Clean up. */
    for (i = 0; i <= NumPts; i++)
        IritFree(A[i]);
    for (i = 0; i <= 3; i++)
        IritFree(V[i]);
    IritFree(A);
    IritFree(V);
    IritFree(S);

    /* Estimate the error. */
    R = 0.0;
    for (i = 0; i < NumPts; i++) {
        R += IRIT_ABS(IRIT_DOT_PROD(Pln, Points[i]) + Pln[3]);
    }

    return R / NumPts;
}
