/******************************************************************************
* CagdRuld.c - Ruled srf operator out of given two profiles.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a ruled surface between the two provided curves.                M
* OtherOrder and OtherLen (equal for Bezier) specifies the desired order and M
* refineness level (if Bspline) of the other ruled direction.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv1, CCrv2: The two curves to form a ruled surface in between.         M
*   OtherOrder: Usually two, but one can specify higher orders in the ruled  M
*               direction. OtherOrder must never be larger than OrderLen.    M
*   OtherLen:   Usually two control points in the ruled direction which      M
*               necesitates a linear interpolation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The ruled surface.                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdRuledSrf, ruled surface, surface constructors                        M
*****************************************************************************/
CagdSrfStruct *CagdRuledSrf(const CagdCrvStruct *CCrv1,
			    const CagdCrvStruct *CCrv2,
			    int OtherOrder,
			    int OtherLen)
{
    CagdSrfStruct *Srf;
    int i, j, k, MaxCoord, Len;
    CagdPointType PType;
    CagdBType IsNotRational;
    CagdRType **SrfPoints, *Crv1SrcPt, *Crv2SrcPt, *SrfDestPt, t, t1,
						 **Crv1Points, **Crv2Points;
    CagdCrvStruct
        *Crv1 = CagdCrvCopy(CCrv1),
        *Crv2 = CagdCrvCopy(CCrv2);

    CagdMakeCrvsCompatible(&Crv1, &Crv2, TRUE, TRUE);

    MaxCoord = CAGD_NUM_OF_PT_COORD(Crv1 -> PType),
    Len = Crv1 -> Length;
    PType = Crv1 -> PType;
    IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv1);
    Crv1Points = Crv1 -> Points;
    Crv2Points = Crv2 -> Points;

    switch (Crv1 -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Srf = BzrSrfNew(Len, OtherLen, PType);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Srf = BspPeriodicSrfNew(Len, OtherLen, Crv1 -> Order, OtherOrder,
				    Crv1 -> Periodic, FALSE, PType);
	    CAGD_GEN_COPY(Srf -> UKnotVector, Crv1 -> KnotVector,
			  sizeof(CagdRType) * (CAGD_CRV_PT_LST_LEN(Crv1) +
					       Crv1 -> Order));
	    BspKnotUniformOpen(OtherLen, OtherOrder, Srf -> VKnotVector);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    /* Copy the control mesh - first row is exactly the same as the first    */
    /* curve while last row is the same as second curve.		     */
    /* The middle rows are convex blend of the first/last rows.		     */
    SrfPoints = Srf -> Points;

    for (i = IsNotRational; i <= MaxCoord; i++)		       /* First row. */
	CAGD_GEN_COPY(SrfPoints[i], Crv1Points[i],
		      sizeof(CagdRType) * Len);

    /* Make a copy of the last row. */
    for (i = IsNotRational; i <= MaxCoord; i++)		        /* Last row. */
	CAGD_GEN_COPY(&SrfPoints[i][Len * (OtherLen - 1)], Crv2Points[i],
		      sizeof(CagdRType) * Len);

    /* And compute the internal rows, if any: */
    for (j = 1; j < OtherLen - 1; j++) {
	t = ((CagdRType) j) / (OtherLen - 1);
	t1 = 1.0 - t;
	for (i = IsNotRational; i <= MaxCoord; i++) {
	    SrfDestPt = &SrfPoints[i][Len * j];
	    Crv1SrcPt = Crv1Points[i];
	    Crv2SrcPt = Crv2Points[i];
	    for (k = 0; k < Len; k++)
		SrfDestPt[k] = t1 * Crv1SrcPt[k] + t * Crv2SrcPt[k];
	}
    }		

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_RULED_SRF);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a bilinear surface between the four provided points.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Pt00, Pt01, Pt10, Pt11: The four points to consturct a bilinear between. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A bilinear surface with four corners at Ptij.          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdBilinearSrf, Bilinear surface, surface constructors                  M
*****************************************************************************/
CagdSrfStruct *CagdBilinearSrf(const CagdPtStruct *Pt00,
			       const CagdPtStruct *Pt01,
			       const CagdPtStruct *Pt10,
			       const CagdPtStruct *Pt11)
{
    CagdCrvStruct
	*Crv1 = CagdMergePtPt(Pt00, Pt01),
	*Crv2 = CagdMergePtPt(Pt10, Pt11);
    CagdSrfStruct
	*Srf = CagdRuledSrf(Crv1, Crv2, 2, 2);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_BILINEAR);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Promotes a curve to a surface by creating a ruled surface between the      M 
* curve to itself. Dir controls if the curve should be U or V surface        M
* direction.	                                                             M
*   The resulting surface is degenerate in that its speed is zero in the     M
* ruled direction and hence thesurface is not regular.                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:      A Crv to promote into a surface                                M
*   Dir:      Direction of ruling. Either U or V.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *: The surface promoted from Crv.                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPromoteCrvToSrf                                                      M
*****************************************************************************/
CagdSrfStruct *CagdPromoteCrvToSrf(const CagdCrvStruct *Crv,
				   CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf, *Srf;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType),
	Len = Crv -> Length;
    CagdPointType
	PType = Crv -> PType;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Srf = BzrSrfNew(Len, 1, PType);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Srf = BspPeriodicSrfNew(Len, 1, Crv -> Order, 1,
				    Crv -> Periodic, FALSE, PType);
	    CAGD_GEN_COPY(Srf -> UKnotVector, Crv -> KnotVector,
			  sizeof(CagdRType) * (CAGD_CRV_PT_LST_LEN(Crv) +
					       Crv -> Order));
	    BspKnotUniformOpen(1, 1, Srf -> VKnotVector);
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    /* Copy the control mesh - one row that is exactly the same as the curve */
    for (i = IsNotRational; i <= MaxCoord; i++)
	CAGD_GEN_COPY(Srf -> Points[i], Crv -> Points[i],
		      sizeof(CagdRType) * Len);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    break;
	case CAGD_CONST_V_DIR:
	    TSrf = CagdSrfReverse2(Srf);
	    CagdSrfFree(Srf);
	    Srf = TSrf;
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
	
    }

    return Srf;
}


