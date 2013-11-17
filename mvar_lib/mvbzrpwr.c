/******************************************************************************
* MvBzrPwr.c - Bezier to power basis conversion.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 2002.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mvar_loc.h"

static CagdRType BinomCoef(int n, int i);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts the given multivariate from Bezier basis functions to a Power     M
* basis functions. Using:						     M
*									     M
*         n                                                                  V
*         __                                                                 V
*  n      \      p-i n   p   p                                               V
* B (t) = /  (-1)   ( ) ( ) t                                                V
*  i      --         p   i                                                   V
*         p=i                                                                V
*                                                                            V
* or                                                                         M
*                							     V
*  n0     n1         nm							     V
* B (u0) B (v1) ... B (um) =						     V
*  i0     i1         im							     V
*                							     V
*   n0       nm                                                              V
*   __       __                                                              V
*   \        \     p0-i0  n0   p0         pm-im  nm   pm      p0      pm     V
*   /   ...  /  (-1)     (  ) (  ) ... (-1)     (  ) (  )   u0  ... um       V
*   --       --           p0   i0                pm   im                     V
*  p0=i0   pm=im                                                             V
*                                                                            V
*                                               			     M
* This routine simply take the weight of each product of m Bezier basis      M
* functions B0(u0)... Bm(u0) and spread it into the different power basis    M
* u0^p0 ...um^pm functions scaled by:					     M
*                                               			     M
*      p0-i0 n0   p0        pm-im  nm   pm 		                     V
*   (-1)    (  ) (  ) ... (-1)    (  ) (  )		                     V
*            p0   i0               pm   im 		                     V
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To convert into Power basis function representation.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  Same geometry, but in the Power basis.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCnvrtPwr2BzrMV, CagdCnvrtBzr2PwrSrf, CagdCnvrtPwr2BzrSrf	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCnvrtBzr2PwrMV, power basis, conversion	                             M
*****************************************************************************/
MvarMVStruct *MvarCnvrtBzr2PwrMV(const MvarMVStruct *MV)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, l, *Indices, *PwrIndices,
	Len = MVAR_CTL_MESH_LENGTH(MV),
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *PwrMV;

    if (!MVAR_IS_BEZIER_MV(MV)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BEZIER_EXPECTED);
	return NULL;
    }

    PwrMV = MvarMVNew(MV -> Dim, MVAR_POWER_TYPE, MV -> PType, MV -> Lengths);
    IRIT_GEN_COPY(PwrMV -> Orders, PwrMV -> Lengths,
		  sizeof(int) * PwrMV -> Dim);

    /* Update the control mesh. */
    Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    PwrIndices = (int *) IritMalloc(sizeof(int) * MV -> Dim);

    for (i = IsNotRational; i <= MaxCoord; i++) {
        int Index = 0;
	CagdRType
	    *PwrP = PwrMV -> Points[i],
	    *BzrP = MV -> Points[i];

	IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);
	IRIT_ZAP_MEM(PwrP, sizeof(CagdRType) * Len);

	do {
	    int PwrIndex;

	    IRIT_GEN_COPY(PwrIndices, Indices, sizeof(int) * MV -> Dim);
	    PwrIndex = MvarGetPointsMeshIndices(PwrMV, PwrIndices);

	    do {
		CagdRType
		    Coef = BzrP[Index];

		for (l = 0; l < MV -> Dim; l++) {
		    Coef *= BinomCoef(MV -> Lengths[l] - 1, PwrIndices[l]) *
		            BinomCoef(PwrIndices[l], Indices[l]) *
			    (((PwrIndices[l] - Indices[l]) & 0x01) ? -1 : 1);
		}

		PwrP[PwrIndex] += Coef;
	    }
	    while (MVAR_INC_BOUND_MESH_INDICES(PwrMV, PwrIndices, Indices,
					       PwrMV -> Lengths, PwrIndex));
	}
	while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));
    }

    IritFree(Indices);
    IritFree(PwrIndices);

    CAGD_PROPAGATE_ATTR(PwrMV, MV);

    return PwrMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts the given multivariate from Power basis functions to Bezier basis M
