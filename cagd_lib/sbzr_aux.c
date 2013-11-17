/******************************************************************************
* SBzr-Aux.c - Bezier surface auxilary routines.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, July. 90.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"

/* Define some marcos to make some of the routines below look better. They  */
/* calculate the index of the U, V point of the control mesh in Points.	    */
#define DERIVED_SRF(U, V)	CAGD_MESH_UV(DerivedSrf, U, V)
#define INT_SRF(U, V)		CAGD_MESH_UV(IntSrf, U, V)
#define RAISED_SRF(U, V)	CAGD_MESH_UV(RaisedSrf, U, V)
#define SRF(U, V)		CAGD_MESH_UV(Srf, U, V)
#define VEC_FIELD_TRIES		10
#define VEC_FIELD_START_STEP	1e-6

IRIT_STATIC_DATA CagdBType
    GlblDeriveScalar = FALSE;

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply Bezier subdivision to the given curve at parameter value t, and    M
* save the result in data LPoints/RPoints.  Note this function could also be M
* called from a B-spline curve with a Bezier knot sequence.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Points:            To subdivide at parametr value t.                     M
*   LPoints, RPoints:  Where the results are kept.			     M
*   ULength, VLength:  Of this Bezier surface, dimensions of Points.	     M
*   PType:	       Points types we have here.			     M
*   t:                 Parameter value to subdivide curve at.                M
*   Dir:               Direction of subdivision.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvSubdivAtParam, BspCrvSubdivCtlPoly, BzrCrvSubdivCtlPolyStep        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfSubdivCtlMesh                                                      M
*****************************************************************************/
void BzrSrfSubdivCtlMesh(CagdRType * const *Points,
			 CagdRType **LPoints,
			 CagdRType **RPoints,
			 int ULength,
			 int VLength,
			 CagdPointType PType,
			 CagdRType t,
			 CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_PT(PType);
    int i, Row, Col,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType);
    CagdRType *Pts[CAGD_MAX_PT_SIZE], *LPts[CAGD_MAX_PT_SIZE],
	*RPts[CAGD_MAX_PT_SIZE];

    IRIT_GEN_COPY(Pts, Points, sizeof(CagdRType *) * CAGD_MAX_PT_SIZE);
    IRIT_GEN_COPY(LPts, LPoints, sizeof(CagdRType *) * CAGD_MAX_PT_SIZE);
    IRIT_GEN_COPY(RPts, RPoints, sizeof(CagdRType *) * CAGD_MAX_PT_SIZE);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    for (Row = 0; Row < VLength; Row++) {
	        BzrCrvSubdivCtlPoly(Pts, LPts, RPts, ULength, PType, t);

		for (i = IsNotRational; i <= MaxCoord; i++) {
		    Pts[i] += ULength;
		    LPts[i] += ULength;
		    RPts[i] += ULength;
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    for (Col = 0; Col < ULength; Col++) {
	        BzrCrvSubdivCtlPolyStep(Pts, LPts, RPts, VLength,
					PType, t, ULength);

		for (i = IsNotRational; i <= MaxCoord; i++) {
		    Pts[i]++;
		    LPts[i]++;
		    RPts[i]++;
		}
	    }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a Bezier surface - subdivides it into two sub-surfaces at the given  M
* parametric value.                                                          M
*   Returns pointer to first surface in a list of two subdivided surfaces.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:      To subdivide at parameter value t.                             M
*   t:        Parameter value to subdivide Srf at.                           M
*   Dir:      Direction of subdivision. Either U or V.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A list of the two subdivided surfaces.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfSubdivAtParam, BspSrfSubdivAtParam, TrimSrfSubdivAtParam          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfSubdivAtParam, subdivision, refinement                             M
*****************************************************************************/
CagdSrfStruct *BzrSrfSubdivAtParam(const CagdSrfStruct *Srf,
				   CagdRType t,
				   CagdSrfDirType Dir)
{
    int ULength = Srf -> ULength,
	VLength = Srf -> VLength;
    CagdSrfStruct
	*RSrf = BzrSrfNew(ULength, VLength, Srf ->PType),
	*LSrf = BzrSrfNew(ULength, VLength, Srf ->PType);

    BzrSrfSubdivCtlMesh(Srf -> Points, LSrf -> Points, RSrf -> Points,
			ULength, VLength, Srf -> PType, t, Dir);

    LSrf -> Pnext = RSrf;

    CAGD_PROPAGATE_ATTR(LSrf, Srf);
    CAGD_PROPAGATE_ATTR(RSrf, Srf);

    return LSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bezier surface, identical to the original but with one       M
* degree higher, in the requested direction Dir.                             M
* Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then:   M
*		       i	    k-i					     V
* Q(0) = P(0), Q(i) = --- P(i-1) + (---) P(i), Q(k) = P(k-1).		     V
*		       k	     k					     V
* This is applied to all rows/cols of the surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise it degree by one.                                   M
*   Dir:        Direction to degree raise. Either U or V.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with one degree higher in direction Dir,     M
*                     representing the same geometry as Srf.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BspSrfDegreeRaise, TrimSrfDegreeRaise,               M
*   PwrSrfDegreeRaise, PwrSrfDegreeRaiseN				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfDegreeRaise, degree raising                                        M
*****************************************************************************/
CagdSrfStruct *BzrSrfDegreeRaise(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, Row, Col,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
	*RaisedSrf = NULL;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    RaisedSrf = BzrSrfNew(ULength + 1, VLength, Srf -> PType);

	    for (Row = 0; Row < VLength; Row++) {
		for (j = IsNotRational; j <= MaxCoord; j++)	    /* Q(0). */
		    RaisedSrf -> Points[j][RAISED_SRF(0, Row)] =
		       Srf -> Points[j][SRF(0, Row)];

		for (i = 1; i < ULength; i++)			    /* Q(i). */
		    for (j = IsNotRational; j <= MaxCoord; j++)
			RaisedSrf -> Points[j][RAISED_SRF(i, Row)] =
			    Srf -> Points[j][SRF(i - 1, Row)] *
			    		         (i / ((CagdRType) ULength)) +
			    Srf -> Points[j][SRF(i, Row)] *
	    			     ((ULength - i) / ((CagdRType) ULength));

		for (j = IsNotRational; j <= MaxCoord; j++)	    /* Q(k). */
		    RaisedSrf -> Points[j][RAISED_SRF(ULength, Row)] =
			Srf -> Points[j][SRF(ULength - 1, Row)];
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    RaisedSrf = BzrSrfNew(ULength, VLength + 1, Srf -> PType);

	    for (Col = 0; Col < ULength; Col++) {
		for (j = IsNotRational; j <= MaxCoord; j++)	    /* Q(0). */
		    RaisedSrf -> Points[j][RAISED_SRF(Col, 0)] =
		       Srf -> Points[j][SRF(Col, 0)];

		for (i = 1; i < VLength; i++)			    /* Q(i). */
		    for (j = IsNotRational; j <= MaxCoord; j++)
			RaisedSrf -> Points[j][RAISED_SRF(Col, i)] =
			    Srf -> Points[j][SRF(Col, i - 1)] *
			    		         (i / ((CagdRType) VLength)) +
			    Srf -> Points[j][SRF(Col, i)] *
				     ((VLength - i) / ((CagdRType) VLength));

		for (j = IsNotRational; j <= MaxCoord; j++)	    /* Q(k). */
		    RaisedSrf -> Points[j][RAISED_SRF(Col, VLength)] =
			Srf -> Points[j][SRF(Col, VLength - 1)];
            }
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    CAGD_PROPAGATE_ATTR(RaisedSrf, Srf);

    return RaisedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bezier surface, identical to the original but with higher    M
* degrees, as prescribed by NewUOrder, NewVOrder.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise its degrees.                                        M
*   NewUOrder:  New U order of Srf.					     M
*   NewVOrder:  New V order of Srf.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with higher degrees as prescribed by	     M
*                     NewUOrder/NewVOrder.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BzrSrfDegreeRaise, TrimSrfDegreeRaise,               M
*   BspSrfDegreeRaise, BzrSrfDegreeRaiseN, CagdSrfDegreeRaiseN,              M
*   PwrSrfDegreeRaise, PwrSrfDegreeRaiseN				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfDegreeRaiseN, degree raising                                       M
*****************************************************************************/
CagdSrfStruct *BzrSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				  int NewUOrder,
				  int NewVOrder)
{
    int i, j, RaisedUOrder, RaisedVOrder,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *RaisedSrf, *UnitSrf;

    if (NewUOrder < UOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedUOrder = NewUOrder - UOrder + 1;

    if (NewVOrder < VOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    RaisedVOrder = NewVOrder - VOrder + 1;

    UnitSrf = BzrSrfNew(RaisedUOrder, RaisedVOrder,
			CAGD_MAKE_PT_TYPE(FALSE, MaxCoord));
    for (i = 1; i <= MaxCoord; i++)
	for (j = 0; j < RaisedUOrder * RaisedVOrder; j++)
	    UnitSrf -> Points[i][j] = 1.0;

    RaisedSrf = BzrSrfMult(Srf, UnitSrf);

    CagdSrfFree(UnitSrf);

    CAGD_PROPAGATE_ATTR(RaisedSrf, Srf);

    return RaisedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new power basis surface, identical to the original but with one  M
* degree higher, in the requested direction Dir.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise it degree by one.                                   M
*   Dir:        Direction to degree raise. Either U or V.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with one degree higher in direction Dir,     M
*                     representing the same geometry as Srf.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BzrSrfDegreeRaise, TrimSrfDegreeRaise,               M
*   BspSrfDegreeRaise, BzrSrfDegreeRaiseN, CagdSrfDegreeRaiseN,              M
*   PwrSrfDegreeRaiseN							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrSrfDegreeRaise, degree raising                                        M
*****************************************************************************/
CagdSrfStruct *PwrSrfDegreeRaise(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    return PwrSrfDegreeRaiseN(Srf, Srf -> UOrder + 1, Srf -> VOrder);
	case CAGD_CONST_V_DIR:
	    return PwrSrfDegreeRaiseN(Srf, Srf -> UOrder, Srf -> VOrder + 1);
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new power basis surface, identical to the original but with      M
* higher degrees, as prescribed by NewUOrder, NewVOrder.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To raise its degrees.                                        M
*   NewUOrder:  New U order of Srf.					     M
*   NewVOrder:  New V order of Srf.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A surface with higher degrees as prescribed by	     M
*                     NewUOrder/NewVOrder.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDegreeRaise, BzrSrfDegreeRaise, TrimSrfDegreeRaise,               M
*   BspSrfDegreeRaise, BzrSrfDegreeRaiseN, CagdSrfDegreeRaiseN,              M
*   PwrSrfDegreeRaise							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   PwrSrfDegreeRaiseN, degree raising                                       M
*****************************************************************************/
CagdSrfStruct *PwrSrfDegreeRaiseN(const CagdSrfStruct *Srf,
				  int NewUOrder,
				  int NewVOrder)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int j, Row,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *RaisedSrf;

    if (NewUOrder < UOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }
    if (NewVOrder < VOrder) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_ORDER);
	return NULL;
    }

    RaisedSrf = PwrSrfNew(NewUOrder, NewVOrder, Srf -> PType);

    for (Row = 0; Row < VOrder; Row++) {
        for (j = IsNotRational; j <= MaxCoord; j++) {
	    IRIT_GEN_COPY(&RaisedSrf -> Points[j][RAISED_SRF(0, Row)],
		     &Srf -> Points[j][SRF(0, Row)],
		     UOrder * sizeof(CagdRType));

	    if (NewUOrder > UOrder)
	        IRIT_ZAP_MEM(&RaisedSrf -> Points[j][SRF(UOrder, Row)],
			(NewUOrder - UOrder) * sizeof(CagdRType));
	}
    }

    for (Row = VOrder; Row < NewVOrder; Row++) {
        for (j = IsNotRational; j <= MaxCoord; j++)
	    IRIT_ZAP_MEM(&RaisedSrf -> Points[j][SRF(0, Row)],
		    NewUOrder * sizeof(CagdRType));
    }

    CAGD_PROPAGATE_ATTR(RaisedSrf, Srf);

    return RaisedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface equal to the given surface, differentiated once in   M
* the direction Dir.							     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
* This is applied to all rows/cols of the surface.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To differentiate.                                            M
*   Dir:        Direction of differentiation. Either U or V.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Differentiated surface.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfDerive, BspSrfDerive, BzrSrfDeriveRational, BspSrfDeriveRational  M
*   BzrSrfDeriveScalar							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfDerive, derivatives, partial derivatives                           M
*****************************************************************************/
CagdSrfStruct *BzrSrfDerive(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf);
    int i, j, Row, Col,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct
        *DerivedSrf = NULL;

    if (!GlblDeriveScalar && !IsNotRational)
	return BzrSrfDeriveRational(Srf, Dir);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    DerivedSrf = BzrSrfNew(IRIT_MAX(ULength - 1, 1), VLength,
				   Srf -> PType);

	    for (Row = 0; Row < VLength; Row++)
		for (i = 0; i < IRIT_MAX(ULength - 1, 1); i++)
		    for (j = IsNotRational; j <= MaxCoord; j++)
			DerivedSrf -> Points[j][DERIVED_SRF(i, Row)] =
			    ULength < 2 ? 0.0
					: (ULength - 1) *
					   (Srf -> Points[j][SRF(i + 1, Row)] -
					    Srf -> Points[j][SRF(i, Row)]);
	    break;
	case CAGD_CONST_V_DIR:
	    DerivedSrf = BzrSrfNew(ULength, IRIT_MAX(VLength - 1, 1),
				   Srf -> PType);

	    for (Col = 0; Col < ULength; Col++)
		for (i = 0; i < IRIT_MAX(VLength - 1, 1); i++)
		    for (j = IsNotRational; j <= MaxCoord; j++)
			DerivedSrf -> Points[j][DERIVED_SRF(Col, i)] =
			    VLength < 2 ? 0.0
					: (VLength - 1) *
					   (Srf -> Points[j][SRF(Col, i + 1)] -
					    Srf -> Points[j][SRF(Col, i)]);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return DerivedSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new surface equal to the given surface, differentiated once in   M
* the direction Dir.							     M
*   Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then: M
* Q(i) = (k - 1) * (P(i+1) - P(i)), i = 0 to k-2.			     V
* This is applied to all rows/cols of the surface.			     M
*   For a Euclidean surface this is the same as CagdCrvDerive but for a      M
* rational surface the returned surface is not the vector field but simply   M
* the derivatives of all the surface's coefficients, including the weights.  M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        To differentiate.                                            M
*   Dir:        Direction of tangent vector. Either U or V.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Differentiated curve.                                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrfDerive, CagdSrfDerive, BzrSrfDeriveRational, BspSrfDeriveRational  M
*   BspSrfDerive, BspSrfDeriveScalar, CagdSrfDeriveScalar		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfDeriveScalar, derivatives                                          M
*****************************************************************************/
CagdSrfStruct *BzrSrfDeriveScalar(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    CagdSrfStruct *TSrf;

    GlblDeriveScalar = TRUE;

    TSrf = BzrSrfDerive(Srf, Dir);

    GlblDeriveScalar = FALSE;

    return TSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bezier surface, equal to the integral of the given Bezier    M
* srf.		                                                             M
* The given Bezier surface should be nonrational.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Surface to integrate.                                       M
*   Dir:        Direction of integration. Either U or V.	             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Integrated surface.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfIntegrate, BzrCrvIntegrate, CagdSrfIntegrate                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfIntegrate, integrals                                               M
*****************************************************************************/
CagdSrfStruct *BzrSrfIntegrate(const CagdSrfStruct *Srf, CagdSrfDirType Dir)
{
    int i, j, k, Row, Col,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdSrfStruct *IntSrf;

    if (CAGD_IS_RATIONAL_SRF(Srf))
	CAGD_FATAL_ERROR(CAGD_ERR_RATIONAL_NO_SUPPORT);

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    IntSrf = BzrSrfNew(ULength + 1, VLength, Srf -> PType);

	    for (k = 1; k <= MaxCoord; k++) {
		CagdRType *R,
		    *Points = Srf -> Points[k],
		    *IntPoints = IntSrf -> Points[k];

		for (Row = 0; Row < VLength; Row++) {
		    for (j = 0; j < ULength + 1; j++) {
			R = &IntPoints[INT_SRF(j, Row)];
			*R = 0.0;
			for (i = 0; i < j; i++)
			    *R += Points[SRF(i, Row)];
			*R /= ULength;
		    }
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    IntSrf = BzrSrfNew(ULength, VLength + 1, Srf -> PType);

	    for (k = 1; k <= MaxCoord; k++) {
		CagdRType *R,
		    *Points = Srf -> Points[k],
		    *IntPoints = IntSrf -> Points[k];

		for (Col = 0; Col < ULength; Col++) {
		    for (j = 0; j < VLength + 1; j++) {
			R = &IntPoints[INT_SRF(Col, j)];
			*R = 0.0;
			for (i = 0; i < j; i++)
			    *R += Points[SRF(Col, i)];
			*R /= VLength;
		    }
		}
	    }
	    break;
	default:
	    IntSrf = NULL;
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return IntSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the (unit) tangent to a surface at a given parametric location   M
( u, v) and given direction Dir.                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Bezier surface to evaluate (unit) tangent vector for.        M
*   u, v:       Parametric location of required (unit) tangent.              M
*   Dir:        Direction of tangent vector. Either U or V.                  M
*   Normalize:  If TRUE, attempt is made to normalize the returned vector.   M
*               If FALSE, length is a function of given parametrization.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the (unit)        M
*                     tangent information.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfTangent, BspSrfTangent					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfTangent, tangent                                                   M
*****************************************************************************/
CagdVecStruct *BzrSrfTangent(const CagdSrfStruct *Srf,
			     CagdRType u,
			     CagdRType v,
			     CagdSrfDirType Dir,
			     CagdBType Normalize)
{
    CagdVecStruct
	*Tangent = NULL;
    CagdCrvStruct *Crv;

    switch (Dir) {
	case CAGD_CONST_V_DIR:
	    Crv = BzrSrfCrvFromSrf(Srf, v, Dir);
	    Tangent = BzrCrvTangent(Crv, u, Normalize);
	    CagdCrvFree(Crv);
	    break;
	case CAGD_CONST_U_DIR:
	    Crv = BzrSrfCrvFromSrf(Srf, u, Dir);
	    Tangent = BzrCrvTangent(Crv, v, Normalize);
	    CagdCrvFree(Crv);
	    break;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    return Tangent;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluate the (unit) normal of a surface at a given parametric location.    M
*   If we fail to compute the normal at given location we retry by moving a  M
* tad.                                                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Bezier surface to evaluate (unit) normal vector for.         M
*   u, v:       Parametric location of required (unit) normal.               M
*   Normalize:  If TRUE, attempt is made to normalize the returned vector.   M
*               If FALSE, length is a function of given parametrization.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:  A pointer to a static vector holding the (unit) normal M
*                     information.                                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, BspSrfNormal, SymbSrfNormalSrf			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfNormal, normal                                                     M
*****************************************************************************/
CagdVecStruct *BzrSrfNormal(const CagdSrfStruct *Srf,
			    CagdRType u,
			    CagdRType v,
			    CagdBType Normalize)
{
    IRIT_STATIC_DATA CagdVecStruct Normal;
    CagdVecStruct *V, T1, T2;

    V = BzrSrfTangent(Srf, u, v, CAGD_CONST_U_DIR, FALSE);
    if (CAGD_SQR_LEN_VECTOR(*V) < IRIT_UEPS)
	V = BzrSrfTangent(Srf,
			  u > 0.5 ? u - IRIT_EPS : u + IRIT_EPS,
			  v > 0.5 ? v - IRIT_EPS : v + IRIT_EPS,
			  CAGD_CONST_U_DIR, FALSE);
    CAGD_COPY_VECTOR(T1, *V);

    V = BzrSrfTangent(Srf, u, v, CAGD_CONST_V_DIR, FALSE);
    if (CAGD_SQR_LEN_VECTOR(*V) < IRIT_UEPS)
	V = BzrSrfTangent(Srf,
			  u > 0.5 ? u - IRIT_EPS : u + IRIT_EPS,
			  v > 0.5 ? v - IRIT_EPS : v + IRIT_EPS,
			  CAGD_CONST_V_DIR, FALSE);
    CAGD_COPY_VECTOR(T2, *V);

    /* The normal is the cross product of T1 and T2: */
    Normal.Vec[0] = T1.Vec[1] * T2.Vec[2] - T1.Vec[2] * T2.Vec[1];
    Normal.Vec[1] = T1.Vec[2] * T2.Vec[0] - T1.Vec[0] * T2.Vec[2];
    Normal.Vec[2] = T1.Vec[0] * T2.Vec[1] - T1.Vec[1] * T2.Vec[0];

    if (Normalize)
	CAGD_NORMALIZE_VECTOR_MSG_ZERO(Normal);    /* Normalize the vector. */

    return &Normal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Evaluates the unit normals of a surface at a mesh defined by subdividing   M
* the parametric space into a grid of size UFineNess by VFineNess.	     M
*   The normals are saved in a linear CagdVecStruct vector which is          M
* allocated dynamically. Data is saved u inc. first.			     M
*   This routine is much faster than evaluating normal for each point,       M
* individually.                                                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          To compute normals on a grid of its parametric domain.     M
*   UFineNess:    U Fineness of imposed grid on Srf's parametric domain.     M
*   VFineNess:    V Fineness of imposed grid on Srf's parametric domain.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdVecStruct *:   An vector of unit normals (u increments first).       M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfNormal, BspSrfNormal, SymbSrfNormalSrf, BspSrfMeshNormals	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfMeshNormals, normal                                                M
*****************************************************************************/
CagdVecStruct *BzrSrfMeshNormals(const CagdSrfStruct *Srf,
				 int UFineNess,
				 int VFineNess)
{
    int i, j,
	UFineNess1 = UFineNess - 1,
	VFineNess1 = VFineNess - 1,
	MeshSize = UFineNess * VFineNess;
    CagdVecStruct *PtNrmlPtr, *PtNrml,
	*Nl1 = NULL;

    CagdSrfEffiNrmlPrelude(Srf);

    PtNrmlPtr = PtNrml = CagdVecArrayNew(MeshSize);
    for (i = 0; i < UFineNess; i++) {
	for (j = 0; j < VFineNess; j++) {
	    Nl1 = CagdSrfEffiNrmlEval(((CagdRType) i) / UFineNess1,
				      ((CagdRType) j) / VFineNess1, FALSE);

	    if (IRIT_PT_SQR_LENGTH(Nl1 -> Vec) < IRIT_SQR(IRIT_UEPS)) {
		int k = 0;
		CagdRType U, V,
		    Step = VEC_FIELD_START_STEP;

		U = ((CagdRType) i) / UFineNess1;
		V = ((CagdRType) j) / VFineNess1;
		while (IRIT_PT_SQR_LENGTH(Nl1 -> Vec) < IRIT_SQR(IRIT_UEPS) &&
		       k++ < VEC_FIELD_TRIES) {
		    U += U < 0.5 ? Step : -Step;
		    V += V < 0.5 ? Step : -Step;
		    Step *= 2.0;

		    Nl1 = CagdSrfEffiNrmlEval(U, V, FALSE);
		}
	    }

	    CAGD_COPY_VECTOR(*PtNrmlPtr, *Nl1);
	    CAGD_NORMALIZE_VECTOR(*PtNrmlPtr);
	    PtNrmlPtr++;
	}
    }

    CagdSrfEffiNrmlPostlude();

    return PtNrml;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a bezier surface into a Bspline surface by adding open end        M
* knot vector with no interior knots.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Bezier surface to convert to a Bspline surface.               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A Bspline surface representing same geometry as Srf.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtBsp2BzrSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBzr2BspSrf, conversion                                          M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtBzr2BspSrf(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *BspSrf;

    if (Srf -> GType != CAGD_SBEZIER_TYPE) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SRF);
	return NULL;
    }

    BspSrf = CagdSrfCopy(Srf);

    BspSrf -> UOrder = BspSrf -> ULength;
    BspSrf -> VOrder = BspSrf -> VLength;
    BspSrf -> UKnotVector = BspKnotUniformOpen(BspSrf -> ULength,
						    BspSrf -> UOrder, NULL);
    BspSrf -> VKnotVector = BspKnotUniformOpen(BspSrf -> VLength,
						    BspSrf -> VOrder, NULL);
    BspSrf -> GType = CAGD_SBSPLINE_TYPE;

    CAGD_PROPAGATE_ATTR(BspSrf, Srf);

    return BspSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Convert a Bspline surface into a set of Bezier surfaces by subdiving the   M
* Bspline surface at all its internal knots.				     M
*   Returned is a list of Bezier surface.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       Bspline surface to convert to a Bezier surface.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A list of Bezier surfaces representing same geometry   M
*                     as Srf.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCnvrtBzr2BspSrf                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCnvrtBsp2BzrSrf, conversion                                          M
*****************************************************************************/
CagdSrfStruct *CagdCnvrtBsp2BzrSrf(const CagdSrfStruct *CSrf)
{
    CagdSrfStruct *Srf;

    if (CSrf -> GType != CAGD_SBSPLINE_TYPE) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SRF);
	return NULL;
    }

    if (!BspSrfHasOpenEC(CSrf))
	Srf = BspSrfOpenEnd(CSrf);
    else
	Srf = CagdSrfCopy(CSrf);

    if (Srf -> ULength > Srf -> UOrder) {
	CagdRType
	    t = Srf -> UKnotVector[(Srf -> ULength + Srf -> UOrder) >> 1];
	CagdSrfStruct *Srf1Bzrs, *Srf2Bzrs,
	    *Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_U_DIR),
	    *Srf2 = Srf1 -> Pnext;

	Srf1 -> Pnext = NULL;
	CagdSrfFree(Srf);

	Srf1Bzrs = CagdCnvrtBsp2BzrSrf(Srf1);
	Srf2Bzrs = CagdCnvrtBsp2BzrSrf(Srf2);

	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	return CagdListAppend(Srf1Bzrs, Srf2Bzrs);
    }
    else if (Srf -> VLength > Srf -> VOrder) {
	CagdRType
	    t = Srf -> VKnotVector[(Srf -> VLength + Srf -> VOrder) >> 1];
	CagdSrfStruct *Srf1Bzrs, *Srf2Bzrs,
	    *Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_V_DIR),
	    *Srf2 = Srf1 -> Pnext;

	Srf1 -> Pnext = NULL;
	CagdSrfFree(Srf);

	Srf1Bzrs = CagdCnvrtBsp2BzrSrf(Srf1);
	Srf2Bzrs = CagdCnvrtBsp2BzrSrf(Srf2);

	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	return CagdListAppend(Srf1Bzrs, Srf2Bzrs);
    }
    else {
        CagdRType UMin, UMax, VMin, VMax;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	Srf -> GType = CAGD_SBEZIER_TYPE;
	IritFree(Srf -> UKnotVector);
	IritFree(Srf -> VKnotVector);
	Srf -> UKnotVector = NULL;
	Srf -> VKnotVector = NULL;

	AttrSetUVAttrib(&Srf -> Attr, "BspDomainMin", UMin, VMin);
	AttrSetUVAttrib(&Srf -> Attr, "BspDomainMax", UMax, VMax);

	return Srf;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Apply the Moebius transformation to a ration Bezier surface.             M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       Surface to apply the Moebius transformation to.              M
*   c:          The scaling coefficient - c^n is the ratio between the first M
*	        and last weight of the surface, along each row or column.    M
*		If c == 0, the first and last weights are made equal, in the M
*		first row/column.				             M
*   Dir:        Direction to apply the Moebius transformation, row or col.   M
*		If Dir == CAGD_BOTH_DIR, the transformation is applied to    M
*		both the row and column directions, in this order.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The modified surface with the same shape but different M
*		speeds.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrvMoebiusTransform, BspSrfMoebiusTransform                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrfMoebiusTransform                                                   M
*****************************************************************************/
CagdSrfStruct *BzrSrfMoebiusTransform(const CagdSrfStruct *CSrf,
				      CagdRType c,
				      CagdSrfDirType Dir)
{
    int i, j, l,
	MaxCoord = CAGD_NUM_OF_PT_COORD(CSrf -> PType),
	UOrder = -1,
	VOrder = -1;
    CagdRType c0, **Points,
	MaxW = IRIT_UEPS;    
    CagdSrfStruct *T1Srf, *T2Srf, *Srf;

    if (CSrf -> GType != CAGD_SBEZIER_TYPE) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SRF);
	return NULL;
    }

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	case CAGD_CONST_V_DIR:
	    UOrder = CSrf -> UOrder;
	    VOrder = CSrf -> VOrder;
	    break;
	case CAGD_BOTH_DIR:
	    T1Srf = BzrSrfMoebiusTransform(CSrf, c, CAGD_CONST_U_DIR);
	    T2Srf = BzrSrfMoebiusTransform(T1Srf, c, CAGD_CONST_V_DIR);
	    CagdSrfFree(T1Srf);
	    return T2Srf;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    if (!CAGD_IS_RATIONAL_SRF(CSrf)) {
        if (c == 1.0)
	    return CagdSrfCopy(CSrf);
	else
	    Srf = CagdCoerceSrfTo(CSrf, CAGD_MAKE_PT_TYPE(TRUE, MaxCoord), FALSE);
    }
    else
        Srf = CagdSrfCopy(CSrf);
    Points = Srf -> Points;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    if (Points[0][0] == 0 ||
		Points[0][CAGD_MESH_UV(Srf, UOrder - 1, 0)] == 0) {
	        CAGD_FATAL_ERROR(CAGD_ERR_W_ZERO);
		return NULL;
	    }
	    if (c == 0.0) {
	        c = Points[0][0] / Points[0][CAGD_MESH_UV(Srf, UOrder - 1, 0)];
		c = pow(IRIT_FABS(c), 1.0 / (UOrder - 1.0));
	    }

	    for (c0 = c, i = 1; i < UOrder; i++) {
	        for (j = 0; j < VOrder; j++)
		    for (l = 0; l <= MaxCoord; l++)
		        Points[l][CAGD_MESH_UV(Srf, i, j)] *= c;
		c *= c0;
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    if (Points[0][0] == 0 ||
		Points[0][CAGD_MESH_UV(Srf, 0, VOrder - 1)] == 0) {
	        CAGD_FATAL_ERROR(CAGD_ERR_W_ZERO);
		return NULL;
	    }
	    if (c == 0.0) {
	        c = Points[0][0] / Points[0][CAGD_MESH_UV(Srf, 0, VOrder - 1)];
		c = pow(IRIT_FABS(c), 1.0 / (VOrder - 1.0));
	    }

	    for (c0 = c, i = 1; i < VOrder; i++) {
	        for (j = 0; j < UOrder; j++)
		    for (l = 0; l <= MaxCoord; l++)
		        Points[l][CAGD_MESH_UV(Srf, j, i)] *= c;
		c *= c0;
	    }
	    break;
	default:
	    assert(0);
    }

    /* Normalize all weights so largest has magnitude of one. */
    for (i = 0; i < UOrder * VOrder; i++) {
	if (MaxW < IRIT_FABS(Points[0][i]))
	    MaxW = IRIT_FABS(Points[0][i]);
    }
    for (i = 0; i < UOrder * VOrder; i++) {
	for (j = 0; j <= MaxCoord; j++)
	    Points[j][i] /= MaxW;
    }

    return Srf;
}
