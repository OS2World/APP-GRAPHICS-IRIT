/******************************************************************************
* MvarTopo.c - Handle topology issues of freeforms.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 04.					      *
******************************************************************************/

#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes U or V extreme values of the zero set (implicit form) of 	     M
* Srf = 0.							             M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Implicit surface definition.                                      M
*   Dir:   U to compute U-extreme values in Srf = 0, V for V-extreme.        M
*   SubdivTol:    Tolerance of the solution.  This tolerance is measured in  M
*		  the parametric space of the surfaces.			     M
*   NumerTol:     Numeric tolerance of a possible numeric improvment stage.  M
*		  The numeric stage is employed if NumericTol < SubdivTol.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   The computed extreme values, in the parametric space   M
*		      of surface Srf.		                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarImplicitCrvExtreme                                                   M
*****************************************************************************/
MvarPtStruct *MvarImplicitCrvExtreme(const CagdSrfStruct *Srf,
				     CagdSrfDirType Dir,
				     CagdRType SubdivTol,
				     CagdRType NumerTol)
{
    MvarConstraintType Constraints[2];
    CagdSrfStruct
	*DSrf = CagdSrfDerive(Srf, Dir);
    MvarMVStruct *MVs[2];
    MvarPtStruct *Pts;

    MVs[0] = MvarSrfToMV(Srf);
    MVs[1] = MvarSrfToMV(DSrf);
    CagdSrfFree(DSrf);

    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    Pts = MvarMVsZeros(MVs, Constraints, 2, SubdivTol, NumerTol);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    return Pts;
}
