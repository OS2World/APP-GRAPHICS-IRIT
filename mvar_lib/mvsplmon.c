/******************************************************************************
* mvsplmon.c - Split a zero set of a bivariate function into monotone pieces. *
*                                                                             *
* The algorithm is based on Section 4.3 and 8.3 of the paper:                 *
*   J. Keyser, T. Culver, D. Manocha and S. Krishnan,			      *
*   Efficient and exact manipulation of algebraic points and curves,          *
*   Computer-Aided Design, Volume 32, Number 11, 15 September 2000,           *
*   pp 649--662.							      *
*  (also available as UNC-CS Technical Report TR98-038).                      *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Hanniel, March 2005.				              *
******************************************************************************/

#include <stdio.h>
#include "geom_lib.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "mvar_loc.h"

#define FlatCornerTrimParam 0.05

static MvarPtStruct *MvarBsctRemovePtsFromLstAux(MvarPtStruct **Extreme);
static MvarPtStruct *MvarBsctGetUVSingularPoints(MvarPtStruct **UExtreme, 
						 MvarPtStruct **VExtreme, 
						 CagdRType MvarBsctNumerTol);
static CagdBBoxStruct *MvarBsctMatchOneToThree(MvarPtStruct *First, 
					       int FirstLen, 
					       MvarPtStruct *Other1, 
					       MvarPtStruct *Other2, 
					       MvarPtStruct *Other3);
static void MvarBsctMateTBLRPoints(CagdSrfStruct *Srf,
				   CagdSrfStruct **OutLst, 
				   MvarPtStruct *T,
				   int LenT,
				   MvarPtStruct *B, 
				   int LenB,
				   MvarPtStruct *L,
				   int LenL, 
				   MvarPtStruct *R,
				   int LenR);
static CagdRType MvarBsctFindMidParam(MvarPtStruct *Lst1,
			       int LenLst1, 
			       MvarPtStruct *Lst2,
			       int LenLst2,
			       CagdSrfDirType Dir,
			       CagdRType *LargestGapOut);
static void MvarBsctFindClosestParamsAroundPoint(MvarPtStruct *Pt, 
						 MvarPtStruct *Lst, 
						 CagdRType *UInf, 
						 CagdRType *USup, 
						 CagdRType *VInf, 
						 CagdRType *VSup);
static CagdBType RemoveIsolatedCornersIfExist(CagdSrfStruct *Srf, 
                                              MvarPtStruct **T,
					      int *LenT, 
                                              MvarPtStruct **B,
					      int *LenB);
static CagdBType HasC0Discont(CagdSrfStruct *Srf);

