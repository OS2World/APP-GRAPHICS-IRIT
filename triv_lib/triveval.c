/******************************************************************************
* TrivEval.c - tri-variate function handling routines - evaluation routines.  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include <string.h>
#include "triv_loc.h"

IRIT_STATIC_DATA int 
    *GlblBlockLengths, *GlblBlockOrders, *GlblBlockNumOfParams,
    *GlblBlockBlockSizes;
IRIT_STATIC_DATA CagdBspBasisFuncEvalStruct
    *GlblBlockUBasisFuncs, *GlblBlockVBasisFuncs, *GlblBlockWBasisFuncs,
    *GlblBlockDUBasisFuncs, *GlblBlockDVBasisFuncs, *GlblBlockDWBasisFuncs;
IRIT_STATIC_DATA CagdPType
    *GlblBlockParams, *GlblBlockMesh;
IRIT_STATIC_DATA TrivTVBlockEvalStruct
    *GlblBlockEvals;

static void TrivTVMultEvalAux(CagdPType *Mesh,
			      int Dir,
			      int ULength,
			      int VLength,
			      int WLength,
			      int UOrder,
			      int VOrder,
			      int WOrder,
			      CagdBspBasisFuncEvalStruct *UBasisFuncs,
			      CagdBspBasisFuncEvalStruct *VBasisFuncs,
			      CagdBspBasisFuncEvalStruct *WBasisFuncs,
			      int NumOfParams,
			      IrtRType *Results,
			      int ResultsStep);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the given tensor product trivariate at a given point, by         M
* extracting an isoparamteric surface along w and evaluating (u,v) in it.    M
*									     V
*           +-----------------------+					     V
*       W  /                       /|					     V
*      /  /                       / |					     V
*     /  /	U -->		 /  |  Orientation of			     V
*       +-----------------------+   |  control tri-variate mesh.	     V
*   V | |P0                 Pi-1|   +					     V
*     v	|Pi                P2i-1|  /					     V
*	|			| /					     V
*	|Pn-i		    Pn-1|/      				     V
*	+-----------------------+					     V
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To evaluate at given (u, v, w) parametric location.            M
*   u, v, w:  Parametric location to evaluate TV at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the trivariate's point type. If for example trivariate  M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVEval, evaluation, trivariates                                      M
*****************************************************************************/
CagdRType *TrivTVEval(const TrivTVStruct *TV,
		      CagdRType u,
		      CagdRType v,
		      CagdRType w)
{
    IRIT_STATIC_DATA CagdSrfStruct
	*IsoSubSrf = NULL;
    CagdRType *Pt, *WBasisFunc, UMin, UMax, VMin, VMax, WMin, WMax;
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int k, UIndexFirst, VIndexFirst, WIndexFirst,
	UOrder = TV -> UOrder,
	VOrder = TV -> VOrder,
	WOrder = TV -> WOrder,
        ULength = TV -> ULength,
        VLength = TV -> VLength,
        WLength = TV -> WLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);

    /* The code below is optimized for Bspline trivariates. For Bezier    */
    /* trivariate we have to process the entire data any way.             */
    if (TRIV_IS_BEZIER_TV(TV))
        return TrivTVEval2(TV, u, v, w);

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);
    if (u < UMin - IRIT_EPS || u > UMax + IRIT_EPS ||
        v < VMin - IRIT_EPS || v > VMax + IRIT_EPS ||
        w < WMin - IRIT_EPS || w > WMax + IRIT_EPS)
        TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);

    if (u > UMax - IRIT_UEPS * 2)
	u = UMax - IRIT_UEPS * 2;
    else if (u < UMin)
        u = UMin;
    if (v > VMax - IRIT_UEPS * 2)
	v = VMax - IRIT_UEPS * 2;
    else if (v < VMin)
        v = VMin;
    if (w > WMax - IRIT_UEPS * 2)
	w = WMax - IRIT_UEPS * 2;
    else if (w < WMin)
        w = WMin;

    UIndexFirst = BspKnotLastIndexLE(TV -> UKnotVector, ULength + UOrder, u) -
								  (UOrder - 1);
    VIndexFirst = BspKnotLastIndexLE(TV -> VKnotVector, VLength + VOrder, v) -
								  (VOrder - 1);
    WBasisFunc = BspCrvCoxDeBoorBasis(TV -> WKnotVector, WOrder,
				      WLength, TV -> WPeriodic,
				      w, &WIndexFirst);

    if (IsoSubSrf != NULL &&
	(TV -> PType != IsoSubSrf -> PType ||
	 UOrder != IsoSubSrf -> UOrder ||
	 VOrder != IsoSubSrf -> VOrder)) {
	/* The cached surface is not the proper type - release it. */
	CagdSrfFree(IsoSubSrf);
	IsoSubSrf = NULL;
    }
    if (IsoSubSrf == NULL) {
        IsoSubSrf = BspSrfNew(UOrder, VOrder, UOrder, VOrder, TV -> PType);
    }
    CAGD_GEN_COPY(IsoSubSrf -> UKnotVector,
		  &TV -> UKnotVector[UIndexFirst],
		  sizeof(CagdRType) * UOrder * 2);
    CAGD_GEN_COPY(IsoSubSrf -> VKnotVector,
		  &TV -> VKnotVector[VIndexFirst],
		  sizeof(CagdRType) * VOrder * 2);

    /* Clear IsoSubSrf cache if has any, as we change its control points. */
    CagdSrfFreeCache(IsoSubSrf);
 
    for (k = 0; k < UOrder; k++, UIndexFirst++) {
	int n,
	    VIndexFirstTmp = VIndexFirst;

	for (n = 0; n < VOrder; n++, VIndexFirstTmp++) {
	    int l;

	    for (l = IsNotRational; l <= MaxCoord; l++) {
		int i;
		CagdRType
		    *TVP = &TV -> Points[l][TRIV_MESH_UVW(TV,
							  UIndexFirst,
							  VIndexFirstTmp,
							  WIndexFirst)],
		    *SrfP = &IsoSubSrf -> Points[l][CAGD_MESH_UV(IsoSubSrf,
								 k, n)];

		*SrfP = 0.0;
		for (i = 0; i < WOrder; i++) {
		    *SrfP += WBasisFunc[i] * *TVP;
		    TVP += TRIV_NEXT_W(TV);
		}
	    }
	}
    }

    Pt = BspSrfEvalAtParam(IsoSubSrf, u, v);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function is the same as TrivTVEval2 above. Cleaner, but much less   M
* efficient.							             M
*   Evaluates the given tensor product trivariate at a given point, by       M
* extracting an isoparamteric surface along w and evaluating (u, v) in it.   M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To evaluate at given (u, v, w) parametric location.            M
*   u, v, w:  Parametric location to evaluate TV at.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector holding all the coefficients of all components    M
*                 of the trivariate's point type. If for example trivariate  M
*                 point type is P2, the W, X, and Y will be saved in the     M
*                 first three locations of the returned vector. The first    M
*                 location (index 0) of the returned vector is reserved for  M
*                 the rational coefficient W and XYZ always starts at second M
*                 location of the returned vector (index 1).                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVEval2, evaluation, trivariates                                     M
*****************************************************************************/
CagdRType *TrivTVEval2(const TrivTVStruct *TV,
		       CagdRType u,
		       CagdRType v,
		       CagdRType w)
{
    CagdRType *Pt;
    CagdSrfStruct
	*IsoSrf = TrivSrfFromTV(TV, u, TRIV_CONST_U_DIR, FALSE);

    if (!TrivParamsInDomain(TV, u, v, w)) {
	TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);
	return NULL;
    }

    Pt = CagdSrfEval(IsoSrf, v, w);

    CagdSrfFree(IsoSrf);

    return Pt;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract an isoparametric surface out of the given tensor product	     M
