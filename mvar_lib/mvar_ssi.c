/*****************************************************************************
* Computes intersection curve of two surfaces				     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Michael Barton				Ver 1.0, April 2008  *
*****************************************************************************/

#include "irit_sm.h"
#include "iritprsr.h"
#include "allocate.h"
#include "attribut.h"
#include "cagd_lib.h"
#include "geom_lib.h"
#include "grap_lib.h"
#include "mvar_loc.h"

#define MVAR_SSI_EPS_WED_PROD	    1e-6
#define MVAR_SSI_END_TOL_FACTOR	    2
#define MVAR_SSI_SUBD_PRTRB	    1.301060e-2

/* Auxiliary structure for SSI computation.  */
typedef struct MvarSSIAuxStruct {           
    MvarVecStruct **OrthoBasis;
    MvarVecStruct *TempVec;
    MvarVecStruct *CorrVec;
    CagdRType *MinDmn;
    CagdRType *MaxDmn;
    MvarVecStruct **GradVecs;
    MvarVecStruct *SITTempVec;/* SIT = StepInTangentVector+CorrStep related.*/
    MvarVecStruct *SITTanVec;
    CagdRType *A; 
    CagdRType *x;
    CagdRType *b;
    CagdRType *bCopy;
    MvarVecStruct **TempList; 
    MvarConstraintType *Constraints;  
} MvarSSIAuxStruct;

IRIT_STATIC_DATA int
    GlblMvarInterMergeSingularPts = TRUE;

static MvarPolyStruct *MvarSSIMergeTwoPoly(MvarPolyStruct *Poly1, 
					   MvarPolyStruct *Poly2, 
					   CagdRType Tol);
static int MvarSSIPolyPtOnBound(MvarPolyStruct *Poly,
				CagdRType SubdivTol, 
				CagdRType NumericTol, 
				int BoundarySide, 
				CagdRType BoundaryValue);
static int MvarPtCmpTwoPointsSSI(const MvarPtStruct *P1,
				 const MvarPtStruct *P2,
				 CagdRType SubdivTol, 
				 CagdRType NumericTol,
				 CagdRType *UsedTol);
static MvarPolyStruct *MvarSSIPrelimLink(MvarPolyStruct **PolyList, 
					 MvarPolyStruct *Poly,
					 CagdRType SubdivTol, 
					 CagdRType NumericTol,
					 CagdRType *UsedTol);
static MvarPolyStruct *MvarSSILinkNeighbours(MvarPolyStruct *PolyList1, 
					     MvarPolyStruct *PolyList2, 
					     CagdRType SubdivTol, 
					     CagdRType NumericTol, 
					     int BoundarySide, 
					     CagdRType BoundaryValue);
static MvarPolyStruct *MvarSSISubdToInterCrv(MvarMVStruct * const *MVs, 
					     CagdRType Step, 
					     CagdRType SubdivTol,
					     CagdRType NumericTol,
					     MvarMVGradientStruct **MVGrads,
					     MvarSSIAuxStruct *AS,
					     MvarPtStruct *BoundaryPts);
static int MvarSSIHasC1Discont(MvarMVStruct * const *MVs,
			       int *JLoc,
			       CagdRType *t);
static MvarPolyStruct *MvarSSISubdToC1Cont(MvarMVStruct * const *MVs, 
					   CagdRType Step, 
					   CagdRType SubdivTol,
					   CagdRType NumericTol,
					   MvarSSIAuxStruct *AS);
static MvarMVStruct **MvarSSICreateMVs(const CagdSrfStruct *Srf1, 
				       const CagdSrfStruct *Srf2);
static MvarSSIAuxStruct *MvarSSIAllocateOnce(int Number);
static void MvarSSIDeallocateOnce(MvarSSIAuxStruct *Struct, int Number);
static MvarPtStruct *MvarSSIListOfDifferentPts(MvarPtStruct *PtList, 
					       CagdRType Tol);
static int MvarSSINoRoot(const MvarMVStruct *MV, CagdRType Tol);
static int MvarSSILinDep(MvarVecStruct **Vectors,
			 int Size, 
			 MvarSSIAuxStruct *AS);
static MvarVecStruct *MvarSSIWedgeProduct(MvarVecStruct **Vectors,
					  int Size, 
					  MvarSSIAuxStruct *AS);
static MvarPtStruct *MvarSSIStepInTangDir(MvarMVStruct * const *MVs,  
					  MvarPtStruct *StartPoint,
					  MvarVecStruct *DirVec,
					  CagdRType Step,
					  MvarMVGradientStruct **MVGrads,
					  MvarSSIAuxStruct *AS);
static MvarPtStruct *MvarSSICorrectionStep(MvarMVStruct * const *MVs,  
					   MvarPtStruct *StartPoint, 
					   MvarMVGradientStruct **MVGrads,
					   MvarSSIAuxStruct *AS);
static int MvarSSICloseToIntersCrv(MvarMVStruct * const *MVs,
				   int Size, 
				   const MvarPtStruct *Point,
				   CagdRType Tol);
static CagdRType MvarSSIEvalError(MvarMVStruct * const *MVs,
				  int Size, 
				  const MvarPtStruct *Pt);
static MvarPtStruct *MvarSSICurveTracing(MvarMVStruct * const *MVs,  
					 MvarPtStruct *StartPoint,
					 MvarPtStruct *EndPoint,
					 MvarVecStruct *DirVec,
					 CagdRType Step, 
					 CagdRType NumericTol,
					 MvarMVGradientStruct **MVGrads,
					 MvarSSIAuxStruct *AS);
static MvarPtStruct *MvarSSIStartEndPts(MvarMVStruct * const *MVs,
					MvarSSIAuxStruct *AS,
					CagdRType SubdivTol,
					CagdRType NumericTol);
static MvarPtStruct *MvarSSIMiddlePlaneCutPts(MvarMVStruct * const *MVs,
					      MvarSSIAuxStruct *AS,
					      CagdRType SubdivTol,
					      CagdRType NumericTol,
					      int BoundarySide, 
					      CagdRType BoundaryValue);
static void MvarSSISplitBoundaryPts(const MvarPtStruct *BoundaryPts,
				    int BoundarySide, 
				    CagdRType BoundaryValue,
				    MvarPtStruct **SplitPts0,
				    MvarPtStruct **SplitPts1);
static MvarVecStruct *MvarSSIStartingVec(MvarMVStruct * const *MVs, 
					 MvarPtStruct *BoundaryPt,
					 MvarSSIAuxStruct *AS,
					 CagdRType NumericTol);
static int MvarSSINoLoopTest(MvarMVStruct * const *MVs,
			     MvarSSIAuxStruct *AS);
static MvarPolyStruct *MvarSSIPolyWithDom(MvarPolyStruct *Poly, 
					  const MvarMVStruct *MV);

#ifdef DEBUG_MVAR_SSI_LINK_NEIGH
static void MvarDbgSSIPrintEndPtPlList(MvarPolyStruct *PolyList);
static void MvarDbgSSIExamineSegmentBndry(MvarPtStruct *PtList,
					  const MvarMVStruct *MV);
