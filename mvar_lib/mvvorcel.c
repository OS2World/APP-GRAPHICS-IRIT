/******************************************************************************
* mvvorcel.c - computation of the Voronoi cell of list of  curves.            *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Hanniel and Ramanathan Muthuganapathy, March 2005.   	      *
******************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "geom_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "mvar_loc.h"  

#define BBOX_MAX_X  10000.0
#define BBOX_MAX_Y  10000.0
#define DisplayFactor 100

static IPObjectStruct *DisplayVoronoiCell(MvarVoronoiCrvStruct  *LowerEnvelope,
					  CagdCrvStruct *PtCrvBisector);

/*****************************************************************************
* DESCRIPTION:                                                               *
* Trims the bisector between two curves.                                     *
* This is done by:                                                           *
*  1. Find R-LowerEnvelope - Reverses curves (swaps t and r) and perform LE. *
*  2. Reverse the results and compute T-LowerEnvelope on the result.         *
*  3. The result is the trimmed bisector (ordered according to the T param). *
*                                                                            *
* PARAMETERS:                                                                *
*   LegitCrvs:   TR-monotone Voronoi curves (with corner interpolation) that *
*                represent the untrimmed bisector between two curves (after  *
*		 LL has been applied). LegitCrvs can be freed after          *
*		 finishing the function call.				     *
*   TrimmedBsctCrvs: N.S.F.I.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void TrimBsctrsInTAndR(MvarVoronoiCrvStruct *LegitCrvs, 
			      MvarVoronoiCrvStruct **TrimmedBsctCrvs)
{
    MvarVoronoiCrvStruct *LowerEnvelopeR = NULL, *VorCrvsIter = NULL,
        *AuxCrv = NULL, *ReversedCrvs = NULL;

    /* We need to reverse and compute the R LowerEnvelope first so in the    */
    /* end we will have the curve segments sorted according to T.            */
    VorCrvsIter = LegitCrvs;
    while (VorCrvsIter) {
        AuxCrv = MvarVoronoiCrvReverse(VorCrvsIter);
        IRIT_LIST_PUSH(AuxCrv,ReversedCrvs);       
        VorCrvsIter = VorCrvsIter -> Pnext;
    }

    MvarBsctComputeLowerEnvelope(ReversedCrvs, &LowerEnvelopeR);
    MvarVoronoiCrvFreeList(ReversedCrvs);
    ReversedCrvs = NULL;

    /* Reversing the result of the R Lower Envelope. */
    VorCrvsIter = LowerEnvelopeR;
    while (VorCrvsIter) {
        AuxCrv = MvarVoronoiCrvReverse(VorCrvsIter);
        IRIT_LIST_PUSH(AuxCrv, ReversedCrvs);       
        VorCrvsIter = VorCrvsIter -> Pnext;
    }
   
    MvarBsctComputeLowerEnvelope(ReversedCrvs, TrimmedBsctCrvs);

    /* Memory freeing. */
    MvarVoronoiCrvFreeList(ReversedCrvs);
    MvarVoronoiCrvFreeList(LowerEnvelopeR);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Check for C1 discontinuity in a curve Crv.                               *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   Curve for checking discontinuity and splitting.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdCrvStruct *:  List of curves after splitting.                        *
*****************************************************************************/
static CagdCrvStruct *SplitCrvAtDiscontinuity(CagdCrvStruct *Crv)
{
    /* We currently implement C1 discontinuity only at endpoints. */
    CagdCrvStruct 
        *CrvCpy = NULL;
    CagdRType t;
    CagdBType 
        IsC1Discont = FALSE;
    if (CAGD_IS_BSPLINE_CRV(Crv))
        IsC1Discont = BspCrvKnotC1Discont(Crv, &t);

    if (IsC1Discont)
        MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);

    CrvCpy = CagdCrvCopy(Crv);
    CrvCpy -> Pnext = NULL;
    return CrvCpy;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the Voronoi cell of the first curve in the given list of        M
