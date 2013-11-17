/******************************************************************************
* SBspEval.c - Bspline surfaces handling routines - evaluation routines.      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include <string.h>
#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given tensor product Bspline surface at a given point, by    M
* extracting an isoparamteric curve along u from the surface and evaluating  M
* the curve at parameter v.                                                  M
*									     M
*		u -->							     V
*     +----------------------+						     V
*     |P0		 Pi-1|						     V
*   V |Pi		P2i-1|	Parametric space orientation - control mesh. V
*   | |			     |						     V
*   v |Pn-i		 Pn-1|						     V
*     +----------------------+						     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to evaluate at the given (u, v) location.             M
*   u, v:      Location where to evaluate the surface.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of curve Crv's point type. If for example the curve's      M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfEval, BzrSrfEvalAtParam, BspSrfEvalAtParam2, TrimSrfEval          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfEvalAtParam, evaluation, Bsplines                                  M
*****************************************************************************/
CagdRType *BspSrfEvalAtParam(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v)
{
    CagdCrvStruct *IsoSubCrv;
    CagdSrfEvalCacheStruct *SrfEvalCache;
    CagdRType *Pt, *VBasisFunc, UMin, UMax, VMin, VMax;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int k, UIndexFirst, VIndexFirst, SrfNextV,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
        ULength = Srf -> ULength,
        VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    if (u < UMin - IRIT_EPS || u > UMax + IRIT_EPS)
	CAGD_FATAL_ERROR(CAGD_ERR_U_NOT_IN_SRF);
    if (v < VMin - IRIT_EPS || v > VMax + IRIT_EPS)
	CAGD_FATAL_ERROR(CAGD_ERR_V_NOT_IN_SRF);
    CAGD_VALIDATE_MIN_MAX_DOMAIN(u, UMin, UMax);
    CAGD_VALIDATE_MIN_MAX_DOMAIN(v, VMin, VMax);

    /* Create the cached data structure, if surface has none. */
    if (Srf -> PAux == NULL) {
	SrfEvalCache = (CagdSrfEvalCacheStruct *)
				    IritMalloc(sizeof(CagdSrfEvalCacheStruct));
        ((CagdSrfStruct *) Srf) -> PAux = (VoidPtr) SrfEvalCache;

	UIndexFirst = BspKnotLastIndexLE(Srf -> UKnotVector,
				     (CAGD_SRF_UPT_LST_LEN(Srf) + UOrder), u) -
								  (UOrder - 1);
	SrfEvalCache -> UIndexFirst = UIndexFirst - 1;

	VIndexFirst = BspCrvCoxDeBoorIndexFirst(Srf -> VKnotVector, VOrder,
						CAGD_SRF_VPT_LST_LEN(Srf), v);
	SrfEvalCache -> VIndexFirst = VIndexFirst - 1;

	IsoSubCrv =
	    SrfEvalCache -> IsoSubCrv = BspCrvNew(UOrder, UOrder, Srf -> PType);
	SrfEvalCache -> VBasisFunc = IritMalloc(sizeof(CagdRType) * VOrder);
	SrfEvalCache -> v = v - 1.0;
    }
    else {
        SrfEvalCache = (CagdSrfEvalCacheStruct *) Srf -> PAux;

	UIndexFirst = BspKnotLastIndexLE(Srf -> UKnotVector,
				     (CAGD_SRF_UPT_LST_LEN(Srf) + UOrder), u) -
								  (UOrder - 1);
	VIndexFirst = BspCrvCoxDeBoorIndexFirst(Srf -> VKnotVector, VOrder,
						CAGD_SRF_VPT_LST_LEN(Srf), v);

	IsoSubCrv = SrfEvalCache -> IsoSubCrv;
    }

    /* Make sure the cached knot sequence in curve is the proper one. */
    if (SrfEvalCache -> UIndexFirst != UIndexFirst) {
        CAGD_GEN_COPY(IsoSubCrv -> KnotVector,
		      &Srf -> UKnotVector[UIndexFirst],
		      sizeof(CagdRType) * UOrder * 2);
    }

    if (SrfEvalCache -> v != v) {
        /* Make sure the cached V basis functions are at proper location. */
        VBasisFunc = BspCrvCoxDeBoorBasis(Srf -> VKnotVector, VOrder,
					  Srf -> VLength, Srf -> VPeriodic,
					  v, &VIndexFirst);

	CAGD_GEN_COPY(SrfEvalCache -> VBasisFunc, VBasisFunc,
		      sizeof(CagdRType) * VOrder);
    }
    else {
        VBasisFunc = SrfEvalCache -> VBasisFunc;
    }

    if (SrfEvalCache -> UIndexFirst != UIndexFirst || SrfEvalCache -> v != v) {
        SrfEvalCache -> UIndexFirst = UIndexFirst;
	SrfEvalCache -> VIndexFirst = VIndexFirst;
	SrfEvalCache -> v = v;

        /* Update the control points of the cached curve. */
        SrfNextV = CAGD_NEXT_V(Srf);
	for (k = 0; k < UOrder; k++) {
	    int l,
	        IndexFirst = CAGD_MESH_UV(Srf, UIndexFirst, VIndexFirst);

	    for (l = IsNotRational; l <= MaxCoord; l++) {
	        int i,
		    VIndexFirstTmp = VIndexFirst;
		CagdRType *R,
		    *SrfP = &Srf -> Points[l][IndexFirst],
		    *CrvP = &IsoSubCrv -> Points[l][k];

		*CrvP = 0.0;
		for (i = 0, R = VBasisFunc; i < VOrder; i++) {
		    *CrvP += *R++ * *SrfP;
		    if (++VIndexFirstTmp >= VLength) {
		        VIndexFirstTmp -= VLength;
			SrfP -= VLength * SrfNextV;
		    }
		    else
		        SrfP += SrfNextV;
		}
	    }

	    if (++UIndexFirst >= ULength)
	        UIndexFirst -= ULength;
	}
    }

    Pt = BspCrvEvalAtParam(IsoSubCrv, u);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function is the same as BspSrfEvalAtParam above. Cleaner, but much  M
* less efficient.						             M
*   Evaluates the given tensor product Bspline surface at a given point, by  M
* extracting an isoparamteric curve along u from the surface and evaluating  M
* the curve at parameter v.                                                  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to evaluate at the given (u, v) location.             M
*   u, v:      Location where to evaluate the surface.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of curve Crv's point type. If for example the curve's      M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                   This vector is allocated statically and a second         M
*		  invokation of this function will overwrite the first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfEval, BzrSrfEvalAtParam, BspSrfEvalAtParam, TrimSrfEval           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfEvalAtParam2, evaluation, Bsplines                                 M
*****************************************************************************/
CagdRType *BspSrfEvalAtParam2(const CagdSrfStruct *Srf,
			      CagdRType u,
			      CagdRType v)
{
    CagdRType *Pt;
    CagdCrvStruct
	*IsoCrv = BspSrfCrvFromSrf(Srf, u, CAGD_CONST_U_DIR);

    if (!BspKnotParamInDomain(IsoCrv -> KnotVector, IsoCrv -> Length,
			      IsoCrv -> Order, IsoCrv -> Periodic, v))
	CAGD_FATAL_ERROR(CAGD_ERR_V_NOT_IN_SRF);

    Pt = BspCrvEvalAtParam(IsoCrv, v);

    CagdCrvFree(IsoCrv);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts an isoparametric curve out of the given tensor product Bspline    M
* surface in direction Dir at the parameter value of t.                      M
*   Operations should prefer the CONST_U_DIR, in which the extraction is     M
* somewhat faster if that is possible.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract an isoparametric ocurve from.                      M
*   t:         Parameter value of extracted isoparametric curve.             M
*   dir:       Direction of the isocurve on the surface. Either U or V.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  An isoparametric curve of Srf. This curve inherits the M
*		      order and continuity of surface Srf in direction Dir.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvFromSrf, BzrSrfCrvFromSrf, CagdCrvFromMesh, BzrSrfCrvFromMesh,    M
*   BspSrfCrvFromMesh                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfCrvFromSrf, isoparametric curves, curve from surface               M
*****************************************************************************/
CagdCrvStruct *BspSrfCrvFromSrf(const CagdSrfStruct *Srf,
				CagdRType t,
				CagdSrfDirType dir)
{
    CagdCrvStruct
	*Crv = NULL;
    CagdBType Periodic,
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, CrvLen, IndexFirst, Order, Len, Step, OtherStep,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *CrvP, *SrfP, *BasisFuncs, *KV;

    switch (dir) {
	case CAGD_CONST_U_DIR:
	    if (!BspKnotParamInDomain(Srf -> UKnotVector, Srf -> ULength,
				      Srf -> UOrder, Srf -> UPeriodic, t))
		CAGD_FATAL_ERROR(CAGD_ERR_U_NOT_IN_SRF);
	    Crv = BspPeriodicCrvNew(CrvLen = Srf -> VLength, Srf -> VOrder,
				    Srf -> VPeriodic, Srf -> PType);
	    CAGD_GEN_COPY(Crv -> KnotVector, Srf -> VKnotVector,
			  sizeof(CagdRType) *
			      (CAGD_SRF_VPT_LST_LEN(Srf) + Srf -> VOrder));

	    KV = Srf -> UKnotVector;
	    Order = Srf -> UOrder;
	    Len = Srf -> ULength;
	    Periodic = Srf -> UPeriodic;
	    Step = CAGD_NEXT_U(Srf);
	    OtherStep = CAGD_NEXT_V(Srf);
	    break;
	case CAGD_CONST_V_DIR:
	    if (!BspKnotParamInDomain(Srf -> VKnotVector, Srf -> VLength,
				      Srf -> VOrder, Srf -> VPeriodic, t))
		CAGD_FATAL_ERROR(CAGD_ERR_V_NOT_IN_SRF);
	    Crv = BspPeriodicCrvNew(CrvLen = Srf -> ULength, Srf -> UOrder,
				    Srf -> UPeriodic, Srf -> PType);
	    CAGD_GEN_COPY(Crv -> KnotVector, Srf -> UKnotVector,
			  sizeof(CagdRType) *
			      (CAGD_SRF_UPT_LST_LEN(Srf) + Srf -> UOrder));

	    KV = Srf -> VKnotVector;
	    Order = Srf -> VOrder;
	    Len = Srf -> VLength;
	    Periodic = Srf -> VPeriodic;
	    Step = CAGD_NEXT_V(Srf);
	    OtherStep = CAGD_NEXT_U(Srf);
	    break;
	default:
	    KV = NULL;
	    Order = Len = Step = OtherStep = CrvLen = -1;
	    Periodic = FALSE;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    BasisFuncs = BspCrvCoxDeBoorBasis(KV, Order, Len, Periodic,
				      t, &IndexFirst);

    for (i = IsNotRational; i <= MaxCoord; i++) {
	CrvP = Crv -> Points[i];
	SrfP = Srf -> Points[i];
	for (j = 0; j < CrvLen; j++, CrvP++) {
	    BSP_CRV_EVAL_VEC_AT_PARAM(CrvP, SrfP, Step, Order,
				      Len, t, BasisFuncs, IndexFirst);
	    SrfP += OtherStep;
	}
    }

    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts a curve from the mesh of a tensor product Bspline surface Srf in  M
* direction Dir at index Index.                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       To extract a curve from.  		                     M
*   Index:     Index along the mesh of Srf to extract the curve from.        M
*   Dir:       Direction of extracted curve. Either U or V.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   A curve from Srf. This curve inherit the order and    M
*                      continuity of surface Srf in direction Dir. However,  M
*                      thiscurve is not on surface Srf, in general.          M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCrvFromSrf, BzrSrfCrvFromSrf, BspSrfCrvFromSrf,                      M
*   CagdCrvFromMesh, BzrSrfCrvFromMesh                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfCrvFromMesh, isoparametric curves, curve from mesh                 M
*****************************************************************************/
CagdCrvStruct *BspSrfCrvFromMesh(const CagdSrfStruct *Srf,
				 int Index,
				 CagdSrfDirType Dir)
{
    CagdCrvStruct
	*Crv = NULL;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, CrvLen,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdRType *CrvP, *SrfP;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    if (Index >= CAGD_SRF_UPT_LST_LEN(Srf) || Index < 0)
		CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

	    Index %= Srf -> ULength;
	    Crv = BspPeriodicCrvNew(CrvLen = Srf -> VLength, Srf -> VOrder,
				    Srf -> VPeriodic, Srf -> PType);
	    CAGD_GEN_COPY(Crv -> KnotVector, Srf -> VKnotVector,
			  sizeof(CagdRType) *
			      (CAGD_SRF_VPT_LST_LEN(Srf) + Srf -> VOrder));

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i] + Index * CAGD_NEXT_U(Srf);
		for (j = 0; j < CrvLen; j++) {
		    *CrvP++ = *SrfP;
		    SrfP += CAGD_NEXT_V(Srf);
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    if (Index >= CAGD_SRF_VPT_LST_LEN(Srf) || Index < 0)
		CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

	    Index %= Srf -> VLength;
	    Crv = BspPeriodicCrvNew(CrvLen = Srf -> ULength, Srf -> UOrder,
				    Srf -> UPeriodic, Srf -> PType);
	    CAGD_GEN_COPY(Crv -> KnotVector, Srf -> UKnotVector,
			  sizeof(CagdRType) *
			      (CAGD_SRF_UPT_LST_LEN(Srf) + Srf -> UOrder));

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		CrvP = Crv -> Points[i];
		SrfP = Srf -> Points[i] + Index * CAGD_NEXT_V(Srf);
		for (j = 0; j < CrvLen; j++) {
		    *CrvP++ = *SrfP;
		    SrfP += CAGD_NEXT_U(Srf);
		}
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
    return Crv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Extracts the C1 discontinuity curves from the given Bspline surface.     M
*   This routine detects potential discontinuities in the control mesh by    M
* seeking knots of Order-1 multiplicity.  Potential discontinuities that     M
* materialize as real (by examining the mesh itself along that line) are     M
* extracted as isoparametric curves.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To extract its C1 discontinuity curves.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  The C1 discontinuities as a list of isoaprametric      M
*		  curves.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfCrvFromSrf, BspKnotAllC1Discont, BspSrfIsC1DiscontAt,              M
*   BspSrfHasC1Discont						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfC1DiscontCrvs                                                      M
*****************************************************************************/
CagdCrvStruct *BspSrfC1DiscontCrvs(const CagdSrfStruct *Srf)
{
    int i, n;
    CagdRType *C1PotDisc;
    CagdCrvStruct *Crv,
	*C1DiscCrvs = NULL;

    if (!CAGD_IS_BSPLINE_SRF(Srf))
	return NULL;

    /* Examine the potential discontinuities along the U direction. */
    if ((C1PotDisc = BspKnotAllC1Discont(Srf -> UKnotVector, Srf -> UOrder,
					 Srf -> ULength, &n)) != NULL) {
	for (i = 0; i < n; i++) {
	    if (BspSrfIsC1DiscontAt(Srf, CAGD_CONST_U_DIR, C1PotDisc[i])) {
	        Crv = CagdCrvFromSrf(Srf, C1PotDisc[i], CAGD_CONST_U_DIR);
		AttrSetIntAttrib(&Crv -> Attr, "C1DiscDir", CAGD_CONST_U_DIR);
		AttrSetRealAttrib(&Crv -> Attr, "C1DiscVal", C1PotDisc[i]);
		IRIT_LIST_PUSH(Crv, C1DiscCrvs);
	    }
	}

	IritFree(C1PotDisc);
    }

    /* Examine the potential discontinuities along the V direction. */
    if ((C1PotDisc = BspKnotAllC1Discont(Srf -> VKnotVector, Srf -> VOrder,
					 Srf -> VLength, &n)) != NULL) {
	for (i = 0; i < n; i++) {
	    if (BspSrfIsC1DiscontAt(Srf, CAGD_CONST_V_DIR, C1PotDisc[i])) {
	        Crv = CagdCrvFromSrf(Srf, C1PotDisc[i], CAGD_CONST_V_DIR);
		AttrSetIntAttrib(&Crv -> Attr, "C1DiscDir", CAGD_CONST_V_DIR);
		AttrSetRealAttrib(&Crv -> Attr, "C1DiscVal", C1PotDisc[i]);
		IRIT_LIST_PUSH(Crv, C1DiscCrvs);
	    }
	}

	IritFree(C1PotDisc);
    }

    return C1DiscCrvs;  
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examines if the given Bspline surface has C^1 discontinuities.	     M
*   This routine detects potential discontinuities in the control mesh by    M
* seeking knots of Order-1 multiplicity.  Only potential discontinuities     M
* that materialize as real (by examining the mesh itself along that line)    M
* are reported as true.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To examine for C^1 discontinuity curves.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    True if Srf has C^1 discontinuities, false otherwise.      M
*		  curves.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfC1DiscontCrvs, BspKnotAllC1Discont, BspSrfIsC1DiscontAt            M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfHasC1Discont                                                       M
*****************************************************************************/
CagdBType BspSrfHasC1Discont(const CagdSrfStruct *Srf)
{
    int i, n;
    CagdRType *C1PotDisc;

    /* Examine the potential discontinuities along the U direction. */
    if ((C1PotDisc = BspKnotAllC1Discont(Srf -> UKnotVector, Srf -> UOrder,
					 Srf -> ULength, &n)) != NULL) {
	for (i = 0; i < n; i++) {
	    if (BspSrfIsC1DiscontAt(Srf, CAGD_CONST_U_DIR, C1PotDisc[i]))
	        return TRUE;
	}

	IritFree(C1PotDisc);
    }

    /* Examine the potential discontinuities along the V direction. */
    if ((C1PotDisc = BspKnotAllC1Discont(Srf -> VKnotVector, Srf -> VOrder,
					 Srf -> VLength, &n)) != NULL) {
	for (i = 0; i < n; i++) {
	    if (BspSrfIsC1DiscontAt(Srf, CAGD_CONST_V_DIR, C1PotDisc[i]))
	        return TRUE;
	}

	IritFree(C1PotDisc);
    }

    return FALSE;  
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Examines the mesh at given parametric location for a C1 discontinuity.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to examine for C1 discontinuity.                     M
*   Dir:        Paramertic direction to examine at.                          M
*   t:          Parameter value to examine at.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if Srf has a C1 discontinuity at parameter t in         M
*		direction Dir, FALSE otherwise.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfC1DiscontCrvs, BspKnotAllC1Discont, BspSrfMeshC1Continuous,        M
*   BspSrfHasC1Discont						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfIsC1DiscontAt                                                      M
*****************************************************************************/
CagdBType BspSrfIsC1DiscontAt(const CagdSrfStruct *Srf,
			      CagdSrfDirType Dir,
			      CagdRType t)
{
    CagdRType
	*KV = Dir == CAGD_CONST_U_DIR ? Srf -> UKnotVector
				      : Srf -> VKnotVector;
    int Order = Dir == CAGD_CONST_U_DIR ? Srf -> UOrder : Srf -> VOrder,
        Length = Dir == CAGD_CONST_U_DIR ? Srf -> ULength : Srf -> VLength,
	IndxFirst = BspKnotLastIndexL(KV, Length + Order, t) + 1,
	IndxLast = BspKnotFirstIndexG(KV, Length + Order, t) - 1;

    /* Make sure multiplicity is indeed at least Order - 1. */
    if (IndxLast - IndxFirst + 2 < Order)
        return FALSE;
    else if (IndxLast - IndxFirst + 2 > Order)
        return TRUE;		    /* This is, in fact, a C0 discontinuity. */

    return !BspSrfMeshC1Continuous(Srf, Dir, IndxFirst - 1);
}
