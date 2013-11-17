/******************************************************************************
* MvarCoer.c - Handle point coercions/conversions.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a list of multivariates to point type PType.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To coerce to a new point type PType.                           M
*   PType:    New point type for MV.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   New multivariates with PType as their point type.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCoerceMVsTo, coercion                                                M
*****************************************************************************/
MvarMVStruct *MvarCoerceMVsTo(const MvarMVStruct *MV, MvarPointType PType)
{
    MvarMVStruct *CoercedMV,
        *CoercedMVs = NULL;

    for ( ; MV != NULL; MV = MV -> Pnext) {
        CoercedMV = MvarCoerceMVTo(MV, PType);
	IRIT_LIST_PUSH(CoercedMV, CoercedMVs);
    }

    return CagdListReverse(CoercedMVs);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Coerces a multi-variate to point type PType.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To coerce to a new point type PType.                           M
*   PType:    New point type for MV.                                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A new multi-variate with PType as its point type.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCoerceMVTo, coercion                                                 M
*****************************************************************************/
MvarMVStruct *MvarCoerceMVTo(const MvarMVStruct *MV, MvarPointType PType)
{
    MvarMVStruct *TMV;

#ifdef MVAR_MALLOC_STRUCT_ONCE
    int i, *q,
	Dim = MV -> Dim,
	MaxAxis = MVAR_NUM_OF_PT_COORD(PType),
	Len = MVAR_CTL_MESH_LENGTH(MV),
	ExtraMem = sizeof(int) * Dim + 8 +          /* Lengths. */
		   sizeof(int) * (Dim + 1) + 8 +  /* SubSpaces. */
		   sizeof(int) * Dim + 8 +           /* Orders. */
		   sizeof(CagdBType) * Dim + 8 +  /* Periodics. */
		   sizeof(CagdRType **) * Dim + 8;      /* KVs. */

    TMV = CagdStructOnceCoercePointsTo(MV -> Points, MV,
				       sizeof(MvarMVStruct) + 8,
				       ExtraMem, Len,
				       (CagdPointType) MV -> PType,
				       (CagdPointType) PType);

    /* Find address beyond the struct and beyond the control points. */
    q = (int *) &TMV -> Points[MaxAxis][Len];

    TMV -> Lengths = q;
    q += Dim;

    TMV -> SubSpaces = q;
    q += Dim + 1;

    TMV -> Orders = q;
    q += Dim;

    TMV -> Periodic = q; /* Assume CagdBType same as int. */
    q += Dim;

    /* Align it to 8 bytes. */
    q = (int *) ((((IritIntPtrSizeType) q) + 7) & ~0x07);

    TMV -> KnotVectors = (CagdRType **) q;

    TMV -> GType = MV -> GType;
    TMV -> Dim = Dim;
    TMV -> Pnext = NULL;
    TMV -> PAux = TMV -> PAux2 = NULL;

    TMV -> Attr = IP_ATTR_COPY_ATTRS(MV -> Attr);

    CAGD_GEN_COPY(TMV -> Lengths, MV -> Lengths, Dim * sizeof(int));
    CAGD_GEN_COPY(TMV -> SubSpaces, MV -> SubSpaces, (Dim + 1) * sizeof(int));
    CAGD_GEN_COPY(TMV -> Orders, MV -> Orders, Dim * sizeof(int));
    CAGD_GEN_COPY(TMV -> Periodic, MV -> Periodic, Dim * sizeof(int));

    if (TMV -> GType == MVAR_BSPLINE_TYPE) {
        for (i = 0; i < Dim; i++) {
	    assert(MV -> KnotVectors[i] != NULL);
	    TMV -> KnotVectors[i] =
	        BspKnotCopy(NULL, MV -> KnotVectors[i],
			    MVAR_MVAR_ITH_PT_LST_LEN(MV, i) + MV -> Orders[i]);
	}
    }
    else {
        IRIT_ZAP_MEM(TMV -> KnotVectors, Dim * sizeof(CagdRType *));
    }

    if (MV -> AuxDomain != NULL) {
        TMV -> AuxDomain = IritMalloc(sizeof(MvarMinMaxType) * MV -> Dim);
	CAGD_GEN_COPY(TMV -> AuxDomain, MV -> AuxDomain,
		      sizeof(MvarMinMaxType) * MV -> Dim);
    }
    else
	TMV -> AuxDomain = NULL;

    TMV -> Pnext = NULL;
    TMV -> Attr = NULL;
    CAGD_PROPAGATE_ATTR(TMV, MV);
#else
    TMV = MvarMVCopy(MV);
    CagdCoercePointsTo(TMV -> Points, MVAR_CTL_MESH_LENGTH(TMV),
		       (CagdPointType) TMV -> PType, (CagdPointType) PType);
#endif /* MVAR_MALLOC_STRUCT_ONCE */

    TMV -> PType = PType;

    return TMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns a point type which spans the spaces of both two given point types. M
*                                                                            *
* PARAMETERS:                                                                M
*   PType1, PType2: To point types to find the point type of their union.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPointType:  A point type of the union of the spaces of PType1 and    M
*                   PType2.                                                  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMergeIrtPtType, coercion                                             M
*****************************************************************************/
MvarPointType MvarMergeIrtPtType(MvarPointType PType1, MvarPointType PType2)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_PT(PType1) || MVAR_IS_RATIONAL_PT(PType2);
    int NumCoords = IRIT_MAX(MVAR_NUM_OF_PT_COORD(PType1),
		        MVAR_NUM_OF_PT_COORD(PType2));

    return MVAR_MAKE_PT_TYPE(IsRational, NumCoords);
}
