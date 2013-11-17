/******************************************************************************
* Moments.c - moments computation of surfaces.				      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Octavian Soldea & Gershon Elber		      November 2000.  *
******************************************************************************/

#include "symb_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to compute the enclosed volume function of the given surface. M
*   The computed volume is the (signed) volume between the surface and its   M
* projection onto the XY plane.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:	Surface to computes its enclosed volume.                     M
*   Integrate:  TRUE to also integrate the resulting surface.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Integral volume function.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfVolume1, SymbSrfVolume2Srf, SymbSrfVolume2                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfVolume1Srf                                                        M
*****************************************************************************/
CagdSrfStruct *SymbSrfVolume1Srf(const CagdSrfStruct *Srf, CagdBType Integrate)
{
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *SrfNormal, *SrfNormalZ,
	*SrfResult, *SrfResultAux, *SrfResultInt;

    if (CAGD_IS_RATIONAL_SRF(Srf)) {
        SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    /* Get the Z coefficient of the surface. */
    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);

    /* Get the Z coefficient of the normal field. */
    SrfNormal = SymbSrfNormalSrf(Srf);
    SymbSrfSplitScalar(SrfNormal, &SrfW, &SrfX, &SrfY, &SrfNormalZ);
    CagdSrfFree(SrfNormal);
    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);

    SrfResult = SymbSrfMult(SrfZ, SrfNormalZ);

    CagdSrfFree(SrfZ);
    CagdSrfFree(SrfNormalZ);

    if (Integrate) {
	SrfResultAux = CagdSrfIntegrate(SrfResult, CAGD_CONST_U_DIR); 
	SrfResultInt = CagdSrfIntegrate(SrfResultAux, CAGD_CONST_V_DIR); 
  
	CagdSrfFree(SrfResultAux);
	CagdSrfFree(SrfResult);

	return SrfResultInt;
    }
    else
	return SrfResult;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to compute the enclosed volume by the given surface.          M