* trivariate.         							     M
*   Operations should favor the CONST_W_DIR, in which the extraction is	     M
* somewhat faster, if it is possible.					     M
*   surfaces that are on the boundary are reoriented so their normals are    M
* pointing into the trivariate, if OrientBoundary is TRUE.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To extract an isoparametric surface from at parameter value t  M
*	      in direction Dir.						     M
*   t:        Parameter value at which to extract the isosurface.            M
*   Dir:      Direction of isosurface extraction. Either U or V or W.        M
*   OrientBoundary:  TRUE to reorient boundary surafces to point with their  M
*		     normals into the trivariate.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A bivariate surface which is an isosurface of TV.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBndrySrfsFromTV                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivSrfFromTV, trivariates                                               M
*****************************************************************************/
CagdSrfStruct *TrivSrfFromTV(const TrivTVStruct *TV,
			     CagdRType t,
			     TrivTVDirType Dir,
			     int OrientBoundary)
{
    CagdSrfStruct
	*Srf = NULL;
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, j, k, SrfLen,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    CagdRType *SrfP, *TVP, UMin, UMax, VMin, VMax, WMin, WMax;
    CagdVecStruct
	*OrientN = NULL;
    CagdPType OrientEvalPt, OrientEvalPt2;

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    if (OrientBoundary) {
	OrientEvalPt[0] = (UMin + UMax) * 0.5;
	OrientEvalPt[1] = (VMin + VMax) * 0.5;
	OrientEvalPt[2] = (WMin + WMax) * 0.5;
	IRIT_PT_COPY(OrientEvalPt2, OrientEvalPt);

	switch (Dir) {
	    case TRIV_CONST_U_DIR:
	        if (IRIT_APX_EQ(t, UMin)) {
		    OrientEvalPt[0] = t;
		    OrientEvalPt2[0] = t + IRIT_EPS;
		}
		else if (IRIT_APX_EQ(t, UMax)) {
		    OrientEvalPt[0] = t;
		    OrientEvalPt2[0] = t - IRIT_EPS;
		}
		else
		    OrientBoundary = FALSE; /* Not a boundary. */
	        break;
	    case TRIV_CONST_V_DIR:
	        if (IRIT_APX_EQ(t, VMin)) {
		    OrientEvalPt[1] = t;
		    OrientEvalPt2[1] = t + IRIT_EPS;
		}
		else if (IRIT_APX_EQ(t, VMax)) {
		    OrientEvalPt[1] = t;
		    OrientEvalPt2[1] = t - IRIT_EPS;
		}
		else
		    OrientBoundary = FALSE; /* Not a boundary. */
	        break;
	    case TRIV_CONST_W_DIR:
	        if (IRIT_APX_EQ(t, WMin)) {
		    OrientEvalPt[2] = t;
		    OrientEvalPt2[2] = t + IRIT_EPS;
		}
		else if (IRIT_APX_EQ(t, WMax)) {
		    OrientEvalPt[2] = t;
		    OrientEvalPt2[2] = t - IRIT_EPS;
		}
		else
		    OrientBoundary = FALSE; /* Not a boundary. */
	        break;
	    default:
	        assert(0);
	}
    }

    if (!TrivParamInDomain(TV, t, Dir)) {
	TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);
	return NULL;
    }

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
		Srf = BspPeriodicSrfNew(TV -> VLength, TV -> WLength,
					TV -> VOrder, TV -> WOrder,
					TV -> VPeriodic, TV -> WPeriodic,
					TV -> PType);
		CAGD_GEN_COPY(Srf -> UKnotVector, TV -> VKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) +
						   TV -> VOrder));
		CAGD_GEN_COPY(Srf -> VKnotVector, TV -> WKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) +
						   TV -> WOrder));
	    }
	    else {
		Srf = BzrSrfNew(TV -> VLength, TV -> WLength, TV -> PType);
	    }
	    SrfLen = Srf -> ULength * Srf -> VLength;

	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	        int IndexFirst;
	        CagdRType
		    *BasisFuncs = BspCrvCoxDeBoorBasis(TV -> UKnotVector,
						       TV -> UOrder,
						       TV -> ULength,
						       TV -> UPeriodic,
						       t, &IndexFirst);

		for (i = IsNotRational; i <= MaxCoord; i++) {
		    SrfP = Srf -> Points[i];
		    TVP = TV -> Points[i];
		    for (j = 0; j < SrfLen; j++, SrfP++) {
		        BSP_CRV_EVAL_VEC_AT_PARAM(SrfP, TVP, TRIV_NEXT_U(TV),
						  TV -> UOrder, TV -> ULength,
						  t, BasisFuncs, IndexFirst);

			TVP += TRIV_NEXT_V(TV);
		    }
		}
	    }
	    else {
		for (i = IsNotRational; i <= MaxCoord; i++) {
		    SrfP = Srf -> Points[i];
		    TVP = TV -> Points[i];
		    for (j = 0; j < SrfLen; j++) {
			*SrfP++ = BzrCrvEvalVecAtParam(TVP, TRIV_NEXT_U(TV),
						       TV -> ULength, t);
			TVP += TRIV_NEXT_V(TV);
		    }
		}
	    }

	    if (OrientBoundary)
		OrientN = CagdSrfNormal(Srf, (VMin + VMax) * 0.5,
					     (WMin + WMax) * 0.5, FALSE);
	    break;
	case TRIV_CONST_V_DIR:
	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
		Srf = BspPeriodicSrfNew(TV -> ULength, TV -> WLength,
					TV -> UOrder, TV -> WOrder,
					TV -> UPeriodic, TV -> WPeriodic,
					TV -> PType);
		CAGD_GEN_COPY(Srf -> UKnotVector, TV -> UKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) +
						   TV -> UOrder));
		CAGD_GEN_COPY(Srf -> VKnotVector, TV -> WKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) +
						   TV -> WOrder));
	    }
	    else {
		Srf = BzrSrfNew(TV -> ULength, TV -> WLength, TV -> PType);
	    }
	    SrfLen = Srf -> ULength * Srf -> VLength;

	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	        int IndexFirst;
	        CagdRType
		    *BasisFuncs = BspCrvCoxDeBoorBasis(TV -> VKnotVector,
						       TV -> VOrder,
						       TV -> VLength,
						       TV -> VPeriodic,
						       t, &IndexFirst);

		for (k = 0, i = IsNotRational; i <= MaxCoord; i++) {
		    SrfP = Srf -> Points[i];
		    TVP = TV -> Points[i];
		    for (j = 0; j < SrfLen; j++, SrfP++) {
		        BSP_CRV_EVAL_VEC_AT_PARAM(SrfP, TVP, TRIV_NEXT_V(TV),
						  TV -> VOrder, TV -> VLength,
						  t, BasisFuncs, IndexFirst);

			TVP += TRIV_NEXT_U(TV);
			if (++k == TV -> ULength) {
			    TVP += TRIV_NEXT_W(TV) - TRIV_NEXT_U(TV) * k;
			    k = 0;
			}
		    }
		}
	    }
	    else {
		for (k = 0, i = IsNotRational; i <= MaxCoord; i++) {
		    SrfP = Srf -> Points[i];
		    TVP = TV -> Points[i];
		    for (j = 0; j < SrfLen; j++) {
			*SrfP++ = BzrCrvEvalVecAtParam(TVP, TRIV_NEXT_V(TV),
						       TV -> VLength, t);
			TVP += TRIV_NEXT_U(TV);
			if (++k == TV -> ULength) {
			    TVP += TRIV_NEXT_W(TV) - TRIV_NEXT_U(TV) * k;
			    k = 0;
			}
		    }
		}
	    }

	    if (OrientBoundary)
		OrientN = CagdSrfNormal(Srf, (UMin + UMax) * 0.5,
				             (WMin + WMax) * 0.5, FALSE);

	    break;
	case TRIV_CONST_W_DIR:
	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
		Srf = BspPeriodicSrfNew(TV -> ULength, TV -> VLength,
					TV -> UOrder, TV -> VOrder,
					TV -> UPeriodic, TV -> VPeriodic,
					TV -> PType);
		CAGD_GEN_COPY(Srf -> UKnotVector, TV -> UKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) +
						   TV -> UOrder));
		CAGD_GEN_COPY(Srf -> VKnotVector, TV -> VKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) +
						   TV -> VOrder));
	    }
	    else {
		Srf = BzrSrfNew(TV -> ULength, TV -> VLength, TV -> PType);
	    }
	    SrfLen = Srf -> ULength * Srf -> VLength;

	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	        int IndexFirst;
	        CagdRType
		    *BasisFuncs = BspCrvCoxDeBoorBasis(TV -> WKnotVector,
						       TV -> WOrder,
						       TV -> WLength,
						       TV -> WPeriodic,
						       t, &IndexFirst);

		for (i = IsNotRational; i <= MaxCoord; i++) {
		    SrfP = Srf -> Points[i];
		    TVP = TV -> Points[i];
		    for (j = 0; j < SrfLen; j++, SrfP++) {
		        BSP_CRV_EVAL_VEC_AT_PARAM(SrfP, TVP, TRIV_NEXT_W(TV),
						  TV -> WOrder, TV -> WLength,
						  t, BasisFuncs, IndexFirst);

			TVP += TRIV_NEXT_U(TV);
		    }
		}
	    }
	    else {
		for (i = IsNotRational; i <= MaxCoord; i++) {
		    SrfP = Srf -> Points[i];
		    TVP = TV -> Points[i];
		    for (j = 0; j < SrfLen; j++) {
			*SrfP++ = BzrCrvEvalVecAtParam(TVP, TRIV_NEXT_W(TV),
						       TV -> WLength, t);
			TVP += TRIV_NEXT_U(TV);
		    }
		}
	    }

	    if (OrientBoundary)
		OrientN = CagdSrfNormal(Srf, (UMin + UMax) * 0.5,
				             (VMin + VMax) * 0.5, FALSE);
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);
	    break;
    }

    if (OrientBoundary) {
        CagdRType *R;
	CagdPType Pt1, Pt2;
	CagdVType N;

	R = TrivTVEval(TV, OrientEvalPt[0], OrientEvalPt[1], OrientEvalPt[2]);
	CagdCoerceToE3(Pt1, &R, -1, TV -> PType);
	R = TrivTVEval(TV, OrientEvalPt2[0], OrientEvalPt2[1], OrientEvalPt2[2]);
	CagdCoerceToE3(Pt2, &R, -1, TV -> PType);
	IRIT_VEC_SUB(N, Pt2, Pt1);

	if (IRIT_DOT_PROD(OrientN -> Vec, N) < 0) {
	    CagdSrfStruct
		*TSrf = CagdSrfReverse(Srf);

	    CagdSrfFree(Srf);
	    Srf = TSrf;
	}
    }

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extracts the six boundary surfaces of the given tensor product trivariate. M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To extract the six boundary surfaces from.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct **:  A pointer to a static vector of six surface pointers, M
*		       representing the six boundaries of the trivariate TV  M
*		       in order of UMin, UMax, VMin, VMax, WMin, WMax.       M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivSrfFromTV                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBndrySrfsFromTV, trivariates                                         M
*****************************************************************************/
CagdSrfStruct **TrivBndrySrfsFromTV(const TrivTVStruct *TV)
{
    static CagdSrfStruct *Srfs[6];
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    Srfs[0] = TrivSrfFromTV(TV, UMin, TRIV_CONST_U_DIR, TRUE);
    Srfs[1] = TrivSrfFromTV(TV, UMax, TRIV_CONST_U_DIR, TRUE);
    Srfs[2] = TrivSrfFromTV(TV, VMin, TRIV_CONST_V_DIR, TRUE);
    Srfs[3] = TrivSrfFromTV(TV, VMax, TRIV_CONST_V_DIR, TRUE);
    Srfs[4] = TrivSrfFromTV(TV, WMin, TRIV_CONST_W_DIR, TRUE);
    Srfs[5] = TrivSrfFromTV(TV, WMax, TRIV_CONST_W_DIR, TRUE);

    return Srfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Extract a bivariate surface out of the given trivariate's mesh.	     M
*   The provided (zero based) Index specifies which Index to extract.        M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        Trivariate to extract a bivariate surface out of its mesh.    M
*   Index:     Index of row/column/level of TV's mesh in direction Dir.      M
*   Dir:       Direction of isosurface extraction. Either U or V or W.       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A bivariate surface which was extracted from TV's     M
*                      Mesh. This surface is not necessarily on TV.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivSrfFromMesh, trivariates                                             M
*****************************************************************************/
CagdSrfStruct *TrivSrfFromMesh(const TrivTVStruct *TV,
			       int Index,
			       TrivTVDirType Dir)
{
    CagdSrfStruct
	*Srf = NULL;
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, j, k, SrfLen,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    CagdRType *SrfP, *TVP;

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    if (Index >= TV -> ULength || Index < 0)
		TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
		Srf = BspPeriodicSrfNew(TV -> VLength, TV -> WLength,
					TV -> VOrder, TV -> WOrder,
					TV -> VPeriodic, TV -> WPeriodic,
					TV -> PType);
		CAGD_GEN_COPY(Srf -> UKnotVector, TV -> VKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) +
						   TV -> VOrder));
		CAGD_GEN_COPY(Srf -> VKnotVector, TV -> WKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) +
						   TV -> WOrder));
	    }
	    else {
		Srf = BzrSrfNew(TV -> VLength, TV -> WLength, TV -> PType);
	    }
	    SrfLen = Srf -> ULength * Srf -> VLength;

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		SrfP = Srf -> Points[i];
		TVP = TV -> Points[i] + Index * TRIV_NEXT_U(TV);
		for (j = 0; j < SrfLen; j++) {
		    *SrfP++ = *TVP;
		    TVP += TRIV_NEXT_V(TV);
		}
	    }
	    break;
	case TRIV_CONST_V_DIR:
	    if (Index >= TV -> VLength || Index < 0)
		TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
		Srf = BspPeriodicSrfNew(TV -> ULength, TV -> WLength,
					TV -> UOrder, TV -> WOrder,
					TV -> UPeriodic, TV -> WPeriodic,
					TV -> PType);
		CAGD_GEN_COPY(Srf -> UKnotVector, TV -> UKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) +
						   TV -> UOrder));
		CAGD_GEN_COPY(Srf -> VKnotVector, TV -> WKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_WPT_LST_LEN(TV) +
						   TV -> WOrder));
	    }
	    else {
		Srf = BzrSrfNew(TV -> ULength, TV -> WLength, TV -> PType);
	    }
	    SrfLen = Srf -> ULength * Srf -> VLength;

	    for (k = 0, i = IsNotRational; i <= MaxCoord; i++) {
		SrfP = Srf -> Points[i];
		TVP = TV -> Points[i]+ Index * TRIV_NEXT_V(TV);
		for (j = 0; j < SrfLen; j++) {
		    *SrfP++ = *TVP;
		    TVP += TRIV_NEXT_U(TV);
		    if (++k == TV -> ULength) {
			TVP += TRIV_NEXT_W(TV) - TRIV_NEXT_U(TV) * k;
			k = 0;
		    }
		}
	    }
	    break;
	case TRIV_CONST_W_DIR:
	    if (Index >= TV -> WLength || Index < 0)
		TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

	    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
		Srf = BspPeriodicSrfNew(TV -> ULength, TV -> VLength,
					TV -> UOrder, TV -> VOrder,
					TV -> UPeriodic, TV -> VPeriodic,
					TV -> PType);
		CAGD_GEN_COPY(Srf -> UKnotVector, TV -> UKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_UPT_LST_LEN(TV) +
						   TV -> UOrder));
		CAGD_GEN_COPY(Srf -> VKnotVector, TV -> VKnotVector,
			      sizeof(CagdRType) * (TRIV_TV_VPT_LST_LEN(TV) +
						   TV -> VOrder));
	    }
	    else {
		Srf = BzrSrfNew(TV -> ULength, TV -> VLength, TV -> PType);
	    }
	    SrfLen = Srf -> ULength * Srf -> VLength;

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		SrfP = Srf -> Points[i];
		TVP = TV -> Points[i]+ Index * TRIV_NEXT_W(TV);
		for (j = 0; j < SrfLen; j++) {
		    *SrfP++ = *TVP;
		    TVP += TRIV_NEXT_U(TV);
		}
	    }
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);
	    break;
    }
    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Substitute a bivariate surface into a given trivariate's mesh.	     M
