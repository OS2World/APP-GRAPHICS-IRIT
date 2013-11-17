/******************************************************************************
* MvarDist.c - Computes ditsances between multi-variates.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, May. 97.					      *
******************************************************************************/

#include "mvar_loc.h"
#include "geom_lib.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and a point, finds the nearest point (if MinDist) or the   M
* farthest location (if MinDist FALSE) from the surface to the given point.  M
*   Returned is the parameter value of the surface. Both internal as well as M
* boundary extrema are considered.					     M
*   Computes the simultaneous zeros of:					     M
*       (Srf(u, v) - Pt) . dSrf(u, v)/Du = 0,				     V
*       (Srf(u, v) - Pt) . dSrf(u, v)/Dv = 0,				     V
* and also include all extrema on the boundaries.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        The surface to find its nearest (farthest) point to Pt.      M
*   Pt:         The point to find the nearest (farthest) point on Srf to it. M
*   MinDist:    If TRUE nearest points is needed, if FALSE farthest.         M
*   SubdivTol:  Tolerance of the first zero set finding subdivision stage.   M
*   NumericTol: Tolerance of the second zero set finding numeric stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:  UV Parameter values in the parameter space of Srf of the   M
*                 nearest (farthest) point to point Pt.                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarDistSrfPoint, surface point distance                                 M
*****************************************************************************/
CagdRType *MvarDistSrfPoint(const CagdSrfStruct *Srf,
			    const CagdPType Pt,
			    CagdBType MinDist,
			    CagdRType SubdivTol,
			    CagdRType NumericTol)
{
    IRIT_STATIC_DATA CagdUVType ExtremeDistUV;
    int i;
    CagdRType UMin, UMax, VMin, VMax,
	ExtremeDistSqr = MinDist ? IRIT_INFNTY : -IRIT_INFNTY;
    MvarPtStruct *TPt,
	*Pts = MvarLclDistSrfPoint(Srf, Pt, SubdivTol, NumericTol);
    CagdCrvStruct
	**BndryCrvs = CagdBndryCrvsFromSrf(Srf);

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Add extrema along the boundary curves. The curves' test also check   */
    /* the four corners of the surface.					    */
    for (i = 0; i < 4; i++) {
        CagdRType
	    R = SymbDistCrvPoint(BndryCrvs[i], Pt, MinDist,
				 IRIT_FABS(NumericTol));

	TPt = MvarPtNew(2);

	switch (i) {
	    case 0: /* UMin */
	        TPt -> Pt[0] = UMin;
	        TPt -> Pt[1] = R;
		break;
	    case 1: /* UMax */
	        TPt -> Pt[0] = UMax;
	        TPt -> Pt[1] = R;
		break;
	    case 2: /* VMin */
	        TPt -> Pt[0] = R;
	        TPt -> Pt[1] = VMin;
		break;
	    case 3: /* VMax */
	        TPt -> Pt[0] = R;
	        TPt -> Pt[1] = VMax;
		break;
	}

	IRIT_LIST_PUSH(TPt, Pts);
	CagdCrvFree(BndryCrvs[i]);
    }

    /* Now look over all extrema and find the right one.. */
    IRIT_PT2D_COPY(ExtremeDistUV, Pts -> Pt);
    for (TPt = Pts; TPt != NULL; TPt = TPt -> Pnext) {
	CagdPType EPt;
	CagdRType
	    DistSqr = 0.0,
	    *R = CagdSrfEval(Srf, TPt -> Pt[0], TPt -> Pt[1]);

	CagdCoerceToE3(EPt, &R, - 1, Srf -> PType);
	DistSqr = IRIT_PT_PT_DIST_SQR(EPt, Pt);

	if (MinDist) {
	    if (DistSqr < ExtremeDistSqr) {
		IRIT_PT2D_COPY(ExtremeDistUV, TPt -> Pt);
		ExtremeDistSqr = DistSqr;
	    }
	}
	else {
	    if (DistSqr > ExtremeDistSqr) {
		IRIT_PT2D_COPY(ExtremeDistUV, TPt -> Pt);
		ExtremeDistSqr = DistSqr;
	    }
	}
    }
    MvarPtFreeList(Pts);

    return ExtremeDistUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and a point, find the local extremum distance points on    M
* the surface to the given point.  Only interior extrema are considered.     M
*   Returned is a list of parameter value with local extremum.		     M
*   Computes the simultaneous zeros of:					     M
*       (Srf(u, v) - Pt) . dSrf(u, v)/Du = 0.				     V
*       (Srf(u, v) - Pt) . dSrf(u, v)/Dv = 0.				     V
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       The surface to find its extreme distance locations to Pt.    M
*   Pt:         The point to find the extreme distance locations from Srf.   M
*   SubdivTol:  Tolerance of the first zero set finding subdivision stage.   M
*   NumericTol: Tolerance of the second zero set finding numeric stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   A list of parameter values of extreme distance         M
*		      locations.                                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarLclDistSrfPoint, surface point distance                              M
*****************************************************************************/
MvarPtStruct *MvarLclDistSrfPoint(const CagdSrfStruct *CSrf,
				  const CagdPType Pt,
				  CagdRType SubdivTol,
				  CagdRType NumericTol)
{
    int i;
    CagdSrfStruct *TSrf, *Srf,
	*DuSrf = CagdSrfDerive(CSrf, CAGD_CONST_U_DIR),
	*DvSrf = CagdSrfDerive(CSrf, CAGD_CONST_V_DIR);
    CagdPType MinusPt;
    MvarPtStruct *MVPts;
    MvarMVStruct *MVs[2];
    MvarConstraintType Constraints[2];

    for (i = 0; i < 3; i++)
	MinusPt[i] = -Pt[i];

    Srf = CagdSrfCopy(CSrf);
    CagdSrfTransform(Srf, MinusPt, 1.0);

    TSrf = SymbSrfDotProd(Srf, DuSrf);
    CagdSrfFree(DuSrf);
    MVs[0] = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);

    TSrf = SymbSrfDotProd(Srf, DvSrf);
    CagdSrfFree(DvSrf);
    MVs[1] = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);

    CagdSrfFree(Srf);

    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, 2, SubdivTol, NumericTol);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and a line, finds the nearest point (if MinDist) or the    M
