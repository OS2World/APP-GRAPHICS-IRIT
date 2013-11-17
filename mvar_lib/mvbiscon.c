/******************************************************************************
* mvbiscon.c - Orientation (LL) and curvature (CC) constraints.	              *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Muthuganapathy Ramanathan and Iddoh Hanniel, March 2005.	      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h> 
#include "geom_lib.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h" 
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point is  to   M
*   the left of the curve or not.                			     M
*   Compute the constraints as dot products                                  M
*   1st constraint <P(t,r) -  C1(t), N1(t)>  >  0                            M
*   2nd constraint <P(t,r) -  C2(r), N2(r)>  >  0                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv:     MvarVoronoiCrvStruct                                             M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE or FALSE.                                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctApplyLL                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctIsCurveLL, bisectors                                             M
*****************************************************************************/
int MvarBsctIsCurveLL(MvarVoronoiCrvStruct *Cv)
{ 	
    CagdRType UMin1, UMax1, VMin1, VMax1, UMid, VMid, *R2, *R3; 	
    CagdPType Pt1, Pt2, BP, Nrml1, Nrml2, DotProd; 	
    CagdVecStruct *Vec; 	
    IrtPtType Vtemp;    
    CagdSrfStruct 
        *MonotoneF3Srf = Cv -> F3;   
    CagdCrvStruct 
        *Crv1 = Cv -> Crv1,
        *Crv2 = Cv -> Crv2;     
    MvarPtStruct *PtZerosLst; 
    	
    CagdSrfDomain(MonotoneF3Srf, &UMin1, &UMax1, &VMin1, &VMax1);
  
    /* Compute mid paramater. */ 	
    UMid = (UMin1 + UMax1) / 2.0;    
    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(MonotoneF3Srf, UMid,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MVAR_BSCT_NUMER_TOL),
						    TRUE);
           
    if (PtZerosLst != NULL) {
        VMid = PtZerosLst -> Pt[1];
        MvarPtFreeList(PtZerosLst);
    }
    else {
        /* If we got here there was a numeric error, we force the VMid       */
        /* parameter to be (VMax+VMin)/2 and let MvarComputeInterMidPoint    */
        /* coerce it.							     */
#       ifdef DEBUG_VORONOI     
	printf("PtZerosLst is NULL.\n"); 
#       endif 
        VMid = (VMin1 + VMax1) / 2.0;    
    }

    /* Compute P(t,r). */   
    R3 = CagdCrvEval(Crv1, UMid); 	   
    CagdCoerceToE3(Pt1, &R3, -1, Crv1 -> PType);      
    R3 = CagdCrvEval(Crv2, VMid);      
    CagdCoerceToE3(Pt2, &R3, -1, Crv2 -> PType);

    /* CagdCrvNormalXY should give the `right' normal. */      
    Vec = CagdCrvNormalXY(Crv1, UMid, TRUE);   
    IRIT_PT_COPY(Nrml1, Vec -> Vec);     
    Vec = CagdCrvNormalXY(Crv2, VMid, TRUE);   
    IRIT_PT_COPY(Nrml2, Vec -> Vec);
       
    GMVecCrossProd(Vtemp, Nrml1, Nrml2);

    /* Check to see if they are parallel. */      
    if (GMVecLength(Vtemp) < IRIT_UEPS) {     
 	BP[0] = (Pt1[0] + Pt2[0]) / 2.0;     
	BP[1] = (Pt1[1] + Pt2[1]) / 2.0;   
    }   
    else { 	  
	R2 = MvarComputeInterMidPoint(Crv1, UMid, Crv2, VMid); 	  
	BP[0] = R2[0]; 	  
	BP[1] = R2[1];   
    }

    /* Compute the dot product < P - C1, N1> and < P - C2, N2>. */
    DotProd[0] = (BP[0] - Pt1[0]) * (-Nrml1[0]) +
                 (BP[1] - Pt1[1]) * (-Nrml1[1]); 
    DotProd[1] = (BP[0] - Pt2[0]) * (-Nrml2[0]) +
                 (BP[1] - Pt2[1]) * (-Nrml2[1]); 
   
    return DotProd[0] >= 0.0 && DotProd[1] >= 0.0;
}  

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point is  to   M
*   the left of the curve or not.					     M
*   Compute the constraints as dot products                                  M
*   1st constraint <P(t,r) -  C1(t), N1(t)>  >  0                            M
*   2nd constraint <P(t,r) -  C2(r), N2(r)>  >  0                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv1:     VoronoiCrvStruct                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVoronoiCrvStruct *: Returns the VoronoiCrvStruct after appying       M
*   the LL constaint							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctIsCurveLL                                                        M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctApplyLL, bisectors                                               M
*****************************************************************************/
MvarVoronoiCrvStruct *MvarBsctApplyLL(MvarVoronoiCrvStruct *Cv1) 
{   
    int IsCurveLL;
    MvarConstraintType Constraints[2];
    MvarMVStruct *MVs[2];
    CagdBBoxStruct Bbox;
    CagdRType UMin, UMax, VMin, VMax;
    CagdSrfStruct *Denom, *TrimmedSrf;
    MvarPtStruct *Pts, *Head1, *Head2, *Tmp;
    MvarVoronoiCrvStruct *SplitCrvsTmp, *CvCpy2, *CvCpy,
        *SplitCrvs = NULL, 
        *LLCrvs = NULL,
        *Cv = Cv1,
        *CvLeft = NULL, 
        *CvRight = NULL;
        
    CagdSrfDomain(Cv -> F3, &UMin, &UMax, &VMin, &VMax);
    
    Bbox.Min[0] = UMin;
    Bbox.Min[1] = VMin;
    Bbox.Max[0] = UMax;
    Bbox.Max[1] = VMax;

    MVs[0] = MvarSrfToMV(Cv -> F3);
 
    MvarBsctComputeDenomOfP(Cv -> Crv1, Cv -> Crv2, &Denom);
    TrimmedSrf = MvarBsctTrimSurfaceByUVBbox(Denom, Bbox);
    CagdSrfFree(Denom);
    
    MVs[1] = MvarSrfToMV(TrimmedSrf);
    CagdSrfFree(TrimmedSrf);
   
    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
    Pts = MvarMVsZeros(MVs, Constraints, 2, MvarBsctSubdivTol, 
		       MVAR_BSCT_NUMER_TOL);

    /* There is a problem of double values, we sort the points and  */
    /* purge away of double values.                                 */
    Pts = MvarPtSortListAxis(Pts, 1);

    if (Pts != NULL) {
        Head1 = Pts;
	Head2 = Pts -> Pnext;
	while(Head2 != NULL) {
	    if (IRIT_APX_EQ_EPS(Head1 -> Pt[0], Head2 -> Pt[0], 
			   IRIT_FABS(10.0 * MVAR_BSCT_NUMER_TOL)) &&
		IRIT_APX_EQ_EPS(Head1 -> Pt[1], Head2 -> Pt[1], 
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

    MvarMVFree(MVs[0]);
    MvarMVFree(MVs[1]);

    /* Splitting at Denom == 0 points. */   
    CvCpy = MvarVoronoiCrvCopy(Cv1);
    for (Head1 = Pts; Head1 != NULL; Head1 = Head1 -> Pnext) {
        if (Head1 -> Pt[0] - UMin > 10.0 * MVAR_BSCT_NUMER_TOL && 
            UMax - Head1 -> Pt[0] > 10.0 * MVAR_BSCT_NUMER_TOL) {
            MvarBsctSplitCurve(CvCpy, Head1, &CvLeft, &CvRight);
    	
	    /* Free CvCpy. */
	    MvarVoronoiCrvFree(CvCpy);
	    CvCpy = CvRight;
	    IRIT_LIST_PUSH(CvLeft, SplitCrvs);
        }
    }
    IRIT_LIST_PUSH(CvCpy, SplitCrvs);
    MvarPtFreeList(Pts);

    for (SplitCrvsTmp = SplitCrvs; SplitCrvsTmp != NULL; 
	 SplitCrvsTmp = SplitCrvsTmp -> Pnext) {   	  
        IsCurveLL = MvarBsctIsCurveLL(SplitCrvsTmp); 
	if (IsCurveLL) {       
	    CvCpy2 = MvarVoronoiCrvCopy(SplitCrvsTmp);
	    IRIT_LIST_PUSH(CvCpy2, LLCrvs);     
	}     
	else {  
#           ifdef DEBUG_VORONOI     
	    printf("not LL curve\n"); 
#           endif 
	}   
    }

    /* Free SplitCrvs .*/
    MvarVoronoiCrvFreeList(SplitCrvs);

    return LLCrvs; 
}  

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point satisfiesM
*   the the following curvature constraints or not.     		     M
*   Calculating the curvature constraints				     M
*   1st constraint <P(t,r) -  C1(t), k(t)N1(t)> - 1 <  0		     M
*   2nd constraint <P(t,r) -  C2(r), k(r)N2(r)> - 1 <  0	             M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv1:     VoronoiCrvStruct                                                M
*   CCFreeCrvs:    VoronoiCrvStruct for storing the resultant curves         M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    TRUE or FALSE.                                                   M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctApplyLL                                                          M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctApplyCC, bisectors                                               M
*****************************************************************************/
int MvarBsctApplyCC(MvarVoronoiCrvStruct *Cv1, 
		    MvarVoronoiCrvStruct **CCFreeCrvs) 
{ 
    CagdRType UMin1, UMax1, VMin1, VMax1, UMid, VMid, *R2, *R3, *R4; 	
    CagdPType Pt1, Pt2, BP, Nrml1, Nrml2, DotProd, kN1, kN2; 	
    CagdVecStruct *Vec; 	
    IrtPtType Vtemp; 	
    MvarVoronoiCrvStruct *CvCpy1,
        *Cv = Cv1;
    CagdSrfStruct 
        *MonotoneF3Srf = Cv -> F3;   
    CagdCrvStruct *NkCrv1, *NkCrv2,
        *Crv1 = Cv -> Crv1,
        *Crv2 = Cv -> Crv2;   
    MvarPtStruct *PtZerosLst;   
        
    CagdSrfDomain(MonotoneF3Srf, &UMin1, &UMax1, &VMin1, &VMax1);

    /* Compute mid paramater. */ 	
    UMid = (UMin1 + UMax1) / 2;
    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(MonotoneF3Srf, UMid,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MVAR_BSCT_NUMER_TOL),
						    TRUE);

    if (PtZerosLst != NULL) {
        if (PtZerosLst -> Pnext != NULL) {
	    /* Endcase we do not handle yet. */
            MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);
	}

        if (PtZerosLst -> Pt[0] != UMid)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);        /* Sanity check. */

        VMid = PtZerosLst -> Pt[1];
        MvarPtFreeList(PtZerosLst);
    }
    else {
        /* If we got here there was a numeric error, we force the VMid       */
        /* parameter to be (VMax+VMin)/2 and let MvarComputeInterMidPoint    */
        /* coerce it.							     */
#       ifdef DEBUG_VORONOI     
	printf("PtZerosLst is NULL.\n"); 
#       endif 
        VMid = (VMin1 + VMax1) / 2.0;    
    }


    /* Computing the kN curve from curvatur.c and evaluating at t. */       
    NkCrv1 = SymbCrv3DCurvatureNormal(Crv1); 	   
    R4 = CagdCrvEval(NkCrv1, UMid); 	   
    CagdCoerceToE3(kN1, &R4, -1, NkCrv1 -> PType);  	
    NkCrv2 = SymbCrv3DCurvatureNormal(Crv2);      
    R4 = CagdCrvEval(NkCrv2, VMid);      
    CagdCoerceToE3(kN2, &R4, -1, NkCrv2 -> PType);

    CagdCrvFree(NkCrv1);
    CagdCrvFree(NkCrv2);

    /* Compute P(t,r). */
    R3 = CagdCrvEval(Crv1, UMid); 	   
    CagdCoerceToE3(Pt1, &R3, -1, Crv1 -> PType);      
    R3 = CagdCrvEval(Crv2, VMid);      
    CagdCoerceToE3(Pt2, &R3, -1, Crv2 -> PType);             
    Vec = CagdCrvNormalXY(Crv1, UMid, TRUE);      
    IRIT_PT_COPY(Nrml1, Vec -> Vec);      
    Vec = CagdCrvNormalXY(Crv2, VMid, TRUE);      
    IRIT_PT_COPY(Nrml2, Vec -> Vec);              
	
    GMVecCrossProd(Vtemp, Nrml1, Nrml2);

    /* Check to see if they are parallel. */      
    if (GMVecLength(Vtemp) < IRIT_UEPS) { 	     
        BP[0] = (Pt1[0] + Pt2[0]) / 2.0; 	     
	BP[1] = (Pt1[1] + Pt2[1]) / 2.0;      
    }      
    else { 	     
        R2 = MvarComputeInterMidPoint(Crv1, UMid, Crv2, VMid); 	     
	BP[0] = R2[0]; 	     
	BP[1] = R2[1];      
    }

    /*	Compute the dot product < P - C1, kN1> and < P - C2, N2> 	 */
    /*  and evaluate the curvature constraint. 	                         */
    DotProd[0] = ((BP[0] - Pt1[0])*kN1[0] + (BP[1] - Pt1[1])*kN1[1]) - 1; 
    DotProd[1] = ((BP[0] - Pt2[0])*kN2[0] + (BP[1] - Pt2[1])*kN2[1]) - 1; 
    if(DotProd[0] <= 0.0 && DotProd[1] <= 0.0){ 		
        CvCpy1 = MvarVoronoiCrvCopy(Cv1);
	IRIT_LIST_PUSH(CvCpy1, (*CCFreeCrvs));
	
	/* In the future if there is a possibility of several CCFreeCurves...*/
	return 1; 	
    } 	
    else{         
        return 0; 	
    } 
}  

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the struct, this function says whether the bisector point satisfiesM
*   the LL and Curvature constraints					     M
*                                                                            *
* PARAMETERS:                                                                M
*   InputCrvs:  A VoronoiCrvStruct of monotone pieces                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVoronoiCrvStruct *: VoronoiCrvStruct for storing the resultant       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctApplyLL, MvarBsctApplyCC                                         M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctPurgeAwayLLAndCCConstraints, bisectors                           M
*****************************************************************************/
MvarVoronoiCrvStruct *MvarBsctPurgeAwayLLAndCCConstraints(MvarVoronoiCrvStruct
							           *InputCrvs) 
{
    /* LL check. */   
    MvarVoronoiCrvStruct *LLCrvs, *LLCrvsTmp,
        *InputCrvsIter = InputCrvs, 
        *CCFinal = NULL;   
    int IsCCFree;

    while (InputCrvsIter != NULL) { 	  
        /* Call LL function. */ 	  
        LLCrvs = MvarBsctApplyLL(InputCrvsIter); 	  
	/* Call CC function. */
	LLCrvsTmp = LLCrvs; 	  
	while (LLCrvsTmp != NULL) {
	    IsCCFree = MvarBsctApplyCC(LLCrvsTmp, &CCFinal); 
	    LLCrvsTmp = LLCrvsTmp -> Pnext; 	  
	}  	

        /* Free LLCrvs list.*/
        MvarVoronoiCrvFreeList(LLCrvs);

	InputCrvsIter = InputCrvsIter -> Pnext;   
    }   
    return CCFinal; 
}

