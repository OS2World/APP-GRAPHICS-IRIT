/******************************************************************************
* RvrsEng.c - Reverse engineer freeform curves and surfaces.   		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* The reverse engineering of curves and surfaces is heavily based on the      *
* paper:								      *
*       Gershon Elber and Myung-Soo Kim.				      *
*	``Geometric Shape Recognition of Freeform Curves and Surfaces.''      *
*	Graphics Models and Image Processing, Vol 59, No 6, pp 417-433,       *
*	November 1997.							      *
* Written by Gershon Elber, Nov. 98.					      *
******************************************************************************/

#include "symb_loc.h"
#include "iritprsr.h"
#include "geom_lib.h"
#include "allocate.h"

#define VEC_2D_ROTATE(V)  { V[2] = V[0]; V[0] = -V[1]; V[1] = V[2]; V[2] = 0; }

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recognize if the given curve Crv is a constant curve.        M
* If TRUE, ConstVal is reference to a static location holding the constant   M
* value.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   CCrv:      Curve to attempt and recognize as a constant curve.           M
*   ConstVal:  Resulting constant value if indeed a constant curve.  This    M
*	       value is always Euclidean, even for projective curves.	     M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if a constant curve, FALSE otherwise.                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsConstSrf                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsConstCrv                                                           M
*****************************************************************************/
CagdBType SymbIsConstCrv(const CagdCrvStruct *CCrv,
			 CagdCtlPtStruct **ConstVal,
			 CagdRType Eps)
{
    IRIT_STATIC_DATA CagdCtlPtStruct LclConstVal;
    int i, j,
	Length = CCrv -> Length,
	NumCoords = CAGD_NUM_OF_PT_COORD(CCrv -> PType);
    CagdRType **Points;
    CagdCrvStruct *Crv;

    *ConstVal = NULL;

    /* Make sure the curve is Euclidean. */
    Crv = CagdCoerceCrvTo(CCrv,
		CAGD_MAKE_PT_TYPE(FALSE, CAGD_NUM_OF_PT_COORD(CCrv -> PType)),
		FALSE);
    LclConstVal.PtType = Crv -> PType;
    IRIT_ZAP_MEM(LclConstVal.Coords, sizeof(CagdRType) * CAGD_MAX_PT_SIZE);
    LclConstVal.Coords[0] = 1.0;
    Points = Crv -> Points;

    for (i = 1; i <= NumCoords; i++) {
	CagdRType
	    *R = Points[i],
	    R0 = *R++;

	LclConstVal.Coords[i] = R0;
	for (j = 1; j < Length; j++, R++) {
	    if (!IRIT_APX_EQ_EPS(R0, *R, Eps)) {
	        CagdCrvFree(Crv);
		return FALSE;
	    }
	    LclConstVal.Coords[i] += *R;
	}
	LclConstVal.Coords[i] /= Length;
    }

    *ConstVal = &LclConstVal;
    CagdCrvFree(Crv);

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recognize if the given curve Crv is an identically zero      M
* curve.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Curve to attempt and recognize as a zero curve.               M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if a zero curve, FALSE otherwise.                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsConstCrv, SymbIsZeroSrf                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsZeroCrv                                                            M
*****************************************************************************/
CagdBType SymbIsZeroCrv(const CagdCrvStruct *Crv, CagdRType Eps)
{
    int i,
	NumCoords = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCtlPtStruct *ConstVal;

    if (!SymbIsConstCrv(Crv, &ConstVal, Eps))
	return FALSE;

    for (i = 1; i <= NumCoords; i++) {
	if (!IRIT_APX_EQ_EPS(ConstVal -> Coords[i], 0.0, Eps))
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given curve Crv is indeed circular.	     M
*   If the curve is found to be circular, its center and radius are          M
* returned.  Crv is tested for circularity in the XY plane.		     M
*   A curve is circular if its evolute curve is constant.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Curve to attempt and recognize as circular. 		     M
*   Center:    If circular, the center of the circle, zero vector otherwise. M
*   Radius:    If circular, the radius of the circle, zero otherwise.	     M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if circular curve, FALSE otherwise. 	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsSphericalSrf                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsCircularCrv                                                        M
*****************************************************************************/
CagdBType SymbIsCircularCrv(const CagdCrvStruct *Crv,
			    CagdPType Center,
			    CagdRType *Radius,
			    CagdRType Eps)
{
    CagdCtlPtStruct *ConstVal;
    CagdCrvStruct *CurvatureSqr;
    CagdRType *R, Tmin, Tmax;
    CagdPType Pt1, Pt2, Pt3, Ln1Pos, Ln2Pos;
    CagdVType Ln1Dir, Ln2Dir;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(Crv -> PType) > 3) {
        SYMB_FATAL_ERROR(SYMB_ERR_ONLY_2D_OR_3D);
	return FALSE;
    }

    CurvatureSqr = SymbCrv2DCurvatureSqr(Crv);

    if (SymbIsConstCrv(CurvatureSqr, &ConstVal, Eps)) {
	/* Compute the radius. */
	*Radius = ConstVal -> Coords[1] <= 0.0
		 ? IRIT_INFNTY : 1.0 / sqrt(ConstVal -> Coords[1]);

	/* Compute the center numerically. */
	CagdCrvDomain(Crv, &Tmin, &Tmax);
	R = CagdCrvEval(Crv, Tmin);
	CagdCoerceToE3(Pt1, &R, -1, Crv -> PType);
	R = CagdCrvEval(Crv, Tmin * 0.75 + Tmax * 0.25);
	CagdCoerceToE3(Pt2, &R, -1, Crv -> PType);
	R = CagdCrvEval(Crv, (Tmin + Tmax) * 0.5);
	CagdCoerceToE3(Pt3, &R, -1, Crv -> PType);

	IRIT_PT_BLEND(Ln1Pos, Pt1, Pt2, 0.5);
	IRIT_PT_BLEND(Ln2Pos, Pt2, Pt3, 0.5);
	IRIT_PT_SUB(Ln1Dir, Pt1, Pt2);
	VEC_2D_ROTATE(Ln1Dir);
	IRIT_PT_SUB(Ln2Dir, Pt2, Pt3);
	VEC_2D_ROTATE(Ln2Dir);

	CagdCrvFree(CurvatureSqr);

	if (GM2PointsFromLineLine(Ln1Pos, Ln1Dir, Ln2Pos, Ln2Dir,
				  Pt1, &Tmin, Pt2, &Tmax)) {
	    IRIT_PT_BLEND(Center, Pt1, Pt2, 0.5);
	    return TRUE;
	}
	else
	    return FALSE;
    }
    else {
	CagdCrvFree(CurvatureSqr);

	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given curve Crv is indeed circular.	     M
*   If the curve is found to be circular, its center and radius are          M
* returned.  Crv is tested for circularity in the XY plane.		     M
*   A curve is circular if its evolute curve is constant.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       Curve to attempt and recognize as circular. 		     M
*   LnPos:     A point on the line, if the curve is indeed a line.           M
*   LnDir:     A unit direction along the line, if the curve is a line.      M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if a line, FALSE otherwise.	 	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsPlanarSrf, SymbIsSphericalSrf, SymbIsCircularCrv                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsLineCrv                                                            M
*****************************************************************************/
CagdBType SymbIsLineCrv(const CagdCrvStruct *Crv,
			CagdPType LnPos,
			CagdVType LnDir,
			CagdRType Eps)
{
    CagdCtlPtStruct *ConstVal;
    CagdCrvStruct *CurvatureSqr;

    if (CAGD_NUM_OF_PT_COORD(Crv -> PType) < 2 ||
	CAGD_NUM_OF_PT_COORD(Crv -> PType) > 3) {
        SYMB_FATAL_ERROR(SYMB_ERR_ONLY_2D_OR_3D);
	return FALSE;
    }

    CurvatureSqr = SymbCrv2DCurvatureSqr(Crv);

    if (SymbIsConstCrv(CurvatureSqr, &ConstVal, Eps) &&
	IRIT_APX_EQ_EPS(ConstVal -> Coords[1], 0.0, Eps)) {
	CagdCrvFree(CurvatureSqr);

	CagdCoerceToE3(LnPos, Crv -> Points, 0, Crv -> PType);
	CagdCoerceToE3(LnDir, Crv -> Points, Crv -> Length - 1, Crv -> PType);

	IRIT_PT_SUB(LnDir, LnDir, LnPos);
	IRIT_VEC_NORMALIZE(LnDir);
	return TRUE;
    }
    else {
	CagdCrvFree(CurvatureSqr);

	return FALSE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recognize if the given surface Srf is a constant surface.    M
*                                                                            *
* PARAMETERS:                                                                M
*   CSrf:      Surface to attempt and recognize as a constant surface.       M
*   ConstVal:  Resulting constant value if indeed a constant surface. This   M
*	       value is always Euclidean, even for projective surfaces.      M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if a constant surface, FALSE otherwise.                 M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsConstCrv                                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsConstSrf                                                           M
*****************************************************************************/
CagdBType SymbIsConstSrf(const CagdSrfStruct *CSrf,
			 CagdCtlPtStruct **ConstVal,
			 CagdRType Eps)
{
    IRIT_STATIC_DATA CagdCtlPtStruct LclConstVal;
    int i, j,
	Length = CSrf -> ULength * CSrf -> VLength,
	NumCoords = CAGD_NUM_OF_PT_COORD(CSrf -> PType);
    CagdRType **Points;
    CagdSrfStruct *Srf;

    *ConstVal = NULL;

    /* Make sure the surface is Euclidean. */
    Srf = CagdCoerceSrfTo(CSrf,
		CAGD_MAKE_PT_TYPE(FALSE, CAGD_NUM_OF_PT_COORD(CSrf -> PType)),
		FALSE);
    LclConstVal.PtType = Srf -> PType;
    IRIT_ZAP_MEM(LclConstVal.Coords, sizeof(CagdRType) * CAGD_MAX_PT_SIZE);
    LclConstVal.Coords[0] = 1.0;
    Points = Srf -> Points;

    for (i = 1; i <= NumCoords; i++) {
	CagdRType
	    *R = Points[i],
	    R0 = *R++;

	LclConstVal.Coords[i] = R0;
	for (j = 1; j < Length; j++, R++) {
	    if (!IRIT_APX_EQ_EPS(R0, *R, Eps)) {
	        CagdSrfFree(Srf);
		return FALSE;
	    }
	    LclConstVal.Coords[i] += *R;
	}
	LclConstVal.Coords[i] /= Length;
    }

    *ConstVal = &LclConstVal;
    CagdSrfFree(Srf);
    
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recognize if the given surface Srf is an identically zero    M
* surface.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as a zero surface.           M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:  TRUE if a zero surface, FALSE otherwise.                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsConstSrf, SymbIsZeroCrv                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsZeroSrf                                                            M
*****************************************************************************/
CagdBType SymbIsZeroSrf(const CagdSrfStruct *Srf, CagdRType Eps)
{
    int i,
	NumCoords = CAGD_NUM_OF_PT_COORD(Srf -> PType);
    CagdCtlPtStruct *ConstVal;

    if (!SymbIsConstSrf(Srf, &ConstVal, Eps))
	return FALSE;

    for (i = 1; i <= NumCoords; i++) {
	if (!IRIT_APX_EQ_EPS(ConstVal -> Coords[i], 0.0, Eps))
	    return FALSE;
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recognize if the given surface Srf is indeed an extrusion    M
* surface.  If the surface is found to be an extrusion, it is decomposed     M
* into a cross section curve Crv, and an extrusion direction ExtDir.         M
*   A surface is an extrusion surface if one of its partial derivatives is   M
* constant.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as an extrusion surface.     M
*   Crv:       If an extrusion surface, the cross section curve,	     M
*	       NULL otherwise.						     M
*   ExtDir:    The extrusion direction, if an extrusion surface,	     M
*	       zero vector otherwise.				             M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     1 if an extrusion surface along U, 2 if an extrusion surface    M
*	     along V, 0 otherwise.				             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsPlanarSrf, SymbIsRuledSrf, SymbIsDevelopSrf, SymbIsSrfOfRevSrf,    M
*   SymbIsSphericalSrf                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsExtrusionSrf                                                       M
*****************************************************************************/
int SymbIsExtrusionSrf(const CagdSrfStruct *Srf,
		       CagdCrvStruct **Crv,
		       CagdVType ExtDir,
		       CagdRType Eps)
{
    CagdCtlPtStruct *ConstVal;
    CagdBType RetVal;
    CagdPType Pt1, Pt2;
    CagdSrfStruct
	*DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR),
	*DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);

    if (SymbIsConstSrf(DuSrf, &ConstVal, Eps)) {
	*Crv = CagdCrvFromMesh(Srf, 0, CAGD_CONST_U_DIR);
	CagdCoerceToE3(Pt1, Srf -> Points, 0, Srf -> PType);
	CagdCoerceToE3(Pt2, Srf -> Points,
		       CAGD_MESH_UV(Srf, Srf -> ULength - 1, 0), Srf -> PType);
	IRIT_PT_SUB(ExtDir, Pt2, Pt1);
	RetVal = 1;
    }
    else if (SymbIsConstSrf(DvSrf, &ConstVal, Eps)) {
	*Crv = CagdCrvFromMesh(Srf, 0, CAGD_CONST_V_DIR);
	CagdCoerceToE3(Pt1, Srf -> Points, 0, Srf -> PType);
	CagdCoerceToE3(Pt2, Srf -> Points,
		       CAGD_MESH_UV(Srf, 0, Srf -> VLength - 1), Srf -> PType);
	IRIT_PT_SUB(ExtDir, Pt2, Pt1);
	RetVal = 2;
    }
    else {
	IRIT_PT_RESET(ExtDir);
	*Crv = NULL;
	RetVal = 0;
    }

    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given surface Srf is indeed a ruled	     M
* surface. If the surface is found to be a ruled surface, it is decomposed   M
* into its two rail curves (two boundary curves essentially).	             M
*   A surface is a ruled surface if one of its partial derivatives is in the M
* same direction for that parameter.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as a ruled surface.          M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if a ruled surface, FALSE otherwise.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsPlanarSrf, SymbIsRuledSrf, SymbIsSrfOfRevSrf, SymbIsSphericalSrf   M
*   SymbIsExtrusionSrf							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsDevelopSrf                                                         M
*****************************************************************************/
CagdBType SymbIsDevelopSrf(const CagdSrfStruct *Srf, CagdRType Eps)
{
    CagdBType RetVal;
    CagdCtlPtStruct *ConstVal;
    CagdSrfStruct
	*GaussianSrf = SymbSrfGaussCurvature(Srf, FALSE);

    if (SymbIsConstSrf(GaussianSrf, &ConstVal, Eps) &&
	IRIT_APX_EQ_EPS(ConstVal -> Coords[1], 0.0, Eps)) {
	RetVal = TRUE;
    }
    else {
	RetVal = FALSE;
    }

    CagdSrfFree(GaussianSrf);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given surface Srf is indeed a ruled	     M
* surface. If the surface is found to be a ruled surface, it is decomposed   M
* into its two rail curves (two boundary curves essentially).	             M
*   A surface is a ruled surface if one of its partial derivatives is in the M
* same direction for that parameter.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as a ruled surface.          M
*   Crv1:      If a ruled surface, the first curve, NULL otherwise.	     M
*   Crv2:      If a ruled surface, the second curve, NULL otherwise.	     M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:       1 if an extrusion surface along U, 2 if an extrusion surface  M
*	       long V, 0 otherwise.					     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsPlanarSrf, SymbIsDevelopSrf, SymbIsSrfOfRevSrf, SymbIsSphericalSrf M
*   SymbIsExtrusionSrf                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsRuledSrf                                                           M
*****************************************************************************/
int SymbIsRuledSrf(const CagdSrfStruct *Srf,
		   CagdCrvStruct **Crv1,
		   CagdCrvStruct **Crv2,
		   CagdRType Eps)
{
    CagdBType RetVal;
    CagdSrfStruct *DuSrf, *DvSrf, *DuuSrf, *DvvSrf, *CpSrf;

    if (CAGD_IS_RATIONAL_SRF(Srf))
        Srf = CpSrf = CagdCoerceSrfTo(Srf, CAGD_MAKE_PT_TYPE(FALSE,
					  CAGD_NUM_OF_PT_COORD(Srf -> PType)),
			              FALSE);
    else
        CpSrf = NULL;

    DuSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR);
    DvSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR);
    DuuSrf = CagdSrfDerive(DuSrf, CAGD_CONST_U_DIR);
    DvvSrf = CagdSrfDerive(DvSrf, CAGD_CONST_V_DIR);

    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);

    if (SymbIsZeroSrf(DuuSrf, Eps)) {
	*Crv1 = CagdCrvFromMesh(Srf, 0, CAGD_CONST_U_DIR);
	*Crv2 = CagdCrvFromMesh(Srf, Srf -> ULength - 1, CAGD_CONST_U_DIR);

	RetVal = 1;
    }
    else if (SymbIsZeroSrf(DvvSrf, Eps)) {
	*Crv1 = CagdCrvFromMesh(Srf, 0, CAGD_CONST_V_DIR);
	*Crv2 = CagdCrvFromMesh(Srf, Srf -> VLength - 1, CAGD_CONST_V_DIR);

	RetVal = 2;
    }
    else {
	*Crv1 = *Crv2 = NULL;
	RetVal = 0;
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    CagdSrfFree(DuuSrf);
    CagdSrfFree(DvvSrf);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given surface Srf is indeed a surface	     M
* of revolution. If the surface is found to be a surface of revolution, it   M
* is decomposed into its generator (cross section) curve, and the axis of    M
* revolution.								     M
*   A surface is a surface of revolution if the focal surface of one of the  M
* pseudo iso focal surfaces degenerates into a line.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as a ruled surface.          M
*   CrossSec:  If a surface of revolution, the cross section curve, NULL     M
*	       otherwise.						     M
*   AxisPos:   If a surface of revolution, a point on axis of revolution,    M
*	       NULL otherwise.						     M
*   AxisDir:   If a surface of revolution, the direction of the axis of      M
*	       revolution, NULL otherwise.				     M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if a surface of revolution, FALSE otherwise.           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsPlanarSrf, SymbIsRuledSrf, SymbIsDevelopSrf, SymbIsSphericalSrf    M
*   SymbIsExtrusionSrf                                                       M
*                                                                            M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsSrfOfRevSrf                                                        M
*****************************************************************************/
CagdBType SymbIsSrfOfRevSrf(const CagdSrfStruct *Srf,
			    CagdCrvStruct **CrossSec,
			    CagdPType AxisPos,
			    CagdVType AxisDir,
			    CagdRType Eps)
{
    int i, j;
    CagdSrfStruct *IsoFocalE3[2], *TSrf,
	*IsoFocal1 = SymbSrfIsoFocalSrf(Srf, CAGD_CONST_U_DIR),
	*IsoFocal2 = SymbSrfIsoFocalSrf(Srf, CAGD_CONST_V_DIR);

    *CrossSec = NULL;

    /* Add the surface itself to the focal data. */
    TSrf = SymbSrfAdd(IsoFocal1, Srf);
    CagdSrfFree(IsoFocal1);
    IsoFocal1 = TSrf;

    TSrf = SymbSrfAdd(IsoFocal2, Srf);
    CagdSrfFree(IsoFocal2);
    IsoFocal2 = TSrf;

    /* Make sure iso focal surfaces are Euclidean. */
    IsoFocalE3[0] = CagdCoerceSrfTo(IsoFocal1,
		CAGD_MAKE_PT_TYPE(FALSE, CAGD_NUM_OF_PT_COORD(Srf -> PType)),
				    FALSE);
    IsoFocalE3[1] = CagdCoerceSrfTo(IsoFocal2,
		CAGD_MAKE_PT_TYPE(FALSE, CAGD_NUM_OF_PT_COORD(Srf -> PType)),
				    FALSE);
    CagdSrfFree(IsoFocal1);
    CagdSrfFree(IsoFocal2);

    for (i = 0; i < 2; i++) {
	int Len = IsoFocalE3[i] -> ULength * IsoFocalE3[i] -> VLength;
	CagdRType MaxError,
	    **Points = IsoFocalE3[i] -> Points;
	CagdPtStruct
	    *Pts = NULL;

	for (j = 0; j < Len; j++) {
	    CagdPtStruct
		*Pt = CagdPtNew();
	    IRIT_LIST_PUSH(Pt, Pts);
	    Pt -> Pt[0] = Points[1][j];
	    Pt -> Pt[1] = Points[2][j];
	    Pt -> Pt[2] = Points[3][j];
	}

	/* Fit a line to the points and if the fit is good enough - done! */
	MaxError = CagdLineFitToPts(Pts, AxisDir, AxisPos);
	CagdPtFreeList(Pts);

	if (MaxError < Eps)
	    break;
    }

    CagdSrfFree(IsoFocalE3[0]);
    CagdSrfFree(IsoFocalE3[1]);

    if (i < 2) {
	/* Extract the other direction as a cross section curve. */
	*CrossSec = CagdCrvFromMesh(Srf, 0, i == 1 ? CAGD_CONST_V_DIR
						   : CAGD_CONST_U_DIR);
	return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given surface Srf is indeed a sphere.	     M
*   If the surface is found to be a sphere, its center and radius are        M
* returned.								     M
*   A surface is a sphere if its Gaussian and Mean sqaure surfaces are       M
* constant and equal.  The center of the sphere is derived from the mean     M
* evolute surface.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as a sphere. 		     M
*   Center:    If a sphere, the center of the sphere, zero vector otherwise. M
*   Radius:    If a sphere, the radius of the sphere, zero otherwise.	     M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if a sphere surface, FALSE otherwise. 	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsPlanarSrf, SymbIsRuledSrf, SymbIsDevelopSrf, SymbIsSrfOfRevSrf     M
*   SymbIsExtrusionSrf, SymbIsCircularCrv                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsSphericalSrf                                                       M
*****************************************************************************/
CagdBType SymbIsSphericalSrf(const CagdSrfStruct *Srf,
			     CagdPType Center,
			     CagdRType *Radius,
			     CagdRType Eps)
{
    CagdRType KappaSqr;
    CagdCtlPtStruct *ConstVal;
    CagdSrfStruct
	*MeanSqrSrf = SymbSrfMeanCurvatureSqr(Srf),
	*GaussianSrf = SymbSrfGaussCurvature(Srf, FALSE);

    if (SymbIsConstSrf(GaussianSrf, &ConstVal, Eps) &&
	(KappaSqr = ConstVal -> Coords[1]) > 0.0 &&
	SymbIsConstSrf(MeanSqrSrf, &ConstVal, Eps) &&
	IRIT_APX_EQ_EPS(KappaSqr, ConstVal -> Coords[1], Eps)) {
        CagdVecStruct *Nrml;
        CagdRType *R, u, v, UMin, UMax, VMin, VMax;
	CagdPType Ln1Pos, Ln2Pos, Pt1, Pt2;
	CagdVType Ln1Dir, Ln2Dir;

	CagdSrfFree(GaussianSrf);
	CagdSrfFree(MeanSqrSrf);

	/* Compute the radius. */
	*Radius = 1.0 / sqrt(KappaSqr);

	/* Compute the center numerically. */
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

	/* Compute two lines through the sphere's center. */
	u = 0.75 * UMin + 0.25 * UMax;
	v = 0.75 * VMin + 0.25 * VMax;
	if ((Nrml = CagdSrfNormal(Srf, u, v, FALSE)) == NULL)
	    return FALSE;
	IRIT_VEC_COPY(Ln1Dir, Nrml -> Vec);
	R = CagdSrfEval(Srf, u, v);
	CagdCoerceToE3(Ln1Pos, &R, -1, Srf -> PType);

	u = 0.25 * UMin + 0.75 * UMax;
	v = 0.25 * VMin + 0.75 * VMax;
	if ((Nrml = CagdSrfNormal(Srf, u, v, FALSE)) == NULL)
	    return FALSE;
	IRIT_VEC_COPY(Ln2Dir, Nrml -> Vec);
	R = CagdSrfEval(Srf, u, v);
	CagdCoerceToE3(Ln2Pos, &R, -1, Srf -> PType);

	GM2PointsFromLineLine(Ln1Pos, Ln1Dir, Ln2Pos, Ln2Dir,
			      Pt1, &u, Pt2, &v);

	IRIT_PT_BLEND(Center, Pt1, Pt2, 0.5);

	return TRUE;
    }
    else {
	CagdSrfFree(GaussianSrf);
	CagdSrfFree(MeanSqrSrf);

	return FALSE;
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Attempts to recongnize if the given curve Crv is indeed circular.	     M
*   If the curve is found to be circular, its center and radius are          M
* returned.  Crv is tested for circularity in the XY plane.		     M
*   A surface is plane if its gaussian and mean curvatures are zero.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to attempt and recognize as circular. 		     M
*   Plane:     The plane equation, if the surface is indeed planar.          M
*   Eps:       Tolarence of "same" value.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType:   TRUE if a line, FALSE otherwise.	 	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbIsRuledSrf, SymbIsDevelopSrf, SymbIsSrfOfRevSrf, SymbIsSphericalSrf  M
*   SymbIsExtrusionSrf, SymbIsLineCrv                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbIsPlanarSrf                                                          M
*****************************************************************************/
CagdBType SymbIsPlanarSrf(const CagdSrfStruct *Srf,
			  IrtPlnType Plane,
			  CagdRType Eps)
{
    CagdCtlPtStruct *ConstVal;
    CagdSrfStruct
	*MeanSqrSrf = SymbSrfMeanCurvatureSqr(Srf),
	*GaussianSrf = SymbSrfGaussCurvature(Srf, FALSE);

    if (SymbIsConstSrf(GaussianSrf, &ConstVal, Eps) &&
	IRIT_APX_EQ_EPS(ConstVal -> Coords[1], 0.0, Eps) &&
	SymbIsConstSrf(MeanSqrSrf, &ConstVal, Eps) &&
	IRIT_APX_EQ_EPS(ConstVal -> Coords[1], 0.0, Eps)) {
        CagdPType Pt1, Pt2, Pt3;

	CagdSrfFree(GaussianSrf);
	CagdSrfFree(MeanSqrSrf);

	CagdCoerceToE3(Pt1, Srf -> Points,
		       CAGD_MESH_UV(Srf, 0, 0), Srf -> PType);
	CagdCoerceToE3(Pt2, Srf -> Points,
		       CAGD_MESH_UV(Srf, Srf -> ULength - 1, 0), Srf -> PType);
	CagdCoerceToE3(Pt3, Srf -> Points,
		       CAGD_MESH_UV(Srf, 0, Srf -> VLength - 1), Srf -> PType);

	if (GMPlaneFrom3Points(Plane, Pt1, Pt2, Pt3))
	    return TRUE;
    }

    CagdSrfFree(GaussianSrf);
    CagdSrfFree(MeanSqrSrf);

    return FALSE;
}
