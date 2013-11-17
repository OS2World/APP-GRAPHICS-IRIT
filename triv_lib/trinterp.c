/******************************************************************************
* TriInterp.c - Interpolation of trivariate data using trivariates.           *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Oct. 94.					      *
******************************************************************************/

#include "geom_lib.h"
#include "extra_fn.h"
#include "triv_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Interpolates control points of given trivariate, preserving the order    M
* and continuity of the original trivariate.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   TV:   Trivariate to interpolate its control mesh.                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *: The interpolating trivariate.                            M
*									     * 
* SEE ALSO:                                                                  M
*   TrivTVInterpPts, TrivTVInterpolate					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivInterpTrivar, interpolation                                          M
*****************************************************************************/
TrivTVStruct *TrivInterpTrivar(const TrivTVStruct *TV)
{
    return TrivTVInterpolate(TV,
			     TV -> ULength, TV -> VLength, TV -> WLength,
			     TV -> UOrder, TV -> VOrder, TV -> WOrder);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points, PtList, computes a Bspline trivariate of order      M
* UOrder by VOrder by WOrder that interpolates or least square approximates  M
* the given set of points.	                                             M
*   PtGrid is a trivariate whose point data is employed toward the fitting.  M
* PtGrid also prescribes the parametric domain of the result.		     M
* lists.		                                   	             M
*   The size of the control mesh of the resulting Bspline trivariate         M
* Trivar defaults to the number of points in PtGrid (if TV?Size = 0).	     M
*   However, either numbers can smaller to yield a least square              M
* approximation of the gievn data set.                     	             M
*                                                                            *
* PARAMETERS:                                                                M
*   PtGrid:      Input data grid as a trivariate.                            M
*   UOrder:      Of the to be created trivariate.                            M
*   VOrder:      Of the to be created trivariate.                            M
*   WOrder:      Of the to be created trivariate.                            M
*   TVUSize:     U size of the to be created trivariate. Must be at least as M
*                large as the array PtGrid.			             M
*   TVVSize:     V size of the to be created trivariate. Must be at least as M
*                large as the array PtGrid. 			             M
*   TVWSize:     W size of the to be created trivariate. Must be at least as M
*                large as the array PtList.			             M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Constructed interpolating/approximating trivariate.    M
*									     * 
* SEE ALSO:                                                                  M
*   TrivInterpTrivar, TrivTVInterpolate					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVInterpPts, interpolation, least square approximation               M
*****************************************************************************/
TrivTVStruct *TrivTVInterpPts(const TrivTVStruct *PtGrid,
			      int UOrder,
			      int VOrder,
			      int WOrder,
			      int TVUSize,
			      int TVVSize,
			      int TVWSize)
{
    return TrivTVInterpolate(PtGrid,
			     TVUSize, TVVSize, TVWSize,
			     UOrder, VOrder, WOrder);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points on a box grid, PtGrid, the expected lengths          M
* U/V/WLength and orders U/V/WOrder of the Bspline trivariate, computes the  M
* Bspline trivariate's coefficients that interpolates or least square        M
* approximates the given set of points, PtGrid.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtGrid:   Input data grid as a trivariate.                               M
*   ULength:  Requested length of control mesh of trivariate in U direction. M
*	      If zero, length of PtGrid in U is used.			     M
*   VLength:  Requested length of control mesh of trivariate in V direction. M
*	      If zero, length of PtGrid in V is used.			     M
*   WLength:  Requested length of control mesh of trivariate in W direction. M
*	      If zero, length of PtGrid in W is used.			     M
*   UOrder:   Requested order of trivariate in U direction.                  M
*	      If zero, order of PtGrid in U is used.			     M
*   VOrder:   Requested order of trivariate in V direction.                  M
*	      If zero, order of PtGrid in V is used.			     M
*   WOrder:   Requested order of trivariate in W direction.                  M
*	      If zero, order of PtGrid in W is used.			     M
*									     *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Constructed interpolating/approximating trivariate.    M
*									     * 
* SEE ALSO:                                                                  M
*   TrivInterpTrivar, TrivTVInterpPts, TrivTVInterpScatPts		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVInterpolate, interpolation, least square approximation             M
*****************************************************************************/
TrivTVStruct *TrivTVInterpolate(const TrivTVStruct *PtGrid,
				int ULength,
				int VLength,
				int WLength,
				int UOrder,
				int VOrder,
				int WOrder)
{

    CagdPointType
	PType = PtGrid -> PType;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PType);
    int i, j, k, l, m, UVLength,
	MaxCoord = CAGD_NUM_OF_PT_COORD(PType),
	UGrid = PtGrid -> ULength,
	VGrid = PtGrid -> VLength,
	WGrid = PtGrid -> WLength,
	UVGrid = UGrid * VGrid;
    TrivTVStruct *InterpTV;
    CagdRType *R, *KV, *UParams, *VParams, *WParams, **InterpPoints,
        * const *GridPoints = PtGrid -> Points;
    CagdSrfStruct
	**Srfs = (CagdSrfStruct **) IritMalloc(sizeof(CagdSrfStruct *) *
								     WGrid);

    if (ULength == 0)
        ULength = UGrid;
    if (VLength == 0)
        VLength = VGrid;
    if (WLength == 0)
        WLength = WGrid;
    ULength = IRIT_MAX(ULength, 2);
    VLength = IRIT_MAX(VLength, 2);
    WLength = IRIT_MAX(WLength, 2);
    UVLength = ULength * VLength;

    if (UOrder == 0)
        UOrder = PtGrid -> UOrder;
    if (VOrder == 0)
        VOrder = PtGrid -> VOrder;
    if (WOrder == 0)
        WOrder = PtGrid -> WOrder;
    UOrder = IRIT_MAX(UOrder, 1);
    VOrder = IRIT_MAX(VOrder, 1);
    WOrder = IRIT_MAX(WOrder, 1);

    InterpTV = TrivBspTVNew(ULength, VLength, WLength,
			    UOrder, VOrder, WOrder, PType);
    InterpPoints = InterpTV -> Points;

    /* Update the new knot sequences/sampling. */
    UParams = (CagdRType *) IritMalloc(sizeof(CagdRType) * UGrid);
    VParams = (CagdRType *) IritMalloc(sizeof(CagdRType) * VGrid);
    WParams = (CagdRType *) IritMalloc(sizeof(CagdRType) * WGrid);
    for (i = 0, R = UParams; i < UGrid; i++)
	*R++ = ((CagdRType) i) / (UGrid - 1);
    for (i = 0, R = VParams; i < VGrid; i++)
        *R++ = ((CagdRType) i) / (VGrid - 1);
    for (i = 0, R = WParams; i < WGrid; i++)
        *R++ = ((CagdRType) i) / (WGrid - 1);

    KV = BspPtSamplesToKV(UParams, UGrid, UOrder, ULength);
    IRIT_GEN_COPY(InterpTV -> UKnotVector, KV,
		   sizeof(CagdRType) * (UOrder + ULength));
    IritFree(KV);

    KV = BspPtSamplesToKV(VParams, VGrid, VOrder, VLength);
    IRIT_GEN_COPY(InterpTV -> VKnotVector, KV,
		  sizeof(CagdRType) * (VOrder + VLength));
    IritFree(KV);

    KV = BspPtSamplesToKV(WParams, WGrid, WOrder, WLength);
    IRIT_GEN_COPY(InterpTV -> WKnotVector, KV,
		  sizeof(CagdRType) * (WOrder + WLength));
    IritFree(KV);

    /* Interpolate the UV direction of the trivariate: */
    for (k = 0; k < WGrid; k++) {
	int Index = k * UVGrid;
	CagdCtlPtStruct *Pt,
	     *PtList = NULL;

	for (l = 0; l < UVGrid; l++, Index++) {
	    Pt = CagdCtlPtNew(PType);
	    for (m = !IsRational; m <= MaxCoord; m++)
		Pt -> Coords[m] = GridPoints[m][Index];

	    IRIT_LIST_PUSH(Pt, PtList);
	}
	PtList = CagdListReverse(PtList);

	Srfs[k] = BspSrfInterpolate(PtList, UGrid, VGrid, UParams, VParams,
				    InterpTV -> UKnotVector,
				    InterpTV -> VKnotVector,
				    ULength, VLength,
				    UOrder, VOrder);

	CagdCtlPtFreeList(PtList);
    }

    /* Interpolate the third, W, dimension. */
    for (i = 0; i < ULength; i++) {
	for (j = 0; j< VLength; j++) {
	    int Index = TRIV_MESH_UVW(InterpTV, i, j, 0);
	    CagdRType **CrvPoints;
	    CagdCrvStruct *Crv;
	    CagdCtlPtStruct *Pt,
	        *PtList = NULL;

	    for (k = 0; k < WGrid; k++) {
		CagdRType
		    **SrfPoints = Srfs[k] -> Points;

		Pt = CagdCtlPtNew(PType);
		for (m = !IsRational; m <= MaxCoord; m++)
		    Pt -> Coords[m] = SrfPoints[m][Index];

		IRIT_LIST_PUSH(Pt, PtList);
	    }
	    PtList = CagdListReverse(PtList);

	    Crv = BspCrvInterpolate(PtList, WParams, InterpTV -> WKnotVector,
				    WLength, WOrder, FALSE);
	    CrvPoints = Crv -> Points;
	    CagdCtlPtFreeList(PtList);

	    for (k = 0; k < WLength; k++) {
		for (m = !IsRational; m <= MaxCoord; m++)
		   InterpPoints[m][Index + k * UVLength] = CrvPoints[m][k];
	    }

	    CagdCrvFree(Crv);
	}
    }

    IritFree(UParams);
    IritFree(VParams);
    IritFree(WParams);

    for (k = 0; k < WGrid; k++)
	CagdSrfFree(Srfs[k]);
    IritFree(Srfs);

    return InterpTV;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of scattered points, PtList, computes a Bspline trivariate of  M
* order UOrder by VOrder by WOrder that interpolates or least squares        M
* approximates the given set of scattered points.                            M
*   PtList is a NULL terminated lists of CagdPtStruct structs, with each     M
* point holding (u, v, w, x [, y[, z]]).  That is, E4 points create an E1    M
* scalar trivariate and E6 points create an E3 trivariate,           	     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:      A NULL terminating array of linked list of points.          M
*   USize:       U size of the to be created trivariate.                     M
*   VSize:       V size of the to be created trivariate.                     M
*   WSize:       W size of the to be created trivariate.                     M
*   UOrder:      Of the to be created trivariate.                            M
*   VOrder:      Of the to be created trivariate.                            M
*   WOrder:      Of the to be created trivariate.                            M
*   UKV:	 Expected knot vector in U direction, NULL for uniform open. M
*   VKV:	 Expected knot vector in V direction, NULL for uniform open. M
*   WKV:	 Expected knot vector in W direction, NULL for uniform open. M
*                                                                            *
* RETURN VALUE:                                                              M
*   TrivTVStruct *:   Constructed interpolating/approximating trivariate.    M
*									     * 
* SEE ALSO:                                                                  M
*   TrivInterpTrivar, TrivTVInterpPts, TrivTVInterpolate,		     M
*   BspSrfInterpScatPts							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   TrivTVInterpScatPts, interpolation, least square approximation           M
*****************************************************************************/
TrivTVStruct *TrivTVInterpScatPts(const CagdCtlPtStruct *PtList,
				  int USize,
				  int VSize,
				  int WSize,
				  int UOrder,
				  int VOrder,
				  int WOrder,
				  CagdRType *UKV,
				  CagdRType *VKV,
				  CagdRType *WKV)
{
    int i, j, k,
	NumCoords = CAGD_NUM_OF_PT_COORD(PtList -> PtType),
	PtListLen = CagdListLength(PtList),
	Size = USize * VSize * WSize;
    CagdBType
	NewUKV = FALSE,
	NewVKV = FALSE,
	NewWKV = FALSE;
    CagdRType *M, *R, *InterpPts,
	*ULine = (CagdRType *) IritMalloc(sizeof(CagdRType) * UOrder),
	*VLine = (CagdRType *) IritMalloc(sizeof(CagdRType) * VOrder),
	*Mat = (CagdRType *) IritMalloc(sizeof(CagdRType) * Size *
					IRIT_MAX(Size, PtListLen));
    const CagdCtlPtStruct *Pt;
    TrivTVStruct *TV;

    if (NumCoords < 3) {
	CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

    IRIT_ZAP_MEM(Mat, sizeof(CagdRType) * Size * IRIT_MAX(Size, PtListLen));

    if (UKV == NULL) {
	UKV = BspKnotUniformOpen(USize, UOrder, NULL);
	BspKnotAffineTrans2(UKV, USize + UOrder, 0.0, 1.0);
	NewUKV = TRUE;
    }
    if (VKV == NULL) {
	VKV = BspKnotUniformOpen(VSize, VOrder, NULL);
	BspKnotAffineTrans2(VKV, VSize + VOrder, 0.0, 1.0);
	NewVKV = TRUE;
    }
    if (WKV == NULL) {
	WKV = BspKnotUniformOpen(WSize, WOrder, NULL);
	BspKnotAffineTrans2(WKV, WSize + WOrder, 0.0, 1.0);
	NewWKV = TRUE;
    }

    for (Pt = PtList, M = Mat; Pt != NULL; Pt = Pt -> Pnext, M += Size) {
	int UIndex, VIndex, WIndex;
	CagdRType *WLine;

	if (NumCoords != CAGD_NUM_OF_PT_COORD(Pt -> PtType)) {
	    CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);
	    IritFree(ULine);
	    IritFree(Mat);
	    return NULL;
	}

	WLine = BspCrvCoxDeBoorBasis(UKV, UOrder, USize, FALSE,
				     Pt -> Coords[1], &UIndex);
	IRIT_GEN_COPY(ULine, WLine, sizeof(CagdRType) * UOrder);
	WLine = BspCrvCoxDeBoorBasis(VKV, VOrder, VSize, FALSE,
				     Pt -> Coords[2], &VIndex);
	IRIT_GEN_COPY(VLine, WLine, sizeof(CagdRType) * VOrder);
	WLine = BspCrvCoxDeBoorBasis(WKV, WOrder, WSize, FALSE,
				     Pt -> Coords[3], &WIndex);

	for (k = WIndex; k < WIndex + WOrder; k++) {
	    for (j = VIndex; j < VIndex + VOrder; j++) {
	        for (i = UIndex; i < UIndex + UOrder; i++) {
		    M[k * VSize * USize + j * USize + i] =
		        ULine[i - UIndex] *
		        VLine[j - VIndex] *
			WLine[k - WIndex];
		}
	    }
	}
    }
    IritFree(ULine);
    IritFree(VLine);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugTVSCatInterpMat, FALSE) {
	    for (i = 0; i < PtListLen; i++) {
	        IRIT_INFO_MSG("[");
		for (j = 0; j < Size; j++) {
		    IRIT_INFO_MSG_PRINTF("%5.3f ", Mat[i * Size + j]);
		}
		IRIT_INFO_MSG("]\n");
	    }
	}
    }
#   endif /* DEBUG */

    /* Compute SVD decomposition for Mat. */
    if (IRIT_FABS(SvdLeastSqr(Mat, NULL, NULL,
			 IRIT_MAX(Size, PtListLen), Size)) < IRIT_UEPS &&
	Size <= PtListLen) {
	CAGD_FATAL_ERROR(CAGD_ERR_NO_SOLUTION);

	IritFree(Mat);
	return NULL;
    }
    IritFree(Mat);

    /* Construct the Bspline trivariate and copy its knot vectors. */
    TV = TrivBspTVNew(USize, VSize, WSize, UOrder, VOrder, WOrder,
		  CAGD_MAKE_PT_TYPE(FALSE, NumCoords - 3));
    CAGD_GEN_COPY(TV -> UKnotVector, UKV,
		  (TRIV_TV_UPT_LST_LEN(TV) + UOrder) * sizeof(CagdRType));
    CAGD_GEN_COPY(TV -> VKnotVector, VKV,
		  (TRIV_TV_VPT_LST_LEN(TV) + VOrder) * sizeof(CagdRType));
    CAGD_GEN_COPY(TV -> WKnotVector, WKV,
		  (TRIV_TV_WPT_LST_LEN(TV) + WOrder) * sizeof(CagdRType));

    /* Solve for the coefficients of all the coordinates of the curve. */
    InterpPts = (CagdRType *) IritMalloc(sizeof(CagdRType) *
					 IRIT_MAX(Size, PtListLen));
    for (i = 4; i <= NumCoords; i++) {
	for (Pt = PtList, R = InterpPts; Pt != NULL; Pt = Pt -> Pnext)
	    *R++ = Pt -> Coords[i];

	SvdLeastSqr(NULL, TV -> Points[i - 3], InterpPts, PtListLen, Size);
    }
    SvdLeastSqr(NULL, NULL, NULL, 0, 0);			/* Clean up. */
    IritFree(InterpPts);

    if (NewUKV)
	IritFree(UKV);
    if (NewVKV)
	IritFree(VKV);
    if (NewWKV)
	IritFree(WKV);

    return TV;
}
