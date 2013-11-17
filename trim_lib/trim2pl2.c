/*****************************************************************************
* Trim2pl2.c - polygonal approximation of trimmed surfaces                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by Gershon Elber	       		      	         July 1998   *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "trim_loc.h"
#include "mdl_lib.h"
#include "ip_cnvrt.h"
#include "symb_lib.h"

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugTrim2Ply, FALSE);
#endif /* DEBUG */

#ifdef IRIT_DOUBLE
#define TRIM_FLT_PT_ROUND_OFF_ERR   1.050964e-13
#define TRIM_ON_GRID_BNDRY	    2.160988e-10
#define TRIM_ON_GRID_BNDRY2	    5.160988e-5
#define TRIM_DOMAIN_EPS_EXPAN	    1.30106e-7
#else
#define TRIM_FLT_PT_ROUND_OFF_ERR   1.050964e-6
#define TRIM_ON_GRID_BNDRY	    2.160988-5
#define TRIM_ON_GRID_BNDRY2	    1.160988-2
#define TRIM_DOMAIN_EPS_EXPAN	    1.30106e-3
#endif /* IRIT_DOUBLE */

#define TRIM_INTRR_PLLS_BNDRY_EPS	5e-5
#define TRIM_NUM_INTRR_VRTCS_IN_CELL	8
#define TRIM_INTRR_VRTCS_EPS		0.98
#define TRIM_FINENESS_SRF_CRV_RATIO	0.05  /* Tol. of trim crvs vs. srfs. */

#define INT_GREATER_THAN(v)	    ((int) ((v) + 1.0))
#define INT_LESS_THAN(v)	    ((v) == ((int) (v)) ? (int) ((v) - 1.0) \
							: (int) (v))
#define INT_TRUNC(v)	            (((int) (v)) - ((v) < 0.0 ? 1 : 0))
#define IS_IN_DOMAIN(X, XMin) ((X) >= (XMin) && (X) <= ((XMin) + 1))

#define	IS_SIMPLE_INSIDE_CELL(Cell) \
	((Cell) -> Plls == NULL && (Cell) -> Inside && !(Cell) -> OnBndry)

typedef struct Trim2PolyCellStruct {
    CagdRType Weight;    /* The original weight, if ration, to detect poles. */
    CagdRType *Pos;      /* Euclidean location of this point in three-space. */
    CagdRType *Nrml;   /* Unit normal of vetrex, if exists (NULL otherwise). */
    CagdRType *Uv;    /* UV parametric location, if exists (NULL otherwise). */
    CagdBType Inside;   /* TRUE for vertex inside srf, FALSE if trimmed out. */
    CagdBType OnBndry; /* If vertex is very close to trimming curve boundry. */
    CagdBType Processed;			   /* To tag processed cell. */
    IPPolygonStruct *Plls;           /* Trimming edges in cell goes to here. */
} Trim2PolyCellStruct;

typedef struct TrimBndrySortStruct {
    struct TrimBndrySortStruct *Pnext;
    CagdRType Val;     /* 0-1 left, 1-2 top, 2-3 right, 3-4 bottom boundary. */
    IPVertexStruct *V;     /* Pointer to beginning of linear trimming curve. */
    CagdBType Entering;  /* TRUE for boundary entering into this trim curve. */
    CagdBType StartOfCurve;/* TRUE if start of curve, FALSE if end of curve. */
} TrimBndrySortStruct;

IRIT_STATIC_DATA int
    TrimNumTrimVrtcsInCell = 2;

static CagdPolygonStruct *TrimSrf2Polygons2Aux(TrimSrfStruct *TrimSrf,
					       int FineNess,
					       CagdBType ComputeNormals,
					       CagdBType ComputeUV);
static CagdPolygonStruct *TrimC1Srf2Polygons(TrimSrfStruct *TrimSrf,
					     int FineNess,
					     CagdBType ComputeNormals,
					     CagdBType ComputeUV);
static void SplitLinearTrimmingCurves(IPVertexStruct *V,
				      Trim2PolyCellStruct **Grid,
				      int FineNessU,
				      int FineNessV);
static CagdPolygonStruct *CnvrtCellToPolygons1(TrimSrfStruct *TrimSrf,
					       Trim2PolyCellStruct **Grid,
					       int Iu,
					       int Iv,
					       int IuMax,
					       int IvMax);
static CagdPolygonStruct *CnvrtCellToPolygons2(TrimSrfStruct *TrimSrf,
					       Trim2PolyCellStruct **Grid,
					       int Iu,
					       int Iv,
					       int IuMax,
					       int IvMax);
static void ScanMeshForCoplanarity(Trim2PolyCellStruct **Grid,
				   int IuStart,
				   int IvStart,
				   int IuSize,
				   int IvSize,
				   int *IuEnd,
				   int *IvEnd);
static void CleanUpOnBndry(IPPolygonStruct *Plls,
			   Trim2PolyCellStruct *Cells[4],
			   int Iu,
			   int Iv);
static CagdBType CleanOneVrtxOnBndry(IPVertexStruct *VHead,
				     Trim2PolyCellStruct *Cells[4],
				     int Iu,
				     int Iv);
static void UpdateBndryCellCorners(CagdRType *UV,
				   Trim2PolyCellStruct *Cells[4],
				   CagdRType Eps,
				   int Iu,
				   int Iv);
static IPVertexStruct *ChainVertexLists(int Iu,
					int Iv,
					CagdRType Val1,
					IPVertexStruct *V1,
					CagdRType Val2,
					IPVertexStruct *V2);
static void AddCornerVertices(int Iu,
			      int Iv,
			      CagdRType Val1End,
			      IPVertexStruct *V1,
			      CagdRType Val2Strt);
static TrimBndrySortStruct *GetNextEntering(TrimBndrySortStruct *SInterList,
					    CagdRType CrntVal,
					    CagdRType BeginVal);
static TrimBndrySortStruct *FreeSInterFromSortedList(TrimBndrySortStruct *Item,
						     TrimBndrySortStruct *List);
static void CnvrtPlgsToCagdPlgs(IPVertexStruct *VList,
				Trim2PolyCellStruct *Cells[4],
				int Iu,
				int Iv);
static TrimBndrySortStruct *InsertSInterToList(TrimBndrySortStruct *Item,
					       TrimBndrySortStruct *List);
