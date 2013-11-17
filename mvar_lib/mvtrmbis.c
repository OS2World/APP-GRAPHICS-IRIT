/******************************************************************************
* mvtrmbis.c - Compute F3 and its denominator.			              *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Hanniel, March 2005.					      *
******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "geom_lib.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "mvar_loc.h"

#define MAX_NUMER_ITER     3		  /* Number of numerical iterations. */

/* General Tolerance defintions. */
IRIT_GLOBAL_DATA CagdRType 
    MvarBsctSubdivTol = 0.001,
    MvarBsctNumerTol = 1.0e-6,
    MvarBsctUVTol = 10.0 * 1.0e-6;
   
/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the denominator of the bisector surface F3 of two given curves. M
*   Solve for the normal intersection surface in the plane and then	     M
*   substitute into (the bisector's correspondance is the zero set then).    M
*	      C1(s) + C2(t)						     V
*	< P - -------------, C1(t) - C2(s) > = 0.			     V
*		    2							     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Inp, Crv2Inp: Two curves to compute bisectors for.  Assumes          M
*		      E2 curves.					     M
*   DenomOut: The resulting denominator surface is stored here.              M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*									     *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectors, SymbCrvBisectorsSrf2,M
*   SymbCrvBisectorsSrf, SymbCrvPtBisectorsSrf3D, SymbCrvCrvBisectorSrf3D.   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctComputeDenomOfP, bisectors, skeleton                             M
*****************************************************************************/
void MvarBsctComputeDenomOfP(CagdCrvStruct *Crv1Inp,
			     CagdCrvStruct *Crv2Inp,
			     CagdSrfStruct **DenomOut)
{
    CagdCrvStruct *Crv1, *Crv2, *DCrv1, *DCrv2;
    CagdSrfStruct *DSrf1, *DSrf2, *Denom, *DSrf1X, *DSrf1Y, *DSrf2X, *DSrf2Y,
        *SrfAux1, *SrfAux2;
    
    Crv1 = Crv1Inp;
    Crv2 = Crv2Inp;

    if (CAGD_IS_BEZIER_CRV(Crv1)) {
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }
    if (CAGD_IS_BEZIER_CRV(Crv2)) {
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv2);

	CagdCrvFree(Crv2);
	Crv2 = TCrv;
    }

    DCrv1 = CagdCrvDerive(Crv1);
    DCrv2 = CagdCrvDerive(Crv2);

    DSrf1 = CagdPromoteCrvToSrf(DCrv1, CAGD_CONST_U_DIR);
    DSrf2 = CagdPromoteCrvToSrf(DCrv2, CAGD_CONST_V_DIR);

    CagdCrvFree(DCrv1);
    CagdCrvFree(DCrv2);

    SymbSrfSplitScalar(DSrf1, &SrfAux1, &DSrf1X, &DSrf1Y, &SrfAux2);
    CagdSrfFree(DSrf1);
    CagdSrfFree(SrfAux1);
    CagdSrfFree(SrfAux2);

    SymbSrfSplitScalar(DSrf2, &SrfAux1, &DSrf2X, &DSrf2Y, &SrfAux2);
    CagdSrfFree(DSrf2);
    CagdSrfFree(SrfAux1);
    CagdSrfFree(SrfAux2);

    Denom = SymbSrfDeterminant2(DSrf1X, DSrf1Y, DSrf2X, DSrf2Y);
    CagdSrfFree(DSrf1X);
    CagdSrfFree(DSrf1Y);
    CagdSrfFree(DSrf2X);
    CagdSrfFree(DSrf2Y);


    (*DenomOut) = Denom;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector surface definition of two curves.                  M
*   The result is a scalar surface whose zero set is the set of              M
*   bisector(s) of the curves.						     M
*   Solve for the normal intersection surface in the plane and then	     M
* substitute into (the bisector's correspondance is the zero set then).      M
*	      C1(s) + C2(t)						     V
*	< P - -------------, C1(t) - C2(s) > = 0.			     V
*		    2							     V
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1Inp, Crv2Inp: Two curves to compute bisectors for.  Assumes          M
*		      E2 curves.					     M
*   Crv1Coerced: N.S.F.I.                                                    M
*   Crv2Coerced: N.S.F.I.                                                    M
*   F3: 	 The resulting bisector surface is stored here.              M
*   L1:  	 N.S.F.I.                                                    M
*   L2:  	 N.S.F.I.                                                    M
*   CC1: 	 N.S.F.I.                                                    M
*   CC2: 	 N.S.F.I.                                                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void				                                     M
*								             *
* SEE ALSO:                                                                  M
*   SymbCrvCnvxHull, SymbCrvDiameter, SymbCrvBisectors, SymbCrvBisectorsSrf2,M
*   SymbCrvBisectorsSrf, SymbCrvPtBisectorsSrf3D, SymbCrvCrvBisectorSrf3D    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctComputeF3, bisectors, skeleton				     M
*****************************************************************************/
void MvarBsctComputeF3(CagdCrvStruct *Crv1Inp,
		       CagdCrvStruct *Crv2Inp,
		       CagdCrvStruct **Crv1Coerced,
		       CagdCrvStruct **Crv2Coerced,
		       CagdSrfStruct **F3,
		       CagdSrfStruct **L1,
		       CagdSrfStruct **L2,
		       CagdSrfStruct **CC1,
		       CagdSrfStruct **CC2)
{
    IRIT_STATIC_DATA CagdVType 
        Scale = { 0.5, 0.5, 0.5 };
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *Crv1, *Crv2, *DCrv1, *DCrv2, *NCrv1, *NCrv2, *NkCrv1, 
        *NkCrv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2, *NSrf1, *NSrf2, *Res, *Denom, 
        *Denom1, *NumerX, *NumerY, *DSrf1X, 
        *DSrf1Y, *DSrf2X, *DSrf2Y, *TSrf1, *TSrf2, *TSrf3, *TSrf4, *TSrf5, 
        *TSrf6, *TSrf7, *TSrf8, *TSrf9, *NkSrf1, *NkSrf2;
    
    Crv1 = CagdCrvCopy(Crv1Inp);
    Crv2 = CagdCrvCopy(Crv2Inp);

    if (CAGD_IS_BEZIER_CRV(Crv1)) {
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv1);

	CagdCrvFree(Crv1);
	Crv1 = TCrv;
    }
    if (CAGD_IS_BEZIER_CRV(Crv2)) {
        CagdCrvStruct
	    *TCrv = CagdCnvrtBzr2BspCrv(Crv2);

	CagdCrvFree(Crv2);
	Crv2 = TCrv;
    }

    DCrv1 = CagdCrvDerive(Crv1);
    DCrv2 = CagdCrvDerive(Crv2);

    /* Computing the normal from curvatur.c.  */

    NCrv1 = SymbCrv2DUnnormNormal(Crv1);
    NCrv2 = SymbCrv2DUnnormNormal(Crv2);

    /* Converting the computed normal to the left normal if required. */

    /* Computing the LL constraint. */

    CagdCrvTransform(NCrv1, NULL, -1);
    CagdCrvTransform(NCrv2, NULL, -1);

    /* Computing the kN curve from curvatur.c. */

    NkCrv1 = SymbCrv3DCurvatureNormal(Crv1);
    NkCrv2 = SymbCrv3DCurvatureNormal(Crv2);

    Srf1 = CagdPromoteCrvToSrf(Crv1, CAGD_CONST_U_DIR);
    Srf2 = CagdPromoteCrvToSrf(Crv2, CAGD_CONST_V_DIR);

    CagdSrfDomain(Srf1, &UMin1, &UMax1, &VMin1, &VMax1);
    CagdSrfDomain(Srf2, &UMin2, &UMax2, &VMin2, &VMax2);
    BspKnotAffineTrans2(Srf1 -> VKnotVector, Srf1 -> VLength + Srf1 -> VOrder,
			VMin2, VMax2);
    BspKnotAffineTrans2(Srf2 -> UKnotVector, Srf2 -> ULength + Srf2 -> UOrder,
			UMin1, UMax1);

    DSrf1 = CagdPromoteCrvToSrf(DCrv1, CAGD_CONST_U_DIR);
    DSrf2 = CagdPromoteCrvToSrf(DCrv2, CAGD_CONST_V_DIR);
    CagdCrvFree(DCrv1);
    CagdCrvFree(DCrv2);

    NSrf1 = CagdPromoteCrvToSrf(NCrv1, CAGD_CONST_U_DIR);
    NSrf2 = CagdPromoteCrvToSrf(NCrv2, CAGD_CONST_V_DIR);
    CagdCrvFree(NCrv1);
    CagdCrvFree(NCrv2);

    NkSrf1 = CagdPromoteCrvToSrf(NkCrv1, CAGD_CONST_U_DIR);
    NkSrf2 = CagdPromoteCrvToSrf(NkCrv2, CAGD_CONST_V_DIR);
    CagdCrvFree(NkCrv1);
    CagdCrvFree(NkCrv2);
    
    SymbSrfSplitScalar(DSrf1, &TSrf1, &DSrf1X, &DSrf1Y, &TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    SymbSrfSplitScalar(DSrf2, &TSrf1, &DSrf2X, &DSrf2Y, &TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    Denom = SymbSrfDeterminant2(DSrf1X, DSrf1Y, DSrf2X, DSrf2Y);
    Denom1 = CagdSrfCopy(Denom);
    TSrf1 = SymbSrfDotProd(Srf1, DSrf1);
    TSrf2 = SymbSrfDotProd(Srf2, DSrf2);
    NumerX = SymbSrfDeterminant2(TSrf1, DSrf1Y, TSrf2, DSrf2Y);
    NumerY = SymbSrfDeterminant2(DSrf1X, TSrf1, DSrf2X, TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    CagdSrfFree(DSrf1);
    CagdSrfFree(DSrf1X);
    CagdSrfFree(DSrf1Y);
    CagdSrfFree(DSrf2);
    CagdSrfFree(DSrf2X);
    CagdSrfFree(DSrf2Y);

    /* Compute:								     */
    /*	      C1(s) + C2(t)	                                             */
    /*	< P - -------------, C1(t) - C2(s) > = 0.                            */
    /*		    2		                                             */

    TSrf1 = SymbSrfAdd(Srf1, Srf2);
    CagdSrfScale(TSrf1, Scale);
    TSrf2 = SymbSrfMultScalar(TSrf1, Denom);
    CagdSrfFree(TSrf1);
    TSrf3 = SymbSrfMergeScalar(NULL, NumerX, NumerY, NULL);
    CagdSrfFree(NumerX);
    CagdSrfFree(NumerY);

    TSrf1 = SymbSrfSub(TSrf3, TSrf2);

    CagdSrfFree(TSrf2);

    TSrf2 = SymbSrfSub(Srf1, Srf2);
    Res = SymbSrfDotProd(TSrf1, TSrf2); /* The F3 surface. */
    
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    /* Compute the constraints as dot products                               */
    /* 1st constraint is <P(t,r) -  C1(t), N1(t)>  >  0                      */
    /*  = (< X(t,r) - Y(t,r)C1(t), N1(t)>) / Y(t,r) >  0                     */
    /* 2nd constraint is <P(t,r) -  C2(r), N2(r)>  >  0                      */
    /*  = (< X(t,r) - Y(t,r)C2(r), N2(r)>) / Y(t,r) >  0                     */

    TSrf1 = SymbSrfMultScalar(Srf1, Denom);
    TSrf2 = SymbSrfMultScalar(Srf2, Denom);
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);

    TSrf4 = SymbSrfSub(TSrf3, TSrf1);
    TSrf5 = SymbSrfSub(TSrf3, TSrf2);

    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf3);

    TSrf1 = SymbSrfDotProd(TSrf4, NSrf1);
    TSrf2 = SymbSrfDotProd(TSrf5, NSrf2);
    CagdSrfFree(NSrf1);
    CagdSrfFree(NSrf2);


    /*  Making the surfaces compatible.   */

    CagdMakeSrfsCompatible(&Denom, &TSrf1, TRUE, TRUE, TRUE, TRUE);
    TSrf6 = SymbSrfMergeScalar(Denom, TSrf1, NULL, NULL);
    CagdMakeSrfsCompatible(&Denom, &TSrf2, TRUE, TRUE, TRUE, TRUE);
    TSrf7 = SymbSrfMergeScalar(Denom, TSrf2, NULL, NULL);

    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    /* Calculating the curvature constraints.                                */
    /* 1st constraint <P(t,r) -  C1(t), k(t)N1(t)> - 1 <  0                  */
    /*  = < X(t,r)/Y(t,r) - C1(t), k(t)N1(t)> - 1 <  0                       */
    /*  = (< X(t,r) - Y(t,r)C1(t), k(t)N1(t)> - Y(t,r)) / Y(t,r) <  0        */
    /* 2nd constraint <P(t,r) -  C2(r), k(r)N2(r)> - 1 <  0                  */
    /* 2nd constraint is also deduced similarly                              */

    CagdMakeSrfsCompatible(&TSrf4, &NkSrf1, TRUE, TRUE, TRUE, TRUE);
    CagdMakeSrfsCompatible(&TSrf5, &NkSrf2, TRUE, TRUE, TRUE, TRUE);
    TSrf8 = SymbSrfDotProd(TSrf4, NkSrf1);
    TSrf9 = SymbSrfDotProd(TSrf5, NkSrf2);
    
    CagdSrfFree(NkSrf1);
    CagdSrfFree(NkSrf2);
    CagdSrfFree(TSrf5);
    CagdSrfFree(TSrf4);
            
    CagdMakeSrfsCompatible(&Denom1, &TSrf8, TRUE, TRUE, TRUE, TRUE);
    TSrf1 = SymbSrfSub(TSrf8, Denom1);
    TSrf4 = SymbSrfMergeScalar(Denom1, TSrf1, NULL, NULL);
    CagdMakeSrfsCompatible(&Denom1, &TSrf9, TRUE, TRUE, TRUE, TRUE);
    TSrf2 = SymbSrfSub(TSrf9, Denom1);
    TSrf5 = SymbSrfMergeScalar(Denom1, TSrf2, NULL, NULL);
      
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    CagdSrfFree(TSrf8);
    CagdSrfFree(TSrf9);
    CagdSrfFree(Denom1);
    CagdSrfFree(Denom);

    /* Assigning return values. */

    (*Crv1Coerced) = Crv1; 
    (*Crv2Coerced) = Crv2; 
    (*F3) = Res;
    (*L1) = TSrf6;
    (*L2) = TSrf7;
    (*CC1) = TSrf4;
    (*CC2) = TSrf5;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the intersection point of the normals of the given two points   M
* on the given two curves. Taken from crvskel.c - need to modify to avoid    M
* artifacts in near parallel normals.				             M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:      First curve of the matching mid point.                        M
*   t1:        Parameter value of first curve's mid point.                   M
*   Crv2:      Second curve of the matching mid point.                       M
*   t2:        Parameter value of second curve's mid point.                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Point of intersection, statically allocated.              M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarComputeInterMidPoint                                                 M
*****************************************************************************/
CagdRType *MvarComputeInterMidPoint(CagdCrvStruct *Crv1,
				    CagdRType t1,
				    CagdCrvStruct *Crv2,
				    CagdRType t2)
{
    IRIT_STATIC_DATA CagdPType Inter1;
    CagdPType Pt1, Pt2, Nrml1, Nrml2, Inter2;
    CagdRType *R;
    CagdVecStruct *Vec;

    R = CagdCrvEval(Crv1, t1);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, t2);
    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

    Vec = CagdCrvNormalXY(Crv1, t1, TRUE);
    IRIT_PT_COPY(Nrml1, Vec -> Vec);
    Vec = CagdCrvNormalXY(Crv2, t2, TRUE);
    IRIT_PT_COPY(Nrml2, Vec -> Vec);

    GM2PointsFromLineLine(Pt1, Nrml1, Pt2, Nrml2, Inter1, &t1, Inter2, &t2);
    return Inter1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Numerically march along curve 1 in order to improve the result of the    *
* mid point computation, using Newton Raphson.			             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1:      First curve of the matching mid point.                        *
*   t1:        Parameter value of first curve's mid point.                   *
*   Crv2       Second curve of the matching mid point.                       *
*   t2:        Parameter value of second curve's mid point.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void       TRUE if succesfully improved the mid point, FALSE otherwise.  *
*****************************************************************************/
static int MvarNumerMarchMidPoint(CagdCrvStruct *Crv1,
				  CagdRType *t1,
				  CagdCrvStruct *Crv2,
				  CagdRType *t2)
{
    int i = 0,
        Improve = FALSE;
    CagdPType Pt1, Pt2;
    CagdRType Error, *R, TMin1, TMax1, NewError, DErrDt1, t1n;

    CagdCrvDomain(Crv1, &TMin1, &TMax1);

    R = CagdCrvEval(Crv1, *t1);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, *t2);
    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

    R = MvarComputeInterMidPoint(Crv1, *t1, Crv2, *t2);
    Error = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
		      IRIT_PT_PT_DIST_SQR(Pt2, R));

    do {
        t1n = *t1 + IRIT_EPS;
	if (t1n < TMin1 || t1n > TMax1)
	    break;

	R = CagdCrvEval(Crv1, t1n);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = MvarComputeInterMidPoint(Crv1, t1n, Crv2, *t2),
	NewError = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
			     IRIT_PT_PT_DIST_SQR(Pt2, R));
	
	/* Compute the change in error as a function of parameter change. */
	DErrDt1 = (Error - NewError) / IRIT_EPS;
	if (DErrDt1 == 0)
	    break;

	t1n = *t1 + Error / DErrDt1;
	if (t1n < TMin1 || t1n > TMax1)
	    break;

	R = CagdCrvEval(Crv1, t1n);
	CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
	R = MvarComputeInterMidPoint(Crv1, t1n, Crv2, *t2);
	NewError = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
			     IRIT_PT_PT_DIST_SQR(Pt2, R));

	if (NewError < Error) {
	    Error = NewError;
	    Improve = TRUE;
	    *t1 = t1n;
	}
    }
    while (i++ < MAX_NUMER_ITER && Improve);

    return Improve;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the bisector point given tr-values				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv1:     First curve.						     M
