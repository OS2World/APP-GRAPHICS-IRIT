/******************************************************************************
* MvLowEnv.c - Lower Envelope Algorithm Functions.		              *
*                                                                             *
*   The current implementation was improved by using the LECrvStruct, which   *
* reduces the number of operations actually performed on the surfaces.        *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by Iddo Hanniel, September 2005.				      *
******************************************************************************/

#include "irit_sm.h"
#include "geom_lib.h"
#include "misc_lib.h"
#include "cagd_lib.h"
#include "symb_lib.h" 
#include "mvar_loc.h"

/*****************************************************************************
* AUXILIARY:   LECrvStruct		                                     *
*                                                                            *
* A local auxilary data structure for the lower envelope algorithm. Instead  *
* of performing an actual split of the MvarVoronoiCrvStruct during the       *
* algorithm we perfom a symbolic split by storing the original curve and     *
* updating the t domain parameters t0,t1. This is both more efficient and    *
* more robust.                                                               *
*****************************************************************************/

typedef struct MvarLECrvStruct {
    struct MvarLECrvStruct *Pnext;
    VoidPtr Orig;
    CagdRType t0;
    CagdRType t1;
} MvarLECrvStruct;

/* Flag for determining what type of curve we are handling. */
typedef enum {
    MVAR_LE_VOR_CRV = 0,
    MVAR_LE_UNI_FUNC_CRV
} MvarLECrvType;

IRIT_STATIC_DATA MvarLECrvType
    GlblCrvTypeFlag = MVAR_LE_VOR_CRV; 

static MvarLECrvStruct *LECrvNew(void);
static MvarLECrvStruct *LECrvCopy(MvarLECrvStruct *Crv);
static void LECrvFree(MvarLECrvStruct *Crv);
static void LECrvLstFree(MvarLECrvStruct *Crv);
static CagdBType LEBsctIsXSmaller(CagdRType t1, CagdRType t2);
static void LEBsctSplitCurve(MvarLECrvStruct *Cv, 
			     CagdRType SplitPt, 
			     MvarLECrvStruct **CvLeft, 
			     MvarLECrvStruct **CvRight);
static void LEBsctTrimCurveBetween(MvarLECrvStruct *Cv, 
				   CagdRType t1, 
				   CagdRType t2, 
				   MvarLECrvStruct **TrimmedCurve);
static void LEBsctSplitEnvelope1AtEnvelope2(MvarLECrvStruct *Envelope1, 
					    MvarLECrvStruct *Envelope2, 
					    MvarLECrvStruct **SplitEnvelope1);
static void LEBsctComputeLowerEnvelopeOfOverlap(
					    MvarLECrvStruct *Cv1, 
					    MvarLECrvStruct *Cv2, 
					    MvarLECrvStruct **ResultEnvelope);
static void LEBsctMergeLowerEnvelopes(MvarLECrvStruct *Envelope1, 
				      MvarLECrvStruct *Envelope2, 
				      MvarLECrvStruct **LowerEnvelope);
static void LEBsctComputeLowerEnvelopeAux(MvarLECrvStruct *InputCurves, 
					  MvarLECrvStruct **LowerEnvelope);


static int LESetCrvType(int CrvType);


/* Wrapper functions for LE functionality (proxys to curve-dependent funcs). */
static int LECv1IsYSmallerAt(MvarLECrvStruct *Cv1, 
			    MvarLECrvStruct *Cv2, 
			    CagdRType MidParam);
static void LEGetAllIntersectionPoints(MvarLECrvStruct *Cv1, 
				       MvarLECrvStruct *Cv2, 
				       MvarPtStruct **Points);

/* Implementation of functions for VorCrv. */
static int LEVorCv1IsYSmallerAt(MvarLECrvStruct *Cv1, 
				MvarLECrvStruct *Cv2, 
				CagdRType MidParam);
static void LEVorGetAllIntersectionPoints(MvarLECrvStruct *Cv1, 
					  MvarLECrvStruct *Cv2, 
					  MvarPtStruct **Points);

/* Implementation of functions for UniCrv. */
static int LEUniCv1IsYSmallerAt(MvarLECrvStruct *Cv1, 
				MvarLECrvStruct *Cv2, 
				CagdRType MidParam);