* functions. Using:							     M
*                                                                            M
*                                                                            V
*                n0       nm    i0       im                                  V
*                __       __   (  ) ... (  )                                 V
*    p0     pm   \        \     p0       pm    n0         nm                 V
*  u0 ... um  =  /    ... /    -------------- B (u0) ... B (um)              V
*                --       --    n0       nm    i0         im                 V
*               i0=p0    im=pm (  ) ... (  )                                 V
*                               p0       pm                                  V
*                                                                            V
* This routine simply take the weight of each product of m power basis       M
* functions u0^p0 ...um^pm and spread it into the different Bezier basis     M
* B0(u0)... Bm(u0) functions scaled by:					     M
*                                                                            V
*      i0       im 			                                     V
*     (  ) ... (  )			                                     V
*      p0       pm 			                                     V
*     --------------			                                     V
*      n0       nm  			                                     V
*     (  ) ... (  ) 			                                     V
*      p0       pm  			                                     V
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To convert into Bezier basis function representation.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  Same geometry, but in the Bezier basis.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCnvrtBzr2PwrMV, CagdCnvrtBzr2PwrMV, MvarCnvrtPwr2BzrMV	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCnvrtPwr2BzrMV, power basis, conversion	                             M
*****************************************************************************/
MvarMVStruct *MvarCnvrtPwr2BzrMV(const MvarMVStruct *MV)
{
    CagdBType
	IsNotRational = !MVAR_IS_RATIONAL_MV(MV);
    int i, l, *Indices, *BzrIndices,
	Len = MVAR_CTL_MESH_LENGTH(MV),
	MaxCoord = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *BzrMV;

    if (!MVAR_IS_POWER_MV(MV)) {
	MVAR_FATAL_ERROR(MVAR_ERR_POWER_EXPECTED);
	return NULL;
    }

    BzrMV = MvarMVNew(MV -> Dim, MVAR_BEZIER_TYPE, MV -> PType, MV -> Lengths);
    IRIT_GEN_COPY(BzrMV -> Orders, BzrMV -> Lengths,
		  sizeof(int) * BzrMV -> Dim);

    /* Update the control mesh. */
    Indices = (int *) IritMalloc(sizeof(int) * MV -> Dim);
    BzrIndices = (int *) IritMalloc(sizeof(int) * MV -> Dim);

    for (i = IsNotRational; i <= MaxCoord; i++) {
	int Index = 0;
	CagdRType
	    *BzrP = BzrMV -> Points[i],
	    *PwrP = MV -> Points[i];

	IRIT_ZAP_MEM(Indices, sizeof(int) * MV -> Dim);
	IRIT_ZAP_MEM(BzrP, sizeof(CagdRType) * Len);

	do {
	    int BzrIndex;

	    IRIT_GEN_COPY(BzrIndices, Indices, sizeof(int) * MV -> Dim);
	    BzrIndex = MvarGetPointsMeshIndices(BzrMV, BzrIndices);
	    do {
		CagdRType
		    Coef = PwrP[Index];

		for (l = 0; l < MV -> Dim; l++) {
		    Coef *= BinomCoef(BzrIndices[l], Indices[l]) /
		            BinomCoef(MV -> Lengths[l] - 1, Indices[l]);
		}

		BzrP[BzrIndex] += Coef;
	    }
	    while (MVAR_INC_BOUND_MESH_INDICES(BzrMV, BzrIndices, Indices,
					       BzrMV -> Lengths, BzrIndex));
	}
	while (MVAR_INCREMENT_MESH_INDICES(MV, Indices, Index));
    }

    IritFree(Indices);
    IritFree(BzrIndices);

    CAGD_PROPAGATE_ATTR(BzrMV, MV);

    return BzrMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Evaluate the following:						     *
*			 n         n!					     *
*			( ) = -------------				     *
*			 i    i! * (n - i)!				     *
*                                                                            *
* PARAMETERS:                                                                *
*   n, i:    Coefficients of the binom.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Result in floating point form to prevent from overflows.    *
*****************************************************************************/
static CagdRType BinomCoef(int n, int i)
{
    int j;
    CagdRType c = 1.0;

    if ((n >> 1) > i) {				/* i is less than half of n: */
	for (j = n - i + 1; j <= n; j++)
	    c *= j;
	for (j = 2; j <= i; j++)
	    c /= j;
    }
    else {
	for (j = i + 1; j <= n; j++)
	    c *= j;
	for (j = 2; j <= n - i; j++)
	    c /= j;
    }

    return c;
}
