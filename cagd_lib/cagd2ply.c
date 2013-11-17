/******************************************************************************
* Cagd2Ply.c - Surfaces to polygons adaptive conversion routines.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Mar. 2001.					      *
******************************************************************************/

#include "cagd_loc.h"
#include "geom_lib.h"

#define CAGD_RECT_ALLOC_BLOCK		100
#define CAGD_MAX_SUBDIV_ONE_DIR		2
#define CAGD_DISCONT_NORMAL_EPS 	1e-4
#define CAGD_DIV_FRACTION		0.00003010196028071611
#define CAGD_MAX_POLES_SUBDIV		(CAGD2PLY_MAX_SUBDIV_INDEX / 16)
#define CAGD_ADAP_FIX_DISCONT_NRML(SrfPt) \
		    (SrfPt -> Nrml[0] == IRIT_INFNTY ? NULL : SrfPt -> Nrml)

#ifdef DEBUG
    IRIT_SET_DEBUG_PARAMETER(_DebugDumpSubdivMinSize, FALSE);
    IRIT_STATIC_DATA int
	GlblSubdivMinSize = CAGD2PLY_MAX_SUBDIV_INDEX;
#endif /* DEBUG */

IRIT_STATIC_DATA int
    CagdSrf2PolyComputeNormals = FALSE,
    CagdSrf2PolyComputeUV = FALSE;
IRIT_STATIC_DATA CagdSrfErrorFuncType
    CagdSrfAdap2PolyErrFunc = NULL;
IRIT_STATIC_DATA CagdSrfAdapPolyGenFuncType
    CagdSrfAdapPolyGenFunc = NULL;
IRIT_STATIC_DATA CagdSrfAdapAuxDataFuncType
    CagdSrfAdapAuxDataFunc = NULL;
IRIT_STATIC_DATA CagdRType
    GlblUMin = 0.0,
    GlblUMax = 0.0,
    GlblVMin = 0.0,
    GlblVMax = 0.0,
    GlblSrfAdapTolerance = 0.01;
IRIT_STATIC_DATA CagdPolygonStruct
    *GlblPolys = NULL;
IRIT_STATIC_DATA CagdSrfAdapRectStruct
    *GlblRectFreeList = NULL,
    *GlblRectList = NULL;

static void CagdSrfAdapGetE3Pt(CagdRType *E3Point,
			       CagdRType * const Points[CAGD_MAX_PT_SIZE],
			       int Index,
			       CagdPointType PType);
static void CagdSrfAdap2PolygonsAux1(const CagdSrfStruct *Srf,
				     int UIndexBase,
				     int UIndexSize,
				     int VIndexBase,
				     int VIndexSize,
				     VoidPtr AuxSrfData);
static void CagdSrfAdap2PolygonsAux2(const CagdSrfStruct *Srf,
				     struct CagdA2PGridStruct *A2PGrid,
				     int UIndexBase,
				     int UIndexSize,
				     int VIndexBase,
				     int VIndexSize,
				     VoidPtr AuxSrfData);
static void CagdSrfAdap2PolygonsAux3(const CagdSrfStruct *Srf,
				     struct CagdA2PGridStruct *A2PGrid,
				     IrtRType SrfErr,
				     int UVBias,
				     int UIndexBase,
				     int UIndexSize,
				     int VIndexBase,
				     int VIndexSize,
				     VoidPtr AuxSrfData);
static CagdPolygonStruct *CagdSrfAdapRect2Polys(const CagdSrfStruct *Srf,
						struct CagdA2PGridStruct
						                    *A2PGrid,
						CagdSrfAdapRectStruct *Rect);
static CagdSrfAdapRectStruct *CagdSrfAdap2PolyAllocRect(int UIndexBase,
							int UIndexSize,
							int VIndexBase,
							int VIndexSize,
							VoidPtr AuxSrfData,
							CagdRType Err);
static void CagdSrfAdap2PolysFlat(const CagdSrfStruct *Srf,
				  struct CagdA2PGridStruct *A2PGrid,
				  CagdRType Err,
				  int UIndexBase,
				  int UIndexSize,
				  int VIndexBase,
				  int VIndexSize,
				  VoidPtr AuxSrfData);
static CagdPolygonStruct *CagdGenAdapTriangle(CagdBType ComputeNormals,
					      CagdBType ComputeUV,
					      const CagdRType *Pt1,
					      const CagdRType *Pt2,
					      const CagdRType *Pt3,
					      const CagdRType *Nl1,
					      const CagdRType *Nl2,
					      const CagdRType *Nl3,
					      const CagdRType *UV1,
					      const CagdRType *UV2,
					      const CagdRType *UV3,
					      CagdBType *GenPoly);
static CagdPolygonStruct *CagdGenAdapRectangle(CagdBType ComputeNormals,
					       CagdBType ComputeUV,
					       const CagdRType *Pt1,
					       const CagdRType *Pt2,
					       const CagdRType *Pt3,
					       const CagdRType *Pt4,
					       const CagdRType *Nl1,
					       const CagdRType *Nl2,
					       const CagdRType *Nl3,
					       const CagdRType *Nl4,
					       const CagdRType *UV1,
					       const CagdRType *UV2,
					       const CagdRType *UV3,
					       const CagdRType *UV4,
					       CagdBType *GenPoly);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the surface approximation error function.  The error function       M
