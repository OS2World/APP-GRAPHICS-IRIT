/******************************************************************************
* Bsp2Poly.c - Bezier to polygon/polylines conversion routines.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include "cagd_loc.h"

IRIT_STATIC_DATA CagdSrfErrorFuncType
    BspSrf2PolygonErrFunc = NULL;

IRIT_GLOBAL_DATA int
    _CagdSrf2PolygonStrips = FALSE,
    _CagdSrf2PolygonFast = 0x02;            /* Fast normals, exact polygons. */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the polygonal approximation of surfaces to create polygonal strips  M
* (if TRUE) or regular individual polygons (if FALSE).  If TRUE this hints   M
* the ability and desire to use polygonal strips but it does not guarantee   M
* that only polygonal strips would indeed be returned.  Regular polygonal    M
* data should always be handled as well.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolygonStrip:        New setting to use.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value.				                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2Polygons, CagdSrf2PolygonFast, CagdSrf2PolygonMergeCoplanar      M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolygonStrip                                                     M
*****************************************************************************/
int CagdSrf2PolygonStrip(int PolygonStrip)
{
    int OldVal = _CagdSrf2PolygonStrips;

    _CagdSrf2PolygonStrips = PolygonStrip;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the polygonal approximation of surfaces to create polygonal data    M
*fast and approximated (if TRUE) or slowly and exact (if FALSE).	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PolygonFast:        New setting to use.		                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value.				                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2Polygons, CagdSrf2PolygonStrip, CagdSrf2PolygonMergeCoplanar     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolygonFast                                                      M
*****************************************************************************/
int CagdSrf2PolygonFast(int PolygonFast)
{
    int OldVal = _CagdSrf2PolygonFast;

    _CagdSrf2PolygonFast = PolygonFast;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the surface approximation error function.  The error function       M
* will return a negative value if this patch must be purged or otherwise a   M
* non negative error measure.                                                M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfErrorFuncType:  Old value of function.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2Polygons                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrf2PolygonSetErrFunc                                                 M
*****************************************************************************/
CagdSrfErrorFuncType BspSrf2PolygonSetErrFunc(CagdSrfErrorFuncType Func)
{
    CagdSrfErrorFuncType
	OldFunc = BspSrf2PolygonErrFunc;

    BspSrf2PolygonErrFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single Bspline surface to set of triangles	     M
* approximating it. FineNess is a fineness control on result and the larger  M
t is more triangles may result. A value of 10 is a good start value.	     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
*   This routine looks for C1 discontinuities in the surface and splits it   M
* into C1 continuous patches to invoke BspC1Srf2Polygons to gen. polygons.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To approximate into triangles.                         M
*   FineNess:         Control on accuracy, the higher the finer.             M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   FourPerFlat:      If TRUE, four triangles are created per flat surface.  M
*                     If FALSE, only 2 triangles are created.                M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2PolygonSetErrFunc, BzrSrf2Polygons, IritSurface2Polygons,	     M
*   IritTrimSrf2Polygons, CagdSrf2Polygons, TrimSrf2Polygons,		     M
*   BspC1Srf2Polygons, CagdSrf2Polygons					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrf2Polygons, polygonization, surface approximation                   M
*****************************************************************************/
CagdPolygonStruct *BspSrf2Polygons(const CagdSrfStruct *Srf,
				   int FineNess,
				   CagdBType ComputeNormals,
				   CagdBType FourPerFlat,
				   CagdBType ComputeUV)
{
    CagdBType HasUDiscont, HasVDiscont;
    CagdRType u, v;
    CagdPolygonStruct *Poly;
    CagdSrfStruct *CpSrf;

    if (CAGD_IS_PERIODIC_SRF(Srf)) {
        Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    }
    else
        CpSrf = NULL;

    HasUDiscont = BspSrfKnotC1Discont(Srf, CAGD_CONST_U_DIR, &u);
    HasVDiscont = BspSrfKnotC1Discont(Srf, CAGD_CONST_V_DIR, &v);

    if (HasUDiscont || HasVDiscont) {
	CagdSrfStruct
	    *Srf1 = HasUDiscont ? BspSrfSubdivAtParam(Srf, u,
						      CAGD_CONST_U_DIR)
				: BspSrfSubdivAtParam(Srf, v,
						      CAGD_CONST_V_DIR),
	    *Srf2 = Srf1 -> Pnext;
	CagdPolygonStruct *Poly1, *Poly2;

	CAGD_PROPAGATE_ATTR(Srf1, Srf);
	CAGD_PROPAGATE_ATTR(Srf2, Srf);

	Poly1 = BspSrf2Polygons(Srf1, FineNess,
				ComputeNormals, FourPerFlat, ComputeUV),
	Poly2 = BspSrf2Polygons(Srf2, FineNess,
				ComputeNormals, FourPerFlat, ComputeUV);

	CagdSrfFreeList(Srf1);

	/* Chain the two lists together: */
	Poly = (CagdPolygonStruct *) CagdListAppend(Poly1, Poly2);
    }
    else {
	if (BspSrf2PolygonErrFunc != NULL && BspSrf2PolygonErrFunc(Srf) < 0.0)
	    Poly = NULL;
	else {
	    int FineNessU, FineNessV;
	    CagdRType *PtWeights;
	    CagdPtStruct *PtMesh;
	    CagdVecStruct *PtNrml;
	    CagdUVStruct *UVMesh;

	    if (BspC1Srf2PolygonsSamples(Srf, FineNess, ComputeNormals,
					 ComputeUV, &PtWeights, &PtMesh,
					 &PtNrml, &UVMesh,
					 &FineNessU, &FineNessV))
	        Poly = CagdSrf2PolygonsGenPolys(Srf, FourPerFlat, PtWeights,
						PtMesh, PtNrml, UVMesh,
						FineNessU, FineNessV);
	    else
	        Poly = NULL;
	}
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single Bspline surface to set of triangles	     M
* approximating it. FineNess is a fineness control on result and the larger  M
t is more triangles may result. A value of 10 is a good start value.	     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
*   This routine looks for C1 discontinuities in the surface and splits it   M
* into C1 continuous patches to invoke BspC1Srf2Polygons to gen. polygons.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To approximate into triangles.                         M
*   Nu, Nv:           The number of uniform samples in U and V of surface.   M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   FourPerFlat:      If TRUE, four triangles are created per flat surface.  M
*                     If FALSE, only 2 triangles are created.                M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2PolygonSetErrFunc, BzrSrf2Polygons, IritSurface2Polygons,	     M
*   IritTrimSrf2Polygons, CagdSrf2Polygons, TrimSrf2Polygons,		     M
*   BspC1Srf2Polygons, CagdSrf2Polygons, BspSrf2Polygons,		     M
*   BzrSrf2PolygonsN, CagdSrf2PolygonsN					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrf2PolygonsN, polygonization, surface approximation                  M
*****************************************************************************/
CagdPolygonStruct *BspSrf2PolygonsN(const CagdSrfStruct *Srf,
				    int Nu,
				    int Nv,
				    CagdBType ComputeNormals,
				    CagdBType FourPerFlat,
				    CagdBType ComputeUV)
{
    CagdRType *PtWeights;
    CagdPtStruct *PtMesh;
    CagdVecStruct *PtNrml;
    CagdUVStruct *UVMesh;
			     
    if (BspSrf2PolygonsSamplesNuNv(Srf, Nu, Nv, ComputeNormals, ComputeUV,
				   &PtWeights, &PtMesh, &PtNrml, &UVMesh))
	return CagdSrf2PolygonsGenPolys(Srf, FourPerFlat, PtWeights, PtMesh,
					PtNrml,	UVMesh, Nu, Nv);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to uniformly sample a single Bspline srf as a grid.              M
*   Nu and Nv fix the grid's sizes.					     M
*   FALSE is returned in case of an error, TRUE otherwise.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To sample in a grid.                                   M
*   Nu, Nv:           The number of uniform samples in U and V of surface.   M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*   PtWeights:        Weights of the evaluations, if rational, to detect     M
*		      poles.  NULL if surface nor rational.		     M
*   PtMesh:	      Evaluted positions of grid of samples.                 M
*   PtNrml:	      Evaluted normals of grid of samples or NULL if none.   M
*   UVMesh:	      Evaluted UV vals of grid of samples or NULL if none.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:              FALSE is returned in case of an error, TRUE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2Polygons, IritSurface2Polygons, IritTrimSrf2Polygons,             M
*   CagdSrf2Polygons, TrimSrf2Polygons, BzrSrf2PolygonsSamples,              M
*   BspC1Srf2PolygonsSamples						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrf2PolygonsSamplesNuNv                                               M
*****************************************************************************/
int BspSrf2PolygonsSamplesNuNv(const CagdSrfStruct *Srf,
			       int Nu,
			       int Nv,
			       CagdBType ComputeNormals,
			       CagdBType ComputeUV,
			       CagdRType **PtWeights,
			       CagdPtStruct **PtMesh,
			       CagdVecStruct **PtNrml,
			       CagdUVStruct **UVMesh)
{
    int i, j, MeshSize, FineNessU1, FineNessV1,
	IsRational = CAGD_IS_RATIONAL_SRF(Srf);
    CagdRType UMin, UMax, VMin, VMax, *PtWeightsPtr, *Pt;
    CagdPointType
	PType = Srf -> PType;
    CagdPtStruct *PtMeshPtr;
    CagdUVStruct *UVMeshPtr;

    if (Nu < 2 || Nv < 2) {
	CAGD_FATAL_ERROR(CAGD_ERR_WRONG_SIZE);
	return FALSE;
    }

    *PtNrml = NULL;
    *UVMesh = NULL;

    BspSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    FineNessU1 = Nu - 1;
    FineNessV1 = Nv - 1;
    MeshSize = Nu * Nv;

    /* Allocate a mesh to hold all vertices so common vertices need not be   */
    /* Evaluated twice, and evaluate the surface at these mesh points.	     */
    PtMeshPtr = (*PtMesh) = CagdPtArrayNew(MeshSize);

    PtWeightsPtr = (*PtWeights) = 
	IsRational ? IritMalloc(sizeof(CagdRType) * MeshSize) : NULL;

    for (i = 0; i < Nu; i++) {
	CagdCrvStruct
	    *Crv = BspSrfCrvFromSrf(Srf, UMin + (UMax - UMin) * 
				                 ((CagdRType) i) / FineNessU1,
				    CAGD_CONST_U_DIR);

	for (j = 0; j < Nv; j++) {
	    Pt = BspCrvEvalAtParam(Crv, VMin + (VMax - VMin) *
				                ((CagdRType) j) / FineNessV1);
	    CagdCoerceToE3(PtMeshPtr -> Pt, &Pt, -1, PType);
	    PtMeshPtr++;

	    if (IsRational)
		*PtWeightsPtr++ = Pt[0];
	}

	CagdCrvFree(Crv);
    }

    if (ComputeNormals) {
	*PtNrml = BspSrfMeshNormals(Srf, Nu, Nv);
    }

    if (ComputeUV) {
        CagdRType u, v,
	    du = (UMax - UMin) / FineNessU1,
	    dv = (VMax - VMin) / FineNessV1;

	UVMeshPtr = (*UVMesh) = CagdUVArrayNew(MeshSize);

	for (i = 0, u = UMin; i <= FineNessU1; i++, u += du) {
	    if (u > UMax)                /* Due to floating point round off. */
	        u = UMax;

	    for (j = 0, v = VMin; j <= FineNessV1; j++, v += dv) {
	        if (v > VMax)        /* Due to floating point round off. */
		    v = VMax;

		UVMeshPtr -> UV[0] = u;
		UVMeshPtr++ -> UV[1] = v;
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to uniformly sample a single C1 continuous Bspline srf as grid.  M
* FineNess is a fineness control on the result and the larger it is more     M
* samples may result. A value of 10 is a good starting                       M
* value.  FALSE is returned in case of an error, TRUE otherwise.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To sample in a grid.                                   M
*   FineNess:         Control on accuracy, the higher the finer.             M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*   PtWeights:        Weights of the evaluations, if rational, to detect     M
*		      poles.  NULL if surface nor rational.		     M
*   PtMesh:	      Evaluted positions of grid of samples.                 M
*   PtNrml:	      Evaluted normals of grid of samples or NULL if none.   M
*   UVMesh:	      Evaluted UV vals of grid of samples or NULL if none.   M
*   FineNessU, FineNessV:  Actual size of PtMesh, PtNrml, UVMesh.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:              FALSE is returned in case of an error, TRUE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrf2Polygons, IritSurface2Polygons, IritTrimSrf2Polygons,             M
*   CagdSrf2Polygons, TrimSrf2Polygons, BspSrf2PolygonsSamplesNuNv           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspC1Srf2PolygonsSamples, polygonization, surface approximation          M
*****************************************************************************/
int BspC1Srf2PolygonsSamples(const CagdSrfStruct *Srf,
			     int FineNess,
			     CagdBType ComputeNormals,
			     CagdBType ComputeUV,
			     CagdRType **PtWeights,
			     CagdPtStruct **PtMesh,
			     CagdVecStruct **PtNrml,
			     CagdUVStruct **UVMesh,
			     int *FineNessU,
			     int *FineNessV)
{
    int i, j, MeshSize, FineNessU1, FineNessV1, FineNessURef, FineNessVRef,
	IsRational = CAGD_IS_RATIONAL_SRF(Srf);
    CagdRType t, du, dv, u, v, UMin, UMax, VMin, VMax, **Points, *Pt,
	*PtWeightsPtr, RealFineNessU, RealFineNessV;
    CagdPtStruct *PtMeshPtr;
    CagdUVStruct
	*UVMeshPtr = NULL;
    CagdCrvStruct *Crv;
    CagdSrfStruct *TSrf;
    CagdPointType
	PType = Srf -> PType;

    if (!CAGD_IS_BSPLINE_SRF(Srf))
	return FALSE;

    *PtNrml = NULL;
    *UVMesh = NULL;

#   ifdef CAGD_ARC_LEN_CRVTR_POLY_ESTIMATE
    {
	CagdRType UCurveness, VCurveness;

        /* Simple heuristic to estimate how many samples to compute. */
        if ((t = CagdSrfAvgArgLenMesh(Srf, &du, &dv)) > 1.0) {	 /* du > dv. */
	    dv = dv / du;
	    du = 1.0;
	}
	else {							 /* dv > du. */
	    du = du / dv;
	    dv = 1.0;
	}
	CagdSrfEstimateCurveness(Srf, &UCurveness, &VCurveness);
	if (UCurveness > VCurveness) {		 /* UCurveness > VCurveness. */
	    VCurveness = VCurveness / UCurveness;
	    UCurveness = 1.0;
	}
	else {					 /* VCurveness > UCurveness. */
	    UCurveness = UCurveness / VCurveness;
	    VCurveness = 1.0;
	}
	RealFineNessU = Srf -> ULength * du * UCurveness * FineNess / 10;
	RealFineNessV = Srf -> VLength * dv * VCurveness * FineNess / 10;
    }
#   else
	RealFineNessU = Srf -> ULength * FineNess / 10;
        RealFineNessV = Srf -> VLength * FineNess / 10;
#   endif /* CAGD_ARC_LEN_CRVTR_POLY_ESTIMATE */

    /* Take the mesh if sample less than the mesh size but not twice that! */
    if (RealFineNessU < Srf -> ULength)
	RealFineNessU = Srf -> ULength;
    if (RealFineNessV < Srf -> VLength)
	RealFineNessV = Srf -> VLength;
    u = RealFineNessU / (FineNess * 2.0);
    v = RealFineNessV / (FineNess * 2.0);
    if (u > 1.0 || v > 1.0) {
        if (u > v) {
	    RealFineNessU /= u;
	    RealFineNessV /= u;
	}
	else {
	    RealFineNessU /= v;
	    RealFineNessV /= v;
	}
    }
    t = AttrGetRealAttrib(Srf -> Attr, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        RealFineNessU *= t;
    t = AttrGetRealAttrib(Srf -> Attr, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        RealFineNessV *= t;

    *FineNessU = (int) IRIT_BOUND(RealFineNessU, 3, CAGD_MAX_FINENESS);
    *FineNessV = (int) IRIT_BOUND(RealFineNessV, 3, CAGD_MAX_FINENESS);

    FineNessU1 = *FineNessU - 1;
    FineNessV1 = *FineNessV - 1;
    MeshSize = *FineNessU * *FineNessV;

    /* Current to surface property such as curvature is used as subdivison   */
    /* criterion and the surface is subdivided, equally spaced in parametric */
    /* space, using FineNess as number of subdivisions per axis.	     */

    /* Allocate a mesh to hold all vertices so common vertices need not be   */
    /* Evaluated twice, and evaluate the surface at these mesh points.	     */
    PtMeshPtr = (*PtMesh) = CagdPtArrayNew(MeshSize);

    PtWeightsPtr = (*PtWeights) = 
	IsRational ? IritMalloc(sizeof(CagdRType) * MeshSize) : NULL;

    if (ComputeUV)
	UVMeshPtr = (*UVMesh) = CagdUVArrayNew(MeshSize);

    BspSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    if (_CagdSrf2PolygonFast & 0x01) { /* Creat polys fast and approximated. */
        /* Evaluate the mesh by global refinments of the contol mesh. */
        FineNessURef = *FineNessU - Srf -> ULength;
	FineNessVRef = *FineNessV - Srf -> VLength;

        if (FineNessURef > 0 || FineNessVRef > 0) {
	    CagdSrfStruct *TSrf2;
	    CagdRType u, v, du, dv,
	        *RefKV = (CagdRType *) IritMalloc(sizeof(CagdRType) *
					      IRIT_MAX(FineNessURef, FineNessVRef));

	    if (FineNessURef > 0) {
	        du = (UMax - UMin) / (FineNessURef + 1);
		for (i = 0, u = UMin + du; i < FineNessURef; i++, u += du)
		    RefKV[i] = u;

		TSrf = CagdSrfRefineAtParams(Srf, CAGD_CONST_U_DIR, FALSE,
					     RefKV, FineNessURef);
	    }
	    else
	        TSrf = CagdSrfCopy(Srf);
	
	    if (FineNessVRef > 0) {
	        dv = (VMax - VMin) / (FineNessVRef + 1);
		for (i = 0, v = VMin + dv; i < FineNessVRef; i++, v += dv)
		    RefKV[i] = v;

		TSrf2 = CagdSrfRefineAtParams(TSrf, CAGD_CONST_V_DIR, FALSE,
					      RefKV, FineNessVRef);
		CagdSrfFree(TSrf);
		TSrf = TSrf2;
	    }

	    IritFree(RefKV);
	}
	else
	    TSrf = CagdSrfCopy(Srf);

	/* Copy the refined control mesh to the sampled mesh in E3 form. */
	Points = TSrf -> Points;
	du = (TSrf -> ULength - 1) / ((CagdRType) FineNessU1);
	dv = (TSrf -> VLength - 1) / ((CagdRType) FineNessV1);
	for (i = 0, u = 0; i <= FineNessU1; i++, u += du) {
	    for (j = 0, v = 0; j <= FineNessV1; j++, v += dv, PtMeshPtr++) {
	        int Idx = ((int) (v + 0.5)) * TSrf -> ULength
			+ ((int) (u + 0.5));

	        CagdCoerceToE3(PtMeshPtr -> Pt, Points, Idx, PType);

		if (IsRational)
		    *PtWeightsPtr++ = Points[0][Idx];
	    }
	}

	if (ComputeUV) {
	    du = (UMax - UMin) / FineNessU1;
	    dv = (VMax - VMin) / FineNessV1;

	    for (i = 0, u = UMin; i <= FineNessU1; i++, u += du) {
		if (u > UMax)            /* Due to floating point round off. */
		    u = UMax;

		for (j = 0, v = VMin; j <= FineNessV1; j++, v += dv) {
		    if (v > VMax)        /* Due to floating point round off. */
		        v = VMax;

		    UVMeshPtr -> UV[0] = u;
		    UVMeshPtr++ -> UV[1] = v;
		}
	    }
	}

	if (ComputeNormals) {
	    *PtNrml = BspSrfMeshNormals(TSrf, *FineNessU, *FineNessV);
	}

	CagdSrfFree(TSrf);
    }
    else {			/* Create polygons slowely but more exactly. */
        /* Evaluate the mesh by extracting curves and evaluating them. */
        for (i = 0; i <= FineNessU1; i++) {
	    CagdRType
	        u = UMin + (UMax - UMin) * i / ((CagdRType) FineNessU1);

	    if (u > UMax)                /* Due to floating point round off. */
	        u = UMax;
	    Crv = BspSrfCrvFromSrf(Srf, u, CAGD_CONST_U_DIR);

	    for (j = 0; j <= FineNessV1; j++, PtMeshPtr++) {
	        CagdRType
		    v = VMin + (VMax - VMin) * j / ((CagdRType) FineNessV1);

		if (v > VMax)            /* Due to floating point round off. */
		    v = VMax;
		Pt = BspCrvEvalAtParam(Crv, v);
		CagdCoerceToE3(PtMeshPtr -> Pt, &Pt, -1, PType);

		if (IsRational)
		    *PtWeightsPtr++ = Pt[0];

		if (ComputeUV) {
		    UVMeshPtr -> UV[0] = u;
		    UVMeshPtr -> UV[1] = v;
		    UVMeshPtr++;
		}
	    }

	    CagdCrvFree(Crv);
	}

	if (ComputeNormals) {
	    *PtNrml = BspSrfMeshNormals(Srf, *FineNessU, *FineNessV);
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single Bspline surface to NumOfIsolines polylines   M
* in each parametric direction with SamplesPerCurve in each isoparametric    M
* curve.                                                                     M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. Attempt is made to extract isolines along C1           M
* discontinuities first.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:                 Srf to extract isoparametric curves from.           M
*   NumOfIsocurves:      To extarct from Srf in each (U or V) direction.     M
*   SamplesPerCurve:     Fineness control on piecewise linear curve          M
*                        approximation.                                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *: List of polygons representing a piecewise linear   M
*                         approximation of the extracted isoparamteric       M
*                         curves or NULL is case of an error.                M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrSrf2Polylines, IritSurface2Polylines,                M
*   IritTrimSrf2Polylines, SymbSrf2Polylines, TrimSrf2Polylines              M
*   CagdSrf2Polylines                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrf2Polylines, polylines, isoparametric curves                        M
*****************************************************************************/
CagdPolylineStruct *BspSrf2Polylines(const CagdSrfStruct *Srf,
				     int NumOfIsocurves[2],
				     int SamplesPerCurve)
{
    CagdBType
	NewSrf = FALSE;
    int i, NumC1Disconts, NumOfIsos, ULength, VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder;
    CagdRType u, v, UMin, UMax, VMin, VMax, *C1Disconts, *IsoParams, *RefKV,
	*UKV, *VKV;
    CagdCrvStruct *Crv;
    CagdSrfStruct *CpSrf;
    CagdPolylineStruct *Poly, *p,
	*PolyList = NULL;
    BspKnotAlphaCoeffStruct *A;

    if (!CAGD_IS_BSPLINE_SRF(Srf))
	return NULL;

    if (CAGD_IS_PERIODIC_SRF(Srf))
        Srf = CpSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
    else
        CpSrf = NULL;

    UKV = Srf -> UKnotVector;
    VKV = Srf -> VKnotVector;
    ULength = Srf -> ULength;
    VLength = Srf -> VLength;

    /* Make sure surface is open. We move 2 Epsilons to make sure region    */
    /* extraction will occur. Otherwise the curve will be copied as is.     */
    if (!BspKnotHasOpenEC(UKV, ULength, UOrder) ||
	!BspKnotHasOpenEC(VKV, VLength, VOrder)) {
	CagdSrfStruct
	    *TSrf = CagdSrfRegionFromSrf(Srf,
					 UKV[UOrder - 1],
					 UKV[ULength],
					 CAGD_CONST_U_DIR);

	if (CpSrf != NULL) {
	    CagdSrfFree(CpSrf);
	    CpSrf = NULL;
	}

	Srf = CagdSrfRegionFromSrf(TSrf,
				   VKV[VOrder - 1],
				   VKV[VLength],
				   CAGD_CONST_V_DIR);
	NewSrf = TRUE;

	CagdSrfFree(TSrf);
    }

    /* Make sure requested format is something reasonable. */
    if (SamplesPerCurve < 2)
	SamplesPerCurve = 2;

    if (NumOfIsocurves[0] < 0)
	NumOfIsocurves[0] = 0;
    if (NumOfIsocurves[1] < 0)
	NumOfIsocurves[1] = 0;

    BspSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Compute discontinuities along the u axis and use that to determine    */
    /* where to extract isolines along u.				     */
    /* Note C1Disconts is freed by BspKnotParamValues.			     */
    if ((NumOfIsos = NumOfIsocurves[0]) > 0) {
	C1Disconts = BspKnotAllC1Discont(Srf -> UKnotVector, UOrder,
					 ULength, &NumC1Disconts);
	IsoParams = BspKnotParamValues(UMin, UMax, NumOfIsos, C1Disconts,
								NumC1Disconts);
	RefKV = BspKnotPrepEquallySpaced(IRIT_MAX(SamplesPerCurve - VLength, 1),
					 VMin, VMax);
	A = BspKnotEvalAlphaCoefMerge(VOrder, Srf -> VKnotVector,
				      VLength, RefKV,
				      IRIT_MAX(SamplesPerCurve - VLength, 1),
				      FALSE);
	IritFree(RefKV);

	for (i = 0; i < NumOfIsos; i++) {
	    u = IsoParams[i];

	    Crv = BspSrfCrvFromSrf(Srf, u, CAGD_CONST_U_DIR);
	    Poly = BspCrv2Polyline(Crv, SamplesPerCurve, A, TRUE);
	    for (p = Poly; p != NULL; p = p -> Pnext)
	        AttrSetRealAttrib(&p -> Attr, "UIsoParam", u);
	    ((CagdPolylineStruct * ) CagdListLast(Poly)) -> Pnext = PolyList;
	    PolyList = Poly;
	    CagdCrvFree(Crv);
	}
	IritFree(IsoParams);
	BspKnotFreeAlphaCoef(A);
    }

    /* Compute discontinuities along the v axis and use that to determine    */
    /* where to extract isolines along v.				     */
    /* Note C1Disconts is freed by BspKnotParamValues.			     */
    if ((NumOfIsos = NumOfIsocurves[1]) > 0) {
	C1Disconts = BspKnotAllC1Discont(Srf -> VKnotVector, VOrder,
					 VLength, &NumC1Disconts);
	IsoParams = BspKnotParamValues(VMin, VMax, NumOfIsos, C1Disconts,
								NumC1Disconts);
	RefKV = BspKnotPrepEquallySpaced(IRIT_MAX(SamplesPerCurve - ULength, 1),
					 UMin, UMax);
	A = BspKnotEvalAlphaCoefMerge(UOrder, Srf -> UKnotVector,
				      ULength, RefKV,
				      IRIT_MAX(SamplesPerCurve - ULength, 1),
				      FALSE);
	IritFree(RefKV);

	for (i = 0; i < NumOfIsos; i++) {
	    v = IsoParams[i];

	    Crv = BspSrfCrvFromSrf(Srf, v, CAGD_CONST_V_DIR);
	    Poly = BspCrv2Polyline(Crv, SamplesPerCurve, A, TRUE);
	    for (p = Poly; p != NULL; p = p -> Pnext)
	        AttrSetRealAttrib(&p -> Attr, "VIsoParam", v);
	    ((CagdPolylineStruct * ) CagdListLast(Poly)) -> Pnext = PolyList;
	    PolyList = Poly;
	    CagdCrvFree(Crv);
	}
	IritFree(IsoParams);
	BspKnotFreeAlphaCoef(A);
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    return PolyList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to extract from a Bspline surface NumOfIsoline isocurve list     M
* in each param. direction.						     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of CagdCrvStruct.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             To extract isoparametric curves from.                   M
*   NumOfIsocurves:  In each (U or V) direction.                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of extracted isoparametric curves. These curves   M
*                     inherit the order and continuity of the original Srf.  M
*                     NULL is returned in case of an error.                  M
* SEE ALSO:                                                                  M
*   BspSrf22Polylines, BzrSrf2PCurves, SymbSrf2Curves                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrf2Curves, curves, isoparametric curves                              M
*****************************************************************************/
CagdCrvStruct *BspSrf2Curves(const CagdSrfStruct *Srf,
			     int NumOfIsocurves[2])
{
    int i, NumC1Disconts,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength,
	UOrder = Srf -> UOrder,
	VOrder = Srf -> VOrder;
    CagdRType u, v, UMin, UMax, VMin, VMax, *C1Disconts, *IsoParams;
    CagdCrvStruct *Crv,
	*CrvList = NULL;

    if (!CAGD_IS_BSPLINE_SRF(Srf))
	return NULL;

    /* Make sure requested format is something reasonable. */
    if (NumOfIsocurves[0] < 0)
	NumOfIsocurves[0] = 0;
    if (NumOfIsocurves[1] < 0)
	NumOfIsocurves[1] = 0;

    BspSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Compute discontinuities along the u axis and use that to determine    */
    /* where to extract isolines along u.				     */
    /* Note C1Disconts is freed by BspKnotParamValues.			     */
    C1Disconts = BspKnotAllC1Discont(Srf -> UKnotVector, UOrder,
				     ULength, &NumC1Disconts);
    IsoParams = BspKnotParamValues(UMin, UMax, NumOfIsocurves[0], C1Disconts,
								NumC1Disconts);

    for (i = 0; i < NumOfIsocurves[0]; i++) {
	u = IsoParams[i];

	Crv = BspSrfCrvFromSrf(Srf, u, CAGD_CONST_U_DIR);
	AttrSetRealAttrib(&Crv -> Attr, "UIsoParam", u);
	Crv -> Pnext = CrvList;
	CrvList = Crv;
    }
    if (IsoParams != NULL)
        IritFree(IsoParams);

    /* Compute discontinuities along the v axis and use that to determine    */
    /* where to extract isolines along v.				     */
    /* Note C1Disconts is freed by BspKnotParamValues.			     */
    C1Disconts = BspKnotAllC1Discont(Srf -> VKnotVector, VOrder,
				     VLength, &NumC1Disconts);
    IsoParams = BspKnotParamValues(VMin, VMax, NumOfIsocurves[1], C1Disconts,
								NumC1Disconts);

    for (i = 0; i < NumOfIsocurves[1]; i++) {
	v = IsoParams[i];

	Crv = BspSrfCrvFromSrf(Srf, v, CAGD_CONST_V_DIR);
	AttrSetRealAttrib(&Crv -> Attr, "VIsoParam", v);
	Crv -> Pnext = CrvList;
	CrvList = Crv;
    }
    if (IsoParams != NULL)
        IritFree(IsoParams);

    return CrvList;
}



/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to approx. a single Bspline curve as a polyline with	     M
* SamplesPerCurve samples. Polyline is always E3 CagdPolylineStruct type.    M
*   Curve is refined equally spaced in parametric space, unless the curve is M
* linear in which the control polygon is simply being copied.		     M
*   If A is specified, it is used to refine the curve.			     M
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:              To approximate as a polyline.                          M
*   SamplesPerCurve:  Number of samples to approximate with.                 M
*   A:                Alpha matrix (Oslo algorithm) if precumputed.          M
*   OptiLin:          If TRUE, optimize linear curves.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A polyline representing the piecewise linear      M
*                          approximation from, or NULL in case of an error.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrv2Polyline, BspSrf2Polylines, IritCurve2Polylines,                  M
*   SymbCrv2Polyline						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspCrv2Polyline, piecewise linear approximation, polyline                M
*****************************************************************************/
CagdPolylineStruct *BspCrv2Polyline(const CagdCrvStruct *Crv,
				    int SamplesPerCurve,
				    BspKnotAlphaCoeffStruct *A,
				    CagdBType OptiLin)
{
    IRIT_STATIC_DATA CagdRType
	*Polyline[CAGD_MAX_PT_SIZE] = { NULL };
    IRIT_STATIC_DATA int
	PolylineLen = 0;
    CagdBType
	NewCrv = FALSE;
    int i, n,
	Order = Crv -> Order,
	Len = Crv -> Length,
	IsRational = CAGD_IS_RATIONAL_CRV(Crv),
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdRType
	*KV = Crv -> KnotVector;
    CagdPolylineStruct *PList;
    CagdCrvStruct *CpCrv;

    if (!CAGD_IS_BSPLINE_CRV(Crv))
	return NULL;

    if (CAGD_IS_PERIODIC_CRV(Crv)) {
        Crv = CpCrv = CagdCnvrtPeriodic2FloatCrv(Crv);
	Len += Order - 1;
	KV = Crv -> KnotVector;
    }
    else
        CpCrv = NULL;

    /* Make sure the curve is open. We move 2 Epsilons to make sure region  */
    /* extraction will occur. Otherwise the curve will be copied as is.     */
    if (!BspKnotHasOpenEC(KV, Len, Order)) {
	CagdCrvStruct
	    *TCrv = CagdCrvRegionFromCrv(Crv, KV[Order - 1], KV[Len]);

	if (CpCrv != NULL) {
	    CagdCrvFree(CpCrv);
	    CpCrv = NULL;
	}
	Crv = TCrv;
	NewCrv = TRUE;
    }

    /* Make sure requested format is something reasonable. */
    if (SamplesPerCurve < 2)
	SamplesPerCurve = 2;

    if (SamplesPerCurve <= Len || (Order == 2 && OptiLin)) {
	/* Make sure SamplesPerCurve can hold the entire control polygon. */
	SamplesPerCurve = Len;
    }

    n = IRIT_MAX(A ? A -> RefLength : 0, SamplesPerCurve);

    /* Allocate temporary memory to hold evaluated curve. */
    if (PolylineLen < n) {
	if (PolylineLen > 0) {
	    for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
		IritFree(Polyline[i]);
	}

	for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
	    Polyline[i] = (CagdRType *) IritMalloc(sizeof(CagdRType) * n);
	PolylineLen = n;
    }

    if (MaxCoord > 3)
	MaxCoord = 3;

    n = CagdCrvEvalToPolyline(Crv, A == NULL ? n : 0, Polyline, A, OptiLin);
    PList = CagdPtPolyline2E3Polyline(Polyline, n, MaxCoord, IsRational);

    if (CpCrv != NULL)
	CagdCrvFree(CpCrv);

    return PList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to approx. a single curve as a polyline with TolSamples          M
* samples/tolerance. Polyline is always E3 CagdPolylineStruct type.	     M
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:              To approximate as a polyline.                          M
*   SamplesPerCurve:  Number of samples to compute on polyline.		     M
*   OptiLin:          If TRUE, optimize linear curves.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A polyline representing the piecewise linear      M
*                          approximation from, or NULL in case of an error.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrCrv2Polyline, IritCurve2Polylines                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdCrv2Polyline, piecewise linear approximation, polyline               M
*****************************************************************************/
CagdPolylineStruct *CagdCrv2Polyline(const CagdCrvStruct *Crv,
				     int SamplesPerCurve,
				     CagdBType OptiLin)
{
    CagdCrvStruct *TCrv;
    CagdPolylineStruct *P;

    switch (Crv -> GType) {
	case CAGD_CBEZIER_TYPE:
	    return BzrCrv2Polyline(Crv, SamplesPerCurve);
	case CAGD_CBSPLINE_TYPE:
	    return BspCrv2Polyline(Crv, SamplesPerCurve,
				   NULL, OptiLin);
	case CAGD_CPOWER_TYPE:
	    TCrv = CagdCnvrtPwr2BzrCrv(Crv);
	    P = BzrCrv2Polyline(TCrv, SamplesPerCurve);
	    CagdCrvFree(TCrv);
	    return P;
	default:
	    CAGD_FATAL_ERROR(CAGD_ERR_UNDEF_CRV);
	    return NULL;
    }
}
