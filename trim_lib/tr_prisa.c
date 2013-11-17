/******************************************************************************
* Tr_Prisa.c - piecewise ruled srf approx and layout (prisa for trimmed srfs) *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Aug. 97					      *
******************************************************************************/

#include "trim_loc.h"
#include "symb_lib.h"
#include "allocate.h"
#include "geom_lib.h"

static TrimSrfStruct *TrimPiecewiseRuledSrfAux(TrimSrfStruct *Srf,
					       CagdBType ConsistentDir,
					       CagdRType Epsilon,
					       CagdSrfDirType Dir);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a piecewise ruled surface approximation to a given set of         M
* trimmed surfaces with given Epsilon, and lay them out "nicely" onto the XY M
* plane, by approximating each ruled surface as a developable surface with   M
* SamplesPerCurve samples.						     M
*   Dir controls the direction of ruled approximation, SpaceScale and        M
* Offset controls the placement of the different planar pieces.		     M
*   Prisa is the hebrew word for the process of flattening out a three       M
* dimensional surface. I have still to find an english word for it.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrfs:           To approximate and flatten out.                         M
*   SamplesPerCurve: During the approximation of a ruled surface as a        M
*		     developable surface.				     M
*   Epsilon:         Accuracy control for the piecewise ruled surface        M
*		     approximation.					     M
*		     if Epsilon us positive, the surfaces are laid down on   M
*		     the plane, otherwise they are return as 3-space ruled   M
*		     surfaces and form a piecewise ruled-surface	     M
*		     approximation to Srfs.				     M
*   Dir:             Direction of ruled/developable surface approximation.   M
*		     Either U or V.					     M
*   Space:           A vector in the XY plane to denote the amount of        M
*		     translation from one flattened out surface to the next. M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   A list of planar trimmed surfaces denoting the layout M
*		       (prisa) of the given TSrfs to the accuracy requested. M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimPiecewiseRuledSrfApprox, TrimPrisaRuledSrf, SymbAllPrisaSrfs         M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimAllPrisaSrfs, layout, prisa                                          M
*****************************************************************************/
TrimSrfStruct *TrimAllPrisaSrfs(const TrimSrfStruct *TSrfs,
				int SamplesPerCurve,
			        CagdRType Epsilon,
				CagdSrfDirType Dir,
			        CagdVType Space)
{
    int SrfIndex = 1;
    const TrimSrfStruct *TSrf;
    TrimSrfStruct 
	*PrisaTSrfsAll = NULL;
    CagdVType Offset;

    for (TSrf = TSrfs; TSrf != NULL; TSrf = TSrf -> Pnext, SrfIndex++) {
	TrimSrfStruct *PSrf, *RSrf, *RuledSrfs;

	if (Epsilon > 0) {
	    RuledSrfs = TrimPiecewiseRuledSrfApprox(TSrf, TRUE, Epsilon, Dir);

	    Offset[0] = SrfIndex * Space[0];
	    Offset[1] = 0.0;
	    Offset[2] = 0.0;

	    for (RSrf = RuledSrfs; RSrf != NULL; RSrf = RSrf -> Pnext) {
		PSrf = TrimPrisaRuledSrf(RSrf, SamplesPerCurve,
					 Space[1], Offset, Dir);
		PSrf -> Pnext = PrisaTSrfsAll;
		PrisaTSrfsAll = PSrf;
	    }

	    TrimSrfFreeList(RuledSrfs);
	}
	else {
	    /* Return the 3D ruled surface approximation. */
	    RuledSrfs = TrimPiecewiseRuledSrfApprox(TSrf, TRUE,
						    -Epsilon, Dir);

	    PSrf = (TrimSrfStruct *) CagdListLast(RuledSrfs);
	    PSrf -> Pnext = PrisaTSrfsAll;
	    PrisaTSrfsAll = RuledSrfs;
	}
    }

    return PrisaTSrfsAll;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a piecewise ruled surface approximation to the given trimmed    M
* surface, TSrf, in the given direction, Dir, that is close to the surface   M
* to within Epsilon.							     M
*   If ConsitentDir then ruled surface parametrization is set to be the      M
* same as original surface TSrf. Otherwise, ruling dir is always	     M
* CAGD_CONST_V_DIR.							     M
*   Surface is assumed to have point types E3 or P3 only.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CTSrf:         To approximate using piecewise ruled surfaces.            M
*   ConsistentDir: Do we want parametrization to be the same as TSrf?        M
*   Epsilon:       Accuracy of piecewise ruled surface approximation.        M
*   Dir:           Direction of piecewise ruled surface approximation.       M
*		   Either U or V.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:   A list of trimmed ruled surfaces approximating TSrf   M
*                      to within Epsilon in direction Dir.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimAllPrisaSrfs, TrimPrisaRuledSrf, SymbAllPrisaSrfs	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimPiecewiseRuledSrfApprox, layout, prisa, ruled surface approximation  M
*****************************************************************************/
TrimSrfStruct *TrimPiecewiseRuledSrfApprox(const TrimSrfStruct *CTSrf,
					   CagdBType ConsistentDir,
					   CagdRType Epsilon,
					   CagdSrfDirType Dir)
{
    TrimSrfStruct *RuledSrfs, *TSrf;
    CagdSrfStruct *TmpSrf;

    if (!CAGD_IS_BSPLINE_SRF(CTSrf -> Srf)) {
	TRIM_FATAL_ERROR(TRIM_ERR_BSPLINE_EXPECT);
	return NULL;
    }

    TSrf = TrimSrfCopy(CTSrf);
    TmpSrf = TSrf -> Srf;

    if (CAGD_NUM_OF_PT_COORD(TmpSrf -> PType) != 3) {
	if (CAGD_IS_RATIONAL_PT(TmpSrf -> PType))
	    TSrf -> Srf = CagdCoerceSrfTo(TmpSrf, CAGD_PT_P3_TYPE, FALSE);
	else
	    TSrf -> Srf = CagdCoerceSrfTo(TmpSrf, CAGD_PT_E3_TYPE, FALSE);

	CagdSrfFree(TmpSrf);
    }

    RuledSrfs = TrimPiecewiseRuledSrfAux(TSrf, ConsistentDir, Epsilon, Dir);

    TrimSrfFree(TSrf);
    return RuledSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function TrimPiecewiseRuledSrfApprox		     *
*****************************************************************************/
static TrimSrfStruct *TrimPiecewiseRuledSrfAux(TrimSrfStruct *TSrf,
					       CagdBType ConsistentDir,
					       CagdRType Epsilon,
					       CagdSrfDirType Dir)
{
    CagdSrfStruct *DiffSrf, *DistSqrSrf,
	*Srf = TSrf -> Srf,
	*RuledSrf = CagdSrfCopy(Srf);
    CagdRType *XPts, *WPts, UMin, UMax, VMin, VMax,
	**Points = RuledSrf -> Points,
	MaxError = 0.0,
	t = 0.0;
    int i, j, k, Index,
	ULength = RuledSrf -> ULength,
	VLength = RuledSrf -> VLength;
    IrtPtType E3PtStart, E3PtEnd, E3Pt;

    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    for (j = 0; j < VLength; j++) {
		/* First order approximation to the ratios of the   */
		/* distance of interior point to the end points.    */
		CagdCoerceToE3(E3PtStart, Points,
			       CAGD_MESH_UV(RuledSrf, ULength >> 1, 0),
			       Srf -> PType);
		CagdCoerceToE3(E3PtEnd, Points,
			       CAGD_MESH_UV(RuledSrf, ULength >> 1,
					              VLength - 1),
			       Srf -> PType);
		CagdCoerceToE3(E3Pt, Points,
			       CAGD_MESH_UV(RuledSrf, ULength >> 1, j),
			       Srf -> PType);
		IRIT_PT_SUB(E3PtStart, E3PtStart, E3PtEnd);
		IRIT_PT_SUB(E3Pt, E3Pt, E3PtEnd);
		t = IRIT_PT_LENGTH(E3PtStart);
		t = IRIT_APX_EQ(t, 0.0) ? 0.5 : IRIT_PT_LENGTH(E3Pt) / t;

		for (i = 0; i < ULength; i++) {
		    CagdCoerceToE3(E3PtStart, Points,
				   CAGD_MESH_UV(RuledSrf, i, 0),
				   Srf -> PType);
		    CagdCoerceToE3(E3PtEnd, Points,
				   CAGD_MESH_UV(RuledSrf, i, VLength - 1),
				   Srf -> PType);
		    IRIT_PT_BLEND(E3Pt, E3PtStart, E3PtEnd, t);

		    Index = CAGD_MESH_UV(RuledSrf, i, j);
		    if (CAGD_IS_RATIONAL_PT(RuledSrf -> PType)) {
			for (k = 0; k < 3; k++)
			    Points[k + 1][Index] = 
				E3Pt[k] * Points[0][Index];
		    }
		    else {
			for (k = 0; k < 3; k++)
			    Points[k + 1][Index] = E3Pt[k];
		    }
		}
	    }
	    break;
	case CAGD_CONST_V_DIR:
	    for (i = 0; i < ULength; i++) {
		/* First order approximation to the ratios of the   */
		/* distance of interior point to the end points.    */
		CagdCoerceToE3(E3PtStart, Points,
			       CAGD_MESH_UV(RuledSrf, 0, VLength >> 1),
			       Srf -> PType);
		CagdCoerceToE3(E3PtEnd, Points,
			       CAGD_MESH_UV(RuledSrf, ULength - 1,
						      VLength >> 1),
			       Srf -> PType);
		CagdCoerceToE3(E3Pt, Points,
			       CAGD_MESH_UV(RuledSrf, i, VLength >> 1),
			       Srf -> PType);
		IRIT_PT_SUB(E3PtStart, E3PtStart, E3PtEnd);
		IRIT_PT_SUB(E3Pt, E3Pt, E3PtEnd);
		t = IRIT_PT_LENGTH(E3PtStart);
		t = IRIT_APX_EQ(t, 0.0) ? 0.5 : IRIT_PT_LENGTH(E3Pt) / t;

		for (j = 0; j < VLength; j++) {
		    CagdCoerceToE3(E3PtStart, Points,
				   CAGD_MESH_UV(RuledSrf, 0, j),
				   Srf -> PType);
		    CagdCoerceToE3(E3PtEnd, Points,
				   CAGD_MESH_UV(RuledSrf, ULength - 1, j),
				   Srf -> PType);
		    IRIT_PT_BLEND(E3Pt, E3PtStart, E3PtEnd, t);

		    Index = CAGD_MESH_UV(RuledSrf, i, j);
		    if (CAGD_IS_RATIONAL_PT(RuledSrf -> PType)) {
			for (k = 0; k < 3; k++)
			    Points[k + 1][Index] = 
				E3Pt[k] * Points[0][Index];
		    }
		    else {
			for (k = 0; k < 3; k++)
			    Points[k + 1][Index] = E3Pt[k];
		    }
		}
	    }
	    break;
	default:
	    TRIM_FATAL_ERROR(TRIM_ERR_DIR_NOT_CONST_UV);
	    break;
    }

    DiffSrf = SymbSrfSub(Srf, RuledSrf);
    CagdSrfFree(RuledSrf);
    DistSqrSrf = SymbSrfDotProd(DiffSrf, DiffSrf);
    CagdSrfFree(DiffSrf);
    XPts = DistSqrSrf -> Points[1];
    WPts = CAGD_IS_RATIONAL_PT(DistSqrSrf -> PType) ?
			    DistSqrSrf -> Points[0] : NULL;

    for (i = DistSqrSrf -> ULength * DistSqrSrf -> VLength; i > 0; i--) {
	CagdRType
	    V = WPts != NULL ? *XPts++ / *WPts++ : *XPts++;

	if (MaxError < V)
	    MaxError = V;
    }
    CagdSrfFree(DistSqrSrf);

    if (MaxError > IRIT_SQR(Epsilon)) {
	/* Subdivide and try again. */
	TrimSrfStruct *TSrf1, *TSrf2, *TRuledSrf1, *TRuledSrf2;

	TrimSrfDomain(TSrf, &UMin, &UMax, &VMin, &VMax);
	t = Dir == CAGD_CONST_V_DIR ? (UMax + UMin) * 0.5
				    : (VMax + VMin) * 0.5;
	TSrf1 = TrimSrfSubdivAtParam(TSrf, t, CAGD_OTHER_DIR(Dir));
	TSrf2 = TSrf1 -> Pnext;
	TSrf1 -> Pnext = NULL;

	TRuledSrf1 = TrimPiecewiseRuledSrfAux(TSrf1, ConsistentDir,
					      Epsilon, Dir);
	TrimSrfFree(TSrf1);

	if (TSrf2 != NULL) {
	    TRuledSrf2 = TrimPiecewiseRuledSrfAux(TSrf2, ConsistentDir,
						  Epsilon, Dir);
	    TrimSrfFree(TSrf2);
	}
	else
	    TRuledSrf2 = NULL;
	    
	return (TrimSrfStruct *) CagdListAppend(TRuledSrf1, TRuledSrf2);
    }
    else {
	/* Returns the trimmed ruled surface as a linear in ruled direction. */
	CagdCrvStruct
	    *Crv1 = CagdCrvFromMesh(Srf, 0, CAGD_OTHER_DIR(Dir)),
	    *Crv2 = CagdCrvFromMesh(Srf,
				    Dir == CAGD_CONST_V_DIR ? ULength - 1
							    : VLength - 1,
				    CAGD_OTHER_DIR(Dir));

	RuledSrf = CagdRuledSrf(Crv1, Crv2, 2, 2);
	if (ConsistentDir && Dir == CAGD_CONST_V_DIR) {
	    /* Needs to reverse the ruled surface so it matches TSrf. */
	    CagdSrfStruct
		*TmpSrf = CagdSrfReverse2(RuledSrf);

	    CagdSrfFree(RuledSrf);
	    RuledSrf = TmpSrf;
	}

	if (CAGD_IS_BSPLINE_SRF(Srf)) {
	    /* Updates the knot vector's domain. */
	    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	    BspKnotAffineTrans2(RuledSrf -> UKnotVector,
				RuledSrf -> ULength + RuledSrf -> UOrder,
				UMin, UMax);
	    BspKnotAffineTrans2(RuledSrf -> VKnotVector,
				RuledSrf -> VLength + RuledSrf -> VOrder,
				VMin, VMax);
	}
	CagdCrvFree(Crv1);
	CagdCrvFree(Crv2);

	return TrimSrfNew(RuledSrf, TrimCrvCopyList(TSrf -> TrimCrvList),
			  TRUE);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Layout a single trimmed ruled surface, by approximating it as a set of     M
* polygons.								     M
*   The given trimmed ruled surface might be non-developable, in which case  M
* approximation will be of a surface with no twist.			     M
*   The trimmed ruled surface is assumed to be constructed using	     M
* CagdRuledSrf and that the ruled direction is consistent and is always	     M
* CAGD_CONST_V_DIR.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   TSrf:            A trimmed ruled surface to layout flat on the XY plane. M
*   SamplesPerCurve: During the approximation of a ruled surface as a        M
*		     developable surface.				     M
*   Space:	     Increment on Y on the offset vector, after this         M
*		     surface was placed in the XY plane.		     M
*   Offset:	     A vector in the XY plane to denote the amount of        M
*		     translation for the flatten surface in the XY plane.    M
*   Dir:             Direction of piecewise ruled surface approximation.     M
*		     Either U or V.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrimSrfStruct *:  A planar trimmed surface in the XY plane approximating M
*                     the falttening process of TSrf.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   TrimPiecewiseRuledSrfApprox, TrimAllPrisaSrfs, SymbAllPrisaSrfs          M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimPrisaRuledSrf, layout, prisa                                         M
*****************************************************************************/
TrimSrfStruct *TrimPrisaRuledSrf(const TrimSrfStruct *TSrf,
				 int SamplesPerCurve,
				 CagdRType Space,
				 CagdVType Offset,
				 CagdSrfDirType Dir)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *Srf2D;

    if (Dir == CAGD_CONST_V_DIR) {
	/* Needs to reverse the ruled surface so it matches TSrf. */
	CagdSrfStruct
	    *TmpSrf = CagdSrfReverse2(TSrf -> Srf),
	    *TmpSrf2D = SymbPrisaRuledSrf(TmpSrf, SamplesPerCurve,
					  Space, Offset);
	CagdSrfFree(TmpSrf);
	Srf2D = CagdSrfReverse2(TmpSrf2D);
	CagdSrfFree(TmpSrf2D);
    }
    else {
	Srf2D = SymbPrisaRuledSrf(TSrf -> Srf, SamplesPerCurve,
				  Space, Offset);
    }

    CagdSrfDomain(TSrf -> Srf, &UMin, &UMax, &VMin, &VMax);

    BspKnotAffineTrans2(Srf2D -> UKnotVector, 
			Srf2D -> ULength + Srf2D -> UOrder, UMin, UMax);
    BspKnotAffineTrans2(Srf2D -> VKnotVector,
			Srf2D -> VLength + Srf2D -> VOrder, VMin, VMax);

    return TrimSrfNew(Srf2D, TrimCrvCopyList(TSrf -> TrimCrvList), TRUE);
}