* will return a negative value if this patch is flat enough, and positive    M
* value if flat enough.                                                      M
*   Either case, the magnitude will equal to the actual error.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfErrorFuncType:  Old value of function.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrf2PolyAdapSetAuxDataFunc,		     M
*   CagdSrf2PolyAdapSetPolyGenFunc					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolyAdapSetErrFunc                                               M
*****************************************************************************/
CagdSrfErrorFuncType CagdSrf2PolyAdapSetErrFunc(CagdSrfErrorFuncType Func)
{
    CagdSrfErrorFuncType
	OldFunc = CagdSrfAdap2PolyErrFunc;

    CagdSrfAdap2PolyErrFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the surface approximation auxiliary function.  This function        M
* will be invoked on each subdivision step during the approximation process, M
* for auxiliary processing that are application specific.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfAdapAuxDataFuncType:  Old value of function.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrf2PolyAdapSetPolyGenFunc,		     M
*   CagdSrf2PolyAdapSetErrFunc						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolyAdapSetAuxDataFunc                                           M
*****************************************************************************/
CagdSrfAdapAuxDataFuncType
	      CagdSrf2PolyAdapSetAuxDataFunc(CagdSrfAdapAuxDataFuncType Func)
{
    CagdSrfAdapAuxDataFuncType
	OldFunc = CagdSrfAdapAuxDataFunc;

    CagdSrfAdapAuxDataFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the function to convert flat surface rectangle domains into         M
* polygons.								     M
*                                                                            *
* PARAMETERS:                                                                M
*   Func:        New function to use, NULL to disable.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfAdapPolyGenFuncType:  Old value of function.                      M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrf2PolyAdapSetAuxDataFunc,		     M
*   CagdSrf2PolyAdapSetErrFunc						     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrf2PolyAdapSetPolyGenFunc                                           M
*****************************************************************************/
CagdSrfAdapPolyGenFuncType
	      CagdSrf2PolyAdapSetPolyGenFunc(CagdSrfAdapPolyGenFuncType Func)
{
    CagdSrfAdapPolyGenFuncType
	OldFunc = CagdSrfAdapPolyGenFunc;

    CagdSrfAdapPolyGenFunc = Func;

    return OldFunc;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Tolerance evaluation of flatness for given surface.  Constructs a plane  M
* from the four corner points, if possible, and measure distance to rest     M
* of control points.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Surface to test for flatness.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    Negative value if flat enough, positive if not flat.       M
*		  Either case, magnitude will equal to the actual error.     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfIsCoplanarCtlMesh, CagdSrfIsLinearBndryCtlMesh,                   M
*   CagdSrfAdap2Polygons                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfAdap2PolyDefErrFunc                                               M
*****************************************************************************/
CagdRType CagdSrfAdap2PolyDefErrFunc(const CagdSrfStruct *Srf)
{
    CagdRType R;

    /* Check if boundary curves are almost linear. */
    if ((R = CagdSrfIsLinearBndryCtlMesh(Srf)) > GlblSrfAdapTolerance)
        return R;

    /* Check if mesh is almost coplanar. */
    if ((R = CagdSrfIsCoplanarCtlMesh(Srf)) > GlblSrfAdapTolerance)
        return R;

    /* Check if mesh's rows/columns are almost linear. */
    if ((R = CagdSrfIsLinearCtlMesh(Srf, TRUE)) > GlblSrfAdapTolerance)
        return R;

    return -R;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fetches an E3 point at given Index.  Input control points are assumed    M
* E3 or P3 only. 		                                             M
*                                                                            *
* PARAMETERS:                                                                M
*   E3Point:   Where the coerced information is to besaved.                  M
*   Points:    The control points vector of the surface.		     M
*   Index:     Index into the vectors of Points.                             M
*   PType:     Point type of Srf.			                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfAdapGetE3Pt, coercion                                             M
*****************************************************************************/
static void CagdSrfAdapGetE3Pt(CagdRType *E3Point,
			       CagdRType * const Points[CAGD_MAX_PT_SIZE],
			       int Index,
			       CagdPointType PType)
{
    int i;
    CagdRType Weight;

    if (CAGD_IS_RATIONAL_PT(PType)) {
	Weight = Points[0][Index] == 0.0 ? IRIT_INFNTY 
					 : 1.0 / Points[0][Index];
	for (i = 1; i <= 3; i++)
	    *E3Point++ = Points[i][Index] * Weight;
    }
    else {
        for (i = 1; i <= 3; i++)
	    *E3Point++ = Points[i][Index];
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the linearity of the control mesh for a given surface, along    M
*one row or column.							     M
*   Constructs a line through the two end points and measure distance	     M
* to rest of control points on that row/col.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Surface to test for linearity of its control mesh's row/col.     M
*   Idx:  Of row/column.						     M
*   Dir:    A row or column specification.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:  A bound on the distance between the control points on the    M
*		row/col and the line through end points of that row/column.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrfAdap2PolyDefErrFunc                         M
*   CagdSrfIsLinearBndryCtlMesh, CagdSrfIsLinearCtlMesh			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfIsLinearCtlMeshOneRowCol                                          M
*****************************************************************************/
CagdRType CagdSrfIsLinearCtlMeshOneRowCol(const CagdSrfStruct *Srf,
					  int Idx,
					  CagdSrfDirType Dir)
{
    CagdBType LnZero;
    int i, Stride, n,
	ULen1 = Srf -> ULength - 1,
	VLen1 = Srf -> VLength - 1;
    CagdRType DistSqr,
	MaxDistSqr = 0.0;
    CagdRType
	* const *Points = Srf -> Points;
    CagdPointType
	PType = Srf -> PType;
    CagdPType Pt, Pt1, Pt2;
    CagdVType LnDir, VTmp1, VTmp2;

    /* Get the two end points, and compute a line. */
    switch (Dir) {
	case CAGD_CONST_U_DIR:
	    CagdSrfAdapGetE3Pt(Pt1, Points, CAGD_MESH_UV(Srf, Idx,     0),
			       PType);
	    CagdSrfAdapGetE3Pt(Pt2, Points, CAGD_MESH_UV(Srf, Idx, VLen1),
			       PType);
	    Idx = CAGD_MESH_UV(Srf, Idx, 1);
	    Stride = CAGD_NEXT_V(Srf);
	    n = VLen1;
	    break;
	case CAGD_CONST_V_DIR:
	    CagdSrfAdapGetE3Pt(Pt1, Points, CAGD_MESH_UV(Srf,     0, Idx),
			       PType);
	    CagdSrfAdapGetE3Pt(Pt2, Points, CAGD_MESH_UV(Srf, ULen1, Idx),
			       PType);
	    Idx = CAGD_MESH_UV(Srf, 1, Idx);
	    Stride = CAGD_NEXT_U(Srf);
	    n = ULen1;
	    break;
        default:
	    Stride = n = 0;
	    assert(0);
    }

    IRIT_PT_SUB(LnDir, Pt1, Pt2);
    LnZero = IRIT_PT_APX_EQ_ZERO_EPS(LnDir, IRIT_UEPS);
    if (!LnZero)
        IRIT_VEC_NORMALIZE(LnDir);

    for (i = 1; i < n; i++, Idx += Stride) {
        CagdSrfAdapGetE3Pt(Pt, Points, Idx, PType);
	if (LnZero) 
	    DistSqr = IRIT_PT_PT_DIST_SQR(Pt, Pt1);
	else {
	    IRIT_PT_SUB(VTmp1, Pt, Pt1);
	    IRIT_CROSS_PROD(VTmp2, VTmp1, LnDir);
	    DistSqr = IRIT_DOT_PROD(VTmp2, VTmp2);
	}
	MaxDistSqr = IRIT_MAX(MaxDistSqr, DistSqr);
    }

    return sqrt(MaxDistSqr);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the linearity of the control mesh for a given surface.	     M
*   Constructs a line for each row/col, if possible, and measure distance    M
* to rest of control points on that row/col.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to test for linearity of its control mesh's boundary. M
*   Interior:  TRUE to handle interior rows/columns only.  FALSE to check    M
*              Boundary as well.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    A bound on the distance between the control mesh boundary  M
*		  and a linear rectangle connecting the four corners.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrfAdap2PolyDefErrFunc                         M
*   CagdSrfIsLinearBndryCtlMesh, CagdSrfIsLinearCtlMeshOneRowCol	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfIsLinearCtlMesh                                                   M
*****************************************************************************/
CagdRType CagdSrfIsLinearCtlMesh(const CagdSrfStruct *Srf, CagdBType Interior)
{
    int i,
	UMax = Interior ? Srf -> ULength - 1 : Srf -> ULength,
	VMax = Interior ? Srf -> VLength - 1 : Srf -> VLength;
    CagdRType Dist,
	MaxDist = 0.0;

    for (i = Interior ? 1 : 0; i < UMax; i++) {
        Dist = CagdSrfIsLinearCtlMeshOneRowCol(Srf, i, CAGD_CONST_U_DIR);
	MaxDist = IRIT_MAX(MaxDist, Dist);
    }

    for (i = Interior ? 1 : 0; i < VMax; i++) {
        Dist = CagdSrfIsLinearCtlMeshOneRowCol(Srf, i, CAGD_CONST_V_DIR);
	MaxDist = IRIT_MAX(MaxDist, Dist);
    }

    return MaxDist;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Evaluate the linearity of the boundary of the control mesh for a given   M
* surface.							             M
*   Constructs a line for each boundary, if possible, and measure distance   M
* to rest of control points on that boundary.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Surface to test for linearity of of its control mesh's boundary. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    A bound on the distance between the control mesh boundary  M
*		  and a linear rectangle connecting the four corners.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrfAdap2PolyDefErrFunc                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfIsLinearBndryCtlMesh                                              M
*****************************************************************************/
CagdRType CagdSrfIsLinearBndryCtlMesh(const CagdSrfStruct *Srf)
{
    CagdRType R1, R2, R3, R4, R12, R34;

    R1 = CagdSrfIsLinearCtlMeshOneRowCol(Srf, 0, CAGD_CONST_U_DIR);
    R2 = CagdSrfIsLinearCtlMeshOneRowCol(Srf, Srf -> ULength - 1,
					 CAGD_CONST_U_DIR);
    R3 = CagdSrfIsLinearCtlMeshOneRowCol(Srf, 0, CAGD_CONST_V_DIR);
    R4 = CagdSrfIsLinearCtlMeshOneRowCol(Srf, Srf -> VLength - 1,
					 CAGD_CONST_V_DIR);

    R12 = IRIT_MAX(R1, R2);
    R34 = IRIT_MAX(R3, R4);

    return IRIT_MAX(R12, R34);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   evaluate the coplanarity of the control mesh for a given surface.        M
* Constructs a plane from the four corner points, if possible, and measure   M
* distance to rest of control points.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:    Surface to test for flatness of its control mesh.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType:    A bound on the distance between the control points and the M
*		  plane fitted to the four corners.			     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrfAdap2PolyDefErrFunc                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfIsCoplanarCtlMesh                                                 M
*****************************************************************************/
CagdRType CagdSrfIsCoplanarCtlMesh(const CagdSrfStruct *Srf)
{
    int i,
	ULen = Srf -> ULength,
	VLen = Srf -> VLength;
    CagdPointType
	PType = Srf -> PType;
    CagdPType P, P11, P12, P21, P22;
    CagdVType V, V1, V2, V3, V4;
    CagdRType
	* const *Points = Srf -> Points,
	*PtsW = Points[0],
	*PtsX = Points[1],
	*PtsY = Points[2],
	*PtsZ = Points[3];
    CagdRType PlnX, PlnY, PlnZ, PlnW,
	ErrPos = 0.0,
	ErrNeg = 0.0;
    IrtPlnType Pln;

    /* Get the four corner points. */
    CagdSrfAdapGetE3Pt(P11, Points, CAGD_MESH_UV(Srf, 0,        0), PType);
    CagdSrfAdapGetE3Pt(P12, Points, CAGD_MESH_UV(Srf, ULen - 1, 0), PType);
    CagdSrfAdapGetE3Pt(P21, Points, CAGD_MESH_UV(Srf, 0,        VLen - 1),
		       PType);
    CagdSrfAdapGetE3Pt(P22, Points, CAGD_MESH_UV(Srf, ULen - 1, VLen - 1),
		       PType);

    /* Construct the plane. */
    IRIT_VEC_SUB(V1, P21, P11);
    IRIT_VEC_SUB(V2, P11, P12);
    IRIT_VEC_SUB(V3, P12, P22);
    IRIT_VEC_SUB(V4, P22, P21);

    IRIT_CROSS_PROD(Pln, V1, V2);
    IRIT_CROSS_PROD(V, V2, V3);
    IRIT_VEC_ADD(Pln, Pln, V);
    IRIT_CROSS_PROD(V, V3, V4);
    IRIT_VEC_ADD(Pln, Pln, V);
    IRIT_CROSS_PROD(V, V4, V1);
    IRIT_VEC_ADD(Pln, Pln, V);
    if (IRIT_PT_EQ_ZERO(Pln))
        return 0.0;	      /* Boundary is shared or surface is singular. */
    IRIT_VEC_NORMALIZE(Pln);

    IRIT_PT_COPY(P, P11);
    IRIT_PT_ADD(P, P, P12);
    IRIT_PT_ADD(P, P, P21);
    IRIT_PT_ADD(P, P, P22);
    IRIT_PT_SCALE(P, 0.25);

    PlnX = Pln[0];
    PlnY = Pln[1];
    PlnZ = Pln[2];
    PlnW = -IRIT_DOT_PROD(Pln, P);

    for (i = ULen * VLen; i > 0 ; i--) {
        CagdRType d, w;

	switch (PType) {
	    case CAGD_PT_E3_TYPE:
	        d = *PtsX++ * PlnX +
		    *PtsY++ * PlnY +
		    *PtsZ++ * PlnZ + PlnW;
		break;
	    case CAGD_PT_P3_TYPE:
		if ((w = *PtsW++) == 0.0)
		    w = IRIT_UEPS;
	        d = (*PtsX++ * PlnX +
		     *PtsY++ * PlnY +
		     *PtsZ++ * PlnZ) / w + PlnW;
		break;
	    default:
		d = 0;
	        /* We should get only E3/P3 points at this time. */
	        assert(0);
	}

	if (d > 0)
	    ErrPos = IRIT_MAX(ErrPos, d);
	else
	    ErrNeg = IRIT_MIN(ErrNeg, d);
    }

    /* Inflection areas will show up as both negative and positive areas.    */
    /* Penalize more such areas by adding up positive and negative errors.   */
    ErrPos -= ErrNeg;

    return ErrPos;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to convert a single surface to set of polygons approximating     M
* it.  									     M
*    Tolerance is a tolerance control on result, typically related to the    M
* the accuracy of the apporximation. A value of 0.1 is a good rough start.   M
*   NULL is returned in case of an error or use of call back function to get M
* a hold over the created polygons, otherwise list of CagdPolygonStruct.     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:              To approximate into triangles.                         M
*   Tolerance:        of approximation - a value that depends on the error   M
*		      function used.					     M
*   ComputeNormals:   If TRUE, normal information is also computed.          M
*   FourPerFlat:      If TRUE, four triangles are created per flat surface.  M
*                     If FALSE, only 2 triangles are created.                M
*   ComputeUV:        If TRUE, UV values are stored and returned as well.    M
*   AuxSrfData:       Optional data structure that will be passed to all     M
*                     subdivided sub-surfaces, or NULL if not needed.        M
*		      See also CagdSrf2PolyAdapSetAuxDataFunc.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  A list of polygons with optional normal and/or     M
*                         UV parametric information.                         M
*                         NULL is returned in case of an error or if use of  M
*			  call back function to collect the polygons.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrf2PolygonSetErrFunc, CagdSrfAdap2PolyDefErrFunc,		     M
*   CagdSrf2PolyAdapSetErrFunc, CagdSrf2PolyAdapSetAuxDataFunc		     M
*   CagdSrf2PolyAdapSetPolyGenFunc, BzrSrf2Polygons, CagdSrf2Polygons,	     M
*   CagdSrf2Polygons, TrimSrf2Polygons, BspC1Srf2Polygons, CagdSrf2Polygons  M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfAdap2Polygons, polygonization, surface approximation              M
*****************************************************************************/
CagdPolygonStruct *CagdSrfAdap2Polygons(const CagdSrfStruct *Srf,
					CagdRType Tolerance,
					CagdBType ComputeNormals,
					CagdBType FourPerFlat,
					CagdBType ComputeUV,
					VoidPtr AuxSrfData)
{
    CagdBType
	SetMakeTriRecFuncs = FALSE;
    CagdSrfStruct *TSrf, *CpSrf;
    CagdSrfMakeTriFuncType
	OldTriFunc = NULL;
    CagdSrfMakeRectFuncType
	OldRectFunc = NULL;

    CagdSrfEffiNrmlPrelude(Srf);

    if (CagdSrfAdap2PolyErrFunc == NULL)
	CagdSrfAdap2PolyErrFunc = CagdSrfAdap2PolyDefErrFunc;
    if (CagdSrfAdapPolyGenFunc == NULL)
        CagdSrfAdapPolyGenFunc = CagdSrfAdapRectPolyGen;

    if (_CagdSrfMakeTriFunc == NULL || _CagdSrfMakeRectFunc == NULL) {
	OldRectFunc = CagdSrfSetMakeRectFunc(CagdMakeRectangle);
        OldTriFunc = CagdSrfSetMakeTriFunc(CagdMakeTriangle);
	SetMakeTriRecFuncs = TRUE;
    }

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugDumpSubdivMinSize)
        GlblSubdivMinSize = CAGD2PLY_MAX_SUBDIV_INDEX;
#   endif /* DEBUG */

    CagdSrf2PolyComputeNormals = ComputeNormals;
    CagdSrf2PolyComputeUV = ComputeUV;

    if (CAGD_IS_BEZIER_SRF(Srf))
	Srf = CpSrf = CagdCnvrtBzr2BspSrf(Srf);
    else
        CpSrf = NULL;

    if (CAGD_IS_PERIODIC_SRF(Srf)) {
        TSrf = CagdCnvrtPeriodic2FloatSrf(Srf);
	if (CpSrf != NULL)
	    CagdSrfFree(CpSrf);
	Srf = CpSrf = TSrf;
    }

    if (!BspSrfHasOpenEC(Srf)) {
        TSrf = CagdCnvrtFloat2OpenSrf(Srf);
	if (CpSrf != NULL)
	    CagdSrfFree(CpSrf);
	Srf = CpSrf = TSrf;
    }

    /* Make sure the surface is E3 or P3. */
    if (!CAGD_NUM_OF_PT_COORD(Srf -> PType) != 3) {
        TSrf = CAGD_IS_RATIONAL_PT(Srf -> PType) ?
	    CagdCoerceSrfsTo(Srf, CAGD_PT_P3_TYPE, FALSE) :
	    CagdCoerceSrfsTo(Srf, CAGD_PT_E3_TYPE, FALSE);

	if (CpSrf != NULL)
	    CagdSrfFree(CpSrf);
	Srf = CpSrf = TSrf;
    }

    GlblSrfAdapTolerance = Tolerance;
    GlblPolys = NULL;

    /* Find the rectangular domains to form polygons from. */
    CagdSrfAdap2PolygonsAux1(Srf,
			     0, CAGD2PLY_MAX_SUBDIV_INDEX,
			     0, CAGD2PLY_MAX_SUBDIV_INDEX,
			     AuxSrfData);

#   ifdef DEBUG
        IRIT_IF_DEBUG_ON_PARAMETER(_DebugDumpSubdivMinSize)
	    IRIT_INFO_MSG_PRINTF("Global minimal binary subdiv size equal %d\n",
				 GlblSubdivMinSize);
#   endif /* DEBUG */

    if (CpSrf != NULL)
	CagdSrfFree(CpSrf);

    if (SetMakeTriRecFuncs) {
	CagdSrfSetMakeTriFunc(OldTriFunc);
	CagdSrfSetMakeRectFunc(OldRectFunc);
    }

    CagdSrfEffiNrmlPostlude();

    return GlblPolys;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of CagdSrfAdap2Polygons.  Assumes Bspline srfs only.  *
* Process poles, if any, in the (rational) surface.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:              Bspline surface to approximate into triangles.         *
*   UIndexBase:       2's power index of base of this U parameteric domain.  *
*   UIndexSize:       2's power size of this U parameteric domain.	     *
*   VIndexBase:       2's power index of base of this V parameteric domain.  *
*   VIndexSize:       2's power size of this V parameteric domain.	     *
*   AuxSrfData:       Optional data structure that will be passed to all     *
*                     subdivided sub-surfaces, or NULL if not needed.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void CagdSrfAdap2PolygonsAux1(const CagdSrfStruct *Srf,
				     int UIndexBase,
				     int UIndexSize,
				     int VIndexBase,
				     int VIndexSize,
				     VoidPtr AuxSrfData)
{
    void
	*AuxSrf1Data = NULL,
	*AuxSrf2Data = NULL;
    CagdSrfDirType
        PolesDivision = CAGD_NO_DIR;

    CagdSrfDomain(Srf, &GlblUMin, &GlblUMax, &GlblVMin, &GlblVMax);

    if (CAGD_IS_RATIONAL_SRF(Srf) &&
	CagdPointsHasPoles(Srf -> Points, Srf -> ULength * Srf -> VLength)) {

	if (CAGD_MAX_POLES_SUBDIV < IRIT_MAX(UIndexSize, VIndexSize))
	    PolesDivision = UIndexSize > VIndexSize ? CAGD_CONST_U_DIR
	                                            : CAGD_CONST_V_DIR;

	if (PolesDivision == CAGD_CONST_U_DIR) {
	    CagdBType
	        IgnoreFirst = FALSE,
	        IgnoreSecond = FALSE;
	    CagdRType
	        t = (GlblUMin + GlblUMax) / 2.0;
	    CagdSrfStruct
	        *Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_U_DIR),
	        *Srf2 = Srf1 -> Pnext;

	    if (CagdSrfAdapAuxDataFunc != NULL) {
	        CagdSrfAdapAuxDataFunc(Srf, AuxSrfData, t, CAGD_CONST_U_DIR,
				       Srf1, &AuxSrf1Data, Srf2, &AuxSrf2Data);

		/* If both NULL, proceed as if no AuxSrfData.  If one is    */
		/*  NULL, ignore it.					    */
		if (AuxSrf1Data == NULL && AuxSrf2Data != NULL)
		    IgnoreFirst = TRUE;
		if (AuxSrf2Data == NULL && AuxSrf1Data != NULL)
		    IgnoreSecond = TRUE;
	    }

	    if (!IgnoreFirst)
	        CagdSrfAdap2PolygonsAux1(Srf1,
					 UIndexBase, UIndexSize >> 1,
					 VIndexBase, VIndexSize, AuxSrf1Data);
	    if (!IgnoreSecond)
	        CagdSrfAdap2PolygonsAux1(Srf2,
					 UIndexBase + (UIndexSize >> 1),
					 UIndexSize >> 1, VIndexBase, VIndexSize,
					 AuxSrf2Data);
	    CagdSrfFreeList(Srf1);
	    return;
	}
	else if (PolesDivision == CAGD_CONST_V_DIR) {
	    CagdBType
	        IgnoreFirst = FALSE,
	        IgnoreSecond = FALSE;
	    CagdRType
	        t = (GlblVMin + GlblVMax) / 2.0;
	    CagdSrfStruct
	        *Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_V_DIR),
	        *Srf2 = Srf1 -> Pnext;

	    if (CagdSrfAdapAuxDataFunc != NULL) {
	        CagdSrfAdapAuxDataFunc(Srf, AuxSrfData, t, CAGD_CONST_V_DIR,
				       Srf1, &AuxSrf1Data, Srf2, &AuxSrf2Data);

		/* If both NULL, proceed as if no AuxSrfData.  If one is    */
		/*  NULL, ignore it.					    */
		if (AuxSrf1Data == NULL && AuxSrf2Data != NULL)
		    IgnoreFirst = TRUE;
		if (AuxSrf2Data == NULL && AuxSrf1Data != NULL)
		    IgnoreSecond = TRUE;
	    }

	    if (!IgnoreFirst)
		CagdSrfAdap2PolygonsAux1(Srf1, UIndexBase, UIndexSize,
				         VIndexBase, VIndexSize >> 1,
					 AuxSrf1Data);
	    if (!IgnoreSecond)
		CagdSrfAdap2PolygonsAux1(Srf2, UIndexBase, UIndexSize,
					 VIndexBase + (VIndexSize >> 1),
					 VIndexSize >> 1, AuxSrf2Data);
	    CagdSrfFreeList(Srf1);
	    return;
	}
    }

    if (PolesDivision == CAGD_NO_DIR) {
        CagdSrfAdapRectStruct *Rect;
	struct CagdA2PGridStruct
	    *A2PGrid = CagdSrfA2PGridInit(Srf);

        GlblRectList = NULL;

	CagdSrfDomain(Srf, &GlblUMin, &GlblUMax, &GlblVMin, &GlblVMax);

        CagdSrfAdap2PolygonsAux2(Srf, A2PGrid,
				 0, CAGD2PLY_MAX_SUBDIV_INDEX,
				 0, CAGD2PLY_MAX_SUBDIV_INDEX,
				 AuxSrfData);

	/* Evaluate the surface at all desired point locations. */
	CagdSrfA2PGridProcessUV(A2PGrid);

	/* Traverse rect. domains & convert to polys while closing cracks. */
	for (Rect = GlblRectList; Rect != NULL; Rect = Rect -> Pnext) {
	    GlblPolys = (CagdPolygonStruct *) 
	        CagdListAppend(GlblPolys, CagdSrfAdapRect2Polys(Srf, A2PGrid,
								Rect));
	}

	/* Chain the Rect list to the freed version. */
	if (GlblRectList != NULL) {
	    for (Rect = GlblRectList;
		 Rect -> Pnext != NULL;
		 Rect = Rect -> Pnext);
	    Rect -> Pnext = GlblRectFreeList;
	    GlblRectFreeList = GlblRectList;
	}

	CagdSrfA2PGridFree(A2PGrid);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of CagdSrfAdap2Polygons.  Assumes B-spline srfs only. *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:              B-spline surface to approximate into triangles.        *
*   A2PGrid:          Srf sampling points grid data structure.               *
*   UIndexBase:       2's power index of base of this U parametric domain.   *
*   UIndexSize:       2's power size of this U parametric domain.	     *
*   VIndexBase:       2's power index of base of this V parametric domain.   *
*   VIndexSize:       2's power size of this V parametric domain.	     *
*   AuxSrfData:       Optional data structure that will be passed to all     *
*                     subdivided sub-surfaces, or NULL if not needed.        *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void CagdSrfAdap2PolygonsAux2(const CagdSrfStruct *Srf,
				     struct CagdA2PGridStruct *A2PGrid,
				     int UIndexBase,
				     int UIndexSize,
				     int VIndexBase,
				     int VIndexSize,
				     VoidPtr AuxSrfData)
{
    void
	*AuxSrf1Data = NULL,
	*AuxSrf2Data = NULL;
    IrtRType 
	SrfErr = CagdSrfAdap2PolyErrFunc(Srf);

    if ((UIndexSize <= 2 && VIndexSize <= 2) || SrfErr <= 0.0) {
	/* Flat enough. */
        CagdSrfAdap2PolysFlat(Srf, A2PGrid, -SrfErr, UIndexBase, UIndexSize,
			      VIndexBase, VIndexSize, AuxSrfData);
    }
    else if (UIndexSize > 2 && Srf -> ULength > Srf -> UOrder) {
	CagdBType
	    IgnoreFirst = FALSE,
	    IgnoreSecond = FALSE;
	CagdRType
	    t = Srf -> UKnotVector[(Srf -> ULength + Srf -> UOrder) >> 1];
	CagdSrfStruct
	    *Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_U_DIR),
	    *Srf2 = Srf1 -> Pnext;

	if (CagdSrfAdapAuxDataFunc != NULL) {
	    CagdSrfAdapAuxDataFunc(Srf, AuxSrfData, t, CAGD_CONST_U_DIR,
				   Srf1, &AuxSrf1Data, Srf2, &AuxSrf2Data);

	    /* If both NULL, proceed as if no AuxSrfData.  If one is NULL,  */
	    /* ignore it.						    */
	    if (AuxSrf1Data == NULL && AuxSrf2Data != NULL)
		IgnoreFirst = TRUE;
	    if (AuxSrf2Data == NULL && AuxSrf1Data != NULL)
		IgnoreSecond = TRUE;
	}

	if (!IgnoreFirst)
	    CagdSrfAdap2PolygonsAux2(Srf1, A2PGrid,
				     UIndexBase, UIndexSize >> 1,
				     VIndexBase, VIndexSize, AuxSrf1Data);
	if (!IgnoreSecond)
	    CagdSrfAdap2PolygonsAux2(Srf2, A2PGrid,
				     UIndexBase + (UIndexSize >> 1),
				     UIndexSize >> 1, VIndexBase, VIndexSize,
				     AuxSrf2Data);
	CagdSrfFreeList(Srf1);
    }
    else if (VIndexSize > 2 && Srf -> VLength > Srf -> VOrder) {
        CagdBType
	    IgnoreFirst = FALSE,
	    IgnoreSecond = FALSE;
	CagdRType
	    t = Srf -> VKnotVector[(Srf -> VLength + Srf -> VOrder) >> 1];
	CagdSrfStruct
	    *Srf1 = CagdSrfSubdivAtParam(Srf, t, CAGD_CONST_V_DIR),
	    *Srf2 = Srf1 -> Pnext;

	if (CagdSrfAdapAuxDataFunc != NULL) {
	    CagdSrfAdapAuxDataFunc(Srf, AuxSrfData, t, CAGD_CONST_V_DIR,
				   Srf1, &AuxSrf1Data, Srf2, &AuxSrf2Data);

	    /* If both NULL, proceed as if no AuxSrfData.  If one is NULL,  */
	    /* ignore it.						    */
	    if (AuxSrf1Data == NULL && AuxSrf2Data != NULL)
		IgnoreFirst = TRUE;
	    if (AuxSrf2Data == NULL && AuxSrf1Data != NULL)
		IgnoreSecond = TRUE;
	}

	if (!IgnoreFirst)
	    CagdSrfAdap2PolygonsAux2(Srf1, A2PGrid, UIndexBase, UIndexSize,
				     VIndexBase, VIndexSize >> 1, AuxSrf1Data);
	if (!IgnoreSecond)
	    CagdSrfAdap2PolygonsAux2(Srf2, A2PGrid, UIndexBase, UIndexSize,
				     VIndexBase + (VIndexSize >> 1),
				     VIndexSize >> 1, AuxSrf2Data);
	CagdSrfFreeList(Srf1);
    }
    else {
        CagdSrfAdap2PolygonsAux3(Srf, A2PGrid, SrfErr, 0,
				 UIndexBase, UIndexSize,
				 VIndexBase, VIndexSize, AuxSrfData);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxiliary function of CagdSrfAdap2Polygons.  Assumes B-spline srfs only  *
* with no C1 discontinuities.						     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:             B-spline surface with no discontinuities to approximate *
*		     into triangles.					     *
*   A2PGrid:         Srf sampling points grid data structure.                *
*   SrfErr:	     Already computed error for Srf.			     *
*   UVBias:	     Bias against U-all or V-all subdivisions.		     *
*   UIndexBase:      2's power index of base of this U parametric domain.    *
*   UIndexSize:      2's power size of this U parametric domain.	     *
*   VIndexBase:      2's power index of base of this V parametric domain.    *
*   VIndexSize:      2's power size of this V parametric domain.	     *
*   AuxSrfData:      Optional data structure that will be passed to all      *
*                    subdivided sub-surfaces, or NULL if not needed.         *
*									     *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void CagdSrfAdap2PolygonsAux3(const CagdSrfStruct *Srf,
				     struct CagdA2PGridStruct *A2PGrid,
				     IrtRType SrfErr,
				     int UVBias,
				     int UIndexBase,
				     int UIndexSize,
				     int VIndexBase,
				     int VIndexSize,
				     VoidPtr AuxSrfData)
{
    CagdRType UMin, UMax, VMin, VMax;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    if (VMax - VMin < IRIT_EPS ||
	UMax - UMin < IRIT_EPS ||
	(UIndexSize <= 2 && VIndexSize <= 2) ||
	SrfErr <= 0.0) {
        /* Flat enough. */
#	ifdef DEBUG
        {
	    IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugCagd2PlyGridRes, TRUE) {
	        if (UIndexSize <= 2 ||
		    VIndexSize <= 2 ||
		    UMax - UMin < IRIT_EPS ||
		    VMax - VMin < IRIT_EPS)
		    IRIT_INFO_MSG("Tesselation max grid resolution or domain resolution reached.\n");
	    }
	}
#	endif /* DEBUG */

	CagdSrfAdap2PolysFlat(Srf, A2PGrid, -SrfErr, UIndexBase, UIndexSize,
			      VIndexBase, VIndexSize, AuxSrfData);
	return;
    }
    else {			         /* Need to continue and subdivide: */
	void
	    *AuxSrf1Data = NULL,
	    *AuxSrf2Data = NULL;
	CagdBType
	    IgnoreFirst = FALSE,
	    IgnoreSecond = FALSE;
        CagdRType ErrUSub1, ErrUSub2, ErrVSub1, ErrVSub2,
	    UDiv = UMin * (0.5 + CAGD_DIV_FRACTION) +
		   UMax * (0.5 - CAGD_DIV_FRACTION),
	    VDiv = VMin * (0.5 + CAGD_DIV_FRACTION) +
		   VMax * (0.5 - CAGD_DIV_FRACTION);
	CagdSrfStruct *Srf1u, *Srf1v;

	Srf1u = BspSrfSubdivAtParam(Srf, UDiv, CAGD_CONST_U_DIR);
	ErrUSub1 = CagdSrfAdap2PolyErrFunc(Srf1u);
	ErrUSub2 = CagdSrfAdap2PolyErrFunc(Srf1u -> Pnext);
	Srf1v = BspSrfSubdivAtParam(Srf, VDiv, CAGD_CONST_V_DIR);
	ErrVSub1 = CagdSrfAdap2PolyErrFunc(Srf1v);
	ErrVSub2 = CagdSrfAdap2PolyErrFunc(Srf1v -> Pnext);

	if (UVBias > CAGD_MAX_SUBDIV_ONE_DIR) {
	    /* We have CAGD_MAX_SUBDIV_ONE_DIR subdivisions in a row in the  */
	    /* U direction - force one subdivision in the V direction now.   */
	    UVBias = 0;
	    ErrUSub1 = ErrUSub2 = IRIT_INFNTY;
	    ErrVSub1 = ErrVSub2 = IRIT_INFNTY / 2.0;
	}
	else if (UVBias < -CAGD_MAX_SUBDIV_ONE_DIR) {
	    /* We have CAGD_MAX_SUBDIV_ONE_DIR subdivisions in a row in the  */
	    /* V direction - force one subdivision in the U direction now.   */
	    UVBias = 0;
	    ErrUSub1 = ErrUSub2 = IRIT_INFNTY / 2.0;
	    ErrVSub1 = ErrVSub2 = IRIT_INFNTY;
	}

	if (IRIT_MAX(ErrUSub1, ErrUSub2) < IRIT_MAX(ErrVSub1, ErrVSub1)) {
	    /* U subdivision yields smaller error. */
	    if (CagdSrfAdapAuxDataFunc != NULL) {
	        CagdSrfAdapAuxDataFunc(Srf, AuxSrfData, UDiv,
				       CAGD_CONST_U_DIR,
				       Srf1u, &AuxSrf1Data,
				       Srf1u -> Pnext, &AuxSrf2Data);

		/* If both NULL, proceed as if no AuxSrfData.  If one is    */
		/* NULL, ignore it.					    */
		if (AuxSrf1Data == NULL && AuxSrf2Data != NULL)
		    IgnoreFirst = TRUE;
		if (AuxSrf2Data == NULL && AuxSrf1Data != NULL)
		    IgnoreSecond = TRUE;
	    }

	    if (!IgnoreFirst)
	        CagdSrfAdap2PolygonsAux3(Srf1u, A2PGrid, ErrUSub1, UVBias + 1,
					 UIndexBase, UIndexSize >> 1,
					 VIndexBase, VIndexSize, AuxSrf1Data);
	    if (!IgnoreSecond)
	        CagdSrfAdap2PolygonsAux3(Srf1u -> Pnext, A2PGrid, ErrUSub2,
					 UVBias + 1,
					 UIndexBase + (UIndexSize >> 1),
					 UIndexSize >> 1,
					 VIndexBase, VIndexSize, AuxSrf2Data);
	}
	else {
	    /* V subdivision yields smaller error. */
	    if (CagdSrfAdapAuxDataFunc != NULL) {
	        CagdSrfAdapAuxDataFunc(Srf, AuxSrfData, VDiv,
				       CAGD_CONST_V_DIR,
				       Srf1v, &AuxSrf1Data,
				       Srf1v -> Pnext, &AuxSrf2Data);

		/* If both NULL, proceed as if no AuxSrfData.  If one is    */
		/* NULL, ignore it.					    */
		if (AuxSrf1Data == NULL && AuxSrf2Data != NULL)
		    IgnoreFirst = TRUE;
		if (AuxSrf2Data == NULL && AuxSrf1Data != NULL)
		    IgnoreSecond = TRUE;
	    }

	    if (!IgnoreFirst)
	        CagdSrfAdap2PolygonsAux3(Srf1v, A2PGrid, ErrVSub1, UVBias - 1,
					 UIndexBase, UIndexSize,
					 VIndexBase, VIndexSize >> 1,
					 AuxSrf1Data);
	    if (!IgnoreSecond)
	        CagdSrfAdap2PolygonsAux3(Srf1v -> Pnext, A2PGrid, ErrVSub2,
					 UVBias - 1,
					 UIndexBase, UIndexSize,
					 VIndexBase + (VIndexSize >> 1),
					 VIndexSize >> 1, AuxSrf2Data);
	}

	CagdSrfFreeList(Srf1u);
	CagdSrfFreeList(Srf1v);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a flat enough surface into polygons.                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:           Flat enough surface to convert to polygons.               *
*   A2PGrid:       Srf sampling points grid data structure.                  *
*   Err:           Amount of flatness error measured.                        *
*   UIndexBase:    2's power index of base of this U parametric domain.      *
*   UIndexSize:    2's power size of this U parametric domain.		     *
*   VIndexBase:    2's power index of base of this V parametric domain.      *
*   VIndexSize:    2's power size of this V parametric domain.		     *
*   AuxSrfData:    Optional data structure that will be passed to all        *
*                  subdivided sub-surfaces, or NULL if not needed.           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void CagdSrfAdap2PolysFlat(const CagdSrfStruct *Srf,
				  struct CagdA2PGridStruct *A2PGrid,
				  CagdRType Err,
				  int UIndexBase,
				  int UIndexSize,
				  int VIndexBase,
				  int VIndexSize,
				  VoidPtr AuxSrfData)
{
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfAdapRectStruct *Rect;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* Update the four corners in the global mesh/corner data structs. */
    if (UIndexBase + UIndexSize > CAGD2PLY_MAX_SUBDIV_INDEX ||
	VIndexBase + VIndexSize > CAGD2PLY_MAX_SUBDIV_INDEX)
        CAGD_FATAL_ERROR(CAGD_ERR_INDEX_NOT_IN_MESH);

    CagdSrfA2PGridInsertUV(A2PGrid, UIndexBase, VIndexBase, UMin, VMin);
    CagdSrfA2PGridInsertUV(A2PGrid, UIndexBase, VIndexBase + VIndexSize,
			   UMin, VMax);
    CagdSrfA2PGridInsertUV(A2PGrid, UIndexBase + UIndexSize, VIndexBase,
			   UMax, VMin);
    CagdSrfA2PGridInsertUV(A2PGrid, UIndexBase + UIndexSize,
			   VIndexBase + VIndexSize, UMax, VMax);

    if (CAGD_IS_RATIONAL_SRF(Srf) &&
	Cagd2PolyClipPolysAtPoles(CAGD_QUERY_VALUE) &&
	CagdPointsHasPoles(Srf -> Points, Srf -> ULength * Srf -> VLength))
	return;		   /* We have poles in this small patch - purge it. */

    /* Push this rectangular for late evaluation. */
    Rect = CagdSrfAdap2PolyAllocRect(UIndexBase, UIndexSize,
				     VIndexBase, VIndexSize, AuxSrfData, Err);
    IRIT_LIST_PUSH(Rect, GlblRectList);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Converts a single rectangular domain into polygons.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:    Bspline surface with no discontinuities to approximate into      *
*	    triangles.							     *
*   Rect:   The rectangular domain to convert to polygons.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:   Constructed polygons.                             *
*****************************************************************************/
static CagdPolygonStruct *CagdSrfAdapRect2Polys(const CagdSrfStruct *Srf,
						struct CagdA2PGridStruct
						                    *A2PGrid,
						CagdSrfAdapRectStruct *Rect)
{
    CagdSrfPtStruct
        *SrfPtList = CagdSrfA2PGridFetchRect(A2PGrid,
					     Rect -> UIndexBase,
					     Rect -> VIndexBase,
					     Rect -> UIndexBase +
					         Rect -> UIndexSize,
					     Rect -> VIndexBase +
					         Rect -> VIndexSize);

    /* Convert this (convex!) domain to triangles. */
    return SrfPtList != NULL ? CagdSrfAdapPolyGenFunc(Srf, SrfPtList, Rect)
			     : NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Converts the given circular list of surface points into polygons.        M
* The list is assumed a convex parametric domain (which ease the process     M
* of decomposition).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:          Bspline surface with no discontinuities to approximate     M
*	          into triangles.					     M
*   SrfPtList:    Circular list of a convex surface domain to convert to     M
*		  triangles.						     M
*   Rect:         The rectangular domain to convert to polygons.             M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPolygonStruct *:  List of polygons out of the closed srf pt list;    M
*		 Could be NULL if polygons are call back created.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons, CagdSrf2PolygonSetErrFunc,			     M
*   CagdSrfAdap2PolyDefErrFunc, CagdSrf2PolyAdapSetErrFunc,		     M
*   CagdSrf2PolyAdapSetAuxDataFunc CagdSrf2PolyAdapSetPolyGenFunc	     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfAdapRectPolyGen, polygonization, surface approximation            M
*****************************************************************************/
CagdPolygonStruct *CagdSrfAdapRectPolyGen(const CagdSrfStruct *Srf,
					  CagdSrfPtStruct *SrfPtList,
					  const CagdSrfAdapRectStruct *Rect)
{
    CagdBType Has3ColinearPts;
    int ListLen, GenPoly;
    CagdSrfPtStruct *SrfPt, *SrfPtMax, *SrfPtPrev, *SrfPtNext, *S2, *S3;
    CagdPolygonStruct *Poly,
	*Polys = NULL;

    for (SrfPt = SrfPtList -> Pnext, ListLen = 1;
	 SrfPt != NULL && SrfPt != SrfPtList;
	 SrfPt = SrfPt -> Pnext, ListLen++);

#ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugDumpRectLoops, FALSE) {
	    IRIT_INFO_MSG_PRINTF("\n\nRect:\nVertex at u = %g, v = %g\n",
				 SrfPtList -> Uv[0], SrfPtList -> Uv[1]);
	    for (SrfPt = SrfPtList -> Pnext;
		 SrfPt != NULL && SrfPt != SrfPtList;
		 SrfPt = SrfPt -> Pnext)
	      IRIT_INFO_MSG_PRINTF("Vertex at u = %g, v = %g\n",
		     SrfPt -> Uv[0], SrfPt -> Uv[1]);
	}
    }
#endif /* DEBUG */

    /* If a simple and planar rectangle, dump as such. */
    S2 = SrfPtList -> Pnext;
    S3 = S2 -> Pnext;
    if (ListLen == 4) {
	CagdSrfPtStruct
	    *S4 = S3 -> Pnext;

	if (_CagdSrfMakeOnlyTri ||
	    !GMCoplanar4Pts(SrfPtList -> Pt, S2 -> Pt, S3 -> Pt, S4 -> Pt)) {
	    Poly = CagdGenAdapTriangle(CagdSrf2PolyComputeNormals,
				       CagdSrf2PolyComputeUV,
				       SrfPtList -> Pt, S2 -> Pt, S3 -> Pt,
				       CAGD_ADAP_FIX_DISCONT_NRML(SrfPtList),
				       CAGD_ADAP_FIX_DISCONT_NRML(S2),
				       CAGD_ADAP_FIX_DISCONT_NRML(S3),
				       SrfPtList -> Uv, S2 -> Uv, S3 -> Uv,
				       &GenPoly);
	    if (Poly != NULL)
	        IRIT_LIST_PUSH(Poly, Polys);

	    Poly = CagdGenAdapTriangle(CagdSrf2PolyComputeNormals,
				       CagdSrf2PolyComputeUV,
				       SrfPtList -> Pt, S3 -> Pt, S4 -> Pt,
				       CAGD_ADAP_FIX_DISCONT_NRML(SrfPtList),
				       CAGD_ADAP_FIX_DISCONT_NRML(S3),
				       CAGD_ADAP_FIX_DISCONT_NRML(S4),
				       SrfPtList -> Uv, S3 -> Uv, S4 -> Uv,
			     &GenPoly);
	    if (Poly != NULL)
	        IRIT_LIST_PUSH(Poly, Polys);
	}
	else {
	    Polys = CagdGenAdapRectangle(CagdSrf2PolyComputeNormals,
					 CagdSrf2PolyComputeUV,
					 SrfPtList -> Pt, S2 -> Pt,
					 S3 -> Pt, S4 -> Pt,
					 CAGD_ADAP_FIX_DISCONT_NRML(SrfPtList),
					 CAGD_ADAP_FIX_DISCONT_NRML(S2),
					 CAGD_ADAP_FIX_DISCONT_NRML(S3),
					 CAGD_ADAP_FIX_DISCONT_NRML(S4),
					 SrfPtList -> Uv, S2 -> Uv, S3 -> Uv,
					 S4 -> Uv, &GenPoly);

	    if (Polys == NULL && !GenPoly) {
		/* This rectangle is degenerate - try two triangles. */
		Poly = CagdGenAdapTriangle(CagdSrf2PolyComputeNormals,
					 CagdSrf2PolyComputeUV,
					 SrfPtList -> Pt, S2 -> Pt, S3 -> Pt,
					 CAGD_ADAP_FIX_DISCONT_NRML(SrfPtList),
					 CAGD_ADAP_FIX_DISCONT_NRML(S2),
					 CAGD_ADAP_FIX_DISCONT_NRML(S3),
					 SrfPtList -> Uv, S2 -> Uv, S3 -> Uv, &GenPoly);
		if (Poly != NULL)
		    IRIT_LIST_PUSH(Poly, Polys);

		Poly = CagdGenAdapTriangle(CagdSrf2PolyComputeNormals,
					 CagdSrf2PolyComputeUV,
					 SrfPtList -> Pt, S3 -> Pt, S4 -> Pt,
					 CAGD_ADAP_FIX_DISCONT_NRML(SrfPtList),
					 CAGD_ADAP_FIX_DISCONT_NRML(S3),
					 CAGD_ADAP_FIX_DISCONT_NRML(S4),
					 SrfPtList -> Uv, S3 -> Uv, S4 -> Uv,
					 &GenPoly);
		if (Poly != NULL)
		    IRIT_LIST_PUSH(Poly, Polys);
	    }
	}

	return Polys;
    }

    Has3ColinearPts = TRUE;

    while (ListLen-- > 3) {
	/* Look for 3 or more colinear vertices along a parametric line and */
	/* cut the end corner of it, at the last colinear vertex.           */
	if (Has3ColinearPts) {
	    CagdBType SameU, SameV;

	    SrfPtMax = NULL;
	    SrfPtPrev = SrfPtList;
	    SrfPt = SrfPtPrev -> Pnext;
	    SameU = IRIT_APX_EQ(SrfPtPrev -> Uv[0], SrfPt -> Uv[0]);
	    SameV = IRIT_APX_EQ(SrfPtPrev -> Uv[1], SrfPt -> Uv[1]);
	    do {
	        SrfPtNext = SrfPt -> Pnext;
		if (SameU &&
		    IRIT_APX_EQ(SrfPt -> Uv[0], SrfPtNext -> Uv[0]) &&
		    !IRIT_APX_EQ(SrfPtNext -> Uv[0], SrfPtNext -> Pnext -> Uv[0])) {
		    SrfPtMax = SrfPt;
		    break;
		}
		else if (SameV &&
			 IRIT_APX_EQ(SrfPt -> Uv[1], SrfPtNext -> Uv[1]) &&
			 !IRIT_APX_EQ(SrfPtNext -> Uv[1],
				      SrfPtNext -> Pnext -> Uv[1])) {
		    SrfPtMax = SrfPt;
		    break;
		}

		SameU = IRIT_APX_EQ(SrfPtNext -> Uv[0], SrfPt -> Uv[0]);
		SameV = IRIT_APX_EQ(SrfPtNext -> Uv[1], SrfPt -> Uv[1]);

		SrfPtPrev = SrfPt;
		SrfPt = SrfPtNext;
	    }
	    while (SrfPtPrev != SrfPtList);

	    if (SrfPtMax == NULL) {
	        Has3ColinearPts = FALSE;
	        SrfPtMax = SrfPtList;
	    }
	}
	else {                 /* No 3 colinear vertices along param. line. */
	    /* Look for a most convex vertex as a corner to cut. */
	    SrfPtMax = SrfPtList;
	}

	S2 = SrfPtMax -> Pnext;
	S3 = S2 -> Pnext;
	if ((Poly = CagdGenAdapTriangle(CagdSrf2PolyComputeNormals,
			     CagdSrf2PolyComputeUV,
			     SrfPtMax -> Pt, S2 -> Pt, S3 -> Pt,
			     CAGD_ADAP_FIX_DISCONT_NRML(SrfPtMax),
				 CAGD_ADAP_FIX_DISCONT_NRML(S2),
				 CAGD_ADAP_FIX_DISCONT_NRML(S3),
			     SrfPtMax -> Uv, S2 -> Uv, S3 -> Uv, &GenPoly))
								    != NULL) {
	    IRIT_LIST_PUSH(Poly, Polys);
	}

	SrfPtMax -> Pnext = S3;   /* Skip S2, effectively skipping triangle. */
	SrfPtList = SrfPtMax; /* Make sure SrfPtList points to valid vertex. */
    }

    /* Do the last triangle */
    S2 = SrfPtList -> Pnext;
    S3 = S2 -> Pnext;
    if ((Poly = CagdGenAdapTriangle(CagdSrf2PolyComputeNormals,
			       CagdSrf2PolyComputeUV,
			       SrfPtList -> Pt, S2 -> Pt, S3 -> Pt,
			       CAGD_ADAP_FIX_DISCONT_NRML(SrfPtList),
				   CAGD_ADAP_FIX_DISCONT_NRML(S2),
				   CAGD_ADAP_FIX_DISCONT_NRML(S3),
			       SrfPtList -> Uv, S2 -> Uv, S3 -> Uv, &GenPoly))
								    != NULL) {
        IRIT_LIST_PUSH(Poly, Polys);
    }

    return Polys;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates a rectangle to convert to polygons later on.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   UIndexBase, UIndexSize:  U domain of rectangle.                          *
*   VIndexBase, VIndexSize:  V domain of rectangle.                          *
*   AuxSrfData:              Optional data structure that was passed to all  *
*                            subdivided sub-surfaces, or NULL if not needed. *
*   Err:		     Error of approximation.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdSrfAdapRectStruct *:   Allocated rectangle.                          *
*****************************************************************************/
static CagdSrfAdapRectStruct *CagdSrfAdap2PolyAllocRect(int UIndexBase,
							int UIndexSize,
							int VIndexBase,
							int VIndexSize,
							VoidPtr AuxSrfData,
							CagdRType Err)
{
    CagdSrfAdapRectStruct *Rect;

#   ifdef DEBUG
        IRIT_IF_DEBUG_ON_PARAMETER(_DebugDumpSubdivMinSize) {
	    GlblSubdivMinSize = IRIT_MIN(GlblSubdivMinSize, UIndexSize);
	    GlblSubdivMinSize = IRIT_MIN(GlblSubdivMinSize, VIndexSize);
	}
#   endif /* DEBUG */

    if (GlblRectFreeList == NULL) {
	int i;
        CagdSrfAdapRectStruct
	    *Rects = IritMalloc(sizeof(CagdSrfAdapRectStruct) *
				CAGD_RECT_ALLOC_BLOCK);
	GlblRectFreeList = &Rects[0];

	/* Allocate a block of CAGD_RECT_ALLOC_BLOCK such structures. */
	for (i = 0; i < CAGD_RECT_ALLOC_BLOCK - 1; i++)
	    Rects[i].Pnext = &Rects[i + 1];
	Rects[i].Pnext = NULL;
    }

    IRIT_LIST_POP(Rect, GlblRectFreeList);

    Rect -> UIndexBase = UIndexBase;
    Rect -> UIndexSize = UIndexSize;
    Rect -> VIndexBase = VIndexBase;
    Rect -> VIndexSize = VIndexSize;
    Rect -> AuxSrfData = AuxSrfData;
    Rect -> Err = Err;

    return Rect;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compute the normal to the surface very close to UV1 in the triangle      M
* defined by UV1/UV2/UV3.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   UV1:      To compute the surface normal close to.                        M
*   UV2, UV3: Two other UV''s of the triangle.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Computed normal in a static location.  Upto 4 normals     M
*                  could be satatically saved simultaneously.	             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSrfAdap2Polygons                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSrfAdap2PolyEvalNrmlBlendedUV                                        M
*****************************************************************************/
CagdRType *CagdSrfAdap2PolyEvalNrmlBlendedUV(const CagdRType *UV1,
					     const CagdRType *UV2,
					     const CagdRType *UV3)
{
    IRIT_STATIC_DATA int
	Idx = -1;
    IRIT_STATIC_DATA CagdPType Nrmls[4];
    CagdRType UV[2];
    CagdVecStruct *N;

    UV[0] = UV1[0] * (1.0 - CAGD_DISCONT_NORMAL_EPS) +
	    (UV2[0] + UV3[0]) * 0.5 * CAGD_DISCONT_NORMAL_EPS;
    UV[1] = UV1[1] * (1.0 - CAGD_DISCONT_NORMAL_EPS) +
	    (UV2[1] + UV3[1]) * 0.5 * CAGD_DISCONT_NORMAL_EPS;

    N = CagdSrfEffiNrmlEval(UV[0], UV[1], TRUE);

    if (++Idx >= 4)
        Idx = 0;
    IRIT_VEC_COPY(Nrmls[Idx], N -> Vec);

    return Nrmls[Idx];
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Routine to create one triangular polygon, given its vertices,	     *
* and, optionally, normals and uv coordinates. 			             *
*                                                                            *
* PARAMETERS:                                                                *
*   ComputeNormals:      If TRUE then use Nl? parameters. Nl? are valid.     *
*   ComputeUV:           If TRUE then use UV? parameters. UV? are valid.     *
*   Pt1, Pt2, Pt3:       Euclidean locations of vertices.                    *
*   Nl1, Nl2, Nl3:       Optional Normals of vertices (if ComputeNormals).   *
*   UV1, UV2, UV3:       Optional UV parametric location of vertices (if     *
*                        ComputeUV).                                         *
*   GenPoly:             Returns TRUE if a polygon was generated, FALSE      *
*		         otherwise.  Note this function can return NULL and  *
*		         still generate a polygon as a call back for         *
*		         CagdSrf2Polygons.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:   Allocated triangle or NULL if none.		     *
*****************************************************************************/
static CagdPolygonStruct *CagdGenAdapTriangle(CagdBType ComputeNormals,
					      CagdBType ComputeUV,
					      const CagdRType *Pt1,
					      const CagdRType *Pt2,
					      const CagdRType *Pt3,
					      const CagdRType *Nl1,
					      const CagdRType *Nl2,
					      const CagdRType *Nl3,
					      const CagdRType *UV1,
					      const CagdRType *UV2,
					      const CagdRType *UV3,
					      CagdBType *GenPoly)
{
    if (ComputeNormals) {
        if (Nl1 == NULL)
	    Nl1 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV1, UV2, UV3);
	if (Nl2 == NULL)
	    Nl2 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV2, UV1, UV3);
	if (Nl3 == NULL)
	    Nl3 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV3, UV2, UV1);
    }

    return _CagdSrfMakeTriFunc(ComputeNormals, ComputeUV, Pt1, Pt2, Pt3,
			       Nl1, Nl2, Nl3, UV1, UV2, UV3, GenPoly);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back routine to create one rectangular polygon, given its vertices, *
* and, optionally, normals and uv coordinates.  Places the constructed       *
* polygon in a global polygonal list.				             *
*                                                                            *
* PARAMETERS:                                                                *
*   ComputeNormals:      If TRUE then use Nl? parameters. Nl? are valid.     *
*   ComputeUV:           If TRUE then use UV? parameters. UV? are valid.     *
*   Pt1, Pt2, Pt3, Pt4:  Euclidean locations of vertices.                    *
*   Nl1, Nl2, Nl3, Nl4:  Optional Normals of vertices (if ComputeNormals).   *
*   UV1, UV2, UV3, UV4:  Optional UV parametric location of vertices (if     *
*                        ComputeUV).                                         *
*   GenPoly:             Returns TRUE if a polygon was generated, FALSE      *
*		         otherwise.  Note this function can return NULL and  *
*		         still generate a polygon as a call back for         *
*		         CagdSrf2Polygons.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdPolygonStruct *:  Allocated rectangle or NULL if none.	             *
*****************************************************************************/
static CagdPolygonStruct *CagdGenAdapRectangle(CagdBType ComputeNormals,
					       CagdBType ComputeUV,
					       const CagdRType *Pt1,
					       const CagdRType *Pt2,
					       const CagdRType *Pt3,
					       const CagdRType *Pt4,
					       const CagdRType *Nl1,
					       const CagdRType *Nl2,
					       const CagdRType *Nl3,
					       const CagdRType *Nl4,
					       const CagdRType *UV1,
					       const CagdRType *UV2,
					       const CagdRType *UV3,
					       const CagdRType *UV4,
					       CagdBType *GenPoly)
{
    /* Normal could be NULL if on a discontinuity.  Evaluate manually then. */
    if (ComputeNormals) {
        if (Nl1 == NULL)
	    Nl1 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV1, UV2, UV4);
	if (Nl2 == NULL)
	    Nl2 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV2, UV1, UV3);
	if (Nl3 == NULL)
	    Nl3 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV3, UV2, UV4);
	if (Nl4 == NULL)
	    Nl4 = CagdSrfAdap2PolyEvalNrmlBlendedUV(UV4, UV3, UV1);
    }

    return _CagdSrfMakeRectFunc(ComputeNormals, ComputeUV, Pt1, Pt2, Pt3, Pt4,
				Nl1, Nl2, Nl3, Nl4, UV1, UV2, UV3, UV4,
				GenPoly);
}
