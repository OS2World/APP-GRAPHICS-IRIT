/******************************************************************************
* MvarPrim.c - primitive multivairate constructors.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Dec 2006.					      *
******************************************************************************/

#include "irit_sm.h"
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a mulit-linear multivariates that spans the [Min, Max]        M
* ranges in all Dim dimenions.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Min:   Minimal values of the expected ranges.                            M
*   Max:   Maximal values of the expected ranges.                            M
*   Dim:   Dimension of expect multivariate.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  Constructed multi-linear function.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVMultiLinearMV                                                      M
*****************************************************************************/
MvarMVStruct *MvarMVMultiLinearMV(const IrtRType *Min,
				  const IrtRType *Max,
				  int Dim)
{
    int i, j, Len, Bit,
	*Lengths = (int *) IritMalloc(sizeof(int) * Dim);
    IrtRType *R;
    MvarMVStruct *MV;

    for (i = 0; i < Dim; i++)
	Lengths[i] = 2;

    MV = MvarBzrMVNew(Dim, Lengths,  MVAR_MAKE_PT_TYPE(FALSE, Dim));
    IritFree(Lengths);
    Len = MVAR_CTL_MESH_LENGTH(MV);

    for (i = 0, Bit = 1; i < Dim; i++) {
        R = MV -> Points[i + 1];

	for (j = 0; j < Len; j++)
	    R[j] = (j & Bit) ? Max[i] : Min[i];

	Bit *= 2;
    }

    return MV;
}
