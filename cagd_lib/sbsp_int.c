/******************************************************************************
* SBsp-Int.c - Bspline surface interpolation/approximation schemes.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, Feb. 94.					      *
* BspSrfInterpScatPts2 and related functions contributed by Jacob Barhak.     *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cagd_loc.h"
#include "extra_fn.h"

#define BSP_SRF_FIT_SAMPLE_RATIO	10

/* Sparse matrix structure:                                                  */
/*   The matrix structure holds all the cells that are not zero. It holds    */
/* pointers to the begining of each row and each column in an array.         */
/*   Each row/column is represented by a linked list of cells (no ordering   */
/* is suported in this version.                                              */
/*   Access to a cell is obtained by starting a search the row with it's row */
/* index and searching the linked list for a cell with the approprite column */
/* index. This can also be done by searching the column linked list for      */
/* a cell with the wanted row index.                                         */
/*   The sparse matrix structure can also hold an index indicator array that */
/* speeds matrix multiplication. This option is optional due to memory cost. */
typedef struct CagdSparseCellStruct {
    int CellRow;          /* The row number index of the cell in the matrix. */
    int CellCol;       /* The column number index of the cell in the matrix. */
    struct CagdSparseCellStruct *NextRow;/* Pointer to next cell in the row. */
    struct CagdSparseCellStruct *NextCol; /* Pointer to next cell in column. */
    IrtRType CellValue;                         /* Value stored in the cell. */
} CagdSparseCellStruct;

typedef struct CagdSparseMatStruct {
    int RowNum;	                        /* The number of rows in the matrix. */
    int ColNum;                       /* The number of column in the matrix. */
    CagdSparseCellStruct **RowStart;   /* Array of pointers to row of cells. */
    CagdSparseCellStruct **ColStart;  /* Array of pointers to cols of cells. */
    unsigned char *IndexIndicator;  /* Pointer to a bit array of indicators. */
    int NonZeroCellNumber;      /* Total number of non-zero cells in matrix. */
} CagdSparseMatStruct;

static CagdSparseCellStruct *CagdSparseCellAllocAux(void);
CagdSparseMatStruct *CagdSparseMatNew(int RowNum,
				      int ColNum,
				      int AddIndicator);
void CagdSparseMatFree(CagdSparseMatStruct *Mat);
static void CagdSparseCellInsertToRowListAux(CagdSparseMatStruct *Mat,
					     CagdSparseCellStruct *Cell);
static void CagdSparseCellInsertToColListAux(CagdSparseMatStruct *Mat,
					     CagdSparseCellStruct *Cell);
void CagdSparseMatNewCell(CagdSparseMatStruct *Mat,
			  int CellRow,
			  int CellCol,
			  IrtRType CellValue);
CagdSparseMatStruct *CagdSparseMatTranspose(CagdSparseMatStruct *Mat,
					    int AddIndicator);
static int CagdCheckIndicatorAux(CagdSparseMatStruct *Mat,
				 int CellRow,
				 int CellCol,
				 int MarkPosition);