*   The computed volume is the (signed) volume between the surface and its   M
* projection onto the XY plane.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Surface to computes its enclosed volume.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The ensclosed volume.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfVolume1Srf, SymbSrfVolume2Srf, SymbSrfVolume2                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfVolume1                                                           M
*****************************************************************************/
CagdRType SymbSrfVolume1(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *VolSrf;
    CagdRType Umin, Umax, Vmin, Vmax, *Result;

    VolSrf = SymbSrfVolume1Srf(Srf, TRUE);
 
    CagdSrfDomain(VolSrf, &Umin, &Umax, &Vmin, &Vmax);
  
    Result = CagdSrfEval(VolSrf, Umax, Vmax);
    CagdSrfFree(VolSrf);

    return Result[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to compute the enclosed volume function of the given surface. M
*   The computed volume is the (signed) volume occupied by all rays from the M
* origin to the surface.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:	Surface to computes its enclosed volume.                     M
*   Integrate:  TRUE to also integrate the resulting surface.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:    Integral volume function.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfVolume2, SymbSrfVolume1Srf, SymbSrfVolume1                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfVolume2Srf                                                        M
*****************************************************************************/
CagdSrfStruct *SymbSrfVolume2Srf(const CagdSrfStruct *Srf,
				 CagdBType Integrate)
{
    CagdRType
	Scale = 1.0 / 3.0;
    CagdSrfStruct *SrfResultAux, *SrfResultInt,
	*SrfNormal = SymbSrfNormalSrf(Srf),
	*SrfResult = SymbSrfDotProd(Srf, SrfNormal);

    CagdSrfFree(SrfNormal);
    CagdSrfScale(SrfResult, &Scale);

    if (Integrate) {
	SrfResultAux = CagdSrfIntegrate(SrfResult, CAGD_CONST_U_DIR); 
	SrfResultInt = CagdSrfIntegrate(SrfResultAux, CAGD_CONST_V_DIR); 

	CagdSrfFree(SrfResultAux);
	CagdSrfFree(SrfResult);

	return SrfResultInt;
    }
    else
	return SrfResult;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A function to compute the enclosed volume by the given surface.          M
*   The computed volume is the (signed) volume occupied by all rays from the M
* origin to the surface.						     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Surface to computes its enclosed volume.                         M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:   The ensclosed volume.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfVolume1Srf, SymbSrfVolume2Srf, SymbSrfVolume2                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfVolume2                                                           M
*****************************************************************************/
CagdRType SymbSrfVolume2(const CagdSrfStruct *Srf)
{
    CagdSrfStruct *VolSrf;
    CagdRType Umin, Umax, Vmin, Vmax, *Result;

    VolSrf = SymbSrfVolume2Srf(Srf, TRUE);
 
    CagdSrfDomain(VolSrf, &Umin, &Umax, &Vmin, &Vmax);
  
    Result = CagdSrfEval(VolSrf, Umax, Vmax);
    CagdSrfFree(VolSrf);

    return Result[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the first moment function of the given surface.                 M
*   The computed moment is for the (signed) volume between the surface and   M
* its projection onto the XY plane.					     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        Surface to compute the first moment for.                     M
*   Axis:       1 for X, 2 for Y, 3 for Z.				     M
*   Integrate:  TRUE to also integrate the resulting surface.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The computed moment function.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFirstMoment, SymbSrfFirstMomentSrf, SymbSrfVolume                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfFirstMomentSrf                                                    M
*****************************************************************************/
CagdSrfStruct *SymbSrfFirstMomentSrf(const CagdSrfStruct *Srf,
				     int Axis,
				     CagdBType Integrate)
{
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *SrfMomentSrf, *SrfVol,
	*SrfResultAux, *SrfResultInt;

    if (CAGD_IS_RATIONAL_SRF(Srf)) {
        SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    SrfVol = SymbSrfVolume1Srf(Srf, FALSE);

    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
    switch (Axis) {
	case 1:
	    SrfMomentSrf = SymbSrfMult(SrfX, SrfVol);
	    break;
	case 2:
	    SrfMomentSrf = SymbSrfMult(SrfY, SrfVol);
	    break;
	case 3:
	    SrfMomentSrf = SymbSrfMult(SrfZ, SrfVol);
	    break;
        default:
	    assert(0);
	    SrfMomentSrf = NULL;
	    break;
    }

    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(SrfZ);
    CagdSrfFree(SrfVol);

    if (Integrate) {
	SrfResultAux = CagdSrfIntegrate(SrfMomentSrf, CAGD_CONST_U_DIR);
	SrfResultInt = CagdSrfIntegrate(SrfResultAux, CAGD_CONST_V_DIR);

	CagdSrfFree(SrfMomentSrf);
	CagdSrfFree(SrfResultAux);

	return SrfResultInt;
    }
    else
	return SrfMomentSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the first moment of the given surface.                          M
*   The computed moment is for the (signed) volume between the surface and   M
* its projection onto the XY plane.					     M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Surface to compute the first moment for.                          M
*   Axis:  1 for X, 2 for Y, 3 for Z.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  The computed moment.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFirstMomentSrf, SymbSrfFirstMomentSrf, SymbSrfVolume              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfFirstMoment                                                       M
*****************************************************************************/
CagdRType SymbSrfFirstMoment(const CagdSrfStruct *Srf, int Axis)
{
    CagdSrfStruct *MomentSrf;
    CagdRType Umin, Umax, Vmin, Vmax, *Result;

    MomentSrf = SymbSrfFirstMomentSrf(Srf, Axis, TRUE);
 
    CagdSrfDomain(MomentSrf, &Umin, &Umax, &Vmin, &Vmax);
  
    Result = CagdSrfEval(MomentSrf, Umax, Vmax);
    CagdSrfFree(MomentSrf);

    return Axis == 3 ? Result[1] * 0.5 : Result[1];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the second moment function of the given surface.                M
*   The computed moment is for the (signed) volume between the surface and   M
* its projection onto the XY plane.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           Surface to compute the first moment for.                  M
*   Axis1, Axis2:  1 for X, 2 for Y, 3 for Z.				     M
*   Integrate:     TRUE to also integrate the resulting surface.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  The computed moment function.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFirstMoment, SymbSrfFirstMomentSrf, SymbSrfVolumeSrf              M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSecondMomentSrf                                                   M
*****************************************************************************/
CagdSrfStruct *SymbSrfSecondMomentSrf(const CagdSrfStruct *Srf,
				      int Axis1,
				      int Axis2,
				      CagdBType Integrate)
{
    CagdSrfStruct *SrfW, *SrfX, *SrfY, *SrfZ, *SrfFirstMoment,
	*SrfSecondMoment, *SrfResultAux, *SrfResultInt;

    if (CAGD_IS_RATIONAL_SRF(Srf)) {
        SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	return NULL;
    }

    SrfFirstMoment = SymbSrfFirstMomentSrf(Srf, Axis1, FALSE);

    SymbSrfSplitScalar(Srf, &SrfW, &SrfX, &SrfY, &SrfZ);
    switch (Axis2) {
	case 1:
	    SrfSecondMoment = SymbSrfMult(SrfX, SrfFirstMoment);
	    break;
	case 2:
	    SrfSecondMoment = SymbSrfMult(SrfY, SrfFirstMoment);
	    break;
	case 3:
	    SrfSecondMoment = SymbSrfMult(SrfZ, SrfFirstMoment);
	    break;
        default:
	    assert(0);
	    SrfSecondMoment = NULL;
	    break;
    }

    CagdSrfFree(SrfX);
    CagdSrfFree(SrfY);
    CagdSrfFree(SrfZ);
    CagdSrfFree(SrfFirstMoment);

    if (Integrate) {
	SrfResultAux = CagdSrfIntegrate(SrfSecondMoment, CAGD_CONST_U_DIR);
	SrfResultInt = CagdSrfIntegrate(SrfResultAux, CAGD_CONST_V_DIR);

	CagdSrfFree(SrfSecondMoment);
	CagdSrfFree(SrfResultAux);

	return SrfResultInt;
    }
    else
	return SrfSecondMoment;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the second moment of the given surface.                         M
*   The computed moment is for the (signed) volume between the surface and   M
* its projection onto the XY plane.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:           Surface to compute the second moment for.                 M
*   Axis1, Axis2:  1 for X, 2 for Y, 3 for Z.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  The computed moment.                                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfSecondMomentSrf, SymbSrfFirstMoment, SymbSrfVolume                M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbSrfSecondMoment                                                      M
*****************************************************************************/
CagdRType SymbSrfSecondMoment(const CagdSrfStruct *Srf, int Axis1, int Axis2)
{
    CagdSrfStruct *MomentSrf;
    CagdRType Umin, Umax, Vmin, Vmax, *Result;

    MomentSrf = SymbSrfSecondMomentSrf(Srf, Axis1, Axis2, TRUE);
 
    CagdSrfDomain(MomentSrf, &Umin, &Umax, &Vmin, &Vmax);
  
    Result = CagdSrfEval(MomentSrf, Umax, Vmax);
    CagdSrfFree(MomentSrf);

    if (Axis1 != 3 && Axis2 != 3)
	return Result[1];
    else if (Axis1 == 3 && Axis2 == 3)
	return Result[1] / 3.0;
    return Result[1] * 0.5;
}
