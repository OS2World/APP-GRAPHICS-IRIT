/******************************************************************************
 * mvtrmptcrv.c - Orientation (L) and curvature (C) constraints.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Muthuganapathy Ramanathan,  March 2005.             	      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h> 
#include "geom_lib.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h" 
#include "mvar_loc.h"

#define DUPLICATE_E1_TO_E3_CRV(CrvE3, CrvE1) { \
		CrvE3 = CagdCoerceCrvTo(CrvE1, CAGD_PT_E3_TYPE, FALSE); \
		CAGD_GEN_COPY(CrvE3 -> Points[2], CrvE3 -> Points[1], \
			      sizeof(CagdRType) * CrvE3 -> Length); \
		CAGD_GEN_COPY(CrvE3 -> Points[3], CrvE3 -> Points[1], \
			      sizeof(CagdRType) * CrvE3 -> Length); \
	    }

/******************************************************************************
* AUXILIARY:   Trimming functions of bisector of Crv and Pt  		      *
******************************************************************************/
CagdCrvStruct *MvarBsctDenomPtCrvBis(CagdCrvStruct *Crv,
				     CagdPType Pt,
				     CagdRType Alpha);
CagdCrvStruct *MvarBsctCrvPtLeft(CagdCrvStruct *Crv, 
				 CagdRType *Pt, 
				 CagdRType Alpha); 
CagdCrvStruct *MvarBsctCrvPtCurvature(CagdCrvStruct *Crv, 
				      CagdRType *Pt, 
				      CagdRType Alpha); 
CagdCrvStruct *MvarBsctTrimCrvPtPair(CagdCrvStruct *Crv, 
				     CagdRType *Pt, 
				     CagdRType Alpha); 
CagdPtStruct *MvarBsctComputeCrvPtBis(CagdCrvStruct *Crv, 
				      CagdRType *Pt,
				      CagdRType t);
CagdBType MvarBsctCheckFootPtEqualsMinDistPt(CagdCrvStruct *Crv1,
					     CagdRType *Pt,
					     CagdPType BP); 
CagdRType MvarBsctIsCrvLeftToLine(CagdCrvStruct *Crv, 
				  CagdRType *Pt,
				  CagdPType Normal); 
static CagdPtStruct *MvarGetMiniumIntnPar(CagdCrvStruct *TrimmedBis, 
					  CagdRType *Pt,
					  CagdPtStruct *Inter,
					  CagdPType LeftNormal);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the denominator of the alpha-/bi-sector curve of a planar curve M
* and a point, all in the XY plane.  The result is the denominator of the    M
* solution to the following two linear equations in alpha-/bi-sector's two   M
* unknowns, the x and y coefficients:                                        M
*									     M
*	<C'(t),     B(t)> = <C'(t), C(t)>				     V
*	<C(t) - Pt, B(t)> = <C(t) - Pt, a Pt + (1 - a) C(t)>		     V
*									     M
* where a is the Alpha of the alpha-sector, 0.5 for a bisector, Pt is the    M
* point entity, C(t) is the curve entity and B(t) is the saught bisector.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:          Planar curve to compute its bisector curve with Pt.        M
*   Pt:           A point in the plane to compute its bisector with Crv.     M
*   Alpha:        Alpha-sector ratio (0.5 for a bisector).		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *:   The bisector curve, in the XY plane.                  M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvDiameter, SymbCrvCnvxHull, SymbCrvBisectorsSrf,                   M
*   SymbCrvCrvBisectorSrf3D, SymbSrfPtBisectorSrf3D, SymbCrvPtBisectorSrf3D  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctDenomPtCrvBis, bisector                                          M
*****************************************************************************/
CagdCrvStruct *MvarBsctDenomPtCrvBis(CagdCrvStruct *Crv,
				     CagdPType Pt,
				     CagdRType Alpha)
{
    CagdBType
	IsRational = MVAR_IS_RATIONAL_PT(Crv -> PType);
    CagdCrvStruct *DCrv, *CrvE3, *DCrvE3, *TCrv1, *TCrv2, *TCrv3, *TCrv4,
	*A11, *A12, *A21, *A22, *B1, *B2, *CrvW;

    Crv = CagdCoerceCrvTo(Crv, IsRational ? CAGD_PT_P2_TYPE : CAGD_PT_E2_TYPE,
			  FALSE);
    DCrv = CagdCrvDerive(Crv);
    if (IsRational) {
        /* Constructs an E2/E3 version of these two rational curves. */
	SymbCrvSplitScalar(DCrv, &CrvW, &TCrv1, &TCrv2, &TCrv3);
	CagdCrvFree(CrvW);
	DCrvE3 = SymbCrvMergeScalar(NULL, TCrv1, TCrv2, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);
	SymbCrvSplitScalar(Crv, &CrvW, &TCrv1, &TCrv2, &TCrv3);
	CrvE3 = SymbCrvMergeScalar(NULL, TCrv1, TCrv2, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);
	SymbCrvSplitScalar(Crv, &CrvW, &TCrv1, &TCrv2, &TCrv3);
	CrvE3 = SymbCrvMergeScalar(NULL, TCrv1, TCrv2, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);

        /* Prepare the 2x2 determinant coefficients. */
	TCrv1 = SymbCrvMultScalar(DCrvE3, CrvW);
	SymbCrvSplitScalar(TCrv1, &TCrv2, &A11, &A12, &TCrv3);
	CagdCrvFree(TCrv2);
	if (TCrv3 != NULL)
	    CagdCrvFree(TCrv3);

	DUPLICATE_E1_TO_E3_CRV(TCrv4, CrvW);
	CagdCrvScale(TCrv4, Pt);
	TCrv3 = SymbCrvSub(CrvE3, TCrv4);
	TCrv2 = CagdCrvCopy(TCrv3);
	TCrv1 = SymbCrvMultScalar(TCrv2, CrvW);
	CagdCrvFree(TCrv2);
	CagdCrvFree(CrvW);
	SymbCrvSplitScalar(TCrv1, &CrvW, &A21, &A22, &TCrv2);
	CagdCrvFree(TCrv1);
	if (TCrv2 != NULL)
	    CagdCrvFree(TCrv2);

	/* And the B coefficients in the "Ax = B" system. */
	B1 = SymbCrvDotProd(DCrvE3, CrvE3);

	CagdCrvTransform(TCrv4, NULL, Alpha);
	TCrv2 = CagdCrvCopy(CrvE3);
	CagdCrvTransform(TCrv2, NULL, 1.0 - Alpha);
	TCrv1 = SymbCrvAdd(TCrv2, TCrv4);
	B2 = SymbCrvDotProd(TCrv1, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv2);
	CagdCrvFree(TCrv3);
	CagdCrvFree(TCrv4);

	CagdCrvFree(CrvE3);
	CagdCrvFree(DCrvE3);
    }
    else {
	CagdVType Trans;

        /* Prepare the 2x2 determinant coefficients. */
	SymbCrvSplitScalar(DCrv, &CrvW, &A11, &A12, &TCrv2);
	if (TCrv2 != NULL)
	    CagdCrvFree(TCrv2);

	Trans[0] = -Pt[0];
	Trans[1] = -Pt[1];
	Trans[2] = 0.0;
	TCrv3 = CagdCrvCopy(Crv);
	CagdCrvTransform(TCrv3, Trans, 1.0);
	SymbCrvSplitScalar(TCrv3, &CrvW, &A21, &A22, &TCrv2);
	if (TCrv2 != NULL)
	    CagdCrvFree(TCrv2);

	/* And the B coefficients in the "Ax = B" system. */
        B1 = SymbCrvDotProd(DCrv, Crv);

	TCrv1 = CagdCrvCopy(Crv);
	CagdCrvTransform(TCrv1, NULL, 1.0 - Alpha);
	Trans[0] = Alpha * Pt[0];
	Trans[1] = Alpha * Pt[1];
	Trans[2] = 0.0;
	CagdCrvTransform(TCrv1, Trans, 1.0);
	B2 = SymbCrvDotProd(TCrv1, TCrv3);
	CagdCrvFree(TCrv1);
	CagdCrvFree(TCrv3);
    }

    TCrv1 = SymbCrvDeterminant2(A11, A12,
				A21, A22);
    TCrv2 = SymbCrvDeterminant2(B1, A12,
				B2, A22);
    TCrv3 = SymbCrvDeterminant2(A11, B1,
				A21, B2);
    CagdCrvFree(A11);
    CagdCrvFree(A12);
    CagdCrvFree(A21);
    CagdCrvFree(A22);
    CagdCrvFree(B1);
    CagdCrvFree(B2);

    CagdCrvFree(TCrv2);
    CagdCrvFree(TCrv3);

    CagdCrvFree(DCrv);
    CagdCrvFree(Crv);

    return TCrv1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a Crv and a Pt, this function computes the point on the bisector   M
*   using determinants for a particular parameter t on the curve.            M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  The input curve - CagdCrvStruct.                                   M
*   Pt:  The input point - CagdRType.                                        M
*   t: Parameter on the curve Crv - CagdRType.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *: The identified bisector point.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbCrvPtBisectorCrv2D                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctComputeCrvPtBis                                                  M
*****************************************************************************/
CagdPtStruct *MvarBsctComputeCrvPtBis(CagdCrvStruct *Crv, 
				      CagdRType *Pt,
				      CagdRType t)
{
    CagdCrvStruct *Crv1;
    CagdPType Pt1, TanVec;
    CagdPtStruct 
        *bis = (CagdPtStruct *) IritMalloc(sizeof(CagdPtStruct));
    CagdRType *R, rhs1, rhs2, A21, A22, DeterA, DeterB, Deter;
    CagdVecStruct *TVec;
    
    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    
    if (CAGD_IS_BEZIER_CRV(Crv1)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }
    
    /* Compute P(t). */   
    R = CagdCrvEval(Crv1, t); 	   
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType); 
  
    TVec = CagdCrvTangent(Crv1, t, FALSE);
    IRIT_PT_COPY(TanVec, TVec -> Vec);
    rhs1 = (Pt1[0] * TanVec[0]) + (Pt1[1] * TanVec[1]);
    rhs2 = ((Pt[0] + Pt1[0]) / 2.0) * (Pt1[0] - Pt[0]) +
           ((Pt[1] + Pt1[1]) / 2.0) * (Pt1[1] - Pt[1]);
    A21 = Pt1[0] - Pt[0];
    A22 = Pt1[1] - Pt[1];
    DeterA = (rhs1 * A22) - (rhs2 * TanVec[1]);
    DeterB = (TanVec[0] * rhs2) - (rhs1 * A21);
    Deter = (TanVec[0] * A22) - (TanVec[1] * A21);
    bis -> Pt[0] = DeterA / Deter;
    bis -> Pt[1] = DeterB / Deter;

    /* Raman: Mem. leak sealing. */
    CagdVecFree(TVec);
    CagdCrvFree(Crv1);
    return bis;
} 

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point satisfiesM
*   the Left constraint.                				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  The input curve - CagdCrvStruct.                                   M
*   Pt:  The input point - CagdRType.                                        M
*   Alpha: Parameter of the alpha-sector - CagdRType. 0.5 for bisector.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: CagdCrvStruct of the resultant.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctCrvPtCurvature                                                   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctCrvPtLeft                                                        M
*****************************************************************************/
CagdCrvStruct *MvarBsctCrvPtLeft(CagdCrvStruct *Crv, 
				 CagdRType *Pt, 
				 CagdRType Alpha) 
{
    IrtRType DotProdAssert;
    CagdCrvStruct *Crv1, *Crv2, *BisectCrv1, *BisectCrvCpy,
        *SubDivCrv1, *DenomCrv, 
        *LeftPtCrv = NULL;
    CagdRType UMid, TMin, TMax;
    MvarMVStruct *MVVec[1];
    MvarPtStruct *MVPts, *MVPts1, *MVPts2;
    MvarConstraintType Constraints[1];
    CagdPType Pt1, BP, Nrml1, DotProd, Tangent, ResSub;
    CagdRType *R, *R2;
    CagdVecStruct *Vec, *CrvTangent;
 
    /* Computing the bisector curve of the given pt and curve. */

    Crv2 = CagdCrvCopy(Crv);
    BisectCrv1 = SymbCrvPtBisectorCrv2D(Crv2, Pt, 0.5);
    CagdCrvDomain(BisectCrv1, &TMin, &TMax);

    /* The constratint part starts from here. */

    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    
    if (CAGD_IS_BEZIER_CRV(Crv1)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }
   
    /* Computing the parameters at which the DenomCrv is zero. */
    
    DenomCrv = MvarBsctDenomPtCrvBis(Crv2, Pt, 0.5);
    CagdCrvFree(Crv2);

    MVVec[0] = MvarCrvToMV(DenomCrv);
    /* MEM Leak Sealed. */
    CagdCrvFree(DenomCrv);

    Constraints[0] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVVec, Constraints, 1, MvarBsctSubdivTol, 
			 MVAR_BSCT_NUMER_TOL);
    if (MVPts != NULL)
        MVPts1 = MvarPtSortListAxis(MVPts, 1);

    MvarMVFree(MVVec[0]);
    
    /* Appending TMin and TMax to the list of MVPt1. */
    
    if (MVPts != NULL) {
        MvarPtStruct *MVTmp, *MVTmp1;
	
	/* Adding the TMin as the Head of the list */

	MVTmp1 = MVPts1;
	if (TMin < MVTmp1 -> Pt[0]) {
	    MVTmp = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct));
	    MVTmp -> Pt = (CagdRType *) IritMalloc(sizeof(CagdRType) * 3); 
	    MVTmp -> Pt[0] = TMin;
            /* why not use IRIT_LIST_PUSH(MVTmp, MVPts)? */
	    MVTmp -> Pnext = MVPts1;
	    MVPts1 = MVTmp;