static CagdRType CnvrtToBndryIndex(IrtRType *Pt, int Iu, int Iv);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single trimmed surface to set of triangles	     M
* approximating it. FineNess is a fineness control on result and the larger  M
t is more triangles may result. A value of 10 is a good start value.	     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
*   This routine looks for C1 discontinuities in the surface and splits it   M
* into C1 continuous patches to invoke TrimC1Srf2Polygons to gen. polygons.  M
*                                                                            *
* PARAMETERS:                                                                M
*   CTrimSrf:         To approximate into triangles.                         M
*   FineNess:         Control on accuracy, the higher the finer.             M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2PolygonSetErrFunc, BzrSrf2Polygons, IritSurface2Polygons,	     M
*   IritTrimSrf2Polygons, CagdSrf2Polygons, BspSrf2Polygons,		     M
*   BspC1Srf2Polygons, TrimSetNumTrimVrtcsInCell 		             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSrf2Polygons2, polygonization, surface approximation                 M
*****************************************************************************/
CagdPolygonStruct *TrimSrf2Polygons2(const TrimSrfStruct *CTrimSrf,
				     int FineNess,
				     CagdBType ComputeNormals,
				     CagdBType ComputeUV) 
{
    CagdRType UMin, UMax, VMin, VMax, OldApproxTol;
    CagdPolygonStruct *Polys;
    SymbCrvApproxMethodType OldApproxMethod;
    CagdSrfStruct *Srf;
    TrimSrfStruct *TrimSrf;

    if (_CagdSrfMakeTriFunc == NULL || _CagdSrfMakeRectFunc == NULL) {
        _CagdSrfMakeTriFunc = CagdMakeTriangle;
        _CagdSrfMakeRectFunc = CagdMakeRectangle;
    }

    if (CAGD_IS_BSPLINE_SRF(CTrimSrf -> Srf))
	TrimSrf = TrimSrfNew(CagdCnvrtBsp2OpenSrf(CTrimSrf -> Srf),
			     TrimCrvCopyList(CTrimSrf -> TrimCrvList),
			     TRUE);
    else if (CAGD_IS_BEZIER_SRF(CTrimSrf -> Srf))
	TrimSrf = TrimSrfNew(CagdCnvrtBzr2BspSrf(CTrimSrf -> Srf),
			     TrimCrvCopyList(CTrimSrf -> TrimCrvList),
			     TRUE);
    else {
        TRIM_FATAL_ERROR(TRIM_ERR_BZR_BSP_EXPECT);
	return NULL;
    }

    Srf = TrimSrf -> Srf;

    /* Eliminate the cases of trivial trimming domain. */
    if (TrimSrfTrimCrvAllDomain(TrimSrf))
        return CagdSrf2Polygons(Srf, FineNess, ComputeNormals, FALSE,
				ComputeUV);
    else if (TrimSrfTrimCrvSquareDomain(TrimSrf -> TrimCrvList,
					&UMin, &UMax, &VMin, &VMax)) {
        CagdSrfStruct *TSrf;
        CagdRType SrfUMin, SrfUMax, SrfVMin, SrfVMax;

	CagdSrfDomain(Srf, &SrfUMin, &SrfUMax, &SrfVMin, &SrfVMax);

	if (!IRIT_APX_EQ(SrfUMin, UMin) || !IRIT_APX_EQ(SrfUMax, UMax))
	    TSrf = CagdSrfRegionFromSrf(Srf, UMin, UMax, CAGD_CONST_U_DIR);
	else
	    TSrf = CagdSrfCopy(Srf);

	if (!IRIT_APX_EQ(SrfVMin, VMin) || !IRIT_APX_EQ(SrfVMax, VMax)) {
	    Srf = CagdSrfRegionFromSrf(TSrf, VMin, VMax, CAGD_CONST_V_DIR);
	    CagdSrfFree(TSrf);
	}
	else
	    Srf = TSrf;

	Polys = CagdSrf2Polygons(Srf, FineNess, ComputeNormals, FALSE,
				ComputeUV);
	CagdSrfFree(Srf);
	TrimSrfFree(TrimSrf);
	return Polys;
    }

    if (CAGD_IS_BEZIER_SRF(Srf)) {
        Polys = TrimC1Srf2Polygons(TrimSrf, FineNess,
				   ComputeNormals, ComputeUV);
	TrimSrfFree(TrimSrf);
	return Polys;
    }
    else if (!CAGD_IS_BSPLINE_SRF(Srf)) {
	TrimSrfFree(TrimSrf);
        TRIM_FATAL_ERROR(TRIM_ERR_BZR_BSP_EXPECT);
	return NULL;
    }

    OldApproxMethod = _TrimUVCrvApproxMethod;
    OldApproxTol = _TrimUVCrvApproxTolSamples;
    TrimSetTrimCrvLinearApprox(IRIT_MIN(TRIM_FINENESS_SRF_CRV_RATIO / FineNess,
				   0.01),
			       SYMB_CRV_APPROX_TOLERANCE);

    /* Process all trimming curves into piecewise linear if not already. */
    TrimPiecewiseLinearTrimmingCurves(TrimSrf, FALSE);

    Polys = TrimSrf2Polygons2Aux(TrimSrf, FineNess, ComputeNormals, ComputeUV);

    TrimSetTrimCrvLinearApprox(OldApproxTol, OldApproxMethod);

    TrimSrfFree(TrimSrf);

    return Polys;

}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Auxiliary function of TrimSrf2Polygons2				     *
*****************************************************************************/
static CagdPolygonStruct *TrimSrf2Polygons2Aux(TrimSrfStruct *TrimSrf,
					       int FineNess,
					       CagdBType ComputeNormals,
					       CagdBType ComputeUV) 
{
    CagdBType HasUDiscont, HasVDiscont;
    CagdRType u, v;
    CagdPolygonStruct *Poly;
    CagdSrfStruct
        *Srf = TrimSrf -> Srf;

    HasUDiscont = BspSrfKnotC1Discont(Srf, CAGD_CONST_U_DIR, &u);
    HasVDiscont = BspSrfKnotC1Discont(Srf, CAGD_CONST_V_DIR, &v);

    if (HasUDiscont || HasVDiscont) {
	TrimSrfStruct
	    *TrimSrf1 = HasUDiscont ? TrimSrfSubdivAtParam(TrimSrf, u,
							   CAGD_CONST_U_DIR)
				    : TrimSrfSubdivAtParam(TrimSrf, v,
							   CAGD_CONST_V_DIR),
	    *TrimSrf2 = TrimSrf1 -> Pnext;
	CagdPolygonStruct *Poly1, *Poly2;

	if (TrimSrf1 -> TrimCrvList != NULL) {
	    /* Trimming curves in a low resolution could vanish. */
	    TrimSrf1 -> Attr = IP_ATTR_COPY_ATTRS(TrimSrf -> Attr);
	    Poly1 = TrimSrf2Polygons2Aux(TrimSrf1, FineNess,
					 ComputeNormals, ComputeUV);
	}
	else
	    Poly1 = NULL;

	if (TrimSrf2 != NULL) {
	    TrimSrf2 -> Attr = IP_ATTR_COPY_ATTRS(TrimSrf -> Attr);

	    Poly2 = TrimSrf2Polygons2Aux(TrimSrf2, FineNess,
					 ComputeNormals, ComputeUV);
	}
	else
	    Poly2 = NULL;

	TrimSrfFreeList(TrimSrf1);

	/* Chain the two lists together: */
	Poly = (CagdPolygonStruct *) CagdListAppend(Poly1, Poly2);
    }
    else {
        Poly = TrimC1Srf2Polygons(TrimSrf, FineNess,
				  ComputeNormals, ComputeUV);
    }

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the way trimming curves contributes to each cell in the domain      M
* by setting the number of vertices trimming curves can contribute to each   M
* such cell.                                            		     M
*                                                                            *
* PARAMETERS:                                                                M
*   NumTrimVrtcsInCell:   Number of requested trimming vertices in a cell.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value of way of num of cells.			             M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrimSetNumTrimVrtcsInCell                                                M
*****************************************************************************/
int TrimSetNumTrimVrtcsInCell(int NumTrimVrtcsInCell)
{
    int OldNumTrimVrtcsInCell = TrimNumTrimVrtcsInCell;

    TrimNumTrimVrtcsInCell = NumTrimVrtcsInCell;

    return OldNumTrimVrtcsInCell;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of TrimSrf2Polygons that receives a C1 continuous     *
* surface and uniformly sample it, trim it according to trimming curves, and *
* generate the triangles by splitting the trimming curves into the cells of  *
* the grid and triangulating the cells.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:           A C1 continuous surface to convert to triangles.      *
*   FineNess:          Fineness of approximation. 10 is a good start.        *
*   ComputeNormals:    TRUE if normals are required.                         *
*   ComputeUV:         TRUE if UV values are required.                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:   List of triangles approximating TrimSrf.          *
*****************************************************************************/
static CagdPolygonStruct *TrimC1Srf2Polygons(TrimSrfStruct *TrimSrf,
					     int FineNess,
					     CagdBType ComputeNormals,
					     CagdBType ComputeUV)
{
    CagdBType
	OldCircVLists = IPSetPolyListCirc(TRUE),
	OldHandleNormals = GMConvexPolyNormals(FALSE);
    int i, j, FineNessU, FineNessV, FineNessU1, FineNessV1, MeshSize,
	IsRational;
    CagdRType UScale, VScale, UMin, UMax, VMin, VMax, *VIsoVals, *VIsoValsAux,
	v, dv, *PtWeights, *PtWeightsPtr;
    CagdPtStruct *PtMesh, *PtMeshPtr;
    CagdVecStruct *PtNrml, *PtNrmlPtr;
    CagdUVStruct *UVMesh, *UVMeshPtr;
    CagdPolylineStruct *TrimCagdPolys, *TCagdPoly;
    IPPolygonStruct *TrimPolys, *TPoly;
    TrimIsoInterStruct **IsoInters, *IsoInter;
    Trim2PolyCellStruct **Grid;
    CagdSrfStruct
	*Srf = TrimSrf -> Srf;
    CagdPolygonStruct
	*CagdPolygons = NULL;

    IsRational = CAGD_IS_RATIONAL_SRF(Srf);

    /* Copy resolution attributes if were any. */
    Srf -> Attr = IP_ATTR_COPY_ATTRS(TrimSrf -> Attr);

    /* Sample the surface at the requested fineness: */
    if (CAGD_IS_BEZIER_SRF(Srf)) {
	/* Make sure we have at least 3 samples per size. */
	i = IRIT_MIN(Srf -> UOrder, Srf -> VOrder);
	if (i * FineNess / 10 < 3)
	    FineNess = 1 + 30 / i;

        if (!BzrSrf2PolygonsSamples(Srf, FineNess, ComputeNormals, ComputeUV,
				    &PtWeights, &PtMesh, &PtNrml, &UVMesh,
				    &FineNessU, &FineNessV)) {
	    IPSetPolyListCirc(OldCircVLists);
	    GMConvexPolyNormals(OldHandleNormals);
	    return NULL;
	}
	Srf = CagdCnvrtBzr2BspSrf(Srf);
    }
    else {				       /* Must be a Bspline surface. */
	/* Make sure we have at least 3 samples per size. */
	i = IRIT_MIN(Srf -> ULength, Srf -> VLength);
	if (i * FineNess / 10 < 3)
	    FineNess = 1 + 30 / i;

        if (!BspC1Srf2PolygonsSamples(Srf, FineNess, ComputeNormals,
				      ComputeUV, &PtWeights, &PtMesh, &PtNrml,
				      &UVMesh, &FineNessU, &FineNessV)) {
	    IPSetPolyListCirc(OldCircVLists);
	    GMConvexPolyNormals(OldHandleNormals);
	    return NULL;
	}
	Srf = CagdSrfCopy(Srf);
    }
    MeshSize = FineNessU * FineNessV;
    FineNessU1 = FineNessU - 1;
    FineNessV1 = FineNessV - 1;

    /* Approximate the trimming curves using a piecewise linear              */
    /* approximation and map the domain to be between zero and fineness.     */
    TrimCagdPolys = TrimCrvs2Polylines(TrimSrf, TRUE,
				       _TrimUVCrvApproxTolSamples,
				       _TrimUVCrvApproxMethod);
    TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);
    UScale = (TRIM_DOMAIN_EPS_EXPAN * 2 + FineNessU1) / (UMax - UMin);
    VScale = (TRIM_DOMAIN_EPS_EXPAN * 2 + FineNessV1) / (VMax - VMin);

    for (TCagdPoly = TrimCagdPolys;
	 TCagdPoly != NULL;
	 TCagdPoly = TCagdPoly -> Pnext) {
	CagdPolylnStruct
	    *Pl = TCagdPoly -> Polyline;

	for (i = TCagdPoly -> Length - 1; i >= 0; i--) {
	    Pl[i].Pt[0] =
	            (Pl[i].Pt[0] - UMin) * UScale - TRIM_DOMAIN_EPS_EXPAN;
	    Pl[i].Pt[1] =
	            (Pl[i].Pt[1] - VMin) * VScale - TRIM_DOMAIN_EPS_EXPAN;
	}
    }
    TrimPolys = IPCagdPllns2IritPllns(TrimCagdPolys);

    /* Build the intersections of the lines of the grid with the trimming   */
    /* curves so we can decide if each node in the grid is inside or not.   */
    VIsoValsAux = (CagdRType *) IritMalloc((2 + FineNessV) * sizeof(CagdRType));
    VIsoValsAux[0] = VMin - TRIM_ISO_PARAM_PERTURB * 0.5;
    VIsoVals = &VIsoValsAux[1];
    dv = (VMax - VMin) / (FineNessV - 1);
    for (j = 0, v = VMin; j < FineNessV; j++, v += dv) {
        VIsoVals[j] = v - 2 * TRIM_ISO_PARAM_PERTURB
	    * (j / ((CagdRType) (FineNessV - 1)) - 0.51);
    }
    VIsoVals[FineNessV] = VMax + TRIM_ISO_PARAM_PERTURB * 0.5;
    IsoInters = TrimIntersectTrimCrvIsoVals(TrimSrf, CAGD_CONST_V_DIR,
					    VIsoValsAux, 2 + FineNessV, FALSE);
    IritFree(VIsoValsAux);

    for (j = 0; j < FineNessV; j++) {
        IsoInters[j] = IsoInters[j + 1];        /* Skip boundary conditions. */
        for (IsoInter = IsoInters[j];
	     IsoInter != NULL;
	     IsoInter = IsoInter -> Pnext) {
	    /* Update intersections to scale of (-Eps, ..., FineNessU+Eps). */
	    IsoInter -> Param =
	        (IsoInter -> Param - UMin - TRIM_ISO_PARAM_PERTURB) * UScale;
	}
    }

    /* Time to build the grid and split the trimming curves into cells. */
    Grid = (Trim2PolyCellStruct **)
        IritMalloc(sizeof(Trim2PolyCellStruct *) * (FineNessV + 1));
    for (j = 0; j <= FineNessV; j++)
        Grid[j] = IritMalloc(sizeof(Trim2PolyCellStruct) * (FineNessU + 1));

    /* Set inside/outside relations for all grid points. */
    for (j = 0; j < FineNessV; j++) {
	Trim2PolyCellStruct
	    *Cell = Grid[j];
	CagdBType
	    Inside = FALSE;
	TrimIsoInterStruct
	    *LastIsoInter = IsoInters[j];

	IsoInter = LastIsoInter;

	for (i = 0; i < FineNessU; i++, Cell++) {
	    if (IsoInter != NULL) {
	        while (IsoInter -> Param < i) {
		    /* Toggle inside/outside. */
		    Inside = !Inside;
		    LastIsoInter = IsoInter;
		    if ((IsoInter = IsoInter -> Pnext) == NULL) {
		        Inside = FALSE;
			break;
		    }
		}
	    }
	    else
	        Inside = FALSE;

	    Cell -> Inside = Inside;
	    Cell -> OnBndry = (LastIsoInter != NULL &&
			       IRIT_APX_EQ(LastIsoInter -> Param, i)) ||
	      		      (IsoInter != NULL &&
			       (IRIT_APX_EQ(IsoInter -> Param, i) ||
				(IsoInter -> Pnext != NULL &&
				 IRIT_APX_EQ(IsoInter -> Pnext -> Param, i))));
	    Cell -> Plls = NULL;
	}

	/* Free the intersection list. */
	while (IsoInters[j] != NULL) {
	    IsoInter = IsoInters[j];
	    IsoInters[j] = IsoInters[j] -> Pnext;
	        IritFree(IsoInter);
	}
    }
    IritFree(IsoInters);

    /* Update position and optionally normals/UV locations. */
    PtWeightsPtr = PtWeights;
    PtMeshPtr = PtMesh;
    PtNrmlPtr = PtNrml;
    UVMeshPtr = UVMesh;
    for (i = 0; i < FineNessU; i++) {
        for (j = 0; j < FineNessV; j++) {
	    Trim2PolyCellStruct
	         *Cell = &Grid[j][i];

	    Cell -> Processed = FALSE;

	    Cell -> Pos = (PtMeshPtr++) -> Pt;
	    if (IsRational)
		Cell -> Weight = *PtWeightsPtr++;

	    if (PtNrmlPtr != NULL)
	        Cell -> Nrml = (PtNrmlPtr++) -> Vec;
	    else
	        Cell -> Nrml = NULL;

	    if (UVMeshPtr)
	        Cell -> Uv = (UVMeshPtr++) -> UV;
	    else
	        Cell -> Uv = NULL;
	}
    }

    /* Go over all the trimming curves and destructively split them into the */
    /* different cells.							     */
    for (TPoly = TrimPolys; TPoly != NULL; TPoly = TPoly -> Pnext) {
	SplitLinearTrimmingCurves(TPoly -> PVertex, Grid,
				  FineNessU, FineNessV);
	TPoly -> PVertex = NULL;
    }
    IPFreePolygonList(TrimPolys);

    /* Go over all cells and form polygons out of them. */
#   ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTrimPrintGrid, FALSE) {
	    for (i = 0; i < FineNessU; i++) {
	        for (j = 0; j < FineNessV; j++) {
		    Trim2PolyCellStruct
		        *Cell = &Grid[j][i];

		    if (Cell -> OnBndry)
		        IRIT_INFO_MSG("B");
		    else if (Cell -> Inside)
		        IRIT_INFO_MSG("I");
		    else
		        IRIT_INFO_MSG("O");
		}
		IRIT_INFO_MSG("\n");
	    }
	}
    }