static void LEUniGetAllIntersectionPoints(MvarLECrvStruct *Cv1, 
					  MvarLECrvStruct *Cv2, 
					  MvarPtStruct **Points);

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Splitting the Cv at the given points Pt1 and Pt2.			     M
*									     *
* PARAMETERS:                                                                M
*   Cv:   Given a MvarVoronoiCrvStruct Cv.				     M
*   Pt1:  A MvarPtStruct.						     M
*   Pt2:  A MvarPtStruct.						     M
*   TrimmedCurve: A MvarVoronoiCrvStruct of the resultant.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void  								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctComputeLowerEnvelope, MvarBsctMergeLowerEnvelopes		     M
*   MvarBsctComputeLowerEnvelopeAux, MvarBsctComputeLowerEnvelopeOfOverlap   M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctTrimCurveBetween			  			     M
*****************************************************************************/
void MvarBsctTrimCurveBetween(MvarVoronoiCrvStruct *Cv, 
			      MvarPtStruct *Pt1, 
			      MvarPtStruct *Pt2, 
			      MvarVoronoiCrvStruct **TrimmedCurve)
{
    MvarVoronoiCrvStruct 
        *CvResAux = NULL,
        *CvTemp = NULL;
    MvarPtStruct 
        *Cvl = NULL,
        *Cvr = NULL;

    (*TrimmedCurve) = MvarVoronoiCrvCopy(Cv);

    Cvl = MvarPtNew(2);
    MvarBsctCurveLeft(Cv,Cvl);
    if (MvarBsctIsXSmaller(Cvl, Pt1)) {
        /* Freeing TrimmedCurve before splitting. */
        MvarVoronoiCrvFree(*TrimmedCurve);
        MvarBsctSplitCurve(Cv, Pt1, &CvResAux, TrimmedCurve);
        /* Freeing of CvResAux. */
	MvarVoronoiCrvFree(CvResAux);
    }
    MvarPtFree(Cvl);

    CvTemp = MvarVoronoiCrvCopy(*TrimmedCurve);

    Cvr = MvarPtNew(2); 
    MvarBsctCurveRight(Cv, Cvr);
    if (MvarBsctIsXSmaller(Pt2, Cvr)) {
        /* Freeing of TrimmedCurve. */
        MvarVoronoiCrvFree(*TrimmedCurve);
        MvarBsctSplitCurve(CvTemp, Pt2, TrimmedCurve, &CvResAux);
	MvarVoronoiCrvFree(CvResAux);
    }
    MvarVoronoiCrvFree(CvTemp);
    MvarPtFree(Cvr);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Allocates and resets all slots of a MvarLECrvStruct structure.             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarLECrvStruct *:  An MvarLECrvStruct structure.		             *
*****************************************************************************/
static MvarLECrvStruct *LECrvNew(void)
{
    MvarLECrvStruct
        *NewCrv = (MvarLECrvStruct *) IritMalloc(sizeof(MvarLECrvStruct));

    NewCrv -> Pnext = NULL;
    NewCrv -> Orig = NULL;
    NewCrv -> t0 = 0;
    NewCrv -> t1 = 0;

    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Allocates and copies all slots of a MvarLECrvStruct structure.             *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:       To be copied.                                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   MvarLECrvStruct *:  A duplicate of Crv.                                  *
*****************************************************************************/
static MvarLECrvStruct *LECrvCopy(MvarLECrvStruct *Crv)
{
    MvarLECrvStruct
        *NewCrv = (MvarLECrvStruct *) IritMalloc(sizeof(MvarLECrvStruct));
    
    NewCrv -> Orig = Crv -> Orig; /* Shallow Copy! */
    NewCrv -> t0 = Crv -> t0;
    NewCrv -> t1 = Crv -> t1;

    NewCrv -> Pnext = NULL;
    return NewCrv;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Deallocates a MvarLECrvStruct structure.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   Voronoi curve structure to free.	        		     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void LECrvFree(MvarLECrvStruct *Crv)
{
    IritFree(Crv); 
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Deallocates a MvarLECrvStruct list structure.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   Crv:   Voronoi curve list to free.	        		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void LECrvLstFree(MvarLECrvStruct *Crv)
{
    while (Crv) {
        MvarLECrvStruct
	    *Tmp = Crv -> Pnext;

        IritFree(Crv);
        Crv = Tmp;
    }
}

/*****************************************************************************
* AUXILIARY:   Wrapper functions for LE functionality.	                     *
* These functions serve as proxys to curve-dependent functions.              *
*****************************************************************************/

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Wrapper function for a Geometric Primitive Function that compares the  *
*   value of two generic LE curves at a given paraneter.                     *
*                                                                            *
*****************************************************************************/
static int LECv1IsYSmallerAt(MvarLECrvStruct *Cv1, 
			     MvarLECrvStruct *Cv2, 
			     CagdRType MidParam)
{
    switch (GlblCrvTypeFlag) {
        case MVAR_LE_VOR_CRV:
            return LEVorCv1IsYSmallerAt(Cv1, Cv2, MidParam);
            break;
        case MVAR_LE_UNI_FUNC_CRV:
            return LEUniCv1IsYSmallerAt(Cv1, Cv2, MidParam);
            break;
        default:
            assert (FALSE);
    }

    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Wrapper function for a Geometric Primitive Function that computes the  *
*   intersection of two generic LE curves.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv1, Cv2: input LECurves						     *
*   Points: the resultant points					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*                                                                            *
* NOTE: The returned Points list is required to contain only 1D points       *
* (it can contain 2D points, but only Points->Pt[0] is actually used by the  *
* algorithm).                                                                *
*****************************************************************************/
static void LEGetAllIntersectionPoints(MvarLECrvStruct *Cv1, 
				       MvarLECrvStruct *Cv2, 
				       MvarPtStruct **Points)
{
    switch (GlblCrvTypeFlag) {
        case MVAR_LE_VOR_CRV:
            LEVorGetAllIntersectionPoints(Cv1, Cv2, Points);
            return;
        case MVAR_LE_UNI_FUNC_CRV:
            LEUniGetAllIntersectionPoints(Cv1, Cv2, Points);
            return;
        default:
            assert (FALSE);
            return;
    }
}


/*****************************************************************************
* AUXILIARY:   Functions for VorCrv LE:	                                     *
*****************************************************************************/

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A function for a Geometric Primitive Function that compares the          *
*   value of two generic Voronoi LE curves at a given paraneter.             *
*                                                                            *
*****************************************************************************/
static int LEVorCv1IsYSmallerAt(MvarLECrvStruct *Cv1, 
				MvarLECrvStruct *Cv2, 
				CagdRType MidParam)
{
    int Cv1IsYSmaller = FALSE;
    MvarPtStruct
        *MidPoint = MvarPtNew(2); 

    MidPoint -> Pt[0] = MidParam;
    /* Y-coordinate needed for MvarBsctCv1IsYSmallerAt I/F. */
    MidPoint -> Pt[1] = 0.0; 

    Cv1IsYSmaller =
        MvarBsctCv1IsYSmallerAt((MvarVoronoiCrvStruct*) Cv1 -> Orig,
				(MvarVoronoiCrvStruct*) Cv2 -> Orig,
				MidPoint);
    MvarPtFree(MidPoint);

    return Cv1IsYSmaller;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Geometric Primitive Function that computes the intersection of two     *
*   Voronoi LE curves. These intersections correspond to equidistant         *
*   points of three curves.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv1, Cv2: input LECurves						     *
*   Points: the resultant points					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void LEVorGetAllIntersectionPoints(MvarLECrvStruct *Cv1, 
					  MvarLECrvStruct *Cv2, 
					  MvarPtStruct **Points)
{
    CagdRType
        UMin = Cv1 -> t0,		  /* Should be the same as the Cv2. */
        UMax = Cv1 -> t1;
    MvarPtStruct
	*Pt1 = MvarPtNew(2),
        *Pt2 = MvarPtNew(2);
    MvarVoronoiCrvStruct
	*Cv1OrigTrimmed = NULL,
        *Cv2OrigTrimmed = NULL;

    /* Sanity check. */
    if (IRIT_FABS(Cv1 -> t0 - Cv2 -> t0) >= IRIT_FABS(10.0 * MvarBsctNumerTol))
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
    if (IRIT_FABS(Cv1 -> t1 - Cv2 -> t1) >= IRIT_FABS(10.0 * MvarBsctNumerTol))
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    /* Need to trim the original surfaces at [t0,t1] (otherwise the solver   */
    /* can fail us).							     */
    Pt1 -> Pt[0] = UMin;
    Pt2 -> Pt[0] = UMax;
    Pt1 -> Pt[1] = Pt2 -> Pt[1] = 0.0;
    MvarBsctTrimCurveBetween((MvarVoronoiCrvStruct*) Cv1 -> Orig,
			     Pt1, Pt2, &Cv1OrigTrimmed);
    MvarBsctTrimCurveBetween((MvarVoronoiCrvStruct*) Cv2 -> Orig,
			     Pt1, Pt2, &Cv2OrigTrimmed);

    MvarBsctGetAllIntersectionPoints(Cv1OrigTrimmed, Cv2OrigTrimmed, Points);

    MvarPtFree(Pt1);
    MvarPtFree(Pt2);
    MvarVoronoiCrvFree(Cv1OrigTrimmed);
    MvarVoronoiCrvFree(Cv2OrigTrimmed);
}


/*****************************************************************************
* AUXILIARY:   Functions for UniCrv LE:	                                     *
*****************************************************************************/

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A function for a Geometric Primitive Function that compares the          *
*   value of two generic univariate function LE curves at a given paraneter. *
*                                                                            *
*****************************************************************************/
static int LEUniCv1IsYSmallerAt(MvarLECrvStruct *Cv1, 
				MvarLECrvStruct *Cv2, 
			    CagdRType MidParam)
{
    CagdRType *R, y1, y2;
    CagdCrvStruct
	*Crv1 = (CagdCrvStruct*) Cv1->Orig,
	*Crv2 = (CagdCrvStruct*) Cv2->Orig;

    R = CagdCrvEval(Crv1, MidParam);
    y1 = R[1];                             /* R[0] is reserved for w value. */
    if (CAGD_IS_RATIONAL_CRV(Crv1))
        y1 /= R[0];

    R = CagdCrvEval(Crv2, MidParam);
    y2 = R[1];
    if (CAGD_IS_RATIONAL_CRV(Crv2))
        y2 /= R[0];

    return y1 < y2;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   A Geometric Primitive Function that computes the intersection of two     *
*   univariate function LE curves. These intersections correspond to         *
*   points of intersection between the functions, i.e., to roots of the      *
*   polynomial Cv1-Cv2.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv1, Cv2: input LECurves						     *
*   Points: the resultant points					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void								     *
*****************************************************************************/
static void LEUniGetAllIntersectionPoints(MvarLECrvStruct *Cv1, 
					  MvarLECrvStruct *Cv2, 
					  MvarPtStruct **Points)
{
    /* Start by finding the intersection of the domain and continue on the */
    /* mutual domain we can assume [tMin,tMax] are within the orig domain. */
    CagdRType OverlapMin, OverlapMax;
    CagdBType
        CrvsAreSame = FALSE;
    CagdCrvStruct *Crv1, *Crv2;    
    MvarConstraintType Constraints[1];
    MvarMVStruct *MVVec[1], *MVCv1, *MVCv2;
    MvarPtStruct *MVPts;

    if (Cv1 -> t0 > Cv2 -> t1 || Cv2 -> t0 > Cv1 -> t1) {
        /* No overlap. */
        *Points = NULL;
        return;
    }

    /* Find overlap: */
    OverlapMin = IRIT_MAX(Cv1 -> t0, Cv2 -> t0);
    OverlapMax = IRIT_MIN(Cv1 -> t1, Cv2 -> t1);

    /* Extract curves over overlapping domains. */
    Crv1 = CagdCrvRegionFromCrv((CagdCrvStruct *) Cv1 -> Orig,
				OverlapMin, OverlapMax);
    Crv2 = CagdCrvRegionFromCrv((CagdCrvStruct *) Cv2 -> Orig,
				OverlapMin, OverlapMax);

    /* Check degenerate position - when curves totally overlap.             */
    /* Note: CagdCrvsSame might not be strong enough to detect all such     */
    /* degeneracies.							    */
    CrvsAreSame = CagdCrvsSame(Crv1, Crv2, 10.0*MvarBsctNumerTol);
    if (CrvsAreSame) {
        /* Totally overlapping curves - it doesn't matter which you take,   */
        /* handle it as no intersection.                                    */
        *Points = NULL;
        CagdCrvFree(Crv1);
        CagdCrvFree(Crv2);

        return;
    }

    MVCv1 = MvarCrvToMV(Crv1);
    MVCv2 = MvarCrvToMV(Crv2);
    MVVec[0] = MvarMVSub(MVCv1, MVCv2);
    MvarMVFree(MVCv1);
    MvarMVFree(MVCv2);

    Constraints[0] = MVAR_CNSTRNT_ZERO;
    MVPts = MvarMVsZeros(MVVec, Constraints, 1, MvarBsctSubdivTol,
			 -IRIT_FABS(MvarBsctNumerTol));
    MvarMVFree(MVVec[0]);

    /* Sorting is always according to first parameter */
    /* (since it is a scalar curve).                  */
    MVPts = MvarPtSortListAxis(MVPts, 1);

    /* Sanity check: if assertion below failed, CvsAreSame probably         */
    /* returned FALSE when it should have returned TRUE.                    */
    assert(CagdListLength(MVPts) <= IRIT_MAX(Crv1 -> Length, Crv2 -> Length));

    CagdCrvFree(Crv1);
    CagdCrvFree(Crv2);

    *Points = MVPts;
}

/*****************************************************************************
* AUXILIARY:   Basic functions used in the D&Q Lower Envelope algorithm.     *
*****************************************************************************/

/*****************************************************************************
* DESCRIPTION:                                                               *
*    A Geometric Primitive Function that identifies which x-coord of the     *
* two points P1 and P2 is smaller.					     *
*                                                                            *
* PARAMETERS:                                                                *
*   t1, t2: t parameters (of points in tr-space) as input.                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   CagdBType: Returns true or false. 					     *
*****************************************************************************/
static CagdBType LEBsctIsXSmaller(CagdRType t1, CagdRType t2)
{
    return t2 - t1 > 10.0 * MvarBsctNumerTol;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*	A Geometric Primitive Function that splits a tr-montone curve at a   *
*   given t parameter.	                                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv: input curve.       						     *
*   SplitPt: given t parameter for split.			             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	               	            				     *
*****************************************************************************/
static void LEBsctSplitCurve(MvarLECrvStruct *Cv, 
			     CagdRType SplitPt, 
			     MvarLECrvStruct **CvLeft, 
			     MvarLECrvStruct **CvRight)
{
    (*CvLeft) = LECrvCopy(Cv);
    (*CvLeft) -> t1 = SplitPt;

    (*CvRight) = LECrvCopy(Cv);
    (*CvRight) -> t0 = SplitPt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*	A Geometric Primitive Function that trims a tr-montone curve         *
*   between two given t parameters.	                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv: input curve.       						     *
*   t1, t2: given t parameters for trimming.			             *
*   TrimmedCurve: output trimmed curve.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*   void	               	            				     *
*****************************************************************************/
static void LEBsctTrimCurveBetween(MvarLECrvStruct *Cv, 
				   CagdRType t1, 
				   CagdRType t2, 
				   MvarLECrvStruct **TrimmedCurve)
{
    CagdRType 
        Cvl, Cvr;

    (*TrimmedCurve) = LECrvCopy(Cv);

    Cvl = Cv -> t0;
    if (t1 - Cvl > 10.0 * IRIT_FABS(MvarBsctNumerTol)) {
        /*Cvl is to the left of t1.*/
        (*TrimmedCurve) -> t0 = t1;
    }

    Cvr = Cv -> t1;
    if (Cvr - t2 > 10.0 * IRIT_FABS(MvarBsctNumerTol)) {
        /*Cvr is to the right of t2.*/
        (*TrimmedCurve) -> t1 = t2;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*	Splits the curves of a lower envelope Envelope1 at the end points    *
*   of Envelope2. Therefore, after the function is performed, SplitEnvelope1 *
*   will not contain an endpoint of Envelope2 in its interior (only possibly *
*   at endpoints).							     *
*                                                                            *
* PARAMETERS:                                                                *
*   Envelope1, Envelope2: input lists of LECrvs.       			     *
*   SplitEnvelope1: output list of curves containing the split Envelope1     *
*   curves.                                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:               	            				     *
*****************************************************************************/
static void LEBsctSplitEnvelope1AtEnvelope2(MvarLECrvStruct *Envelope1, 
					    MvarLECrvStruct *Envelope2, 
					    MvarLECrvStruct **SplitEnvelope1)
{
    MvarLECrvStruct
        *Aux = NULL,
        *Iter = NULL;
    MvarPtStruct 
        *Envelope2EndPoints = NULL,          /* Will hold unique end points. */
        *LeftMost = NULL,
        *RightMost = NULL,
        *IterPt = NULL,
        *IterPtRight = NULL;

    /* Sanity checks. */
    if (Envelope1 == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);
    if (Envelope2 == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    /* Collecting Envelope2 endpoints.*/
    /* Move code below to separate function. */
    Iter = Envelope2;

    LeftMost = MvarPtNew(1); 
    LeftMost -> Pt[0] = Iter -> t0;
    IRIT_LIST_PUSH(LeftMost, Envelope2EndPoints); 

    RightMost = MvarPtNew(1); 
    RightMost -> Pt[0] = Iter -> t1;
    IRIT_LIST_PUSH(RightMost, Envelope2EndPoints); 

    Iter = Iter -> Pnext;
    for(; Iter != NULL; Iter = Iter -> Pnext) {
        MvarPtStruct 
	    *Left=MvarPtNew(1),
            *Right = MvarPtNew(1);

        Left -> Pt[0] = Iter -> t0;;
        Right -> Pt[0] = Iter -> t1;

        if (LEBsctIsXSmaller(RightMost -> Pt[0], Left -> Pt[0])) {
            IRIT_LIST_PUSH(Left, Envelope2EndPoints);
        }
        else {
            MvarPtFree(Left);
        }

        IRIT_LIST_PUSH(Right, Envelope2EndPoints); 
	/* Right will always be inserted.                     */
	/* and therefore no need to free etc. like in Left.   */ 
        RightMost = Right; 
	/* RightMost will always point to the first in the list. */
    }

    /* The points should be arranged from left to Right so a reverse */
    /* is needed.                                                    */
    Envelope2EndPoints =  (MvarPtStruct *) CagdListReverse(Envelope2EndPoints);

    /* Finished collecting, start splitting. */

    IterPt = Envelope2EndPoints;
    Iter = Envelope1;
    while (Iter != NULL) {
        MvarLECrvStruct 
	    *CvRight = NULL,
            *CvLeft = NULL,
            *TmpCrv = NULL;

        /* Get first Pt that is larger than curve's left point. */
        while(IterPt != NULL) {
            CagdRType Cvl = Iter -> t0; /*Iter left point.*/

            if (LEBsctIsXSmaller(Cvl,IterPt -> Pt[0])) {
                break;
            }
            IterPt = IterPt -> Pnext;
        }

        /* Get first Pt that is larger or equal to curve's Right point. */
        IterPtRight = IterPt;

        while(IterPtRight != NULL) {
            CagdRType Cvr = Iter -> t1; /*Iter right point.*/

            if (!LEBsctIsXSmaller(IterPtRight -> Pt[0], Cvr)) {
                break;
            }
            IterPtRight = IterPtRight -> Pnext;
        }

        /* Now between IterPt and IterPtRight (not including IterPtRight!) */
        /* are all points inner to the current curve - split at them.      */
        CvRight = Iter;
        CvLeft = NULL;

        TmpCrv = LECrvCopy(CvRight); 
        while (IterPt != IterPtRight) {
            /* Split the curve at the point. */
            LEBsctSplitCurve(TmpCrv, IterPt -> Pt[0], &CvLeft, &CvRight);
            /* Freeing of TmpCrv. */
	    LECrvFree(TmpCrv);
            IRIT_LIST_PUSH(CvLeft, (*SplitEnvelope1));
            TmpCrv = CvRight;
			
	    /* We do not free CvRight since we use it at the next */
	    /* iteration (freed as Tmp).                          */
            IterPt = IterPt -> Pnext;
        }
        Aux = LECrvCopy(TmpCrv);
        IRIT_LIST_PUSH(Aux, (*SplitEnvelope1));
        LECrvFree(TmpCrv);

        Iter = Iter -> Pnext;
    }

    MvarPtFreeList(Envelope2EndPoints);

    /* The curves should be arranged from left to right and so a reversing */
    /* of the list is required. */
    *SplitEnvelope1 = (MvarLECrvStruct *) CagdListReverse(*SplitEnvelope1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given two curves over the same domain (i.e., overlapping domains),       *
*   computes their lower envelope. The function splits the curves at the     *
*   points of intersection, and compares the overlapping curves at a         *
*   midpoint between intersections.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*   Cv1, Cv2: input LECrvs.       		                	     *
*   ResultEnvelope: output list of curves representing the lower envelope.   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:               	            				     *
*****************************************************************************/
static void LEBsctComputeLowerEnvelopeOfOverlap(
					     MvarLECrvStruct *Cv1, 
					     MvarLECrvStruct *Cv2, 
					     MvarLECrvStruct **ResultEnvelope)
{
    MvarPtStruct 
        *Tmp = NULL,
        *IterPts = NULL,
        *IterPtsNext = NULL,
        *Points = NULL;
    
    /* We are currently working with points but actually we are interested */
    /* in x-coordinates.                                                   */
    LEGetAllIntersectionPoints(Cv1, Cv2, &Points);

    /* Intersection points are from left to right. */
    if (Points == NULL) {
        MvarPtStruct 
	    *L1 = MvarPtNew(1),
            *R1 = MvarPtNew(1); 

        L1 -> Pt[0] = Cv1 -> t0; /* Cv1 left point.  */
        R1 -> Pt[0] = Cv1 -> t1; /* Cv1 right point. */

        IRIT_LIST_PUSH(R1, Points); 
        IRIT_LIST_PUSH(L1, Points); 
    }
    else {
        MvarPtStruct 
	    *PtTarget = NULL,
	    *R1 = NULL,
	    *PtStart = Points, /* First element of points. */
            *L1 = MvarPtNew(1);

        L1 -> Pt[0] = Cv1 -> t0;

        if (LEBsctIsXSmaller(L1 -> Pt[0], PtStart -> Pt[0])) {
            IRIT_LIST_PUSH(L1, Points);
        }
        else {
            MvarPtFree(L1);
        }

        PtTarget = (MvarPtStruct *) CagdListLast(Points);

        R1 = MvarPtNew(1);
        R1 -> Pt[0] = Cv1 -> t1;

        if (LEBsctIsXSmaller(PtTarget -> Pt[0], R1 -> Pt[0])) {
            Tmp = MvarPtCopy(R1); 
            Tmp -> Pnext = NULL;
            CagdListAppend(Points, Tmp);
        }
        MvarPtFree(R1);

    }    
    /* Sanity check. */
    if (Points -> Pnext == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    /* Points list is now an ordered list from left to right */
    /* (endpoints at ends).                                  */

    /* Get all mid-points in points and compare curves at these points,   */
    /* Split the appropriate curve at the endpoints and insert into list. */
    IterPts = Points;
    IterPtsNext = IterPts -> Pnext;

    for ( ;
	 IterPtsNext != NULL;
	 IterPts=IterPts -> Pnext, IterPtsNext = IterPtsNext -> Pnext) {
        int Cv1IsYSmaller;
        MvarLECrvStruct 
	    *TrimmedCurve = NULL;
        Cv1IsYSmaller = LECv1IsYSmallerAt(Cv1, Cv2, 
					  0.5 * (IterPts -> Pt[0] +
						 IterPtsNext -> Pt[0]));

        if (Cv1IsYSmaller) {
	    LEBsctTrimCurveBetween(Cv1, IterPts -> Pt[0],
				   IterPtsNext -> Pt[0], &TrimmedCurve);
        }
        else {
            LEBsctTrimCurveBetween(Cv2, IterPts -> Pt[0],
				   IterPtsNext -> Pt[0], &TrimmedCurve);
        }

        IRIT_LIST_PUSH(TrimmedCurve, (*ResultEnvelope)); 
    }

    MvarPtFreeList(Points);

    /* Reverse the ResultEnvelope list (since we used IRIT_LIST_PUSH). */
    *ResultEnvelope = (MvarLECrvStruct *) CagdListReverse(*ResultEnvelope);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Performs the merging step of the Divide and Conquer lower envelope       *
*   algorithm. Given two lower envelopes over the same domain, computes      *
*   the mutual lower envelope by a list-merge-like algorithm.                * 
*                                                                            *
* PARAMETERS:                                                                *
*   Envelope1, Envelope2: input Envelopes.       		             *
*   LowerEnvelope: output list of curves representing the lower envelope.    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:               	            				     *
*****************************************************************************/
static void LEBsctMergeLowerEnvelopes(MvarLECrvStruct *Envelope1, 
				      MvarLECrvStruct *Envelope2, 
				      MvarLECrvStruct **LowerEnvelope)
{
    MvarLECrvStruct 
	*Aux = NULL,
        *Iter1 = NULL,
        *Iter2 = NULL;

    /* Split the envelopes at endpoints to deal only with totally */
    /* overlapping segments.                                      */
    MvarLECrvStruct 
        *SplitEnvelope1 = NULL,
        *SplitEnvelope2 = NULL;

    LEBsctSplitEnvelope1AtEnvelope2(Envelope1, Envelope2, &SplitEnvelope1);
    LEBsctSplitEnvelope1AtEnvelope2(Envelope2, Envelope1, &SplitEnvelope2);

    /* We now have only totally overlapping or totally non-overlapping */
    /* segments (except maybe at endpoints).                           */
    Iter1 = SplitEnvelope1;
    Iter2 = SplitEnvelope2;
    while (Iter1 != NULL && Iter2 != NULL) {
        MvarLECrvStruct 
	    *Cv1 = Iter1,
	    *Cv2 = Iter2;
        CagdRType
	    Ps1 = Cv1 -> t0,  /* Cv1 left point. */
            Ps2 = Cv2 -> t0;  /* Cv2 left point. */

        if (LEBsctIsXSmaller(Ps1, Ps2)) {
            /* Sanity check - Ps2 is not smaller than right point of Cv1 */
	    /* (or there is an overlap).                                 */
            if (LEBsctIsXSmaller(Ps2,Cv1 -> t1))
                MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

            /* Copy Cv1 to Lower envelope.*/
            Aux = LECrvCopy(Cv1); 
	    /* Aux is essential here (since we need a copy of Cv1). */
            IRIT_LIST_PUSH(Aux, (*LowerEnvelope));
            Iter1 = Iter1 -> Pnext;
        }
        else if (LEBsctIsXSmaller(Ps2,Ps1)) {
	    /* Sanity check - Ps1 is not smaller than right point of */
	    /* Cv2 (or there is an overlap).                         */
            if (LEBsctIsXSmaller(Ps1,Cv2 -> t1))
                MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

            /* Copy Cv2 to Lower envelope. */
            Aux = LECrvCopy(Cv2); 
	    /* Aux is essential here (since we need a copy of Cv2). */
            IRIT_LIST_PUSH(Aux, (*LowerEnvelope));
            Iter2 = Iter2 -> Pnext;
        }
        else {
            /* Curves totally overlap - compute Lower envelope of two */
	    /* overlapping curves. */
            MvarLECrvStruct 
	        *Temp1 = NULL,
	        *Temp2 = NULL;
            LEBsctComputeLowerEnvelopeOfOverlap(Cv1, Cv2, &Temp1);
            /* Pushing at the front of the result list (we maintain the */
	    /* reverse order...).                                       */
            while (Temp1 != NULL) {
                Temp2 = Temp1 -> Pnext;
                IRIT_LIST_PUSH(Temp1, (*LowerEnvelope));
                Temp1 = Temp2;
            }
            Iter1 = Iter1 -> Pnext;
            Iter2 = Iter2 -> Pnext;
        }
    }

    /* If there is a non-overlapping "tail" copy it to lower envelope. */
    if (Iter1 != NULL) {
        /* Copy Iter1-end to lower envelope using Iter2 as Temp2. */
        while (Iter1 != NULL) {
            Iter2 = Iter1 -> Pnext;
            Aux = LECrvCopy(Iter1); 
            IRIT_LIST_PUSH(Aux, (*LowerEnvelope));
            Iter1 = Iter2;
        }
    }
    else if (Iter2 != NULL) {
        /* Copy Iter2-end to lower envelope using Iter1 as Temp2. */
        while (Iter2 != NULL) {
            Iter1 = Iter2 -> Pnext;
            Aux = LECrvCopy(Iter2); 
            IRIT_LIST_PUSH(Aux, (*LowerEnvelope));
            Iter2 = Iter1;
        }
    }

    LECrvLstFree(SplitEnvelope1);
    LECrvLstFree(SplitEnvelope2);

    /* Reverse result list to maintain left to right order. */
    *LowerEnvelope = (MvarLECrvStruct*) CagdListReverse(*LowerEnvelope);

    /* A merging step for consecutive LECrvs with the same Orig.            */
    /* This reduces the size of the envelopes.                              */

    Iter1 = *LowerEnvelope;
    if (Iter1 -> Pnext == NULL)
        return;
    Iter2 = Iter1 -> Pnext;
    while (Iter2 != NULL) {
        if (Iter1 -> Orig == Iter2 -> Orig) {
            /* Merge them. */
            Iter1 -> t1 = Iter2 -> t1;
            Iter1 -> Pnext = Iter2 -> Pnext;
            LECrvFree(Iter2);
        }
        else {
            Iter1 = Iter1 -> Pnext;
        }

        Iter2 = Iter1 -> Pnext;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Implements the recursive lower envelope algorithm of the input LECrvs.   *
*   This auxilary function is called from the public function                *
*   MvarBsctComputeLowerEnvelope.                                            *
*									     *
* PARAMETERS:                                                                *
*   InputCurves:  A MvarLECrvStruct of monotone pieces.                      *
*   LowerEnvelope: A MvarLECrvStruct of lower envelope.		             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void:								     *
*****************************************************************************/
static void LEBsctComputeLowerEnvelopeAux(MvarLECrvStruct *InputCurves, 
					  MvarLECrvStruct **LowerEnvelope)
{
    int i, InputCurvesLength;
    MvarLECrvStruct 
        *Aux = NULL,
        *Curves1 = NULL,
        *Curves2 = NULL,
        *Envelope1 = NULL,
        *Envelope2 = NULL,
        *Iter = NULL;

    /* Sanity checks. */
    if (InputCurves == NULL)
        MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

    if (InputCurves -> Pnext == NULL) {
        (*LowerEnvelope) = LECrvCopy(InputCurves);
        return;
    }

    /* Else recurse. */
    InputCurvesLength = CagdListLength(InputCurves);

    /* Splitting the input curves. */
    Iter = InputCurves;
    for (i = 0; Iter != NULL; ++i, Iter = Iter -> Pnext) {
        Aux = LECrvCopy(Iter);
        if (i < InputCurvesLength/2) {
            IRIT_LIST_PUSH(Aux,Curves1);
        }
        else {
            IRIT_LIST_PUSH(Aux,Curves2);
        }
    }
    /* Reverse result lists to maintain left to right order. */
    Curves1 = (MvarLECrvStruct *) CagdListReverse(Curves1);
    Curves2 = (MvarLECrvStruct *) CagdListReverse(Curves2);

    LEBsctComputeLowerEnvelopeAux(Curves1, &Envelope1);
    LEBsctComputeLowerEnvelopeAux(Curves2, &Envelope2);
    LEBsctMergeLowerEnvelopes(Envelope1, Envelope2, LowerEnvelope);

    LECrvLstFree(Envelope1);
    LECrvLstFree(Envelope2);

    LECrvLstFree(Curves1);
    LECrvLstFree(Curves2);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Sets the state of the LE curve type support.                             *
*                                                                            *
* PARAMETERS:                                                                *
*   CrvType:   New curve type.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:      Old curve type.                                                *
*****************************************************************************/
static int LESetCrvType(int CrvType)
{
    int OldVal = GlblCrvTypeFlag;

    GlblCrvTypeFlag = CrvType;

    return OldVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given the monotone curves, compute the lower envelope. This is the main  M
* calling function.                                                          M
*									     M
*   The current implementation is an improved version that uses the auxilary M
* MvarLECrvStruct structure for efficiency and robustness (less solver	     M
* calls).								     M
*									     *
* PARAMETERS:                                                                M
*   InputCurves:    A MvarVoronoiCrvStruct of monotone pieces.               M
*   LowerEnvelope:  A MvarVoronoiCrvStruct of lower envelope.		     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void:								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   MvarBsctComputeLowerEnvelopeAux, MvarBsctComputeLowerEnvelopeOfOverlap   M
*   MvarBsctMergeLowerEnvelopes, MvarBsctSplitEnvelope1AtEnvelope2           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarBsctComputeLowerEnvelope					     M
*****************************************************************************/
void MvarBsctComputeLowerEnvelope(MvarVoronoiCrvStruct *InputCurves, 
				  MvarVoronoiCrvStruct **LowerEnvelope)
{
    /* Copying the input list and calling the                               */
    /* MvarBsctComputeLowerEnvelopeAux function which modifies its input    */
    /* curves.								    */
    MvarLECrvStruct
	*InputLECrvs = NULL,
	*Aux = NULL, 
	*LECrvsEnvelope = NULL;
    MvarVoronoiCrvStruct 
        *Iter = InputCurves;
    MvarPtStruct
	*Pt1 = MvarPtNew(2),
	*Pt2 = MvarPtNew(2);
    int OldFlagVal = LESetCrvType(MVAR_LE_VOR_CRV);

    /* Transform to LE crvs. */
    while (Iter != NULL) {
        CagdRType UMin, UMax, VMin, VMax;

        if (Iter -> Type == MV_CV_CV) {
            CagdSrfDomain(Iter -> F3, &UMin, &UMax, &VMin, &VMax);
        }
        else {
            /* Sanity check. */
            if (Iter -> Type != MV_CV_PT)
                MVAR_FATAL_ERROR(MVAR_ERR_UNDEFINE_ERR);

            CagdCrvDomain(Iter -> Crv2, &UMin, &UMax);
        }

        Aux = LECrvNew();
        Aux -> Orig = Iter;
        Aux -> t0 = UMin;
        Aux -> t1 = UMax;
        
        IRIT_LIST_PUSH(Aux, InputLECrvs);
        Iter = Iter -> Pnext;
    }

    LEBsctComputeLowerEnvelopeAux(InputLECrvs, &LECrvsEnvelope);

    /* Transform LECrvs back to VorCrvs. */
    Aux = LECrvsEnvelope;
    while (Aux != NULL) {
        Pt1 -> Pt[0] = Aux -> t0;
        Pt2 -> Pt[0] = Aux -> t1;
        Iter = NULL;
        /* Note the use of MvarBsctTrimCurveBetween on the original VorCrv   */
	/* (and not LECrv).						     */
        MvarBsctTrimCurveBetween((MvarVoronoiCrvStruct*) Aux -> Orig,
				 Pt1, Pt2, &Iter);
        IRIT_LIST_PUSH(Iter, (*LowerEnvelope));
        Aux = Aux -> Pnext;
    }
    LECrvLstFree(LECrvsEnvelope);
    LECrvLstFree(InputLECrvs);
    MvarPtFree(Pt1);
    MvarPtFree(Pt2);

    /* Reverse result list to maintain left to right order. */
    *LowerEnvelope = (MvarVoronoiCrvStruct *) CagdListReverse((*LowerEnvelope));

    LESetCrvType(OldFlagVal);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given monotone univariate function curves, and their domain, compute the M
* lower envelope. This is the main calling function for such curves.         M
*									     M
* PARAMETERS:                                                                M
*   InputCurves:  A CagdCrvStruct of univariate (non-rational) curves in R^1 M
*   LowerEnvelope: A CagdCrvStruct of lower envelope			     M
*                                                                            *
* RETURN VALUE:                                                              M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   MvarUniFuncsComputeLowerEnvelope					     M
*****************************************************************************/
void MvarUniFuncsComputeLowerEnvelope(CagdCrvStruct *InputCurves, 
				      CagdCrvStruct **LowerEnvelope)
{
    /* Copying the input list and calling the                               */
    /* MvarBsctComputeLowerEnvelopeAux function which modifies its input    */
    /* curves.								    */
    MvarLECrvStruct
	*InputLECrvs = NULL,
	*Aux = NULL, 
	*LECrvsEnvelope = NULL;
    CagdCrvStruct 
        *Iter = InputCurves;

    int OldFlagVal = LESetCrvType(MVAR_LE_UNI_FUNC_CRV);

    /* Transform to LE crvs. */
    while (Iter != NULL) {
        CagdRType UMin, UMax;

        /* Currently we support only B-spline (possibly add conversion to  */
	/* Bsplines here.						   */
        assert(Iter -> GType == CAGD_CBSPLINE_TYPE); 

        CagdCrvDomain(Iter, &UMin, &UMax);

        Aux = LECrvNew();
        Aux -> Orig = Iter;
        Aux -> t0 = UMin;
        Aux -> t1 = UMax;
        
        IRIT_LIST_PUSH(Aux, InputLECrvs);
        Iter = Iter -> Pnext;
    }

    LEBsctComputeLowerEnvelopeAux(InputLECrvs, &LECrvsEnvelope);

    /* Transform LECrvs back to trimmed CagdCrvs. */
    Aux = LECrvsEnvelope;
    while (Aux != NULL) {
        Iter = NULL;
        /* Note the use of MvarBsctTrimCurveBetween on the original UniCrv  */
	/* (and not LECrv).						    */
        Iter = CagdCrvRegionFromCrv((CagdCrvStruct *) Aux -> Orig,
				    Aux -> t0, Aux -> t1);
        IRIT_LIST_PUSH(Iter, (*LowerEnvelope));
        Aux = Aux -> Pnext;
    }
    LECrvLstFree(LECrvsEnvelope);
    LECrvLstFree(InputLECrvs);

    /* Reverse result list to maintain left to right order. */
    *LowerEnvelope = (CagdCrvStruct *) CagdListReverse((*LowerEnvelope));

    LESetCrvType(OldFlagVal);
}
