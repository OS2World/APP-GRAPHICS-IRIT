/******************************************************************************
* Bzr2Poly.c - Bezier to polygon/polylines conversion routines.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 90.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define SRF(U, V)		  CAGD_MESH_UV(Srf, U, V)
#define CAGD_PT_IN_PLANE(Pln, Pt) (IRIT_FABS(IRIT_DOT_PROD(Pln, Pt) + Pln[3]) < IRIT_EPS)
#define VALID_UV(UVVal)	  	  (UVVal == NULL ? NULL : UVVal -> UV)
#define VALID_NL(Nrml)	  	  (Nrml == NULL ? NULL : Nrml -> Vec)

static int
    _CagdMergeCoplanarPolys = TRUE,
    _CagdClipPolysAtPoles = FALSE;

static CagdPolygonStruct *CagdSrf2PolygonsGenPolysRglr(const CagdSrfStruct
						                         *Srf,
						       CagdBType FourPerFlat,
						       CagdRType *PtWeights,
						       CagdPtStruct *PtMesh,
						       CagdVecStruct *PtNrml,
						       CagdUVStruct *UVMesh,
						       int FineNessU,
						       int FineNessV);
static void ScanMeshForCoplanarity(CagdPtStruct *PtMesh,
				   char *GridMask,
				   int IStart,
				   int JStart,
				   int ISize,
				   int JSize,
				   int *IEnd,
				   int *JEnd);
static CagdPolygonStruct *CagdSrf2PolygonsGenPolysStrips(const CagdSrfStruct
							                  *Srf,
							 CagdBType FourPerFlat,
							 CagdRType *PtWeights,
							 CagdPtStruct *PtMesh,
							 CagdVecStruct *PtNrml,
							 CagdUVStruct *UVMesh,
							 int FineNessU,
							 int FineNessV);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the polygonal approximation of surfaces to mrge or not adjacent     M
* coplanar polygons into one.  The default is to apply this optimization     M
* but in cases where a uniform mesh is need, it should be disabled.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   MergeCoplanarPolys:        New setting to use.		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value.				                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2Polygons, CagdSrf2PolygonFast, CagdSrf2PolygonStrip              M
*   Cagd2PolyClipPolysAtPoles						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolygonMergeCoplanar                                             M
*****************************************************************************/
int CagdSrf2PolygonMergeCoplanar(int MergeCoplanarPolys)
{
    int OldVal = _CagdMergeCoplanarPolys;

    _CagdMergeCoplanarPolys = MergeCoplanarPolys;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the option of clipping polylines and polygon at poles (when the     M
* rational curves/surface goes to infinity due to division by zero.).        M
*   If ClipPolysAtPoles == CAGD_QUERY_VALUE, current state is only queried.  M
*                                                                            *
* PARAMETERS:                                                                M
*   ClipPolysAtPoles:      New setting to use or CAGD_QUERY_VALUE to query.  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:  Old value.				                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2Polygons, CagdSrf2PolygonFast, CagdSrf2PolygonStrip              M
*   CagdSrf2PolygonMergeCoplanar					     M 
*                                                                            *
* KEYWORDS:                                                                  M
*   Cagd2PolyClipPolysAtPoles                                                M
*****************************************************************************/
int Cagd2PolyClipPolysAtPoles(int ClipPolysAtPoles)
{
    if (ClipPolysAtPoles == CAGD_QUERY_VALUE)
        return _CagdClipPolysAtPoles;
    else {
        int OldVal = _CagdClipPolysAtPoles;

	_CagdClipPolysAtPoles = ClipPolysAtPoles;

	return OldVal;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single Bezier surface to set of triangles	     M
* approximating it. FineNess is a fineness control on result and the larger  M
t is more triangles may result. A value of 10 is a good start value.	     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
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
*   BspSrf2Polygons, IritSurface2Polygons, IritTrimSrf2Polygons,             M
*   CagdSrf2Polygons, TrimSrf2Polygons, CagdSrf2Polygons	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrf2Polygons, polygonization, surface approximation                   M
*****************************************************************************/
CagdPolygonStruct *BzrSrf2Polygons(const CagdSrfStruct *Srf,
				   int FineNess,
				   CagdBType ComputeNormals,
				   CagdBType FourPerFlat,
				   CagdBType ComputeUV)
{
    int FineNessU, FineNessV;
    CagdRType *PtWeights;
    CagdPtStruct *PtMesh;
    CagdVecStruct *PtNrml;
    CagdUVStruct *UVMesh;
			     
    if (BzrSrf2PolygonsSamples(Srf, FineNess, ComputeNormals, ComputeUV,
			       &PtWeights, &PtMesh, &PtNrml, &UVMesh,
			       &FineNessU, &FineNessV))
	return CagdSrf2PolygonsGenPolys(Srf, FourPerFlat, PtWeights,
					PtMesh, PtNrml, UVMesh,
					FineNessU, FineNessV);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single freeform surface to set of triangles	     M
* approximating it using a uniform fixed resolution of Nu x Nv.		     M
* NULL is returned in case of an error, otherwise list of CagdPolygonStruct. M
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
*   BzrSrf2Polygons, BspSrf2Polygons, CagdCrv2Polyline, CagdSrf2Polylines,   M
*   CagdSrf2PolygonStrip, CagdSrf2Polygons				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrf2PolygonsN, evaluation, polygonal approximation                    M
*****************************************************************************/
CagdPolygonStruct *BzrSrf2PolygonsN(const CagdSrfStruct *Srf,
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
			     
    if (BzrSrf2PolygonsSamplesNuNv(Srf, Nu, Nv, ComputeNormals, ComputeUV,
				   &PtWeights, &PtMesh, &PtNrml, &UVMesh))
	return CagdSrf2PolygonsGenPolys(Srf, FourPerFlat, PtWeights, PtMesh,
					PtNrml,	UVMesh, Nu, Nv);
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert uniform grid samples of a freeform srf to a set of    M
* triangles/rectangles/polystrips approximating it.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To approximate into triangles.                         M
*   FourPerFlat:      If TRUE, four triangles are created per flat surface.  M
*                     If FALSE, only 2 triangles are created.                M
*   PtWeights:        Weights of the evaluations, if rational, to detect     M
*		      poles.  NULL if surface not rational.		     M
*		      Freed by this function.				     M
*   PtMesh:	      Evaluted positions of grid of samples.                 M
*		      Freed by this function.				     M
*   PtNrml:	      Evaluted normals of grid of samples or NULL if none.   M
*		      Freed by this function.				     M
*   UVMesh:	      Evaluted UV vals of grid of samples or NULL if none.   M
*		      Freed by this function.				     M
*   FineNessU, FineNessV:  Actual size of PtMesh, PtNrml, UVMesh.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error.              M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrf2Polygons, BspSrf2Polygons, IritSurface2Polygons,	             M
*   IritTrimSrf2Polygons, CagdSrf2Polygons, TrimSrf2Polygons	             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolygonsGenPolys, polygonization, surface approximation  	     M
*****************************************************************************/
CagdPolygonStruct *CagdSrf2PolygonsGenPolys(const CagdSrfStruct *Srf,
					    CagdBType FourPerFlat,
					    CagdRType *PtWeights,
					    CagdPtStruct *PtMesh,
					    CagdVecStruct *PtNrml,
					    CagdUVStruct *UVMesh,
					    int FineNessU,
					    int FineNessV)
{
    if (_CagdSrf2PolygonStrips)
	return CagdSrf2PolygonsGenPolysStrips(Srf, FourPerFlat, PtWeights,
					      PtMesh, PtNrml, UVMesh,
					      FineNessU, FineNessV);
    else
	return CagdSrf2PolygonsGenPolysRglr(Srf, FourPerFlat, PtWeights,
					    PtMesh, PtNrml, UVMesh,
					    FineNessU, FineNessV);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of CagdSrf2PolygonsGenPolys for regular polygons.     *
*****************************************************************************/
static CagdPolygonStruct *CagdSrf2PolygonsGenPolysRglr(const CagdSrfStruct 
						                         *Srf,
						       CagdBType FourPerFlat,
						       CagdRType *PtWeights,
						       CagdPtStruct *PtMesh,
						       CagdVecStruct *PtNrml,
						       CagdUVStruct *UVMesh,
						       int FineNessU,
						       int FineNessV)
{
    char *GridMask;
    int i, j, GenPoly,
	FineNessU1 = FineNessU - 1,
	FineNessV1 = FineNessV - 1,
	MeshSize = FineNessU * FineNessV;
    CagdBType
	SetMakeTriRecFuncs = FALSE,
	ComputeNormals = PtNrml != NULL,
	ComputeUV = UVMesh != NULL;
    CagdPtStruct PtCenter, *Pt1, *Pt2, *Pt3, *Pt4;
    CagdUVStruct UVCenter,
	*UV1 = NULL,
	*UV2 = NULL,
	*UV3 = NULL,
	*UV4 = NULL;
    CagdVecStruct NlCenter,
	*Nl1 = NULL,
	*Nl2 = NULL,
	*Nl3 = NULL,
	*Nl4 = NULL;
    CagdPolygonStruct *Poly,
	*PolyHead = NULL;
    CagdSrfMakeTriFuncType
	OldTriFunc = NULL;
    CagdSrfMakeRectFuncType
	OldRectFunc = NULL;

    GridMask = (char *) IritMalloc(sizeof(char) * FineNessU * FineNessV);
    IRIT_ZAP_MEM(GridMask, sizeof(char) * FineNessU * FineNessV);

    if (_CagdSrfMakeTriFunc == NULL || _CagdSrfMakeRectFunc == NULL) {
	OldRectFunc = CagdSrfSetMakeRectFunc(CagdMakeRectangle);
        OldTriFunc = CagdSrfSetMakeTriFunc(CagdMakeTriangle);
	SetMakeTriRecFuncs = TRUE;
    }

    /* Now that we have the mesh, create the polygons. */
    for (i = 0; i < FineNessU1; i++) {
	for (j = 0; j < FineNessV1; j++) {
	    int BaseIndex = i * FineNessV + j,
		BaseIndex1 = BaseIndex + 1,
		BaseIndexV = BaseIndex + FineNessV,
		BaseIndexV1 = BaseIndexV + 1;

	    if (GridMask[BaseIndex]) /* Did we already expand into this one? */
	        continue;

	    Pt1 = &PtMesh[BaseIndex];        /* Cache the four flat corners. */
	    Pt2 = &PtMesh[BaseIndex1];
	    Pt3 = &PtMesh[BaseIndexV1];
	    Pt4 = &PtMesh[BaseIndexV];

	    if (_CagdClipPolysAtPoles &&
		PtWeights != NULL) {        /* If rational - test for poles. */
		CagdRType
		    W1 = PtWeights[BaseIndex],
		    W2 = PtWeights[BaseIndex1],
		    W3 = PtWeights[BaseIndexV1],
		    W4 = PtWeights[BaseIndexV];

		if (W1 * W2 < 0.0 || W1 * W3 < 0.0 || W1 * W4 < 0.0)
		    continue; /* The cell sits over a pole - purge it. */
	    }

	    if (_CagdMergeCoplanarPolys &&
		GMCoplanar4Pts(Pt1 -> Pt, Pt2 -> Pt, Pt3 -> Pt, Pt4 -> Pt)) {
		int IEnd, JEnd;

		ScanMeshForCoplanarity(PtMesh, GridMask, i, j,
				       FineNessU, FineNessV, &IEnd, &JEnd);

		if (IEnd == i && JEnd == j) {
		    /* Only this one is coplanar. */
		    GridMask[BaseIndex] = 1;
		}
		else {
		    int ii, jj;

		    BaseIndex1 = BaseIndex + (JEnd - j + 1),
		    BaseIndexV = BaseIndex + FineNessV * (IEnd - i + 1),
		    BaseIndexV1 = BaseIndexV + (JEnd - j + 1);

		    Pt2 = &PtMesh[BaseIndex1];
		    Pt3 = &PtMesh[BaseIndexV1];
		    Pt4 = &PtMesh[BaseIndexV];

		    /* Mark all these cells as covered. */
		    for (ii = i; ii <= IEnd; ii++) {
			int l = ii * FineNessV;

			for (jj = j; jj <= JEnd; jj++)
			    GridMask[l + jj] = 1;
		    }
		}

		if (ComputeNormals) {
		    Nl1 = &PtNrml[BaseIndex];
		    Nl2 = &PtNrml[BaseIndex1];
		    Nl3 = &PtNrml[BaseIndexV1];
		    Nl4 = &PtNrml[BaseIndexV];
		}
		if (ComputeUV) {
		    UV1 = &UVMesh[BaseIndex];
		    UV2 = &UVMesh[BaseIndex1];
		    UV3 = &UVMesh[BaseIndexV1];
		    UV4 = &UVMesh[BaseIndexV];
		}

		if ((Poly = _CagdSrfMakeRectFunc(-ComputeNormals, ComputeUV,
		    Pt1 -> Pt, Pt2 -> Pt, Pt3 -> Pt, Pt4 -> Pt,
		    VALID_NL(Nl1), VALID_NL(Nl2), VALID_NL(Nl3), VALID_NL(Nl4),
		    VALID_UV(UV1), VALID_UV(UV2), VALID_UV(UV3), VALID_UV(UV4),
		    &GenPoly)) != NULL) {
		    IRIT_LIST_PUSH(Poly, PolyHead);
		}
		else if (!GenPoly) {
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
				   ComputeUV,
				   Pt1 -> Pt, Pt2 -> Pt, Pt3 -> Pt,
				   VALID_NL(Nl1), VALID_NL(Nl2), VALID_NL(Nl3),
				   VALID_UV(UV1), VALID_UV(UV2), VALID_UV(UV3),
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
				   ComputeUV,
				   Pt3 -> Pt, Pt4 -> Pt, Pt1 -> Pt,
				   VALID_NL(Nl3), VALID_NL(Nl4), VALID_NL(Nl1),
				   VALID_UV(UV3), VALID_UV(UV4), VALID_UV(UV1),
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		}
	    }
	    else {
	        /* It is a general surface area - generate local polygons. */
		if (ComputeNormals) {
		    Nl1 = &PtNrml[BaseIndex];
		    Nl2 = &PtNrml[BaseIndex1];
		    Nl3 = &PtNrml[BaseIndexV1];
		    Nl4 = &PtNrml[BaseIndexV];
		}
		if (ComputeUV) {
		    UV1 = &UVMesh[BaseIndex];
		    UV2 = &UVMesh[BaseIndex1];
		    UV3 = &UVMesh[BaseIndexV1];
		    UV4 = &UVMesh[BaseIndexV];
		}

	        if (FourPerFlat) {/* Eval mid pt and create 4 triangles. */
		    CAGD_COPY_POINT(PtCenter, *Pt1);
		    CAGD_ADD_POINT(PtCenter, *Pt2);
		    CAGD_ADD_POINT(PtCenter, *Pt3);
		    CAGD_ADD_POINT(PtCenter, *Pt4);
		    CAGD_MULT_POINT(PtCenter, 0.25);

		    if (ComputeNormals) {
		        /* Average four normals to find the middle one. */
		        CAGD_COPY_VECTOR(NlCenter, *Nl1);
			CAGD_ADD_VECTOR(NlCenter, *Nl2);
			CAGD_ADD_VECTOR(NlCenter, *Nl3);
			CAGD_ADD_VECTOR(NlCenter, *Nl4);
			CAGD_NORMALIZE_VECTOR(NlCenter);
		    }

		    if (ComputeUV) {
		        UVCenter.UV[0] = (UV1 -> UV[0] + UV2 -> UV[0] +
					  UV3 -> UV[0] + UV4 -> UV[0]) * 0.25;
			UVCenter.UV[1] = (UV1 -> UV[1] + UV2 -> UV[1] +
					  UV3 -> UV[1] + UV4 -> UV[1]) * 0.25;
		    }

		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
				   ComputeUV,
				   Pt1 -> Pt, Pt2 -> Pt, PtCenter.Pt,
				   VALID_NL(Nl1), VALID_NL(Nl2), NlCenter.Vec,
				   VALID_UV(UV1), VALID_UV(UV2), UVCenter.UV,
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
				   ComputeUV,
				   Pt2 -> Pt, Pt3 -> Pt, PtCenter.Pt,
				   VALID_NL(Nl2), VALID_NL(Nl3), NlCenter.Vec,
				   VALID_UV(UV2), VALID_UV(UV3), UVCenter.UV,
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
                                   ComputeUV,
				   Pt3 -> Pt, Pt4 -> Pt, PtCenter.Pt,
				   VALID_NL(Nl3), VALID_NL(Nl4), NlCenter.Vec,
				   VALID_UV(UV3), VALID_UV(UV4), UVCenter.UV,
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
                                   ComputeUV,
				   Pt4 -> Pt, Pt1 -> Pt, PtCenter.Pt,
				   VALID_NL(Nl4), VALID_NL(Nl1), NlCenter.Vec,
				   VALID_UV(UV4), VALID_UV(UV1), UVCenter.UV,
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		}
		else {		           /* Only two along the diagonal... */
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
                                   ComputeUV,
				   Pt1 -> Pt, Pt2 -> Pt, Pt3 -> Pt,
				   VALID_NL(Nl1), VALID_NL(Nl2), VALID_NL(Nl3),
				   VALID_UV(UV1), VALID_UV(UV2), VALID_UV(UV3),
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		    if ((Poly = _CagdSrfMakeTriFunc(-ComputeNormals,
                                   ComputeUV,
				   Pt3 -> Pt, Pt4 -> Pt, Pt1 -> Pt,
				   VALID_NL(Nl3), VALID_NL(Nl4), VALID_NL(Nl1),
				   VALID_UV(UV3), VALID_UV(UV4), VALID_UV(UV1),
				   &GenPoly)) != NULL)
		        IRIT_LIST_PUSH(Poly, PolyHead);
		}
	    }
	}
    }

    CagdPtArrayFree(PtMesh, MeshSize);
    if (PtWeights != NULL)
	IritFree(PtWeights);
    if (ComputeNormals)
	CagdVecArrayFree(PtNrml, MeshSize);
    if (ComputeUV)
	CagdUVArrayFree(UVMesh, MeshSize);

    IritFree(GridMask);

    if (SetMakeTriRecFuncs) {
	CagdSrfSetMakeTriFunc(OldTriFunc);
	CagdSrfSetMakeRectFunc(OldRectFunc);
    }

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Scans grid for the largest coplanar domain in the grid that is coplanar  *
* starting from (IStart, JStart).  Returns other corner in (IEnd, JEnd).     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtMesh:	    Evaluted positions of grid of samples.                   *
*   GridMask:       Mask for visited (or not) cells.			     *
*   IStart, JStart: Starting location of coplanar domain search.             *
*   ISize, JSize:   Size of grid.                                            *
*   IEnd, JEnd:     Detected end locations.                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScanMeshForCoplanarity(CagdPtStruct *PtMesh,
				   char *GridMask,
				   int IStart,
				   int JStart,
				   int ISize,
				   int JSize,
				   int *IEnd,
				   int *JEnd)
{
    int i, j,
	ILast = ISize - 1,
	JLast = JSize - 1,
	BaseIndex = IStart * JSize + JStart,
	BaseIndex1 = BaseIndex + 1,
	BaseIndexV = BaseIndex + JSize,
	BaseIndexV1 = BaseIndexV + 1;
    IrtPlnType Pln;
    CagdPtStruct *Pt5, *Pt6, *PtTmp,
	*Pt1 = &PtMesh[BaseIndex],          /* Cache the four flat corners. */
	*Pt2 = &PtMesh[BaseIndex1],
	*Pt3 = &PtMesh[BaseIndexV1],
	*Pt4 = &PtMesh[BaseIndexV];

    if (!GMPlaneFrom3Points(Pln, Pt1 -> Pt, Pt2 -> Pt, Pt3 -> Pt)) {
	*IEnd = IStart;
	*JEnd = JStart;
	return;
    }

    /* Expand along the first direction. */
    Pt5 = Pt3;
    Pt6 = Pt4;
    for (i = IStart + 1; i < ILast; i++) {
	Pt5 += JSize;
	Pt6 += JSize;

	if (!GridMask[i * JSize + JStart] &&
	    CAGD_PT_IN_PLANE(Pln, Pt5 -> Pt) &&
	    CAGD_PT_IN_PLANE(Pln, Pt6 -> Pt) &&
	    GMCollinear3Pts(Pt1 -> Pt, Pt4 -> Pt, Pt6 -> Pt) &&
	    GMCollinear3Pts(Pt2 -> Pt, Pt3 -> Pt, Pt5 -> Pt)) {
	    /* Still coplanar - continue. */
	}
	else {
	    i--;
	    Pt5 -= JSize;
	    Pt6 -= JSize;
	    break;
	}
    }
    *IEnd = i < ILast ? i : ILast - 1;
    Pt3 = Pt5; /* Update the domain. */
    Pt4 = Pt6;

    /* Expand along the second direction. */
    Pt5 = Pt2;
    Pt6 = Pt3;
    for (j = JStart + 1; j < JLast; j++) {
	Pt5++;
	Pt6++;

	for (i = IStart, PtTmp = Pt5; i <= *IEnd; i++, PtTmp += JSize)
	    if (GridMask[i * JSize + j] ||
		!CAGD_PT_IN_PLANE(Pln, PtTmp -> Pt))
	        break;
	if (i <= *IEnd) {
	    j--;
	    break;
	}

	if (CAGD_PT_IN_PLANE(Pln, Pt5 -> Pt) &&
	    CAGD_PT_IN_PLANE(Pln, Pt6 -> Pt) &&
	    GMCollinear3Pts(Pt1 -> Pt, Pt2 -> Pt, Pt5 -> Pt) &&
	    GMCollinear3Pts(Pt4 -> Pt, Pt3 -> Pt, Pt6 -> Pt)) {
	    /* Still coplanar - continue. */
	}
	else {
	    j--;
	    break;
	}
    }
    *JEnd = j < JLast ? j : JLast - 1;

#   ifdef DEBUG
    {
	IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintExpandDomain, FALSE) {
	    IRIT_INFO_MSG_PRINTF("From %d %d to %d %d\n",
				    IStart, JStart, *IEnd, *JEnd);
	}
    }
#   endif /* DEBUG */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of CagdSrf2PolygonsGenPolys for polygon strips.       *
*****************************************************************************/
static CagdPolygonStruct *CagdSrf2PolygonsGenPolysStrips(const CagdSrfStruct
							                  *Srf,
							 CagdBType FourPerFlat,
							 CagdRType *PtWeights,
							 CagdPtStruct *PtMesh,
							 CagdVecStruct *PtNrml,
							 CagdUVStruct *UVMesh,
							 int FineNessU,
							 int FineNessV)
{
    int i, j,
	FineNessU1 = FineNessU - 1,
	FineNessV1 = FineNessV - 1,
	MeshSize = FineNessU * FineNessV;
    CagdBType
	ComputeNormals = PtNrml != NULL,
	ComputeUV = UVMesh != NULL;
    CagdPtStruct PtCenter, *Pt1, *Pt2, *Pt3, *Pt4;
    CagdUVStruct UVCenter,
	*UV1 = NULL,
	*UV2 = NULL,
	*UV3 = NULL,
	*UV4 = NULL;
    CagdVecStruct NlCenter,
	*Nl1 = NULL,
	*Nl2 = NULL,
	*Nl3 = NULL,
	*Nl4 = NULL;
    CagdPolygonStruct *Poly2,
	*Poly = NULL,
	*PolyHead = NULL;

    /* Now that we have the mesh, create the polygons. */
    for (i = 0; i < FineNessU1; i++) {
	if (!FourPerFlat) {
	    Poly = CagdPolygonStripNew(FineNessV1 * 2);
	    Poly -> U.PolyStrip.NumOfPolys = FineNessV1 * 2;
	}

	for (j = 0; j < FineNessV1; j++) {
	    int BaseIndex = i * FineNessV + j,
		BaseIndex1 = BaseIndex + 1,
		BaseIndexV = BaseIndex + FineNessV,
		BaseIndexV1 = BaseIndexV + 1;

	    Pt1 = &PtMesh[BaseIndex];        /* Cache the four flat corners. */
	    Pt2 = &PtMesh[BaseIndex1];
	    Pt3 = &PtMesh[BaseIndexV1];
	    Pt4 = &PtMesh[BaseIndexV];

	    if (ComputeNormals) {
		Nl1 = &PtNrml[BaseIndex];
		Nl2 = &PtNrml[BaseIndex1];
		Nl3 = &PtNrml[BaseIndexV1];
		Nl4 = &PtNrml[BaseIndexV];
	    }
	    if (ComputeUV) {
		UV1 = &UVMesh[BaseIndex];
		UV2 = &UVMesh[BaseIndex1];
		UV3 = &UVMesh[BaseIndexV1];
		UV4 = &UVMesh[BaseIndexV];
	    }

	    if (FourPerFlat) { /* Eval mid point and create 4 triangles. */
	        CAGD_COPY_POINT(PtCenter, *Pt1);
		CAGD_ADD_POINT(PtCenter, *Pt2);
		CAGD_ADD_POINT(PtCenter, *Pt3);
		CAGD_ADD_POINT(PtCenter, *Pt4);
		CAGD_MULT_POINT(PtCenter, 0.25);

		Poly = CagdPolygonStripNew(2);
		Poly2 = CagdPolygonStripNew(2);
		Poly -> U.PolyStrip.NumOfPolys =
		    Poly2 -> U.PolyStrip.NumOfPolys = 2;

		IRIT_PT_COPY(Poly -> U.PolyStrip.FirstPt[0], Pt1 -> Pt);
		IRIT_PT_COPY(Poly -> U.PolyStrip.FirstPt[1], Pt2 -> Pt);
		IRIT_PT_COPY(Poly -> U.PolyStrip.StripPt[0], PtCenter.Pt);
		IRIT_PT_COPY(Poly -> U.PolyStrip.StripPt[1], Pt3 -> Pt);

		IRIT_PT_COPY(Poly2 -> U.PolyStrip.FirstPt[0], Pt3 -> Pt);
		IRIT_PT_COPY(Poly2 -> U.PolyStrip.FirstPt[1], Pt4 -> Pt);
		IRIT_PT_COPY(Poly2 -> U.PolyStrip.StripPt[0], PtCenter.Pt);
		IRIT_PT_COPY(Poly2 -> U.PolyStrip.StripPt[1], Pt1 -> Pt);

		if (ComputeNormals) {
		    /* Average the four normals to find the middle one. */
		    CAGD_COPY_VECTOR(NlCenter, *Nl1);
		    CAGD_ADD_VECTOR(NlCenter, *Nl2);
		    CAGD_ADD_VECTOR(NlCenter, *Nl3);
		    CAGD_ADD_VECTOR(NlCenter, *Nl4);
		    CAGD_NORMALIZE_VECTOR(NlCenter);

		    IRIT_PT_COPY(Poly -> U.PolyStrip.FirstNrml[0], Nl1 -> Vec);
		    IRIT_PT_COPY(Poly -> U.PolyStrip.FirstNrml[1], Nl2 -> Vec);
		    IRIT_PT_COPY(Poly -> U.PolyStrip.StripNrml[0], NlCenter.Vec);
		    IRIT_PT_COPY(Poly -> U.PolyStrip.StripNrml[1], Nl3 -> Vec);

		    IRIT_PT_COPY(Poly2 -> U.PolyStrip.FirstNrml[0], Nl3 -> Vec);
		    IRIT_PT_COPY(Poly2 -> U.PolyStrip.FirstNrml[1], Nl4 -> Vec);
		    IRIT_PT_COPY(Poly2 -> U.PolyStrip.StripNrml[0], NlCenter.Vec);
		    IRIT_PT_COPY(Poly2 -> U.PolyStrip.StripNrml[1], Nl1 -> Vec);
		}

		if (ComputeUV) {
		    UVCenter.UV[0] = (UV1 -> UV[0] + UV2 -> UV[0] +
				      UV3 -> UV[0] + UV4 -> UV[0]) * 0.25;
		    UVCenter.UV[1] = (UV1 -> UV[1] + UV2 -> UV[1] +
				      UV3 -> UV[1] + UV4 -> UV[1]) * 0.25;

		    IRIT_UV_COPY(Poly -> U.PolyStrip.FirstUV[0], UV1 -> UV);
		    IRIT_UV_COPY(Poly -> U.PolyStrip.FirstUV[1], UV2 -> UV);
		    IRIT_UV_COPY(Poly -> U.PolyStrip.StripUV[0], UVCenter.UV);
		    IRIT_UV_COPY(Poly -> U.PolyStrip.StripUV[1], UV3 -> UV);

		    IRIT_UV_COPY(Poly2 -> U.PolyStrip.FirstUV[0], UV3 -> UV);
		    IRIT_UV_COPY(Poly2 -> U.PolyStrip.FirstUV[1], UV4 -> UV);
		    IRIT_UV_COPY(Poly2 -> U.PolyStrip.StripUV[0], UVCenter.UV);
		    IRIT_UV_COPY(Poly2 -> U.PolyStrip.StripUV[1], UV1 -> UV);
		}

		IRIT_LIST_PUSH(Poly, PolyHead);
		IRIT_LIST_PUSH(Poly2, PolyHead);
	    }
	    else {			   /* Only two along the diagonal... */
	        int j2 = j * 2,
		    j3 = j2 + 1;

		if (j == 0) {           /* Needs to update the strip's base. */
		    IRIT_PT_COPY(Poly -> U.PolyStrip.FirstPt[0], Pt4 -> Pt);
		    IRIT_PT_COPY(Poly -> U.PolyStrip.FirstPt[1], Pt1 -> Pt);
		}

		IRIT_PT_COPY(Poly -> U.PolyStrip.StripPt[j2], Pt3 -> Pt);
		IRIT_PT_COPY(Poly -> U.PolyStrip.StripPt[j3], Pt2 -> Pt);

		if (ComputeNormals) {
		    if (j == 0) {       /* Needs to update the strip's base. */
		      IRIT_PT_COPY(Poly -> U.PolyStrip.FirstNrml[0], Nl4 -> Vec);
		      IRIT_PT_COPY(Poly -> U.PolyStrip.FirstNrml[1], Nl1 -> Vec);
		    }

		    IRIT_PT_COPY(Poly -> U.PolyStrip.StripNrml[j2], Nl3 -> Vec);
		    IRIT_PT_COPY(Poly -> U.PolyStrip.StripNrml[j3], Nl2 -> Vec);
		}

		if (ComputeUV) {
		    if (j == 0) {	/* Needs to update the strip's base. */
		        IRIT_UV_COPY(Poly -> U.PolyStrip.FirstUV[0], UV4 -> UV);
			IRIT_UV_COPY(Poly -> U.PolyStrip.FirstUV[1], UV1 -> UV);
		    }

		    IRIT_UV_COPY(Poly -> U.PolyStrip.StripUV[j2], UV3 -> UV);
		    IRIT_UV_COPY(Poly -> U.PolyStrip.StripUV[j3], UV2 -> UV);
		}
	    }
	}

	if (!FourPerFlat)
	    IRIT_LIST_PUSH(Poly, PolyHead);
    }

    CagdPtArrayFree(PtMesh, MeshSize);
    if (ComputeNormals)
	CagdVecArrayFree(PtNrml, MeshSize);
    if (ComputeUV)
	CagdUVArrayFree(UVMesh, MeshSize);

    return PolyHead;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to uniformly sample a single Bezier srf as a grid.               M
* FineNess is a fineness control on the result and the larger it is, more    M
* samples may result. A value of 10 is a good starting                       M
* value.  FALSE is returned in case of an error, TRUE otherwise.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To sample in a grid.                                   M
*   FineNess:         Control on accuracy, the higher the finer.             M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*   PtWeights:        Weights of the evaluations, if rational, to detect     M
*		      poles.  NULL if surface not rational.		     M
*   PtMesh:	      Evaluted positions of grid of samples.                 M
*   PtNrml:	      Evaluted normals of grid of samples or NULL if none.   M
*   UVMesh:	      Evaluted UV vals of grid of samples or NULL if none.   M
*   FineNessU, FineNessV:  Actual size of PtMesh, PtNrml, UVMesh.	     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:              FALSE is returned in case of an error, TRUE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2Polygons, IritSurface2Polygons, IritTrimSrf2Polygons,             M
*   CagdSrf2Polygons, TrimSrf2Polygons, BzrSrf2PolygonsSamplesNuNv           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrf2PolygonsSamples, polygonization, surface approximation            M
*****************************************************************************/
int BzrSrf2PolygonsSamples(const CagdSrfStruct *Srf,
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
    CagdRType t, RealFineNessU, RealFineNessV;

    if (!CAGD_IS_BEZIER_SRF(Srf))
	return FALSE;

    /* Simple heuristic to estimate how many samples to compute. */
    RealFineNessU = Srf -> UOrder * FineNess / 10.0;
    RealFineNessV = Srf -> VOrder * FineNess / 10.0;

    t = AttrGetRealAttrib(Srf -> Attr, "u_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        RealFineNessU *= t;
    t = AttrGetRealAttrib(Srf -> Attr, "v_resolution");
    if (!IP_ATTR_IS_BAD_REAL(t))
        RealFineNessV *= t;

    *FineNessU = (int) IRIT_BOUND(RealFineNessU, 2, CAGD_MAX_FINENESS);
    *FineNessV = (int) IRIT_BOUND(RealFineNessV, 2, CAGD_MAX_FINENESS);

    switch (_CagdLin2Poly) {
	case CAGD_REG_POLY_PER_LIN:
	    break;
    	case CAGD_ONE_POLY_PER_LIN:
	    if (Srf -> UOrder == 2)
		*FineNessU = 2;
	    if (Srf -> VOrder == 2)
		*FineNessV = 2;
	    break;
    	case CAGD_ONE_POLY_PER_COLIN:
	    break;
    }

    return BzrSrf2PolygonsSamplesNuNv(Srf, *FineNessU, *FineNessV,
				      ComputeNormals, ComputeUV,
				      PtWeights, PtMesh, PtNrml, UVMesh);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to uniformly sample a single Bezier srf as a grid.               M
*   Nu and Nv fix the grid's sizes.					     M
*   FALSE is returned in case of an error, TRUE otherwise.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To sample in a grid.                                   M
*   Nu, Nv:           The number of uniform samples in U and V of surface.   M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*   PtWeights:        Weights of the evaluations, if rational, to detect     M
*		      poles.  NULL if surface not rational.		     M
*   PtMesh:	      Evaluted positions of grid of samples.                 M
*   PtNrml:	      Evaluted normals of grid of samples or NULL if none.   M
*   UVMesh:	      Evaluted UV vals of grid of samples or NULL if none.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:              FALSE is returned in case of an error, TRUE otherwise. M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrf2Polygons, IritSurface2Polygons, IritTrimSrf2Polygons,             M
*   CagdSrf2Polygons, TrimSrf2Polygons, BzrSrf2PolygonsSamples               M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrf2PolygonsSamplesNuNv                                               M
*****************************************************************************/
int BzrSrf2PolygonsSamplesNuNv(const CagdSrfStruct *Srf,
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
    CagdRType *PtWeightsPtr, *Pt;
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
	    *Crv = BzrSrfCrvFromSrf(Srf, ((CagdRType) i) / FineNessU1,
				    CAGD_CONST_U_DIR);

	for (j = 0; j < Nv; j++) {
	    Pt = BzrCrvEvalAtParam(Crv, ((CagdRType) j) / FineNessV1);
	    CagdCoerceToE3(PtMeshPtr -> Pt, &Pt, -1, PType);
	    PtMeshPtr++;

	    if (IsRational)
	        *PtWeightsPtr++ = Pt[0];
	}

	CagdCrvFree(Crv);
    }

    if (ComputeNormals) {
	*PtNrml = BzrSrfMeshNormals(Srf, Nu, Nv);
    }

    if (ComputeUV) {
        CagdRType u, v,
	    du = 1.0 / FineNessU1,
	    dv = 1.0 / FineNessV1;

	UVMeshPtr = (*UVMesh) = CagdUVArrayNew(MeshSize);

	for (i = 0, u = 0.0; i <= FineNessU1; i++, u += du) {
	    if (u > 1.0)                /* Due to floating point round off. */
	        u = 1.0;

	    for (j = 0, v = 0.0; j <= FineNessV1; j++, v += dv) {
	        if (v > 1.0)        /* Due to floating point round off. */
		    v = 1.0;

		UVMeshPtr -> UV[0] = u;
		UVMeshPtr++ -> UV[1] = v;
	    }
	}
    }

    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single Bezier surface to NumOfIsolines polylines    M
* in each parametric direction with SamplesPerCurve in each isoparametric    M
* curve.                                                                     M
*   Polyline are always E3 of CagdPolylineStruct type.			     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of                  M
* CagdPolylineStruct. 							     M
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
*   BzrCrv2Polyline, BspSrf2Polylines, IritSurface2Polylines,                M
*   IritTrimSrf2Polylines, SymbSrf2Polylines, TrimSrf2Polylines              M
*   CagdSrf2Polylines							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrf2Polylines, polylines, isoparametric curves                        M
*****************************************************************************/
CagdPolylineStruct *BzrSrf2Polylines(const CagdSrfStruct *Srf,
				     int NumOfIsocurves[2],
				     int SamplesPerCurve)
{
    int i;
    CagdRType t;
    CagdCrvStruct *Crv;
    CagdPolylineStruct *Poly,
	*PolyList = NULL;

    if (!CAGD_IS_BEZIER_SRF(Srf))
	return NULL;

    /* Make sure requested format is something reasonable. */
    if (SamplesPerCurve < 2)
	SamplesPerCurve = 2;

    if (NumOfIsocurves[0] < 0)
	NumOfIsocurves[0] = 0;
    if (NumOfIsocurves[1] < 0)
	NumOfIsocurves[1] = 0;

    for (i = 0; i < NumOfIsocurves[0]; i++) {
	t = ((CagdRType) i) / (NumOfIsocurves[0] - 1);
	if (t > 1.0)
	    t = 1.0;		   	      /* In case of round off error. */

	Crv = BzrSrfCrvFromSrf(Srf, t, CAGD_CONST_U_DIR);
	Poly = BzrCrv2Polyline(Crv, SamplesPerCurve);
	AttrSetRealAttrib(&Poly -> Attr, "UIsoParam", t);
	Poly -> Pnext = PolyList;
	PolyList = Poly;
	CagdCrvFree(Crv);
    }

    for (i = 0; i < NumOfIsocurves[1]; i++) {
	t = ((CagdRType) i) / (NumOfIsocurves[1] - 1);
	if (t > 1.0)
	    t = 1.0;		   	      /* In case of round off error. */

	Crv = BzrSrfCrvFromSrf(Srf, t, CAGD_CONST_V_DIR);
	Poly = BzrCrv2Polyline(Crv, SamplesPerCurve);
	AttrSetRealAttrib(&Poly -> Attr, "VIsoParam", t);
	Poly -> Pnext = PolyList;
	PolyList = Poly;
	CagdCrvFree(Crv);
    }

    return PolyList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to extract from a bezier surface NumOfIsoline isocurve list      M
* in each param. direction.						     M
*   Iso parametric curves are sampled equally spaced in parametric space.    M
*   NULL is returned in case of an error, otherwise list of CagdCrvStruct.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:             To extract isoparametric curves from.                   M
*   NumOfIsocurves:  In reach (U or V) direction                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:  List of extracted isoparametric curves. These curves   M
*                     inherit the order and continuity of the original Srf.  M
*                     NULL is returned in case of an error.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrSrf22Polylines, BspSrf2PCurves, SymbSrf2Curves                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrSrf2Curves, curves, isoparametric curves                              M
*****************************************************************************/
CagdCrvStruct *BzrSrf2Curves(const CagdSrfStruct *Srf, int NumOfIsocurves[2])
{
    int i;
    CagdRType t;
    CagdCrvStruct *Crv,
	*CrvList = NULL;

    if (!CAGD_IS_BEZIER_SRF(Srf))
	return NULL;

    /* Make sure requested format is something reasonable. */
    if (NumOfIsocurves[0] < 0)
	NumOfIsocurves[0] = 0;
    if (NumOfIsocurves[1] < 0)
	NumOfIsocurves[1] = 0;

    for (i = 0; i < NumOfIsocurves[0]; i++) {
	t = ((CagdRType) i) / (NumOfIsocurves[0] - 1);
	if (t > 1.0)
	    t = 1.0;		   	      /* In case of round off error. */

	Crv = CagdCrvFromSrf(Srf, t, CAGD_CONST_U_DIR);
	AttrSetRealAttrib(&Crv -> Attr, "UIsoParam", t);
	Crv -> Pnext = CrvList;
	CrvList = Crv;
    }

    for (i = 0; i < NumOfIsocurves[1]; i++) {
	t = ((CagdRType) i) / (NumOfIsocurves[1] - 1);
	if (t > 1.0)
	    t = 1.0;		   	      /* In case of round off error. */

	Crv = CagdCrvFromSrf(Srf, t, CAGD_CONST_V_DIR);
	AttrSetRealAttrib(&Crv -> Attr, "VIsoParam", t);
	Crv -> Pnext = CrvList;
	CrvList = Crv;
    }

    return CrvList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to approx. a single Bezier curve as a polyline with		     M
* SamplesPerCurve samples. Polyline is always E3 CagdPolylineStruct type.    M
*   Curve is sampled equally spaced in parametric space.                     M
*   NULL is returned in case of an error, otherwise CagdPolylineStruct.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:              To approximate as a polyline.                          M
*   SamplesPerCurve:  Number of samples to approximate with.                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A polyline representing the piecewise linear      M
*                          approximation from, or NULL in case of an error.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspCrv2Polyline, BzrSrf2Polylines, IritCurve2Polylines,                  M
*   SymbCrv2Polyline						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   BzrCrv2Polyline, piecewise linear approximation, polyline                M
*****************************************************************************/
CagdPolylineStruct *BzrCrv2Polyline(const CagdCrvStruct *Crv,
				    int SamplesPerCurve)
{
    IRIT_STATIC_DATA CagdRType
	*Polyline[CAGD_MAX_PT_SIZE] = { NULL };
    IRIT_STATIC_DATA int
	PolylineLen = 0;
    int i,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdPolylineStruct *PList;

    if (!CAGD_IS_BEZIER_CRV(Crv))
	return NULL;

    /* Make sure requested format is something reasonable. */
    if (SamplesPerCurve < 2 || Crv -> Order == 2)
	SamplesPerCurve = 2;

    /* Allocate temporary memory to hold evaluated curve. */
    if (PolylineLen < SamplesPerCurve) {
	if (PolylineLen > 0) {
	    for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
		IritFree(Polyline[i]);
	}

	for (i = 0; i < CAGD_MAX_PT_SIZE; i++)
	    Polyline[i] = (CagdRType *)
		IritMalloc(sizeof(CagdRType) * SamplesPerCurve);
	PolylineLen = SamplesPerCurve;
    }

    if (MaxCoord > 3)
	MaxCoord = 3;

    BzrCrvEvalToPolyline(Crv, SamplesPerCurve, Polyline);
    PList = CagdPtPolyline2E3Polyline(Polyline, SamplesPerCurve,
				      MaxCoord, CAGD_IS_RATIONAL_CRV(Crv));

    return PList;
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
*   Polyline:         A vector of evaluations with MaxCoord each evaluation. M
*   n:		      Number of evaluation (length of Polyline).	     M
*   MaxCoord: 	      Number of coordinates in the polyline Polyline.	     M
*   IsRational:       TRUE, if original curve was rational, FALSE otherwise. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolylineStruct *:  A list of E3 polylines representing the piecewise M
*                          linear approximation.  Typically, only one        M
*			   polyline, unless the (rational) curve has poles.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BzrCrv2Polyline, BspSrf2Polylines, IritCurve2Polylines,                  M
*   SymbCrv2Polyline						             M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdPtPolyline2E3Polyline, piecewise linear approximation, polyline      M
*****************************************************************************/
CagdPolylineStruct *CagdPtPolyline2E3Polyline(
				CagdRType * const Polyline[CAGD_MAX_PT_SIZE],
				int n,
				int MaxCoord,
				int IsRational)
{
    int i, j;
    CagdPolylineStruct
	*P = CagdPolylineNew(n),
	*PList = NULL;
    CagdPolylnStruct
	*NewPolyline = P -> Polyline;


    if (IsRational) {
        const CagdRType
	    *Wgts = Polyline[0];

	for (i = n - 1; i >= 0; ) {	          /* Convert to E3 polyline. */
	    int k, l;

	    /* Testing for the existance of poles, if rational. */
	    for (l = i; l >= 0; l--) { /* Make sure all weight of same sign. */
	        if (_CagdClipPolysAtPoles && Wgts[i] * Wgts[l] < 0)
		    break;      /* Break polyline if weights do change sign. */
	    }
	    P -> Length = i - l;

	    for (k = i - l - 1; i > l; i--, k--) {
	        CagdRType
		    PtW = Wgts[i] == 0 ? IRIT_UEPS : Wgts[i];

	        for (j = 0; j < MaxCoord; j++)
		    NewPolyline[k].Pt[j] = Polyline[j + 1][i] / PtW;
		for (j = MaxCoord; j < 3; j++)
		    NewPolyline[k].Pt[j] = 0.0;
	    }

	    IRIT_LIST_PUSH(P, PList);

	    if (i >= 0) {
	        P = CagdPolylineNew(n);
		NewPolyline = P -> Polyline;
	    }
	}
    }
    else {
	for (i = n - 1; i >= 0; i--) {	          /* Convert to E3 polyline. */
	    for (j = 0; j < MaxCoord; j++)
		NewPolyline[i].Pt[j] = Polyline[j + 1][i];
	    for (j = MaxCoord; j < 3; j++)
		NewPolyline[i].Pt[j] = 0.0;
	}
	PList = P;
    }

    return PList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Estimate a relative surface curveness measure in U and V (no twist       M
* consideration).  A flat surface (or a bilinear) would return two zeros.    M
*   A highly curved surface would return values near one.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         To consider.                                                M
*   UCurveness:  The surface curveness in the U direction.                   M
*   VCurveness:  The surface curveness in the V direction.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfEstimateCurveness                                                 M
*****************************************************************************/
void CagdSrfEstimateCurveness(const CagdSrfStruct *Srf,
			      CagdRType *UCurveness,
			      CagdRType *VCurveness)
{
    int i, j, l,
	ULength = Srf -> ULength,
	VLength = Srf -> VLength;
    CagdRType R;
    CagdRType
	* const *Points = Srf -> Points;
    CagdVType Vec1, Vec2, Vec3, V;
    CagdSrfStruct *CpSrf;

    if (CAGD_IS_RATIONAL_SRF(Srf))
	Srf = CpSrf = CagdCoerceSrfTo(Srf, CAGD_PT_E3_TYPE, FALSE);
    else
        CpSrf = NULL;

    if (VLength < 3)
	*VCurveness = 0.01;
    else {
	for (R = 0.0, i = 0; i < ULength; i++) {
	    for (j = 2; j < VLength; j++) {
		IrtRType d;

		for (l = 1; l <= 3; l++) {
		    Vec1[l - 1] = Points[l][SRF(i, j - 1)] -
			          Points[l][SRF(i, j - 2)];
		    Vec2[l - 1] = Points[l][SRF(i, j)] -
			          Points[l][SRF(i, j - 1)];
		}
		IRIT_VEC_ADD(Vec3, Vec1, Vec2);

		IRIT_CROSS_PROD(V, Vec1, Vec2);
		d = IRIT_VEC_LENGTH(Vec3) + IRIT_UEPS;
		R += IRIT_VEC_LENGTH(V) / IRIT_SQR(d);
	    }
	}
	*VCurveness = IRIT_MAX(R / (ULength * (VLength - 2)), 0.01);
    }

    if (ULength < 3)
	*UCurveness = 0.1;
    else {
	for (R = 0.0, j = 0; j < VLength; j++) {
	    for (i = 2; i < ULength; i++) {
		IrtRType d;

		for (l = 1; l <= 3; l++) {
		    Vec1[l - 1] = Points[l][SRF(i - 1, j)] -
			          Points[l][SRF(i - 2, j)];
		    Vec2[l - 1] = Points[l][SRF(i, j)] -
			          Points[l][SRF(i - 1, j)];
		}
		IRIT_VEC_ADD(Vec3, Vec1, Vec2);

		IRIT_CROSS_PROD(V, Vec1, Vec2);
		d = IRIT_VEC_LENGTH(Vec3) + IRIT_UEPS;
		R += IRIT_VEC_LENGTH(V) / IRIT_SQR(d);
	    }
	}
	*UCurveness = IRIT_MAX(R / (VLength * (ULength - 2)), 0.01);
    }

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);
}
