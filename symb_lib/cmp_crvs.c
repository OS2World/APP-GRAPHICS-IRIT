/******************************************************************************
* cmp_crvs.c - curves' comparison routines.                                   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Diana Pekerman, November. 2004.                                  *
******************************************************************************/

#include "symb_lib.h"

static SymbCrvRelType BzrCrvsCompare(CagdCrvStruct *Crv1,
                                     CagdCrvStruct *Crv2,
                                     CagdRType Eps,
                                     CagdRType *AffineMapA,
                                     CagdRType *AffineMapB,
                                     CagdCrvStruct **CompsCrvs1,
                                     CagdCrvStruct **CompsCrvs2);
static CagdBType BzrCrvsOverlap(CagdCrvStruct *Crv1,
                                CagdCrvStruct *Crv2,
                                CagdRType Eps,
                                CagdRType *AffineMapA,
                                CagdRType *AffineMapB);
static CagdCrvStruct *CanonicBzrCrv(CagdCrvStruct *Crv,
                                    CagdRType Eps,
                                    CagdCrvStruct **CompsCrvs);
static CagdBType SegmentsOverlapCheck(CagdRType AffineMapA,
                                      CagdRType AffineMapB,
                                      CagdRType Eps);
static void FindOverlapDomain(CagdRType AffineMapA,
                              CagdRType AffineMapB,
                              CagdRType Eps,
                              CagdCrvStruct *CompsCrvs1,
                              CagdCrvStruct *CompsCrvs2,
                              CagdRType *BeginPrmCrv1,
                              CagdRType *EndPrmCrv1,
                              CagdRType *BeginPrmCrv2,
                              CagdRType *EndPrmCrv2);
static void FindSubdivCrvPrm(CagdRType AffineMapA,
                             CagdRType AffineMapB,
                             CagdRType Eps,
                             CagdCrvStruct *CompsCrvs1,
                             CagdCrvStruct *CompsCrvs2,
                             CagdRType *PrmCrv1,
                             CagdRType *PrmCrv2);
static CagdRType RealBzrPrmCompute(CagdCrvStruct *CompsCrvs,
				   CagdRType Eps,
				   CagdRType BzrPrm);
static CagdRType CrvPrmCompute(const CagdCrvStruct *Crv,
                               int BzrCrvNum,
                               CagdRType BzrPrm);
static CagdCrvStruct *MySymbDecomposeCrvCrv(CagdCrvStruct *Crv);