* farthest location (if MinDist FALSE) from the surface to the given line.   M
*   This function assumes the surface does not intersect the line.	     M
*   Returned is the parameter value of the surface. Only internal extrema    M
* are considered.							     M
*   Let S and N be the surface and its normal field.  Then the extrema       M
*  points are computed as the simultaneous solution of,			     M
*   < (S - LnPt) x N, LnDir > = 0,					     V
*   < N, LnDir > = 0.							     V
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:        The surface to find its nearest (farthest) point to Line.    M
*   LnPt:       A point on the line to consider.			     M
*   LnDir:      The direction of the line to consider.			     M
*   MinDist:    If TRUE nearest points is needed, if FALSE farthest.         M
*   SubdivTol:  Tolerance of the first zero set finding subdivision stage.   M
*   NumericTol: Tolerance of the second zero set finding numeric stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Parameter value in the parameter space of Srf of the      M
*                  nearest (farthest) point to line Line.                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarLclDistSrfLine, MvarLclDistCrvLine, MvarDistCrvLine		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarDistSrfLine, surface line distance                                   M
*****************************************************************************/
CagdRType *MvarDistSrfLine(const CagdSrfStruct *Srf,
			   const CagdPType LnPt,
			   const CagdVType LnDir,
			   CagdBType MinDist,
			   CagdRType SubdivTol,
			   CagdRType NumericTol)
{
    static CagdUVType ExtremeDistUV;
    CagdRType
	ExtremeDist = MinDist ? IRIT_INFNTY : -IRIT_INFNTY;
    MvarPtStruct *TPt,
	*Pts = MvarLclDistSrfLine(Srf, LnPt, LnDir, SubdivTol, NumericTol);

    if (Pts == NULL)
	return NULL;

    /* Now look over all extrema and find the right one.. */
    IRIT_PT2D_COPY(ExtremeDistUV, Pts -> Pt);
    for (TPt = Pts; TPt != NULL; TPt = TPt -> Pnext) {
	CagdPType EPt;
	CagdRType
	    Dist = 0.0,
	    *R = CagdSrfEval(Srf, TPt -> Pt[0], TPt -> Pt[1]);

	CagdCoerceToE3(EPt, &R, - 1, Srf -> PType);
	Dist = GMDistPointLine(EPt, LnPt, LnDir);

	if (MinDist) {
	    if (Dist < ExtremeDist) {
		IRIT_PT2D_COPY(ExtremeDistUV, TPt -> Pt);
		ExtremeDist = Dist;
	    }
	}
	else {
	    if (Dist > ExtremeDist) {
		IRIT_PT2D_COPY(ExtremeDistUV, TPt -> Pt);
		ExtremeDist = Dist;
	    }
	}
    }
    MvarPtFreeList(Pts);

    return ExtremeDistUV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a surface and a line, finds the nearest point (if MinDist) or the    M
* farthest location (if MinDist FALSE) from the surface to the given line.   M
*   This function assumes the surface does not intersect the line.	     M
*   Returned is the parameter value of the surface. Only internal extrema    M
* are considered.							     M
*   Let S and N be the surface and its normal field.  Then the extrema       M
*  points are computed as the simultaneous solution of,			     M
*   < (S - LnPt) x N, LnDir > = 0,					     V
*   < N, LnDir > = 0.							     V
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:       The surface to find its nearest (farthest) point to Line.    M
*   LnPt:       A point on the line to consider.			     M
*   LnDir:      The direction of the line to consider.			     M
*   SubdivTol:  Tolerance of the first zero set finding subdivision stage.   M
*   NumericTol: Tolerance of the second zero set finding numeric stage.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   A list of parameter values of extreme distance         M
*		      locations.                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarDistSrfLine, MvarDistCrvLine					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarLclDistSrfLine, surface line distance                                M
*****************************************************************************/
MvarPtStruct *MvarLclDistSrfLine(const CagdSrfStruct *CSrf,
				 const CagdPType LnPt,
				 const CagdVType LnDir,
				 CagdRType SubdivTol,
				 CagdRType NumericTol)
{
    int i;
    CagdPType MinusPt;
    CagdSrfStruct *Srf, *TSrf, *TSrf2,
	*NSrf= SymbSrfNormalSrf(CSrf);
    MvarPtStruct *MVPts;
    MvarMVStruct *MVs[2];
    MvarConstraintType Constraints[2];

    for (i = 0; i < 3; i++)
	MinusPt[i] = -LnPt[i];

    Srf = CagdSrfCopy(CSrf);
    CagdSrfTransform(Srf, MinusPt, 1.0);
    TSrf2 = SymbSrfCrossProd(Srf, NSrf);
    CagdSrfFree(Srf);
    TSrf = SymbSrfVecDotProd(TSrf2, LnDir);
    CagdSrfFree(TSrf2);
    MVs[0] = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);

    TSrf = SymbSrfVecDotProd(NSrf, LnDir);
    CagdSrfFree(NSrf);
    MVs[1] = MvarSrfToMV(TSrf);
    CagdSrfFree(TSrf);

    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVs, Constraints, 2, SubdivTol, NumericTol);
    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    return MVPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a curve and a surface, creates a multivariate scalar field         M
