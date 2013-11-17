/******************************************************************************
* Triv_aux.c - auxiliary routine to interface to tri-variate rep.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include "triv_loc.h"

#define EPS_ROUND_KNOT		1e-9

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a tri-variate, returns its parametric domain.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:           Trivariate function to consider.                           M
*   UMin, UMax:   U Domain of TV will be placed herein.                      M
*   VMin, VMax:   V Domain of TV will be placed herein.                      M
*   WMin, WMax:   W Domain of TV will be placed herein.                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVDomain, trivariates                                                M
*****************************************************************************/
void TrivTVDomain(const TrivTVStruct *TV,
		  CagdRType *UMin,
		  CagdRType *UMax,
		  CagdRType *VMin,
		  CagdRType *VMax,
		  CagdRType *WMin,
		  CagdRType *WMax)
{
    int UOrder,	VOrder, WOrder, ULen, VLen, WLen;

    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	    *UMin = 0.0;
	    *UMax = 1.0;
	    *VMin = 0.0;
	    *VMax = 1.0;
	    *WMin = 0.0;
	    *WMax = 1.0;
	    break;
	case TRIV_TVBSPLINE_TYPE:
	    UOrder = TV -> UOrder;
	    VOrder = TV -> VOrder;
	    WOrder = TV -> WOrder;
	    ULen = TV -> ULength;
	    VLen = TV -> VLength;
	    WLen = TV -> WLength;

	    *UMin = TV -> UKnotVector[UOrder - 1];
	    *UMax = TV -> UKnotVector[ULen];
	    *VMin = TV -> VKnotVector[VOrder - 1];
	    *VMax = TV -> VKnotVector[VLen];
	    *WMin = TV -> WKnotVector[WOrder - 1];
	    *WMax = TV -> WKnotVector[WLen];
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a tri-variate and a domain - validate it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To make sure t is in its Dir domain.                           M
*   t:        Parameter value to verify.                                     M
*   Dir:      Direction. Either U or V or W.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if in domain, FALSE otherwise.                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivParamInDomain, trivariates                                           M
*****************************************************************************/
CagdBType TrivParamInDomain(const TrivTVStruct *TV,
			    CagdRType t,
			    TrivTVDirType Dir)
{
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    return t >= UMin && t <= UMax;
	case TRIV_CONST_V_DIR:
	    return t >= VMin && t <= VMax;
	case TRIV_CONST_W_DIR:
	    return t >= WMin && t <= WMax;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_WRONG_DOMAIN);
	    return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a tri-variate and a domain - validate it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To make sure (u, v, w) is in its domain.                       M
*   u, v, w:  To verify if it is in TV's parametric domain.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if in domain, FALSE otherwise.                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivParamsInDomain, trivariates                                          M
*****************************************************************************/
CagdBType TrivParamsInDomain(const TrivTVStruct *TV,
			     CagdRType u,
			     CagdRType v,
			     CagdRType w)
{
    CagdRType UMin, UMax, VMin, VMax, WMin, WMax;

    TrivTVDomain(TV, &UMin, &UMax, &VMin, &VMax, &WMin, &WMax);

    return u >= UMin && u <= UMax &&
           v >= VMin && v <= VMax &&
	   w >= WMin && w <= WMax;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a tri-variate, returns a sub-region of it.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        To extract a sub-region from.                                 M
*   t1, t2:    Domain to extract from TV, in parametric direction Dir.       M
*   Dir:       Direction to extract the sub-region. Either U or V or W.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   A sub-region of TV from t1 to t2 in direction Dir.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVRegionFromTV, trivariates                                          M
*****************************************************************************/
TrivTVStruct *TrivTVRegionFromTV(const TrivTVStruct *TV,
				 CagdRType t1,
				 CagdRType t2,
				 TrivTVDirType Dir)
{
    CagdRType RMin, RMax, SMin, SMax, TMin, TMax, R1, R2;
    TrivTVStruct *TVs, *CpTV;
    CagdBType
	BezTV = FALSE;

    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	    BezTV = TRUE;
	    R1 = 0.0;
	    R2 = 1.0;
	    break;
	case TRIV_TVBSPLINE_TYPE:
    	    switch (Dir) {
		case TRIV_CONST_U_DIR:
		    TrivTVDomain(TV, &R1, &R2, &SMin, &SMax, &TMin, &TMax);
		    break;
		case TRIV_CONST_V_DIR:
		    TrivTVDomain(TV, &RMin, &RMax, &R1, &R2, &TMin, &TMax);
		    break;
		case TRIV_CONST_W_DIR:
		    TrivTVDomain(TV, &RMin, &RMax, &SMin, &SMax, &R1, &R2);
		    break;
		default:
		    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
		    break;
	    }
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    R1 = R2 = 0.0;
	    return NULL;
    }

    if (t1 > t2)
	IRIT_SWAP(CagdRType, t1, t2);

    if (!IRIT_APX_EQ_EPS(t1, R1, EPS_ROUND_KNOT)) {
	TVs = TrivTVSubdivAtParam(TV, t1, Dir);
	TV = CpTV = TVs -> Pnext;
	TVs -> Pnext = NULL;
	TrivTVFree(TVs);			   /* Free the first region. */
    }
    else
	CpTV = NULL;

    if (IRIT_APX_EQ_EPS(t2, R2, EPS_ROUND_KNOT))
	return CpTV != NULL ? CpTV : TrivTVCopy(TV);
    else {
	if (BezTV)
	    t2 = (t2 - t1) / (R2 - t1);

	TVs = TrivTVSubdivAtParam(TV, t2, Dir);

	if (CpTV != NULL)
	    TrivTVFree(CpTV);

    	TrivTVFree(TVs -> Pnext);		  /* Free the second region. */
    	TVs -> Pnext = NULL;
	return TVs;				/* Returns the first region. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a trivariate freeform function.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBBox, bbox, bounding box                                           M
*****************************************************************************/
void TrivTVBBox(const TrivTVStruct *TV, CagdBBoxStruct *BBox)
{
    CagdPointsBBox(TV -> Points,
		   TV -> ULength * TV -> VLength * TV -> WLength,
		   3, BBox -> Min, BBox -> Max);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of trivariate freeform function.        M
*                                                                            *
* PARAMETERS:                                                                M
*   TVs:      To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVListBBox, bbox, bounding box                                       M
*****************************************************************************/
void TrivTVListBBox(const TrivTVStruct *TVs, CagdBBoxStruct *BBox)
{
    CAGD_RESET_BBOX(BBox);

    for ( ; TVs != NULL; TVs = TVs -> Pnext) {
	CagdBBoxStruct TmpBBox;

	TrivTVBBox(TVs, &TmpBBox);
	CagdMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reverse the designated direction                                         M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:      To construct a reverse TV for.                                  M
*   Dir:     The direction in TV to reverse.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:       Reversed trivariate.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVReverse2Dirs                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVReverseDir                                                         M
*****************************************************************************/
TrivTVStruct *TrivTVReverseDir(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, Len, Row, Col, Dpth, Idx,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
        UVLength = ULength * VLength,
	WLength = TV -> WLength,
	MaxCoord = TRIV_NUM_OF_PT_COORD(TV);
    TrivTVStruct
	*ReversedTV = TrivTVCopy(TV);
    CagdRType *KV,
	**Points = ReversedTV -> Points;

    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	case TRIV_TVBSPLINE_TYPE:
	    break;
	case TRIV_TVPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }

    /* Reverse the Mesh: */
    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    Len = (ULength >> 1);
	    for (Dpth = 0; Dpth < WLength; Dpth++) {
	        for (Row = 0; Row < VLength; Row++) {
		    Idx = Dpth * UVLength + Row * ULength;

		    for (Col = 0; Col < Len; Col++) {
			for (i = IsNotRational; i <= MaxCoord; i++) {
			    IRIT_SWAP(CagdRType,
				      Points[i][Idx + Col],
				      Points[i][Idx + ULength - Col - 1]);
			}
		    }
		}
	    }
	    break;
	case TRIV_CONST_V_DIR:
	    Len = (VLength >> 1);
	    for (Dpth = 0; Dpth < WLength; Dpth++) {
	        for (Col = 0; Col < ULength; Col++) {
		    Idx = Dpth * UVLength + Col;

		    for (Row = 0; Row < Len; Row++) {
		        for (i = IsNotRational; i <= MaxCoord; i++) {

			  IRIT_SWAP(CagdRType,
				    Points[i][Idx + Row * ULength],
				    Points[i][Idx + (VLength - Row - 1)
					                         * ULength]);
			}
		    }
		}
	    }
	    break;
	case TRIV_CONST_W_DIR:
	    Len = (WLength >> 1);
	    for (Row = 0; Row < VLength; Row++) {
	        for (Col = 0; Col < ULength; Col++) {
		    Idx = Row * ULength + Col;

		    for (Dpth = 0; Dpth < Len; Dpth++) {
		        for (i = IsNotRational; i <= MaxCoord; i++) {
			    IRIT_SWAP(CagdRType,
				      Points[i][Idx + Dpth * UVLength],
				      Points[i][Idx + (WLength - Dpth - 1)
					                         * UVLength]);
			}
		    }
		}
	    }
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_CONST_UVW);
    }


    /* Reverse the U/U/W knot vector if it exists: */
    if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
        switch (Dir) {
	    case TRIV_CONST_U_DIR:
	        KV = BspKnotReverse(TV -> UKnotVector,
				    TV -> UOrder + TRIV_TV_UPT_LST_LEN(TV));
		IritFree(ReversedTV -> UKnotVector);
		ReversedTV -> UKnotVector = KV;
		break;
	    case TRIV_CONST_V_DIR:
	        KV = BspKnotReverse(TV -> VKnotVector,
				    TV -> VOrder + TRIV_TV_VPT_LST_LEN(TV));
		IritFree(ReversedTV -> VKnotVector);
		ReversedTV -> VKnotVector = KV;
		break;
	    case TRIV_CONST_W_DIR:
	        KV = BspKnotReverse(TV -> WKnotVector,
				    TV -> WOrder + TRIV_TV_WPT_LST_LEN(TV));
		IritFree(ReversedTV -> WKnotVector);
		ReversedTV -> WKnotVector = KV;
		break;
            default:
	        assert(0);

	}
    }

    return ReversedTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Reverse/swap the designated two dirs.                                    M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:          To construct a reverse TV for.                              M
*   Dir1, Dir2:  The two directions in TV to swap.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Reversed/swap trivariate.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVReverseDir                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVReverse2Dirs                                                       M
*****************************************************************************/
TrivTVStruct *TrivTVReverse2Dirs(const TrivTVStruct *TV,
				 TrivTVDirType Dir1,
				 TrivTVDirType Dir2)
{

    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, Row, Col, Dpth, Dir1Idx, Dir2Idx,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
        UVLength = ULength * VLength,
	WLength = TV -> WLength,
	MaxCoord = TRIV_NUM_OF_PT_COORD(TV);
    TrivTVStruct
	*ReversedTV = TrivTVCopy(TV);
    CagdRType
	* const *OrigPoints = TV -> Points;
    CagdRType
	**RevPoints = ReversedTV -> Points;

    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	case TRIV_TVBSPLINE_TYPE:
	    break;
	case TRIV_TVPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_SRF);
	    return NULL;
    }

    switch (Dir1) {
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_CONST_UVW);
	case TRIV_CONST_U_DIR:
	    Dir1Idx = 1;
	    break;
	case TRIV_CONST_V_DIR:
	    Dir1Idx = 2;
	    break;
	case TRIV_CONST_W_DIR:
	    Dir1Idx = 3;
	    break;
    }
    switch (Dir2) {
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_CONST_UVW);
	case TRIV_CONST_U_DIR:
	    Dir2Idx = 1;
	    break;
	case TRIV_CONST_V_DIR:
	    Dir2Idx = 2;
	    break;
	case TRIV_CONST_W_DIR:
	    Dir2Idx = 3;
	    break;
    }
    assert(Dir1Idx != Dir2Idx);
    if (Dir1Idx > Dir2Idx)
        IRIT_SWAP(int, Dir1Idx, Dir2Idx);

    /* Reverse the Mesh: */
    if (Dir1Idx == 1 && Dir2Idx == 2) {     /* Swap the U and V directions. */
        for (Dpth = 0; Dpth < WLength; Dpth++) {
	    for (Row = 0; Row < VLength; Row++) {
	        for (Col = 0; Col < ULength; Col++) {
		    for (i = IsNotRational; i <= MaxCoord; i++) {
		        RevPoints[i][Col * VLength + Row + Dpth * UVLength] =
			    OrigPoints[i][Col + Row * ULength
					                   + Dpth * UVLength];
		    }
		}
	    }
	}

	/* Swap the knot vectors if they exists: */
	if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	    IRIT_SWAP(CagdRType *, ReversedTV -> UKnotVector,
		                   ReversedTV -> VKnotVector);
	}

	/* And swap the orders and lengths. */
	IRIT_SWAP(int, ReversedTV -> UOrder, ReversedTV -> VOrder);
	IRIT_SWAP(int, ReversedTV -> ULength, ReversedTV -> VLength);
	IRIT_SWAP(int, ReversedTV -> UPeriodic, ReversedTV -> VPeriodic);
    }
    else if (Dir1Idx == 1 && Dir2Idx == 3) {/* Swap the U and W directions. */
        for (Row = 0; Row < VLength; Row++) {
	    for (Dpth = 0; Dpth < WLength; Dpth++) {
	        for (Col = 0; Col < ULength; Col++) {
		    for (i = IsNotRational; i <= MaxCoord; i++) {
		        RevPoints[i][Col * WLength * VLength + Row * WLength
				                                    + Dpth] =
			    OrigPoints[i][Col + Row * ULength +
					                     Dpth * UVLength];
		    }
		}
	    }
	}

	/* Swap the knot vectors if they exists: */
	if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	    IRIT_SWAP(CagdRType *, ReversedTV -> UKnotVector,
		                   ReversedTV -> WKnotVector);
	}

	/* And swap the orders and lengths. */
	IRIT_SWAP(int, ReversedTV -> UOrder, ReversedTV -> WOrder);
	IRIT_SWAP(int, ReversedTV -> ULength, ReversedTV -> WLength);
	IRIT_SWAP(int, ReversedTV -> UPeriodic, ReversedTV -> WPeriodic);
    }
    else if (Dir1Idx == 2 && Dir2Idx == 3) {/* Swap the V and W directions. */
        for (Col = 0; Col < ULength; Col++) {
	    for (Dpth = 0; Dpth < WLength; Dpth++) {
	        for (Row = 0; Row < VLength; Row++) {
		    for (i = IsNotRational; i <= MaxCoord; i++) {
		        RevPoints[i][Col + Row * ULength * WLength
				                          + Dpth * ULength] =
			    OrigPoints[i][Col + Row * ULength
					                   + Dpth * UVLength];
		    }
		}
	    }
	}

	/* Swap the knot vectors if they exists: */
	if (TV -> GType == TRIV_TVBSPLINE_TYPE) {
	    IRIT_SWAP(CagdRType *, ReversedTV -> VKnotVector,
		                   ReversedTV -> WKnotVector);
	}

	/* And swap the orders and lengths. */
	IRIT_SWAP(int, ReversedTV -> VOrder, ReversedTV -> WOrder);
	IRIT_SWAP(int, ReversedTV -> VLength, ReversedTV -> WLength);
	IRIT_SWAP(int, ReversedTV -> VPeriodic, ReversedTV -> WPeriodic);
    }
    else {
        assert(0);
    }

    ReversedTV -> UVPlane = ReversedTV -> ULength * ReversedTV -> VLength;

    return ReversedTV;
}
