/******************************************************************************
* Mvar_aux.c - auxiliary routine to interface to multi-variate rep.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include "mvar_loc.h"

#define EPS_ROUND_KNOT		1e-9

static void MvarBBoxOfCrossProdAux(const MvarBBoxStruct *BBox1,
				   int Axis1,
				   const MvarBBoxStruct *BBox2,
				   int Axis2,
				   CagdRType *Min,
				   CagdRType *Max);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, returns its parametric domain.  The Min/Max arrays  M
* are assumed to of sufficiently large enough space to hold all dimensions,  M
* if Axis == -1.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      Multivariate function to consider.                              M
*   Min:     Minimum domains of MV will be placed herein.                    M
*   Max:     Maximum domains of MV will be placed herein.                    M
*   Axis:    axis to extract or -1 for all axes.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*  MvarMVSetDomain, MvarMVSetAllDomains, MvarParamInDomain,		     M
*  MvarParamsInDomain							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDomain, multi-variates                                             M
*****************************************************************************/
void MvarMVDomain(const MvarMVStruct *MV,
		  CagdRType *Min,
		  CagdRType *Max,
		  int Axis)
{
    int i;

    if (Axis >= MV -> Dim)
	MVAR_FATAL_ERROR(MVAR_ERR_INVALID_AXIS);

    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	case MVAR_POWER_TYPE:
	    if (Axis == -1) {
		for (i = 0; i < MV -> Dim; i++) {
		    Min[i] = 0.0;
		    Max[i] = 1.0;
		}
	    }
	    else {
		*Min = 0.0;
		*Max = 1.0;
	    }
	    break;
	case MVAR_BSPLINE_TYPE:
	     if (Axis == -1) {
		for (i = 0; i < MV -> Dim; i++) {
		    int Order = MV -> Orders[i],
		        Len = MVAR_MVAR_ITH_PT_LST_LEN(MV, i);
		    CagdRType
		        *KV =  MV -> KnotVectors[i];

		    Min[i] = KV[Order - 1];
		    Max[i] = KV[Len];
		}
	    }
	    else {
		int Order = MV -> Orders[Axis],
		    Len = MVAR_MVAR_ITH_PT_LST_LEN(MV, Axis);
		CagdRType
		    *KV =  MV -> KnotVectors[Axis];

		*Min = KV[Order - 1];
		*Max = KV[Len];
	    }
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the volume of a multivariate's domain.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:   The multivariate.                                                 M
*   Dim:   Number of dimensions.                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   IrtRType:   Volume of domain.                                            M
*									     *
* KEYWORDS:                                                                  M
*   MvarMVVolumeOfDomain	                                             M
*****************************************************************************/
IrtRType MvarMVVolumeOfDomain(MvarMVStruct * const MVs, int Dim)
{
    int i;
    IrtRType
        Volume = 1.0,
        *MinArray = (IrtRType *) IritMalloc(sizeof(IrtRType) * Dim),
        *MaxArray = (IrtRType *) IritMalloc(sizeof(IrtRType) * Dim);

    MvarMVDomain(MVs, MinArray, MaxArray, -1);
    for (i = 0; i < Dim; i++)
	Volume *= MaxArray[i] - MinArray[i];

    IritFree(MinArray);
    IritFree(MaxArray);

    return Volume;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Same as MvarMVDomain but also allocate the space to hold the domain.     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:		Multivariate structures to receive its domain.		     M
*   MinDmn:	Array of maximal values.				     M
*   MaxDmn:     Array of minimal values. 				     M
*									     *
* RETURN VALUE:                                                              M
*   void:								     M
*   									     *
* SEE ALSO:								     M
*   MvarMVDomain, MvarMVDomainFree					     M
*									     *
* KEYWORDS:                                                                  M
*   MvarMVDomainAlloc		                                             M
*****************************************************************************/
void MvarMVDomainAlloc(const MvarMVStruct *MV,
		       CagdRType **MinDmn, 
		       CagdRType **MaxDmn)
{
    int	Dim = MV -> Dim;

    *MinDmn = (CagdRType *) IritMalloc(Dim * sizeof(CagdRType) * 2);
    *MaxDmn = &(*MinDmn)[Dim];

    MvarMVDomain(MV, *MinDmn, *MaxDmn, -1);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Deallocates the memory of MV's domain, as allocated by MvarMVDomain2.    M
*                                                                            *
* PARAMETERS:                                                                M
*   MinDmn:	Array of maximal values.				     M
*   MaxDmn:     Array of minimal values. 				     M
*									     *
* RETURN VALUE:                                                              M
*   void:								     M
*   									     *
* SEE ALSO:								     M
*   MvarMVDomain, MvarMVDomainAlloc					     M
*									     *
* KEYWORDS:                                                                  M
*   MvarMVDomainFree			                                     M
*****************************************************************************/
void MvarMVDomainFree(CagdRType *MinDmn, CagdRType *MaxDmn)
{
    IritFree(MinDmn);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, sets its parametric domain in direction Axis to be  M
* between Min and Max.							     M
*   If the MV is a Bezier and the domain requested is not [0, 1], it is      M
* coerced to a Bspline first.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      Multivariate function to update its domain.                     M
*   Min:     New minimum domain in Axis direction of MV.                     M
*   Max:     New maximum domain in Axis direction of MV.                     M
*   Axis:    Axis to set a new domain for.				     M
*   InPlace: If TRUE, updates domain in place if possible.  A Bezier can be  M
*	     converted into a Bspline, in which case the Bezier is released. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: Same multivariate with the updated domain in dir Axis.   M
*                                                                            *
* SEE ALSO:                                                                  M
*  MvarMVDomain, MvarMVSetAllDomains, MvarParamInDomain, MvarParamsInDomain  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVSetDomain, multi-variates                                          M
*****************************************************************************/
MvarMVStruct *MvarMVSetDomain(MvarMVStruct *MV,
			      CagdRType Min,
			      CagdRType Max,
			      int Axis,
			      int InPlace)
{
    MvarMVStruct *TMV;

    if (MVAR_IS_BEZIER_MV(MV)) {
        if ((Min != 0.0 || Max != 1.0))
	    TMV = MvarCnvrtBzr2BspMV(MV);
	else if (InPlace)
	    return MV;
	else {
	    TMV = MvarMVCopy(MV);
	    return TMV;
	}
    }
    else
	MV = InPlace ? MV : MvarMVCopy(MV);

    BspKnotAffineTransOrder2(MV -> KnotVectors[Axis], MV -> Orders[Axis],
			     MV -> Orders[Axis] + MV -> Lengths[Axis],
			     Min, Max);

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Resets an optional aux. domain slot inside MV for use in cases where MV  M
* has no explict domain (such as Bezier and/or Power basis).		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Multivariate to reset domain slot.			     M
*									     M
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVAuxDomainSlotCopy, MvarMVAuxDomainSlotSet,			     M
*   MvarMVAuxDomainSlotSet, MvarMVAuxDomainSlotSetRel			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVAuxDomainSlotReset	                                             M
*****************************************************************************/
void MvarMVAuxDomainSlotReset(MvarMVStruct *MV)
{
    int i;
    MvarMinMaxType
	*AuxDomain = IritMalloc(sizeof(MvarMinMaxType) * MV -> Dim);

    assert(MV -> AuxDomain == NULL);

    for (i = 0; i < MV -> Dim; i++)
        MvarMVDomain(MV, &AuxDomain[i][0], &AuxDomain[i][1], i);

    MV -> AuxDomain = AuxDomain;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Copies the optional aux. domain slot from MVSrc to MVDst.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVDst:     Destination multivariate to copy domain slot to.		     M
*   MVSrc:     Source multivariate to copy domain slot from.		     M
*									     M
* RETURN VALUE:                                                              M
*   int:  TRUE if successful, FALSE otherwise.                               M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVAuxDomainSlotReset, MvarMVAuxDomainSlotSet,			     M
*   MvarMVAuxDomainSlotSet, MvarMVAuxDomainSlotSetRel			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVAuxDomainSlotCopy	                                             M
*****************************************************************************/
int MvarMVAuxDomainSlotCopy(MvarMVStruct *MVDst, const MvarMVStruct *MVSrc)
{
    if (MVSrc -> AuxDomain == NULL)
        return FALSE;

    assert(MVSrc -> AuxDomain != NULL &&
	   MVDst -> AuxDomain == NULL &&
	   MVSrc -> Dim == MVDst -> Dim);

    MVDst -> AuxDomain = IritMalloc(sizeof(MvarMinMaxType) * MVSrc -> Dim);
    CAGD_GEN_COPY(MVDst -> AuxDomain, MVSrc -> AuxDomain,
		  sizeof(MvarMinMaxType) * MVSrc -> Dim);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets one aux. domain range in the given direction.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Multivariate to set its aux. domain slot.		     M
*   Min, Max:   The domain of this Bezier multivariate.			     M
*   Dir:        The direction where to set the MV domain.                    M
*									     M
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVAuxDomainSlotReset, MvarMVAuxDomainSlotCopy,			     M
*   MvarMVAuxDomainSlotGet, MvarMVAuxDomainSlotSetRel			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVAuxDomainSlotSet		                                     M
*****************************************************************************/
void MvarMVAuxDomainSlotSet(MvarMVStruct *MV,
			    CagdRType Min,
			    CagdRType Max,
			    int Dir)
{
    MV -> AuxDomain[Dir][0] = Min;
    MV -> AuxDomain[Dir][1] = Max;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets one aux. domain range in the given direction.  Input Min/Max is     M
* relative to current domain.			 			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Multivariate to set its aux. domain slot.		     M
*   Min, Max:   The relative to current domain new interval of this Bezier   M
*		multivariate.						     M
*   Dir:        The direction where to set the MV domain.                    M
*									     M
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVAuxDomainSlotReset, MvarMVAuxDomainSlotCopy,			     M
*   MvarMVAuxDomainSlotSet, MvarMVAuxDomainSlotGet			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVAuxDomainSlotSetRel		                                     M
*****************************************************************************/
void MvarMVAuxDomainSlotSetRel(MvarMVStruct *MV,
			       CagdRType Min,
			       CagdRType Max,
			       int Dir)
{
    CagdRType
        Dt = MV -> AuxDomain[Dir][1] - MV -> AuxDomain[Dir][0];

    MV -> AuxDomain[Dir][1] = MV -> AuxDomain[Dir][0] + Dt * Max;
    MV -> AuxDomain[Dir][0] = MV -> AuxDomain[Dir][0] + Dt * Min;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Gets one aux. domain range in the given Direction.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Multivariate to get its aux. domain slot.		     M
*   Min, Max:   The domain of this Bezier multivariate.			     M
*   Dir:        The direction to get the domain of MV.                       M
*									     M
* RETURN VALUE:                                                              M
*   int:    TRUE if aux. domain exist and can get domain, FALSE otherwise.   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVAuxDomainSlotReset, MvarMVAuxDomainSlotCopy,			     M
*   MvarMVAuxDomainSlotSet, MvarMVAuxDomainSlotSetRel			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVAuxDomainSlotGet		                                     M
*****************************************************************************/
int MvarMVAuxDomainSlotGet(const MvarMVStruct *MV,
			   CagdRType *Min,
			   CagdRType *Max,
			   int Dir)
{
    if (MV -> AuxDomain == NULL)
        return FALSE;

    *Min = MV -> AuxDomain[Dir][0];
    *Max = MV -> AuxDomain[Dir][1];
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, sets its parametric domain in all directions to be  M
* between Min and Max.							     M
*   If the MV is a Bezier, it is coerced to a Bspline first (and if InPlace  M
* TRUE, the original Bezier is freed).					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      Multivariate function to update its domain.                     M
*   Min:     New minimum domains of MV.			                     M
*   Max:     New maximum domains of MV.			                     M
*   InPlace: If TRUE, updates domain in place, unless was a Bezier that was  M
*	     converted into a Bspline, in which case the Bezier is released. M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *: Same multivariate with the updated domain in dir Axis.   M
*                                                                            *
* SEE ALSO:                                                                  M
*  MvarMVDomain, MvarMVSetAllDomains, MvarParamInDomain, MvarParamsInDomain  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVSetAllDomains, multi-variates                                      M
*****************************************************************************/
MvarMVStruct *MvarMVSetAllDomains(MvarMVStruct *MV,
				  CagdRType *Min,
				  CagdRType *Max,
				  int InPlace)
{
    int i;
    MvarMVStruct *TMV;

    if (MVAR_IS_BEZIER_MV(MV)) {
        TMV = MvarCnvrtBzr2BspMV(MV);
	if (InPlace)
	    MvarMVFree(MV);
	MV = TMV;
    }
    else
	MV = InPlace ? MV : MvarMVCopy(MV);

    for (i = 0; i < MV -> Dim; i++)
        BspKnotAffineTransOrder2(MV -> KnotVectors[i], MV -> Orders[i],
				 MV -> Orders[i] + MV -> Lengths[i],
				 Min[i], Max[i]);

    return MV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate and a domain - validate it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To make sure t is in its Dir domain.                           M
*   t:        Parameter value to verify.                                     M
*   Dir:      Direction. Either U or V or W.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:    TRUE if in domain, FALSE otherwise.                        M
*                                                                            *
* SEE ALSO:                                                                  M
*  MvarMVSetDomain, MvarMVSetAllDomains, MvarParamInDomain		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarParamInDomain, multi-variates                                        M
*****************************************************************************/
CagdBType MvarParamInDomain(const MvarMVStruct *MV,
			    CagdRType t,
			    MvarMVDirType Dir)
{
    CagdRType Min, Max;

    MvarMVDomain(MV, &Min, &Max, Dir);

    return t >= Min && t <= Max;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate and a domain - validate it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To make sure (u, v, w) is in its domain.                       M
*   Params:   Array of real valued parameters of size Dim to verify if this  M
*	      point is in MV's parametric domain.                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if in domain, FALSE otherwise.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*  MvarMVSetDomain, MvarMVSetAllDomains, MvarParamsInDomain		     M
*  MvarMVIntersPtOnBndry						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarParamsInDomain, multi-variates                                       M
*****************************************************************************/
CagdBType MvarParamsInDomain(const MvarMVStruct *MV, const CagdRType *Params)
{
    int i;

    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	case MVAR_POWER_TYPE:
	    for (i = 0; i < MV -> Dim; i++) {
		if (Params[i] < 0.0 || Params[i] > 1.0)
		    return FALSE;
	    }
	    break;
	case MVAR_BSPLINE_TYPE:
	    for (i = 0; i < MV -> Dim; i++) {
		int Order = MV -> Orders[i],
		    Len = MV -> Lengths[i];
		CagdRType
		    *KV =  MV -> KnotVectors[i];

		if (Params[i] < KV[Order - 1] || Params[i] > KV[Len])
		    return FALSE;
	    }
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    break;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a vector of MVs, some with constant degrees and invalid domain,    M
* update all MVs domains, exploiting MVs with non constant degrees' domains. M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:       To update their domains.                                      M
*   NumOfMVs:  Size of MVs vector.		                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarUpdateConstDegDomains                                                M
*****************************************************************************/
void MvarUpdateConstDegDomains(MvarMVStruct **MVs, int NumOfMVs)
{
    int i, d,
	Dim = MVs[0] -> Dim;
    CagdRType Min, Max;

    for (d = 0; d < Dim; d++) {
	for (i = 0; i < NumOfMVs; i++) {
	    if (MVs[i] -> Orders[d] > 1) {
	        /* Grab this non trivial degree's domain. */
	        MvarMVDomain(MVs[i], &Min, &Max, d);
		break;
	    }
	}
	if (i < NumOfMVs) {
	    for (i = 0; i < NumOfMVs; i++)
	        MVs[i] = MvarMVSetDomain(MVs[i], Min, Max, d, TRUE);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection point of line (PoinIns, PointOuts) with	     M
* the boundary of the domain of multivariate MV.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:		Multivariate structure.					     M
*   PointIns:   point inside the domain.				     M
*   PointOuts:  point outside the domain.				     M
*									     *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:	The intersection point.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarParamsInDomain							     M
*								             *
* KEYWORDS:                                                                  M
*   MvarMVIntersPtOnBndry		                                     M
*****************************************************************************/
MvarPtStruct *MvarMVIntersPtOnBndry(MvarMVStruct *MV, 
				    MvarPtStruct *PointIns, 
				    MvarPtStruct *PointOuts)  
{
    int i, j,
	Dim = MV -> Dim;
    CagdRType *MinDmn, *MaxDmn, Distance2, TempDist2, Lambda, M;
    MvarPtStruct *NewPoint, *TempPoint;

    NewPoint = MvarPtNew(Dim);
    TempPoint = MvarPtNew(Dim);

    MvarMVDomainAlloc(MV, &MinDmn, &MaxDmn);

    Distance2 = MvarPtDistSqrTwoPoints(PointIns, PointOuts);

    for (j = 0; j < Dim; j++) {
	if (PointOuts -> Pt[j] < MinDmn[j] + IRIT_UEPS || 
	    PointOuts -> Pt[j] > MaxDmn[j] - IRIT_UEPS) {
	    M = PointOuts -> Pt[j] < MinDmn[j] + IRIT_UEPS ? MinDmn[j] 
							   : MaxDmn[j];

	    Lambda = (M - PointIns -> Pt[j]) /
		     (PointOuts -> Pt[j] - PointIns -> Pt[j]);
	    if (Lambda < -IRIT_UEPS || Lambda > 1.0 + IRIT_UEPS) {
	        /* Failed to intersect with boundary - boundary could be   */
	        /* singular, in which case we can only abort.		   */
	        MvarPtFree(TempPoint);
		MvarPtFree(NewPoint);
		MvarMVDomainFree(MinDmn, MaxDmn);

		return NULL;
	    }

	    for (i = 0; i < Dim; i++) {
		TempPoint -> Pt[i] = PointIns -> Pt[i] +
		          Lambda * (PointOuts -> Pt[i] - PointIns -> Pt[i]);
	    }

	    TempDist2 = MvarPtDistSqrTwoPoints(PointIns, TempPoint);
	    if (TempDist2 <= Distance2) {
		Distance2 = TempDist2;
		IRIT_GEN_COPY(NewPoint -> Pt, TempPoint -> Pt, 
			 Dim * sizeof(CagdRType));
	    }
	}
    }
    MvarPtFree(TempPoint);
    MvarMVDomainFree(MinDmn, MaxDmn);

    return NewPoint;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a multi-variate, returns a sub-region of it.                         M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:        To extract a sub-region from.                                 M
*   t1, t2:    Domain to extract from MV, in parametric direction Dir.       M
*   Dir:       Direction to extract the sub-region. Either U or V or W.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   A sub-region of MV from t1 to t2 in direction Dir.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVRegionFromMV, multi-variates                                       M
*****************************************************************************/
MvarMVStruct *MvarMVRegionFromMV(const MvarMVStruct *MV,
				 CagdRType t1,
				 CagdRType t2,
				 MvarMVDirType Dir)
{
    CagdBType
	OpenEnd = MvarBspMVIsOpenInDir(MV, Dir);
    CagdRType TMin, TMax;
    MvarMVStruct *MVs, *CpMV;
    CagdBType
	BezMV = FALSE;

    switch (MV -> GType) {
	case MVAR_BEZIER_TYPE:
	    BezMV = TRUE;
	    break;
	case MVAR_BSPLINE_TYPE:
	    /* Might want to check for openend conditions here. */
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_MVAR);
	    return NULL;
    }

    MvarMVDomain(MV, &TMin, &TMax, Dir);

    if (t1 > t2)
	IRIT_SWAP(CagdRType, t1, t2);

    if (!IRIT_APX_EQ_EPS(t1, TMin, EPS_ROUND_KNOT) || !OpenEnd) {
	MVs = MvarMVSubdivAtParam(MV, t1, Dir);
	MV = CpMV = MVs -> Pnext;
	MVs -> Pnext = NULL;
	MvarMVFree(MVs);			   /* Free the first region. */
    }
    else
        CpMV = NULL;

    if (IRIT_APX_EQ_EPS(t2, TMax, EPS_ROUND_KNOT) && OpenEnd)
	return CpMV != NULL ? CpMV : MvarMVCopy(MV);
    else {
	if (BezMV)
	    t2 = (t2 - t1) / (TMax - t1);

	MVs = MvarMVSubdivAtParam(MV, t2, Dir);

	if (CpMV != NULL)
	    MvarMVFree(CpMV);

    	MvarMVFree(MVs -> Pnext);		  /* Free the second region. */
    	MVs -> Pnext = NULL;
	return MVs;				/* Returns the first region. */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a multi-variate freeform function.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       To compute a bounding box for.                                 M
*   BBox:     Where bounding information is to be saved.                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVBBox, bbox, bounding box                                           M
*****************************************************************************/
void MvarMVBBox(const MvarMVStruct *MV, MvarBBoxStruct *BBox)
{
    CagdPointsBBox(MV -> Points, MVAR_CTL_MESH_LENGTH(MV),
		   BBox -> Dim = MVAR_NUM_OF_MV_COORD(MV),
		   BBox -> Min, BBox -> Max);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a bounding box for a list of multi-variate freeform function.     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:    To compute a bounding box for.                                   M
*   BBox:   Where bounding information is to be saved.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVListBBox, bbox, bounding box                                       M
*****************************************************************************/
void MvarMVListBBox(const MvarMVStruct *MVs, MvarBBoxStruct *BBox)
{
    int l;

    for (l = 0; l < MVAR_MAX_PT_COORD; l++) {
        BBox -> Min[l] = IRIT_INFNTY;
	BBox -> Max[l] = -IRIT_INFNTY;
    }
    BBox -> Dim = 0;

    for ( ; MVs != NULL; MVs = MVs -> Pnext) {
	MvarBBoxStruct TmpBBox;

	MvarMVBBox(MVs, &TmpBBox);
	MvarMergeBBox(BBox, &TmpBBox);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges (union) two bounding boxes into one, in place.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   DestBBox:    One BBox operand as well as the result.                     M
*   SrcBBox:     Second BBox operand.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMergeBBox, bbox, bounding box                                        M
*****************************************************************************/
void MvarMergeBBox(MvarBBoxStruct *DestBBox, const MvarBBoxStruct *SrcBBox)
{
    int i,
    Dim = IRIT_MAX(DestBBox -> Dim, SrcBBox -> Dim);

    for (i = 0; i < Dim; i++) {
	if (DestBBox -> Min[i] > SrcBBox -> Min[i])
	    DestBBox -> Min[i] = SrcBBox -> Min[i];
	if (DestBBox -> Max[i] < SrcBBox -> Max[i])
	    DestBBox -> Max[i] = SrcBBox -> Max[i];
    }
    DestBBox -> Dim = Dim;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the dot product of two bounding boxes in R^n, fetching the      M
* possible values that could result from the dot product of the original     M
* multiariate data these bounding boxes bound.  Returned bbox is a scalar    M
* bbox with bounds on those possible dot product values.                     M
*   Computation is done by computing min/max value for each axis of the      M
* pair of bbox's and summing that up.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox1, BBox2:   Two bounding boxes to compute their inner product.       M
*   DProdBBox:	    Where to place the returned result.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVBBox, MvarBBoxOfCrossProd, MvarBBoxOfDotProd2                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBBoxOfDotProd	                                                     M
*****************************************************************************/
void MvarBBoxOfDotProd(const MvarBBoxStruct *BBox1,
		       const MvarBBoxStruct *BBox2,
		       MvarBBoxStruct *DProdBBox)
{
    int i,
        Dim = IRIT_MIN(BBox1 -> Dim, BBox2 -> Dim);

    MVAR_BBOX_RESET(*DProdBBox);
    DProdBBox -> Dim = 1;

    for (i = 0; i < Dim; i++) {
        IrtRType
	    v1 = BBox1 -> Min[i] * BBox2 -> Min[i],
	    v2 = BBox1 -> Min[i] * BBox2 -> Max[i],
	    v3 = BBox1 -> Max[i] * BBox2 -> Min[i],
	    v4 = BBox1 -> Max[i] * BBox2 -> Max[i];
	
	DProdBBox -> Min[0] += IRIT_MIN(IRIT_MIN(v1, v2), IRIT_MIN(v3, v4));
	DProdBBox -> Max[0] += IRIT_MAX(IRIT_MAX(v1, v2), IRIT_MAX(v3, v4));
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the dot product of two bounding boxes in R^n, fetching the      M
* possible values that could result from the dot product of the original     M
* multiariate data these bounding boxes bound.  Returned bbox is a scalar    M
* bbox with bounds on those possible dot product values.                     M
*   Computation is done by enumerating all 2^n vertices of each bbox and     M
* computing their dot products.	 Slower than MvarBBoxOfDotProd.	             M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox1, BBox2:   Two bounding boxes to compute their inner product.       M
*   DProdBBox:	    Where to place the returned result.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVBBox, MvarBBoxOfDotProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBBoxOfDotProd2	                                                     M
*****************************************************************************/
void MvarBBoxOfDotProd2(const MvarBBoxStruct *BBox1,
			const MvarBBoxStruct *BBox2,
			MvarBBoxStruct *DProdBBox)
{
    int i, ii, j, jj, k, bit,
	Dim = IRIT_MIN(BBox1 -> Dim, BBox2 -> Dim),
        n = (int) pow(2, Dim);/* Num of possible vertices of a box in R^Dim. */
    IrtRType V,
	V1[MVAR_MAX_PT_COORD], V2[MVAR_MAX_PT_COORD];

    MVAR_BBOX_RESET(*DProdBBox);
    DProdBBox -> Dim = 1;
    DProdBBox -> Min[0] = IRIT_INFNTY;
    DProdBBox -> Max[0] = -IRIT_INFNTY;

    for (i = 0; i < n; i++) {
	for (ii = 0, bit = 1; ii < Dim; ii++, bit *= 2) /* Build vertex V1. */
	    V1[ii] = (i & bit) ? BBox1 -> Max[ii] : BBox1 -> Min[ii];

	for (j = 0; j < n; j++) {
	    for (jj = 0, bit = 1; jj < Dim; jj++, bit *= 2)    /* Build V2. */
	        V2[jj] = (j & bit) ? BBox2 -> Max[jj] : BBox2 -> Min[jj];

	    /* Compute the inner product and update result scalar bbox. */
	    for (k = 0, V = 0.0; k < Dim; k++)
	        V += V1[k] * V2[k];

	    if (DProdBBox -> Min[0] > V)
	        DProdBBox -> Min[0] = V;
	    if (DProdBBox -> Max[0] < V)
	        DProdBBox -> Max[0] = V;
	}
    }
}

/*****************************************************************************
* AUXILIARY:								     *
*   Aux. function of MvarBBoxOfCrossProd to compute bounding interval to     *
*     BBox1[Axis1] BBox2[Axis2] - BBox1[Axis2] Bbox2[Axis1].                 *
*                                                                            *
* PARAMETERS:                                                                *
*   BBox1:   First bbox to cvonsider in the cross-prod computation.	     *
*   Axis1:   Axis into Bbox1.                                                *
*   BBox2:   Second bbox to cvonsider in the cross-prod computation.	     *
*   Axis2:   Axis into Bbox2.                                                *
*   Min:     Lower bound on the computed interval.                           *
*   Max:     Upper bound on the computed interval.                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBBoxOfCrossProdAux(const MvarBBoxStruct *BBox1,
				   int Axis1,
				   const MvarBBoxStruct *BBox2,
				   int Axis2,
				   CagdRType *Min,
				   CagdRType *Max)
{
    IrtRType
	Term1Min = IRIT_MIN(IRIT_MIN(BBox1 -> Min[Axis1] * BBox2 -> Min[Axis2],
			   BBox1 -> Min[Axis1] * BBox2 -> Max[Axis2]),
		       IRIT_MIN(BBox1 -> Max[Axis1] * BBox2 -> Min[Axis2],
			   BBox1 -> Max[Axis1] * BBox2 -> Max[Axis2])),
        Term1Max = IRIT_MAX(IRIT_MAX(BBox1 -> Min[Axis1] * BBox2 -> Min[Axis2],
				     BBox1 -> Min[Axis1] * BBox2 -> Max[Axis2]),
		       IRIT_MAX(BBox1 -> Max[Axis1] * BBox2 -> Min[Axis2],
				BBox1 -> Max[Axis1] * BBox2 -> Max[Axis2])),

	Term2Min = IRIT_MIN(IRIT_MIN(BBox1 -> Min[Axis2] * BBox2 -> Min[Axis1],
			   BBox1 -> Min[Axis2] * BBox2 -> Max[Axis1]),
		       IRIT_MIN(BBox1 -> Max[Axis2] * BBox2 -> Min[Axis1],
			   BBox1 -> Max[Axis2] * BBox2 -> Max[Axis1])),
        Term2Max = IRIT_MAX(IRIT_MAX(BBox1 -> Min[Axis2] * BBox2 -> Min[Axis1],
				     BBox1 -> Min[Axis2] * BBox2 -> Max[Axis1]),
		       IRIT_MAX(BBox1 -> Max[Axis2] * BBox2 -> Min[Axis1],
				BBox1 -> Max[Axis2] * BBox2 -> Max[Axis1]));

    *Min = Term1Min - Term2Max;
    *Max = Term1Max - Term2Min;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the cross product of two bounding boxes in R^3, fetching the    M
* possible values that could result from the cross product of the original   M
* multiariate data these bounding boxes bound.  Returned bbox is a vector    M
* bbox with bounds on those possible cross product values.                   M
*   Computation is done by computing min/max value for each axis of the      M
* pair of bbox's cross product.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   BBox1, BBox2:   Two bounding boxes to compute their cross product.       M
*   DCrossBBox:	    Where to place the returned result.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void			                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVBBox, MvarBBoxOfDotProd                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBBoxOfCrossProd	                                                     M
*****************************************************************************/
void MvarBBoxOfCrossProd(const MvarBBoxStruct *BBox1,
			 const MvarBBoxStruct *BBox2,
			 MvarBBoxStruct *DCrossBBox)
{
    assert(IRIT_MIN(BBox1 -> Dim, BBox2 -> Dim) >= 3);

    MVAR_BBOX_RESET(*DCrossBBox);
    DCrossBBox -> Dim = 3;

    MvarBBoxOfCrossProdAux(BBox1, 1, BBox2, 2,
			   &DCrossBBox -> Min[0], &DCrossBBox -> Max[0]);
    MvarBBoxOfCrossProdAux(BBox1, 2, BBox2, 0,
			   &DCrossBBox -> Min[1], &DCrossBBox -> Max[1]);
    MvarBBoxOfCrossProdAux(BBox1, 0, BBox2, 1,
			   &DCrossBBox -> Min[2], &DCrossBBox -> Max[2]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increment the index of the control mesh of the multivariate function     M
* by one.   Should only be called via the macro MVAR_INCREMENT_MESH_INDICES. M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To increment Indices to its control mesh.                    M
*   Indices:    To increment one step.                                       M
*   Index:      The total current index to be incremented as well, or zero   M
*		if we wrapped around all incides.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        The non zero advanced index if indices are in domain, zero   M
*		if done (out of domain).				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   _MvarIncSkipMeshIndices, _MvarIncBoundMeshIndices,			     M
*   _MvarIncSkipMeshIndices1st, _MvarIncrementMeshOrderIndices		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _MvarIncrementMeshIndices                                                M
*****************************************************************************/
int _MvarIncrementMeshIndices(const MvarMVStruct *MV, int *Indices, int *Index)
{
    int i;

    /* This first index overflows in MVAR_INCREMENT_MESH_INDICES. */
    *Indices++ = 0;

    for (i = 0; ++i < MV -> Dim; ) {
	if (++(*Indices) < MV -> Lengths[i]) {
	    return ++(*Index);
	}
	*Indices++ = 0;
    }

    return *Index = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increment the index of the control mesh of the multivariate function     M
* by one.   Should only be called via the macro				     M
* MVAR_INCREMENT_MESH_ORDER_INDICES.  This macro is useful when traversing   M
* a Bspline mesh for evaluation, Orders[i] control points in i'th dimension. M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To increment Indices to its control mesh.                    M
*   Indices:    To increment one step.                                       M
*   Index:      The total current index to be incremented as well, or zero   M
*		if we wrapped around all incides.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        The non zero advanced index if indices are in domain, zero   M
*		if done (out of domain).				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   _MvarIncSkipMeshIndices, _MvarIncBoundMeshIndices,			     M
*   _MvarIncSkipMeshIndices1st				                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   _MvarIncrementMeshOrderIndices                                           M
*****************************************************************************/
int _MvarIncrementMeshOrderIndices(const MvarMVStruct *MV,
				   int *Indices,
				   int *Index)
{
    int i;

    /* This first index overflows in MVAR_INCREMENT_MESH_INDICES. */
    *Indices++ = 0;
    *Index -= MV -> Orders[0];

    for (i = 0; ++i < MV -> Dim; ) {
	if (++(*Indices) < MV -> Orders[i]) {
	    *Index += MV -> SubSpaces[i];
	    return ++(*Index);
	}
	*Indices++ = 0;
	*Index -= (MV -> Orders[i] - 1) * MV -> SubSpaces[i];
    }

    return *Index = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increment the index of the control mesh of the multivariate function     M
* by one, skipping axis Dir zero.  Should only be called via the macro       M
* MVAR_INC_SKIP_MESH_INDICES_1ST.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To increment Indices to its control mesh.                    M
*   Indices:    To increment one step.                                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if Indices are in domain, FALSE if done.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   _MvarIncSkipMeshIndices, _MvarIncBoundMeshIndices,			     M
*   _MvarIncrementMeshIndices, _MvarIncrementMeshOrderIndices                M
*                                                                            *
* KEYWORDS:                                                                  M
*   _MvarIncSkipMeshIndices1st                                               M
*****************************************************************************/
int _MvarIncSkipMeshIndices1st(const MvarMVStruct *MV, int *Indices)
{
    int i;

    /* This first index overflows in MVAR_INC_SKIP_MESH_INDICES_1ST. */
    *++Indices = 0;
    Indices++;

    for (i = 1; ++i < MV -> Dim; ) {
	if (++(*Indices) < MV -> Lengths[i])
	    return TRUE;
	*Indices++ = 0;
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increment the index of the control mesh of the multivariate function     M
* by one, skipping axis Dir.  Should only be called via the macro            M
* MVAR_INC_SKIP_MESH_INDICES.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To increment Indices to its control mesh.                    M
*   Indices:    To increment one step.                                       M
*   Dir:	To skip in the incrementation.                               M
*   Index:      The total current index to be incremented as well, or zero   M
*		if we wrapped around all incides.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Current non negative Index if Indices are in domain, zero    M
*		(FALSE) if done - out of the domain.		             M
*                                                                            *
* SEE ALSO:                                                                  M
*   _MvarIncBoundMeshIndices, _MvarIncSkipMeshIndices1st,		     M
*   _MvarIncrementMeshIndices, _MvarIncrementMeshOrderIndices                M
*                                                                            *
* KEYWORDS:                                                                  M
*   _MvarIncSkipMeshIndices                                                  M
*****************************************************************************/
int _MvarIncSkipMeshIndices(const MvarMVStruct *MV,
			    int *Indices,
			    int Dir,
			    int *Index)
{
    int i;

    switch (Dir) {
        case 0:
	    /* This first index overflows in MVAR_INC_SKIP_MESH_INDICES_1ST. */
	    Indices[1] = 0;
	    *Index -= (MV -> Lengths[1] - 1) * MVAR_NEXT_DIM(MV, 1);

	    for (i = 2; i < MV -> Dim; i++) {
	        if (++(Indices[i]) < MV -> Lengths[i])
		    return (*Index += MVAR_NEXT_DIM(MV, i));
		Indices[i] = 0;
		*Index -= (MV -> Lengths[i] - 1) * MVAR_NEXT_DIM(MV, i);
	    }
	    break;

        case 1:
	    /* This first index overflows in MVAR_INC_SKIP_MESH_INDICES_1ST. */
	    Indices[0] = 0;
	    *Index -= (MV -> Lengths[0] - 1) * MVAR_NEXT_DIM(MV, 0);

	    for (i = 2; i < MV -> Dim; i++) {
	        if (++(Indices[i]) < MV -> Lengths[i])
		    return (*Index += MVAR_NEXT_DIM(MV, i));
		Indices[i] = 0;
		*Index -= (MV -> Lengths[i] - 1) * MVAR_NEXT_DIM(MV, i);
	    }
	    break;

        default:
	    /* This first index overflows in MVAR_INC_SKIP_MESH_INDICES_1ST. */
	    Indices[0] = 0;
	    *Index -= (MV -> Lengths[0] - 1) * MVAR_NEXT_DIM(MV, 0);

	    for (i = 1; i < MV -> Dim; ++i == Dir ? i++ : i) {
	        if (++(Indices[i]) < MV -> Lengths[i])
		    return (*Index += MVAR_NEXT_DIM(MV, i));
		Indices[i] = 0;
		*Index -= (MV -> Lengths[i] - 1) * MVAR_NEXT_DIM(MV, i);
	    }
    }

    return *Index = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Increment the index of the control mesh of the multivariate function     M
* by one, with given lower and upper bounds: LowerBound <= Idx < UpperBound. M
*   Should only be called via the macro MVAR_INC_BOUND_MESH_INDICES.  	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         To increment Indices to its control mesh.                    M
*   Indices:    To increment one step.                                       M
*   LowerBound:	Minimal values to assume.                                    M
*   UpperBound:	One above the maximal values to assume.                      M
*   Index:	Index to increment.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if Indices are in domain, FALSE if done.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   _MvarIncSkipMeshIndices, _MvarIncSkipMeshIndices1st,		     M
*   _MvarIncrementMeshIndices, _MvarIncrementMeshOrderIndices                M
*                                                                            *
* KEYWORDS:                                                                  M
*   _MvarIncBoundMeshIndices                                                 M
*****************************************************************************/
int _MvarIncBoundMeshIndices(const MvarMVStruct *MV,
			     int *Indices,
			     int *LowerBound,
			     int *UpperBound,
			     int *Index)
{
    int i;

    /* This first index overflows in MVAR_INC_BOUND_MESH_INDICES. */
    *Indices++ = *LowerBound;

    /* Note UpperBound should be larger than LowerBound so for an empty */
    /* dimension UpperBound should be 1, but just in case it is 0...    */
    *Index -= (*UpperBound == 0 ? *UpperBound++ - *LowerBound++
		                : *UpperBound++ - *LowerBound++ - 1);

    for (i = 0; ++i < MV -> Dim; ) {
	if (++(*Indices) < *UpperBound)
	    return (*Index += MVAR_NEXT_DIM(MV, i));
	*Indices++ = *LowerBound;

	/* Note UpperBound should be larger than LowerBound so for an empty */
	/* dimension UpperBound should be 1, but just in case if it is 0... */
	*Index -= (*UpperBound == 0 ? *UpperBound++ - *LowerBound++
		                    : *UpperBound++ - *LowerBound++ - 1)
						       * MVAR_NEXT_DIM(MV, i);
    }

    return *Index = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given indices into the control mesh, return the index in the vector      M
* representation Points of that single point.                                M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Whose indices are for.                                       M
*   Indices:    To compute the exact point location in MV -> Points          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Index of point whose indices are Indices in MV -> Points.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMeshIndicesFromIndex, MvarGetPointsPeriodicMeshIndices               M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarGetPointsMeshIndices                                                 M
*****************************************************************************/
int MvarGetPointsMeshIndices(const MvarMVStruct *MV, int *Indices)
{
    int i, Index,
        *SubSpaces = MV -> SubSpaces;

    switch (MV -> Dim) {
	case 1:
	    return *SubSpaces * *Indices;
	case 2:
	    return SubSpaces[0] * Indices[0] +
	           SubSpaces[1] * Indices[1];
	case 3:
	    return SubSpaces[0] * Indices[0] +
	           SubSpaces[1] * Indices[1] +
	           SubSpaces[2] * Indices[2];
	case 4:
	    return SubSpaces[0] * Indices[0] +
	           SubSpaces[1] * Indices[1] +
	           SubSpaces[2] * Indices[2] +
	           SubSpaces[3] * Indices[3];
	case 5:
	    return SubSpaces[0] * Indices[0] +
	           SubSpaces[1] * Indices[1] +
	           SubSpaces[2] * Indices[2] +
	           SubSpaces[3] * Indices[3] +
	           SubSpaces[4] * Indices[4];
	default:
	    Index = 0;
	    for (i = 0; i < MV -> Dim; i++)
	        Index += *SubSpaces++ * *Indices++;
	    return Index;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given indices into the control mesh, return the index in the vector      M
* representation Points of that single point.  MV can be periodic.           M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:         Whose indices are for.                                       M
*   Indices:    To compute the exact point location in MV -> Points          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        Index of point whose indices are Indices in MV -> Points.    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMeshIndicesFromIndex, MvarGetPointsMeshIndices                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarGetPointsPeriodicMeshIndices                                         M
*****************************************************************************/
int MvarGetPointsPeriodicMeshIndices(const MvarMVStruct *MV, int *Indices)
{
    int i, Index,
        *SubSpaces = MV -> SubSpaces,
	*Lengths = MV -> Lengths;

    switch (MV -> Dim) {
	case 1:
	    return *SubSpaces * (*Indices % *Lengths);
	case 2:
	    return SubSpaces[0] * (Indices[0] % Lengths[0]) +
	           SubSpaces[1] * (Indices[1] % Lengths[0]);
	case 3:
	    return SubSpaces[0] * (Indices[0] % Lengths[0]) +
	           SubSpaces[1] * (Indices[1] % Lengths[1]) +
	           SubSpaces[2] * (Indices[2] % Lengths[2]);
	case 4:
	    return SubSpaces[0] * (Indices[0] % Lengths[0]) +
	           SubSpaces[1] * (Indices[1] % Lengths[1]) +
	           SubSpaces[2] * (Indices[2] % Lengths[2]) +
	           SubSpaces[3] * (Indices[3] % Lengths[3]);
	case 5:
	    return SubSpaces[0] * (Indices[0] % Lengths[0]) +
	           SubSpaces[1] * (Indices[1] % Lengths[1]) +
	           SubSpaces[2] * (Indices[2] % Lengths[2]) +
	           SubSpaces[3] * (Indices[3] % Lengths[3]) +
	           SubSpaces[4] * (Indices[4] % Lengths[4]);
	default:
	    Index = 0;
	    for (i = 0; i < MV -> Dim; i++)
	        Index += *SubSpaces++ * (*Indices++ % *Lengths++);
	    return Index;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a linear Index into the vector of control points, compute the      M
* indices of the multivariates in all dimensions.                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Index:      To decompose into the different axes of the multivariate.    M
*   MV:         Whose indices are for.                                       M
*   Indices:    To compute the exact point location in MV -> Points          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        TRUE if Index in range, false otherwise.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarGetPointsMeshIndices                                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMeshIndicesFromIndex                                                 M
*****************************************************************************/
int MvarMeshIndicesFromIndex(int Index, const MvarMVStruct *MV, int *Indices)
{
    int i;

    for (i = MV -> Dim - 1; i >= 0; i--) {
	Indices[i] = Index / MV -> SubSpaces[i];
        Index -= Indices[i] * MV -> SubSpaces[i];

	if (Index < 0 || Indices[i] >= MV -> Lengths[i])
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Merges two multivariates in the requested direction Dir.		     M
*  It is assumed that last edge of MV1 is identical to first edge of MV2.    M
*  It is assumed that both MVs have open end conditions and share the same   M
* orders and knot sequences in all axes, but the merged axes which can have  M
* different knots.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   CMV1:        To connect to CMV2's starting boundary at its end.	     M
*   CMV2:        To connect to CMV1's end boundary at its start. 	     M
*   Dir:         Direction the merge should take place.			     M
*   Discont:     If TRUE, assumes the merged "edge" is discontinuous.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:     The merged multivariate.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdMergeSrfSrf                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMergeMVMV, merge, multivariate                                       M
*****************************************************************************/
MvarMVStruct *MvarMergeMVMV(const MvarMVStruct *CMV1,
			    const MvarMVStruct *CMV2,
			    MvarMVDirType Dir,
			    CagdBType Discont)
{
    CagdBType IsNotRational,
	MergedEdgeDiscont = Discont ||
			    CMV1 -> Orders[Dir] == 1 ||
                            CMV2 -> Orders[Dir] == 1;
    int i, MaxCoord, *Lengths, *MergedIndices, Index1, Index2, MergedIndex,
	*LowerBound, *UpperBound, IsScalar;
    CagdRType **MergedPoints, **Points1, **Points2;
    MvarMVStruct *MergedMV, *MV1, *MV2;

    if (CMV1 -> Dim != CMV2 -> Dim ||
	CMV1 -> GType != CMV2 -> GType ||
	CMV1 -> PType != CMV2 -> PType)  {
	MVAR_FATAL_ERROR(MVAR_ERR_FAIL_CMPT);
	return NULL;
    }

    /* Verify multivariate geometric types. */
    switch (CMV1 -> GType) {
	case MVAR_BEZIER_TYPE:
	    MV1 = MvarCnvrtBzr2BspMV(CMV1);
	    MV2 = MvarCnvrtBzr2BspMV(CMV2);
	    break;
	case MVAR_BSPLINE_TYPE:
	    MV1 = MvarMVCopy(CMV1);
	    MV2 = MvarMVCopy(CMV2);
	    break;
	default:
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEF_GEOM);
	    return NULL;
    }

    IsNotRational = !MVAR_IS_RATIONAL_MV(MV1);
    MaxCoord = MVAR_NUM_OF_MV_COORD(MV1);
    IsScalar = IsNotRational && MaxCoord == 1;

    Lengths = (int *) IritMalloc(sizeof(int) * MV1 -> Dim);
    for (i = 0; i < MV1 -> Dim; i++) {
	if (i == Dir)
	    Lengths[i] = MV1 -> Lengths[i] + MV2 -> Lengths[i] - 1 +
							    MergedEdgeDiscont;
	else if (MV1 -> Lengths[i] == MV2 -> Lengths[i])
	    Lengths[i] = MV1 -> Lengths[i];
	else {
	    MvarMVFree(MV1);
	    MvarMVFree(MV2);
	    MVAR_FATAL_ERROR(MVAR_ERR_MVS_INCOMPATIBLE);
	    return NULL;
	}
    }

    MergedMV = MvarBspMVNew(MV1 -> Dim, Lengths, MV1 -> Orders,
			    MV1 -> PType);
    IritFree(Lengths);

    /* Update knot vectors. We assume open end condition here... */
    for (i = 0; i < MV1 -> Dim; i++) {
	CAGD_GEN_COPY(MergedMV -> KnotVectors[i], MV1 -> KnotVectors[i],
		      (MV1 -> Lengths[i] + MV1 -> Orders[i])
		                                          * sizeof(CagdRType));
	if (i == Dir) {
	    /* Append the second knot vector and affine transform so it      */
	    /* directly continues the first with Degree knots at connection. */
	    CAGD_GEN_COPY(&MergedMV -> KnotVectors[i][MV1 -> Lengths[i] +
						      MV1 -> Orders[i] - 1 +
						      MergedEdgeDiscont],
			  &MV2 -> KnotVectors[i][MV2 -> Orders[i]],
			  MV2 -> Lengths[i] * sizeof(CagdRType));
	    BspKnotAffineTrans(&MergedMV -> KnotVectors[i][MV1 -> Lengths[i] +
							   MV1 -> Orders[i] - 1],
			       MV2 -> Lengths[i],
			       MergedMV -> KnotVectors[i][MV1 -> Lengths[i] +
							  MV1 -> Orders[i] - 2 +
						          MergedEdgeDiscont] -
			           MV2 -> KnotVectors[i][0],
			       1.0);
	}
    }

    MergedPoints = MergedMV -> Points;
    Points1 = MV1 -> Points;
    Points2 = MV2 -> Points;

    MergedIndices = (int *) IritMalloc(sizeof(int) * MergedMV -> Dim);
    LowerBound = (int *) IritMalloc(sizeof(int) * MergedMV -> Dim);
    UpperBound = (int *) IritMalloc(sizeof(int) * MergedMV -> Dim);

    IRIT_ZAP_MEM(LowerBound, sizeof(int) * MergedMV -> Dim);
    CAGD_GEN_COPY(UpperBound, MergedMV -> Lengths,
		  MergedMV -> Dim * sizeof(int));

    /* Copy the first control mesh into the merged multivariate. */
    UpperBound[Dir] = MV1 -> Lengths[Dir];
    IRIT_ZAP_MEM(MergedIndices, sizeof(int) * MergedMV -> Dim);
    Index1 = MergedIndex = 0;
    if (IsScalar) {
        do {
	    MergedPoints[1][MergedIndex] = Points1[1][Index1];

	    MVAR_INC_BOUND_MESH_INDICES(MergedMV, MergedIndices,
					LowerBound, UpperBound, MergedIndex);
	}
	while (++Index1 < MVAR_CTL_MESH_LENGTH(MV1));
    }
    else {
        do {
	    for (i = IsNotRational; i <= MaxCoord; i++)
	        MergedPoints[i][MergedIndex] = Points1[i][Index1];

	    MVAR_INC_BOUND_MESH_INDICES(MergedMV, MergedIndices,
					LowerBound, UpperBound, MergedIndex);
	}
	while (++Index1 < MVAR_CTL_MESH_LENGTH(MV1));
    }

    /* Copy the second control mesh into the merged multivariate. */
    LowerBound[Dir] = MV1 -> Lengths[Dir] - 1 + MergedEdgeDiscont;
    UpperBound[Dir] = MergedMV -> Lengths[Dir];
    Index2 = 0;
    CAGD_GEN_COPY(MergedIndices, LowerBound, sizeof(int) * MergedMV -> Dim);
    MergedIndex = MvarGetPointsMeshIndices(MergedMV, MergedIndices);
    if (IsScalar) {
        do {
	    MergedPoints[1][MergedIndex] = Points2[1][Index2];

	    MVAR_INC_BOUND_MESH_INDICES(MergedMV, MergedIndices,
					LowerBound, UpperBound, MergedIndex);
	}
	while (++Index2 < MVAR_CTL_MESH_LENGTH(MV2));

    }
    else {
        do {
	    for (i = IsNotRational; i <= MaxCoord; i++)
	        MergedPoints[i][MergedIndex] = Points2[i][Index2];

	    MVAR_INC_BOUND_MESH_INDICES(MergedMV, MergedIndices,
					LowerBound, UpperBound, MergedIndex);
	}
	while (++Index2 < MVAR_CTL_MESH_LENGTH(MV2));
    }

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    IritFree(MergedIndices);
    IritFree(LowerBound);
    IritFree(UpperBound);

    return MergedMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a Bspline multivariate into a Bspline multivariate with floating  M
* end conditions.                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Bspline multivariate to convert to floating end conditions.    M
*             Assume MV is either periodic or has floating end condition.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A Bspline multivariate with floating end conditions,    M
*                     representing the same geometry as MV.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CnvrtPeriodic2FloatMV, MvarCnvrtFloat2OpenMV			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCnvrtPeriodic2FloatMV, conversion                                    M
*****************************************************************************/
MvarMVStruct *MvarCnvrtPeriodic2FloatMV(const MvarMVStruct *MV)
{
    int i, NewIdx, *Lengths, *Indices, *NewIndices,
	Dim = MV -> Dim,
	MaxAxis = MVAR_NUM_OF_MV_COORD(MV);
    MvarMVStruct *NewMV;

    if (!MVAR_IS_BSPLINE_MV(MV)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BSPLINE_EXPECTED);
	return NULL;
    }

    for (i = 0; i < Dim; i++)
        if (MVAR_IS_ITH_PERIODIC_MVAR(MV, i))
	    break;
    if (i >= MV -> Dim)
        return MvarMVCopy(MV);

    Lengths = (int *) IritMalloc(Dim * sizeof(int));
    for (i = 0; i < Dim; i++)
	Lengths[i] = MVAR_MVAR_ITH_PT_LST_LEN(MV, i);
    NewMV = MvarBspMVNew(Dim, Lengths, MV -> Orders, MV -> PType);
    IritFree(Lengths);

    for (i = 0; i < Dim; i++)
        CAGD_GEN_COPY(NewMV -> KnotVectors[i], MV -> KnotVectors[i],
		      sizeof(CagdRType) * (MVAR_MVAR_ITH_PT_LST_LEN(MV, i) +
					   MV -> Orders[i]));

    Indices = (int *) IritMalloc(Dim * sizeof(int));
    NewIndices = (int *) IritMalloc(Dim * sizeof(int));
    NewIdx = 0;
    IRIT_ZAP_MEM(NewIndices, sizeof(int) * Dim);
    do {
        int Idx;

	for (i = 0; i < Dim; i++)
	    Indices[i] = NewIndices[i] % MV -> Lengths[i];

	Idx = MvarGetPointsMeshIndices(MV, Indices);

	for (i = !MVAR_IS_RATIONAL_PT(MV -> PType); i <= MaxAxis; i++)
	    NewMV -> Points[i][NewIdx] = MV -> Points[i][Idx];
    }
    while (MVAR_INCREMENT_MESH_INDICES(NewMV, NewIndices, NewIdx));

    IritFree(NewIndices);
    IritFree(Indices);

    for (i = MaxAxis + 1; i <= MVAR_MAX_PT_COORD; i++)
	NewMV -> Points[i] = NULL;

    CAGD_PROPAGATE_ATTR(NewMV, MV);

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Converts a float Bspline multivariate to a Bspline multivariate with open  M
* end conditions.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:       Bspline multivariate to convert to open end conditions.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  A Bspline multivariate with open end conditions,	     M
*                     representing the same geometry as MV.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarCnvrtPeriodic2FloatMV, CnvrtFloat2OpenMV                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarCnvrtFloat2OpenMV, conversion                                        M
*****************************************************************************/
MvarMVStruct *MvarCnvrtFloat2OpenMV(const MvarMVStruct *MV)
{
    int i,
	Dim = MV -> Dim;
    MvarMVStruct
	*NewMV = MvarMVCopy(MV);

    if (MvarBspMVIsOpen(NewMV))
	return NewMV;

    if (!MVAR_IS_BSPLINE_MV(MV)) {
	MVAR_FATAL_ERROR(MVAR_ERR_BSPLINE_EXPECTED);
	return NULL;
    }

    for (i = 0; i < Dim; i++) {
	if (!MvarBspMVIsOpenInDir(NewMV, i)) {
	    CagdRType Min, Max;
	    MvarMVStruct *TmpMV;

	    MvarMVDomain(NewMV, &Min, &Max, i);

	    TmpMV = MvarMVRegionFromMV(NewMV, Min, Max, i);

	    MvarMVFree(NewMV);
	    NewMV = TmpMV;
	}
    }

    CAGD_PROPAGATE_ATTR(NewMV, MV);

    return NewMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline multivariate has open end coditions in  M
* the specified direction Dir.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      To check for open end conditions.                               M
*   Dir:     Direction to test for open end conditions.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if MV has open end conditions in Dir, FALSE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvHasOpenEC, MvarBspMVIsOpen, MvarBspMVIsPeriodic,		     M
*   MvarBspMVIsPeriodicInDir			                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVIsOpenInDir, open end conditions                                M
*****************************************************************************/
CagdBType MvarBspMVIsOpenInDir(const MvarMVStruct *MV, MvarMVDirType Dir)
{
    if (!MVAR_IS_BSPLINE_MV(MV))
	return TRUE;

    return BspKnotHasOpenEC(MV -> KnotVectors[Dir],
			    MV -> Lengths[Dir],
			    MV -> Orders[Dir]);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline multivariate has open end coditions in  M
* all direction directions.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      To check for open end conditions.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if MV has open end conditions in all directions, FALSE M
*		otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvHasOpenEC, MvarBspMVIsOpenInDir, MvarBspMVIsOpenInDir              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVIsOpen, open end conditions   	                             M
*****************************************************************************/
CagdBType MvarBspMVIsOpen(const MvarMVStruct *MV)
{
    CagdBType
	Open = TRUE;
    int i;

    if (MVAR_IS_BEZIER_MV(MV))
	return TRUE;

    for (i = 0; i < MV -> Dim; i++)
        Open = (Open && BspKnotHasOpenEC(MV -> KnotVectors[i],
					 MV -> Lengths[i],
					 MV -> Orders[i]));

    return Open;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline multivariate has periodic end coditions M
* in the specified direction Dir.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      To check for periodic end conditions.                           M
*   Dir:     Direction to test for periodic end conditions.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if MV has periodic end conditions in Dir, FALSE	     M
*		otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvHasOpenEC, MvarBspMVIsOpen, MvarBspMVIsOpenInDir,		     M
*   MvarBspMVIsPeriodic							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVIsPeriodicInDir, periodic end conditions                        M
*****************************************************************************/
CagdBType MvarBspMVIsPeriodicInDir(const MvarMVStruct *MV, MvarMVDirType Dir)
{
    return MV -> Periodic[Dir];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns TRUE iff the given Bspline multivariate has periodic end coditions M
* in at least one direction.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      To check for periodic end conditions.                           M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE, if MV has periodic end conditions in some Dir, FALSE   M
*		otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvHasOpenEC, MvarBspMVIsOpen, MvarBspMVIsOpenInDir,		     M
*   MvarBspMVIsPeriodicInDir, MvarBspMVInteriorKnots		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVIsPeriodic, periodic end conditions                             M
*****************************************************************************/
CagdBType MvarBspMVIsPeriodic(const MvarMVStruct *MV)
{
    CagdBType
	Periodic = FALSE;
    int i;

    for (i = 0; i < MV -> Dim; i++)
        Periodic = (Periodic || MV -> Periodic[i]);

    return Periodic;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Returns -1 if the given Bspline multivariate has not interior knots in     M
* no direction.	  Otherwise, return the direction that has an interior knot  M
* and returns the knot velue in Knot.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   MV:      To check for interior knots.		                     M
*   Knot:    Where to return an interior knot if found one.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  -1 if MV has no interior knots, Axis of interior knot        M
*		otherwise.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrvHasOpenEC, MvarBspMVIsOpen, MvarBspMVIsOpenInDir,		     M
*   MvarBspMVIsPeriodicInDir					             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBspMVInteriorKnots, Internal knots, Bezier                           M
*****************************************************************************/
CagdBType MvarBspMVInteriorKnots(const MvarMVStruct *MV, CagdRType *Knot)
{
    int j,
	DirHoldingKnot = -1,
	NumOfInteriorKnots = -1,
        Dim = MV -> Dim;

    for (j = 0; j < Dim; j++) {
        if (MV -> Lengths[j] != MV -> Orders[j] &&
	    MV -> Lengths[j] - MV -> Orders[j] > NumOfInteriorKnots) {
	    /* Found a direction with internal knots. */
	    *Knot = MV -> KnotVectors[j]
				     [(MV -> Lengths[j] + MV -> Orders[j])/ 2];
	    NumOfInteriorKnots = MV -> Lengths[j] - MV -> Orders[j];
	    DirHoldingKnot = j;
	}
    }

    return DirHoldingKnot;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Creates a Bezier multivariate that is constant in all directions but one.M 
*                                                                            *
* PARAMETERS:                                                                M
*   Dim:   DImension f the sought multivariate Bezier.                       M
*   Dir:   The direction that is to be linear, between 0 and Dim-1.          M
*   PType: Type of points of this new multivariate.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:   The constructed multivariate.                          M
*                                                                            *
* SEE ALSO:                                                                  M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBzrLinearInOneDir                                                    M
*****************************************************************************/
MvarMVStruct *MvarBzrLinearInOneDir(int Dim, int Dir, MvarPointType PType)
{
    int i, k,
	*Lens = (int *) IritMalloc(sizeof(int) * Dim);
    MvarMVStruct *MV;

    assert(Dir >= 0 && Dir < Dim);

    for (i = 0; i < Dim; i++)
        Lens[i] = i == Dir ? 2 : 1;

    MV = MvarBzrMVNew(Dim, Lens, PType);
    
    for (k = !CAGD_IS_RATIONAL_PT(PType);
	 k <= CAGD_NUM_OF_PT_COORD(PType);
	 k++) {
        MV -> Points[k][0] = 0.0;
	MV -> Points[k][1] = 1.0;
    }

    IritFree(Lens);

    return MV;
}
