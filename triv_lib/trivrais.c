/******************************************************************************
* TrivRais.c - Degree raising from trivariate functions.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, October 1994.				      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "triv_loc.h"

#define TRIV_DEGREE_RAISE_BLOSSOM_BSP      /* Degree raising via blossoming. */
/* #define TRIV_DEGREE_RAISE_BLOSSOM_BZR      Degree raising via blossoming. */

/* Define some marcos to make some of the routines below look better. They  */
/* calculate the index of U, V, W point of the control mesh in Points.	    */
#define RAISED_TV(U, V, W)	TRIV_MESH_UVW(RaisedTV, U, V, W)
#define ORIG_TV(U, V, W)	TRIV_MESH_UVW(TV, U, V, W)

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new trivariate representing the same curve as TV but with its    M
* degree raised by one, in Dir direction.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To raise its degree.                                           M
*   Dir:      Direction of degree raising. Either U, V or W.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A trivariate with same geometry as TV but with one      M
*                    degree higher.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVDegreeRaise, degree raising                                        M
*****************************************************************************/
TrivTVStruct *TrivTVDegreeRaise(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    switch (TV -> GType) {
	case TRIV_TVBEZIER_TYPE:
#	    ifdef TRIV_DEGREE_RAISE_BLOSSOM_BZR
		return TrivTVBlossomDegreeRaise(TV, Dir);
#	    else
	        return TrivBzrTVDegreeRaise(TV, Dir);
#	    endif /* TRIV_DEGREE_RAISE_BLOSSOM_BZR */
	case TRIV_TVBSPLINE_TYPE:
#	    ifdef TRIV_DEGREE_RAISE_BLOSSOM_BSP
		return TrivTVBlossomDegreeRaise(TV, Dir);
#	    else
	        return TrivBspTVDegreeRaise(TV, Dir);
#	    endif /* TRIV_DEGREE_RAISE_BLOSSOM_BSP */
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_GEOM);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new trivariate representing the same curve as TV but with its    M
* degree raised to a NewOrder in Dir direction.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:       To raise its degree.                                           M
*   Dir:      Direction of degree raising. Either U, V or W.		     M
*   NewOrder: New order to raise TV in direction Dir.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A trivariate with same geometry as TV but with one      M
*                    degree higher.                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVDegreeRaiseN, degree raising                                       M
*****************************************************************************/
TrivTVStruct *TrivTVDegreeRaiseN(const TrivTVStruct *TV,
				 TrivTVDirType Dir,
				 int NewOrder)
{
    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    return TrivTVBlossomDegreeRaiseN(TV, NewOrder, TV -> VOrder,
					     TV -> WOrder);
	case TRIV_CONST_V_DIR:
	    return TrivTVBlossomDegreeRaiseN(TV, TV -> UOrder, NewOrder,
					     TV -> WOrder);
	case TRIV_CONST_W_DIR:
	    return TrivTVBlossomDegreeRaiseN(TV, TV -> UOrder,
					     TV -> VOrder, NewOrder);
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bezier trivariate, identical to the original but with one    M
* degree higher, in the requested direction Dir.                             M
* Let old control polygon be P(i), i = 0 to k-1, and Q(i) be new one then:   M
*		       i	    k-i					     V
* Q(0) = P(0), Q(i) = --- P(i-1) + (---) P(i), Q(k) = P(k-1).		     V
*		       k	     k					     V
* This is applied to all rows/cols of the trivariate.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        To raise it degree by one.                                    M
*   Dir:       Direction of degree raising. Either U, V or W.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A trivariate with one degree higher in direction Dir,   M
*                     representing the same geometry as TV.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVBlossomDegreeRaise, TrivBspTVDegreeRaise, TrivTVDegreeRaise        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBzrTVDegreeRaise, degree raising                                     M
*****************************************************************************/
TrivTVStruct *TrivBzrTVDegreeRaise(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, j, k, l,
	ULength = TV -> ULength,
	VLength = TV -> VLength,
	WLength = TV -> WLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct
	*RaisedTV = NULL;

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    RaisedTV = TrivBzrTVNew(ULength + 1, VLength, WLength,
				    TV -> PType);

	    for (j = 0; j < VLength; j++)
		for (k = 0; k < WLength; k++) {
		    for (l = IsNotRational; l <= MaxCoord; l++)	    /* Q(0). */
			RaisedTV -> Points[l][RAISED_TV(0, j, k)] =
			    TV -> Points[l][ORIG_TV(0, j, k)];

		    for (i = 1; i < ULength; i++)		    /* Q(i). */
		        for (l = IsNotRational; l <= MaxCoord; l++)
			    RaisedTV -> Points[l][RAISED_TV(i, j, k)] =
			        TV -> Points[l][ORIG_TV(i - 1, j, k)] *
			    		         (i / ((CagdRType) ULength)) +
			        TV -> Points[l][ORIG_TV(i, j, k)] *
				     ((ULength - i) / ((CagdRType) ULength));

		    for (l = IsNotRational; l <= MaxCoord; l++)	    /* Q(k). */
		        RaisedTV -> Points[l][RAISED_TV(ULength, j, k)] =
			    TV -> Points[l][ORIG_TV(ULength - 1, j, k)];
		}
	    break;
	case TRIV_CONST_V_DIR:
	    RaisedTV = TrivBzrTVNew(ULength, VLength + 1, WLength,
				    TV -> PType);

	    for (i = 0; i < ULength; i++)
		for (k = 0; k < WLength; k++) {
		    for (l = IsNotRational; l <= MaxCoord; l++)	    /* Q(0). */
			RaisedTV -> Points[l][RAISED_TV(i, 0, k)] =
			    TV -> Points[l][ORIG_TV(i, 0, k)];

		    for (j = 1; j < VLength; j++)		    /* Q(i). */
		        for (l = IsNotRational; l <= MaxCoord; l++)
			    RaisedTV -> Points[l][RAISED_TV(i, j, k)] =
			        TV -> Points[l][ORIG_TV(i, j - 1, k)] *
			    		         (j / ((CagdRType) VLength)) +
			        TV -> Points[l][ORIG_TV(i, j, k)] *
				     ((VLength - j) / ((CagdRType) VLength));

		    for (l = IsNotRational; l <= MaxCoord; l++)	    /* Q(k). */
		        RaisedTV -> Points[l][RAISED_TV(i, VLength, k)] =
			    TV -> Points[l][ORIG_TV(i, VLength - 1, k)];
		}
	    break;
	case TRIV_CONST_W_DIR:
	    RaisedTV = TrivBzrTVNew(ULength, VLength, WLength + 1,
				    TV -> PType);

	    for (i = 0; i < ULength; i++)
		for (j = 0; j < VLength; j++) {
		    for (l = IsNotRational; l <= MaxCoord; l++)	    /* Q(0). */
			RaisedTV -> Points[l][RAISED_TV(i, j, 0)] =
			    TV -> Points[l][ORIG_TV(i, j, 0)];

		    for (k = 1; k < WLength; k++)		    /* Q(i). */
		        for (l = IsNotRational; l <= MaxCoord; l++)
			    RaisedTV -> Points[l][RAISED_TV(i, j, k)] =
			        TV -> Points[l][ORIG_TV(i, j, k - 1)] *
			    		         (k / ((CagdRType) WLength)) +
			        TV -> Points[l][ORIG_TV(i, j, k)] *
				     ((WLength - k) / ((CagdRType) WLength));

		    for (l = IsNotRational; l <= MaxCoord; l++)	    /* Q(k). */
		        RaisedTV -> Points[l][RAISED_TV(i, j, WLength)] =
			    TV -> Points[l][ORIG_TV(i, j, WLength - 1)];
		}
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
	    break;
    }

    return RaisedTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline trivariate, identical to the original but with one   M
* degree higher, in the requested direction Dir.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        To raise it degree by one.                                    M
*   Dir:       Direction of degree raising. Either U, V or W.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A trivariate with one degree higher in direction Dir,   M
*                    representing the same geometry as TV.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivTVBlossomDegreeRaise, TrivBzrTVDegreeRaise, TrivTVDegreeRaise        M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivBspTVDegreeRaise, degree raising                                     M
*****************************************************************************/
TrivTVStruct *TrivBspTVDegreeRaise(const TrivTVStruct *TV, TrivTVDirType Dir)
{
    CagdBType
	IsNotRational = !TRIV_IS_RATIONAL_TV(TV);
    int i, i2, j, j2, k, k2, l, RaisedLen,
        ULength = TV -> ULength,
        VLength = TV -> VLength,
        WLength = TV -> WLength,
        UOrder = TV -> UOrder,
        VOrder = TV -> VOrder,
        WOrder = TV -> WOrder,
	MaxCoord = CAGD_NUM_OF_PT_COORD(TV -> PType);
    TrivTVStruct
	*RaisedTV = NULL;

    /* If trivariate is linear, degree raising means basically to increase   */
    /* the knot multiplicity of each segment by one and add a middle point   */
    /* for each such segment.						     */

    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    if (UOrder > 2) {
		TRIV_FATAL_ERROR(TRIV_ERR_WRONG_ORDER);
		return NULL;
	    }

	    RaisedLen = ULength * 2 - 1;
	    RaisedTV = TrivBspTVNew(RaisedLen, VLength, WLength,
				    UOrder + 1, VOrder, WOrder,
				    TV -> PType);

	    /* Update the knot vectors. */
	    for (i = 0; i < 3; i++)
		RaisedTV -> UKnotVector[i] = TV -> UKnotVector[0];
	    for (i = 2, j = 3; i < ULength; i++, j += 2)
		RaisedTV -> UKnotVector[j] =
		    RaisedTV -> UKnotVector[j + 1] = 
		        TV -> VKnotVector[i];
	    for (i = j; i < j + 3; i++)
		RaisedTV -> UKnotVector[i] = TV -> UKnotVector[ULength];
	    CAGD_GEN_COPY(RaisedTV -> VKnotVector, TV -> VKnotVector,
			  sizeof(CagdRType) * (VLength + VOrder));
	    CAGD_GEN_COPY(RaisedTV -> WKnotVector, TV -> WKnotVector,
			  sizeof(CagdRType) * (WLength + WOrder));

	    /* Update the mesh. */
	    for (k = 0; k < WLength; k++)
		for (j = 0; j < VLength; j++) {
		    for (l = IsNotRational; l <= MaxCoord; l++)
			RaisedTV -> Points[l][RAISED_TV(0, j, k)] =
			    TV -> Points[l][ORIG_TV(0, j, k)];

		    for (i = 1, i2 = 1; i < ULength; i++, i2 += 2)
		        for (l = IsNotRational; l <= MaxCoord; l++) {
			    RaisedTV -> Points[l][RAISED_TV(i2, j, k)] =
				TV -> Points[l][ORIG_TV(i - 1, j, k)] * 0.5 +
				TV -> Points[l][ORIG_TV(i, j, k)] * 0.5;
			    RaisedTV -> Points[l][RAISED_TV(i2 + 1, j, k)] =
			        TV -> Points[l][ORIG_TV(i, j, k)];
			}
		}
	    break;
	case TRIV_CONST_V_DIR:
	    if (VOrder > 2) {
		TRIV_FATAL_ERROR(TRIV_ERR_WRONG_ORDER);
		return NULL;
	    }

	    RaisedLen = VLength * 2 - 1;
	    RaisedTV = TrivBspTVNew(ULength, RaisedLen, WLength,
				    UOrder, VOrder + 1, WOrder,
				    TV -> PType);

	    /* Update the knot vectors. */
	    CAGD_GEN_COPY(RaisedTV -> UKnotVector, TV -> UKnotVector,
			  sizeof(CagdRType) * (ULength + UOrder));
	    for (i = 0; i < 3; i++)
		RaisedTV -> VKnotVector[i] = TV -> VKnotVector[0];
	    for (i = 2, j = 3; i < VLength; i++, j += 2)
		RaisedTV -> VKnotVector[j] =
		    RaisedTV -> VKnotVector[j + 1] = 
		        TV -> VKnotVector[i];
	    for (i = j; i < j + 3; i++)
		RaisedTV -> VKnotVector[i] = TV -> VKnotVector[VLength];
	    CAGD_GEN_COPY(RaisedTV -> WKnotVector, TV -> WKnotVector,
			  sizeof(CagdRType) * (WLength + WOrder));

	    /* Update the mesh. */
	    for (k = 0; k < WLength; k++)
		for (i = 0; i < ULength; i++) {
		    for (l = IsNotRational; l <= MaxCoord; l++)
			RaisedTV -> Points[l][RAISED_TV(i, 0, k)] =
			    TV -> Points[l][ORIG_TV(i, 0, k)];

		    for (j = 1, j2 = 1; j < VLength; j++, j2 += 2)
		        for (l = IsNotRational; l <= MaxCoord; l++) {
			    RaisedTV -> Points[l][RAISED_TV(i, j2, k)] =
				TV -> Points[l][ORIG_TV(i, j - 1, k)] * 0.5 +
				TV -> Points[l][ORIG_TV(i, j, k)] * 0.5;
			    RaisedTV -> Points[l][RAISED_TV(i, j2 + 1, k)] =
			        TV -> Points[l][ORIG_TV(i, j, k)];
			}
		}
	    break;
	case TRIV_CONST_W_DIR:
	    if (WOrder > 2) {
		TRIV_FATAL_ERROR(TRIV_ERR_WRONG_ORDER);
		return NULL;
	    }

	    RaisedLen = WLength * 2 - 1;
	    RaisedTV = TrivBspTVNew(ULength, VLength, RaisedLen,
				    UOrder, VOrder, WOrder + 1,
				    TV -> PType);

	    /* Update the knot vectors. */
	    CAGD_GEN_COPY(RaisedTV -> UKnotVector, TV -> UKnotVector,
			  sizeof(CagdRType) * (ULength + UOrder));
	    CAGD_GEN_COPY(RaisedTV -> VKnotVector, TV -> VKnotVector,
			  sizeof(CagdRType) * (VLength + VOrder));
	    for (i = 0; i < 3; i++)
		RaisedTV -> WKnotVector[i] = TV -> WKnotVector[0];
	    for (i = 2, j = 3; i < WLength; i++, j += 2)
		RaisedTV -> WKnotVector[j] = RaisedTV -> WKnotVector[j + 1] = 
		    TV -> WKnotVector[i];
	    for (i = j; i < j + 3; i++)
		RaisedTV -> WKnotVector[i] = TV -> WKnotVector[WLength];

	    /* Update the mesh. */
	    for (j = 0; j < VLength; j++)
		for (i = 0; i < ULength; i++) {
		    for (l = IsNotRational; l <= MaxCoord; l++)
			RaisedTV -> Points[l][RAISED_TV(i, j, 0)] =
			    TV -> Points[l][ORIG_TV(i, j, 0)];

		    for (k = 1, k2 = 1; k < WLength; k++, k2 += 2)
		        for (l = IsNotRational; l <= MaxCoord; l++) {
			    RaisedTV -> Points[l][RAISED_TV(i, j, k2)] =
				TV -> Points[l][ORIG_TV(i, j, k - 1)] * 0.5 +
				TV -> Points[l][ORIG_TV(i, j, k)] * 0.5;
			    RaisedTV -> Points[l][RAISED_TV(i, j, k2 + 1)] =
			        TV -> Points[l][ORIG_TV(i, j, k)];
			}
		}
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
	    break;
    }

    return RaisedTV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline trivariate, identical to the original but with one   M
* degree higher, in the requested direction Dir.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:         To raise it degree by one.                                   M
*   NewUOrder:   New degree raised order in U.				     M
*   NewVOrder:   New degree raised order in V.				     M
*   NewWOrder:   New degree raised order in W.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A trivariate with one degree higher in direction Dir,   M
*                    representing the same geometry as TV.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBspTVDegreeRaise, TrivBzrTVDegreeRaise, TrivTVDegreeRaise            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBlossomDegreeRaiseN                                                M
*****************************************************************************/
TrivTVStruct *TrivTVBlossomDegreeRaiseN(const TrivTVStruct *TV,
					int NewUOrder,
					int NewVOrder,
					int NewWOrder)
{
    CagdBType
	IsBezier = FALSE;
    CagdPointType
	PType = TV -> PType;
    int i, j, l, m, n, NewLen, StepSize, RStepSize;
    TrivTVStruct *RTV, *TTV, *CpTV;
    CagdBlsmAlphaCoeffStruct *A;

    if (TRIV_IS_BEZIER_TV(TV)) {         /* Convert to a Bspline trivariate. */
	IsBezier = TRUE;
	TV = CpTV = TrivCnvrtBzr2BspTV(TV);
    }
    else
        CpTV = NULL;

    /* We now have an open end Bspline trivar to deal with - compute degree  */
    /* raising matrices for this setup and apply them - U then V then W.     */

    if (NewUOrder > TV -> UOrder) {
        A = CagdBlossomDegreeRaiseNMat(TV -> UKnotVector, TV -> UOrder,
				       NewUOrder, TV -> ULength);
	NewLen = A -> NewLength;

	/* Allocate space for the new raised surface. */
	RTV = TrivBspTVNew(NewLen, TV -> VLength, TV -> WLength,
			   NewUOrder, TV -> VOrder, TV -> WOrder, PType);
	IRIT_GEN_COPY(RTV -> UKnotVector, A -> NewKV,
		      sizeof(CagdRType) * (NewLen + RTV -> UOrder));
	IRIT_GEN_COPY(RTV -> VKnotVector, TV -> VKnotVector,
		      sizeof(CagdRType) * (TV -> VLength + TV -> VOrder));
	IRIT_GEN_COPY(RTV -> WKnotVector, TV -> WKnotVector,
		      sizeof(CagdRType) * (TV -> WLength + TV -> WOrder));

	/* And apply the blossom alpha matrix to the control points. */
	for (i = !CAGD_IS_RATIONAL_PT(PType);
	     i <= CAGD_NUM_OF_PT_COORD(PType);
	     i++) {
	    CagdRType
	        *Points = TV -> Points[i],
	        *NewPoints = RTV -> Points[i];

	    for (n = 0; n < TV -> WLength; n++) {
	        for (m = 0; m < TV -> VLength; m++) {
		    CagdRType
		        *np = &NewPoints[TRIV_MESH_UVW(RTV, 0, m, n)],
		        *p = &Points[TRIV_MESH_UVW(TV, 0, m, n)];

		    for (l = 0; l < NewLen; l++, np++) {
		        CagdRType
			    *BlendVals = &A -> Rows[l][A -> ColIndex[l]],
			    *pp = &p[A -> ColIndex[l]];

			*np = 0.0;
			for (j = 0; j < A -> ColLength[l]; j++)
			    *np += *pp++ * *BlendVals++;
		    }
		}
	    }
	}

	CagdBlsmFreeAlphaCoef(A);

	if (CpTV != NULL)
	    TrivTVFree(CpTV);
	TV = CpTV = RTV;
    }
 
    if (NewVOrder > TV -> VOrder) {
        A = CagdBlossomDegreeRaiseNMat(TV -> VKnotVector, TV -> VOrder,
				       NewVOrder, TV -> VLength);
	NewLen = A -> NewLength;

	/* Allocate space for the new raised surface. */
	RTV = TrivBspTVNew(TV -> ULength, NewLen, TV -> WLength,
			   TV -> UOrder, NewVOrder, TV -> WOrder, PType);
	IRIT_GEN_COPY(RTV -> UKnotVector, TV -> UKnotVector,
		      sizeof(CagdRType) * (TV -> ULength + TV -> UOrder));
	IRIT_GEN_COPY(RTV -> VKnotVector, A -> NewKV,
		      sizeof(CagdRType) * (NewLen + RTV -> VOrder));
	IRIT_GEN_COPY(RTV -> WKnotVector, TV -> WKnotVector,
		      sizeof(CagdRType) * (TV -> WLength + TV -> WOrder));
	RStepSize = TRIV_NEXT_V(RTV);
	StepSize = TRIV_NEXT_V(TV);

	/* And apply the blossom alpha matrix to the control points. */
	for (i = !CAGD_IS_RATIONAL_PT(PType);
	     i <= CAGD_NUM_OF_PT_COORD(PType);
	     i++) {
	    CagdRType
	        *Points = TV -> Points[i],
	        *NewPoints = RTV -> Points[i];

	    for (n = 0; n < TV -> WLength; n++) {
	        for (l = 0; l < TV -> ULength; l++) {
		    CagdRType
		        *np = &NewPoints[TRIV_MESH_UVW(RTV, l, 0, n)],
		        *p = &Points[TRIV_MESH_UVW(TV, l, 0, n)];

		    for (m = 0; m < NewLen; m++, np += RStepSize) {
		        CagdRType
			    *BlendVals = &A -> Rows[m][A -> ColIndex[m]],
			    *pp = &p[A -> ColIndex[m] * StepSize];

			*np = 0.0;
			for (j = 0; j < A -> ColLength[m]; j++, pp += StepSize)
			    *np += *pp * *BlendVals++;
		    }
		}
	    }
	}

	CagdBlsmFreeAlphaCoef(A);

	if (CpTV != NULL)
	    TrivTVFree(CpTV);
	TV = CpTV = RTV;
    }
 
    if (NewWOrder > TV -> WOrder) {
        A = CagdBlossomDegreeRaiseNMat(TV -> WKnotVector, TV -> WOrder,
				       NewWOrder, TV -> WLength);
	NewLen = A -> NewLength;

	/* Allocate space for the new raised surface. */
	RTV = TrivBspTVNew(TV -> ULength, TV -> VLength, NewLen,
			   TV -> UOrder, TV -> VOrder, NewWOrder, PType);
	IRIT_GEN_COPY(RTV -> UKnotVector, TV -> UKnotVector,
		      sizeof(CagdRType) * (TV -> ULength + TV -> UOrder));
	IRIT_GEN_COPY(RTV -> VKnotVector, TV -> VKnotVector,
		      sizeof(CagdRType) * (TV -> VLength + TV -> VOrder));
	IRIT_GEN_COPY(RTV -> WKnotVector, A -> NewKV,
		      sizeof(CagdRType) * (NewLen + RTV -> WOrder));
	RStepSize = TRIV_NEXT_W(RTV);
	StepSize = TRIV_NEXT_W(TV);

	/* And apply the blossom alpha matrix to the control points. */
	for (i = !CAGD_IS_RATIONAL_PT(PType);
	     i <= CAGD_NUM_OF_PT_COORD(PType);
	     i++) {
	    CagdRType
	        *Points = TV -> Points[i],
	        *NewPoints = RTV -> Points[i];

	    for (m = 0; m < TV -> VLength; m++) {
	        for (l = 0; l < TV -> ULength; l++) {
		    CagdRType
		        *np = &NewPoints[TRIV_MESH_UVW(RTV, l, m, 0)],
		        *p = &Points[TRIV_MESH_UVW(TV, l, m, 0)];

		    for (n = 0; n < NewLen; n++, np += RStepSize) {
		        CagdRType
			    *BlendVals = &A -> Rows[n][A -> ColIndex[n]],
			    *pp = &p[A -> ColIndex[n] * StepSize];

			*np = 0.0;
			for (j = 0; j < A -> ColLength[n]; j++, pp += StepSize)
			    *np += *pp * *BlendVals++;
		    }
		}
	    }
	}

	CagdBlsmFreeAlphaCoef(A);

	if (CpTV != NULL)
	    TrivTVFree(CpTV);
	TV = CpTV = RTV;
    }
 
    if (IsBezier) {
	TTV = TrivCnvrtBsp2BzrTV(TV);
	if (CpTV != NULL)
	    TrivTVFree(CpTV);
	TV = CpTV = TTV;
    }

    return CpTV != NULL ? CpTV : TrivTVCopy(TV);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a new Bspline trivariate, identical to the original but with one   M
* degree higher, in the requested direction Dir.                             M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:        To raise it degree by one.                                    M
*   Dir:       Direction of degree raising. Either U, V or W.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  A trivariate with one degree higher in direction Dir,   M
*                    representing the same geometry as TV.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrivBspTVDegreeRaise, TrivBzrTVDegreeRaise, TrivTVDegreeRaise            M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVBlossomDegreeRaise                                                 M
*****************************************************************************/
TrivTVStruct *TrivTVBlossomDegreeRaise(const TrivTVStruct *TV,
				       TrivTVDirType Dir)
{
    switch (Dir) {
	case TRIV_CONST_U_DIR:
	    return TrivTVBlossomDegreeRaiseN(TV,
					     TV -> UOrder + 1,
					     TV -> VOrder,
					     TV -> WOrder);
	    break;
        case TRIV_CONST_V_DIR:
	    return TrivTVBlossomDegreeRaiseN(TV,
					     TV -> UOrder,
					     TV -> VOrder + 1,
					     TV -> WOrder);
	    break;
        case TRIV_CONST_W_DIR:
	    return TrivTVBlossomDegreeRaiseN(TV,
					     TV -> UOrder,
					     TV -> VOrder,
					     TV -> WOrder + 1);
	    break;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_DIR_NOT_VALID);
	    break;
    }

    return NULL;
}
