/******************************************************************************
* TrivRuld.c - Ruled trivariate operator out of given two surface.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 2000.					      *
******************************************************************************/

#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a ruled trivariate between the two provided surfaces.           M
* OtherOrder and OtherLen (equal for Bezier) specifies the desired order and M
* refineness level (if Bspline) of the other ruled direction.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf1, CSrf2: The two surfaces to form a ruled trivariate in between.    M
*   OtherOrder: Usually two, but one can specify higher orders in the ruled  M
*               direction. OtherOrder must never be larger than OrderLen.    M
*   OtherLen:   Usually two control points in the ruled direction which      M
*               necesitates a linear interpolation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:  The ruled trivariate.                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdRuledSrf                                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivRuledTV, ruled trivariate, trivariate constructors                   M
*****************************************************************************/
TrivTVStruct *TrivRuledTV(const CagdSrfStruct *CSrf1,
			  const CagdSrfStruct *CSrf2,
			  int OtherOrder,
			  int OtherLen)
{
    TrivTVStruct *TV;
    int i, j, k, MaxCoord, Len;
    CagdPointType PType;
    CagdBType IsNotRational;
    CagdRType **TVPoints, *Srf1SrcPt, *Srf2SrcPt, *TVDestPt, t, t1,
						 **Srf1Points, **Srf2Points;
    CagdSrfStruct *Srf1, *Srf2;

    Srf1 = CagdSrfCopy(CSrf1);
    Srf2 = CagdSrfCopy(CSrf2);

    CagdMakeSrfsCompatible(&Srf1, &Srf2, TRUE, TRUE, TRUE, TRUE);

    MaxCoord = CAGD_NUM_OF_PT_COORD(Srf1 -> PType),
    Len = Srf1 -> ULength * Srf1 -> VLength;
    PType = Srf1 -> PType;
    IsNotRational = !CAGD_IS_RATIONAL_SRF(Srf1);
    Srf1Points = Srf1 -> Points;
    Srf2Points = Srf2 -> Points;

    switch (Srf1 -> GType) {
	case CAGD_SBEZIER_TYPE:
	    TV = TrivBzrTVNew(Srf1 -> ULength, Srf1 -> VLength, OtherLen, PType);
	    break;
	case CAGD_SBSPLINE_TYPE:
	    TV = TrivBspTVNew(Srf1 -> ULength, Srf1 -> VLength, OtherLen,
			      Srf1 -> UOrder, Srf1 -> VOrder, OtherOrder,
			      PType);
	    CAGD_GEN_COPY(TV -> UKnotVector, Srf1 -> UKnotVector,
			  sizeof(CagdRType) * (TV -> ULength + TV -> UOrder));
	    CAGD_GEN_COPY(TV -> VKnotVector, Srf1 -> VKnotVector,
			  sizeof(CagdRType) * (TV -> VLength + TV -> VOrder));
	    BspKnotUniformOpen(OtherLen, OtherOrder, TV -> WKnotVector);
	    break;
	case CAGD_SPOWER_TYPE:
	    TRIV_FATAL_ERROR(TRIV_ERR_POWER_NO_SUPPORT);
	    return NULL;
	default:
	    TRIV_FATAL_ERROR(TRIV_ERR_UNDEF_SRF);
	    return NULL;
    }

    /* Copy the control mesh - first row is exactly the same as the first    */
    /* surface while last row is the same as second surface.		     */
    /* The middle rows are convex blend of the first/last rows.		     */
    TVPoints = TV -> Points;

    for (i = IsNotRational; i <= MaxCoord; i++)	       /* First depth layer. */
	CAGD_GEN_COPY(TVPoints[i], Srf1Points[i], sizeof(CagdRType) * Len);

    /* Make a copy of the last row. */
    for (i = IsNotRational; i <= MaxCoord; i++)		/* Last depth layer. */
	CAGD_GEN_COPY(&TVPoints[i][Len * (OtherLen - 1)], Srf2Points[i],
		      sizeof(CagdRType) * Len);

    /* And compute the internal rows, if any: */
    for (j = 1; j < OtherLen - 1; j++) {
	t = ((CagdRType) j) / (OtherLen - 1);
	t1 = 1.0 - t;
	for (i = IsNotRational; i <= MaxCoord; i++) {
	    TVDestPt = &TVPoints[i][Len * j];
	    Srf1SrcPt = Srf1Points[i];
	    Srf2SrcPt = Srf2Points[i];
	    for (k = 0; k < Len; k++)
		TVDestPt[k] = t1 * Srf1SrcPt[k] + t * Srf2SrcPt[k];
	}
    }		

    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    TRIV_SET_GEOM_TYPE(TV, TRIV_GEOM_RULED_TV);

    return TV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Connstructs a trilinear volume between the given eight corner points.    M
*                                                                            *
* PARAMETERS:                                                                M
*  Pt000, Pt001, Pt010, Pt011, Pt100, Pt101, Pt110, Pt111:  The eight        M
*						corners of the trilinear.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:    Constructed trilinear.                                M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdBilinearSrf                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTrilinearSrf                                                         M
*****************************************************************************/
TrivTVStruct *TrivTrilinearSrf(const CagdPtStruct *Pt000,
			       const CagdPtStruct *Pt001,
			       const CagdPtStruct *Pt010,
			       const CagdPtStruct *Pt011,
			       const CagdPtStruct *Pt100,
			       const CagdPtStruct *Pt101,
			       const CagdPtStruct *Pt110,
			       const CagdPtStruct *Pt111)
{
    CagdSrfStruct
        *S1 = CagdBilinearSrf(Pt000, Pt001, Pt010, Pt011),
        *S2 = CagdBilinearSrf(Pt100, Pt101, Pt110, Pt111);
    TrivTVStruct
        *TV = TrivRuledTV(S1, S2, 2, 2);

    CagdSrfFree(S1);
    CagdSrfFree(S2);

    return TV;    
}

