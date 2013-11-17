/******************************************************************************
* Triv_Der.c - Compute derivatives of tri-variates.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Sep. 94.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "triv_loc.h"

/* Define some marcos to make some of the routines below look better. They  */
/* calculate the index of the U, V, W point of the control mesh in Points.  */
#define DERIVED_TV(U, V, W)	TRIV_MESH_UVW(DerivedTV, (U), (V), (W))
#define ORIG_TV(U, V, W)	TRIV_MESH_UVW(TV, (U), (V), (W))

IRIT_STATIC_DATA CagdBType
    GlblDeriveScalar = FALSE;

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trivariate, computes its partial derivative trivariate in          M
* direction Dir.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to differentiate.                                   M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Differentiated trivariate in direction Dir.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVDerive, TrivBspTVDerive, TrivTVDeriveScalar                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVDerive, trivariates                                                M
*****************************************************************************/
TrivTVStruct *TrivTVDerive(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	    return TrivBzrTVDerive(TV, Dir);
	case TRIV_TVBSPLINE_TYPE:
	    return TrivBspTVDerive(TV, Dir);
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    return NULL;
    }
}


/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a trivariate, computes its partial derivative trivariate in          M
* direction Dir.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to differentiate.                                   M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Differentiated trivariate in direction Dir.            M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVDeriveScalar, TrivBspTVDeriveScalar, TrivTVDerive               M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVDeriveScalar, trivariates                                          M
*****************************************************************************/
TrivTVStruct *TrivTVDeriveScalar(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
	    return TrivBzrTVDeriveScalar(TV, Dir);
	case TRIV_TVBSPLINE_TYPE:
	    return TrivBspTVDeriveScalar(TV, Dir);
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier trivariate, computes its partial derivative trivariate in   M
* direction Dir.							     M
* Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then:   M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to differentiate.                                   M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Differentiated trivariate in direction Dir. A Bezier   M
*		      trivariate.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVDeriveScalar, TrivBspTVDerive, TrivTVDerive		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVDerive, trivariates                                             M
*****************************************************************************/
TrivTVStruct *TrivBzrTVDerive(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, j, k, l,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct
        *DerivedTV = NULL;

    if (!GlblDeriveScalar && !IsNotRational) {
	TRIV_FATAL_ERROR(TRIV_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    DerivedTV = TrivBzrTVNew(IRIT_MAX(ULength - 1, 1), VLength, WLength,
				     TV -> PType);

	    for (i = 0; i < IRIT_MAX(ULength - 1, 1); i++)
		for (j = 0; j < VLength; j++)
		    for (k = 0; k < WLength; k++) {
			for (l = IsNotRational; l <= MaxCoord; l++)
			    DerivedTV -> Points[l][DERIVED_TV(i, j, k)] =
			        ULength < 2 ? 0.0
				    : (ULength - 1) *
				   (TV -> Points[l][ORIG_TV(i + 1, j, k)] -
				    TV -> Points[l][ORIG_TV(i, j, k)]);
		    }
	    break;
	case TRIV_CONST_V_DIR:
	    DerivedTV = TrivBzrTVNew(ULength, IRIT_MAX(VLength - 1, 1), WLength,
				     TV -> PType);

	    for (i = 0; i < ULength; i++)
		for (j = 0; j < IRIT_MAX(VLength - 1, 1); j++)
		    for (k = 0; k < WLength; k++) {
			for (l = IsNotRational; l <= MaxCoord; l++)
			    DerivedTV -> Points[l][DERIVED_TV(i, j, k)] =
				VLength < 2 ? 0.0
				    : (VLength - 1) *
				   (TV -> Points[l][ORIG_TV(i, j + 1, k)] -
				    TV -> Points[l][ORIG_TV(i, j, k)]);
		    }
	    break;
	case TRIV_CONST_W_DIR:
	    DerivedTV = TrivBzrTVNew(ULength, VLength, IRIT_MAX(WLength - 1, 1),
				     TV -> PType);

	    for (i = 0; i < ULength; i++)
		for (j = 0; j < VLength; j++)
		    for (k = 0; k < IRIT_MAX(WLength - 1, 1); k++) {
			for (l = IsNotRational; l <= MaxCoord; l++)
			    DerivedTV -> Points[l][DERIVED_TV(i, j, k)] =
				WLength < 2 ? 0.0
				    : (WLength - 1) *
				   (TV -> Points[l][ORIG_TV(i, j, k + 1)] -
				    TV -> Points[l][ORIG_TV(i, j, k)]);
		    }
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
	    break;
    }

    return DerivedTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier trivariate, computes its scalar partial derivative          M
* trivariate in direction Dir, each dimension on its own including the       M
* weights, if any.							     M
* Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then:   M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to differentiate.                                   M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Differentiated trivariate in direction Dir. A Bezier   M
*		      trivariate.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVDerive, TrivBspTVDeriveScalar, TrivTVDeriveScalar               M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVDeriveScalar, trivariates                                       M
*****************************************************************************/
TrivTVStruct *TrivBzrTVDeriveScalar(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    TrivTVStruct *TV2;

    GlblDeriveScalar = TRUE;

    TV2 = TrivBzrTVDerive(TV, Dir);

    GlblDeriveScalar = FALSE;

    return TV2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline trivariate, computes its partial derivative trivariate in  M
* direction Dir.							     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to differentiate.                                   M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Differentiated trivariate in direction Dir. A Bspline  M
*		      trivariate.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBzrTVDerive, TrivBspTVDeriveScalar, TrivTVDerive                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVDerive, trivariates                                             M
*****************************************************************************/
TrivTVStruct *TrivBspTVDerive(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, j, k, l, NewUOrder, NewVOrder, NewWOrder,
        NewULength, NewVLength, NewWLength,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength,
	UOrder = TV -> UOrder,
	VOrder = TV -> VOrder,
	WOrder = TV -> WOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    CagdRType **DPoints,
	*UKv = TV -> UKnotVector,
	*VKv = TV -> VKnotVector,
	*WKv = TV -> WKnotVector,
	* const *Points = TV -> Points;
    TrivTVStruct
        *DerivedTV = NULL;

    if (!IsNotRational) {
	TRIV_FATAL_ERROR(TRIV_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    NewULength = UOrder < 2 ? ULength : ULength - 1;
	    NewUOrder = IRIT_MAX(UOrder - 1, 1);
	    DerivedTV = TrivBspTVNew(NewULength, VLength, WLength,
				     NewUOrder, VOrder, WOrder, TV -> PType);
	    CAGD_GEN_COPY(DerivedTV -> UKnotVector, &UKv[UOrder < 2 ? 0 : 1],
			  sizeof(CagdRType) * (NewULength + NewUOrder));
	    CAGD_GEN_COPY(DerivedTV -> VKnotVector, VKv,
			  sizeof(CagdRType) * (VLength + VOrder));
	    CAGD_GEN_COPY(DerivedTV -> WKnotVector, WKv,
			  sizeof(CagdRType) * (WLength + WOrder));
	    DPoints = DerivedTV -> Points;

	    for (i = 0; i < NewULength; i++)
		for (j = 0; j < VLength; j++)
		    for (k = 0; k < WLength; k++) {
			CagdRType
			    Denom = UKv[i + UOrder] - UKv[i + 1];

			if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
			    Denom = IRIT_UEPS;

			for (l = IsNotRational; l <= MaxCoord; l++) {
			    DPoints[l][DERIVED_TV(i, j, k)] =
				UOrder < 2 ? 0.0
				   : (UOrder - 1) *
				      (Points[l][ORIG_TV(i + 1, j, k)] -
				       Points[l][ORIG_TV(i, j, k)]) / Denom;
			}
		    }
	    break;
	case TRIV_CONST_V_DIR:
	    NewVLength = VOrder < 2 ? VLength : VLength - 1;
	    NewVOrder = IRIT_MAX(VOrder - 1, 1);
	    DerivedTV = TrivBspTVNew(ULength, NewVLength, WLength,
				     UOrder, NewVOrder, WOrder, TV -> PType);
	    CAGD_GEN_COPY(DerivedTV -> UKnotVector, UKv,
			  sizeof(CagdRType) * (ULength + UOrder));
	    CAGD_GEN_COPY(DerivedTV -> VKnotVector, &VKv[VOrder < 2 ? 0 : 1],
			  sizeof(CagdRType) * (NewVLength + NewVOrder));
	    CAGD_GEN_COPY(DerivedTV -> WKnotVector, WKv,
			  sizeof(CagdRType) * (WLength + WOrder));
	    DPoints = DerivedTV -> Points;

	    for (i = 0; i < ULength; i++)
		for (j = 0; j < NewVLength; j++)
		    for (k = 0; k < WLength; k++) {
			CagdRType
			    Denom = VKv[j + VOrder] - VKv[j + 1];

			if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
			    Denom = IRIT_UEPS;

			for (l = IsNotRational; l <= MaxCoord; l++) {
			    DPoints[l][DERIVED_TV(i, j, k)] =
				VOrder < 2 ? 0.0
				   : (VOrder - 1) *
				      (Points[l][ORIG_TV(i, j + 1, k)] -
				       Points[l][ORIG_TV(i, j, k)]) / Denom;
		    }
		}
	    break;
	case TRIV_CONST_W_DIR:
	    NewWLength = WOrder < 2 ? WLength : WLength - 1;
	    NewWOrder = IRIT_MAX(WOrder - 1, 1);
	    DerivedTV = TrivBspTVNew(ULength, VLength, NewWLength,
				     UOrder, VOrder, NewWOrder, TV -> PType);
	    CAGD_GEN_COPY(DerivedTV -> UKnotVector, UKv,
			  sizeof(CagdRType) * (ULength + UOrder));
	    CAGD_GEN_COPY(DerivedTV -> VKnotVector, VKv,
			  sizeof(CagdRType) * (VLength + VOrder));
	    CAGD_GEN_COPY(DerivedTV -> WKnotVector, &WKv[WOrder < 2 ? 0 : 1],
			  sizeof(CagdRType) * (NewWLength + NewWOrder));
	    DPoints = DerivedTV -> Points;

	    for (i = 0; i < ULength; i++)
		for (j = 0; j < VLength; j++)
		    for (k = 0; k < NewWLength; k++) {
			CagdRType
			    Denom = WKv[k + WOrder] - WKv[k + 1];

			if (IRIT_APX_EQ_EPS(Denom, 0.0, IRIT_UEPS))
			    Denom = IRIT_UEPS;

			for (l = IsNotRational; l <= MaxCoord; l++) {
			    DPoints[l][DERIVED_TV(i, j, k)] =
				WOrder < 2 ? 0.0
				   : (WOrder - 1) *
				      (Points[l][ORIG_TV(i, j, k + 1)] -
				       Points[l][ORIG_TV(i, j, k)]) / Denom;
		    }
		}
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
	    break;
    }

    return DerivedTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bspline trivariate, computes its partial derivative trivariate in  M
* direction Dir, each dimension on its own including the weights, if any.    M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)) / (Kv(i + k) - Kv(i + 1)), i = 0 to k-2.  V
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       Trivariate to differentiate.                                   M
*   Dir:      Direction of differentiation. Either U or V or W.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Differentiated trivariate in direction Dir. A Bspline  M
*		      trivariate.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBspTVDerive, TrivBzrTVDeriveScalar, TrivTVDeriveScalar               M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVDeriveScalar, trivariates                                       M
*****************************************************************************/
TrivTVStruct *TrivBspTVDeriveScalar(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    TrivTVStruct *TV2;
    
    GlblDeriveScalar = TRUE;

    TV2 = TrivBspTVDerive(TV, Dir);

    GlblDeriveScalar = FALSE;

    return TV2;
}