IrtRType *CagdSparseMatMultNonSparseResult(CagdSparseMatStruct *Mat1,
					   CagdSparseMatStruct *Mat2);

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points, PtList, computes a Bspline surface of order UOrder  M
* by VOrder that interpolates or least square approximates the given set of  M
* points.	                                                             M
*   PtList is a NULL terminated array of linked lists of CagdPtStruct        M
* structs.                                                 	             M
*   All linked lists in PtList must have the same length.		     M
*   U direction of surface is associated with array, V with the linked       M
* lists.		                                   	             M
*   The size of the control mesh of the resulting Bspline surface defaults   M
* to the number of points in PtList (if SrfUSize = SrfVSize = 0).	     M
*   However, either numbers can smaller to yield a least square              M
* approximation of the gievn data set.                     	             M
*   The created surface can be parametrized as specified by ParamType.       M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:      A NULL terminating array of linked list of points.          M
*   UOrder:      Of the to be created surface.                               M
*   VOrder:      Of the to be created surface.                               M
*   SrfUSize:    U size of the to be created surface. Must be at least as    M
*                large as the array PtList.			             M
*   SrfVSize:    V size of the to be created surface. Must be at least as    M
*                large as the length of each list in PtList.                 M
*   ParamType:   Type of parametrization.                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed interpolating/approximating surface.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfInterpPts, interpolation, least square approximation               M
*****************************************************************************/
CagdSrfStruct *BspSrfInterpPts(const CagdPtStruct **PtList,
			       int UOrder,
			       int VOrder,
			       int SrfUSize,
			       int SrfVSize,
			       CagdParametrizationType ParamType)
{
    int i, NumLinkedLists, NumPtsInList;
    const CagdPtStruct *Pt, **PtPtr;
    CagdRType *UKV, *VKV, *PtUKnots, *PtVKnots, *RU, *RV;
    CagdSrfStruct *Srf;
    CagdCtlPtStruct
	*CtlPt = NULL,
	*CtlPtList = NULL;

    for (NumLinkedLists = 0, PtPtr = PtList;
	 *PtPtr != NULL;
	 NumLinkedLists++, PtPtr++);
    for (NumPtsInList = 0, Pt = *PtList;
	 Pt != NULL;
	 NumPtsInList++, Pt = Pt -> Pnext);

    if (SrfUSize == 0)
	SrfUSize = NumLinkedLists;
    if (SrfVSize == 0)
	SrfVSize = NumPtsInList;
    if (NumLinkedLists < 3 ||
	NumLinkedLists < UOrder ||
	NumLinkedLists < SrfUSize ||
	SrfUSize < UOrder ||
	NumPtsInList < 3 ||
	NumPtsInList < VOrder ||
	NumPtsInList < SrfVSize ||
	SrfVSize < VOrder)
	return NULL;

    RU = PtUKnots =
	(CagdRType *) IritMalloc(sizeof(CagdRType) * NumLinkedLists);
    RV = PtVKnots =
	(CagdRType *) IritMalloc(sizeof(CagdRType) * NumPtsInList);

    /* Compute parameter values at which surface will interpolate PtList. */
    switch (ParamType) {
	case CAGD_CHORD_LEN_PARAM:
        case CAGD_CENTRIPETAL_PARAM:
        case CAGD_NIELSON_FOLEY_PARAM:
	    /* No real support for chord length although we might be able to */
	    /* do something useful and better than uniform parametrization.  */
	case CAGD_UNIFORM_PARAM:
	default:
	    for (i = 0; i < NumLinkedLists; i++)
		*RU++ = ((CagdRType) i) / (NumLinkedLists - 1);
	    for (i = 0; i < NumPtsInList; i++)
		*RV++ = ((CagdRType) i) / (NumPtsInList - 1);
	    break;
    }

    /* Construct the two knot vectors of the Bspline surface. */
    UKV = BspPtSamplesToKV(PtUKnots, NumLinkedLists, UOrder, SrfUSize);
    VKV = BspPtSamplesToKV(PtVKnots, NumPtsInList, VOrder, SrfVSize);

    /* Convert to control points in a linear list */
    for (PtPtr = PtList; *PtPtr != NULL; PtPtr++) {
	for (Pt = *PtPtr, i = 0; Pt != NULL; Pt = Pt -> Pnext, i++) {
	    int j;

	    if (CtlPtList == NULL)
		CtlPtList = CtlPt = CagdCtlPtNew(CAGD_PT_E3_TYPE);
	    else {
		CtlPt -> Pnext = CagdCtlPtNew(CAGD_PT_E3_TYPE);
		CtlPt = CtlPt -> Pnext;
	    }
	    for (j = 0; j < 3; j++)
		CtlPt -> Coords[j + 1] = Pt -> Pt[j];
	}
	if (i != NumPtsInList) {
	    CagdCtlPtFreeList(CtlPtList);

	    IritFree(PtUKnots);
	    IritFree(PtVKnots);
	    IritFree(UKV);
	    IritFree(VKV);
    
	    CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);
	    return NULL;
	}
    }

    Srf = BspSrfInterpolate(CtlPtList, NumPtsInList, NumLinkedLists,
			    PtVKnots, PtUKnots, VKV, UKV,
			    SrfVSize, SrfUSize, VOrder, UOrder);
    CagdCtlPtFreeList(CtlPtList);

    IritFree(PtUKnots);
    IritFree(PtVKnots);
    IritFree(UKV);
    IritFree(VKV);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of points on a rectangular grid, PtList, parameter values the  M