/*****************************************************************************
* DESCRIPTION:							 	     M
*   For given Bezier curve, returns its irreducible version (its canonical   M
*   representation), by reversing the processes of degree raising and        M
*   composition.                                                             M
*									     *
* PARAMETERS:								     M
*   Crv:       A Bezier curve.						     M
*   Eps:       A threshold for "canonical representation" computations.	     M
*									     *
* RETURN VALUE:							             M
*   CagdCrvStruct *: Canonically represented Bezier curve.		     M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCanonicBzrCrv                                                        M
*****************************************************************************/
CagdCrvStruct *SymbCanonicBzrCrv(const CagdCrvStruct *Crv, CagdRType Eps)
{
    CagdBType
	CanonicFlag = FALSE;
    CagdCrvStruct 
	*CanonicCrv = NULL,
	*DegReducedCrv = NULL,
	*DecmpCrv = NULL;
    CagdBType DegReduceTest, DecmpTest;

    if (Crv == NULL || Crv -> GType != CAGD_CBEZIER_TYPE)
	return NULL;

    CanonicCrv = CagdCrvCopy(Crv);
    
    while (!CanonicFlag) {
	/* Applying degree reduction: */
	DegReducedCrv = SymbBzrDegReduce(CanonicCrv, Eps);
	if (DegReducedCrv == NULL)
	    DegReduceTest = FALSE;
	else {
	    CagdCrvFree(CanonicCrv);
	    CanonicCrv = CagdCrvCopy(DegReducedCrv);
	    DegReduceTest = TRUE;
	}

	/* Applying decomposition: */
	DecmpCrv = MySymbDecomposeCrvCrv(CanonicCrv);
	if (DecmpCrv == NULL)
	    DecmpTest = FALSE;
	else {
	    CagdCrvFree(CanonicCrv);
	    CanonicCrv = CagdCrvCopy(DecmpCrv);
	    DecmpTest = TRUE;
	}
	CagdCrvFree(DegReducedCrv);
	CagdCrvFreeList(DecmpCrv);

	if ((DegReduceTest == FALSE) && (DecmpTest == FALSE))
	    CanonicFlag = TRUE;
    }
    return CanonicCrv;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   For given Bezier curve, performs maximal degree reduction of the curve.  M
*   Maximal degree reduction is defined as a process of degree redusing the  M
*   curve as long as the new curve represents the same curve.                M
*									     *
* PARAMETERS:								     M
*   Crv:   A Bezier curve.						     M
*   Eps:   A threshold for degree reduction computations.	             M
*									     *
* RETURN VALUE:							             M
*   CagdCrvStruct *: Maximal degree reduced curve. If no degree reduction    M
*		     was done, returns NULL.	                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbBzrDegReduce                                                         M
*****************************************************************************/
CagdCrvStruct *SymbBzrDegReduce(const CagdCrvStruct *Crv, CagdRType Eps)
{
    int i, Deg,
        ZeroCount = 0,
        MaxCoord = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdBType 
        IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    CagdCrvStruct *DegReducedCrv,
	*PwrCrv = NULL;

    if (Crv == NULL || Crv -> GType != CAGD_CBEZIER_TYPE)
	return NULL;
    
    PwrCrv = CagdCnvrtBzr2PwrCrv(Crv);
    
    Deg = (PwrCrv -> Length) - 1;
    while (Deg >= 0) {
	for (i = IsNotRational; i <= MaxCoord; i++)
	    if (fabs(PwrCrv -> Points[i][Deg]) <= Eps) 
		ZeroCount++;
	if (ZeroCount != MaxCoord) {
	    if (Deg == (PwrCrv -> Length) - 1) { 
		/* In this case no degree reduction of the curve was done. */
		CagdCrvFree(PwrCrv);
		return NULL;
	    }
	    else {
		/* In this case we return degree reduced curve. */
		DegReducedCrv = CagdCrvNew(PwrCrv -> GType,
					   PwrCrv -> PType, Deg + 1);
		for (i = !CAGD_IS_RATIONAL_PT(PwrCrv -> PType);
		     i <= MaxCoord;
		     i++) {
		    CAGD_GEN_COPY(DegReducedCrv -> Points[i],
				  PwrCrv -> Points[i],
				  sizeof(CagdRType) * DegReducedCrv -> Length);
		}
		DegReducedCrv = CagdCnvrtPwr2BzrCrv(DegReducedCrv);
		CagdCrvFree(PwrCrv);
		return DegReducedCrv;
	    }
	}
	else {
	    Deg--;
	    ZeroCount = 0;
	}
    }

    /* Return a zero-degree curve with "zero-coefficient". */
    DegReducedCrv = CagdCrvNew(PwrCrv -> GType, PwrCrv -> PType, 1);
    for (i = !CAGD_IS_RATIONAL_PT(PwrCrv -> PType); i <= MaxCoord; i++) {
	CAGD_GEN_COPY(DegReducedCrv -> Points[i],
		      Crv -> Points[i],
		      sizeof(CagdRType) * DegReducedCrv -> Length);
    }
    DegReducedCrv = CagdCnvrtPwr2BzrCrv(DegReducedCrv);
    CagdCrvFree(PwrCrv);

    return DegReducedCrv;	   
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Compares the given two curves. Each curve is converted, if necessary,    M
*   into a set of Bezier curves, and the comparison is done by applying      M
*   comparison algorithm for each Bezier curve.                              M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1, Crv2:        The two curves to be compared.                        M
*   Eps:               A threshold for numerical computations.               M
*   StartOvrlpPrmCrv1: If the curves are the same and overlapping, here the  M
*		       start of overlapping domain of Crv1 will be returned. M
*   EndOvrlpPrmCrv1:   If the curves are the same and overlapping, here the  M
*		       end of overlapping domain of Crv1 will be returned.   M
*   StartOvrlpPrmCrv2: If the curves are the same and overlapping, here the  M
*		       start of overlapping domain of Crv2 will be returned. M
*   EndOvrlpPrmCrv2:   If the curves are the same and overlapping, here the  M
*		       end of overlapping domain of Crv2 will be returned.   M
*                                                                            *
* RETURN VALUE:                                                              M
*   SymbCrvRelType:    Either Same curves (1), overlapping curves (2), or    M
*		       distinct curves (3).  If Overlapping, the Start/End   M
*		       overlapping parametric domain variables are set.      M
*                                                                            *
* KEYWORDS:                                                                  M
*   SymbCrvsCompare                                                          M
*****************************************************************************/
SymbCrvRelType SymbCrvsCompare(const CagdCrvStruct *Crv1,
			       const CagdCrvStruct *Crv2,
			       CagdRType Eps,
			       CagdRType *StartOvrlpPrmCrv1,
			       CagdRType *EndOvrlpPrmCrv1,
			       CagdRType *StartOvrlpPrmCrv2,
			       CagdRType *EndOvrlpPrmCrv2)
{   
    int BeginCnt1, BeginCnt2, EndCnt1, EndCnt2, TempCnt1, TempCnt2,
	LengthCrv1, LengthCrv2;
    CagdBType
        CommonBeginDetect = FALSE,
        TheEnd = FALSE;
    CagdRType AffineMapA, AffineMapB, BeginBzrPrm1, EndBzrPrm1, BeginBzrPrm2,
	EndBzrPrm2, EndAffineMapA, EndAffineMapB,
	BeginAffineMapA = 0.0,
	BeginAffineMapB = 0.0;
    SymbCrvRelType Relation;
    CagdCrvStruct 
	*BzrCrvList1 = NULL,
	*BzrCrvList2 = NULL,
	*TempCrv1 = NULL,
	*TempCrv2 = NULL,
	*TempPrevCrv1 = NULL,
	*TempPrevCrv2 = NULL,
	*TempCrv = NULL,
	*BeginCrv1 = NULL,
	*RefinedCrv1 = NULL,
	*RefinedCrv2 = NULL,
	*TempCompsCrvs1 = NULL,
	*TempCompsCrvs2 = NULL,
	*BeginCompsCrvs1 = NULL,
	*BeginCompsCrvs2 = NULL,
	*EndCompsCrvs1 = NULL,
	*EndCompsCrvs2 = NULL;

    *StartOvrlpPrmCrv1 =
        *EndOvrlpPrmCrv1 =
            *StartOvrlpPrmCrv2 =
                *EndOvrlpPrmCrv2 = 0.0;

    if (Crv1 -> GType == CAGD_CBEZIER_TYPE)
        BzrCrvList1 = CagdCrvCopy(Crv1);
    else
        if (Crv1 -> GType == CAGD_CBSPLINE_TYPE)
            BzrCrvList1 = CagdCnvrtBsp2BzrCrv(Crv1);

    if (Crv2 -> GType == CAGD_CBEZIER_TYPE)
        BzrCrvList2 = CagdCrvCopy(Crv2);
    else
        if (Crv2 -> GType == CAGD_CBSPLINE_TYPE)
            BzrCrvList2 = CagdCnvrtBsp2BzrCrv(Crv2);

    if (((Crv1 -> GType != CAGD_CBSPLINE_TYPE) &&
	 (Crv1 -> GType != CAGD_CBEZIER_TYPE)) ||
        ((Crv2 -> GType != CAGD_CBSPLINE_TYPE) &&
	 (Crv2 -> GType != CAGD_CBEZIER_TYPE)) ||
	(BzrCrvList1 == NULL || BzrCrvList2 == NULL)) {
	Relation = SYMB_CRVREL_NO_RELATION;
	TheEnd = TRUE;
	return Relation;
    }

    TempCrv1 = BzrCrvList1;
    TempCrv2 = BzrCrvList2;
    BeginCrv1 = BzrCrvList1;
    RefinedCrv1 = CagdCrvCopy(Crv1);
    RefinedCrv2 = CagdCrvCopy(Crv2);

    BeginCnt1 = BeginCnt2 = EndCnt1 = EndCnt2 = TempCnt1 = TempCnt2 = 1;
    Relation = SYMB_CRVREL_SAME_CRVS;

    while (!TheEnd) {
	if (TempCompsCrvs1 != NULL)
	    CagdCrvFreeList(TempCompsCrvs1);
	if (TempCompsCrvs2 != NULL)
	    CagdCrvFreeList(TempCompsCrvs2);

	Relation = BzrCrvsCompare(TempCrv1,
				  TempCrv2,
				  Eps,
				  &AffineMapA,
				  &AffineMapB,
				  &TempCompsCrvs1,
				  &TempCompsCrvs2);

	switch (Relation) {	
	    case SYMB_CRVREL_DISTINCT_CRVS:
	        if (CommonBeginDetect == TRUE) { 
		    /* In this case the curves are overlapping. */
		    Relation = SYMB_CRVREL_OVERLAPPING_CRVS;
		    EndCnt1 = TempCnt1 - 1;
		    EndCnt2 = TempCnt2 - 1;
		    BzrCrvsCompare(TempPrevCrv1,
				   TempPrevCrv2,
				   Eps,
				   &EndAffineMapA,
				   &EndAffineMapB,
				   &EndCompsCrvs1,
				   &EndCompsCrvs2);
		    TheEnd = TRUE;
		}
		else {
		    if (NULL != TempCrv2 -> Pnext) {
		        TempPrevCrv2 = TempCrv2; 
			TempCrv2 = TempCrv2 -> Pnext;
			TempCnt2++;
		    }
		    else {
		        if (NULL == TempCrv1 -> Pnext) {
			    Relation = SYMB_CRVREL_DISTINCT_CRVS;
			    TheEnd = TRUE;
			}
			if (NULL != TempCrv1 -> Pnext) {
			    TempPrevCrv1 = TempCrv1; 
			    BeginCrv1 = BeginCrv1 -> Pnext;
			    TempCrv1 = BeginCrv1;
			    TempCnt1++;
			    TempCrv2 = BzrCrvList2;
			    TempPrevCrv2 = NULL; 
			    TempCnt2 = 1;
			}
		    }
		}
		break;

	    case SYMB_CRVREL_SAME_CRVS:
	        if (CommonBeginDetect == FALSE) {
		    CommonBeginDetect = TRUE;
		    BeginCnt1 = TempCnt1;
		    BeginCnt2 = TempCnt2;
		    BeginAffineMapA = 1;
		    BeginAffineMapB = 0;
		    BeginCompsCrvs1 = CagdCrvCopyList(TempCompsCrvs1);
		    BeginCompsCrvs2 = CagdCrvCopyList(TempCompsCrvs2);
		}

		if (NULL == TempCrv1 -> Pnext || NULL == TempCrv2 -> Pnext) {
		    EndCnt1 = TempCnt1;
		    EndCnt2 = TempCnt2;
		    EndAffineMapA = 1;
		    EndAffineMapB = 0;
		    EndCompsCrvs1 = CagdCrvCopyList(TempCompsCrvs1);
		    EndCompsCrvs2 = CagdCrvCopyList(TempCompsCrvs2);
		    TheEnd = TRUE;
		}
		else {
		    TempPrevCrv1 = TempCrv1; 
		    TempCrv1 = TempCrv1 -> Pnext;
		    TempCnt1++;
		    TempPrevCrv2 = TempCrv2; 
		    TempCrv2 = TempCrv2 -> Pnext;
		    TempCnt2++;
		}
		break;

	    case SYMB_CRVREL_OVERLAPPING_CRVS:
	        if (CommonBeginDetect == FALSE) {
		    CommonBeginDetect = TRUE;
		    BeginCnt1 = TempCnt1;
		    BeginCnt2 = TempCnt2;
		    BeginAffineMapA = AffineMapA;
		    BeginAffineMapB = AffineMapB;
		    BeginCompsCrvs1 = CagdCrvCopyList(TempCompsCrvs1);
		    BeginCompsCrvs2 = CagdCrvCopyList(TempCompsCrvs2);
		}
		FindSubdivCrvPrm(AffineMapA, AffineMapB, Eps,
				 TempCompsCrvs1, TempCompsCrvs2,
				 &EndBzrPrm1, &EndBzrPrm2);
		if ((NULL == TempCrv1 -> Pnext && EndBzrPrm1 == 1) ||
		    (NULL == TempCrv2 -> Pnext && EndBzrPrm2 == 1)) {
		    EndCnt1 = TempCnt1;
		    EndCnt2 = TempCnt2;
		    EndAffineMapA = AffineMapA;
		    EndAffineMapB = AffineMapB;
		    EndCompsCrvs1 = CagdCrvCopyList(TempCompsCrvs1);
		    EndCompsCrvs2 = CagdCrvCopyList(TempCompsCrvs2);
		    TheEnd = TRUE;
		}
		else {
		    if (EndBzrPrm1 == 0) {
		        if (EndBzrPrm2 == 0) {
			    /* Do nothing...*/
			}
			if (EndBzrPrm2 == 1) {
			    TempPrevCrv2 = TempCrv2; 
			    TempCrv2 = TempCrv2 -> Pnext;
			    TempCnt2++;
			}
			if (EndBzrPrm2 != 0 && EndBzrPrm2 != 1) {
			    TempCrv = CagdCrvSubdivAtParam(TempCrv2,
							   EndBzrPrm2);
			    if (TempPrevCrv2 != NULL) 
			        TempPrevCrv2 -> Pnext = TempCrv;
			    else
			        BzrCrvList2 = TempCrv;
			    TempCrv -> Pnext -> Pnext = TempCrv2 -> Pnext;
			    TempPrevCrv2 = TempCrv;
			    TempCrv2 -> Pnext = NULL;
			    CagdCrvFree(TempCrv2);
			    TempCrv2 = TempCrv -> Pnext;
			    RefinedCrv2 =
			        BspCrvKnotInsert(RefinedCrv2,
						 CrvPrmCompute(RefinedCrv2,
							       TempCnt2,
							       EndBzrPrm2));
			    TempCnt2++; 
			}
		    }
		    if (EndBzrPrm1 == 1) {
		        TempPrevCrv1 = TempCrv1; 
			TempCrv1 = TempCrv1 -> Pnext;
			TempCnt1++;
			if (EndBzrPrm2 == 0) {
			    /* Do nothing...*/
			}
			if (EndBzrPrm2 == 1) {
			    TempPrevCrv2 = TempCrv2; 
			    TempCrv2 = TempCrv2 -> Pnext;
			    TempCnt2++;
			}
			if (EndBzrPrm2 != 0 && EndBzrPrm2 != 1) {
			    TempCrv = CagdCrvSubdivAtParam(TempCrv2,
							   EndBzrPrm2);
			    if (TempPrevCrv2 != NULL)
			        TempPrevCrv2 -> Pnext = TempCrv;
			    else
			        BzrCrvList2 = TempCrv;
			    TempCrv -> Pnext -> Pnext = TempCrv2 -> Pnext;
			    TempPrevCrv2 = TempCrv;
			    TempCrv2 -> Pnext = NULL;
			    CagdCrvFree(TempCrv2);
			    TempCrv2 = TempCrv -> Pnext;
			    RefinedCrv2 =
			        BspCrvKnotInsert(RefinedCrv2,
						 CrvPrmCompute(RefinedCrv2,
							       TempCnt2,
							       EndBzrPrm2));
			    TempCnt2++; 
			}
		    }
		    if (EndBzrPrm1 != 0 && EndBzrPrm1 != 1) {
		        TempCrv = CagdCrvSubdivAtParam(TempCrv1, EndBzrPrm1);
			if (TempPrevCrv1 != NULL) 
			    TempPrevCrv1 -> Pnext = TempCrv;
			else
			    BzrCrvList1 = TempCrv;
			TempCrv -> Pnext -> Pnext = TempCrv1 -> Pnext;
			TempPrevCrv1 = TempCrv;
			TempCrv1 -> Pnext = NULL;
			CagdCrvFree(TempCrv1);
			TempCrv1 = TempCrv -> Pnext;
			RefinedCrv1 =
			    BspCrvKnotInsert(RefinedCrv1,
					     CrvPrmCompute(RefinedCrv1,
							   TempCnt1,
							   EndBzrPrm1));
			TempCnt1++; 
			if (EndBzrPrm2 == 0) {
			    /* Do nothing...*/
			}
			if (EndBzrPrm2 == 1) {
			    TempPrevCrv2 = TempCrv2; 
			    TempCrv2 = TempCrv2 -> Pnext;
			    TempCnt2++;
			}
			if (EndBzrPrm2 != 0 && EndBzrPrm2 != 1) {
			    TempCrv = CagdCrvSubdivAtParam(TempCrv2,
							   EndBzrPrm2);
			    if (TempPrevCrv2 != NULL) 
			        TempPrevCrv2 -> Pnext = TempCrv;
			    else
			        BzrCrvList2 = TempCrv;
			    TempCrv -> Pnext -> Pnext = TempCrv2 -> Pnext;
			    TempPrevCrv2 = TempCrv;
			    TempCrv2 -> Pnext = NULL;
			    CagdCrvFree(TempCrv2);
			    TempCrv2 = TempCrv -> Pnext;
			    RefinedCrv2 =
			        BspCrvKnotInsert(RefinedCrv2,
						 CrvPrmCompute(RefinedCrv2,
							       TempCnt2,
							       EndBzrPrm2));
			    TempCnt2++; 
			}
		    }
		}
		break;
	    default:
	        assert(0);
	}
    }

    /* A check if the curves are the same or just overlapping. */
    if ((Relation != SYMB_CRVREL_DISTINCT_CRVS)) {
	/* Then the B-spline curves are at least overlapping. */
	Relation = SYMB_CRVREL_OVERLAPPING_CRVS;
	FindOverlapDomain(BeginAffineMapA, BeginAffineMapB, Eps,
			  BeginCompsCrvs1, BeginCompsCrvs2,
			  &BeginBzrPrm1, &EndBzrPrm1,
			  &BeginBzrPrm2, &EndBzrPrm2);
	*StartOvrlpPrmCrv1 = CrvPrmCompute(Crv1, BeginCnt1, BeginBzrPrm1);
	*StartOvrlpPrmCrv2 = CrvPrmCompute(Crv2, BeginCnt2, BeginBzrPrm2);

	/* A suspicion that the B-spline curves are the same. */
	if (BeginCnt1 == 1 &&
	    BeginCnt2 == 1 && 
	    BeginBzrPrm1 <= Eps &&
	    BeginBzrPrm2 <= Eps)
	    Relation = SYMB_CRVREL_SAME_CRVS;/* Not final, needs extra check.*/
	else 
	    Relation = SYMB_CRVREL_OVERLAPPING_CRVS;

	FindOverlapDomain(EndAffineMapA, EndAffineMapB, Eps,
			  EndCompsCrvs1, EndCompsCrvs2,
			  &BeginBzrPrm1, &EndBzrPrm1,
			  &BeginBzrPrm2, &EndBzrPrm2);
	*EndOvrlpPrmCrv1 = CrvPrmCompute(RefinedCrv1, EndCnt1, EndBzrPrm1);
	*EndOvrlpPrmCrv2 = CrvPrmCompute(RefinedCrv2, EndCnt2, EndBzrPrm2);

	/* A continuation of a check if the curves are the same. */
	LengthCrv1 = CagdListLength(BzrCrvList1);
	LengthCrv2 = CagdListLength(BzrCrvList2);

	if (EndCnt1 == LengthCrv1 &&
	    EndCnt2 == LengthCrv2 && 
	    EndBzrPrm1 >= 1 - Eps &&
	    EndBzrPrm2 >= 1 - Eps && 
	    Relation == SYMB_CRVREL_SAME_CRVS)
	    /* Now it's final - the curves are the same. */
	    Relation = SYMB_CRVREL_SAME_CRVS;
	else 
	    Relation = SYMB_CRVREL_OVERLAPPING_CRVS;
    }

    if (BzrCrvList1 != NULL)
	CagdCrvFreeList(BzrCrvList1);
    if (RefinedCrv1 != NULL)
	CagdCrvFree(RefinedCrv1);

    if (BzrCrvList2 != NULL)
	CagdCrvFreeList(BzrCrvList2);
    if (RefinedCrv2 != NULL)
	CagdCrvFree(RefinedCrv2);

    if (TempCompsCrvs1 != NULL)
	CagdCrvFreeList(TempCompsCrvs1);
    if (TempCompsCrvs2 != NULL)
	CagdCrvFreeList(TempCompsCrvs2);
    if (BeginCompsCrvs1 != NULL)
	CagdCrvFreeList(BeginCompsCrvs1);
    if (BeginCompsCrvs2 != NULL)
	CagdCrvFreeList(BeginCompsCrvs2);
    if (EndCompsCrvs1 != NULL)
	CagdCrvFreeList(EndCompsCrvs1);
    if (EndCompsCrvs2 != NULL)
	CagdCrvFreeList(EndCompsCrvs2);

    return Relation;
}

/******************************************************************************
* DESCRIPTION:							              *
*   Compares the given two Bezier curves.				      *
*									      *
* PARAMETERS:								      *
*   Crv1, Crv2: Two Bezier curves to be compared.			      *
*   Eps:        A threshold for numerical computations.			      *
*   AffineMapA: Scaling factor of the Affine Map of the canonic curve	      *
*	        parameter in case the curves are overlapping.		      *
*   AffineMapB: Shifting factor of the Affine Map of the canonic curve        *
*	        parameter in case the curves are overlapping.		      *
*   CompsCrv1:  A list of composing curves of Crv1 to be returned.	      *
*   CompsCrv2:  A list of composing curves of Crv2 to be returned.	      *
*									      *
* RETURN VALUE:							              *
*   SymbCrvRelType: Type of the relation between the two curves.	      *
******************************************************************************/
static SymbCrvRelType BzrCrvsCompare(CagdCrvStruct *Crv1,
				     CagdCrvStruct *Crv2,
				     CagdRType Eps,
				     CagdRType *AffineMapA,
				     CagdRType *AffineMapB,
				     CagdCrvStruct **CompsCrvs1,
				     CagdCrvStruct **CompsCrvs2)
{   
    SymbCrvRelType Relation;
    CagdRType TempA, TempB;
    CagdCrvStruct
	*CanonicCrv1 = NULL,
	*CanonicCrv2 = NULL; 

    if ((Crv1 == NULL) || (Crv2 == NULL))
	return SYMB_CRVREL_NO_RELATION;

    /* Finding canonical representation: */
    CanonicCrv1 = CanonicBzrCrv(Crv1, Eps, CompsCrvs1);
    CanonicCrv2 = CanonicBzrCrv(Crv2, Eps, CompsCrvs2);

    if (CanonicCrv1 -> Length != CanonicCrv2 -> Length) {
	CagdCrvFree(CanonicCrv1);
	CagdCrvFree(CanonicCrv2);
	return SYMB_CRVREL_DISTINCT_CRVS;
    }

    CagdMakeCrvsCompatible(&CanonicCrv1, &CanonicCrv2, FALSE, FALSE);
    
    /* Applying the test for similarity or overlapping of curves.*/
    if (BzrCrvsOverlap(CanonicCrv1, CanonicCrv2, Eps, &TempA, &TempB)) {
	if ((fabs(TempA - 1) <= Eps) && (fabs(TempB) <= Eps))
	    Relation = SYMB_CRVREL_SAME_CRVS;
	else {
	    Relation = SYMB_CRVREL_OVERLAPPING_CRVS;
	    *AffineMapA = TempA;
	    *AffineMapB = TempB;
	}
    }
    else 
	Relation = SYMB_CRVREL_DISTINCT_CRVS;
    
    CagdCrvFree(CanonicCrv1);
    CagdCrvFree(CanonicCrv2);

    return Relation;
}

/******************************************************************************
* DESCRIPTION:								      *
*   For given two canonically represented Bezier curves from the same degree, *
*   checks whether they are overlapping.				      *
*     The overlapping check is done by examining the existance of the affine  *
*   transformation (and its characteristic) of curve parameter such that:     *
*                C1(t) = C2(A*t + B),					      *
*   where C1(t) and C2(t), 0 <= t <= 1 denote the Bezier curves.              *
*   Note 1: The overlapping curves can have a gap within the given threshold. *
*   Note 2: The overlapping curves can be the same.			      *
*									      *
* PARAMETERS:								      *
*   Crv1, Crv2:  Two Bezier curves to be checked.			      *
*   Eps:         A threshold for numerical computations.		      *
*   AffineMapA:  Scaling factor of the Affine Map of the curve parameter.     *
*   AffineMapB:  Shifting factor of the Affine Map of the curve parameter.    *
*									      *
* RETURN VALUE:							              *
*   CagdBType: TRUE - in case the given curves are overlapping, else - FALSE. *
******************************************************************************/
static CagdBType BzrCrvsOverlap(CagdCrvStruct *Crv1,
				CagdCrvStruct *Crv2,
				CagdRType Eps,
				CagdRType *AffineMapA,
				CagdRType *AffineMapB)
{   
    int i, Coord, Degree,
	MaxCoord = CAGD_NUM_OF_PT_COORD(Crv1 -> PType);
    CagdRType Ak, Bk, Ak_1, Bk_1, TempA, TempB, TempA_k_1 ;
    CagdRType BeginPrmCrv1, EndPrmCrv1, BeginPrmCrv2, EndPrmCrv2;
    CagdBType 
	Result = FALSE,
	NonZeroFlag = FALSE,
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv1);
    CagdCrvStruct
	*PwrCrv1 = NULL,
	*PwrCrv2 = NULL,
	*TempCrv1 = NULL,
	*TempCrv2 = NULL;

    *AffineMapA = *AffineMapB = 0.0;

    PwrCrv1 = CagdCnvrtBzr2PwrCrv(Crv1);
    PwrCrv2 = CagdCnvrtBzr2PwrCrv(Crv2);

    if (PwrCrv1 == NULL || PwrCrv2 == NULL)
	return FALSE;

    /* Finding the coordinate with non-zero coefficient. */
    i = MaxCoord;
    Degree = PwrCrv1 -> Length - 1;
    while (!NonZeroFlag && i >= IsNotRational) {
	if (fabs(PwrCrv1 -> Points[i][Degree]) > Eps && 
	    fabs(PwrCrv2 -> Points[i][Degree]) > Eps)
	    NonZeroFlag = TRUE;
	else 
	    i--;
    }

    Coord = i;

    Ak = PwrCrv1 -> Points[Coord][Degree];
    Ak_1 = PwrCrv1 -> Points[Coord][Degree - 1];
    Bk = PwrCrv2 -> Points[Coord][Degree];
    Bk_1 = PwrCrv2 -> Points[Coord][Degree - 1];

    /* Calculation of AffineMapA. */
    TempA = pow(Ak / Bk, 1 / (CagdRType) Degree);

    /* Calculation of AffineMapB. */
    TempA_k_1 = pow(TempA, Degree - 1);
    TempB = (Ak_1 - Bk_1 * TempA_k_1) / (Degree * Bk * TempA_k_1);
    TempB = TempB / TempA; 

    if (SegmentsOverlapCheck(TempA, TempB, Eps)) {
	FindOverlapDomain(TempA, TempB, Eps, NULL, NULL,
			  &BeginPrmCrv1, &EndPrmCrv1,
			  &BeginPrmCrv2, &EndPrmCrv2);
	TempCrv1 = CagdCrvRegionFromCrv(Crv1, BeginPrmCrv1, EndPrmCrv1);
	TempCrv2 = CagdCrvRegionFromCrv(Crv2, BeginPrmCrv2, EndPrmCrv2);
	
	Result = CagdCrvsSame(TempCrv1, TempCrv2, Eps);
	if (Result == TRUE) {
	    *AffineMapA = TempA; 
	    *AffineMapB = TempB; 
	}
    }
    CagdCrvFree(PwrCrv1);
    CagdCrvFree(PwrCrv2);
    CagdCrvFree(TempCrv1);
    CagdCrvFree(TempCrv2);

    return Result;    
}

/******************************************************************************
* DESCRIPTION:							 	      *
*   Returns the canonical representation for given Bezier curve, and the      *
* the composing curves.							      *
*									      *
* PARAMETERS:								      *
*   Crv:       A Bezier curve.						      *
*   Eps:       A threshold for "canonical representation" computations.	      *
*   CompsCrv:  A list of composing curves to be returned.		      *
*									      *
* RETURN VALUE:							              *
*   CagdCrvStruct *: Canonically represented Bezier curve.		      *
******************************************************************************/
static CagdCrvStruct *CanonicBzrCrv(CagdCrvStruct *Crv,
				    CagdRType Eps,
				    CagdCrvStruct **CompsCrvs)
{
    CagdBType
	CanonicFlag = FALSE;
    CagdCrvStruct 
	*TempCrv = NULL,
	*CanonicCrv = NULL,
	*DegReducedCrv = NULL,
	*DecmpCrv = NULL;
    CagdBType DegReduceTest, DecmpTest;

    if (Crv == NULL)
	return NULL;

    CanonicCrv = CagdCrvCopy(Crv);
    
    *CompsCrvs = NULL;
    TempCrv = *CompsCrvs;

    while (!CanonicFlag) {
	/* Applying degree reduction: */
	DegReducedCrv = SymbBzrDegReduce(CanonicCrv, Eps);
	if (DegReducedCrv == NULL)
	    DegReduceTest = FALSE;
	else {
	    CagdCrvFree(CanonicCrv);
	    CanonicCrv = CagdCrvCopy(DegReducedCrv);
	    DegReduceTest = TRUE;
	}
	/* Applying decomposition: */
	DecmpCrv = MySymbDecomposeCrvCrv(CanonicCrv);
	if (DecmpCrv == NULL)
	    DecmpTest = FALSE;
	else {
	    CagdCrvFree(CanonicCrv);
	    CanonicCrv = CagdCrvCopy(DecmpCrv);
	    TempCrv = CagdCrvCopy(DecmpCrv -> Pnext);
	    TempCrv -> Pnext = *CompsCrvs;
	    *CompsCrvs = TempCrv;
	    DecmpTest = TRUE;
	}
	CagdCrvFree(DegReducedCrv);
	CagdCrvFreeList(DecmpCrv);

	if ((DegReduceTest == FALSE) && (DecmpTest == FALSE))
	    CanonicFlag = TRUE;
    }
    return CanonicCrv;
}

/******************************************************************************
* DESCRIPTION:								      *
*   For a given affine map of unit segment [0, 1], checks if the unit segment *
*   and affine transformed segment are overlapping.			      *
*									      *
* PARAMETERS:								      *
*   AffineMapA: Scaling factor of the Affine Map of the unit segment.	      *
*   AffineMapB: Shifting factor of the Affine Map of the unit segment.	      *
*   Eps: A threshold for numerical computations.			      *
*									      *
* RETURN VALUE:							              *
*   CagdBType: TRUE - in case the segments are overlapping, else - FALSE.     *
******************************************************************************/
static CagdBType SegmentsOverlapCheck(CagdRType AffineMapA,
				      CagdRType AffineMapB,
				      CagdRType Eps) 
{
    if (AffineMapB > 0 && 1 / AffineMapA - AffineMapB < - Eps) {
	/* In this case "1 / AffineMapA - AffineMapB" is end value of second */
	/* segment.							     */
	return FALSE;
    }

    if (AffineMapB < 0 && -AffineMapB > 1 + Eps) {
        /* In this case "- AffineMapB" is start value of second segment. */
	return FALSE;
    }

    /* If we got here, the curves are overlapping. */
    return TRUE;
}

/******************************************************************************
* DESCRIPTION:								      *
*   For given two overlapping Bezier curves, first curve with parameter t and *
*   the second curve with parameter s, where t, s in [0,1], and for given     *
*   affine transform of the parameter of the second curve, computes the       *
*   overlapping domain of the two curves.				      *
*									      *
* PARAMETERS:								      *
*   AffineMapA:   Scaling factor of the Affine Map of the second curve	      *
*		  parameter.						      *
*   AffineMapB:   Shifting factor of the Affine Map of the second curve       *
*		  parameter.						      *
*   Eps:	  A threshold for numerical computations.		      *
*   CompsCrv1:    A list of composing curves of Crv1.			      *
*   CompsCrv2:    A list of composing curves of Crv2.			      *
*   StartPrmCrv1: Start parameter of the overlapping domain of the first      *
*		  Bezier curve to be returned.				      *
*   EndPrmcrv1:   End parameter of the overlapping domain of the first Bezier *
*		  curve to be returned.				              *
*   StartPrmCrv2: Start parameter of the overlapping domain of the second     *
*		  Bezier curve to be returned.				      *
*   EndPrmcrv2:   End parameter of the overlapping domain of the second       *
*		  Bezier curve to be returned.				      *
*									      *
* RETURN VALUE:							              *
*   void								      *
******************************************************************************/
static void FindOverlapDomain(CagdRType AffineMapA,
			      CagdRType AffineMapB,
			      CagdRType Eps,
			      CagdCrvStruct *CompsCrvs1,
			      CagdCrvStruct *CompsCrvs2,
			      CagdRType *StartPrmCrv1,
			      CagdRType *EndPrmCrv1,
			      CagdRType *StartPrmCrv2,
			      CagdRType *EndPrmCrv2)
{   
    /* The case of "overlapping" curves with the gap: */
    if (AffineMapB > 0 && AffineMapA - AffineMapB < 0) {
	*StartPrmCrv1 = 0;
	*EndPrmCrv1 = 0;
	*StartPrmCrv2 = 1;
	*EndPrmCrv2 = 1;
	return;
    }
    if (AffineMapB < 0 && -AffineMapB > 1) {
	*StartPrmCrv1 = 1;
	*EndPrmCrv1 = 1;
	*StartPrmCrv2 = 0;
	*EndPrmCrv2 = 0;
	return;
    }

    /* The case of "overlapping" curves without the gap:                     */
    /* The case of enlargement of the second parametric domain.              */
    if (AffineMapA >= 1) {
	if (AffineMapB >= 0) {
	    *StartPrmCrv1 = 0;
	    *EndPrmCrv1 =  RealBzrPrmCompute(CompsCrvs1, Eps,
					     1 / AffineMapA - AffineMapB);
	    *StartPrmCrv2 =  RealBzrPrmCompute(CompsCrvs2, Eps,
					       AffineMapB * AffineMapA);
	    *EndPrmCrv2 = 1;
	    return;
	}
	if (1 / AffineMapA - AffineMapB <= 1) {
	    *StartPrmCrv1 = RealBzrPrmCompute(CompsCrvs1, Eps, - AffineMapB);
	    *EndPrmCrv1 = RealBzrPrmCompute(CompsCrvs1, Eps,
					    1 / AffineMapA - AffineMapB);
	    *StartPrmCrv2 = 0;
	    *EndPrmCrv2 = 1;
	    return;
	}
	if (1 / AffineMapA - AffineMapB > 1) {
	    *StartPrmCrv1 = RealBzrPrmCompute(CompsCrvs1, Eps, - AffineMapB);
	    *EndPrmCrv1 = 1;
	    *StartPrmCrv2 = 0;
	    *EndPrmCrv2 = RealBzrPrmCompute(CompsCrvs2, Eps,
					 AffineMapA + AffineMapB * AffineMapA);
	    return;
	}
    }

    /* The case of reduction of the second parametric domain. */
    if (AffineMapA < 1) {
	if (AffineMapB <= 0) {
	    *StartPrmCrv1 = RealBzrPrmCompute(CompsCrvs1, Eps, - AffineMapB);
	    *EndPrmCrv1 = 1;
	    *StartPrmCrv2 = 0;
	    *EndPrmCrv2 = RealBzrPrmCompute(CompsCrvs2, Eps,
					 AffineMapA + AffineMapB * AffineMapA);
	    return;
	}
	if (1 / AffineMapA - AffineMapB <= 1) {
	    *StartPrmCrv1 = 0;
	    *EndPrmCrv1 = RealBzrPrmCompute(CompsCrvs1, Eps,
					    1 / AffineMapA - AffineMapB);
	    *StartPrmCrv2 = RealBzrPrmCompute(CompsCrvs2, Eps,
					      AffineMapB * AffineMapA);
	    *EndPrmCrv2 = 1;
	    return;
	}
	if (1 / AffineMapA - AffineMapB > 1) {
	    *StartPrmCrv1 = 0;
	    *EndPrmCrv1 = 1;
	    *StartPrmCrv2 = RealBzrPrmCompute(CompsCrvs2, Eps,
					      AffineMapB * AffineMapA);
	    *EndPrmCrv2 = RealBzrPrmCompute(CompsCrvs2, Eps,
					AffineMapA + AffineMapB * AffineMapA);
	    return;	
	}
    }
}

/******************************************************************************
* DESCRIPTION:								      *
*   For given two overlapping Bezier curves, first curve with parameter t and *
*   the second curve with parameter s, where t, s in [0,1], and for given     *
*   affine transform of the parameter of the second curve, computes the       *
*   parameters of overlapping domain of the two curves where the subdivision  *
*   should be done for the purposes of "BspCrvsCompare" function.	      *
*									      *
* PARAMETERS:								      *
*   AffineMapA: Scaling factor of the second curve parameter.		      *
*   AffineMapB: Shifting factor of the second curve parameter.		      *
*   Eps:	A threshold for numerical computations.			      *
*   CompsCrv1:    A list of composing curves of Crv1.			      *
*   CompsCrv2:    A list of composing curves of Crv2.			      *
*   PrmCrv1:    Parameter of the first Bezier curve to be returned.	      *
*   PrmCrv2:    Parameter of the second Bezier curve to be returned.	      *
*									      *
* RETURN VALUE:							              *
*   void								      *
*									      *
******************************************************************************/
static void FindSubdivCrvPrm(CagdRType AffineMapA,
			     CagdRType AffineMapB,
			     CagdRType Eps,
			     CagdCrvStruct *CompsCrvs1,
			     CagdCrvStruct *CompsCrvs2,
			     CagdRType *PrmCrv1,
			     CagdRType *PrmCrv2)
{
    CagdRType StartPrmCrv1, EndPrmCrv1, StartPrmCrv2, EndPrmCrv2;

    FindOverlapDomain(AffineMapA, AffineMapB, Eps, CompsCrvs1, CompsCrvs2,
		      &StartPrmCrv1, &EndPrmCrv1, &StartPrmCrv2, &EndPrmCrv2);

    /* Rounding of "boundary cases" due to numerical computations. */
    if (EndPrmCrv1 <= Eps) 
	EndPrmCrv1 = 0;
    if (EndPrmCrv1 >= 1 - Eps) 
	EndPrmCrv1 = 1;
    if (EndPrmCrv2 <= Eps) 
	EndPrmCrv2 = 0;
    if (EndPrmCrv2 >= 1 - Eps) 
	EndPrmCrv2 = 1;

    *PrmCrv1 = EndPrmCrv1;
    *PrmCrv2 = EndPrmCrv2;
}

/******************************************************************************
* DESCRIPTION:								      *
*   For given list of bezier curves: Crv1, Crv2 ... CrvN and a parameter,     *
*   finds the solution to the equation:					      *
*                   Crv1(Crv2(...CrvN(t))...) = parameter.		      *
*									      *
* PARAMETERS:								      *
*   CompsCrvs:  List of Bezier curves.					      *
*   Eps:	A threshold for numerical computations.		              *
*   BzrPrm:     A given parameter.					      *
*									      *
* RETURN VALUE:							              *
*   CagdRType: A solution of the equation.				      *
******************************************************************************/
static CagdRType RealBzrPrmCompute(CagdCrvStruct *CompsCrvs,
				   CagdRType Eps,
				   CagdRType BzrPrm)
{ 
    CagdRType Solution;
    CagdPtStruct 
	*ZeroPts = NULL,
	*TempZeroPt = NULL;
    CagdCrvStruct *TCrv,
	*TempCrv = NULL,
	*CompsCrv = NULL;

    if (CompsCrvs == NULL || fabs(BzrPrm) <= Eps || fabs(BzrPrm - 1) <= Eps)
        return BzrPrm;
    
    TempCrv = CompsCrvs;
    while (TempCrv != NULL && TempCrv -> Pnext != NULL) {
	CompsCrv = SymbComposeCrvCrv(TempCrv, TempCrv -> Pnext);
	TempCrv = TempCrv -> Pnext;
    }
    if (CompsCrv == NULL)
	CompsCrv = CagdCrvCopy(CompsCrvs);
    else {
	TCrv = CagdCnvrtBsp2BzrCrv(CompsCrv);
	CagdCrvFree(CompsCrv);
	CompsCrv = TCrv;
    }

    TCrv = CagdCnvrtBzr2PwrCrv(CompsCrv);
    CagdCrvFree(CompsCrv);
    CompsCrv = TCrv;

    CompsCrv -> Points[1][0] -= BzrPrm;
    TCrv = CagdCnvrtPwr2BzrCrv(CompsCrv);
    CagdCrvFree(CompsCrv);
    CompsCrv = TCrv;

    ZeroPts = SymbCrvZeroSet(CompsCrv, 1, 10e-6, TRUE);
    
    TempZeroPt = ZeroPts; 
    Solution = TempZeroPt -> Pt[0];
    while (TempZeroPt != NULL && (Solution < 0 || Solution > 1)) {
	TempZeroPt = TempZeroPt -> Pnext;
	Solution = TempZeroPt -> Pt[0];
    }
    CagdPtFreeList(ZeroPts);
    CagdCrvFree(CompsCrv);
    
    return Solution; 
}

/******************************************************************************
* DESCRIPTION:								      *
*   For given B-Spline curve, a number of Bezier curve in the set of Bezier   *
*   curves that represent the given B-Spline curve, and a parameter of this   *
*   Bezier curve, computes the appropriate parameter of the B-Spline curve.   *
*   If the given curve is a Bezier curve, returns the given parameter.        *
*									      *
* PARAMETERS:								      *
*   Crv:        The given B-Spline or Bezier curve.		              *
*   BzrCrvNum:  The given number of Bezier curve in the set of Bezier curves, *
*	        that represents the given B-Spline curve.		      *
*   BzrParam:   The given parameter of the Bezier curve.		      *
*									      *
* RETURN VALUE:							              *
*   CagdRType: The appropriate parameter of the B-Spline curve to be returned.*
******************************************************************************/
static CagdRType CrvPrmCompute(const CagdCrvStruct *Crv,
			       int BzrCrvNum,
			       CagdRType BzrPrm)

{
    int i, Order, Length, KIndex;
    CagdRType BeginDomain, EndDomain;
    CagdRType CrvPrm,
	*KnotVector = NULL;
    CagdCrvStruct *TempCrv;

    if (Crv -> GType == CAGD_CBEZIER_TYPE)
        return BzrPrm;

    if (CAGD_IS_BSPLINE_CRV(Crv) && !BspCrvHasOpenEC(Crv))
	TempCrv = BspCrvOpenEnd(Crv);
    else
        TempCrv = CagdCrvCopy(Crv);

    Order = TempCrv -> Order,
    Length = TempCrv -> Length;
    KnotVector = TempCrv -> KnotVector;
    
    /* Finding the appropriate segment in the knot vector. */
    for (KIndex = 0, i = 1; i <= BzrCrvNum; i++)
	KIndex = BspKnotFirstIndexG(KnotVector, Length + Order,
				    KnotVector[KIndex]);

    /* Computing the parameter of the B-Spline curve. */
    EndDomain = KnotVector[KIndex];
    BeginDomain = KnotVector[KIndex - 1];
    CrvPrm = BeginDomain + (EndDomain - BeginDomain) * BzrPrm;

    CagdCrvFree(TempCrv);

    return CrvPrm;
}

/******************************************************************************
* DESCRIPTION:								      *
*   Try to decompose a curve of arbitrary degree if possible.		      *
*   Among all possibilities, return one pair of two curves F and G which      *
* satisfy F(G(t)) = H(t).						      *
*   F can have arbitrary number of coordinates:				      *
*	F(x) = x^m + sum b_j x^j.					      *
*   G should be a scalar monotone curve having a range [0, 1]:		      *
*	G(x) = c_k x^k + ..... + c_1 x.					      *
*									      *
* PARAMETERS:								      *
*   Crv:   A curve H(t) to try and decompose.				      *
*									      *
* RETURN VALUE:							              *
*   CagdCrvStruct *:  Pairs of curves F and G that are composable to the      *
*		      input curve, or NULL if nonreducable.		      *
*		      F: [0, 1] -> R^3,					      *
*		      G: [0, 1] -> [0, 1].				      *
******************************************************************************/
static CagdCrvStruct *MySymbDecomposeCrvCrv(CagdCrvStruct *Crv)
{
    CagdBType
	IsNotRational = !CAGD_IS_RATIONAL_CRV(Crv);
    int i,
	NumCoords = CAGD_NUM_OF_PT_COORD(Crv -> PType);
    CagdCrvStruct *DecmpCrvVec[CAGD_MAX_PT_SIZE],
	*TempCrv = NULL,
	*TempCrv1 = NULL,
	*TempCrv2 = NULL,
	**CrvVec = NULL,
	*DecmpCrv = NULL,
	*DecmpCrvScalar = NULL,
	*DecmpCrvList1 = NULL,
	*DecmpCrvList2 = NULL;	
    
    if (Crv == NULL)
	return NULL;

    CrvVec = SymbCrvSplitScalarN(Crv);
    /* Applying the decomposition on each curve in the vector of curves. */
    for (i = IsNotRational; i <= NumCoords; i++) {
	DecmpCrvScalar = SymbDecomposeCrvCrv(CrvVec[i]);
	if (DecmpCrvScalar == NULL) {
	    for (i = IsNotRational; i < NumCoords + 1; i++)
	        CagdCrvFree(CrvVec[i]);
	    if (DecmpCrvList1 != NULL)
		CagdCrvFreeList(DecmpCrvList1);
	    if (DecmpCrvList2 != NULL)
		CagdCrvFreeList(DecmpCrvList2);
	    return NULL;
	}
	TempCrv1 = CagdCrvCopy(DecmpCrvScalar);
	TempCrv2 = CagdCrvCopy(DecmpCrvScalar -> Pnext);
	TempCrv1 -> Pnext = DecmpCrvList1;
	TempCrv2 -> Pnext = DecmpCrvList2;
	DecmpCrvList1 = TempCrv1;
	DecmpCrvList2 = TempCrv2;
	CagdCrvFreeList(DecmpCrvScalar);
    }
    DecmpCrvList1 = CagdListReverse(DecmpCrvList1);
    
    /* A check whether the "G(t) function" is the same for each curve in    */
    /* the vector of curves.						    */
    TempCrv = DecmpCrvList2;
    TempCrv1 = CagdCrvCopy(DecmpCrvList2);
    while (TempCrv != NULL) {
	TempCrv2 = CagdCrvCopy(TempCrv);
	if (CagdCrvsSame(TempCrv1, TempCrv2, IRIT_EPS))
	    TempCrv = TempCrv -> Pnext;
	else {
	    for (i = IsNotRational; i < NumCoords + 1; i++)
	        CagdCrvFree(CrvVec[i]);
	    CagdCrvFree(TempCrv1);
	    CagdCrvFree(TempCrv2);
	    CagdCrvFreeList(DecmpCrvList1);
	    CagdCrvFreeList(DecmpCrvList2);
	    return NULL;
	}
	CagdCrvFree(TempCrv2);
    }
    CagdCrvFree(TempCrv1);

    /* Applying a transformation from linked list of curves to vector of    */
    /* curves:								    */
    TempCrv = DecmpCrvList1;
    DecmpCrvVec[0] = IsNotRational == TRUE ? NULL : CagdCrvCopy(DecmpCrvList1);
    for (i = IsNotRational; i <= NumCoords; i++) {
	DecmpCrvVec[i] = CagdCrvCopy(TempCrv);
	TempCrv = TempCrv -> Pnext; 
    }

    DecmpCrv = SymbCrvMergeScalarN(DecmpCrvVec[0],
				   (CagdCrvStruct const **) &DecmpCrvVec[1],
				   NumCoords);
    for (i = IsNotRational; i <= NumCoords; i++) {
	CagdCrvFree(CrvVec[i]);
	CagdCrvFree(DecmpCrvVec[i]);
    }
    IritFree(CrvVec);

    DecmpCrv -> Pnext = CagdCrvCopy(DecmpCrvList2);
    CagdCrvFree(DecmpCrvList1);
    CagdCrvFree(DecmpCrvList2);

    return DecmpCrv;
}