#endif /* DEBUG_MVAR_SSI_LINK_NEIGH */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges two neighboring polylines together, in place.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly1:      1st polyline.						     *
*   Poly2:	2nd polyline.						     *
*   Tol:	The tolerance under which two points are proclaimed to be    *
*		identical.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPolyStruct *:   If the start/endpoint of Poly1 coincides with the    *
*		        start/endpoint of Poly2, up to Tol, they are merged. *
*****************************************************************************/
static MvarPolyStruct *MvarSSIMergeTwoPoly(MvarPolyStruct *Poly1, 
					   MvarPolyStruct *Poly2, 
					   CagdRType Tol)
{
    MvarPtStruct
        *Pt1Last = CagdListLast(Poly1 -> Pl),
	*Pt2Last = CagdListLast(Poly2 -> Pl);

    if (!MvarPtCmpTwoPoints(Poly1 -> Pl, Poly2 -> Pl, Tol)) {
	Poly1 -> Pl = CagdListReverse(Poly1 -> Pl);
    }
    else if (!MvarPtCmpTwoPoints(Poly1 -> Pl, Pt2Last, Tol)) {   
        IRIT_SWAP(MvarPolyStruct *, Poly1, Poly2);
    } 
    else if (!MvarPtCmpTwoPoints(Pt1Last, Poly2 -> Pl, Tol)) {
    }
    else if (!MvarPtCmpTwoPoints(Pt1Last, Pt2Last, Tol)) { 
	Poly2 -> Pl = CagdListReverse(Poly2 -> Pl);
    }
    else {
#	ifdef DEBUG
	    fprintf(stderr, "SSI: Polylines can not be merged.\n");
#	endif /* DEBUG */	

	return NULL;
    }

    Pt1Last = CagdListLast(Poly1 -> Pl);
    if (MvarPtCmpTwoPoints(Pt1Last, Poly2 -> Pl, IRIT_EPS) == 0) {
	Poly1 -> Pl = CagdListAppend(Poly1 -> Pl, Poly2 -> Pl -> Pnext);
	Poly2 -> Pl -> Pnext = NULL;
    }
    else {
        Poly1 -> Pl = CagdListAppend(Poly1 -> Pl, Poly2 -> Pl);
	Poly2 -> Pl = NULL;
    }

    MvarPolyFree(Poly2);

    return Poly1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Classifies the mutual position of polyline and the boundary (of	     *
* its domain).								     *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:	    A polyline to classify.				     *
*   SubdivTol, NumericTol: The tolerances under which two points are         *
*                   proclaimed to be identical.				     *
*   BoundarySide:   The side of the domain.				     *
*   BoundaryValue:  The value at the boundary.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:	    0	neither startpoint nor endpoint of polyline lies     *
*			on the boundary.				     *
*		    1	startpoint lies on the boundary, endpoint not.	     *
*		    2	endpoint lies on the boundary, startpoint not.	     *
*		    3	startpoint and endpoint both lie on the boundary.    *
*****************************************************************************/
static int MvarSSIPolyPtOnBound(MvarPolyStruct *Poly,
				CagdRType SubdivTol, 
				CagdRType NumericTol, 
				int BoundarySide,
				CagdRType BoundaryValue)
{  
    MvarPtStruct
        *TempPt = CagdListLast(Poly -> Pl);
    int MidPt1 = AttrGetIntAttrib(Poly -> Pl -> Attr, "_MidSSIPt") == TRUE,
        MidPt2 = AttrGetIntAttrib(TempPt -> Attr, "_MidSSIPt") == TRUE;
    CagdRType
        Tol1 = MidPt1 ? SubdivTol : NumericTol,
        Tol2 = MidPt2 ? SubdivTol : NumericTol;

    return (IRIT_APX_EQ_EPS(Poly -> Pl -> Pt[BoundarySide],
			    BoundaryValue, Tol1) ? 1 : 0) +
           (IRIT_APX_EQ_EPS(TempPt -> Pt[BoundarySide],
			    BoundaryValue, Tol2) ? 2 : 0);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A comparison function to examine if the given two points are the same.   *
* This special SSI version is due to single points that we might merge at    *
* subdivision tolerance if we stop at subdiv. tol.                           *
*									     *
* PARAMETERS:                                                                *
*   P1, P2:         Two multivariate points to compare.	                     *
*   SubdivTol, NumericTol:   The tolerance of the comparison.                *
*   UsedTol:        The actual tolerance used in the comparison (Single mid  *
*                   points use SubdivTol while normally NumericTol is used). *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      0 if identical, -1 or +1 if first point is less than/greater   *
*	      than second point, in lexicographic order over dimensions.     *
*****************************************************************************/
static int MvarPtCmpTwoPointsSSI(const MvarPtStruct *P1,
				 const MvarPtStruct *P2,
				 CagdRType SubdivTol, 
				 CagdRType NumericTol,
				 CagdRType *UsedTol)
{
    int i,
        Dim = P1 -> Dim,
        MidPt1 = AttrGetIntAttrib(P1 -> Attr, "_MidSSIPt") == TRUE,
        MidPt2 = AttrGetIntAttrib(P2 -> Attr, "_MidSSIPt") == TRUE;
    CagdRType
        Tol = MidPt1 || MidPt2 ? SubdivTol : NumericTol;

    *UsedTol = Tol;

    if (Dim != P2 -> Dim)
	return FALSE;

    for (i = 0; i < Dim; i++) {
	if (!IRIT_APX_EQ_EPS(P1 -> Pt[i], P2 -> Pt[i], Tol))
	    return IRIT_SIGN(P1 -> Pt[i] - P2 -> Pt[i]);
    }

    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Finds a polyline in the list, which is to be merged to a given polyline  *
* and remove it from the input list.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   PolyList:	    List of polylines (found poly is removed from).	     *
*   Poly:	    Polyline, to be merged with the one from the list.	     *
*   SubdivTol, NumericTol: The tolerances under which two points are         *
*		    proclaimed to be identical.				     *
*   UsedTol:        The actual tolerance used in the comparison (Single mid  *
*                   points use SubdivTol while normally NumericTol is used). *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPolyStruct *:	Polyline which is suitable for Poly to be merged,    *
*			NULL, if no such polyline exist.		     *
*****************************************************************************/
static MvarPolyStruct *MvarSSIPrelimLink(MvarPolyStruct **PolyList, 
					 MvarPolyStruct *Poly,
					 CagdRType SubdivTol, 
					 CagdRType NumericTol,
					 CagdRType *UsedTol)
{
    MvarPolyStruct
        *PrevPoly = NULL,
        *TempPoly = *PolyList;
    MvarPtStruct
        *PtLast = CagdListLast(Poly -> Pl);

    while (TempPoly != NULL) {
        MvarPtStruct
	    *PtTempLast = CagdListLast(TempPoly -> Pl);

	if (!MvarPtCmpTwoPointsSSI(TempPoly -> Pl, Poly -> Pl,
				   SubdivTol, NumericTol, UsedTol) ||
	    !MvarPtCmpTwoPointsSSI(TempPoly -> Pl, PtLast,
				   SubdivTol, NumericTol, UsedTol) ||
	    !MvarPtCmpTwoPointsSSI(PtTempLast, Poly -> Pl,
				   SubdivTol, NumericTol, UsedTol) ||
	    !MvarPtCmpTwoPointsSSI(PtTempLast, PtLast,
				   SubdivTol, NumericTol, UsedTol)) {
	    if (PrevPoly == NULL)
	        *PolyList = (*PolyList) -> Pnext;
	    else
	        PrevPoly -> Pnext = TempPoly -> Pnext;

	    return TempPoly;
	}
	PrevPoly = TempPoly;
	TempPoly = TempPoly -> Pnext;
    }

    return NULL;
}

#ifdef DEBUG_MVAR_SSI_LINK_NEIGH

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Print the end points of a list of polylines.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   PolyList:	    List of polylines.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void MvarDbgSSIPrintEndPtPlList(MvarPolyStruct *PolyList)
{
    MvarPolyStruct *Pl;

    for (Pl = PolyList; Pl != NULL; Pl = Pl -> Pnext) {
        int i;
        MvarPtStruct
	    *Pt1 = Pl -> Pl,
	    *Pt2 = CagdListLast(Pt1);

	fprintf(stderr, "Plln from:");
	for (i = 0; i < Pt1 -> Dim; i++)
	    fprintf(stderr, " %12.10f", Pt1 -> Pt[i]);
	fprintf(stderr, " to:");
	for (i = 0; i < Pt2 -> Dim; i++)
	    fprintf(stderr, " %12.10f", Pt2 -> Pt[i]);
	fprintf(stderr, "\n");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verify the given segment is closed or on MV's boundary.                  *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:	    One traced segment to examine...			     *
*   MV:             If PtList starting/ending on MV boundary or closed.      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void		                                                     *
*****************************************************************************/
static void MvarDbgSSIExamineSegmentBndry(MvarPtStruct *PtList,
					  const MvarMVStruct *MV)
{
    int i, j;
    CagdRType t1, t2;
    MvarPtStruct
        *LastPt = CagdListLast(PtList);

    for (i = 0; i < MV -> Dim; i++) {
        MvarMVDomain(MV, &t1, &t2, i);
	if (IRIT_APX_EQ(PtList -> Pt[i], t1) ||
	    IRIT_APX_EQ(PtList -> Pt[i], t2))
	    break; /* Starting point is on boundary. */
    }

    for (j = 0; j < MV -> Dim; j++) {
        MvarMVDomain(MV, &t1, &t2, j);
	if (IRIT_APX_EQ(LastPt -> Pt[j], t1) ||
	    IRIT_APX_EQ(LastPt -> Pt[j], t2))
	    break; /* End point is on boundary. */
    }

    if (i >= MV -> Dim || j >= MV -> Dim) {
        fprintf(stderr,
		"Incomplete segment created (not on boundary), %d points.\n", 
		CagdListLength(PtList));

	fprintf(stderr, "Plln from:");
	for (i = 0; i < PtList -> Dim; i++)
	    fprintf(stderr, " %6.3f", PtList -> Pt[i]);
	fprintf(stderr, " to:");
	for (i = 0; i < LastPt -> Dim; i++)
	    fprintf(stderr, " %6.3f", LastPt -> Pt[i]);

	fprintf(stderr, "\nMV Domain:");
	for (i = 0; i < MV -> Dim; i++) {
	    MvarMVDomain(MV, &t1, &t2, i);
	    fprintf(stderr, " [%6.3f, %6.3f]", t1, t2);
	}
	fprintf(stderr, "\n");
    }
}

#endif /* DEBUG_MVAR_SSI_LINK_NEIGH */

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Merges two neighboring lists of polylines that approximate intersection  *
* curve in SSI. ( "inversion" operation to subdivision)			     *
*                                                                            *
* PARAMETERS:                                                                *
*   PolyList1:	    1st list of polylines.				     *
*   PolyList2:	    2nd list of polylines.				     *
*   SubdivTol, NumericTol: The tolerances under which two points are         *
*                   proclaimed to be identical.				     *
*   BoundarySide:   The side of the domain along which the lists are merged. *
*   BoundaryValue:  The value at the boundary.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarPolyStruct *:	A list of polylines, merged PolyList1 and PolyList2. *
*****************************************************************************/
static MvarPolyStruct *MvarSSILinkNeighbours(MvarPolyStruct *PolyList1, 
					     MvarPolyStruct *PolyList2, 
					     CagdRType SubdivTol, 
					     CagdRType NumericTol,
					     int BoundarySide, 
					     CagdRType BoundaryValue)
{
    MvarPolyStruct *Pl, *Polys,
	*PlsTmp = NULL,
        *PolysOut = NULL;

#ifdef DEBUG_MVAR_SSI_LINK_NEIGH
    if (PolyList1 != NULL && PolyList2 != NULL) {
        fprintf(stderr, "\n\nFirst list (Side = %d, Param = %f):\n",
		BoundarySide, BoundaryValue);
	MvarDbgSSIPrintEndPtPlList(PolyList1);
	fprintf(stderr, "\nSecond list:\n");
	MvarDbgSSIPrintEndPtPlList(PolyList2);
    }
#endif /* DEBUG_MVAR_SSI_LINK_NEIGH */

    Polys = CagdListAppend(PolyList1, PolyList2);
    if (PolyList1 == NULL || PolyList2 == NULL)
        return Polys;

    /* Move to out-list all polys that are not on the share boundary. */
    while (Polys != NULL) {
        IRIT_LIST_POP(Pl, Polys);

	if (MvarSSIPolyPtOnBound(Pl, SubdivTol, NumericTol,
				 BoundarySide, BoundaryValue) == 0) {
	    IRIT_LIST_PUSH(Pl, PolysOut);
	}
	else {
	    IRIT_LIST_PUSH(Pl, PlsTmp);
	}
    }
    Polys = PlsTmp;

    /* Start the merge process. */
    while (Polys != NULL) {
        CagdRType UsedTol;
	MvarPolyStruct *Pl2;

        IRIT_LIST_POP(Pl, Polys);

	if ((Pl2 = MvarSSIPrelimLink(&Polys, Pl, SubdivTol, NumericTol,
				     &UsedTol)) == NULL) {
	    IRIT_LIST_PUSH(Pl, PolysOut);
	}
	else {
	    Pl = MvarSSIMergeTwoPoly(Pl, Pl2, UsedTol);
	    IRIT_LIST_PUSH(Pl, Polys);
	}
    }

#ifdef DEBUG_MVAR_SSI_LINK_NEIGH
    if (PolyList1 != NULL && PolyList2 != NULL) {
        fprintf(stderr, "\nMerged list:\n");
	MvarDbgSSIPrintEndPtPlList(PolysOut);
    }
#endif /* EBUG_MVAR_LINK_NEIGH */

    return PolysOut;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Allocates the memory required for a new MvarSSIAuxStruct.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Number:      Number = Dim - 1 = number of constraints that describe	     *
*		 the SSI problem (4eq. with 3 unknowns)			     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarSSIAuxStruct *:    An uninitialized auxiliary structure.             *
*****************************************************************************/
static MvarSSIAuxStruct *MvarSSIAllocateOnce(int Number)
{
    int i,
        Num1 = Number + 1;
    MvarSSIAuxStruct
	*Struct = (MvarSSIAuxStruct *) IritMalloc(sizeof(MvarSSIAuxStruct));

    Struct -> OrthoBasis =
        (MvarVecStruct **) IritMalloc(sizeof(MvarVecStruct *) * Num1); 

    for (i = 0; i < Num1; i++)
        Struct -> OrthoBasis[i] = MvarVecNew(Num1);

    Struct -> TempVec = MvarVecNew(Num1);
    Struct -> CorrVec = MvarVecNew(Num1);

    Struct -> MinDmn = (CagdRType *) IritMalloc(Num1 * sizeof(CagdRType));
    Struct -> MaxDmn = (CagdRType *) IritMalloc(Num1 * sizeof(CagdRType));

    Struct -> GradVecs =
	(MvarVecStruct **) IritMalloc(sizeof(MvarVecStruct *) * Number);

    for (i = 0; i < Number; i++)
        Struct -> GradVecs[i] = MvarVecNew(Num1);

    Struct -> SITTempVec = MvarVecNew(Num1);
    Struct -> SITTanVec = MvarVecNew(Num1);

    Struct -> A = (CagdRType *) IritMalloc(IRIT_SQR(Num1) * sizeof(CagdRType));
    Struct -> x = (CagdRType *) IritMalloc(Num1 * sizeof(CagdRType));
    Struct -> b = (CagdRType *) IritMalloc(Num1 * sizeof(CagdRType));
    Struct -> bCopy = (CagdRType *) IritMalloc(Num1 * sizeof(CagdRType));

    Struct -> TempList =
        (MvarVecStruct **) IritMalloc(sizeof(MvarVecStruct *) * Num1);

    for (i = 0; i < Number; i++)
        Struct -> TempList[i] = MvarVecNew(Num1);	    	    

    Struct -> Constraints =
	(MvarConstraintType *) IritMalloc(sizeof(MvarConstraintType) * Number);

    for (i = 0; i < Number; i++)
        Struct -> Constraints[i] = MVAR_CNSTRNT_ZERO;

    return Struct;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Deallocates the memory required for a MvarSSIAuxStruct.	    	     *
*                                                                            *
* PARAMETERS:                                                                *
*   Vecs:	Structure to free.					     *
*   Number:     Number = Dim - 1 = number of constraints that describe	     *
*		the SSI problem (4equations with 3 unknowns)		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:							             *
*****************************************************************************/
static void MvarSSIDeallocateOnce(MvarSSIAuxStruct *Struct, int Number)
{
    int i;
    
    for (i = 0; i < Number+1; i++)
	MvarVecFree(Struct -> OrthoBasis[i]);

    IritFree(Struct -> OrthoBasis);

    MvarVecFree(Struct -> TempVec);
    MvarVecFree(Struct -> CorrVec);

    IritFree(Struct -> MinDmn);
    IritFree(Struct -> MaxDmn);
 
    for (i = 0; i < Number; i++)
	MvarVecFree(Struct -> GradVecs[i]);	    	    

    IritFree(Struct -> GradVecs);

    MvarVecFree(Struct -> SITTempVec);
    MvarVecFree(Struct -> SITTanVec);

    IritFree(Struct -> A);
    IritFree(Struct -> b);
    IritFree(Struct -> bCopy);
    IritFree(Struct -> x);

    for (i = 0; i < Number; i++)
	MvarVecFree(Struct -> TempList[i]);

    IritFree(Struct -> TempList);

    IritFree(Struct -> Constraints);

    IritFree(Struct);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Eliminates similar points stored in the PtList, in place.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   PtList:	List of points, to use in place.			     *
*   Tol:	The tolerance under which two points are proclaimed to be    *
*		identical						     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:	New list with mutually different points.	     *
*****************************************************************************/
static MvarPtStruct *MvarSSIListOfDifferentPts(MvarPtStruct *PtList, 
					       CagdRType Tol)
{
    MvarPtStruct *p, *q, *LastQ, *TempPt;

    for (p = PtList; p != NULL && p -> Pnext != NULL; p = p -> Pnext) {
        for (LastQ = p, q = LastQ -> Pnext; q != NULL; ) {
	    if (MvarPtCmpTwoPoints(p, q,
				   Tol * MVAR_SSI_END_TOL_FACTOR) == 0) {
	        TempPt = q;
		LastQ -> Pnext = q = TempPt -> Pnext;
		MvarPtFree(TempPt);
	    }
	    else {
		LastQ = q;
	        q = q -> Pnext;
	    }
	}
    }

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks the sign of the control points mesh.	      			     *
*                                                                            *
* PARAMETERS:                                                                *
*   MV:		Multivar structure to check its control points		     *
*   Tol:	Numerical tolerance					     *
*									     *
* RETURN VALUE:                                                              *
*   int:	TRUE if all B-spline or Bezier coefficients have the same    *
*		- nonzero - sign. If some control point is closer to the     *
*		zero level then Tol, it is assumed to be zero.		     *
*****************************************************************************/
static int MvarSSINoRoot(const MvarMVStruct *MV, CagdRType Tol)
{
    int j;
    CagdRType Start;
    const CagdRType
	*R = &MV -> Points[1][0];

    assert(MVAR_IS_BEZIER_MV(MV) || MVAR_IS_BSPLINE_MV(MV));
 
    Start = *R++;
    if (IRIT_FABS(Start) < Tol)
	return FALSE;

    for (j = 1; j < MVAR_CTL_MESH_LENGTH(MV); j++, R++) { 
        if (IRIT_FABS(*R) < Tol || Start * (*R) < 0.0)
	    return FALSE;	    
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Checks the linearly dependency of vectors.	      			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Vectors:	Set of N vectors in N-dim space.			     *
*   Size:	The size N of the set.					     *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*									     *
* RETURN VALUE:                                                              *
*   int:	TRUE if the vectors are linearly dependent		     *
*****************************************************************************/
static int MvarSSILinDep(MvarVecStruct **Vectors,
			 int Size, 
			 MvarSSIAuxStruct *AS)
{
    int i, j; 
    CagdRType c;

    MVAR_VEC_COPY(AS -> OrthoBasis[0], Vectors[0]);

    for (i = 1; i < Size; i++) {
	MVAR_VEC_RESET(AS -> CorrVec);
	for (j = 0; j < i; j++) { 
	    MVAR_VEC_COPY(AS -> TempVec, AS -> OrthoBasis[j]);
	    c = MvarVecDotProd(AS -> OrthoBasis[j], Vectors[i]) /
		MvarVecDotProd(AS -> OrthoBasis[j], AS -> OrthoBasis[j]);
	    MvarVecScale(AS -> TempVec, -c);
	    MvarVecAdd(AS -> CorrVec, AS -> CorrVec, AS -> TempVec);
	}
	MvarVecAdd(AS -> OrthoBasis[i], Vectors[i], AS -> CorrVec);

	if (MvarVecLength(AS -> OrthoBasis[i]) < MVAR_SSI_EPS_WED_PROD)
	    return TRUE;
    }
    return FALSE;
}


/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the orthogonal complement (wedge product) to the set of	     *
*   n = SizeM vectors in (Size+1)-dimensional linear space.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   Vectors:   The set of (Number+1)-dimensional vectors.		     *
*   Size:      The size of vector set.					     *
*   AS:	       Auxiliary SSI structure that holds auxiliary data.	     *
*									     *
* RETURN VALUE:                                                              *
*   MvarMVStruct *:	Wedge product.			                     *
*****************************************************************************/
static MvarVecStruct *MvarSSIWedgeProduct(MvarVecStruct **Vectors,
					  int Size, 
					  MvarSSIAuxStruct *AS) 
{
    int i, j; 
    CagdRType c;
    MvarVecStruct
       *RandomVec = MvarVecNew(Size + 1);

    MVAR_VEC_COPY(AS -> OrthoBasis[0], Vectors[0]);

    for (i = 1; i < Size; i++) {
	MVAR_VEC_RESET(AS -> CorrVec);
	for (j = 0; j < i; j++) { 
	    MVAR_VEC_COPY(AS -> TempVec, AS -> OrthoBasis[j]);
	    c = MvarVecDotProd(AS -> OrthoBasis[j], Vectors[i]) /
		MvarVecDotProd(AS -> OrthoBasis[j], AS -> OrthoBasis[j]);
	    MvarVecScale(AS -> TempVec, -c);
	    MvarVecAdd(AS -> CorrVec, AS -> CorrVec, AS -> TempVec);
	}
	MvarVecAdd(AS -> OrthoBasis[i], Vectors[i], AS -> CorrVec);

#	ifdef DEBUG
	{
	    CagdRType
	        R = MvarVecLength(AS -> OrthoBasis[i]);

	    if (R < MVAR_SSI_EPS_WED_PROD) {
	        fprintf(stderr, "WedgeProduct: Input vectors are linearly dependant.\n");
	    }
	}
#	endif /* DEBUG */
    }

#ifdef DEBUG
    for (i = 1; i < Size; i++) {
	for (j = 0; j < i; j++) {
	    CagdRType
	        R = MvarVecDotProd(AS -> OrthoBasis[i], AS -> OrthoBasis[j]);

	    assert(fabs(R) < IRIT_EPS);
	}
    }
#endif /* DEBUG */

    do {
	for (i = 0; i < Size + 1; i++)
	    RandomVec -> Vec[i] = IritRandom(-1.0, 1.0);

	MvarVecNormalize(RandomVec);
	MVAR_VEC_RESET(AS -> CorrVec);
	for (j = 0; j < Size; j++) { 
	    MVAR_VEC_COPY(AS -> TempVec, AS -> OrthoBasis[j]);
	    c = MvarVecDotProd(AS -> OrthoBasis[j], AS -> OrthoBasis[j]);
	    c = c == 0.0 ? 0.0
		         : MvarVecDotProd(AS -> OrthoBasis[j], RandomVec) / c;
	    MvarVecScale(AS -> TempVec, -c);
	    MvarVecAdd(AS -> CorrVec, AS -> CorrVec, AS -> TempVec);
	}	
	MvarVecAdd(RandomVec, RandomVec, AS -> CorrVec);
    }
    while (MvarVecLength(RandomVec) < MVAR_SSI_EPS_WED_PROD); 

#ifdef DEBUG
    for (i = 0; i < Size; i++) {
        CagdRType
	    R = MvarVecDotProd(AS -> OrthoBasis[i], RandomVec);

	assert(fabs(R) < IRIT_EPS);
    }
#endif /* DEBUG */

    return RandomVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the wedge product gradients of multivariates MVs		     *
* at StartPoint and in this direction (TanVec) goes the distance Step.       *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	Multivariate structures to compute the wedge product of      *
*		its gradients at StartPoint				     *
*   StartPoint: Multivariate point to compute tangent line at		     *
*   DirVec:     Control direction that points the same half-space like 	     *
*		the tangent vector					     *
*   Step:       NewPoint = StartPoint + Step * (Unit)TanVec		     *
*   MVGrads:	Gradients of MVs					     *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:  NewPoint that lies at the distance Step from the 	     *
*                    the original point StartPoint in the direction of the   *
*	             wedge product vector of gradients of MVS at StartPoint. *
*****************************************************************************/
static MvarPtStruct *MvarSSIStepInTangDir(MvarMVStruct * const *MVs,  
					  MvarPtStruct *StartPoint,
					  MvarVecStruct *DirVec,
					  CagdRType Step,
					  MvarMVGradientStruct **MVGrads,
					  MvarSSIAuxStruct *AS)  
{
    int j,
	Dim = MVs[0] -> Dim;
    MvarPtStruct *NewPoint;
    MvarVecStruct *TanVec;

    NewPoint = MvarPtNew(Dim);

    for (j = 0; j < Dim - 1; j++) {
	CAGD_GEN_COPY(AS -> GradVecs[j] -> Vec, 
		      MvarMVEvalGradient(MVGrads[j], StartPoint -> Pt, 0),
		      sizeof(CagdRType) * Dim);
    }		
     
    TanVec = MvarSSIWedgeProduct(AS -> GradVecs, Dim - 1, AS);
    MvarVecNormalize(TanVec);

    /* TanVec has to point into the same half-space like the DirVec. */
    MvarVecScale(TanVec, MvarVecDotProd(TanVec, DirVec) < 0.0 ? -Step : Step);

    for (j = 0; j < NewPoint -> Dim; j++)
        NewPoint -> Pt[j] = StartPoint -> Pt[j] + TanVec -> Vec[j];

    MvarVecFree(TanVec);

#   ifdef DEBUG
    if (!MvarParamsInDomain(MVs[0], NewPoint -> Pt)) {
        fprintf(stderr, "SSI: Step in the tangent direction is out of domain.\n");
    }
#   endif /* DEBUG */

    return NewPoint;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Correction step of multivariate Newton-Raphson. The intersection of	     *
* N = Dim hyperplanes is computed; Tangent hyperplanes to MVs[i]	     *
* i = 0..N-2 from the StartPoint; the last hyperplane is perpendicular       *
* to all gradient vectors of MVs[i] at StartPoint and passes through	     *
* this point.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	Multivariate structures					     *
*   StartPoint: Starting point from which we approach to the intersection    *
*		curve of MVs					  	     *
*   MVGrads:	Precomputed derivatives of multivariates MVs. In order to    *
*		speed up the computation, they are computed a priori and     * 
*		in the function are only evaluated			     *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:   NewPoint						     *
*****************************************************************************/
static MvarPtStruct *MvarSSICorrectionStep(MvarMVStruct * const *MVs,  
					   MvarPtStruct *StartPoint, 
					   MvarMVGradientStruct **MVGrads,
					   MvarSSIAuxStruct *AS)
{
    int i, j,
	Dim = MVs[0] -> Dim;
    CagdRType *R;
    MvarVecStruct *TanVec;
    MvarPtStruct *NewPoint; 

    NewPoint = MvarPtNew(Dim);
 
    for (j = 0; j < Dim - 1; j++) {
	CAGD_GEN_COPY(AS -> GradVecs[j] -> Vec, 
		      MvarMVEvalGradient(MVGrads[j], StartPoint -> Pt, 0),
		      sizeof(CagdRType) * Dim);
    }	

    TanVec = MvarSSIWedgeProduct(AS -> GradVecs, Dim - 1, AS);
    MvarVecNormalize(TanVec);

    /* Creates a Dim * Dim linear system. */
    for (i = 0; i < Dim - 1; i++) {
	IRIT_GEN_COPY(&AS -> A[i * Dim], 
		      AS -> GradVecs[i] -> Vec, Dim * sizeof(CagdRType));
    }
    IRIT_GEN_COPY(&AS -> A[(Dim - 1) * Dim], TanVec -> Vec,
		  Dim * sizeof(CagdRType));

    IRIT_GEN_COPY(AS -> SITTempVec -> Vec, 
		  StartPoint -> Pt, Dim * sizeof(CagdRType));
    
    for (i = 0; i < Dim - 1; i++) {
	R = MvarMVEval(MVs[i], StartPoint -> Pt);
	AS -> b[i] = MvarVecDotProd(AS -> SITTempVec, AS -> GradVecs[i]) -
	             R[1];
    }
    AS -> b[Dim - 1] =  MvarVecDotProd(TanVec, AS -> SITTempVec);
  
    if (IritQRUnderdetermined(AS -> A, NULL, NULL, Dim, Dim))
        return NULL;
    IritQRUnderdetermined(NULL, NewPoint -> Pt, AS -> b, Dim, Dim);

    MvarVecFree(TanVec);

    return NewPoint;  
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if the Point is "close enough" to the zero level of MVs.	     *
*   Close enough means that function value of MVs at Point is less than      *
*   given tolerance Tol for all MVs.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	Multivariate structures.				     *
*   Size:	Size of Mvars to test.					     *
*   Point:	Point, where the MVs are evaluated.			     *
*   Tol:	Numerical tolerance.				  	     *
*									     *
* RETURN VALUE:                                                              *
*   int:	TRUE if Point is "close enough".			     *
*****************************************************************************/
static int MvarSSICloseToIntersCrv(MvarMVStruct * const *MVs,
				   int Size, 
				   const MvarPtStruct *Point,
				   CagdRType Tol)
{
    int i;
    CagdRType *d;

    for (i = 0; i < Size; i++) {
	d = MvarMVEval(MVs[i], Point -> Pt); 
	if (IRIT_FABS(d[1]) > Tol)
	    return FALSE;
    }

    return TRUE;  
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluate the error at the given position.                                *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	Multivariate structures.				     *
*   Size:	Size of Mvars to test.					     *
*   Pt:         Position where to evaluate the error.                        *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdRType:   The error.                                                  *
*                                                                            *
* SEE ALSO:                                                                  *
*   MvarMVsZeros                                                             *
*                                                                            *
* KEYWORDS:                                                                  *
*   MvarSSIEvalError                                                         *
*****************************************************************************/
static CagdRType MvarSSIEvalError(MvarMVStruct * const *MVs,
				  int Size, 
				  const MvarPtStruct *Pt)
{
    int i;
    CagdRType
        Err = 0.0;

    for (i = 0; i < Size; i++) {
        CagdRType
	    *R = MvarMVEval(MVs[i], Pt -> Pt);

	Err += IRIT_FABS(R[1]);
    }

    return Err;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Traces a intersection curve of (Dim-1) multi-variables MVs in Dim space. *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	Multivariate structures.				     *
*   StartPoint:	Point, where the tracing starts.			     *
*   EndPoint:	Point, where the tracing is ended up.			     *
*   NumericTol:	Numerical tolerance.					     *
*   Step:	Distance in the direction of tangent vector.		     *
*   DirVec:     Control direction that points the same half-space like       *
*		the tangent vector					     *
*   MVGrads:	Gradients of MVs.					     *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:    List of MvarPoints that approximate the intersection  *
*		       curve.						     *
*****************************************************************************/
static MvarPtStruct *MvarSSICurveTracing(MvarMVStruct * const *MVs,  
					 MvarPtStruct *StartPoint,
					 MvarPtStruct *EndPoint,
					 MvarVecStruct *DirVec,
					 CagdRType Step, 
					 CagdRType NumericTol,
					 MvarMVGradientStruct **MVGrads,
					 MvarSSIAuxStruct *AS)
{
    int i,
	Dim = MVs[0] -> Dim;
    MvarPtStruct *TempPoint, *TanDirPoint, *CorrPoint, *PtList, *LocStart;
    MvarVecStruct *ControlVec;
    CagdRType Error, NewError,
        Tol = Step * MVAR_SSI_END_TOL_FACTOR;

    PtList = MvarPtCopy(StartPoint);

    /*	If Step > Dist(StartPoint, EndPoint), return those 2 points. */
    if (MvarPtDistSqrTwoPoints(StartPoint, EndPoint) < IRIT_SQR(Step)) { 
	TempPoint = MvarPtCopy(EndPoint);
	IRIT_LIST_PUSH(TempPoint, PtList);
	TempPoint = NULL;
	return PtList;	
    }

    LocStart = MvarPtCopy(StartPoint);
    ControlVec = MvarVecCopy(DirVec);
 
    MvarMVDomain(MVs[0], AS -> MinDmn, AS -> MaxDmn, -1);

#   ifdef DEBUG_MVAR_SSI_CRV_TRC_DMN
        fprintf(stderr, "Domain:");
	for (i = 0; i < MVs[0] -> Dim; i++) {
	    fprintf(stderr, "\t[%10.9f, %10.9f]\n",
		    AS -> MinDmn[i], AS -> MaxDmn[i]);
	}
	fprintf(stderr, "Start Pt:");
	for (i = 0; i < MVs[0] -> Dim; i++)
	    fprintf(stderr, " %10.9lf", StartPoint -> Pt[i]);
	fprintf(stderr, "\nEnd   Pt:");
	for (i = 0; i < MVs[0] -> Dim; i++)
	    fprintf(stderr, " %10.9lf", EndPoint -> Pt[i]);
	fprintf(stderr, "\n");
#   endif /* DEBUG_MVAR_SSI_CRV_TRC_DMN */

    do {
	/* Starting prediction stage - Step motion in tangent direction. */
	TanDirPoint = MvarSSIStepInTangDir(MVs, LocStart, ControlVec, 
					   Step, MVGrads, AS);

	if (MvarParamsInDomain(MVs[0], TanDirPoint -> Pt))
	    TempPoint = TanDirPoint;
	else {
	    if ((TempPoint = MvarMVIntersPtOnBndry(MVs[0], LocStart,
		                                   TanDirPoint)) == NULL) {
	        MvarPtFree(TanDirPoint);
#	        ifdef DEBUG
	            fprintf(stderr, "Failed to find boundary intersection.\n");
#	        endif /* DEBUG */
		break;
	    }
	    MvarPtFree(TanDirPoint);
	}

	if (!MvarPtCmpTwoPoints(LocStart, TempPoint, NumericTol)) {
#	    ifdef DEBUG
	        fprintf(stderr, "Could move too little in tan dir!\n");
#	    endif /* DEBUG */
	    
	    break;    
	}

	TanDirPoint = MvarPtCopy(TempPoint);
	Error = MvarSSIEvalError(MVs, Dim - 1, TempPoint);
#       ifdef DEBUG_SSI_CORRECTION_STAGE
	    fprintf(stderr, "Initial Error is %10.9f\n", Error);
#       endif /* DEBUG_SSI_CORRECTION_STAGE */

	/* Starting correction stage. */
	do {
	    if ((CorrPoint = MvarSSICorrectionStep(MVs, TempPoint,
						   MVGrads, AS)) == NULL)
	        break;

	    /* Test if the correction was successful. */
	    NewError = Error;
	    if (MvarParamsInDomain(MVs[0], CorrPoint -> Pt) &&
		(NewError = MvarSSIEvalError(MVs, Dim - 1,
					     CorrPoint)) < Error) {
#               ifdef DEBUG_SSI_CORRECTION_STAGE
	            fprintf(stderr, "    Correction Error is %10.9f\n",
			    NewError);
#               endif /* DEBUG_SSI_CORRECTION_STAGE */

		IRIT_GEN_COPY(TempPoint -> Pt, CorrPoint -> Pt, 
			      Dim * sizeof(CagdRType));	    	 
	    }
	    else {
	        MvarPtFree(CorrPoint);
		CorrPoint = MvarPtInBetweenPoint(LocStart, TanDirPoint, 0.5);

#		ifdef DEBUG_SSI_CORRECTION_STAGE
		    fprintf(stderr, "Correction step failed.\n");
#		endif /* DEBUG_SSI_CORRECTION_STAGE */

		IRIT_GEN_COPY(TempPoint -> Pt, CorrPoint -> Pt, 
			      Dim * sizeof(CagdRType));
		IRIT_GEN_COPY(TanDirPoint -> Pt, CorrPoint -> Pt, 
			      Dim * sizeof(CagdRType));
	    }
	    Error = NewError;

	    /* Cannot correct any more. */
	    if (!MvarPtCmpTwoPoints(LocStart, TempPoint, NumericTol)) {
		IRIT_GEN_COPY(TempPoint -> Pt, EndPoint -> Pt,
			      Dim * sizeof(CagdRType));

#		ifdef DEBUG
		    fprintf(stderr, "No progress in the tangent direction.");
#		endif /* DEBUG */
		
		break; 
	    }		   
	    MvarPtFree(CorrPoint);
	}
	while (!MvarSSICloseToIntersCrv(MVs, Dim - 1, 
					TempPoint, NumericTol));

	MvarPtFree(TanDirPoint);

	for (i = 0; i < Dim; i++) {
	    ControlVec -> Vec[i] = TempPoint -> Pt[i] - LocStart -> Pt[i];
	}

	IRIT_GEN_COPY(LocStart -> Pt, TempPoint -> Pt, Dim * sizeof(CagdRType));
	IRIT_LIST_PUSH(TempPoint, PtList);
    }
    while (MvarPtDistTwoPoints(TempPoint, EndPoint) >= Tol);

    MvarPtFree(LocStart);
    MvarVecFree(ControlVec);

    IRIT_GEN_COPY(PtList -> Pt, EndPoint -> Pt, Dim * sizeof(CagdRType));

#   ifdef DEBUG_MVAR_SSI_LINK_NEIGH4
    MvarDbgSSIExamineSegmentBndry(PtList, MVs[0]);
#   endif /* DEBUG_MVAR_SSI_LINK_NEIGH4 */

    return PtList;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   On each boundary hyperplane, the polynomial system of (Dim-1)	     *
* equations with (Dim-1) unknowns is solved. The intersection points	     *
* are returned.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	(Dim - 1) Multivariate structures.			     *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*   SubdivTol:	Subdivision tolerance.					     *
*   NumericTol:	Numerical tolerance.					     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:	Intersection points; list of the 0-dimensional	     *
*			solutions of the system MVs on the boundary of	     *
*			the domain.					     *
*****************************************************************************/
static MvarPtStruct *MvarSSIStartEndPts(MvarMVStruct * const *MVs,
					MvarSSIAuxStruct *AS,
					CagdRType SubdivTol,
					CagdRType NumericTol)
{
    int i, j, k, l,
	Dim = MVs[0] -> Dim;
    CagdRType *TempBound;
    MvarPtStruct *Aux,
	*StartEndPts = NULL,
        *BoundaryIntersPts = NULL;
    MvarMVStruct
	**BoundaryMVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) 
						                * (Dim - 1));
   
    MvarMVDomain(MVs[0], AS -> MinDmn, AS -> MaxDmn, -1);

    /* Twice the same for TempBound = MinDmn, MaxDmn. */
    TempBound = AS -> MinDmn;
    for (l = 0; l < 2; l++) {	
	for (i = 0; i < Dim; i++) {
	    for (j = 0; j < Dim - 1; j++)
		BoundaryMVs[j] = MvarMVFromMV(MVs[j], TempBound[i], i);

	    BoundaryIntersPts = MvarMVsZeros(BoundaryMVs, AS -> Constraints,
					     Dim - 1, SubdivTol,
					     -IRIT_FABS(NumericTol));
	 
	    /* The dimension of inters. points on the boundary is extended */
	    /* by 1 to Dim.						   */
	    for (Aux = BoundaryIntersPts; Aux != NULL; Aux = Aux -> Pnext) {
		MvarPtStruct 
		    *NewPt = MvarPtNew(Dim);

#		ifdef DEBUG_MVAR_SSI_TEST_BNDRY_PTS
		{
		    CagdRType R, S, T;

		    R = MvarMVEval(BoundaryMVs[0], Aux -> Pt)[1];
		    S = MvarMVEval(BoundaryMVs[1], Aux -> Pt)[1];
		    T = MvarMVEval(BoundaryMVs[2], Aux -> Pt)[1];
		    fprintf(stderr, "Point %.8f %.8f %.8f [%.8f %.8f %.8f]\n",
			    R, S, T, Aux -> Pt[0], Aux -> Pt[1], Aux -> Pt[2]);

		}
#		endif /* DEBUG_MVAR_SSI_TEST_BNDRY_PTS */
		
		for (k = 0; k < i; k++)
		    NewPt -> Pt[k] = Aux -> Pt[k];	    
		NewPt -> Pt[i] = TempBound[i];
		for (k = i+1; k < Dim; k++)
		    NewPt -> Pt[k] = Aux -> Pt[k-1];	    

		IRIT_LIST_PUSH(NewPt, StartEndPts);
	    } 

	    for (j = 0; j < Dim - 1; j++)
	        MvarMVFree(BoundaryMVs[j]);
	    MvarPtFreeList(BoundaryIntersPts);
	} 	
	TempBound = AS -> MaxDmn;
    }

    IritFree(BoundaryMVs);
    
    if (CagdListLength(StartEndPts) > 1)
	return MvarSSIListOfDifferentPts(StartEndPts, NumericTol);
    else
	return StartEndPts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   MVs is a system of N-1 constraints in N = Dim unknowns. In some middle   *
* (splitting) hyperplane of its domain, the well constrained system of	     *
*  (Dim-1) equations with (Dim-1) unknowns is solved.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	    (Dim - 1) Multivariate structures.			     *
*   AS:		    Auxiliary SSI structure that holds auxiliary data.	     *
*   SubdivTol:	    Subdivision tolerance.				     *
*   NumericTol:	    Numerical tolerance.				     *
*   BoundarySide:   Hyperplane direction.				     *
*   BoundaryValue:  Value, where MVs is evaluated at.			     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPtStruct *:	Intersection points; list of the 0-dimensional	     *
*			solutions of the system MVs in the middle	     *
*			hyperplane.					     *
*****************************************************************************/
static MvarPtStruct *MvarSSIMiddlePlaneCutPts(MvarMVStruct * const *MVs,
					      MvarSSIAuxStruct *AS,
					      CagdRType SubdivTol,
					      CagdRType NumericTol,
					      int BoundarySide, 
					      CagdRType BoundaryValue)
{
    int j, k,
	Dim = MVs[0] -> Dim;
    MvarPtStruct *Aux,
	*MiddlePts = NULL,
	*ExtendedMiddlePts = NULL;
    MvarMVStruct
	**MiddleMVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) 
						                * (Dim - 1));

    for (j = 0; j < Dim - 1; j++)
	MiddleMVs[j] = MvarMVFromMV(MVs[j], BoundaryValue, BoundarySide);

    MiddlePts = MvarMVsZeros(MiddleMVs, AS -> Constraints,
			     Dim - 1, SubdivTol, -IRIT_FABS(NumericTol));

    /* The dimension of middle points is extended by 1 to Dim.	           */
    for (Aux = MiddlePts; Aux != NULL; Aux = Aux -> Pnext) {
        MvarPtStruct 
	    *NewPt = MvarPtNew(Dim);

	for (k = 0; k < BoundarySide; k++)
	    NewPt -> Pt[k] = Aux -> Pt[k];	    
	NewPt -> Pt[BoundarySide] = BoundaryValue;
	for (k = BoundarySide + 1; k < Dim; k++)
	    NewPt -> Pt[k] = Aux -> Pt[k-1];	    

	IRIT_LIST_PUSH(NewPt, ExtendedMiddlePts);
    }

    for (j = 0; j < Dim - 1; j++)
	MvarMVFree(MiddleMVs[j]);
    IritFree(MiddleMVs);

    MvarPtFreeList(MiddlePts);

    if (CagdListLength(ExtendedMiddlePts) > 1)
	return MvarSSIListOfDifferentPts(ExtendedMiddlePts, NumericTol);
    else
	return ExtendedMiddlePts;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Boundary points are split into two lists. Splitting hyperplane is	     *
* perpendicular to BoundarySide axis and passes through BoundaryValue        *
* point.								     *
*                                                                            *
* PARAMETERS:                                                                *
*   BoundaryPts:           Points to split.				     *
*   BoundarySide:          Hyperplane direction.			     *
*   BoundaryValue:         Value, where MVs is evaluated at.		     *
*   SplitPts0, SplitPts1:  The two new list.				     *
*									     *
* RETURN VALUE:                                                              *
*   void			                                             *
*****************************************************************************/
static void MvarSSISplitBoundaryPts(const MvarPtStruct *BoundaryPts,
				    int BoundarySide, 
				    CagdRType BoundaryValue,
				    MvarPtStruct **SplitPts0,
				    MvarPtStruct **SplitPts1)
{
    *SplitPts0 = *SplitPts1 = NULL;

    if (BoundaryPts == NULL)
	return;
    else {
        MvarPtStruct *TempPt, *TempPt2,
	    *BoundaryPtsCp = MvarPtCopyList(BoundaryPts);

	while (BoundaryPtsCp != NULL) {
	    IRIT_LIST_POP(TempPt, BoundaryPtsCp);

	    if (IRIT_APX_EQ_EPS(TempPt -> Pt[BoundarySide], BoundaryValue,
				IRIT_UEPS)) {
	        TempPt-> Pt[BoundarySide] = BoundaryValue;
	        TempPt2 = MvarPtCopy(TempPt);
		IRIT_LIST_PUSH(TempPt, *SplitPts0);
		IRIT_LIST_PUSH(TempPt2, *SplitPts1);
	    }
	    else if (TempPt -> Pt[BoundarySide] < BoundaryValue){
		IRIT_LIST_PUSH(TempPt, *SplitPts0);
	    }
	    else {
		IRIT_LIST_PUSH(TempPt, *SplitPts1);
	    }	    
	}
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes a vector, which points to the same half-space as the tangent    *
*   vector of the intersection curve (solution of MVs[i] = 0, i=0,..,N) at   *
*   the boundary point BoundaryPt.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	The system of N = Dim - 1 equations with N + 1 unknowns	     *
*		" MVs[i] = 0, i=0,..,N ".				     *
*   BoundaryPt: Point on the boundary of the domain of MVs                   *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*   NumericTol: Numeric Tolerance.					     *
*									     *
* RETURN VALUE:                                                              *
*   MvarVecStruct *:	Vector that points into the domain of MVs.	     *
*****************************************************************************/
static MvarVecStruct *MvarSSIStartingVec(MvarMVStruct * const *MVs, 
					 MvarPtStruct *BoundaryPt,
					 MvarSSIAuxStruct *AS,
					 CagdRType NumericTol)
{   
    int i,
	Dim = MVs[0] -> Dim;
    MvarVecStruct
	*StartVec = MvarVecNew(Dim);

    MVAR_VEC_RESET(StartVec);

    MvarMVDomain(MVs[0], AS -> MinDmn, AS -> MaxDmn, -1);

    for (i = 0; i < Dim; i++) {
	if (BoundaryPt -> Pt[i] < AS -> MinDmn[i] + NumericTol)
	    StartVec -> Vec[i] = 1.0;
	else if (BoundaryPt -> Pt[i] > AS -> MaxDmn[i] - NumericTol)
	    StartVec -> Vec[i] = -1.0;
    }

    return StartVec;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Tests if the hyper-surfaces MVs are flat enough such that the	     *
*   intersection curve has no loop. Note, the intersection curve can	     *
*   consist of more than one segment.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs:	The system of N = Dim - 1 equations with N + 1 unknowns	     *
*		" MVs[i] = 0, i=0,..,N "				     *
*   AS:	        Auxiliary SSI structure that holds auxiliary data.	     *
*									     *
* RETURN VALUE:                                                              *
*   int:	TRUE if the intersection curve has no loops		     *
*****************************************************************************/
static int MvarSSINoLoopTest(MvarMVStruct * const *MVs,
			     MvarSSIAuxStruct *AS)
{
    int i, j, k, 
	Dim = MVs[0] -> Dim,
	PowerTwoDim = (int) pow(2, Dim - 1);
    MvarVecStruct *WedProd;
      
    for (i = 0; i < Dim - 1; i++) {
        MvarNormalConeStruct *Cone;

        if ((Cone = MVarMVNormalCone(MVs[i])) == NULL)	   
            return FALSE;

	CAGD_GEN_COPY(AS -> TempList[i] -> Vec, 
		      Cone -> ConeAxis -> Vec, sizeof(CagdRType) * Dim);

        /* Cone is valid: add plane orthogonal to cone axis to matrix A. */
        CAGD_GEN_COPY(&AS -> A[i * (Dim)], Cone -> ConeAxis -> Vec,
		      sizeof(CagdRType) * Dim);

        /* Take the anti-cone = cos(90 - angle) = sqrt(1-cos^2). */
        AS -> b[i] = sqrt(1.0 - IRIT_SQR(Cone -> ConeAngleCosine));

        MvarNormalConeFree(Cone);
    }

    /* Axes of tangent cones are linearly dependent. */
    if (MvarSSILinDep(AS -> TempList, Dim - 1, AS))
	 return FALSE;

    WedProd = MvarSSIWedgeProduct(AS -> TempList, Dim - 1, AS); 
    CAGD_GEN_COPY(&AS -> A[(Dim - 1) * Dim], WedProd -> Vec, 
		  sizeof(CagdRType) * Dim);
    MvarVecFree(WedProd);

    AS -> b[Dim - 1] = 0;

    IritQRUnderdetermined(AS -> A, NULL, NULL, Dim, Dim);
   
    for (i = 0; i < PowerTwoDim; i++) {
        CagdRType VecSqrLength;

        k = i;
        for (j = 0; j < Dim - 1; ++j) {
	    AS -> bCopy[j] = (k & 1) ? AS -> b[j] : -AS -> b[j];
            k >>= 1;
        }
	AS -> bCopy[Dim - 1] = 0;

        IritQRUnderdetermined(NULL, AS -> x, AS -> bCopy, Dim, Dim);

        for (VecSqrLength = 0.0, j = 0; j < Dim; ++j)
            VecSqrLength += IRIT_SQR(AS -> x[j]);

        if (VecSqrLength >= 1.0)
            return FALSE;
    }

    return TRUE;    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   In order to plot domains where MvarSSICurveTracing was applied, polyline *
* is provided by the domain. Used for debugging.			     *
*                                                                            *
* PARAMETERS:                                                                *
*   Poly:	Polyline, Poly -> PAux points to the domain.		     *
*   MV:	        Multivar structure with the domain.			     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPolyStruct *:	Polyline provided by domain.			     *
*****************************************************************************/
static MvarPolyStruct *MvarSSIPolyWithDom(MvarPolyStruct *Poly, 
					  const MvarMVStruct *MV)
{
#ifdef DEBUG_MVAR_SSI_SAVE_POLY_DOMAIN
    int Dim = MV -> Dim;
    CagdRType *MinDmn, *MaxDmn, *Domain;
   
    Domain = IritMalloc(2 * Dim * sizeof(CagdRType));
    MvarMVDomainAlloc(MV, &MinDmn, &MaxDmn);

    IRIT_GEN_COPY(&Domain[0], MinDmn, Dim * sizeof(CagdRType));
    IRIT_GEN_COPY(&Domain[Dim], MaxDmn, Dim * sizeof(CagdRType));

    Poly -> PAux = Domain;

    MvarMVDomainFree(MinDmn, MaxDmn);
#endif /* DEBUG_MVAR_SSI_SAVE_POLY_DOMAIN */

    return Poly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Computes the intersection curve of the system of N = Dim - 1 equations   *
* with N + 1 unknowns " MVs[i] = 0, i=0,..,N ". If MvarSSINoLoopTest is      *
* satisfied and there are only 2 intersection points of MVs with the	     *
* boundary of the domain, numerical tracing is applied. Otherwise, we	     *
* subdivide the longest side of the domain.				     *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs: 	 The system of N = Dim - 1 equations with N + 1 unknowns     *
*		                                    MVs[i] = 0, i=0,..,N.    *
*   Step:	 The step size for numerical tracing.		    	     *
*   SubdivTol:	 Subdivision tolerance in MvarMVZeros.			     *
*   NumericTol:  Numerical tolerance.					     *
*   MVGrads:     Precomputed gradients of MVs.				     *
*   AS:	         Auxiliary SSI structure that holds auxiliary data.	     *
*   BoundaryPts: Solution points on the boundary of the domain.		     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPolyStruct *:	The list of polylines that approximate the	     *
*			univariate solution curve.			     *
*****************************************************************************/
static MvarPolyStruct *MvarSSISubdToInterCrv(MvarMVStruct * const *MVs, 
					     CagdRType Step, 
					     CagdRType SubdivTol,
					     CagdRType NumericTol,
					     MvarMVGradientStruct **MVGrads,
					     MvarSSIAuxStruct *AS,
					     MvarPtStruct *BoundaryPts)
{
    int i, j, JLoc, 
	Dim = MVs[0] -> Dim;
    CagdRType *MinDmn, *MaxDmn, t, MaxSide; 
    MvarPtStruct *SolutionPts;
    MvarVecStruct *StartVec;

    for (j = 0; j < Dim - 1; j++) {
	if (MvarSSINoRoot(MVs[j], NumericTol))
	    return NULL;
    }
 
    MvarMVDomainAlloc(MVs[0], &MinDmn, &MaxDmn);

    JLoc = 0;
    MaxSide = 0;
    for (j = 0; j < Dim; j++) {
        if (IRIT_ABS(MaxDmn[j] - MinDmn[j]) > MaxSide) {
	    MaxSide = IRIT_ABS(MaxDmn[j] - MinDmn[j]);
	    JLoc = j;
	}
    }

    if (MaxSide > SubdivTol) {
        MvarPolyStruct *NewPoly, *List1, *List2;
	MvarPtStruct *MiddlePts1, *MiddlePts2, *SplitPts[2],
					    *BoundaryPts1, *BoundaryPts2;
	MvarMVStruct **MVs1, **MVs2;

        if (MvarSSINoLoopTest(MVs, AS)) {
	    switch (CagdListLength(BoundaryPts)) {
	        case 0:
		    MvarMVDomainFree(MinDmn, MaxDmn);
		    return NULL;
	        case 1:
		    MvarMVDomainFree(MinDmn, MaxDmn);
		    return NULL;
	        case 2:	
		    StartVec = MvarSSIStartingVec(MVs, BoundaryPts,
						  AS, NumericTol);
		    SolutionPts = MvarSSICurveTracing(MVs, BoundaryPts, 
						      BoundaryPts -> Pnext,
						      StartVec, Step, 
						      NumericTol,
						      MVGrads, AS);
		    assert(SolutionPts != NULL);

		    MvarVecFree(StartVec);
		    MvarMVDomainFree(MinDmn, MaxDmn);

		    /* Each polyline is provided by the current domain. */
		    NewPoly = MvarPolyNew(SolutionPts);
		    NewPoly = MvarSSIPolyWithDom(NewPoly, MVs[0]);
		    return NewPoly;
	        default:
		    break;
	    }
	}

	MVs1 = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * (Dim - 1));
	MVs2 = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * (Dim - 1));

	/* Subdivision at the middle of the domain, at    		     */
	/* t = (TMin + TMax) * 0.5, is more likely to create problems of     */
	/* intersections along (subdivided) domains.  Subdivisions at a      */
	/* small perturbed domain is going to be more robust. 		     */
	t = MVAR_SSI_SUBD_PRTRB * (1 + JLoc * 0.1);
	assert(t < 0.1);
	t = (0.5 + t) * MinDmn[JLoc] + (0.5 - t) * MaxDmn[JLoc];

	for (i = 0; i < Dim - 1; i++) {
	    MVs1[i] = MvarMVSubdivAtParam(MVs[i], t, JLoc);
	    MVs2[i] = MVs1[i] -> Pnext;
	    MVs1[i] -> Pnext = NULL;
	}

	/* Find middle points and make sure they are not the same as the    */
	/* input boundary points.				            */
	MiddlePts1 = MvarSSIMiddlePlaneCutPts(MVs, AS, SubdivTol, NumericTol, 
					      JLoc, t);
	MiddlePts2 = MvarPtCopyList(MiddlePts1);

	MvarSSISplitBoundaryPts(BoundaryPts, JLoc, t,
				&SplitPts[0], &SplitPts[1]);

	SplitPts[0] = CagdListAppend(MiddlePts1, SplitPts[0]);
	SplitPts[1] = CagdListAppend(MiddlePts2, SplitPts[1]);

	BoundaryPts1 = MvarSSIListOfDifferentPts(SplitPts[0], NumericTol);
	BoundaryPts2 = MvarSSIListOfDifferentPts(SplitPts[1], NumericTol);

#       ifdef DEBUG_DUMP_SINGULAR_CASES
	if (CagdListLength(BoundaryPts1) % 2 != 0 ||
	    CagdListLength(BoundaryPts2) % 2 != 0) {
	    MvarPtStruct *Pt;

	    fprintf(stderr, "ERROR: Odd number of boundary points, JLoc = %d at t = %f\n",
		    JLoc, t);

	    for (Pt = BoundaryPts; Pt != NULL; Pt = Pt -> Pnext) {
	        fprintf(stderr, "\tInput Pts:");
	        for (i = 0; i < Dim; i++) {
		    fprintf(stderr, " %.5lf", Pt -> Pt[i]);
		}
		
		for (i = 0; i < Dim - 1; i++) {
		    CagdRType
		        *R = MvarMVEval(MVs[i], Pt -> Pt);

		    if (IRIT_ABS(R[1]) < IRIT_UEPS)
		        fprintf(stderr, " [0]");		      
		    else
		        fprintf(stderr, " [%.5g]", R[1]);
		}

		fprintf(stderr, "\n");
	    }

	    MiddlePts1 = MvarSSIMiddlePlaneCutPts(MVs, AS, SubdivTol,
						  NumericTol, JLoc, t);
	    for (Pt = MiddlePts1; Pt != NULL; Pt = Pt -> Pnext) {
	        fprintf(stderr, "\tMiddle Pts:");
	        for (i = 0; i < Dim; i++) {
		    fprintf(stderr, " %.5lf", Pt -> Pt[i]);
		}

		for (i = 0; i < Dim - 1; i++) {
		    CagdRType
		        *R = MvarMVEval(MVs[i], Pt -> Pt);

		    if (IRIT_ABS(R[1]) < IRIT_UEPS)
		        fprintf(stderr, " [0]");		      
		    else
		        fprintf(stderr, " [%.5g]", R[1]);
		}

		fprintf(stderr, "\n");
	    }
	    MvarPtFreeList(MiddlePts1);

	    for (Pt = BoundaryPts1; Pt != NULL; Pt = Pt -> Pnext) {
	        fprintf(stderr, "\tPts1:");
	        for (i = 0; i < Dim; i++) {
		    fprintf(stderr, " %.8lf", Pt -> Pt[i]);
		}
		fprintf(stderr, "\n");
	    }
	    for (Pt = BoundaryPts2; Pt != NULL; Pt = Pt -> Pnext) {
	        fprintf(stderr, "\tPts2:");
	        for (i = 0; i < Dim; i++) {
		    fprintf(stderr, " %.8lf", Pt -> Pt[i]);
		}
		fprintf(stderr, "\n");
	    }
	}
#       endif /* DEBUG_DUMP_SINGULAR_CASES */

	List1 = MvarSSISubdToInterCrv(MVs1, Step, SubdivTol, NumericTol,
				      MVGrads, AS, BoundaryPts1);
	List2 = MvarSSISubdToInterCrv(MVs2, Step, SubdivTol, NumericTol,
				      MVGrads, AS, BoundaryPts2);

	MvarPtFreeList(BoundaryPts1);
	MvarPtFreeList(BoundaryPts2);

#       ifdef DEBUG_MVAR_SSI_LINK_NEIGH2
	fprintf(stderr, "\nDomain 1:");
	for (i = 0; i < MVs1[0] -> Dim; i++) {
	    CagdRType t1, t2;

	    MvarMVDomain(MVs1[0], &t1, &t2, i);
	    fprintf(stderr, "[%6.3f, %6.3f] ", t1, t2);
	}
	fprintf(stderr, "\nDomain 2:");
	for (i = 0; i < MVs2[0] -> Dim; i++) {
	    CagdRType t1, t2;

	    MvarMVDomain(MVs2[0], &t1, &t2, i);
	    fprintf(stderr, "[%6.3f, %6.3f] ", t1, t2);
	}
	fprintf(stderr, "\n\n");
#	endif /* DEBUG_MVAR_SSI_LINK_NEIGH2 */

	for (i = 0; i < Dim - 1; i++) {
	    MvarMVFree(MVs1[i]);
	    MvarMVFree(MVs2[i]);		  
	}
	IritFree(MVs1);
	IritFree(MVs2);

	MvarMVDomainFree(MinDmn, MaxDmn);

	NewPoly = MvarSSILinkNeighbours(List1, List2, SubdivTol, NumericTol,
					JLoc, t);

#       ifdef DEBUG_MVAR_SSI_LINK_NEIGH2
	if (NewPoly != NULL)
	    MvarDbgSSIExamineSegmentBndry(NewPoly -> Pl, MVs[0]);
#	endif /* DEBUG_MVAR_SSI_LINK_NEIGH2 */

	return NewPoly;
    }
    /* The subdivision tolerance is reached - take the center of the box. */
    else {
        MvarPtStruct *Center;
	MvarPolyStruct *PolyCenter;

	Center = MvarPtNew(Dim);
	for (i = 0; i < Dim; i++)
	    Center -> Pt[i] = (MaxDmn[i] + MinDmn[i]) * 0.5;
	PolyCenter = MvarPolyNew(Center);
	if (GlblMvarInterMergeSingularPts)/* Mark singular domains as such. */
	    AttrSetIntAttrib(&Center -> Attr, "_MidSSIPt", TRUE);

	MvarMVDomainFree(MinDmn, MaxDmn);

	PolyCenter = MvarSSIPolyWithDom(PolyCenter, MVs[0]);

	return PolyCenter;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Searches all MVs for  parameter locations that areC1 discont.  If found  *
* update JLoc and t to this finding and returns TRUE.                        *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs: 	 The system of N = Dim - 1 equations with N + 1 unknowns     *
*		                                    MVs[i] = 0, i=0,..,N.    *
*   JLoc:        The direction in the domain with C1 discont. if has one.    *
*   t:           The parameter at the C1 discont. if has one.		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:        TRUE if found a C1 discont., FALSE otherwise.                *
*****************************************************************************/
static int MvarSSIHasC1Discont(MvarMVStruct * const *MVs,
			       int *JLoc,
			       CagdRType *t)
{
    int i, j, 
	Dim = MVs[0] -> Dim;

    for (i = 0; i < Dim - 1; i++) {
        MvarMVStruct const 
	    *MV = MVs[i];

	if (MVAR_IS_BEZIER_MV(MV))
	    continue;

        for (j = 0; j < Dim; j++) {
	    if (BspKnotC1Discont(MV -> KnotVectors[j],
				 MV -> Orders[j], MV -> Lengths[j], t)) {
	        /* Found a discontinuity - return it. */
	        *JLoc = j;
		return TRUE;
	    }
	}
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Subdivides the input MVs until they are C1 continuous at every knot      *
* multiplicity that yields C1 discontinuity or worse.                        *
*   Then every C1 continuous domain is invoking the regular solver only to   *
* be merged back at the end.                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   MVs: 	 The system of N = Dim - 1 equations with N + 1 unknowns     *
*		                                    MVs[i] = 0, i=0,..,N.    *
*   Step:	 The step size for numerical tracing.		    	     *
*   SubdivTol:	 Subdivision tolerance in MvarMVZeros.			     *
*   NumericTol:  Numerical tolerance.					     *
*									     *
* RETURN VALUE:                                                              *
*   MvarPolyStruct *:	The list of polylines that approximate the           *
*                       univariate solution curve.			     *
*****************************************************************************/
static MvarPolyStruct *MvarSSISubdToC1Cont(MvarMVStruct * const *MVs, 
					   CagdRType Step, 
					   CagdRType SubdivTol,
					   CagdRType NumericTol,
					   MvarSSIAuxStruct *AS)
{
    int i, JLoc, 
	Dim = MVs[0] -> Dim;
    CagdRType t;
    MvarPolyStruct *NewPoly;

    if (MvarSSIHasC1Discont(MVs, &JLoc, &t)) {
        MvarPolyStruct *List1, *List2;
	MvarMVStruct **MVs1, **MVs2;

	/* Subdivision at the discontinuity. */

	MVs1 = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * (Dim - 1));
	MVs2 = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * (Dim - 1));

	for (i = 0; i < Dim - 1; i++) {
	    MVs1[i] = MvarMVSubdivAtParam(MVs[i], t, JLoc);
	    MVs2[i] = MVs1[i] -> Pnext;
	    MVs1[i] -> Pnext = NULL;
	}

	List1 = MvarSSISubdToC1Cont(MVs1, Step, SubdivTol, NumericTol, AS);
	List2 = MvarSSISubdToC1Cont(MVs2, Step, SubdivTol, NumericTol, AS);

#       ifdef DEBUG_MVAR_SSI_LINK_NEIGH3
	fprintf(stderr, "\nDomain 1:");
	for (i = 0; i < MVs1[0] -> Dim; i++) {
	    CagdRType t1, t2;

	    MvarMVDomain(MVs1[0], &t1, &t2, i);
	    fprintf(stderr, "[%6.3f, %6.3f] ", t1, t2);
	}
	fprintf(stderr, "\nDomain 2:");
	for (i = 0; i < MVs2[0] -> Dim; i++) {
	    CagdRType t1, t2;

	    MvarMVDomain(MVs2[0], &t1, &t2, i);
	    fprintf(stderr, "[%6.3f, %6.3f] ", t1, t2);
	}
	fprintf(stderr, "\n\n");
	fprintf(stderr, "\nDomain 1:");
	for (i = 0; i < MVs1[0] -> Dim; i++) {
	    CagdRType t1, t2;

	    MvarMVDomain(MVs1[0], &t1, &t2, i);
	    fprintf(stderr, "[%6.3f, %6.3f] ", t1, t2);
	}
	fprintf(stderr, "\nDomain 2:");
	for (i = 0; i < MVs2[0] -> Dim; i++) {
	    CagdRType t1, t2;

	    MvarMVDomain(MVs2[0], &t1, &t2, i);
	    fprintf(stderr, "[%6.3f, %6.3f] ", t1, t2);
	}
	fprintf(stderr, "\n\n");
#	endif /* DEBUG_MVAR_SSI_LINK_NEIGH3 */

	for (i = 0; i < Dim - 1; i++) {
	    MvarMVFree(MVs1[i]);
	    MvarMVFree(MVs2[i]);		  
	}
	IritFree(MVs1);
	IritFree(MVs2);

	NewPoly = MvarSSILinkNeighbours(List1, List2, SubdivTol, NumericTol,
					JLoc, t);

#       ifdef DEBUG_MVAR_SSI_LINK_NEIGH3
	if (NewPoly != NULL)
	    MvarDbgSSIExamineSegmentBndry(NewPoly -> Pl, MVs[0]);
#	endif /* DEBUG_MVAR_SSI_LINK_NEIGH3 */
    }
    else {              /* No C1 discont here - invoke the regular solver. */
        int NumOfMVs = MVs[0] -> Dim - 1;
	MvarPtStruct *InitialBoundaryPts;
	MvarMVGradientStruct
	    **MVGrads = (MvarMVGradientStruct **) 
			IritMalloc(sizeof(MvarMVGradientStruct *) * NumOfMVs);

	for (i = 0; i < NumOfMVs; i++)
	    MVGrads[i] = MvarMVPrepGradient(MVs[i], FALSE);	    	    

	InitialBoundaryPts = MvarSSIStartEndPts(MVs, AS,
						SubdivTol, NumericTol);
	NewPoly = MvarSSISubdToInterCrv(MVs, Step, SubdivTol, NumericTol,
					MVGrads, AS, InitialBoundaryPts);
	MvarPtFreeList(InitialBoundaryPts);

	for (i = 0; i < NumOfMVs; i++)
	    MvarMVFreeGradient(MVGrads[i]);	    	    
    }

    return NewPoly;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates a polyn. system for SSI (surface-surface intersection) problem.  *
*   Two parametric surfaces MV1(u1,v1) and MV2(u2,v2) are given. The system  *
*   of 3 equations with 4 unknowns is created:		    		     *
*	x(u1,v1) - x(u1,v1) = 0						     *
*	y(u1,v1) - y(u1,v1) = 0						     *
*       z(u1,v1) - z(u1,v1) = 0.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Srf1, Srf2:	    Surfaces to be intersected.				     *
*									     *
* RETURN VALUE:                                                              *
*   MvarMVStruct **:  The multivar system, its solution is desired curve.    *
*****************************************************************************/
static MvarMVStruct **MvarSSICreateMVs(const CagdSrfStruct *Srf1, 
				       const CagdSrfStruct *Srf2)
{
    int i;
    CagdRType Min, Max;
    MvarMVStruct *TMV, *MVX1, *MVY1, *MVZ1, *MVW1, **TempMVs,
        *MVX2, *MVY2, *MVZ2, *MVW2, *MVTmp,
        *MV1 = MvarSrfToMV(Srf1),
        *MV2 = MvarSrfToMV(Srf2),
        **MVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *) * 3);
  
    if (MVAR_IS_BEZIER_MV(MV1)) {
	MVTmp = MvarCnvrtBzr2BspMV(MV1);
	MvarMVFree(MV1);
	MV1 = MVTmp;
    }

    if (MVAR_IS_BEZIER_MV(MV2)) {
	MVTmp = MvarCnvrtBzr2BspMV(MV2);
	MvarMVFree(MV2);
	MV2 = MVTmp;
    }

    TMV = MvarPromoteMVToMV2(MV1, 4, 0);
    MvarMVFree(MV1);
    MV1 = TMV;

    TMV = MvarPromoteMVToMV2(MV2, 4, 2);
    MvarMVFree(MV2);
    MV2 = TMV;
   
    for (i = 0; i < 2; i++) {
	MvarMVDomain(MV1, &Min, &Max, i);
	BspKnotAffineTrans2(MV2 -> KnotVectors[i],
			    MV2 -> Lengths[i] + MV2 -> Orders[i],
			    Min, Max);
    }

    for (i = 2; i < 4; i++) {
	MvarMVDomain(MV2, &Min, &Max, i);
	BspKnotAffineTrans2(MV1 -> KnotVectors[i],
			    MV1 -> Lengths[i] + MV1 -> Orders[i],
			    Min, Max);
    }

    TempMVs = MvarMVSplitScalar(MV1);
    MVW1 = TempMVs[0];
    MVX1 = TempMVs[1];
    MVY1 = TempMVs[2];
    MVZ1 = TempMVs[3];

    TempMVs = MvarMVSplitScalar(MV2);
    MVW2 = TempMVs[0];
    MVX2 = TempMVs[1];
    MVY2 = TempMVs[2];
    MVZ2 = TempMVs[3];

    MvarMVFree(MV1);
    MvarMVFree(MV2);

    if (MVW1 != NULL) {
	TMV = MvarMVMult(MVW1, MVX2); /* X2 = W1*X2. */
	MvarMVFree(MVX2);
	MVX2 = TMV;

	TMV = MvarMVMult(MVW1, MVY2); /* Y2 = W1*Y2. */
	MvarMVFree(MVY2);
	MVY2 = TMV;

	TMV = MvarMVMult(MVW1, MVZ2); /* Z2 = W1*Z2. */
	MvarMVFree(MVZ2);
	MVZ2 = TMV;

	MvarMVFree(MVW1);
    }

    if (MVW2 != NULL) {
	TMV = MvarMVMult(MVW2, MVX1); /* X1 = W2*X1. */
	MvarMVFree(MVX1);
	MVX1 = TMV;

	TMV = MvarMVMult(MVW2, MVY1); /* Y1 = W2*Y1. */
	MvarMVFree(MVY1);
	MVY1 = TMV;

	TMV = MvarMVMult(MVW2, MVZ1); /* Z1 = W2*Z1. */
	MvarMVFree(MVZ1);
	MVZ1 = TMV;

	MvarMVFree(MVW2);
    }

    MVs[0] = MvarMVSub(MVX1, MVX2);
    MVs[1] = MvarMVSub(MVY1, MVY2);
    MVs[2] = MvarMVSub(MVZ1, MVZ2);

    MvarMVFree(MVX1);
    MvarMVFree(MVY1);
    MvarMVFree(MVZ1);
    MvarMVFree(MVX2);
    MvarMVFree(MVY2);
    MvarMVFree(MVZ2);

    return MVs;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes the common intersection curve of n multivariate.		     M
*                                                                            *
* PARAMETERS:                                                                M
*   MVs:	    A vector of MV constraints.  Should be of size Dim-1     M
*                   where Dim is the dimension of the domain of MVs.	     M
*   Step:	    Step size for curve tracing.			     M
*   SubdivTol:	    The subdivision tolerance to use.			     M
*   NumericTol:	    The numerical tolerance to use.			     M
*									     *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:   The list of polylines which approximate the curve.   M
*			Each polyline corresponds to the topologically	     M
*			isolated component of the curve and is in R^k, the   M
*                       unioned parametric spaces of all input MVs.	     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfSrfInter2, MvarMVUnivarInterMergeSingularPts                      M
*									     *
* KEYWORDS:                                                                  M
*   MvarMVUnivarInter			                                     M
*****************************************************************************/
MvarPolyStruct *MvarMVUnivarInter(MvarMVStruct * const *MVs,
				  CagdRType Step,
				  CagdRType SubdivTol,
				  CagdRType NumericTol)
{
    int i,
	NumOfMVs = MVs[0] -> Dim - 1;
    MvarMVStruct *MV;
    MvarMVStruct
	 **LclMVs = (MvarMVStruct **) IritMalloc(sizeof(MvarMVStruct *)
							          * NumOfMVs);
    MvarSSIAuxStruct *AS;
    MvarPolyStruct *IntrCrv;

    /* Gradient structures for all MVs, computed just once! */
    for (i = 0; i < NumOfMVs; i++) {
	if (MVAR_IS_RATIONAL_MV(MVs[i])) {
	    /* Convert P1 point type to E1, in place. */
	    LclMVs[i] = MV = MvarMVCopy(MVs[i]);
	    MV -> PType = MVAR_PT_E1_TYPE;
#	    ifndef MVAR_MALLOC_STRUCT_ONCE
		IritFree(MV -> Points[0]);
#	    endif /* MVAR_MALLOC_STRUCT_ONCE */
	    MV -> Points[0] = NULL;
	}
	else
	    LclMVs[i] = MVs[i];
    }

    AS = MvarSSIAllocateOnce(NumOfMVs);

    IntrCrv = MvarSSISubdToC1Cont(LclMVs, Step, SubdivTol, NumericTol, AS);

    for (i = 0; i < NumOfMVs; i++) {
	if (MVs[i] != LclMVs[i]) {
	    IritFree(LclMVs[i]);
	}
    }

    MvarSSIDeallocateOnce(AS, NumOfMVs);

    return IntrCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the state of the singular points merger.  If TRUE, singular         M
* locations are merged using the subdivision tolerance which improves the    M
* changes of a complete long merged curves.  If FALSE, singular locations    M
* are merged using the numeric tolerances (like every other case) which      M
* means most likely they will be left as isolated points.                    M
*                                                                            *
* PARAMETERS:                                                                M
*   MergeSingularPts:   Set the desired state of singular points mergers.    M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old state.                                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarSrfSrfInter2, MvarMVUnivarInter                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarMVUnivarInterMergeSingularPts                                        M
*****************************************************************************/
int MvarMVUnivarInterMergeSingularPts(int MergeSingularPts)
{
    int OldVal = MergeSingularPts;

    GlblMvarInterMergeSingularPts = MergeSingularPts;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Computes intersection curve of two surfaces.			     M
*                                                                            *
* PARAMETERS:                                                                M
*   Srf1, Srf2:	    Cagd surfaces to be intersected.			     M
*   Step:	    Step size for curve tracing.			     M
*   SubdivTol:	    The subdivision tolerance to use.			     M
*   NumericTol:	    The numerical tolerance to use.			     M
*									     *
* RETURN VALUE:                                                              M
*   MvarPolyStruct *:   The list of polylines which approximate the curve.   M
*			Each polyline corresponds to the topologically	     M
*			isolated component of the curve and is in R^4, the   M
*                       parametric spaces of both surfaces.   		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarMVUnivarInterMergeSingularPts, MvarMVUnivarInter                     M
*									     *
* KEYWORDS:                                                                  M
*   MvarSrfSrfInter2			                                     M
*****************************************************************************/
MvarPolyStruct *MvarSrfSrfInter2(const CagdSrfStruct *Srf1, 
				 const CagdSrfStruct *Srf2,
				 CagdRType Step,
				 CagdRType SubdivTol,
				 CagdRType NumericTol)
{
    int i;
    MvarPolyStruct *IntrCrv;
    MvarMVStruct **MVs;
    CagdSrfStruct
	*NewSrf1 = NULL,
	*NewSrf2 = NULL;

    if (Srf1 -> PType != CAGD_PT_E3_TYPE && Srf1 -> PType != CAGD_PT_P3_TYPE)
        Srf1 = NewSrf1 = CagdCoerceSrfTo(Srf1,
					 CAGD_IS_RATIONAL_PT(Srf1 -> PType) ?
							  CAGD_PT_P3_TYPE : 
						          CAGD_PT_E3_TYPE,
				FALSE);

    if (Srf2 -> PType != CAGD_PT_E3_TYPE && Srf2 -> PType != CAGD_PT_P3_TYPE)
        Srf2 = NewSrf2 = CagdCoerceSrfTo(Srf2,
					 CAGD_IS_RATIONAL_PT(Srf2 -> PType) ?
							  CAGD_PT_P3_TYPE : 
						          CAGD_PT_E3_TYPE,
				FALSE);

    MVs = MvarSSICreateMVs(Srf1, Srf2);

    if (NewSrf1 != NULL)
        CagdSrfFree(NewSrf1);
    if (NewSrf2 != NULL)
        CagdSrfFree(NewSrf2);

#   ifdef DEBUG_MVAR_SSI_MV_ZEROS
    {
        /* Test solution with the regular solver. */
        static MvarConstraintType
	    Constraints[3] = { MVAR_CNSTRNT_ZERO,
			       MVAR_CNSTRNT_ZERO,
			       MVAR_CNSTRNT_ZERO };
        MvarPtStruct
	    *Pts = MvarMVsZeros(MVs, Constraints, 3, SubdivTol, NumericTol);

	IntrCrv = MvarMatchPointListIntoPolylines(Pts, SubdivTol * 10);
    }
#   else
        IntrCrv = MvarMVUnivarInter(MVs, Step, SubdivTol, NumericTol);
#   endif /* DEBUG_MVAR_SSI_MV_ZEROS */

    for (i = 0; i < 3; i++)
        MvarMVFree(MVs[i]);	    	    
    IritFree(MVs);

    return IntrCrv;
}
