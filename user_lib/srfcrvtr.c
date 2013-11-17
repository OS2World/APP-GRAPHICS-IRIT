/******************************************************************************
* SrfCrvtr.c - curvature computation of curves and surfaces.		      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Gershon Elber, June 03.					      *
******************************************************************************/

#include "triv_lib.h"
#include "mvar_lib.h"
#include "user_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Computes the umbilical points of a given surface.                          M
*   The umbilicals are computed as the zeros of the function C = H^2 - K.    M
* C is never nagative and hence we actually solve for dC/du = dC/dv = 0 and  M
* test the values of C there.						     M
*   Hence (We only consider numerator of C which is sufficient for zeros),   M
*      C = H^2 - K = (2FM - EN - GL)^2 - 4(LN - M^2)(EG - F^2).		     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:       Surface to compute its umbilical points, if any.              M
*   SubTol:    Subdivision tolerance of computation.			     M
*   NumTol:    Numerical tolerance of computation.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   A list of UV parameters of umbilical points, or NULL   M
*		      if none.						     M
*                                                                            *
* SEE ALSO:                                                                  M
*   SymbSrfFff, SymbSrfSff, SymbSrfMeanCurvatureSqr, SymbSrfGaussCurvature,  M
*   MvarMVsZeros							     M
*                                                                            *
* KEYWORDS:                                                                  M
*   UserSrfUmbilicalPts, curvature                                           M
*****************************************************************************/
MvarPtStruct *UserSrfUmbilicalPts(const CagdSrfStruct *Srf,
				  CagdRType SubTol,
				  CagdRType NumTol)
{
    MvarConstraintType 
    	Constraints[2] = {
	    MVAR_CNSTRNT_ZERO,
	    MVAR_CNSTRNT_ZERO
	};
    CagdSrfStruct *DuSrf, *DvSrf, *SNormal, *STmp1, *STmp2, *STmp3, *STmp4,
	*HNumer, *FffDeterminant, *SffDeterminant, *CSrf,
	*FffE, *FffF, *FffG, *SffL, *SffM, *SffN;
    MvarPtStruct *MVPts, *MVPt, *MVPtTmp,
	*RetList = NULL;
    MvarMVStruct *Finals[2];

    SymbSrfFff(Srf, &DuSrf, &DvSrf, &FffE, &FffF, &FffG);
    SymbSrfSff(DuSrf, DvSrf, &SffL, &SffM, &SffN, &SNormal);
    CagdSrfFree(DuSrf);
    CagdSrfFree(DvSrf);
    CagdSrfFree(SNormal);

    FffDeterminant = SymbSrfDeterminant2(FffE, FffF, FffF, FffG);
    SffDeterminant = SymbSrfDeterminant2(SffL, SffM, SffM, SffN);

    STmp1 = SymbSrfMult(FffE, SffN);
    STmp2 = SymbSrfMult(FffG, SffL);
    STmp3 = SymbSrfMult(FffF, SffM);
    STmp4 = SymbSrfScalarScale(STmp3, 2.0);
    CagdSrfFree(STmp3);
    STmp3 = SymbSrfAdd(STmp1, STmp2);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp2);
    HNumer = SymbSrfSub(STmp3, STmp4);
    CagdSrfFree(STmp3);
    CagdSrfFree(STmp4);

    CagdSrfFree(FffE);
    CagdSrfFree(FffF);
    CagdSrfFree(FffG);
    CagdSrfFree(SffL);
    CagdSrfFree(SffM);
    CagdSrfFree(SffN);

    /* Combine the numerator of H and Fff and Sff together. */
    STmp1 = SymbSrfMult(HNumer, HNumer);
    CagdSrfFree(HNumer);

    STmp2 = SymbSrfMult(SffDeterminant, FffDeterminant);
    CagdSrfFree(FffDeterminant);
    CagdSrfFree(SffDeterminant);
    STmp3 = SymbSrfScalarScale(STmp2, 4.0);
    CagdSrfFree(STmp2);

    CSrf = SymbSrfSub(STmp1, STmp3);
    CagdSrfFree(STmp1);
    CagdSrfFree(STmp3);

    /* Construct the two derivatives of the C function, now that we have it. */
    STmp1 = CagdSrfDerive(CSrf, CAGD_CONST_U_DIR);
    STmp2 = CagdSrfDerive(CSrf, CAGD_CONST_V_DIR);

    Finals[0] = MvarSrfToMV(STmp1);
    Finals[1] = MvarSrfToMV(STmp2);
	
    MVPts = MvarMVsZeros(Finals, Constraints, 2, SubTol, NumTol);

    MvarMVFree(Finals[0]);
    MvarMVFree(Finals[1]);

    /* Evaluate all solution points and pick those that has zero CSrf value. */
    while (MVPts != NULL) {
        CagdRType *R;

        MVPt = MVPts;
        MVPts = MVPts -> Pnext;
	MVPt -> Pnext = NULL;
        
	R = CagdSrfEval(CSrf, MVPt -> Pt[0], MVPt -> Pt[1]);
	if ((CAGD_IS_RATIONAL_SRF(CSrf) ? R[1] / R[0] : R[1]) < NumTol) {
	    for (MVPtTmp = RetList; MVPtTmp != NULL; MVPtTmp = MVPtTmp -> Pnext) {
		if (IRIT_PT_APX_EQ_E2_EPS(MVPt -> Pt, MVPtTmp -> Pt, NumTol))
		    break;
	    }
	    if (MVPtTmp == NULL) {
		IRIT_LIST_PUSH(MVPt, RetList);
	    }
	    else
	        MvarPtFree(MVPt);
	}
	else
	    MvarPtFree(MVPt);
    }

    return RetList;
}