#           ifdef DEBUG_VORONOI
	    printf("Adding to the HEAD of the list is succeeded\n");
#           endif
	}

	/* Adding the TMax as the Tail of the list */

	MVTmp1 = CagdListLast(MVPts1);
	
	if (TMax > MVTmp1 -> Pt[0]) {
	    MVTmp = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct));
	    MVTmp -> Pt = (CagdRType *) IritMalloc(sizeof(CagdRType) * 3); 
	    MVTmp -> Pt[0] = TMax;
            /* why not use CagdListAppend(MVTmp1, MVPts) ?*/
	    MVTmp1 -> Pnext = MVTmp;
	    MVTmp -> Pnext = NULL;
#           ifdef DEBUG_VORONOI
	    printf("Adding to the TAIL of the list is succeeded\n");
#           endif
	}
    }
    /* Subdividing the bisector curve between each segment and chekc for */
    /* orientation criterion at the mid parameter and purging away if it */
    /* is not satisfied.                                                 */
    
    if (MVPts != NULL) {
    
        /* Checking between points of MVPts2 -> Pt[0]. */
        MVPts2 = MVPts1;
	while (MVPts2 -> Pnext != NULL) {
	    UMid = (MVPts2 -> Pt[0] + MVPts2 ->Pnext -> Pt[0]) / 2.0;
	
	    /* Compute the bisector point at the parameter */
    
	    R = CagdCrvEval(BisectCrv1, UMid); 	   
	    CagdCoerceToE3(BP, &R, -1, BisectCrv1 -> PType); 

	    /* Computing the parameter correspoing to the bisector point on */
	    /* the boundary curve using the distance between the curve and  */
	    /* a point.                                                     */
	    
	    /* Evaluation the point and curvature of Crv1 at UMid. */

	    R2 = CagdCrvEval(Crv1, UMid); 
	    CagdCoerceToE3(Pt1, &R2, -1, Crv1 -> PType); 

	    /* Asserting UMid gives the correct bisector point, the    */
	    /* following is checked : (crv - bp).crv' = 0.             */

	    CrvTangent = CagdCrvTangent(Crv1, UMid, TRUE);
	    IRIT_PT_COPY(Tangent, CrvTangent -> Vec);   

	    IRIT_PT2D_SUB(ResSub, Pt1, BP);

	    DotProdAssert = IRIT_DOT_PROD_2D(ResSub, Tangent);
	    
	    if (IRIT_FABS(DotProdAssert) <= MVAR_BSCT_NUMER_TOL) {
#               ifdef DEBUG_VORONOI
	        printf("Dot product ASSERTION passed\n");
#               endif
	    }
	    else {
#               ifdef DEBUG_VORONOI
	        printf("Dot product ASSERTION failed : exiting\n");
#               endif
		MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
	    }
	    /*  Compute the dot product < P - C1, N1>              	 */
	    /*  and evaluate the curvature constraint.               */
	    /*  CagdCrvNormalXY should give the `right' normal.       */    
	    	
	    Vec = CagdCrvNormalXY(Crv1, UMid, TRUE);   
	    IRIT_PT_COPY(Nrml1, Vec -> Vec);   

	    /* Compute the dot product < P - C1, N1> and < P - C2, N2>. */
	    DotProd[0] = (BP[0] - Pt1[0]) * (-Nrml1[0]) +
	      (BP[1] - Pt1[1]) * (-Nrml1[1]);  
     
	    if (DotProd[0] >= 0.0) { 
#               ifdef DEBUG_VORONOI
	            printf("Orientation condition is satisfied\n");
#               endif
		BisectCrvCpy = CagdCrvCopy(BisectCrv1);
		SubDivCrv1 = CagdCrvRegionFromCrv(BisectCrvCpy, 
						MVPts2 -> Pt[0],
						MVPts2 -> Pnext -> Pt[0]);
		IRIT_LIST_PUSH(SubDivCrv1, LeftPtCrv); 
		CagdCrvFree(BisectCrvCpy);
	    } 	
	    else {     
#               ifdef DEBUG_VORONOI    
	            printf("Orientation condition is NOT satisfied\n");
#               endif
	    }
	    MVPts2 = MVPts2 -> Pnext;
	}
    }
    else {
#       ifdef DEBUG_VORONOI
            printf("No zeros of left constraint is found\n");
	    printf("returning the bisector curve\n");
#       endif

        /* MEM Leak fixed. */
	LeftPtCrv = CagdCrvCopy(BisectCrv1);
    }

    /* MEM Leak fixed. */
    CagdCrvFree(BisectCrv1);
    CagdCrvFree(Crv1);
    return LeftPtCrv;
}     

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point satisfiesM
*   the Curvature constraint.                   			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  The input curve - CagdCrvStruct.                                   M
*   Pt:  The input point - CagdRType.                                        M
*   Alpha: Parameter of the alpha-sector - CagdRType. 0.5 for bisector.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: CagdCrvStruct of the resultant.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctCrvPtLeft                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctCrvPtCurvature                                                   M
*****************************************************************************/
CagdCrvStruct *MvarBsctCrvPtCurvature(CagdCrvStruct *Crv, 
				      CagdRType *Pt, 
				      CagdRType Alpha) 
{
    CagdCrvStruct *Crv1, *Crv2, *NkCrv1, *SubDivCrv1, *SubDivCrv2, *LeftPtCrv,
        *DBisectCrv1, *DBCrvW, *DBCrvX, *DBCrvY, *DBCrvZ, *LeftPtCrvTmp,
        *CCPtCrv = NULL;
    CagdRType UMid, TMin, TMax, *R, *R2, *R4;
    MvarMVStruct *MVVec1[1];
    MvarPtStruct *MVPts2, *MVPts3, *MVPts4, *Head1, *Head2, *Tmp;
    MvarConstraintType Constraints1[1];
    CagdPType Pt1, BP, DPCurvature, kN1;
      
    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    
    if (CAGD_IS_BEZIER_CRV(Crv1)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }

    /* Computing the kN curve from curvatur.c. */

    NkCrv1 = SymbCrv3DCurvatureNormal(Crv1);

    /* Calling the Orientation constraint function. */

    Crv2 = CagdCrvCopy(Crv);
    LeftPtCrv = MvarBsctCrvPtLeft(Crv2, Pt, 0.5);
    CagdCrvFree(Crv2);
 
    /* Check for the curvature constraint starts from here. */

    if (LeftPtCrv != NULL) {
        LeftPtCrvTmp = LeftPtCrv;
	while (LeftPtCrvTmp != NULL) {
	    CagdCrvStruct *CrvTmp1, *CrvTmp2, *CrvTmp3;
	    DBisectCrv1 = CagdCrvDerive(LeftPtCrvTmp);
	    SymbCrvSplitScalar(DBisectCrv1, &DBCrvW, &DBCrvX, 
			       &DBCrvY, &DBCrvZ);
            /* MEM leak sealed. */
            CagdCrvFree(DBisectCrv1);
            CagdCrvFree(DBCrvW);
            CagdCrvFree(DBCrvZ);
   
	    CrvTmp1 = SymbCrvMultScalar(DBCrvX, DBCrvX);
	    CrvTmp2 = SymbCrvMultScalar(DBCrvY, DBCrvY);
	    CrvTmp3 = SymbCrvAdd(CrvTmp1, CrvTmp2);
	    MVVec1[0] = MvarCrvToMV(CrvTmp3);
            CagdCrvFree(DBCrvX);
            CagdCrvFree(DBCrvY);
	    CagdCrvFree(CrvTmp1);
	    CagdCrvFree(CrvTmp2);
	    CagdCrvFree(CrvTmp3);
			
	    Constraints1[0] = MVAR_CNSTRNT_ZERO;

	    MVPts2 = MvarMVsZeros(MVVec1, Constraints1, 1, MvarBsctSubdivTol, 
				  MVAR_BSCT_NUMER_TOL);

            /* MEM leak sealed. */
            MvarMVFree(MVVec1[0]);
	    if (MVPts2 != NULL)
	        MVPts3 = MvarPtSortListAxis(MVPts2, 1);

	    /* Purging away points that are close to each other. */

	    if (MVPts2 != NULL) {
	        Head1 = MVPts3;
		Head2 = MVPts3 -> Pnext;
		while(Head2 != NULL) {
		    if (IRIT_APX_EQ_EPS(Head1 -> Pt[0], Head2 -> Pt[0], 
				   IRIT_FABS(10.0 * MVAR_BSCT_NUMER_TOL))) {
		      Tmp = Head2 -> Pnext;
		      MvarPtFree(Head2);

		      Head2 = Tmp;
		      Head1 -> Pnext = Head2;                  
		    }
		    else {
		        Head1 = Head1 -> Pnext;
			Head2 = Head2 -> Pnext;
		    }
		}
	    }

	    CagdCrvDomain(LeftPtCrvTmp, &TMin, &TMax);
	    if (MVPts2 != NULL) {
	        MvarPtStruct *MVTmp, *MVTmp1;
#               ifdef DEBUG_VORONOI
	            printf("Splitting here is done now\n");
#               endif
		
		
		/* Adding the TMin as the Head of the list. */

		MVTmp1 = MVPts3;
		if (TMin < MVTmp1 -> Pt[0]) {
		    MVTmp = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct));
		    MVTmp -> Pt = (CagdRType *) 
		        IritMalloc(sizeof(CagdRType) * 3); 
		    MVTmp -> Pt[0] = TMin;
		    MVTmp -> Pnext = MVPts3;
		    MVPts3 = MVTmp;
#                   ifdef DEBUG_VORONOI
		        printf("Adding to the HEAD of the list is succeeded\n");
#                   endif
		}

		/* Adding the TMax as the Tail of the list. */
		
		MVTmp1 = CagdListLast(MVPts3);
	
		if (TMax > MVTmp1 -> Pt[0]) {
		    MVTmp = (MvarPtStruct *) IritMalloc(sizeof(MvarPtStruct));
		    MVTmp -> Pt = (CagdRType *) 
		        IritMalloc(sizeof(CagdRType) * 3); 
		    MVTmp -> Pt[0] = TMax;
		    MVTmp1 -> Pnext = MVTmp;
		    MVTmp -> Pnext = NULL;
#                   ifdef DEBUG_VORONOI
		        printf("Adding to the TAIL of the list is succeeded\n");
#                   endif
		}
					
		/* Checking between points in MVPts4. */
    
		for (MVPts4 = MVPts3; MVPts4 -> Pnext != NULL; 
		     MVPts4 = MVPts4 -> Pnext) {
    
		    UMid = (MVPts4 -> Pt[0] + MVPts4 -> Pnext -> Pt[0]) / 2.0;

		    /* Compute the bisector point at the parameter. */
    
		    R = CagdCrvEval(LeftPtCrvTmp, UMid); 	   
		    CagdCoerceToE3(BP, &R, -1, LeftPtCrvTmp -> PType); 

		    /* Computing the parameter correspoing to the bisector */
		    /* point on the boundary curve using the distance      */
		    /* between the curve and a point.                      */
		    
		    /* Evaluation the point and curvature of Crv1 at UMid. */
		    R2 = CagdCrvEval(Crv1, UMid); 	   
		    CagdCoerceToE3(Pt1, &R2, -1, Crv1 -> PType); 

		    R4 = CagdCrvEval(NkCrv1, UMid); 	   
		    CagdCoerceToE3(kN1, &R4, -1, NkCrv1 -> PType); 

		    /*	Compute the dot product < P - C1, kN1>              */
		    /*  and evaluate the curvature constraint.              */
		    DPCurvature[0] = ((BP[0] - Pt1[0])*kN1[0] + 
				      (BP[1] - Pt1[1])*kN1[1]) - 1; 
     
		    if (DPCurvature[0] <= 0.0){ 	
#                       ifdef DEBUG_VORONOI	
		            printf("Curvature condition is satisfied\n");
#                       endif
			SubDivCrv2 = CagdCrvRegionFromCrv(LeftPtCrvTmp, 
						     MVPts4 -> Pt[0],
						     MVPts4 -> Pnext -> Pt[0]);
			IRIT_LIST_PUSH(SubDivCrv2, CCPtCrv);
		    } 	
		    else{ 
#                       ifdef DEBUG_VORONOI        
		        printf("Curvature condition is NOT satisfied\n");
#                       endif
		    } 
		}
	    } 
	    else {
		UMid = (TMin + TMax) / 2.0;

		/* Compute the bisector point at the parameter */
    
		R = CagdCrvEval(LeftPtCrvTmp, UMid); 	   
		CagdCoerceToE3(BP, &R, -1, LeftPtCrvTmp -> PType); 

		/* Computing the parameter correspoing to the bisector    */
		/* point on the boundary curve using the distance between */
		/* the curve and a point.                                 */
		
		/* Evaluation the point and curvature of Crv1 at UMid. */
		
		R2 = CagdCrvEval(Crv1, UMid); 	   
		CagdCoerceToE3(Pt1, &R2, -1, Crv1 -> PType); 
		
		R4 = CagdCrvEval(NkCrv1, UMid); 	   
		CagdCoerceToE3(kN1, &R4, -1, NkCrv1 -> PType); 
		
		/*  Compute the dot product < P - C1, kN1>     	 */
		/*  and evaluate the curvature constraint.       */
		DPCurvature[0] = ((BP[0] - Pt1[0])*kN1[0] + 
				  (BP[1] - Pt1[1])*kN1[1]) - 1; 
		
		if (DPCurvature[0] <= 0.0) { 
#                   ifdef DEBUG_VORONOI		
		        printf("Curvature condition is satisfied\n");
#                   endif
		    SubDivCrv1 = CagdCrvCopy(LeftPtCrvTmp);
		    
		    IRIT_LIST_PUSH(SubDivCrv1, CCPtCrv); 
		} 	
		else {   
#                   ifdef DEBUG_VORONOI      
		        printf("Curvature condition is NOT satisfied\n");
#                   endif
			
		} 
	    }
	    LeftPtCrvTmp = LeftPtCrvTmp -> Pnext;
	}
    }

    /* MEM - freeing LeftPtCrv. */
    CagdCrvFreeList(LeftPtCrv);
    CagdCrvFreeList(NkCrv1);
    CagdCrvFreeList(Crv1);

    return CCPtCrv; 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the Crv1, point Pt and the Bisector point (BP), this function says M