/*****************************************************************************
* DESCRIPTION:                                                               *
*  A local utility that purges away marked points from a list.               *
*  Removes points if they are marked by "Remove", or removes them and copies *
*  to the output list if they are marked by "RemoveAndCopy".                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Extreme:  a list of points.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *: list of points marked by "RemoveAndCopy" (NULL if none). *
*****************************************************************************/
static MvarPtStruct *MvarBsctRemovePtsFromLstAux(MvarPtStruct **Extreme)
{
    /* Copying of the identical point to Outset, and removal from the */
    /* original lists (UExtreme,VExtrem).                             */
    MvarPtStruct
	*OutSet = NULL;

    if ((*Extreme) != NULL) {
        MvarPtStruct 
            *Head1 = *Extreme,
            *Head2 = (*Extreme) -> Pnext;

        while (Head2 != NULL) {
	    if (AttrGetIntAttrib(Head2 -> Attr, "CopyAndRemove") == TRUE) {
                /* Advance Head2 only & move what Head2 used to be to OutSet.*/
                Head1 -> Pnext = Head2 -> Pnext;

                Head2 -> Pnext = NULL;
                IRIT_LIST_PUSH(Head2, OutSet);

                Head1 = Head1 -> Pnext;
            }
            else if (AttrGetIntAttrib(Head2 -> Attr, "Remove") == TRUE) {
                /*Advance Head2 only and remove what Head2 used to be. */ 
                Head1 -> Pnext = Head2 -> Pnext;
                MvarPtFree(Head2);
                Head2 = Head1 -> Pnext;
            } 
            else {
                /*Advance Head1 and Head2.*/
                Head1 = Head2;
                Head2 = Head2 -> Pnext;
            }
        }

        /* Check for first element (wasn't checked in the above). */
        if (AttrGetIntAttrib((*Extreme) -> Attr, "CopyAndRemove") == TRUE) {
            Head2 = *Extreme;
            (*Extreme) = (*Extreme) -> Pnext;

            Head2 -> Pnext = NULL;
            IRIT_LIST_PUSH(Head2, OutSet);
        }
        else if (AttrGetIntAttrib((*Extreme) -> Attr, "Remove") == TRUE) {
            Head2 = *Extreme;
            (*Extreme) = (*Extreme) -> Pnext;

            MvarPtFree(Head2);
        }
    }

    return OutSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Purges away two points if they are same, determined by the tolerance      *
*                                                                            *
* PARAMETERS:                                                                *
*   UExtreme, VExtreme:   U and V Extreme parameter values.                  *
*   MvarBsctNumerTol:     Tolerance used for aquiring the extreme points.    *
*     We consider 10*MvarBsctNumerTol as the tolerance for identical points. *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:   Resultant points after purging.			     *
*                                                                            *
* KEYWORDS:                                                                  *
*   MvarBsctGetUVSingularPoints                                              *
*****************************************************************************/
static MvarPtStruct *MvarBsctGetUVSingularPoints(MvarPtStruct **UExtreme, 
						 MvarPtStruct **VExtreme, 
						 CagdRType MvarBsctNumerTol)
{
    CagdRType 
        Tol = 10.0 * MvarBsctNumerTol;
    MvarPtStruct *Head1, *Head2,
        *OutSet = NULL;
    int i;
 
    /* Identifying UV singular points (i.e., extreme points both in u and v).*/
    for (Head1 = *UExtreme; Head1 != NULL; Head1 = Head1 -> Pnext) {
        for (Head2 = *VExtreme; Head2 != NULL; Head2 = Head2 -> Pnext) {
	    for (i = 0; i < Head2 -> Dim; i++) {
	        if (!IRIT_APX_EQ_EPS(Head1 -> Pt[i], Head2 -> Pt[i],
				     IRIT_FABS(Tol))) 
		    break;
	    }
	    if (i >= Head2 -> Dim) {
	        /* Pt1 and Pt2 are same - mark the points for removal. */
		AttrSetIntAttrib(&Head1 -> Attr, "CopyAndRemove", TRUE);
		AttrSetIntAttrib(&Head2 -> Attr, "Remove", TRUE);
	    }
	}
    }

    /* Removing the identified points and moving a copy to Outset.*/
    OutSet = MvarBsctRemovePtsFromLstAux(VExtreme);
    assert(OutSet==NULL); /* Sanity check, should return NULL. */
    OutSet = MvarBsctRemovePtsFromLstAux(UExtreme);

    return OutSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find the zeroset value at the given paramter of the given surface 	     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Implicit surface definition.                                      M
*   Param:   Parametric value for computation.				     M
*   Dir:   U or V direction for computation				     M
*   MvarBsctSubdivTol:  Subdivision tolerance for the multivariate solver.   M
*   MvarBsctNumerTol:   Numeric tolerance of a possible numeric improvement  M
*			stage.  The numeric stage is employed if             M
*			MvarBsctNumerTol < MvarBsctSubdivTol.	             M
*   ShouldCheckEndPoints: To include end points also for checking.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   MvarPtStruct *:   The computed values				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctNewFindZeroSetOfSrfAtParam                                       M
*****************************************************************************/
MvarPtStruct *MvarBsctNewFindZeroSetOfSrfAtParam(CagdSrfStruct *Srf, 
						 CagdRType Param,
						 CagdSrfDirType Dir, 
						 CagdRType MvarBsctSubdivTol,
						 CagdRType MvarBsctNumerTol,
						 CagdBType ShouldCheckEndPoints)
{
    MvarPtStruct *Aux,
        *OutSet = NULL;
    CagdCrvStruct 
        *Crv = CagdCrvFromSrf(Srf, Param, Dir);    
    MvarConstraintType Constraints[1];
    MvarMVStruct *MVVec[MVAR_MAX_PT_SIZE];
    MvarPtStruct *MVPts;
    CagdRType UMin, UMax, VMin, VMax, u, v, MinParam, MaxParam,
    	Tol = 10.0 * MvarBsctNumerTol;

    Constraints[0] = MVAR_CNSTRNT_ZERO;
  
    MVVec[0] = MvarCrvToMV(Crv);
    MVPts = MvarMVsZeros(MVVec, Constraints, 1, MvarBsctSubdivTol,
			 -IRIT_FABS(MvarBsctNumerTol));
    MvarMVFree(MVVec[0]);

    /* Sorting is always according to first parameter */
    /* (since it is a scalar curve).                  */
    MVPts = MvarPtSortListAxis(MVPts, 1);
	
    /* Two ways to evaluate at end points: compare endpoints to zero, or add */
    /* constraint GreaterThan and LessThan. We implemented the first way.    */
  
    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);
  
    if (IRIT_FABS(Crv -> Points[1][0]) < IRIT_FABS(Tol)) {
        if (Dir == CAGD_CONST_U_DIR) {
	    u = Param; 
            v = MinParam = VMin;
	}
	else {
	    u = MinParam = UMin; 
            v = Param;
	}
		
	/* Compare start point to MVPts start point and insert/delete */
	/* if needed.                                                 */
	if (MVPts == NULL || IRIT_FABS(MVPts -> Pt[0]-MinParam) > 
	    IRIT_FABS(Tol)) {
#           ifdef DEBUG_VORONOI
	    printf("MVPts didn't detect start point - add it if ShouldCheckEndPoints\n");
#           endif
	    if (ShouldCheckEndPoints) {
	        /* Add start point. */
	        Aux = MvarPtNew(1); /* Assuming no rational w... */
		Aux -> Pt[0] = MinParam;
		IRIT_LIST_PUSH(Aux, MVPts);
	    }
	}
	/* Else if we already have it and !ShouldCheckEndPoints. */
        else {
            /* Sanity check.*/
            if (!(MVPts != NULL &&
		IRIT_FABS(MVPts -> Pt[0]- MinParam) <= IRIT_FABS(Tol)))
                MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

	    if (!ShouldCheckEndPoints) {
	        /* Delete start point. */
	        Aux = MVPts -> Pnext;
                MvarPtFree(MVPts);
		MVPts = Aux;
	    }
        }
    }

    if (IRIT_FABS(Crv -> Points[1][Crv -> Length-1]) 
	< IRIT_FABS(Tol)) {
        if (Dir == CAGD_CONST_U_DIR) {
	    u = Param; 
            v = MaxParam = VMax;
	}
	else {
	    u = MaxParam = UMax; 
            v = Param;
	}
	
	if (MVPts == NULL ||
	    IRIT_FABS(MaxParam - (MvarGetLastPt(MVPts)) -> Pt[0]) 
							    > IRIT_FABS(Tol)) {
#           ifdef DEBUG_VORONOI
	    printf("MVPts didn't detect end point - add it if ShouldCheckEndPoints\n");
#           endif
	    if (ShouldCheckEndPoints) {
	        /* Append endpoint. */
	        Aux = MvarPtNew(1);
		Aux -> Pt[0] = MaxParam;
		if (MVPts != NULL)
		    CagdListAppend(MVPts, Aux);
		else
		    MVPts = Aux;
	    }
	}
	/* Else if we already have it and !ShouldCheckEndPoints. */
        else {
            /* Sanity check.*/
            if (!(MVPts != NULL && 
		IRIT_FABS(MaxParam - (MvarGetLastPt(MVPts)) -> Pt[0]) <=
							      IRIT_FABS(Tol)))
                MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

	    if (!ShouldCheckEndPoints) {
                /*Free last point in list (which is actually the endpoint).*/
                if (MVPts -> Pnext == NULL) {
                    MvarPtFree(MVPts);
                    MVPts = NULL;
                }
                else {
                    MvarPtStruct 
                        *Aux1 = MVPts;
	            Aux = MVPts -> Pnext;
                    while (Aux -> Pnext != NULL) {
                        Aux1 = Aux;
                        Aux = Aux -> Pnext;
                    }
                    MvarPtFree(Aux);
		    Aux1 ->Pnext = NULL;
                }
            }
        }
    }

    CagdCrvFree(Crv);

    /* Copy PtZeroLst to MVar format and free it. */
    for (Aux = MVPts; Aux != NULL; Aux = Aux -> Pnext) {
        MvarPtStruct 
	    *NewPt = MvarPtNew(2);

	if (Dir == CAGD_CONST_U_DIR) {
	    NewPt -> Pt[0] = Param;
	    NewPt -> Pt[1] = Aux -> Pt[0];
	}
	else {
	    NewPt -> Pt[1] = Param;
	    NewPt -> Pt[0] = Aux -> Pt[0];        
	}
		
	if (OutSet != NULL)
	    CagdListAppend(OutSet, NewPt);
	else
	    OutSet = NewPt;
    }
  
    MvarPtFreeList(MVPts);

    return OutSet;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Trims the given surface Srf using the given box dimensions. Uses	     M
*   CagdSrfRegionFromSrf twice.						     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Implicit surface definition.                                      M
*   UVBbox:   Given box dimensions.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   CagdSrfStruct *:   The trimmed surface				     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctTrimSurfaceByUVBbox                                              M
*****************************************************************************/
CagdSrfStruct *MvarBsctTrimSurfaceByUVBbox(CagdSrfStruct *Srf, 
					   CagdBBoxStruct UVBbox)
{
    CagdRType SrfUMin, SrfUMax, SrfVMin, SrfVMax,
        UMin = UVBbox.Min[0],
        UMax = UVBbox.Max[0],
        VMin = UVBbox.Min[1],
        VMax = UVBbox.Max[1];
    CagdSrfStruct *Srf1, *Srf2;
    
    CagdSrfDomain(Srf, &SrfUMin, &SrfUMax, &SrfVMin, &SrfVMax);

    if (!IRIT_APX_EQ(SrfUMin, UMin) || !IRIT_APX_EQ(SrfUMax, UMax))
        Srf1 = CagdSrfRegionFromSrf(Srf, UMin, UMax, CAGD_CONST_U_DIR);
    else
        Srf1 = CagdSrfCopy(Srf);

    if (!IRIT_APX_EQ(SrfVMin, VMin) || !IRIT_APX_EQ(SrfVMax, VMax)) {
        Srf2 = CagdSrfRegionFromSrf(Srf1, VMin, VMax, CAGD_CONST_V_DIR);
	CagdSrfFree(Srf1);
    }
    else
        Srf2 = Srf1;

    return Srf2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Auxilary function: reconstructs the topology of the curve based on the   *
*    intersection points of the curve with the boundaries of the domain.     *
*   This function is only called when:                                       *
*          LenL = LenB+LenR+LenT or LenR = LenB+LenL+LenT or                 *
*          LenT = LenL+LenB+LenR or LenB = LenL+LenT+LenR.                   *
*    It reconstructs the topology of the curve by matching First with Other1,*
*    Other2, Other3 and inserts the match into an array of BBoxes.           *
*    See also Section 8.3 of the paper by Keyser et al.                      *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   First:    MvarPtStruct - a list of points intersecting a boundary edge.  *
*   FirstLen: Length of the `First' paramter.                                *
*   Other1:   MvarPtStruct - a list of points intersecting a boundary edge.  *
*   Other2:   MvarPtStruct - a list of points intersecting a boundary edge.  *
*   Other3:   MvarPtStruct - a list of points intersecting a boundary edge.  *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBBoxStruct *: An array of size FirstLen, of bounding boxes           *
*   containing single branches of the curve.				     *
*****************************************************************************/
static CagdBBoxStruct *MvarBsctMatchOneToThree(MvarPtStruct *First, 
					int FirstLen, 
					MvarPtStruct *Other1, 
					MvarPtStruct *Other2, 
					MvarPtStruct *Other3)
{
    int i = 0;
    MvarPtStruct *Iter1, *Iter2;
    CagdBBoxStruct 
        *BboxArray = CagdBBoxArrayNew(FirstLen);

    Iter1 = First;
    for (Iter2 = Other1; Iter2 != NULL; Iter1 = Iter1 -> Pnext, 
	   Iter2 = Iter2 -> Pnext) {
        BboxArray[i].Min[0] = IRIT_MIN(Iter1 -> Pt[0], Iter2 -> Pt[0]);
	BboxArray[i].Min[1] = IRIT_MIN(Iter1 -> Pt[1], Iter2 -> Pt[1]);
	BboxArray[i].Max[0] = IRIT_MAX(Iter1 -> Pt[0], Iter2 -> Pt[0]);
	BboxArray[i].Max[1] = IRIT_MAX(Iter1 -> Pt[1], Iter2 -> Pt[1]);
	++i;  
    }	
    for (Iter2 = Other2; Iter2 != NULL; Iter1 = Iter1 -> Pnext, 
	   Iter2 = Iter2 -> Pnext) {
        BboxArray[i].Min[0] = IRIT_MIN(Iter1 -> Pt[0], Iter2 -> Pt[0]);
	BboxArray[i].Min[1] = IRIT_MIN(Iter1 -> Pt[1], Iter2 -> Pt[1]);
	BboxArray[i].Max[0] = IRIT_MAX(Iter1 -> Pt[0], Iter2 -> Pt[0]);
	BboxArray[i].Max[1] = IRIT_MAX(Iter1 -> Pt[1], Iter2 -> Pt[1]);
	++i;  
    }
    for (Iter2 = Other3; Iter2 != NULL; Iter1 = Iter1 -> Pnext, 
	   Iter2 = Iter2 -> Pnext) {
        BboxArray[i].Min[0] = IRIT_MIN(Iter1 -> Pt[0], Iter2 -> Pt[0]);
	BboxArray[i].Min[1] = IRIT_MIN(Iter1 -> Pt[1], Iter2 -> Pt[1]);
	BboxArray[i].Max[0] = IRIT_MAX(Iter1 -> Pt[0], Iter2 -> Pt[0]);
	BboxArray[i].Max[1] = IRIT_MAX(Iter1 -> Pt[1], Iter2 -> Pt[1]);
	++i;  
    }

    return BboxArray;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given surface, prepare OutLst based on the values of T, B, L, R. For     *
*   the details of the algorithm, see Section 8.3 of the paper               *
*   by Keyser et al.                                                         *
*                                                                            *
*   This function is only called when:                                       *
*          LenL = LenB+LenR+LenT or LenR = LenB+LenL+LenT or                 *
*          LenT = LenL+LenB+LenR or LenB = LenL+LenT+LenR.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   Given surface.						     *
*   OutLst:   Output list of the processed surface.			     *
*   T, LenT: List of points on Top edge (i.e., Srf(u,VMax) and its length.   *
*   B, LenB: List of points on the Bottom edge (i.e., Srf(u,VMin)).          *
*   L, LenL: List of points on the Left edge (i.e., Srf(UMin,v)).            *
*   R, LenR: List of points on the Right edge (i.e., Srf(UMax,v).            *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	               						     *
*****************************************************************************/
static void MvarBsctMateTBLRPoints(CagdSrfStruct *Srf,
				   CagdSrfStruct **OutLst,
				   MvarPtStruct *T,
				   int LenT,
				   MvarPtStruct *B, 
				   int LenB,
				   MvarPtStruct *L,
				   int LenL, 
				   MvarPtStruct *R,
				   int LenR)
{
    int NumBBoxes,
	i = 0;
    CagdBBoxStruct *BboxArray;
    CagdSrfStruct *TrimmedSrf;

    /* All the lists TBLR are ordered incremetally (TB according to u, LR    */
    /* according to v).							     */

    if (IRIT_FABS(LenL-LenR) == LenT+LenB) {
        if (LenL > LenR) {
	    /* LenL = LenB + LenR + LenT. */
#           ifdef DEBUG_VORONOI
	    printf("LenL = %d\n", LenL);
#           endif
	    if (LenL > 0) {
	        BboxArray = MvarBsctMatchOneToThree(L, LenL, B, R, T);
	    }
	    NumBBoxes = LenL;
	}
	else {
	    /* LenR = LenB + LenL + LenT. */ 
#           ifdef DEBUG_VORONOI
	    printf("LenR = %d\n", LenR);
#           endif
	    if (LenR > 0) {
	        BboxArray = MvarBsctMatchOneToThree(R, LenR, B, L, T); 
	    }
	    NumBBoxes = LenR;
	    
	}
    }
    else {
        /* Consistency check.*/
        if (IRIT_FABS(LenT - LenB) != LenL+LenR)
            MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

	if (LenT > LenB) {
	    /* LenT = LenL + LenB + LenR. */
#           ifdef DEBUG_VORONOI
	    printf("LenT = %d\n", LenT);
#           endif
	    if (LenT > 0) {
	        BboxArray = MvarBsctMatchOneToThree(T, LenT, L, B, R); 
	    }
	    NumBBoxes = LenT;
	}
	else {
	    /* LenB = LenL + LenT + LenR. */
#           ifdef DEBUG_VORONOI
	    printf("LenB = %d\n", LenB);
#           endif
	    if (LenB > 0) {
	        BboxArray = MvarBsctMatchOneToThree(B, LenB, L, T, R); 
	    }
	    NumBBoxes = LenB;
	}
    }
  
    /* Go over the bboxes and trim the surface accordingly (free bboxes). */
    for (i = 0; i < NumBBoxes; ++i) {
        TrimmedSrf = MvarBsctTrimSurfaceByUVBbox(Srf, BboxArray[i]);
	IRIT_LIST_PUSH(TrimmedSrf, (*OutLst));
    }

    if (NumBBoxes > 0)
        CagdBBoxArrayFree(BboxArray, NumBBoxes);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds mid param of largest gap in the both input lists (i.e., when both  *
*   input lists are merged). The largest gap is computed in the direction    *
*   Dir.                					             *
*                                                                            *
* PARAMETERS:                                                                *
*   Lst1, LenLst1:  MvarPtStruct of points and its length.		     *
*   Lst2, LenLst2:  MvarPtStruct of points and its length.		     *
*   Dir: Direction for processing					     *
*   LargestGapOut: Largest gap to be processed				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   Computed mid-parameter value.                               *
*****************************************************************************/
static CagdRType MvarBsctFindMidParam(MvarPtStruct *Lst1,
			       int LenLst1, 
			       MvarPtStruct *Lst2,
			       int LenLst2,
			       CagdSrfDirType Dir,
			       CagdRType *LargestGapOut)
{
    int Coord = (Dir == CAGD_CONST_U_DIR) ? 0 : 1;
    MvarPtStruct *PtList, *PtListSorted;
    CagdRType LargestGap, LargestGapMidPoint;
    MvarPtStruct
	*Lst1Cpy = MvarPtCopyList(Lst1),  /* Needs copy to enable mem free. */
        *Lst2Cpy = MvarPtCopyList(Lst2);

    /* Sanity check. */
    if (LenLst1 + LenLst2 <= 1)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    PtList = (MvarPtStruct*) CagdListAppend(Lst1Cpy, Lst2Cpy); /* In-place. */
    PtListSorted = MvarPtSortListAxis(PtList, Coord + 1);      /* In-place. */
    PtList = PtListSorted;           /* So we can free the list in the end. */
    LargestGap = PtListSorted -> Pnext -> Pt[Coord] 
                 - PtListSorted -> Pt[Coord];
    LargestGapMidPoint = 0.5 * (PtListSorted -> Pt[Coord] + 
			        PtListSorted -> Pnext -> Pt[Coord]);

    while (PtListSorted -> Pnext) {
        CagdRType 
            Gap = PtListSorted -> Pnext -> Pt[Coord] 
	                - PtListSorted -> Pt[Coord];

	if (Gap > LargestGap) {
	    LargestGap = Gap;
	    LargestGapMidPoint = 0.5 * (PtListSorted -> Pt[Coord] + 
				        PtListSorted -> Pnext -> Pt[Coord]);
	}
	PtListSorted = PtListSorted -> Pnext;
    }
    MvarPtFreeList(PtList);

    (*LargestGapOut) = LargestGap;
    return LargestGapMidPoint;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given an inner point Pt, and a list of boundary points Lst, the function *
* finds a box around Pt, consisting of the closest parameters in Lst, which  *
* are above and below of Pt. That is, the smallest box containg Pt, which    *
* is constructed by projecting points of Lst in the U and V directions.      *
*   NOTE: The function caller must initialize UInf, USup, VInf, VSup,        *
* and the function updates them according to the algorithm.                  *
*   The values are returned in the parameters UInf, USup, VInf, VSup.        *
*                                                                            *
* PARAMETERS:                                                                *
*   Pt: Inner point (the function will be called with an extreme point here).* 
*   Lst:  MvarPtStruct of boundary points.			             *
*   UInf: U parameter bounding Pt from below. 				     *
*   USup: U parameter bounding Pt from above.  				     *
*   VInf: V parameter bounding Pt from below.  				     *
*   VSup: V parameter bounding Pt from above.  				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void MvarBsctFindClosestParamsAroundPoint(MvarPtStruct *Pt, 
						 MvarPtStruct *Lst, 
						 CagdRType *UInf, 
						 CagdRType *USup, 
						 CagdRType *VInf, 
						 CagdRType *VSup)
{
    while (Lst != NULL) {
        if (Lst -> Pt[0] > Pt -> Pt[0] && Lst -> Pt[0] < *USup) {
	    *USup = Lst -> Pt[0];
	}
	if (Lst -> Pt[0] < Pt -> Pt[0] && Lst -> Pt[0] > *UInf) {
	    *UInf = Lst -> Pt[0];
	}
	if (Lst -> Pt[1] > Pt -> Pt[1] && Lst -> Pt[1] < *VSup) {
	    *VSup = Lst -> Pt[1];
	}
	if (Lst -> Pt[1] < Pt -> Pt[1] && Lst -> Pt[1] > *VInf) {
	    *VInf = Lst -> Pt[1];
	}
	
	Lst = Lst -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the "flat corners" (if exist) and handle them by trimming.	     *
*   Flat corners are corner points of the implicit function where            *
* both partial derivatives are zero. Such points occur in our context        *
* when the original adjacent input curves touch at their endpoints. Since    *
* this causes numerical instability for the iterative method (the            *
* attainable accuracy problem), we handle it specifically.                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   Implicit surface definition.                                      *
*   OutLst:   Output list of uv-monotone surfaces.			     *
*   SubdivLst:            Where subdivided pieces of Srf will go.	     *
*   MvarBsctSubdivTol:    Subdivision tolerance for multivariate solver.     *
*   MvarBsctNumerTol:     Numeric tolerance of a possible numeric improvment *
*			  stage.  The numeric stage is employed if           *
*			  MvarBsctNumerTol < MvarBsctSubdivTol.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType: returns TRUE if a flat corner has been handled. 		     *
*****************************************************************************/
static CagdBType HandleFlatCornersIfExist(CagdSrfStruct *Srf,
					  CagdSrfStruct **OutLst,
					  CagdSrfStruct **SubdivLst,
					  CagdRType MvarBsctSubdivTol,
					  CagdRType MvarBsctNumerTol)
{
    CagdRType UMin, UMax, VMin, VMax, UMinNew, UMaxNew, VMinNew, VMaxNew,
	F, DV, DU;
    CagdBType 
        HasFlatCorner = FALSE;   
    CagdSrfStruct
        *DVSrf = CagdSrfDerive(Srf, CAGD_CONST_V_DIR),
        *DUSrf = CagdSrfDerive(Srf, CAGD_CONST_U_DIR),
        *Srf1;
    CagdBBoxStruct UVBbox;
    MvarPtStruct 
        *PtZerosLst = NULL;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    UMinNew = UMin; 
    UMaxNew = UMax; 
    VMinNew = VMin; 
    VMaxNew = VMax;

    /* (UMin, VMin) point. */
    F = (CagdSrfEval(Srf, UMin, VMin))[1];
    DV = (CagdSrfEval(DVSrf, UMin, VMin))[1];
    DU = (CagdSrfEval(DUSrf, UMin, VMin))[1];

    if (IRIT_APX_EQ_EPS(F, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	IRIT_APX_EQ_EPS(DU, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	IRIT_APX_EQ_EPS(DV, 0, IRIT_FABS(10.0*MvarBsctNumerTol))) {
#       ifdef DEBUG_VORONOI
            printf("%f,%f is a flat corner.\n", UMin, VMin);
#       endif
	UMinNew = UMin + FlatCornerTrimParam*(UMax - UMin);
	VMinNew = VMin + FlatCornerTrimParam*(VMax - VMin);
	HasFlatCorner = TRUE;
            
	UVBbox.Min[0] = UMin;
	UVBbox.Max[0] = UMinNew;
	UVBbox.Min[1] = VMin;
	UVBbox.Max[1] = VMinNew;

	Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);

	PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, UMinNew,
							CAGD_CONST_U_DIR, 
							MvarBsctSubdivTol, 
						       -IRIT_FABS(MvarBsctNumerTol),
							TRUE);
	if (PtZerosLst == NULL) {
	    /* Check VMinNew and trim there. */
	    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, VMinNew,
						       CAGD_CONST_V_DIR, 
						       MvarBsctSubdivTol, 
						       -IRIT_FABS(MvarBsctNumerTol),
						       TRUE);
	    if (PtZerosLst == NULL)
	        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

	    if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[0], UMinNew,
			    IRIT_FABS(10.0*MvarBsctNumerTol))) {
	        UMinNew = PtZerosLst -> Pt[0];
		UVBbox.Max[0] = PtZerosLst -> Pt[0];

		CagdSrfFree(Srf1);
		Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
	    }
	}
	else if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[1], VMinNew,
			     IRIT_FABS(10.0 * MvarBsctNumerTol))) {
	    VMinNew = PtZerosLst -> Pt[1];
	    UVBbox.Max[1] = PtZerosLst -> Pt[1];

	    CagdSrfFree(Srf1);
	    Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
	}

	IRIT_LIST_PUSH(Srf1, *OutLst);
    }
    /* Else check for (UMin,VMax) point for flat corner. */
    else {
        /* (UMin, VMax) point. */
        F = (CagdSrfEval(Srf, UMin, VMax))[1];
        DV = (CagdSrfEval(DVSrf, UMin, VMax))[1];
        DU = (CagdSrfEval(DUSrf, UMin, VMax))[1];

        if (IRIT_APX_EQ_EPS(F, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	    IRIT_APX_EQ_EPS(DU, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	    IRIT_APX_EQ_EPS(DV, 0, IRIT_FABS(10.0*MvarBsctNumerTol))) {
#           ifdef DEBUG_VORONOI
                printf("%f,%f is a flat corner.\n", UMin, VMax);
#           endif

            UMinNew = UMin + FlatCornerTrimParam*(UMax - UMin);
	    VMaxNew = VMax - FlatCornerTrimParam*(VMax - VMin);
	    HasFlatCorner = TRUE;

	    UVBbox.Min[0] = UMin;
	    UVBbox.Max[0] = UMinNew;
	    UVBbox.Min[1] = VMaxNew;
	    UVBbox.Max[1] = VMax;

	    Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);

	    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, UMinNew,
						       CAGD_CONST_U_DIR, 
						       MvarBsctSubdivTol, 
						       -IRIT_FABS(MvarBsctNumerTol),
						       TRUE);
	    if (PtZerosLst == NULL) {
	        /* Check VMinNew and trim there. */
	        PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, VMaxNew,
						       CAGD_CONST_V_DIR, 
						       MvarBsctSubdivTol, 
						       -IRIT_FABS(MvarBsctNumerTol),
						       TRUE);
		if (PtZerosLst == NULL)
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

		if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[0], UMinNew,
				IRIT_FABS(10.0 * MvarBsctNumerTol))) {
		    UMinNew = PtZerosLst -> Pt[0];
		    UVBbox.Max[0] = PtZerosLst -> Pt[0];
 
		    CagdSrfFree(Srf1);
		    Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
		}
	    }
	    else if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[1], VMaxNew,
				 IRIT_FABS(10.0 * MvarBsctNumerTol))) {
	        VMaxNew = PtZerosLst -> Pt[1];
		UVBbox.Min[1] = PtZerosLst -> Pt[1];
 
		CagdSrfFree(Srf1);
		Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
	    }

	    IRIT_LIST_PUSH(Srf1, *OutLst);

        }
    }

    /* (UMax, VMin) point. */
    F = (CagdSrfEval(Srf, UMax, VMin))[1];
    DV = (CagdSrfEval(DVSrf, UMax, VMin))[1];
    DU = (CagdSrfEval(DUSrf, UMax, VMin))[1];

    if (IRIT_APX_EQ_EPS(F, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	IRIT_APX_EQ_EPS(DU, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	IRIT_APX_EQ_EPS(DV, 0, IRIT_FABS(10.0*MvarBsctNumerTol))) {
#       ifdef DEBUG_VORONOI
            printf("%f,%f is a flat corner.\n", UMax, VMin);
#       endif

        UMaxNew = UMax - FlatCornerTrimParam*(UMax - UMin);
	VMinNew = VMin + FlatCornerTrimParam*(VMax - VMin);
	HasFlatCorner = TRUE;

	UVBbox.Min[0] = UMaxNew;
	UVBbox.Max[0] = UMax;
	UVBbox.Min[1] = VMin;
	UVBbox.Max[1] = VMinNew;

	Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);

	PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, UMaxNew,
						    CAGD_CONST_U_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol), 
						    TRUE);
	if (PtZerosLst == NULL) {
	    /* Check VMinNew and trim there. */
	    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, VMinNew,
						    CAGD_CONST_V_DIR, 
						    MvarBsctSubdivTol, 
						    -IRIT_FABS(MvarBsctNumerTol),
						    TRUE);
	    if (PtZerosLst == NULL)
	        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

	    if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[0], UMaxNew,
			    IRIT_FABS(10.0 * MvarBsctNumerTol))) {
	        UMaxNew = PtZerosLst -> Pt[0];
		UVBbox.Min[0] = PtZerosLst -> Pt[0];
		
		CagdSrfFree(Srf1);
		Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
	    }
	}
	else if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[1], VMinNew,
			     IRIT_FABS(10.0 * MvarBsctNumerTol))) {
	    VMinNew = PtZerosLst -> Pt[1];
	    UVBbox.Max[1] = PtZerosLst -> Pt[1];
 
	    CagdSrfFree(Srf1);
	    Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
	}

	IRIT_LIST_PUSH(Srf1, *OutLst);
    }
    /* Else check for (UMax,VMax) point for flat corner. */
    else {
        /* (UMax, VMax) point. */
        F = (CagdSrfEval(Srf, UMax, VMax))[1];
        DV = (CagdSrfEval(DVSrf, UMax, VMax))[1];
        DU = (CagdSrfEval(DUSrf, UMax, VMax))[1];

        if (IRIT_APX_EQ_EPS(F, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	    IRIT_APX_EQ_EPS(DU, 0, IRIT_FABS(10.0*MvarBsctNumerTol)) &&
	    IRIT_APX_EQ_EPS(DV, 0, IRIT_FABS(10.0*MvarBsctNumerTol))) {
#           ifdef DEBUG_VORONOI
                printf("%f,%f is a flat corner.\n", UMax, VMax);
#           endif

            UMaxNew = UMax - FlatCornerTrimParam*(UMax - UMin);
	    VMaxNew = VMax - FlatCornerTrimParam*(VMax - VMin);
	    HasFlatCorner = TRUE;

	    UVBbox.Min[0] = UMaxNew;
	    UVBbox.Max[0] = UMax;
	    UVBbox.Min[1] = VMaxNew;
	    UVBbox.Max[1] = VMax;

	    Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);

	    PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, UMaxNew,
						     CAGD_CONST_U_DIR, 
						     MvarBsctSubdivTol, 
						     -IRIT_FABS(MvarBsctNumerTol), 
						     TRUE);
	    if (PtZerosLst == NULL) {
	        /* Check VMinNew and trim there. */
	        PtZerosLst = MvarBsctNewFindZeroSetOfSrfAtParam(Srf1, VMaxNew,
						       CAGD_CONST_V_DIR,
						       MvarBsctSubdivTol,
						       -IRIT_FABS(MvarBsctNumerTol),
						       TRUE);
		if (PtZerosLst != NULL)
		    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

		if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[0], UMaxNew,
				IRIT_FABS(10.0 * MvarBsctNumerTol))) {
		    UMaxNew = PtZerosLst -> Pt[0];
		    UVBbox.Min[0] = PtZerosLst -> Pt[0];

		    CagdSrfFree(Srf1);
		    Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
		}
	    }
	    else if (!IRIT_APX_EQ_EPS(PtZerosLst -> Pt[1], VMaxNew,
				 IRIT_FABS(10.0 * MvarBsctNumerTol))) {
	        VMaxNew = PtZerosLst -> Pt[1];
		UVBbox.Min[1] = PtZerosLst -> Pt[1];

		CagdSrfFree(Srf1);
		Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
	    }

	    IRIT_LIST_PUSH(Srf1, *OutLst);
        }
    }

    CagdSrfFree(DUSrf);
    CagdSrfFree(DVSrf);

    if (HasFlatCorner == TRUE) {
        /* Since we already handled the flat corners, we can continue with   */
        /* the algorithm, for the portions of the region that are outside    */
        /* the corners.  We take the portions that are not in the corners    */
        /* (see figure below) and add them to SubdivLst.                     */
        /* Portion that is between UMinNew and UMaxNew:                      */
        /* _| |_                                                             */
        /* _|1|_                                                             */
        /*  | |                                                              */
        UVBbox.Min[0] = UMinNew;
        UVBbox.Max[0] = UMaxNew;
        UVBbox.Min[1] = VMin;
        UVBbox.Max[1] = VMax;

        Srf1 = MvarBsctTrimSurfaceByUVBbox(Srf, UVBbox);
        IRIT_LIST_PUSH(Srf1, *SubdivLst);      
    }

    return HasFlatCorner;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Find the "isolated corners" (if exist) and remove them from TBLR lists.  *
*   Isolated corners are corner points of the implicit function where        *
*   the function is zero in the corner but non-zero in all its neighborhood. *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:          N.S.F.I.                                                   *
*   MvarPtStruct: N.S.F.I.                                                   *
*   LenT:         N.S.F.I.                                                   *
*   MvarPtStruct: N.S.F.I.                                                   *
*   LenB: 	  N.S.F.I.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType:  Returns TRUE if an isolated corner has been handled.         *
*****************************************************************************/
static CagdBType RemoveIsolatedCornersIfExist(CagdSrfStruct *Srf, 
                                              MvarPtStruct **T,
					      int *LenT, 
                                              MvarPtStruct **B,
					      int *LenB)
{
    CagdBType Ret = FALSE;
    CagdRType UMin, UMax, VMin, VMax;
    int ULast = Srf -> ULength - 1, VLast = Srf -> VLength - 1;
    CagdRType
	**Points = Srf -> Points;

    CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

    /* We assume corners were computed so that they can be only in T or B. */

    if ((*B) && IRIT_APX_EQ_EPS((*B) -> Pt[0], UMin,
			        IRIT_FABS(10.0 * MvarBsctNumerTol))) {
        /* (UMin, VMin) might be an isolated corner, check it.               */
        /* We compare the sign of the U and V derives at the corner points   */
        /* by looking at the control points. If both signs are the same then */
        /* the corner is isolated.                                           */
	CagdRType 
            DU = Points[1][CAGD_MESH_UV(Srf, 1, 0)],
            DV = Points[1][CAGD_MESH_UV(Srf, 0, 1)];

        if (IRIT_SIGN(DU) == IRIT_SIGN(DV)) {
            /* Remove the isolated corner and update B and LenB. */
            MvarPtStruct 
                *Tmp = (*B);

            (*B) = (*B) -> Pnext;
            MvarPtFree(Tmp);
            --(*LenB);
            Ret = TRUE;
        }
    }
    if ((*T) && IRIT_APX_EQ_EPS((*T) -> Pt[0], UMin,
			   IRIT_FABS(10.0 * MvarBsctNumerTol))) {
        /* (UMin, VMax) might be an isolated corner, check it. */
	CagdRType 
            DU = Points[1][CAGD_MESH_UV(Srf, 1, VLast)],
            DV = Points[1][CAGD_MESH_UV(Srf, 0, VLast - 1)];

        if (IRIT_SIGN(DU) == IRIT_SIGN(DV)) {
            /* Remove the isolated corner and update T and LenT. */
            MvarPtStruct 
                *Tmp = (*T);

            (*T) = (*T) -> Pnext;
            MvarPtFree(Tmp);
            --(*LenT);
            Ret = TRUE;
        }       
    }
    if ((*B) && IRIT_APX_EQ_EPS(((MvarGetLastPt(*B)) -> Pt[0]), UMax,
			   IRIT_FABS(10.0 * MvarBsctNumerTol))) {
        /* (UMax, VMin) might be an isolated corner, check it. */
	CagdRType 
	    DU = Points[1][CAGD_MESH_UV(Srf, ULast - 1, 0)],
            DV = Points[1][CAGD_MESH_UV(Srf, ULast, 1)];

        if (IRIT_SIGN(DU) == IRIT_SIGN(DV)) {
            /* Remove the isolated corner and update B and LenB. */
            MvarPtStruct 
                *Tmp = (*B);

            if (Tmp -> Pnext == NULL) {
                (*B) = NULL;
            }
            else {
                while (Tmp -> Pnext -> Pnext != NULL)
                    Tmp = Tmp -> Pnext;
            }
            MvarPtFree(Tmp -> Pnext);
            Tmp -> Pnext = NULL;
            --(*LenB);
            Ret = TRUE;
        }
    }
    if ((*T) && IRIT_APX_EQ_EPS(((MvarGetLastPt(*T)) -> Pt[0]), UMax,
			   IRIT_FABS(10.0 * MvarBsctNumerTol))) {
        /* (UMax, VMax) might be an isolated corner, check it. */
	CagdRType 
	    DU = Points[1][CAGD_MESH_UV(Srf, ULast - 1, VLast)],
            DV = Points[1][CAGD_MESH_UV(Srf, ULast, VLast - 1)];

        if (IRIT_SIGN(DU) == IRIT_SIGN(DV)) {
            /* Remove the isolated corner and update T and LenT. */
            MvarPtStruct 
                *Tmp = (*T);

            if (Tmp -> Pnext == NULL) {
                (*T) = NULL;
            }
            else {
                while (Tmp -> Pnext -> Pnext != NULL)
                    Tmp = Tmp -> Pnext;
            }
            MvarPtFree(Tmp -> Pnext);
            Tmp -> Pnext = NULL;
            --(*LenT);
            Ret = TRUE;
        }
    }

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks whether a surface has a C0 discontinuity.  			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf:   A B-Spline surface.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType: TRUE if Srf contains a C0 discontinuity.			     *
*****************************************************************************/
static CagdBType HasC0Discont(CagdSrfStruct *Srf)
{
    int OrderU = Srf -> UOrder, LengthU = Srf -> ULength,
        OrderV = Srf -> VOrder, LengthV = Srf -> VLength;
    CagdRType
 	*KVU = Srf -> UKnotVector,
 	*KVV = Srf -> VKnotVector;
    CagdRType t;

    if (!CAGD_IS_BSPLINE_SRF(Srf))
        return FALSE;

    return (BspKnotC0Discont(KVU, OrderU, LengthU, &t) ||
            BspKnotC0Discont(KVV, OrderV, LengthV, &t));
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Splits the bivariate zero set Srf=0 (implicit form) into uv-monotone     M
*   pieces. The recursive algorithm is based on the paper by Keyser et al.   M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf:   Implicit surface definition.                                      M
*   OutLst:   Output list of uv-monotone surfaces.			     M
*   MvarBsctSubdivTol:    Subdivision tolerance for multivariate solver.     M
*   MvarBsctNumerTol:     Numeric tolerance of a possible numeric improvment M
*			  stage.  The numeric stage is employed if           M
*			  MvarBsctNumerTol < MvarBsctSubdivTol.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctSplitImplicitCrvToMonotonePieces                                 M
*****************************************************************************/
void MvarBsctSplitImplicitCrvToMonotonePieces(CagdSrfStruct *Srf,
					      CagdSrfStruct **OutLst,
					      CagdRType MvarBsctSubdivTol,
					      CagdRType MvarBsctNumerTol)
{
    CagdSrfStruct 
        *Srf1, *Srf2, *Srf3, *Srf3Tmp, *Srf4, *Srf5, *TrimmedSrf1, 
        *TrimmedSrf2, *SubdivLstIter, 
        *SubdivLst = NULL;
    CagdSrfDirType Dir;
    CagdRType SubdivParam, UMin, UMax, VMin, VMax, UInf, USup, VInf, VSup;
    int i, LenUExtreme, LenVExtreme, Coord, LenT, LenB, LenL, LenR;
    MvarPtStruct *ExtremePointsIter, *T, *B, *L, *R, *ExtremePt, *Aux;
    CagdBBoxStruct Bbox1, Bbox2;
    CagdBType 
        HasKnotLine = HasC0Discont(Srf);

    if (HasKnotLine) {
        /* Check for knot lines, if exist subdivide at them. - not          */
        /* implemented yet.                                                 */
#       ifdef DEBUG_VORONOI
        printf("Srf has a knot line\n");
        /* 0. TBD. */
	printf("not implemented yet\n");
#       endif
	MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT); /* Not implemented yet. */
    }
    else if (HandleFlatCornersIfExist(Srf, OutLst, &SubdivLst, 
                                      MvarBsctSubdivTol, MvarBsctNumerTol)) {
#       ifdef DEBUG_VORONOI
        printf("Handled flat corners..\n");
#       endif
    }
    else {
        /* We have a smooth surface. Compute U, V, UV extremes. UExtreme     */
	/* holds points d/dv=0 (horizontal normal = vertical tangent).       */
        MvarPtStruct
	    *UExtreme = MvarImplicitCrvExtreme(Srf, CAGD_CONST_V_DIR,
					       MvarBsctSubdivTol,
					       MvarBsctNumerTol);

	/* VExtreme holds points d/du=0 (vertical normal = horizontal        */
	/* tangent).                                                         */
	MvarPtStruct
	    *VExtreme = MvarImplicitCrvExtreme(Srf, CAGD_CONST_U_DIR,
					       MvarBsctSubdivTol,
					       MvarBsctNumerTol);

	/* Modifies input lists - removes identical points from lists into   */
	/* new list.                                                         */
        MvarPtStruct
	    *UVSingular = MvarBsctGetUVSingularPoints(&UExtreme, &VExtreme, 
						      MvarBsctNumerTol);

	if (UVSingular != NULL)
            MVAR_FATAL_ERROR(MVAR_ERR_GEOM_NO_SUPPORT);; 
	
	/* Singular points are not handled currently. */

	LenUExtreme = CagdListLength(UExtreme);
	LenVExtreme = CagdListLength(VExtreme);
    
	CagdSrfDomain(Srf, &UMin, &UMax, &VMin, &VMax);

        /* Based on Section 8.3 of Keyser et al., there are 5 states the    */
        /* region can be in. The states and the actions taken for them      */
        /* are:                                                             */
        /* 1. The region contains multiple turning (i.e., extreme) points.  */
        /*    Subdivide until we get a single extreme point.                */
        /* 2. The region contains a single extreme point and more than two  */
        /*    edge points (i.e., points on boundary).                       */
        /*    Subdivide around the turning point until we get a region with */
        /*    a single extreme point and two edge points, and the other     */
        /*    regions contain only edge points.                             */
        /* 3. The region contains one turning point and exactly two edge    */
        /*    points.                                                       */
        /*    Connect the turning point to each edge point.                 */
        /* 4. The region contains only edge points, there are 4 groups of   */
        /*    edge points T,B,L,R (Top, Bottom, Left and Right). Based on   */
        /*    the number of points in each group, if |T-B|=L+R or |L-R|=T+B,*/
        /*    then we can easily connect the points (see paper, this is     */
        /*    implemented in the MateTBLRPoints function).                  */
        /* 5. The region contains only edge points, but |T-B|!=L+R and      */
        /*    |L-R|!=T+B.                                                   */
        /*    Subdivide until state 4 is achieved.                          */

	if (LenUExtreme + LenVExtreme > 1) {
	    /* 1. Subdivide at a mid point until we get a single            */
	    /* extreme point. Append LenUExtreme and LenVExtreme, sort      */
	    /* and take a mid param for subdivision.                        */
	    MvarPtStruct 
	      *ExtremePoints = (MvarPtStruct *)
	                            CagdListAppend(UExtreme, VExtreme);
	    
	    /* The following is just a heauristic to get "fatter" boxes.    */
	    if (UMax - UMin > VMax - VMin) {
	        ExtremePoints =  MvarPtSortListAxis(ExtremePoints, 1);
		Dir = CAGD_CONST_U_DIR;
	    }
	    else {
	        ExtremePoints = MvarPtSortListAxis(ExtremePoints, 2);
		Dir = CAGD_CONST_V_DIR;
	    }

	    ExtremePointsIter = ExtremePoints;
	    for (i=1; i < (LenUExtreme+LenVExtreme)/2; ++i)
	        ExtremePointsIter = ExtremePointsIter -> Pnext;
	    Coord = (Dir == CAGD_CONST_U_DIR) ? 0 : 1;
	    SubdivParam = 0.5*(ExtremePointsIter -> Pt[Coord] + 
			       ExtremePointsIter -> Pnext -> Pt[Coord]);
      
	    /* Subdivide and free ExtremePoints (== appended UExtreme and   */
	    /* VExtreme).                                                   */
	    SubdivLst = CagdSrfSubdivAtParam(Srf, SubdivParam, Dir);
	    MvarPtFreeList(ExtremePoints);
	}
	else {
	    /* We have a single extreme point or none. Compute edge points.  */
            /* NOTE: We ensure that a corner point will always belong to T   */
	    /* or B and not to L or R (so it won't be counted twice). This   */
            /* is done by calling MvarBsctNewFindZeroSetOfSrfAtParam with    */
            /* ShouldCheckEndpoints==TRUE for T and B, and FALSE otherwise.  */
						
#           ifdef DEBUG_VORONOI
	    static int
		Dbg = 0;

	        ++Dbg;
	        printf("dbg=%d\n", Dbg);
#           endif
	    
	    T = MvarBsctNewFindZeroSetOfSrfAtParam(Srf, VMax, 
						   CAGD_CONST_V_DIR, 
						   MvarBsctSubdivTol, 
						   MvarBsctNumerTol, TRUE);
	    LenT = CagdListLength(T);

	    B = MvarBsctNewFindZeroSetOfSrfAtParam(Srf, VMin, 
						   CAGD_CONST_V_DIR, 
						   MvarBsctSubdivTol, 
						   MvarBsctNumerTol, TRUE);
	    LenB = CagdListLength(B);

	    L = MvarBsctNewFindZeroSetOfSrfAtParam(Srf, UMin, 
						   CAGD_CONST_U_DIR, 
						   MvarBsctSubdivTol, 
						   MvarBsctNumerTol, FALSE);
	    LenL = CagdListLength(L);
	    
	    R = MvarBsctNewFindZeroSetOfSrfAtParam(Srf, UMax, 
						   CAGD_CONST_U_DIR, 
						   MvarBsctSubdivTol, 
						   MvarBsctNumerTol, FALSE);
	    LenR = CagdListLength(R);

            /* Consistency check.*/
            if ((LenT+LenB+LenL+LenR)%2 != 0) {
#               ifdef DEBUG_VORONOI
	            printf("Edge points uneven - something is wrong.\n");
#               endif
		/* The most common reason for uneven points is isolated      */
		/* corners - handle them.                                    */
                RemoveIsolatedCornersIfExist(Srf, &T, &LenT, &B, &LenB); 

                /* Check if removing isolated corners helped.*/
                if ((LenT + LenB + LenL + LenR) % 2 != 0)
                    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
            }

	    ExtremePt = NULL;

	    if (LenUExtreme + LenVExtreme == 1) {
#               ifdef DEBUG_VORONOI
	            printf("one extreme point\n"); 
#               endif
	        /* If there are just 2 edge points we are done, otherwise    */
	        /* subdivide.                                                */
	        if (LenT + LenB + LenL + LenR > 2) {
#                   ifdef DEBUG_VORONOI
		    printf("more than two edge points, one extreme point - subdivide\n");
#                   endif
		    /* 2. More than two edge points with one extreme point - */
		    /*   find sufficient parameter for subdivision.          */
		    ExtremePt = (LenUExtreme == 1) ? UExtreme : VExtreme;

		    Aux = (MvarPtStruct *) CagdListAppend(T, B);
		    Aux = (MvarPtStruct *) CagdListAppend(Aux, L);
		    Aux = (MvarPtStruct *) CagdListAppend(Aux, R);

		    UInf = UMin;
		    USup = UMax;
		    VInf = VMin;
		    VSup = VMax;                 /* Input and output params. */
		    MvarBsctFindClosestParamsAroundPoint(ExtremePt, Aux, 
							 &UInf, 
							 &USup, &VInf, &VSup);
		    
		    if (IRIT_APX_EQ_EPS(UInf, ExtremePt -> Pt[0],
				   IRIT_FABS(10.0*MvarBsctNumerTol)))
		        UInf = UMin;
		    else
		        UInf = 0.5*(UInf+ExtremePt -> Pt[0]);
		    if (IRIT_APX_EQ_EPS(USup, ExtremePt -> Pt[0],
				   IRIT_FABS(10.0*MvarBsctNumerTol)))
		        USup = UMax;
		    else
		        USup = 0.5*(USup+ExtremePt -> Pt[0]);

		    if (IRIT_APX_EQ_EPS(VInf, ExtremePt -> Pt[1],
				   IRIT_FABS(10.0*MvarBsctNumerTol)))
		        VInf = VMin;
		    else
		        VInf = 0.5*(VInf+ExtremePt -> Pt[1]);
		    if (IRIT_APX_EQ_EPS(VSup, ExtremePt -> Pt[1],
				   IRIT_FABS(10.0*MvarBsctNumerTol)))
		        VSup = VMax;
		    else
		        VSup = 0.5*(VSup+ExtremePt -> Pt[1]);

		    /* Avoiding infinite loop. */
		    if (!(UInf != UMin ||
			  USup != UMax ||
			  VInf != VMin  ||
			  VSup != VMax))
		        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

		    /* Subdivide through mid points with closest params.     */
		    /* (5 resulting subdiv surfaces).                        */
		    /* |_5|                                                  */
		    /* 1|_3|4_                                               */
		    /* | 2                                                   */
          
		    Srf1 = CagdSrfSubdivAtParam(Srf, UInf, CAGD_CONST_U_DIR); 
		    Srf2 = CagdSrfSubdivAtParam(Srf1 -> Pnext,
						VInf, CAGD_CONST_V_DIR); 
		    CagdSrfFree(Srf1 -> Pnext);
		    Srf1 -> Pnext = NULL;
		    Srf3Tmp = CagdSrfSubdivAtParam(Srf2 -> Pnext,
						   USup, CAGD_CONST_U_DIR); 
		    Srf4 = Srf3Tmp -> Pnext;
		    Srf3Tmp -> Pnext = NULL;
		    CagdSrfFree(Srf2 -> Pnext);
		    Srf2 -> Pnext = NULL;
		    Srf3 = CagdSrfSubdivAtParam(Srf3Tmp,
						VSup, CAGD_CONST_V_DIR); 
		    Srf5 = Srf3 -> Pnext;
		    CagdSrfFree(Srf3Tmp);
		    Srf3 -> Pnext = NULL;

		    if (UMin < UInf) {
		        IRIT_LIST_PUSH(Srf1, SubdivLst);
		    }
		    else
		        CagdSrfFree(Srf1);

		    if (VMin < VInf) {
		        IRIT_LIST_PUSH(Srf2, SubdivLst);
		    }
		    else
		        CagdSrfFree(Srf2);

		    IRIT_LIST_PUSH(Srf3, SubdivLst);

		    if (USup < UMax) { 
		        IRIT_LIST_PUSH(Srf4, SubdivLst);
		    }
		    else
		        CagdSrfFree(Srf4);

		    if (VSup < VMax) {
		        IRIT_LIST_PUSH(Srf5, SubdivLst);
		    }
		    else
		        CagdSrfFree(Srf5);

		    /* Free Aux and extreme (==TBLR and UExtreme, VExtreme) */
		    /* and return.                                          */
		    MvarPtFree(ExtremePt);
		    MvarPtFreeList(Aux);          
		}
		else {
                    /* Sanity check. */
		    if (LenT + LenB + LenL + LenR != 2)
                        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

		    /* We are done, subdivide into two monotone subregions, */
		    /* with corner interpolation and free the rest.         */
#                   ifdef DEBUG_VORONOI
		        printf("Two edge points, one extreme point - we're done\n");
#                   endif
		    /* 3. Subdivision of region to two boxes and return! */

		    Aux = (MvarPtStruct *) CagdListAppend(T, B);
		    Aux = (MvarPtStruct *) CagdListAppend(Aux, L);
		    Aux = (MvarPtStruct *) CagdListAppend(Aux, R);

		    ExtremePt = (LenUExtreme == 1) ? UExtreme : VExtreme;

		    Bbox1.Min[0] = IRIT_MIN(Aux -> Pt[0], ExtremePt -> Pt[0]);
		    Bbox1.Min[1] = IRIT_MIN(Aux -> Pt[1], ExtremePt -> Pt[1]);
		    Bbox1.Max[0] = IRIT_MAX(Aux -> Pt[0], ExtremePt -> Pt[0]);
		    Bbox1.Max[1] = IRIT_MAX(Aux -> Pt[1], ExtremePt -> Pt[1]);

		    Bbox2.Min[0] = IRIT_MIN(ExtremePt -> Pt[0], 
				       Aux -> Pnext -> Pt[0]);
		    Bbox2.Min[1] = IRIT_MIN(ExtremePt -> Pt[1], 
				       Aux -> Pnext -> Pt[1]);
		    Bbox2.Max[0] = IRIT_MAX(ExtremePt -> Pt[0], 
				       Aux -> Pnext -> Pt[0]);
		    Bbox2.Max[1] = IRIT_MAX(ExtremePt -> Pt[1], 
				       Aux -> Pnext -> Pt[1]);

		    TrimmedSrf1 = MvarBsctTrimSurfaceByUVBbox(Srf, Bbox1);
		    IRIT_LIST_PUSH(TrimmedSrf1, (*OutLst));
		    TrimmedSrf2 = MvarBsctTrimSurfaceByUVBbox(Srf, Bbox2);
		    IRIT_LIST_PUSH(TrimmedSrf2, (*OutLst));

		    /* Free Aux and extreme (==TBLR and UExtreme, VExtreme) */
		    /* and return.                                          */
		    MvarPtFree(ExtremePt);
		    MvarPtFreeList(Aux);
		}
	    }
	    else {
                /* Sanity check. */
	        if (LenUExtreme + LenVExtreme != 0)
                    MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
			
		if ((abs(LenT - LenB) == LenL + LenR) ||
		    (abs(LenL - LenR) == LenT + LenB)) {
		    /* We are done, subdivide into all the subregions, with */
		    /* corner interpolation and free the rest.              */
#                   ifdef DEBUG_VORONOI
		        printf("(abs(LenT-LenB) == LenL+LenR) or the opposite, we're done\n");
#                   endif
		    /* 4. Mate pairs, subdivide into subregions and return. */
		    MvarBsctMateTBLRPoints(Srf, OutLst, T, LenT, B, LenB,
					   L, LenL, R, LenR); 
                    MvarPtFreeList(T);
                    MvarPtFreeList(B);
                    MvarPtFreeList(L);
                    MvarPtFreeList(R);
		    return;
		}
		else {
                    CagdRType SubdivParamU, SubdivParamV,
			LargestGapU, LargestGapV;

#                   ifdef DEBUG_VORONOI
		        printf("LenT = %d, LenB = %d, LenL = %d, LenR = %d\n",
			       LenT, LenB, LenL, LenR);
		        printf("(abs(LenT-LenB) < LenL+LenR) or similar - subdivide\n");
#                   endif

                    /* 5. Subdivide at largest gap (horizontal or vertical). */
                    assert(LenT + LenB >= 1);
		    SubdivParamU = MvarBsctFindMidParam(T, LenT, B, LenB, 
							CAGD_CONST_U_DIR, 
							&LargestGapU);
                    assert(LenL + LenR >= 1);
		    SubdivParamV = MvarBsctFindMidParam(L, LenL, R, LenR, 
							CAGD_CONST_V_DIR, 
							&LargestGapV);
                    if (LargestGapU > LargestGapV) {
                            assert(LargestGapU > 1000.0*MvarBsctNumerTol);
                            SubdivLst = CagdSrfSubdivAtParam(Srf, SubdivParamU,
							     CAGD_CONST_U_DIR);
                    }
                    else {
                            assert(LargestGapV > 1000.0*MvarBsctNumerTol);
                            SubdivLst = CagdSrfSubdivAtParam(Srf, SubdivParamV,
							     CAGD_CONST_V_DIR);
                    }
          
                  MvarPtFreeList(T);
                  MvarPtFreeList(B);
                  MvarPtFreeList(L);
                  MvarPtFreeList(R);
		}
	    }
	}
	/* Freeing UExtreme and VExtreme crashes the code. */
    }
    /* Perform recursion on the subdivided list (and free it). */
    SubdivLstIter = SubdivLst;
    for (; SubdivLstIter != NULL; SubdivLstIter = SubdivLstIter -> Pnext) {
        MvarBsctSplitImplicitCrvToMonotonePieces(SubdivLstIter,  OutLst, 
						 MvarBsctSubdivTol, 
						 MvarBsctNumerTol);
    }
    CagdSrfFreeList(SubdivLst);
}
