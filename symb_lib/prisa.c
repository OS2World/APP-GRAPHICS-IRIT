/******************************************************************************
* Prisa.c - piecewise ruled srf approximation and layout (prisa).	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Apr. 93.					      *
******************************************************************************/

#include "symb_loc.h"
#include "geom_lib.h"

static CagdSrfStruct *SymbPiecewiseRuledSrfAux(const CagdSrfStruct *Srf,
					       CagdBType ConsistentDir,
					       CagdRType Epsilon,
					       CagdSrfDirType Dir);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes a piecewise ruled surface approximation to a given set of         M
* surfaces with given Epsilon, and lay them out "nicely" onto the XY plane,  M
* by approximating each ruled surface as a developable surface with          M
* SamplesPerCurve samples.						     M
*   Dir controls the direction of ruled approximation, SpaceScale and        M
* Offset controls the placement of the different planar pieces.		     M
*   Prisa is the hebrew word for the process of flattening out a three       M
* dimensional surface. I have still to find an english word for it.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srfs:            To approximate and flatten out.                         M
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
*   CagdSrfStruct *:   A list of planar surfaces denoting the layout (prisa) M
*		       of the given Srfs to the accuracy requested.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPiecewiseRuledSrfApprox, SymbPrisaRuledSrf, TrimAllPrisaSrfs,        M
*   SymbPrisaGetCrossSections						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbAllPrisaSrfs, layout, prisa                                          M
*****************************************************************************/
CagdSrfStruct *SymbAllPrisaSrfs(const CagdSrfStruct *Srfs,
				int SamplesPerCurve,
			        CagdRType Epsilon,
				CagdSrfDirType Dir,
			        const CagdVType Space)
{
    int SrfIndex = 1;
    const CagdSrfStruct *Srf;
    CagdSrfStruct
	*PrisaSrfsAll = NULL;
    CagdVType Offset;

    for (Srf = Srfs; Srf != NULL; Srf = Srf -> Pnext, SrfIndex++) {
	CagdSrfStruct *TSrf, *RSrf, *RuledSrfs;

	if (Epsilon > 0) {
	    RuledSrfs = SymbPiecewiseRuledSrfApprox(Srf, FALSE, Epsilon, Dir);

	    Offset[0] = SrfIndex * Space[0];
	    Offset[1] = 0.0;
	    Offset[2] = 0.0;

	    for (RSrf = RuledSrfs; RSrf != NULL; RSrf = RSrf -> Pnext) {
		TSrf = SymbPrisaRuledSrf(RSrf, SamplesPerCurve,
					 Space[1], Offset);
		TSrf -> Pnext = PrisaSrfsAll;
		PrisaSrfsAll = TSrf;
	    }

	    CagdSrfFreeList(RuledSrfs);
	}
	else {
	    /* Return the 3D ruled surface approximation. */
	    RuledSrfs = SymbPiecewiseRuledSrfApprox(Srf, FALSE, -Epsilon, Dir);

	    TSrf = (CagdSrfStruct *) CagdListLast(RuledSrfs);
	    TSrf -> Pnext = PrisaSrfsAll;
	    PrisaSrfsAll = RuledSrfs;
	}
    }

    return PrisaSrfsAll;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Constructs a piecewise ruled surface approximation to the given surface,   M
* Srf, in the given direction, Dir, that is close to the surface to within   M
* Epsilon.								     M
*   If ConsitentDir then ruled surface parametrization is set to be the      M
* same as original surface Srf. Otherwise, ruling dir is always		     M
* CAGD_CONST_V_DIR.							     M
*   Surface is assumed to have point types E3 or P3 only.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:          To approximate using piecewise ruled surfaces.            M
*   ConsistentDir: Do we want parametrization to be the same as Srf?         M
*   Epsilon:       Accuracy of piecewise ruled surface approximation.        M
*   Dir:           Direction of piecewise ruled surface approximation.       M
*		   Either U or V.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A list of ruled surfaces approximating Srf to within  M
*                      Epsilon in direction Dir.                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbAllPrisaSrfs, SymbPrisaRuledSrf, TrimAllPrisaSrfs	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPiecewiseRuledSrfApprox, layout, prisa, ruled surface approximation  M
*****************************************************************************/
CagdSrfStruct *SymbPiecewiseRuledSrfApprox(const CagdSrfStruct *CSrf,
					   CagdBType ConsistentDir,
					   CagdRType Epsilon,
					   CagdSrfDirType Dir)
{
    CagdSrfStruct *RuledSrfs, *Srf;

    if (CAGD_NUM_OF_PT_COORD(CSrf -> PType) != 3) {
	if (CAGD_IS_RATIONAL_PT(CSrf -> PType))
	    Srf = CagdCoerceSrfTo(CSrf, CAGD_PT_P3_TYPE, FALSE);
	else
	    Srf = CagdCoerceSrfTo(CSrf, CAGD_PT_E3_TYPE, FALSE);
    }
    else
	Srf = CagdSrfCopy(CSrf);

    RuledSrfs = SymbPiecewiseRuledSrfAux(Srf, ConsistentDir, Epsilon, Dir);

    CagdSrfFree(Srf);
    return RuledSrfs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Auxiliary function to function SymbPiecewiseRuledSrfApprox		     *
*****************************************************************************/
static CagdSrfStruct *SymbPiecewiseRuledSrfAux(const CagdSrfStruct *Srf,
					       CagdBType ConsistentDir,
					       CagdRType Epsilon,
					       CagdSrfDirType Dir)
{
    CagdSrfStruct *DiffSrf, *DistSqrSrf,
	*RuledSrf = CagdSrfCopy(Srf);
    CagdRType *XPts, *WPts, UMin, UMax, VMin, VMax,
	**Points = RuledSrf -> Points,
	MaxError = 0.0,
	t = 0.0;
    int i, j, k, Index,
	ULength = RuledSrf -> ULength,
	VLength = RuledSrf -> VLength;
    CagdPType E3PtStart, E3PtEnd, E3Pt;

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
	    SYMB_FATAL_ERROR(SYMB_ERR_DIR_NOT_CONST_UV);
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
	CagdSrfStruct *Srf1, *Srf2, *RuledSrf1, *RuledSrf2;

	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
	t = Dir == CAGD_CONST_V_DIR ? (UMax + UMin) * 0.5
				    : (VMax + VMin) * 0.5;
	Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_OTHER_DIR(Dir));
	Srf2 = Srf1 -> Pnext;
	Srf1 -> Pnext = NULL;

	RuledSrf1 = SymbPiecewiseRuledSrfAux(Srf1, ConsistentDir, Epsilon, Dir);
	RuledSrf2 = SymbPiecewiseRuledSrfAux(Srf2, ConsistentDir, Epsilon, Dir);
	CagdSrfFree(Srf1);
	CagdSrfFree(Srf2);

	return (CagdSrfStruct *) CagdListAppend(RuledSrf1, RuledSrf2);
    }
    else {
	/* Returns the ruled surface as a linear in the ruled direction. */
	CagdCrvStruct
	    *Crv1 = CagdCrvFromMesh(Srf, 0, CAGD_OTHER_DIR(Dir)),
	    *Crv2 = CagdCrvFromMesh(Srf,
				    Dir == CAGD_CONST_V_DIR ? ULength - 1
							    : VLength - 1,
				    CAGD_OTHER_DIR(Dir));

	RuledSrf = CagdRuledSrf(Crv1, Crv2, 2, 2);
	if (ConsistentDir && Dir == CAGD_CONST_V_DIR) {
	    /* Needs to reverse the ruled surface so it matches Srf. */
	    CagdSrfStruct
		*TSrf = CagdSrfReverse2(RuledSrf);

	    CagdSrfFree(RuledSrf);
	    RuledSrf = TSrf;
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
	return RuledSrf;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Layout a single ruled surface, by approximating it as a set of polygons.   M
*   The given ruled surface might be non-developable, in which case	     M
* approximation will be of a surface with no twist.			     M
*   The ruled surface is assumed to be constructed using CagdRuledSrf and    M
* that the ruled direction is consistent and is always CAGD_CONST_V_DIR.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             A ruled surface to layout flat on the XY plane.         M
*   SamplesPerCurve: During the approximation of a ruled surface as a        M
*		     developable surface.				     M
*   Space:	     Increment on Y on the offset vector, after this         M
*		     surface was placed in the XY plane.		     M
*   Offset:	     A vector in the XY plane to denote the amount of        M
*		     translation for the flatten surface in the XY plane.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:  A planar surface in the XY plane approximating the     M
*                     falttening process of Srf.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPiecewiseRuledSrfApprox, SymbAllPrisaSrfs, TrimAllPrisaSrfs          M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPrisaRuledSrf, layout, prisa                                         M
*****************************************************************************/
CagdSrfStruct *SymbPrisaRuledSrf(const CagdSrfStruct *Srf,
				 int SamplesPerCurve,
				 CagdRType Space,
				 CagdVType Offset)
{
    int i, j,
	VLength = Srf -> VLength;
    CagdRType Angle, PtTmp1[3], PtTmp2[3], PtTmp3[3], UMin, UMax, VMin, VMax;
    CagdCrvStruct
	*Crv1 = CagdCrvFromMesh(Srf, 0, CAGD_CONST_V_DIR),
	*Crv2 = CagdCrvFromMesh(Srf, VLength - 1, CAGD_CONST_V_DIR);
    CagdSrfStruct *TSrf, *TSrf2;
    CagdPolylineStruct
	*Poly1 = SymbCrv2Polyline(Crv1, SamplesPerCurve, 
				  SYMB_CRV_APPROX_UNIFORM, TRUE),
	*Poly2 = SymbCrv2Polyline(Crv2, SamplesPerCurve,
				  SYMB_CRV_APPROX_UNIFORM, TRUE),
	*Poly1Prisa = CagdPolylineNew(Poly1 -> Length),
	*Poly2Prisa = CagdPolylineNew(Poly2 -> Length);
    CagdPolylnStruct
        *Pt1 = Poly1 -> Polyline,
        *Pt2 = Poly2 -> Polyline,
        *Pt1Prisa = Poly1Prisa -> Polyline,
        *Pt2Prisa = Poly2Prisa -> Polyline;
    CagdMType Mat1, Mat;
    CagdBBoxStruct BBox;

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Anchor the location of the first line. */
    for (j = 0; j < 3; j++) {
	Pt1Prisa -> Pt[j] = 0.0;
	Pt2Prisa -> Pt[j] = 0.0;
    }
    IRIT_PT_SUB(PtTmp1, Pt1 -> Pt, Pt2 -> Pt);
    Pt2Prisa -> Pt[1] = IRIT_PT_LENGTH(PtTmp1);

    /* Move alternately along Poly1 and Poly2 and anchor the next point of */
    /* the next triangle using the distances to current Pt1 and Pt2.	   */
    for (i = 2; i < Poly1 -> Length + Poly2 -> Length; i++) {
	CagdRType Dist1, Dist2, Inter1[3], Inter2[3],
	    *NextPt = i & 0x01 ? Pt1[1].Pt : Pt2[1].Pt;

	/* Compute distance from previous two locations to new location. */ 
	IRIT_PT_SUB(PtTmp1, Pt1 -> Pt, NextPt);
	Dist1 = IRIT_PT_LENGTH(PtTmp1);
	IRIT_PT_SUB(PtTmp1, Pt2 -> Pt, NextPt);
	Dist2 = IRIT_PT_LENGTH(PtTmp1);

	/* Find the (two) intersection points of circles with radii Dist?. */
	GM2PointsFromCircCirc(Pt1Prisa -> Pt, Dist1, Pt2Prisa -> Pt, Dist2,
			      Inter1, Inter2);

	/* Find which of the two intersection points is the "good" one. */
	IRIT_PT_SUB(PtTmp1, Inter1, Pt1Prisa -> Pt);
	IRIT_PT_SUB(PtTmp2, Inter1, Pt2Prisa -> Pt);
	IRIT_CROSS_PROD(PtTmp3, PtTmp1, PtTmp2);
	if (i & 0x01) {
	    Pt1Prisa++;
	    for (j = 0; j < 3; j++)
		Pt1Prisa -> Pt[j] = (PtTmp3[2] > 0 ? Inter2[j] : Inter1[j]);
	    Pt1++;
	}
	else {
	    Pt2Prisa++;
	    for (j = 0; j < 3; j++)
		Pt2Prisa -> Pt[j] = (PtTmp3[2] > 0 ? Inter2[j] : Inter1[j]);
	    Pt2++;
	}
    }

    /* Save centering location so we can orient the resulting data nicely. */
    IRIT_PT_COPY(PtTmp1, Poly1Prisa -> Polyline[Poly1Prisa -> Length >> 1].Pt);
    IRIT_PT_COPY(PtTmp2, Poly2Prisa -> Polyline[Poly2Prisa -> Length >> 1].Pt);
    IRIT_PT_SUB(PtTmp3, PtTmp2, PtTmp1);

    Crv1 = CagdCnvrtPolyline2LinBspCrv(Poly1Prisa);
    Crv2 = CagdCnvrtPolyline2LinBspCrv(Poly2Prisa);
    CagdPolylineFree(Poly1);
    CagdPolylineFree(Poly2);
    CagdPolylineFree(Poly1Prisa);
    CagdPolylineFree(Poly2Prisa);

    TSrf2 = CagdRuledSrf(Crv1, Crv2, 2, 2);
    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    /* Translate PtTmp1 to the origin. */
    MatGenMatTrans(-PtTmp1[0], -PtTmp1[1], -PtTmp1[2], Mat);

    /* Rotate PtTmp3 direction to the +Y direction. */
    Angle = atan2(PtTmp3[1], PtTmp3[0]);
    MatGenMatRotZ1(M_PI_DIV_2 - Angle, Mat1);

    MatMultTwo4by4(Mat, Mat, Mat1);

    TSrf = CagdSrfMatTransform(TSrf2, Mat);
    CagdSrfFree(TSrf2);

    /* Translate by the Offset. */
    CagdSrfBBox(TSrf, &BBox);
    MatGenMatTrans(Offset[0], Offset[1] - BBox.Min[1], Offset[2], Mat);
    Offset[1] += (BBox.Max[1] - BBox.Min[1]) + Space;

    TSrf2 = CagdSrfMatTransform(TSrf, Mat);
    CagdSrfFree(TSrf);

    /* Update the parametric domain of the surface. */
    BspKnotAffineTrans2(TSrf2 -> UKnotVector, 
			TSrf2 -> ULength + TSrf2 -> UOrder, UMin, UMax);
    BspKnotAffineTrans2(TSrf2 -> VKnotVector,
			TSrf2 -> VLength + TSrf2 -> VOrder, VMin, VMax);

    return TSrf2;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a list of n ruled surface in 3-space, extract their cross sections M
* and return a list of n+1 cross sections.  The given ruled surfaces are     M
* assumed to be a layout decomposition of a freeform surface using the       M
* function SymbPiecewiseRuledSrfApprox.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   RSrfs:    A list of ruled surfaces to extract cross sections from.       M
*   Dir:      Ruling direction of ruled/developable surface approximation.   M
*	      Typically CAGD_CONST_U_DIR.				     M
*   Space:    Increment on Y on the offset vector, after this		     M
*	      cross section was placed in the XY plane.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  A list of cross sections.  The cross sections will     M
*		      be in the XY plane the cross section is indeed planar  M
*		      or approximately in the XY plane otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPrisaRuledSrf, SymbPiecewiseRuledSrfApprox, SymbAllPrisaSrfs,	     M
*   SymbPrisaGetOneCrossSection						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPrisaGetCrossSections                                                M
*****************************************************************************/
CagdCrvStruct *SymbPrisaGetCrossSections(const CagdSrfStruct *RSrfs,
					 CagdSrfDirType Dir,
					 const CagdVType Space)
{
    CagdRType
	Base = 0.0;
    CagdVType Offset;
    CagdCrvStruct *Crv, *TCrv, *TCrv1, *TCrv2,
	*CTail = NULL,
	*TransCrossSections = NULL,
	*CrossSections = NULL;
    const CagdSrfStruct *RSrf;

    for (RSrf = RSrfs; RSrf != NULL; RSrf = RSrf -> Pnext) {
	if (RSrf == RSrfs) {
	    CrossSections = SymbPrisaGetOneCrossSection(RSrf, Dir, TRUE, TRUE);
	    CTail = CrossSections -> Pnext;
	}
	else {
	    CTail -> Pnext = SymbPrisaGetOneCrossSection(RSrf, Dir,
							 FALSE, TRUE);
	    CTail = CTail -> Pnext;
	}
    }

    /* Rotate all cross sections to XY plane (approximately) and place them. */
    IRIT_PT_RESET(Offset);
    for (Crv = CrossSections; Crv != NULL; Crv = Crv -> Pnext) {
	CagdBBoxStruct BBox;
	IrtHmgnMatType Mat;

	if ((TCrv = CagdCrvRotateToXY(Crv)) == NULL)
	    TCrv = CagdCrvCopy(Crv);
	
	/* Translate by the Offset. */
	CagdCrvBBox(TCrv, &BBox);

	Offset[0] = -(BBox.Min[0] + BBox.Max[0]) * 0.5;
	Offset[1] = Base - BBox.Min[1];
	MatGenMatTrans(Offset[0], Offset[1], 0.0, Mat);
	Base += (BBox.Max[1] - BBox.Min[1]) + Space[1];

	TCrv1 = CagdCrvMatTransform(TCrv, Mat);
	CagdCrvFree(TCrv);
	MatGenMatScale(1.0, 1.0, 0.0, Mat);             /* Purge the Z axis. */
	TCrv2 = CagdCrvMatTransform(TCrv1, Mat);
	CagdCrvFree(TCrv1);
 	IRIT_LIST_PUSH(TCrv2, TransCrossSections);
    }

    CagdCrvFreeList(CrossSections);

    return CagdListReverse(TransCrossSections);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a ruled surface in 3-space, extract its starting/ending cross	     M
* sections and return a list of one or two cross sections.  The given ruled  M
* surface is assumed to be a layout decomposition of a freeform surface      M
* using the function SymbPiecewiseRuledSrfApprox.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   RSrf:       A ruled surface to extract cross section(s) from.	     M
*   Dir:      Ruling direction of ruled/developable surface approximation.   M
*	      Typically CAGD_CONST_U_DIR.				     M
*   Starting:   If TRUE, extracts the first cross section.		     M
*   Ending:	If TRUE, extracts the last cross section.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: A list of one or two cross sections. The cross sections M
*		     will be in the XY plane the cross section is indeed     M
*		     planar or approximately in the XY plane otherwise.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbPrisaRuledSrf, SymbPiecewiseRuledSrfApprox, SymbAllPrisaSrfs,	     M
*   SymbPrisaGetCrossSections						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbPrisaGetOneCrossSection                                              M
*****************************************************************************/
CagdCrvStruct *SymbPrisaGetOneCrossSection(const CagdSrfStruct *RSrf,
					   CagdSrfDirType Dir,
					   CagdBType Starting,
					   CagdBType Ending)
{
    CagdCrvStruct
	*StartCrv = Starting ? CagdCrvFromMesh(RSrf, 0, CAGD_OTHER_DIR(Dir))
			     : NULL,
	*EndCrv = Ending ? CagdCrvFromMesh(RSrf, 1, CAGD_OTHER_DIR(Dir))
			 : NULL;

    if (StartCrv != NULL && EndCrv != NULL) {
	StartCrv -> Pnext = EndCrv;
	return StartCrv;
    }
    else if (StartCrv != NULL) {
	return StartCrv;
    }
    else if (EndCrv != NULL) {
	return EndCrv;
    }
    else
	return NULL;
}
