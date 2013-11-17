/******************************************************************************
* TrivEXTR.c - Extrusion operator out of a given surface and a direction vec. *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 2000.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs an extrusion trivariate in the Vector direction for the given   M
* surface. Input surface can be either a Bspline or a Bezier surface and     M
* the resulting output trivariate will be of the same type.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     To exturde in direction specified by Vec.                       M
*   Vec:     Direction as well as magnitude of extursion.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   An extrusion trivariate volume with Orders of the      M
*                      original Srf order and 2 in the extrusion direction.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdExtrudeSrf                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivExtrudeTV, trivariate constructors                                   M
*****************************************************************************/
TrivTVStruct *TrivExtrudeTV(const CagdSrfStruct *Srf, const CagdVecStruct *Vec)
{
    TrivTVStruct *TV;
    int i, j,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType),
	Len = Srf -> ULength * Srf -> VLength;
    CagdPointType
	PType = Srf -> PType;
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    CagdRType **TVPoints;
    CagdRType
	* const *SrfPoints = Srf -> Points;
    CagdRType
	const *Dir = Vec -> Vec;

    switch (PType) {
	case CAGD_PT_P2_TYPE:
	    PType = CAGD_PT_P3_TYPE;
	    break;
	case CAGD_PT_E2_TYPE:
	    PType = CAGD_PT_E3_TYPE;
	    break;
	case CAGD_PT_P3_TYPE:
	case CAGD_PT_E3_TYPE:
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNSUPPORT_PT);
	    break;
    }

    switch (Srf -> GType) {
	case CAGD_SBEZIER_TYPE:
	    TV = TrivBzrTVNew(Srf -> ULength, Srf -> VLength, 2, PType);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    TV = TrivBspTVNew(Srf -> ULength, Srf -> VLength, 2,
			      Srf -> UOrder, Srf -> VOrder, 2,
			      PType);
	    CAGD_GEN_COPY(TV -> UKnotVector, Srf -> UKnotVector,
			  sizeof(CagdRType) * (TV -> ULength + TV -> UOrder));
	    CAGD_GEN_COPY(TV -> VKnotVector, Srf -> VKnotVector,
			  sizeof(CagdRType) * (TV -> VLength + TV -> VOrder));
	    TV -> WKnotVector[0] = TV -> WKnotVector[1] = 0.0;
	    TV -> WKnotVector[2] = TV -> WKnotVector[3] = 1.0;
	    break;
	case CAGD_SPOWER_TYPE:
	    TRIV_FATAL_ERROR(TRIV_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_SRF);
	    return NULL;
    }

    /* Copy the control mesh - first layer is exactly the same as the        */
    /* surface while second one is the same as first one translated by Vec.  */
    TVPoints = TV -> Points;

    for (i = IsNotRational; i <= MaxCoord; i++)	       /* First depth layer. */
	CAGD_GEN_COPY(TVPoints[i], SrfPoints[i], sizeof(CagdRType) * Len);

    /* Make a copy of the Second layer do we can "work" on it. */
    for (i = IsNotRational; i <= MaxCoord; i++)	      /* Second depth layer. */
	CAGD_GEN_COPY(&TVPoints[i][Len], SrfPoints[i],
		      sizeof(CagdRType) * Len);

    /* If the surface has lesser dimension (i.e. was 2D), Add zeros. */
    for (i = MaxCoord + 1; i <= 3; i++)
	for (j = 0; j < Len * 2; j++)
	    TVPoints[i][j] = 0.0;

    for (i = 1; i <= 3; i++)		      /* Translate the second layer. */
	for (j = Len; j < Len * 2; j++)
	    TVPoints[i][j] += IsNotRational ? Dir[i - 1] :
					       Dir[i - 1] * TVPoints[0][j];

    TRIV_SET_GEOM_TYPE(TV, TRIV_GEOM_EXTRUSION);

    return TV;
}
