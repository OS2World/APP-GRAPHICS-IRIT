/******************************************************************************
* mvvorcrv.c -  VoronoiCrv functions and Geometric Primitive Functions.	      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Hanniel and Ramanathan Muthuganapathy, March 2005.	      *
******************************************************************************/

#include "geom_lib.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "mvar_loc.h"

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and resets all slots of a MvarVoronoiCrvStruct structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   None                                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVoronoiCrvStruct *:  A VoronoiCrv structure.			     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVoronoiCrvNew, allocation                                            M
*****************************************************************************/
MvarVoronoiCrvStruct *MvarVoronoiCrvNew(void)
{
    MvarVoronoiCrvStruct
        *NewVoronoiCrv = (MvarVoronoiCrvStruct *) 
            IritMalloc(sizeof(MvarVoronoiCrvStruct));

    NewVoronoiCrv -> Pnext = NULL;
    NewVoronoiCrv -> F3 = NULL;
    NewVoronoiCrv -> Crv1 = NULL;
    NewVoronoiCrv -> Crv2 = NULL;

    NewVoronoiCrv -> Type = MV_NONE;

    return NewVoronoiCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Allocates and copies all slots of a MvarVoronoiCrvStruct structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be copied.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVoronoiCrvStruct *:  A duplicate of Crv.                             M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVoronoiCrvCopy, copy                                                 M
*****************************************************************************/
MvarVoronoiCrvStruct *MvarVoronoiCrvCopy(MvarVoronoiCrvStruct *Crv)
{
    MvarVoronoiCrvStruct
        *NewCrv = (MvarVoronoiCrvStruct *)
            IritMalloc(sizeof(MvarVoronoiCrvStruct));
    
    if (Crv -> Type == MV_CV_CV) {
        NewCrv -> F3 = CagdSrfCopy(Crv -> F3);
        NewCrv -> Type = MV_CV_CV;
    }
    else {
        /* Snaity check.*/
        if (Crv -> Type != MV_CV_PT)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

        NewCrv -> F3 = NULL;
        NewCrv -> Type = MV_CV_PT;
        IRIT_PT_COPY(NewCrv -> Pt, Crv -> Pt);
    }

    NewCrv -> Crv1 = CagdCrvCopy(Crv -> Crv1);
    NewCrv -> Crv2 = CagdCrvCopy(Crv -> Crv2);

    NewCrv -> Pnext = NULL;
    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a VoronoiCrv structure.                                        M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:   Voronoi curve structure to free.	        		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVoronoiCrvFree, allocation                     	                     M
*****************************************************************************/
void MvarVoronoiCrvFree(MvarVoronoiCrvStruct *Crv)
{
    if (Crv -> F3)
        CagdSrfFree(Crv -> F3);
    if (Crv -> Crv1)
        CagdCrvFree(Crv -> Crv1);
    if (Crv -> Crv2)
        CagdCrvFree(Crv -> Crv2);
    IritFree(Crv); 
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Deallocates a VoronoiCrv list structure.                                   M
*                                                                            *
* PARAMETERS:                                                                M
*   CrvList:   Voronoi curve list to free.       		             M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVoronoiCrvFreeList, allocation                	                     M
*****************************************************************************/
void MvarVoronoiCrvFreeList(MvarVoronoiCrvStruct *CrvList)
{
    MvarVoronoiCrvStruct *CrvTemp;

    while (CrvList) {
	CrvTemp = CrvList -> Pnext;
	MvarVoronoiCrvFree(CrvList);
	CrvList = CrvTemp;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
* Reverses the t and r parameters of an MvarVoronoiCrvStruct structure.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Crv:       To be reversed.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarVoronoiCrvStruct *:  A single duplicate of Crv that is reversed.     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarVoronoiCrvReverse                                                    M
*****************************************************************************/
MvarVoronoiCrvStruct *MvarVoronoiCrvReverse(MvarVoronoiCrvStruct *Crv)
{
    MvarVoronoiCrvStruct
        *NewCrv = (MvarVoronoiCrvStruct *)
            IritMalloc(sizeof(MvarVoronoiCrvStruct));
   
    /* This function is only supported for CV_CV vorcrvs.*/
    if (Crv -> Type != MV_CV_CV)
        MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);

    NewCrv -> Type = Crv -> Type;

    NewCrv -> F3 = CagdSrfReverse2(Crv -> F3);
    NewCrv -> Crv1 = CagdCrvCopy(Crv -> Crv2);
    NewCrv -> Crv2 = CagdCrvCopy(Crv -> Crv1);

    NewCrv -> Pnext = NULL;
    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*	A Geometric Primitive Function that identifies which x-coord of the  M
*   two points P1 and P2 is smaller.					     M
*                                                                            *
* PARAMETERS:                                                                M
*   P1, P2: Points as input.                                                 M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: Returns true or false. 					     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctIsXSmaller				                             M
*****************************************************************************/
int MvarBsctIsXSmaller(MvarPtStruct *P1, MvarPtStruct *P2)
{
    /*Sanity checks.*/
    if (P1 == NULL || P2 == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    return P2 -> Pt[0] - P1 -> Pt[0] > 10.0 * MvarBsctNumerTol;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Functions for getting the Left and Right points of a curve              *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv:  N.S.F.I.                                                            *
*   Res: N.S.F.I.                                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBsctCCCurveLeft(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res)
{
    MvarPtStruct 
        *PtZerosLst = NULL;
    CagdRType UMin, UMax, VMin, VMax;

    /* Sanity check.*/
    if (Res == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    CagdSrfDomain(Cv -> F3, &UMin, &UMax, &VMin, &VMax);

    /* If we apply the convention that we have a corner-corner box,      */
    /* this can also be done by evaluating at UMin, VMin/VMax (currently */
    /* the split is not yet implemented that way).                       */
    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Cv -> F3, UMin,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol), 
						    TRUE);
#   ifdef DEBUG_VORONOI
    if (PtZerosLst -> Pnext != NULL) {
        printf("Left(): Found %d points for the same parameter (expected one)!\n", CagdListLength(PtZerosLst)); 
    }
#   endif /* DEBUG_VORONOI */
    Res -> Pt[0] = PtZerosLst->Pt[0];
    Res -> Pt[1] = PtZerosLst->Pt[1];

    /* Free PtZeroLst. */  
    MvarPtFreeList(PtZerosLst);
}

static void MvarBsctCPCurveLeft(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res)
{
    /* Since we are just interseted in the t-param we just construct a point with the t-param. */ 
    CagdRType UMin, UMax;
    CagdCrvDomain(Cv -> Crv2, &UMin, &UMax);

    /* Sanity check.*/
    if (Res == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    Res -> Pt[0] = UMin;
    Res -> Pt[1] = 0.0;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A Geometric Primitive Function that returns the leftmost point of Cv.    M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv:   Input curve.		                                             M
*   Res:  Output stored here.                                                M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctCurveRight							     M
*									     *
* KEYWORDS:                                                                  M
*   MvarBsctCurveLeft				                             M
*****************************************************************************/
void MvarBsctCurveLeft(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res)
{
    if (Cv -> Type == MV_CV_CV) {
        MvarBsctCCCurveLeft(Cv, Res);
    }
    else {
        /* Sanity check.*/
        if (Cv -> Type != MV_CV_PT)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

        MvarBsctCPCurveLeft(Cv, Res);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv:  N.S.F.I.                                                            *
*   Res: N.S.F.I.                                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBsctCCCurveRight(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res)
{
    MvarPtStruct 
        *PtZerosLst = NULL;
    CagdRType UMin, UMax, VMin, VMax;

    /* Sanity check. */
    if (Res == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    CagdSrfDomain(Cv -> F3, &UMin, &UMax, &VMin, &VMax);

    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Cv -> F3, UMax,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol), 
						    TRUE);
#   ifdef DEBUG_VORONOI
    if (PtZerosLst -> Pnext != NULL) {
        printf("Right(): Found %d points for the same parameter (expected one)!\n", CagdListLength(PtZerosLst));
    }
#   endif /* DEBUG_VORONOI */
    Res -> Pt[0] = PtZerosLst->Pt[0];
    Res -> Pt[1] = PtZerosLst->Pt[1];

    /* Free PtZeroLst. */  
    MvarPtFreeList(PtZerosLst);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*                                                                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv:  N.S.F.I.                                                            *
*   Res: N.S.F.I.                                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBsctCPCurveRight(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res)
{
    /* Since we are just interseted in the t-param we just construct a     */
    /* point with the t-param.						   */ 
    CagdRType UMin, UMax;
    CagdCrvDomain(Cv -> Crv2, &UMin, &UMax);

    /* Sanity check. */
    if (Res == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    Res -> Pt[0] = UMax;
    Res -> Pt[1] = 0.0;    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A Geometric Primitive Function that returns the rightmost point of Cv.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv: input curve		                                             M
*   Res: ouput stored here                                                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctCurveLeft							     M
*									     *
* KEYWORDS:                                                                  M
*   MvarBsctCurveRight				                             M
*****************************************************************************/
void MvarBsctCurveRight(MvarVoronoiCrvStruct *Cv, MvarPtStruct *Res)
{
    if (Cv -> Type == MV_CV_CV) {
        MvarBsctCCCurveRight(Cv, Res);
    }
    else {
        /* Sanity check.*/
        if (Cv -> Type != MV_CV_PT)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

        MvarBsctCPCurveRight(Cv, Res);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Functions for comparing the distance of the bisector curves at a t-param. *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv1:  N.S.F.I.                                                           *
*   t:    N.S.F.I.                                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:                                                               *
*****************************************************************************/
static CagdRType MvarBsctDistSqAt(MvarVoronoiCrvStruct *Cv1, CagdRType t)
{
    CagdRType Dist1Sq, PtOnTParam[3], *PtAux;

    /* We can assume Cv1.c1 is our t-curve. */
    CagdCrvStruct *Crv = Cv1 -> Crv1; /*the curve we compare on...(t-param). */

    PtAux = CagdCrvEval(Crv, t); 

    /* Returns global variable that can be overwritten - therefore */
    /* we need to copy it.                                         */
    PtOnTParam[0] = PtAux[0];
    PtOnTParam[1] = PtAux[1];
    PtOnTParam[2] = PtAux[2];


    if (Cv1 -> Type == MV_CV_CV) {
        MvarPtStruct 
            *PtZerosLst1 = NULL;
        CagdRType XY1Pt[2];

        
        /* PtOnTParam[0] holds the W param (if exists), currently we assume  */
        /* it doesn't therefore (PtOnTParam[1],PtOnTParam[2]) now hold the   */
        /* X,Y coordinates.                                                  */

        /* Get the corresponding r1 on Cv1->c2 and r2 on Cv2->c2, calculate  */
        /* the distance and compare them.                                    */
        PtZerosLst1 = MvarBsctNewFindZeroSetOfSrfAtParam(Cv1 -> F3, t,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol), 
						    TRUE);

        /* Assert point is the only one in the domain (t-monoticity).        */
        if (PtZerosLst1 == NULL)
            PtAux = NULL;
        else
            PtAux = MvarBsctComputeXYFromBisTR(Cv1 -> Crv1, t, Cv1 -> Crv2,
					       PtZerosLst1 -> Pt[1]);

        /* Returns a global variable that can be modified, thus the aux. */

        /* Freeing PtZerosLst1. */
        MvarPtFreeList(PtZerosLst1);

        if (PtAux != NULL) {
            XY1Pt[0] = PtAux[0];
            XY1Pt[1] = PtAux[1];

            Dist1Sq = (XY1Pt[0] - PtOnTParam[1])*(XY1Pt[0] - PtOnTParam[1]) +
                      (XY1Pt[1] - PtOnTParam[2])*(XY1Pt[1] - PtOnTParam[2]);
        }
        else {
            /* PtAux might return NULL for infinite points. */
            Dist1Sq = IRIT_INFNTY;
        }
    }
    else {
        /* Sanity check. */
        if (Cv1 -> Type != MV_CV_PT)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

        /* Evaluating the corresponding t-param on Crv2 (the pt-crv rational */
	/* bisector).							     */
        PtAux = CagdCrvEval(Cv1 -> Crv2, t);
        if (CAGD_IS_RATIONAL_CRV(Cv1 -> Crv2)){
            PtAux[1] /= PtAux[0];
            PtAux[2] /= PtAux[0];
        }

        Dist1Sq = (PtAux[1] - PtOnTParam[1])*(PtAux[1] - PtOnTParam[1]) +
        (PtAux[2] - PtOnTParam[2])*(PtAux[2] - PtOnTParam[2]);
    }

    return Dist1Sq;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A Geometric Primitive Function that identifies whether Cv1 has Y min.    M
* at given mid-point parameter, assuming both curves are in range.	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv1, Cv2: Input curves.						     M
*   MidPoint: Mid-point parameter.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: Returns TRUE or FALSE.	            				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctCv1IsYSmallerAt		                                     M
*****************************************************************************/
int MvarBsctCv1IsYSmallerAt(MvarVoronoiCrvStruct *Cv1, 
			    MvarVoronoiCrvStruct *Cv2, 
			    MvarPtStruct *MidPoint)
{
    CagdRType t = MidPoint -> Pt[0], 
        Dist1Sq, Dist2Sq;
    Dist1Sq = MvarBsctDistSqAt(Cv1, t);
    Dist2Sq = MvarBsctDistSqAt(Cv2, t);

    return Dist1Sq < Dist2Sq;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*    Functions for finding the intersection points between bisectors.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv1:        N.S.F.I. 	                                             *
*   Cv2:        N.S.F.I.	                                             *
*   Points:     N.S.F.I.	                                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBsctCPCPGetAllIntersectionPoints(MvarVoronoiCrvStruct *Cv1, 
						 MvarVoronoiCrvStruct *Cv2, 
						 MvarPtStruct **Points)
{
    CagdCrvStruct *Bsct1, *Bsct2;

    CagdCrvStruct *CrvCpy, *Res1, *DistSq1, *DistSq2, 
        *DistSq1W, *DistSq1X, *DistSq2W, *DistSq2X, *Tmp;
    MvarPtStruct *MVPts, *MVPt, *MVPtAux;
    CagdRType UMin, UMax;
    MvarMVStruct *MVVec[1];
    MvarConstraintType Constraints[1];
    int BspMultFlag = TRUE;

    /* Sanity check.*/
    if (!((Cv1 -> Type == MV_CV_PT) && (Cv2 -> Type == MV_CV_PT)))
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    /* We assume Cv1 -> Crv1 == Cv2 -> Crv1 is our base curve. */
    Bsct1 = CagdCrvCopy(Cv1 -> Crv2);
    Bsct2 = CagdCrvCopy(Cv2 -> Crv2);

    CagdCrvDomain(Bsct1, &UMin, &UMax); 
    CrvCpy = CagdCrvRegionFromCrv(Cv1 -> Crv1, UMin, UMax);

    /* We compute intersection points by finding the univariate solution of: */
    /* ||Bsct1(t)-C(t)||=||Bsct2(t)-C(t)||, i.e., DistSq1(t) = DistSq2(t).   */
    Res1 = SymbCrvSub(CrvCpy, Bsct1);
    DistSq1 = SymbCrvDotProd(Res1, Res1);
    CagdCrvFree(Res1);

    Res1 = SymbCrvSub(CrvCpy, Bsct2);
    DistSq2 = SymbCrvDotProd(Res1, Res1);
    CagdCrvFree(Res1);

    /* DistSq1(t) = DistSq1X(t)/DistSq1W(t). */ 
    SymbCrvSplitScalar(DistSq1, &DistSq1W, &DistSq1X, &Tmp, &Tmp);
    SymbCrvSplitScalar(DistSq2, &DistSq2W, &DistSq2X, &Tmp, &Tmp);
    CagdCrvFree(DistSq1);
    CagdCrvFree(DistSq2);
    
    /* DistSq1X/DistSq1W - DistSq2X/DistSq2W = 0 => 			     */
    /* 				  DistSq1X*DistSq2W - DistSq2X*DistSq1W = 0. */
    /* Interpolation can fail on ill-condition (small Ws).		     */
    BspMultFlag =  BspMultComputationMethod(BSP_MULT_BEZ_DECOMP);
    DistSq2 = SymbCrvMult(DistSq1W, DistSq2X);
    DistSq1 = SymbCrvMult(DistSq2W, DistSq1X);
    BspMultComputationMethod(BspMultFlag);

    Res1 = SymbCrvSub(DistSq1, DistSq2);

    Constraints[0] = MVAR_CNSTRNT_ZERO;
  
    MVVec[0] = MvarCrvToMV(Res1);
    MVPts = MvarMVsZeros(MVVec, Constraints,1, MvarBsctSubdivTol,
			 -IRIT_FABS(MvarBsctNumerTol));
    MvarMVFree(MVVec[0]);
	
    /* Sorting is always according to first parameter			     */
    /* (since it is a scalar curve).			                     */
    MVPts = MvarPtSortListAxis(MVPts, 1);

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        if (MVPt -> Pt[0] - UMin > 10.0*MvarBsctNumerTol  && 
	    UMax - MVPt -> Pt[0] > 10.0*MvarBsctNumerTol) {
            if ((*Points) != NULL &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[0], (*Points) -> Pt[0], 0.01) &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[1], (*Points) -> Pt[1], 0.01)) {
#               ifdef DEBUG_VORONOI
                    printf("EQUAL\n");
#               endif /* DEBUG_VORONOI */
                continue;
            }

            /* We are interested in the t-param of the tr-points that */
            /* intersec therefore we join together same-t points. */
            if ((*Points) != NULL &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[0], (*Points) -> Pt[0], 0.01)) {
#               ifdef DEBUG_VORONOI
                    printf("EQUAL T - PARAM\n");
#               endif /* DEBUG_VORONOI */
                continue;
            }
            MVPtAux = MvarPtNew(2);
            MVPtAux -> Pt[0] = MVPt -> Pt[0];
            MVPtAux -> Pt[1] = 0.0;
            IRIT_LIST_PUSH(MVPtAux, (*Points));
            MVPtAux = NULL;
        }
    }

    /* Free MVPts. */
    MvarPtFreeList(MVPts);

    /* Reversing the list since we inserted it in a reverse order. */
    (*Points) = (MvarPtStruct*) CagdListReverse((*Points));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computing the symbolic squared distance function Dist2(t,r).             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv1Inp:   N.S.F.I.                                                      *
*   Crv2Inp:   N.S.F.I.                                                      *
*   DistSrf:   N.S.F.I.                                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBsctComputeDistSqTR(CagdCrvStruct *Crv1Inp,
				    CagdCrvStruct *Crv2Inp,
				    CagdSrfStruct **DistSrf)
{
    CagdRType UMin1, UMax1, VMin1, VMax1, UMin2, UMax2, VMin2, VMax2;
    CagdCrvStruct *DCrv1, *DCrv2;
    CagdSrfStruct *Srf1, *Srf2, *DSrf1, *DSrf2, *Res, *Denom, *NumerX, *NumerY,
        *DSrf1X, *DSrf1Y, *DSrf2X, *DSrf2Y, *TSrf1, *TSrf2, *TSrf3;
    
    DCrv1 = CagdCrvDerive(Crv1Inp);
    DCrv2 = CagdCrvDerive(Crv2Inp);

    Srf1 = CagdPromoteCrvToSrf(Crv1Inp, CAGD_CONST_U_DIR);
    Srf2 = CagdPromoteCrvToSrf(Crv2Inp, CAGD_CONST_V_DIR);

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

    BspKnotAffineTrans2(DSrf1 -> VKnotVector,
			DSrf1 -> VLength + DSrf1 -> VOrder,
			VMin2, VMax2);
    BspKnotAffineTrans2(DSrf2 -> UKnotVector,
			DSrf2 -> ULength + DSrf2 -> UOrder,
			UMin1, UMax1);

    SymbSrfSplitScalar(DSrf1, &TSrf1, &DSrf1X, &DSrf1Y, &TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);
    SymbSrfSplitScalar(DSrf2, &TSrf1, &DSrf2X, &DSrf2Y, &TSrf2);
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf2);

    Denom = SymbSrfDeterminant2(DSrf1X, DSrf1Y, DSrf2X, DSrf2Y);
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
    /*	       	                                                             */
    /*< P(t,r) - C1(t), P(t,r) - C1(t) > - <B(t) - C1(t), B(t) - C1(t)> = 0. */
    /*		   		                                             */
    /*	(not implemented yet)                                                */
    /* < Denom*P(t,r) - Denom*C1(t), Denom*P(t,r) - Denom*C1(t) > - 	     */
    /*                         Denom*Denom<B(t) - C1(t), B(t) - C1(t)> = 0.  */
    TSrf3 = SymbSrfMergeScalar(Denom, NumerX, NumerY, NULL);
    TSrf1 = SymbSrfSub(TSrf3, Srf1);         /* TSrf1(t,r) = P(t,r) - C1(t). */
    CagdSrfFree(Srf1);
    CagdSrfFree(Srf2);
    CagdSrfFree(Denom);
    CagdSrfFree(NumerX);
    CagdSrfFree(NumerY);

    Res = SymbSrfDotProd(TSrf1, TSrf1);		      /* The SqDist surface. */
    
    CagdSrfFree(TSrf1);
    CagdSrfFree(TSrf3);

    (*DistSrf) = Res;
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxilary function for finding intersections between Cv-Pt and Cv-Cv	     *
* bisectors.								     *
*****************************************************************************/
static void MvarBsctCPCCGetAllIntersectionPoints(MvarVoronoiCrvStruct *Cv1, 
						 MvarVoronoiCrvStruct *Cv2, 
						 MvarPtStruct **Points)
{
    CagdCrvStruct *Bsct1, *CrvCpy;
    MvarPtStruct *MVPts, *MVPt, *MVPtAux;
    CagdRType UMin, UMax, VMin, VMax;

    CagdCrvStruct *Res1, *Res2;
    CagdSrfStruct *DistSqSrf1, *DistSqSrf2, *DistSqSrf, *DistSqAux;
    MvarMVStruct *MVVec[MVAR_MAX_PT_SIZE];
    MvarConstraintType Constraints[2];

    /* Sanity check. */
    if (Cv1 -> Type != MV_CV_PT)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
    /* Assuming Cv1->Crv1==Cv2->Crv1 is our base curve. */
    Bsct1 = CagdCrvCopy(Cv1 -> Crv2);

    CagdCrvDomain(Bsct1, &UMin, &UMax);    
    CrvCpy = CagdCrvRegionFromCrv(Cv1 -> Crv1, UMin, UMax);

    /* DistSq(t) = <Crv1(t)-Bsct1(t), Crv1(t)-Bsct1(t)>. */
    Res1 = SymbCrvSub(CrvCpy, Bsct1);
    Res2 = SymbCrvDotProd(Res1, Res1);
    CagdCrvFree(Bsct1);
    CagdCrvFree(Res1);

    DistSqSrf1 = CagdPromoteCrvToSrf(Res2, CAGD_CONST_U_DIR);
    CagdCrvFree(Res2);

    CagdSrfDomain(Cv2 -> F3, &UMin, &UMax, &VMin, &VMax); 
    DistSqAux = CagdSrfRegionFromSrf(DistSqSrf1, VMin, VMax, CAGD_CONST_V_DIR);
    CagdSrfFree(DistSqSrf1);
    DistSqSrf1 = DistSqAux;

    MvarBsctComputeDistSqTR(CrvCpy,
		       Cv2 -> Crv2,
		       &DistSqSrf2);
    CagdCrvFree(CrvCpy);

    DistSqAux = CagdSrfRegionFromSrf(DistSqSrf2, VMin, VMax, CAGD_CONST_V_DIR);
    CagdSrfFree(DistSqSrf2);
    DistSqSrf2 = DistSqAux;

    DistSqSrf = SymbSrfSub(DistSqSrf2, DistSqSrf1);
    CagdSrfFree(DistSqSrf1);
    CagdSrfFree(DistSqSrf2);

    /* Solve: DistSq(t,r) - DistSq(t) = 0, F3(t,r) = 0.			    */
    MVVec[0] = MvarSrfToMV(DistSqSrf);
    CagdSrfFree(DistSqSrf);
    MVVec[1] = MvarSrfToMV(Cv2 -> F3);

    Constraints[0] = Constraints[1] = MVAR_CNSTRNT_ZERO;
  
    MVPts = MvarMVsZeros(MVVec, Constraints, 2, MvarBsctSubdivTol,
			 -IRIT_FABS(MvarBsctNumerTol));
    MvarMVFree(MVVec[0]);
    MvarMVFree(MVVec[1]);
    
    if (MVPts != NULL) {
        MvarPtStruct
	    *MVPts1 = MvarPtSortListAxis(MVPts, 1);
        MVPts = MVPts1;
    }

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        if (MVPt -> Pt[0] - UMin > 10.0*MvarBsctNumerTol  && 
	    UMax - MVPt -> Pt[0] > 10.0*MvarBsctNumerTol) {
            if ((*Points) != NULL &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[0], (*Points) -> Pt[0], 0.01) &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[1], (*Points) -> Pt[1], 0.01)) {
#               ifdef DEBUG_VORONOI
                    printf("EQUAL\n");
#               endif /* DEBUG_VORONOI */
                continue;
            }

            /* We are interested in the t-param of the tr-points that */
            /* intersec therefore we join together same-t points. */
            if ((*Points) != NULL &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[0], (*Points) -> Pt[0], 0.01)) {
#               ifdef DEBUG_VORONOI
                    printf("EQUAL T - PARAM\n");
#               endif /* DEBUG_VORONOI */
                continue;
            }
            MVPtAux = MvarPtCopy(MVPt);
            IRIT_LIST_PUSH(MVPtAux, (*Points));
            MVPtAux = NULL;
        }
    }

    /* Free MVPts. */
    MvarPtFreeList(MVPts);

    /* Reversing the list since we inserted it in a reverse order. */
    (*Points) = (MvarPtStruct*) CagdListReverse((*Points));
}

/*****************************************************************************
* AUXILIARY:								     *
* Auxilary function for finding intersections between Cv-Cv and Cv-Cv	     *
* bisectors.								     *
*****************************************************************************/
static void MvarBsctCCCCGetAllIntersectionPoints(MvarVoronoiCrvStruct *Cv1, 
						 MvarVoronoiCrvStruct *Cv2, 
						 MvarPtStruct **Points)
{
    CagdCrvStruct 
        *Crv1Cpy = NULL, 
        *Crv2Cpy = NULL, 
        *Crv3Cpy = NULL;
    MvarPtStruct 
        *MVPts = NULL, 
        *MVPt = NULL, 
        *MVPtAux = NULL;
    CagdRType UMin, UMax, VMin, VMax;

    /* Precondition (sanity check). */
    if (!((Cv1 -> Type == MV_CV_CV) && (Cv2 -> Type == MV_CV_CV)))
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    CagdSrfDomain(Cv1 -> F3, &UMin, &UMax, &VMin, &VMax);
    
    Crv1Cpy = CagdCrvRegionFromCrv(Cv1 -> Crv1, UMin, UMax);/* The curve we compare on (t-param). */
    Crv2Cpy = CagdCrvRegionFromCrv(Cv1 -> Crv2, VMin, VMax);/* The r1 curve.                      */

    CagdSrfDomain(Cv2 -> F3, &UMin, &UMax, &VMin, &VMax);
    Crv3Cpy = CagdCrvRegionFromCrv(Cv2 -> Crv2, VMin, VMax);/* The r2 curve.                      */

    MVPts = MvarBsctSkel2DEqPts3Crvs(Crv1Cpy,
				     Crv2Cpy,
				     Crv3Cpy);

    CagdCrvFree(Crv1Cpy);
    CagdCrvFree(Crv2Cpy);
    CagdCrvFree(Crv3Cpy);

    /* Domain needed for purging away solution points that are out of the */
    /* domain (since we don't trim x_curve.c1/c2).                        */
    CagdSrfDomain(Cv1 -> F3, &UMin, &UMax, &VMin, &VMax);

    for (MVPt = MVPts; MVPt != NULL; MVPt = MVPt -> Pnext) {
        if (MVPt -> Pt[0] - UMin > 10.0*MvarBsctNumerTol  && 
	    UMax - MVPt -> Pt[0] > 10.0*MvarBsctNumerTol) {
            if ((*Points) != NULL &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[0], (*Points) -> Pt[0], 0.01) &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[1], (*Points) -> Pt[1], 0.01)) {
#               ifdef DEBUG_VORONOI
                    printf("EQUAL\n");
#               endif /* DEBUG_VORONOI */
                continue;
            }

            /* We are interested in the t-param of the tr-points that */
            /* intersec therefore we join together same-t points. */
            if ((*Points) != NULL &&
                IRIT_APX_EQ_EPS(MVPt -> Pt[0], (*Points) -> Pt[0], 0.01)) {
#               ifdef DEBUG_VORONOI
                    printf("EQUAL T - PARAM\n");
#               endif /* DEBUG_VORONOI */
                continue;
            }
            MVPtAux = MvarPtCopy(MVPt);
            IRIT_LIST_PUSH(MVPtAux, (*Points));
            MVPtAux = NULL;
        }
    }

    /* Free MVPts. */
    MvarPtFreeList(MVPts);

    /* Reversing the list since we inserted it in a reverse order. */
    (*Points) = (MvarPtStruct*) CagdListReverse((*Points));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   A Geometric Primitive Function that computes the equidistant points of   M
*   three curves Cv1, Cv2 and Cv2 (third one also the second curve).	     M
*   Currently implemented just by calling Skel2DEqPts3Crvs (and purges       M
*   away solutions). ToDo: implement a more efficient version, based on      M
*   the already calculated F3.                                               M 
*                                                                            *
* PARAMETERS:                                                                M
*   Cv1, Cv2: Input curves.						     M
*   Points:   The resultant points.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctSkel2DEqPts3Crvs						     M
*									     *
* KEYWORDS:                                                                  M
*   MvarBsctGetAllIntersectionPoints			                     M
*****************************************************************************/
void MvarBsctGetAllIntersectionPoints(MvarVoronoiCrvStruct *Cv1, 
				      MvarVoronoiCrvStruct *Cv2, 
				      MvarPtStruct **Points)
{
    /* Precondition (sanity check). */
    if ((*Points) != NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    /* Find all equal distance points between Cv1.c1 (assert==Cv2.c1), */
    /* Cv1.c2 and Cv2.c2.                                              */

    /* We can assume Cv1.c1 is our t-curve. */
    if (!CagdCrvsSame(Cv1 -> Crv1, Cv2 -> Crv1, 100.0*MvarBsctNumerTol))
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    if ((Cv1 -> Type == MV_CV_PT) && (Cv2 -> Type == MV_CV_PT)) {
        MvarBsctCPCPGetAllIntersectionPoints(Cv1, Cv2, Points);
    }
    else if (Cv1 -> Type == MV_CV_PT) {
        /* Sanity check.*/
        if (Cv2 -> Type != MV_CV_CV)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
        MvarBsctCPCCGetAllIntersectionPoints(Cv1, Cv2, Points);
    }
    else if (Cv2 -> Type == MV_CV_PT) {
        /* Sanity check.*/
        if (Cv1 -> Type != MV_CV_CV)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
        MvarBsctCPCCGetAllIntersectionPoints(Cv2, Cv1, Points);
    }
    else {
        /* Sanity check.*/
        if (!(Cv1 -> Type == MV_CV_CV && Cv2 -> Type == MV_CV_CV))
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
        MvarBsctCCCCGetAllIntersectionPoints(Cv1, Cv2, Points);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Functions for splitting a curve.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv:                   N.S.F.I.                                           *
*   CagdRType:            N.S.F.I.                                           *
*   MvarVoronoiCrvStruct: N.S.F.I.                                           *
*   MvarVoronoiCrvStruct: N.S.F.I.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MvarBsctCPSplitCurve(MvarVoronoiCrvStruct *Cv, 
				 CagdRType t, 
				 MvarVoronoiCrvStruct **CvLeft, 
				 MvarVoronoiCrvStruct **CvRight)
{
    CagdCrvStruct *SubdivCrvs = CagdCrvSubdivAtParam(Cv -> Crv2, t);

    (*CvLeft) = MvarVoronoiCrvCopy(Cv);
    /* Freeing CvLeft -> Crv2. */
    CagdCrvFree((*CvLeft) -> Crv2);
    (*CvLeft) -> Crv2 = SubdivCrvs;

    (*CvRight) = MvarVoronoiCrvCopy(Cv);
    /* Freeing CvRight -> Crv2. */
    CagdCrvFree((*CvRight) -> Crv2);
    (*CvRight) -> Crv2 = SubdivCrvs -> Pnext;

    (*CvLeft) -> Crv2 -> Pnext = NULL;
    (*CvRight) -> Crv2 -> Pnext = NULL;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  A Geometric Primitive Function that splits the given curve into two -     M
*  left and right curve of a given parameter t.				     M
*                                                                            *
* PARAMETERS:                                                                M
*   Cv:              Input curve.					     M
*   SplitPt:         The parameter at which to split.   		     M
*   CvLeft, CvRight: The resultant split curves.			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctSplitCurve							     M
*****************************************************************************/
void MvarBsctSplitCurve(MvarVoronoiCrvStruct *Cv, 
			MvarPtStruct *SplitPt, 
			MvarVoronoiCrvStruct **CvLeft, 
			MvarVoronoiCrvStruct **CvRight)
{
    CagdSrfStruct 
        *F3Left = NULL, 
        *F3Right = NULL,
        *AuxSrfLeft = NULL,
        *AuxSrfRight = NULL; 
    CagdRType 
        t = SplitPt -> Pt[0];    /* Projection. */
    CagdRType r, LeftR;
    MvarPtStruct *PtZerosLst; 

    CagdRType UMin, UMax, VMin, VMax;

    /* CV_PT split. */
    if (Cv -> Type == MV_CV_PT) {
        MvarBsctCPSplitCurve(Cv, t, CvLeft, CvRight);
        return;
    }

    /* Otherwise it is a CV_CV split - perform a sanity check. */
    if (Cv -> Type != MV_CV_CV)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    /* Split at t parameter, find the r parameter of the end seam and split */
    /* there too (take upper or lower depending whether it is increasing or */
    /* decreasing...).                                                      */

    CagdSrfDomain(Cv -> F3, &UMin, &UMax, &VMin, &VMax);

    F3Left = CagdSrfRegionFromSrf(Cv -> F3, UMin, t, CAGD_CONST_U_DIR);
    F3Right = CagdSrfRegionFromSrf(Cv -> F3, t, UMax, CAGD_CONST_U_DIR);

    /* Finding r parameter (V domain) at t parameter and splitting there. */
    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Cv -> F3, t,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol), 
						    TRUE);
    /* Sanity check.*/
    if (PtZerosLst == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
    /* Decide whether to leave either left-up+right-down or                  */
    /* left-down+right-up, and then trim V of F3Left and F3Right at          */
    /* PtZerosLst -> Pt[1].					             */
    r = PtZerosLst -> Pt[1];
    MvarPtFreeList(PtZerosLst);

    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Cv -> F3, UMin,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol), 
						    TRUE);
    /* Sanity check. */
    if (PtZerosLst == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    LeftR = PtZerosLst -> Pt[1];
    MvarPtFreeList(PtZerosLst);

    if (LeftR < r) {
        /* LeftR > r, the curve is a '/' curve - leave left-down + right-up. */
        AuxSrfLeft = CagdSrfRegionFromSrf(F3Left, VMin, r, CAGD_CONST_V_DIR);
        AuxSrfRight = CagdSrfRegionFromSrf(F3Right, r, VMax, CAGD_CONST_V_DIR);
    }
    else {
        /* LeftR > r, the curve is a '\' curve - leave left-up + right-down. */
        AuxSrfLeft = CagdSrfRegionFromSrf(F3Left, r, VMax, CAGD_CONST_V_DIR);
        AuxSrfRight = CagdSrfRegionFromSrf(F3Right, VMin, r, CAGD_CONST_V_DIR);
    }
    CagdSrfFree(F3Left);
    CagdSrfFree(F3Right);
    F3Left = AuxSrfLeft;
    F3Right = AuxSrfRight;

    /* We do not split Crv1,2 as we might need them in future computations. */
    (*CvLeft) = MvarVoronoiCrvCopy(Cv);
    
    /* Freeing CvLeft -> F3. */
    CagdSrfFree((*CvLeft) -> F3);
    (*CvLeft) -> F3 = F3Left;

    (*CvRight) = MvarVoronoiCrvCopy(Cv);

    /* Freeing CvRight -> F3. */
    CagdSrfFree((*CvRight) -> F3);
    (*CvRight) -> F3 = F3Right;
}