#   endif /* DEBUG */

    for (j = 0; j < FineNessV1; j++) {
	for (i = 0; i < FineNessU1; i++) {
	    if (!Grid[j][i].Processed) {
		CagdPolygonStruct
		    *CagdPolys = CnvrtCellToPolygons1(TrimSrf, Grid, i, j,
						      FineNessU1 - 1,
						      FineNessV1 - 1);

		CagdPolygons = CagdListAppend(CagdPolys, CagdPolygons);
	    }
	}
    }

    for (j = 0; j <= FineNessV; j++)
	IritFree(Grid[j]);
    IritFree(Grid);

    if (PtWeights != NULL)
        IritFree(PtWeights);
    CagdPtArrayFree(PtMesh, MeshSize);
    if (PtNrml != NULL)
	CagdVecArrayFree(PtNrml, MeshSize);
    if (UVMesh != NULL)
	CagdUVArrayFree(UVMesh, MeshSize);

    CagdSrfFree(Srf);

    IPSetPolyListCirc(OldCircVLists);
    GMConvexPolyNormals(OldHandleNormals);

    return CagdPolygons;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Splits the given piecewise linear trimming curve into the different      *
* cells.                                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:         Piecewise linear trimming curve.                              *
*   Grid:      of cells of the sampling of the trimming surface.             *
*   FineNessU: U size of grid.						     *
*   FineNessV: V size of grid.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void.                                                                    *
*****************************************************************************/
static void SplitLinearTrimmingCurves(IPVertexStruct *V,
				      Trim2PolyCellStruct **Grid,
				      int FineNessU,
				      int FineNessV)
{
    int Iu, Iv;
    CagdRType u, v;
    IPVertexStruct
	*VHead = V;

    if (V -> Pnext == NULL)
        return;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
	IRIT_INFO_MSG_PRINTF("FINENESS = %d %d\n", FineNessU, FineNessV);
#   endif /* DEBUG */

    /* Make sure no vertex is directly on the grid, for numerical stability. */
    for  ( ; V != NULL; V = V -> Pnext) {
        int i;

	for (i = 0; i < 2; i++) {
	    int IVal = (int) (V -> Coord[i] + TRIM_FLT_PT_ROUND_OFF_ERR * 2);
		
	    if (IRIT_APX_EQ_EPS(V -> Coord[i], IVal, TRIM_FLT_PT_ROUND_OFF_ERR * 4))
	        V -> Coord[i] = IVal + TRIM_FLT_PT_ROUND_OFF_ERR * 2;
	}
    }

    /* Make sure no vertex is outside the domain, for numerical stability. */
    for  (V = VHead; V != NULL; V = V -> Pnext) {
	if (V -> Coord[0] < -TRIM_DOMAIN_EPS_EXPAN)
	    V -> Coord[0] = -TRIM_DOMAIN_EPS_EXPAN;
	if (V -> Coord[0] > FineNessU - 1 + TRIM_DOMAIN_EPS_EXPAN)
	    V -> Coord[0] = FineNessU - 1 + TRIM_DOMAIN_EPS_EXPAN;

	if (V -> Coord[1] < -TRIM_DOMAIN_EPS_EXPAN)
	    V -> Coord[1] = -TRIM_DOMAIN_EPS_EXPAN;
	if (V -> Coord[1] > FineNessV - 1 + TRIM_DOMAIN_EPS_EXPAN)
	    V -> Coord[1] = FineNessV - 1 + TRIM_DOMAIN_EPS_EXPAN;
    }

    V = VHead;
    while (V != NULL && V -> Pnext != NULL) {
        IPVertexStruct
	    *VMid = NULL,
	    *VNext = V -> Pnext;
	int Val, Axis,
	    ThisVrtxU = INT_TRUNC(V -> Coord[0]),
	    ThisVrtxV = INT_TRUNC(V -> Coord[1]),
	    NextVrtxU = INT_TRUNC(VNext -> Coord[0]),
	    NextVrtxV = INT_TRUNC(VNext -> Coord[1]),
	    MinU = IRIT_MIN(ThisVrtxU, NextVrtxU),
	    MinV = IRIT_MIN(ThisVrtxV, NextVrtxV),
	    SameU = IS_IN_DOMAIN(V -> Coord[0], MinU) &&
		    IS_IN_DOMAIN(VNext -> Coord[0], MinU),
	    SameV = IS_IN_DOMAIN(V -> Coord[1], MinV) &&
		    IS_IN_DOMAIN(VNext -> Coord[1], MinV);

	if (!SameU || !SameV) {
	    int i;
	    CagdRType t,
		t1 = IRIT_INFNTY,
		t2 = IRIT_INFNTY;
	    IPVertexStruct
		*VMid2 = IPAllocVertex2(NULL);

	    VMid = IPAllocVertex2(VNext);

	    /* We cross a boundary of cell here. Split trimming curve here. */
	    if (!SameU) {
	        if (VNext -> Coord[0] > V -> Coord[0]) {
		    t1 = INT_GREATER_THAN(V -> Coord[0]);
		    t1 = (t1 - V -> Coord[0]) /
		                           (VNext -> Coord[0] - V -> Coord[0]);
		}
		else {
		    t1 = INT_LESS_THAN(V -> Coord[0]);
		    t1 = (V -> Coord[0] - t1) /
		    		           (V -> Coord[0] - VNext -> Coord[0]);
		}
		Axis = 0;
		Val = IRIT_MAX(ThisVrtxU, NextVrtxU);
	    }
	    else {
	        assert(!SameV);

	        if (VNext -> Coord[1] > V -> Coord[1]) {
		    t2 = INT_GREATER_THAN(V -> Coord[1]);
		    t2 = (t2 - V -> Coord[1]) /
		                           (VNext -> Coord[1] - V -> Coord[1]);
		}
		else {
		    t2 = INT_LESS_THAN(V -> Coord[1]);
		    t2 = (V -> Coord[1] - t2) /
		                           (V -> Coord[1] - VNext -> Coord[1]);
		}
		Axis = 1;
		Val = IRIT_MAX(ThisVrtxV, NextVrtxV);
	    }

	    /* Blend coordinates amd make sure we are properly on boundary. */
	    if ((t = IRIT_MIN(t1, t2)) == IRIT_INFNTY) {
	        V = V -> Pnext;
		continue;
	    }

	    /* Compute intersection point and make sure it is on the edge. */
	    IRIT_UV_BLEND(VMid -> Coord, VNext -> Coord, V -> Coord, t);
	    VMid -> Coord[2] = 0.0;
	    if (IRIT_APX_EQ_EPS(VMid -> Coord[Axis], Val,
				TRIM_FLT_PT_ROUND_OFF_ERR))
	        VMid -> Coord[Axis] = Val;
	    for (i = 0; i < 2; i++) {
	        if ((int) (VMid -> Coord[i]) !=
		    (int) (VMid -> Coord[i] + TRIM_FLT_PT_ROUND_OFF_ERR))
		    VMid -> Coord[i] = (int) (VMid -> Coord[i] +
					      TRIM_FLT_PT_ROUND_OFF_ERR);
	        else if ((int) (VMid -> Coord[i]) !=
			 (int) (VMid -> Coord[i] - TRIM_FLT_PT_ROUND_OFF_ERR))
		    VMid -> Coord[i] = (int) (VMid -> Coord[i]);
	    }

	    IRIT_UV_COPY(VMid2 -> Coord, VMid -> Coord);
	    V -> Pnext = VMid2;

	    /* Insert the segment of the trimming curve from VHead to VMid2. */
	    if (V -> Pnext -> Pnext == NULL) { /* Two vrtcs in segments. */
	        u = (V -> Coord[0] + V -> Pnext -> Coord[0]) * 0.5;
		v = (V -> Coord[1] + V -> Pnext -> Coord[1]) * 0.5;
	    }
	    else {     /* More than two vertices.  Use some interior vertex. */
		u = V -> Pnext -> Coord[0];
		v = V -> Pnext -> Coord[1];
	    }
	    Iu = INT_TRUNC(u);
	    Iv = INT_TRUNC(v);

#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
		if (Iu >= 0 && Iu < FineNessU - 1 &&
		    Iv >= 0 && Iv < FineNessV - 1) {
		    IRIT_INFO_MSG_PRINTF("Add (1) trim segments at %d %d\n",
					 Iu, Iv);
		    IPStderrObject(IPGenPOLYLINEObject(IPAllocPolygon(0, VHead,
								      NULL)));
		}
#	    endif /* DEBUG */

	    if (Iu >= 0 && Iu < FineNessU - 1 && Iv >= 0 && Iv < FineNessV - 1)
		Grid[Iv][Iu].Plls = IPAllocPolygon(0, VHead,
						   Grid[Iv][Iu].Plls);
	    else
	        IPFreeVertexList(VHead);
	    VHead = V = VMid;
	}
	else
	    V = V -> Pnext;
    }

    /* Insert the last segment of the trimming curve from VHead. */
    if (VHead -> Pnext -> Pnext == NULL) { /* Two vrtcs in segments. */
	u = (VHead -> Coord[0] + VHead -> Pnext -> Coord[0]) * 0.5;
	v = (VHead -> Coord[1] + VHead -> Pnext -> Coord[1]) * 0.5;
    }
    else {	       /* More than two vertices.  Use some interior vertex. */
	u = VHead -> Pnext -> Coord[0];
	v = VHead -> Pnext -> Coord[1];
    }
    Iu = INT_TRUNC(u);
    Iv = INT_TRUNC(v);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
	if (Iu >= 0 && Iu < FineNessU - 1 && Iv >= 0 && Iv < FineNessV - 1) {
	    IRIT_INFO_MSG_PRINTF("Add (2) trim segments at %d %d\n",
				 Iu, Iv);
	    IPStderrObject(IPGenPOLYLINEObject(IPAllocPolygon(0, VHead,
							      NULL)));
	}
#   endif /* DEBUG */

    if (Iu >= 0 && Iu < FineNessU - 1 && Iv >= 0 && Iv < FineNessV - 1)
	Grid[Iv][Iu].Plls = IPAllocPolygon(0, VHead, Grid[Iv][Iu].Plls);
    else
	IPFreeVertexList(VHead);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process one cell of the sampling grid into polygons.  This function      *
* scans for cells with no trimming curves and merges neigboring cells, if    *
* possible.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:   processed trimeed surface.                                    *
*   Grid:          To process.                                               *
*   Iu, Iv:        Indices of cell in grid.                                  *
*   IuMax, IvMax:  Maximal values of cell indices in grid.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:   List of polygons generated from the given cell    *
*                          or NULL if none.                                  *
*****************************************************************************/
static CagdPolygonStruct *CnvrtCellToPolygons1(TrimSrfStruct *TrimSrf,
					       Trim2PolyCellStruct **Grid,
					       int Iu,
					       int Iv,
					       int IuMax,
					       int IvMax)
{
    int IuEnd, IvEnd, GenPoly;
    CagdPolygonStruct *Plg, *Plg2;
    Trim2PolyCellStruct *Cell[4];

    Cell[0] = &Grid[Iv][Iu];
    Cell[1] = &Grid[Iv + 1][Iu];
    Cell[2] = &Grid[Iv + 1][Iu + 1];
    Cell[3] = &Grid[Iv][Iu + 1];

    if (IS_SIMPLE_INSIDE_CELL(Cell[0]) &&
	GMCoplanar4Pts(Cell[0] -> Pos, Cell[1] -> Pos,
		       Cell[2] -> Pos, Cell[3] -> Pos)) {

	ScanMeshForCoplanarity(Grid, Iu, Iv, IuMax, IvMax, &IuEnd, &IvEnd);

	if (IuEnd == Iu && IvEnd == Iv) {
	    /* Only this one is coplanar. */
	    Cell[0] -> Processed = TRUE;
	}
	else {
	    int i, j;

	    Cell[1] = &Grid[IvEnd + 1][Iu];
	    Cell[2] = &Grid[IvEnd + 1][IuEnd + 1];
	    Cell[3] = &Grid[Iv][IuEnd + 1];

	    /* Mark all these cells as covered. */
	    for (i = Iu; i <= IuEnd; i++)
	        for (j = Iv; j <= IvEnd; j++)
		    Grid[j][i].Processed = TRUE;
	}

	/* Create one planar polygon for all four cells. */
	Plg = _CagdSrfMakeRectFunc(Cell[0] -> Nrml != NULL,
				   Cell[0] -> Uv != NULL,
				   Cell[0] -> Pos,
				   Cell[1] -> Pos,
				   Cell[2] -> Pos,
				   Cell[3] -> Pos,
				   Cell[0] -> Nrml,
				   Cell[1] -> Nrml,
				   Cell[2] -> Nrml,
				   Cell[3] -> Nrml,
				   Cell[0] -> Uv,
				   Cell[1] -> Uv,
				   Cell[2] -> Uv,
				   Cell[3] -> Uv,
				   &GenPoly);
	if (GenPoly)
	    return Plg;

	Plg = _CagdSrfMakeTriFunc(Cell[0] -> Nrml != NULL,
				  Cell[0] -> Uv != NULL,
				  Cell[0] -> Pos,
				  Cell[1] -> Pos,
				  Cell[2] -> Pos,
				  Cell[0] -> Nrml,
				  Cell[1] -> Nrml,
				  Cell[2] -> Nrml,
				  Cell[0] -> Uv,
				  Cell[1] -> Uv,
				  Cell[2] -> Uv,
				  &GenPoly);
	Plg2 = _CagdSrfMakeTriFunc(Cell[0] -> Nrml != NULL,
				   Cell[0] -> Uv != NULL,
				   Cell[0] -> Pos,
				   Cell[2] -> Pos,
				   Cell[3] -> Pos,
				   Cell[0] -> Nrml,
				   Cell[2] -> Nrml,
				   Cell[3] -> Nrml,
				   Cell[0] -> Uv,
				   Cell[2] -> Uv,
				   Cell[3] -> Uv,
				   &GenPoly);
	if (Plg != NULL)
	    Plg -> Pnext = Plg2;
	else
	    Plg = Plg2;

	return Plg;
    }

    /* Default to a non coplanar cell... */
    return CnvrtCellToPolygons2(TrimSrf, Grid, Iu, Iv, IuMax, IvMax);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Process one cell of the sampling grid into polygons.  This cell, if has  *
* no trimming curves, is neither planar nor merged with neigboring cells.    *
*                                                                            *
* PARAMETERS:                                                                *
*   TrimSrf:   processed trimeed surface.                                    *
*   Grid:      To process.                                                   *
*   Iu, Iv:    Indices of cell in grid.                                      *
*   IuMax, IvMax:  Maximal values of cell indices in grid.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:   List of polygons generated from the given cell    *
*                          or NULL if none.                                  *
*****************************************************************************/
static CagdPolygonStruct *CnvrtCellToPolygons2(TrimSrfStruct *TrimSrf,
					       Trim2PolyCellStruct **Grid,
					       int Iu,
					       int Iv,
					       int IuMax,
					       int IvMax)
{
    CagdBType
	HasInteriorTrimCrv = FALSE;
    IPPolygonStruct *Pll,
	*Plls = GMMergePolylines(Grid[Iv][Iu].Plls, IRIT_EPS);
    TrimBndrySortStruct *SInter,
	*SortedInterList = NULL;
    CagdPolygonStruct *Plg,
	*Plgs = NULL;
    Trim2PolyCellStruct *Cell[4];

    Cell[0] = &Grid[Iv][Iu];
    Cell[1] = &Grid[Iv + 1][Iu];
    Cell[2] = &Grid[Iv + 1][Iu + 1];
    Cell[3] = &Grid[Iv][Iu + 1];

    /* Clean edges on boundary of cell, and zero length edges. */
    CleanUpOnBndry(Plls, Cell, Iu, Iv);
    GMCleanUpPolylineList(&Plls, TRIM_DOMAIN_EPS_EXPAN);

    if (TrimNumTrimVrtcsInCell) {
        for (Pll = Plls; Pll != NULL; Pll = Pll -> Pnext) {
	    Pll -> PVertex = GMFilterInteriorVertices(Pll -> PVertex,
					       TRIM_INTRR_VRTCS_EPS,
					       TRIM_NUM_INTRR_VRTCS_IN_CELL);
	}
    }

    /* Find out where are the end points of the trimming polylines. */
    for (Pll = Plls; Pll != NULL; Pll = Pll -> Pnext) {
        /* Insert the starting point of the trimming curve. */
	SInter = (TrimBndrySortStruct *)
	    IritMalloc(sizeof(TrimBndrySortStruct));

	/* Could be that this entire trimming curve is inside the domain and */
	/* in such a case simply ignore this trimming curve.		     */
	if ((SInter -> Val = CnvrtToBndryIndex(Pll -> PVertex -> Coord,
					       Iu, Iv)) >= 0.0) {
	    SInter -> V = Pll -> PVertex;
	    SInter -> StartOfCurve = TRUE;
	    SortedInterList = InsertSInterToList(SInter, SortedInterList);

	    /* Insert the termination point of the trimming curve. */
	    SInter = (TrimBndrySortStruct *)
		IritMalloc(sizeof(TrimBndrySortStruct));

	    /* If starting point is on the boundary, end point must be also. */
	    if ((SInter -> Val =
		 CnvrtToBndryIndex(IPGetLastVrtx(Pll -> PVertex) -> Coord,
				   Iu, Iv)) < 0.0) {
	        /* This is an error: we have a trimming curve that starts on */
	        /* boundary but terminates inside - handle this gracefully.  */
	        IritFree(SInter);
		HasInteriorTrimCrv = TRUE;
		while (SortedInterList != NULL)
		    SortedInterList = FreeSInterFromSortedList(SortedInterList,
							       SortedInterList);
		break;
	    }
	    else {
	        SInter -> V = Pll -> PVertex;
		SInter -> StartOfCurve = FALSE;
		SortedInterList = InsertSInterToList(SInter, SortedInterList);
	    }
	}
	else {
	    /* We have a completely interior trimming curve in this cell.    */
	    /* We put out the entire cell in this case.			     */
	    IritFree(SInter);
	    HasInteriorTrimCrv = TRUE;
	    while (SortedInterList != NULL)
		SortedInterList = FreeSInterFromSortedList(SortedInterList,
							   SortedInterList);
	    break;
	}

	Pll -> PVertex = NULL;
    }
    IPFreePolygonList(Plls);

    if (SortedInterList == NULL) {        /* No trimming curve in this cell. */
        CagdBType CornersOnBndryYetInside;

        if (Cell[0] -> OnBndry &&
	    Cell[1] -> OnBndry &&
	    Cell[2] -> OnBndry &&
	    Cell[3] -> OnBndry) {
	    CagdRType UMin, UMax, VMin, VMax;
	    CagdUVType UV;

	    TrimSrfDomain(TrimSrf, &UMin, &UMax, &VMin, &VMax);
	    UV[0] = UMin + (UMax - UMin) * (Iu + 0.5) / (IuMax + 1);
	    UV[1] = VMin + (VMax - VMin) * (Iv + 0.5) / (IvMax + 1);
	    CornersOnBndryYetInside = TrimIsPointInsideTrimSrf(TrimSrf, UV);
	}
	else
	    CornersOnBndryYetInside = FALSE;

	if (HasInteriorTrimCrv ||
	    CornersOnBndryYetInside ||
	    ((Cell[0] -> Inside && !Cell[0] -> OnBndry) ||
	     (Cell[1] -> Inside && !Cell[1] -> OnBndry) ||
	     (Cell[2] -> Inside && !Cell[2] -> OnBndry) ||
	     (Cell[3] -> Inside && !Cell[3] -> OnBndry))) {
	    int GenPoly = FALSE;

	    /* The cell is part of the valid domain of the trimmed surface. */
	    if (GMCoplanar4Pts(Cell[0] -> Pos, Cell[1] -> Pos,
			       Cell[2] -> Pos, Cell[3] -> Pos)) {
		Plgs = _CagdSrfMakeRectFunc(Cell[2] -> Nrml != NULL,
					    Cell[2] -> Uv != NULL,
					    Cell[0] -> Pos,
					    Cell[1] -> Pos,
					    Cell[2] -> Pos,
					    Cell[3] -> Pos,
					    Cell[0] -> Nrml,
					    Cell[1] -> Nrml,
					    Cell[2] -> Nrml,
					    Cell[3] -> Nrml,
					    Cell[0] -> Uv,
					    Cell[1] -> Uv,
					    Cell[2] -> Uv,
					    Cell[3] -> Uv,
					    &GenPoly);
	    }

	    /* If the Rect generator above failed for some reason. */
	    if (!GenPoly) {
		Plg = _CagdSrfMakeTriFunc(Cell[0] -> Nrml != NULL,
					  Cell[0] -> Uv != NULL,
					  Cell[0] -> Pos,
					  Cell[1] -> Pos,
					  Cell[2] -> Pos,
					  Cell[0] -> Nrml,
					  Cell[1] -> Nrml,
					  Cell[2] -> Nrml,
					  Cell[0] -> Uv,
					  Cell[1] -> Uv,
					  Cell[2] -> Uv,
					  &GenPoly);
		Plgs = _CagdSrfMakeTriFunc(Cell[2] -> Nrml != NULL,
					   Cell[2] -> Uv != NULL,
					   Cell[2] -> Pos,
					   Cell[3] -> Pos,
					   Cell[0] -> Pos,
					   Cell[2] -> Nrml,
					   Cell[3] -> Nrml,
					   Cell[0] -> Nrml,
					   Cell[2] -> Uv,
					   Cell[3] -> Uv,
					   Cell[0] -> Uv,
					   &GenPoly);
		if (Plgs == NULL)
		    Plgs = Plg;
		else
		    Plgs -> Pnext = Plg;
	    }
	}
    }
    else {
	IPVertexStruct
	    *VHead = NULL;
	CagdBType
	    Inside = Cell[0] -> Inside;
	CagdRType
	    BeginVal = 0.0,
	    CrntVal = 0.0;

	/* Mark all end points of the trimming curves as Entering or not. */
	if (Cell[0] -> OnBndry) {
	    int i;
	    TrimBndrySortStruct
		*SInterTmp = NULL;

	    /* Search of another corner that is not on the boundary. */
	    for (i = 1; i < 4; i++)
	        if (!Cell[i] -> OnBndry)
		    break;

	    if (i >= 4) {
		/* All four corners are on boundary - ignore this cell. */
	        return NULL;
	    }
	    Inside = Cell[i] -> Inside;
	    BeginVal = CrntVal = i;

	    for (SInter = SortedInterList;
		 SInter != NULL;
		 SInter = SInter -> Pnext) {
	        SInter -> Entering = Inside;
	        if (SInter -> Val > i) {
		    SInterTmp = SInter;
		    Inside = !Inside;
		}
	    }
	    for (SInter = SortedInterList;
		 SInter != NULL && SInter != SInterTmp;
		 SInter = SInter -> Pnext) {
	        SInter -> Entering = Inside;
		Inside = !Inside;
	    }
	}
	else {
	    for (SInter = SortedInterList;
		 SInter != NULL;
		 SInter = SInter -> Pnext) {
	        SInter -> Entering = Inside;
		Inside = !Inside;
	    }
	}

	/* Merge every !Entering with the next Entering that follows it. */
	while (TRUE) {
	    if ((SInter = GetNextEntering(SortedInterList,
					  CrntVal, BeginVal)) != NULL) {
		TrimBndrySortStruct *SInterTmp;

		if (VHead == NULL)
		    BeginVal = SInter -> Val;

	        if (SInter -> StartOfCurve)
		    VHead = ChainVertexLists(Iu, Iv,
					     CrntVal, VHead,
					     SInter -> Val, SInter -> V);
		else
		    VHead = ChainVertexLists(Iu, Iv,
					     CrntVal, VHead,
					     SInter -> Val,
					     IPReverseVrtxList2(SInter -> V));

		/* Find the other end point of the curve. */ 
	        for (SInterTmp = SortedInterList;
		     SInterTmp != NULL;
		     SInterTmp = SInterTmp -> Pnext)
		    if (SInter -> V == SInterTmp -> V &&
			!SInterTmp -> Entering)
		        break;
		if (SInterTmp == NULL) {
		    /* Trimming curves are too complex for our resolution. */
		    return Plgs;
		}
		CrntVal = SInterTmp -> Val;

	        /* Free the used SInter as well as its other end complement. */
		SortedInterList = FreeSInterFromSortedList(SInterTmp,
							   SortedInterList);
		SortedInterList = FreeSInterFromSortedList(SInter,
							   SortedInterList);
	    }
	    else {
	        if (VHead == NULL)
		    return NULL;

	        /* Close the loop. */
	        AddCornerVertices(Iu, Iv, CrntVal, VHead,
				  CnvrtToBndryIndex(VHead -> Coord, Iu, Iv));

		/* We can dump this loop - it is complete. */
		CnvrtPlgsToCagdPlgs(VHead, Cell, Iu, Iv);
		BeginVal = 0.0;
		CrntVal = 0.0;
		VHead = NULL;

		if (SortedInterList == NULL)
		    break;
	    }
	}
    }

    return Plgs;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scans grid for the largest coplanar domain in the grid that is coplanar  *
* starting from (IStart, JStart).  Returns other corner in (IEnd, JEnd).     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtMesh:	       Evaluted positions of grid of samples.                *
*   IuStart, IvStart:  Starting location of coplanar domain search.          *
*   IuMax, IvMax:      Maximal values of cell indices in grid.               *
*   IuEnd, IvEnd:      Detected end locations.                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScanMeshForCoplanarity(Trim2PolyCellStruct **Grid,
				   int IuStart,
				   int IvStart,
				   int IuMax,
				   int IvMax,
				   int *IuEnd,
				   int *IvEnd)
{
    int i, j;
    Trim2PolyCellStruct *Cell[6];

    Cell[0] = &Grid[IvStart][IuStart];
    Cell[1] = &Grid[IvStart + 1][IuStart];
    Cell[2] = &Grid[IvStart + 1][IuStart + 1];
    Cell[3] = &Grid[IvStart][IuStart + 1];

    /* Expand along the first direction. */
    for (i = IuStart + 1; i <= IuMax; i++) {
        Cell[4] = &Grid[IvStart][i + 1];
	Cell[5] = &Grid[IvStart + 1][i + 1];

	if (!Cell[4] -> Processed &&
	    IS_SIMPLE_INSIDE_CELL(Cell[4]) &&
	    GMCoplanar4Pts(Cell[0] -> Pos, Cell[1] -> Pos,
			   Cell[4] -> Pos, Cell[5] -> Pos) &&
	    GMCollinear3Pts(Cell[0] -> Pos, Cell[3] -> Pos, Cell[4] -> Pos) &&
	    GMCollinear3Pts(Cell[1] -> Pos, Cell[2] -> Pos, Cell[5] -> Pos)) {
	    /* Still coplanar - continue. */
	}
	else {
	    i--;
	    break;
	}
    }
    *IuEnd = i < IuMax ? i : IRIT_MAX(IuMax - 1, IuStart);
    Cell[2] = &Grid[IvStart + 1][*IuEnd + 1];
    Cell[3] = &Grid[IvStart][*IuEnd + 1];

    /* Expand along the second direction. */
    for (j = IvStart + 1; j < IvMax; j++) {
	for (i = IuStart; i <= *IuEnd + 1; i++) {
	    Trim2PolyCellStruct
	        *c = &Grid[j + 1][i];

	    if (c -> Processed || !IS_SIMPLE_INSIDE_CELL(c))
	        break;
	}
	if (i <= *IuEnd + 1) {
	    j--;
	    break;
	}

        Cell[4] = &Grid[j + 1][IuStart];
	Cell[5] = &Grid[j + 1][*IuEnd + 1];

	if (GMCoplanar4Pts(Cell[0] -> Pos, Cell[3] -> Pos,
			   Cell[5] -> Pos, Cell[4] -> Pos) &&
	    GMCollinear3Pts(Cell[0] -> Pos, Cell[1] -> Pos, Cell[4] -> Pos) &&
	    GMCollinear3Pts(Cell[3] -> Pos, Cell[2] -> Pos, Cell[5] -> Pos)) {
	    /* Still coplanar - continue. */
	}
	else {
	    j--;
	    break;
	}
    }
    *IvEnd = j < IvMax ? j : IRIT_MAX(IvMax - 1, IvStart);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Removes edges on the boundary of the cell, and make sure no interior     *
* points of the Plls are on the boundary.	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Plls:      Polylines to clean.                                           *
*   Cells:     Four corners of the cell.                                     *
*   Iu, Iv:    Indices of cell in grid.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CleanUpOnBndry(IPPolygonStruct *Plls,
			   Trim2PolyCellStruct *Cells[4],
			   int Iu,
			   int Iv)
{
    IPPolygonStruct *Pll;

    /* Clean edges on boundary of cell. */
    for (Pll = Plls; Pll != NULL; Pll = Pll -> Pnext) {
        IPVertexStruct
	    *VHead = Pll -> PVertex;

	do {
	    if (VHead -> Pnext == NULL)
	        return;
	}
	while (CleanOneVrtxOnBndry(VHead, Cells, Iu, Iv));

	do {
	    if (Pll -> PVertex -> Pnext == NULL)
	        return;

	    for (VHead = Pll -> PVertex;
		 VHead -> Pnext -> Pnext != NULL;
		 VHead = VHead -> Pnext);
	}
	while (CleanOneVrtxOnBndry(VHead, Cells,  Iu, Iv));
    }

    /* Move inside interior Plls points that are on the boundary */
    for (Pll = Plls; Pll != NULL; Pll = Pll -> Pnext) {
	IPVertexStruct
	    *V = Pll -> PVertex;

	if (V -> Pnext != NULL && V -> Pnext -> Pnext != NULL) {
	    for (V = V -> Pnext; V -> Pnext -> Pnext != NULL; V = V -> Pnext) {
		if (IRIT_APX_EQ_EPS(V -> Coord[0], Iu, TRIM_INTRR_PLLS_BNDRY_EPS))
		    V -> Coord[0] = Iu + TRIM_INTRR_PLLS_BNDRY_EPS;
	        if (IRIT_APX_EQ_EPS(V -> Coord[0], Iu + 1,
			       TRIM_INTRR_PLLS_BNDRY_EPS))
		    V -> Coord[0] = Iu + 1 - TRIM_INTRR_PLLS_BNDRY_EPS;

	        if (IRIT_APX_EQ_EPS(V -> Coord[1], Iv, TRIM_INTRR_PLLS_BNDRY_EPS))
		    V -> Coord[1] = Iv + TRIM_INTRR_PLLS_BNDRY_EPS;
	        if (IRIT_APX_EQ_EPS(V -> Coord[1], Iv + 1,
			       TRIM_INTRR_PLLS_BNDRY_EPS))
		    V -> Coord[1] = Iv + 1 - TRIM_INTRR_PLLS_BNDRY_EPS;
	    }
        }
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Remove the vertex after VHead if this edge is on boundary.               *
*                                                                            *
* PARAMETERS:                                                                *
*   VHead:     Edge (VHead, VHead->Pnext) to test if on boundary.            *
*   Cells:     Four corners of the cell.                                     *
*   Iu, Iv:    Indices of cell in grid.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType: TRUE if vertex was deleted, FALSE otherwise.                  *
*****************************************************************************/
static CagdBType CleanOneVrtxOnBndry(IPVertexStruct *VHead,
				     Trim2PolyCellStruct *Cells[4],
				     int Iu,
				     int Iv)
{
    int i;
    IPVertexStruct
	*VNext = VHead -> Pnext;
    IrtRType
	*UV1 = VHead -> Coord,
	*UV2 = VNext -> Coord;

    if (VNext == NULL)
        return FALSE;

    for (i = 0; i < 2; i++) {
        int i0 = !i,
	    Iuv = i == 0 ? Iu : Iv;

        if (IRIT_APX_EQ_EPS(UV1[i], UV2[i], TRIM_FLT_PT_ROUND_OFF_ERR * 10) &&
	    (IRIT_APX_EQ_EPS(UV1[i], Iuv, TRIM_FLT_PT_ROUND_OFF_ERR * 10) ||
	     IRIT_APX_EQ_EPS(UV1[i], Iuv + 1, TRIM_FLT_PT_ROUND_OFF_ERR * 10))) {
	    /* This edge is on the boundary.  Remove this vertex. */
#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
	        IRIT_INFO_MSG_PRINTF("UV(%d) remove 1 %.14f %.14f    %.14f %.14f\n",
				     i, UV1[0], UV1[1], UV2[0], UV2[1]);
#	    endif /* DEBUG */

	    /* Update the trimming vertex to be on boundary. */
	    if (IRIT_APX_EQ_EPS(UV1[i], Iuv, TRIM_FLT_PT_ROUND_OFF_ERR * 10))
	        UV1[i] = Iuv;
	    else
	        UV1[i] = Iuv + 1;

	    /* Mark corners to be on boundary if so. */
	    UpdateBndryCellCorners(UV1, Cells, TRIM_FLT_PT_ROUND_OFF_ERR * 10,
				   Iu, Iv);
	    UpdateBndryCellCorners(UV2, Cells, TRIM_FLT_PT_ROUND_OFF_ERR * 10,
				   Iu, Iv);

	    if (VNext -> Pnext != NULL)
	        UV1[i0] = UV2[!i];

	    VHead -> Pnext = VNext -> Pnext;
	    IPFreeVertex(VNext);
	    return TRUE;
	}
    }

    for (i = 0; i < 2; i++) {
        int Iuv = i == 0 ? Iu : Iv;

        if (IRIT_APX_EQ_EPS(UV1[i], UV2[i], TRIM_DOMAIN_EPS_EXPAN * 10) &&
	    (IRIT_APX_EQ_EPS(UV1[i], Iuv, TRIM_DOMAIN_EPS_EXPAN * 10) ||
	     IRIT_APX_EQ_EPS(UV1[i], Iuv + 1, TRIM_DOMAIN_EPS_EXPAN * 10))) {
	    /* This edge is on the boundary.  Remove this vertex. */
#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
	        IRIT_INFO_MSG_PRINTF("UV(%d) remove 2 %.14f %.14f    %.14f %.14f\n",
				     i, UV1[0], UV1[1], UV2[0], UV2[1]);
#	    endif /* DEBUG */

	    /* Update the trimming vertex to be on boundary. */
	    if (IRIT_APX_EQ_EPS(UV1[i], Iuv, TRIM_DOMAIN_EPS_EXPAN * 10))
	        UV1[i] = Iuv;
	    else
	        UV1[i] = Iuv + 1;

	    /* Mark corners to be on boundary if so. */
	    UpdateBndryCellCorners(UV1, Cells, TRIM_DOMAIN_EPS_EXPAN * 10,
				   Iu, Iv);
	    UpdateBndryCellCorners(UV2, Cells, TRIM_DOMAIN_EPS_EXPAN * 10,
				   Iu, Iv);

	    if (VNext -> Pnext != NULL)
	        UV1[!i] = UV2[!i];

	    VHead -> Pnext = VNext -> Pnext;
	    IPFreeVertex(VNext);
	    return TRUE;
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compares the removed UV to the four corners and update on boundary if    *
* close enough.                                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   UV:        To Compare.                                                   *
*   Cells:     To update if close to UV.                                     *
*   Eps:       Tolerance of comparison.	                                     *
*   Iu, Iv:    Indices of cell in grid.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void UpdateBndryCellCorners(CagdRType *UV,
				   Trim2PolyCellStruct *Cells[4],
				   CagdRType Eps,
				   int Iu,
				   int Iv)
{
    if (IRIT_APX_EQ_EPS(UV[0], Iu, Eps)) {
        if (IRIT_APX_EQ_EPS(UV[1], Iv, Eps))
	    Cells[0] -> OnBndry = TRUE;
	else if (IRIT_APX_EQ_EPS(UV[1], Iv + 1, Eps))
	    Cells[1] -> OnBndry = TRUE;
    }
    else if (IRIT_APX_EQ_EPS(UV[0], Iu + 1, Eps)) {
        if (IRIT_APX_EQ_EPS(UV[1], Iv, Eps))
	    Cells[3] -> OnBndry = TRUE;
	else if (IRIT_APX_EQ_EPS(UV[1], Iv + 1, Eps))
	    Cells[2] -> OnBndry = TRUE;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Chain V1 and V2 into one vertex list and return it.  If end of V1 list   *
* and the beginning of V1 lists are on different edges, respective corners   *
* are added.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Iu, Iv: Indices of cell in grid.                                         *
*   Val1:   Termination boundary value of first vertex list.                 *
*   V1:     First vertex list.                                               *
*   Val2:   Starting boundary value of second vertex list.                   *
*   V2:     Second vertex list                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPVertexList *:   Chained vertex list.                                   *
*****************************************************************************/
static IPVertexStruct *ChainVertexLists(int Iu,
					int Iv,
					CagdRType Val1,
					IPVertexStruct *V1,
					CagdRType Val2,
					IPVertexStruct *V2)
{
    IPVertexStruct *VLast;

    AddCornerVertices(Iu, Iv, Val1, V1, Val2);
    if (V1 == NULL)
        return V2;
    else {
	VLast = IPGetLastVrtx(V1);
	VLast -> Pnext = V2;
	return V1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Chain corner edges between end of V1 at Val1End and Val2Strt.  The new   *
* vertices are appended at the end of list V1.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   Iu, Iv:    Indices of cell in grid.                                      *
*   Val1End:   Termination boundary value of vertex list V1.	             *
*   V1:        First vertex list.                                            *
*   Val2Strt:  Starting boundary value of next vertex list.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void AddCornerVertices(int Iu,
			      int Iv,
			      CagdRType Val1End,
			      IPVertexStruct *V1,
			      CagdRType Val2Strt)
{
    int i;
    IPVertexStruct *VLast;

    if (V1 == NULL)
	return;

    VLast = IPGetLastVrtx(V1);

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
	IRIT_INFO_MSG_PRINTF("Add corners from %f to %f: ", Val1End, Val2Strt);
#   endif /* DEBUG */
    
    if (Val2Strt > Val1End) {
        for (i = INT_GREATER_THAN(Val1End); i <= INT_LESS_THAN(Val2Strt); i++) {
#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
		IRIT_INFO_MSG_PRINTF("%d ", i);
#	    endif /* DEBUG */

	    VLast -> Pnext = IPAllocVertex2(NULL);
	    VLast = VLast -> Pnext;
	    VLast -> Coord[0] = (i == 2 || i == 3) ? Iu + 1 : Iu;
	    VLast -> Coord[1] = (i == 1 || i == 2) ? Iv + 1 : Iv;
	}
    }
    else {
        for (i = INT_GREATER_THAN(Val1End); i < 4; i++) {
#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
		IRIT_INFO_MSG_PRINTF("%d ", i);
#	    endif /* DEBUG */

	    VLast -> Pnext = IPAllocVertex2(NULL);
	    VLast = VLast -> Pnext;
	    VLast -> Coord[0] = (i == 2 || i == 3) ? Iu + 1 : Iu;
	    VLast -> Coord[1] = (i == 1 || i == 2) ? Iv + 1 : Iv;
	}
        for (i = 0; i <= INT_LESS_THAN(Val2Strt); i++) {
#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply)
		IRIT_INFO_MSG_PRINTF("%d ", i);
#	    endif /* DEBUG */

	    VLast -> Pnext = IPAllocVertex2(NULL);
	    VLast = VLast -> Pnext;
	    VLast -> Coord[0] = (i == 2 || i == 3) ? Iu + 1 : Iu;
	    VLast -> Coord[1] = (i == 1 || i == 2) ? Iv + 1 : Iv;
	}
    }
#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugTrim2Ply) {
        IRIT_INFO_MSG("\n");
	IPStderrObject(IPGenPOLYLINEObject(IPAllocPolygon(0,V1,NULL)));
    }
#endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find in SInterList the minimal next intersection that is greater than    *
* CrntVal and is not going beyond BeginVal (closing the loop).               *
*                                                                            *
* PARAMETERS:                                                                *
*   SInterList:    List to examine for the proper minimal next intersection. *
*   CrntVal:       Current value of intersection.                            *
*   BeginVal:      Beginning value of this loop, for termination condition.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrimBndrySortStruct *:     Next intersection or NULL if none.            *
*****************************************************************************/
static TrimBndrySortStruct *GetNextEntering(TrimBndrySortStruct *SInterList,
					    CagdRType CrntVal,
					    CagdRType BeginVal)
{
    TrimBndrySortStruct *SInter;
    CagdRType MinValGrtrThan;

    if (SInterList == NULL)
        return NULL;

    /* Find the intersection that is beyond CrntVal. */
    for (SInter = SInterList; SInter != NULL; SInter = SInter -> Pnext) {
	if (SInter -> Entering && SInter -> Val > CrntVal)
	    break;
    }
    
    if (SInter == NULL) {
        for (SInter = SInterList; SInter != NULL; SInter = SInter -> Pnext)
	    if (SInter -> Entering)
	        break;
    }
    if (SInter == NULL)
        return NULL;

    MinValGrtrThan = SInter -> Val;

    if (MinValGrtrThan > CrntVal) {
        if (BeginVal > MinValGrtrThan || BeginVal <= CrntVal)
	    return SInter;
    }
    else {
        if (BeginVal > MinValGrtrThan && BeginVal <= CrntVal)
	    return SInter;
    }

    return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Deletes one item out of the given list.                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Item:      To delete.                                                    *
*   List:      From this list.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrimBndrySortStruct *:   Returned list without Item.                     *
*****************************************************************************/
static TrimBndrySortStruct *FreeSInterFromSortedList(TrimBndrySortStruct *Item,
						     TrimBndrySortStruct *List)
{
    if (Item == List) {
        List = List -> Pnext;
    }
    else {
        TrimBndrySortStruct *Step;

	for (Step = List; Step -> Pnext != NULL; Step = Step -> Pnext) {
	    if (Step -> Pnext == Item) {
		Step -> Pnext = Item -> Pnext;
		break;
	    }
	}
    }
    IritFree(Item);

    return List;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts the piecewise linear polyline into a closed polygon and convert *
* the later on triangles.  The given VList is assumed to hold the UV params. *
* of the polygons which is evaluated to Euclidean space via the four corner  *
* Cells.  Cells can optionally hold normals and UV values if are needed.     *
*                                                                            *
* PARAMETERS:                                                                *
*   VList:  List of vertices forming the polygon. May be open.               *
*   Cells:  Four corners of the cell.                                        *
*   Iu, Iv:      Indices of cell (from (Iu, Iv) to (Iu + 1, Iv + 1)).        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void CnvrtPlgsToCagdPlgs(IPVertexStruct *VList,
				Trim2PolyCellStruct *Cells[4],
				int Iu,
				int Iv)
{
    IPVertexStruct *V,
        *VLast = IPGetLastVrtx(VList);
    IPPolygonStruct *Pl;
    IPObjectStruct *PObjPolys, *PObjNGons;
    CagdPolygonStruct CagdPl;

    Pl = IPAllocPolygon(0, VList, NULL);

    /* Zero the Z component and close the loop as circular. */
    for (V = VList; V != NULL; V = V -> Pnext)
	V -> Coord[2] = 0.0;
    VLast -> Pnext = VList;

    IPUpdatePolyPlane(Pl);
    GMCleanUpPolygonList(&Pl, TRIM_DOMAIN_EPS_EXPAN);

    PObjPolys = IPGenPOLYObject(Pl);

    PObjNGons = GMConvertPolysToNGons(PObjPolys, 4);
    IPFreeObject(PObjPolys);

    for (Pl = PObjNGons -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	int i, GenPoly,
	    Len = IPVrtxListLen(Pl -> PVertex);

	for (V = Pl -> PVertex, i = 0; i < Len; V = V -> Pnext, i++) {
	    CagdPType Pt1, Pt2;
	    CagdRType
	        Fu = V -> Coord[0] - Iu,
	        Fv = V -> Coord[1] - Iv;

	    IRIT_PT_BLEND(Pt1, Cells[1] -> Pos, Cells[0] -> Pos, Fv);
	    IRIT_PT_BLEND(Pt2, Cells[2] -> Pos, Cells[3] -> Pos, Fv);
	    IRIT_PT_BLEND(CagdPl.U.Polygon[i].Pt, Pt2, Pt1, Fu);

	    if (Cells[0] -> Nrml != NULL) {
	        CagdVType Nrml1, Nrml2;

		IRIT_VEC_BLEND(Nrml1, Cells[1] -> Nrml, Cells[0] -> Nrml, Fv);
		IRIT_VEC_BLEND(Nrml2, Cells[2] -> Nrml, Cells[3] -> Nrml, Fv);
		IRIT_VEC_BLEND(CagdPl.U.Polygon[i].Nrml, Nrml2, Nrml1, Fu);
		IRIT_VEC_NORMALIZE(CagdPl.U.Polygon[i].Nrml);
	    }

	    if (Cells[0] -> Uv != NULL) {
	        CagdUVType Uv1, Uv2;

		IRIT_UV_BLEND(Uv1, Cells[1] -> Uv, Cells[0] -> Uv, Fv);
		IRIT_UV_BLEND(Uv2, Cells[2] -> Uv, Cells[3] -> Uv, Fv);
		IRIT_UV_BLEND(CagdPl.U.Polygon[i].UV, Uv2, Uv1, Fu);
	    }
	}

	if (Len == 3)
	    _CagdSrfMakeTriFunc(Cells[0] -> Nrml != NULL,
				Cells[0] -> Uv != NULL,
				CagdPl.U.Polygon[0].Pt,
				CagdPl.U.Polygon[1].Pt,
				CagdPl.U.Polygon[2].Pt,
				CagdPl.U.Polygon[0].Nrml,
				CagdPl.U.Polygon[1].Nrml,
				CagdPl.U.Polygon[2].Nrml,
				CagdPl.U.Polygon[0].UV,
				CagdPl.U.Polygon[1].UV,
				CagdPl.U.Polygon[2].UV,
				&GenPoly);
	else {
	    _CagdSrfMakeRectFunc(Cells[0] -> Nrml != NULL,
				 Cells[0] -> Uv != NULL,
				 CagdPl.U.Polygon[0].Pt,
				 CagdPl.U.Polygon[1].Pt,
				 CagdPl.U.Polygon[2].Pt,
				 CagdPl.U.Polygon[3].Pt,
				 CagdPl.U.Polygon[0].Nrml,
				 CagdPl.U.Polygon[1].Nrml,
				 CagdPl.U.Polygon[2].Nrml,
				 CagdPl.U.Polygon[3].Nrml,
				 CagdPl.U.Polygon[0].UV,
				 CagdPl.U.Polygon[1].UV,
				 CagdPl.U.Polygon[2].UV,
				 CagdPl.U.Polygon[3].UV,
				 &GenPoly);
	    if (!GenPoly) {
	        /* The rectilinear is degenerate - try two triangles, only  */
	        /* one of which is likely to be generated.		    */
	        _CagdSrfMakeTriFunc(Cells[0] -> Nrml != NULL,
				    Cells[0] -> Uv != NULL,
				    CagdPl.U.Polygon[0].Pt,
				    CagdPl.U.Polygon[1].Pt,
				    CagdPl.U.Polygon[2].Pt,
				    CagdPl.U.Polygon[0].Nrml,
				    CagdPl.U.Polygon[1].Nrml,
				    CagdPl.U.Polygon[2].Nrml,
				    CagdPl.U.Polygon[0].UV,
				    CagdPl.U.Polygon[1].UV,
				    CagdPl.U.Polygon[2].UV,
				    &GenPoly);
	        _CagdSrfMakeTriFunc(Cells[0] -> Nrml != NULL,
				    Cells[0] -> Uv != NULL,
				    CagdPl.U.Polygon[0].Pt,
				    CagdPl.U.Polygon[2].Pt,
				    CagdPl.U.Polygon[3].Pt,
				    CagdPl.U.Polygon[0].Nrml,
				    CagdPl.U.Polygon[2].Nrml,
				    CagdPl.U.Polygon[3].Nrml,
				    CagdPl.U.Polygon[0].UV,
				    CagdPl.U.Polygon[2].UV,
				    CagdPl.U.Polygon[3].UV,
				    &GenPoly);
	    }
	}
    }
    IPFreeObject(PObjNGons);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Insert one trimming boundart item into its sorted list and return the    *
* updated list.                                                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Item:      To insert into the list.                                      *
*   List:      Current list.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   TrimBndrySortStruct *:  Updated list.                                    *
*****************************************************************************/
static TrimBndrySortStruct *InsertSInterToList(TrimBndrySortStruct *Item,
					       TrimBndrySortStruct *List)
{
    TrimBndrySortStruct *Last, *Crnt;

    if (List == NULL) {
        Item -> Pnext = NULL;
	return Item;
    }

    if (Item -> Val <= List -> Val) {
	Item -> Pnext = List;
	return Item;
    }

    Last = List;
    Crnt = List -> Pnext;
    while (Crnt != NULL && Item -> Val > Crnt -> Val) {
	Last = Crnt;
	Crnt = Crnt -> Pnext;
    }
    Item -> Pnext = Crnt;
    Last -> Pnext = Item;

    return List;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a point on the boundary of a cell to a real value between zero  *
* and four as 0-1 in left boundary, 1-2 top, 2-3 right, and 3-4 bottom.      *
*   the 0 and 4 are the same location (bottom left corner).		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt:         Point to convert to its boundary.                            *
*   Iu, Iv:      Indices of cell (from (Iu, Iv) to (Iu + 1, Iv + 1)).        *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:  A number between zero and four, or negative if interior.     *
*****************************************************************************/
static CagdRType CnvrtToBndryIndex(IrtRType *Pt, int Iu, int Iv)
{
    CagdRType t, Eps;

    for (Eps = TRIM_ON_GRID_BNDRY;
	 Eps <= TRIM_ON_GRID_BNDRY2;
	 Eps *= 10.0) {
        if (IRIT_APX_EQ_EPS(Pt[0], Iu, Eps)) {		   /* Left boundary. */
	    t = Pt[1] - Iv;
	    if (t > 1.0 || t < 0.0)
	        return -1.0;
	    return t;
	}
	else if (IRIT_APX_EQ_EPS(Pt[0], Iu + 1, Eps)) {	     /* Right bndry. */
	    t = Pt[1] - Iv;
	    if (t > 1.0 || t < 0.0)
	        return -1.0;
	    return 3.0 - t;
	}
	else if (IRIT_APX_EQ_EPS(Pt[1], Iv, Eps)) {		    /* Bottom bndry. */
	    t = Pt[0] - Iu;
	    if (t > 1.0 || t < 0.0)
	        return -1.0;
	    return 4.0 - t;
	}
	else if (IRIT_APX_EQ_EPS(Pt[1], Iv + 1, Eps)) {	       /* Top bndry. */
	    t = Pt[0] - Iu;
	    if (t > 1.0 || t < 0.0)
	        return -1.0;
	    return 1.0 + t;
	}
    }

    return -1.0;
}