*   The provided (zero based) Index specifies which Index to extract.        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to substitute into the trivariate TV.		     M
*   Index:     Index of row/column/level of TV's mesh in direction Dir.      M
*   Dir:       Direction of isosurface extraction. Either U or V or W.       M
*   TV:        Trivariate to substitute a bivariate surface into its mesh.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void	                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivSrfToMesh, trivariates                                               M
*****************************************************************************/
void TrivSrfToMesh(const CagdSrfStruct *Srf,
		   int Index,
		   TrivTVDirType Dir,
		   TrivTVStruct *TV)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, j, k,
	SrfLen = Srf -> ULength * Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    CagdRType *SrfP, *TVP;

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    if (Index >= TV -> ULength || Index < 0)
		TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		SrfP = Srf -> Points[i];
		TVP = TV -> Points[i] + Index * TRIV_NEXT_U(TV);
		for (j = 0; j < SrfLen; j++) {
		    *TVP = *SrfP++;
		    TVP += TRIV_NEXT_V(TV);
		}
	    }
	    break;
	case TRIV_CONST_V_DIR:
	    if (Index >= TV -> VLength || Index < 0)
		TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

	    for (k = 0, i = IsNotRational; i <= MaxCoord; i++) {
		SrfP = Srf -> Points[i];
		TVP = TV -> Points[i]+ Index * TRIV_NEXT_V(TV);
		for (j = 0; j < SrfLen; j++) {
		    *TVP = *SrfP++;
		    TVP += TRIV_NEXT_U(TV);
		    if (++k == TV -> ULength) {
			TVP += TRIV_NEXT_W(TV) - TRIV_NEXT_U(TV) * k;
			k = 0;
		    }
		}
	    }
	    break;
	case TRIV_CONST_W_DIR:
	    if (Index >= TV -> WLength || Index < 0)
		TRIV_FATAL_ERROR(TRIV_ERR_INDEX_NOT_IN_MESH);

	    for (i = IsNotRational; i <= MaxCoord; i++) {
		SrfP = Srf -> Points[i];
		TVP = TV -> Points[i]+ Index * TRIV_NEXT_W(TV);
		for (j = 0; j < SrfLen; j++) {
		    *TVP = *SrfP++;
		    TVP += TRIV_NEXT_U(TV);
		}
	    }
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes multiple evaluations of the given trivariate space, as          M
* prescribed by U/V/WKnotVectors and U/V/WOrders, U/V/WLengths, and Mesh,    M
* at the requested NumOfParams parameter values, Params.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   UKnotVector:  U Knot sequence defining the spline space in U.            M
*   VKnotVector:  V Knot sequence defining the spline space in V.            M
*   WKnotVector:  W Knot sequence defining the spline space in W.            M
*   ULength:      Length of Mesh in the U direction.			     M
*   VLength:      Length of Mesh in the V direction.			     M
*   WLength:      Length of Mesh in the W direction.			     M
*   UOrder:       Of the spline space in the U Direction.		     M
*   VOrder:       Of the spline space in the V Direction.		     M
*   WOrder:       Of the spline space in the W Direction.		     M
*   Mesh:         ULength * VLength * WLength control points, in R^3.	     M
*   Params:       At which to evaluate and compute the volume functions.     M
*   NumOfParams:  Size of Params vector.                                     M
*   RetSize:	  Number of values returned per evaluation. 3 for position   M
*		  9 for 1st derivative, etc.		                     M
*   EvalType:     Type of evaluation requested:  value (position), 1st       M
*		  derivative.			                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  A vector of size NumOfParams * RetSize, holding the	     M
*		  NumOfParams evaluation results, each of size RetSize.      M
*		    For position evaluation, RetSize = 3 and XYZ are	     M
*		  returned. For 1st derivatives, RetSize = 9 and the	     M
*		  Jacobian is returned, with dX/du, dX/dV, dX/dw first.      M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVMultEval                                                           M
*****************************************************************************/
CagdRType *TrivTVMultEval(CagdRType *UKnotVector,
			  CagdRType *VKnotVector,
			  CagdRType *WKnotVector,
			  int ULength,
			  int VLength,
			  int WLength,
			  int UOrder,
			  int VOrder,
			  int WOrder,
			  CagdPType *Mesh,
			  CagdPType *Params,
			  int NumOfParams,
			  int *RetSize,
			  CagdBspBasisFuncMultEvalType EvalType)
{
    int i;
    IrtRType *RetVec,
	*ScalarParams = (IrtRType *) IritMalloc(sizeof(IrtRType) *
								NumOfParams);
    CagdBspBasisFuncEvalStruct *UBasisFuncs, *VBasisFuncs, *WBasisFuncs,
				*DUBasisFuncs, *DVBasisFuncs, *DWBasisFuncs;

    /* Evaluate the basis functions in the U/V/W/ directions at Params. */
    for (i = 0; i < NumOfParams; i++)
        ScalarParams[i] = Params[i][0];
    UBasisFuncs = BspBasisFuncMultEval(UKnotVector, ULength + UOrder, UOrder,
				       FALSE, ScalarParams, NumOfParams,
				       CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);

    for (i = 0; i < NumOfParams; i++)
        ScalarParams[i] = Params[i][1];
    VBasisFuncs = BspBasisFuncMultEval(VKnotVector, VLength + VOrder, VOrder,
				       FALSE, ScalarParams, NumOfParams,
				       CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);

    for (i = 0; i < NumOfParams; i++)
        ScalarParams[i] = Params[i][2];
    WBasisFuncs = BspBasisFuncMultEval(WKnotVector, WLength + WOrder, WOrder,
				       FALSE, ScalarParams, NumOfParams,
				       CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);


    switch (EvalType) {
        case CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE:
	    *RetSize = 3;
	    RetVec = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumOfParams
							      * *RetSize);
	    for (i = 0; i < 3; i++)
	        TrivTVMultEvalAux(Mesh, i, ULength, VLength, WLength,
				  UOrder, VOrder, WOrder,
				  UBasisFuncs, VBasisFuncs, WBasisFuncs,
				  NumOfParams, &RetVec[i], *RetSize);
	    break;
        case CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST:
	    *RetSize = 9;
	    RetVec = (IrtRType *) IritMalloc(sizeof(IrtRType) * NumOfParams
							      * *RetSize);
	    for (i = 0; i < NumOfParams; i++)
	        ScalarParams[i] = Params[i][0];
	    DUBasisFuncs = BspBasisFuncMultEval(UKnotVector, ULength + UOrder,
				      UOrder, FALSE, ScalarParams, NumOfParams,
				      CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

	    for (i = 0; i < NumOfParams; i++)
	        ScalarParams[i] = Params[i][1];
	    DVBasisFuncs = BspBasisFuncMultEval(VKnotVector, VLength + VOrder,
				      VOrder, FALSE, ScalarParams, NumOfParams,
				      CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

	    for (i = 0; i < NumOfParams; i++)
	        ScalarParams[i] = Params[i][2];
	    DWBasisFuncs = BspBasisFuncMultEval(WKnotVector, WLength + WOrder,
				      WOrder, FALSE, ScalarParams, NumOfParams,
				      CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

	    for (i = 0; i < 3; i++)
	        TrivTVMultEvalAux(Mesh, i, ULength, VLength, WLength,
				  UOrder, VOrder, WOrder,
				  DUBasisFuncs, VBasisFuncs, WBasisFuncs,
				  NumOfParams, &RetVec[i], *RetSize);
	    for (i = 0; i < 3; i++)
	        TrivTVMultEvalAux(Mesh, i, ULength, VLength, WLength,
				  UOrder, VOrder, WOrder,
				  UBasisFuncs, DVBasisFuncs, WBasisFuncs,
				  NumOfParams, &RetVec[i + 3], *RetSize);
	    for (i = 0; i < 3; i++)
	        TrivTVMultEvalAux(Mesh, i, ULength, VLength, WLength,
				  UOrder, VOrder, WOrder,
				  UBasisFuncs, VBasisFuncs, DWBasisFuncs,
				  NumOfParams, &RetVec[i + 6], *RetSize);

	    BspBasisFuncMultEvalFree(DUBasisFuncs, NumOfParams);
	    BspBasisFuncMultEvalFree(DVBasisFuncs, NumOfParams);
	    BspBasisFuncMultEvalFree(DWBasisFuncs, NumOfParams);
	    break;
	default:
	    RetVec = NULL;
	    TRIV_FATAL_ERROR(TRIV_ERR_UNSUPPORT_DERIV);
    }

    IritFree(ScalarParams);

    BspBasisFuncMultEvalFree(UBasisFuncs, NumOfParams);
    BspBasisFuncMultEvalFree(VBasisFuncs, NumOfParams);
    BspBasisFuncMultEvalFree(WBasisFuncs, NumOfParams);

    return RetVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   An auxiliary function of TrivTVMultEval to blend U, V, W basis functions *
* with the Mesh, creating NumOfParams scalar results.  Results are placed in *
* the preallocated Results vectors, with step ResultsStep.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Mesh:        ULength * VLength * WLength control points, in R^3.	     *
*   Dir:         0, 1, 2 denoting X, Y, Z coefficients to use in Mesh..      *
*   ULength:     Length of Mesh in the U direction.			     *
*   VLength:     Length of Mesh in the V direction.			     *
*   WLength:     Length of Mesh in the W direction.			     *
*   UOrder:      Of the spline space in the U Direction.		     *
*   VOrder:      Of the spline space in the V Direction.		     *
*   WOrder:      Of the spline space in the W Direction.		     *
*   UBasisFuncs: Evaluated basis functions, in U direction, at NumOfParams.  *
*   VBasisFuncs: Evaluated basis functions, in V direction, at NumOfParams.  *
*   WBasisFuncs: Evaluated basis functions, in W direction, at NumOfParams.  *
*   NumOfParams: Size of basis function evaluated vectors, U/V/WBasisFuncs.  *
*   Results:     Returned evaluation vector.  Preallocated by caller.        *
*   ResultsStep: Stride to make between results.                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void TrivTVMultEvalAux(CagdPType *Mesh,
			      int Dir,
			      int ULength,
			      int VLength,
			      int WLength,
			      int UOrder,
			      int VOrder,
			      int WOrder,
			      CagdBspBasisFuncEvalStruct *UBasisFuncs,
			      CagdBspBasisFuncEvalStruct *VBasisFuncs,
			      CagdBspBasisFuncEvalStruct *WBasisFuncs,
			      int NumOfParams,
			      IrtRType *Results,
			      int ResultsStep)
{
    int l, i, j, k,
	UVLength = ULength * VLength;

    for (l = 0; l < NumOfParams; l++) {
        int Base = WBasisFuncs[l].FirstBasisFuncIndex * UVLength +
	           VBasisFuncs[l].FirstBasisFuncIndex * ULength +
		   UBasisFuncs[l].FirstBasisFuncIndex;
	CagdRType
	    *U = UBasisFuncs[l].BasisFuncsVals;

        *Results = 0.0;

        for (k = 0; k < WOrder; k++) {
	    int KStep = k * UVLength + Base;

	    for (j = 0; j < VOrder; j++) {
	        int JKStep = j * ULength + KStep;
		CagdRType
		    VW = VBasisFuncs[l].BasisFuncsVals[j] *
			 WBasisFuncs[l].BasisFuncsVals[k];

		for (i = 0; i < UOrder; i++)
		    *Results += Mesh[i + JKStep][Dir] * U[i] * VW;
	    }
	}

	Results += ResultsStep;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initialize the computation of evaluations of blocks of parameter values  M
* for the given trivariate space, as prescribed by U/V/WKnotVectors and      M
* Orders, Lengths, and BlockSizes, at the requested NumOfParams              M
* parameter values, Params.  All parameters below should stay valid for the  M
* duration of the block evaluation.                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   UKnotVector:  U Knot sequence defining the spline space in U.            M
*   VKnotVector:  V Knot sequence defining the spline space in V.            M
*   WKnotVector:  W Knot sequence defining the spline space in W.            M
*   Lengths:      Lengths of Mesh in the U,V,W directions.		     M
*   Orders:       Of the spline space in the U,V,W Directions.		     M
*   BlockSizes:   Of the evaluation block sizes in U,V,W Directions.	     M
*   Params:       At which to evaluate and compute the volume functions.     M
*		  This vectror is of size IRIT_MAX(NumOfparams[0],[1],[2]).  M
*   NumOfParams:  Size of Params vector, in U, V, W.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVBlockEvalSetMesh, TrivTVBlockEvalOnce, TrivTVBlockEvalDone         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBlockEvalInit                                                      M
*****************************************************************************/
void TrivTVBlockEvalInit(CagdRType *UKnotVector,
			 CagdRType *VKnotVector,
			 CagdRType *WKnotVector,
			 int Lengths[3],
			 int Orders[3],
			 int BlockSizes[3],
			 CagdPType *Params,
			 int NumOfParams[3])
{
    int i;
    IrtRType
	*ScalarParams = (IrtRType *) IritMalloc(sizeof(IrtRType) *
		    IRIT_MAX(IRIT_MAX(NumOfParams[0], NumOfParams[1]),
			     NumOfParams[2]));

    GlblBlockLengths = Lengths;
    GlblBlockOrders = Orders;
    GlblBlockBlockSizes = BlockSizes;
    GlblBlockNumOfParams = NumOfParams;
    GlblBlockParams = Params;

    GlblBlockEvals = (TrivTVBlockEvalStruct *)
	IritMalloc(sizeof(TrivTVBlockEvalStruct)
			   * BlockSizes[0] * BlockSizes[1] * BlockSizes[2]);

    /* Evaluate the basis functions in the U/V/W/ directions at Params. */
    for (i = 0; i < NumOfParams[0]; i++)
        ScalarParams[i] = Params[i][0];
    GlblBlockUBasisFuncs = BspBasisFuncMultEval(UKnotVector,
				Lengths[0] + Orders[0],
				Orders[0], FALSE, ScalarParams, NumOfParams[0],
				CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    GlblBlockDUBasisFuncs = BspBasisFuncMultEval(UKnotVector,
				Lengths[0] + Orders[0],
				Orders[0], FALSE, ScalarParams, NumOfParams[0],
				CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

    for (i = 0; i < NumOfParams[1]; i++)
        ScalarParams[i] = Params[i][1];
    GlblBlockVBasisFuncs = BspBasisFuncMultEval(VKnotVector,
			        Lengths[1] + Orders[1],
				Orders[1], FALSE, ScalarParams, NumOfParams[1],
				CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    GlblBlockDVBasisFuncs = BspBasisFuncMultEval(VKnotVector,
				Lengths[1] + Orders[1],
				Orders[1], FALSE, ScalarParams, NumOfParams[1],
				CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

    for (i = 0; i < NumOfParams[2]; i++)
        ScalarParams[i] = Params[i][2];
    GlblBlockWBasisFuncs = BspBasisFuncMultEval(WKnotVector,
				Lengths[2] + Orders[2],
				Orders[2], FALSE, ScalarParams, NumOfParams[2],
				CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);
    GlblBlockDWBasisFuncs = BspBasisFuncMultEval(WKnotVector,
				Lengths[2] + Orders[2],
				Orders[2], FALSE, ScalarParams, NumOfParams[2],
				CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

    IritFree(ScalarParams);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the current mesh of size Lengths[1] * Lengths[2] * Lengths[3] (see  M
* TrivTVBlockEvalInit) to use in the block evaluation.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mesh:         Provide the current mesh.                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVBlockEvalInit, TrivTVBlockEvalOnce, TrivTVBlockEvalDone            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBlockEvalSetMesh                                                   M
*****************************************************************************/
void TrivTVBlockEvalSetMesh(CagdPType *Mesh)
{
    GlblBlockMesh = Mesh;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes evaluations of one block of parameter values for the given	     M
* trivariate space, as prescribed by i, j, k.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   i, j, k:  Index of current block evaluation.  This triple index is       M
*	      between 0 and NumOfParams[l]/BlockSize[l], in each dimension.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVBlockEvalStruct *:  Evaluated block.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVBlockEvalInit, TrivTVBlockEvalSetMesh, TrivTVBlockEvalDone         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBlockEvalOnce                                                      M
*****************************************************************************/
TrivTVBlockEvalStruct *TrivTVBlockEvalOnce(int i, int j, int k)
{
    int ii, jj, kk,
	UVLength = GlblBlockLengths[0] * GlblBlockLengths[1],
	ib = i * GlblBlockBlockSizes[0],
	jb = j * GlblBlockBlockSizes[1],
	kb = k * GlblBlockBlockSizes[2];
    TrivTVBlockEvalStruct
	*BE = GlblBlockEvals;

    for (kk = kb; kk < kb + GlblBlockBlockSizes[2]; kk++) {
        for (jj = jb; jj < jb + GlblBlockBlockSizes[1]; jj++) {
	    for (ii = ib; ii < ib + GlblBlockBlockSizes[0]; ii++, BE++) {
		int Base;
		CagdRType *U, *R;

		/* Position evaluation. */
		R = BE -> Pos;
		R[0] = R[1] = R[2] = 0.0;
		Base =
		    GlblBlockWBasisFuncs[kk].FirstBasisFuncIndex * UVLength +
	            GlblBlockVBasisFuncs[jj].FirstBasisFuncIndex *
							  GlblBlockLengths[0] +
		    GlblBlockUBasisFuncs[ii].FirstBasisFuncIndex;
		U = GlblBlockUBasisFuncs[ii].BasisFuncsVals;
		for (k = 0; k < GlblBlockOrders[2]; k++) {
		    int KStep = k * UVLength + Base;

		    for (j = 0; j < GlblBlockOrders[1]; j++) {
		        int Dir,
			    JKStep = j * GlblBlockLengths[0] + KStep;
			CagdRType
			    VW = GlblBlockVBasisFuncs[jj].BasisFuncsVals[j] *
			         GlblBlockWBasisFuncs[kk].BasisFuncsVals[k];

			for (Dir = 0; Dir < 3; Dir++)
			    for (i = 0; i < GlblBlockOrders[0]; i++)
				R[Dir] += GlblBlockMesh[i + JKStep][Dir] *
								  U[i] * VW;
		    }
		}

		/* Du evaluation. */
		R = BE -> Jcbn[0];
		R[0] = R[1] = R[2] = 0.0;
		Base =
		    GlblBlockWBasisFuncs[kk].FirstBasisFuncIndex * UVLength +
	            GlblBlockVBasisFuncs[jj].FirstBasisFuncIndex *
							  GlblBlockLengths[0] +
		    GlblBlockDUBasisFuncs[ii].FirstBasisFuncIndex;
		U = GlblBlockDUBasisFuncs[ii].BasisFuncsVals;
		for (k = 0; k < GlblBlockOrders[2]; k++) {
		    int KStep = k * UVLength + Base;

		    for (j = 0; j < GlblBlockOrders[1]; j++) {
		        int Dir,
			    JKStep = j * GlblBlockLengths[0] + KStep;
			CagdRType
			    VW = GlblBlockVBasisFuncs[jj].BasisFuncsVals[j] *
			         GlblBlockWBasisFuncs[kk].BasisFuncsVals[k];

			for (Dir = 0; Dir < 3; Dir++)
			    for (i = 0; i < GlblBlockOrders[0]; i++)
				R[Dir] += GlblBlockMesh[i + JKStep][Dir] *
								  U[i] * VW;
		    }
		}
		
		/* Dv evaluation. */
		R = BE -> Jcbn[1];
		R[0] = R[1] = R[2] = 0.0;
		Base =
		    GlblBlockWBasisFuncs[kk].FirstBasisFuncIndex * UVLength +
	            GlblBlockDVBasisFuncs[jj].FirstBasisFuncIndex *
							  GlblBlockLengths[0] +
		    GlblBlockUBasisFuncs[ii].FirstBasisFuncIndex;
		U = GlblBlockUBasisFuncs[ii].BasisFuncsVals;
		for (k = 0; k < GlblBlockOrders[2]; k++) {
		    int KStep = k * UVLength + Base;

		    for (j = 0; j < GlblBlockOrders[1]; j++) {
		        int Dir,
			    JKStep = j * GlblBlockLengths[0] + KStep;
			CagdRType
			    VW = GlblBlockDVBasisFuncs[jj].BasisFuncsVals[j] *
			         GlblBlockWBasisFuncs[kk].BasisFuncsVals[k];

			for (Dir = 0; Dir < 3; Dir++)
			    for (i = 0; i < GlblBlockOrders[0]; i++)
				R[Dir] += GlblBlockMesh[i + JKStep][Dir] *
								  U[i] * VW;
		    }
		}
		
		/* Dw evaluation. */
		R = BE -> Jcbn[2];
		R[0] = R[1] = R[2] = 0.0;
		Base =
		    GlblBlockDWBasisFuncs[kk].FirstBasisFuncIndex * UVLength +
	            GlblBlockVBasisFuncs[jj].FirstBasisFuncIndex *
							  GlblBlockLengths[0] +
		    GlblBlockUBasisFuncs[ii].FirstBasisFuncIndex;
		U = GlblBlockUBasisFuncs[ii].BasisFuncsVals;
		for (k = 0; k < GlblBlockOrders[2]; k++) {
		    int KStep = k * UVLength + Base;

		    for (j = 0; j < GlblBlockOrders[1]; j++) {
		        int Dir,
			    JKStep = j * GlblBlockLengths[0] + KStep;
			CagdRType
			    VW = GlblBlockVBasisFuncs[jj].BasisFuncsVals[j] *
			         GlblBlockDWBasisFuncs[kk].BasisFuncsVals[k];

			for (Dir = 0; Dir < 3; Dir++)
			    for (i = 0; i < GlblBlockOrders[0]; i++)
				R[Dir] += GlblBlockMesh[i + JKStep][Dir] *
								  U[i] * VW;
		    }
		}
	    }
	}
    }

    return GlblBlockEvals;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free all auxiliary memory used by this block evaluation procedure.       M
*                                                                            *
* PARAMETERS:                                                                M
*   None								     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVBlockEvalInit, TrivTVBlockEvalSetMesh, TrivTVBlockEvalOnce         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBlockEvalDone                                                      M
*****************************************************************************/
void TrivTVBlockEvalDone(void)
{
    BspBasisFuncMultEvalFree(GlblBlockUBasisFuncs, GlblBlockNumOfParams[0]);
    BspBasisFuncMultEvalFree(GlblBlockVBasisFuncs, GlblBlockNumOfParams[1]);
    BspBasisFuncMultEvalFree(GlblBlockWBasisFuncs, GlblBlockNumOfParams[2]);

    BspBasisFuncMultEvalFree(GlblBlockDUBasisFuncs, GlblBlockNumOfParams[0]);
    BspBasisFuncMultEvalFree(GlblBlockDVBasisFuncs, GlblBlockNumOfParams[1]);
    BspBasisFuncMultEvalFree(GlblBlockDWBasisFuncs, GlblBlockNumOfParams[2]);

    IritFree(GlblBlockEvals);
}

#ifdef DEBUG_TRIV_BLOCK_EVAL

#define ORDER		4
#define BLOCK_SIZE	3
#define U_LENGTH	20
#define V_LENGTH	20
#define W_LENGTH	20

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Simple test routine.                                                     *
*****************************************************************************/
void main(int argc, char **argv)
{
    int i, j, k, l, Lengths[3], Orders[3], BlockSizes[3], NumOfParams[3];
    IrtPtType *Params,
	*Mesh = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * 
					 U_LENGTH * V_LENGTH * W_LENGTH);
    TrivTVStruct *DuTV, *DvTV, *DwTV,
        *TV = TrivBspTVNew(U_LENGTH, V_LENGTH, W_LENGTH, ORDER, ORDER, ORDER,
			   CAGD_PT_E3_TYPE);

    fprintf(stderr, "Testing block-size %d of %d x %d x %d trivariate (order = %d).\n",
	    BLOCK_SIZE, U_LENGTH, V_LENGTH, W_LENGTH, ORDER);

    BspKnotUniformFloat(U_LENGTH, ORDER, TV -> UKnotVector);
    BspKnotUniformFloat(V_LENGTH, ORDER, TV -> VKnotVector);
    BspKnotUniformFloat(W_LENGTH, ORDER, TV -> WKnotVector);

    for (i = l = 0; i < U_LENGTH; i++)
	for (j = 0; j < V_LENGTH; j++) 
	    for (k = 0; k < W_LENGTH; k++, l++) {
	        TV -> Points[1][l] = Mesh[l][0] = IritRandom(-1, 1);
		TV -> Points[2][l] = Mesh[l][1] = IritRandom(-1, 1);
		TV -> Points[3][l] = Mesh[l][2] = IritRandom(-1, 1);
	    }

    DuTV = TrivTVDerive(TV, TRIV_CONST_U_DIR);
    DvTV = TrivTVDerive(TV, TRIV_CONST_V_DIR);
    DwTV = TrivTVDerive(TV, TRIV_CONST_W_DIR);

    /* Test block evaluations. */
    fprintf(stderr, "Starting block-evaluations' check...\n");

    Lengths[0] = U_LENGTH;
    Lengths[1] = V_LENGTH;
    Lengths[2] = W_LENGTH;
    Orders[0] = Orders[1] = Orders[2] = ORDER;
    BlockSizes[0] = BlockSizes[1] = BlockSizes[2] = BLOCK_SIZE;
    NumOfParams[0] = (U_LENGTH - ORDER + 1) * BlockSizes[0];
    NumOfParams[1] = (V_LENGTH - ORDER + 1) * BlockSizes[1];
    NumOfParams[2] = (W_LENGTH - ORDER + 1) * BlockSizes[2];
    Params = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * NumOfParams[0]
				       * NumOfParams[1] * NumOfParams[2]);

    for (i = 0;
	 i < IRIT_MAX(IRIT_MAX(NumOfParams[0], NumOfParams[1]),
		 NumOfParams[2]);
	 i++) {
        Params[i][0] = IritRandom(0, 1 - IRIT_EPS);
        Params[i][1] = IritRandom(0, 1 - IRIT_EPS);
        Params[i][2] = IritRandom(0, 1 - IRIT_EPS);
    }

    TrivTVBlockEvalInit(TV -> UKnotVector, TV -> VKnotVector,
			TV -> WKnotVector, Lengths, Orders, BlockSizes,
			Params, NumOfParams);
    TrivTVBlockEvalSetMesh(Mesh);

    for (k = 0; k < NumOfParams[2] / BlockSizes[2]; k++) {
        for (j = 0; j < NumOfParams[1] / BlockSizes[1]; j++) {
	    for (i = 0; i < NumOfParams[0] / BlockSizes[0]; i++) {
		int ii, jj, kk;
	        TrivTVBlockEvalStruct
		    *BE = TrivTVBlockEvalOnce(i, j, k);

		for (kk = l = 0; kk < BlockSizes[2]; kk++) {
		    for (jj = 0; jj < BlockSizes[1]; jj++) {
		        for (ii = 0; ii < BlockSizes[0]; ii++, l++) {
			    int ib = i * BlockSizes[0],
			        jb = j * BlockSizes[1],
				kb = k * BlockSizes[2];
			    CagdRType *R;

			    R = TrivTVEval(TV, Params[ib + ii][0],
					       Params[jb + jj][1],
					       Params[kb + kk][2]);

			    if (!IRIT_APX_EQ(R[1], BE[l].Pos[0]) ||
				!IRIT_APX_EQ(R[2], BE[l].Pos[1]) ||
				!IRIT_APX_EQ(R[3], BE[l].Pos[2]))
			        fprintf(stderr,
					"Evaluation of position return wrong answer.\n");

			    R = TrivTVEval(DuTV, Params[ib + ii][0],
					         Params[jb + jj][1],
					         Params[kb + kk][2]);
			    if (!IRIT_APX_EQ(R[1], BE[l].Jcbn[0][0]) ||
				!IRIT_APX_EQ(R[2], BE[l].Jcbn[0][1]) ||
				!IRIT_APX_EQ(R[3], BE[l].Jcbn[0][2]))
			        fprintf(stderr,
					"Evaluation of Du deriv. return wrong answer.\n");

			    R = TrivTVEval(DvTV, Params[ib + ii][0],
					         Params[jb + jj][1],
					         Params[kb + kk][2]);
			    if (!IRIT_APX_EQ(R[1], BE[l].Jcbn[1][0]) ||
				!IRIT_APX_EQ(R[2], BE[l].Jcbn[1][1]) ||
				!IRIT_APX_EQ(R[3], BE[l].Jcbn[1][2]))
			        fprintf(stderr,
					"Evaluation of Dv deriv. return wrong answer.\n");

			    R = TrivTVEval(DwTV, Params[ib + ii][0],
					         Params[jb + jj][1],
					         Params[kb + kk][2]);
			    if (!IRIT_APX_EQ(R[1], BE[l].Jcbn[2][0]) ||
				!IRIT_APX_EQ(R[2], BE[l].Jcbn[2][1]) ||
				!IRIT_APX_EQ(R[3], BE[l].Jcbn[2][2]))
			        fprintf(stderr,
					"Evaluation of Dw deriv. return wrong answer.\n");
			}
		    }
		}
	    }
	}
    }
    fprintf(stderr, "Done.  Timing now...\n");
    IritCPUTime(TRUE);
    TrivTVBlockEvalSetMesh(Mesh);

    for (k = 0; k < NumOfParams[2] / BlockSizes[2]; k++) {
        for (j = 0; j < NumOfParams[1] / BlockSizes[1]; j++) {
	    for (i = 0; i < NumOfParams[0] / BlockSizes[0]; i++) {
	        TrivTVBlockEvalStruct
		    *BE = TrivTVBlockEvalOnce(i, j, k);
	    }
	}
    }

    fprintf(stderr, 
	    "Done. Time for %d evaluations (%d block evaluations) = %f Sec\n",
	    NumOfParams[0] * NumOfParams[1] * NumOfParams[2],
	    (NumOfParams[0] / BlockSizes[0]) *
	        (NumOfParams[1] / BlockSizes[1]) *
	        (NumOfParams[2] / BlockSizes[2]),
	    IritCPUTime(FALSE));

    TrivTVBlockEvalDone();

    TrivTVFree(DuTV);
    TrivTVFree(DvTV);
    TrivTVFree(DwTV);
    TrivTVFree(TV);

    IritFree(Params);
    IritFree(Mesh);
}

#endif /* DEBUG_TRIV_BLOCK_EVAL */

#ifdef DEBUG_TRIV_MULTI_EVAL

#define ORDER		4
#define U_LENGTH	100
#define V_LENGTH	100
#define W_LENGTH	100
#define	NUM_OF_PARAMS	1000000

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Simple test routine.                                                     *
*****************************************************************************/
void main(int argc, char **argv)
{
    int i, j, k, l, RetSize;
    IrtRType *EvalVec, *V;
    IrtPtType 
	*Params = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * NUM_OF_PARAMS),
	*Mesh = (IrtPtType *) IritMalloc(sizeof(IrtPtType) * 
					 U_LENGTH * V_LENGTH * W_LENGTH);
    TrivTVStruct *DuTV, *DvTV, *DwTV,
        *TV = TrivBspTVNew(U_LENGTH, V_LENGTH, W_LENGTH, ORDER, ORDER, ORDER,
			   CAGD_PT_E3_TYPE);

    fprintf(stderr, "Testing (%d) multi-evals of %d x %d x %d trivariate (order = %d).\n",
	    NUM_OF_PARAMS, U_LENGTH, V_LENGTH, W_LENGTH, ORDER);

    BspKnotUniformFloat(U_LENGTH, ORDER, TV -> UKnotVector);
    BspKnotUniformFloat(V_LENGTH, ORDER, TV -> VKnotVector);
    BspKnotUniformFloat(W_LENGTH, ORDER, TV -> WKnotVector);

    for (i = l = 0; i < U_LENGTH; i++)
	for (j = 0; j < V_LENGTH; j++) 
	    for (k = 0; k < W_LENGTH; k++, l++) {
	        TV -> Points[1][l] = Mesh[l][0] = IritRandom(-1, 1);
		TV -> Points[2][l] = Mesh[l][1] = IritRandom(-1, 1);
		TV -> Points[3][l] = Mesh[l][2] = IritRandom(-1, 1);
	    }

    for (i = 0; i < NUM_OF_PARAMS; i++) {
        Params[i][0] = IritRandom(0, 1 - IRIT_EPS);
        Params[i][1] = IritRandom(0, 1 - IRIT_EPS);
        Params[i][2] = IritRandom(0, 1 - IRIT_EPS);
    }

    /* Test position evaluations. */
    fprintf(stderr, "Starting multi-evaluations of positions...");
    IritCPUTime(TRUE);

    EvalVec = TrivTVMultEval(TV -> UKnotVector, TV -> VKnotVector,
			     TV -> WKnotVector, U_LENGTH, V_LENGTH, W_LENGTH,
			     ORDER, ORDER, ORDER, Mesh, Params, NUM_OF_PARAMS,
			     &RetSize, CAGD_BSP_BASIS_FUNC_EVAL_MULT_VALUE);

    fprintf(stderr, "%f Secs. per eval.\n",  IritCPUTime(FALSE));

    for (i = 0, V = EvalVec; i < NUM_OF_PARAMS; i++, V += RetSize) {
        CagdRType
	   *R = TrivTVEval(TV, Params[i][0], Params[i][1], Params[i][2]);

	if (!IRIT_APX_EQ(R[1], V[0]) ||
	    !IRIT_APX_EQ(R[2], V[1]) ||
	    !IRIT_APX_EQ(R[3], V[2]))
	    fprintf(stderr, "Evaluation of position return wrong answer.\n");
    }
    IritFree(EvalVec);

    /* Test 1st derivative evaluations. */
    fprintf(stderr, "Starting multi-evaluations of Jacobian...");
    IritCPUTime(TRUE);

    EvalVec = TrivTVMultEval(TV -> UKnotVector, TV -> VKnotVector,
			     TV -> WKnotVector, U_LENGTH, V_LENGTH, W_LENGTH,
			     ORDER, ORDER, ORDER, Mesh, Params, NUM_OF_PARAMS,
			     &RetSize, CAGD_BSP_BASIS_FUNC_EVAL_MULT_DER1ST);

    fprintf(stderr, "%f Secs. per eval.\n",  IritCPUTime(FALSE));

    DuTV = TrivTVDerive(TV, TRIV_CONST_U_DIR);
    DvTV = TrivTVDerive(TV, TRIV_CONST_V_DIR);
    DwTV = TrivTVDerive(TV, TRIV_CONST_W_DIR);

    for (i = 0, V = EvalVec; i < NUM_OF_PARAMS; i++, V += RetSize) {
        CagdRType *R;

	R = TrivTVEval(DuTV, Params[i][0], Params[i][1], Params[i][2]);
	if (!IRIT_APX_EQ(R[1], V[0]) ||
	    !IRIT_APX_EQ(R[2], V[1]) ||
	    !IRIT_APX_EQ(R[3], V[2]))
	    fprintf(stderr, "Evaluation of Du deriv. return wrong answer.\n");

	R = TrivTVEval(DvTV, Params[i][0], Params[i][1], Params[i][2]);
	if (!IRIT_APX_EQ(R[1], V[3]) ||
	    !IRIT_APX_EQ(R[2], V[4]) ||
	    !IRIT_APX_EQ(R[3], V[5]))
	    fprintf(stderr, "Evaluation of Dv deriv. return wrong answer.\n");

	R = TrivTVEval(DwTV, Params[i][0], Params[i][1], Params[i][2]);
	if (!IRIT_APX_EQ(R[1], V[6]) ||
	    !IRIT_APX_EQ(R[2], V[7]) ||
	    !IRIT_APX_EQ(R[3], V[8]))
	    fprintf(stderr, "Evaluation of Dw deriv. return wrong answer.\n");
    }

    TrivTVFree(DuTV);
    TrivTVFree(DvTV);
    TrivTVFree(DwTV);
    TrivTVFree(TV);

    IritFree(EvalVec);
    IritFree(Params);
    IritFree(Mesh);
}

#endif /* DEBUG_TRIV_MULTI_EVAL */