*   BP's footpoint is the minimum distance point to Crv1. Uses the function  M
*   SymbDistCrvPoint from symb_lib.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:  The input curve - CagdCrvStruct.                                  M
*   Pt:  The input point - CagdRType.                                        M
*   BP: Bisector point - CagdPType.                                          M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdBType: Returns TRUE or FALSE.                                        M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbDistCrvPoint                                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctCheckFootPtEqualsMinDistPt                                       M
*****************************************************************************/
CagdBType MvarBsctCheckFootPtEqualsMinDistPt(CagdCrvStruct *Crv1,
					     CagdRType *Pt,
					     CagdPType BP) 
{
    CagdPtStruct *BisPt, *FootPtParam, *FootTmp;
    CagdRType MinDistPar;

    /* Computing the parameter correspoing to the bisector    */
    /* point on the boundary curve using the distance between */
    /* the curve and a point.                                 */
    
    FootPtParam = SymbLclDistCrvPoint(Crv1, BP, MVAR_BSCT_NUMER_TOL);
    
    /* Crosschecking whether FootPtParam generates the same bisector point. */
    FootTmp = FootPtParam;
    while (FootTmp != NULL) {
        BisPt = MvarBsctComputeCrvPtBis(Crv1, Pt, FootTmp -> Pt[0]);
	if (fabs(BisPt -> Pt[0] - BP[0]) <= 10 * MVAR_BSCT_NUMER_TOL && 
	    fabs(BisPt -> Pt[1] - BP[1]) <= 10 * MVAR_BSCT_NUMER_TOL) {
#           ifdef DEBUG_VORONOI
	        printf("Bisector point is correct\n");
#           endif
	    break;
	}
	FootTmp = FootTmp -> Pnext;
    }

    MinDistPar = SymbDistCrvPoint(Crv1, BP, TRUE, MVAR_BSCT_NUMER_TOL); 

    if (FootTmp != NULL) {
        if (fabs(FootTmp -> Pt[0] - MinDistPar) <= 10 * MVAR_BSCT_NUMER_TOL) {
	    return TRUE;
	}
	else 
	    return FALSE;
    }
    else 
        return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point satisfiesM
*   the LL and Curvature constraint for a crv and a point pair and trims it  M
*   to the minimum.                                                          M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  The input curve - CagdCrvStruct.                                   M
*   Pt:  The input point - CagdRType.                                        M
*   Alpha: Parameter of the alpha-sector - CagdRType. 0.5 for bisector.      M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: Return the list of trimmed bisector curves.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctTrimCrvPt, MvarBsctCrvPtLeft, MvarBsctCrvPtCurvature             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctTrimCrvPtPair                                                    M
*****************************************************************************/
CagdCrvStruct *MvarBsctTrimCrvPtPair(CagdCrvStruct *Crv, 
				     CagdRType *Pt, 
				     CagdRType Alpha) 
{
    CagdCrvStruct *Crv1, *Crv2, *BisectCrv1, *CCPtCrv1, *CCPtCrv,
        *TrimmedBis = NULL;
    CagdPtStruct *InterPts;
    CagdRType TMin, TMax;
    
    /* Computing the bisector curve of the given pt and curve. */

    Crv2 = CagdCrvCopy(Crv);
    BisectCrv1 = SymbCrvPtBisectorCrv2D(Crv2, Pt, 0.5);
    CagdCrvDomain(BisectCrv1, &TMin, &TMax);
    /* MEM leak sealed. */
    CagdCrvFree(Crv2);
    CagdCrvFree(BisectCrv1);

    Crv1 = CagdCoerceCrvTo(Crv, CAGD_PT_E2_TYPE, FALSE);
    
    if (CAGD_IS_BEZIER_CRV(Crv1)) {
	CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }

    /* Calling the curvature constraint function. */
    
    CCPtCrv = MvarBsctCrvPtCurvature(Crv1, Pt, 0.5);

    /* Finding the True Minimum. Identify the intersection points between  */
    /* the two bisectors portions, split them there and check for minimum. */

    CCPtCrv1 = CCPtCrv;
    
    if (CCPtCrv1 != NULL && CCPtCrv1 -> Pnext != NULL) {
        if (CagdListLength(CCPtCrv1) == 2) {
	    CagdRType UMin, UMax, VMin, VMax, *R1, *R2, *R3, *R4;
	    CagdPType Pt1, Pt2, Pt3, Pt4;
	    CagdCrvStruct *CCPtCrvTmp;

	    CagdCrvDomain(CCPtCrv1, &UMin, &UMax);
	    CagdCrvDomain(CCPtCrv1 -> Pnext, &VMin, &VMax);

	    /* For first possibility. */

	    R1 = CagdCrvEval(CCPtCrv1, UMin); 	   
	    CagdCoerceToE3(Pt1, &R1, -1, CCPtCrv1 -> PType); 
	    R2 = CagdCrvEval(CCPtCrv1 -> Pnext, VMax); 	   
	    CagdCoerceToE3(Pt2, &R2, -1, CCPtCrv1 -> Pnext -> PType); 
	    
	    /* Second possibility. */

	    R3 = CagdCrvEval(CCPtCrv1, UMax); 	   
	    CagdCoerceToE3(Pt3, &R3, -1, CCPtCrv1 -> PType); 
	    R4 = CagdCrvEval(CCPtCrv1 -> Pnext, VMin); 	   
	    CagdCoerceToE3(Pt4, &R4, -1, CCPtCrv1 -> Pnext -> PType); 
	    
	    if ((IRIT_APX_EQ_EPS(Pt2[0], Pt1[0], 10 * MVAR_BSCT_NUMER_TOL)) &&
		(IRIT_APX_EQ_EPS(Pt2[1], Pt1[1], 10 * MVAR_BSCT_NUMER_TOL))) {
	        InterPts = NULL;
		UMax = UMax - 10 * MVAR_BSCT_NUMER_TOL;
		CCPtCrvTmp =
		    (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct));
		CCPtCrvTmp = CagdCrvRegionFromCrv(CCPtCrv1, UMin, UMax);
		VMin = VMin + 10 * MVAR_BSCT_NUMER_TOL;
		CCPtCrvTmp -> Pnext =
		    (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct));
		CCPtCrvTmp -> Pnext = CagdCrvRegionFromCrv(CCPtCrv1  -> Pnext,
							   VMin, VMax);
		CCPtCrvTmp -> Pnext -> Pnext = NULL;
		CagdCrvFreeList(CCPtCrv);
		CCPtCrv = CagdCrvCopyList(CCPtCrvTmp);
	    }
	    else if ((IRIT_APX_EQ_EPS(Pt4[0], Pt3[0], 10 * MVAR_BSCT_NUMER_TOL)) &&
		     (IRIT_APX_EQ_EPS(Pt4[1], Pt3[1], 10 * MVAR_BSCT_NUMER_TOL))) {
	        InterPts = NULL;
		UMin = UMin + 10 * MVAR_BSCT_NUMER_TOL;
		CCPtCrvTmp =
		    (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct));
		CCPtCrvTmp = CagdCrvRegionFromCrv(CCPtCrv1, UMin, UMax);
		VMax = VMax - 10 * MVAR_BSCT_NUMER_TOL;
		CCPtCrvTmp -> Pnext = 
		    (CagdCrvStruct *) IritMalloc(sizeof(CagdCrvStruct));
		CCPtCrvTmp -> Pnext = CagdCrvRegionFromCrv(CCPtCrv1  -> Pnext,
							   VMin, VMax);
		CCPtCrvTmp -> Pnext -> Pnext = NULL;
		CagdCrvFreeList(CCPtCrv);
		CCPtCrv = CagdCrvCopyList(CCPtCrvTmp);
	    }
	    else 
	        InterPts = CagdCrvCrvInter(CCPtCrv1, CCPtCrv1 -> Pnext, 
					   MVAR_BSCT_NUMER_TOL);
	}
	else {
	    CagdCrvStruct *FirstCrv, *LastCrv, *MiddleCrv,
		*MergeCrv = NULL;
	    CagdPType Pt1, Pt2, BP;
	    CagdRType UMin1, UMax1, VMin1,VMax1, WMin1, WMax1;
	    CagdRType *R;
	    CagdBType MinPar;

	    FirstCrv = CagdCrvCopy(CCPtCrv1);
	    LastCrv = CagdListLast(CCPtCrv1);
	    MiddleCrv = CagdCrvCopy(CCPtCrv1 -> Pnext);

	    CagdCrvDomain(FirstCrv, &UMin1, &UMax1);
	    CagdCrvDomain(LastCrv, &VMin1, &VMax1);
	    CagdCrvDomain(MiddleCrv, &WMin1, &WMax1);

	    R = CagdCrvEval(FirstCrv, UMax1); 	   
	    CagdCoerceToE3(Pt1, &R, -1, FirstCrv -> PType); 
	    
	    R = CagdCrvEval(LastCrv, VMin1); 	   
	    CagdCoerceToE3(Pt2, &R, -1, LastCrv -> PType); 

	    if (fabs(Pt1[0] - Pt2[0]) <= 10 * MVAR_BSCT_NUMER_TOL && 
		fabs(Pt1[1] - Pt2[1]) <= 10 * MVAR_BSCT_NUMER_TOL) {
#               ifdef DEBUG_VORONOI
	            printf("End point matches : Merging the two curves\n");
#               endif
		MergeCrv = CagdMergeCrvCrv(FirstCrv, LastCrv, FALSE);
		CagdCrvSetDomain(MergeCrv, 0.0, 1.0);
	    }
	    
	    InterPts = CagdCrvCrvInter(MergeCrv, MiddleCrv, MVAR_BSCT_NUMER_TOL);
	    if (InterPts != NULL) {
#               ifdef DEBUG_VORONOI
	            printf("Intn pts found for more than 2 crvs\n");
#               endif
	    }
	    
	    if (CagdListLength(InterPts) == 2) {
	        /* Splitting the MergeCrv at the intersection points.       */
	        /* If the intn pts are not in Ascending order of parametric */
	        /* wrt MergeCrvpoints.                                      */
	      
	        CagdRType UMid1, UMid2, UMid3;
		CagdCrvStruct *MergeCrvCpy, *MergeCrv1, *MergeCrv2, *MergeCrv3;
		CagdPtStruct *InterPts1, *InterPts2;

	        if (InterPts -> Pt[0] > InterPts -> Pnext -> Pt[0]) {
		    InterPts1 = CagdPtCopyList(InterPts);		    
		    InterPts2 = CagdListReverse(InterPts1);
		}
		else {
		    InterPts2 = CagdPtCopyList(InterPts);
		}
		
		MergeCrvCpy = CagdCrvCopy(MergeCrv);
		MergeCrv1 = CagdCrvRegionFromCrv(MergeCrvCpy, 0.0,
						 InterPts2 -> Pt[0]);
		UMid1 = (0.0 + InterPts2 -> Pt[0]) / 2.0;
		R = CagdCrvEval(MergeCrv1, UMid1); 	   
		CagdCoerceToE3(BP, &R, -1, MergeCrv1 -> PType); 
		MinPar = MvarBsctCheckFootPtEqualsMinDistPt(Crv1, Pt, BP);
		
		if (MinPar) {
		    IRIT_LIST_PUSH(MergeCrv1, TrimmedBis); 
		}

		MergeCrv2 = CagdCrvRegionFromCrv(MergeCrvCpy, 
						 InterPts2 -> Pt[0],
						 InterPts2 -> Pnext -> Pt[0]);
		
		UMid2 = (InterPts2 -> Pt[0] +
			 InterPts2 -> Pnext -> Pt[0]) / 2.0;
		R = CagdCrvEval(MergeCrv2, UMid2); 	   
		CagdCoerceToE3(BP, &R, -1, MergeCrv2 -> PType); 
		MinPar = MvarBsctCheckFootPtEqualsMinDistPt(Crv1, Pt, BP);
		
		if (MinPar) {
		    IRIT_LIST_PUSH(MergeCrv2, TrimmedBis); 
		}

		MergeCrv3 = CagdCrvRegionFromCrv(MergeCrvCpy,
						 InterPts2 -> Pnext -> Pt[0],
						 1.0);
		
		UMid3 = (InterPts2 -> Pnext -> Pt[0] + 1.0) / 2.0;
		R = CagdCrvEval(MergeCrv3, UMid3); 	   
		CagdCoerceToE3(BP, &R, -1, MergeCrv3 -> PType); 
		MinPar = MvarBsctCheckFootPtEqualsMinDistPt(Crv1, Pt, BP);
		
		if (MinPar) {
		    IRIT_LIST_PUSH(MergeCrv3, TrimmedBis); 
		}
		/* Raman: Mem. leak sealed. */
		CagdPtFreeList(InterPts2);

		/* Splitting the Middle curve. */
	        if (InterPts -> Pt[1] > InterPts -> Pnext -> Pt[1]) {
		    CagdPtStruct *InterPts1, *InterPts2;
		    InterPts1 = CagdPtCopyList(InterPts);		    
		    InterPts2 = CagdListReverse(InterPts1);
		}
		else {
		    InterPts2 = CagdPtCopyList(InterPts);
		}
		CagdCrvFree(MergeCrvCpy);
		MergeCrvCpy = CagdCrvCopy(MiddleCrv);
		MergeCrv1 = CagdCrvRegionFromCrv(MergeCrvCpy, WMin1,
						 InterPts2 -> Pt[1]);
		UMid1 = (WMin1 + InterPts2 -> Pt[1]) / 2.0;
		R = CagdCrvEval(MergeCrv1, UMid1); 	   
		CagdCoerceToE3(BP, &R, -1, MergeCrv1 -> PType); 
		MinPar = MvarBsctCheckFootPtEqualsMinDistPt(Crv1, Pt, BP);
		
		if (MinPar) {
		    IRIT_LIST_PUSH(MergeCrv1, TrimmedBis); 
		}

		MergeCrv2 = CagdCrvRegionFromCrv(MergeCrvCpy, 
						 InterPts2 -> Pt[1],
						 InterPts2 -> Pnext -> Pt[1]);
		
		UMid2 = (InterPts2 -> Pt[1] +
			 InterPts2 -> Pnext -> Pt[1]) / 2.0;
		R = CagdCrvEval(MergeCrv2, UMid2); 	   
		CagdCoerceToE3(BP, &R, -1, MergeCrv2 -> PType); 
		MinPar = MvarBsctCheckFootPtEqualsMinDistPt(Crv1, Pt, BP);
		
		if (MinPar) {
		    IRIT_LIST_PUSH(MergeCrv2, TrimmedBis); 
		}

		MergeCrv3 = CagdCrvRegionFromCrv(MergeCrvCpy,
						 InterPts -> Pnext -> Pt[1],
						 WMax1);
		
		UMid3 = (InterPts -> Pnext -> Pt[1] + WMax1) / 2.0;
		R = CagdCrvEval(MergeCrv3, UMid3); 	   
		CagdCoerceToE3(BP, &R, -1, MergeCrv3 -> PType); 
		MinPar = MvarBsctCheckFootPtEqualsMinDistPt(Crv1, Pt, BP);
		
		if (MinPar) {
		    IRIT_LIST_PUSH(MergeCrv3, TrimmedBis); 
		}

		/* Raman: Mem. leak sealed. */
		CagdPtFreeList(InterPts2);
		CagdCrvFree(MergeCrvCpy);
	    }
	}
    }
    else {
        InterPts = NULL;
#       ifdef DEBUG_VORONOI
        printf("Insufficient no. of segments for intersection\n");
#       endif
    }
    
    if (InterPts == NULL) {
#       ifdef DEBUG_VORONOI
        printf("No intersection point found\n");
#       endif

	/* Iddo: The lines below (resetting the domain) cause problems to  */
	/* the LE and are therefore removed.                               */
	/* Raman : I am resetting the parameters by Epsilon on either side */
	/* so that the radial envelope code works well without inf. loop.  */
	
	if (CagdListLength(CCPtCrv) == 1) {
	    CagdRType UMin, UMax;

	    /* Resetting the domain by Epsilon. */
	    CagdCrvDomain(CCPtCrv, &UMin, &UMax);
	    UMin = UMin + 10 * MVAR_BSCT_NUMER_TOL;
	    UMax = UMax - 10 * MVAR_BSCT_NUMER_TOL;
	    CCPtCrv1 = CagdCrvRegionFromCrv(CCPtCrv, UMin, UMax);
	    TrimmedBis = CagdCrvCopy(CCPtCrv1);
	    /* CagdCrvFree(CCPtCrv1); */
	}
	else
	    /* MEM Leak fixed. */
	    TrimmedBis = CagdCrvCopyList(CCPtCrv);
    }
    else {
#       ifdef DEBUG_VORONOI
        printf("Intersection is found\n");
#       endif
        
    }
    /* MEM Leak fixed. */
    CagdCrvFree(Crv1);
    CagdCrvFree(CCPtCrv);
    return TrimmedBis;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function returns the value of the cross product between two vectors.M