* surface should interpolate or approximate these grid points, U/VParams,    M
* the expected two knot vectors of the surface, U/VKV, the expected lengths  M
* U/VLength and orders U/VOrder of the Bspline surface, computes the Bspline M
* surface's coefficients.						     M
*   All points in PtList are assumed of the same type.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:     A long linked list (NumUPts * NumVPts) of points to          M
*               interpolated or least square approximate.                    M
*   NumUPts:    Number of points in PtList in the U direction.               M
*   NumVPts:    Number of points in PtList in the V direction.               M
*   UParams:    Parameter at which surface should interpolate or             M
*               approximate PtList in the U direction.                       M
*   VParams:    Parameter at which surface should interpolate or             M
*               approximate PtList in the V direction.                       M
*   UKV:        Requested knot vector form the surface in the U direction.   M
*   VKV:        Requested knot vector form the surface in the V direction.   M
*   ULength:    Requested length of control mesh of surface in U direction.  M
*   VLength:    Requested length of control mesh of surface in V direction.  M
*   UOrder:     Requested order of surface in U direction.                   M
*   VOrder:     Requested order of surface in V direction.                   M
*									     *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed interpolating/approximating surface.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfInterpolate, interpolation, least square approximation             M
*****************************************************************************/
CagdSrfStruct *BspSrfInterpolate(const CagdCtlPtStruct *PtList,
				 int NumUPts,
				 int NumVPts,
				 const CagdRType *UParams,
				 const CagdRType *VParams,
				 const CagdRType *UKV,
				 const CagdRType *VKV,
				 int ULength,
				 int VLength,
				 int UOrder,
				 int VOrder)
{
    CagdPointType
	PtType = PtList -> PtType;
    CagdBType
	IsRational = CAGD_IS_RATIONAL_PT(PtType);
    int i, j,
	NumCoords = CAGD_NUM_OF_PT_COORD(PtType);
    CagdCtlPtStruct *PtLast;
    const CagdCtlPtStruct *Pt, *PtNext;
    CagdRType **SPoints;
    CagdSrfStruct *Srf;
    CagdCrvStruct **Crvs;

    /* Construct the Bspline surface and its two knot vectors. */
    Srf = BspSrfNew(ULength, VLength, UOrder, VOrder, PtType);
    SPoints = Srf -> Points;
    CAGD_GEN_COPY(Srf -> UKnotVector, UKV,
		  (ULength + UOrder) * sizeof(CagdRType));
    CAGD_GEN_COPY(Srf -> VKnotVector, VKV,
		  (VLength + VOrder) * sizeof(CagdRType));

    /* Interpolate the rows as set of curves. */
    Crvs = (CagdCrvStruct **) IritMalloc(sizeof(CagdCrvStruct *) * NumVPts);
    for (i = 0, Pt = PtList; i < NumVPts; i++) {
        /* Break the long list of control points into one row at a time.    */
        /* Note we only temporary change Pnext here.       Find end of row. */
        for (j = 1, PtLast = (CagdCtlPtStruct *) Pt; j < NumUPts; j++)
	    PtLast = PtLast -> Pnext;
	PtNext = PtLast -> Pnext;
	PtLast -> Pnext = NULL;

	if ((Crvs[i] = BspCrvInterpolate(Pt, UParams, UKV, ULength,
					 UOrder, FALSE)) == NULL) {
	    CagdSrfFree(Srf);
	    return NULL;
	}

	PtLast -> Pnext = (CagdCtlPtStruct *) PtNext;
	Pt = PtNext;
    }

    /* Interpolate the columns from the curves of the rows. */
    for (i = 0; i < ULength; i++) {
	int k;
	CagdCrvStruct *Crv;
	CagdCtlPtStruct
	    *CtlPtList = NULL,
	    *CtlPt = NULL;
	CagdRType **CPoints;

	for (j = 0; j < NumVPts; j++) {
	    CagdRType
		**Points = Crvs[j] -> Points;

	    if (CtlPtList == NULL)
		CtlPtList = CtlPt = CagdCtlPtNew(Crvs[j] -> PType);
	    else {
		CtlPt -> Pnext = CagdCtlPtNew(Crvs[j] -> PType);
		CtlPt = CtlPt -> Pnext;
	    }

	    for (k = !IsRational; k <= NumCoords; k++)
		CtlPt -> Coords[k] = Points[k][i];
	}

	/* Interpolate the column, copy to mesh, and free the curve. */
	if ((Crv = BspCrvInterpolate(CtlPtList, VParams, VKV, VLength,
				     VOrder, FALSE)) == NULL) {
	    CagdCtlPtFreeList(CtlPtList);
	    CagdSrfFree(Srf);
	    while (--i >= 0)
		CagdCrvFree(Crvs[i]);
	    IritFree(Crvs);
	    return NULL;
	}
	CPoints = Crv -> Points;
	CagdCtlPtFreeList(CtlPtList);

	for (j = 0; j < VLength; j++)
	    for (k = !IsRational; k <= NumCoords; k++)
		SPoints[k][i + j * ULength] = CPoints[k][j];

	CagdCrvFree(Crv);
    }

    for (i = 0; i < NumVPts; i++)
	CagdCrvFree(Crvs[i]);
    IritFree(Crvs);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Fits a surface to the give surface by sampling points on Srf and         M
* fitting a surface of orders U/Order and U/VSize control points.            M
*   Error is measured by the difference between the original and the fitted  M
* surface, as maximum error norm.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:         Surface to fit a new surface to.                            M
*   UOrder:      Of the to be created surface.                               M
*   VOrder:      Of the to be created surface.                               M
*   USize:       U size of the to be created surface.                        M
*   VSize:       V size of the to be created surface.                        M
*   ParamType:   Type of parametrization.                                    M
*   Err:         The maximum error is updated into here                      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Fitted surface.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BspSrfInterpPts                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfFitLstSqr                                                          M
*****************************************************************************/
CagdSrfStruct *BspSrfFitLstSqr(const CagdSrfStruct *Srf,
			       int UOrder,
			       int VOrder,
			       int USize,
			       int VSize, 
			       CagdParametrizationType ParamType,
			       CagdRType *Err)
{
    int i, j, OldInterp,
	USamples = UOrder + USize * BSP_SRF_FIT_SAMPLE_RATIO,
	VSamples = VOrder + VSize * BSP_SRF_FIT_SAMPLE_RATIO;
    CagdRType UMin, UMax, VMin, VMax, u, v, du, dv;
    CagdPtStruct *Pt,
	**PtListArray = (CagdPtStruct **)
	                 IritMalloc(sizeof(CagdPtStruct *) * (VSamples + 1));
    CagdSrfStruct *DSrf, *NewSrf;
    CagdBBoxStruct BBox;

    /* Create the samples on a grid BSP_SRF_FIT_SAMPLE_RATIO larger. */
    PtListArray[VSamples] = NULL;
    
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
    du = (UMax - UMin) / (USamples - 1);
    dv = (VMax - VMin) / (VSamples - 1);

    for (i = 0, v = VMin; i < VSamples; i++, v += dv) {
        Pt = PtListArray[i] = CagdPtNew();
	for (j = 0, u = UMin; j < USamples; j++, u += du) {
	    CagdRType
		*R = CagdSrfEval(Srf, u, v);

	    CagdCoerceToE3(Pt -> Pt, &R, -1, Srf -> PType); 

	    if (j < USamples - 1) {
	        Pt -> Pnext = CagdPtNew();
	        Pt = Pt -> Pnext;
	    }
	}
    }

    /* Construct the fitted surface. */
    NewSrf = BspSrfInterpPts((const CagdPtStruct **) PtListArray,
			     UOrder, VOrder, USize, VSize, ParamType);

    for (i = 0; i < VSamples; i++)
        CagdPtFreeList(PtListArray[i]);
    IritFree(PtListArray);

    if (NewSrf != NULL) {
        /* Match the domains. */
        BspKnotAffineTransOrder2(NewSrf -> UKnotVector, NewSrf -> UOrder,
				 CAGD_SRF_UPT_LST_LEN(NewSrf)
				     + NewSrf -> UOrder,
				 UMin, UMax);
	BspKnotAffineTransOrder2(NewSrf -> VKnotVector, NewSrf -> VOrder,
				 CAGD_SRF_VPT_LST_LEN(NewSrf)
				     + NewSrf -> VOrder,
				 VMin, VMax);

	/* Measure the error. */
	OldInterp = BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);

	DSrf = SymbSrfSub(Srf, NewSrf);
	CagdSrfBBox(DSrf, &BBox);
	CagdSrfFree(DSrf);

	u = IRIT_PT_LENGTH(BBox.Min);
	v = IRIT_PT_LENGTH(BBox.Max);
	*Err = IRIT_MAX(u, v);

	BspMultComputationMethod(OldInterp);
    }

    return NewSrf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Given a set of scattered points, PtList, computes a Bspline surface of     M
* order UOrder by VOrder that interpolates or least square approximates the  M
* given set of scattered points.                                             M
*   PtList is a NULL terminated lists of CagdPtStruct structs, with each     M
* point holding (u, v, x [, y[, z]]).  That is, E3 points create an E1       M
* scalar surface and E5 points create an E3 surface,           	             M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:      A NULL terminating array of linked list of points.          M
*   UOrder:      Of the to be created surface.                               M
*   VOrder:      Of the to be created surface.                               M
*   USize:       U size of the to be created surface.                        M
*   VSize:       V size of the to be created surface.                        M
*   UKV:	 Expected knot vector in U direction, NULL for uniform open. M
*   VKV:	 Expected knot vector in V direction, NULL for uniform open. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed interpolating/approximating surface.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfInterpScatPts, interpolation, least square approximation           M
*****************************************************************************/
CagdSrfStruct *BspSrfInterpScatPts(const CagdCtlPtStruct *PtList,
				   int UOrder,
				   int VOrder,
				   int USize,
				   int VSize,
				   CagdRType *UKV,
				   CagdRType *VKV)
{
    int i, j,
	NumCoords = CAGD_NUM_OF_PT_COORD(PtList -> PtType),
	PtListLen = CagdListLength(PtList),
	Size = USize * VSize;
    CagdBType
	NewUKV = FALSE,
	NewVKV = FALSE;
    CagdRType *M, *R, *InterpPts,
	*ULine = (CagdRType *) IritMalloc(sizeof(CagdRType) * UOrder),
	*Mat = (CagdRType *) IritMalloc(sizeof(CagdRType) * Size *
					IRIT_MAX(Size, PtListLen));
    const CagdCtlPtStruct *Pt;
    CagdSrfStruct *Srf;

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

    for (Pt = PtList, M = Mat; Pt != NULL; Pt = Pt -> Pnext, M += Size) {
	int UIndex, VIndex;
	CagdRType *VLine;

	if (NumCoords != CAGD_NUM_OF_PT_COORD(Pt -> PtType)) {
	    CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);
	    IritFree(ULine);
	    IritFree(Mat);
	    return NULL;
	}

	VLine = BspCrvCoxDeBoorBasis(UKV, UOrder, USize, FALSE,
				     Pt -> Coords[1], &UIndex);
	IRIT_GEN_COPY(ULine, VLine, sizeof(CagdRType) * UOrder);
	VLine = BspCrvCoxDeBoorBasis(VKV, VOrder, VSize, FALSE,
				     Pt -> Coords[2], &VIndex);

	for (j = VIndex; j < VIndex + VOrder; j++)
	    for (i = UIndex; i < UIndex + UOrder; i++)
		M[j * USize + i] = ULine[i - UIndex] * VLine[j - VIndex];
    }
    IritFree(ULine);

#   ifdef DEBUG
    {
        IRIT_SET_IF_DEBUG_ON_PARAMETER(_DebugPrintInputSvd, FALSE) {
	    for (i = 0; i < PtListLen; i++) {
	        IRIT_INFO_MSG("[");
		for (j = 0; j < Size; j++) {
		    IRIT_INFO_MSG_PRINTF("%7.4f ", Mat[i * Size + j]);
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

    /* Construct the Bspline surface and copy its knot vectors. */
    Srf = BspSrfNew(USize, VSize, UOrder, VOrder,
		    CAGD_MAKE_PT_TYPE(FALSE, NumCoords - 2));
    CAGD_GEN_COPY(Srf -> UKnotVector, UKV,
		  (CAGD_SRF_UPT_LST_LEN(Srf) + UOrder) * sizeof(CagdRType));
    CAGD_GEN_COPY(Srf -> VKnotVector, VKV,
		  (CAGD_SRF_VPT_LST_LEN(Srf) + VOrder) * sizeof(CagdRType));

    /* Solve for the coefficients of all the coordinates of the curve. */
    InterpPts = (CagdRType *) IritMalloc(sizeof(CagdRType) *
					 IRIT_MAX(Size, PtListLen));
    for (i = 3; i <= NumCoords; i++) {
	for (Pt = PtList, R = InterpPts; Pt != NULL; Pt = Pt -> Pnext)
	    *R++ = Pt -> Coords[i];

	SvdLeastSqr(NULL, Srf -> Points[i - 2], InterpPts, PtListLen, Size);
    }
    SvdLeastSqr(NULL, NULL, NULL, 0, 0);			/* Clean up. */
    IritFree(InterpPts);

    if (NewUKV)
	IritFree(UKV);
    if (NewVKV)
	IritFree(VKV);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function is a variation BspSrfInterpScatPts function that is less   M
* accurate/stable but is faster.					     M
*   The difference is that we solve a LSQ problem as A'*A*Vertices=A'*points M
* where A' is the transpose matrix of A.                                     M
*   This method is also refered to as pseudo inverse.                        M
*   The SVD decomposition is still used to calculate the above equation set. M
*   Given a set of scattered points, PtList, the function computes a Bspline M
* surface of order UOrder by VOrder that interpolates or least square        M
* approximates the M given set of scattered points.                          M
*   PtList is a NULL terminated lists of CagdPtStruct structs, with each     M
* point holding (u, v, x [, y[, z]]).  That is, E3 points create an E1       M
* scalar surface and E5 points create an E3 surface,           	             M
*                                                                            *
* PARAMETERS:                                                                M
*   PtList:      A NULL terminating array of linked list of points.          M
*   UOrder:      Of the to be created surface.                               M
*   VOrder:      Of the to be created surface.                               M
*   USize:       U size of the to be created surface.                        M
*   VSize:       V size of the to be created surface.                        M
*   UKV:	 Expected knot vector in U direction, NULL for uniform open. M
*   VKV:	 Expected knot vector in V direction, NULL for uniform open. M
*   MatrixCondition: address of a IrtRType to return SVD matrix              M
*                    condition number to. if NULL, this option is ignored    M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   Constructed interpolating/approximating surface.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BspSrfInterpScatPts2, interpolation, least square approximation          M
*****************************************************************************/
CagdSrfStruct *BspSrfInterpScatPts2(const CagdCtlPtStruct *PtList,
				    int UOrder,
				    int VOrder,
				    int USize,
				    int VSize,
				    CagdRType *UKV,
				    CagdRType *VKV,
				    CagdRType *MatrixCondition)
{
    int Row, i, j,
	NumCoords = CAGD_NUM_OF_PT_COORD(PtList -> PtType),
	PtListLen = CagdListLength(PtList),
	Size = USize * VSize;
    CagdBType
	NewUKV = FALSE,
	NewVKV = FALSE;
    CagdRType *MultMat, TempMatConditionNumber, *R, *P, *InterpPts, *RightSide,
	*ULine = (CagdRType *) IritMalloc(sizeof(CagdRType) * UOrder);
    CagdSparseMatStruct *Mat, *MatTranspose;
    CagdSparseCellStruct *SparseCell;
    const CagdCtlPtStruct *Pt;
    CagdSrfStruct *Srf;

    Mat = CagdSparseMatNew(IRIT_MAX(Size, PtListLen),Size,TRUE);	

    if (NumCoords < 3) {
	CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);
	return NULL;
    }

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

    for (Pt = PtList, Row = 0; Pt != NULL; Pt = Pt -> Pnext, Row++) {
        int UIndex, VIndex;
        CagdRType *VLine;

        if (NumCoords != CAGD_NUM_OF_PT_COORD(Pt -> PtType)) {
            CAGD_FATAL_ERROR(CAGD_ERR_PT_OR_LEN_MISMATCH);
            IritFree(ULine);
            IritFree(Mat);
            return NULL;
        }

        VLine = BspCrvCoxDeBoorBasis(UKV, UOrder, USize, FALSE,
            Pt -> Coords[1], &UIndex);
        IRIT_GEN_COPY(ULine, VLine, sizeof(CagdRType) * UOrder);
        VLine = BspCrvCoxDeBoorBasis(VKV, VOrder, VSize, FALSE,
            Pt -> Coords[2], &VIndex);

        for (j = VIndex; j < VIndex + VOrder; j++)
            for (i = UIndex; i < UIndex + UOrder; i++)
		CagdSparseMatNewCell(Mat, Row, (j * USize + i), 
				     ULine[i - UIndex] *
				         VLine[j - VIndex]);
    }
    IritFree(ULine);


    /* The LSQ problem may be solved using the following equation:           */
    /* A' * A * V = A' * P,  where,					     */
    /* A  - the matrix previously calculated				     */
    /* A' - is the tranpost of A					     */
    /* V - vertices							     */
    /* P - points to approximate					     */

    /* Calculate the tranpose matrix. */
    MatTranspose=CagdSparseMatTranspose(Mat, FALSE);

    /* Solve for the coefficients of all the coordinates of the curve. */
    InterpPts = (CagdRType *) IritMalloc(sizeof(CagdRType) * PtListLen);

    /* Calculate the right side of the equation. */
	
    /* Allocate Size * 3 (for easch component x, y, x). */
    RightSide = (CagdRType *) IritMalloc(sizeof(CagdRType) * Size * 3); 

    P = RightSide;

    /* Multiply A' * points and save result in vector for a later use. */
    for (i = 3; i <= NumCoords; i++) {
        for (Pt = PtList, R = InterpPts; Pt != NULL; Pt = Pt -> Pnext)
            *R++ = Pt -> Coords[i];

        for (j = 0; j < Size; j++, P++){
	    SparseCell = MatTranspose->RowStart[j];			
			
            *P = 0.0;
            while (SparseCell != NULL) {
                *P += SparseCell -> CellValue * InterpPts[SparseCell->CellCol];
                SparseCell = SparseCell -> NextCol;
            }
        }
    }

    IritFree(InterpPts);

    /* Calculate A' * A. */
    MultMat = CagdSparseMatMultNonSparseResult(MatTranspose,Mat);

    /* We don't need them anymore. */
    CagdSparseMatFree(Mat); 
    CagdSparseMatFree(MatTranspose);

    /* Compute SVD decomposition for Mat. */
    TempMatConditionNumber = IRIT_FABS(SvdLeastSqr(MultMat, NULL, NULL,
					      Size, Size));

    /* Return the matrix condition number. */
    if (MatrixCondition != NULL)
	*MatrixCondition = TempMatConditionNumber;

    if (TempMatConditionNumber < IRIT_UEPS && Size <= PtListLen) {
	CAGD_FATAL_ERROR(CAGD_ERR_NO_SOLUTION);

	IritFree(MultMat);
	return NULL;
    }

    IritFree(MultMat);

    /* Construct the Bspline surface and copy its knot vectors. */
    Srf = BspSrfNew(USize, VSize, UOrder, VOrder,
		    CAGD_MAKE_PT_TYPE(FALSE, NumCoords - 2));
    CAGD_GEN_COPY(Srf -> UKnotVector, UKV,
		  (CAGD_SRF_UPT_LST_LEN(Srf) + UOrder) * sizeof(CagdRType));
    CAGD_GEN_COPY(Srf -> VKnotVector, VKV,
		  (CAGD_SRF_VPT_LST_LEN(Srf) + VOrder) * sizeof(CagdRType));
    
    for ( i = 0; i < NumCoords-2; i++){
        SvdLeastSqr(NULL, Srf -> Points[i+1], RightSide+(i*Size), Size, Size);
    }

    IritFree(RightSide);

    if (NewUKV)
	IritFree(UKV);
    if (NewVKV)
	IritFree(VKV);

    return Srf;
}

/*****************************************************************************
* DESCRIPTION:		                                                     *
* Allocates memory for a sparse matrix cell structure. This is an auxilarry  *
* function of CagdSparseMatNewCell.                                          *
*	                                                                     *
* RETURN VALUE:								     *
*   CagdSparseCellStruct *: Pointer to allocates cell			     *
*									     * 
* SEE ALSO:                                                                  *
*   CagdSparseMatNew, CagdSparseMatNewCell                                   *
*****************************************************************************/
static CagdSparseCellStruct *CagdSparseCellAllocAux(void)
{
    CagdSparseCellStruct
	*Tmp = (CagdSparseCellStruct *) IritMalloc(sizeof(CagdSparseCellStruct));

    Tmp -> NextRow = NULL;
    Tmp -> NextCol = NULL;
    return Tmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Allocates memory and initialize a sparse matrix of size RowNum, ColNum.  M
*                                                                            *
* PARAMETERS:                                                                M
*   RowNum:       Number of rows in the matrix.                              M
*   ColNum:       Number of columns inthe matrix.                            M
*   AddIndicator: A Boolean indicating whether to allocate memory and        M
*                 initialize the indicator of cell existance.                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSparseMatStruct *: Pointer to allocated sparse matrix.               M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSparseMatFree                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSparseMatNew, Sparse matrix, allocation                              M
*****************************************************************************/
CagdSparseMatStruct *CagdSparseMatNew(int RowNum, int ColNum, int AddIndicator)
{
    int i;
    CagdSparseCellStruct **TmpCell;
    CagdSparseMatStruct
	*Tmp = (CagdSparseMatStruct *) IritMalloc(sizeof(CagdSparseMatStruct));

    Tmp -> RowNum = RowNum;
    Tmp -> ColNum = ColNum;

    Tmp -> RowStart = (CagdSparseCellStruct **)
	IritMalloc(sizeof(CagdSparseCellStruct) * RowNum);
    for (i = 0 , TmpCell = Tmp -> RowStart; i < RowNum; i++, TmpCell++)
        *TmpCell = NULL;

    Tmp -> ColStart = (CagdSparseCellStruct **)
	IritMalloc(sizeof(CagdSparseCellStruct) * ColNum);
    for (i = 0 , TmpCell = Tmp -> ColStart; i < ColNum; i++, TmpCell++) 
        *TmpCell = NULL;

    if (AddIndicator) {
        Tmp -> IndexIndicator = (unsigned char *)
	    IritMalloc(sizeof(unsigned char) * 
			(ColNum * (RowNum / 8 + 1)));/* Indicator bit array. */
        IRIT_ZAP_MEM(Tmp -> IndexIndicator,
		sizeof(unsigned char) * (ColNum * (RowNum / 8 + 1)));
    }
    else
	Tmp -> IndexIndicator = NULL;

    Tmp -> NonZeroCellNumber = 0;

    return Tmp;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Free the memory of the sparse matrix (including all the memory allocated M
* for the cells and for the indicator array.                                 M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:   Pointer to the sparse matrix.                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void					                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   CagdSparseMatNew                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   CagdSparseMatFree, Sparse matrix, allocation                             M
*****************************************************************************/
void CagdSparseMatFree(CagdSparseMatStruct *Mat)
{
    int i;
    CagdSparseCellStruct *Cell, *NextCell;	

    for (i = 0; i < Mat -> RowNum; i++) {
        Cell = Mat -> RowStart[i];
        while (Cell != NULL) {
            NextCell = Cell -> NextCol;
            IritFree(Cell);
            Cell = NextCell;
        }
    }

    if (Mat -> IndexIndicator != NULL)
	IritFree(Mat -> IndexIndicator);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add a cell to the appropriate row in the sparse matrix. The Cell row     *
* number must be set before the use of this function.  Notice that the cell  *
* is not added to the column list in this function. This is an auxilarry     *
* function of CagdSparseMatNewCell.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:   Pointer to the sparse matrix.                                     *
*   Cell:  Pointer to the cell to add to the matrix.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CagdSparseCellInsertToRowListAux(CagdSparseMatStruct *Mat,
					     CagdSparseCellStruct *Cell)
{
    CagdSparseCellStruct **ListStart;

    /* First find the row start you want to inset to... */
    ListStart = Mat->RowStart + Cell->CellRow;

    /* In our case order does not matter, so just insert at the start. */
    Cell -> NextCol = *ListStart;        
    *ListStart = Cell;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add a cell to the appropriate column in the sparse matrix. The Cell col  *
* number must be set before the use of this function. Notice that the cell   *
* is not added to the row list in this function. This is an auxilarry        *
* function of CagdSparseMatNewCell.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:   Pointer to the sparse matrix.                                     *
*   Cell:  Pointer to the cell to add to the matrix.                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CagdSparseCellInsertToColListAux(CagdSparseMatStruct *Mat,
					     CagdSparseCellStruct *Cell)
{
    CagdSparseCellStruct **ListStart;

    /* First find the row start you want to inset to... */
    ListStart = Mat->ColStart + Cell->CellCol;

    /* In our case order does not matter, so just insert at the start. */
    Cell -> NextRow = *ListStart; 
    *ListStart = Cell;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Add a cell to the sparse matrix in the appropriate row and column.       M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:       Pointer to the sparse matrix.                                 M
*   CellRow:   The new cell row index.                                       M
*   CellCol:   The new cell col index.                                       M
*   CellValue: The Value stored in the cell.                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*     CagdSparseMatNew                                                       M 
*                                                                            *
* KEYWORDS:								     M
    CagdSparseMatNewCell, Sparse matrix, allocation		             M
*****************************************************************************/
void CagdSparseMatNewCell(CagdSparseMatStruct *Mat,
			  int CellRow,
			  int CellCol,
			  IrtRType CellValue)
{
    CagdSparseCellStruct
	*Cell = CagdSparseCellAllocAux();

    Cell -> CellRow = CellRow;
    Cell -> CellCol = CellCol;
    Cell -> CellValue = CellValue;
    CagdSparseCellInsertToRowListAux(Mat , Cell);
    CagdSparseCellInsertToColListAux(Mat , Cell);
    CagdCheckIndicatorAux(Mat,CellRow,CellCol,TRUE);/* Mark indicator array. */
    Mat -> NonZeroCellNumber++;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function checks or marks the sparse matrix indicator array. if mark *
* position is TRUE then the indicator bit array is marked to indicate that   *
* the cell positioned in CellRow, CellCol is non zero. Otherwise no mark     *
* is made.                                                                   *
*   The function returns TRUE if the cell positioned in CellRow,CellCol is   *
* non zero.                                                                  *
*   This is an auxilarry function of                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Mat:          Pointer to the sparse matrix.                              *
*   CellRow:      The new cell row index.                                    *
*   CellCol:      The new cell col index.                                    *
*   MarkPosition: if TRUE the indicator array will be set                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static int CagdCheckIndicatorAux(CagdSparseMatStruct *Mat,
				 int CellRow,
				 int CellCol,
				 int MarkPosition)
{
    int RowIndicator, BitRowIndicator;
    unsigned char BitRowIndicatorMask, *PIndicator;

    /* If no indicator exists return TRUE so that no filtering will be     */
    /* done by the indicator.					           */
    if (Mat -> IndexIndicator == NULL)
	return (TRUE); 

    RowIndicator = CellRow / 8;
    BitRowIndicator = CellRow % 8;

	/* the switch is here for speed reasons although it may seem dirty */
    switch (BitRowIndicator) {
        case 0:
            BitRowIndicatorMask = 0x01;
            break;
        case 1:
            BitRowIndicatorMask = 0x02; 
            break;
	case 2:
            BitRowIndicatorMask = 0x04;
            break;
        case 3:
            BitRowIndicatorMask = 0x08;
            break;
	case 4:
            BitRowIndicatorMask = 0x10;
            break;
	case 5:
            BitRowIndicatorMask = 0x20;
            break;
        case 6:
            BitRowIndicatorMask = 0x40;
            break;
        case 7:
            BitRowIndicatorMask = 0x80; 
            break;
        default:
            BitRowIndicatorMask = 0x00; 
	    assert(0);
	    break;
    }
	
    PIndicator = &(Mat -> IndexIndicator[CellCol * (Mat -> RowNum / 8 + 1)
					 + RowIndicator]);
    if (MarkPosition)
	*PIndicator = *PIndicator | BitRowIndicatorMask;
	
    return (*PIndicator) & BitRowIndicatorMask;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Multiply two sparse matrices (the result is non-sparse matrix),         M
* Result = Mat1 * Mat2.                                                      M
*    NULL will be returned if the input matrix sizes' are incompatible.      M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat1:   Pointer to first sparse matrix.                                  M
*   Mat2:   Pointer to second sparse matrix.                                 M
*                                                                            *
* RETURN VALUE:	                                                             M
*   IrtRType *: An array containing the member values of the non sparse      M
*               matrix. The array size will be (Mat1->RowNum * Mat2->ColNum) M
*               The matrix member should be accessed as                      M
*               Result[Row*Mat2->ColNum +Col]                                M
*                                                                            *
* KEYWORDS:								     M
*   CagdSparseMatMultNonSparseResult, Sparse matrix, Multiplication          M
*****************************************************************************/
IrtRType *CagdSparseMatMultNonSparseResult(CagdSparseMatStruct *Mat1,
					   CagdSparseMatStruct *Mat2)
{
    int Row, Col;
    IrtRType *ResMat;

    if (Mat1 -> ColNum != Mat2 -> RowNum) 
        return NULL;

    ResMat = IritMalloc(sizeof(IrtRType) * Mat1 -> RowNum * Mat2 -> ColNum);

    IRIT_ZAP_MEM(ResMat, sizeof(IrtRType) * Mat1 -> RowNum * Mat2 -> ColNum);

    for (Row = 0; Row < Mat1 -> RowNum ; Row++) {
        for (Col = 0 ; Col < Mat2 -> ColNum ; Col++){
            CagdSparseCellStruct *Mat1Cell , *Mat2Cell;
            Mat1Cell = Mat1 -> RowStart[Row];
            while (Mat1Cell!=NULL){
                /* Check if a suitable cell exists in the other array by     */
		/* indicator.						     */
                if (CagdCheckIndicatorAux(Mat2, Mat1Cell -> CellCol,
					  Col, FALSE)) {
		    /* If a suitable cell exists (or indicator not used),    */
		    /* search for it in the linked list.		     */
                    Mat2Cell = Mat2 -> ColStart[Col];
                    while (Mat2Cell != NULL){
                        if (Mat1Cell -> CellCol == Mat2Cell -> CellRow) {
                            ResMat[Row * Mat2 -> ColNum + Col] += 
                                Mat1Cell -> CellValue * Mat2Cell -> CellValue; 
			    break;
                        }
			Mat2Cell = Mat2Cell -> NextRow;
                    }
                }
                Mat1Cell = Mat1Cell -> NextCol;
            }
        }
    }

    return ResMat;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Returns the tranpose of the matrix in the input. The newly allocated     M
* matrix is returned in sparse format (with or without an indicator          M
* as pointed in the AddIndicator parameter).		                     M
*                                                                            *
* PARAMETERS:                                                                M
*   Mat:          Pointer to the sparse matrix we want to transpose.         M
*   AddIndicator: If TRUE, the returned matrix will contain a bit indicator. M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSparseMatStruct *: Newly allocated sparse matrix which is the        M
*                   transpose of the input matrix.                           M
*                                                                            *
* KEYWORDS:								     M
*   CagdSparseMatTranspose, Sparse matrix, Transpose  			     M
*****************************************************************************/
CagdSparseMatStruct *CagdSparseMatTranspose(CagdSparseMatStruct *Mat,
					    int AddIndicator)
{
    int Row;
    CagdSparseMatStruct *ResMat;
	
    ResMat = CagdSparseMatNew(Mat -> ColNum, Mat -> RowNum,AddIndicator);
	
    for (Row = 0; Row < Mat -> RowNum ; Row++) {
	CagdSparseCellStruct *CurrCell;

	CurrCell = Mat -> RowStart[Row];
	while (CurrCell != NULL) {
	    CagdSparseMatNewCell(ResMat,
				 CurrCell -> CellCol,
				 CurrCell -> CellRow,
				 CurrCell -> CellValue);
	    CurrCell = CurrCell -> NextCol;
	}
    }

    return ResMat;
}