*   t:        Parameter value of first curve.				     M
*   Crv2:     Second curve.						     M
*   r:        Parameter value of second curve.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdRType *:   Bisector point from given tr.	                     M
*									     *
* KEYWORDS:                                                                  M
*   MvarBsctComputeXYFromBisTR	                                             M
*****************************************************************************/
CagdRType *MvarBsctComputeXYFromBisTR(CagdCrvStruct *Crv1,
				      CagdRType t,
				      CagdCrvStruct *Crv2,
				      CagdRType r)
{
    IRIT_STATIC_DATA 
        CagdPType Inter1;
    CagdPType Pt1, Pt2, Nrml1, Nrml2;
    CagdRType *R, Error,
        r_old = r, 
        r_new = r, 
        t_new = t;
    CagdVecStruct *Vec;
    CagdRType DotProd;

#ifdef DEBUG_VORONOI
    printf("make the r a pointer so we can make it an in/output parameter??, for now we do without... \n");
#endif

    if (MvarNumerMarchMidPoint(Crv2, &r_new, Crv1, &t_new)) {
        r = r_new;
    }
    else {
#	ifdef DEBUG_VORONOI
	    printf("numeric march failed\n");
#	endif
    }
    if (t_new != t)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR); /* Assert t was not touched.*/
		
    R = CagdCrvEval(Crv1, t);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, r);
    CagdCoerceToE3(Pt2, &R, -1, Crv1 -> PType);
    R = MvarComputeInterMidPoint(Crv1, t, Crv2, r);
    Error = IRIT_FABS(IRIT_PT_PT_DIST_SQR(Pt1, R) -
		      IRIT_PT_PT_DIST_SQR(Pt2, R));

    if (Error < 10.0 * MvarBsctNumerTol)
        return R;

    /* Else we have a large error - an "infinite" intersection. */
    r = r_old; /* In case we marched too much trying to improve. */
  
    R = CagdCrvEval(Crv1, t);
    CagdCoerceToE3(Pt1, &R, -1, Crv1 -> PType);
    R = CagdCrvEval(Crv2, r);
    CagdCoerceToE3(Pt2, &R, -1, Crv2 -> PType);

    Vec = CagdCrvNormalXY(Crv1, t, TRUE);
    IRIT_PT_COPY(Nrml1, Vec -> Vec);
    Vec = CagdCrvNormalXY(Crv2, r, TRUE);
    IRIT_PT_COPY(Nrml2, Vec -> Vec);

    /* A check whether Normals are in opposite directions (return the mid   */
    /* point). If they are in the same direction (i.e., <Nrml1,Nrml2> > 0), */
    /* return NULL for infinte point.				            */
    DotProd = (Nrml1[0]) * (Nrml2[0]) + (Nrml1[1]) * (Nrml2[1]);
    if (DotProd > 0)
        return NULL; /* Normals are in the same direction - infinite point. */

    /* Otherwise, Normals are parallel in opposite directions -		    */
    /* return mid point.						    */
    IRIT_PT_BLEND(Inter1, Pt1, Pt2, 0.5);	       /* Return mid point. */
    return Inter1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Formulate multivariate constraints for the points that are at equal      M
