/******************************************************************************
* Blending.c - A general blending between two edges.			      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct 2001.					      *
******************************************************************************/

#include "symb_loc.h"

#define SYMB_CLIP_CRV_SRF_DOMAIN_EPS	IRIT_UEPS
#define BLEND_ON_SRF_OFFSET_TOL		0.1
#define BLEND_ON_SRF_VOFFSET_TOL	0.01

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clips the given curve to the domain prescribed by Srf.                   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     Surface to clip UVCrv to its parametric domain.                 M
*   UVCrv:   The curve to clip.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   Clipped curve.                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbClipCrvToSrfDomain                                                   M
*****************************************************************************/
CagdCrvStruct *SymbClipCrvToSrfDomain(const CagdSrfStruct *Srf,
				      const CagdCrvStruct *UVCrv)
{
    int i;
    CagdRType UMin, VMin, UMax, VMax;
    CagdCrvStruct *Crv, *Crvs, *TCrvs;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Do the splits at all four boundaries. */
    Crvs = CagdCrvCopy(UVCrv);
    for (i = 0; i < 4; i++) {
	TCrvs = NULL;
	do {
	    CagdPtStruct *Pts, *Pt;

	    Crv = Crvs;
	    Crvs = Crvs -> Pnext;
	    Crv -> Pnext = NULL;

	    switch (i) {
	        case 0:
		    Pts = SymbCrvConstSet(Crv, 1, SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  UMin + SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  TRUE);
		    break;
		case 1:
		    Pts = SymbCrvConstSet(Crv, 1, SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  UMax - SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  TRUE);
		    break;
		case 2:
		    Pts = SymbCrvConstSet(Crv, 2, SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  VMin + SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  TRUE);
		    break;
		case 3:
		    Pts = SymbCrvConstSet(Crv, 2, SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  VMax - SYMB_CLIP_CRV_SRF_DOMAIN_EPS,
					  TRUE);
		    break;
	        default:
		    Pts = NULL;
		    assert(0);
		    break;
	    }

	    for (Pt = Pts; Pt != NULL; Pt = Pt -> Pnext) {
		CagdCrvStruct *Crv1, *Crv2;

		Crv1 = CagdCrvSubdivAtParam(Crv, Pt -> Pt[0]);
		Crv2 = Crv1 -> Pnext;
		IRIT_LIST_PUSH(Crv1, TCrvs);
		CagdCrvFree(Crv);
		Crv = Crv2;
	    }		  
	    CagdPtFreeList(Pts);
	    IRIT_LIST_PUSH(Crv, TCrvs);
	}
	while (Crvs != NULL);

	Crvs = TCrvs;
    }

    /* Purge all curves outside the boundary. */
    TCrvs = NULL;
    do {
	CagdRType TMin, TMax, *R;
	CagdUVType UV;

	Crv = Crvs;
	Crvs = Crvs -> Pnext;
	Crv -> Pnext = NULL;

	CagdCrvDomain(Crv, &TMin, &TMax);

	R = CagdCrvEval(Crv, (TMin + TMax) * 0.5);
	CagdCoerceToE2(UV, &R, -1, Crv -> PType);

	if (UV[0] < UMin || UV[0] > UMax || UV[1] < VMin || UV[1] > VMax) {
	    CagdCrvFree(Crv);
	}
	else {
	    IRIT_LIST_PUSH(Crv, TCrvs)
	}
    }
    while (Crvs != NULL);

    return TCrvs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Constructs a surface, C^1 tangent to given surface and has the           M
* prescribed cross section shape.                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:     Surface to construct the blended shape on, with C^1 continuity. M
*   UVCrv:   The curve along which to blend the formed shae, in the	     M
*	     parametric domain of the surface.  Assumed to be in Srf.	     M
*   CrossSecShape:  The cross section of this blended shape.		     M
*   TanScale:       Scale factor of derived tangent fields.		     M
*   Width:   Of swept shape, in parametric space units.			     M
*   WidthField:     If not NULL, a scaling field to modulate the width of    M
*		    the constructed blended surface.			     M
*   Height:  Of swept shape, in Euclidean space units.			     M
*   HeightField:    If not NULL, a scaling field to modulate the height of   M
*		    the constructed blended surface.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   A newly form swept shape with CrossSecShape as        M
*		approximated cross section, along UVCrv.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbShapeBlendSrf                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbShapeBlendOnSrf                                                      M
*****************************************************************************/
CagdSrfStruct *SymbShapeBlendOnSrf(CagdSrfStruct *Srf,
				   CagdCrvStruct *UVCrv,
				   const CagdCrvStruct *CrossSecShape,
				   CagdRType TanScale,
				   CagdRType Width,
				   const CagdCrvStruct *WidthField,
				   CagdRType Height,
				   const CagdCrvStruct *HeightField)
{
    CagdBType
	OldMultInterp = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    CagdCrvStruct *Crv1, *Crv2, *DCrv1, *DCrv2, *TCrv, *Nrml, *Nrml1, *Nrml2,
		  *Tan1, *Tan2, *UVCrv1, *UVCrv2;
    CagdSrfStruct *NSrf, *RetSrf;

    if (WidthField == NULL) {
        UVCrv1 = SymbClipCrvToSrfDomain(Srf,
		Crv1 = SymbCrvSubdivOffset(UVCrv, Width,
					   Width * BLEND_ON_SRF_OFFSET_TOL,
					   FALSE));
	UVCrv2 = SymbClipCrvToSrfDomain(Srf,
		Crv2 = SymbCrvSubdivOffset(UVCrv, -Width,
					   Width * BLEND_ON_SRF_OFFSET_TOL,
					   FALSE));
    }
    else {
	IrtRType Min, Max, Err,
	    R = -1.0;
	CagdCrvStruct
	    *WidthField1 = CagdCrvCopy(WidthField);

	CagdCrvMinMax(WidthField, 1, &Min, &Max);
	Err = IRIT_MAX(IRIT_FABS(Min),
		       IRIT_FABS(Max)) * BLEND_ON_SRF_VOFFSET_TOL;

        UVCrv1 = SymbClipCrvToSrfDomain(Srf,
		Crv1 = SymbCrvAdapVarOffset(UVCrv, WidthField, Err,
					    NULL, FALSE));

	CagdCrvScale(WidthField1, &R);
	UVCrv2 = SymbClipCrvToSrfDomain(Srf,
		Crv2 = SymbCrvAdapVarOffset(UVCrv, WidthField1, Err,
					    NULL, FALSE));
	CagdCrvFree(WidthField1);
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    if (UVCrv1 == NULL ||
	UVCrv2 == NULL ||
	UVCrv1 -> Pnext != NULL ||
	UVCrv2 -> Pnext != NULL) {
	/* Trimmed into more than one piece - ignore this one. */
	CagdCrvFreeList(UVCrv1);
	CagdCrvFreeList(UVCrv2);
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	BspMultComputationMethod(OldMultInterp);
	return NULL;
    }
    CagdMakeCrvsCompatible(&UVCrv1, &UVCrv2, TRUE, TRUE);

    Crv1 = SymbComposeSrfCrv(Srf, UVCrv1);
    DCrv1 = CagdCrvDerive(Crv1);
    Crv2 = SymbComposeSrfCrv(Srf, UVCrv2);
    DCrv2 = CagdCrvDerive(Crv2);

    NSrf = SymbSrfNormalSrf(Srf);
    TCrv = SymbComposeSrfCrv(NSrf, UVCrv);
    Nrml = SymbCrvUnitLenCtlPts(TCrv);
    CagdCrvFree(TCrv);

    Nrml1 = SymbComposeSrfCrv(NSrf, UVCrv1);
    CagdCrvFreeList(UVCrv1);
    TCrv = SymbCrvCrossProd(DCrv1, Nrml1);
    Tan1 = SymbCrvUnitLenCtlPts(TCrv);
    CagdCrvTransform(Tan1, NULL, TanScale);
    CagdCrvFree(DCrv1);
    CagdCrvFree(Nrml1);
    CagdCrvFree(TCrv);

    Nrml2 = SymbComposeSrfCrv(NSrf, UVCrv2);
    CagdCrvFreeList(UVCrv2);
    TCrv = SymbCrvCrossProd(DCrv2, Nrml2);
    Tan2 = SymbCrvUnitLenCtlPts(TCrv);
    CagdCrvTransform(Tan2, NULL, TanScale);
    CagdCrvFree(DCrv2);
    CagdCrvFree(Nrml2);
    CagdCrvFree(TCrv);
    CagdSrfFree(NSrf);

    CagdCrvTransform(Nrml, NULL, -Height);
    if (HeightField != NULL) {
	CagdRType TMin, TMax;
	CagdCrvStruct *TCrv1, *TCrv2;

	CagdCrvDomain(Nrml, &TMin, &TMax);
	TCrv1 = CagdCrvCopy(HeightField);
	BspKnotAffineTransOrder2(TCrv1 -> KnotVector, TCrv1 -> Order,
				 TCrv1 -> Length + TCrv1 -> Order, TMin, TMax);
	TCrv2 = SymbCrvMultScalar(Nrml, TCrv1);
	CagdCrvFree(Nrml);
	CagdCrvFree(TCrv1);
	Nrml = TCrv2;
    }

    RetSrf = SymbShapeBlendSrf(Crv1, Crv2, Tan1, Tan2, CrossSecShape, Nrml);

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(Tan1);
    CagdCrvFree(Tan2);
    CagdCrvFree(Nrml);

    BspMultComputationMethod(OldMultInterp);

    return RetSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Construct a surface that interpolates Pos1Crv and Pos2Crv so that the    M
* surface is tangent to Dir1Crv and Dir2Crv there.  CrossSecShape is a       M
* blending shaping curve that must satisfy the following (CrossSecShape is   M
* C(t), t in [0, 1]):					                     M
* C(0) = (-1, 0),   C(1) = (1,0),   C'(0) = (0, 0),   C'(1) = (0, 0).        M
*                                                                            *
* PARAMETERS:                                                                M
*   Pos1Crv, Pos2Crv:  Starting and end curves of surface.                   M
*   CDir1Crv, CDir2Crv:  Starting and end tangent fields surface.            M
*   CrossSecShape:     The shape of the cross section of the blend.          M
*   Normal:            A unit vector field orthogonal to Pos1Crv - Pos2Crv.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The blended surface.  This surface will equal to -    M
*		       Let S(t) = ( Pos1Crv(t) + Pos2Crv(t) ) / 2	     M
*		       Let D(t) = ( Pos2Crv(t) - Pos1Crv(t) ) / 2	     M
*		       Then S(t, r) = H01(r) * Dir1Crv(t) +		     M
*				      H11(r) * Dir2Crv(t) + 		     M
*				      S(t) + D(t) * CrossSecShape_x(r)       M
*				      Normal(t) * CrossSecShape_y(r)	     M
*		       where H are the cubic Hermite functions for the two   M
*		       tangent fields.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdCubicHermiteSrf, SymbShapeBlendOnSrf                                 M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbShapeBlendSrf, Hermite                                               M
*****************************************************************************/
CagdSrfStruct *SymbShapeBlendSrf(const CagdCrvStruct *Pos1Crv,
				 const CagdCrvStruct *Pos2Crv,
				 const CagdCrvStruct *CDir1Crv,
				 const CagdCrvStruct *CDir2Crv,
				 const CagdCrvStruct *CrossSecShape,
				 const CagdCrvStruct *Normal)
{
    IRIT_STATIC_DATA CagdPtStruct
	PtOne = { NULL, NULL, { 1.0, 1.0, 1.0 } };
    int i, j, MaxAxis;
    CagdCrvStruct *AvgPos, *DiffPos, *CrvW, *CrvX, *CrvY, *TCrv,
	*Dir1Crv, *Dir2Crv;
    CagdSrfStruct *Srf, *TSrf1, *TSrf2;
    CagdRType **Points;

    /* Constructs the tangent field Hermite surface:			     */
    /*    H01(r) * Dir1Crv(t)  +  H11(r) * Dir2Crv(t)			     */
    Dir1Crv = CagdCrvCopy(CDir1Crv);
    Dir2Crv = CagdCrvCopy(CDir2Crv);

    if (!CagdMakeCrvsCompatible(&Dir1Crv, &Dir2Crv, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_CRV_FAIL_CMPT);
	CagdCrvFree(Dir1Crv);
	CagdCrvFree(Dir2Crv);
	return NULL;
    }

    if (CAGD_IS_BEZIER_CRV(Dir1Crv))
	Srf = BzrSrfNew(4, Dir1Crv -> Order, Dir1Crv -> PType);
    else {
	Srf = BspSrfNew(4, Dir1Crv -> Length, 4, Dir1Crv -> Order,
			Dir1Crv -> PType);
	BspKnotUniformOpen(4, 4, Srf -> UKnotVector);
	CAGD_GEN_COPY(Srf -> VKnotVector, Dir1Crv -> KnotVector,
		      sizeof(CagdRType) *
		          (Dir1Crv -> Length + Dir1Crv -> Order +
			   (Dir1Crv -> Periodic ? Dir1Crv -> Order - 1 : 0)));
    }
    Points = Srf -> Points;

    MaxAxis = CAGD_NUM_OF_PT_COORD(Srf -> PType);

    for (j = 0; j < Dir1Crv -> Length; j++) {
	int Offset = 4 * j;

	for (i = !CAGD_IS_RATIONAL_SRF(Srf); i <= MaxAxis; i++) {
	    Points[i][Offset]     = 0;
	    Points[i][Offset + 1] = Dir1Crv -> Points[i][j] / 3.0;
	    Points[i][Offset + 2] = -Dir2Crv -> Points[i][j] / 3.0;
	    Points[i][Offset + 3] = 0;
	}
    }

    CagdCrvFree(Dir1Crv);
    CagdCrvFree(Dir2Crv);

    /* Constructs S(t). */

    TCrv = CagdMergePtPt(&PtOne, &PtOne);

    AvgPos = SymbCrvAdd(Pos1Crv, Pos2Crv);
    CagdCrvTransform(AvgPos, NULL, 0.5);

    TSrf1 = SymbAlgebraicProdSrf(TCrv, AvgPos);
    CagdCrvFree(TCrv);
    CagdCrvFree(AvgPos);

    if (!CagdMakeSrfsCompatible(&Srf, &TSrf1, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	CagdSrfFree(Srf);
	CagdSrfFree(TSrf1);
	return NULL;
    }
    TSrf2 = SymbSrfAdd(Srf, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(Srf);
    Srf = TSrf2;

    /* Constructs D(t) * CrossSecShape_x(r). */

    SymbCrvSplitScalar(CrossSecShape, &CrvW, &CrvX, &CrvY, &TCrv);
    if (CrvW != NULL) {
	SYMB_FATAL_ERROR(SYMB_ERR_RATIONAL_NO_SUPPORT);
	CagdCrvFree(CrvW);
    }
    if (TCrv != NULL)
	CagdCrvFree(TCrv);

    TCrv = SymbCrvMergeScalar(NULL, CrvX, CrvX, CrvX);
    CagdCrvFree(CrvX);

    DiffPos = SymbCrvSub(Pos2Crv, Pos1Crv);
    CagdCrvTransform(DiffPos, NULL, 0.5);

    TSrf1 = SymbAlgebraicProdSrf(TCrv, DiffPos);
    CagdCrvFree(TCrv);
    CagdCrvFree(DiffPos);

    if (!CagdMakeSrfsCompatible(&Srf, &TSrf1, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	CagdSrfFree(Srf);
	CagdSrfFree(TSrf1);
	return NULL;
    }
    TSrf2 = SymbSrfAdd(Srf, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(Srf);
    Srf = TSrf2;

    /* Constructs Normat(t) * CrossSecShape_y(r). */

    TCrv = SymbCrvMergeScalar(NULL, CrvY, CrvY, CrvY);
    CagdCrvFree(CrvY);

    TSrf1 = SymbAlgebraicProdSrf(TCrv, Normal);
    CagdCrvFree(TCrv);

    if (!CagdMakeSrfsCompatible(&Srf, &TSrf1, TRUE, TRUE, TRUE, TRUE)) {
	SYMB_FATAL_ERROR(SYMB_ERR_SRF_FAIL_CMPT);
	CagdSrfFree(Srf);
	CagdSrfFree(TSrf1);
	return NULL;
    }
    TSrf2 = SymbSrfAdd(Srf, TSrf1);
    CagdSrfFree(TSrf1);
    CagdSrfFree(Srf);
    Srf = TSrf2;

    return Srf;
}
