/******************************************************************************
* CagdEXTR.c - Extrusion operator out of a given profile and a direction vec. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 91.					      *
******************************************************************************/

#include "cagd_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs an extrusion surface in the Vector direction for the given      M
* profile curve. Input curve can be either a Bspline or a Bezier curve and   M
* the resulting output surface will be of the same type.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:    To exturde in direction specified by Vec.                       M
*   Vec:     Direction as well as magnitude of extursion.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   An extrusion surface with Orders of the original      M
*                      Crv order and 2 in the extrusion direction.           M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdExtrudeSrf, surface constructors                                     M
*****************************************************************************/
CagdSrfStruct *CagdExtrudeSrf(const CagdCrvStruct *CCrv,
			      const CagdVecStruct *Vec)
{
    CagdSrfStruct *Srf;
    CagdRType **SrfPoints;
    CagdRType * const *CrvPoints;
    CagdRType const
	*Dir = Vec -> Vec;
    int i, j,
        VecMaxCoord = (Dir[2] != 0.0 ? 3 : (Dir[1] != 0.0 ? 2 : 1)),
	MaxCoord = CAGD_NUM_OF_PT_COORD(CCrv -> PType),
	Len = CCrv -> Length;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(CCrv);
    CagdPointType
        PType = CAGD_MAKE_PT_TYPE(!IsNotRational,
				  MaxCoord = IRIT_MAX(VecMaxCoord, MaxCoord));
    CagdCrvStruct *Crv;

    if (CCrv -> PType != PType)
        Crv = CagdCoerceCrvTo(CCrv, PType, FALSE);
    else
        Crv = CagdCrvCopy(CCrv);
    CrvPoints = Crv -> Points;

    switch (PType) {
	case CAGD_PT_P1_TYPE:
	case CAGD_PT_E1_TYPE:
	case CAGD_PT_P2_TYPE:
	case CAGD_PT_E2_TYPE:
	case CAGD_PT_P3_TYPE:
	case CAGD_PT_E3_TYPE:
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNSUPPORT_PT);
	    break;
    }

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    Srf = BzrSrfNew(Len, 2, PType);
	    break;
	case CAGD_CBSPLINE_TYPE:
	    Srf = BspPeriodicSrfNew(Len, 2,
				    Crv -> Order, 2,
				    Crv -> Periodic, FALSE,
				    PType);
	    CAGD_GEN_COPY(Srf -> UKnotVector, Crv -> KnotVector,
			  sizeof(CagdRType) * (CAGD_CRV_PT_LST_LEN(Crv) +
					       Crv -> Order));
	    Srf -> VKnotVector[0] = Srf -> VKnotVector[1] = 0.0;
	    Srf -> VKnotVector[2] = Srf -> VKnotVector[3] = 1.0;
	    break;
	case CAGD_CPOWER_TYPE:
	    CAGD_FATAL_ERROR(CAGD_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }

    /* Copy the control mesh - first row is exactly the same as the curve    */
    /* while second one is the same as first one translated by Vec.          */
    SrfPoints = Srf -> Points;

    for (i = IsNotRational; i <= MaxCoord; i++)		       /* First row. */
	CAGD_GEN_COPY(SrfPoints[i], CrvPoints[i],
		      sizeof(CagdRType) * Len);

    /* Make a copy of the Second row do we can "work" on it. */
    for (i = IsNotRational; i <= MaxCoord; i++)		      /* Second row. */
	CAGD_GEN_COPY(&SrfPoints[i][Len], CrvPoints[i],
		      sizeof(CagdRType) * Len);

    for (i = 1; i <= MaxCoord; i++)		/* Translate the second row. */
	for (j = Len; j < Len * 2; j++)
	    SrfPoints[i][j] += IsNotRational ? Dir[i - 1] :
					       Dir[i - 1] * SrfPoints[W][j];

    CAGD_SET_GEOM_TYPE(Srf, CAGD_GEOM_EXTRUSION);

    CagdCrvFree(Crv);

    return Srf;
}