* distance from the three primitives and solve for them.  Assumes that the   M
* three primitives are lines or curves and that they do NOT intersect.       M
* Modified from from skel2d.c in mvar_lib to suit our needs                  M
*									     *
* PARAMETERS:                                                                M
*   Crv1Inp, Crv2Inp, Crv3Inp:   The three input primitives to consider.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:  A linked list of all equadistant points computed,       M
*		     or NULL if none found.				     M
*									     *
* KEYWORDS:                                                                  M
*   MvarBsctSkel2DEqPts3Crvs	                                             M
*****************************************************************************/
MvarPtStruct *MvarBsctSkel2DEqPts3Crvs(CagdCrvStruct *Crv1Inp,
				       CagdCrvStruct *Crv2Inp,
				       CagdCrvStruct *Crv3Inp)
{
    CagdRType TMin1, TMax1, TMin2, TMax2, TMin3, TMax3;
    CagdCrvStruct *DCrv, *Crv1, *Crv2, *Crv3;
    MvarMVStruct *MVCrv1, *MVCrv2, *MVCrv3, *MVTan1, *MVTan2, *MVTan3,
        *MVVec[MVAR_MAX_PT_SIZE], *MVA1Split[MVAR_MAX_PT_SIZE],**MVA2Split,
        *MVTmp1, *MVTmp2, *MVb1, *MVb2, *MVA1, *MVA2, *MVPDenom, *MVPNumer;
    MvarConstraintType Constraints[3];
    MvarPtStruct *MVPts, *MVPt;

    /* Make sure all curves are with domain zero to one.                     */
    /* Since we modify the curves, we copy them first.                       */
    Crv1 = CagdCrvCopy(Crv1Inp);
    Crv2 = CagdCrvCopy(Crv2Inp);
    Crv3 = CagdCrvCopy(Crv3Inp);

    CagdCrvDomain(Crv1, &TMin1, &TMax1);
    if (CAGD_IS_BSPLINE_CRV(Crv1))
        BspKnotAffineTransOrder2(Crv1 -> KnotVector, Crv1 -> Order,
				 Crv1 -> Length + Crv1 -> Order, 0.0, 1.0);
    CagdCrvDomain(Crv2, &TMin2, &TMax2);
    if (CAGD_IS_BSPLINE_CRV(Crv2))
        BspKnotAffineTransOrder2(Crv2 -> KnotVector, Crv2 -> Order,
				 Crv2 -> Length + Crv2 -> Order, 0.0, 1.0);
    CagdCrvDomain(Crv3, &TMin3, &TMax3);
    if (CAGD_IS_BSPLINE_CRV(Crv3))
        BspKnotAffineTransOrder2(Crv3 -> KnotVector, Crv3 -> Order,
				 Crv3 -> Length + Crv3 -> Order, 0.0, 1.0);

    /* Convert position curves. */
    MVTmp1 = MvarCrvToMV(Crv1);
    MVCrv1 = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv2);
    MVCrv2 = MvarPromoteMVToMV2(MVTmp1, 3, 1);
    MvarMVFree(MVTmp1);

    MVTmp1 = MvarCrvToMV(Crv3);
    MVCrv3 = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);

    /* Convert tangent curves. */
    DCrv = CagdCrvDerive(Crv1);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan1 = MvarPromoteMVToMV2(MVTmp1, 3, 0);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv2);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan2 = MvarPromoteMVToMV2(MVTmp1, 3, 1);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    DCrv = CagdCrvDerive(Crv3);
    MVTmp1 = MvarCrvToMV(DCrv);
    MVTan3 = MvarPromoteMVToMV2(MVTmp1, 3, 2);
    MvarMVFree(MVTmp1);
    CagdCrvFree(DCrv);

    /* Formulate distance constraints so that distance between Prim1 and     */
    /* Prim2 is similar and the distance between Prim1 and Prim3 is similar: */
    /*									     */
    /* < P - C1(u), P - C1(u) > = < P - C2(v), P - C2(v) >,		     */
    /* < P - C1(u), P - C1(u) > = < P - C3(w), P - C3(w) >,  or		     */
    /*									     */
    /* 2(C2(v) - C1(u)) P = C2(v)^2 - C1(u)^2,               		     */
    /* 2(C3(w) - C1(u)) P = C3(w)^2 - C1(u)^2, and solve for P = P(u, v, w). */
    MVA1 = MvarMVSub(MVCrv2, MVCrv1);
    MVTmp1 = MvarMVAdd(MVCrv2, MVCrv1);
    MVb1 = MvarMVDotProd(MVA1, MVTmp1);
    MvarMVTransform(MVA1, NULL, 2.0);
    MvarMVFree(MVTmp1);
  
    MVA2 = MvarMVSub(MVCrv3, MVCrv1);
    MVTmp1 = MvarMVAdd(MVCrv3, MVCrv1);
    MVb2 = MvarMVDotProd(MVA2, MVTmp1); 
    MvarMVTransform(MVA2, NULL, 2.0);
    MvarMVFree(MVTmp1);

    IRIT_GEN_COPY(MVA1Split, MvarMVSplitScalar(MVA1),
	     sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MVA2Split = MvarMVSplitScalar(MVA2);
    MvarMVFree(MVA1);
    MvarMVFree(MVA2);

    /* Solve for P = P(u, v, w). */
    IRIT_ZAP_MEM(MVVec, sizeof(MvarMVStruct *) * MVAR_MAX_PT_SIZE);
    MVPDenom = MvarMVDeterminant2(MVA1Split[1], MVA1Split[2],
				  MVA2Split[1], MVA2Split[2]);
    MVVec[1] = MvarMVDeterminant2(MVb1, MVA1Split[2],
				  MVb2, MVA2Split[2]);
    MVVec[2] = MvarMVDeterminant2(MVA1Split[1], MVb1,
				  MVA2Split[1], MVb2);
    MvarMVFree(MVA1Split[1]);
    MvarMVFree(MVA1Split[2]);
    if (MVA1Split[3] != NULL)
        MvarMVFree(MVA1Split[3]);

    MvarMVFree(MVA2Split[1]);
    MvarMVFree(MVA2Split[2]);
    if (MVA2Split[3] != NULL)
        MvarMVFree(MVA2Split[3]);

    MvarMVFree(MVb1);
    MvarMVFree(MVb2);

    MVPNumer = MvarMVMergeScalar(MVVec);
    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    /* Now formulate out the three following constraints with P's solution. */
    MVTmp1 = MvarMVMultScalar(MVCrv1, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[0] = MvarMVDotProd(MVTmp2, MVTan1);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMultScalar(MVCrv2, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[1] = MvarMVDotProd(MVTmp2, MVTan2);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MVTmp1 = MvarMVMultScalar(MVCrv3, MVPDenom);
    MVTmp2 = MvarMVSub(MVPNumer, MVTmp1);
    MVVec[2] = MvarMVDotProd(MVTmp2, MVTan3);
    MvarMVFree(MVPDenom);
    MvarMVFree(MVPNumer);
    MvarMVFree(MVTmp1);
    MvarMVFree(MVTmp2);

    MvarMVFree(MVCrv1);
    MvarMVFree(MVCrv2);
    MvarMVFree(MVCrv3);
    MvarMVFree(MVTan1);
    MvarMVFree(MVTan2);
    MvarMVFree(MVTan3);

    /* Invoke the zero set solver. */
    Constraints[0] = Constraints[1] = Constraints[2] = MVAR_CNSTRNT_ZERO;
  
    MVPts = MvarMVsZeros(MVVec, Constraints, 3, MvarBsctSubdivTol,
			 -IRIT_FABS(MvarBsctNumerTol));
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);
    MvarMVFree(MVVec[2]);

    /*Transforming back to the original domains of the curves.*/
    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        MVPt -> Pt[0] = TMin1 + (MVPt -> Pt[0])*(TMax1 - TMin1);
        MVPt -> Pt[1] = TMin2 + (MVPt -> Pt[1])*(TMax2 - TMin2);
        MVPt -> Pt[2] = TMin3 + (MVPt -> Pt[2])*(TMax3 - TMin3);
    }

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);
    CagdCrvFree(Crv3);

    return MVPts; 
}