* representing the distance function square, between them.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Srf2:  The two entities, Crv1(t) and Srf2(u, v), to form their     M
*                distance function square between them as a multivariate     M
*                function.						     M
*   DistType:    0 for distance vector function,			     M
*		 1 for distance square scalar function,			     M
*		 2 for distance vector projected on the normal of Crv1,	     M
*		 3 for distance vector projected on the normal of Srf2.	     M
*		 In cases 2 and 3 the normal field is not normalized.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  The distance function square d2(t, u, v) of the         M
*                    distance from Crv1(t) to Srf2(u, v).                    M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDistCrvCrv, SymbCrvCrvInter, SymbSrfDistFindPoints,		     M
*   MvarMVDistSrfSrf							     m
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDistCrvSrf, curve surface distance                                 M
*****************************************************************************/
MvarMVStruct *MvarMVDistCrvSrf(const CagdCrvStruct *Crv1,
			       const CagdSrfStruct *Srf2,
			       int DistType)
{
    MvarMVStruct *TMV, *DiffMV, *RetMV, *MVNrml,
	*MV1 = MvarCrvToMV(Crv1),
	*MV2 = MvarSrfToMV(Srf2);

    TMV = MvarPromoteMVToMV2(MV1, 3, 0);
    MvarMVFree(MV1);
    MV1 = TMV;

    TMV = MvarPromoteMVToMV2(MV2, 3, 1);
    MvarMVFree(MV2);
    MV2 = TMV;

    DiffMV = MvarMVSub(MV1, MV2);

    switch (DistType) {
	case 0:
	    RetMV = DiffMV;
    	    break;
	case 1:
	default:
	    RetMV = MvarMVDotProd(DiffMV, DiffMV);
	    MvarMVFree(DiffMV);
	    break;
	case 2:
	case 3:
	    if (DistType == 2) {
		CagdCrvStruct
		    *CNrml = SymbCrv3DRadiusNormal(Crv1);

		TMV = MvarCrvToMV(CNrml);
		CagdCrvFree(CNrml);
		MVNrml = MvarPromoteMVToMV2(TMV, 3, 0);
		MvarMVFree(TMV);
	    }
	    else {
		CagdSrfStruct
		    *SNrml = SymbSrfNormalSrf(Srf2);

		TMV = MvarSrfToMV(SNrml);
		CagdSrfFree(SNrml);
		MVNrml = MvarPromoteMVToMV2(TMV, 3, 1);
		MvarMVFree(TMV);
	    }

	    RetMV = MvarMVDotProd(DiffMV, MVNrml);
	    MvarMVFree(DiffMV);
	    MvarMVFree(MVNrml);
	    break;
    }
	
    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return RetMV;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given two surfaces, creates a multivariate scalar field representing the M
* distance function square, between them.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:  The two surfaces, Srf1(u, v) and Srf2(r, t), to form their  M
*                distance function square between them as a multivariate     M
*		 function.						     M
*   DistType:    0 for distance vector function,			     M
*		 1 for distance square scalar function,			     M
*		 2 for distance vector projected on the normal of Srf1,	     M
*		 3 for distance vector projected on the normal of Srf2.	     M
*		 In cases 2 and 3 the normal field is not normalized.        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarMVStruct *:  The distance function square d2(u, v, r, t) of the      M
*                    distance from Srf1(u, v) to Srf2(r, t).                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfDistCrvCrv, SymbCrvCrvInter, SymbSrfDistFindPoints,		     M
*   MvarMVDistCrvSrf							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVDistSrfSrf, surface surface distance                               M
*****************************************************************************/
MvarMVStruct *MvarMVDistSrfSrf(const CagdSrfStruct *Srf1,
			       const CagdSrfStruct *Srf2,
			       int DistType)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *SNrml;
    MvarMVStruct *TMV, *DiffMV, *RetMV, *MVNrml,
	*MV1 = MvarSrfToMV(Srf1),
	*MV2 = MvarSrfToMV(Srf2);

    /* Convert Srf1 to a 4D MV where Srf1 spans the first 2 dims. */
    TMV = MvarPromoteMVToMV2(MV1, 4, 0);
    MvarMVFree(MV1);
    MV1 = TMV;

    if (CAGD_IS_BSPLINE_SRF(Srf1) && CAGD_IS_BSPLINE_SRF(Srf2)) {
        CagdSrfDomain(Srf2, &UMin, &UMax, &VMin, &VMax);
	BspKnotAffineTrans2(MV1 -> KnotVectors[2],
			    MV1 -> Lengths[2] + MV1 -> Orders[2], UMin, UMax);
	BspKnotAffineTrans2(MV1 -> KnotVectors[3],
			    MV1 -> Lengths[3] + MV1 -> Orders[3], VMin, VMax);
    }

    /* Convert Srf2 to a 4D MV where Srf2 spans the last 2 dims. */
    TMV = MvarPromoteMVToMV2(MV2, 4, 2);
    MvarMVFree(MV2);
    MV2 = TMV;

    if (CAGD_IS_BSPLINE_SRF(Srf1) && CAGD_IS_BSPLINE_SRF(Srf2)) {
        CagdSrfDomain(Srf1, &UMin, &UMax, &VMin, &VMax);
	BspKnotAffineTrans2(MV2 -> KnotVectors[0],
			    MV2 -> Lengths[0] + MV2 -> Orders[0], UMin, UMax);
	BspKnotAffineTrans2(MV2 -> KnotVectors[1],
			    MV2 -> Lengths[1] + MV2 -> Orders[1], VMin, VMax);
    }

    DiffMV = MvarMVSub(MV1, MV2);

    switch (DistType) {
	case 0:
	    RetMV = DiffMV;
    	    break;
	case 1:
	default:
	    RetMV = MvarMVDotProd(DiffMV, DiffMV);
	    MvarMVFree(DiffMV);
	    break;
	case 2:
	case 3:
	    if (DistType == 2)
		SNrml = SymbSrfNormalSrf(Srf1);
	    else
		SNrml = SymbSrfNormalSrf(Srf2);

	    TMV = MvarSrfToMV(SNrml);
	    CagdSrfFree(SNrml);
	    MVNrml = MvarPromoteMVToMV2(TMV, 4,
					DistType == 2 ? 0 : 2);
	    MvarMVFree(TMV);

	    RetMV = MvarMVDotProd(DiffMV, MVNrml);
	    MvarMVFree(DiffMV);
	    MvarMVFree(MVNrml);
	    break;
    }
	
    MvarMVFree(MV1);
    MvarMVFree(MV2);

    return RetMV;
}