*   First vector is between the point Pt and the midpoint of Crv and the     M
*   second one is between the point and the LeftNormal.                      M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:         The input curve - CagdCrvStruct.                            M
*   Pt:          The discontinuity point on Crv.                             M
*   LeftNormal:  Left normal at the point Pt.                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType: Return the value of the expression.                           M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctTrimCrvPt                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctIsCrvLeftToLine                                                  M
*****************************************************************************/
CagdRType MvarBsctIsCrvLeftToLine(CagdCrvStruct *Crv, 
				  CagdRType *Pt,
				  CagdPType LeftNormal) 
{
    CagdRType *R, UMin, UMax, UMid, X1, Y1, X2, Y2, X, Y, Nt, Dt, t, Qx, Qy,
        V1x, V1y, V2x, V2y, Left;
    CagdPType Pt1;
    
    /* Line is converted to the end points (x1, y1) and (x2, y2) using the */
    /* given point Pt.                                                     */
    
    X1 = Pt[0];
    Y1 = Pt[1];
    X2 = X1 - (-LeftNormal[0]);
    Y2 = Y1 - (-LeftNormal[1]);

    CagdCrvDomain(Crv, &UMin, &UMax);
    UMid = (UMin + UMax) / 2.0;

    R = CagdCrvEval(Crv, UMid); 	   
    CagdCoerceToE3(Pt1, &R, -1, Crv -> PType);

    /* The point is denoted as (x,y). The line is from (x1, y1) to (x2, y2). */

    X = Pt1[0];
    Y = Pt1[1];

    /* Find the parameter t on the line that satisfies the dot product      */
    /* <(x,y) - (x(t),y(t)), (x'(t), y'(t))>                                */
 
    Nt = (X - X1) * (X2 - X1) + (Y - Y1) * (Y2 - Y1);
    Dt = (X2 - X1) * (X2 - X1) + (Y2 - Y1) * (Y2 - Y1);
    t = Nt / Dt;

    /* The intersection point (qx, qy) using t is calculated as follows.    */

    Qx = X1 + t * (X2 - X1);
    Qy = Y1 + t * (Y2 - Y1);

    /* Vector V1 and V2 are identified as follows. */
    
    V1x = LeftNormal[0];
    V1y = LeftNormal[1];
    V2x = X - X1;
    V2y = Y - Y1;
    Left = V1x * V2y - V2x * V1y;

#   ifdef DEBUG_VORONOI
        printf("Value of Left = %f\n", Left);
#   endif

    return Left;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function returns the radial lower envelope if the input is a (list  M
*   of) crv (s) and a point or returns the trimmed bisector if the input is  M
*   a pair of crv and a point.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:  The input curve - CagdCrvStruct.                                   M
*   Pt:  The input point - CagdRType.                                        M
*   Alpha: Parameter of the alpha-sector - CagdRType. 0.5 for bisector.      M
*   BaseCrv: The Crv for which the trimming is sought along its discontinuityM
*   point.                                                                   M 
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdCrvStruct *: Return the list of trimmed bisector curves.             M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctTrimCrvPtPair, MvarBsctCrvPtLeft, MvarBsctCrvPtCurvature,        M
*   SymbCrvsLowerEnvelop                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctTrimCrvPt, bisectors                                             M
*****************************************************************************/
CagdCrvStruct *MvarBsctTrimCrvPt(CagdCrvStruct *Crv, 
				 CagdRType *Pt, 
				 CagdRType Alpha,
				 CagdCrvStruct *BaseCrv) 
{
    CagdCrvStruct *Crv1, *Crv2, *TrimmedBis1, *RadialLE, *SubDiv1, *SubDiv2,
        *TrimmedBis = NULL;
    CagdRType *R, TMin, TMax, MinDistPar, Left1, Left2;
    CagdPType Pt1, Nrml1, Nrml2, LeftNormal1, LeftNormal2;
    CagdVecStruct *Vec;
    IrtLnType Line1, Line2;
    CagdPtStruct *Inter1, *Inter2, *InterTmp1, *InterTmp2;

    Crv2 = Crv;
    while (Crv2 != NULL) {
       	CagdBType
	    IsRational = MVAR_IS_RATIONAL_PT(Crv2 -> PType);
       
        Crv1 = CagdCoerceCrvTo(Crv2, IsRational ? CAGD_PT_P2_TYPE
			                        : CAGD_PT_E2_TYPE,
			       FALSE);
	
	if (CAGD_IS_BEZIER_CRV(Crv1)) {
	    CagdCrvStruct
	        *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	    CagdCrvFree(Crv1);
	    Crv1 = TCrv;
	}
	TrimmedBis1 = MvarBsctTrimCrvPtPair(Crv1, Pt, 0.5);

        /* MEM leak sealed. */
        CagdCrvFree(Crv1);

        /* MEM prob. here. */
        
        {
	    CagdCrvStruct 
	        *TrimmedBisCpy = TrimmedBis1;

	    while (TrimmedBisCpy != NULL) {
	        CagdCrvStruct *TrimmedBisCpy1 = CagdCrvCopy(TrimmedBisCpy);
		IRIT_LIST_PUSH(TrimmedBisCpy1, TrimmedBis);
		TrimmedBisCpy = TrimmedBisCpy -> Pnext;
	    }
            CagdCrvFreeList(TrimmedBis1);
	}
       
	Crv2 = Crv2 -> Pnext;
    }
    
    CagdCrvDomain(BaseCrv, &TMin, &TMax);
    MinDistPar = SymbDistCrvPoint(BaseCrv, Pt, TRUE, MVAR_BSCT_NUMER_TOL);
    R = CagdCrvEval(BaseCrv, MinDistPar); 	   
    CagdCoerceToE3(Pt1, &R, -1, BaseCrv -> PType); 
    
    if (fabs(Pt[0] - Pt1[0]) <= 10 * MVAR_BSCT_NUMER_TOL && 
	fabs(Pt[1] - Pt1[1]) <= 10 * MVAR_BSCT_NUMER_TOL) {
#       ifdef DEBUG_VORONOI
            printf("Correct discontinuity point\n");
#       endif
	if ((fabs(TMin - MinDistPar) <= 10 * MVAR_BSCT_NUMER_TOL) ||
	    (fabs(TMax - MinDistPar) <= 10 * MVAR_BSCT_NUMER_TOL)) {
	    SubDiv1 = CagdCrvRegionFromCrv(BaseCrv, TMin, (TMin + TMax) / 2.0);
	    Vec = CagdCrvNormalXY(SubDiv1, TMin, TRUE); 
	    IRIT_PT_COPY(Nrml1, Vec -> Vec); 
	    SubDiv2 = CagdCrvRegionFromCrv(BaseCrv, (TMin + TMax) / 2.0, TMax);
	    Vec = CagdCrvNormalXY(SubDiv2, TMax, TRUE);   
	    IRIT_PT_COPY(Nrml2, Vec -> Vec);
	}
	else {
	    SubDiv1 = CagdCrvRegionFromCrv(BaseCrv, TMin, MinDistPar);
	    Vec = CagdCrvNormalXY(SubDiv1, MinDistPar, TRUE); 
	    IRIT_PT_COPY(Nrml1, Vec -> Vec); 
	    SubDiv2 = CagdCrvRegionFromCrv(BaseCrv, MinDistPar, TMax);
	    Vec = CagdCrvNormalXY(SubDiv2, MinDistPar, TRUE);   
	    IRIT_PT_COPY(Nrml2, Vec -> Vec);
	}
	/* MEM leak sealed. */
        CagdCrvFree(SubDiv1);
        CagdCrvFree(SubDiv2);

	/* Normally the line coefficients will be as follows. */
	/* Line1[0] = Nrml1[1];                               */
	/* Line1[1] = -Nrml1[0];                              */
	/* Line1[2] = -(Line1[0] * Pt[0] + Line1[1] * Pt[1]); */
	/* Since CagdCrvNormalXY gives the right normal, line */
	/* coefficients should be the following .             */
	LeftNormal1[0] = -Nrml1[0];
	LeftNormal1[1] = -Nrml1[1];
	LeftNormal1[2] = 0.0;
	Line1[0] = LeftNormal1[1];
	Line1[1] = -LeftNormal1[0];
	Line1[2] = (-Line1[0] * Pt[0] - Line1[1] * Pt[1]);
	LeftNormal2[0] = -Nrml2[0];
	LeftNormal2[1] = -Nrml2[1];
	LeftNormal2[2] = 0.0;
	Line2[0] = LeftNormal2[1];
	Line2[1] = -LeftNormal2[0];
	Line2[2] = (-Line2[0] * Pt[0] - Line2[1] * Pt[1]);

	if (CagdListLength(TrimmedBis) > 1) {
	    CagdCrvStruct *TrimmedBisCpy, *TmpCrv,
	        *TrimmedRLE = NULL; 
	    CagdRType UMin, UMax;

	    if (CagdListLength(TrimmedBis) == 2) {
	        CagdRType UMin, UMax, VMin, VMax, *R1, *R2, *R3, *R4;
		CagdPType Pt1, Pt2, Pt3, Pt4;

		/* Since the list is stored in reverse order. */
		CagdCrvDomain(TrimmedBis, &UMin, &UMax);
		CagdCrvDomain(TrimmedBis -> Pnext, &VMin, &VMax);

		R1 = CagdCrvEval(TrimmedBis, UMax); 	   
		CagdCoerceToE3(Pt1, &R1, -1, TrimmedBis -> PType); 
		R2 = CagdCrvEval(TrimmedBis -> Pnext, VMin); 	   
		CagdCoerceToE3(Pt2, &R2, -1, TrimmedBis -> Pnext -> PType); 
	    
		/* Second possibility. */

		R3 = CagdCrvEval(TrimmedBis, UMin); 	   
		CagdCoerceToE3(Pt3, &R3, -1, TrimmedBis -> PType); 
		R4 = CagdCrvEval(TrimmedBis -> Pnext, VMax); 	   
		CagdCoerceToE3(Pt4, &R4, -1, TrimmedBis -> Pnext -> PType);
		
		if ((IRIT_APX_EQ_EPS(Pt2[0], Pt1[0], 10 * MVAR_BSCT_NUMER_TOL)) &&
		    (IRIT_APX_EQ_EPS(Pt2[1], Pt1[1], 10 * MVAR_BSCT_NUMER_TOL))) {
		    RadialLE = TrimmedBis;
		}
		else if ((IRIT_APX_EQ_EPS(Pt4[0], Pt3[0],
				     10 * MVAR_BSCT_NUMER_TOL)) && 
			 (IRIT_APX_EQ_EPS(Pt4[1], Pt3[1],
				     10 * MVAR_BSCT_NUMER_TOL))) {
		    RadialLE = TrimmedBis;
		}
		else 
		    RadialLE = SymbCrvsLowerEnvelop(TrimmedBis, Pt, 
						    MVAR_BSCT_NUMER_TOL);
	    }
	    else 
	        RadialLE = SymbCrvsLowerEnvelop(TrimmedBis, Pt, 
						MVAR_BSCT_NUMER_TOL);
	    
	    TrimmedBisCpy = CagdCrvCopyList(RadialLE);
	    while (TrimmedBisCpy != NULL) {
	        CagdCrvStruct *DivCrv1, *DivCrv2;
		CagdRType InterPar;
		/* Check whether there is intn between the line1 and curve. */
	        TmpCrv = CagdCrvCopy(TrimmedBisCpy);
		CagdCrvDomain(TmpCrv, &UMin, &UMax);
		
		/* First Try with ONLY intersection points. */
		Inter1 = SymbLclDistCrvLine(TmpCrv, Line1, 
					    MVAR_BSCT_NUMER_TOL, TRUE, FALSE);
		Inter2 = SymbLclDistCrvLine(TmpCrv, Line2, 
					    MVAR_BSCT_NUMER_TOL, TRUE, FALSE);
		if (Inter1 != NULL) {
		    /* Check whether the intn. parameters are within */
		    /* curve limits.                                 */
		    InterTmp1 = MvarGetMiniumIntnPar(TmpCrv, Pt, 
						     Inter1, LeftNormal1);
		    if (InterTmp1 != NULL) {
		        if (!(InterTmp1 -> Pt[0] >= UMin && 
			      InterTmp1 -> Pt[0] <= UMax)) {
		            CagdPtFreeList(Inter1);
			    InterPar = SymbDistCrvLine(TmpCrv, Line1, TRUE, 
						       MVAR_BSCT_NUMER_TOL);
			    InterTmp1 = CagdPtNew();
			    InterTmp1 -> Pt[0] = InterPar;
			}
		    }
		}
		else {
		    InterPar = SymbDistCrvLine(TmpCrv, Line1, TRUE, 
					       MVAR_BSCT_NUMER_TOL);
		    InterTmp1 = CagdPtNew();
		    InterTmp1 -> Pt[0] = InterPar;
		}

		if (Inter2 != NULL) {
		    InterTmp2 = MvarGetMiniumIntnPar(TmpCrv, Pt, 
						     Inter2, LeftNormal2);
		    if (InterTmp2 != NULL) {
		        if (!(InterTmp2 -> Pt[0] >= UMin && 
			      InterTmp2 -> Pt[0] <= UMax)) {
			    CagdPtFreeList(Inter2);
			    InterPar = SymbDistCrvLine(TmpCrv, Line2, TRUE, 
						       MVAR_BSCT_NUMER_TOL);
			    InterTmp2 = CagdPtNew();
			    InterTmp2 -> Pt[0] = InterPar;
			}
		    }
		}
		else {
		    InterPar = SymbDistCrvLine(TmpCrv, Line2, TRUE, 
					       MVAR_BSCT_NUMER_TOL);
		    InterTmp2 = CagdPtNew();
		    InterTmp2 -> Pt[0] = InterPar;
		}
		
		if ((InterTmp1 != NULL && 
		     (InterTmp1 -> Pt[0] <= UMin || 
		      InterTmp1 -> Pt[0] >= UMax)) &&
		     (InterTmp2 != NULL &&
		     (InterTmp2 -> Pt[0] <= UMin || 
		      InterTmp2 -> Pt[0] >= UMax))) {
#                   ifdef DEBUG_VORONOI
		        printf("No intn of RLE with the line\n");
#                   endif 
		    Left1 = MvarBsctIsCrvLeftToLine(TmpCrv, Pt1, LeftNormal1);
		    Left2 = MvarBsctIsCrvLeftToLine(TmpCrv, Pt1, LeftNormal2);
		    if ((Left1 > 0 && Left2 < 0)) {
		        IRIT_LIST_PUSH(TmpCrv, TrimmedRLE);
		    }
		}
		else if ((InterTmp1 != NULL &&
			  InterTmp1 -> Pt[0] > UMin && 
			  InterTmp1 -> Pt[0] < UMax) &&
			 (InterTmp2 != NULL &&
			  InterTmp2 -> Pt[0] > UMin && 
			  InterTmp2 -> Pt[0] < UMax)) {
#                   ifdef DEBUG_VORONOI
		        printf("Intn of RLE with both lines\n");
#                   endif 
		    DivCrv1 = CagdCrvRegionFromCrv(TmpCrv, InterTmp1 -> Pt[0], 
						   InterTmp2 -> Pt[0]);
		    IRIT_LIST_PUSH(DivCrv1, TrimmedRLE);
		}
		else if (InterTmp1 != NULL &&
			 InterTmp1 -> Pt[0] > UMin && 
			 InterTmp1 -> Pt[0] < UMax) {
#                   ifdef DEBUG_VORONOI
		        printf("Intn exists bet. RLE and the line1\n");
#                   endif 
		    DivCrv1 = CagdCrvRegionFromCrv(TmpCrv, UMin, 
						   InterTmp1 -> Pt[0]);
		    DivCrv2 = CagdCrvRegionFromCrv(TmpCrv, 
						   InterTmp1 -> Pt[0], UMax);
		    Left1 = MvarBsctIsCrvLeftToLine(DivCrv1, Pt1, LeftNormal1);
		    Left2 = MvarBsctIsCrvLeftToLine(DivCrv1, Pt1, LeftNormal2);
		    if ((Left1 > 0 && Left2 < 0)) {
		        IRIT_LIST_PUSH(DivCrv1, TrimmedRLE);
		    }
		    Left1 = MvarBsctIsCrvLeftToLine(DivCrv2, Pt1, LeftNormal1);
		    Left2 = MvarBsctIsCrvLeftToLine(DivCrv2, Pt1, LeftNormal2);
		    if ((Left1 > 0 && Left2 < 0)) {
		        IRIT_LIST_PUSH(DivCrv2, TrimmedRLE);
		    }
		}
		else if (InterTmp2 != NULL &&
			 InterTmp2 -> Pt[0] > UMin && 
			 InterTmp2 -> Pt[0] < UMax) {
#                   ifdef DEBUG_VORONOI
		        printf("Intn exists bet. RLE and the line2\n");
#                   endif 
		    DivCrv1 = CagdCrvRegionFromCrv(TmpCrv, UMin, 
						   InterTmp2 -> Pt[0]);
		    DivCrv2 = CagdCrvRegionFromCrv(TmpCrv, 
						   InterTmp2 -> Pt[0], UMax);
		    Left1 = MvarBsctIsCrvLeftToLine(DivCrv1, Pt1, LeftNormal1);
		    Left2 = MvarBsctIsCrvLeftToLine(DivCrv1, Pt1, LeftNormal2);
		    if ((Left1 > 0 && Left2 < 0)) {
		        IRIT_LIST_PUSH(DivCrv1, TrimmedRLE);
		    }
		    Left1 = MvarBsctIsCrvLeftToLine(DivCrv2, Pt1, LeftNormal1);
		    Left2 = MvarBsctIsCrvLeftToLine(DivCrv2, Pt1, LeftNormal2);
		    if ((Left1 > 0 && Left2 < 0)) {
		        IRIT_LIST_PUSH(DivCrv2, TrimmedRLE);
		    }
		}
		else 
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

		/* Memory freeing. */
		if (Inter1 != NULL)
		    CagdPtFreeList(Inter1);
		if (Inter2 != NULL)
		    CagdPtFreeList(Inter2);
		
		TrimmedBisCpy = TrimmedBisCpy -> Pnext;
	    }
	    return TrimmedRLE;
	}
	else {
	    /* Trimming of the radial lower envelope. */
	    CagdRType UMin, UMax;
	    	    
	    /* Givne the Line is certainly a problem. It is not a vector  */
	    /* and hence finds more than one intersection point with the  */
	    /* TrimmedBis. Sometimes, the routine SymbDistCrvLine returns */
	    /* the wrong parameter (in the sense that it is opposite to   */
	    /* the left normal as the minimum. To counter that, identify  */
	    /* all the interpos using SymbLclDistCrvLine and identify the */
	    /* interpos that is having the same direction as the lefthand */
	    /* which can be identified using the dotproduct of the two.   */
	    
	    CagdCrvDomain(TrimmedBis, &UMin, &UMax);
	   	    	    
	    Inter1 = SymbLclDistCrvLine(TrimmedBis, Line1, 
					MVAR_BSCT_NUMER_TOL, TRUE, TRUE);
	    Inter2 = SymbLclDistCrvLine(TrimmedBis, Line2, 
					MVAR_BSCT_NUMER_TOL, TRUE, TRUE);
	    
	    
	    InterTmp1 = MvarGetMiniumIntnPar(TrimmedBis, Pt, 
					     Inter1, LeftNormal1);
	    InterTmp2 = MvarGetMiniumIntnPar(TrimmedBis, Pt, 
					     Inter2, LeftNormal2);
	    if (InterTmp1 != NULL && InterTmp2 != NULL) {
	        TrimmedBis1 = CagdCrvRegionFromCrv(TrimmedBis, 
						   InterTmp1 -> Pt[0], 
						   InterTmp2 -> Pt[0]);
	    	        		
		/* Left test is required here as well. */
		
		Left1 = MvarBsctIsCrvLeftToLine(TrimmedBis1, Pt1, LeftNormal1);
		Left2 = MvarBsctIsCrvLeftToLine(TrimmedBis1, Pt1, LeftNormal2);
		if ((Left1 > 0 && Left2 < 0)) {
		    return TrimmedBis1;
		}
		else 
		    return NULL;
	    }
	    else 
	        return NULL;
	}
    }
    else {
#       ifdef DEBUG_VORONOI
            printf("NOT the Correct discontinuity point\n");
#       endif
	return TrimmedBis;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   This function returns the minimum intersection parameter of the curve    M
*   with the Line after confirming that the vector between the discontinuity M
*   point and the intersection parameter is in the same direction to the     M
*   Left hand normal.                                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   TrimmedBis:  The input curve in CagdCrvStruct.                           M
*   Pt:          The input point - CagdRType.                                M
*   Inter:       List of intersection parameters obtained using              M
8		 SymbLclDistCrvLine.					     M
*   LeftNormal:  Left Normal of TrimmedBis at the point Pt.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdPtStruct *: Return the TrimmedBis parameter.                         M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbLclDistCrvLine                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarGetMiniumIntnPar                                                     M
*****************************************************************************/
static CagdPtStruct *MvarGetMiniumIntnPar(CagdCrvStruct *TrimmedBis, 
					  CagdRType *Pt,
					  CagdPtStruct *Inter,
					  CagdPType LeftNormal)
{
    CagdPtStruct 
        *InterTmp1 = NULL,
        *InterTmp = Inter;

    if (InterTmp == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
    else {
        while (InterTmp != NULL) {
	    CagdRType *R, DotProd, CrossProd;
	    CagdPType InterPt, Vector;
	    int count = 0;

	    R = CagdCrvEval(TrimmedBis, InterTmp -> Pt[0]);
	    CagdCoerceToE2(InterPt, &R, - 1, TrimmedBis -> PType);
	    Vector[0] = InterPt[0] - Pt[0];
	    Vector[1] = InterPt[1] - Pt[1];
	    DotProd = IRIT_DOT_PROD_2D(LeftNormal, Vector);
	    if (DotProd > 0.0) {
	        CrossProd = IRIT_CROSS_PROD_2D(LeftNormal, Vector);
		if (IRIT_FABS(CrossProd) < 10 * 
					     IRIT_FABS(MVAR_BSCT_NUMER_TOL)) {
		    InterTmp1 = InterTmp;
		    count++;
		}
		if (count > 1) {
#                   ifdef DEBUG_VORONOI
		        printf("No. of Intersection points along the normal direction of the line is more than one\n"); 
#                   endif
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
		}
	    }
	    InterTmp = InterTmp -> Pnext;
	}
    }

    return InterTmp1;
}
