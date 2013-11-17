/******************************************************************************
* MvarMrph.c - A simple tool to morph between two compatible multi-variates.  *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May 97.					      *
******************************************************************************/

#include "mvar_loc.h"
#include "misc_lib.h"
#include "geom_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given two compatible multi-variates (See function MvarMakeMVsCompatible),  M
* computes a convex blend between them according to Blend which must be      M
* between zero and one.							     M
*   Returned is the new blended multi-variates.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV1, MV2:  The two multi-variates to blend.                              M
*   Blend:     A parameter between zero and one.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   MV2 * Blend + MV1 * (1 - Blend).                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbTwoCrvsMorphing, SymbTwoCrvsMorphingCornerCut,			     M
*   SymbTwoCrvsMorphingMultiRes, SymbTwoSrfsMorphing, TrivTwoTVsMorphing,    M
*   MvarMakeMVsCompatible						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarTwoMVsMorphing, morphing                                             M
*****************************************************************************/
MvarMVStruct *MvarTwoMVsMorphing(const MvarMVStruct *MV1,
				 const MvarMVStruct *MV2,
				 CagdRType Blend)
{
    int i, j,
	Dim = MV1 -> Dim,
	Length = MVAR_CTL_MESH_LENGTH(MV1),
	MaxAxis = MVAR_NUM_OF_MV_COORD(MV1);
    CagdRType **NewPoints,
	* const *Points1 = MV1 -> Points,
	* const *Points2 = MV2 -> Points,
	Blend1 = 1.0 - Blend;
    MvarMVStruct *NewMV;

    if (Dim != MV2 -> Dim ||
	MV1 -> PType != MV2 -> PType ||
	MV1 -> GType != MV2 -> GType) {
	MVAR_FATAL_ERROR(MVAR_ERR_MVS_INCOMPATIBLE);
	return NULL;
    }

    for (i = 0; i < Dim; i++) {
	if (MV1 -> Orders[i] != MV2 -> Orders[i] ||
	    MV1 -> Lengths[i] != MV2 -> Lengths[i]) {
	    MVAR_FATAL_ERROR(MVAR_ERR_MVS_INCOMPATIBLE);
	    return NULL;
	}
    }
	
    NewMV = MvarMVNew(Dim, MV1 -> GType, MV1 -> PType, MV1 -> Lengths);
    CAGD_GEN_COPY(NewMV -> Orders, MV1 -> Orders, sizeof(int) * Dim);
    NewPoints = NewMV -> Points;
    for (i = 0; i < Dim; i++) {
	if (MV1 -> KnotVectors[i] != NULL)
	    NewMV -> KnotVectors[i] = BspKnotCopy(NULL, MV1 -> KnotVectors[i],
						  MV1 -> Lengths[i] +
						      MV1 -> Orders[i]);
    }

    for (i = !MVAR_IS_RATIONAL_PT(MV1 -> PType); i <= MaxAxis; i++) {
	CagdRType
	    *Pts1 = &Points1[i][0],
	    *Pts2 = &Points2[i][0],
	    *NewPts = &NewPoints[i][0];

	for (j = Length; j >= 0; j--)
	    *NewPts++ = *Pts1++ * Blend1 + *Pts2++ * Blend;
    }

    return NewMV;
}