*   curves. For the details of the algorithm, see the following paper.       M
*                                                                            *
*   Precise Voronoi Cell Extraction of Free-form Rational Planar Closed      M
*   Curves.                                      			     M
*   Iddo Hanniel, Ramanathan Muthuganapathy, Gershon Elber, Myung-Soo Kim    M
*   ACM Symposium on Solid and Physical Modeling, 2005.          	     M
*   The following are the files used for computing the Voronoi cell          M
*   mvvorcel.c - Main function that call other function. It takes the input  M
*     curves, process them for discontinuities and calls other functions.    M
*     Displaying the output is also done in this function.                   M
*   mvsplmon.c - The function that creates monotone segments.                M
*   mvbiscon.c - The monotone segments obtained from mvsplmon.c are then     M
*     subjected to orientation and curvature constraint functions written in M
*     this file.                                                             M
*   mvtrmbis.c - Auxillary functions required for trimming are written here. M
*   mvtrmpcr.c - This file does the trimming of point/crv bisector.          M
*   mvlowenv.c - Functions in this file compute the lower envelope.          M
*   mvvorcrv.c - Operations on the MvarVoronoiCrvStruct are available in     M
*     this file.                                                             M
* Dependency of each of the above file is depicted in the following diagram: M
*                           vorcel, vorcrv                                   V
*                          \               /                                 V
*                           --------------                                   V
*                             1  2  3  4                                     V
*                        1    2       3   4                                  V
*                   1      2            3     4                              V
*              1        2                 3       4                          V
*          splmon     biscon           trmpcr   lowenv                       V
*         \                                           /                      V
*          -------------------------------------------                       V
*                                |                                           V
*                                |                                           V 
*                                |                                           V 
*                               \ /                                          V
*                             trmbis                                         V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   List of curves to compute the Voronoi cell.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPObjectStruct *:  Returns the Voronoi cell.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarComputeVoronoiCell                                                   M
*****************************************************************************/
IPObjectStruct *MvarComputeVoronoiCell(CagdCrvStruct *Crv)
{
    CagdRType UMin, UMax, VMin, VMax, TPar2;       
    CagdBType IsCiCrvDisc;
    CagdCrvStruct *Crv1, *Crv2, *CrvTemp1, *CrvTemp2, *CrvTmp0, *CrvTmpi, 
        *Crv1Coerced, *Crv2Coerced, *SmoothCrvIterNext, *SmoothCrvs,
        *SmoothCrvIter,
        *PtCrvBisector = NULL;
    CagdSrfStruct *F3, *L1, *L2, *CC1, *CC2, *OutLst;
    MvarVoronoiCrvStruct *MonotoneCrvs, *LegitCrvs, *LegitCrvIter, *AuxCrv,
        *InputCurves = NULL,
        *LowerEnvelope = NULL,
        *TrimmedBsct = NULL;
    IPObjectStruct *PPolyObj;
    
    CrvTemp1 = Crv;
    Crv1 = CagdCoerceCrvTo(CrvTemp1, CAGD_PT_E2_TYPE, FALSE);
    if (CAGD_IS_BEZIER_CRV(Crv1)) {
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }
    SmoothCrvs = SplitCrvAtDiscontinuity(Crv1);
    SmoothCrvIter = SmoothCrvs;
    /* Currently we just support discontinuity at endpoints.*/
    if (CagdListLength(SmoothCrvs) != 1)
        MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);

    /* Check if only C0 is disconstinous. If yes, check for non-disc Ci's.   */
    /* If there is one, then the foll. process is not carried. A fatal error */
    /* will be returned. Currently, discontinuity only at the end points is  */
    /* supported.							     */

    /* Handling C1-discontinous C0 points starts here.                       */
    SmoothCrvIterNext = SmoothCrvIter -> Pnext;
    if (SmoothCrvIterNext == NULL) {
        /* Cyclic iteration for closed curves. */
        if (CagdIsClosedCrv(Crv1))
	    SmoothCrvIterNext = SmoothCrvs;
    }

    /* Constructing Pt-Cv bisectors. */
    if (SmoothCrvIterNext != NULL) {
        CagdRType *R, CrossProd;
	CagdVecStruct *Tan1End, *Tan2Start;
	CagdPType Pt1, Pt2;
	/* Compare last point of SmoothCrvIter to first point of             */
	/* SmoothCrvIterNext.				                     */
	CagdCrvDomain(SmoothCrvIter, &UMin, &UMax);
	CagdCrvDomain(SmoothCrvIterNext, &VMin, &VMax);
			
	R = CagdCrvEval(SmoothCrvIter, UMax); 	   
	CagdCoerceToE3(Pt1, &R, -1, SmoothCrvIter -> PType); 
	R = CagdCrvEval(SmoothCrvIterNext, VMin); 	   
	CagdCoerceToE3(Pt2, &R, -1, SmoothCrvIterNext -> PType); 
        
	/* Sanity checks.*/
	if (!(IRIT_APX_EQ_EPS(Pt2[0], Pt1[0], 10 * MVAR_BSCT_NUMER_TOL) 
	      && 
	      IRIT_APX_EQ_EPS(Pt2[1], Pt1[1], 10 * MVAR_BSCT_NUMER_TOL)))
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
	/* Copy is needed since it returns a static.*/
	Tan1End = CagdVecCopy(CagdCrvTangent(SmoothCrvIter,
					     UMax,TRUE)); 
			
	Tan2Start = CagdCrvTangent(SmoothCrvIterNext,VMin, TRUE);
	/* Sanity checks.*/
	if (!(IRIT_APX_EQ_EPS(Tan1End -> Vec[2], 0.0, 
			 10 * MVAR_BSCT_NUMER_TOL)))
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
	if (!(IRIT_APX_EQ_EPS(Tan2Start -> Vec[2], 0.0, 
			 10 * MVAR_BSCT_NUMER_TOL)))
	    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

	CrossProd = IRIT_CROSS_PROD_2D(Tan1End -> Vec, Tan2Start -> Vec);
	CagdVecFree(Tan1End);
	/* We are interested only in points that are outer cusps. Assuming   */
	/* left orientation, these are those where Tan2Start is to the right */
	/* of Tan1End.							     */
	if (CrossProd < -10 * IRIT_FABS(MVAR_BSCT_NUMER_TOL)) {
	    /* Compute the trimmed Pt-Crv bisector, construct the Pt-Crv     */
	    /* VorCrvs and insert them into InputCrvs.			     */
	    
	    /* Check whether other geometries are C1 disc. If yes, fatal     */
	    /* error is returned as they are not supported for now.          */
	    CrvTemp2 = CrvTemp1 -> Pnext;
	    while (CrvTemp2 != NULL) {
	        IsCiCrvDisc = FALSE;
		Crv2 = CagdCoerceCrvTo(CrvTemp2, CAGD_PT_E2_TYPE, FALSE);
		if (CAGD_IS_BEZIER_CRV(Crv2)) {
		    CagdCrvStruct
		        *TCrv = CagdCnvrtBzr2BspCrv(Crv2);

		    CagdCrvFree(Crv2);
		    Crv2 = TCrv;
		}
		IsCiCrvDisc = BspCrvKnotC1Discont(Crv2, &TPar2);
		
		if (IsCiCrvDisc) 
		    MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
		/* Check for end point discontinuity. */
		else {
		    CagdCrvStruct *SmoothCrvIterNext1, *SmoothCrvs1,
		        *SmoothCrvIter1;
		    SmoothCrvs1 = SplitCrvAtDiscontinuity(Crv2);
		    SmoothCrvIter1 = SmoothCrvs1;
		    if (CagdListLength(SmoothCrvs1) != 1)
		        MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT); 

		    SmoothCrvIterNext1 = SmoothCrvIter1 -> Pnext;
		    if (SmoothCrvIterNext1== NULL) {
		        if (CagdIsClosedCrv(Crv2))
			    SmoothCrvIterNext1 = SmoothCrvs1; 
		    }
		    if (SmoothCrvIterNext1 != NULL) {
		        CagdRType *R1, CrossProd1, UMin1, UMax1, VMin1, VMax1;
			CagdVecStruct *Tan1End1, *Tan2Start1;
			CagdPType Pt11, Pt21;
			/* Compare last point of SmoothCrvIter1 to first */
			/* point of SmoothCrvIterNext1.                  */
			CagdCrvDomain(SmoothCrvIter1, &UMin1, &UMax1);
			CagdCrvDomain(SmoothCrvIterNext1, &VMin1, &VMax1);
			
			R1 = CagdCrvEval(SmoothCrvIter1, UMax1); 	   
			CagdCoerceToE3(Pt11, &R1, -1, SmoothCrvIter1 -> PType);
			R1 = CagdCrvEval(SmoothCrvIterNext1, VMin1); 	   
			CagdCoerceToE3(Pt21, &R1, -1,
				       SmoothCrvIterNext1 -> PType); 
        
			/* Sanity checks.*/
			if (!(IRIT_APX_EQ_EPS(Pt21[0], Pt11[0],
					 10 * MVAR_BSCT_NUMER_TOL) && 
			      IRIT_APX_EQ_EPS(Pt21[1], Pt11[1],
					 10 * MVAR_BSCT_NUMER_TOL)))
			    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

			/* Copy is needed since it returns a static. */
			Tan1End1 = CagdVecCopy(CagdCrvTangent(SmoothCrvIter1,
							      UMax1, TRUE)); 
			
			Tan2Start1 = CagdCrvTangent(SmoothCrvIterNext1,
						    VMin1, TRUE);
			/* Sanity checks. */
			if (!(IRIT_APX_EQ_EPS(Tan1End1 -> Vec[2], 0.0, 
					 10 * MVAR_BSCT_NUMER_TOL)))
			    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
			if (!(IRIT_APX_EQ_EPS(Tan2Start1 -> Vec[2], 0.0, 
					 10 * MVAR_BSCT_NUMER_TOL)))
			    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

			CrossProd1 = IRIT_CROSS_PROD_2D(Tan1End1 -> Vec, 
						   Tan2Start1 -> Vec);
			CagdVecFree(Tan1End1);
			if (CrossProd1 < -10 * IRIT_FABS(MVAR_BSCT_NUMER_TOL)) 
			    MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT); 
		    }
		    CagdCrvFree(SmoothCrvs1);
		}
		CagdCrvFree(Crv2);
		
		CrvTemp2 = CrvTemp2 -> Pnext;
	    }
	    /* Calling the PtCrv bisector routine. */
	    CrvTemp2 = CrvTemp1 -> Pnext;
	    PtCrvBisector = MvarBsctTrimCrvPt(CrvTemp2, Pt1, 0.5, CrvTemp1);
	    CagdCrvFree(SmoothCrvs);
	}
    }

    /* When ci's are continuous or discontinuous, the following procedure    */
    /* is adopted.		                                             */
    
    CrvTemp1 = Crv;
    CrvTemp2 = CrvTemp1 -> Pnext;
    while (CrvTemp2 != NULL) {
	/* Splitting at C1 discontinuities.*/
	CagdCrvStruct *SmoothCrvIterNext,
	    *SmoothCrvs = SplitCrvAtDiscontinuity(CrvTemp2),
	    *SmoothCrvIter = SmoothCrvs;
	/* Currently we just support discontinuity at endpoints. */
	if (CagdListLength(SmoothCrvs) != 1)
	    MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	
	while (SmoothCrvIter) {
	    Crv2 = CagdCoerceCrvTo(SmoothCrvIter, CAGD_PT_E2_TYPE, FALSE);

	    if (CAGD_IS_BEZIER_CRV(Crv2)) {
	        CagdCrvStruct
		    *TCrv = CagdCnvrtBzr2BspCrv(Crv2);

		CagdCrvFree(Crv2);
		Crv2 = TCrv;
	    }
	    CrvTmp0 = CagdCrvCopy(Crv1); 
	    CrvTmpi = CagdCrvCopy(Crv2); 
    	
	    /* Freeing memory. */
	    CagdCrvFree(Crv2);

	    MvarBsctComputeF3(CrvTmp0, CrvTmpi, &Crv1Coerced, 
			      &Crv2Coerced,
			      &F3, &L1, &L2, &CC1, &CC2);
	    CagdCrvFree(CrvTmp0);
	    CagdCrvFree(CrvTmpi);

	    OutLst = NULL; 
	    MonotoneCrvs = NULL;
	    MvarBsctSplitImplicitCrvToMonotonePieces(F3, &OutLst, 
						     MvarBsctSubdivTol,
						     -IRIT_FABS(MVAR_BSCT_NUMER_TOL));

	    /* Freeing memory. */
	    CagdSrfFree(F3);
	    CagdSrfFree(L1);
	    CagdSrfFree(L2);
	    CagdSrfFree(CC1);
	    CagdSrfFree(CC2);

	    while (OutLst != NULL) {
	        CagdRType UMin, UMax, VMin, VMax;
		MvarVoronoiCrvStruct *Cv;
		CagdSrfDomain(OutLst, &UMin, &UMax, &VMin, &VMax);

		Cv = MvarVoronoiCrvNew();
		Cv -> Crv1 = CagdCrvCopy(Crv1Coerced);
		Cv -> Crv2 = CagdCrvCopy(Crv2Coerced);
		Cv -> F3 = OutLst;
		Cv -> Type = MV_CV_CV;
		IRIT_LIST_PUSH(Cv, MonotoneCrvs); /* We can push at front    */
		/* because Order is not important here.                      */
		OutLst = OutLst->Pnext;
	    }

	    /* Step 3. Purging away non-LL and non-CC curves. */
	    LegitCrvs = MvarBsctPurgeAwayLLAndCCConstraints(MonotoneCrvs);

	    /* Trimming the curve-curve bisector (for support of */
	    /* non-convex t-domains).                            */
	    TrimBsctrsInTAndR(LegitCrvs, &TrimmedBsct);
	    MvarVoronoiCrvFreeList(LegitCrvs);
	    LegitCrvs = TrimmedBsct;
	    LegitCrvIter = LegitCrvs;
	    /* Copy the whole list. */
	    for ( ; LegitCrvIter != NULL; LegitCrvIter = LegitCrvIter -> Pnext) {
	        AuxCrv = MvarVoronoiCrvCopy(LegitCrvIter);
		IRIT_LIST_PUSH(AuxCrv,InputCurves);
	    }
	    CrvTemp2 = CrvTemp2 -> Pnext;

	    /* Freeing lists for next iteration. */
	    MvarVoronoiCrvFreeList(TrimmedBsct);  /* TrimmedBsct==LegitCrvs. */
	    TrimmedBsct = NULL;
	    MvarVoronoiCrvFreeList(MonotoneCrvs);
	    MonotoneCrvs = NULL;

	    /* Handling C1-discontinous points starts here. */
	    SmoothCrvIterNext = SmoothCrvIter -> Pnext;
	    if (SmoothCrvIterNext == NULL) {
	        /* Cyclic iteration for closed curves. */
	        if (CagdIsClosedCrv(Crv2Coerced))
		    SmoothCrvIterNext = SmoothCrvs;
	    }

	    /* Constructing Pt-Cv bisectors.  (i.e., MvarVoronoiCrvStruct of */
	    /* type CV_PT).					             */
	    if (SmoothCrvIterNext != NULL) {
	        CagdRType *R, CrossProd;
		CagdVecStruct *Tan1End, *Tan2Start;
		CagdPType Pt1, Pt2;
		/* Compare last point of SmoothCrvIter to first point of     */
		/* SmoothCrvIterNext.			                     */
		CagdCrvDomain(SmoothCrvIter, &UMin, &UMax);
		CagdCrvDomain(SmoothCrvIterNext, &VMin, &VMax);
			
		R = CagdCrvEval(SmoothCrvIter, UMax); 	   
		CagdCoerceToE3(Pt1, &R, -1, SmoothCrvIter -> PType); 
		R = CagdCrvEval(SmoothCrvIterNext, VMin); 	   
		CagdCoerceToE3(Pt2, &R, -1, SmoothCrvIterNext -> PType); 
        
		/* Sanity checks. */
		if (!(IRIT_APX_EQ_EPS(Pt2[0], Pt1[0], 10 * MVAR_BSCT_NUMER_TOL) && 
		      IRIT_APX_EQ_EPS(Pt2[1], Pt1[1], 10 * MVAR_BSCT_NUMER_TOL)))
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
		/* Copy is needed since it returns a static. */
		Tan1End = CagdVecCopy(CagdCrvTangent(SmoothCrvIter,
						     UMax,TRUE)); 
			
		Tan2Start = CagdCrvTangent(SmoothCrvIterNext,VMin, TRUE);
		/* Sanity checks. */
		if (!(IRIT_APX_EQ_EPS(Tan1End -> Vec[2], 0.0, 
				 10 * MVAR_BSCT_NUMER_TOL)))
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
		if (!(IRIT_APX_EQ_EPS(Tan2Start -> Vec[2], 0.0, 
				 10 * MVAR_BSCT_NUMER_TOL)))
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

		CrossProd = IRIT_CROSS_PROD_2D(Tan1End -> Vec, Tan2Start -> Vec);
		CagdVecFree(Tan1End);
		/* We are interested only in points that are outer   */
		/* cusps. Assuming left orientation, these are those */
		/* where Tan2Start is to the right of Tan1End.       */
		if (CrossProd < -10 * IRIT_FABS(MVAR_BSCT_NUMER_TOL)) {
		    /* Compute the trimmed Pt-Crv bisector,    */
		    /* construct the Pt-Crv VorCrvs and insert */
		    /* them into InputCrvs.                    */
		    CagdCrvStruct 
		        *BisectorCrvs = MvarBsctTrimCrvPt(Crv1Coerced, Pt1,
							  0.5, Crv2Coerced);

		    /* Construct the Pt-Cv VorCrvs. */
		    CagdCrvStruct 
		        *CrvIter = BisectorCrvs;
		    while (CrvIter) {
		        MvarVoronoiCrvStruct *Cv;
			Cv = MvarVoronoiCrvNew();
			Cv -> Crv1 = CagdCrvCopy(Crv1Coerced);
			Cv -> Crv2 = CagdCrvCopy(CrvIter);
			Cv -> F3 = NULL;
			Cv -> Type = MV_CV_PT;
			Cv -> Pt[0] = Pt1[0];
			Cv -> Pt[1] = Pt1[1];
			IRIT_LIST_PUSH(Cv, InputCurves); /* Push in front. */

			CrvIter = CrvIter -> Pnext;
		    }
		    CagdCrvFreeList(BisectorCrvs);
		}
	    }
		
	    SmoothCrvIter = SmoothCrvIter -> Pnext;
		
	    CagdCrvFree(Crv1Coerced);
	    CagdCrvFree(Crv2Coerced);
	}
	CagdCrvFree(SmoothCrvs);
    }

    CagdCrvFree(Crv1);
            
   
    /* Step 4. Calling lower envelope function. */
    
    /* Main Call to Lower Envelope Function. */
    
    MvarBsctComputeLowerEnvelope(InputCurves, &LowerEnvelope);
    MvarVoronoiCrvFreeList(InputCurves);

    /* Constructing the output polygonal lines. */
    PPolyObj = DisplayVoronoiCell(LowerEnvelope, PtCrvBisector);
    MvarVoronoiCrvFreeList(LowerEnvelope);
    if (PtCrvBisector != NULL)
        CagdCrvFreeList(PtCrvBisector);
    return PPolyObj;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Display of Voronoi cell.                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   LowerEnvelope: List of Lower envelopes for displaying the Voronoi cell.  *
