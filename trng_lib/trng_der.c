/******************************************************************************
* Trng_Der.c - Compute derivatives of triangular surfaces.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 96.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "trng_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a  triangular surface, computes its partial derivative triangular    M
* surface in direction Dir.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   Triangular Surface to differentiate.                           M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:   Differentiated triangular surface in direction  M
*		Dir.						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngTriSrfDerive, triangular surfaces.                                   M
*****************************************************************************/
TrngTriangSrfStruct *TrngTriSrfDerive(const TrngTriangSrfStruct *TriSrf,
				      TrngTriSrfDirType Dir)
{
    switch (TriSrf -> GType) {
	case TRNG_TRISRF_BEZIER_TYPE:
	    return TrngBzrTriSrfDerive(TriSrf, Dir);
	case TRNG_TRISRF_BSPLINE_TYPE:
	    return TrngBspTriSrfDerive(TriSrf, Dir);
	case TRNG_TRISRF_GREGORY_TYPE:
	    TRNG_FATAL_ERROR(TRNG_ERR_GREGORY_NO_SUPPORT);
	    return NULL;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_UNDEF_GEOM);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier triangular surface, computes its principal derivative in    M
* direction Dir.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   Triangular Surface to differentiate.                           M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:   Differentiated triangular surface in direction  M
*		Dir. A Bezier triangular surface.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrngBzrTriSrfDerive                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfDerive, triangular surfaces, derivatives                    M
*****************************************************************************/
TrngTriangSrfStruct *TrngBzrTriSrfDerive(const TrngTriangSrfStruct *TriSrf,
					 TrngTriSrfDirType Dir)
{
    CagdVType V;

    switch (Dir) {
	case TRNG_CONST_U_DIR:
	    V[0] =  1.0;
	    V[1] = -0.5;
	    V[2] = -0.5;
	    break;
	case TRNG_CONST_V_DIR:
	    V[0] = -0.5;
	    V[1] =  1.0;
	    V[2] = -0.5;
	    break;
	case TRNG_CONST_W_DIR:
	    V[0] = -0.5;
	    V[1] = -0.5;
	    V[2] =  1.0;
	    break;
	default:
	    TRNG_FATAL_ERROR(TRNG_ERR_DIR_NOT_VALID);
	    return NULL;
    }

    return TrngBzrTriSrfDirecDerive(TriSrf, V);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier triangular surface, computes its directional derivative in  M
* parametric direction DirectionalDeriv.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:     Triangular Surface to differentiate.			     M
*   DirecDeriv: Derivative direction vector (coefficients must sum to zero!).M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:   Differentiated triangular surface in direction  M
*		Dir. A Bezier triangular surface.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrngBzrTriSrfDerive                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBzrTriSrfDirecDerive, triangular surfaces, derivatives               M
*****************************************************************************/
TrngTriangSrfStruct *TrngBzrTriSrfDirecDerive(const TrngTriangSrfStruct *TriSrf,
					      CagdVType DirecDeriv)
{
    CagdPointType
	PType = TriSrf -> PType;
    int i, j, l,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType),
	Length = TriSrf -> Length,
	Length1 = Length - 1;
    CagdBType
	IsNotRational = !TRNG_IS_RATIONAL_TRISRF(TriSrf);
    TrngTriangSrfStruct
	*DerTriSrf = TrngBzrTriSrfNew(Length - 1, PType);
    CagdRType
	* const *Points = TriSrf -> Points,
	**DerPoints = DerTriSrf -> Points;

    for (i = 0; i < Length; i++) {
	for (j = 0; j < Length1 - i; j++) {
	    int DerIndex = TRNG_MESH_IJK(DerTriSrf, i, j, Length1 - i - j - 1),
		Index1 = TRNG_MESH_IJK(TriSrf, i,     j,     Length - i - j - 1),
		Index2 = TRNG_MESH_IJK(TriSrf, i + 1, j,     Length - i - j - 2),
		Index3 = TRNG_MESH_IJK(TriSrf, i,     j + 1, Length - i - j - 2);

	    for (l = IsNotRational; l <= MaxCoord; l++)
	        DerPoints[l][DerIndex] = Points[l][Index1] * DirecDeriv[0] + 
					 Points[l][Index2] * DirecDeriv[1] + 
					 Points[l][Index3] * DirecDeriv[2];
	}
    }

    return DerTriSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline triangular surface, computes its partial derivative        M
* triangular surface in direction Dir.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   TriSrf:   Triangular Surface to differentiate.                           M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrngTriangSrfStruct *:   Differentiated triangular surface in direction  M
*		     Dir. A Bspline triangular surface.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrngBspTriSrfDerive, triangular surfaces                                 M
*****************************************************************************/
TrngTriangSrfStruct *TrngBspTriSrfDerive(const TrngTriangSrfStruct *TriSrf,
					 TrngTriSrfDirType Dir)
{
    TRNG_FATAL_ERROR(TRNG_ERR_BSPLINE_NO_SUPPORT);
    return NULL;
}