*   PtCrvBisector: N.S.F.I.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   IPObjectStruct *:  Returns the Voronoi cell.                             *
*****************************************************************************/
static IPObjectStruct *DisplayVoronoiCell(MvarVoronoiCrvStruct *LowerEnvelope,
					  CagdCrvStruct *PtCrvBisector)
{
    /* Constructing the output polygonal lines.*/
    int j;
    CagdRType UMin, UMax, VMin, VMax, *XY1Pt;  
    MvarVoronoiCrvStruct  
        *LEnvelopeTmp = LowerEnvelope;
    MvarPtStruct *PtZerosLst;
    CagdCrvStruct 
        *PtCrvBisectorTmp = PtCrvBisector;
    IPObjectStruct *PPolyObj;
    IPVertexStruct *V;
    IPPolygonStruct *PPoly,
	*PlHead = NULL;
    CagdRType *R1;
    CagdPType Pt1, Pt2, Nrml1, Nrml2;
    CagdVecStruct *Vec;

    while (LEnvelopeTmp != NULL) {
        PPoly = IPAllocPolygon(0, NULL, PlHead); 
        if (LEnvelopeTmp -> Type == MV_CV_CV) {
       	    CagdSrfDomain(LEnvelopeTmp -> F3, &UMin, &UMax, &VMin, &VMax);
        }
        else {
            /* Sanity check. */
            if (LEnvelopeTmp -> Type != MV_CV_PT)
                MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
            CagdCrvDomain(LEnvelopeTmp -> Crv2, &UMin, &UMax); 
        }

	for (j = 0; j <= DisplayFactor; ++j) {
	    CagdRType 
	        delta = (UMax - UMin) / DisplayFactor,
	        t = UMin + j*delta;

            if (LEnvelopeTmp -> Type == MV_CV_CV) {
                /* Adding CV_CV attribute to bisector.*/ 
                AttrSetIntAttrib(&PPoly -> Attr, "CV_CV", TRUE);

	        PtZerosLst = 
		    MvarBsctNewFindZeroSetOfSrfAtParam(LEnvelopeTmp -> F3, t,
						       CAGD_CONST_U_DIR, 
						       MvarBsctSubdivTol, 
						       -IRIT_FABS(MVAR_BSCT_NUMER_TOL),
						       TRUE);
	        if (!PtZerosLst) {
#                   ifdef DEBUG_VORONOI
	                printf("Zero set is empty at param %f\n", t);
#                   endif /* DEBUG_VORONOI */
		    continue;
	        }
#               ifdef DEBUG_VORONOI
		if (PtZerosLst -> Pnext != NULL) {
		    printf("Found %d points for the same parameter (expected one)!\n", CagdListLength(PtZerosLst));
		}
#               endif /* DEBUG_VORONOI */
	
	        XY1Pt = MvarBsctComputeXYFromBisTR(LEnvelopeTmp -> Crv1, t,
						   LEnvelopeTmp -> Crv2,
						   PtZerosLst -> Pt[1]);
	
		Vec = CagdCrvNormalXY(LEnvelopeTmp -> Crv1, t, TRUE);
		IRIT_PT_COPY(Nrml1, Vec -> Vec);
		Vec = CagdCrvNormalXY(LEnvelopeTmp -> Crv2, 
				      PtZerosLst -> Pt[1], TRUE);
		IRIT_PT_COPY(Nrml2, Vec -> Vec);

		R1 = CagdCrvEval(LEnvelopeTmp -> Crv1, t);
		CagdCoerceToE3(Pt1, &R1, -1, LEnvelopeTmp -> Crv1 -> PType);
		R1 = CagdCrvEval(LEnvelopeTmp -> Crv2, PtZerosLst -> Pt[1]);
		CagdCoerceToE3(Pt2, &R1, -1, LEnvelopeTmp -> Crv2 -> PType);
		
		/* Checking for the Dot product for collinearity. */
		if (IRIT_DOT_PROD(Nrml1, Nrml2) == -1)
		    IRIT_PT_BLEND(XY1Pt, Pt1, Pt2, 0.5);
	        /* Avoiding infinite artifacts (X1Pt might return NULL for   */
		/* infinite points).					     */
		if ( XY1Pt &&
		      IRIT_FABS(XY1Pt[0]) < BBOX_MAX_X &&
		      IRIT_FABS(XY1Pt[1]) < BBOX_MAX_Y
		     ) {
	      	    V = IPAllocVertex(0, NULL, PPoly -> PVertex);
		    V -> Coord[0] = XY1Pt[0];
		    V -> Coord[1] = XY1Pt[1];
		    PPoly -> PVertex = V;
	        }

                MvarPtFreeList(PtZerosLst);
	    }
            else {
                CagdRType
		    w = 1.0;
                /* Sanity check. */
                if (LEnvelopeTmp -> Type != MV_CV_PT)
                    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

                /* Adding CV_PT attribute to bisector.*/ 
                AttrSetIntAttrib(&PPoly -> Attr, "CV_PT", TRUE);

                XY1Pt = CagdCrvEval(LEnvelopeTmp -> Crv2, t);

                /* Avoiding infinite artifacts (X1Pt might return NULL for   */
		/* infinite points).					     */
	        if (XY1Pt &&
		    IRIT_FABS(XY1Pt[1]) < BBOX_MAX_X &&
		    IRIT_FABS(XY1Pt[2]) < BBOX_MAX_Y) {
	      	    V = IPAllocVertex(0, NULL, PPoly -> PVertex);
                    if (MVAR_IS_RATIONAL_PT(LEnvelopeTmp -> Crv2 -> PType))
                        w = XY1Pt[0];
		    V -> Coord[0] = XY1Pt[1]/w;
		    V -> Coord[1] = XY1Pt[2]/w;
		    PPoly -> PVertex = V;
                }
            }
	}
	PlHead = PPoly;	      
	LEnvelopeTmp = LEnvelopeTmp -> Pnext;
    }
    if (PtCrvBisectorTmp != NULL) {
        while (PtCrvBisectorTmp != NULL) {
	    CagdCrvStruct *PtCrvTmp;
	    PPoly = IPAllocPolygon(0, NULL, PlHead); 
	    PtCrvTmp = CagdCoerceCrvTo(PtCrvBisectorTmp, CAGD_PT_E2_TYPE, FALSE);
	    CagdCrvDomain(PtCrvTmp, &UMin, &UMax); 
        
	    for (j = 0; j <= DisplayFactor; ++j) {
	        CagdRType *R,
		    delta = (UMax - UMin)/DisplayFactor,
		    t = UMin + j*delta;
		CagdPType Pt1;
		
		R = CagdCrvEval(PtCrvTmp, t); 	   
		CagdCoerceToE3(Pt1, &R, -1, PtCrvTmp -> PType); 
		V = IPAllocVertex(0, NULL, PPoly -> PVertex);
		V -> Coord[0] = Pt1[0];
		V -> Coord[1] = Pt1[1];
		PPoly -> PVertex = V;
	    }
	    PlHead = PPoly;	      
	    PtCrvBisectorTmp = PtCrvBisectorTmp -> Pnext;
	}
    }

    PPolyObj = IPGenPOLYLINEObject(PlHead);
    
    return PPolyObj;
}

